#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <json.h>
#include <process.h>
#include "state.h"
#include "run.h"

#define BUFFER_LENGTH 1024
#define MAX_FILE_SIZE 1000000

GlobalContext ctx;

static bool contest_is_running(void)
{
    return ctx.num_teams != 0;
}

static Team* validate_team_username(const char* username)
{
    for (int i = 0; i < ctx.num_teams; i++)
        if (strcmp(username, ctx.teams[i].username) == 0)
            return &ctx.teams[i];
    return NULL;
}

static bool validate_team_password(const Team* team, const char* password)
{
    return strcmp(password, team->password) == 0;
}

static Language* validate_language(const char* str)
{
    for (int i = 0; i < ctx.num_languages; i++)
        if (strcmp(str, ctx.languages[i].name) == 0)
            return &ctx.languages[i];
    return NULL;
}

static void* handle_client(void* vargp)
{
    Packet* packet;
    Packet* run_packet;
    Socket* client_socket;
    Run* run;
    PacketEnum result;
    const Team* team;
    const Language* language;
    char buf[BUFFER_LENGTH];
    int length;

    sprintf(buf, "%d", MAX_FILE_SIZE);

    client_socket = vargp;

    packet = socket_recv(client_socket, BUFFER_LENGTH);
    if (packet == NULL) {
        puts("Could not get client header");
        goto fail;
    }
    if (packet->id != PACKET_CLI_CLIENT && packet->id != PACKET_WEB_CLIENT) {
        puts("Client header is invalid");
        goto fail;
    }
    packet_destroy(packet);

    packet = packet_create((contest_is_running()) ? PACKET_CONTEST : PACKET_NO_CONTEST, strlen(buf)+1, buf);
    if (packet == NULL)
        goto fail;
    socket_send(client_socket, packet);
    packet_destroy(packet);

    team = NULL;
    if (contest_is_running()) {
        packet = socket_recv(client_socket, 1000);
        if (packet == NULL)
            goto fail;
        if (packet->id != PACKET_TEAM_VALIDATE_USERNAME)
            goto fail_packet;
        team = validate_team_username(packet->buffer);
        if (team == NULL) {
            packet_destroy(packet);
            packet = packet_create(PACKET_TEAM_VALIDATION_FAILED, 0, NULL);
            socket_send(client_socket, packet);
            goto fail_packet;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_TEAM_VALIDATION_SUCCESS, 0, NULL);
        if (packet == NULL)
            goto fail;
        socket_send(client_socket, packet);
        packet_destroy(packet);
        packet = socket_recv(client_socket, 1000);
        if (packet == NULL)
            goto fail;
        if (packet->id != PACKET_TEAM_VALIDATE_PASSWORD)
            goto fail_packet;
        if (!validate_team_password(team, packet->buffer)) {
            packet_destroy(packet);
            packet = packet_create(PACKET_TEAM_VALIDATION_FAILED, 0, NULL);
            socket_send(client_socket, packet);
            goto fail_packet;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_TEAM_VALIDATION_SUCCESS, 0, NULL);
        if (packet == NULL)
            goto fail;
        socket_send(client_socket, packet);
        packet_destroy(packet);
    }

    packet = socket_recv(client_socket, BUFFER_LENGTH);
    if (packet == NULL)
        goto fail;
    if (packet->id != PACKET_CODE_NAME_SEND)
        goto fail_packet;
    sprintf(buf, "%s", packet->buffer);
    packet_destroy(packet);

    packet = socket_recv(client_socket, BUFFER_LENGTH);
    if (packet == NULL)
        goto fail;
    if (packet->id != PACKET_LANGUAGE_VALIDATE)
        goto fail_packet;
    language = validate_language(packet->buffer);
    if (language == NULL) {
        packet_destroy(packet);
        packet = packet_create(PACKET_LANGUAGE_VALIDATION_FAILED, 0, NULL);
        socket_send(client_socket, packet);
        goto fail_packet;
    }
    packet_destroy(packet);

    packet = packet_create(PACKET_LANGUAGE_VALIDATION_SUCCESS, 0, NULL);
    if (packet == NULL)
        goto fail;
    socket_send(client_socket, packet);
    packet_destroy(packet);

    run_packet = socket_recv(client_socket, MAX_FILE_SIZE);
    if (packet == NULL)
        goto fail;

    run = run_create(buf, team->id, language->id, 0, run_packet->buffer, run_packet->length-1);
    run_enqueue(run);
    do {
        run_wait(run);
        switch (run->status) {
            case RUN_SUCCESS:
                result = PACKET_CODE_ACCEPTED;
                break;
            case RUN_COMPILATION_ERROR:
            case RUN_RUNTIME_ERROR:
            case RUN_TIME_LIMIT_EXCEEDED:
            case RUN_MEM_LIMIT_EXCEEDED:
            case RUN_WRONG_ANSWER:
            case RUN_SERVER_ERROR:
                result = PACKET_CODE_FAILED;
                break;
            default:
                result = PACKET_CODE_NOTIFICATION;
                break;
        }
        packet = packet_create(result, run->response_length, run->response);
        if (packet == NULL) {
            run_die(run);
            goto fail;
        }
        socket_send(client_socket, packet);
        run_post(run);
    } while (result == PACKET_CODE_NOTIFICATION);
    packet_destroy(run_packet);

    run_destroy(run);
    packet_destroy(packet);
    socket_destroy(client_socket);
    pthread_exit(NULL);

fail_packet:
    packet_destroy(packet);
fail:
    puts("Lost connection to client");
    socket_destroy(client_socket);
    pthread_exit(NULL);
}

static char* get_string(JsonObject* object, char* key)
{
    JsonValue* value;
    
    value = json_get_value(object, key);
    if (value == NULL)
        return NULL;
    if (json_get_type(value) != JTYPE_STRING)
        return NULL;

    return json_get_string(value);
}

static Socket* create_listen_socket(JsonObject* config)
{
    Socket* listen_socket;
    char* ip_str;
    char* port_str;
    
    ip_str = get_string(config, "ip");
    port_str = get_string(config, "port");
    if (port_str == NULL) {
        puts("Could not read port from config file");
        return NULL;
    }
    listen_socket = socket_create(ip_str, port_str, BIT_TCP);
    if (listen_socket == NULL) {
        puts("Could not create socket");
        return NULL;
    }

    if (!socket_bind(listen_socket)) {
        puts("Couldn't bind socket");
        printf("%d\n", socket_get_last_error());
        return NULL;
    }
    if (!socket_listen(listen_socket)) {
        puts("Couldn't listen");
        return NULL;
    }
    return listen_socket;
}

static void* server_daemon(void* vargp)
{
    Socket* listen_socket;
    Socket* client_socket;
    Packet* packet;
    JsonObject* config = vargp;
    pthread_t thread_id;
    while (!ctx.kill) {
        listen_socket = create_listen_socket(config);
        if (listen_socket == NULL) {
            puts("Listen socket is null");
            ctx.kill = true;
            break;
        }
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        if (client_socket == NULL) {
            if (!ctx.kill)
                puts("Client socket is null");
            continue;
        }
        pthread_create(&thread_id, NULL, handle_client, client_socket);
    }
    return NULL;
}

void read_teams(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i;

    value = json_get_value(config, "teams");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_ARRAY)
        return;
    array = json_get_array(value);
    ctx.num_teams = json_array_length(array);
    if (ctx.num_teams == 0)
        return;
    ctx.teams = malloc((ctx.num_teams+1) * sizeof(Team));
    ctx.teams[ctx.num_teams].id = 0;
    ctx.teams[ctx.num_teams].username = "no-team";
    ctx.teams[ctx.num_teams].password = "";
    for (i = 0; i < ctx.num_teams; i++) {
        ctx.teams[i].id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            puts("invalid teamname");
            exit(1);
        }
        object = json_get_object(value);
        if (object == NULL) {
            puts("could not get object in team");
            exit(1);
        }
        value = json_get_value(object, "username");
        if (value == NULL) {
            puts("Missing team username");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid teamname");
            exit(1);
        }
        string = json_get_string(value);
        ctx.teams[i].username = string;
        value = json_get_value(object, "password");
        if (value == NULL) {
            puts("Missing team password");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid teamname");
            exit(1);
        }
        string = json_get_string(value);
        ctx.teams[i].password = string;
    }
}

void read_languages(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i;

    value = json_get_value(config, "languages");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_ARRAY)
        return;
    array = json_get_array(value);
    ctx.num_languages = json_array_length(array);
    if (ctx.num_languages == 0)
        return;
    ctx.languages = malloc(ctx.num_languages * sizeof(Language));
    for (i = 0; i < ctx.num_languages; i++) {
        ctx.languages[i].id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            puts("invalid language");
            exit(1);
        }
        object = json_get_object(value);
        if (object == NULL) {
            puts("could not get object in language");
            exit(1);
        }
        ctx.languages[i].object = object;
        value = json_get_value(object, "language");
        if (value == NULL) {
            puts("Missing language name");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid language");
            exit(1);
        }
        string = json_get_string(value);
        ctx.languages[i].name = string;
        value = json_get_value(object, "extension");
        if (value == NULL || json_get_type(value) != JTYPE_STRING) {
            puts("Invalid language extension, defaulting to .txt");
            string = ".txt";
        }
        else {
            string = json_get_string(value);
        }
        ctx.languages[i].extension = string;
        value = json_get_value(object, "compile");
        if (value == NULL) {
            puts("Missing compile instruction");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid compile instruction");
            exit(1);
        }
        string = json_get_string(value);
        ctx.languages[i].compile = string;
        value = json_get_value(object, "execute");
        if (value == NULL) {
            puts("Missing execution instruction");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid execution instruction");
            exit(1);
        }
        string = json_get_string(value);
        ctx.languages[i].execute = string;
    }
}

void read_problems(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i;

    value = json_get_value(config, "problems");
    if (value == NULL) {
        puts("Could not find problemset");
        exit(1);
    }
    if (json_get_type(value) != JTYPE_ARRAY) {
        puts("Invalid type for problemset");
        exit(1);
    }
    array = json_get_array(value);
    ctx.num_problems = json_array_length(array);
    if (ctx.num_problems == 0) {
        puts("Empty problemset");
        exit(1);
    }
    ctx.problems = malloc(ctx.num_problems * sizeof(Problem));
    for (i = 0; i < ctx.num_problems; i++) {
        ctx.problems[i].id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            puts("invalid problem");
            exit(1);
        }
        object = json_get_object(value);
        if (object == NULL) {
            puts("could not get object in problem");
            exit(1);
        }
        ctx.problems[i].object = object;
        value = json_get_value(object, "name");
        if (value == NULL) {
            puts("Missing problem name");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid problem name");
            exit(1);
        }
        string = json_get_string(value);
        ctx.problems[i].name = string;
        value = json_get_value(object, "dir");
        if (value == NULL) {
            puts("Missing dir path");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid dir path");
            exit(1);
        }
        string = json_get_string(value);
        ctx.problems[i].dir = string;
        value = json_get_value(object, "validate");
        if (value == NULL) {
            puts("Missing validator path");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_STRING) {
            puts("Invalid validator path");
            exit(1);
        }
        string = json_get_string(value);
        ctx.problems[i].validate = string;
        value = json_get_value(object, "testcases");
        if (value == NULL) {
            puts("Missing testcases");
            exit(1);
        }
        if (json_get_type(value) != JTYPE_INT) {
            puts("Invalid number of testcases");
            exit(1);
        }
        ctx.problems[i].num_testcases = json_get_int(value);
        ctx.problems[i].time_limit = 2.0;
        ctx.problems[i].mem_limit = 256<<10;
    }
}

void read_num_run_threads(JsonObject* config)
{
    JsonValue* value;
    int i;
    value = json_get_value(config, "num_threads");
    ctx.num_run_threads = 1;
    if (value == NULL) {
        puts("Number of run threads missing, defaulting to 1");
        goto setup;
    }
    if (json_get_type(value) != JTYPE_INT) {
        puts("Invalid type for number of run threads");
        goto setup;
    }
    ctx.num_run_threads = json_get_int(value);

setup:
    ctx.run_threads = calloc(ctx.num_run_threads, sizeof(pthread_t));
    for (i = 0; i < ctx.num_run_threads; i++)
        pthread_create(&ctx.run_threads[i], NULL, run_daemon, NULL);
}

bool context_init(JsonObject* config)
{
    read_teams(config);
    read_languages(config);
    read_problems(config);
    read_num_run_threads(config);
    puts("Successfully initialized");
    return true;
}

void context_cleanup(void)
{
    for (int i = 0; i < ctx.num_run_threads; i++)
        pthread_join(ctx.run_threads[i], NULL);
    free(ctx.run_threads);
    free(ctx.teams);
    free(ctx.languages);
    free(ctx.problems);
}

int main(int argc, char** argv)
{
    JsonObject* config;
    pthread_t server_thread_id;
    int code;

    const int max_num_conn = 10;

    if (argc == 1) {
        puts("Must supply config file");
        return 1;
    }

    config = json_read(argv[1]);
    if (config == NULL) {
        printf("Could not read config file: %s\n", argv[1]);
        return 1;
    }

    if (!context_init(config))
        goto fail_config;

    if (!networking_init(max_num_conn))
        goto fail_context;

    pthread_create(&server_thread_id, NULL, server_daemon, config);

    code = 0;
    while (code != 1) {
        scanf("%d", &code);
    }

    ctx.kill = true;

    networking_cleanup();

    pthread_join(server_thread_id, NULL);

fail_context:
    context_cleanup();
fail_config:
    json_object_destroy(config);

    return 0;
}

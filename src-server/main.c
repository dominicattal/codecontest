#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <json.h>
#include "run.h"

#define BUFFER_LENGTH   128
#define MAX_FILE_SIZE 1000000

typedef struct {
    char* username;
    char* password;
} Team;

typedef struct {
    int num_teams, num_languages;
    Team* teams;
    char** languages;
} GlobalContext;

static GlobalContext ctx;

static bool contest_is_running(void)
{
    return ctx.num_teams != 0;
}

static bool validate_team_username(const char* username)
{
    for (int i = 0; i < ctx.num_teams; i++)
        if (strcmp(username, ctx.teams[i].username) == 0)
            return true;
    return false;
}

static bool validate_team_password(const char* password)
{
    for (int i = 0; i < ctx.num_teams; i++)
        if (strcmp(password, ctx.teams[i].password) == 0)
            return true;
    return false;
}

static bool validate_language(const char* str)
{
    for (int i = 0; i < ctx.num_languages; i++)
        if (strcmp(str, ctx.languages[i]) == 0)
            return true;
    return false;
}

static void* handle_client(void* vargp)
{
    Packet* packet;
    Packet* language_packet;
    Packet* code_packet;
    Socket* client_socket;
    PacketEnum result;
    char buf[BUFFER_LENGTH];
    char dummy = '\0';

    sprintf(buf, "%d", MAX_FILE_SIZE);

    client_socket = vargp;
    if (contest_is_running())
        packet = packet_create(PACKET_CONTEST, strlen(buf)+1, buf);
    else
        packet = packet_create(PACKET_NO_CONTEST, strlen(buf)+1, buf);
    if (packet == NULL)
        goto fail;
    socket_send(client_socket, packet);
    packet_destroy(packet);

    if (contest_is_running()) {
        packet = socket_recv(client_socket, 1000);
        if (packet == NULL)
            goto fail;
        if (packet->id != PACKET_TEAM_VALIDATE_USERNAME)
            goto fail_packet;
        if (!validate_team_username(packet->buffer)) {
            packet_destroy(packet);
            packet = packet_create(PACKET_TEAM_VALIDATION_FAILED, 1, &dummy);
            socket_send(client_socket, packet);
            goto fail_packet;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_TEAM_VALIDATION_SUCCESS, 1, &dummy);
        if (packet == NULL)
            goto fail;
        socket_send(client_socket, packet);
        packet_destroy(packet);
        packet = socket_recv(client_socket, 1000);
        if (packet == NULL)
            goto fail;
        if (packet->id != PACKET_TEAM_VALIDATE_PASSWORD)
            goto fail_packet;
        if (!validate_team_password(packet->buffer)) {
            packet_destroy(packet);
            packet = packet_create(PACKET_TEAM_VALIDATION_FAILED, 1, &dummy);
            socket_send(client_socket, packet);
            goto fail_packet;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_TEAM_VALIDATION_SUCCESS, 1, &dummy);
        if (packet == NULL)
            goto fail;
        socket_send(client_socket, packet);
        packet_destroy(packet);
    }

    language_packet = socket_recv(client_socket, BUFFER_LENGTH);
    if (language_packet == NULL)
        goto fail;
    if (language_packet->id != PACKET_LANGUAGE_VALIDATE) {
        packet_destroy(language_packet);
        goto fail;
    }
    if (!validate_language(language_packet->buffer)) {
        packet_destroy(language_packet);
        packet = packet_create(PACKET_LANGUAGE_VALIDATION_FAILED, 1, &dummy);
        socket_send(client_socket, packet);
        goto fail_packet;
    }

    packet = packet_create(PACKET_LANGUAGE_VALIDATION_SUCCESS, 1, &dummy);
    if (packet == NULL) {
        packet_destroy(language_packet);
        goto fail;
    }
    socket_send(client_socket, packet);
    packet_destroy(packet);

    code_packet = socket_recv(client_socket, MAX_FILE_SIZE);
    if (code_packet == NULL) {
        packet_destroy(language_packet);
        goto fail;
    }

    Run* run = run_create(language_packet->buffer, code_packet->buffer);
    run_enqueue(run);
    run_wait(run);
    result = (run->status == RUN_SUCCESS) ? PACKET_CODE_ACCEPTED : PACKET_CODE_FAILED;

    packet = packet_create(result, run->response_length+1, run->response);
    if (packet == NULL) {
        run_destroy(run);
        packet_destroy(language_packet);
        packet_destroy(code_packet);
        goto fail;
    }
    socket_send(client_socket, packet);

    run_destroy(run);
    packet_destroy(language_packet);
    packet_destroy(code_packet);
    packet_destroy(packet);
    socket_destroy(client_socket);
    return NULL;

fail_packet:
    packet_destroy(packet);
fail:
    puts("Lost connection to client");
    socket_destroy(client_socket);
    return NULL;
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
    listen_socket = socket_create(ip_str, port_str, BIT_TCP|BIT_BIND);
    if (listen_socket == NULL) {
        puts("Could not create socket");
        return NULL;
    }

    if (!socket_bind(listen_socket)) {
        puts("Couldn't bind socket");
        return NULL;
    }
    if (!socket_listen(listen_socket)) {
        puts("Couldn't listen");
        return NULL;
    }
    return listen_socket;
}

void read_teams(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i, n;

    value = json_get_value(config, "teams");
    if (value == NULL)
        return;
    if (json_get_type(value) != JTYPE_ARRAY)
        return;
    array = json_get_array(value);
    ctx.num_teams = json_array_length(array);
    if (ctx.num_teams == 0)
        return;
    ctx.teams = malloc(ctx.num_teams * sizeof(Team));
    for (i = 0; i < ctx.num_teams; i++) {
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
        n = strlen(string);
        ctx.teams[i].username = malloc((n+1) * sizeof(char));
        memcpy(ctx.teams[i].username, string, n+1);
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
        n = strlen(string);
        ctx.teams[i].password = malloc((n+1) * sizeof(char));
        memcpy(ctx.teams[i].password, string, n+1);
    }
}

bool context_init(JsonObject* config)
{
    read_teams(config);
    const char* language = "python3";
    ctx.num_languages = 1;
    ctx.languages = malloc(sizeof(char*));
    if  (ctx.languages == NULL)
        return false;
    ctx.languages[0] = malloc((strlen(language)+1) * sizeof(char));
    if (ctx.languages[0] == NULL) {
        free(ctx.languages);
        return false;
    }
    memcpy(ctx.languages[0], language, strlen(language)+1);

    if (ctx.num_teams == 0)
        puts("Contest is not running");
    else
        puts("Contest is running");

    return true;
}

void context_cleanup(void)
{
    int i;
    for (i = 0; i < ctx.num_teams; i++) {
        free(ctx.teams[i].username);
        free(ctx.teams[i].password);
    }
    free(ctx.teams);
    for (i = 0; i < ctx.num_languages; i++)
        free(ctx.languages[i]);
    free(ctx.languages);
}

int main(int argc, char** argv)
{
    Socket* client_socket;
    Socket* listen_socket;
    pthread_t thread_id;
    pthread_t run_thread_id;
    JsonObject* config;

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

    networking_init();
    pthread_create(&run_thread_id, NULL, run_daemon, NULL);
    pthread_create(&run_thread_id, NULL, run_daemon, NULL);

    while (1) {
        listen_socket = create_listen_socket(config);
        if (listen_socket == NULL)
            goto fail_cleanup;
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        pthread_create(&thread_id, NULL, handle_client, client_socket);
    }

    pthread_kill(run_thread_id, 1);

fail_cleanup:
    networking_cleanup();
    context_cleanup();
fail_config:
    json_object_destroy(config);

    return 0;
}

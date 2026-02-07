#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <json.h>
#include "main.h"
#include "run.h"

#define BUFFER_LENGTH 1024
#define MAX_FILE_SIZE 1000000

GlobalContext ctx;

void _log(LogLevel level, const char* fmt, const char* file, int line, ...) 
{
    va_list ap;
    static char* level_strs[NUM_LOG_LEVELS] = {
        "\033[31mFATAL\033[0m",
        "\033[38;2;255;165;0mCRITICAL\033[0m",
        "\033[33mWARNING\033[0m",
        "\033[32mINFO\033[0m",
        "\033[34mDEBUG\033[0m",
        "\033[35mDBQUERY\033[0m",
    };
    va_start(ap, line);
    printf("%-s \033[38;2;220;220;220m%s:%d\033[0m ", level_strs[level], file, line);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

bool _db_exec(char* query_fmt, const char* file, int line, ...)
{
    char* query;
    char* error;
    int query_length;
    bool res;
    va_list ap;
    va_start(ap, line);
    query_length = vsnprintf(NULL, 0, query_fmt, ap);
    va_end(ap);
    if (query_length < 0) {
        _log(WARNING, "Query length is zero", file, line);
        return false;
    }
    query = malloc((query_length+1) * sizeof(char));
    va_start(ap, line);
    vsnprintf(query, query_length+1, query_fmt, ap);
    va_end(ap);
    //_log(DBQUERY, query, file, line);
    res = sqlite3_exec(ctx.db, query, NULL, NULL, &error);
    if (res) _log(WARNING, "sqlite3 command failed: %s", file, line, error);
    sqlite3_free(error);
    free(query);
    return res;
}

bool find_dir(const char* path)
{
    DIR* dir;
    dir = opendir(path);
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

bool create_dir(const char* path)
{
    char* path_copy;
    char* up_one;
    int result, n;
    n = strlen(path);
    path_copy = malloc((n+1) * sizeof(char));
    snprintf(path_copy, n+1, "%s", path);
    up_one = dirname(path_copy);
    if (strcmp(up_one, ".") != 0) {
        result = create_dir(up_one);
        if (!result) {
            free(path_copy);
            return false;
        }
    }
    result = mkdir(path, 0777);
    if (result == 0 || errno == EEXIST || errno == EBADF) {
        free(path_copy);
        return true;
    }
    log(ERROR, "Crete dir failed: %s %d", path, errno);
    free(path_copy);
    return true;
}

bool create_file(const char* path, const char* code, int code_length)
{
    FILE* file;
    size_t ret;
    bool status = true;
    file = fopen(path, "w");
    if (file == NULL)
        return false;
    ret = fwrite(code, sizeof(char), code_length, file);
    if (ret < (size_t)code_length)
        status = false;
    fclose(file);
    return status;
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

static Problem* validate_problem(char c)
{
    for (int i = 0; i < ctx.num_problems; i++)
        if (c == ctx.problems[i].letter)
            return &ctx.problems[i];
    return NULL;
}

static void* handle_cli_client(void* vargp)
{
    Packet* packet;
    Packet* run_packet;
    Socket* client_socket;
    Run* run;
    PacketEnum result;
    const Team* team;
    const Language* language;
    const Problem* problem;
    char buf[BUFFER_LENGTH];
    bool async = false;

    sprintf(buf, "%d", MAX_FILE_SIZE);

    client_socket = vargp;

    packet = socket_recv(client_socket);
    if (packet == NULL) {
        log(ERROR, "Could not get client header");
        goto fail;
    }
    if (packet->id != PACKET_CLI_CLIENT && packet->id != PACKET_WEB_CLIENT) {
        log(ERROR, "Client header is invalid");
        goto fail;
    }
    packet_destroy(packet);

    packet = packet_create((contest_is_running()) ? PACKET_CONTEST : PACKET_NO_CONTEST, strlen(buf)+1, buf);
    if (packet == NULL) {
        log(ERROR, "Could not get contest packet");
        goto fail;
    }
    socket_send(client_socket, packet);
    packet_destroy(packet);

    team = NULL;
    if (contest_is_running()) {
        packet = socket_recv(client_socket);
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
        packet = socket_recv(client_socket);
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

    packet = socket_recv(client_socket);
    if (packet == NULL)
        goto fail;
    if (packet->id != PACKET_ASYNC)
        goto fail_packet;
    async = packet->buffer[0];
    packet_destroy(packet);

    packet = socket_recv(client_socket);
    if (packet == NULL)
        goto fail;
    if (packet->id != PACKET_CODE_NAME_SEND)
        goto fail_packet;
    sprintf(buf, "%s", packet->buffer);
    packet_destroy(packet);

    packet = socket_recv(client_socket);
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

    packet = socket_recv(client_socket);
    if (packet == NULL)
        goto fail;
    if (packet->id != PACKET_PROBLEM_VALIDATE)
        goto fail_packet;
    if (packet->buffer == NULL)
        goto fail_packet;
    problem = validate_problem(packet->buffer[0]);
    if (problem == NULL) {
        packet_destroy(packet);
        packet = packet_create(PACKET_PROBLEM_VALIDATION_FAILED, 0, NULL);
        socket_send(client_socket, packet);
        goto fail_packet;
    }
    packet_destroy(packet);

    packet = packet_create(PACKET_PROBLEM_VALIDATION_SUCCESS, 0, NULL);
    if (packet == NULL)
        goto fail;
    socket_send(client_socket, packet);
    packet_destroy(packet);

    run_packet = socket_recv(client_socket);
    if (run_packet == NULL)
        goto fail;
    if (run_packet->id != PACKET_CODE_SEND) {
        packet_destroy(run_packet);
        goto fail;
    }

    run = run_create(buf, team->id, language->id, problem->id, run_packet->buffer, run_packet->length-1, async);
    run_enqueue(run);
    if (async)
        goto success;
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
        goto fail;
    }
    socket_send(client_socket, packet);
    packet_destroy(run_packet);
    run_destroy(run);
    packet_destroy(packet);

success:
    socket_destroy(client_socket);
    return NULL;

fail_packet:
    packet_destroy(packet);
fail:
    log(INFO, "Lost connection to client");
    socket_destroy(client_socket);
    return NULL;
}

static Socket* cli_create_listen_socket(JsonObject* config)
{
    Socket* listen_socket;
    char* ip_str;
    char* port_str;
    ip_str = get_string(config, "ip");
    port_str = get_string(config, "cli_port");
    if (port_str == NULL) {
        log(ERROR, "Could not read cli_port from config file");
        return NULL;
    }
    listen_socket = socket_create(ctx.cli_net_ctx, ip_str, port_str, BIT_TCP);
    if (listen_socket == NULL) {
        log(ERROR, "Could not create cli listen socket");
        return NULL;
    }
    if (!socket_bind(listen_socket)) {
        log(ERROR, "Couldn't bind cli socket");
        socket_destroy(listen_socket);
        return NULL;
    }
    if (!socket_listen(listen_socket)) {
        log(ERROR, "Couldn't listen on cli socket");
        socket_destroy(listen_socket);
        return NULL;
    }
    return listen_socket;
}

static void* cli_server_daemon(void* vargp)
{
    Socket* listen_socket;
    Socket* client_socket;
    JsonObject* config = vargp;
    pthread_t thread_id;
    while (!ctx.kill) {
        listen_socket = cli_create_listen_socket(config);
        if (listen_socket == NULL) {
            log(FATAL, "Listen socket is null");
            ctx.kill = true;
            break;
        }
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        if (client_socket == NULL) {
            if (!ctx.kill)
                log(WARNING, "Client socket is null");
            continue;
        }
        pthread_create(&thread_id, NULL, handle_cli_client, client_socket);
        //pthread_join(thread_id, NULL);
    }
    return NULL;
}

static void* handle_web_client(void* vargp)
{
    Packet* packet;
    Packet* send_packet;
    Socket* client_socket;
    char* msg;
    bool closed = false;
    client_socket = vargp;

    log(INFO, "Connected to web client");

    while (!ctx.kill && !closed) {
        packet = socket_recv_web(client_socket);
        if (packet == NULL) {
            continue;
        }
        switch (packet->id) {
            case WEB_PACKET_TEXT:
                if (packet->buffer)
                    log(INFO, "Received text packet: %s", packet->buffer);
                else
                    log(WARNING, "Received text packet, but packet buffer null");
                break;
            case WEB_PACKET_PING:
                if (packet->length > 125) {
                    log(WARNING, "Invalid ping packet, ignoring");
                    break;
                }
                log(INFO, "Received ping");
                send_packet = packet_create(WEB_PACKET_PONG, packet->length, packet->buffer);
                socket_send_web(client_socket, send_packet);
                packet_destroy(send_packet);
                break;
            case WEB_PACKET_PONG:
                log(INFO, "Received pong");
                break;
            case WEB_PACKET_CLOSE:
                log(INFO, "Closing web socket connection");
                msg = "received close packet";
                send_packet = packet_create(WEB_PACKET_CLOSE, strlen(msg), msg); 
                socket_send_web(client_socket, send_packet);
                packet_destroy(send_packet);
                closed = true;
                break;
            default:
                log(WARNING, "Recveied weird web packet, ignoring");
                break;
        }
        packet_destroy(packet);
    }
    socket_destroy(client_socket);
    return NULL;
}

static Socket* web_create_listen_socket(JsonObject* config)
{
    Socket* listen_socket;
    char* ip_str;
    char* port_str;
    
    ip_str = get_string(config, "ip");
    port_str = get_string(config, "web_port");
    if (port_str == NULL) {
        log(ERROR, "Could not read web_port from config file");
        return NULL;
    }
    listen_socket = socket_create(ctx.web_net_ctx, ip_str, port_str, BIT_TCP);
    if (listen_socket == NULL) {
        log(ERROR, "Could not create web socket");
        return NULL;
    }

    if (!socket_bind(listen_socket)) {
        log(ERROR, "Couldn't bind socket");
        return NULL;
    }
    if (!socket_listen(listen_socket)) {
        log(ERROR, "Couldn't listen");
        return NULL;
    }
    return listen_socket;
}

static void* web_server_daemon(void* vargp)
{
    Socket* listen_socket;
    Socket* client_socket;
    JsonObject* config = vargp;
    pthread_t thread_id;

    while (!ctx.kill) {
        listen_socket = web_create_listen_socket(config);
        if (listen_socket == NULL) {
            log(FATAL, "Listen socket is null");
            ctx.kill = true;
            break;
        }
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        if (client_socket == NULL) {
            if (!ctx.kill)
                log(WARNING, "Client socket is null");
            continue;
        }
        socket_web_handshake(client_socket);
        // should check if client is already connected before creating thread
        pthread_create(&thread_id, NULL, handle_web_client, client_socket);
    }
    return NULL;
}

bool read_teams(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i;

    value = json_get_value(config, "teams");
    if (value == NULL)
        return false;
    if (json_get_type(value) != JTYPE_ARRAY) {
        return false;
    }
    array = json_get_array(value);
    ctx.num_teams = json_array_length(array);
    if (ctx.num_teams == 0) {
        log(ERROR, "There are no teams");
        return false;
    }
    ctx.teams = malloc(ctx.num_teams * sizeof(Team));
    for (i = 0; i < ctx.num_teams; i++) {
        ctx.teams[i].id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            log(ERROR, "invalid teamname");
            return false;
        }
        object = json_get_object(value);
        if (object == NULL) {
            log(ERROR, "could not get object in team");
            return false;
        }
        value = json_get_value(object, "username");
        if (value == NULL) {
            log(ERROR, "Missing team username");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid teamname");
            return false;
        }
        string = json_get_string(value);
        ctx.teams[i].username = string;
        value = json_get_value(object, "password");
        if (value == NULL) {
            log(ERROR, "Missing team password");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid teamname");
            return false;
        }
        string = json_get_string(value);
        ctx.teams[i].password = string;
    }
    return true;
}

bool read_languages(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    const char* string;
    int i;

    value = json_get_value(config, "languages");
    if (value == NULL)
        return false;
    if (json_get_type(value) != JTYPE_ARRAY)
        return false;
    array = json_get_array(value);
    ctx.num_languages = json_array_length(array);
    if (ctx.num_languages == 0)
        return false;
    ctx.languages = malloc(ctx.num_languages * sizeof(Language));
    for (i = 0; i < ctx.num_languages; i++) {
        ctx.languages[i].id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            log(ERROR, "invalid language");
            return false;
        }
        object = json_get_object(value);
        if (object == NULL) {
            log(ERROR, "could not get object in language");
            return false;
        }
        ctx.languages[i].object = object;
        value = json_get_value(object, "language");
        if (value == NULL) {
            log(ERROR, "Missing language name");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid language");
            return false;
        }
        string = json_get_string(value);
        ctx.languages[i].name = string;
        value = json_get_value(object, "extension");
        if (value == NULL || json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid language extension, defaulting to .txt");
            string = ".txt";
        }
        else {
            string = json_get_string(value);
        }
        ctx.languages[i].extension = string;
        value = json_get_value(object, "compile");
        if (value == NULL) {
            log(ERROR, "Missing compile instruction");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid compile instruction");
            return false;
        }
        string = json_get_string(value);
        ctx.languages[i].compile = string;
        value = json_get_value(object, "execute");
        if (value == NULL) {
            log(ERROR, "Missing execution instruction");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid execution instruction");
            return false;
        }
        string = json_get_string(value);
        ctx.languages[i].execute = string;
    }
    return true;
}

bool read_testcases(Problem* problem)
{
    DIR* dir;
    struct dirent* testcase_file;
    Testcase* testcase;
    int len, id;

    problem->testcases = NULL;
    dir = opendir(problem->testcases_dir);
    if (dir == NULL) {
        log(ERROR, "Could not read testcases directory %s", problem->testcases_dir);
        return false;
    }
    testcase_file = readdir(dir);
    if (testcase_file == NULL)
        log(WARNING, "Could not get testcase file for testcases directory %s", problem->testcases_dir);
    problem->num_testcases = 0;
    while (testcase_file != NULL) {
        len = strlen(testcase_file->d_name);
        if (len >= 3 && strcmp(testcase_file->d_name + len - 3, ".in") == 0)
            problem->num_testcases++;
        testcase_file = readdir(dir);
    }
    rewinddir(dir);
    if (problem->num_testcases == 0) {
        log(WARNING, "No testcases for problem %c-%s", problem->letter, problem->name);
        closedir(dir);
        return true;
    }
    problem->testcases = malloc(problem->num_testcases * sizeof(Testcase));
    id = 0;
    testcase_file = readdir(dir);
    while (testcase_file != NULL) {
        len = strlen(testcase_file->d_name);
        if (len >= 3 && strcmp(testcase_file->d_name + len - 3, ".in") == 0) {
            testcase = problem->testcases + id;
            testcase->name = malloc((len-3+1) * sizeof(char));
            snprintf(testcase->name, len-3+1, "%s", testcase_file->d_name);
            testcase->id = id; 
            id++;
        }
        testcase_file = readdir(dir);
    }
    closedir(dir);
    return true;
}

bool read_problems(JsonObject* config)
{
    JsonObject* object;
    JsonValue* value;
    JsonArray* array;
    Problem* problem;
    const char* string;
    int i;

    value = json_get_value(config, "problems");
    if (value == NULL) {
        log(ERROR, "Could not find problemset");
        return false;
    }
    if (json_get_type(value) != JTYPE_ARRAY) {
        log(ERROR, "Invalid type for problemset");
        return false;
    }
    array = json_get_array(value);
    ctx.num_problems = json_array_length(array);
    if (ctx.num_problems == 0) {
        log(ERROR, "Empty problemset");
        return false;
    }
    ctx.problems = malloc(ctx.num_problems * sizeof(Problem));
    for (i = 0; i < ctx.num_problems; i++) {
        problem = &ctx.problems[i];
        problem->id = i;
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_OBJECT) {
            log(ERROR, "invalid problem");
            return false;
        }
        object = json_get_object(value);
        if (object == NULL) {
            log(ERROR, "could not get object in problem");
            return false;
        }
        problem->object = object;
        value = json_get_value(object, "letter");
        if (value == NULL) {
            log(ERROR, "Missing problem letter");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid problem letter - invalid type");
            return false;
        }
        string = json_get_string(value);
        if (strlen(string) != 1) {
            log(ERROR, "Invalid problem letter - too many letters");
            return false;
        }
        problem->letter = string[0];
        value = json_get_value(object, "name");
        if (value == NULL) {
            log(ERROR, "Missing problem name");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid problem name");
            return false;
        }
        string = json_get_string(value);
        problem->name = string;
        value = json_get_value(object, "html");
        if (value == NULL) {
            log(WARNING, "Missing problem html");
            string = "no.html";
        }
        else if (json_get_type(value) != JTYPE_STRING) {
            log(WARNING, "Invalid problem html");
            string = "no.html";
        } else {
            string = json_get_string(value);
        }
        problem->html = string;
        value = json_get_value(object, "dir");
        if (value == NULL) {
            log(ERROR, "Missing dir path");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid dir path");
            return false;
        }
        string = json_get_string(value);
        problem->dir = string;
        value = json_get_value(object, "validate");
        if (value == NULL) {
            log(ERROR, "Missing validator path");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid validator path");
            return false;
        }
        string = json_get_string(value);
        problem->validate = string;
        value = json_get_value(object, "testcases");
        if (value == NULL) {
            log(ERROR, "Missing testcases dir");
            return false;
        }
        if (json_get_type(value) != JTYPE_STRING) {
            log(ERROR, "Invalid testcases path");
            return false;
        }
        string = json_get_string(value);
        problem->testcases_dir = string;
        if (!read_testcases(problem)) {
            log(ERROR, "Failed to read testcases for problem %s", problem->name);
            return false;
        }
        value = json_get_value(object, "pipe");
        if (value == NULL) {
            log(WARNING, "Missing pipe field, defaulting to no pipe for problem %c (%s)", problem->letter, problem->name);
            problem->pipe = false;
        } else {
            problem->pipe = json_get_type(value) == JTYPE_TRUE;
        }
        value = json_get_value(object, "time_limit");
        if (value == NULL) {
            log(ERROR, "Missing time_limit");
            return false;
        }
        if (json_get_type(value) != JTYPE_INT) {
            log(ERROR, "Invalid time_limit");
            return false;
        }
        problem->time_limit = json_get_int(value);
        value = json_get_value(object, "mem_limit");
        if (value == NULL) {
            log(ERROR, "Missing mem_limit");
            return false;
        }
        if (json_get_type(value) != JTYPE_INT) {
            log(ERROR, "Invalid mem_limit");
            return false;
        }
        problem->mem_limit = json_get_int(value);
    }
    return true;
}

bool read_num_run_threads(JsonObject* config)
{
    JsonValue* value;
    int i;
    value = json_get_value(config, "num_threads");
    ctx.num_run_threads = 1;
    if (value == NULL) {
        log(WARNING, "Number of run threads missing, defaulting to 1");
        goto setup;
    }
    if (json_get_type(value) != JTYPE_INT) {
        log(WARNING, "Invalid type for number of run threads");
        goto setup;
    }
    ctx.num_run_threads = json_get_int(value);

setup:
    ctx.run_threads = calloc(ctx.num_run_threads, sizeof(pthread_t));
    for (i = 0; i < ctx.num_run_threads; i++)
        pthread_create(&ctx.run_threads[i], NULL, run_daemon, NULL);

    return true;
}

bool context_init(JsonObject* config)
{
    if (!read_teams(config)) {
        log(ERROR, "Could not read teams");
        return false;
    }
    if (!read_languages(config)) {
        log(ERROR, "Could not read languages");
        return false;
    }
    if (!read_problems(config)) {
        log(ERROR, "Could not read problems");
        return false;
    }
    if (!read_num_run_threads(config)) {
        log(ERROR, "Could not read num threads");
        return false;
    }
    ctx.num_runs = 0;
    log(INFO, "Successfully initialized config");
    return true;
}

void context_cleanup(void)
{
    for (int i = 0; i < ctx.num_run_threads; i++)
        pthread_join(ctx.run_threads[i], NULL);
    for (int i = 0; i < ctx.num_problems; i++) {
        Problem* problem = &ctx.problems[i];
        for (int j = 0; j < problem->num_testcases; j++)
            free(problem->testcases[j].name);        
        free(problem->testcases);
    }
    free(ctx.problems);
    free(ctx.run_threads);
    free(ctx.teams);
    free(ctx.languages);
}

bool db_init(JsonObject* config)
{
    Problem* problem;
    char* db_file_path;
    char* query_fmt;
    char* db_dirname;
    int i, res, len;
    if (!sqlite3_threadsafe()) {
        log(ERROR, "sqlite3 must be threadsafe");
        return false;
    }
    db_file_path = get_string(config, "database");
    if (db_file_path == NULL) {
        log(ERROR, "Missing database file in config");
        return false;
    }
    len = strlen(db_file_path);
    db_dirname = malloc((len+1) * sizeof(char));
    snprintf(db_dirname, len+1, "%s", db_file_path);
    dirname(db_dirname);
    create_dir(db_dirname);
    free(db_dirname);
    res = sqlite3_open(db_file_path, &ctx.db);
    if (res) {
        log(ERROR, "Opening database failed");
        return false;
    }
    log(INFO, "Creating db file at %s", db_file_path);
    db_exec(
        "CREATE TABLE runs ("
        "    id INT PRIMARY KEY,"
        "    team_id INT,"
        "    problem_id INT,"
        "    language_id INT,"
        "    testcase INT,"
        "    status INT,"
        "    timestamp TEXT,"
        "    time INT,"
        "    memory INT"
        ");"
        ""
        "CREATE TABLE teams ("
        "    id INT PRIMARY KEY,"
        "    username TEXT,"
        "    password TEXT"
        ");"
        ""
        "CREATE TABLE languages ("
        "    id INT PRIMARY KEY,"
        "    name TEXT"
        ");"
        ""
        "CREATE TABLE problems ("
        "    id INT PRIMARY KEY,"
        "    letter TEXT,"
        "    name TEXT,"
        "    html_path TEXT,"
        "    time_limit INT,"
        "    mem_limit INT"
        ");");
    db_exec( "DELETE FROM runs;DELETE FROM teams;DELETE FROM languages;DELETE FROM problems;");
    for (i = 0; i < ctx.num_languages; i++) {
        query_fmt = "INSERT INTO languages (id, name) VALUES (%d, '%s');";
        db_exec(query_fmt, ctx.languages[i].id, ctx.languages[i].name);
    }
    for (i = 0; i < ctx.num_problems; i++) {
        problem = &ctx.problems[i];
        query_fmt = "INSERT INTO problems (id, letter, name, html_path, time_limit, mem_limit) VALUES (%d, '%c', '%s', '%s', %d, %d);";
        db_exec(query_fmt, problem->id, problem->letter, problem->name, problem->html, problem->time_limit, problem->mem_limit);
    }
    for (i = 0; i < ctx.num_teams; i++) {
        query_fmt = "INSERT INTO teams (id, username, password) VALUES (%d, '%s', '%s');";
        db_exec(query_fmt, ctx.teams[i].id, ctx.teams[i].username, ctx.teams[i].password);
    }
    return true;
}

void db_cleanup(void)
{
    sqlite3_close(ctx.db);
}

int main(int argc, char** argv)
{
    JsonObject* config;
    pthread_t cli_server_thread_id;
    pthread_t web_server_thread_id;
    int code;

    if (argc == 1) {
        log(FATAL, "Must supply config file");
        return 1;
    }

    // two options: load from database or load from config
    // for now, just load from config and overwite database

    config = json_read(argv[1]);
    if (config == NULL) {
        log(FATAL, "Could not read config file: %s", argv[1]);
        return 1;
    }

    if (!context_init(config)) {
        log(FATAL, "Could not initialize context");
        goto fail_config;
    }

    if (!db_init(config)) {
        log(FATAL, "Could not initialize database");
        goto fail_context;
    }

    ctx.cli_net_ctx = networking_init();
    if (ctx.cli_net_ctx == NULL) {
        log(FATAL, "Could not make cli server");
        goto fail_db;
    }

    ctx.web_net_ctx = networking_init();
    if (ctx.web_net_ctx == NULL) {
        log(FATAL, "Could not make cli server");
        goto fail_db;
    }

    pthread_create(&cli_server_thread_id, NULL, cli_server_daemon, config);
    pthread_create(&web_server_thread_id, NULL, web_server_daemon, config);

    code = 0;
    while (code != 1) {
        scanf("%d", &code);
    }

    ctx.kill = true;

    networking_shutdown_sockets(ctx.cli_net_ctx);
    networking_shutdown_sockets(ctx.web_net_ctx);

    pthread_join(cli_server_thread_id, NULL);
    pthread_join(web_server_thread_id, NULL);

    networking_cleanup(ctx.cli_net_ctx);
    networking_cleanup(ctx.web_net_ctx);

fail_db:
    ctx.kill = true;
    db_cleanup();
fail_context:
    ctx.kill = true;
    context_cleanup();
fail_config:
    ctx.kill = true;
    json_object_destroy(config);

    return 0;
}

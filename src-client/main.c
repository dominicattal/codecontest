#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <networking.h>
#include <json.h>
#include <getopt.h>

#define BUFFER_LENGTH_SMALL 100
#define BUFFER_LENGTH_BIG   10000

char* get_json_string(JsonObject* config, const char* key)
{
    JsonValue* value;
    value = json_get_value(config, key);
    if (value == NULL) {
        printf("Could not get value of %s\n", key);
        return NULL;
    }
    if (json_get_type(value) != JTYPE_STRING) {
        printf("Invalid %s\n", key);
        return NULL;
    }
    return json_get_string(value);
}

char* read_code_file(const char* path)
{
    FILE* file;
    char* code;
    int end, length;
    file = fopen(path, "r");
    if (file == NULL)
        goto fail;
    if (fseek(file, 0, SEEK_END) != 0)
        goto fail_close_file;
    end = ftell(file);
    if (end == -1L)
        goto fail_close_file;
    code = malloc((end+1)*sizeof(char));
    if (code == NULL)
        goto fail_close_file;
    if (fseek(file, 0, SEEK_SET) != 0)
        goto fail_free_buffer;
    length = fread(code, sizeof(char), end, file);
    if (ferror(file) != 0)
        goto fail_free_buffer;
    fclose(file);
    code[length] = '\0';
    return code;

fail_free_buffer:
    free(code);
fail_close_file:
    fclose(file);
fail:
    return NULL;
}

int main(int argc, char** argv)
{
    JsonObject* config = NULL;
    Socket* server_socket;
    Packet* packet;
    int max_file_size;
    char* file_basename;
    char*  file_name;
    char* config_path = NULL;
    char* ip_str = NULL;
    char* port_str = NULL;
    char* username = NULL;
    char* password = NULL;
    char* problem = NULL;
    char* language = NULL;
    char* file = NULL;
    const char* config_path_key = "config";
    const char* ip_str_key = "ip";
    const char* port_str_key = "port";
    const char* username_key = "username";
    const char* password_key = "password";
    const char* problem_key = "problem";
    const char* language_key = "language";
    const char* file_key = "file";
    const char* async_key = "async";
    char* code;
    int i, option_idx;
    char c;
    bool contest_running;
    char async = false;

    struct option long_options[] = {
        {config_path_key,       required_argument,  NULL, 'c'},
        {ip_str_key,            required_argument,  NULL, 'i'},
        {port_str_key,          required_argument,  NULL, 't'},
        {username_key,          required_argument,  NULL, 'u'},
        {password_key,          required_argument,  NULL, 'p'},
        {problem_key,           required_argument,  NULL, 'b'},
        {language_key,          required_argument,  NULL, 'l'},
        {file_key,    required_argument,  NULL, 'f'},
        {async_key,             no_argument,        NULL, 'a'},
        {NULL, 0, NULL, 0}
    };

    option_idx = 0;
    while ((c = getopt_long(argc, argv, "c:i:t:u:p:l:f:ab:", long_options, &option_idx)) != -1) {
        switch (c) {
            case 'c':
                config_path = optarg;
                break;
            case 'i':
                ip_str = optarg;
                break;
            case 't':
                port_str = optarg;
                break;
            case 'u':
                username = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'b':
                problem = optarg;
                break;
            case 'l':
                language = optarg;
                break;
            case 'f':
                file = optarg;
                break;
            case 'a':
                async = true;
                break;
            default:
                break;
        }
    }

    if (config_path != NULL) {
        config = json_read(config_path);
        if (config == NULL) {
            printf("Could not read config path: %s\n", config_path);
            goto fail_config;
        }
    }
    
    if (ip_str == NULL) {
        if (config == NULL) goto missing_ip_str;
        ip_str = get_json_string(config, ip_str_key);
        if (ip_str == NULL) goto missing_ip_str;
        goto found_ip_str;
missing_ip_str:
        printf("Missing ip string\n");
        goto fail_config;
    }
found_ip_str:

    if (port_str == NULL) {
        if (config == NULL) goto missing_port_str;
        port_str = get_json_string(config, port_str_key);
        if (port_str == NULL) goto missing_port_str;
        goto found_port_str;
missing_port_str:
        printf("Missing port string\n");
        goto fail_config;
    }
found_port_str:

    if (username == NULL) {
        if (config == NULL) goto missing_username;
        username = get_json_string(config, username_key);
        if (username == NULL) goto missing_username;
        goto found_username;
missing_username:
        printf("Missing username string\n");
        goto fail_config;
    }
found_username:

    if (password == NULL) {
        if (config == NULL) goto missing_password;
        password = get_json_string(config, password_key);
        if (password == NULL) goto missing_password;
        goto found_password;
missing_password:
        printf("Missing password string\n");
        goto fail_config;
    }
found_password:

    if (language == NULL) {
        if (config == NULL) goto missing_language;
        language = get_json_string(config, language_key);
        if (language == NULL) goto missing_language;
        goto found_language;
missing_language:
        printf("Missing language string\n");
        goto fail_config;
    }
found_language:

    if (problem == NULL) {
        if (config == NULL) goto missing_problem;
        problem = get_json_string(config, problem_key);
        if (problem == NULL) goto missing_problem;
        goto found_problem;
missing_problem:
        printf("Missing problem string\n");
        goto fail_config;
    }
found_problem:

    if (file == NULL) {
        if (config == NULL) goto missing_file;
        file = get_json_string(config, file_key);
        if (file == NULL) goto missing_file;
        goto found_file;
missing_file:
        printf("Missing file string\n");
        goto fail_config;
    }
found_file:

    if (config != NULL && json_get_value(config, async_key) != NULL)
        async = json_get_type(json_get_value(config, async_key)) == JTYPE_TRUE;

    code = read_code_file(file);
    if (code == NULL) {
        printf("Could not read code file: %s\n", file);
        goto fail_config;
    }

    file_basename = basename(file);
    file_name = malloc((strlen(file_basename)+1) * sizeof(char));
    for (i = 0; file_basename[i] != '\0' && file_basename[i] != '.'; i++)
        file_name[i] = file_basename[i];
    file_name[i] = '\0';

    networking_init(1);
    
    server_socket = socket_create(ip_str, port_str, BIT_TCP);
    if (!socket_connect(server_socket)) {
        puts("Could not connect");
        goto fail_destroy_socket;
    }

    packet = packet_create(PACKET_CLI_CLIENT, 0, NULL);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (!socket_send(server_socket, packet)) {
        puts("Could not send preliminary packet");
        goto fail_destroy_socket;
    }
    packet_destroy(packet);

    packet = socket_recv(server_socket);
    contest_running = packet->id == PACKET_CONTEST;
    if (packet->id == PACKET_CONTEST)
        puts("contest is running");
    else
        puts("contest is not running");
    max_file_size = atoi(packet->buffer);
    packet_destroy(packet);
    if (max_file_size <= 0) {
        puts("something went wrong");
        goto fail_destroy_socket;
    }

    if (contest_running) {
        packet = packet_create(PACKET_TEAM_VALIDATE_USERNAME, strlen(username)+1, username);
        if (packet == NULL)
            goto fail_destroy_socket;
        if (!socket_send(server_socket, packet)) {
            puts("Could not validate team username");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
        packet = socket_recv(server_socket);
        if (packet->id != PACKET_TEAM_VALIDATION_SUCCESS) {
            puts("Unrecognized team name");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_TEAM_VALIDATE_PASSWORD, strlen(password)+1, password);
        if (packet == NULL)
            goto fail_destroy_socket;
        if (!socket_send(server_socket, packet)) {
            puts("Could not validate team password");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
        packet = socket_recv(server_socket);
        if (packet->id != PACKET_TEAM_VALIDATION_SUCCESS) {
            puts("Incorrect team password");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
    }

    packet = packet_create(PACKET_ASYNC, 1, &async);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (!socket_send(server_socket, packet)) {
        puts("Could not send code name");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    packet = packet_create(PACKET_CODE_NAME_SEND, strlen(file_name)+1, file_name);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (!socket_send(server_socket, packet)) {
        puts("Could not send code name");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    packet = packet_create(PACKET_LANGUAGE_VALIDATE, strlen(language)+1, language);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (!socket_send(server_socket, packet)) {
        puts("Could not validate language");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    packet = socket_recv(server_socket);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (packet->id != PACKET_LANGUAGE_VALIDATION_SUCCESS) {
        puts("Language validation failed");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    packet = packet_create(PACKET_CODE_SEND, strlen(code)+1, code);
    if (!socket_send(server_socket, packet)) {
        puts("unsuccessful");
        goto fail_destroy_packet;
    }
    puts("sent code");
    packet_destroy(packet);

    if (async) goto success;

    do {
        packet = socket_recv(server_socket);
        if (packet == NULL)
            goto fail_destroy_socket;
        switch (packet->id) {
            case PACKET_CODE_ACCEPTED:
                puts("Accepted");
                break;
            case PACKET_CODE_FAILED:
                printf("Failed: %s\n", packet->buffer);
                break;
            case PACKET_CODE_NOTIFICATION:
                puts(packet->buffer);
                break;
            default:
                puts("Unexpected packet received, ignoring it");
                break;
        }
        packet_destroy(packet);
    } while (1);
    packet_destroy(packet);

success:
    socket_destroy(server_socket);
    networking_cleanup();
    free(code);
    free(file_name);
    if (config != NULL)
        json_object_destroy(config);
    return 0;

fail_destroy_packet:
    packet_destroy(packet);
fail_destroy_socket:
    socket_destroy(server_socket);
    networking_cleanup();
    free(code);
    free(file_name);
fail_config:
    if (config != NULL)
        json_object_destroy(config);
    return 1;
}

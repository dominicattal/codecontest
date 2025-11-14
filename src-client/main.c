#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <networking.h>
#include <json.h>

const char* get_json_string(JsonObject* config, const char* key)
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
    puts(code);
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
    JsonObject* config;
    Socket* server_socket;
    Packet* packet;
    int max_file_size;
    const char* ip_str;
    const char* port_str;
    const char* username;
    const char* password;
    const char* language;
    char* code;
    bool contest_running;

    if (argc != 4) {
        puts("usage: ./client [config] [language] [code_file_path]");
        return 1;
    }

    config = json_read(argv[1]);
    if (config == NULL) {
        printf("Could not read config file: %s\n", argv[1]);
        return 1;
    }
    
    ip_str = get_json_string(config, "ip");
    if (ip_str == NULL)
        goto fail_config;

    port_str = get_json_string(config, "port");
    if (ip_str == NULL)
        goto fail_config;

    username = get_json_string(config, "username");
    if (ip_str == NULL)
        goto fail_config;

    password = get_json_string(config, "password");
    if (ip_str == NULL)
        goto fail_config;

    language = argv[2];

    code = read_code_file(argv[3]);
    if (code == NULL) {
        printf("Could not read code file: %s\n", argv[3]);
        goto fail_config;
    }

    networking_init();
    
    server_socket = socket_create(ip_str, port_str, BIT_TCP);
    if (!socket_connect(server_socket)) {
        puts("Could not connect");
        goto fail_destroy_socket;
    }

    packet = socket_recv(server_socket, 100);
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
        packet = socket_recv(server_socket, 100);
        if (packet->id != PACKET_TEAM_VALIDATION_SUCCESS) {
            puts("Unrecognized team name");
            goto fail_destroy_packet;
        }
        packet = packet_create(PACKET_TEAM_VALIDATE_PASSWORD, strlen(password)+1, password);
        if (packet == NULL)
            goto fail_destroy_socket;
        if (!socket_send(server_socket, packet)) {
            puts("Could not validate team password");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
        packet = socket_recv(server_socket, 100);
        if (packet->id != PACKET_TEAM_VALIDATION_SUCCESS) {
            puts("Incorrect team password");
            goto fail_destroy_packet;
        }
        puts("validation successful");
        packet_destroy(packet);
    }

    packet = packet_create(PACKET_LANGUAGE_VALIDATE, strlen(language)+1, language);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (!socket_send(server_socket, packet)) {
        puts("Could not validate language");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    packet = socket_recv(server_socket, 10000);
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

    packet = socket_recv(server_socket, 10000);
    if (packet == NULL)
        goto fail_destroy_socket;
    if (packet->id == PACKET_CODE_ACCEPTED)
        puts("Accepted");
    else
        printf("Failed: %s\n", packet->buffer);

    packet_destroy(packet);
    socket_destroy(server_socket);
    networking_cleanup();
    free(code);
    json_object_destroy(config);
    return 0;

fail_destroy_packet:
    packet_destroy(packet);
fail_destroy_socket:
    socket_destroy(server_socket);
    networking_cleanup();
    free(code);
fail_config:
    json_object_destroy(config);
    return 1;
}

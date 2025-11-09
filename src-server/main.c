#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <json.h>

#define MAX_FILE_SIZE 1000000

typedef struct {
    int num_teams;
    char** teams;
} GlobalContext;

static GlobalContext ctx;

static int validate_team(const char* str)
{
    for (int i = 0; i < ctx.num_teams; i++)
        if (strcmp(str, ctx.teams[i]) == 0)
            return 1;
    return 0;
}

static void* client_daemon(void* vargp)
{
    Packet* packet;
    Socket* client_socket;
    char dummy = '\0';
    char buf[32];

    sprintf(buf, "%d", MAX_FILE_SIZE);

    client_socket = vargp;
    if (ctx.num_teams != 0)
        packet = packet_create(PACKET_CONTEST, strlen(buf)+1, buf);
    else
        packet = packet_create(PACKET_NO_CONTEST, strlen(buf)+1, buf);
    socket_send(client_socket, packet);
    packet_destroy(packet);

    if (ctx.num_teams != 0) {
        packet = socket_recv(client_socket, 1000);
        if (packet == NULL)
            goto fail;
        if (packet->id != PACKET_TEAM) {
            packet_destroy(packet);
            goto fail;
        }
        if (!validate_team(packet->buffer)) {
            puts(packet->buffer);
            packet_destroy(packet);
            packet = packet_create(PACKET_VALIDATION_FAILED, 1, &dummy);
            socket_send(client_socket, packet);
            packet_destroy(packet);
            goto fail;
        }
        packet_destroy(packet);
        packet = packet_create(PACKET_VALIDATION_SUCCESS, 1, &dummy);
        socket_send(client_socket, packet);
        packet_destroy(packet);
    }

    packet = socket_recv(client_socket, MAX_FILE_SIZE);
    if (!socket_connected(client_socket))
        goto fail;

    printf("%d\n", packet->id);
    printf("%d\n", packet->length);
    puts(packet->buffer);
    packet_destroy(packet);
    socket_destroy(client_socket);
    return NULL;

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

    if (socket_bind(listen_socket)) {
        puts("Couldn't bind socket");
        return NULL;
    }
    if (socket_listen(listen_socket)) {
        puts("Couldn't listen");
        return NULL;
    }
    return listen_socket;
}

void read_teams(JsonObject* config)
{
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
    ctx.teams = malloc(ctx.num_teams * sizeof(char*));
    for (i = 0; i < ctx.num_teams; i++) {
        value = json_array_get(array, i);
        if (json_get_type(value) != JTYPE_STRING) {
            puts("invalid teamname");
            exit(1);
        }
        string = json_get_string(value);
        n = strlen(string);
        ctx.teams[i] = malloc((n+1) * sizeof(char));
        memcpy(ctx.teams[i], string, n+1);
    }

    return;
}

int main(int argc, char** argv)
{
    Socket* client_socket;
    Socket* listen_socket;
    pthread_t thread_id;
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

    read_teams(config);

    if (ctx.num_teams == 0)
        puts("Contest is not running");
    else
        puts("Contest is running");

    networking_init();

    while (1) {
        listen_socket = create_listen_socket(config);
        if (listen_socket == NULL)
            break;
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        pthread_create(&thread_id, NULL, client_daemon, client_socket);
    }

    socket_destroy(client_socket);
    socket_destroy(listen_socket);

    networking_cleanup();

    json_object_destroy(config);
}

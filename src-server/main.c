#include <networking.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_NUM_CLIENTS 1

typedef struct {
    char* name;
    Socket* sock;
} Client;

typedef struct {
    sem_t client_sem;
    sem_t client_mutex;
    Client clients[MAX_NUM_CLIENTS];
} GlobalContext;

static GlobalContext ctx;

static void add_client(const char* name, Socket* sock)
{
    int n = strlen(name);
    char* name_copy = malloc((n+1) * sizeof(char));
    memcpy(name_copy, name, n+1);
    sem_wait(&ctx.client_mutex);
    for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
        if (ctx.clients[i].sock != NULL) {
            ctx.clients[i] = (Client) { name_copy, sock };
            goto done;
        }
    }
done:
    sem_post(&ctx.client_mutex);
}

static void remove_client(Socket* sock)
{
    sem_wait(&ctx.client_mutex);
    for (int i = 0; i < MAX_NUM_CLIENTS; i++) {
        if (ctx.clients[i].sock == sock) {
            free(ctx.clients[i].name);
            socket_destroy(ctx.clients[i].sock);
            ctx.clients[i] = (Client) { NULL, NULL };
            goto done;
        }
    }
done:
    sem_post(&ctx.client_mutex);
}

static void* client_daemon(void* vargp)
{
    Packet* packet;
    Socket* client_socket;
    client_socket = vargp;
    packet = socket_recv(client_socket, 1000);
    if (!socket_connected(client_socket))
        goto fail;
    if (packet->id != 1)
        goto fail;
    add_client(packet->buffer, client_socket);
    packet_destroy(packet);
    while (1) {
        packet = socket_recv(client_socket, 1000);
        if (!socket_connected(client_socket))
            goto fail_remove_client;
        printf("%d\n", packet->id);
        printf("%d\n", packet->length);
        puts(packet->buffer);
        packet_destroy(packet);
    }
    return NULL;

fail_remove_client:
    remove_client(client_socket);
fail:
    puts("Lost connection to client");
    socket_destroy(client_socket);
    sem_post(&ctx.client_sem);
    return NULL;
}

static Socket* create_listen_socket(void)
{
    Socket* listen_socket;
    listen_socket = socket_create(NULL, "27105", BIT_TCP|BIT_BIND);
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

int main()
{
    Socket* client_socket;
    Socket* listen_socket;
    pthread_t thread_id;

    sem_init(&ctx.client_sem, 0, MAX_NUM_CLIENTS);

    networking_init();

    while (1) {
        sem_wait(&ctx.client_sem);
        listen_socket = create_listen_socket();
        client_socket = socket_accept(listen_socket);
        socket_destroy(listen_socket);
        pthread_create(&thread_id, NULL, client_daemon, client_socket);
    }

    socket_destroy(client_socket);
    socket_destroy(listen_socket);

    networking_cleanup();

    sem_destroy(&ctx.client_sem);
}

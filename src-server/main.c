#include <networking.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_NUM_CLIENTS 1

typedef struct {
    sem_t client_sem;
    Socket* clients[MAX_NUM_CLIENTS];
} GlobalContext;

GlobalContext ctx;

void* client_daemon(void* vargp)
{
    Packet* packet;
    Socket* client_socket;
    client_socket = vargp;
    while (1) {
        packet = socket_recv(client_socket, 1000);
        if (!socket_connected(client_socket)) {
            puts("Lost connection to client");
            socket_destroy(client_socket);
            sem_post(&ctx.client_sem);
            return NULL;
        }
        printf("%d\n", packet->id);
        printf("%d\n", packet->length);
        puts(packet->buffer);
        packet_destroy(packet);
    }
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
    puts("Listening on local host");

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

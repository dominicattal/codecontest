#include <networking.h>
#include <stdio.h>
#include <pthread.h>

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
            return NULL;
        }
        printf("%d\n", packet->id);
        printf("%d\n", packet->length);
        puts(packet->buffer);
    }
    return NULL;
}

int main()
{
    Socket* client_socket;
    Socket* listen_socket;
    pthread_t thread_id;

    networking_init();

    listen_socket = socket_create(NULL, "27105", BIT_TCP|BIT_BIND);
    if (listen_socket == NULL) {
        puts("Could not create socket");
        return 1;
    }

    if (socket_bind(listen_socket)) {
        puts("Couldn't bind socket");
        return 1;
    }
    
    if (socket_listen(listen_socket)) {
        puts("Couldn't listen");
        return 1;
    }

    puts("Listening on local host");

    while (1) {
        client_socket = socket_accept(listen_socket);
        pthread_create(&thread_id, NULL, client_daemon, client_socket);
    }

    socket_destroy(client_socket);
    socket_destroy(listen_socket);

    networking_cleanup();
}

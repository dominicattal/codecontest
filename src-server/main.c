#include <networking.h>
#include <stdio.h>
#include <pthread.h>

void* client_daemon(void* vargp)
{
    Packet* packet;
    Socket* socket;

    puts("Received client");

    socket = vargp;
    while (1) {
        packet = socket_recv(socket);
        if (packet->length > 0)
            printf("%d\n", packet->length);
    }
    return NULL;
}

int main()
{
    Socket* client_socket;
    Socket* listen_socket;
    //pthread_t thread_id;

    networking_init();

    listen_socket = socket_create(NULL, "27105", true);
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

    Packet* packet;
    int length;
    while (1) {
        client_socket = socket_accept(listen_socket);
        //pthread_create(&thread_id, NULL, client_daemon, client_socket);
        length = -1;
        while (length < 0) {
            packet = socket_recv(client_socket);
            length = packet->length;
        }
        printf("%d\n", length);
        puts(packet->buffer+2);
    }

    socket_destroy(client_socket);
    socket_destroy(listen_socket);

    networking_cleanup();
}

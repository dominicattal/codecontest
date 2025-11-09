#include <stdio.h>
#include <string.h>
#include <networking.h>

int main()
{
    Socket* client_socket;
    Packet* test_packet;
    const char* test = "team1";

    networking_init();
    
    client_socket = socket_create("127.0.0.1", "27105", BIT_TCP);
    if (socket_connect(client_socket)) {
        puts("Could not connect");
        goto cleanup;
    }

    test_packet = packet_create(1, strlen(test)+1, test);

    if (socket_send(client_socket, test_packet))
        puts("unsuccessful");
    else
        puts("successful");
    //socket_recv(client_socket, 1000);

    socket_destroy(client_socket);

cleanup:
    networking_cleanup();

    return 0;
}

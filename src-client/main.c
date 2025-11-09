#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <networking.h>

int main()
{
    Socket* server_socket;
    Packet* packet;
    int max_file_size;
    const char* teamname;
    bool contest_running;

    networking_init();
    
    server_socket = socket_create("127.0.0.1", "27105", BIT_TCP);
    if (socket_connect(server_socket)) {
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
    if (max_file_size <= 0) {
        puts("something went wrong");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

    if (contest_running) {
        teamname = "team1";
        packet = packet_create(PACKET_TEAM, strlen(teamname)+1, teamname);
        if (socket_send(server_socket, packet)) {
            puts("unsuccessful");
            goto fail_destroy_packet;
        }
        packet_destroy(packet);
        packet = socket_recv(server_socket, 100);
        if (packet->id != PACKET_VALIDATION_SUCCESS) {
            puts("validation failed");
            goto fail_destroy_packet;
        }
        puts("validation successful");
        packet_destroy(packet);
    }

    const char* code = "for i in range(10): print(i)";
    packet = packet_create(PACKET_CODE_SEND, strlen(code)+1, code);
    if (socket_send(server_socket, packet)) {
        puts("unsuccessful");
        goto fail_destroy_packet;
    }
    packet_destroy(packet);

fail_destroy_packet:
    packet_destroy(packet);
fail_destroy_socket:
    socket_destroy(server_socket);
    networking_cleanup();

    return 0;
}

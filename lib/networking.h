#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdbool.h>

typedef struct {
    int id;
    int length;
    char* buffer;
} Packet;

typedef struct Socket Socket;

// returns non-zero if failed
int     networking_init(void);
void    networking_cleanup(void);
char*   networking_hostname(void);

// Create a new socket
// ip   -> ip to create socket for. NULL to create on host
// port -> number from 0-65535 in string format. Undefined if NULL.
// tcp  -> whether the socket support tcp or udp
Socket* socket_create(const char* ip, const char* port, bool tcp);

// Bind a socket so clients can access
int     socket_bind(Socket* socket);

// Listen for incoming connection requests
int     socket_listen(Socket* socket);

// Accept a connection request
Socket* socket_accept(Socket* socket);

// Connect to a socket
int     socket_connect(Socket* socket);

// Destroy socket and all related information
void    socket_destroy(Socket* socket);

// Send a packet over a socket
int     socket_send(Socket* socket, Packet* packet);

// Receive a packet from a socket
Packet* socket_recv(Socket* socket);

Packet* packet_create(int id, int length, const char* buffer);
void    packet_destroy(Packet* packet);

#endif

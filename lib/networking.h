#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdbool.h>

#define BIT_TCP  0x1

typedef enum {
    PACKET_NO_CONTEST,
    PACKET_CONTEST,
    PACKET_TEAM_VALIDATE_USERNAME,
    PACKET_TEAM_VALIDATE_PASSWORD,
    PACKET_TEAM_VALIDATION_SUCCESS,
    PACKET_TEAM_VALIDATION_FAILED,
    PACKET_LANGUAGE_VALIDATE,
    PACKET_LANGUAGE_VALIDATION_SUCCESS,
    PACKET_LANGUAGE_VALIDATION_FAILED,
    PACKET_CODE_NAME_SEND,
    PACKET_CODE_SEND,
    PACKET_CODE_ACCEPTED,
    PACKET_CODE_FAILED
} PacketEnum;

typedef struct {
    int id;
    int length;
    char* buffer;
} Packet;

typedef struct Socket Socket;

// returns true if successful
bool    networking_init(int max_num_conn);
void    networking_cleanup(void);
char*   networking_hostname(void);

// Create a new socket
// ip   -> ip to create socket for. NULL to create on host
// port -> number from 0-65535 in string format. Undefined if NULL.
// tcp  -> whether the socket support tcp or udp
Socket* socket_create(const char* ip, const char* port, int flags);

// Bind a socket so clients can access. Returns true if successful
bool    socket_bind(Socket* socket);

// Listen for incoming connection requests. Returns true if successful
bool    socket_listen(Socket* socket);

// Accept a connection request. Returns the socket if successful, NULL otherwise
Socket* socket_accept(Socket* socket);

// Connect to a socket. Returns true if successful
bool    socket_connect(Socket* socket);

// Destroy socket and all related information
void    socket_destroy(Socket* socket);

// Send a packet over a socket. Returns true if successful
bool    socket_send(Socket* socket, Packet* packet);

// Check if socket still connected. Returns true if successful
bool    socket_connected(Socket* socket);

// Receive a packet from a socket
Packet* socket_recv(Socket* socket, int max_length);

// Return error code of last networking function, will differ based on OS
int     socket_get_last_error(void);

Packet* packet_create(int id, int length, const char* buffer);
void    packet_destroy(Packet* packet);

#endif

#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdbool.h>
#include <stdlib.h>

#define BIT_TCP  0x1

typedef enum {
    PACKET_CLI_CLIENT,
    PACKET_WEB_CLIENT,
    PACKET_NO_CONTEST,
    PACKET_CONTEST,
    PACKET_TEAM_VALIDATE_USERNAME,
    PACKET_TEAM_VALIDATE_PASSWORD,
    PACKET_TEAM_VALIDATION_SUCCESS,
    PACKET_TEAM_VALIDATION_FAILED,
    PACKET_LANGUAGE_VALIDATE,
    PACKET_LANGUAGE_VALIDATION_SUCCESS,
    PACKET_LANGUAGE_VALIDATION_FAILED,
    PACKET_PROBLEM_VALIDATE,
    PACKET_PROBLEM_VALIDATE_SUCCESS,
    PACKET_PROBLEM_VALIDATE_FAILED,
    PACKET_CODE_NAME_SEND,
    PACKET_CODE_SEND,
    PACKET_CODE_ACCEPTED,
    PACKET_CODE_FAILED,
    PACKET_CODE_NOTIFICATION,
    PACKET_ASYNC,
    WEB_PACKET_TEXT,
    WEB_PACKET_CLOSE,
    WEB_PACKET_PING,
    WEB_PACKET_PONG
} PacketEnum;

typedef struct {
    PacketEnum id;
    char* buffer;
    size_t length;
} Packet;

typedef struct Socket Socket;
typedef struct NetContext NetContext;

NetContext* networking_init(void);
void        networking_shutdown_sockets(NetContext* ctx);
void        networking_cleanup(NetContext* ctx);

// Create a new socket
// ip   -> ip to create socket for. NULL to create on host
// port -> number from 0-65535 in string format. Undefined if NULL.
// tcp  -> whether the socket support tcp or udp
Socket* socket_create(NetContext* ctx, const char* ip, const char* port, int flags);

// Bind a socket so clients can access. Returns true if successful
bool    socket_bind(Socket* socket);

// Listen for incoming connection requests. Returns true if successful
bool    socket_listen(Socket* socket);

// Accept a connection request. Returns the socket if successful, NULL otherwise
Socket* socket_accept(NetContext* ctx, Socket* socket);

// Connect to a socket. Returns true if successful
bool    socket_connect(Socket* socket);

// Destroy socket and all related information
void    socket_destroy(NetContext* ctx, Socket* socket);

// Check if socket still connected. Returns true if successful
bool    socket_connected(Socket* socket);

// Send a packet over a socket. Returns true if successful
bool    socket_send(Socket* socket, Packet* packet);

// Send packet to all connected clients in context
void    socket_send_all(NetContext* ctx, Packet* packet);

// Send a packet over a web socket. Returns true if successful
bool    socket_send_web(Socket* socket, Packet* packet);

// Send web packet to all connected clients in context
void    socket_send_web_all(NetContext* ctx, Packet* packet);

// Send a generic server-to-client handshake
bool    socket_send_web_handshake(Socket* socket);

// Receive a packet from a socket
Packet* socket_recv(Socket* socket);

// Receive web socket packet
Packet* socket_recv_web(Socket* socket);

// Perform web socket handshake with client socket
bool    socket_web_handshake(Socket* socket);

// Create a packet with id with a buffer of length. buffer can be NULL iff length is 0.
// Returns NULL if buffer is NULL and length is not 0
Packet* packet_create(PacketEnum id, int length, const char* buffer);

// Frees memory from packet
void    packet_destroy(Packet* packet);

#endif

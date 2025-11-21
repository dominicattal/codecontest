#include "networking.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

typedef struct {
    WSADATA wsa_data;
} NetworkingContext;

typedef struct Socket {
    struct addrinfo* info;
    SOCKET* sock;
    bool connected;
} Socket;

static NetworkingContext ctx;

bool networking_init(void)
{
    if (WSAStartup(MAKEWORD(2,2), &ctx.wsa_data))
        return false;
    return true;
}

void networking_cleanup(void)
{
    WSACleanup();
}

char* networking_hostname(void)
{
    struct hostent* local_host;
    local_host = gethostbyname("");
    return inet_ntoa(*(struct in_addr *)*local_host->h_addr_list);
}

Socket* socket_create(const char* ip, const char* port, int flags)
{
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    SOCKET* new_socket;
    Socket* res_socket;

    int tcp = flags & 1;
    int will_bind = (flags>>1) & 1;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = (tcp) ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = (tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = (will_bind) ? AI_PASSIVE : 0;
    if (getaddrinfo(ip, port, &hints, &result))
        goto fail;

    new_socket = malloc(sizeof(SOCKET));
    *new_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (*new_socket == INVALID_SOCKET)
        goto fail_free_addr_info;

    res_socket = malloc(sizeof(Socket));
    if (res_socket == NULL)
        goto fail_close_socket;

    res_socket->sock = new_socket;
    res_socket->info = result;
    res_socket->connected = true;

    return res_socket;

fail_close_socket:
    closesocket(*new_socket);
    free(new_socket);
fail_free_addr_info:
    freeaddrinfo(result);
fail:
    return NULL;
}

bool socket_bind(Socket* sock)
{
    if (bind(*sock->sock, sock->info->ai_addr, (int)sock->info->ai_addrlen) == SOCKET_ERROR)
        return false; 
    return true;
}

bool socket_listen(Socket* sock)
{
    if (listen(*sock->sock, SOMAXCONN) == SOCKET_ERROR)
        return false;
    return true;
}

Socket* socket_accept(Socket* sock)
{
    Socket* client_socket;
    SOCKET* new_socket;

    client_socket = malloc(sizeof(Socket));
    new_socket = malloc(sizeof(SOCKET));
    *new_socket = accept(*sock->sock, NULL, NULL);
    if (*new_socket == INVALID_SOCKET) {
        free(client_socket);
        free(new_socket);
        return NULL;
    }
    client_socket->sock = new_socket;
    client_socket->info = NULL;
    return client_socket;
}

bool socket_connect(Socket* sock)
{
    struct addrinfo* ptr;
    for (ptr = sock->info; ptr != NULL; ptr = ptr->ai_next) {
        *sock->sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (*sock->sock == INVALID_SOCKET)
            return false;
        if (connect(*sock->sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR)
            return true;
    }
    return false;
}

void socket_destroy(Socket* sock)
{
    freeaddrinfo(sock->info);
    closesocket(*sock->sock);
    free(sock->sock);
    free(sock);
}

bool socket_send(Socket* sock, Packet* packet)
{
    if (send(*sock->sock, packet->buffer, packet->length, 0) == SOCKET_ERROR)
        return false;
    return true;
}

Packet* socket_recv(Socket* sock, int max_length)
{
    Packet* packet;
    int length;
    char* buffer;
    buffer = malloc(max_length * sizeof(char));
    length = recv(*sock->sock, buffer, max_length, 0);
    if (length == SOCKET_ERROR || length == 0) {
        sock->connected = false;
        free(buffer);
        return NULL;
    }
    packet = malloc(sizeof(Packet));
    packet->id = (buffer[0]<<8) + buffer[1];
    packet->length = length-2;
    packet->buffer = malloc(packet->length * sizeof(char));
    memcpy(packet->buffer, buffer+2, packet->length * sizeof(char));
    free(buffer);
    return packet;
}

bool socket_connected(Socket* sock)
{
    return sock->connected;
}

int socket_get_last_error(void)
{
    return WSAGetLastError();
}

#else

#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

typedef struct Socket {
    int fd;
    struct sockaddr_in addr;
    bool connected;
} Socket;

bool networking_init(void)
{
    return true;
}

void networking_cleanup(void)
{
}

char* networking_hostname(void)
{
    return NULL;
}

Socket* socket_create(const char* ip, const char* port_str, int flags)
{
    Socket* sock;
    int port;
    port = atoi(port_str);

    sock = malloc(sizeof(Socket));
    sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    sock->addr.sin_family = AF_INET;
    if (ip == NULL)
        ip = "0.0.0.0";
    inet_pton(AF_INET, ip, &sock->addr.sin_addr);
    sock->addr.sin_port = htons(port);
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    return sock;
}

bool socket_bind(Socket* sock)
{
    return bind(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) == 0;
}

bool socket_listen(Socket* sock)
{
    return listen(sock->fd, 2) == 0;
}

Socket* socket_accept(Socket* sock)
{
    Socket* new_sock;
    socklen_t addrlen;
    new_sock = malloc(sizeof(Socket));
    addrlen = sizeof(sock->addr);
    new_sock->fd = accept(sock->fd, (struct sockaddr*)&sock->addr, &addrlen);
    return new_sock;
}

bool socket_connect(Socket* sock)
{
    sock->connected = connect(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) == 0;
    return sock->connected;
}

void socket_destroy(Socket* sock)
{
    close(sock->fd);
    free(sock);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(sock->fd, packet->buffer, packet->length, 0) != -1;
}

bool socket_connected(Socket* socket)
{
    return true;
}

Packet* socket_recv(Socket* sock, int max_length)
{
    Packet* packet;
    int length;
    char* buffer;
    buffer = malloc(max_length * sizeof(char));
    length = read(sock->fd, buffer, max_length);
    if (length <= 0) {
        sock->connected = false;
        free(buffer);
        return NULL;
    }
    packet = malloc(sizeof(Packet));
    packet->id = (buffer[0]<<8) + buffer[1];
    packet->length = length-2;
    packet->buffer = malloc(packet->length * sizeof(char));
    memcpy(packet->buffer, buffer+2, packet->length * sizeof(char));
    free(buffer);
    return packet;
}

int socket_get_last_error(void)
{
    return 0;
}

#endif

// compatible on linux and windows

Packet* packet_create(int id, int length, const char* buffer)
{
    Packet* packet = malloc(sizeof(Packet));
    packet->id = id;
    packet->length = length+2;
    packet->buffer = malloc(packet->length * sizeof(char));
    packet->buffer[0] = (id>>8) & 0xFF;
    packet->buffer[1] = id & 0xFF;
    memcpy(packet->buffer+2, buffer, length);
    return packet;
}


void packet_destroy(Packet* packet)
{
    free(packet->buffer);
    free(packet);
}

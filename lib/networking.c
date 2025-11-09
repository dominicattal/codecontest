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
    SOCKET* sock;
    struct addrinfo* info;
} Socket;

static NetworkingContext ctx;

int networking_init(void)
{
    if (WSAStartup(MAKEWORD(2,2), &ctx.wsa_data))
        return WSAGetLastError();
    return 0;
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

Socket* socket_create(const char* ip, const char* port, bool tcp)
{
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    SOCKET* new_socket;
    Socket* res_socket;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = (tcp) ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_protocol = (tcp) ? IPPROTO_TCP : IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
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

    return res_socket;

fail_close_socket:
    closesocket(*new_socket);
    free(new_socket);
fail_free_addr_info:
    freeaddrinfo(result);
fail:
    return NULL;
}

int socket_bind(Socket* sock)
{
    if (bind(*sock->sock, sock->info->ai_addr, (int)sock->info->ai_addrlen))
        return WSAGetLastError(); 
    return 0;
}

int socket_listen(Socket* sock)
{
    if (listen(*sock->sock, SOMAXCONN))
        return WSAGetLastError();
    return 0;
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
    return client_socket;
}

int socket_connect(Socket* sock)
{
    struct addrinfo* ptr;
    for (ptr = sock->info; ptr != NULL; ptr = ptr->ai_next) {
        *sock->sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (*sock->sock == INVALID_SOCKET)
            return WSAGetLastError();
        if (connect(*sock->sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR)
            return 0;
    }
    return 1;
}

void socket_destroy(Socket* sock)
{
    closesocket(*sock->sock);
    free(sock->sock);
    freeaddrinfo(sock->info);
    free(sock);
}

Packet* packet_create(int id, int length, const char* buffer)
{
    Packet* packet = malloc(sizeof(Packet));
    packet->id = id;
    packet->length = length+2;
    packet->buffer = malloc(length*sizeof(char) + sizeof(int));
    packet->buffer[0] = (id>>8) & 0xFF;
    packet->buffer[1] = id & 0xFF;
    memcpy(packet->buffer+2, buffer, length);
    return packet;
}

int socket_send(Socket* sock, Packet* packet)
{
    if (send(*sock->sock, packet->buffer, packet->length, 0) == SOCKET_ERROR)
        return WSAGetLastError();
    return 0;
}

Packet* socket_recv(Socket* sock)
{
    Packet* packet;
    packet = malloc(sizeof(Packet));
    packet->buffer = malloc(1000 * sizeof(char));
    packet->length = recv(*sock->sock, packet->buffer, 1000, 0);
    return packet;
}

void packet_destroy(Packet* packet)
{
}

#endif

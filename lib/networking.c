#include "networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#define PACKET_HEADER_BYTES 6

// compatible on linux and windows

Packet* packet_create(PacketEnum id, int length, const char* buffer)
{
    Packet* packet;
    if (buffer == NULL && length != 0)
        return NULL;
    packet = malloc(sizeof(Packet));
    packet->id = id;
    packet->length = length + PACKET_HEADER_BYTES;
    packet->buffer = malloc(packet->length * sizeof(char));
    packet->buffer[0] = (length>>24) & 0xFF;
    packet->buffer[1] = (length>>16) & 0xFF;
    packet->buffer[2] = (length>>8) & 0xFF;
    packet->buffer[3] = length & 0xFF;
    packet->buffer[4] = (id>>8) & 0xFF;
    packet->buffer[5] = id & 0xFF;
    memcpy(packet->buffer+PACKET_HEADER_BYTES, buffer, length);
    return packet;
}

void packet_destroy(Packet* packet)
{
    free(packet->buffer);
    free(packet);
}

#ifdef __WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

typedef struct Socket {
    NetContext* ctx;
    struct addrinfo* info;
    struct Socket* prev;
    struct Socket* next;
    SOCKET* sock;
    bool connected;
} Socket;

typedef struct NetContext {
    WSADATA wsa_data;
    Socket* head;
    Socket* tail;
    pthread_mutex_t mutex;
    bool active;
} NetContext;

WSADATA wsa_data;
bool startup = false;

static WSADATA wsa_startup_if_not(void)
{
    if (!startup) {
        if (WSAStartup(MAKEWORD(2,2), &wsa_data))
            printf("WSAStartup failed: %d\n", WSAGetLastError());
        else
            startup = true;
    }
    return wsa_data;
}

NetContext* networking_init(void)
{
    NetContext* ctx = malloc(sizeof(NetContext));
    ctx->wsa_data = wsa_startup_if_not();
    if (!startup) {
        free(ctx);
        return NULL;
    }
    ctx->head = ctx->tail = NULL;
    pthread_mutex_init(&ctx->mutex, NULL);
    ctx->active = true;
    return ctx;
}

void networking_shutdown_sockets(NetContext* ctx)
{
    Socket* sock;
    pthread_mutex_lock(&ctx->mutex);
    sock = ctx->head;
    while (sock != NULL) {
        if (sock->sock != NULL)
            closesocket(*sock->sock);
        sock->sock = NULL;
        sock = sock->next;
    }
    ctx->active = false;
    pthread_mutex_unlock(&ctx->mutex);
}

void networking_cleanup(NetContext* ctx)
{
    Socket* sock = ctx->head;
    Socket* next;
    while (sock != NULL) {
        next = sock->next;
        socket_destroy(sock);
        sock = next;
    }
    pthread_mutex_destroy(&ctx->mutex);
    WSACleanup();
}

int networking_get_last_error(void)
{
    return WSAGetLastError();
}

static Socket* get_free_socket(NetContext* ctx)
{
    Socket* sock = NULL;
    pthread_mutex_lock(&ctx->mutex);
    if (!ctx->active) goto unlock;
    sock = malloc(sizeof(Socket));
    sock->ctx = ctx;
    sock->connected = false;
    sock->next = NULL;
    sock->prev = ctx->tail;
    sock->sock = NULL;
    sock->info = NULL;
    if (ctx->head == NULL) {
        ctx->head = sock;
    } else {
        ctx->tail->next = sock;
    }
    ctx->tail = sock;
unlock:
    pthread_mutex_unlock(&ctx->mutex);
    return sock;
}

Socket* socket_create(NetContext* ctx, const char* ip, const char* port, int flags)
{
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    SOCKET* new_socket;
    Socket* res_socket;

    res_socket = get_free_socket(ctx);
    if (res_socket == NULL)
        return NULL;

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

    //BOOL opt_val = TRUE;
    //int opt_len = sizeof(BOOL);
    //setsockopt(*new_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, opt_len);

    //int iopt_val = 5000;
    //opt_len = sizeof(int);
    //setsockopt(*new_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iopt_val, opt_len);
    //setsockopt(*new_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iopt_val, opt_len);

    res_socket->sock = new_socket;
    res_socket->info = result;

    return res_socket;

fail_free_addr_info:
    freeaddrinfo(result);
fail:
    return NULL;
}

bool socket_bind(Socket* sock)
{
    return bind(*sock->sock, sock->info->ai_addr, (int)sock->info->ai_addrlen) != SOCKET_ERROR;
}

bool socket_listen(Socket* sock)
{
    return listen(*sock->sock, SOMAXCONN) != SOCKET_ERROR;
}

Socket* socket_accept(Socket* sock)
{
    Socket* client_socket;
    SOCKET* new_socket;

    client_socket = get_free_socket(sock->ctx);
    if (client_socket == NULL)
        return NULL;
    new_socket = malloc(sizeof(SOCKET));
    *new_socket = accept(*sock->sock, NULL, NULL);
    if (*new_socket == INVALID_SOCKET) {
        free(new_socket);
        return NULL;
    }
    client_socket->sock = new_socket;
    return client_socket;
}

bool socket_connect(Socket* sock)
{
    struct addrinfo* ptr;
    for (ptr = sock->info; ptr != NULL; ptr = ptr->ai_next) {
        *sock->sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (*sock->sock == INVALID_SOCKET)
            return false;
        if (connect(*sock->sock, ptr->ai_addr, (int)ptr->ai_addrlen) != SOCKET_ERROR) {
            sock->connected = true;
            return true;
        }
    }
    return false;
}

void socket_destroy(Socket* sock)
{
    NetContext* ctx;
    ctx = sock->ctx;
    pthread_mutex_lock(&sock->ctx->mutex);
    if (sock->sock != NULL) {
        shutdown(*sock->sock, SD_BOTH);
        closesocket(*sock->sock);
    }
    if (sock->info != NULL)
        freeaddrinfo(sock->info);
    sock->connected = false;
    sock->sock = NULL;
    sock->info = NULL;
    if (sock == ctx->head)
        ctx->head = sock->next;
    else
        sock->prev->next = sock->next;
    if (sock == ctx->tail)
        ctx->tail = sock->prev;
    else
        sock->next->prev = sock->prev;
    pthread_mutex_unlock(&sock->ctx->mutex);
    free(sock);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(*sock->sock, packet->buffer, packet->length, 0) != SOCKET_ERROR;
}

bool socket_send_web(Socket* socket, Packet* packet)
{
    return false;
}

void socket_send_all(NetContext* ctx, Packet* packet)
{
    Socket* cur;
    pthread_mutex_lock(&ctx->mutex);
    cur = ctx->head;
    while (cur != NULL) {
        if (cur->connected)
            socket_send(cur, packet);
        cur = cur->next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

Packet* socket_recv(Socket* sock)
{
    Packet* packet;
    int length;
    unsigned char* buffer;
    buffer = malloc(6 * sizeof(unsigned char));
    length = recv(*sock->sock, (char*)buffer, 6, 0);
    if (length == SOCKET_ERROR || length == 0) {
        sock->connected = false;
        free(buffer);
        return NULL;
    }
    packet = malloc(sizeof(Packet));
    packet->id = (buffer[4]<<8) + buffer[5];
    packet->length = (buffer[0]<<24)+(buffer[1]<<16)+(buffer[2]<<8)+buffer[3];
    if (packet->length == 0) {
        packet->buffer = NULL;
        free(packet->buffer);
        return packet;
    }
    packet->buffer = malloc(packet->length * sizeof(char));
    length = recv(*sock->sock, packet->buffer, packet->length, 0);
    if ((size_t)length != packet->length) {
        printf("Unexpected packet length %u vs %llu\n", length, packet->length);
        free(packet->buffer);
        free(packet);
        free(buffer);
        return NULL;
    }
    free(buffer);
    return packet;
}

bool socket_connected(Socket* sock)
{
    return sock->connected;
}

void socket_set_thread_id(Socket* socket, pthread_t thread_id)
{
    puts("socket_set_thread_id not implemented on windows");
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
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct Socket {
    NetContext* ctx;
    struct Socket* prev;
    struct Socket* next;
    struct sockaddr_in addr;
    pthread_t thread_id;
    int fd;
    bool has_thread;
    bool connected;
} Socket;

typedef struct NetContext {
   pthread_mutex_t mutex;
   Socket* head;
   Socket* tail;
   bool active;
} NetContext;

NetContext* networking_init(void)
{
    NetContext* ctx = malloc(sizeof(NetContext));
    ctx->head = ctx->tail = NULL;
    pthread_mutex_init(&ctx->mutex, NULL);
    ctx->active = true;
    return ctx;
}

void networking_shutdown_sockets(NetContext* ctx)
{
    Socket* sock;
    pthread_mutex_lock(&ctx->mutex);
    sock = ctx->head;
    while (sock != NULL) {
        if (sock->fd != -1)
            shutdown(sock->fd, SHUT_RDWR);
        sock->fd = -1;
        sock = sock->next;
    }
    ctx->active = false;
    pthread_mutex_unlock(&ctx->mutex);
}

void networking_join_sockets(NetContext* ctx)
{
    Socket* sock;
    pthread_mutex_lock(&ctx->mutex);
    sock = ctx->head;
    while (sock != NULL) {
        if (sock->has_thread)
            pthread_kill(sock->thread_id, 0);
        sock->fd = -1;
        sock = sock->next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

void networking_cleanup(NetContext* ctx)
{
    Socket* sock = ctx->head;
    Socket* next;
    while (sock != NULL) {
        next = sock->next;
        socket_destroy(sock);
        sock = next;
    }
    pthread_mutex_destroy(&ctx->mutex);
    free(ctx);
}

static Socket* get_free_socket(NetContext* ctx)
{
    Socket* sock = NULL;
    pthread_mutex_lock(&ctx->mutex);
    if (!ctx->active) goto unlock;
    sock = malloc(sizeof(Socket));
    sock->ctx = ctx;
    sock->connected = false;
    sock->next = NULL;
    sock->prev = ctx->tail;
    sock->has_thread = false;
    if (ctx->head == NULL) {
        ctx->head = sock;
    } else {
        ctx->tail->next = sock;
    }
    ctx->tail = sock;
unlock:
    pthread_mutex_unlock(&ctx->mutex);
    return sock;
}

Socket* socket_create(NetContext* ctx, const char* ip, const char* port_str, int flags)
{
    Socket* sock;
    int port;
    port = atoi(port_str);

    sock = get_free_socket(ctx);
    if (sock == NULL)
        goto fail;

    sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    sock->addr.sin_family = AF_INET;
    if (ip == NULL)
        ip = "0.0.0.0";
    inet_pton(AF_INET, ip, &sock->addr.sin_addr);
    sock->addr.sin_port = htons(port);
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(sock->fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    return sock;

fail:
    puts("Error creating socketing");
    return NULL;
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
    int fd;
    addrlen = sizeof(sock->addr);
    fd = accept(sock->fd, (struct sockaddr*)&sock->addr, &addrlen);
    if (fd == -1)
        return NULL;
    new_sock = get_free_socket(sock->ctx);
    if (new_sock == NULL) {
        shutdown(fd, SHUT_RDWR);
        return NULL;
    }
    new_sock->fd = fd;
    new_sock->connected = true;
    return new_sock;
}

bool socket_connect(Socket* sock)
{
    sock->connected = connect(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) == 0;
    return sock->connected;
}

bool socket_connected(Socket* socket)
{
    return socket->connected;
}

void socket_destroy(Socket* sock)
{
    NetContext* ctx;
    ctx = sock->ctx;
    pthread_mutex_lock(&ctx->mutex);
    if (sock->fd != -1) {
        shutdown(sock->fd, SHUT_RDWR);
        close(sock->fd);
    }
    if (sock == ctx->head)
        ctx->head = sock->next;
    else
        sock->prev->next = sock->next;
    if (sock == ctx->tail)
        ctx->tail = sock->prev;
    else
        sock->next->prev = sock->prev;
    free(sock);
    pthread_mutex_unlock(&ctx->mutex);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(sock->fd, packet->buffer, packet->length, 0) != -1;
}

void socket_send_all(NetContext* ctx, Packet* packet)
{
    Socket* cur;
    pthread_mutex_lock(&ctx->mutex);
    cur = ctx->head;
    while (cur != NULL) {
        if (cur->connected)
            socket_send(cur, packet);
        cur = cur->next;
    }
    pthread_mutex_unlock(&ctx->mutex);
}

Packet* socket_recv(Socket* sock)
{
    Packet* packet;
    size_t length;
    unsigned char* buffer;
    buffer = malloc(6 * sizeof(char));
    length = read(sock->fd, buffer, 6);
    if (length <= 0) {
        sock->connected = false;
        free(buffer);
        return NULL;
    }
    packet = malloc(sizeof(Packet));
    packet->id = (buffer[4]<<8) + buffer[5];
    packet->length = (buffer[0]<<24)+(buffer[1]<<16)+(buffer[2]<<8)+buffer[3];
    if (packet->length == 0) {
        packet->buffer = NULL;
        free(buffer);
        return packet;
    }
    packet->buffer = malloc(packet->length * sizeof(char));
    length = read(sock->fd, packet->buffer, packet->length);
    if (length != packet->length) {
        printf("Unexpected packet length %lu vs %lu\n", length, packet->length);
        free(packet->buffer);
        free(packet);
        free(buffer);
        return NULL;
    }
    free(buffer);
    return packet;
}

void socket_set_thread_id(Socket* sock, pthread_t thread_id)
{
    sock->has_thread = true;
    sock->thread_id = thread_id;
}

#endif

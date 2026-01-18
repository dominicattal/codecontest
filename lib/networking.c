#include "networking.h"
#define SHA_IMPLEMENTATION
#include "sha.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// compatible on linux and windows

Packet* packet_create(PacketEnum id, int length, const char* buffer)
{
    Packet* packet;
    if (buffer == NULL && length != 0)
        return NULL;
    packet = malloc(sizeof(Packet));
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

bool networking_init(int max_num_conn)
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

bool socket_send_web(Socket* socket, Packet* packet)
{
    return false;
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

Packet* socket_recv_web(Socket* socket)
{
    Packet* packet;
    unsigned long long data;
    return NULL;
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
#include <semaphore.h>
#include <pthread.h>

typedef struct Socket {
    int fd;
    struct sockaddr_in addr;
    bool connected;
    bool in_use;
} Socket;

static struct {
   int num_sockets;
   Socket* sockets;
   pthread_mutex_t mutex;
   sem_t sem;
} net_ctx;

bool networking_init(int max_num_conn)
{
    net_ctx.num_sockets = max_num_conn;
    net_ctx.sockets = calloc(max_num_conn, sizeof(Socket));
    for (int i = 0; i < max_num_conn; i++) {
        net_ctx.sockets[i].fd = -1;
        net_ctx.sockets[i].connected = false;
        net_ctx.sockets[i].in_use = false;
    }
    sem_init(&net_ctx.sem, 0, max_num_conn);
    pthread_mutex_init(&net_ctx.mutex, NULL);
    return true;
}

void networking_cleanup(void)
{
    for (int i = 0; i < net_ctx.num_sockets; i++)
        socket_destroy(&net_ctx.sockets[i]);
    free(net_ctx.sockets);
    sem_destroy(&net_ctx.sem);
    pthread_mutex_destroy(&net_ctx.mutex);
}

char* networking_hostname(void)
{
    return NULL;
}

static Socket* get_free_socket(void)
{
    Socket* sock = NULL;
    int i;
    sem_wait(&net_ctx.sem);
    pthread_mutex_lock(&net_ctx.mutex);
    for (i = 0; i < net_ctx.num_sockets; i++) {
        if (!net_ctx.sockets[i].in_use) {
            sock = &net_ctx.sockets[i];
            break;
        }
    }
    if (sock == NULL)
        puts("Could not find free socket");
    else 
        sock->in_use = true;
    pthread_mutex_unlock(&net_ctx.mutex);
    return sock;
}

Socket* socket_create(const char* ip, const char* port_str, int flags)
{
    Socket* sock;
    int port;
    port = atoi(port_str);

    sock = get_free_socket();
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
    new_sock = get_free_socket();
    new_sock->fd = fd;
    return new_sock;
}

bool socket_connect(Socket* sock)
{
    sock->connected = connect(sock->fd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) == 0;
    return sock->connected;
}

void socket_destroy(Socket* sock)
{
    if (!sock->in_use) return;
    pthread_mutex_lock(&net_ctx.mutex);
    if (sock->fd != -1)
        shutdown(sock->fd, SHUT_RDWR);
    sock->connected = false;
    sock->in_use = false;
    sock->fd = -1;
    pthread_mutex_unlock(&net_ctx.mutex);
    sem_post(&net_ctx.sem);
}

bool socket_send(Socket* sock, Packet* packet)
{
    return send(sock->fd, packet->buffer, packet->length, 0) != -1;
}

bool socket_send_web(Socket* socket, Packet* packet)
{
    return false;
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

Packet* socket_recv_web(Socket* sock)
{
    Packet* packet;
    int length;
    char* buffer;
    char data;
    // fields in web socket packet
    bool fin, rsv1, rsv2, rsv3, mask;
    char opcode;
    int payload_len;
    unsigned long long ext_payload_len = 0;
    unsigned long long buffer_length = 0;
    unsigned long long masking_key;
    read(sock->fd, &data, 1);
    fin = (data>>7) & 1;
    rsv1 = (data>>6) & 1;
    rsv2 = (data>>5) & 1;
    rsv3 = (data>>4) & 1;
    opcode = data & 0xF;
    read(sock->fd, &data, 1);
    mask = (data>>7) & 1;
    payload_len = data & 0x7F;
    buffer_length = payload_len;
    if (payload_len == 126) {
        read(sock->fd, &ext_payload_len, 2);
        buffer_length = ext_payload_len;
    } else if (payload_len == 127) {
        read(sock->fd, &ext_payload_len, 8);
        buffer_length = ext_payload_len;
    }
    buffer = malloc(buffer_length * sizeof(char));
    read(sock->fd, &masking_key, 4);
    read(sock->fd, buffer, 8);
    // read into buffer
    printf("fin=%hhd\n"
           "rsv1=%hhd\n"
           "rsv2=%hhd\n"
           "rsv3=%hhd\n"
           "opcode=0x%hhX\n"
           "mask=%hhd\n"
           "payload_len=%d\n"
           "ext_payload_len=%llu\n"
           "masking-key=%llu\n",
           fin, rsv1, rsv2, rsv3,
           opcode, mask, payload_len,
           ext_payload_len, masking_key);
    for (int i = 0; i < buffer_length; i++)
        printf("%02x ", buffer[i]);
    puts("");
    return NULL;
}

bool socket_web_handshake(Socket* sock)
{
    char magic_string[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char* recv_buffer;
    char* key;
    char c;
    int i, j, length, key_length;
    key = NULL;
    recv_buffer = malloc(1024 * sizeof(char));
    length = read(sock->fd, recv_buffer, 1024);
    for (int i = 0; i < length; i++)
        printf("%c", recv_buffer[i]);
    puts("");
    
    for (i = 0; i+17 < length; i++) {
        if (recv_buffer[i] == 'S' && recv_buffer[i+16] == 'y') {
            c = recv_buffer[i+17];
            recv_buffer[i+17] = '\0';
            if (strcmp(recv_buffer+i, "Sec-WebSocket-Key") != 0) {
                recv_buffer[i+17] = c;
                continue;
            }
            recv_buffer[i+17] = c;
            i += 19;
            for (j = i; j < length && recv_buffer[j] != '\r'; j++)
                ;
            key_length = j-i;
            key = malloc((key_length+1) * sizeof(char));
            key[key_length] = '\0';
            c = recv_buffer[j];
            recv_buffer[j] = '\0';
            sprintf(key, "%s", recv_buffer+i);
            recv_buffer[j] = c;
            goto found;
        }
    }
    free(recv_buffer);
    return false;

found:

    char* utf8 = malloc((key_length+strlen(magic_string)+1)*sizeof(char));
    sprintf(utf8, "%s%s", key, magic_string);
    char* hash = sha1_utf8(utf8, strlen(utf8));
    char* ret = hex_to_base64(hash, strlen(hash));
    char send_buffer_fmt[] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n";
    char* send_buffer;
    length = snprintf(NULL, 0, send_buffer_fmt, ret);
    send_buffer = malloc((length+1) * sizeof(char));
    snprintf(send_buffer, length+1, send_buffer_fmt, ret);
    puts(send_buffer);

    free(key);
    free(utf8);
    free(hash);
    free(ret);
    free(recv_buffer);
    bool success = send(sock->fd, send_buffer, strlen(send_buffer), 0) != -1;
    free(send_buffer);
    return success;
}

int socket_get_last_error(void)
{
    return 0;
}

#endif

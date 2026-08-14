#include "bridge_tcp_client.h"
#include "bridge_stream.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

static int sock_fd = -1;
static bridge_stream_t stream;

int bridge_tcp_client_connect(const char* ip, uint16_t port)
{
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }
    
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }
    
    bridge_stream_init(&stream);
    return 0;
}

void bridge_tcp_client_disconnect(void)
{
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
}

int bridge_tcp_client_is_connected(void)
{
    return sock_fd >= 0;
}

int bridge_tcp_client_poll(bridge_tcp_client_callback_t callback, void* user_data)
{
    if (sock_fd < 0) return -1;
    
    uint8_t buf[4096];
    ssize_t n = recv(sock_fd, buf, sizeof(buf), MSG_DONTWAIT);
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;  /* No data available */
        }
        perror("recv");
        return -1;
    }
    
    if (n == 0) {
        /* Connection closed by server */
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }
    
    if (bridge_stream_feed(&stream, buf, n) < 0) {
        fprintf(stderr, "Stream buffer overflow\n");
        return -1;
    }
    
    int frames_processed = 0;
    uint16_t type;
    uint8_t payload[4096];
    size_t payload_size;
    
    while (bridge_stream_extract(&stream, &type, payload, &payload_size) > 0) {
        if (callback) {
            callback(type, payload, payload_size, user_data);
        }
        frames_processed++;
    }
    
    return frames_processed;
}

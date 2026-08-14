#include "tcp_server.h"
#include "bridge_proto.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include <string.h>

static const char *TAG = "tcp_server";

typedef struct {
    int socket;
    bool active;
} tcp_client_t;

static tcp_client_t clients[TCP_SERVER_MAX_CLIENTS];
static bool running = false;

void tcp_server_broadcast_can(const can_frame_t* frame)
{
    if (!frame) return;

    uint8_t buf[64];
    bridge_can_frame_t payload;
    payload.can_id = frame->id;
    payload.dlc = frame->dlc;
    payload.timestamp_ms = frame->timestamp_ms;
    memcpy(payload.data, frame->data, 8);

    size_t size = bridge_proto_encode(buf, sizeof(buf),
                                      BRIDGE_MSG_CAN_FRAME,
                                      &payload, sizeof(payload));
    if (size == 0) return;

    for (int i = 0; i < TCP_SERVER_MAX_CLIENTS; i++) {
        if (clients[i].active) {
            if (send(clients[i].socket, buf, size, 0) < 0) {
                clients[i].active = false;
            }
        }
    }
}

static void handle_client(int client_sock)
{
    ESP_LOGI(TAG, "Client connected");

    uint8_t buf[64];
    bridge_hello_ack_t ack = {
        .protocol_version = 1,
        .device_serial = 0x12345678
    };
    size_t size = bridge_proto_encode(buf, sizeof(buf),
                                      BRIDGE_MSG_HELLO_ACK, &ack, sizeof(ack));
    send(client_sock, buf, size, 0);

    uint64_t last_heartbeat = 0;

    while (running) {
        uint64_t now = esp_timer_get_time() / 1000;

        if (now - last_heartbeat >= 5000) {
            bridge_heartbeat_t hb = { .uptime_ms = now };
            size = bridge_proto_encode(buf, sizeof(buf),
                                       BRIDGE_MSG_HEARTBEAT, &hb, sizeof(hb));
            if (send(client_sock, buf, size, 0) < 0) break;
            last_heartbeat = now;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    close(client_sock);
    ESP_LOGI(TAG, "Client disconnected");
}

static void server_task_fn(void* arg)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { ESP_LOGE(TAG, "socket failed"); return; }

    int enable = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TCP_SERVER_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "bind failed"); close(server_sock); return;
    }
    if (listen(server_sock, TCP_SERVER_MAX_CLIENTS) < 0) {
        ESP_LOGE(TAG, "listen failed"); close(server_sock); return;
    }

    ESP_LOGI(TAG, "TCP server listening on port %d", TCP_SERVER_PORT);

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;

        int slot = -1;
        for (int i = 0; i < TCP_SERVER_MAX_CLIENTS; i++) {
            if (!clients[i].active) { slot = i; break; }
        }

        if (slot >= 0) {
            clients[slot].socket = client_sock;
            clients[slot].active = true;
            handle_client(client_sock);
            clients[slot].active = false;
        } else {
            ESP_LOGW(TAG, "Max clients reached, rejecting");
            close(client_sock);
        }
    }

    close(server_sock);
    vTaskDelete(NULL);
}

int tcp_server_start(void)
{
    running = true;
    memset(clients, 0, sizeof(clients));

    if (xTaskCreate(server_task_fn, "tcp_server", 4096, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create server task");
        return -1;
    }

    ESP_LOGI(TAG, "TCP server started");
    return 0;
}

void tcp_server_stop(void)
{
    running = false;
    ESP_LOGI(TAG, "TCP server stopped");
}
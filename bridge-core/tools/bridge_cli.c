#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "bridge_tcp_client.h"

static volatile int running = 1;

static void signal_handler(int sig)
{
    running = 0;
}

static void frame_callback(uint16_t type, const void* payload, size_t payload_size, void* user_data)
{
    switch (type) {
        case BRIDGE_MSG_HELLO_ACK: {
            const bridge_hello_ack_t* ack = (const bridge_hello_ack_t*)payload;
            printf("[HELLO_ACK] protocol_version=%u, device_serial=0x%08X\n",
                   ack->protocol_version, ack->device_serial);
            break;
        }
        case BRIDGE_MSG_HEARTBEAT: {
            const bridge_heartbeat_t* hb = (const bridge_heartbeat_t*)payload;
            printf("[HEARTBEAT] uptime=%llu ms\n", (unsigned long long)hb->uptime_ms);
            break;
        }
        case BRIDGE_MSG_CAN_FRAME: {
            const bridge_can_frame_t* frame = (const bridge_can_frame_t*)payload;
            printf("[CAN] ID=0x%03X, DLC=%u, data=[", frame->can_id, frame->dlc);
            for (int i = 0; i < frame->dlc && i < 8; i++) {
                printf("%02X", frame->data[i]);
                if (i < frame->dlc - 1) printf(" ");
            }
            printf("], timestamp=%llu\n", (unsigned long long)frame->timestamp_ms);
            break;
        }
        default:
            printf("[UNKNOWN] type=0x%04X, size=%zu\n", type, payload_size);
            break;
    }
}

int main(int argc, char* argv[])
{
    const char* ip = BRIDGE_TCP_CLIENT_DEFAULT_IP;
    uint16_t port = BRIDGE_TCP_CLIENT_DEFAULT_PORT;
    
    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = (uint16_t)atoi(argv[2]);
    
    printf("bridge_cli: connecting to %s:%u\n", ip, port);
    
    signal(SIGINT, signal_handler);
    
    if (bridge_tcp_client_connect(ip, port) < 0) {
        fprintf(stderr, "Failed to connect\n");
        return 1;
    }
    
    printf("bridge_cli: connected, reading frames...\n");
    
    while (running) {
        int res = bridge_tcp_client_poll(frame_callback, NULL);
        if (res < 0) {
            fprintf(stderr, "Connection lost\n");
            break;
        }
        usleep(10000);  /* 10ms */
    }
    
    bridge_tcp_client_disconnect();
    printf("\nbridge_cli: disconnected\n");
    
    return 0;
}

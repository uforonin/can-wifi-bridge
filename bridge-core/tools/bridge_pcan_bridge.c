#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "bridge_tcp_client.h"

#ifdef __APPLE__
    #include <PCBUSB.h>
#else
    #include <PCANBasic.h>
#endif

static volatile int running = 1;
static uint32_t tx_ok = 0;
static uint32_t tx_fail = 0;
static uint32_t rx_ok = 0;
static unsigned int last_err = 0;
static uint64_t last_stat_ms = 0;

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec / 1000000);
}

static void signal_handler(int sig)
{
    running = 0;
}

static void frame_callback(uint16_t type, const void* payload, size_t payload_size, void* user_data)
{
    if (type != BRIDGE_MSG_CAN_FRAME) return;

    const bridge_can_frame_t* frame = (const bridge_can_frame_t*)payload;

    TPCANMsg msg;
    msg.ID = frame->can_id;
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    msg.LEN = frame->dlc;

    for (int i = 0; i < frame->dlc && i < 8; i++) {
        msg.DATA[i] = frame->data[i];
    }

    TPCANStatus status = CAN_Write(PCAN_USBBUS1, &msg);
    if (status == PCAN_ERROR_OK) {
        tx_ok++;
    } else {
        tx_fail++;
        last_err = (unsigned int)status;
    }
}

int main(int argc, char* argv[])
{
    const char* ip = BRIDGE_TCP_CLIENT_DEFAULT_IP;
    uint16_t port = BRIDGE_TCP_CLIENT_DEFAULT_PORT;

    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = (uint16_t)atoi(argv[2]);

    printf("bridge_pcan_bridge: initializing PCAN-USB\n");

    TPCANStatus status = CAN_Initialize(PCAN_USBBUS1, PCAN_BAUD_500K, 0, 0, 0);
    if (status != PCAN_ERROR_OK) {
        fprintf(stderr, "CAN_Initialize failed: 0x%X\n", (unsigned int)status);
        return 1;
    }

    printf("bridge_pcan_bridge: PCAN-USB initialized at 500 kbit/s\n");
    printf("bridge_pcan_bridge: NOTE: empty CAN bus has no ACK ->\n");
    printf("bridge_pcan_bridge: tx_fail=0x80 is EXPECTED until a 2nd node is on the bus\n");
    printf("bridge_pcan_bridge: connecting to %s:%u\n", ip, port);

    signal(SIGINT, signal_handler);

    if (bridge_tcp_client_connect(ip, port) < 0) {
        fprintf(stderr, "Failed to connect to ESP32\n");
        CAN_Uninitialize(PCAN_USBBUS1);
        return 1;
    }

    printf("bridge_pcan_bridge: connected, forwarding TCP->CAN, reading CAN...\n");

    while (running) {
        int res = bridge_tcp_client_poll(frame_callback, NULL);
        if (res < 0) {
            fprintf(stderr, "Connection lost\n");
            break;
        }

        /* Poll PCAN RX (same process owns the adapter) */
        TPCANMsg msg;
        TPCANTimestamp ts;
        TPCANStatus st = CAN_Read(PCAN_USBBUS1, &msg, &ts);
        if (st == PCAN_ERROR_OK) {
            rx_ok++;
            printf("[PCAN-RX] ID=0x%03X, DLC=%u, data=[", (unsigned int)msg.ID, msg.LEN);
            for (int i = 0; i < msg.LEN && i < 8; i++) {
                printf("%02X", msg.DATA[i]);
                if (i < msg.LEN - 1) printf(" ");
            }
            printf("]\n");
        }

        uint64_t now = now_ms();
        if (now - last_stat_ms >= 1000) {
            printf("[stat] tx_ok=%u tx_fail=%u rx_ok=%u last_err=0x%X\n",
                   tx_ok, tx_fail, rx_ok, last_err);
            last_stat_ms = now;
        }

        usleep(10000);
    }

    bridge_tcp_client_disconnect();
    CAN_Uninitialize(PCAN_USBBUS1);

    printf("\nbridge_pcan_bridge: stopped (tx_ok=%u tx_fail=%u rx_ok=%u)\n", tx_ok, tx_fail, rx_ok);
    return 0;
}

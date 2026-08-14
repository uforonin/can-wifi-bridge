#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#ifdef __APPLE__
    #include <PCBUSB.h>
#else
    #include <PCANBasic.h>
#endif

static volatile int running = 1;

static void signal_handler(int sig)
{
    running = 0;
}

int main(void)
{
    printf("bridge_pcan_reader: initializing PCAN-USB\n");
    
    TPCANStatus status = CAN_Initialize(PCAN_USBBUS1, PCAN_BAUD_500K, 0, 0, 0);
    if (status != PCAN_ERROR_OK) {
        fprintf(stderr, "CAN_Initialize failed: 0x%X\n", (unsigned int)status);
        fprintf(stderr, "Check that PCAN-USB is connected\n");
        return 1;
    }
    
    printf("bridge_pcan_reader: PCAN-USB initialized at 500 kbit/s\n");
    printf("bridge_pcan_reader: reading CAN frames (Ctrl+C to stop)...\n");
    
    signal(SIGINT, signal_handler);
    
    while (running) {
        TPCANMsg msg;
        TPCANTimestamp timestamp;
        
        status = CAN_Read(PCAN_USBBUS1, &msg, &timestamp);
        
        if (status == PCAN_ERROR_OK) {
            printf("[PCAN] ID=0x%03X, DLC=%u, data=[", (unsigned int)msg.ID, msg.LEN);
            for (int i = 0; i < msg.LEN && i < 8; i++) {
                printf("%02X", msg.DATA[i]);
                if (i < msg.LEN - 1) printf(" ");
            }
            printf("]\n");
        } else if (status != PCAN_ERROR_QRCVEMPTY) {
            fprintf(stderr, "CAN_Read error: 0x%X\n", (unsigned int)status);
        }
        
        usleep(1000);  /* 1ms */
    }
    
    CAN_Uninitialize(PCAN_USBBUS1);
    printf("\nbridge_pcan_reader: stopped\n");
    
    return 0;
}

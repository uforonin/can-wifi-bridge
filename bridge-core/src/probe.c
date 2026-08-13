#include <stdio.h>

#ifdef __APPLE__
    #include <PCBUSB.h>
#else
    #include <PCANBasic.h>
#endif

int main(void) {
    printf("bridge_probe: starting...\n");
    printf("bridge_probe: attempting to open PCAN_USBBUS1 at 500 kbit/s\n");
    
    /* Пытаемся открыть первый USB-канал на скорости 500 kbit/s */
    TPCANStatus status = CAN_Initialize(PCAN_USBBUS1, PCAN_BAUD_500K, 0, 0, 0);
    
    if (status == PCAN_ERROR_OK) {
        printf("bridge_probe: SUCCESS - PCAN_USBBUS1 opened at 500 kbit/s\n");
        
        TPCANStatus uninit_status = CAN_Uninitialize(PCAN_USBBUS1);
        if (uninit_status == PCAN_ERROR_OK) {
            printf("bridge_probe: channel closed cleanly\n");
        } else {
            printf("bridge_probe: warning - uninitialize failed with 0x%X\n", 
                   (unsigned int)uninit_status);
        }
        return 0;
    } else {
        printf("bridge_probe: FAILED - status = 0x%X\n", (unsigned int)status);
        printf("bridge_probe: possible causes:\n");
        printf("  - PCAN-USB adapter not connected\n");
        printf("  - PCBUSB driver not installed properly\n");
        printf("  - adapter busy or used by another application\n");
        return 1;
    }
}

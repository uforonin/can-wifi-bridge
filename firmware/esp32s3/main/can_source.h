#ifndef CAN_SOURCE_H
#define CAN_SOURCE_H

#include <stdint.h>
#include <stdbool.h>

#define CAN_SOURCE_MAX_DLC 8

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[CAN_SOURCE_MAX_DLC];
    uint64_t timestamp_ms;
} can_frame_t;

typedef struct can_source can_source_t;

/* Interface functions */
typedef int (*can_source_start_fn)(can_source_t* ctx);
typedef int (*can_source_stop_fn)(can_source_t* ctx);
typedef int (*can_source_read_fn)(can_source_t* ctx, can_frame_t* frame, uint32_t timeout_ms);
typedef int (*can_source_write_fn)(can_source_t* ctx, const can_frame_t* frame);

struct can_source {
    void* priv;  /* Private data for implementation */
    can_source_start_fn start;
    can_source_stop_fn stop;
    can_source_read_fn read;
    can_source_write_fn write;
};

/* Factory functions */
can_source_t* can_source_create_emul(void);
can_source_t* can_source_create_twai(uint32_t bitrate);

void can_source_destroy(can_source_t* src);

#endif /* CAN_SOURCE_H */

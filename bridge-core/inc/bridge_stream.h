#ifndef BRIDGE_STREAM_H
#define BRIDGE_STREAM_H

#include <stdint.h>
#include <stddef.h>
#include "bridge_proto.h"

#define BRIDGE_STREAM_BUF_SIZE 4096

typedef struct {
    uint8_t buf[BRIDGE_STREAM_BUF_SIZE];
    size_t len;
} bridge_stream_t;

/* Initialize stream */
void bridge_stream_init(bridge_stream_t* s);

/* Feed raw TCP bytes into stream. Returns number of complete frames extracted */
int bridge_stream_feed(bridge_stream_t* s, const uint8_t* data, size_t data_len);

/* Extract next complete frame. Returns 1 if frame extracted, 0 if not enough data */
int bridge_stream_extract(bridge_stream_t* s, uint16_t* type, void* payload, size_t* payload_size);

/* Reset stream (clear buffer) */
void bridge_stream_reset(bridge_stream_t* s);

#endif /* BRIDGE_STREAM_H */

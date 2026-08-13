#ifndef BRIDGE_TRANSPORT_H
#define BRIDGE_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BRIDGE_TRANSPORT_MAX_STREAMS 8
#define BRIDGE_TRANSPORT_MAX_PAYLOAD 512
#define BRIDGE_TRANSPORT_TIMEOUT_MS 500

typedef struct {
    uint32_t can_id;
    uint8_t payload[BRIDGE_TRANSPORT_MAX_PAYLOAD];
    size_t size;
    uint64_t last_update_ms;
    bool active;
} bridge_stream_t;

typedef struct {
    bridge_stream_t streams[BRIDGE_TRANSPORT_MAX_STREAMS];
    uint32_t stat_frames_ok;
    uint32_t stat_frames_dropped;
} bridge_transport_t;

void bridge_transport_init(bridge_transport_t* t);
void bridge_transport_reset(bridge_transport_t* t);

/* Feeds raw CAN data. Returns 1 if added, 0 if dropped/ignored, -1 on error */
int bridge_transport_feed(bridge_transport_t* t, uint32_t can_id, const uint8_t* data, uint8_t dlc, uint64_t current_time_ms);

size_t bridge_transport_get_size(const bridge_transport_t* t, uint32_t can_id);
const uint8_t* bridge_transport_get_payload(const bridge_transport_t* t, uint32_t can_id);
void bridge_transport_clear_stream(bridge_transport_t* t, uint32_t can_id);

void bridge_transport_get_stats(const bridge_transport_t* t, uint32_t* ok, uint32_t* dropped);

#endif /* BRIDGE_TRANSPORT_H */

#include "bridge_transport.h"
#include <string.h>

void bridge_transport_init(bridge_transport_t* t) {
    if (!t) return;
    memset(t, 0, sizeof(bridge_transport_t));
}

void bridge_transport_reset(bridge_transport_t* t) {
    if (!t) return;
    for (int i = 0; i < BRIDGE_TRANSPORT_MAX_STREAMS; i++) {
        t->streams[i].active = false;
        t->streams[i].size = 0;
    }
    t->stat_frames_ok = 0;
    t->stat_frames_dropped = 0;
}

static bridge_stream_t* find_stream(bridge_transport_t* t, uint32_t can_id) {
    for (int i = 0; i < BRIDGE_TRANSPORT_MAX_STREAMS; i++) {
        if (t->streams[i].active && t->streams[i].can_id == can_id) {
            return &t->streams[i];
        }
    }
    return NULL;
}

static const bridge_stream_t* find_stream_const(const bridge_transport_t* t, uint32_t can_id) {
    for (int i = 0; i < BRIDGE_TRANSPORT_MAX_STREAMS; i++) {
        if (t->streams[i].active && t->streams[i].can_id == can_id) {
            return &t->streams[i];
        }
    }
    return NULL;
}

static bridge_stream_t* create_stream(bridge_transport_t* t, uint32_t can_id) {
    for (int i = 0; i < BRIDGE_TRANSPORT_MAX_STREAMS; i++) {
        if (!t->streams[i].active) {
            t->streams[i].active = true;
            t->streams[i].can_id = can_id;
            t->streams[i].size = 0;
            return &t->streams[i];
        }
    }
    return NULL;
}

int bridge_transport_feed(bridge_transport_t* t, uint32_t can_id, const uint8_t* data, uint8_t dlc, uint64_t current_time_ms) {
    if (!t || !data || dlc == 0) return -1;

    bridge_stream_t* s = find_stream(t, can_id);

    if (s) {
        /* Timeout check */
        if ((current_time_ms - s->last_update_ms) > BRIDGE_TRANSPORT_TIMEOUT_MS) {
            s->size = 0; 
            t->stat_frames_dropped++;
        }
    } else {
        s = create_stream(t, can_id);
        if (!s) {
            t->stat_frames_dropped++;
            return 0; 
        }
    }

    if (s->size + dlc <= BRIDGE_TRANSPORT_MAX_PAYLOAD) {
        memcpy(&s->payload[s->size], data, dlc);
        s->size += dlc;
        s->last_update_ms = current_time_ms;
        t->stat_frames_ok++;
        return 1;
    } else {
        t->stat_frames_dropped++;
        return 0;
    }
}

size_t bridge_transport_get_size(const bridge_transport_t* t, uint32_t can_id) {
    if (!t) return 0;
    const bridge_stream_t* s = find_stream_const(t, can_id);
    return s ? s->size : 0;
}

const uint8_t* bridge_transport_get_payload(const bridge_transport_t* t, uint32_t can_id) {
    if (!t) return NULL;
    const bridge_stream_t* s = find_stream_const(t, can_id);
    return s ? s->payload : NULL;
}

void bridge_transport_clear_stream(bridge_transport_t* t, uint32_t can_id) {
    if (!t) return;
    bridge_stream_t* s = find_stream(t, can_id);
    if (s) {
        s->active = false;
        s->size = 0;
    }
}

void bridge_transport_get_stats(const bridge_transport_t* t, uint32_t* ok, uint32_t* dropped) {
    if (!t) return;
    if (ok) *ok = t->stat_frames_ok;
    if (dropped) *dropped = t->stat_frames_dropped;
}

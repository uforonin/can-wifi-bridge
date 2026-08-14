#include "bridge_stream.h"
#include <string.h>

void bridge_stream_init(bridge_stream_t* s)
{
    if (!s) return;
    memset(s->buf, 0, sizeof(s->buf));
    s->len = 0;
}

int bridge_stream_feed(bridge_stream_t* s, const uint8_t* data, size_t data_len)
{
    if (!s || !data || data_len == 0) return -1;
    
    /* Check buffer overflow */
    if (s->len + data_len > BRIDGE_STREAM_BUF_SIZE) {
        return -1;
    }
    
    memcpy(s->buf + s->len, data, data_len);
    s->len += data_len;
    
    return 0;
}

int bridge_stream_extract(bridge_stream_t* s, uint16_t* type, void* payload, size_t* payload_size)
{
    if (!s || !type || !payload || !payload_size) return -1;
    
    /* Need at least header */
    if (s->len < sizeof(bridge_frame_header_t)) {
        return 0;
    }
    
    /* Decode header */
    bridge_frame_header_t header;
    memcpy(&header, s->buf, sizeof(header));
    
    /* Check if we have full payload */
    size_t total = sizeof(header) + header.length;
    if (s->len < total) {
        return 0;
    }
    
    /* Extract payload */
    memcpy(payload, s->buf + sizeof(header), header.length);
    *type = header.type;
    *payload_size = header.length;
    
    /* Shift buffer */
    memmove(s->buf, s->buf + total, s->len - total);
    s->len -= total;
    
    return 1;
}

void bridge_stream_reset(bridge_stream_t* s)
{
    if (!s) return;
    s->len = 0;
}

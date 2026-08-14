//
// Created by Владимир on 14.08.2026.
//
#include "bridge_proto.h"
#include <string.h>

size_t bridge_proto_encode(uint8_t* buf, size_t buf_size, uint16_t type, const void* payload, size_t payload_size)
{
    if (!buf || buf_size < sizeof(bridge_frame_header_t) + payload_size) {
        return 0;
    }

    bridge_frame_header_t header = {
        .type = type,
        .length = (uint16_t)payload_size
    };

    memcpy(buf, &header, sizeof(header));

    if (payload && payload_size > 0) {
        memcpy(buf + sizeof(header), payload, payload_size);
    }

    return sizeof(header) + payload_size;
}

const void* bridge_proto_decode(const uint8_t* buf, size_t buf_size, uint16_t* type, size_t* payload_size)
{
    if (!buf || buf_size < sizeof(bridge_frame_header_t)) {
        return NULL;
    }

    bridge_frame_header_t header;
    memcpy(&header, buf, sizeof(header));

    if (buf_size < sizeof(header) + header.length) {
        return NULL;
    }

    if (type) *type = header.type;
    if (payload_size) *payload_size = header.length;

    return buf + sizeof(header);
}
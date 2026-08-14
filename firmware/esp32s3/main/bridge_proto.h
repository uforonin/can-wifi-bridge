//
// Created by Владимир on 14.08.2026.
//

#ifndef BRIDGE_PROTO_H
#define BRIDGE_PROTO_H

#include <stdint.h>
#include <stddef.h>

/* Message types */
#define BRIDGE_MSG_HELLO      0x0001
#define BRIDGE_MSG_HELLO_ACK  0x0002
#define BRIDGE_MSG_CAN_FRAME  0x0003
#define BRIDGE_MSG_HEARTBEAT  0x0004

/* Frame header */
typedef struct {
    uint16_t type;
    uint16_t length;
} __attribute__((packed)) bridge_frame_header_t;

/* HELLO payload (client → server) */
typedef struct {
    uint32_t client_id;
} __attribute__((packed)) bridge_hello_t;

/* HELLO_ACK payload (server → client) */
typedef struct {
    uint16_t protocol_version;
    uint32_t device_serial;
} __attribute__((packed)) bridge_hello_ack_t;

/* CAN_FRAME payload (server → client) */
typedef struct {
    uint32_t can_id;
    uint8_t dlc;
    uint8_t data[8];
    uint64_t timestamp_ms;
} __attribute__((packed)) bridge_can_frame_t;

/* HEARTBEAT payload (server → client) */
typedef struct {
    uint64_t uptime_ms;
} __attribute__((packed)) bridge_heartbeat_t;

/* Encode frame into buffer. Returns total size (header + payload) */
size_t bridge_proto_encode(uint8_t* buf, size_t buf_size, uint16_t type, const void* payload, size_t payload_size);

/* Decode frame from buffer. Returns payload pointer and size, or NULL on error */
const void* bridge_proto_decode(const uint8_t* buf, size_t buf_size, uint16_t* type, size_t* payload_size);

#endif /* BRIDGE_PROTO_H */
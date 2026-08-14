#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bridge_stream.h"

#define TEST(name) void name(void)
#define RUN_TEST(name) do { printf("Running %s... ", #name); name(); printf("PASSED\n"); } while(0)

TEST(test_full_frame) {
    bridge_stream_t s;
    bridge_stream_init(&s);
    
    /* Encode one CAN_FRAME */
    uint8_t buf[64];
    bridge_can_frame_t can = {
        .can_id = 0x123,
        .dlc = 8,
        .data = {1,2,3,4,5,6,7,8},
        .timestamp_ms = 1000
    };
    size_t size = bridge_proto_encode(buf, sizeof(buf), BRIDGE_MSG_CAN_FRAME, &can, sizeof(can));
    
    /* Feed full frame */
    int res = bridge_stream_feed(&s, buf, size);
    assert(res == 0);
    
    /* Extract frame */
    uint16_t type;
    bridge_can_frame_t extracted;
    size_t extracted_size;
    res = bridge_stream_extract(&s, &type, &extracted, &extracted_size);
    assert(res == 1);
    assert(type == BRIDGE_MSG_CAN_FRAME);
    assert(extracted_size == sizeof(can));
    assert(extracted.can_id == 0x123);
    assert(extracted.dlc == 8);
    assert(extracted.data[0] == 1);
    
    /* No more frames */
    res = bridge_stream_extract(&s, &type, &extracted, &extracted_size);
    assert(res == 0);
}

TEST(test_split_frame) {
    bridge_stream_t s;
    bridge_stream_init(&s);
    
    uint8_t buf[64];
    bridge_can_frame_t can = {
        .can_id = 0x456,
        .dlc = 4,
        .data = {0xAA, 0xBB, 0xCC, 0xDD},
        .timestamp_ms = 2000
    };
    size_t size = bridge_proto_encode(buf, sizeof(buf), BRIDGE_MSG_CAN_FRAME, &can, sizeof(can));
    
    /* Feed first 10 bytes */
    bridge_stream_feed(&s, buf, 10);
    
    /* Try extract — not enough */
    uint16_t type;
    bridge_can_frame_t extracted;
    size_t extracted_size;
    int res = bridge_stream_extract(&s, &type, &extracted, &extracted_size);
    assert(res == 0);
    
    /* Feed rest */
    bridge_stream_feed(&s, buf + 10, size - 10);
    
    /* Now extract should work */
    res = bridge_stream_extract(&s, &type, &extracted, &extracted_size);
    assert(res == 1);
    assert(extracted.can_id == 0x456);
    assert(extracted.data[0] == 0xAA);
}

TEST(test_multiple_frames) {
    bridge_stream_t s;
    bridge_stream_init(&s);
    
    uint8_t buf[128];
    size_t offset = 0;
    
    /* Frame 1: HEARTBEAT */
    bridge_heartbeat_t hb1 = { .uptime_ms = 5000 };
    offset += bridge_proto_encode(buf + offset, sizeof(buf) - offset, BRIDGE_MSG_HEARTBEAT, &hb1, sizeof(hb1));
    
    /* Frame 2: CAN_FRAME */
    bridge_can_frame_t can1 = {
        .can_id = 0x789,
        .dlc = 2,
        .data = {0x11, 0x22},
        .timestamp_ms = 3000
    };
    offset += bridge_proto_encode(buf + offset, sizeof(buf) - offset, BRIDGE_MSG_CAN_FRAME, &can1, sizeof(can1));
    
    /* Feed all at once */
    bridge_stream_feed(&s, buf, offset);
    
    /* Extract frame 1 */
    uint16_t type;
    uint8_t payload_buf[64];
    size_t extracted_size;
    int res = bridge_stream_extract(&s, &type, payload_buf, &extracted_size);
    assert(res == 1);
    assert(type == BRIDGE_MSG_HEARTBEAT);
    
    /* Extract frame 2 */
    res = bridge_stream_extract(&s, &type, payload_buf, &extracted_size);
    assert(res == 1);
    assert(type == BRIDGE_MSG_CAN_FRAME);
    bridge_can_frame_t* can_out = (bridge_can_frame_t*)payload_buf;
    assert(can_out->can_id == 0x789);
    
    /* No more */
    res = bridge_stream_extract(&s, &type, payload_buf, &extracted_size);
    assert(res == 0);
}

TEST(test_header_only) {
    bridge_stream_t s;
    bridge_stream_init(&s);
    
    /* Feed only header (4 bytes) */
    uint8_t header[4] = {0x03, 0x00, 0x15, 0x00};  /* CAN_FRAME, length=21 */
    bridge_stream_feed(&s, header, 4);
    
    /* Try extract — not enough */
    uint16_t type;
    uint8_t payload[64];
    size_t extracted_size;
    int res = bridge_stream_extract(&s, &type, payload, &extracted_size);
    assert(res == 0);
}

int main(void) {
    printf("=== Stream Unit Tests ===\n");
    RUN_TEST(test_full_frame);
    RUN_TEST(test_split_frame);
    RUN_TEST(test_multiple_frames);
    RUN_TEST(test_header_only);
    printf("ALL TESTS PASSED\n");
    return 0;
}

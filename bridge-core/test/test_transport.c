#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bridge_transport.h"

#define TEST(name) void name(void)
#define RUN_TEST(name) do { printf("Running %s... ", #name); name(); printf("PASSED\n"); } while(0)

TEST(test_init_and_reset) {
    bridge_transport_t t;
    bridge_transport_init(&t);
    assert(t.stat_frames_ok == 0);
    
    bridge_transport_feed(&t, 0x100, (uint8_t[]){1,2,3}, 3, 1000);
    assert(t.stat_frames_ok == 1);
    
    bridge_transport_reset(&t);
    assert(t.stat_frames_ok == 0);
    assert(bridge_transport_get_size(&t, 0x100) == 0);
}

TEST(test_accumulation) {
    bridge_transport_t t;
    bridge_transport_init(&t);
    
    uint8_t data1[] = {0xAA, 0xBB, 0xCC};
    int res = bridge_transport_feed(&t, 0x200, data1, 3, 1000);
    assert(res == 1);
    assert(bridge_transport_get_size(&t, 0x200) == 3);
    
    uint8_t data2[] = {0xDD, 0xEE};
    res = bridge_transport_feed(&t, 0x200, data2, 2, 1100);
    assert(res == 1);
    assert(bridge_transport_get_size(&t, 0x200) == 5);
    
    const uint8_t* payload = bridge_transport_get_payload(&t, 0x200);
    assert(payload != NULL);
    assert(payload[0] == 0xAA);
    assert(payload[3] == 0xDD);
    
    bridge_transport_clear_stream(&t, 0x200);
    assert(bridge_transport_get_size(&t, 0x200) == 0);
}

TEST(test_timeout_drop) {
    bridge_transport_t t;
    bridge_transport_init(&t);
    
    uint8_t data1[] = {0x11};
    bridge_transport_feed(&t, 0x300, data1, 1, 1000);
    assert(bridge_transport_get_size(&t, 0x300) == 1);
    
    /* Feed after timeout (500ms) */
    uint8_t data2[] = {0x22};
    bridge_transport_feed(&t, 0x300, data2, 1, 1600); 
    
    /* Size should be reset to 1 (only data2), not 2 */
    assert(bridge_transport_get_size(&t, 0x300) == 1);
    const uint8_t* payload = bridge_transport_get_payload(&t, 0x300);
    assert(payload[0] == 0x22);
}

TEST(test_stats) {
    bridge_transport_t t;
    bridge_transport_init(&t);
    
    bridge_transport_feed(&t, 0x400, (uint8_t[]){1}, 1, 1000);
    bridge_transport_feed(&t, 0x400, (uint8_t[]){2}, 1, 1100);
    
    uint32_t ok = 0, dropped = 0;
    bridge_transport_get_stats(&t, &ok, &dropped);
    assert(ok == 2);
    assert(dropped == 0);
}

int main(void) {
    printf("=== Transport Unit Tests ===\n");
    RUN_TEST(test_init_and_reset);
    RUN_TEST(test_accumulation);
    RUN_TEST(test_timeout_drop);
    RUN_TEST(test_stats);
    printf("ALL TESTS PASSED\n");
    return 0;
}

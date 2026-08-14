#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bridge_protocol.h"

#define TEST(name) void name(void)
#define RUN_TEST(name) do { printf("Running %s... ", #name); name(); printf("PASSED\n"); } while(0)

TEST(test_parse_status_valid) {
    /* Payload: voltage=3300mV (0xCE, 0x0C), temp=25.5°C -> raw=2550 (0xF6, 0x09), rssi=-65dBm (0xBF), version=1 */
    uint8_t payload[] = {0xCE, 0x0C, 0xF6, 0x09, 0xBF, 0x01, 0x00, 0x00};
    bridge_device_status_t status;
    
    int res = bridge_protocol_parse_status(payload, sizeof(payload), &status);
    assert(res == 1);
    assert(status.voltage_mv == 3300);
    assert(status.temperature_c == 2550);
    assert(status.wifi_rssi_dbm == -65);
    assert(status.protocol_version == 1);
}

TEST(test_parse_status_invalid_size) {
    uint8_t payload[] = {0x01, 0x02};
    bridge_device_status_t status;
    
    int res = bridge_protocol_parse_status(payload, 2, &status);
    assert(res == 0);
}

TEST(test_build_cmd) {
    bridge_device_cmd_t cmd = {0x10, 0x05};
    uint8_t out[8];
    
    size_t size = bridge_protocol_build_cmd(&cmd, out);
    assert(size == 8);
    assert(out[0] == 0x10);
    assert(out[1] == 0x05);
    assert(out[2] == 0x00); /* Rest should be zeroed */
}

TEST(test_get_msg_type) {
    assert(bridge_protocol_get_msg_type(0x100) == BRIDGE_MSG_DEVICE_STATUS);
    assert(bridge_protocol_get_msg_type(0x200) == BRIDGE_MSG_DEVICE_CMD);
    assert(bridge_protocol_get_msg_type(0x999) == BRIDGE_MSG_UNKNOWN);
}

TEST(test_unit_conversions) {
    /* Test macros */
    assert(BRIDGE_PROTOCOL_VOLTAGE_TO_MV(3300) == 3300);
    
    float temp = BRIDGE_PROTOCOL_TEMP_TO_C(2550);
    assert(temp > 25.4f && temp < 25.6f);
    
    assert(BRIDGE_PROTOCOL_RSSI_TO_DBM(-65) == -65);
}

int main(void) {
    printf("=== Protocol Unit Tests ===\n");
    RUN_TEST(test_parse_status_valid);
    RUN_TEST(test_parse_status_invalid_size);
    RUN_TEST(test_build_cmd);
    RUN_TEST(test_get_msg_type);
    RUN_TEST(test_unit_conversions);
    printf("ALL TESTS PASSED\n");
    return 0;
}

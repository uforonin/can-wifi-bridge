#include "bridge_protocol.h"
#include <string.h>

int bridge_protocol_parse_status(const uint8_t* payload, size_t size, bridge_device_status_t* status) {
    if (!payload || !status) return 0;
    
    /* Expected size: at least 6 bytes for voltage + temperature + rssi + version */
    if (size < 6) return 0;
    
    memset(status, 0, sizeof(bridge_device_status_t));
    
    /* Parse fields (little-endian assumed, will verify with real data) */
    status->voltage_mv = (uint16_t)(payload[0] | (payload[1] << 8));
    status->temperature_c = (int16_t)(payload[2] | (payload[3] << 8));
    status->wifi_rssi_dbm = (int8_t)payload[4];
    status->protocol_version = payload[5];
    
    /* Timestamp will be set by caller */
    status->timestamp_ms = 0;
    
    return 1;
}

size_t bridge_protocol_build_cmd(const bridge_device_cmd_t* cmd, uint8_t* out_data) {
    if (!cmd || !out_data) return 0;
    
    memset(out_data, 0, 8);
    out_data[0] = cmd->cmd_id;
    out_data[1] = cmd->param;
    
    return 8; /* CAN frame is always 8 bytes */
}

bridge_msg_type_t bridge_protocol_get_msg_type(uint32_t can_id) {
    if (can_id == BRIDGE_PROTOCOL_CAN_ID_STATUS) {
        return BRIDGE_MSG_DEVICE_STATUS;
    } else if (can_id == BRIDGE_PROTOCOL_CAN_ID_CMD) {
        return BRIDGE_MSG_DEVICE_CMD;
    }
    return BRIDGE_MSG_UNKNOWN;
}

#ifndef BRIDGE_PROTOCOL_H
#define BRIDGE_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* CAN IDs */
#define BRIDGE_PROTOCOL_CAN_ID_STATUS  0x100
#define BRIDGE_PROTOCOL_CAN_ID_CMD     0x200

/* Message types */
typedef enum {
    BRIDGE_MSG_UNKNOWN = 0,
    BRIDGE_MSG_DEVICE_STATUS,
    BRIDGE_MSG_DEVICE_CMD,
} bridge_msg_type_t;

/* Parsed status structure */
typedef struct {
    uint16_t voltage_mv;      /* Raw: u16, Unit: mV, Scale: 1.0 */
    int16_t temperature_c;    /* Raw: i16, Unit: °C, Scale: 0.01 */
    int8_t wifi_rssi_dbm;     /* Raw: i8, Unit: dBm, Scale: 1.0 */
    uint8_t protocol_version; /* Raw: u8 */
    uint64_t timestamp_ms;    /* When this message was received */
} bridge_device_status_t;

/* Command structure (from PC to device) */
typedef struct {
    uint8_t cmd_id;
    uint8_t param;
} bridge_device_cmd_t;

/* Parse raw payload into status structure. Returns 1 on success, 0 on error */
int bridge_protocol_parse_status(const uint8_t* payload, size_t size, bridge_device_status_t* status);

/* Build command frame. Returns size of frame (always 8), fills out_data */
size_t bridge_protocol_build_cmd(const bridge_device_cmd_t* cmd, uint8_t* out_data);

/* Get message type from CAN ID */
bridge_msg_type_t bridge_protocol_get_msg_type(uint32_t can_id);

/* Unit conversion macros */
#define BRIDGE_PROTOCOL_VOLTAGE_TO_MV(raw)   ((uint16_t)(raw))
#define BRIDGE_PROTOCOL_TEMP_TO_C(raw)       ((float)(raw) * 0.01f)
#define BRIDGE_PROTOCOL_RSSI_TO_DBM(raw)     ((int8_t)(raw))

#endif /* BRIDGE_PROTOCOL_H */

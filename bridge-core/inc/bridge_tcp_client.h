#ifndef BRIDGE_TCP_CLIENT_H
#define BRIDGE_TCP_CLIENT_H

#include <stdint.h>
#include <stddef.h>
#include "bridge_proto.h"

#define BRIDGE_TCP_CLIENT_DEFAULT_IP   "192.168.4.1"
#define BRIDGE_TCP_CLIENT_DEFAULT_PORT 5000

typedef void (*bridge_tcp_client_callback_t)(uint16_t type, const void* payload, size_t payload_size, void* user_data);

/* Connect to TCP server. Returns 0 on success, -1 on error */
int bridge_tcp_client_connect(const char* ip, uint16_t port);

/* Disconnect */
void bridge_tcp_client_disconnect(void);

/* Check if connected */
int bridge_tcp_client_is_connected(void);

/* Read data and call callback for each complete frame.
   Returns number of frames processed, 0 if no data, -1 on error */
int bridge_tcp_client_poll(bridge_tcp_client_callback_t callback, void* user_data);

#endif /* BRIDGE_TCP_CLIENT_H */

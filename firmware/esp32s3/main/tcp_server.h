//
// Created by Владимир on 14.08.2026.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "can_source.h"

#define TCP_SERVER_PORT 5000
#define TCP_SERVER_MAX_CLIENTS 4

int tcp_server_start(void);
void tcp_server_stop(void);

/* Push a CAN frame to all connected clients (called by the single reader) */
void tcp_server_broadcast_can(const can_frame_t* frame);

#endif /* TCP_SERVER_H */
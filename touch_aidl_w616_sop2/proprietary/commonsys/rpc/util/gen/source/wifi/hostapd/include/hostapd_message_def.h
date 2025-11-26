/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t HostapdMsg;

#define HOSTAPD_REQ_BASE                                                            ((HostapdMsg)(0x0000))

#define HOSTAPD_ADD_ACCESS_POINT_REQ                                                (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0000)
#define HOSTAPD_FORCE_CLIENT_DISCONNECT_REQ                                         (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0001)
#define HOSTAPD_REGISTER_CALLBACK_REQ                                               (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0002)
#define HOSTAPD_REMOVE_ACCESS_POINT_REQ                                             (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0003)
#define HOSTAPD_SET_DEBUG_PARAMS_REQ                                                (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0004)
#define HOSTAPD_TERMINATE_REQ                                                       (HostapdMsg)(HOSTAPD_REQ_BASE + 0x0005)

#define HOSTAPD_REQ_COUNT                                                           (HOSTAPD_TERMINATE_REQ - HOSTAPD_ADD_ACCESS_POINT_REQ + 1)
#define IS_HOSTAPD_REQ(t)                                                           (((t) >= HOSTAPD_ADD_ACCESS_POINT_REQ) && ((t) <= HOSTAPD_TERMINATE_REQ))

#define HOSTAPD_IND_BASE                                                            ((HostapdMsg)(0x8000))

#define HOSTAPD_ON_AP_INSTANCE_INFO_CHANGED_IND                                     (HostapdMsg)(HOSTAPD_IND_BASE + 0x0000)
#define HOSTAPD_ON_CONNECTED_CLIENTS_CHANGED_IND                                    (HostapdMsg)(HOSTAPD_IND_BASE + 0x0001)
#define HOSTAPD_ON_FAILURE_IND                                                      (HostapdMsg)(HOSTAPD_IND_BASE + 0x0002)

#define HOSTAPD_IND_COUNT                                                           (HOSTAPD_ON_FAILURE_IND - HOSTAPD_ON_AP_INSTANCE_INFO_CHANGED_IND + 1)
#define IS_HOSTAPD_IND(t)                                                           (((t) >= HOSTAPD_ON_AP_INSTANCE_INFO_CHANGED_IND) && ((t) <= HOSTAPD_ON_FAILURE_IND))
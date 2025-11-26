/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t VendorMsg;

#define HOSTAPD_VENDOR_OFFSET                                                       0x1000

#define HOSTAPD_VENDOR_REQ_BASE                                                     ((VendorMsg)(0x0000 + HOSTAPD_VENDOR_OFFSET))

#define HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_REQ                                   (VendorMsg)(HOSTAPD_VENDOR_REQ_BASE + 0x0000)
#define HOSTAPD_VENDOR_REGISTER_HOSTAPD_VENDOR_CALLBACK_REQ                         (VendorMsg)(HOSTAPD_VENDOR_REQ_BASE + 0x0001)
#define HOSTAPD_VENDOR_DO_DRIVER_CMD_REQ                                            (VendorMsg)(HOSTAPD_VENDOR_REQ_BASE + 0x0002)

#define HOSTAPD_VENDOR_REQ_COUNT                                                    (HOSTAPD_VENDOR_DO_DRIVER_CMD_REQ - HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_REQ + 1)
#define IS_HOSTAPD_VENDOR_REQ(t)                                                    (((t) >= HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_REQ) && ((t) <= HOSTAPD_VENDOR_DO_DRIVER_CMD_REQ))

#define HOSTAPD_VENDOR_IND_BASE                                                     ((VendorMsg)(0x8000 + HOSTAPD_VENDOR_OFFSET))

#define HOSTAPD_VENDOR_ON_CTRL_EVENT_IND                                            (VendorMsg)(HOSTAPD_VENDOR_IND_BASE + 0x0000)
#define HOSTAPD_VENDOR_ON_AP_INSTANCE_INFO_CHANGED_IND                              (VendorMsg)(HOSTAPD_VENDOR_IND_BASE + 0x0001)
#define HOSTAPD_VENDOR_ON_FAILURE_IND                                               (VendorMsg)(HOSTAPD_VENDOR_IND_BASE + 0x0002)

#define HOSTAPD_VENDOR_IND_COUNT                                                    (HOSTAPD_VENDOR_ON_FAILURE_IND - HOSTAPD_VENDOR_ON_CTRL_EVENT_IND + 1)
#define IS_HOSTAPD_VENDOR_IND(t)                                                    (((t) >= HOSTAPD_VENDOR_ON_CTRL_EVENT_IND) && ((t) <= HOSTAPD_VENDOR_ON_FAILURE_IND))
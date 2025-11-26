/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t VendorMsg;

#define SUPPLICANT_VENDOR_OFFSET                                                    0x1000

#define SUPPLICANT_VENDOR_REQ_BASE                                                  ((VendorMsg)(0x0000 + SUPPLICANT_VENDOR_OFFSET))

#define SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_REQ                                  (VendorMsg)(SUPPLICANT_VENDOR_REQ_BASE + 0x0000)
#define SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_REQ                                (VendorMsg)(SUPPLICANT_VENDOR_REQ_BASE + 0x0001)

#define SUPPLICANT_VENDOR_REQ_COUNT                                                 (SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_REQ - SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_REQ + 1)
#define IS_SUPPLICANT_VENDOR_REQ(t)                                                 (((t) >= SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_REQ) && ((t) <= SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_REQ))

#define SUPPLICANT_VENDOR_STA_IFACE_REQ_BASE                                        ((VendorMsg)(0x0100 + SUPPLICANT_VENDOR_OFFSET))

#define SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_REQ                               (VendorMsg)(SUPPLICANT_VENDOR_STA_IFACE_REQ_BASE + 0x0000)
#define SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_REQ \
                                                                                    (VendorMsg)(SUPPLICANT_VENDOR_STA_IFACE_REQ_BASE + 0x0001)

#define SUPPLICANT_VENDOR_STA_IFACE_REQ_COUNT                                       (SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_REQ - SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_REQ + 1)
#define IS_SUPPLICANT_VENDOR_STA_IFACE_REQ(t)                                       (((t) >= SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_REQ) && ((t) <= SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_REQ))

#define SUPPLICANT_VENDOR_STA_IFACE_IND_BASE                                        ((VendorMsg)(0x8000 + SUPPLICANT_VENDOR_OFFSET))

#define SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND                               (VendorMsg)(SUPPLICANT_VENDOR_STA_IFACE_IND_BASE + 0x0000)

#define SUPPLICANT_VENDOR_STA_IFACE_IND_COUNT                                       (SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND - SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND + 1)
#define IS_SUPPLICANT_VENDOR_STA_IFACE_IND(t)                                       (((t) >= SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND) && ((t) <= SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND))
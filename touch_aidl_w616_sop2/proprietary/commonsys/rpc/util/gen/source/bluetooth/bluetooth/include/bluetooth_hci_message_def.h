/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t BluetoothMsg;

#define BLUETOOTH_HCI_REQ_BASE                                                      ((BluetoothMsg)(0x0000))

#define BLUETOOTH_HCI_CLOSE_REQ                                                     (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0000)
#define BLUETOOTH_HCI_INITIALIZE_REQ                                                (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0001)
#define BLUETOOTH_HCI_SEND_ACL_DATA_REQ                                             (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0002)
#define BLUETOOTH_HCI_SEND_HCI_COMMAND_REQ                                          (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0003)
#define BLUETOOTH_HCI_SEND_ISO_DATA_REQ                                             (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0004)
#define BLUETOOTH_HCI_SEND_SCO_DATA_REQ                                             (BluetoothMsg)(BLUETOOTH_HCI_REQ_BASE + 0x0005)

#define BLUETOOTH_HCI_REQ_COUNT                                                     (BLUETOOTH_HCI_SEND_SCO_DATA_REQ - BLUETOOTH_HCI_CLOSE_REQ + 1)
#define IS_BLUETOOTH_HCI_REQ(t)                                                     (((t) >= BLUETOOTH_HCI_CLOSE_REQ) && ((t) <= BLUETOOTH_HCI_SEND_SCO_DATA_REQ))

#define BLUETOOTH_HCI_IND_BASE                                                      ((BluetoothMsg)(0x8000))

#define BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND                                         (BluetoothMsg)(BLUETOOTH_HCI_IND_BASE + 0x0000)
#define BLUETOOTH_HCI_HCI_EVENT_RECEIVED_IND                                        (BluetoothMsg)(BLUETOOTH_HCI_IND_BASE + 0x0001)
#define BLUETOOTH_HCI_INITIALIZATION_COMPLETE_IND                                   (BluetoothMsg)(BLUETOOTH_HCI_IND_BASE + 0x0002)
#define BLUETOOTH_HCI_ISO_DATA_RECEIVED_IND                                         (BluetoothMsg)(BLUETOOTH_HCI_IND_BASE + 0x0003)
#define BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND                                         (BluetoothMsg)(BLUETOOTH_HCI_IND_BASE + 0x0004)

#define BLUETOOTH_HCI_IND_COUNT                                                     (BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND - BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND + 1)
#define IS_BLUETOOTH_HCI_IND(t)                                                     (((t) >= BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND) && ((t) <= BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND))
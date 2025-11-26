/******************************************************************************
Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

REVISION:      $Revision: #1 $
******************************************************************************/

#ifndef _BT_HCI_CALLBACK_H_
#define _BT_HCI_CALLBACK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t    HciStatus;
#define HCI_STATUS_SUCCESS                          ((HciStatus) 0)
#define HCI_STATUS_ALREADY_INITIALIZED              ((HciStatus) 1)
#define HCI_STATUS_UNABLE_TO_OPEN_INTERFACE         ((HciStatus) 2)
#define HCI_STATUS_HARDWARE_INITIALIZATION_ERROR    ((HciStatus) 3)
#define HCI_STATUS_UNKNOWN                          ((HciStatus) 4)

#define IS_SUCCESS(s)                               ((s) == HCI_STATUS_SUCCESS)

/* Callback to handle HCI initialization complete */
typedef void (* HciCbInitializationComplete)(HciStatus status);

/* Callback to handle HCI event */
typedef void (* HciCbEventReceived)(const uint8_t *data, size_t length);

/* Callback to handle HCI ACL data */
typedef void (* HciCbAclDataReceived)(const uint8_t *data, size_t length);

/* Callback to handle HCI SCO data */
typedef void (* HciCbScoDataReceived)(const uint8_t *data, size_t length);

/* Callback to handle HCI ISO data */
typedef void (* HciCbIsoDataReceived)(const uint8_t *data, size_t length);

typedef struct
{
    HciCbInitializationComplete initializationCompleteCb;   /* Callback for HCI initialization complete */
    HciCbEventReceived          hciEventReceivedCb;         /* Callback for HCI event received */
    HciCbAclDataReceived        aclDataReceivedCb;          /* Callback for ACL data received */
    HciCbScoDataReceived        scoDataReceivedCb;          /* Callback for SCO data received */
    HciCbIsoDataReceived        isoDataReceivedCb;          /* Callback for ISO data received */
} BtHciCallbacks;

#ifdef __cplusplus
}
#endif

#endif

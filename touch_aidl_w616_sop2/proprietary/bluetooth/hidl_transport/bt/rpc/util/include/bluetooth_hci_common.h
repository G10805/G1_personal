/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t BtHciMsg;

/*******************************************************************************
 * Primitive definitions
 *******************************************************************************/

/* BT HCI request (downstream primitive) */
#define BT_HCI_CLOSE_REQ            ((BtHciMsg) (0x0000))
#define BT_HCI_INITIALIZE_REQ       ((BtHciMsg) (0x0001))
#define BT_HCI_ACL_DATA_REQ         ((BtHciMsg) (0x0002))
#define BT_HCI_COMMAND_REQ          ((BtHciMsg) (0x0003))
#define BT_HCI_ISO_DATA_REQ         ((BtHciMsg) (0x0004))
#define BT_HCI_SCO_DATA_REQ         ((BtHciMsg) (0x0005))

/* BT HCI indicator (upstream primitive) */
#define BT_HCI_ACL_DATA_IND         ((BtHciMsg) (0x8000))
#define BT_HCI_EVENT_IND            ((BtHciMsg) (0x8001))
#define BT_HCI_INIT_COMPLETE_IND    ((BtHciMsg) (0x8002))
#define BT_HCI_ISO_DATA_IND         ((BtHciMsg) (0x8003))
#define BT_HCI_SCO_DATA_IND         ((BtHciMsg) (0x8004))


void *BtHciMsgSerialize(void *msg);

void BtHciMsgParse(void *msg, uint8_t *data, size_t length);

void BtHciMsgGetPayload(void *buf, uint8_t **data, size_t *length);

void BtHciMsgFree(void *buf);

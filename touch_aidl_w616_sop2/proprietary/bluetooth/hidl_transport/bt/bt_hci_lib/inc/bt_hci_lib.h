/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _BT_HCI_LIB_H_
#define _BT_HCI_LIB_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bt_hci_callback.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 3 Byte = 2 Byte (OpCode) + 1 Byte (Parameter Total Length) */
#define MIN_HCI_CMD_SIZE                    3

/* 1 Byte */
#define MAX_HCI_CMD_SIZE                    0xFF

/* 4 Byte = 2 Byte (Handle plus flag) + 2 Byte (Data Total Length) */
#define MIN_HCI_ACL_SIZE                    4

/* 2 Byte */
#define MAX_HCI_ACL_SIZE                    0xFFFF

/* 3 Byte = 2 Byte (Connection_Handle plus flag) + 1 Byte (Data_Total_Length) */
#define MIN_HCI_SCO_SIZE                    3

/* 1 Byte */
#define MAX_HCI_SCO_SIZE                    0xFF

typedef enum
{
    BT_CHIP_UNINITIALIZED,
    BT_CHIP_INITIALZING,
    BT_CHIP_INITIALIZED,
    BT_CHIP_DEINITIALZING,
} BTChipInitStatus_t;

#define VALID_HCI_CMD(data, length)         (((data) != NULL) && \
                                            ((length) >= MIN_HCI_CMD_SIZE) && \
                                            ((length) <= MAX_HCI_CMD_SIZE))

#define VALID_HCI_ACL_DATA(data, length)    (((data) != NULL) && \
                                            ((length) >= MIN_HCI_ACL_SIZE) && \
                                            ((length) <= MAX_HCI_ACL_SIZE))

#define VALID_HCI_SCO_DATA(data, length)    (((data) != NULL) && \
                                            ((length) >= MIN_HCI_SCO_SIZE) && \
                                            ((length) <= MAX_HCI_SCO_SIZE))



void bt_hci_initialize_once();

bool bt_hci_open(BtHciCallbacks *hciCallback);

bool bt_hci_send_command(const uint8_t *data, size_t length);

bool bt_hci_send_acl_data(const uint8_t *data, size_t length);

bool bt_hci_send_sco_data(const uint8_t *data, size_t length);

bool bt_hci_send_iso_data(const uint8_t *data, size_t length);

void bt_hci_close(void);

void bt_hci_deinitialize(void);

#ifdef __cplusplus
}
#endif

#endif  /* _BT_HCI_LIB_H_ */

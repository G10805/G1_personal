/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <android-base/macros.h>

#include "bt_hci_lib.h"
#include "data_handler.h"

using android::hardware::bluetooth::V1_0::implementation::DataHandler;
using android::hardware::hidl_vec;

typedef struct
{
    bool           initialized;     /* true: instance is initialized, false: not */
    BtHciCallbacks hciCallback;     /* BT HCI callbacks */
} HciInstance;

static HciInstance sHciInstance;

static HciInstance *get_hci_instance();
static BtHciCallbacks *get_hci_callback();
static bool open_hci();
static void initialize_callback(bool success);
static void data_read_callback(HciPacketType type, const hidl_vec<uint8_t>* packet);
static void send_data_to_controller(HciPacketType type, const uint8_t *data, size_t length);


void bt_hci_initialize_once()
{
    HciInstance *inst = get_hci_instance();

    memset(inst, 0, sizeof(HciInstance));

    inst->initialized = true;
}

bool bt_hci_open(BtHciCallbacks *hciCallback)
{
    HciInstance *inst = get_hci_instance();

    if (!hciCallback)
        return false;

    memcpy(&inst->hciCallback, hciCallback, sizeof(BtHciCallbacks));

    if (!open_hci())
    {
        if (hciCallback->initializationCompleteCb)
        {
            hciCallback->initializationCompleteCb(HCI_STATUS_UNKNOWN);
        }
        return false;
    }

    return true;
}

bool bt_hci_send_command(const uint8_t *data, size_t length)
{
    send_data_to_controller(HCI_PACKET_TYPE_COMMAND, data, length);
    return true;
}

bool bt_hci_send_acl_data(const uint8_t *data, size_t length)
{
    send_data_to_controller(HCI_PACKET_TYPE_ACL_DATA, data, length);
    return true;
}

bool bt_hci_send_sco_data(const uint8_t *data, size_t length)
{
    /* NOT supported */
    UNUSED(data);
    UNUSED(length);
    return false;
}

bool bt_hci_send_iso_data(const uint8_t *data, size_t length)
{
    send_data_to_controller(HCI_PACKET_TYPE_ISO_DATA, data, length);
    return true;
}

void bt_hci_close(void)
{
    DataHandler::CleanUp(TYPE_BT);
}

void bt_hci_deinitialize(void)
{
    HciInstance *inst = get_hci_instance();
    inst->initialized = false;
}

static HciInstance *get_hci_instance()
{
    return &sHciInstance;
}

static BtHciCallbacks *get_hci_callback()
{
    HciInstance *inst = get_hci_instance();
    return &inst->hciCallback;
}

static bool open_hci()
{
    return DataHandler::Init(TYPE_BT, initialize_callback, data_read_callback);
}

static void initialize_callback(bool success)
{
    BtHciCallbacks *hciCallback = get_hci_callback();
    HciStatus status = success ? HCI_STATUS_SUCCESS : HCI_STATUS_HARDWARE_INITIALIZATION_ERROR;
    if (hciCallback->initializationCompleteCb)
    {
       hciCallback->initializationCompleteCb(status);
    }
}

static void data_read_callback(HciPacketType type, const hidl_vec<uint8_t>* packet)
{
    BtHciCallbacks *hciCallback = get_hci_callback();
    uint8_t *data = packet ? (uint8_t *) packet->data() : NULL;
    size_t size = packet ? packet->size() : 0;

    switch (type)
    {
        case HCI_PACKET_TYPE_EVENT:
        {
            if (hciCallback->hciEventReceivedCb)
            {
                hciCallback->hciEventReceivedCb(data, size);
            }
            break;
        }
        case HCI_PACKET_TYPE_ACL_DATA:
        {
            if (hciCallback->aclDataReceivedCb)
            {
                hciCallback->aclDataReceivedCb(data, size);
            }
            break;
        }
        case HCI_PACKET_TYPE_SCO_DATA:
        {
            /* NOT supported */
            break;
        }
        case HCI_PACKET_TYPE_ISO_DATA:
        {
            if (hciCallback->isoDataReceivedCb)
            {
                hciCallback->isoDataReceivedCb(data, size);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

static void send_data_to_controller(HciPacketType type, const uint8_t *data, size_t length)
{
    DataHandler *data_handler = DataHandler::Get();

    if (data_handler != nullptr)
    {
        data_handler->SendData(TYPE_BT, type, data, length);
    }
}
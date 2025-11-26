/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <utils/Log.h>

#include <sys/stat.h>
#include <string>

#include "bluetooth_hci_ind.h"

#include "BluetoothHciCallbacks.pb.h"
#include "Status.pb.h"

using std::string;

// Protobuf generated classes.
using BluetoothHci::Status;
using BluetoothHci::InitCompleteInd;
using BluetoothHci::AclDataInd;
using BluetoothHci::EventInd;
using BluetoothHci::IsoDataInd;
using BluetoothHci::ScoDataInd;

using google::protobuf::MessageLite;


void *BtHciMsgCreateInitCompleteInd(uint32_t status)
{
    InitCompleteInd msg;
    msg.set_status((Status) status);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateAclDataInd(const vector<uint8_t>& data)
{
    AclDataInd msg;
    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateEventInd(const vector<uint8_t>& event)
{
    EventInd msg;
    string str(event.begin(), event.end());
    msg.set_event(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateIsoDataInd(const vector<uint8_t>& data)
{
    IsoDataInd msg;
    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateScoDataInd(const vector<uint8_t>& data)
{
    ScoDataInd msg;
    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

static void *ParseInitCompleteInd(uint8_t *data, size_t length)
{
    InitCompleteInd msg;
    BtHciMsgParse(&msg, data, length);
    return new string(std::to_string(msg.status()));
}

static void *ParseAclDataInd(uint8_t *data, size_t length)
{
    AclDataInd msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

static void *ParseEventInd(uint8_t *data, size_t length)
{
    EventInd msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.event());
    return str;
}

static void *ParseIsoDataInd(uint8_t *data, size_t length)
{
    IsoDataInd msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

static void *ParseScoDataInd(uint8_t *data, size_t length)
{
    ScoDataInd msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

bool BtHciMsgParseInd(void **msg, uint16_t type, uint8_t *data, size_t length)
{
    void *mv = NULL;

    if (!msg)
        return false;

    switch (type)
    {
        case BT_HCI_INIT_COMPLETE_IND:
        {
            mv = ParseInitCompleteInd(data, length);
            break;
        }
        case BT_HCI_ACL_DATA_IND:
        {
            mv = ParseAclDataInd(data, length);
            break;
        }
        case BT_HCI_EVENT_IND:
        {
            mv = ParseEventInd(data, length);
            break;
        }
        case BT_HCI_ISO_DATA_IND:
        {
            mv = ParseIsoDataInd(data, length);
            break;
        }
        case BT_HCI_SCO_DATA_IND:
        {
            mv = ParseScoDataInd(data, length);
            break;
        }
        default:
        {
            return false;
        }
    }

    *msg = mv;
    return true;
}

void BtHciMsgFreeInd(uint16_t /* type */, void *msg)
{
    BtHciMsgFree(msg);
}
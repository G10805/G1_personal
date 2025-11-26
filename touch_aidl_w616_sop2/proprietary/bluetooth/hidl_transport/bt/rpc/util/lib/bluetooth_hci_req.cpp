/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <utils/Log.h>

#include <sys/stat.h>
#include <string>

#include "bluetooth_hci_req.h"

#include "BluetoothHci.pb.h"

using std::string;

// Protobuf generated classes.
using BluetoothHci::CloseReq;
using BluetoothHci::InitializeReq;
using BluetoothHci::AclDataReq;
using BluetoothHci::CommandReq;
using BluetoothHci::IsoDataReq;
using BluetoothHci::ScoDataReq;

using google::protobuf::MessageLite;


void *BtHciMsgCreateCloseReq()
{
    /* empty message */
    return NULL;
}

void *BtHciMsgCreateInitializeReq()
{
    /* empty message */
    return NULL;
}

void *BtHciMsgCreateAclDataReq(const vector<uint8_t>& data)
{
    AclDataReq msg;

    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateCommandReq(const vector<uint8_t>& packet)
{
    CommandReq msg;

    string str(packet.begin(), packet.end());
    msg.set_command(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateIsoDataReq(const vector<uint8_t>& data)
{
    IsoDataReq msg;

    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

void *BtHciMsgCreateScoDataReq(const vector<uint8_t>& data)
{
    ScoDataReq msg;

    string str(data.begin(), data.end());
    msg.set_data(str);
    return BtHciMsgSerialize(&msg);
}

static void *ParseCloseReq(uint8_t * /* data */, size_t /* length */)
{
    /* empty message */
    return NULL;
}

static void *ParseInitializeReq(uint8_t * /* data */, size_t /* length */)
{
    /* empty message */
    return NULL;
}

static void *ParseAclDataReq(uint8_t *data, size_t length)
{
    AclDataReq msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

static void *ParseCommandReq(uint8_t *data, size_t length)
{
    CommandReq msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.command());
    return str;
}

static void *ParseIsoDataReq(uint8_t *data, size_t length)
{
    IsoDataReq msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

static void *ParseScoDataReq(uint8_t *data, size_t length)
{
    ScoDataReq msg;
    BtHciMsgParse(&msg, data, length);
    string *str = new string(msg.data());
    return str;
}

bool BtHciMsgParseReq(void **msg, uint16_t type, uint8_t *data, size_t length)
{
    void *mv = NULL;

    if (!msg)
        return false;

    /* for empty message, data is null and length is 0 */

    switch (type)
    {
        case BT_HCI_CLOSE_REQ:
        {
            mv = ParseCloseReq(data, length);
            break;
        }
        case BT_HCI_INITIALIZE_REQ:
        {
            mv = ParseInitializeReq(data, length);
            break;
        }
        case BT_HCI_ACL_DATA_REQ:
        {
            mv = ParseAclDataReq(data, length);
            break;
        }
        case BT_HCI_COMMAND_REQ:
        {
            mv = ParseCommandReq(data, length);
            break;
        }
        case BT_HCI_ISO_DATA_REQ:
        {
            mv = ParseIsoDataReq(data, length);
            break;
        }
        case BT_HCI_SCO_DATA_REQ:
        {
            mv = ParseScoDataReq(data, length);
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

void BtHciMsgFreeReq(uint16_t /* type */, void *msg)
{
    BtHciMsgFree(msg);
}
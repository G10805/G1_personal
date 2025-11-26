/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <utils/Log.h>

#include <cstdint>
#include <sys/stat.h>
#include <string>

#include "bluetooth_hci_req.h"
#include "bluetooth_hci_ind.h"

using std::string;
using std::vector;

#define VERIFY(f)   { \
                        if (!(f)) \
                            { \
                                ALOGE("%s: %s fail, return false", __func__, #f); \
                                return false; \
                            } \
                    }


static bool COMPARE_BUF(void *buf, void *server_buf)
{
    string *str = static_cast<string *> (buf);
    string *server_str = static_cast<string *> (server_buf);

    if (!buf || !server_buf)
        return false;

    ALOGD("%s: client buf size=%d, server buf size=%d", __func__, str->size(), server_str->size());
    return !str->compare(*server_str);
}

static bool VERIFY_BUF(void *buf, void *server_buf)
{
    bool res = COMPARE_BUF(buf, server_buf);
    if (res)
        ALOGD("%s: same buf", __func__);
    else
        ALOGE("%s: mis-matched buf", __func__);

    BtHciMsgFree(server_buf);
    return res;
}

/* Test BT HCI request (downstream primitive) */
static bool T_BT_HCI_CLOSE_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    void *server_buf = NULL;

    /* client */
    buf = BtHciMsgCreateCloseReq();
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_CLOSE_REQ, data, length);
    /* non-payload */

    BtHciMsgFree(buf);
    return true;
}

static bool T_BT_HCI_INITIALIZE_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    void *server_buf = NULL;

    /* client */
    buf = BtHciMsgCreateInitializeReq();
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_INITIALIZE_REQ, data, length);
    /* non-payload */

    BtHciMsgFree(buf);
    return true;
}

static bool T_BT_HCI_ACL_DATA_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *server_buf = NULL;

    ALOGD("%s: client, serialized packet size=%d", __func__, packet.size());

    /* client */
    buf = BtHciMsgCreateAclDataReq(packet);
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_ACL_DATA_REQ, data, length);

    return VERIFY_BUF(&packet_str, server_buf);
}

static bool T_BT_HCI_COMMAND_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *server_buf = NULL;

    ALOGD("%s: client, serialized packet size=%d", __func__, packet.size());

    /* client */
    buf = BtHciMsgCreateCommandReq(packet);
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_COMMAND_REQ, data, length);

    return VERIFY_BUF(&packet_str, server_buf);
}

static bool T_BT_HCI_ISO_DATA_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *server_buf = NULL;

    ALOGD("%s: client, serialized packet size=%d", __func__, packet.size());

    /* client */
    buf = BtHciMsgCreateIsoDataReq(packet);
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_ISO_DATA_REQ, data, length);

    return VERIFY_BUF(&packet_str, server_buf);
}

static bool T_BT_HCI_SCO_DATA_REQ()
{
    void *buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *server_buf = NULL;

    ALOGD("%s: client, serialized packet size=%d", __func__, packet.size());

    /* client */
    buf = BtHciMsgCreateScoDataReq(packet);
    BtHciMsgGetPayload(buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to server */

    /* server */
    BtHciMsgParseReq(&server_buf, BT_HCI_SCO_DATA_REQ, data, length);

    return VERIFY_BUF(&packet_str, server_buf);
}

/* Test BT HCI indicator (upstream primitive) */
static bool T_BT_HCI_INIT_COMPLETE_IND()
{
    void *server_buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    uint32_t server_status = 1;
    void *buf = NULL;
    string *str = NULL;
    uint32_t status = 0;
    bool res = false;

    ALOGD("%s: server status=%x", __func__, server_status);

    /* server */
    server_buf = BtHciMsgCreateInitCompleteInd(server_status);
    BtHciMsgGetPayload(server_buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to client */

    /* client */
    BtHciMsgParseInd(&buf, BT_HCI_INIT_COMPLETE_IND, data, length);

    str = static_cast<string *> (buf);
    status = (uint32_t) atoi(str->c_str());
    res = (server_status == status);
    if (res)
        ALOGD("%s: same status", __func__);
    else
        ALOGE("%s: mis-matched server_status=%x, status=%x", __func__, server_status, status);

    BtHciMsgFree(buf);
    return res;
}

static bool T_BT_HCI_ACL_DATA_IND()
{
    void *server_buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *buf = NULL;

    ALOGD("%s: server, serialized packet size=%d", __func__, packet.size());

    /* server */
    server_buf = BtHciMsgCreateAclDataInd(packet);
    BtHciMsgGetPayload(server_buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to client */

    /* client */
    BtHciMsgParseInd(&buf, BT_HCI_ACL_DATA_IND, data, length);

    return VERIFY_BUF(&packet_str, buf);
}

static bool T_BT_HCI_EVENT_IND()
{
    void *server_buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *buf = NULL;

    ALOGD("%s: server, serialized packet size=%d", __func__, packet.size());

    /* server */
    server_buf = BtHciMsgCreateEventInd(packet);
    BtHciMsgGetPayload(server_buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to client */

    /* client */
    BtHciMsgParseInd(&buf, BT_HCI_EVENT_IND, data, length);

    return VERIFY_BUF(&packet_str, buf);
}

static bool T_BT_HCI_ISO_DATA_IND()
{
    void *server_buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *buf = NULL;

    ALOGD("%s: server, serialized packet size=%d", __func__, packet.size());

    /* server */
    server_buf = BtHciMsgCreateIsoDataInd(packet);
    BtHciMsgGetPayload(server_buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to client */

    /* client */
    BtHciMsgParseInd(&buf, BT_HCI_ISO_DATA_IND, data, length);

    return VERIFY_BUF(&packet_str, buf);
}

static bool T_BT_HCI_SCO_DATA_IND()
{
    void *server_buf = NULL;
    uint8_t *data = NULL;
    size_t length = 0;
    vector<uint8_t> packet = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 };
    string packet_str(packet.begin(), packet.end());
    void *buf = NULL;

    ALOGD("%s: server, serialized packet size=%d", __func__, packet.size());

    /* server */
    server_buf = BtHciMsgCreateScoDataInd(packet);
    BtHciMsgGetPayload(server_buf, &data, &length);
    ALOGD("%s: msg data=0x%p, length=%d", __func__, data, length);

    /* simulate to send serialized message to client */

    /* client */
    BtHciMsgParseInd(&buf, BT_HCI_SCO_DATA_IND, data, length);

    return VERIFY_BUF(&packet_str, buf);
}

static bool T_ALL()
{
    /* BT HCI request (downstream primitive) */
    VERIFY(T_BT_HCI_CLOSE_REQ());
    VERIFY(T_BT_HCI_INITIALIZE_REQ());
    VERIFY(T_BT_HCI_ACL_DATA_REQ());
    VERIFY(T_BT_HCI_COMMAND_REQ());
    VERIFY(T_BT_HCI_ISO_DATA_REQ());
    /* ParseScoDataReq is NOT implemented */
    // VERIFY(T_BT_HCI_SCO_DATA_REQ());

    /* BT HCI indicator (upstream primitive) */
    VERIFY(T_BT_HCI_INIT_COMPLETE_IND());
    VERIFY(T_BT_HCI_ACL_DATA_IND());
    VERIFY(T_BT_HCI_EVENT_IND());
    VERIFY(T_BT_HCI_ISO_DATA_IND());
    /* ParseScoDataInd is NOT implemented */
    // VERIFY(T_BT_HCI_SCO_DATA_IND());
    return true;
}

static void handleBluetoothHciMessage(int type)
{
    BtHciMsg msg = (BtHciMsg) type;
    switch (msg)
    {
        /* BT HCI request (downstream primitive) */
        case BT_HCI_CLOSE_REQ:
        {
            T_BT_HCI_CLOSE_REQ();
            break;
        }
        case BT_HCI_INITIALIZE_REQ:
        {
            T_BT_HCI_INITIALIZE_REQ();
            break;
        }
        case BT_HCI_ACL_DATA_REQ:
        {
            T_BT_HCI_ACL_DATA_REQ();
            break;
        }
        case BT_HCI_COMMAND_REQ:
        {
            T_BT_HCI_COMMAND_REQ();
            break;
        }
        case BT_HCI_ISO_DATA_REQ:
        {
            T_BT_HCI_ISO_DATA_REQ();
            break;
        }
        case BT_HCI_SCO_DATA_REQ:
        {
            T_BT_HCI_SCO_DATA_REQ();
            break;
        }
        /* BT HCI indicator (upstream primitive) */
        case BT_HCI_INIT_COMPLETE_IND:
        {
            T_BT_HCI_INIT_COMPLETE_IND();
            break;
        }
        case BT_HCI_ACL_DATA_IND:
        {
            T_BT_HCI_ACL_DATA_IND();
            break;
        }
        case BT_HCI_EVENT_IND:
        {
            T_BT_HCI_EVENT_IND();
            break;
        }
        case BT_HCI_ISO_DATA_IND:
        {
            T_BT_HCI_ISO_DATA_IND();
            break;
        }
        case BT_HCI_SCO_DATA_IND:
        {
            T_BT_HCI_SCO_DATA_IND();
            break;
        }
        default:
        {
            ALOGE("%s: Unknown msg 0x%04x", __func__, msg);
            break;
        }
    }
}

int main(int argc, char** argv)
{
    int msgType = 0;

    if (argc <= 1)
    {
        T_ALL();
    }
    else
    {
        sscanf(argv[1], "%x", &msgType);
        handleBluetoothHciMessage(msgType);
    }
    return 0;
}
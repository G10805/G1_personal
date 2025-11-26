/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <utils/Log.h>

#include <sys/stat.h>
#include <string>

#include "bluetooth_hci_common.h"

#include "BluetoothHci.pb.h"

using std::string;

using google::protobuf::MessageLite;


void *BtHciMsgSerialize(void *msg)
{
    MessageLite *msgLite = static_cast<MessageLite *> (msg);
    string *str = new string();
    msgLite->SerializeToString(str);
    return str;
}

void BtHciMsgParse(void *msg, uint8_t *data, size_t length)
{
    MessageLite *msgLite = static_cast<MessageLite *> (msg);

    if (data && length)
    {
        msgLite->ParseFromString(string((char *) data, length));
    }
    else
    {
        /* parse empty string for null payload */
        msgLite->ParseFromString(string());
    }
}

void BtHciMsgGetPayload(void *buf, uint8_t **data, size_t *length)
{
    if (!data || !length)
        return;

    if (!buf)
    {
        *data = NULL;
        *length = 0;
        return;
    }

    string *str = static_cast<string *> (buf);
    size_t size = str->size();
    *length = size;
    *data = size ? ((uint8_t *) str->c_str()) : NULL;
}

void BtHciMsgFree(void *buf)
{
    string *str = static_cast<string *> (buf);
    delete str;
}

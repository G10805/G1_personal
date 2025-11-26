/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <google/protobuf/message_lite.h>
#include "proto_common.h"

using google::protobuf::MessageLite;

bool ProtoMessageSerialize(void* msg, std::vector<uint8_t>& payload)
{
    MessageLite* msgLite = static_cast<MessageLite*> (msg);
    size_t length = msgLite->ByteSizeLong();
    payload.resize(length);

    return msgLite->SerializeToArray(payload.data(), length);
}

bool ProtoMessageParse(const uint8_t* data, size_t length, void* msg)
{
    MessageLite* msgLite = static_cast<MessageLite*> (msg);

    return msgLite->ParseFromArray(data, length);
}

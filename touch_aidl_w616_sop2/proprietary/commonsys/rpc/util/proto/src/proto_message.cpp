/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "proto_message.h"

namespace qti {
namespace hal {
namespace rpc {

Message::Message()
    : Message(0) {
}

Message::Message(uint16_t type)
    : type_(type) {
    payload_.clear();
}

Message::Message(uint16_t type, const uint8_t* data, size_t length)
    : type_(type)
    , payload_(data, data + length) {
}

Message::Message(uint16_t type, const vector<uint8_t>& payload)
    : type_(type)
    , payload_(payload) {
}

shared_ptr<Message> Message::create(uint16_t type) {
    return std::make_shared<Message>(type);
}

shared_ptr<Message> Message::create(uint16_t type, const uint8_t* data, size_t length) {
    return std::make_shared<Message>(type, data, length);
}

shared_ptr<Message> Message::create(uint16_t type, const vector<uint8_t>& payload) {
    return std::make_shared<Message>(type, payload);
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

using ::qti::hal::rpc::Message;
using std::static_pointer_cast;

std::shared_ptr<void> ProtoMessageCreate(uint16_t type, std::vector<uint8_t>& buf) {
    return Message::create(type, buf);
}

bool ProtoMessageParse(const std::shared_ptr<void> mv, uint16_t* type, const uint8_t** data, size_t* length) {
    if (!mv || !type || !data || !length)
        return false;

    std::shared_ptr<Message> sp = std::static_pointer_cast<Message>(mv);
    if (!sp)
        return false;

    *type = sp->getType();
    *data = sp->getData();
    *length = sp->getLength();

    return true;
}

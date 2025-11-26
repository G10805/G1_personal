/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <vector>

namespace qti {
namespace hal {
namespace rpc {

using std::shared_ptr;
using std::vector;

class Message {
  public:
    Message();
    Message(uint16_t type);
    Message(uint16_t type, const uint8_t* data, size_t length);
    Message(uint16_t type, const vector<uint8_t>& payload);
    ~Message() {}

    static shared_ptr<Message> create(uint16_t type);
    static shared_ptr<Message> create(uint16_t type, const uint8_t* data, size_t length);
    static shared_ptr<Message> create(uint16_t type, const vector<uint8_t>& payload);

    uint16_t getType() { return type_; }
    vector<uint8_t>& getPayload() { return payload_; }
    const uint8_t* getData() { return payload_.data(); }
    size_t getLength() { return payload_.size(); }

    inline Message& operator=(const Message& from) {
        if (this == &from) return *this;
        copyFrom(from.type_, from.payload_);
        return *this;
    }

  private:
    inline void copyFrom(uint16_t type, const vector<uint8_t>& payload) {
        this->type_ = type;
        this->payload_ = payload;
    }

    uint16_t type_;
    vector<uint8_t> payload_;
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

std::shared_ptr<void> ProtoMessageCreate(uint16_t type, std::vector<uint8_t>& buf);

bool ProtoMessageParse(const std::shared_ptr<void> mv, uint16_t* type, const uint8_t** data, size_t* length);
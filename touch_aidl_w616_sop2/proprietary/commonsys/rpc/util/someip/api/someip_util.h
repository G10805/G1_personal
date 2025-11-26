/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <vsomeip/vsomeip.hpp>

namespace qti {
namespace hal {
namespace rpc {

class Someip;
class SomeipMessage;

typedef std::function<void(uint16_t, uint16_t, bool)> SomeipAvailabilityHandler;
typedef std::function<void(const std::shared_ptr<SomeipMessage>&)> SomeipMessageHandler;

class SomeipMessage {
  public:
    SomeipMessage();
    SomeipMessage(const std::shared_ptr<vsomeip::message> &_message);
    SomeipMessage(const SomeipMessage &_source);
    ~SomeipMessage();

    SomeipMessage & operator=(const SomeipMessage &_source);

    static std::shared_ptr<vsomeip::message> createRequest(
        uint16_t service_id, uint16_t instance_id, uint16_t message_id,
        const std::vector<uint8_t>& data = {}, bool reliable = false);

    static std::shared_ptr<vsomeip::message> createEvent(
        uint16_t service_id, uint16_t instance_id, uint16_t event_id,
        const std::vector<uint8_t>& data = {}, bool reliable = false);

    static std::shared_ptr<vsomeip::message> createResponse(
        const std::shared_ptr<vsomeip::message>& msg,
        const std::vector<uint8_t>& data = {});
    std::shared_ptr<vsomeip::message> createResponse(
        const std::vector<uint8_t>& data = {});

    bool isRequest();
    bool isEvent();
    bool isResponse();

    uint16_t getServiceId();
    uint16_t getInstanceId();
    uint16_t getMethodId();
    uint8_t* getData();
    size_t getLength();

    std::shared_ptr<vsomeip::payload> getPayload() { return payload_; }
    std::shared_ptr<vsomeip::message> getMessage() { return message_; }

    std::vector<uint8_t>& createPayload(size_t length = 0);
    void setPayload(std::vector<uint8_t>&& data);
    void setPayload(const std::vector<uint8_t>& payload);

  private:
    static void initMessage(std::shared_ptr<vsomeip::message>& message,
        uint16_t service_id, uint16_t instance_id, uint16_t message_id,
        const std::vector<uint8_t>& data);

    static void initPayload(std::shared_ptr<vsomeip::message>& message,
        const std::vector<uint8_t>& data);

    std::vector<uint8_t> data_;
    std::shared_ptr<vsomeip::payload> payload_;
    std::shared_ptr<vsomeip::message> message_;
};

class SomeipCallback {
  public:
    SomeipCallback()
        : SomeipCallback(nullptr, nullptr) {};
    SomeipCallback(SomeipAvailabilityHandler availability_handler_,
        SomeipMessageHandler message_handler_)
        : availability_handler(availability_handler_)
        , message_handler(message_handler_) {}
    ~SomeipCallback() {}

    SomeipCallback & operator=(const SomeipCallback& from) {
        if (this != &from) {
            availability_handler = from.availability_handler;
            message_handler = from.message_handler;
        }
        return *this;
    }

  public:
    SomeipAvailabilityHandler availability_handler;
    SomeipMessageHandler message_handler;
};

class SomeipContext {
  public:
    SomeipContext() {}
    SomeipContext(uint16_t service_id_, uint16_t instance_id_);
    SomeipContext(uint16_t service_id_, uint16_t instance_id_,
        uint16_t eventgroup_id, const std::vector<uint16_t>& event_ids);
    ~SomeipContext();

    SomeipContext& operator=(const SomeipContext& from);

    void addEvents(uint16_t eventgroup_id, const std::vector<uint16_t>& event_ids);

  public:
    uint16_t service_id;
    uint16_t instance_id;
    std::map<uint16_t, std::vector<uint16_t>> event_map;
};

class SomeipKey {
  public:
    SomeipKey() {}
    SomeipKey(uint16_t key1, uint16_t key2) {
        copyFrom(key1, key2);
    }

    inline SomeipKey& operator=(const SomeipKey& from) {
        if (this == &from) return *this;
        copyFrom(from.key1_, from.key2_);
        return *this;
    }
    inline bool operator<(const SomeipKey& other) const {
        return get(key1_, key2_) < get(other.key1_, other.key2_);
    }

    static uint32_t get(uint16_t key1, uint16_t key2) {
        return (uint32_t)(key1) | ((uint32_t)(key2) << 16);
    }
    static uint16_t getKey1(uint32_t key) {
        return (uint16_t)(key & 0x0000FFFF);
    }
    static uint16_t getKey2(uint32_t key) {
        return (uint16_t)((key & 0xFFFF0000) >> 16);
    }
  private:
    inline void copyFrom(uint16_t key1, uint16_t key2) {
        this->key1_ = key1;
        this->key2_ = key2;
    }
  public:
    uint16_t key1_;
    uint16_t key2_;
};

typedef std::function<void(uint16_t, uint16_t, void*)> FreeFunc;

class SomeipMap {
  public:
    SomeipMap(FreeFunc free_func);
    SomeipMap(FreeFunc free_func, size_t max_size);
    ~SomeipMap();

    void add(uint16_t key1, uint16_t key2, void* value);
    void add(uint32_t key, void* value);

    void remove(uint16_t key1, uint16_t key2);
    void remove(uint32_t key);

    void* get(uint16_t key1, uint16_t key2);
    void* get(uint32_t key);

    void clear();

  private:
    void remove(uint16_t key1, uint16_t key2, void* value);

    std::map<uint32_t, void*> map_;
    std::mutex mutex_;
    size_t max_size_;
    FreeFunc free_func_;
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

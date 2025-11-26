/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "someip.h"
#include "someip_util.h"

namespace qti {
namespace hal {
namespace rpc {

SomeipMessage::SomeipMessage()
    : payload_(nullptr), message_(nullptr) {
}

SomeipMessage::SomeipMessage(
    const std::shared_ptr<vsomeip::message> &_message)
    : message_(_message) {
    if (message_)
        payload_ = message_->get_payload();
}

SomeipMessage::SomeipMessage(const SomeipMessage &_source)
    : message_(_source.message_) {
    if (message_)
        payload_ = message_->get_payload();
}

SomeipMessage::~SomeipMessage() {
    data_.clear();
}

SomeipMessage & SomeipMessage::operator=(const SomeipMessage &_source) {
    if (this != &_source)
        message_ = _source.message_;
    if (message_)
        payload_ = message_->get_payload();

    return (*this);
}

std::shared_ptr<vsomeip::message> SomeipMessage::createRequest(
    uint16_t service_id, uint16_t instance_id, uint16_t message_id,
    const std::vector<uint8_t>& data, bool reliable) {
    std::shared_ptr<vsomeip::message> message =
        vsomeip::runtime::get()->create_request(reliable);
    if (!message)
        return nullptr;

    initMessage(message, service_id, instance_id, message_id, data);
    return message;
}

std::shared_ptr<vsomeip::message> SomeipMessage::createEvent(
    uint16_t service_id, uint16_t instance_id, uint16_t event_id,
    const std::vector<uint8_t>& data, bool reliable) {
    std::shared_ptr<vsomeip::message> message =
        vsomeip::runtime::get()->create_notification(reliable);
    if (!message)
        return nullptr;

    initMessage(message, service_id, instance_id, event_id, data);
    return message;
}

std::shared_ptr<vsomeip::message> SomeipMessage::createResponse(
    const std::shared_ptr<vsomeip::message>& msg,
    const std::vector<uint8_t>& data) {
    std::shared_ptr<vsomeip::message> message =
        vsomeip::runtime::get()->create_response(msg);
    if (!message)
        return nullptr;

    initPayload(message, data);
    return message;
}

std::shared_ptr<vsomeip::message> SomeipMessage::createResponse(
    const std::vector<uint8_t>& data) {
    return createResponse(message_, data);
}

void SomeipMessage::initMessage(std::shared_ptr<vsomeip::message>& message,
    uint16_t service_id, uint16_t instance_id, uint16_t message_id,
    const std::vector<uint8_t>& data) {
    message->set_service(service_id);
    message->set_instance(instance_id);
    message->set_method(message_id);
    initPayload(message, data);
}

void SomeipMessage::initPayload(std::shared_ptr<vsomeip::message>& message,
    const std::vector<uint8_t>& data) {
    if (data.size()) {
        std::shared_ptr<vsomeip::payload> payload = message->get_payload();
        if (!payload)
            payload = vsomeip::runtime::get()->create_payload();
        if (!payload)
            return;
        payload->set_data(data);
        message->set_payload(payload);
    }
}

bool SomeipMessage::isRequest() {
    return (message_ &&
        (message_->get_message_type() == vsomeip::message_type_e::MT_REQUEST));
}

bool SomeipMessage::isEvent() {
return (message_ &&
        (message_->get_message_type() ==
            vsomeip::message_type_e::MT_NOTIFICATION));
}

bool SomeipMessage::isResponse() {
    return (message_ &&
        (message_->get_message_type() == vsomeip::message_type_e::MT_RESPONSE));
}

uint16_t SomeipMessage::getServiceId() {
    return message_ ? message_->get_service() : 0;
}

uint16_t SomeipMessage::getInstanceId() {
    return message_ ? message_->get_instance() : 0;
}

uint16_t SomeipMessage::getMethodId() {
    return message_ ? message_->get_method() : 0;
}

uint8_t* SomeipMessage::getData() {
    return payload_ ? payload_->get_data() : NULL;
}

size_t SomeipMessage::getLength() {
    return payload_ ? payload_->get_length() : 0;
}

std::vector<uint8_t>& SomeipMessage::createPayload(size_t length) {
    if (length)
        data_.reserve(length);
    return data_;
}

void SomeipMessage::setPayload(std::vector<uint8_t>&& data) {
    if (!payload_)
       payload_ = vsomeip::runtime::get()->create_payload();
    if (!payload_)
        return;
    payload_->set_data(data);
    message_->set_payload(payload_);
}

void SomeipMessage::setPayload(const std::vector<uint8_t>& data) {
    if (!payload_)
        payload_ = vsomeip::runtime::get()->create_payload();
    if (!payload_)
        return;
    payload_->set_data(data);
    message_->set_payload(payload_);
}

SomeipContext::SomeipContext(uint16_t service_id_, uint16_t instance_id_)
    : service_id(service_id_)
    , instance_id(instance_id_) {
}

SomeipContext::SomeipContext(uint16_t service_id_, uint16_t instance_id_,
    uint16_t eventgroup_id, const std::vector<uint16_t>& event_ids)
    : SomeipContext(service_id_, instance_id_) {
    event_map[eventgroup_id] = event_ids;
}

SomeipContext::~SomeipContext() {
    event_map.clear();
}

SomeipContext& SomeipContext::operator=(const SomeipContext& from) {
    if (this != &from) {
        service_id = from.service_id;
        instance_id = from.instance_id;
        event_map.clear();
        event_map = from.event_map;
    }

    return *this;
}

void SomeipContext::addEvents(uint16_t eventgroup_id,
    const std::vector<uint16_t>& event_ids) {
    event_map[eventgroup_id] = event_ids;
}

SomeipMap::SomeipMap(FreeFunc free_func)
    : SomeipMap(free_func, 0) {
}

SomeipMap::SomeipMap(FreeFunc free_func, size_t max_size)
    : free_func_(free_func)
    , max_size_(max_size) {
}

SomeipMap::~SomeipMap() {
    clear();
}

void SomeipMap::add(uint16_t key1, uint16_t key2, void* value) {
    add(SomeipKey::get(key1, key2), value);
}

void SomeipMap::add(uint32_t key, void* value) {
    const std::lock_guard<std::mutex> lock(mutex_);
    if (max_size_ && (map_.size() >= max_size_)) {
        /* exceed max map size */
        return;
    }
    map_.insert(std::pair<uint32_t, void*>(key, value));
}

void SomeipMap::remove(uint16_t key1, uint16_t key2) {
    remove(SomeipKey::get(key1, key2));
}

void SomeipMap::remove(uint32_t key) {
    const std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it != map_.end()) {
        remove(SomeipKey::getKey1(key), SomeipKey::getKey2(key), it->second);
        map_.erase(it);
    }
}

void* SomeipMap::get(uint16_t key1, uint16_t key2) {
    return get(SomeipKey::get(key1, key2));
}

void* SomeipMap::get(uint32_t key) {
    const std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it != map_.end()) {
        return it->second;
    }
    return NULL;
}

void SomeipMap::clear() {
    const std::lock_guard<std::mutex> lock(mutex_);
    if (!map_.empty()) {
        auto it = map_.begin();
        while (it != map_.end()) {
            uint32_t key = it->first;
            remove(SomeipKey::getKey1(key), SomeipKey::getKey2(key), it->second);
            ++it;
        }
        map_.clear();
    }
}

void SomeipMap::remove(uint16_t key1, uint16_t key2, void* value) {
    if (free_func_)
        free_func_(key1, key2, value);
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

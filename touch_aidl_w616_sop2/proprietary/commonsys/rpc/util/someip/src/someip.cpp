/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifdef ANDROID
#include <cutils/properties.h>
#endif

#include <chrono>

#include "log_common.h"
#include "someip.h"

namespace qti {
namespace hal {
namespace rpc {

#define VSOMEIP_ENV_CONFIGURATION   "VSOMEIP_CONFIGURATION"
#define VSOMEIP_ENV_BASE_PATH       "VSOMEIP_BASE_PATH"

#ifdef ENABLE_RPC_SERVER
#define SOMEIP_CONFIG_FILE          "/vendor/etc/someip/vsomeip_server.json"
#else
#define SOMEIP_CONFIG_FILE          "/vendor/etc/someip/vsomeip.json"
#endif
#define SOMEIP_BASE_PATH            "/data/vendor/someip/"

#define PROPERTY_SOMEIP_CONFIG_FILE "persist.vendor.someip.config_file"
#define PROPERTY_SOMEIP_BASE_PATH   "persist.vendor.someip.base_path"

SomeipCallback Someip::sCallback;

Someip::Someip()
    : Someip("") {
}

Someip::Someip(const std::string& app_name)
    : app_name_(app_name)
    , runtime_(nullptr)
    , app_(nullptr) {
    waiting_for_response_ = false;
}

Someip::~Someip() {
    deinit();
}

void Someip::addService(const SomeipContext& _context) {
    /* TODO : support multiple service context */
    context_ = _context;
}

void Someip::setup(const SomeipCallback& callback) {
    sCallback = callback;
    initLogging();
    initConfiguration();
}

void Someip::destroy() {
    deinitLogging();
}

const SomeipCallback& Someip::getCallback() {
    return sCallback;
}

bool Someip::init() {
    runtime_ = vsomeip::runtime::get();
    if (!runtime_)
        return false;

    app_ = runtime_->create_application(app_name_);
    if (!app_)
        return false;

    return app_->init();
}

void Someip::deinit() {
    app_ = nullptr;
    runtime_ = nullptr;
}

bool Someip::sendMessage(std::shared_ptr<SomeipMessage> message) {
    if (!validApp() || !message)
        return false;

    uint16_t service_id = message->getServiceId();
    uint16_t instance_id = message->getInstanceId();
    uint16_t method_id = message->getMethodId();

    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Send %s Message: [%04x:%04x:%04x] "
        "length %d"), (message->isRequest() ? "Request" :
        (message->isResponse() ? "Response" : "Event")), service_id,
        instance_id, method_id, message->getLength()));

    if (message->isEvent())
        app_->notify(service_id, instance_id, method_id, message->getPayload());
    else
        app_->send(message->getMessage());

    return true;
}

bool Someip::sendRequest(uint16_t method_id, const std::vector<uint8_t>& data, uint16_t* session_id, bool reliable) {
    if (!validApp())
        return false;

    uint16_t service_id = context_.service_id;
    uint16_t instance_id = context_.instance_id;
    std::shared_ptr<vsomeip::message> request =
        SomeipMessage::createRequest(service_id, instance_id, method_id, data, reliable);
    if (!request)
        return false;

    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Send Request: [%04x:%04x:%04x] "
        "length %d"), service_id, instance_id, method_id, data.size()));

    app_->send(request);
    if (session_id) {
        *session_id = request->get_session();
    }
    return true;
}

bool Someip::sendResponse(const std::shared_ptr<vsomeip::message>& msg, const std::vector<uint8_t>& data) {
    if (!validApp() || !msg)
        return false;

    std::shared_ptr<vsomeip::message> response = SomeipMessage::createResponse(msg, data);
    if (!response)
        return false;

    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Send Response: [%04x:%04x:%04x] "
        "length %d"), response->get_service(), response->get_instance(),
        response->get_method(), data.size()));

    app_->send(response);
    return true;
}

bool Someip::sendEvent(uint16_t method_id, const std::vector<uint8_t>& data) {
    if (!validApp())
        return false;

    uint16_t service_id = context_.service_id;
    uint16_t instance_id = context_.instance_id;

    std::shared_ptr<vsomeip::payload> payload = runtime_->create_payload();
    if (!payload)
        return false;
    payload->set_data(data);

    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Send Event: [%04x:%04x:%04x] length %d"),
        service_id, instance_id, method_id, data.size()));

    app_->notify(service_id, instance_id, method_id, payload);
    return true;
}

std::shared_ptr<SomeipMessage> Someip::sendRequestAndWaitForReply(
    uint16_t method_id, const std::vector<uint8_t>& payload, uint32_t timeout) {
    if (!sendRequest(method_id, payload))
        return nullptr;

    return waitForResponse(method_id, timeout);
}

std::shared_ptr<SomeipMessage> Someip::sendRequestAndWaitForReply(
    std::shared_ptr<SomeipMessage> request, uint32_t timeout) {
    if (!sendMessage(request))
        return nullptr;

    return waitForResponse(request->getMethodId(), timeout);
}

void Someip::initLogging() {
    InitLog();
}

void Someip::deinitLogging() {
    DeinitLog();
}

void Someip::initConfiguration() {
#ifdef CONFIG_SOMEIP_ENV
    char str[PROPERTY_VALUE_MAX];

    property_get(PROPERTY_SOMEIP_BASE_PATH, str, SOMEIP_BASE_PATH);

    IFDBG(DebugOut(DEBUG_INFO, TEXT("%s: set env %s as %s"), __func__,
        VSOMEIP_ENV_BASE_PATH, str));

    if (setenv(VSOMEIP_ENV_BASE_PATH, str, 1)) {
        IFDBG(DebugOut(DEBUG_ERROR,
            TEXT("%s: fail to set env for VSOMEIP_BASE_PATH"), __func__));
        return;
    }

    property_get(PROPERTY_SOMEIP_CONFIG_FILE, str, SOMEIP_CONFIG_FILE);

    IFDBG(DebugOut(DEBUG_INFO, TEXT("%s: set env %s as %s"), __func__,
        VSOMEIP_ENV_CONFIGURATION, str));

    if (setenv(VSOMEIP_ENV_CONFIGURATION, str, 1))
        IFDBG(DebugOut(DEBUG_ERROR,
            TEXT("%s: fail to set env for VSOMEIP_CONFIGURATION"), __func__));
#endif
}

std::shared_ptr<SomeipMessage> Someip::waitForResponse(
    uint16_t method_id, uint32_t timeout) {
    std::shared_ptr<SomeipMessage> response = nullptr;

    waiting_for_response_ = true;
    std::chrono::steady_clock::time_point elapsed(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout));
    std::unique_lock<std::mutex> lock(mutex_);
    if (condition_.wait_until(lock, elapsed) == std::cv_status::timeout) {
        IFDBG(DebugOut(DEBUG_ERROR, TEXT("Timeout waiting for response %04x"),
            method_id));
        goto out;
    }

    if (!response_ || response_->getMethodId() != method_id) {
        IFDBG(DebugOut(DEBUG_ERROR, TEXT("Received mismatched response")));
        goto out;
    }

    response = response_;

out:
    waiting_for_response_ = false;
    return response;
}

void Someip::recvResponse(std::shared_ptr<SomeipMessage> response) {
    response_ = response;
    condition_.notify_one();
}

void Someip::onMessage(const std::shared_ptr<vsomeip::message>& msg) {
    if (!msg)
        return;

    std::shared_ptr<SomeipMessage> message =
        std::make_shared<SomeipMessage>(msg);
    if (!message)
        return;

    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Recv %s: [%04x:%04x:%04x] "
        "length %d"), (message->isRequest() ? "Request" :
        (message->isResponse() ? "Response" : "Event")),
        message->getServiceId(), message->getInstanceId(),
        message->getMethodId(), message->getLength()));

    /*
     * When response message received and waiting flag is set, we must
     * notify waiting condition in someip thread, can not schedule
     * to message handler thread which is now waiting for response.
     * If waiting flag not set, response message can be handled in any
     * thread as no blocking case now.
     */
    if (message->isResponse() && waiting_for_response_) {
        recvResponse(message);
        return;
    }

    SomeipMessageHandler message_handler = sCallback.message_handler;
    if (message_handler) {
        message_handler(message);
        return;
    }
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

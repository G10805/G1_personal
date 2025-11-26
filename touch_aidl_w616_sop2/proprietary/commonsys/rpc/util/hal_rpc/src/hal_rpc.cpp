/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>

#include <android-base/logging.h>
#include <android-base/macros.h>

#include "hal_rpc.h"

#include "log_common.h"
#include "proto_message.h"

namespace qti {
namespace hal {
namespace rpc {

using std::pair;

using ::qti::hal::rpc::SomeipClient;
using ::qti::hal::rpc::SomeipKey;
using ::qti::hal::rpc::SomeipServer;

bool HalRpc::sAddHeader = false;
SomeipMap HalRpc::sHalRpc(HalRpc::removeHalRpc);


HalRpc::HalRpc(const string& instanceName, bool syncApi)
    : HalRpc(map2InstanceId(instanceName), syncApi)  {
}

HalRpc::HalRpc(uint16_t instanceId, bool syncApi)
    : service_id_(0)
    , instance_id_(instanceId)
    , sync_api_(syncApi)
    , someip_(nullptr)
    , someip_app_name_("")
    , available_(false)
    , hal_status_(nullptr)
    , proxy_(nullptr)
    , event_handle_(NULL) {

}

HalRpc::~HalRpc() {
    deinit();
}

void HalRpc::initService() {
    SomeipCallback callback(someipAvailabilityHandler, someipMessageHandler);
    Someip::setup(callback);
    openMessageSched();
}

void HalRpc::deinitService() {
    Someip::destroy();
    closeMessageSched();
}

void HalRpc::openMessageSched() {
    MessageSchedRegisterInfo registerInfo;
    registerInfo.messageHandler = messageSchedHandler;
    MessageSchedOpen(&registerInfo);
}

void HalRpc::closeMessageSched() {
    MessageSchedClose();
}

uint16_t HalRpc::map2InstanceId(const string& instanceName) {
    static uint16_t sHalInstanceId = HAL_DEFAULT_INSTANCE_ID;
    if (!instanceName.compare(HAL_DEFAULT_INSTANCE)) {
        return HAL_DEFAULT_INSTANCE_ID;
    } else {
        return ++sHalInstanceId;
    }
}

uint16_t HalRpc::map2InstanceId(SomeipInstanceId instanceId) {
    return (instanceId < SOMEIP_INSTANCE_ID_BASE) ?
            instanceId :
            instanceId - SOMEIP_INSTANCE_ID_BASE;
}

SomeipInstanceId HalRpc::map2SomeipInstanceId(uint16_t instanceId) {
    return (SOMEIP_INSTANCE_ID_BASE + instanceId);
}

void HalRpc::init() {
    initSomeipContext();
    initHalStatus();
    initEventHandle();
    initService();
    /* set hal service id */
    service_id_ = someip_context_.service_id;
    addInstance(service_id_, instance_id_);
}

void HalRpc::deinit() {
    deinitHalStatus();
    deinitEventHandle();
    deinitService();
    deleteInstance(service_id_, instance_id_);
}

bool HalRpc::openSomeip() {
    thread_ = std::thread([this]() { someipThreadRoutine(); });
    if (!thread_.joinable()) {
        return false;
    }
    return true;
}

void HalRpc::closeSomeip() {
    if (someip_ != nullptr) {
        someip_->stop();
    }
    if (std::this_thread::get_id() != thread_.get_id()) {
        thread_.join();
    }
}

bool HalRpc::sendRequest(uint16_t msgType, const vector<uint8_t>& payload, uint16_t* session_id) {
    return sendMessage(HAL_MSG_REQ, NULL, msgType, payload, sAddHeader, session_id);
}

bool HalRpc::sendResponse(const shared_ptr<vsomeip::message>& msg, const vector<uint8_t>& payload) {
    return sendMessage(HAL_MSG_CFM, msg, 0, payload, sAddHeader);
}

bool HalRpc::sendEvent(uint16_t msgType, const vector<uint8_t>& payload) {
    return sendMessage(HAL_MSG_EVT, NULL, msgType, payload, sAddHeader);
}

bool HalRpc::sendMessage(HalMsgT type, const shared_ptr<vsomeip::message>& msg, uint16_t msgType,
    const vector<uint8_t>& payload, bool addHeader, uint16_t* session_id) {
    if (addHeader) {
        vector<uint8_t> data;
        SerializePayload(service_id_, instance_id_, payload, data);
        return sendMessage(type, msg, msgType, data, session_id);
    }
    return sendMessage(type, msg, msgType, payload, session_id);
}

bool HalRpc::sendMessage(HalMsgT type, const shared_ptr<vsomeip::message>& msg, uint16_t msgType,
    const vector<uint8_t>& payload, uint16_t* session_id) {
    if (proxy_ != nullptr) {
        return proxy_->sendMessage(type, msg, msgType, payload, session_id);
    }

    if ((someip_ == nullptr) || !available_) {
        /* someip not available */
        return false;
    }

    switch (type) {
        case HAL_MSG_REQ: {
            return someip_->sendRequest(msgType, payload, session_id);
        }
        case HAL_MSG_CFM: {
            return someip_->sendResponse(msg, payload);
        }
        case HAL_MSG_EVT: {
            return someip_->sendEvent(msgType, payload);
        }
        default: {
            /* unknown hal msg type */
            return false;
        }
    }
}

HalMsgT HalRpc::map2HalMsgType(const shared_ptr<vsomeip::message>& msg) {
    if (msg == nullptr)
        return HAL_MSG_UNKNOWN;

    switch (msg->get_message_type()) {
        case vsomeip::message_type_e::MT_REQUEST:
            /* fall-through */
        case vsomeip::message_type_e::MT_REQUEST_NO_RETURN: {
            return HAL_MSG_REQ;
        }
        case vsomeip::message_type_e::MT_RESPONSE: {
            return HAL_MSG_CFM;
        }
        case vsomeip::message_type_e::MT_NOTIFICATION: {
            return HAL_MSG_EVT;
        }
        default: {
            return HAL_MSG_UNKNOWN;
        }
    }
}

void HalRpc::removeHalRpc(uint16_t /* service_id */, uint16_t /* instance_id */, void* halRpc) {
    delete (reinterpret_cast<HalRpc*>(halRpc));
}

void HalRpc::addInstance(uint16_t service_id, uint16_t instance_id) {
    /* remove old key/value if exist */
    deleteInstance(service_id, instance_id);
    sHalRpc.add(service_id, instance_id, this);
}

void HalRpc::deleteInstance(uint16_t service_id, uint16_t instance_id) {
    sHalRpc.remove(service_id, instance_id);
}

HalRpc* HalRpc::getInstance(uint16_t service_id, uint16_t instance_id) {
    void* halRpc = sHalRpc.get(service_id, instance_id);
    return reinterpret_cast<HalRpc*>(halRpc);
}

void HalRpc::someipThreadRoutine() {
    startSomeip();
}

bool HalRpc::waitHalStatus(HalMsg msgType, uint16_t sessionId, void** halStatus) {
    uint32_t value = SomeipKey::get(msgType, sessionId);
    void* status = NULL;
    bool res = false;

    if (!event_handle_ || !halStatus)
        return false;

    while (1)
    {
        if (!IS_RESULT_SUCCESS(EventWait(event_handle_, EVENT_WAIT_INFINITE)))
        {
            break;
        }

        status = getHalStatus(msgType, sessionId);
        if (!status)
        {
            continue;
        }

        *halStatus = status;
        res = true;
        break;
    }
    return res;
}

void HalRpc::deinitHalStatus() {
    if (hal_status_) {
        hal_status_->clear();
        hal_status_ = nullptr;
    }
}

void HalRpc::addHalStatus(uint16_t message_type, uint16_t session_id, void* status) {
    if (hal_status_) {
        hal_status_->add(message_type, session_id, status);
    }
}

void HalRpc::removeHalStatus(uint16_t message_type, uint16_t session_id) {
    if (hal_status_) {
        hal_status_->remove(message_type, session_id);
    }
}

void* HalRpc::getHalStatus(uint16_t message_type, uint16_t session_id) {
    return hal_status_ ? hal_status_->get(message_type, session_id) : NULL;
}

void HalRpc::initEventHandle() {
    event_handle_ = EventCreate();
}

void HalRpc::deinitEventHandle() {
    EventDestroy(event_handle_);
    event_handle_ = NULL;
}

void HalRpc::someipAvailabilityHandler(uint16_t service_id, uint16_t instance_id, bool available) {
    HalRpc* halRpc = getInstance(service_id, map2InstanceId(instance_id));
    if (!halRpc) {
        return;
    }

    halRpc->setAvailable(available);
}

void HalRpc::someipMessageHandler(const std::shared_ptr<SomeipMessage>& msg) {
    void* mv = CreateMessage(msg);
    MessageSchedSend(mv);
}

void HalRpc::messageSchedHandler(void* mv) {
    SomeipMessage* sm = static_cast<SomeipMessage*> (mv);
    if (!sm)
        return;

    do {
        const shared_ptr<vsomeip::message> msg = sm->getMessage();
        if (!msg)
            break;

        uint16_t service_id = msg->get_service();
        uint16_t instance_id = map2InstanceId(msg->get_instance());
        uint16_t message_type = msg->get_method();
        uint16_t session_id = msg->get_session();
        std::shared_ptr<vsomeip::payload> msg_payload = msg->get_payload();
        uint8_t* data = msg_payload ? msg_payload->get_data() : NULL;
        size_t length = msg_payload ? msg_payload->get_length() : 0;
        uint8_t* payload = data;
        size_t size = length;

        if (sAddHeader) {
            if (!ParsePayload(data, length, service_id, instance_id, payload, size))
                break;
        }

        HalRpc* halRpc = getInstance(service_id, instance_id);
        if (!halRpc)
            break;

        halRpc->handleMsg(message_type, payload, size, msg, session_id);
    } while (0);

    DeleteMessage(mv);
}

void HalRpc::handleMsg(uint16_t message_type, uint8_t* data, size_t length,
    const shared_ptr<vsomeip::message>& msg, uint16_t session_id) {
    HalMsgT type = map2HalMsgType(msg);
    switch (type) {
        case HAL_MSG_REQ: {
            if (sync_api_) {
                vector<uint8_t> result;
                handleHalReq(message_type, data, length, result);
                sendResponse(msg, result);
            } else {
                handleHalReq(message_type, data, length);
            }
            break;
        }
        case HAL_MSG_CFM: {
            void* halStatus = NULL;
            handleHalCfm(message_type, data, length, &halStatus);
            notifyResponse(message_type, session_id, halStatus);
            break;
        }
        case HAL_MSG_EVT: {
            handleHalInd(message_type, data, length);
            break;
        }
        default: {
            break;
        }
    }
}

void HalRpc::setAvailable(bool available) {
    available_ = available;
}

bool HalRpc::isAvailable() {
    return available_;
}

void HalRpc::notifyResponse(HalMsg msgType, uint16_t sessionId, void* halStatus) {
    uint32_t value = SomeipKey::get(msgType, sessionId);
    addHalStatus(msgType, sessionId, halStatus);
    EventSet(event_handle_);
}

HalRpcClient::HalRpcClient(const string& instanceName, bool syncApi)
    : HalRpc(instanceName, syncApi)  {
}

HalRpcClient::HalRpcClient(uint16_t instanceId, bool syncApi)
    : HalRpc(instanceId, syncApi)  {
}

HalRpcClient::~HalRpcClient() {
}

bool HalRpcClient::send(uint16_t msgType, const vector<uint8_t>& payload, uint16_t* session_id) {
    return sendRequest(msgType, payload, session_id);
}

void HalRpcClient::startSomeip() {
    someip_ = std::make_shared<SomeipClient>(someip_app_name_, someip_context_);
    if (someip_ != nullptr) {
        someip_->start();
    }
}

HalRpcServer::HalRpcServer(const string& instanceName, bool syncApi)
    : HalRpc(instanceName, syncApi)  {
}

HalRpcServer::HalRpcServer(uint16_t instanceId, bool syncApi)
    : HalRpc(instanceId, syncApi)  {
}

HalRpcServer::~HalRpcServer() {
}

bool HalRpcServer::send(uint16_t msgType, const vector<uint8_t>& payload) {
    return sendEvent(msgType, payload);
}

void HalRpcServer::startSomeip() {
    someip_ = std::make_shared<SomeipServer>(someip_app_name_, someip_context_);
    setAvailable(true);
    if (someip_ != nullptr) {
        someip_->start();
    }
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <set>
#include <vector>

#include "event_util.h"
#include "hal_status.h"
#include "hal_util.h"
#include "message_sched.h"
#include "someip_client.h"
#include "someip_common_def.h"
#include "someip_server.h"

namespace qti {
namespace hal {
namespace rpc {

using std::map;
using std::mutex;
using std::set;
using std::shared_ptr;
using std::thread;
using std::vector;

using ::qti::hal::rpc::Someip;
using ::qti::hal::rpc::SomeipContext;
using ::qti::hal::rpc::SomeipMap;

class HalRpc {
  public:
    HalRpc(const string& instanceName, bool syncApi = true);
    HalRpc(uint16_t instanceId, bool syncApi = true);
    virtual ~HalRpc();

    void setProxy(shared_ptr<HalRpc> proxy) { proxy_ = proxy; }

    static void initService();
    static void deinitService();

    static void openMessageSched();
    static void closeMessageSched();

    static uint16_t map2InstanceId(const string& instanceName = HAL_DEFAULT_INSTANCE);
    static uint16_t map2InstanceId(SomeipInstanceId instanceId);
    static SomeipInstanceId map2SomeipInstanceId(uint16_t instanceId = HAL_DEFAULT_INSTANCE_ID);

    static void setAddHeaderFlag(bool addHeader) { sAddHeader = addHeader; }

    template<typename T>
    uint16_t getInstanceIdAvailable(const T& inst) {
        uint16_t instanceId = INVALID_HAL_INSTANCE_ID;
        set<uint16_t> values;
        for (auto& it: inst) {
            values.insert(it.second);
        }
        for (instanceId = 0; instanceId < HAL_MAX_INSTANCE_ID; instanceId++) {
            if (values.find(instanceId) == values.end())
                break;
        }
        return instanceId;
    }

    template<typename T, typename K>
    bool existInstance(T& inst, const uint16_t instanceId, K* intf) {
        for (auto& it: inst) {
            if (it.second == instanceId) {
                if (intf) {
                    *intf = it.first;
                }
                return true;
            }
        }
        return false;
    }

    template<typename T, typename K>
    bool existInstance(T& inst, const K& intf, uint16_t& instanceId) {
        for (auto& it: inst) {
            if ((it.first != nullptr) && (it.first->getInterface() == intf)) {
                instanceId = it.second;
                return true;
            }
        }
        return false;
    }

    template<typename T, typename K>
    void removeInstance(T& inst, const K& intf) {
        for (auto it = inst.begin(); it != inst.end();) {
            if ((it->first != nullptr) && (it->first->getInterface() == intf)) {
                inst.erase(it);
                break;
            }
            ++it;
        }
    }

    virtual void init();
    virtual void deinit();

    virtual bool openSomeip();
    virtual void closeSomeip();

    virtual bool sendRequest(uint16_t msgType, const vector<uint8_t>& payload, uint16_t* session_id = NULL);
    virtual bool sendResponse(const shared_ptr<vsomeip::message>& msg, const vector<uint8_t>& payload);
    virtual bool sendEvent(uint16_t msgType, const vector<uint8_t>& payload);

    virtual bool sendMessage(HalMsgT type, const shared_ptr<vsomeip::message>& msg, uint16_t msgType,
        const vector<uint8_t>& payload, bool addHeader, uint16_t* session_id = NULL);
    virtual bool sendMessage(HalMsgT type, const shared_ptr<vsomeip::message>& msg, uint16_t msgType,
        const vector<uint8_t>& payload, uint16_t* session_id = NULL);

    virtual size_t getMaxResponse() { return MAX_HAL_MSG; }

    virtual bool waitHalStatus(HalMsg msgType, uint16_t sessionId, void** halStatus);

  protected:
    virtual void initSomeipContext() = 0;
    virtual void startSomeip() = 0;

    virtual void initHalStatus() {};
    virtual void deinitHalStatus();

    virtual void addHalStatus(uint16_t message_type, uint16_t session_id, void* status);
    virtual void removeHalStatus(uint16_t message_type, uint16_t session_id);

    void* getHalStatus(uint16_t message_type, uint16_t session_id);

    virtual void handleHalCfm(uint16_t message_type, uint8_t* data, size_t length, void** halStatus) {}

    virtual void handleHalInd(uint16_t message_type, uint8_t* data, size_t length) {}

    virtual void handleHalReq(uint16_t message_type, uint8_t* data, size_t length, vector<uint8_t>& result) {}
    virtual void handleHalReq(uint16_t message_type, uint8_t* data, size_t length) {}

    void setAvailable(bool available);
    bool isAvailable();

    void* getHalStatus(HalMsg msgType);

  private:
    void addInstance(uint16_t service_id, uint16_t instance_id);
    void deleteInstance(uint16_t service_id, uint16_t instance_id);

    void someipThreadRoutine();

    void initEventHandle();
    void deinitEventHandle();

    void handleMsg(uint16_t message_type, uint8_t* data, size_t length,
        const shared_ptr<vsomeip::message>& msg, uint16_t session_id);

    void notifyResponse(HalMsg msgType, uint16_t sessionId, void* halStatus);

    static void someipAvailabilityHandler(uint16_t service_id, uint16_t instance_id, bool available);

    static void someipMessageHandler(const std::shared_ptr<SomeipMessage>& msg);

    static void messageSchedHandler(void* mv);

    static HalRpc* getInstance(uint16_t service_id, uint16_t instance_id);

    static HalMsgT map2HalMsgType(const shared_ptr<vsomeip::message>& msg);

    static void removeHalRpc(uint16_t /* service_id */, uint16_t /* instance_id */, void* halRpc);

  protected:
    uint16_t service_id_;  /* hal service id */
    uint16_t instance_id_;  /* hal instance id */
    bool sync_api_;   /* true: sync api, false: async api */
    shared_ptr<Someip> someip_;
    string someip_app_name_;    /* someip app name */
    SomeipContext someip_context_;  /* someip context */
    thread thread_;
    bool available_;    /* flag for someip availability */
    shared_ptr<SomeipMap> hal_status_;
    shared_ptr<HalRpc> proxy_;

  private:
    EventHandle* event_handle_;

    static SomeipMap sHalRpc;
    static bool sAddHeader;
};

class HalRpcClient : public HalRpc {
  public:
    HalRpcClient(const string& instanceName, bool syncApi = true);
    HalRpcClient(uint16_t instanceId, bool syncApi = true);
    virtual ~HalRpcClient();

    bool send(uint16_t msgType, const vector<uint8_t>& payload, uint16_t* session_id = NULL);

  private:
    void startSomeip() override;
};

class HalRpcServer : public HalRpc {
  public:
    HalRpcServer(const string& instanceName, bool syncApi = true);
    HalRpcServer(uint16_t instanceId, bool syncApi = true);
    virtual ~HalRpcServer();

    bool send(uint16_t msgType, const vector<uint8_t>& payload);

  private:
    void startSomeip() override;
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

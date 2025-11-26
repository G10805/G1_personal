/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <thread>

#include "someip_common_def.h"
#include "someip_util.h"

namespace qti {
namespace hal {
namespace rpc {

class Someip {
  public:
    Someip();
    Someip(const std::string& app_name);
    virtual ~Someip();

    static void setup(const SomeipCallback& callback);
    static void destroy();
    static const SomeipCallback& getCallback();

    void addService(const SomeipContext& _context);

    bool init();
    void deinit();

    virtual bool start(bool enable_thread = false) = 0;
    virtual void stop() = 0;

    bool sendMessage(std::shared_ptr<SomeipMessage> message);
    bool sendRequest(uint16_t method_id, const std::vector<uint8_t>& data, uint16_t* session_id = NULL, bool reliable = true);
    bool sendResponse(const std::shared_ptr<vsomeip::message>& msg, const std::vector<uint8_t>& data);
    bool sendEvent(uint16_t method_id, const std::vector<uint8_t>& data);

    /* TODO: Move to upper layer */
    std::shared_ptr<SomeipMessage> sendRequestAndWaitForReply(
        uint16_t method_id, const std::vector<uint8_t>& payload,
        uint32_t timeout = 500);
    std::shared_ptr<SomeipMessage> sendRequestAndWaitForReply(
        std::shared_ptr<SomeipMessage> request, uint32_t timeout = 500);

    void onMessage(const std::shared_ptr<vsomeip::message>& msg);

    inline bool validApp() {
        return (runtime_ != nullptr) && (app_ != nullptr);
    }

  private:
    static void initLogging();
    static void deinitLogging();
    static void initConfiguration();

  protected:
    std::shared_ptr<SomeipMessage> waitForResponse(uint16_t method_id,
        uint32_t timeout);
    void recvResponse(std::shared_ptr<SomeipMessage> response);

    static SomeipCallback sCallback;

    std::string app_name_;
    SomeipContext context_;
    std::shared_ptr<vsomeip::runtime> runtime_;
    std::shared_ptr<vsomeip::application> app_;

    std::shared_ptr<std::thread> someip_thread_;

    std::mutex mutex_;
    bool waiting_for_response_;
    std::condition_variable condition_;
    std::shared_ptr<SomeipMessage> response_;
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

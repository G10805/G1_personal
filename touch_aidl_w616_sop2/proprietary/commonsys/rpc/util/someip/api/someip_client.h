/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "someip.h"

namespace qti {
namespace hal {
namespace rpc {

class SomeipClient : public Someip {
  public:
    SomeipClient(const std::string& _app_name);
    SomeipClient(const std::string& _app_name, const SomeipContext& _context);
    ~SomeipClient();

    bool isServiceAvailable();

    bool start(bool enable_thread = false) override;
    void stop() override;

  protected:
    virtual vsomeip::message_handler_t getMessageHandler();

  private:
    void onState(vsomeip::state_type_e state);
    void onAvailability(vsomeip::service_t service_id,
        vsomeip::instance_t instance_id, bool available);

    void subscribeEvent();
    void unsubscribeEvent();

    enum class SomeipState {STOPPED, STARTED, UNAVAILABLE, AVAILABLE};
    SomeipState state_;
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

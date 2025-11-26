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

class SomeipServer : public Someip {
  public:
    SomeipServer(const std::string& _app_name);
    SomeipServer(const std::string& _app_name, const SomeipContext& _context);
    ~SomeipServer();

    bool start(bool enable_thread = false) override;
    void stop() override;

  protected:
    virtual vsomeip::message_handler_t getMessageHandler();

  private:
    void onState(vsomeip::state_type_e state);

    void offerEvent();
    void stopOfferEvent();
};

}  // namespace rpc
}  // namespace hal
}  // namespace qti

/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "log_common.h"
#include "someip_server.h"

namespace qti {
namespace hal {
namespace rpc {

SomeipServer::SomeipServer(const std::string& _app_name)
    : Someip(_app_name) {
    Someip::init();
}

SomeipServer::SomeipServer(const std::string& _app_name, const SomeipContext& _context)
    : SomeipServer(_app_name) {
    addService(_context);
}

SomeipServer::~SomeipServer() {
}

bool SomeipServer::start(bool enable_thread) {
    if (!app_)
        return false;

    app_->register_state_handler(
        std::bind(&SomeipServer::onState, this, std::placeholders::_1));

    app_->register_message_handler(
        context_.service_id, context_.instance_id, vsomeip::ANY_METHOD,
        getMessageHandler());

    offerEvent();

    /* Create thread to run someip opration */
    if (enable_thread) {
        someip_thread_ = std::make_shared<std::thread>([this] () { app_->start(); });
        if (!someip_thread_ || !someip_thread_->joinable()) {
            someip_thread_ = nullptr;
            return false;
        }
        return true;
    }

    /* Someip main loop in application start */
    app_->start();

    /* Someip main loop exited, call stop to clear state */
    stop();
    return true;
}

void SomeipServer::stop() {
    if (!app_)
        return;

    app_->clear_all_handler();

    stopOfferEvent();

    app_->stop_offer_service(context_.service_id, context_.instance_id);

    app_->stop();

    if (someip_thread_)
        someip_thread_->join();
}

vsomeip::message_handler_t SomeipServer::getMessageHandler() {
    return std::bind(&Someip::onMessage, this, std::placeholders::_1);
}

void SomeipServer::offerEvent() {
    for (const auto& events : context_.event_map) {
        vsomeip::eventgroup_t eventgroup_id = events.first;
        std::set<vsomeip::eventgroup_t> groups = {eventgroup_id};
        for (const auto& event : events.second)
            app_->offer_event(context_.service_id, context_.instance_id,
                event, groups);
    }
}

void SomeipServer::stopOfferEvent() {
    for (const auto& events : context_.event_map) {
        for (const auto& event : events.second)
            app_->stop_offer_event(context_.service_id,
                context_.instance_id, event);
    }
}

void SomeipServer::onState(vsomeip::state_type_e state) {
    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Application %s is %s"),
        app_name_.c_str(),
        state == vsomeip::state_type_e::ST_REGISTERED ?
            "registered" : "deregistered"));
    if (state == vsomeip::state_type_e::ST_REGISTERED)
        app_->offer_service(context_.service_id, context_.instance_id);
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

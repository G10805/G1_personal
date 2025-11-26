/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "log_common.h"
#include "someip_client.h"

namespace qti {
namespace hal {
namespace rpc {

SomeipClient::SomeipClient(const std::string& _app_name)
    : Someip(_app_name)
    , state_(SomeipState::STOPPED) {
    Someip::init();
}

SomeipClient::SomeipClient(const std::string& _app_name, const SomeipContext& _context)
    : SomeipClient(_app_name) {
    addService(_context);
}

SomeipClient::~SomeipClient() {
}

bool SomeipClient::isServiceAvailable() {
    return state_ == SomeipState::AVAILABLE;
}

bool SomeipClient::start(bool enable_thread) {
    if (!app_)
        return false;

    if (state_ != SomeipState::STOPPED)
        return true;

    app_->register_state_handler(
        std::bind(&SomeipClient::onState, this, std::placeholders::_1));

    app_->register_message_handler(
        context_.service_id, context_.instance_id, vsomeip::ANY_METHOD,
        getMessageHandler());

    app_->register_availability_handler(
        context_.service_id, context_.instance_id,
        std::bind(&SomeipClient::onAvailability, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));

    subscribeEvent();

    /* Create thread to run someip opration */
    if (enable_thread) {
        someip_thread_ = std::make_shared<std::thread>([this] () { app_->start(); });
        if (!someip_thread_ || !someip_thread_->joinable()) {
            someip_thread_ = nullptr;
            return false;
        }
        state_ = SomeipState::STARTED;
        return true;
    }

    /* Someip main loop in application start */
    state_ = SomeipState::STARTED;
    app_->start();

    /* Someip main loop exited, call stop to clear state */
    stop();
    return true;
}

void SomeipClient::stop() {
    if (!app_ || (state_ == SomeipState::STOPPED))
        return;

    app_->clear_all_handler();

    unsubscribeEvent();

    app_->release_service(context_.service_id, context_.instance_id);

    app_->stop();

    if (someip_thread_)
        someip_thread_->join();

    state_ = SomeipState::STOPPED;
}

vsomeip::message_handler_t SomeipClient::getMessageHandler() {
    return std::bind(&Someip::onMessage, this, std::placeholders::_1);
}

void SomeipClient::onState(vsomeip::state_type_e state) {
    IFDBG(DebugOut(DEBUG_OUTPUT, TEXT("Application %s is %s"),
        app_name_.c_str(),
        state == vsomeip::state_type_e::ST_REGISTERED ?
            "registered" : "deregistered"));

    if (state == vsomeip::state_type_e::ST_REGISTERED)
        app_->request_service(context_.service_id, context_.instance_id);
}

void SomeipClient::onAvailability(vsomeip::service_t service_id,
    vsomeip::instance_t instance_id, bool available) {
    IFDBG(DebugOut(DEBUG_INFO, TEXT("%s: service 0x%04x instance 0x%04x %s"),
        __func__, service_id, instance_id,
        available ? "available" : "unavailable"));

    bool prev_available = isServiceAvailable();
    state_ = available ? SomeipState::AVAILABLE : SomeipState::UNAVAILABLE;

    if (!Someip::sCallback.availability_handler)
        return;

    /**
     * Only need handle two case about state change:
     * 1. unavailable -> available
     * 2. available -> unavailable
     */
    if (prev_available ^ available)
        Someip::sCallback.availability_handler(service_id, instance_id, available);
}

void SomeipClient::subscribeEvent() {
    for (const auto& events : context_.event_map) {
        vsomeip::eventgroup_t eventgroup_id = events.first;
        std::set<vsomeip::eventgroup_t> groups = {eventgroup_id};
        for (const auto& event : events.second)
            app_->request_event(context_.service_id, context_.instance_id,
                event, groups);
        app_->subscribe(context_.service_id, context_.instance_id,
            eventgroup_id);
    }
}

void SomeipClient::unsubscribeEvent() {
    for (const auto& events : context_.event_map) {
        vsomeip::eventgroup_t eventgroup_id = events.first;
        app_->unsubscribe(context_.service_id, context_.instance_id,
            eventgroup_id);
        for (const auto& event : events.second)
            app_->release_event(context_.service_id, context_.instance_id,
                event);
    }
}

}  // namespace rpc
}  // namespace hal
}  // namespace qti

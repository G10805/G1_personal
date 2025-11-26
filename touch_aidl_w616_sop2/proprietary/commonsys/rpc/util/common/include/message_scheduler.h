/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#include <mutex>
#include <queue>

#include "message_queue.h"

typedef std::function<void()> MessageSchedulerHandler;

template<typename T, typename H>
class MessageScheduler {
  public:
    MessageScheduler()
        : scheduler_handler_(NULL)
        , message_handler_(NULL) {
    }
    ~MessageScheduler() {
        message_queue_.clear();
    }

    void registerSchedulerHandler(MessageSchedulerHandler handler) {
        scheduler_handler_ = handler;
    }

    void registerMessageHandler(H handler) {
        message_handler_ = handler;
    }

    void schedule(T message) {
        message_queue_.push(message);
        if (scheduler_handler_)
            scheduler_handler_();
    }

    void handle() {
        auto lock = message_queue_.acquireLock();
        while (!message_queue_.empty()) {
            auto message = message_queue_.front();
            message_queue_.pop();

            lock.unlock();

            if (message_handler_)
                message_handler_(message);

            message.reset();
            lock.lock();
        }
    }

  private:
    MessageQueue<T> message_queue_;

    MessageSchedulerHandler scheduler_handler_;
    H message_handler_;
};

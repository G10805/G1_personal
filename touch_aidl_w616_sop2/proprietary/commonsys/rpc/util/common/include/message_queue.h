/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#include <mutex>
#include <queue>

// Typename T can only be std::shared_ptr. E.g. std::shared_ptr<int>
template<typename T>
class MessageQueue {
  public:
    MessageQueue() {
    }

    ~MessageQueue() {
        clear();
    }

    std::unique_lock<std::recursive_mutex> acquireLock() {
        return std::unique_lock<std::recursive_mutex>{mutex_};
    }

    void push(T message) {
        acquireLock();
        queue_.push(message);
    }

    void pop() {
        acquireLock();
        queue_.pop();
    }

    T front() {
        acquireLock();
        return queue_.front();
    }

    bool empty() {
        acquireLock();
        return queue_.empty();
    }

    void clear() {
        acquireLock();
        while (!queue_.empty()) {
            auto message = front();
            pop();
            message.reset();
        }
    }

  private:
    std::recursive_mutex mutex_;
    std::queue<T> queue_;
};

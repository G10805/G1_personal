/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _SYNC_QUEUE_H_
#define _SYNC_QUEUE_H_

#include <memory>
#include <mutex>
#include <condition_variable>

namespace qc2audio {

template <typename T>
/**
 * @brief synchronized queue
 */
class QC2SyncQueue {
 public:
    /// queue an item
    QC2Status enqueue(T item) {
        bool wake = false;
        {
            std::lock_guard<std::mutex> lock(mLock);
            wake = mQ.empty();
            mQ.push_back(item);
        }
        if (wake) {
            mCond.notify_one();
        }
        return QC2_OK;
    }
    /// dequeue an item. Can block if the queue is empty
    QC2Status dequeue(T* item) {
        if (item == nullptr) {
            return QC2_ERROR;
        }
        std::unique_lock<std::mutex> lock(mLock);
        if (mQ.empty()) {
            mCond.wait(lock);
        }
        if (mQ.empty()) {   // must have been interrupted
            return QC2_TIMED_OUT;
        }
        *item = mQ.front();
        mQ.pop_front();
        return QC2_OK;
    }
    /// peek the front end of the queue. returns ERROR if queue is empty
    QC2Status peek(T* item) {
        std::lock_guard<std::mutex> lock(mLock);
        if (mQ.empty()) {
            return QC2_ERROR;
        }
        *item = mQ.front();
        return QC2_OK;
    }
    /// force-wake-up the blocked dequeue
    void interrupt() {
        mCond.notify_one();
    }
    /// empty the queue and return a copy of the items in queue.
    QC2Status flush(std::list<T> *flushed) {
        std::lock_guard<std::mutex> lock(mLock);
        if (flushed != nullptr) {
            *flushed = mQ;
        }
        mQ.clear();
        return QC2_OK;
    }
    /// number of items in queue
    uint32_t size() {
        std::lock_guard<std::mutex> lock(mLock);
        return mQ.size();
    }

 private:
    std::list<T> mQ;
    std::mutex mLock;
    std::condition_variable mCond;
};

}   // namespace qc2audio

#endif  // _SYNC_QUEUE_H_

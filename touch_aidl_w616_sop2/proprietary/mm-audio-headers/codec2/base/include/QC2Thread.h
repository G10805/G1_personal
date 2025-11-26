/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_UTILS_THREAD_H_
#define _QC2AUDIO_UTILS_THREAD_H_

#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "QC2.h"

namespace qc2audio {

/// @addtogroup utils Utilities
/// @{

/**
 * @brief Thread base that provides a thread context to the implementing class
 *
 * Class that needs a thread can subclass and implement threadLoop() method.
 * @note: If a class needs more than 1 thread, then it must implement Thread via another (inner)
 *   class and use composition
 */
class QC2Thread {
 public:
    /// init the thread object. Thread is not started here
    explicit QC2Thread(const std::string& name)
        : mThreadObj(nullptr),
          mName(name) {
        mThreadRunning.store(false);
    }

    virtual ~QC2Thread();

    /**
     * @brief start the thread
     *
     * @return QC2_OK thread was started successfully
     * @return QC2_BAD_STATE thread is already found running
     * @return QC2_ERROR there was a technical problem spawning the thread
     */
    QC2Status startThread();

    /**
     * check if the thread is running
     */
    inline bool isThreadRunning() {
        std::lock_guard<std::mutex> lock(mThreadLock);
        return mThreadRunning == true;
    }

    /**
     * request the thread to stop (asynchronously).
     */
    inline void requestStopThread() {
        std::lock_guard<std::mutex> lock(mThreadLock);
        mThreadStopRequested.store(true);
    }

    /**
     * @brief wait for the thread to terminate (imperative to have called requestStopThread)
     *
     * This will block until the threadLoop exits \n
     * NOTE: implementation must ensure that the necessary conditions to exit threadLoop()
     * have been signaled prior to joining.
     * @return QC2_OK thread stopped successfully
     * @return QC2_BAD_STATE thread not found running
     */
    QC2Status joinThread();

 protected:
    /**
     * Thread's context implemented by the subclass of Thread
     *
     * if shouldExitThread() evaluates to true, when checked, threadLoop must return immediately
     */
    virtual void threadLoop() = 0;

    /**
     * check if stop has been requested
     */
    inline bool shouldExitThread() {
        std::lock_guard<std::mutex> lock(mThreadLock);
        return mThreadStopRequested;
    }

 private:
    std::mutex mThreadLock;
    std::condition_variable mThreadStartEvent;
    std::shared_ptr<std::thread> mThreadObj;
    std::atomic_bool mThreadRunning;
    std::atomic_bool mThreadStopRequested;
    std::string mName;
};

/// @}

}   // namespace qc2audio

#endif  // _QC2AUDIO_UTILS_THREAD_H_

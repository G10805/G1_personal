/*
 *******************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#include "QC2Filter.h"
#include "QC2FilterCommon.h"
#include <numeric>
#include <chrono>
using namespace std::chrono_literals;

#undef LOG_TAG
#define LOG_TAG "QC2FilterCommon"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2 {
/*******************************************************************************
                         QC2Filter Helpers
*******************************************************************************/
QC2Status QC2Filter::startEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (!mHandler) {
        mHandler = std::make_shared<QC2Filter::Handler>(*this);
    }
    if (!mEventQueue) {
        mEventQueue = std::make_shared<EventQueue>("EVENT_Q_QC2_FILTER");
    }
    if (mHandler && mEventQueue) {
        mHandler->attach(mEventQueue);
        mEventQueue->start();
    } else {
        mHandler = nullptr;
        mEventQueue = nullptr;
        return QC2_NO_MEMORY;
    }
    return QC2_OK;
}

QC2Status QC2Filter::stopEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (mEventQueue) {
        mEventQueue->stop();
        mEventQueue = nullptr;
    }
    mHandler = nullptr;
    return QC2_OK;
}

/*******************************************************************************
                                 QC2FilterBufferList :
//DRUNWAL: This can be a common impl between V4L2Codec and VppFilter and QC2Filter
*******************************************************************************/
QC2FilterBufferList::QC2FilterBufferList(const std::string& name, int maxBufs)
    : mName(name),
      mFreeBufferList(maxBufs) {
    std::iota(mFreeBufferList.begin(), mFreeBufferList.end(), 0);
}

QC2Status QC2FilterBufferList::store(const std::shared_ptr<QC2Buffer>& buf, int& index) {
    std::lock_guard<std::mutex> listLock(mLock);
    QLOGV("BufferList:store: [%s] list-size=%zu free-size=%zu",
            mName.c_str(), mUseBufferList.size(), mFreeBufferList.size());
    index = -1;
    // If free buffer list is empty, means no free index is available to store the new buffer
    if (mFreeBufferList.empty()) {
        index = -1;
        QLOGE("BufferList:store: [%s] No free index available", mName.c_str());
        return QC2_ERROR;
    } else {
        index = mFreeBufferList.front();
        if (index >= 0) {
            mFreeBufferList.pop_front();
            // Also push the index and the buffer to the use buffer list
            mUseBufferList.push_back(std::make_pair(index, buf));
        }
    }
    return QC2_OK;
}

QC2Status QC2FilterBufferList::pop(int index, std::shared_ptr<QC2Buffer>& buf) {
    std::lock_guard<std::mutex> listLock(mLock);
    for (auto itr = mUseBufferList.begin(); itr != mUseBufferList.end(); ++itr) {
        if (std::get<0>(*itr) == index) {
            buf = std::get<1>(*itr);
            mUseBufferList.erase(itr);
            mFreeBufferList.push_back(index);
            QLOGV("BufferList:pop: [%s] list-size=%zu free-size=%zu",
                    mName.c_str(), mUseBufferList.size(), mFreeBufferList.size());
            if (mUseBufferList.empty()) {
                QLOGV("BufferList:pop: [%s] list is emptied..", mName.c_str());
                mEmptyCondition.notify_one();
            }
            return QC2_OK;
        }
    }
    buf = nullptr;
    if (!mUseBufferList.empty()) {
        QLOGE("BufferList::pop: [%s] could not find stashed buffer for id=%d !", mName.c_str(),
                index);
    }
    return QC2_ERROR;
}

size_t QC2FilterBufferList::listDepth() {
    std::lock_guard<std::mutex> listLock(mLock);
    return mUseBufferList.size();
}

QC2Status QC2FilterBufferList::waitUntilEmpty(uint32_t timeOutMs) {
    std::unique_lock<std::mutex> ul(mLock);
    while (!mUseBufferList.empty()) {
        if (mEmptyCondition.wait_for(ul, timeOutMs * 1ms) == std::cv_status::timeout) {
            QLOGE("BufferList::waitUntilEmpty: [%s] Timed out waiting for all bufs to be freed !",
                    mName.c_str());
            return QC2_TIMED_OUT;
        }
    }
    return QC2_OK;
}

}   // namespace qc2

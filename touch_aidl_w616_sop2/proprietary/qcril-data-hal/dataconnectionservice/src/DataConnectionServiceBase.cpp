/******************************************************************************
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "DataConnectionServiceBase.h"
#include "MessageCommon.h"
#include "RegisterBearerAllocationUpdateRequestMessage.h"

namespace vendor::qti::hardware::data::connection {

void DataConnectionServiceBase::acquireWakelock(TimeKeeper::millisec timeout) {
#ifndef QMI_RIL_UTF
    std::lock_guard<std::mutex> lock(wakelockMutex);
    // if already acquired, refresh the timer
    if (wakelockAcquired) {
        Log::getInstance().d("DataConnectionServiceBase::refreshing wakelock");
        bool timerCleared = TimeKeeper::getInstance().clear_timer(wakelockTimerId);
        if (!timerCleared) {
            Log::getInstance().d("Failed to clear wakelock timer");
        }
    } else {
        Log::getInstance().d("DataConnectionServiceBase::acquiring wakelock");
        if (acquire_wake_lock(PARTIAL_WAKE_LOCK, BEARER_ALLOCATION_WAKELOCK) < 0) {
            Log::getInstance().d("Failed to acquire wakelock");
        }
    }
    wakelockTimerId = TimeKeeper::getInstance().set_timer(
        std::bind(&DataConnectionServiceBase::releaseWakelockCb, this, std::placeholders::_1),
        nullptr,
        timeout);
#endif
}

void DataConnectionServiceBase::releaseWakelockCb(void *) {
#ifndef QMI_RIL_UTF
    Log::getInstance().d("DataConnectionServiceBase::release wakelock");
    std::lock_guard<std::mutex> lock(wakelockMutex);
    if (wakelockAcquired)
    {
        if (release_wake_lock(BEARER_ALLOCATION_WAKELOCK) < 0)
        {
            Log::getInstance().d("DataConnectionServiceBase::wakelock not acquired");
        }
        wakelockAcquired = false;
    }
#endif
}

/**
 * ClientDeathRecipient::serviceDied()
 *
 * @brief
 * Removes client from the callback maps once the client dies. Notify DataModule when there are
 * no remaining clients registered for indications.
 */
void DataConnectionServiceBase::serviceDied(uint64_t cookie,
        const android::wp<android::hidl::base::V1_0::IBase>& who) {
    Log::getInstance().d("DataConnectionServiceBase::client died -cookie" + std::to_string(cookie));
    clearResponseFunctions(cookie);
    (void)who;
}

}

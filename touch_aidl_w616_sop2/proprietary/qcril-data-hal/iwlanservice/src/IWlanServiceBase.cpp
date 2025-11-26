/******************************************************************************
#  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "IWLAN"
#include "IWlanServiceBase.h"

#define IWLAN_WAKE_LOCK_NSECS 200000000

using vendor::qti::hardware::data::iwlan::IWlanServiceBase;

std::mutex IWlanServiceBase::wakelockMutex;
uint8_t IWlanServiceBase::wakelockRefCount = 0;
timer_t IWlanServiceBase::wakelockTimerId = NULL;
std::string IWlanServiceBase::iwlanSvcWakelock = "";

void IWlanServiceBase::serviceDied(uint64_t cookie,
        const android::wp<android::hidl::base::V1_0::IBase>& who) {
    QCRIL_LOG_DEBUG("IWlan Service Cb died. cookie: %llu, who: %p",
                static_cast<unsigned long long>(cookie), &who);
    clearResponseFunctions();
}

void IWlanServiceBase::acquireWakeLock() {
#ifndef QMI_RIL_UTF
    wakelockMutex.lock();
    if (wakelockRefCount == 0) {
        if (acquire_wake_lock(PARTIAL_WAKE_LOCK, iwlanSvcWakelock.c_str()) < 0)
        {
            QCRIL_LOG_ERROR("%s: failed to acquire wake lock [%d:%s]",
                             __func__, errno, strerror(errno));
            wakelockMutex.unlock();
            return;

        }
    }
    //Create a new timer if required

    struct sigevent sigev;
    struct itimerspec itimers;

    memset(&sigev, 0, sizeof(struct sigevent));
    memset(&itimers, 0,sizeof(struct itimerspec));

    sigev.sigev_notify            = SIGEV_THREAD;
    sigev.sigev_notify_attributes = NULL;
    sigev.sigev_value.sival_ptr   = NULL;
    sigev.sigev_notify_function   = wakeLockTimeoutHdlr;
    if (wakelockTimerId == NULL) {
        if (-1 == timer_create(CLOCK_MONOTONIC, &sigev, &wakelockTimerId) )
        {
            QCRIL_LOG_ERROR( "failed to create wakelock timer ");
            wakelockMutex.unlock();
            return;
        }
        else
        {
            QCRIL_LOG_DEBUG( "wakelock timer creation success:");
        }
    }

    itimers.it_value.tv_sec     = 0;
    itimers.it_value.tv_nsec    = IWLAN_WAKE_LOCK_NSECS;
    itimers.it_interval.tv_sec  = 0;
    itimers.it_interval.tv_nsec = 0;

    // Start the timer, value gets overwritten if timer had already been started
    if (-1 == timer_settime(wakelockTimerId, 0, &itimers, NULL))
    {
        QCRIL_LOG_ERROR( "failed to start timer for timer_id , deleting... ");
        timer_delete(wakelockTimerId);
        wakelockTimerId = NULL;
        wakelockMutex.unlock();
        return;
    }
    else
    {
        wakelockRefCount++;
        wakelockMutex.unlock();
    }
#endif
}

void IWlanServiceBase::releaseWakeLock() {
#ifndef QMI_RIL_UTF
    wakelockMutex.lock();
    if (wakelockRefCount > 0) {
        wakelockRefCount--;
        if (wakelockRefCount == 0) {
            QCRIL_LOG_DEBUG( "ref count is 0, releasing wakelock");
            timer_delete(wakelockTimerId);
            wakelockTimerId = NULL;
            if (release_wake_lock(iwlanSvcWakelock.c_str()) < 0)
            {
                QCRIL_LOG_ERROR("%s: release wakelock %s failed. ",__func__,iwlanSvcWakelock.c_str() );
            }
        }
    }
    wakelockMutex.unlock();
#endif
}

void IWlanServiceBase::wakeLockTimeoutHdlr(union sigval sval) {
    std::ignore = sval;
    QCRIL_LOG_DEBUG( "wakelock timeout called, releasing wakelock");
    IWlanServiceBase::resetWakeLock();

}

void IWlanServiceBase::resetWakeLock() {
    wakelockMutex.lock();
    if (wakelockRefCount > 0) {
        wakelockRefCount = 0;
        timer_delete(wakelockTimerId);
        wakelockTimerId = NULL;
        if (release_wake_lock(iwlanSvcWakelock.c_str()) < 0)
        {
            QCRIL_LOG_ERROR("%s: release wakelock %s failed. ",__func__,iwlanSvcWakelock.c_str() );
        }
    }
    wakelockMutex.unlock();
}

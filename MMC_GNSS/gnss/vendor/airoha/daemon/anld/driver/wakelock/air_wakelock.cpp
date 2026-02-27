#define LOG_TAG "WAKELOCK"
#include "air_wakelock.h"
#include <assert.h>
#ifndef AIROHA_SIMULATION
#include <hardware_legacy/power.h>
#endif
#include "pthread.h"
#include "simulation.h"
using airoha::WakeLock;
using airoha::WakeSource;
const char *kAirohaNavigationLockName = "ANLD_WAKELOCK";
WakeLock *WakeLock::sInstance = new WakeLock();
WakeLock::WakeLock() {
    pthread_mutex_init(&mMutex, nullptr);
    mIsLock = false;
    return;
}
WakeLock *WakeLock::getInstnace() { return sInstance; }
bool WakeLock::acquireLock(WakeLockUser user) {
    if (user >= WAKELOCK_USER_MAX) {
        LOG_E("lock user is invalid: %u", user);
        return false;
    }
    updateWakeLock(user, 1);
    return true;
}
bool WakeLock::releaseLock(WakeLockUser user) {
    if (user >= WAKELOCK_USER_MAX) {
        LOG_E("lock user is invalid: %u", user);
        return false;
    }
    updateWakeLock(user, -1);
    return true;
}
void WakeLock::updateWakeLock(WakeLockUser user, int8_t counter) {
    pthread_mutex_lock(&mMutex);
    LOG_I("Update Wakelock: user=%d[%d][%s], counter=%d", user, mCount[user],
          getWakeUserStr(user), counter);
    TRACE_D("Update Wakelock: user=%d[%d], counter=%d", user, mCount[user],
            counter);
    EVENT_I("wakelock[user=%s] counter=%d", getWakeUserStr(user), counter);
    mCount[user] += counter;
    if (mCount[user] < 0) {
        LOG_W("user %u request counter less than 0.", user);
        mCount[user] = 0;
    }
    if (mCount[user] > kCounterMax) {
        LOG_E("user %u counter > %d", user, kCounterMax);
        assert(0 && "unexpected count exceed");
    }
    bool toLock = false;
    for (size_t i = 0; i < WAKELOCK_USER_MAX; i++) {
        if (mCount[i] > 0) {
            toLock = true;
        }
    }
    if (toLock) {
        if (!mIsLock) {
            LOG_I("acquire_wake_lock");
            TRACE_D("acquire_wake_lock");
            EVENT_I("acquire_wake_lock");
#ifndef AIROHA_SIMULATION
            if (acquire_wake_lock(PARTIAL_WAKE_LOCK,
                                  kAirohaNavigationLockName) != 0) {
                LOG_E("acquire lock error!");
            }
#endif
            mIsLock = true;
        }
    } else {
        if (mIsLock) {
            LOG_I("release_wake_lock");
            TRACE_D("release_wake_lock");
            EVENT_I("release_wake_lock");
#ifndef AIROHA_SIMULATION
            if (release_wake_lock(kAirohaNavigationLockName) != 0) {
                LOG_E("release lock error!");
            }
#endif
            mIsLock = false;
        }
    }
    pthread_mutex_unlock(&mMutex);
}
bool WakeLock::commonAcquireWakeLock() {
    return sInstance->acquireLock(WAKELOCK_USER_COMMON);
}
bool WakeLock::commonReleaseWakeLock() {
    return sInstance->releaseLock(WAKELOCK_USER_COMMON);
}
void WakeLock::addWakeSource(WakeSource *wakeSource) {
    std::lock_guard<std::mutex> locker(mWakeSourceMutex);
    size_t preSize = mWakeSourceMap.size();
    mWakeSourceMap[wakeSource] = 1;
    if (mWakeSourceMap.size() > 0 && preSize == 0) {
        acquireLock(WakeLockUser::WAKELOCK_USER_SHARED_PTR);
    }
    LOG_D("wake_source_num: %zu", mWakeSourceMap.size());
}
void WakeLock::removeWakeSource(WakeSource *wakeSource) {
    std::lock_guard<std::mutex> locker(mWakeSourceMutex);
    size_t preSize = mWakeSourceMap.size();
    mWakeSourceMap.erase(wakeSource);
    if (mWakeSourceMap.size() == 0 && preSize > 0) {
        releaseLock(WakeLockUser::WAKELOCK_USER_SHARED_PTR);
    }
    LOG_D("wake_source_num: %zu", mWakeSourceMap.size());
}
#define WENUM(x) \
    case x:      \
        return #x;
const char *WakeLock::getWakeUserStr(WakeLockUser user) {
    switch (user) {
        WENUM(WAELOCK_USER_SYSTEM);
        WENUM(WAKELOCK_USER_COMMON);
        WENUM(WAKELOCK_USER_TESTCASE);
        WENUM(WAKELOCK_USER_DOWNLOAD_FIRMWARE);
        WENUM(WAKELOCK_USER_GPS_COMMUNICATION);
        WENUM(WAKELOCK_USER_GPS_NVRAM_SAVE);
        WENUM(WAKELOCK_USER_GPS_POWER_OFF_SEQUENCE);
        WENUM(WAKELOCK_USER_ANLD_SERVICE);
        WENUM(WAKELOCK_USER_GPS_OPEN_DSP_SEQUENCE);
        WENUM(WAKELOCK_USER_MAX);
        WENUM(WAKELOCK_USER_MINILOADER);
        WENUM(WAKELOCK_USER_SHARED_PTR);
    }
}
WakeSource::WakeSource(const std::string &name) : mName(name) {
    WakeLock::getInstnace()->addWakeSource(this);
}
WakeSource::~WakeSource() { WakeLock::getInstnace()->removeWakeSource(this); }
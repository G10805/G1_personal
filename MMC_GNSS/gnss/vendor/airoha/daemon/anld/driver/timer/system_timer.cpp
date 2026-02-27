/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#include "system_timer.h"
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include "simulation.h"
using airoha::SysTimer;
SysTimer::SysTimer() {
    bool ret = initTimer(0, nullptr, false);
    assert(ret);
    is_static_create_ = 0;
    is_static_remove_ = 0;
}
SysTimer::SysTimer(uint32_t duration_ms, void *userdata, bool suspendAware) {
    bool ret = initTimer(duration_ms, userdata, suspendAware);
    assert(ret);
    is_static_create_ = 0;
    is_static_remove_ = 0;
}
SysTimer::SysTimer(uint32_t duration_ms, void *userdata, bool suspendAware,
                    uint32_t reload_ms) {
    bool ret = initTimer(duration_ms, userdata, suspendAware);
    assert(ret);
    mReloadMs = reload_ms;
    is_static_create_ = 0;
    is_static_remove_ = 0;
}
SysTimer::~SysTimer() {
    if (is_static_remove_ ^ is_static_create_) {
        // LOG_E("a static create timer must be delete by static remove");
        assert(0 && "a static create timer must be delete by static remove");
    }
    timer_delete(mId);
}
bool SysTimer::initTimer(uint32_t duration_ms, void *userdata, bool suspendAware) {
    mId = 0;
    mDurationMs = duration_ms;
    mReloadMs = 0;
    mUserdata = userdata;
    sigevent sigev;
    memset(&sigev, 0, sizeof(sigevent));
    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_notify_function = commonTimerCallback;
    sigev.sigev_value.sival_ptr = this;
    int ret = timer_create(suspendAware ? CLOCK_BOOTTIME : CLOCK_MONOTONIC, &sigev, &mId);
    if (ret != 0) {
        LOG_E("set time value failed! %d", errno);
        return false;
    }
    LOG_D("init timer %p", mId);
    return true;
}
bool SysTimer::start() { return settimerValue(mDurationMs, mReloadMs); }
bool SysTimer::stop() { return settimerValue(0, 0); }
bool SysTimer::restart() { return settimerValue(mDurationMs, mReloadMs); }
void SysTimer::setDuration(uint32_t duration_ms) { mDurationMs = duration_ms; }
void SysTimer::setUserData(void *userdata) { mUserdata = userdata; }
bool SysTimer::settimerValue(uint32_t set_duration_ms, uint32_t reload_ms) {
    // LOG_D("set timer %u", set_duration_ms);
    struct itimerspec to_set_value;
    struct timespec expire_time;
    struct timespec interval_time;
    memset(&to_set_value, 0, sizeof(to_set_value));
    memset(&expire_time, 0, sizeof(expire_time));
    memset(&interval_time, 0, sizeof(interval_time));
    expire_time.tv_sec = set_duration_ms / 1000;
    expire_time.tv_nsec = (set_duration_ms % 1000) * 1000000;
    interval_time.tv_sec = reload_ms / 1000;
    interval_time.tv_nsec = (reload_ms % 1000) * 1000000;
    to_set_value.it_value = expire_time;
    to_set_value.it_interval = interval_time;
    int ret = timer_settime(mId, 0, &to_set_value, nullptr);
    if (ret != 0) {
        LOG_E("set time value failed! %d", errno);
        return false;
    }
    return true;
}
int64_t SysTimer::getRemainTime() {
    struct itimerspec expireValue;
    memset(&expireValue, 0, sizeof(expireValue));
    /**
     * timer_gettime() returns the time until next expiration, and
     * the interval, for the timer specified by timerid, in the buffer pointed
     * to by curr_value.  The time remaining until the next timer expiration is
     * returned in curr_value->it_value; this is always a relative value,
     * regardless of whether the TIMER_ABSTIME flag was used when arming the
     * timer.  If the value returned in curr_value->it_value is zero, then the
     * timer is currently disarmed.  The timer interval is returned in
     * curr_value->it_interval.  If the value returned in
     * curr_value->it_interval is zero, then this is a "one-shot" timer.
     *
     */
    int ret = timer_gettime(mId, &expireValue);
    if (ret != 0) {
        LOG_E("get time value failed! %d", errno);
        return -1;
    }
    int64_t value = 0;
    value = (int64_t)expireValue.it_value.tv_sec * 1000000000LL +
            (int64_t)expireValue.it_value.tv_nsec;
    return value;
}
bool SysTimer::getNextExpireMs(uint32_t *duration_ms) {
    struct itimerspec expireValue; 
    memset(&expireValue, 0, sizeof(expireValue));
    /**
     * timer_gettime() returns the time until next expiration, and
     * the interval, for the timer specified by timerid, in the buffer pointed to
     * by curr_value.  The time remaining until the next timer expiration is
     * returned in curr_value->it_value; this is always a relative value,
     * regardless of whether the TIMER_ABSTIME flag was used when arming the
     * timer.  If the value returned in curr_value->it_value is zero, then the
     * timer is currently disarmed.  The timer interval is returned in
     * curr_value->it_interval.  If the value returned in
     * curr_value->it_interval is zero, then this is a "one-shot" timer.
     *
     */
    int ret = timer_gettime(mId, &expireValue);
    if (ret != 0) {
        LOG_E("get time value failed! %d", errno);
        return false;
    }
    uint32_t value = 0;
    value = expireValue.it_value.tv_sec * 1000 +
            expireValue.it_value.tv_nsec / 1000000;
    if (value & 0x800000000) {
        LOG_E("get time value but value too big %u", value);
        // value too big
        return false;
    }
    *duration_ms = value;
    return true;
}
bool SysTimer::startIfNotRunning() {
    int64_t remainTime = getRemainTime();
    if (remainTime == 0) {
        start();
    } else if (remainTime == -1) {
        return false;
    }
    return true;
}
void SysTimer::commonTimerCallback(sigval value) {
    SysTimer *t = static_cast<SysTimer *>(value.sival_ptr);
    t->onExpire(t->mUserdata);
}
SysTimer *SysTimer::createTimer(timerCb cb, uint32_t duration_ms,
                                void *userdata, bool suspendAware) {
    SysTimer *tmp = new SysTimer(duration_ms, userdata, suspendAware);
    tmp->is_static_create_ = 1;
    tmp->user_cb_function_ = cb;
    return tmp;
}
SysTimer *SysTimer::createTimer(timerCb cb, uint32_t duration_ms, void *userdata,
                                 bool suspendAware, uint32_t reload_ms) {
    SysTimer *tmp = new SysTimer(duration_ms, userdata, suspendAware, reload_ms);
    tmp->is_static_create_ = 1;
    tmp->user_cb_function_ = cb;
    return tmp;
}
void SysTimer::removeTimer(SysTimer *timer) {
    timer->is_static_remove_ = 1;
    delete (timer);
}
void SysTimer::onExpire(void *userdata) {
    if (user_cb_function_) {
        user_cb_function_(this, userdata);
    }
}
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
#ifndef DRIVER_TIMER_SYSTEM_TIMER_H
#define DRIVER_TIMER_SYSTEM_TIMER_H
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <vector>
namespace airoha {
class SysTimer {
 public:
    typedef void (*timerCb)(SysTimer *timer_ptr, void *vp);
    SysTimer();
    SysTimer(uint32_t duration_ms, void *userdata, bool suspendAware = false);
    SysTimer(uint32_t duration_ms, void *userdata, bool suspendAware, uint32_t reload_ms);
    // do not allow copy
    SysTimer(const SysTimer &) = delete;
    virtual ~SysTimer();
    bool start();
    bool startIfNotRunning();
    bool stop();
    bool restart();
    void setDuration(uint32_t duration_ms);
    bool getNextExpireMs(uint32_t *duration_ms);
    int64_t getRemainTime();
    void setUserData(void *userdata);
    static SysTimer *createTimer(timerCb, uint32_t duration_ms, void *userdata, bool suspendAware);
    static SysTimer *createTimer(timerCb, uint32_t duration_ms, void *userdata,
                                 bool suspendAware, uint32_t reload_ms);
    // a static create timer must be remove by static remove
    static void removeTimer(SysTimer *);

 protected:
    virtual void onExpire(void *userdata);

 private:
    bool initTimer(uint32_t duration_ms, void *userdata, bool suspendAware);
    uint32_t mDurationMs;
    uint32_t mReloadMs;
    void *mUserdata;
    static std::vector<SysTimer *> exist_timer_list_;
    bool settimerValue(uint32_t duration_ms, uint32_t reload_ms);
    static void commonTimerCallback(sigval value);
    timer_t mId;
    timerCb user_cb_function_;
    uint8_t is_static_create_;
    uint8_t is_static_remove_;
};
};  // namespace airoha
#endif
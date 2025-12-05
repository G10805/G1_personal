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
#pragma once
#include <pthread.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
namespace airoha {
class WakeSource;
class WakeLock {
 public:
    enum WakeLockUser : uint8_t {
        WAELOCK_USER_SYSTEM,
        WAKELOCK_USER_COMMON,
        WAKELOCK_USER_TESTCASE,
        WAKELOCK_USER_DOWNLOAD_FIRMWARE,
        WAKELOCK_USER_GPS_COMMUNICATION,
        WAKELOCK_USER_GPS_NVRAM_SAVE,
        WAKELOCK_USER_GPS_POWER_OFF_SEQUENCE,
        WAKELOCK_USER_GPS_OPEN_DSP_SEQUENCE,
        WAKELOCK_USER_ANLD_SERVICE,
        WAKELOCK_USER_MINILOADER,
        WAKELOCK_USER_SHARED_PTR,
        WAKELOCK_USER_MAX
    };
    static WakeLock *getInstnace();
    bool acquireLock(WakeLockUser);
    bool releaseLock(WakeLockUser);
    static bool commonAcquireWakeLock();
    static bool commonReleaseWakeLock();
    static const char *getWakeUserStr(WakeLockUser user);
    void addWakeSource(WakeSource *wakesouce);
    void removeWakeSource(WakeSource *wakesource);

 protected:
    WakeLock();

 private:
    void updateWakeLock(WakeLockUser, int8_t counter);
    int8_t mCount[WAKELOCK_USER_MAX] = {0};
    bool mIsLock;
    pthread_mutex_t mMutex;
    std::mutex mWakeSourceMutex;
    static WakeLock *sInstance;
    static const int8_t kCounterMax = 10;
    std::map<WakeSource *, int> mWakeSourceMap;
};
class WakeSource {
 public:
    WakeSource(const std::string &name);
    ~WakeSource();

 private:
    std::string mName;
};
}  // namespace airoha
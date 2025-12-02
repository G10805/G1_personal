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
/*
 * native public static long uptimeMillis();
 */
#include "native_time.h"
#include <assert.h>
#include <string.h>
#include <time.h>
#include "simulation.h"
namespace airoha {
namespace native {
enum {
    NATIVE_TIME_REALTIME = 0,  // system-wide realtime clock
    // monotonic time since unspecified starting point
    NATIVE_TIME_MONOTONIC = 1,
    NATIVE_TIME_PROCESS = 2,  // high-resolution per-process clock
    NATIVE_TIME_THREAD = 3,   // high-resolution per-thread clock
    // same as SYSTEM_TIME_MONOTONIC, but including CPU suspend time
    NATIVE_TIME_BOOTTIME = 4,
};
static constexpr size_t clock_id_max = 5;
static void checkClockId(int clock) {
    (void)clock;
    assert(clock >= 0 && clock <= clock_id_max);
}
#if defined(__linux__)
nsecs_t systemTime(int clock) {
    checkClockId(clock);
    (void)clock_id_max;
    static constexpr clockid_t clocks[] = {
        CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID,
        CLOCK_THREAD_CPUTIME_ID, CLOCK_BOOTTIME};
    assert(clock_id_max == (sizeof(clocks) / sizeof(clockid_t)));
    timespec t = {};
    clock_gettime(clocks[clock], &t);
    return nsecs_t(t.tv_sec) * 1000000000LL + t.tv_nsec;
}
#else
#error "Why code cames here?"
nsecs_t systemTime(int clock) {
    // TODO: is this ever called with anything but REALTIME on mac/windows?
    checkClockId(clock);
    // Clock support varies widely across hosts. Mac OS doesn't support
    // CLOCK_BOOTTIME (and doesn't even have clock_gettime until 10.12).
    // Windows is windows.
    timeval t = {};
    gettimeofday(&t, nullptr);
    return nsecs_t(t.tv_sec) * 1000000000LL + nsecs_t(t.tv_usec) * 1000LL;
}
#endif
int64_t uptimeMillis() { return nanoseconds_to_milliseconds(uptimeNanos()); }
/*
 * public static native long uptimeNanos();
 */
int64_t uptimeNanos() { return systemTime(NATIVE_TIME_MONOTONIC); }
/*
 * native public static long elapsedRealtime();
 */
int64_t elapsedRealtime() {
    return nanoseconds_to_milliseconds(elapsedRealtimeNano());
}
/*
 * native public static long elapsedRealtimeNano();
 */
int64_t elapsedRealtimeNano() {
#if defined(__linux__)
    struct timespec ts;
    int err = clock_gettime(CLOCK_BOOTTIME, &ts);
    if (err == -1) {
        // This should never happen, but just in case ...
        LOG_E("clock_gettime(CLOCK_BOOTTIME) failed: %s", strerror(errno));
        return 0;
    }
    return seconds_to_nanoseconds(ts.tv_sec) + ts.tv_nsec;
#else
#error "elapsedRealtimeNano: Why code cames here?"
    return systemTime(SYSTEM_TIME_MONOTONIC);
#endif
}
int64_t nativeRealtime() {
#if defined(__linux__)
    struct timespec ts;
    int err = clock_gettime(CLOCK_REALTIME, &ts);
    if (err == -1) {
        // This should never happen, but just in case ...
        LOG_E("clock_gettime(CLOCK_REALTIME) failed: %s", strerror(errno));
        return 0;
    }
    return seconds_to_milliseconds(ts.tv_sec) +
           nanoseconds_to_milliseconds(ts.tv_nsec);
#else
#error "nativeRealtime: Why code cames here?"
    return systemTime(SYSTEM_TIME_MONOTONIC);
#endif
}
}  // namespace native
}  // namespace airoha
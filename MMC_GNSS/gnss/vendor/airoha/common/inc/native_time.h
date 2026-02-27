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
#include <stdint.h>
namespace airoha {
namespace native {
#if __cplusplus >= 201103L
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif
typedef int64_t nsecs_t;  // nano-seconds
static CONSTEXPR inline nsecs_t seconds_to_nanoseconds(nsecs_t secs) {
    return secs * 1000000000;
}
static CONSTEXPR inline nsecs_t seconds_to_milliseconds(nsecs_t secs) {
    return secs * 1000;
}
static CONSTEXPR inline nsecs_t milliseconds_to_nanoseconds(nsecs_t secs) {
    return secs * 1000000;
}
static CONSTEXPR inline nsecs_t microseconds_to_nanoseconds(nsecs_t secs) {
    return secs * 1000;
}
static CONSTEXPR inline nsecs_t nanoseconds_to_seconds(nsecs_t secs) {
    return secs / 1000000000;
}
static CONSTEXPR inline nsecs_t nanoseconds_to_milliseconds(nsecs_t secs) {
    return secs / 1000000;
}
static CONSTEXPR inline nsecs_t nanoseconds_to_microseconds(nsecs_t secs) {
    return secs / 1000;
}
static CONSTEXPR inline nsecs_t s2ns(nsecs_t v) {
    return seconds_to_nanoseconds(v);
}
static CONSTEXPR inline nsecs_t ms2ns(nsecs_t v) {
    return milliseconds_to_nanoseconds(v);
}
static CONSTEXPR inline nsecs_t us2ns(nsecs_t v) {
    return microseconds_to_nanoseconds(v);
}
static CONSTEXPR inline nsecs_t ns2s(nsecs_t v) {
    return nanoseconds_to_seconds(v);
}
static CONSTEXPR inline nsecs_t ns2ms(nsecs_t v) {
    return nanoseconds_to_milliseconds(v);
}
static CONSTEXPR inline nsecs_t ns2us(nsecs_t v) {
    return nanoseconds_to_microseconds(v);
}
static CONSTEXPR inline nsecs_t seconds(nsecs_t v) { return s2ns(v); }
static CONSTEXPR inline nsecs_t milliseconds(nsecs_t v) { return ms2ns(v); }
static CONSTEXPR inline nsecs_t microseconds(nsecs_t v) { return us2ns(v); }
// Returns milliseconds since boot, not counting time spent in deep sleep.
int64_t uptimeMillis();
// Returns nanoseconds since boot, not counting time spent in deep sleep.
int64_t uptimeNanos();
// Returns milliseconds since boot, including time spent in sleep.
int64_t elapsedRealtime();
// Returns nanoseconds since boot, including time spent in sleep.
int64_t elapsedRealtimeNano();
// Returns milliseconds with CLOCK_REALTIME(current system time).
int64_t nativeRealtime();
}  // namespace native
}  // namespace airoha

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
#include "timer_util.h"
#include <inttypes.h>
#include <memory.h>
#include <signal.h>
#include <stdlib.h>
#include "simulation.h"
namespace airoha {
namespace timer {
timer_t create_timer(timer_callback_func_t func, void *userdata) {
    timer_t id;
    struct sigevent sigevt;
    memset(&sigevt, 0, sizeof(sigevt));
    sigevt.sigev_notify = SIGEV_THREAD;
    sigevt.sigev_value.sival_ptr = userdata;
    sigevt.sigev_notify_function = func;
    if (timer_create(CLOCK_MONOTONIC, &sigevt, &id) == -1) {
        LOG_E("create timer error %d, str=%s", errno, strerror(errno));
        return (timer_t) nullptr;
    }
    return id;
}
int destroy_timer(timer_t id) {
    if (timer_delete(id) == -1) {
        return -1;
    }
    return 0;
}
int start_timer(timer_t id, int64_t millSeconds) {
    struct itimerspec ts;
    ts.it_interval.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_value.tv_sec = millSeconds / 1000;
    ts.it_value.tv_nsec = (millSeconds % 1000) * 1000000;
    if (timer_settime(id, 0, &ts, nullptr) == -1) {
        LOG_D("start timer error!! %p, %" PRId64 " err=%d, %s", id, millSeconds,
              errno, strerror(errno));
        return -1;
    }
    return 0;
}
int stop_timer(timer_t id) { return start_timer(id, 0); }
int64_t get_timer_remain_ms(timer_t id) {
    struct itimerspec ts;
    timer_gettime(id, &ts);
    int64_t ms = 0;
    ms = (int64_t)ts.it_value.tv_sec * 1000 +
         (int64_t)ts.it_value.tv_nsec / 1000000;
    return ms;
}
bool is_timer_alive(timer_t id) {
    struct itimerspec ts;
    timer_gettime(id, &ts);
    if (ts.it_value.tv_nsec == 0 && ts.it_value.tv_sec == 0) {
        return false;
    }
    return true;
}
}  // namespace timer
}  // namespace airoha
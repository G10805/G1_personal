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
#define LOG_TAG "NATIVE"
#include "native_call.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "simulation.h"
ssize_t native_safe_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || buf == nullptr || count == 0) {
        LOG_E("native_safe_write error: fd=%d, buf=%p, count=%zu", fd, buf,
              count);
        return -1;
    }
    int retry_count = 10;
    size_t written = 0;
    uint8_t *p = (uint8_t *)buf;
    while (written < count) {
        ssize_t num = write(fd, p + written, count - written);
        if (num == -1) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN && retry_count > 0) {
                retry_count--;
                usleep(100 * 1000);
                continue;
            }
            LOG_E(
                "native_safe_write error: buf=%p,count=%zu,error=[%s],retry=%d",
                buf, count, strerror(errno), retry_count);
            return -1;
        }
        written += num;
    }
    return written;
}
ssize_t native_safe_read(int fd, void *buf, size_t count) {
    if (fd < 0 || buf == nullptr || count == 0) {
        LOG_E("native_safe_write error: fd=%d, buf=%p, count=%zu", fd, buf,
              count);
        return -1;
    }
    int retry_count = 10;
    ssize_t num = 0;
    uint8_t *p = (uint8_t *)buf;
    while ((num = read(fd, p, count)) < 0) {
        if (errno == EINTR) continue;
        if (errno == EAGAIN && retry_count > 0) {
            retry_count--;
            usleep(100 * 1000);
            continue;
        }
        LOG_E("native_safe_read error: buf=%p,count=%zu,error=[%s],retry=%d",
              buf, count, strerror(errno), retry_count);
        return -1;
    }
    return num;
}

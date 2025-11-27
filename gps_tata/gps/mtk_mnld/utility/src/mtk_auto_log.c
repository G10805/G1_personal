/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#include "mtk_auto_log.h"

#if defined(__LINUX_OS__)
// -1 means failure
int get_time_str(char* buf, int len) {
    struct timeval  tv;
    struct timezone tz;
    struct tm      *tm;

    gettimeofday(&tv, &tz);
    tm = localtime(&tv.tv_sec);

    memset(buf, 0, len);
    sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
        tm->tm_year + 1900, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec, (int)(tv.tv_usec / 1000));

    return 0;
}
#endif

int set_log_level(int *dst_level, int src_level)
{
#if defined(__ANDROID_OS__)

    ALOGI("Current debug level=%d", *dst_level);

    if (src_level < L_VERBOSE || src_level >  L_SUPPRESS)
    {
        ALOGE("Invalid debug level, level=%d", src_level);
        ALOGE("  [level] -");
        ALOGE("  QUIET      = 6");
        ALOGE("  ASSERT     = 5");
        ALOGE("  ERROR      = 4");
        ALOGE("  WARNING    = 3");
        ALOGE("  INFO       = 2");
        ALOGE("  DEBUG      = 1");
        ALOGE("  LOGALL     = 0");
        return -1;
    }

    *dst_level = src_level;

    ALOGI("New debug level=%d", *dst_level);

#elif defined(__LINUX_OS__)

    printf("Current debug level=%d\n", *dst_level);

    if (src_level < L_VERBOSE || src_level >  L_SUPPRESS)
    {
        printf("Invalid debug level, level=%d", src_level);
        printf("  [level] -");
        printf("  QUIET      = 6");
        printf("  ASSERT     = 5");
        printf("  ERROR      = 4");
        printf("  WARNING    = 3");
        printf("  INFO       = 2");
        printf("  DEBUG      = 1");
        printf("  LOGALL     = 0");
        return -1;
    }

    *dst_level = src_level;

    printf("New debug level=%d\n", *dst_level);

#endif
    return 0;
}

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
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <inttypes.h>

#include "adr_release_api.h"   //from algo

extern int log_dbg_level;

#define LOG_IS_ENABLED(level) (log_dbg_level <= level)

void usage(int error_code);
unsigned int get_time(void);
int set_log_level(int *dst_level, int src_level);

#define FILE_NAME(x) strrchr(x, '/')?strrchr(x, '/') + 1 : x

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

#if defined(__ANDROID_OS__)

#include <log/log.h>     // Android log

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MTK_ADR"
#endif

#define PRINT_LOG(loglevel, fmt, args...) \
        do {\
                if (LOG_IS_ENABLED(loglevel)) {\
                    switch (loglevel){\
                        case L_ASSERT:\
                        {\
                            ALOG_ASSERT("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_ERROR:\
                        {\
                            ALOGE("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_WARN:\
                        {\
                            ALOGW("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_INFO:\
                        {\
                            ALOGI("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_DEBUG:\
                        {\
                            ALOGD("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_VERBOSE:\
                        {\
                            ALOGV("%s %s %d " fmt, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                    }\
                }\
        } while (0)

#define LOG_ASSERT(fmt, args...)  PRINT_LOG(L_ASSERT, fmt, ##args)
#define LOG_ERROR(fmt, args...)   PRINT_LOG(L_ERROR, fmt, ##args)
#define LOG_WARN(fmt, args...)    PRINT_LOG(L_WARN, fmt, ##args)
#define LOG_INFO(fmt, args...)    PRINT_LOG(L_INFO, fmt, ##args)
#define LOG_DEBUG(fmt, args...)   PRINT_LOG(L_DEBUG, fmt, ##args)
#define LOG_VERBOSE(fmt, args...) PRINT_LOG(L_VERBOSE, fmt, ##args)

#elif defined(__LINUX_OS__)

#define LOG_TAG "MTK_ADR"

char time_buff[64];
int get_time_str(char* buf, int len);

#ifndef gettid
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
#define getpid() syscall(__NR_getpid)
#endif

#define PRINT_LOG(loglevel, tag, fmt, args...) \
    do {\
            if (LOG_IS_ENABLED(loglevel)) {\
                get_time_str(time_buff, sizeof(time_buff));\
                printf("%ld %ld [%s]" LOG_TAG tag "%s %s %d "  fmt "\n",\
                    getpid(), gettid(), time_buff, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                fflush(stdout);\
        }\
    } while (0)

#define LOG_ASSERT(fmt, args...)  PRINT_LOG(L_ASSERT, "[ASSERT]: ", fmt, ##args)
#define LOG_ERROR(fmt, args...)   PRINT_LOG(L_ERROR, "[ERROR]: ", fmt, ##args)
#define LOG_WARN(fmt, args...)    PRINT_LOG(L_WARN, "[WARNING]: ",  fmt, ##args)
#define LOG_INFO(fmt, args...)    PRINT_LOG(L_INFO, "[INFO]: ",  fmt, ##args)
#define LOG_DEBUG(fmt, args...)   PRINT_LOG(L_DEBUG, "[DEBUG]: ",  fmt, ##args)
#define LOG_VERBOSE(fmt, args...) PRINT_LOG(L_VERBOSE, "[VERBOSE]: ",  fmt, ##args)

#endif

#endif

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
#ifndef MTK_AUTO_LOG_H
#define MTK_AUTO_LOG_H

/**
 *  @enum LOG_LEVEL
 *  @brief Define Log Level
 */
typedef enum {
    L_VERBOSE = 0, L_DEBUG, L_INFO, L_WARN, L_ERROR, L_ASSERT, L_SUPPRESS
} LOG_LEVEL;

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif

#ifndef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "mnld"
#endif

#include <string.h>

int set_log_level(int *dst_level, int src_level);

extern int log_dbg_level;
#define LOG_IS_ENABLED(level) (log_dbg_level <= level)

#define FILE_NAME(x) strrchr(x, '/')?strrchr(x, '/') + 1 : x

#if defined(__ANDROID_OS__)

#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/

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

#define LOGA(fmt, args...)    PRINT_LOG(L_ASSERT, fmt, ##args)
#define LOGE(fmt, args...)    PRINT_LOG(L_ERROR, fmt, ##args)
#define LOGW(fmt, args...)    PRINT_LOG(L_WARN, fmt, ##args)
#define LOGI(fmt, args...)    PRINT_LOG(L_INFO, fmt, ##args)
#define LOGD(fmt, args...)    PRINT_LOG(L_DEBUG, fmt, ##args)
#define LOGV(fmt, args...)    PRINT_LOG(L_VERBOSE, fmt, ##args)

#define  TRC(f)       ALOGD("%s", __func__)

#elif defined(__LINUX_OS__)

#include <stdio.h>

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
                        getpid(), gettid(),time_buff, FILE_NAME(__FILE__), __FUNCTION__, __LINE__, ##args);\
                    fflush(stdout);\
                }\
        } while (0)

#define LOGA(fmt, args...)    PRINT_LOG(L_ASSERT, "[ASSERT]: ", fmt, ##args)
#define LOGE(fmt, args...)    PRINT_LOG(L_ERROR, "[ERROR]: ", fmt, ##args)
#define LOGW(fmt, args...)    PRINT_LOG(L_WARN, "[WARNING]: ", fmt, ##args)
#define LOGI(fmt, args...)    PRINT_LOG(L_INFO, "[INFO]: ", fmt, ##args)
#define LOGD(fmt, args...)    PRINT_LOG(L_DEBUG, "[DEBUG]: ", fmt, ##args)
#define LOGV(fmt, args...)    PRINT_LOG(L_VERBOSE, "[VERBOSE]: ", fmt, ##args)

#define  TRC(f)       ((void)0)

#elif defined(__TIZEN_OS__)

#include <dlog/dlog.h>

#define PRINT_LOG(loglevel, tag, fmt, args...) \
        do {\
                if (LOG_IS_ENABLED(loglevel)) {\
                    dlog_print(DLOG_DEBUG, fmt "\n", ##args);\
                    printf(fmt "\n", ##args);
                    fflush(stdout);\
                }\
        } while (0)

#define LOGA(fmt, args...)    PRINT_LOG(L_ASSERT, "[ASSERT]: ", fmt, ##args)
#define LOGE(fmt, args...)    PRINT_LOG(L_ERROR, "[ERROR]: ", fmt, ##args)
#define LOGW(fmt, args...)    PRINT_LOG(L_WARN, "[WARNING]: ", fmt, ##args)
#define LOGI(fmt, args...)    PRINT_LOG(L_INFO, "[INFO]: ", fmt, ##args)
#define LOGD(fmt, args...)    PRINT_LOG(L_DEBUG, "[DEBUG]: ", fmt, ##args)
#define LOGV(fmt, args...)    PRINT_LOG(L_VERBOSE, "[VERBOSE]: ", fmt, ##args)

#endif

#endif

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

#ifndef VEHICLE_INFO_H_
#define VEHICLE_INFO_H_

#include <time.h>
#include <stdio.h>

/**
 *  @enum VEHICLE_RET
 *  @brief Return Value of Vehicle Information API
 */
typedef enum
{
    V_SUCCESS = 0, V_ERROR = -1, V_NOTSUPPORT = -2
}VEHICLE_RET;

/**
 *  @enum GEAR
 *  @brief Vehicle Gear Type
 */
typedef enum
{
    GEAR_P = 0, GEAR_R, GEAR_N, GEAR_D
}GEAR;

/**
 *  @enum LOG_LEVEL
 *  @brief Define Log Level
 */
typedef enum {
    L_VERBOSE = 0, L_DEBUG, L_INFO, L_WARN, L_ERROR, L_ASSERT, L_SUPPRESS
} LOG_LEVEL;

int32_t vehicle_set_log_level(void *handle, int32_t level);
int32_t vehicle_init(void **handle);
int32_t vehicle_deinit(void **handle);
int32_t vehicle_start(void *handle);
int32_t vehicle_stop(void *handle);
int32_t vehicle_update(void *handle);
int32_t vehicle_get_vehicle_speed(void *handle, float *vehicle_speed,
            struct timespec *timestamp);
int32_t vehicle_get_wheel_speed(void *handle, float *l_front_speed,
            float *r_front_speed, float *l_rear_speed, float *r_rear_speed,
            struct timespec *timestamp);
int32_t vehicle_get_gear(void *handle, int32_t *gear, struct timespec *timestamp);
int32_t vehicle_get_steering_angle(void *handle, float *angle,
            struct timespec *timestamp);

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

unsigned int get_time(void);

extern int  vehicle_log_level;
#define PATH_LEN 128

#define LOG_IS_ENABLED(level) \
             (vehicle_log_level <= level)

#if defined(__ANDROID_OS__)

#include <log/log.h>     // Android log

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MTK_ADR_VEHICLE"
#endif

#define PRINT_LOG(loglevel, fmt, args...) \
        do {\
                if (LOG_IS_ENABLED(loglevel)) {\
                    switch (loglevel){\
                        case L_ASSERT:\
                        {\
                            ALOGE("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_ERROR:\
                        {\
                            ALOGE("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_WARN:\
                        {\
                            ALOGW("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_INFO:\
                        {\
                            ALOGI("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_DEBUG:\
                        {\
                            ALOGD("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
                            break;\
                        }\
                        case L_VERBOSE:\
                        {\
                            ALOGV("%s %s %d " fmt, __FILE__, __FUNCTION__, __LINE__, ##args);\
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

#define LOG_TAG "MTK_ADR_VEHICLE"

#ifndef gettid
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
#endif

#define PRINT_LOG(loglevel, tag, fmt, args...) \
        do {\
                if (LOG_IS_ENABLED(loglevel)) {\
                    printf("%ld [%u]" LOG_TAG tag "%s %s %d "  fmt "\n",\
                        gettid(), get_time(), __FILE__, __FUNCTION__, __LINE__, ##args);\
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

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
 * @Author: Rain Luo
 * @Date: 2020-07-21 10:01:47
 * @LastEditTime: 2020-07-21 15:26:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \anld\simulation.h
 */
#ifndef __AIROHA_SIMULATION_H
#define __AIROHA_SIMULATION_H
#include <stdio.h>
#ifdef __cplusplus
#include "AdvancedLogger.h"
#include "LogRelayer.h"
// #define SD_CARD_LOG_SUPPORT
// define for debug
#ifdef LOG_TAG
#define LOG_D(format, ...)                                                    \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_DEBUG, \
                               "[D][" LOG_TAG "] " format, ##__VA_ARGS__);
#define TRACE_D(format, ...) \
    LogUtil::LogRelay::relayTrace("[D][" LOG_TAG "] " format, ##__VA_ARGS__);
#define TRACE_D_BIN(binary, length) \
    LogUtil::LogRelay::relayTraceBinary(binary, length);
#else
#define LOG_D(format, ...)                                                    \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_DEBUG, \
                               "[D][] " format "\n", ##__VA_ARGS__);
#define TRACE_D(format, ...) \
    LogUtil::LogRelay::relayTrace("[D][] " format, ##__VA_ARGS__);
#endif  // LOG_TAG
// define for info
#ifdef LOG_TAG
#define LOG_I(format, ...)                                                   \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_INFO, \
                               "[I][" LOG_TAG "] " format, ##__VA_ARGS__);
#define EVENT_I(format, ...) \
    LogUtil::LogRelay::relayEvent("[I][" LOG_TAG "] " format, ##__VA_ARGS__);
#else
#define LOG_I(format, ...)                                                   \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_INFO, \
                               "[I][] " format "\n", ##__VA_ARGS__);
#endif  // LOG_TAG
// define for warning
#ifdef LOG_TAG
#define LOG_W(format, ...)                               \
    AdvancedLogger::advanceLog(                          \
        AdvancedLogger::LogLevel::ADV_LOG_LEVEL_WARNING, \
        "[W][" LOG_TAG "] " format, ##__VA_ARGS__);
#else
#define LOG_W(format, ...)                                                     \
    AdvancedLogger::advanceLog(                                                \
        AdvancedLogger::LogLevel::ADV_LOG_LEVEL_WARNING, "[W][] " format "\n", \
        ##__VA_ARGS__);
#endif  // LOG_TAG
// define for error
#ifdef LOG_TAG
#define LOG_E(format, ...)                                                    \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_ERROR, \
                               "[E][" LOG_TAG "] " format, ##__VA_ARGS__);
#define TRACE_E(format, ...)                                      \
    LogUtil::LogRelay::relayTrace("[Error][" LOG_TAG "] " format, \
                                  ##__VA_ARGS__);
#else
#define LOG_E(format, ...)                                                    \
    AdvancedLogger::advanceLog(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_ERROR, \
                               "[E][] " format "\n", ##__VA_ARGS__);
#define TRACE_E(format, ...) \
    LogUtil::LogRelay::relayTrace("[Error][] " format, ##__VA_ARGS__);
#endif  // LOG_TAG
// raw print
#define RAW_D(data, size) AdvancedLogger::advanceRawData(data, size);
// sd card support
#ifdef SD_CARD_LOG_SUPPORT
#ifndef LOG_TAG
#define LOG_TAG "N"
#endif  // LOG_TAG
#define ASLOGD(format, ...)                                        \
    LogUtil::LogRelay::relaySdLog("[D][" LOG_TAG "] " format "\n", \
                                  ##__VA_ARGS__);
#define ASLOGI(format, ...) \
    LogUtil::LogRelay::relaySdLog("[I][" LOG_TAG "] " format "\n", \
                                  ##__VA_ARGS__);
#define ASLOGW(format, ...) \
    LogUtil::LogRelay::relaySdLog("[W][" LOG_TAG "] " format "\n", \
                                  ##__VA_ARGS__);
#define ASLOGE(format, ...)                                        \
    LogUtil::LogRelay::relaySdLog("[E][" LOG_TAG "] " format "\n", \
                                  ##__VA_ARGS__);
#else
#define ASLOGD(format, ...)
#define ASLOGI(format, ...)
#define ASLOGW(format, ...)
#define ASLOGE(format, ...)
#endif  // SD_CARD_LOG_SUPPORT
#else
#ifndef LOG_TAG
#define LOG_TAG "U"
#endif
#ifndef AIROHA_SIMULATION
#include <android/log.h>
#define LOG_D(format, ...)                             \
    __android_log_print(ANDROID_LOG_DEBUG, "ANLD_LOG", \
                        "[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_I(format, ...)                                                     \
    __android_log_print(ANDROID_LOG_INFO, "ANLD_LOG", "[" LOG_TAG "] " format, \
                        ##__VA_ARGS__)
#define LOG_W(format, ...)                                                     \
    __android_log_print(ANDROID_LOG_WARN, "ANLD_LOG", "[" LOG_TAG "] " format, \
                        ##__VA_ARGS__)
#define LOG_E(format, ...)                             \
    __android_log_print(ANDROID_LOG_ERROR, "ANLD_LOG", \
                        "[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_F(format, ...)                             \
    __android_log_print(ANDROID_LOG_FATAL, "ANLD_LOG", \
                        "[" LOG_TAG "] " format, ##__VA_ARGS__)
#else
#define LOG_D(format, ...)  printf("[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_I(format, ...)  printf("[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_W(format, ...)  printf("[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_E(format, ...)  printf("[" LOG_TAG "] " format, ##__VA_ARGS__)
#define LOG_F(format, ...)  printf("[" LOG_TAG "] " format, ##__VA_ARGS__)
#endif  // AIROHA_SIMULATION
#endif  // __cplusplus

#endif  // __AIROHA_SIMULATION_H


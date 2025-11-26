/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <iostream>
#ifdef ANDROID
#include <utils/Log.h>
#else
#include <cstdio>
#include <cerrno>
#include <cstring>
#endif

#include "log.h"

// #define ENABLE_LOG
// #define ENABLE_DUMP

#define LOG_DEBUG_PREFIX   ""
#define LOG_INFO_PREFIX    ""
#define LOG_WARN_PREFIX    "[WARN] "
#define LOG_ERR_PREFIX     "[ERR]  "

#define BUFFER_SIZE         2048

typedef struct
{
    char outputBuffer[BUFFER_SIZE];
} LogInstance;

static LogInstance sLogInstance;


static void logd_(const char *msg)
{
#ifdef ENABLE_DUMP
#ifdef ANDROID
    ALOGD("%s", msg);
#elif _WIN32
    std::cout << LOG_DEBUG_PREFIX << msg << std::endl;
#else
    fprintf(stderr, "%s %s\n", LOG_DEBUG_PREFIX, msg);
#endif
#endif
}

static void logi_(const char *msg)
{
#ifdef ANDROID
    ALOGI("%s", msg);
#elif _WIN32
    std::cout << LOG_INFO_PREFIX << msg << std::endl;
#else
    fprintf(stderr, "%s %s\n", LOG_INFO_PREFIX, msg);
#endif
}

static void logw_(const char *msg)
{
#ifdef ANDROID
    ALOGW("%s", msg);
#elif _WIN32
    std::cout << LOG_WARN_PREFIX << msg << std::endl;
#else
    fprintf(stderr, "%s %s\n", LOG_WARN_PREFIX, msg);
#endif
}

static void loge_(const char *msg)
{
#ifdef ANDROID
    ALOGE("%s", msg);
    ALOGE("exit app");
#elif _WIN32
    std::cout << LOG_ERR_PREFIX << msg << std::endl;
    std::cout << LOG_ERR_PREFIX << "exit app" << std::endl;
#else
    fprintf(stderr, "%s %s\n", LOG_ERR_PREFIX, msg);
    fprintf(stderr, "%s exit app\n", LOG_ERR_PREFIX);
#endif
    abort();
}

static void init_log_instance()
{
    LogInstance *inst = &sLogInstance;
    memset(inst->outputBuffer, 0, sizeof(inst->outputBuffer));
}

/* ----------------------------------------------------------- */

void log_init()
{
    init_log_instance();
}

void log_deinit()
{
    init_log_instance();
}

void logd(const char *format, ...)
{
#ifdef ENABLE_LOG
    LogInstance *inst = &sLogInstance;
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(inst->outputBuffer, sizeof(inst->outputBuffer), format, arglist);
    va_end(arglist);

    logd_(inst->outputBuffer);
#endif
}

void logi(const char *format, ...)
{
#ifdef ENABLE_LOG
    LogInstance *inst = &sLogInstance;
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(inst->outputBuffer, sizeof(inst->outputBuffer), format, arglist);
    va_end(arglist);

    logi_(inst->outputBuffer);
#endif
}

void logw(const char *format, ...)
{
#ifdef ENABLE_LOG
    LogInstance *inst = &sLogInstance;
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(inst->outputBuffer, sizeof(inst->outputBuffer), format, arglist);
    va_end(arglist);

    logw_(inst->outputBuffer);
#endif
}

void loge(const char *format, ...)
{
#ifdef ENABLE_LOG
    LogInstance *inst = &sLogInstance;
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(inst->outputBuffer, sizeof(inst->outputBuffer), format, arglist);
    va_end(arglist);

    loge_(inst->outputBuffer);
#endif
}
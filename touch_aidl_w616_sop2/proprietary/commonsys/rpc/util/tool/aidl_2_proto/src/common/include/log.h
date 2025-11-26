/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <string>

using std::string;

#define FUNC_ENTER()            // logd("%s: +++", __func__)
#define FUNC_LEAVE()            // logd("%s: ---", __func__)

#define OUTPUT_FUNC()           // logd("%s", __func__)

void log_init();

void log_deinit();

void logd(const char *format, ...);

void logi(const char *format, ...);

void logw(const char *format, ...);

void loge(const char *format, ...);
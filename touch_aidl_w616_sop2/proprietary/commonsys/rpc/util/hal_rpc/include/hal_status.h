/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#include <android/binder_auto_utils.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>

using ndk::ScopedAStatus;
using std::string;
using std::tuple;

typedef int32_t     HalStatusCode;

#define HAL_ERROR_UNKNOWN           ((HalStatusCode) -1)

typedef struct
{
    int32_t status;
    string info;
} HalStatusT;

ScopedAStatus CreateScopedAStatus(int32_t status);
ScopedAStatus CreateScopedAStatus(int32_t status, const string& info);
ScopedAStatus CreateScopedAStatus(const HalStatusT* halStatus);

ScopedAStatus CreateScopedAStatusError();

/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "aidl_util.h"

bool aidl_init(const string& aidlPath);

const AidlInfo& aidl_get_info();

EnumStatsInfo* aidl_get_enum_stats();

string aidl_get_package_from_message_name(const string& str);

void aidl_deinit();
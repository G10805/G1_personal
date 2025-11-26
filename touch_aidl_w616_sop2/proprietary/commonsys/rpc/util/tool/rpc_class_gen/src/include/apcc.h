/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "apcc_common.h"

bool apcc_init(const ApccParam& param);

const ApccInfo& apcc_get_info();

ApccParam& apcc_get_param();

bool apcc_generate_files();

void apcc_deinit();

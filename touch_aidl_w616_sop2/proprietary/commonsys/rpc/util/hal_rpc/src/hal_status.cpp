/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <android/binder_auto_utils.h>

#include "hal_status.h"

ScopedAStatus CreateScopedAStatus(int32_t status) {
    return ScopedAStatus::fromServiceSpecificError(status);
}

ScopedAStatus CreateScopedAStatus(int32_t status, const string& info) {
    return ScopedAStatus::fromServiceSpecificErrorWithMessage(status, info.c_str());
}

ScopedAStatus CreateScopedAStatus(const HalStatusT* halStatus) {
    return (halStatus != NULL) ?
        (halStatus->info.empty() ?
        CreateScopedAStatus(halStatus->status) :
        CreateScopedAStatus(halStatus->status, halStatus->info)) :
        CreateScopedAStatus(HAL_ERROR_UNKNOWN);
}

ScopedAStatus CreateScopedAStatusError() {
    return CreateScopedAStatus(HAL_ERROR_UNKNOWN);
}

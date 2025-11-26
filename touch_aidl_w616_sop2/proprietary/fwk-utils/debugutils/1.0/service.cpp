/******************************************************************************
Copyright (c) 2020,2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include <vendor/qti/hardware/debugutils/1.0/IDebugUtils.h>
#include <DebugUtils.h>
#include <hidl/LegacySupport.h>

using vendor::qti::hardware::debugutils::V1_0::IDebugUtils;
using android::hardware::defaultPassthroughServiceImplementation;
int main() {
    return defaultPassthroughServiceImplementation<IDebugUtils>(16);
}

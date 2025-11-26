/*===========================================================================

*//** @file service.cpp

Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header:  $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
01/15/20           sh          Bringup DVR on LA GVM on Hana
01/22/19           sh          Impement HIDL interface for DVR
============================================================================*/
#include <vendor/qti/dvr/1.0/IDvrDisplay.h>
#include <hidl/LegacySupport.h>
#include "hyp_dvr_display_be.h"

using vendor::qti::dvr::V1_0::implementation::DvrDisplay;
using vendor::qti::dvr::V1_0::IDvrDisplay;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;

int main() {
    DvrDisplay *dvrObj = new DvrDisplay();

    if (dvrObj != NULL) {
        android::sp<IDvrDisplay> service = dvrObj;

        configureRpcThreadpool(1, true /*callerWillJoin*/);

        if (service->registerAsService() != OK) {
            ALOGE("Cannot register IDvrDisplay HAL service");
            return 1;
        }

        ALOGE("Registered IDvrDisplay HAL service success!");
        joinRpcThreadpool();
    }
    return 0;//defaultPassthroughServiceImplementation<IDvr>();
}

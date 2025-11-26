/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_AIDL_vendor.qti.gnss@service"

#include <log_util.h>
#include <loc_cfg.h>
#include <loc_pla.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "LocAidlGnss.h"

using aidl::vendor::qti::gnss::implementation::LocAidlGnss;

int main(int /* argc */, char* /* argv */ []) {
    UTIL_READ_CONF_DEFAULT(LOC_PATH_GPS_CONF);
    std::shared_ptr<LocAidlGnss> gnssService = ndk::SharedRefBase::make<LocAidlGnss>();
    if (gnssService != nullptr) {
        ndk::SpAIBinder locAidlBinder = gnssService->asBinder();
        const std::string instance = std::string() + LocAidlGnss::descriptor + "/default";
        AServiceManager_addService(locAidlBinder.get(), instance.c_str());
    }
    return 0;
}

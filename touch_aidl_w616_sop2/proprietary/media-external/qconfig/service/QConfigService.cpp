/*
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <android-base/logging.h>
#include <utils/Log.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include "QConfig.h"
#include <string>

using ::aidl::vendor::qti::hardware::qconfig::QConfig;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(10);
    std::string path = "/vendor/etc/qconfigpresets.json";
    std::shared_ptr<QConfig> qConfigSvc = ndk::SharedRefBase::make<QConfig>(path);
    if (qConfigSvc == nullptr) {
        ALOGE("qConfigSvc is null");
        return 1;
    }
    const std::string instance = std::string() + QConfig::descriptor + "/default";
    ALOGI("Starting QConfig service...");
    binder_status_t status = AServiceManager_addService(qConfigSvc->asBinder().get(), instance.c_str());
    if (status) {
        ALOGE("failed to register qconfig service: 0x%x", status);
    }
    CHECK_EQ(status, STATUS_OK);
    ALOGI("QConfig service is registered, starting thread pool");
    ABinderProcess_startThreadPool();
    ABinderProcess_joinThreadPool();
    ALOGE("Can't register QConfig service");
    return status;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.

 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "EvsEnumerator.h"
#include "EvsGlDisplay.h"

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <utils/Log.h>

#include <unistd.h>
#include <cutils/properties.h>

#include <atomic>
#include <cstdlib>
#include <string_view>

namespace {

using android::frameworks::automotive::display::V1_0::IAutomotiveDisplayProxyService;
using ::aidl::android::hardware::automotive::evs::implementation::EvsEnumerator;

constexpr std::string_view kDisplayServiceInstanceName = "/default";
constexpr std::string_view kHwInstanceName = "/hw/1";
constexpr int kNumBinderThreads = 1;

}  // namespace

/*abort condition*/
static pthread_mutex_t mutex_abort = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_abort = PTHREAD_COND_INITIALIZER;

#ifdef QCC_SOC_OVERRIDE

static bool isMakena()
{
    int     socFd = 0;
    char    buf[32]  = { 0 };
    int     chipsetVersion = -1;
    char    fileName[] = "/sys/devices/soc0/soc_id";

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        int ret = read(socFd, buf, sizeof(buf) - 1);

        if (-1 == ret)
            return false;
        else
            chipsetVersion = atoi(buf);

        close(socFd);

        if (chipsetVersion == 460) //CHIP_ID_SA8295P = 460
            return true;
    }

    return false;
}

#endif

int main() {
    LOG(INFO) << "EVS Hardware Enumerator service is starting";

    char value[92];
    int QCC_FLAG_ENABLED = 0;
    property_get("ro.boot.camera.qcc.version", value, "");
    LOG(INFO) << "value of property for qcc:" << value;
    QCC_FLAG_ENABLED = (strlen(value) > 0) ? 1:0;

    if (QCC_FLAG_ENABLED) {
        if ((value[0] - '0') != QCARCAM_VERSION_MAJOR) {
            LOG(INFO) << "Blocking EVS service for QCC.6.0";
            pthread_cond_wait(&cond_abort, &mutex_abort);
            return 1;
        }
    }
    else {
#ifdef QCC_SOC_OVERRIDE
    if (!isMakena()) {
        LOG(INFO) << "Blocking EVS service for QCC.6.0";
        pthread_cond_wait(&cond_abort, &mutex_abort);
        return 1;
    }
#endif
    }
    LOG(INFO) << "EVS service (QCC.6.0) started...";

    android::sp<IAutomotiveDisplayProxyService> carWindowService =
        IAutomotiveDisplayProxyService::getService("default");
    if (carWindowService == nullptr) {
        LOG(ERROR) << "Cannot use AutomotiveDisplayProxyService.  Exiting.";
        return 1;
    }

    // Register our service -- if somebody is already registered by our name,
    // they will be killed (their thread pool will throw an exception).
    std::shared_ptr<EvsEnumerator> service =
            ndk::SharedRefBase::make<EvsEnumerator>(carWindowService);
    if (!service) {
        LOG(ERROR) << "Failed to instantiate the service";
        return EXIT_FAILURE;
    }


    const std::string instanceName =
            std::string(EvsEnumerator::descriptor) + std::string(kHwInstanceName);
    auto err = AServiceManager_addService(service->asBinder().get(), instanceName.data());
    if (err != EX_NONE) {
        LOG(ERROR) << "Failed to register " << instanceName << ", exception = " << err;
        return EXIT_FAILURE;
    }

    if (!ABinderProcess_setThreadPoolMaxThreadCount(kNumBinderThreads)) {
        LOG(ERROR) << "Failed to set thread pool";
        return EXIT_FAILURE;
    }

    ABinderProcess_startThreadPool();
    LOG(INFO) << "EVS Hardware Enumerator is ready";

    ABinderProcess_joinThreadPool();
    // In normal operation, we don't expect the thread pool to exit
    LOG(INFO) << "EVS Hardware Enumerator is shutting down";


    return EXIT_SUCCESS;
}

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *Not a contribution.
 *
 * Copyright 2017 The Android Open Source Project
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

#ifdef LAZY_SERVICE
#define LOG_TAG "vendor.aidl.provider-service-lazy"
#else
#define LOG_TAG "vendor.aidl.provider-service"
#endif

#include <aidl/android/hardware/camera/provider/ICameraProvider.h>

#include <hidl/HidlTransportSupport.h>

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <cutils/properties.h>
#include <malloc.h>
#include <utils/Errors.h>

#include <cinttypes>


#include "camera_provider.h"
#include <sched.h>

using android::status_t;
using aidl::android::hardware::camera::provider::ICameraProvider;
using ::android::hardware::camera::provider::implementation::CameraProvider;

#ifdef LAZY_SERVICE
const bool kLazyService = true;
#else
const bool kLazyService = false;
#endif


const std::string kProviderInstance = "/vendor_qti/0";

int main()
{
    using namespace android::hardware::camera::provider::implementation;

    ALOGI("CameraProvider service is starting. kLazyService %d", kLazyService);

    struct sched_param param = {0};

    if(sched_getparam(0, &param) != 0)
    {
        ALOGE("Fail to get scheduling param: %d", errno);
    }

    else
    {
        int currentSchedPolicy = -1;

        currentSchedPolicy = sched_getscheduler(0);

        if (currentSchedPolicy < 0)
        {
            ALOGE("Fail to get current policy %d", errno);
        }
        else
        {

            if (sched_setscheduler(0, currentSchedPolicy | SCHED_RESET_ON_FORK,
                &param) != 0)
            {
                ALOGE("Fail to set SCHED_RESET_ON_FORK: %d", errno);
                ALOGE("The current scheduling policy is %d", currentSchedPolicy);
            }
        }
    }

    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::hardware::configureRpcThreadpool(/*maxThreads=*/HWBINDER_THREAD_COUNT,
                                            /*callerWillJoin=*/true);
    // Don't depend on vndbinder setting up threads in case we stop using them
    // some day
    ABinderProcess_setThreadPoolMaxThreadCount(6);
    ABinderProcess_startThreadPool();
    std::shared_ptr<ICameraProvider> provider = CameraProvider::Create();
    if (NULL == provider)
    {
        ALOGE("Failed to instantiate the provider service");
        return android::NO_INIT;
    }

    std::string instance =
    std::string() + CameraProvider::descriptor + kProviderInstance;

    if (kLazyService) {
        if (AServiceManager_addService(provider->asBinder().get(),
                                            instance.c_str()) != STATUS_OK) {
            ALOGE("Cannot register AIDL QTI camera provider lazy service");
            return android::NO_INIT;
        }
    } else {
        if (AServiceManager_addService(provider->asBinder().get(),
                                   instance.c_str()) != STATUS_OK) {
            ALOGE("Cannot register AIDL QTI camera provider service");
            return android::NO_INIT;
        }
        ALOGI("Added camera provider service");
    }
    ABinderProcess_joinThreadPool();

    ALOGI("CameraProvider service is started. kLazyService %d, provider %p",
          kLazyService, provider.get());
    return 0;
}

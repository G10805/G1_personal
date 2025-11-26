/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
/* Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
*/

/*
 * Copyright 2018 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "vendor.qti.media.c2audio@1.0-service"

#include <codec2/hidl/1.0/ComponentStore.h>
#include <hidl/HidlTransportSupport.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <minijail.h>

#include <C2Component.h>
#include "QC2ComponentStoreFactory.h"
using namespace qc2audio;

#include <stdlib.h>

#include <vndksupport/linker.h>
#include <dlfcn.h>
#include <fcntl.h>

static constexpr char kBaseSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/c2audio.vendor.base-arm.policy";

static constexpr char kExtSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/c2audio.vendor.ext-arm.policy";

static constexpr char kBaseSeccomp64BitPolicyPath[] =
        "/vendor/etc/seccomp_policy/c2audio.vendor.base-arm64.policy";

static constexpr char kExtSeccomp64BitPolicyPath[] =
        "/vendor/etc/seccomp_policy/c2audio.vendor.ext-arm64.policy";

int main(int argc, char** argv) {
    ALOGD("media HW codec service starting...");

    signal(SIGPIPE, SIG_IGN);
    if (sizeof(intptr_t) == 8) {
        ALOGD("media HW codec (64-bit) service starting...");
        android::SetUpMinijail(kBaseSeccomp64BitPolicyPath, kExtSeccomp64BitPolicyPath);
    } else {
        ALOGD("media HW codec (32-bit) service starting...");
        android::SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);
    }

    if (argc > 0) {
        if (argv && argv[0]) {
            size_t len = strlen(argv[0]);
            strlcpy(argv[0], "media.audio.qc.codec", len);
        }
    }

    // vndbinder is needed by BufferQueue.
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::ProcessState::self()->startThreadPool();

    android::hardware::configureRpcThreadpool(8, true /* callerWillJoin */);

    // Create IComponentStore service.
    using namespace ::android::hardware::media::c2::V1_0;

    android::sp<IComponentStore> store;
    void *lib = dlopen("libqc2audio_core.so", RTLD_NOW);
    if (lib == nullptr) {
        ALOGE("QC2AudioHALService: failed to open %s: %s", "libqc2audio_core.so", dlerror());
        goto exit;
    } else {
        auto factoryGetter =
                    (QC2ComponentStoreFactoryGetter_t)dlsym(lib, kFn_QC2AudioComponentStoreFactoryGetter);
        if (factoryGetter == nullptr) {
            ALOGE("QC2AudioHALService: failed to load symbol %s: %s",
            kFn_QC2AudioComponentStoreFactoryGetter, dlerror());
        } else {
            auto c2StoreFactory = (*factoryGetter)(1, 0);    // get version 1.0
            if (c2StoreFactory == nullptr) {
                ALOGE("QC2AudioHALService: failed to get Store factory !");
                goto cleanup_before_exit;
            }
            store = new utils::ComponentStore(c2StoreFactory->getInstance());
            if (store == nullptr) {
                ALOGE("Cannot create Codec2Audio's IComponentStore default service.");
                goto cleanup_before_exit;
            } else {
                if (store->registerAsService("default2") != android::OK) {
                ALOGE("Cannot register Codec2Audio's IComponentStore default service...");
                goto cleanup_before_exit;
                } else {
                    ALOGI("Codec2Audio's IComponentStore default service created.");
                }
            }
        }
    }

cleanup_before_exit:
    dlclose(lib);
exit:
    android::hardware::joinRpcThreadpool();
    return 0;
}

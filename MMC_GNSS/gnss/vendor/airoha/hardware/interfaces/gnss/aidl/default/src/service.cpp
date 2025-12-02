/*
 * Copyright 2020, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "Gnss-main"
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <pthread.h>
#include "Gnss.h"
#include "GnssAidlImpl.h"
#include "simulation.h"
using aidl::android::hardware::gnss::Gnss;
using ::android::OK;
using ::android::sp;
using ::android::hardware::configureRpcThreadpool;
static std::thread connectingThread;
extern const char* HIDL_BUILD_VERSION;
int main() {
    AdvancedLogger::setLogCatagory("GnssAidl");
    GnssAidlImpl::getInstance()->startTransaction();
    ABinderProcess_setThreadPoolMaxThreadCount(2);
    ABinderProcess_startThreadPool();
    ALOGD("Airoha Gnss AIDL Version: %s", HIDL_BUILD_VERSION);
    std::shared_ptr<Gnss> gnssAidl = ndk::SharedRefBase::make<Gnss>();
    std::string instance = std::string() + Gnss::descriptor + "/default";
    ALOGD("Airoha Gnss AIDL Version: %s", instance.c_str());
    binder_status_t status = AServiceManager_addService(
        gnssAidl->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK);
    ABinderProcess_joinThreadPool();
    GnssAidlImpl::getInstance()->stopTransaction();
    return EXIT_FAILURE;  // should not reach
}
#define MAX_SPRINTF_BUFFER_LEN 2048
void print_ipc_log(debug_level_t level, const char* format, ...) {
    char buffer[MAX_SPRINTF_BUFFER_LEN];
    va_list args;
    va_start(args, format);
    int k = vsnprintf(buffer, MAX_SPRINTF_BUFFER_LEN, format, args);
    if (k == MAX_SPRINTF_BUFFER_LEN) {
        ALOGE("The following log may be incomplete !");
    }
    if (level == LOG_LEVEL_DEBUG) {
        // ALOGD("%s",sprintfBuffer);
    } else {
        ALOGE("%s", buffer);
    }
    va_end(args);
}
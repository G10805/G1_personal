#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <iostream>
#include <log/log.h>
#include <android-base/macros.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include "LedService.h"

using aidl::vendor::visteon::led::LedService;

int main() {
  ALOGI("LED service is starting");
  android::ProcessState::initWithDriver("/dev/vndbinder");
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();
    
    std::shared_ptr<LedService> service = LedService::getInstance();
const std::string instance = std::string() + LedService::descriptor + "/default";

    ALOGI("Registering as service...");
    binder_status_t status = AServiceManager_addService(service->asBinder().get(),instance.c_str());

    if (status == STATUS_OK) {
        ALOGD("%s is ready.", instance.c_str());
        ABinderProcess_joinThreadPool();
    } else {
        ALOGE("Could not register service %s .", instance.c_str());
    }


    return 0;
}


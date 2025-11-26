#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <iostream>
#include <log/log.h>
#include <android-base/macros.h>
#include "Touch.h"

#define LOG_TAG "TouchService"

using aidl::vendor::visteon::hardware::interfaces::touch::ITouch;



int main() {
  ALOGI("Touch service is starting");
  android::ProcessState::initWithDriver("/dev/vndbinder");
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();
    
    std::shared_ptr<ITouch> service =  vendor::visteon::hardware::interfaces::touch::implementation::Touch::getInstance();
    const std::string instance = std::string() + ITouch::descriptor + "/default";

    ALOGI("Touch service Registering as service...");
    binder_status_t status = AServiceManager_addService(service->asBinder().get(),instance.c_str());

    if (status == STATUS_OK) {
        ALOGD("%s is ready.", instance.c_str());
        ABinderProcess_joinThreadPool();
    } else {
        ALOGV("Could not register service %s .", instance.c_str());
    }


    return 0;
}

#include <aidl/vendor/visteon/led/ILedService.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <iostream>
#include <log/log.h>

using namespace aidl::vendor::visteon::led;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();

    // Get the service
    ndk::SpAIBinder binder(AServiceManager_getService("vendor.visteon.led.ILedService/default"));
    if (!binder.get()) {
        ALOGE("Failed to get LED service");
        return 1;
    }else{
      ALOGV("LED Client instatiated....");
    }

    std::shared_ptr<ILedService> ledService = ILedService::fromBinder(binder);
    if (!ledService) {
        ALOGE("Failed to create LED service interface");
        return 1;
    }

    // Test APIs
    ALOGV("Turning RED LED ON...");
    ledService->turn_red_led_On();

    ALOGV("Turning RED LED OFF...");
    ledService->turn_red_led_Off();

    ALOGV("Turning GREEN LED ON......");
    ledService->turn_green_led_On();

    ALOGV("Turning GREEN LED ON...");
    ledService->turn_green_led_Off();

    ALOGV("All API calls completed.");
    return 0;
}

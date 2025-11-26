#include <vendor/visteon/hardware/interfaces/touch/1.0/ITouch.h>

#include <android/log.h>
#include <hidl/HidlTransportSupport.h>

#include "Touch.h"

using namespace android;
using namespace vendor::visteon;
using namespace vendor::visteon::hardware::interfaces::touch::implementation;
//using namespace vendor::visteon::hardware::interfaces::touch::V1_0::implementation::Touch;
using vendor::visteon::hardware::interfaces::touch::V1_0::ITouch;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;



int main(){
  ALOGE("Touch Service is starting");
    android::hardware::configureRpcThreadpool(1, true /* callerWillJoin */);

    android::sp<Touch>service = Touch::getInstance();
    const std::string instance = std::string() + Touch::descriptor + "/default";
    
      if(service != nullptr){
        const android::status_t status = service->registerAsService();
        if (status == ::android::OK) {
            ALOGD("Touch Service is ready.");
            android::hardware::joinRpcThreadpool();
            //struct Touch touch;
            //touch.touch_start();
        } else {
            ALOGE("Could not register Touch service (%d).",status);
        }
    } else {
	    ALOGE("Touch Service Instance not available");
    }
}



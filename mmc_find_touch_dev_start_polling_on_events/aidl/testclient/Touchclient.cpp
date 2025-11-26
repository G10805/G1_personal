#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <aidl/vendor/visteon/hardware/interfaces/touch/ITouch.h>
#include <aidl/vendor/visteon/hardware/interfaces/touch/ITouchCallback.h>
#include <log/log.h>
#include <unistd.h>
#include <iostream>

using aidl::vendor::visteon::hardware::interfaces::touch::ITouch;
using aidl::vendor::visteon::hardware::interfaces::touch::ITouchCallback;

class TouchCallback : public ITouchCallback {
public:
    ndk::ScopedAStatus onTouchReceived(int32_t in_id) override {
        ALOGI("Touch event received: ID = %d", in_id);
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus getInterfaceVersion(int32_t* _aidl_return) override {
        *_aidl_return = ITouchCallback::version;
        return ndk::ScopedAStatus::ok();
    }

    ndk::ScopedAStatus getInterfaceHash(std::string* _aidl_return) override {
        *_aidl_return = ITouchCallback::hash;
        return ndk::ScopedAStatus::ok();
    }
    
    ndk::SpAIBinder asBinder() override {
        return 0;//ITouchCallback::asBinder(); 
    }

    bool isRemote() override {
        return 0;//ITouchCallback::isRemote(); 
    }

};

int main() {
    ABinderProcess_startThreadPool();

    auto binder = ndk::SpAIBinder(AServiceManager_waitForService("vendor.visteon.hardware.interfaces.touch.ITouch/default"));
    if (!binder.get()) {
        ALOGE("Failed to get ITouch service");
        return 1;
    }

    std::shared_ptr<ITouch> touchService = ITouch::fromBinder(binder);
    if (!touchService) {
        ALOGE("Failed to cast binder to ITouch");
        return 1;
    }

    auto callback = ndk::SharedRefBase::make<TouchCallback>();
    auto status = touchService->registerCallback(callback, 123);
    if (!status.isOk()) {
        ALOGE("Failed to register callback: %s", status.getDescription().c_str());
        return 1;
    }

    ALOGI("Callback registered successfully. Waiting for touch events...");
    while (true) {
        sleep(1);
    }
    return 0;
}
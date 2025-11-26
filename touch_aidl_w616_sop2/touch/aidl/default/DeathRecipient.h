#pragma once

#include <aidl/vendor/visteon/hardware/interfaces/touch/ITouchCallback.h>
#include <android/binder_ibinder.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

using aidl::vendor::visteon::hardware::interfaces::touch::ITouchCallback;

class Touch;

class TouchDeathRecipient {
public:
    TouchDeathRecipient(std::shared_ptr<ITouchCallback> callback, Touch* touch);
    static void onBinderDied(void* cookie);

private:
    std::shared_ptr<ITouchCallback> mClient;
    Touch* mTouch;
    AIBinder_DeathRecipient* mDeathRecipient;
};

} // namespace
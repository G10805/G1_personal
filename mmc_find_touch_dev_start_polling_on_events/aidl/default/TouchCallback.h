#pragma once

#include <aidl/vendor/visteon/hardware/interfaces/touch/BnTouchCallback.h>
#include <android/binder_ibinder.h>
#include <android/binder_interface_utils.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

class TouchCallback : public aidl::vendor::visteon::hardware::interfaces::touch::BnTouchCallback {
public:
    TouchCallback() = default;
    ~TouchCallback() = default;

    ndk::ScopedAStatus onTouchReceived(int32_t displayId) override;
};

} // namespace
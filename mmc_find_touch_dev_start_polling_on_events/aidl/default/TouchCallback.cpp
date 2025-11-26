#include "TouchCallback.h"
#include <log/log.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

ndk::ScopedAStatus TouchCallback::onTouchReceived(int32_t displayId) {
    ALOGE("[%s] :: Callback received for display ID: %d", __func__, displayId);
    // Add your logic here if needed
    return ndk::ScopedAStatus::ok();
}

} // namespace

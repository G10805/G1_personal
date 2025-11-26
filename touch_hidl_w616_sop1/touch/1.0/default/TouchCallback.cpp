// FIXME: your file license if you have one

#include "TouchCallback.h"
#include <log/log.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

// Methods from ::vendor::visteon::hardware::interfaces::touch::V1_0::ITouchCallback follow.
Return<void> TouchCallback::onTouchReceived(int32_t in_id) {
    ALOGV("[%s] :: Callback recieved ", __func__);
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ITouchCallback* HIDL_FETCH_ITouchCallback(const char* /* name */) {
    //return new TouchCallback();
//}
//
}  // namespace vendor::visteon::hardware::interfaces::touch::implementation

// FIXME: your file license if you have one

#pragma once

#include <vendor/visteon/hardware/interfaces/touch/1.0/ITouchCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <Touch.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct TouchCallback : public V1_0::ITouchCallback {
    // Methods from ::vendor::visteon::hardware::interfaces::touch::V1_0::ITouchCallback follow.
    Return<void> onTouchReceived(int32_t in_id) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ITouchCallback* HIDL_FETCH_ITouchCallback(const char* name);

}  // namespace vendor::visteon::hardware::interfaces::touch::implementation

/* auto-generated source file */

#include "bluetooth_hci_status_util.h"

namespace aidl {
namespace android {
namespace hardware {
namespace bluetooth {
namespace rpc {

ndk::ScopedAStatus createBluetoothHciStatus(Status status, const string& info) {
    return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(static_cast<int32_t>(status), info.c_str());
}

ndk::ScopedAStatus createBluetoothHciStatus(Status status) {
    return ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(status));
}

}  // namespace rpc
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
}  // namespace aidl

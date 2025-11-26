/* auto-generated source file */

#pragma once

#include <aidl/android/hardware/bluetooth/IBluetoothHci.h>
#include <aidl/android/hardware/bluetooth/Status.h>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace bluetooth {
namespace rpc {

using aidl::android::hardware::bluetooth::Status;
using std::string;

ndk::ScopedAStatus createBluetoothHciStatus(Status status, const string& info);
ndk::ScopedAStatus createBluetoothHciStatus(Status status);

}  // namespace rpc
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
}  // namespace aidl

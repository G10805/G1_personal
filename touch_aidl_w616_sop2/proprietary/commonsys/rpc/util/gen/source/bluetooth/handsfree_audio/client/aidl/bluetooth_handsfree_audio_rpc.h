/* auto-generated source file */

#pragma once

#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/BnBluetoothHandsfreeAudio.h>
#include <android-base/macros.h>

#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/HandsfreeAudioParameter.h>
#include <vector>

#include "hal_rpc.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth {
namespace handsfree_audio {
namespace rpc {

using aidl::vendor::qti::hardware::bluetooth::handsfree_audio::HandsfreeAudioParameter;
using std::vector;
using std::shared_ptr;
using ndk::ScopedAStatus;

using std::pair;
using ::qti::hal::rpc::HalRpc;
using ::qti::hal::rpc::HalRpcClient;

/**
 * Root AIDL interface object used to control the BluetoothHandsfreeAudio HAL.
 */
class BluetoothHandsfreeAudioRpc : public BnBluetoothHandsfreeAudio, public HalRpcClient, public std::enable_shared_from_this<BluetoothHandsfreeAudioRpc> {
  public:
    BluetoothHandsfreeAudioRpc();
    BluetoothHandsfreeAudioRpc(const string& instance_name);
    ~BluetoothHandsfreeAudioRpc();

    // AIDL methods exposed
    ScopedAStatus start(const HandsfreeAudioParameter& data) override;
    ScopedAStatus stop() override;

    // HalRpc methods exposed
    void initSomeipContext() override;

    static shared_ptr<BnBluetoothHandsfreeAudio> create(const char* instance_name);
    static bool isRpcEnabled();

  private:
    // Corresponding worker functions for the AIDL methods
    ScopedAStatus startInternal(const HandsfreeAudioParameter& data);
    ScopedAStatus stopInternal();

    static shared_ptr<BnBluetoothHandsfreeAudio> sBluetoothHandsfreeAudioRpc;

    DISALLOW_COPY_AND_ASSIGN(BluetoothHandsfreeAudioRpc);
};

}  // namespace rpc
}  // namespace handsfree_audio
}  // namespace bluetooth
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl

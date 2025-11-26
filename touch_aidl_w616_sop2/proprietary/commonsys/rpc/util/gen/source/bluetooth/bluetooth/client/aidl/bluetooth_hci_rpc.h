/* auto-generated source file */

#pragma once

#include <aidl/android/hardware/bluetooth/BnBluetoothHci.h>
#include <android-base/macros.h>

#include <aidl/android/hardware/bluetooth/IBluetoothHciCallbacks.h>
#include <vector>

#include "hal_rpc.h"

namespace aidl {
namespace android {
namespace hardware {
namespace bluetooth {
namespace rpc {

using aidl::android::hardware::bluetooth::IBluetoothHciCallbacks;
using aidl::android::hardware::bluetooth::Status;
using std::vector;
using std::shared_ptr;
using ndk::ScopedAStatus;

using std::pair;
using ::qti::hal::rpc::HalRpc;
using ::qti::hal::rpc::HalRpcClient;

/**
 * Root AIDL interface object used to control the BluetoothHci HAL.
 */
class BluetoothHciRpc : public BnBluetoothHci, public HalRpcClient, public std::enable_shared_from_this<BluetoothHciRpc> {
  public:
    BluetoothHciRpc();
    BluetoothHciRpc(const string& instance_name);
    ~BluetoothHciRpc();

    // AIDL methods exposed
    ScopedAStatus close() override;
    ScopedAStatus initialize(const shared_ptr<IBluetoothHciCallbacks>& callback) override;
    ScopedAStatus sendAclData(const vector<uint8_t>& data) override;
    ScopedAStatus sendHciCommand(const vector<uint8_t>& command) override;
    ScopedAStatus sendIsoData(const vector<uint8_t>& data) override;
    ScopedAStatus sendScoData(const vector<uint8_t>& data) override;

    // HalRpc methods exposed
    void initSomeipContext() override;
    void handleHalInd(uint16_t message_type, uint8_t* data, size_t length) override;

    static shared_ptr<BnBluetoothHci> create(const char* instance_name);
    static bool isRpcEnabled();

  private:
    // Corresponding worker functions for the AIDL methods
    ScopedAStatus closeInternal();
    ScopedAStatus initializeInternal(const shared_ptr<IBluetoothHciCallbacks>& callback);
    ScopedAStatus sendAclDataInternal(const vector<uint8_t>& data);
    ScopedAStatus sendHciCommandInternal(const vector<uint8_t>& command);
    ScopedAStatus sendIsoDataInternal(const vector<uint8_t>& data);
    ScopedAStatus sendScoDataInternal(const vector<uint8_t>& data);

    void handleAclDataReceivedInd(uint8_t* msg_data, size_t msg_length);
    void handleHciEventReceivedInd(uint8_t* msg_data, size_t msg_length);
    void handleInitializationCompleteInd(uint8_t* msg_data, size_t msg_length);
    void handleIsoDataReceivedInd(uint8_t* msg_data, size_t msg_length);
    void handleScoDataReceivedInd(uint8_t* msg_data, size_t msg_length);

    void addCallback(const shared_ptr<IBluetoothHciCallbacks>& callback);

    shared_ptr<IBluetoothHciCallbacks> callback_;

    static shared_ptr<BnBluetoothHci> sBluetoothHciRpc;

    DISALLOW_COPY_AND_ASSIGN(BluetoothHciRpc);
};

}  // namespace rpc
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
}  // namespace aidl

/* auto-generated source file */

#include <stdlib.h>
#include <android-base/logging.h>
#include <cutils/properties.h>


#include "bluetooth_handsfree_audio_api.h"
#include "bluetooth_handsfree_audio_message_def.h"
#include "bluetooth_handsfree_audio_msg.h"
#include "bluetooth_handsfree_audio_rpc.h"
#include "bluetooth_handsfree_audio_someip_def.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth {
namespace handsfree_audio {
namespace rpc {

shared_ptr<BnBluetoothHandsfreeAudio> BluetoothHandsfreeAudioRpc::sBluetoothHandsfreeAudioRpc = nullptr;

BluetoothHandsfreeAudioRpc::BluetoothHandsfreeAudioRpc()
    : BluetoothHandsfreeAudioRpc("default") {
}

BluetoothHandsfreeAudioRpc::BluetoothHandsfreeAudioRpc(const string& instance_name)
    : HalRpcClient(instance_name, false) {
}

BluetoothHandsfreeAudioRpc::~BluetoothHandsfreeAudioRpc() {
    HalRpc::closeSomeip();
}

shared_ptr<BnBluetoothHandsfreeAudio> BluetoothHandsfreeAudioRpc::create(const char* instance_name) {
    if (!isRpcEnabled())
        return nullptr;

    if (sBluetoothHandsfreeAudioRpc == nullptr) {
        shared_ptr<BluetoothHandsfreeAudioRpc> bluetoothHandsfreeAudioRpc =
            ndk::SharedRefBase::make<BluetoothHandsfreeAudioRpc>(instance_name ? instance_name : "default");
        bluetoothHandsfreeAudioRpc->init();
        bluetoothHandsfreeAudioRpc->openSomeip();
        sBluetoothHandsfreeAudioRpc = bluetoothHandsfreeAudioRpc;
    }
    return sBluetoothHandsfreeAudioRpc;
}

bool BluetoothHandsfreeAudioRpc::isRpcEnabled() {
    char value[PROPERTY_VALUE_MAX] = { '\0' };
    property_get("persist.vendor.bluetooth_handsfree_audio.hal_mode", value, "");
    return !strcmp(value, "rpc");
}

ScopedAStatus BluetoothHandsfreeAudioRpc::start(const HandsfreeAudioParameter& data) {
    return startInternal(data);
}

ScopedAStatus BluetoothHandsfreeAudioRpc::stop() {
    return stopInternal();
}

ScopedAStatus BluetoothHandsfreeAudioRpc::startInternal(const HandsfreeAudioParameter& data) {
    uint16_t message_type = BLUETOOTH_HANDSFREE_AUDIO_START_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHandsfreeAudioSerializeStartReq(data, payload))
        return CreateScopedAStatusError();

    if (!HalRpcClient::send(message_type, payload))
        return CreateScopedAStatusError();

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHandsfreeAudioRpc::stopInternal() {
    uint16_t message_type = BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHandsfreeAudioSerializeStopReq(payload))
        return CreateScopedAStatusError();

    if (!HalRpcClient::send(message_type, payload))
        return CreateScopedAStatusError();

    return ScopedAStatus::ok();
}

void BluetoothHandsfreeAudioRpc::initSomeipContext() {
    someip_app_name_ = BLUETOOTH_HANDSFREE_AUDIO_CLIENT_APP_NAME;

    someip_context_.service_id = BLUETOOTH_HANDSFREE_AUDIO_SERVICE_ID;
    someip_context_.instance_id = HalRpc::map2SomeipInstanceId(instance_id_);
}

}  // namespace rpc
}  // namespace handsfree_audio
}  // namespace bluetooth
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl

std::shared_ptr<IBluetoothHandsfreeAudio> create_bluetooth_handsfree_audio_rpc(const char* instance_name) {
    return aidl::vendor::qti::hardware::bluetooth::handsfree_audio::rpc::BluetoothHandsfreeAudioRpc::create(instance_name);
}

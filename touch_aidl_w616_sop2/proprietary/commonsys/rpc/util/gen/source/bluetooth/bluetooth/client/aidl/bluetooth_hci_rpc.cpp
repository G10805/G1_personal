/* auto-generated source file */

#include <stdlib.h>
#include <android-base/logging.h>
#include <cutils/properties.h>

#include "bluetooth_hci_status_util.h"

#include "bluetooth_hci_api.h"
#include "bluetooth_hci_message_def.h"
#include "bluetooth_hci_msg.h"
#include "bluetooth_hci_rpc.h"
#include "bluetooth_hci_someip_def.h"

namespace aidl {
namespace android {
namespace hardware {
namespace bluetooth {
namespace rpc {

shared_ptr<BnBluetoothHci> BluetoothHciRpc::sBluetoothHciRpc = nullptr;

BluetoothHciRpc::BluetoothHciRpc()
    : BluetoothHciRpc("default") {
}

BluetoothHciRpc::BluetoothHciRpc(const string& instance_name)
    : HalRpcClient(instance_name, false) {
}

BluetoothHciRpc::~BluetoothHciRpc() {
    HalRpc::closeSomeip();
}

shared_ptr<BnBluetoothHci> BluetoothHciRpc::create(const char* instance_name) {
    if (!isRpcEnabled())
        return nullptr;

    if (sBluetoothHciRpc == nullptr) {
        shared_ptr<BluetoothHciRpc> bluetoothHciRpc =
            ndk::SharedRefBase::make<BluetoothHciRpc>(instance_name ? instance_name : "default");
        bluetoothHciRpc->init();
        bluetoothHciRpc->openSomeip();
        sBluetoothHciRpc = bluetoothHciRpc;
    }
    return sBluetoothHciRpc;
}

bool BluetoothHciRpc::isRpcEnabled() {
    char value[PROPERTY_VALUE_MAX] = { '\0' };
    property_get("persist.vendor.bluetooth.hal_mode", value, "");
    return !strcmp(value, "rpc");
}

ScopedAStatus BluetoothHciRpc::close() {
    return closeInternal();
}

ScopedAStatus BluetoothHciRpc::initialize(const shared_ptr<IBluetoothHciCallbacks>& callback) {
    return initializeInternal(callback);
}

ScopedAStatus BluetoothHciRpc::sendAclData(const vector<uint8_t>& data) {
    return sendAclDataInternal(data);
}

ScopedAStatus BluetoothHciRpc::sendHciCommand(const vector<uint8_t>& command) {
    return sendHciCommandInternal(command);
}

ScopedAStatus BluetoothHciRpc::sendIsoData(const vector<uint8_t>& data) {
    return sendIsoDataInternal(data);
}

ScopedAStatus BluetoothHciRpc::sendScoData(const vector<uint8_t>& data) {
    return sendScoDataInternal(data);
}

ScopedAStatus BluetoothHciRpc::closeInternal() {
    uint16_t message_type = BLUETOOTH_HCI_CLOSE_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHciSerializeCloseReq(payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHciRpc::initializeInternal(const shared_ptr<IBluetoothHciCallbacks>& callback) {
    uint16_t message_type = BLUETOOTH_HCI_INITIALIZE_REQ;
    vector<uint8_t> payload;
    addCallback(callback);

    if (!BluetoothHciSerializeInitializeReq(payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHciRpc::sendAclDataInternal(const vector<uint8_t>& data) {
    uint16_t message_type = BLUETOOTH_HCI_SEND_ACL_DATA_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHciSerializeSendAclDataReq(data, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHciRpc::sendHciCommandInternal(const vector<uint8_t>& command) {
    uint16_t message_type = BLUETOOTH_HCI_SEND_HCI_COMMAND_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHciSerializeSendHciCommandReq(command, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHciRpc::sendIsoDataInternal(const vector<uint8_t>& data) {
    uint16_t message_type = BLUETOOTH_HCI_SEND_ISO_DATA_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHciSerializeSendIsoDataReq(data, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

ScopedAStatus BluetoothHciRpc::sendScoDataInternal(const vector<uint8_t>& data) {
    uint16_t message_type = BLUETOOTH_HCI_SEND_SCO_DATA_REQ;
    vector<uint8_t> payload;
    if (!BluetoothHciSerializeSendScoDataReq(data, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    if (!HalRpcClient::send(message_type, payload))
        return createBluetoothHciStatus(Status::UNKNOWN);

    return ScopedAStatus::ok();
}

void BluetoothHciRpc::initSomeipContext() {
    vector<uint16_t> event_id;
    someip_app_name_ = BLUETOOTH_HCI_CLIENT_APP_NAME;

    someip_context_.service_id = BLUETOOTH_HCI_SERVICE_ID;
    someip_context_.instance_id = HalRpc::map2SomeipInstanceId(instance_id_);
    for (vector<uint16_t>::size_type index = 0; index < BLUETOOTH_HCI_IND_COUNT; index++) {
        event_id.push_back(BLUETOOTH_HCI_IND_BASE + index);
    }
    someip_context_.addEvents(BLUETOOTH_HCI_EVENTGROUP_ID, event_id);
}

void BluetoothHciRpc::handleHalInd(uint16_t message_type, uint8_t* data, size_t length) {
    switch (message_type) {
        case BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND: {
            handleAclDataReceivedInd(data, length);
            break;
        }
        case BLUETOOTH_HCI_HCI_EVENT_RECEIVED_IND: {
            handleHciEventReceivedInd(data, length);
            break;
        }
        case BLUETOOTH_HCI_INITIALIZATION_COMPLETE_IND: {
            handleInitializationCompleteInd(data, length);
            break;
        }
        case BLUETOOTH_HCI_ISO_DATA_RECEIVED_IND: {
            handleIsoDataReceivedInd(data, length);
            break;
        }
        case BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND: {
            handleScoDataReceivedInd(data, length);
            break;
        }
        default: {
            /* unknown message type */
            break;
        }
    }
}

void BluetoothHciRpc::handleAclDataReceivedInd(uint8_t* msg_data, size_t msg_length) {
    vector<uint8_t> data;

    if (!BluetoothHciParseAclDataReceivedInd(msg_data, msg_length, data))
        return;

    callback_->aclDataReceived(data);
}

void BluetoothHciRpc::handleHciEventReceivedInd(uint8_t* msg_data, size_t msg_length) {
    vector<uint8_t> event;

    if (!BluetoothHciParseHciEventReceivedInd(msg_data, msg_length, event))
        return;

    callback_->hciEventReceived(event);
}

void BluetoothHciRpc::handleInitializationCompleteInd(uint8_t* msg_data, size_t msg_length) {
    Status status;

    if (!BluetoothHciParseInitializationCompleteInd(msg_data, msg_length, status))
        return;

    callback_->initializationComplete(status);
}

void BluetoothHciRpc::handleIsoDataReceivedInd(uint8_t* msg_data, size_t msg_length) {
    vector<uint8_t> data;

    if (!BluetoothHciParseIsoDataReceivedInd(msg_data, msg_length, data))
        return;

    callback_->isoDataReceived(data);
}

void BluetoothHciRpc::handleScoDataReceivedInd(uint8_t* msg_data, size_t msg_length) {
    vector<uint8_t> data;

    if (!BluetoothHciParseScoDataReceivedInd(msg_data, msg_length, data))
        return;

    callback_->scoDataReceived(data);
}

void BluetoothHciRpc::addCallback(const shared_ptr<IBluetoothHciCallbacks>& callback) {
    callback_ = callback;
}

}  // namespace rpc
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
}  // namespace aidl

std::shared_ptr<IBluetoothHci> create_bluetooth_hci_rpc(const char* instance_name) {
    return aidl::android::hardware::bluetooth::rpc::BluetoothHciRpc::create(instance_name);
}

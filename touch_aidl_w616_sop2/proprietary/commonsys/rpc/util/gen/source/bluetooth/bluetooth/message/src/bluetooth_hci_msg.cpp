/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "bluetooth_hci_msg.h"

#include "HalStatus.pb.h"
#include "IBluetoothHci.pb.h"
#include "IBluetoothHciCallbacks.pb.h"
#include "Status.pb.h"

using AclDataReceivedInd = ::bluetooth::IBluetoothHciCallbacks_AclDataReceivedInd;
using HciEventReceivedInd = ::bluetooth::IBluetoothHciCallbacks_HciEventReceivedInd;
using InitializationCompleteInd = ::bluetooth::IBluetoothHciCallbacks_InitializationCompleteInd;
using IsoDataReceivedInd = ::bluetooth::IBluetoothHciCallbacks_IsoDataReceivedInd;
using ScoDataReceivedInd = ::bluetooth::IBluetoothHciCallbacks_ScoDataReceivedInd;
using SendAclDataReq = ::bluetooth::IBluetoothHci_SendAclDataReq;
using SendHciCommandReq = ::bluetooth::IBluetoothHci_SendHciCommandReq;
using SendIsoDataReq = ::bluetooth::IBluetoothHci_SendIsoDataReq;
using SendScoDataReq = ::bluetooth::IBluetoothHci_SendScoDataReq;

using HalStatus_P = ::bluetooth::HalStatus;
using Status_P = ::bluetooth::Status;


bool BluetoothHciSerializeCloseReq(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeCloseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeInitializeReq(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeInitializeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeSendAclDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    SendAclDataReq msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeSendAclDataCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeSendHciCommandReq(const vector<uint8_t>& command, vector<uint8_t>& payload)
{
    SendHciCommandReq msgBluetoothHci;
    string commandStr;
    Vector2String(command, commandStr);
    msgBluetoothHci.set_command(commandStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeSendHciCommandCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeSendIsoDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    SendIsoDataReq msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeSendIsoDataCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeSendScoDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    SendScoDataReq msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeSendScoDataCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHciSerializeAclDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    AclDataReceivedInd msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeHciEventReceivedInd(const vector<uint8_t>& event, vector<uint8_t>& payload)
{
    HciEventReceivedInd msgBluetoothHci;
    string eventStr;
    Vector2String(event, eventStr);
    msgBluetoothHci.set_event(eventStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeInitializationCompleteInd(Status status, vector<uint8_t>& payload)
{
    InitializationCompleteInd msgBluetoothHci;
    msgBluetoothHci.set_status((Status_P) status);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeIsoDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    IsoDataReceivedInd msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciSerializeScoDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    ScoDataReceivedInd msgBluetoothHci;
    string dataStr;
    Vector2String(data, dataStr);
    msgBluetoothHci.set_data(dataStr);
    return ProtoMessageSerialize(&msgBluetoothHci, payload);
}

bool BluetoothHciParseCloseReq(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseCloseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseInitializeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseInitializeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseSendAclDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SendAclDataReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseSendAclDataCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseSendHciCommandReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SendHciCommandReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.command(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseSendHciCommandCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseSendIsoDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SendIsoDataReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseSendIsoDataCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseSendScoDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SendScoDataReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseSendScoDataCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHciParseAclDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    AclDataReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseHciEventReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    HciEventReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.event(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseInitializationCompleteInd(const uint8_t* data, size_t length, Status& param)
{
    InitializationCompleteInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (Status) msg.status();
        return true;
    }
    return false;
}

bool BluetoothHciParseIsoDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    IsoDataReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHciParseScoDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    ScoDataReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.data(), param);
        return true;
    }
    return false;
}
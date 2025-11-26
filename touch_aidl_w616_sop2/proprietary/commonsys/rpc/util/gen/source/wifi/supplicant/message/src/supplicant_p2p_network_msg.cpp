/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_p2p_network_msg.h"

#include "HalStatus.pb.h"
#include "ISupplicantP2pNetwork.pb.h"
#include "IfaceType.pb.h"
#include "MacAddress.pb.h"

using GetBssidCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetBssidCfm;
using GetClientListCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetClientListCfm;
using GetIdCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetIdCfm;
using GetInterfaceNameCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetInterfaceNameCfm;
using GetSsidCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetSsidCfm;
using GetTypeCfm = ::wifi::supplicant::ISupplicantP2pNetwork_GetTypeCfm;
using IsCurrentCfm = ::wifi::supplicant::ISupplicantP2pNetwork_IsCurrentCfm;
using IsGroupOwnerCfm = ::wifi::supplicant::ISupplicantP2pNetwork_IsGroupOwnerCfm;
using IsPersistentCfm = ::wifi::supplicant::ISupplicantP2pNetwork_IsPersistentCfm;
using SetClientListReq = ::wifi::supplicant::ISupplicantP2pNetwork_SetClientListReq;

using HalStatus_P = ::wifi::supplicant::HalStatus;
using IfaceType_P = ::wifi::supplicant::it::IfaceType;
using MacAddress_P = ::wifi::supplicant::MacAddress;


bool SupplicantP2pNetworkSerializeGetBssidReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantP2pNetwork)
{
    msgSupplicantP2pNetwork.set_status(status.status);
    msgSupplicantP2pNetwork.set_info(status.info);
}

bool SupplicantP2pNetworkSerializeGetBssidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetBssidCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeGetClientListReq(vector<uint8_t>& payload)
{
    return true;
}

static void MacAddress2Message(const MacAddress& result, MacAddress_P& msgSupplicantP2pNetwork)
{
    string data;
    Vector2String(result.data, data);
    msgSupplicantP2pNetwork.set_data(data);
}

bool SupplicantP2pNetworkSerializeGetClientListCfm(const HalStatusParam& status, const vector<MacAddress>& result, vector<uint8_t>& payload)
{
    GetClientListCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            MacAddress_P *result_p = msgSupplicantP2pNetwork.add_result();
            MacAddress2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeGetIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetIdCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeGetInterfaceNameReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeGetInterfaceNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetInterfaceNameCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeGetSsidReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetSsidCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeGetTypeReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload)
{
    GetTypeCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result((IfaceType_P) result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeIsCurrentReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeIsCurrentCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsCurrentCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeIsGroupOwnerReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeIsGroupOwnerCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsGroupOwnerCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeIsPersistentReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkSerializeIsPersistentCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsPersistentCfm msgSupplicantP2pNetwork;
    HalStatus_P* status_p = msgSupplicantP2pNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeSetClientListReq(const vector<MacAddress>& clients, vector<uint8_t>& payload)
{
    SetClientListReq msgSupplicantP2pNetwork;
    int size_clients = clients.size();
    if (0 < size_clients)
    {
        for (int index = 0; index < size_clients; index++)
        {
            MacAddress_P *clients_p = msgSupplicantP2pNetwork.add_clients();
            MacAddress2Message(clients[index], *clients_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantP2pNetwork, payload);
}

bool SupplicantP2pNetworkSerializeSetClientListCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pNetworkParseGetBssidReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantP2pNetwork, HalStatusParam& status)
{
    status.status = msgSupplicantP2pNetwork.status();
    status.info = msgSupplicantP2pNetwork.info();
}

bool SupplicantP2pNetworkParseGetBssidCfm(const uint8_t* data, size_t length, GetBssidCfmP2pNetworkParam& param)
{
    GetBssidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseGetClientListReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2MacAddress(const MacAddress_P& msgSupplicantP2pNetwork, MacAddress& result)
{
    String2Vector(msgSupplicantP2pNetwork.data(), result.data);
}

bool SupplicantP2pNetworkParseGetClientListCfm(const uint8_t* data, size_t length, GetClientListCfmP2pNetworkParam& param)
{
    GetClientListCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2MacAddress(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseGetIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmP2pNetworkParam& param)
{
    GetIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseGetInterfaceNameReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseGetInterfaceNameCfm(const uint8_t* data, size_t length, GetInterfaceNameCfmP2pNetworkParam& param)
{
    GetInterfaceNameCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseGetSsidReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmP2pNetworkParam& param)
{
    GetSsidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseGetTypeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmP2pNetworkParam& param)
{
    GetTypeCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (IfaceType) msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseIsCurrentReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseIsCurrentCfm(const uint8_t* data, size_t length, IsCurrentCfmP2pNetworkParam& param)
{
    IsCurrentCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseIsGroupOwnerReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseIsGroupOwnerCfm(const uint8_t* data, size_t length, IsGroupOwnerCfmP2pNetworkParam& param)
{
    IsGroupOwnerCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseIsPersistentReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pNetworkParseIsPersistentCfm(const uint8_t* data, size_t length, IsPersistentCfmP2pNetworkParam& param)
{
    IsPersistentCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseSetClientListReq(const uint8_t* data, size_t length, vector<MacAddress>& param)
{
    SetClientListReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_clients = msg.clients_size();
        if (0 < size_clients)
        {
            param.resize(size_clients);
            for (int index = 0; index < size_clients; index++)
            {
                Message2MacAddress(msg.clients(index), param[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantP2pNetworkParseSetClientListCfm(const uint8_t* data, size_t length)
{
    return true;
}
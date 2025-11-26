/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_msg.h"

#include "DebugLevel.pb.h"
#include "HalStatus.pb.h"
#include "ISupplicant.pb.h"
#include "ISupplicantCallback.pb.h"
#include "ISupplicantP2pIface.pb.h"
#include "ISupplicantStaIface.pb.h"
#include "IfaceInfo.pb.h"
#include "IfaceType.pb.h"

using AddP2pInterfaceCfm = ::wifi::supplicant::ISupplicant_AddP2pInterfaceCfm;
using AddP2pInterfaceReq = ::wifi::supplicant::ISupplicant_AddP2pInterfaceReq;
using AddStaInterfaceCfm = ::wifi::supplicant::ISupplicant_AddStaInterfaceCfm;
using AddStaInterfaceReq = ::wifi::supplicant::ISupplicant_AddStaInterfaceReq;
using GetDebugLevelCfm = ::wifi::supplicant::ISupplicant_GetDebugLevelCfm;
using GetP2pInterfaceCfm = ::wifi::supplicant::ISupplicant_GetP2pInterfaceCfm;
using GetP2pInterfaceReq = ::wifi::supplicant::ISupplicant_GetP2pInterfaceReq;
using GetStaInterfaceCfm = ::wifi::supplicant::ISupplicant_GetStaInterfaceCfm;
using GetStaInterfaceReq = ::wifi::supplicant::ISupplicant_GetStaInterfaceReq;
using IsDebugShowKeysEnabledCfm = ::wifi::supplicant::ISupplicant_IsDebugShowKeysEnabledCfm;
using IsDebugShowTimestampEnabledCfm = ::wifi::supplicant::ISupplicant_IsDebugShowTimestampEnabledCfm;
using ListInterfacesCfm = ::wifi::supplicant::ISupplicant_ListInterfacesCfm;
using OnInterfaceCreatedInd = ::wifi::supplicant::ISupplicantCallback_OnInterfaceCreatedInd;
using OnInterfaceRemovedInd = ::wifi::supplicant::ISupplicantCallback_OnInterfaceRemovedInd;
using RemoveInterfaceReq = ::wifi::supplicant::ISupplicant_RemoveInterfaceReq;
using SetConcurrencyPriorityReq = ::wifi::supplicant::ISupplicant_SetConcurrencyPriorityReq;
using SetDebugParamsReq = ::wifi::supplicant::ISupplicant_SetDebugParamsReq;

using DebugLevel_P = ::wifi::supplicant::DebugLevel;
using HalStatus_P = ::wifi::supplicant::HalStatus;
using ISupplicantP2pIface_P = ::wifi::supplicant::ISupplicantP2pIface;
using ISupplicantStaIface_P = ::wifi::supplicant::ISupplicantStaIface;
using IfaceInfo_P = ::wifi::supplicant::IfaceInfo;
using IfaceType_P = ::wifi::supplicant::it::IfaceType;


bool SupplicantSerializeAddP2pInterfaceReq(const string& ifName, vector<uint8_t>& payload)
{
    AddP2pInterfaceReq msgSupplicant;
    msgSupplicant.set_ifname(ifName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicant)
{
    msgSupplicant.set_status(status.status);
    msgSupplicant.set_info(status.info);
}

static void ISupplicantP2pIface2Message(int32_t result, ISupplicantP2pIface_P& msgSupplicant)
{
    msgSupplicant.set_instanceid(result);
}

bool SupplicantSerializeAddP2pInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddP2pInterfaceCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantP2pIface_P* result_p = msgSupplicant.mutable_result();
    ISupplicantP2pIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeAddStaInterfaceReq(const string& ifName, vector<uint8_t>& payload)
{
    AddStaInterfaceReq msgSupplicant;
    msgSupplicant.set_ifname(ifName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

static void ISupplicantStaIface2Message(int32_t result, ISupplicantStaIface_P& msgSupplicant)
{
    msgSupplicant.set_instanceid(result);
}

bool SupplicantSerializeAddStaInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddStaInterfaceCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantStaIface_P* result_p = msgSupplicant.mutable_result();
    ISupplicantStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeGetDebugLevelReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeGetDebugLevelCfm(const HalStatusParam& status, DebugLevel result, vector<uint8_t>& payload)
{
    GetDebugLevelCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicant.set_result((DebugLevel_P) result);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeGetP2pInterfaceReq(const string& ifName, vector<uint8_t>& payload)
{
    GetP2pInterfaceReq msgSupplicant;
    msgSupplicant.set_ifname(ifName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeGetP2pInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetP2pInterfaceCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantP2pIface_P* result_p = msgSupplicant.mutable_result();
    ISupplicantP2pIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeGetStaInterfaceReq(const string& ifName, vector<uint8_t>& payload)
{
    GetStaInterfaceReq msgSupplicant;
    msgSupplicant.set_ifname(ifName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeGetStaInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetStaInterfaceCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantStaIface_P* result_p = msgSupplicant.mutable_result();
    ISupplicantStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeIsDebugShowKeysEnabledReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeIsDebugShowKeysEnabledCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsDebugShowKeysEnabledCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicant.set_result(result);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeIsDebugShowTimestampEnabledReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeIsDebugShowTimestampEnabledCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsDebugShowTimestampEnabledCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicant.set_result(result);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeListInterfacesReq(vector<uint8_t>& payload)
{
    return true;
}

static void IfaceInfo2Message(const IfaceInfo& result, IfaceInfo_P& msgSupplicant)
{
    msgSupplicant.set_type((IfaceType_P) result.type);
    msgSupplicant.set_name(result.name);
}

bool SupplicantSerializeListInterfacesCfm(const HalStatusParam& status, const vector<IfaceInfo>& result, vector<uint8_t>& payload)
{
    ListInterfacesCfm msgSupplicant;
    HalStatus_P* status_p = msgSupplicant.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            IfaceInfo_P *result_p = msgSupplicant.add_result();
            IfaceInfo2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeRegisterCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeRegisterCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeRemoveInterfaceReq(const IfaceInfo& ifaceInfo, vector<uint8_t>& payload)
{
    RemoveInterfaceReq msgSupplicant;
    IfaceInfo_P* ifaceInfo_p = msgSupplicant.mutable_ifaceinfo();
    IfaceInfo2Message(ifaceInfo, *ifaceInfo_p);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeRemoveInterfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeSetConcurrencyPriorityReq(IfaceType type, vector<uint8_t>& payload)
{
    SetConcurrencyPriorityReq msgSupplicant;
    msgSupplicant.set_type((IfaceType_P) type);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeSetConcurrencyPriorityCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeSetDebugParamsReq(DebugLevel level, bool showTimestamp, bool showKeys, vector<uint8_t>& payload)
{
    SetDebugParamsReq msgSupplicant;
    msgSupplicant.set_level((DebugLevel_P) level);
    msgSupplicant.set_showtimestamp(showTimestamp);
    msgSupplicant.set_showkeys(showKeys);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeSetDebugParamsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeTerminateReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeTerminateCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeRegisterNonStandardCertCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeRegisterNonStandardCertCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantSerializeOnInterfaceCreatedInd(const string& ifaceName, vector<uint8_t>& payload)
{
    OnInterfaceCreatedInd msgSupplicant;
    msgSupplicant.set_ifacename(ifaceName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantSerializeOnInterfaceRemovedInd(const string& ifaceName, vector<uint8_t>& payload)
{
    OnInterfaceRemovedInd msgSupplicant;
    msgSupplicant.set_ifacename(ifaceName);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantParseAddP2pInterfaceReq(const uint8_t* data, size_t length, string& param)
{
    AddP2pInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicant, HalStatusParam& status)
{
    status.status = msgSupplicant.status();
    status.info = msgSupplicant.info();
}

static void Message2ISupplicantP2pIface(const ISupplicantP2pIface_P& msgSupplicant, int32_t& result)
{
    result = msgSupplicant.instanceid();
}

bool SupplicantParseAddP2pInterfaceCfm(const uint8_t* data, size_t length, AddP2pInterfaceCfmParam& param)
{
    AddP2pInterfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantP2pIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantParseAddStaInterfaceReq(const uint8_t* data, size_t length, string& param)
{
    AddStaInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

static void Message2ISupplicantStaIface(const ISupplicantStaIface_P& msgSupplicant, int32_t& result)
{
    result = msgSupplicant.instanceid();
}

bool SupplicantParseAddStaInterfaceCfm(const uint8_t* data, size_t length, AddStaInterfaceCfmParam& param)
{
    AddStaInterfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantParseGetDebugLevelReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseGetDebugLevelCfm(const uint8_t* data, size_t length, GetDebugLevelCfmParam& param)
{
    GetDebugLevelCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (DebugLevel) msg.result();
        return true;
    }
    return false;
}

bool SupplicantParseGetP2pInterfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetP2pInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool SupplicantParseGetP2pInterfaceCfm(const uint8_t* data, size_t length, GetP2pInterfaceCfmParam& param)
{
    GetP2pInterfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantP2pIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantParseGetStaInterfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetStaInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool SupplicantParseGetStaInterfaceCfm(const uint8_t* data, size_t length, GetStaInterfaceCfmParam& param)
{
    GetStaInterfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantParseIsDebugShowKeysEnabledReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseIsDebugShowKeysEnabledCfm(const uint8_t* data, size_t length, IsDebugShowKeysEnabledCfmParam& param)
{
    IsDebugShowKeysEnabledCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantParseIsDebugShowTimestampEnabledReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseIsDebugShowTimestampEnabledCfm(const uint8_t* data, size_t length, IsDebugShowTimestampEnabledCfmParam& param)
{
    IsDebugShowTimestampEnabledCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantParseListInterfacesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2IfaceInfo(const IfaceInfo_P& msgSupplicant, IfaceInfo& result)
{
    result.type = (IfaceType) msgSupplicant.type();
    result.name = msgSupplicant.name();
}

bool SupplicantParseListInterfacesCfm(const uint8_t* data, size_t length, ListInterfacesCfmParam& param)
{
    ListInterfacesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2IfaceInfo(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantParseRegisterCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseRegisterCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseRemoveInterfaceReq(const uint8_t* data, size_t length, IfaceInfo& param)
{
    RemoveInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2IfaceInfo(msg.ifaceinfo(), param);
        return true;
    }
    return false;
}

bool SupplicantParseRemoveInterfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseSetConcurrencyPriorityReq(const uint8_t* data, size_t length, IfaceType& param)
{
    SetConcurrencyPriorityReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (IfaceType) msg.type();
        return true;
    }
    return false;
}

bool SupplicantParseSetConcurrencyPriorityCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseSetDebugParamsReq(const uint8_t* data, size_t length, SetDebugParamsReqParam& param)
{
    SetDebugParamsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.level = (DebugLevel) msg.level();
        param.showTimestamp = msg.showtimestamp();
        param.showKeys = msg.showkeys();
        return true;
    }
    return false;
}

bool SupplicantParseSetDebugParamsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseTerminateReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseTerminateCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseRegisterNonStandardCertCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseRegisterNonStandardCertCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantParseOnInterfaceCreatedInd(const uint8_t* data, size_t length, string& param)
{
    OnInterfaceCreatedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifacename();
        return true;
    }
    return false;
}

bool SupplicantParseOnInterfaceRemovedInd(const uint8_t* data, size_t length, string& param)
{
    OnInterfaceRemovedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifacename();
        return true;
    }
    return false;
}
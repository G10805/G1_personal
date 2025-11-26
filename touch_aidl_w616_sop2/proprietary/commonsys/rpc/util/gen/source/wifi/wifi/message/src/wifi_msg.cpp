/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_msg.h"

#include "HalStatus.pb.h"
#include "IWifi.pb.h"
#include "IWifiChip.pb.h"
#include "IWifiEventCallback.pb.h"
#include "WifiStatusCode.pb.h"

using GetChipCfm = ::wifi::IWifi_GetChipCfm;
using GetChipIdsCfm = ::wifi::IWifi_GetChipIdsCfm;
using GetChipReq = ::wifi::IWifi_GetChipReq;
using IsStartedCfm = ::wifi::IWifi_IsStartedCfm;
using OnFailureInd = ::wifi::IWifiEventCallback_OnFailureInd;
using OnSubsystemRestartInd = ::wifi::IWifiEventCallback_OnSubsystemRestartInd;

using HalStatus_P = ::wifi::HalStatus;
using IWifiChip_P = ::wifi::IWifiChip;
using WifiStatusCode_P = ::wifi::wsc::WifiStatusCode;


bool WifiSerializeGetChipReq(int32_t chipId, vector<uint8_t>& payload)
{
    GetChipReq msgWifi;
    msgWifi.set_chipid(chipId);
    return ProtoMessageSerialize(&msgWifi, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifi)
{
    msgWifi.set_status(status.status);
    msgWifi.set_info(status.info);
}

static void IWifiChip2Message(int32_t result, IWifiChip_P& msgWifi)
{
    msgWifi.set_instanceid(result);
}

bool WifiSerializeGetChipCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetChipCfm msgWifi;
    HalStatus_P* status_p = msgWifi.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiChip_P* result_p = msgWifi.mutable_result();
    IWifiChip2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiSerializeGetChipIdsReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeGetChipIdsCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload)
{
    GetChipIdsCfm msgWifi;
    HalStatus_P* status_p = msgWifi.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifi.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiSerializeIsStartedReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeIsStartedCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    IsStartedCfm msgWifi;
    HalStatus_P* status_p = msgWifi.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifi.set_result(result);
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiSerializeRegisterEventCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeStartReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeStartCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeStopReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeStopCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeOnFailureInd(WifiStatusCode status, vector<uint8_t>& payload)
{
    OnFailureInd msgWifi;
    msgWifi.set_status((WifiStatusCode_P) status);
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiSerializeOnStartInd(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeOnStopInd(vector<uint8_t>& payload)
{
    return true;
}

bool WifiSerializeOnSubsystemRestartInd(WifiStatusCode status, vector<uint8_t>& payload)
{
    OnSubsystemRestartInd msgWifi;
    msgWifi.set_status((WifiStatusCode_P) status);
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiParseGetChipReq(const uint8_t* data, size_t length, int32_t& param)
{
    GetChipReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.chipid();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgWifi, HalStatusParam& status)
{
    status.status = msgWifi.status();
    status.info = msgWifi.info();
}

static void Message2IWifiChip(const IWifiChip_P& msgWifi, int32_t& result)
{
    result = msgWifi.instanceid();
}

bool WifiParseGetChipCfm(const uint8_t* data, size_t length, GetChipCfmParam& param)
{
    GetChipCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiChip(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiParseGetChipIdsReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseGetChipIdsCfm(const uint8_t* data, size_t length, GetChipIdsCfmParam& param)
{
    GetChipIdsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool WifiParseIsStartedReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseIsStartedCfm(const uint8_t* data, size_t length, IsStartedCfmParam& param)
{
    IsStartedCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool WifiParseRegisterEventCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseRegisterEventCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseStartReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseStartCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseStopReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseStopCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseOnFailureInd(const uint8_t* data, size_t length, WifiStatusCode& param)
{
    OnFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (WifiStatusCode) msg.status();
        return true;
    }
    return false;
}

bool WifiParseOnStartInd(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseOnStopInd(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiParseOnSubsystemRestartInd(const uint8_t* data, size_t length, WifiStatusCode& param)
{
    OnSubsystemRestartInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (WifiStatusCode) msg.status();
        return true;
    }
    return false;
}
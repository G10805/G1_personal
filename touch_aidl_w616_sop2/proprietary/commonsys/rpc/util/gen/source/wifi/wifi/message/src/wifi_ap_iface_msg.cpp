/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_ap_iface_msg.h"

#include "HalStatus.pb.h"
#include "IWifiApIface.pb.h"

using GetBridgedInstancesCfm = ::wifi::IWifiApIface_GetBridgedInstancesCfm;
using GetFactoryMacAddressCfm = ::wifi::IWifiApIface_GetFactoryMacAddressCfm;
using GetNameCfm = ::wifi::IWifiApIface_GetNameCfm;
using SetCountryCodeReq = ::wifi::IWifiApIface_SetCountryCodeReq;
using SetMacAddressReq = ::wifi::IWifiApIface_SetMacAddressReq;

using HalStatus_P = ::wifi::HalStatus;


bool WifiApIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiApIface)
{
    msgWifiApIface.set_status(status.status);
    msgWifiApIface.set_info(status.info);
}

bool WifiApIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgWifiApIface;
    HalStatus_P* status_p = msgWifiApIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiApIface.set_result(result);
    return ProtoMessageSerialize(&msgWifiApIface, payload);
}

bool WifiApIfaceSerializeGetBridgedInstancesReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceSerializeGetBridgedInstancesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    GetBridgedInstancesCfm msgWifiApIface;
    HalStatus_P* status_p = msgWifiApIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifiApIface.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifiApIface, payload);
}

bool WifiApIfaceSerializeGetFactoryMacAddressReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceSerializeGetFactoryMacAddressCfm(const HalStatusParam& status, const array<uint8_t, 6>& result, vector<uint8_t>& payload)
{
    GetFactoryMacAddressCfm msgWifiApIface;
    HalStatus_P* status_p = msgWifiApIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Array2String(result, resultStr);
    msgWifiApIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgWifiApIface, payload);
}

bool WifiApIfaceSerializeSetCountryCodeReq(const array<uint8_t, 2>& code, vector<uint8_t>& payload)
{
    SetCountryCodeReq msgWifiApIface;
    string codeStr;
    Array2String(code, codeStr);
    msgWifiApIface.set_code(codeStr);
    return ProtoMessageSerialize(&msgWifiApIface, payload);
}

bool WifiApIfaceSerializeSetCountryCodeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceSerializeResetToFactoryMacAddressReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceSerializeResetToFactoryMacAddressCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceSerializeSetMacAddressReq(const array<uint8_t, 6>& mac, vector<uint8_t>& payload)
{
    SetMacAddressReq msgWifiApIface;
    string macStr;
    Array2String(mac, macStr);
    msgWifiApIface.set_mac(macStr);
    return ProtoMessageSerialize(&msgWifiApIface, payload);
}

bool WifiApIfaceSerializeSetMacAddressCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiApIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgWifiApIface, HalStatusParam& status)
{
    status.status = msgWifiApIface.status();
    status.info = msgWifiApIface.info();
}

bool WifiApIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmApIfaceParam& param)
{
    GetNameCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool WifiApIfaceParseGetBridgedInstancesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiApIfaceParseGetBridgedInstancesCfm(const uint8_t* data, size_t length, GetBridgedInstancesCfmApIfaceParam& param)
{
    GetBridgedInstancesCfm msg;
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

bool WifiApIfaceParseGetFactoryMacAddressReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiApIfaceParseGetFactoryMacAddressCfm(const uint8_t* data, size_t length, GetFactoryMacAddressCfmApIfaceParam& param)
{
    GetFactoryMacAddressCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Array(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiApIfaceParseSetCountryCodeReq(const uint8_t* data, size_t length, array<uint8_t, 2>& param)
{
    SetCountryCodeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Array(msg.code(), param);
        return true;
    }
    return false;
}

bool WifiApIfaceParseSetCountryCodeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiApIfaceParseResetToFactoryMacAddressReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiApIfaceParseResetToFactoryMacAddressCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiApIfaceParseSetMacAddressReq(const uint8_t* data, size_t length, array<uint8_t, 6>& param)
{
    SetMacAddressReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Array(msg.mac(), param);
        return true;
    }
    return false;
}

bool WifiApIfaceParseSetMacAddressCfm(const uint8_t* data, size_t length)
{
    return true;
}
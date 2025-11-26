/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "qti_wifi_msg.h"

#include "HalStatus.pb.h"
#include "IQtiWifi.pb.h"
#include "IQtiWifiCallback.pb.h"
#include "IfaceInfo.pb.h"
#include "IfaceType.pb.h"

using DoQtiWifiCmdCfm = ::qti::wifi::IQtiWifi_DoQtiWifiCmdCfm;
using DoQtiWifiCmdReq = ::qti::wifi::IQtiWifi_DoQtiWifiCmdReq;
using ListAvailableInterfacesCfm = ::qti::wifi::IQtiWifi_ListAvailableInterfacesCfm;
using OnCtrlEventInd = ::qti::wifi::IQtiWifiCallback_OnCtrlEventInd;

using HalStatus_P = ::qti::wifi::HalStatus;
using IfaceInfo_P = ::qti::wifi::IfaceInfo;
using IfaceType_P = ::qti::wifi::IfaceType;


bool QtiWifiSerializeListAvailableInterfacesReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgQtiWifi)
{
    msgQtiWifi.set_status(status.status);
    msgQtiWifi.set_info(status.info);
}

static void IfaceInfo2Message(const IfaceInfo& result, IfaceInfo_P& msgQtiWifi)
{
    msgQtiWifi.set_type((IfaceType_P) result.type);
    msgQtiWifi.set_name(result.name);
}

bool QtiWifiSerializeListAvailableInterfacesCfm(const HalStatusParam& status, const vector<IfaceInfo>& result, vector<uint8_t>& payload)
{
    ListAvailableInterfacesCfm msgQtiWifi;
    HalStatus_P* status_p = msgQtiWifi.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            IfaceInfo_P *result_p = msgQtiWifi.add_result();
            IfaceInfo2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgQtiWifi, payload);
}

bool QtiWifiSerializeRegisterQtiWifiCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool QtiWifiSerializeRegisterQtiWifiCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool QtiWifiSerializeDoQtiWifiCmdReq(const string& iface, const string& cmd, vector<uint8_t>& payload)
{
    DoQtiWifiCmdReq msgQtiWifi;
    msgQtiWifi.set_iface(iface);
    msgQtiWifi.set_cmd(cmd);
    return ProtoMessageSerialize(&msgQtiWifi, payload);
}

bool QtiWifiSerializeDoQtiWifiCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    DoQtiWifiCmdCfm msgQtiWifi;
    HalStatus_P* status_p = msgQtiWifi.mutable_status();
    HalStatus2Message(status, *status_p);
    msgQtiWifi.set_result(result);
    return ProtoMessageSerialize(&msgQtiWifi, payload);
}

bool QtiWifiSerializeOnCtrlEventInd(const string& iface, const string& event, vector<uint8_t>& payload)
{
    OnCtrlEventInd msgQtiWifi;
    msgQtiWifi.set_iface(iface);
    msgQtiWifi.set_event(event);
    return ProtoMessageSerialize(&msgQtiWifi, payload);
}

bool QtiWifiParseListAvailableInterfacesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgQtiWifi, HalStatusParam& status)
{
    status.status = msgQtiWifi.status();
    status.info = msgQtiWifi.info();
}

static void Message2IfaceInfo(const IfaceInfo_P& msgQtiWifi, IfaceInfo& result)
{
    result.type = (IfaceType) msgQtiWifi.type();
    result.name = msgQtiWifi.name();
}

bool QtiWifiParseListAvailableInterfacesCfm(const uint8_t* data, size_t length, ListAvailableInterfacesCfmParam& param)
{
    ListAvailableInterfacesCfm msg;
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

bool QtiWifiParseRegisterQtiWifiCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool QtiWifiParseRegisterQtiWifiCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool QtiWifiParseDoQtiWifiCmdReq(const uint8_t* data, size_t length, DoQtiWifiCmdReqParam& param)
{
    DoQtiWifiCmdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.iface = msg.iface();
        param.cmd = msg.cmd();
        return true;
    }
    return false;
}

bool QtiWifiParseDoQtiWifiCmdCfm(const uint8_t* data, size_t length, DoQtiWifiCmdCfmParam& param)
{
    DoQtiWifiCmdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool QtiWifiParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param)
{
    OnCtrlEventInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.iface = msg.iface();
        param.event = msg.event();
        return true;
    }
    return false;
}
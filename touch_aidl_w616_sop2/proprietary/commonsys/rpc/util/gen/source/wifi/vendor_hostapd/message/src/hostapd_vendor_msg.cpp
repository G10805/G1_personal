/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "hostapd_vendor_msg.h"

#include "HalStatus.pb.h"
#include "IHostapdVendor.pb.h"
#include "IHostapdVendorCallback.pb.h"
#include "VendorApInfo.pb.h"

using DoDriverCmdCfm = ::hostapd::vendor::IHostapdVendor_DoDriverCmdCfm;
using DoDriverCmdReq = ::hostapd::vendor::IHostapdVendor_DoDriverCmdReq;
using ListVendorInterfacesCfm = ::hostapd::vendor::IHostapdVendor_ListVendorInterfacesCfm;
using OnApInstanceInfoChangedInd = ::hostapd::vendor::IHostapdVendorCallback_OnApInstanceInfoChangedInd;
using OnCtrlEventInd = ::hostapd::vendor::IHostapdVendorCallback_OnCtrlEventInd;
using OnFailureInd = ::hostapd::vendor::IHostapdVendorCallback_OnFailureInd;

using HalStatus_P = ::hostapd::vendor::HalStatus;
using VendorApInfo_P = ::hostapd::vendor::VendorApInfo;


bool HostapdVendorSerializeListVendorInterfacesReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgHostapdVendor)
{
    msgHostapdVendor.set_status(status.status);
    msgHostapdVendor.set_info(status.info);
}

bool HostapdVendorSerializeListVendorInterfacesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    ListVendorInterfacesCfm msgHostapdVendor;
    HalStatus_P* status_p = msgHostapdVendor.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgHostapdVendor.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

bool HostapdVendorSerializeRegisterHostapdVendorCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdVendorSerializeRegisterHostapdVendorCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdVendorSerializeDoDriverCmdReq(const string& iface, const string& cmd, vector<uint8_t>& payload)
{
    DoDriverCmdReq msgHostapdVendor;
    msgHostapdVendor.set_iface(iface);
    msgHostapdVendor.set_cmd(cmd);
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

bool HostapdVendorSerializeDoDriverCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    DoDriverCmdCfm msgHostapdVendor;
    HalStatus_P* status_p = msgHostapdVendor.mutable_status();
    HalStatus2Message(status, *status_p);
    msgHostapdVendor.set_result(result);
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

bool HostapdVendorSerializeOnCtrlEventInd(const string& ifaceName, const string& event_str, vector<uint8_t>& payload)
{
    OnCtrlEventInd msgHostapdVendor;
    msgHostapdVendor.set_ifacename(ifaceName);
    msgHostapdVendor.set_event_str(event_str);
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

static void VendorApInfo2Message(const VendorApInfo& apInfo, VendorApInfo_P& msgHostapdVendor)
{
    msgHostapdVendor.set_ifacename(apInfo.ifaceName);
    msgHostapdVendor.set_apifaceinstance(apInfo.apIfaceInstance);
}

bool HostapdVendorSerializeOnApInstanceInfoChangedInd(const VendorApInfo& apInfo, vector<uint8_t>& payload)
{
    OnApInstanceInfoChangedInd msgHostapdVendor;
    VendorApInfo_P* apInfo_p = msgHostapdVendor.mutable_apinfo();
    VendorApInfo2Message(apInfo, *apInfo_p);
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

bool HostapdVendorSerializeOnFailureInd(const string& ifname, const string& instanceName, vector<uint8_t>& payload)
{
    OnFailureInd msgHostapdVendor;
    msgHostapdVendor.set_ifname(ifname);
    msgHostapdVendor.set_instancename(instanceName);
    return ProtoMessageSerialize(&msgHostapdVendor, payload);
}

bool HostapdVendorParseListVendorInterfacesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgHostapdVendor, HalStatusParam& status)
{
    status.status = msgHostapdVendor.status();
    status.info = msgHostapdVendor.info();
}

bool HostapdVendorParseListVendorInterfacesCfm(const uint8_t* data, size_t length, ListVendorInterfacesCfmParam& param)
{
    ListVendorInterfacesCfm msg;
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

bool HostapdVendorParseRegisterHostapdVendorCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdVendorParseRegisterHostapdVendorCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdVendorParseDoDriverCmdReq(const uint8_t* data, size_t length, DoDriverCmdReqParam& param)
{
    DoDriverCmdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.iface = msg.iface();
        param.cmd = msg.cmd();
        return true;
    }
    return false;
}

bool HostapdVendorParseDoDriverCmdCfm(const uint8_t* data, size_t length, DoDriverCmdCfmParam& param)
{
    DoDriverCmdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool HostapdVendorParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param)
{
    OnCtrlEventInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ifaceName = msg.ifacename();
        param.event_str = msg.event_str();
        return true;
    }
    return false;
}

static void Message2VendorApInfo(const VendorApInfo_P& msgHostapdVendor, VendorApInfo& apInfo)
{
    apInfo.ifaceName = msgHostapdVendor.ifacename();
    apInfo.apIfaceInstance = msgHostapdVendor.apifaceinstance();
}

bool HostapdVendorParseOnApInstanceInfoChangedInd(const uint8_t* data, size_t length, VendorApInfo& param)
{
    OnApInstanceInfoChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2VendorApInfo(msg.apinfo(), param);
        return true;
    }
    return false;
}

bool HostapdVendorParseOnFailureInd(const uint8_t* data, size_t length, OnFailureIndParam& param)
{
    OnFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ifname = msg.ifname();
        param.instanceName = msg.instancename();
        return true;
    }
    return false;
}
/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_vendor_sta_iface_msg.h"

#include "HalStatus.pb.h"
#include "ISupplicantVendorStaIface.pb.h"
#include "ISupplicantVendorStaIfaceCallback.pb.h"

using DoDriverCmdCfm = ::supplicant::vendor::ISupplicantVendorStaIface_DoDriverCmdCfm;
using DoDriverCmdReq = ::supplicant::vendor::ISupplicantVendorStaIface_DoDriverCmdReq;
using OnCtrlEventInd = ::supplicant::vendor::ISupplicantVendorStaIfaceCallback_OnCtrlEventInd;

using HalStatus_P = ::supplicant::vendor::HalStatus;


bool SupplicantVendorStaIfaceSerializeDoDriverCmdReq(const string& command, vector<uint8_t>& payload)
{
    DoDriverCmdReq msgSupplicantVendorStaIface;
    msgSupplicantVendorStaIface.set_command(command);
    return ProtoMessageSerialize(&msgSupplicantVendorStaIface, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantVendorStaIface)
{
    msgSupplicantVendorStaIface.set_status(status.status);
    msgSupplicantVendorStaIface.set_info(status.info);
}

bool SupplicantVendorStaIfaceSerializeDoDriverCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    DoDriverCmdCfm msgSupplicantVendorStaIface;
    HalStatus_P* status_p = msgSupplicantVendorStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantVendorStaIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantVendorStaIface, payload);
}

bool SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantVendorStaIfaceSerializeOnCtrlEventInd(const string& ifaceName, const string& eventStr, vector<uint8_t>& payload)
{
    OnCtrlEventInd msgSupplicantVendorStaIface;
    msgSupplicantVendorStaIface.set_ifacename(ifaceName);
    msgSupplicantVendorStaIface.set_eventstr(eventStr);
    return ProtoMessageSerialize(&msgSupplicantVendorStaIface, payload);
}

bool SupplicantVendorStaIfaceParseDoDriverCmdReq(const uint8_t* data, size_t length, string& param)
{
    DoDriverCmdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.command();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantVendorStaIface, HalStatusParam& status)
{
    status.status = msgSupplicantVendorStaIface.status();
    status.info = msgSupplicantVendorStaIface.info();
}

bool SupplicantVendorStaIfaceParseDoDriverCmdCfm(const uint8_t* data, size_t length, DoDriverCmdCfmParam& param)
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

bool SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantVendorStaIfaceParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param)
{
    OnCtrlEventInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ifaceName = msg.ifacename();
        param.eventStr = msg.eventstr();
        return true;
    }
    return false;
}
/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_vendor_msg.h"

#include "HalStatus.pb.h"
#include "ISupplicantVendor.pb.h"
#include "ISupplicantVendorStaIface.pb.h"
#include "IVendorIfaceInfo.pb.h"
#include "IVendorIfaceType.pb.h"

using GetVendorInterfaceCfm = ::supplicant::vendor::ISupplicantVendor_GetVendorInterfaceCfm;
using GetVendorInterfaceReq = ::supplicant::vendor::ISupplicantVendor_GetVendorInterfaceReq;
using ListVendorInterfacesCfm = ::supplicant::vendor::ISupplicantVendor_ListVendorInterfacesCfm;

using HalStatus_P = ::supplicant::vendor::HalStatus;
using ISupplicantVendorStaIface_P = ::supplicant::vendor::ISupplicantVendorStaIface;
using IVendorIfaceInfo_P = ::supplicant::vendor::IVendorIfaceInfo;
using IVendorIfaceType_P = ::supplicant::vendor::IVendorIfaceType;


static void IVendorIfaceInfo2Message(const IVendorIfaceInfo& ifaceInfo, IVendorIfaceInfo_P& msgSupplicantVendor)
{
    msgSupplicantVendor.set_type((IVendorIfaceType_P) ifaceInfo.type);
    msgSupplicantVendor.set_name(ifaceInfo.name);
}

bool SupplicantVendorSerializeGetVendorInterfaceReq(const IVendorIfaceInfo& ifaceInfo, vector<uint8_t>& payload)
{
    GetVendorInterfaceReq msgSupplicantVendor;
    IVendorIfaceInfo_P* ifaceInfo_p = msgSupplicantVendor.mutable_ifaceinfo();
    IVendorIfaceInfo2Message(ifaceInfo, *ifaceInfo_p);
    return ProtoMessageSerialize(&msgSupplicantVendor, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantVendor)
{
    msgSupplicantVendor.set_status(status.status);
    msgSupplicantVendor.set_info(status.info);
}

static void ISupplicantVendorStaIface2Message(int32_t result, ISupplicantVendorStaIface_P& msgSupplicantVendor)
{
    msgSupplicantVendor.set_instanceid(result);
}

bool SupplicantVendorSerializeGetVendorInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetVendorInterfaceCfm msgSupplicantVendor;
    HalStatus_P* status_p = msgSupplicantVendor.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantVendorStaIface_P* result_p = msgSupplicantVendor.mutable_result();
    ISupplicantVendorStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantVendor, payload);
}

bool SupplicantVendorSerializeListVendorInterfacesReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantVendorSerializeListVendorInterfacesCfm(const HalStatusParam& status, const vector<IVendorIfaceInfo>& result, vector<uint8_t>& payload)
{
    ListVendorInterfacesCfm msgSupplicantVendor;
    HalStatus_P* status_p = msgSupplicantVendor.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            IVendorIfaceInfo_P *result_p = msgSupplicantVendor.add_result();
            IVendorIfaceInfo2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantVendor, payload);
}

static void Message2IVendorIfaceInfo(const IVendorIfaceInfo_P& msgSupplicantVendor, IVendorIfaceInfo& ifaceInfo)
{
    ifaceInfo.type = (IVendorIfaceType) msgSupplicantVendor.type();
    ifaceInfo.name = msgSupplicantVendor.name();
}

bool SupplicantVendorParseGetVendorInterfaceReq(const uint8_t* data, size_t length, IVendorIfaceInfo& param)
{
    GetVendorInterfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2IVendorIfaceInfo(msg.ifaceinfo(), param);
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantVendor, HalStatusParam& status)
{
    status.status = msgSupplicantVendor.status();
    status.info = msgSupplicantVendor.info();
}

static void Message2ISupplicantVendorStaIface(const ISupplicantVendorStaIface_P& msgSupplicantVendor, int32_t& result)
{
    result = msgSupplicantVendor.instanceid();
}

bool SupplicantVendorParseGetVendorInterfaceCfm(const uint8_t* data, size_t length, GetVendorInterfaceCfmParam& param)
{
    GetVendorInterfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantVendorStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantVendorParseListVendorInterfacesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantVendorParseListVendorInterfacesCfm(const uint8_t* data, size_t length, ListVendorInterfacesCfmParam& param)
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
                Message2IVendorIfaceInfo(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}
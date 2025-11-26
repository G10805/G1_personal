/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "hostapd_vendor_msg_common.h"

#include "HalStatus.pb.h"

using HalStatus_P = ::hostapd::vendor::HalStatus;


bool VendorSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload)
{
    HalStatus_P msgVendor;
    msgVendor.set_status(status);
    msgVendor.set_info(info);
    return ProtoMessageSerialize(&msgVendor, payload);
}

bool VendorParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param)
{
    HalStatus_P msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.status = msg.status();
        param.info = msg.info();
        return true;
    }
    return false;
}
/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "hostapd_msg_common.h"

#include "HalStatus.pb.h"

using HalStatus_P = ::wifi::hostapd::HalStatus;


bool HostapdSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload)
{
    HalStatus_P msgHostapd;
    msgHostapd.set_status(status);
    msgHostapd.set_info(info);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

bool HostapdParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param)
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
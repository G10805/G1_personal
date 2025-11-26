/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "qti_wifi_msg_common.h"

#include "HalStatus.pb.h"

using HalStatus_P = ::qti::wifi::HalStatus;


bool WifiSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload)
{
    HalStatus_P msgWifi;
    msgWifi.set_status(status);
    msgWifi.set_info(info);
    return ProtoMessageSerialize(&msgWifi, payload);
}

bool WifiParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param)
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
/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_msg_common.h"

#include "HalStatus.pb.h"

using HalStatus_P = ::wifi::supplicant::HalStatus;


bool SupplicantSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload)
{
    HalStatus_P msgSupplicant;
    msgSupplicant.set_status(status);
    msgSupplicant.set_info(info);
    return ProtoMessageSerialize(&msgSupplicant, payload);
}

bool SupplicantParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param)
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
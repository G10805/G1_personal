/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "interceptor_msg_common.h"

#include "HalStatus.pb.h"

using HalStatus_P = ::net::nlinterceptor::HalStatus;


bool NlinterceptorSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload)
{
    HalStatus_P msgNlinterceptor;
    msgNlinterceptor.set_status(status);
    msgNlinterceptor.set_info(info);
    return ProtoMessageSerialize(&msgNlinterceptor, payload);
}

bool NlinterceptorParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param)
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
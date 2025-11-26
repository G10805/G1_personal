/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "interceptor_msg.h"

#include "HalStatus.pb.h"
#include "IInterceptor.pb.h"
#include "InterceptedSocket.pb.h"

using CloseSocketReq = ::net::nlinterceptor::IInterceptor_CloseSocketReq;
using CreateSocketCfm = ::net::nlinterceptor::IInterceptor_CreateSocketCfm;
using CreateSocketReq = ::net::nlinterceptor::IInterceptor_CreateSocketReq;
using SubscribeGroupReq = ::net::nlinterceptor::IInterceptor_SubscribeGroupReq;
using UnsubscribeGroupReq = ::net::nlinterceptor::IInterceptor_UnsubscribeGroupReq;

using HalStatus_P = ::net::nlinterceptor::HalStatus;
using InterceptedSocket_P = ::net::nlinterceptor::InterceptedSocket;


bool InterceptorSerializeCreateSocketReq(int32_t nlFamily, int32_t clientNlPid, const string& clientName, vector<uint8_t>& payload)
{
    CreateSocketReq msgInterceptor;
    msgInterceptor.set_nlfamily(nlFamily);
    msgInterceptor.set_clientnlpid(clientNlPid);
    msgInterceptor.set_clientname(clientName);
    return ProtoMessageSerialize(&msgInterceptor, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgInterceptor)
{
    msgInterceptor.set_status(status.status);
    msgInterceptor.set_info(status.info);
}

static void InterceptedSocket2Message(const InterceptedSocket& result, InterceptedSocket_P& msgInterceptor)
{
    msgInterceptor.set_nlfamily(result.nlFamily);
    msgInterceptor.set_portid(result.portId);
}

bool InterceptorSerializeCreateSocketCfm(const HalStatusParam& status, const InterceptedSocket& result, vector<uint8_t>& payload)
{
    CreateSocketCfm msgInterceptor;
    HalStatus_P* status_p = msgInterceptor.mutable_status();
    HalStatus2Message(status, *status_p);
    InterceptedSocket_P* result_p = msgInterceptor.mutable_result();
    InterceptedSocket2Message(result, *result_p);
    return ProtoMessageSerialize(&msgInterceptor, payload);
}

bool InterceptorSerializeCloseSocketReq(const InterceptedSocket& handle, vector<uint8_t>& payload)
{
    CloseSocketReq msgInterceptor;
    InterceptedSocket_P* handle_p = msgInterceptor.mutable_handle();
    InterceptedSocket2Message(handle, *handle_p);
    return ProtoMessageSerialize(&msgInterceptor, payload);
}

bool InterceptorSerializeCloseSocketCfm(vector<uint8_t>& payload)
{
    return true;
}

bool InterceptorSerializeSubscribeGroupReq(const InterceptedSocket& handle, int32_t nlGroup, vector<uint8_t>& payload)
{
    SubscribeGroupReq msgInterceptor;
    InterceptedSocket_P* handle_p = msgInterceptor.mutable_handle();
    InterceptedSocket2Message(handle, *handle_p);
    msgInterceptor.set_nlgroup(nlGroup);
    return ProtoMessageSerialize(&msgInterceptor, payload);
}

bool InterceptorSerializeSubscribeGroupCfm(vector<uint8_t>& payload)
{
    return true;
}

bool InterceptorSerializeUnsubscribeGroupReq(const InterceptedSocket& handle, int32_t nlGroup, vector<uint8_t>& payload)
{
    UnsubscribeGroupReq msgInterceptor;
    InterceptedSocket_P* handle_p = msgInterceptor.mutable_handle();
    InterceptedSocket2Message(handle, *handle_p);
    msgInterceptor.set_nlgroup(nlGroup);
    return ProtoMessageSerialize(&msgInterceptor, payload);
}

bool InterceptorSerializeUnsubscribeGroupCfm(vector<uint8_t>& payload)
{
    return true;
}

bool InterceptorParseCreateSocketReq(const uint8_t* data, size_t length, CreateSocketReqParam& param)
{
    CreateSocketReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.nlFamily = msg.nlfamily();
        param.clientNlPid = msg.clientnlpid();
        param.clientName = msg.clientname();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgInterceptor, HalStatusParam& status)
{
    status.status = msgInterceptor.status();
    status.info = msgInterceptor.info();
}

static void Message2InterceptedSocket(const InterceptedSocket_P& msgInterceptor, InterceptedSocket& result)
{
    result.nlFamily = msgInterceptor.nlfamily();
    result.portId = msgInterceptor.portid();
}

bool InterceptorParseCreateSocketCfm(const uint8_t* data, size_t length, CreateSocketCfmParam& param)
{
    CreateSocketCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2InterceptedSocket(msg.result(), param.result);
        return true;
    }
    return false;
}

bool InterceptorParseCloseSocketReq(const uint8_t* data, size_t length, InterceptedSocket& param)
{
    CloseSocketReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2InterceptedSocket(msg.handle(), param);
        return true;
    }
    return false;
}

bool InterceptorParseCloseSocketCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool InterceptorParseSubscribeGroupReq(const uint8_t* data, size_t length, SubscribeGroupReqParam& param)
{
    SubscribeGroupReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2InterceptedSocket(msg.handle(), param.handle);
        param.nlGroup = msg.nlgroup();
        return true;
    }
    return false;
}

bool InterceptorParseSubscribeGroupCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool InterceptorParseUnsubscribeGroupReq(const uint8_t* data, size_t length, UnsubscribeGroupReqParam& param)
{
    UnsubscribeGroupReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2InterceptedSocket(msg.handle(), param.handle);
        param.nlGroup = msg.nlgroup();
        return true;
    }
    return false;
}

bool InterceptorParseUnsubscribeGroupCfm(const uint8_t* data, size_t length)
{
    return true;
}
/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/net/nlinterceptor/IInterceptor.h>

#include "interceptor_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::net::nlinterceptor;


typedef struct
{
    int32_t nlFamily;
    int32_t clientNlPid;
    string clientName;
} CreateSocketReqParam;

typedef struct
{
    HalStatusParam status;
    InterceptedSocket result;
} CreateSocketCfmParam;

typedef struct
{
    InterceptedSocket handle;
    int32_t nlGroup;
} SubscribeGroupReqParam;

typedef struct
{
    InterceptedSocket handle;
    int32_t nlGroup;
} UnsubscribeGroupReqParam;

bool InterceptorSerializeCreateSocketReq(int32_t nlFamily, int32_t clientNlPid, const string& clientName, vector<uint8_t>& payload);

bool InterceptorSerializeCreateSocketCfm(const HalStatusParam& status, const InterceptedSocket& result, vector<uint8_t>& payload);

bool InterceptorSerializeCloseSocketReq(const InterceptedSocket& handle, vector<uint8_t>& payload);

bool InterceptorSerializeCloseSocketCfm(vector<uint8_t>& payload);

bool InterceptorSerializeSubscribeGroupReq(const InterceptedSocket& handle, int32_t nlGroup, vector<uint8_t>& payload);

bool InterceptorSerializeSubscribeGroupCfm(vector<uint8_t>& payload);

bool InterceptorSerializeUnsubscribeGroupReq(const InterceptedSocket& handle, int32_t nlGroup, vector<uint8_t>& payload);

bool InterceptorSerializeUnsubscribeGroupCfm(vector<uint8_t>& payload);

bool InterceptorParseCreateSocketReq(const uint8_t* data, size_t length, CreateSocketReqParam& param);

bool InterceptorParseCreateSocketCfm(const uint8_t* data, size_t length, CreateSocketCfmParam& param);

bool InterceptorParseCloseSocketReq(const uint8_t* data, size_t length, InterceptedSocket& param);

bool InterceptorParseCloseSocketCfm(const uint8_t* data, size_t length);

bool InterceptorParseSubscribeGroupReq(const uint8_t* data, size_t length, SubscribeGroupReqParam& param);

bool InterceptorParseSubscribeGroupCfm(const uint8_t* data, size_t length);

bool InterceptorParseUnsubscribeGroupReq(const uint8_t* data, size_t length, UnsubscribeGroupReqParam& param);

bool InterceptorParseUnsubscribeGroupCfm(const uint8_t* data, size_t length);
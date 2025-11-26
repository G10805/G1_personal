/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "interceptor_msg.h"
#include "interceptor_msg_common.h"

using std::array;
using std::string;
using std::vector;

#define VERIFY(f)   { \
                        if (!(f)) \
                            { \
                                ALOGE("%s: %s fail, return false", __func__, #f); \
                                return false; \
                            } \
                    }

static bool COMPARE_STR(string& str1, string& str2, const char *output)
{
    ALOGD("%s: %s: str1 size=%d, str2 size=%d", __func__, output, str1.size(), str2.size());
    if (!str1.compare(str2))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename VectorType>
static bool COMPARE_VEC(vector<VectorType>& vec1, vector<VectorType>& vec2, const char *output)
{
    ALOGD("%s: %s: vec1 size=%d, vec2 size=%d", __func__, output, vec1.size(), vec2.size());
    if (vec1 == vec2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename ArrayType, size_t N>
static bool COMPARE_ARY(array<ArrayType, N>& array1, array<ArrayType, N>& array2, const char *output)
{
    ALOGD("%s: %s: array1 size=%d, array2 size=%d", __func__, output, array1.size(), array2.size());
    if (array1 == array2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

static bool COMPARE_BUF(void *buf1, void *buf2, size_t length, const char *output)
{
    ALOGD("%s: size: %d: %s", __func__, length, output);
    if (!memcmp(buf1, buf2, length))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename ObjType>
static bool COMPARE_OBJ(ObjType &obj1, ObjType &obj2, const char *output)
{
    ALOGD("%s: %s: obj1 size=%d, obj2 size=%d", __func__, output, sizeof(obj1), sizeof(obj2));
    if (obj1 == obj2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

static void INIT_HAL_STATUS(HalStatusParam& status)
{
    status.status = 47;
    status.info = "3VQr6OZjdJM";
}

static bool T_INTERCEPTOR_CREATE_SOCKET_REQ()
{
    int32_t nlFamily = 93;
    int32_t clientNlPid = 100;
    string clientName = "dtXF3n9";
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeCreateSocketReq(nlFamily, clientNlPid, clientName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateSocketReqParam param;
    bool msgParseResult = InterceptorParseCreateSocketReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&nlFamily, &param.nlFamily, sizeof(int32_t), "nlFamily (int32_t)"));
    VERIFY(COMPARE_BUF(&clientNlPid, &param.clientNlPid, sizeof(int32_t), "clientNlPid (int32_t)"));
    VERIFY(COMPARE_STR(clientName, param.clientName, "clientName (string)"));
    return true;
}

static void INIT_INTERCEPTED_SOCKET(InterceptedSocket& result)
{
    result.nlFamily = 110;
    result.portId = 26;
}

static bool T_INTERCEPTOR_CREATE_SOCKET_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    InterceptedSocket result;
    INIT_INTERCEPTED_SOCKET(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeCreateSocketCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateSocketCfmParam param;
    bool msgParseResult = InterceptorParseCreateSocketCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (InterceptedSocket)"));
    return true;
}

static bool T_INTERCEPTOR_CLOSE_SOCKET_REQ()
{
    InterceptedSocket handle;
    INIT_INTERCEPTED_SOCKET(handle);
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeCloseSocketReq(handle, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InterceptedSocket param;
    bool msgParseResult = InterceptorParseCloseSocketReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(handle, param, "handle (InterceptedSocket)"));
    return true;
}

static bool T_INTERCEPTOR_CLOSE_SOCKET_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeCloseSocketCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = InterceptorParseCloseSocketCfm(payload.data(), payload.size());
    return true;
}

static bool T_INTERCEPTOR_SUBSCRIBE_GROUP_REQ()
{
    InterceptedSocket handle;
    INIT_INTERCEPTED_SOCKET(handle);
    int32_t nlGroup = 75;
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeSubscribeGroupReq(handle, nlGroup, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SubscribeGroupReqParam param;
    bool msgParseResult = InterceptorParseSubscribeGroupReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(handle, param.handle, "handle (InterceptedSocket)"));
    VERIFY(COMPARE_BUF(&nlGroup, &param.nlGroup, sizeof(int32_t), "nlGroup (int32_t)"));
    return true;
}

static bool T_INTERCEPTOR_SUBSCRIBE_GROUP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeSubscribeGroupCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = InterceptorParseSubscribeGroupCfm(payload.data(), payload.size());
    return true;
}

static bool T_INTERCEPTOR_UNSUBSCRIBE_GROUP_REQ()
{
    InterceptedSocket handle;
    INIT_INTERCEPTED_SOCKET(handle);
    int32_t nlGroup = 126;
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeUnsubscribeGroupReq(handle, nlGroup, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    UnsubscribeGroupReqParam param;
    bool msgParseResult = InterceptorParseUnsubscribeGroupReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(handle, param.handle, "handle (InterceptedSocket)"));
    VERIFY(COMPARE_BUF(&nlGroup, &param.nlGroup, sizeof(int32_t), "nlGroup (int32_t)"));
    return true;
}

static bool T_INTERCEPTOR_UNSUBSCRIBE_GROUP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = InterceptorSerializeUnsubscribeGroupCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = InterceptorParseUnsubscribeGroupCfm(payload.data(), payload.size());
    return true;
}

static bool T_NLINTERCEPTOR_HAL_STATUS()
{
    int32_t status = 90;
    string info = "Y6kf87J";
    vector<uint8_t> payload;
    bool msgSerializeResult = NlinterceptorSerializeHalStatus(status, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HalStatusParam param;
    bool msgParseResult = NlinterceptorParseHalStatus(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(int32_t), "status (int32_t)"));
    VERIFY(COMPARE_STR(info, param.info, "info (string)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_INTERCEPTOR_CREATE_SOCKET_REQ());
    VERIFY(T_INTERCEPTOR_CREATE_SOCKET_CFM());
    VERIFY(T_INTERCEPTOR_CLOSE_SOCKET_REQ());
    VERIFY(T_INTERCEPTOR_CLOSE_SOCKET_CFM());
    VERIFY(T_INTERCEPTOR_SUBSCRIBE_GROUP_REQ());
    VERIFY(T_INTERCEPTOR_SUBSCRIBE_GROUP_CFM());
    VERIFY(T_INTERCEPTOR_UNSUBSCRIBE_GROUP_REQ());
    VERIFY(T_INTERCEPTOR_UNSUBSCRIBE_GROUP_CFM());
    VERIFY(T_NLINTERCEPTOR_HAL_STATUS());
    ALOGD("ALL 9 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("interceptor-msg-test starts");
    int count = 1;
    if (1 < argc)
    {
        count = atoi(argv[1]);
    }
    for (int i = 0; i < count; i++)
    {
        if (T_ALL())
        {
            ALOGD("LOOP #%d FINISHED AND PASSED", i + 1);
        }
        else
        {
            ALOGE("LOOP #%d FAILED", i + 1);
        }
    }
    return 0;
}
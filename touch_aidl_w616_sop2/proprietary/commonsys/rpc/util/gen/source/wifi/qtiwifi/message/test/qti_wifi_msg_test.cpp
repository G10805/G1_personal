/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "qti_wifi_msg.h"
#include "qti_wifi_msg_common.h"

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
    status.status = 82;
    status.info = "2ow2hRDfUL6wg";
}

static bool T_QTI_WIFI_LIST_AVAILABLE_INTERFACES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeListAvailableInterfacesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = QtiWifiParseListAvailableInterfacesReq(payload.data(), payload.size());
    return true;
}

static void INIT_IFACE_INFO(IfaceInfo& result)
{
    result.type = IfaceType(1);
    result.name = "ulLzMaErA";
}

static bool T_QTI_WIFI_LIST_AVAILABLE_INTERFACES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<IfaceInfo> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_IFACE_INFO(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeListAvailableInterfacesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListAvailableInterfacesCfmParam param;
    bool msgParseResult = QtiWifiParseListAvailableInterfacesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<IfaceInfo>)"));
    return true;
}

static bool T_QTI_WIFI_REGISTER_QTI_WIFI_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeRegisterQtiWifiCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = QtiWifiParseRegisterQtiWifiCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_QTI_WIFI_REGISTER_QTI_WIFI_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeRegisterQtiWifiCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = QtiWifiParseRegisterQtiWifiCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_QTI_WIFI_DO_QTI_WIFI_CMD_REQ()
{
    string iface = "tGX3pBPfffdcKB";
    string cmd = "ZYyeCV8JTyO";
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeDoQtiWifiCmdReq(iface, cmd, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DoQtiWifiCmdReqParam param;
    bool msgParseResult = QtiWifiParseDoQtiWifiCmdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(iface, param.iface, "iface (string)"));
    VERIFY(COMPARE_STR(cmd, param.cmd, "cmd (string)"));
    return true;
}

static bool T_QTI_WIFI_DO_QTI_WIFI_CMD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "FVVA1Im";
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeDoQtiWifiCmdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DoQtiWifiCmdCfmParam param;
    bool msgParseResult = QtiWifiParseDoQtiWifiCmdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_QTI_WIFI_ON_CTRL_EVENT_IND()
{
    string iface = "RpGF3pi";
    string event = "6BxASKrTMJ8Mi";
    vector<uint8_t> payload;
    bool msgSerializeResult = QtiWifiSerializeOnCtrlEventInd(iface, event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnCtrlEventIndParam param;
    bool msgParseResult = QtiWifiParseOnCtrlEventInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(iface, param.iface, "iface (string)"));
    VERIFY(COMPARE_STR(event, param.event, "event (string)"));
    return true;
}

static bool T_WIFI_HAL_STATUS()
{
    int32_t status = 21;
    string info = "dluDPTtAS9XaPv0";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeHalStatus(status, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HalStatusParam param;
    bool msgParseResult = WifiParseHalStatus(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(int32_t), "status (int32_t)"));
    VERIFY(COMPARE_STR(info, param.info, "info (string)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_QTI_WIFI_LIST_AVAILABLE_INTERFACES_REQ());
    VERIFY(T_QTI_WIFI_LIST_AVAILABLE_INTERFACES_CFM());
    VERIFY(T_QTI_WIFI_REGISTER_QTI_WIFI_CALLBACK_REQ());
    VERIFY(T_QTI_WIFI_REGISTER_QTI_WIFI_CALLBACK_CFM());
    VERIFY(T_QTI_WIFI_DO_QTI_WIFI_CMD_REQ());
    VERIFY(T_QTI_WIFI_DO_QTI_WIFI_CMD_CFM());
    VERIFY(T_QTI_WIFI_ON_CTRL_EVENT_IND());
    VERIFY(T_WIFI_HAL_STATUS());
    ALOGD("ALL 8 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("qti-wifi-msg-test starts");
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
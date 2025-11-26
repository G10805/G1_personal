/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "supplicant_vendor_msg.h"
#include "supplicant_vendor_msg_common.h"
#include "supplicant_vendor_sta_iface_msg.h"

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
    status.status = 35;
    status.info = "vePL5Lalu";
}

static void INIT_I_VENDOR_IFACE_INFO(IVendorIfaceInfo& ifaceInfo)
{
    ifaceInfo.type = IVendorIfaceType(1);
    ifaceInfo.name = "Y7v4e0Rrrn0iIQB";
}

static bool T_SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_REQ()
{
    IVendorIfaceInfo ifaceInfo;
    INIT_I_VENDOR_IFACE_INFO(ifaceInfo);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorSerializeGetVendorInterfaceReq(ifaceInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IVendorIfaceInfo param;
    bool msgParseResult = SupplicantVendorParseGetVendorInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(ifaceInfo, param, "ifaceInfo (IVendorIfaceInfo)"));
    return true;
}

static bool T_SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 71;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorSerializeGetVendorInterfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetVendorInterfaceCfmParam param;
    bool msgParseResult = SupplicantVendorParseGetVendorInterfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorSerializeListVendorInterfacesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantVendorParseListVendorInterfacesReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<IVendorIfaceInfo> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_I_VENDOR_IFACE_INFO(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorSerializeListVendorInterfacesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListVendorInterfacesCfmParam param;
    bool msgParseResult = SupplicantVendorParseListVendorInterfacesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<IVendorIfaceInfo>)"));
    return true;
}

static bool T_SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_REQ()
{
    string command = "w7w5XLhp6Wf0phTL";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorStaIfaceSerializeDoDriverCmdReq(command, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantVendorStaIfaceParseDoDriverCmdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(command, param, "command (string)"));
    return true;
}

static bool T_SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "Us";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorStaIfaceSerializeDoDriverCmdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DoDriverCmdCfmParam param;
    bool msgParseResult = SupplicantVendorStaIfaceParseDoDriverCmdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND()
{
    string ifaceName = "C";
    string eventStr = "S0pT0md6HeK";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantVendorStaIfaceSerializeOnCtrlEventInd(ifaceName, eventStr, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnCtrlEventIndParam param;
    bool msgParseResult = SupplicantVendorStaIfaceParseOnCtrlEventInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    VERIFY(COMPARE_STR(eventStr, param.eventStr, "eventStr (string)"));
    return true;
}

static bool T_VENDOR_HAL_STATUS()
{
    int32_t status = 81;
    string info = "a3gVI6Fcx";
    vector<uint8_t> payload;
    bool msgSerializeResult = VendorSerializeHalStatus(status, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HalStatusParam param;
    bool msgParseResult = VendorParseHalStatus(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(int32_t), "status (int32_t)"));
    VERIFY(COMPARE_STR(info, param.info, "info (string)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_VENDOR_GET_VENDOR_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_REQ());
    VERIFY(T_SUPPLICANT_VENDOR_LIST_VENDOR_INTERFACES_CFM());
    VERIFY(T_SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_REQ());
    VERIFY(T_SUPPLICANT_VENDOR_STA_IFACE_DO_DRIVER_CMD_CFM());
    VERIFY(T_SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_VENDOR_STA_IFACE_REGISTER_SUPPLICANT_VENDOR_STA_IFACE_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_VENDOR_STA_IFACE_ON_CTRL_EVENT_IND());
    VERIFY(T_VENDOR_HAL_STATUS());
    ALOGD("ALL 10 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("supplicant-vendor-msg-test starts");
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
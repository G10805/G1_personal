/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "hostapd_vendor_msg.h"
#include "hostapd_vendor_msg_common.h"

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
    status.status = 94;
    status.info = "L";
}

static bool T_HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeListVendorInterfacesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdVendorParseListVendorInterfacesReq(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "EmJEAkAX5Yc4m";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeListVendorInterfacesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListVendorInterfacesCfmParam param;
    bool msgParseResult = HostapdVendorParseListVendorInterfacesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_HOSTAPD_VENDOR_REGISTER_HOSTAPD_VENDOR_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeRegisterHostapdVendorCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdVendorParseRegisterHostapdVendorCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_VENDOR_REGISTER_HOSTAPD_VENDOR_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeRegisterHostapdVendorCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdVendorParseRegisterHostapdVendorCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_VENDOR_DO_DRIVER_CMD_REQ()
{
    string iface = "xWPrIdYCCh6j";
    string cmd = "A";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeDoDriverCmdReq(iface, cmd, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DoDriverCmdReqParam param;
    bool msgParseResult = HostapdVendorParseDoDriverCmdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(iface, param.iface, "iface (string)"));
    VERIFY(COMPARE_STR(cmd, param.cmd, "cmd (string)"));
    return true;
}

static bool T_HOSTAPD_VENDOR_DO_DRIVER_CMD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "4o";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeDoDriverCmdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DoDriverCmdCfmParam param;
    bool msgParseResult = HostapdVendorParseDoDriverCmdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_HOSTAPD_VENDOR_ON_CTRL_EVENT_IND()
{
    string ifaceName = "2b4Oa";
    string event_str = "FIaYfi93HGI";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeOnCtrlEventInd(ifaceName, event_str, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnCtrlEventIndParam param;
    bool msgParseResult = HostapdVendorParseOnCtrlEventInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    VERIFY(COMPARE_STR(event_str, param.event_str, "event_str (string)"));
    return true;
}

static void INIT_VENDOR_AP_INFO(VendorApInfo& apInfo)
{
    apInfo.ifaceName = "VNcQFagFdmLZ";
    apInfo.apIfaceInstance = "lk5fU2TpJ";
}

static bool T_HOSTAPD_VENDOR_ON_AP_INSTANCE_INFO_CHANGED_IND()
{
    VendorApInfo apInfo;
    INIT_VENDOR_AP_INFO(apInfo);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeOnApInstanceInfoChangedInd(apInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    VendorApInfo param;
    bool msgParseResult = HostapdVendorParseOnApInstanceInfoChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(apInfo, param, "apInfo (VendorApInfo)"));
    return true;
}

static bool T_HOSTAPD_VENDOR_ON_FAILURE_IND()
{
    string ifname = "ifBn1STiFzDu4YD";
    string instanceName = "RfeuTAYMAR";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdVendorSerializeOnFailureInd(ifname, instanceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnFailureIndParam param;
    bool msgParseResult = HostapdVendorParseOnFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param.ifname, "ifname (string)"));
    VERIFY(COMPARE_STR(instanceName, param.instanceName, "instanceName (string)"));
    return true;
}

static bool T_VENDOR_HAL_STATUS()
{
    int32_t status = 6;
    string info = "4PVif";
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
    VERIFY(T_HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_REQ());
    VERIFY(T_HOSTAPD_VENDOR_LIST_VENDOR_INTERFACES_CFM());
    VERIFY(T_HOSTAPD_VENDOR_REGISTER_HOSTAPD_VENDOR_CALLBACK_REQ());
    VERIFY(T_HOSTAPD_VENDOR_REGISTER_HOSTAPD_VENDOR_CALLBACK_CFM());
    VERIFY(T_HOSTAPD_VENDOR_DO_DRIVER_CMD_REQ());
    VERIFY(T_HOSTAPD_VENDOR_DO_DRIVER_CMD_CFM());
    VERIFY(T_HOSTAPD_VENDOR_ON_CTRL_EVENT_IND());
    VERIFY(T_HOSTAPD_VENDOR_ON_AP_INSTANCE_INFO_CHANGED_IND());
    VERIFY(T_HOSTAPD_VENDOR_ON_FAILURE_IND());
    VERIFY(T_VENDOR_HAL_STATUS());
    ALOGD("ALL 10 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("hostapd-vendor-msg-test starts");
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
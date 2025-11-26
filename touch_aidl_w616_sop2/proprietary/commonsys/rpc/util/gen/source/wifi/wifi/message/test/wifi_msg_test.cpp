/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "wifi_ap_iface_msg.h"
#include "wifi_chip_msg.h"
#include "wifi_msg.h"
#include "wifi_msg_common.h"
#include "wifi_nan_iface_msg.h"
#include "wifi_p2p_iface_msg.h"
#include "wifi_rtt_controller_msg.h"
#include "wifi_sta_iface_msg.h"

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
    status.status = 97;
    status.info = "cBFvStEux";
}

static bool T_WIFI_GET_CHIP_REQ()
{
    int32_t chipId = 10;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeGetChipReq(chipId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiParseGetChipReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&chipId, &param, sizeof(int32_t), "chipId (int32_t)"));
    return true;
}

static bool T_WIFI_GET_CHIP_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 101;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeGetChipCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetChipCfmParam param;
    bool msgParseResult = WifiParseGetChipCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_GET_CHIP_IDS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeGetChipIdsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseGetChipIdsReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_GET_CHIP_IDS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<int32_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 90;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeGetChipIdsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetChipIdsCfmParam param;
    bool msgParseResult = WifiParseGetChipIdsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<int32>)"));
    return true;
}

static bool T_WIFI_IS_STARTED_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeIsStartedReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseIsStartedReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_IS_STARTED_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeIsStartedCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsStartedCfmParam param;
    bool msgParseResult = WifiParseIsStartedCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_WIFI_REGISTER_EVENT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeRegisterEventCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseRegisterEventCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_REGISTER_EVENT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeRegisterEventCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseRegisterEventCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_START_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeStartReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseStartReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_START_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeStartCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseStartCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STOP_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeStopReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseStopReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STOP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeStopCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseStopCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_ON_FAILURE_IND()
{
    WifiStatusCode status = WifiStatusCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeOnFailureInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    WifiStatusCode param;
    bool msgParseResult = WifiParseOnFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param, sizeof(WifiStatusCode), "status (WifiStatusCode)"));
    return true;
}

static bool T_WIFI_ON_START_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeOnStartInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseOnStartInd(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_ON_STOP_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeOnStopInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiParseOnStopInd(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_ON_SUBSYSTEM_RESTART_IND()
{
    WifiStatusCode status = WifiStatusCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiSerializeOnSubsystemRestartInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    WifiStatusCode param;
    bool msgParseResult = WifiParseOnSubsystemRestartInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param, sizeof(WifiStatusCode), "status (WifiStatusCode)"));
    return true;
}

static bool T_WIFI_AP_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "2EuYzWUZXrL";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmApIfaceParam param;
    bool msgParseResult = WifiApIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_WIFI_AP_IFACE_GET_BRIDGED_INSTANCES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetBridgedInstancesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseGetBridgedInstancesReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_GET_BRIDGED_INSTANCES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "j4";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetBridgedInstancesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetBridgedInstancesCfmApIfaceParam param;
    bool msgParseResult = WifiApIfaceParseGetBridgedInstancesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_WIFI_AP_IFACE_GET_FACTORY_MAC_ADDRESS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetFactoryMacAddressReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseGetFactoryMacAddressReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_GET_FACTORY_MAC_ADDRESS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    array<uint8_t, 6> result;
    for (int index = 0; index < 6; index++)
    {
        result[index] = 0xc4;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeGetFactoryMacAddressCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetFactoryMacAddressCfmApIfaceParam param;
    bool msgParseResult = WifiApIfaceParseGetFactoryMacAddressCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_ARY(result, param.result, "result (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_AP_IFACE_SET_COUNTRY_CODE_REQ()
{
    array<uint8_t, 2> code;
    for (int index = 0; index < 2; index++)
    {
        code[index] = 0x04;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeSetCountryCodeReq(code, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    array<uint8_t, 2> param;
    bool msgParseResult = WifiApIfaceParseSetCountryCodeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_ARY(code, param, "code (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_AP_IFACE_SET_COUNTRY_CODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeSetCountryCodeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseSetCountryCodeCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_RESET_TO_FACTORY_MAC_ADDRESS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeResetToFactoryMacAddressReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseResetToFactoryMacAddressReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_RESET_TO_FACTORY_MAC_ADDRESS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeResetToFactoryMacAddressCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseResetToFactoryMacAddressCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_AP_IFACE_SET_MAC_ADDRESS_REQ()
{
    array<uint8_t, 6> mac;
    for (int index = 0; index < 6; index++)
    {
        mac[index] = 0xae;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeSetMacAddressReq(mac, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    array<uint8_t, 6> param;
    bool msgParseResult = WifiApIfaceParseSetMacAddressReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_ARY(mac, param, "mac (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_AP_IFACE_SET_MAC_ADDRESS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiApIfaceSerializeSetMacAddressCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiApIfaceParseSetMacAddressCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CONFIGURE_CHIP_REQ()
{
    int32_t modeId = 50;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeConfigureChipReq(modeId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiChipParseConfigureChipReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&modeId, &param, sizeof(int32_t), "modeId (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CONFIGURE_CHIP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeConfigureChipCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseConfigureChipCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_AP_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateApIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseCreateApIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_AP_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 91;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateApIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateApIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateApIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_BRIDGED_AP_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateBridgedApIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseCreateBridgedApIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_BRIDGED_AP_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 80;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateBridgedApIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateBridgedApIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateBridgedApIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_NAN_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateNanIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseCreateNanIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_NAN_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 17;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateNanIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateNanIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateNanIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_P2P_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateP2pIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseCreateP2pIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_P2P_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 84;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateP2pIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateP2pIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateP2pIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_RTT_CONTROLLER_REQ()
{
    int32_t boundIface = 43;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateRttControllerReq(boundIface, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiChipParseCreateRttControllerReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&boundIface, &param, sizeof(int32_t), "boundIface (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_RTT_CONTROLLER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 90;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateRttControllerCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateRttControllerCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateRttControllerCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_CREATE_STA_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateStaIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseCreateStaIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_CREATE_STA_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 16;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeCreateStaIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateStaIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseCreateStaIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_ENABLE_DEBUG_ERROR_ALERTS_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeEnableDebugErrorAlertsReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = WifiChipParseEnableDebugErrorAlertsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_WIFI_CHIP_ENABLE_DEBUG_ERROR_ALERTS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeEnableDebugErrorAlertsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseEnableDebugErrorAlertsCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_FLUSH_RING_BUFFER_TO_FILE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeFlushRingBufferToFileReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseFlushRingBufferToFileReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_FLUSH_RING_BUFFER_TO_FILE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeFlushRingBufferToFileCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseFlushRingBufferToFileCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_FORCE_DUMP_TO_DEBUG_RING_BUFFER_REQ()
{
    string ringName = "hWcqiT4FNWi";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeForceDumpToDebugRingBufferReq(ringName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseForceDumpToDebugRingBufferReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ringName, param, "ringName (string)"));
    return true;
}

static bool T_WIFI_CHIP_FORCE_DUMP_TO_DEBUG_RING_BUFFER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeForceDumpToDebugRingBufferCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseForceDumpToDebugRingBufferCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_AP_IFACE_REQ()
{
    string ifname = "7yWVngV";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetApIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseGetApIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_GET_AP_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 85;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetApIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetApIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseGetApIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_AP_IFACE_NAMES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetApIfaceNamesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetApIfaceNamesReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_AP_IFACE_NAMES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "0Kjw";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetApIfaceNamesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetApIfaceNamesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetApIfaceNamesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_AVAILABLE_MODES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetAvailableModesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetAvailableModesReq(payload.data(), payload.size());
    return true;
}

static void INIT_CHIP_CONCURRENCY_COMBINATION_LIMIT(IWifiChip::ChipConcurrencyCombinationLimit& limits)
{
    limits.types.resize(4);
    for (int index = 0; index < 4; index++)
    {
        limits.types[index] = IfaceConcurrencyType(1);
    }
    limits.maxIfaces = 51;
}

static void INIT_CHIP_CONCURRENCY_COMBINATION(IWifiChip::ChipConcurrencyCombination& availableCombinations)
{
    availableCombinations.limits.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_CHIP_CONCURRENCY_COMBINATION_LIMIT(availableCombinations.limits[index]);
    }
}

static void INIT_CHIP_MODE(IWifiChip::ChipMode& result)
{
    result.id = 90;
    result.availableCombinations.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_CHIP_CONCURRENCY_COMBINATION(result.availableCombinations[index]);
    }
}

static bool T_WIFI_CHIP_GET_AVAILABLE_MODES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<IWifiChip::ChipMode> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_CHIP_MODE(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetAvailableModesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetAvailableModesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetAvailableModesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<IWifiChip::ChipMode>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_FEATURE_SET_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetFeatureSetReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetFeatureSetReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_FEATURE_SET_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 73;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetFeatureSetCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetFeatureSetCfmChipParam param;
    bool msgParseResult = WifiChipParseGetFeatureSetCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_DEBUG_HOST_WAKE_REASON_STATS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetDebugHostWakeReasonStatsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetDebugHostWakeReasonStatsReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_PACKET_DETAILS(WifiDebugHostWakeReasonRxPacketDetails& rxPktWakeDetails)
{
    rxPktWakeDetails.rxUnicastCnt = 39;
    rxPktWakeDetails.rxMulticastCnt = 108;
    rxPktWakeDetails.rxBroadcastCnt = 85;
}

static void INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_MULTICAST_PACKET_DETAILS(WifiDebugHostWakeReasonRxMulticastPacketDetails& rxMulticastPkWakeDetails)
{
    rxMulticastPkWakeDetails.ipv4RxMulticastAddrCnt = 28;
    rxMulticastPkWakeDetails.ipv6RxMulticastAddrCnt = 70;
    rxMulticastPkWakeDetails.otherRxMulticastAddrCnt = 47;
}

static void INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_ICMP_PACKET_DETAILS(WifiDebugHostWakeReasonRxIcmpPacketDetails& rxIcmpPkWakeDetails)
{
    rxIcmpPkWakeDetails.icmpPkt = 30;
    rxIcmpPkWakeDetails.icmp6Pkt = 105;
    rxIcmpPkWakeDetails.icmp6Ra = 33;
    rxIcmpPkWakeDetails.icmp6Na = 21;
    rxIcmpPkWakeDetails.icmp6Ns = 34;
}

static void INIT_WIFI_DEBUG_HOST_WAKE_REASON_STATS(WifiDebugHostWakeReasonStats& result)
{
    result.totalCmdEventWakeCnt = 106;
    result.cmdEventWakeCntPerType.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result.cmdEventWakeCntPerType[index] = 38;
    }
    result.totalDriverFwLocalWakeCnt = 123;
    result.driverFwLocalWakeCntPerType.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result.driverFwLocalWakeCntPerType[index] = 28;
    }
    result.totalRxPacketWakeCnt = 17;
    INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_PACKET_DETAILS(result.rxPktWakeDetails);
    INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_MULTICAST_PACKET_DETAILS(result.rxMulticastPkWakeDetails);
    INIT_WIFI_DEBUG_HOST_WAKE_REASON_RX_ICMP_PACKET_DETAILS(result.rxIcmpPkWakeDetails);
}

static bool T_WIFI_CHIP_GET_DEBUG_HOST_WAKE_REASON_STATS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    WifiDebugHostWakeReasonStats result;
    INIT_WIFI_DEBUG_HOST_WAKE_REASON_STATS(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetDebugHostWakeReasonStatsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDebugHostWakeReasonStatsCfmChipParam param;
    bool msgParseResult = WifiChipParseGetDebugHostWakeReasonStatsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (WifiDebugHostWakeReasonStats)"));
    return true;
}

static bool T_WIFI_CHIP_GET_DEBUG_RING_BUFFERS_STATUS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetDebugRingBuffersStatusReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetDebugRingBuffersStatusReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_DEBUG_RING_BUFFER_STATUS(WifiDebugRingBufferStatus& result)
{
    result.ringName = "hQXMTMMRwws";
    result.flags = 104;
    result.ringId = 87;
    result.sizeInBytes = 35;
    result.freeSizeInBytes = 71;
    result.verboseLevel = 106;
}

static bool T_WIFI_CHIP_GET_DEBUG_RING_BUFFERS_STATUS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<WifiDebugRingBufferStatus> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_DEBUG_RING_BUFFER_STATUS(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetDebugRingBuffersStatusCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDebugRingBuffersStatusCfmChipParam param;
    bool msgParseResult = WifiChipParseGetDebugRingBuffersStatusCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<WifiDebugRingBufferStatus>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetIdReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 78;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetIdCfmChipParam param;
    bool msgParseResult = WifiChipParseGetIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_MODE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetModeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetModeReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_MODE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 74;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetModeCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetModeCfmChipParam param;
    bool msgParseResult = WifiChipParseGetModeCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_NAN_IFACE_REQ()
{
    string ifname = "LnbVfh";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetNanIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseGetNanIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_GET_NAN_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 47;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetNanIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNanIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseGetNanIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_NAN_IFACE_NAMES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetNanIfaceNamesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetNanIfaceNamesReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_NAN_IFACE_NAMES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "klhC";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetNanIfaceNamesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNanIfaceNamesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetNanIfaceNamesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_P2P_IFACE_REQ()
{
    string ifname = "ukVmsGoHLi";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetP2pIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseGetP2pIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_GET_P2P_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 64;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetP2pIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetP2pIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseGetP2pIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_P2P_IFACE_NAMES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetP2pIfaceNamesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetP2pIfaceNamesReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_P2P_IFACE_NAMES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "KppBJxgJ";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetP2pIfaceNamesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetP2pIfaceNamesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetP2pIfaceNamesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_STA_IFACE_REQ()
{
    string ifname = "j4NL";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetStaIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseGetStaIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_GET_STA_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 100;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetStaIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetStaIfaceCfmChipParam param;
    bool msgParseResult = WifiChipParseGetStaIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_GET_STA_IFACE_NAMES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetStaIfaceNamesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetStaIfaceNamesReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_GET_STA_IFACE_NAMES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "YiB";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetStaIfaceNamesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetStaIfaceNamesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetStaIfaceNamesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<string>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_SUPPORTED_RADIO_COMBINATIONS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetSupportedRadioCombinationsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetSupportedRadioCombinationsReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_RADIO_CONFIGURATION(WifiRadioConfiguration& radioConfigurations)
{
    radioConfigurations.bandInfo = WifiBand(1);
    radioConfigurations.antennaMode = WifiAntennaMode(1);
}

static void INIT_WIFI_RADIO_COMBINATION(WifiRadioCombination& result)
{
    result.radioConfigurations.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_RADIO_CONFIGURATION(result.radioConfigurations[index]);
    }
}

static bool T_WIFI_CHIP_GET_SUPPORTED_RADIO_COMBINATIONS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<WifiRadioCombination> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_RADIO_COMBINATION(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetSupportedRadioCombinationsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSupportedRadioCombinationsCfmChipParam param;
    bool msgParseResult = WifiChipParseGetSupportedRadioCombinationsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<WifiRadioCombination>)"));
    return true;
}

static bool T_WIFI_CHIP_GET_WIFI_CHIP_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetWifiChipCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseGetWifiChipCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_CHIP_CAPABILITIES(WifiChipCapabilities& result)
{
    result.maxMloAssociationLinkCount = 82;
    result.maxMloStrLinkCount = 7;
    result.maxConcurrentTdlsSessionCount = 119;
}

static bool T_WIFI_CHIP_GET_WIFI_CHIP_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    WifiChipCapabilities result;
    INIT_WIFI_CHIP_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetWifiChipCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWifiChipCapabilitiesCfmChipParam param;
    bool msgParseResult = WifiChipParseGetWifiChipCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (WifiChipCapabilities)"));
    return true;
}

static bool T_WIFI_CHIP_GET_USABLE_CHANNELS_REQ()
{
    WifiBand band = WifiBand(1);
    int32_t ifaceModeMask = 23;
    int32_t filterMask = 115;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetUsableChannelsReq(band, ifaceModeMask, filterMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetUsableChannelsReqChipParam param;
    bool msgParseResult = WifiChipParseGetUsableChannelsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&band, &param.band, sizeof(WifiBand), "band (WifiBand)"));
    VERIFY(COMPARE_BUF(&ifaceModeMask, &param.ifaceModeMask, sizeof(int32_t), "ifaceModeMask (int32_t)"));
    VERIFY(COMPARE_BUF(&filterMask, &param.filterMask, sizeof(int32_t), "filterMask (int32_t)"));
    return true;
}

static void INIT_WIFI_USABLE_CHANNEL(WifiUsableChannel& result)
{
    result.channel = 44;
    result.channelBandwidth = WifiChannelWidthInMhz(1);
    result.ifaceModeMask = 62;
}

static bool T_WIFI_CHIP_GET_USABLE_CHANNELS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<WifiUsableChannel> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_USABLE_CHANNEL(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeGetUsableChannelsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetUsableChannelsCfmChipParam param;
    bool msgParseResult = WifiChipParseGetUsableChannelsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<WifiUsableChannel>)"));
    return true;
}

static void INIT_AVAILABLE_AFC_FREQUENCY_INFO(AvailableAfcFrequencyInfo& availableAfcFrequencyInfos)
{
    availableAfcFrequencyInfos.startFrequencyMhz = 68;
    availableAfcFrequencyInfos.endFrequencyMhz = 65;
    availableAfcFrequencyInfos.maxPsd = 88;
}

static void INIT_AVAILABLE_AFC_CHANNEL_INFO(AvailableAfcChannelInfo& availableAfcChannelInfos)
{
    availableAfcChannelInfos.globalOperatingClass = 23;
    availableAfcChannelInfos.channelCfi = 126;
    availableAfcChannelInfos.maxEirpDbm = 28;
}

static void INIT_AFC_CHANNEL_ALLOWANCE(AfcChannelAllowance& afcChannelAllowance)
{
    afcChannelAllowance.availableAfcFrequencyInfos.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_AVAILABLE_AFC_FREQUENCY_INFO(afcChannelAllowance.availableAfcFrequencyInfos[index]);
    }
    afcChannelAllowance.availableAfcChannelInfos.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_AVAILABLE_AFC_CHANNEL_INFO(afcChannelAllowance.availableAfcChannelInfos[index]);
    }
    afcChannelAllowance.availabilityExpireTimeMs = 4720967295876494310;
}

static bool T_WIFI_CHIP_SET_AFC_CHANNEL_ALLOWANCE_REQ()
{
    AfcChannelAllowance afcChannelAllowance;
    INIT_AFC_CHANNEL_ALLOWANCE(afcChannelAllowance);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetAfcChannelAllowanceReq(afcChannelAllowance, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AfcChannelAllowance param;
    bool msgParseResult = WifiChipParseSetAfcChannelAllowanceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(afcChannelAllowance, param, "afcChannelAllowance (AfcChannelAllowance)"));
    return true;
}

static bool T_WIFI_CHIP_SET_AFC_CHANNEL_ALLOWANCE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetAfcChannelAllowanceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetAfcChannelAllowanceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REGISTER_EVENT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRegisterEventCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRegisterEventCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REGISTER_EVENT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRegisterEventCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRegisterEventCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REMOVE_AP_IFACE_REQ()
{
    string ifname = "7Y11JwMYNx";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveApIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseRemoveApIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_REMOVE_AP_IFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveApIfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRemoveApIfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REMOVE_IFACE_INSTANCE_FROM_BRIDGED_AP_IFACE_REQ()
{
    string brIfaceName = "tCpDVu2fN15AAmQ";
    string ifaceInstanceName = "gE7jCSzbHkZiX2Na";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceReq(brIfaceName, ifaceInstanceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RemoveIfaceInstanceFromBridgedApIfaceReqChipParam param;
    bool msgParseResult = WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(brIfaceName, param.brIfaceName, "brIfaceName (string)"));
    VERIFY(COMPARE_STR(ifaceInstanceName, param.ifaceInstanceName, "ifaceInstanceName (string)"));
    return true;
}

static bool T_WIFI_CHIP_REMOVE_IFACE_INSTANCE_FROM_BRIDGED_AP_IFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REMOVE_NAN_IFACE_REQ()
{
    string ifname = "cOguXjv1PDpn";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveNanIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseRemoveNanIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_REMOVE_NAN_IFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveNanIfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRemoveNanIfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REMOVE_P2P_IFACE_REQ()
{
    string ifname = "c9ajO4JZOAs";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveP2pIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseRemoveP2pIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_REMOVE_P2P_IFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveP2pIfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRemoveP2pIfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REMOVE_STA_IFACE_REQ()
{
    string ifname = "ntaA12PSbm";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveStaIfaceReq(ifname, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseRemoveStaIfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifname, param, "ifname (string)"));
    return true;
}

static bool T_WIFI_CHIP_REMOVE_STA_IFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRemoveStaIfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRemoveStaIfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REQUEST_CHIP_DEBUG_INFO_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestChipDebugInfoReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRequestChipDebugInfoReq(payload.data(), payload.size());
    return true;
}

static void INIT_CHIP_DEBUG_INFO(IWifiChip::ChipDebugInfo& result)
{
    result.driverDescription = "aNVIgb9Ji";
    result.firmwareDescription = "eoa83A9d9Eca";
}

static bool T_WIFI_CHIP_REQUEST_CHIP_DEBUG_INFO_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    IWifiChip::ChipDebugInfo result;
    INIT_CHIP_DEBUG_INFO(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestChipDebugInfoCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RequestChipDebugInfoCfmChipParam param;
    bool msgParseResult = WifiChipParseRequestChipDebugInfoCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (IWifiChip::ChipDebugInfo)"));
    return true;
}

static bool T_WIFI_CHIP_REQUEST_DRIVER_DEBUG_DUMP_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestDriverDebugDumpReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRequestDriverDebugDumpReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REQUEST_DRIVER_DEBUG_DUMP_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x9f;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestDriverDebugDumpCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RequestDriverDebugDumpCfmChipParam param;
    bool msgParseResult = WifiChipParseRequestDriverDebugDumpCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_CHIP_REQUEST_FIRMWARE_DEBUG_DUMP_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestFirmwareDebugDumpReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseRequestFirmwareDebugDumpReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_REQUEST_FIRMWARE_DEBUG_DUMP_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x07;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeRequestFirmwareDebugDumpCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RequestFirmwareDebugDumpCfmChipParam param;
    bool msgParseResult = WifiChipParseRequestFirmwareDebugDumpCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_CHIP_RESET_TX_POWER_SCENARIO_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeResetTxPowerScenarioReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseResetTxPowerScenarioReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_RESET_TX_POWER_SCENARIO_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeResetTxPowerScenarioCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseResetTxPowerScenarioCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SELECT_TX_POWER_SCENARIO_REQ()
{
    IWifiChip::TxPowerScenario scenario = IWifiChip::TxPowerScenario(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSelectTxPowerScenarioReq(scenario, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IWifiChip::TxPowerScenario param;
    bool msgParseResult = WifiChipParseSelectTxPowerScenarioReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&scenario, &param, sizeof(IWifiChip::TxPowerScenario), "scenario (IWifiChip::TxPowerScenario)"));
    return true;
}

static bool T_WIFI_CHIP_SELECT_TX_POWER_SCENARIO_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSelectTxPowerScenarioCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSelectTxPowerScenarioCfm(payload.data(), payload.size());
    return true;
}

static void INIT_COEX_UNSAFE_CHANNEL(IWifiChip::CoexUnsafeChannel& unsafeChannels)
{
    unsafeChannels.band = WifiBand(1);
    unsafeChannels.channel = 74;
    unsafeChannels.powerCapDbm = 46;
}

static bool T_WIFI_CHIP_SET_COEX_UNSAFE_CHANNELS_REQ()
{
    vector<IWifiChip::CoexUnsafeChannel> unsafeChannels;
    unsafeChannels.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_COEX_UNSAFE_CHANNEL(unsafeChannels[index]);
    }
    int32_t restrictions = 37;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetCoexUnsafeChannelsReq(unsafeChannels, restrictions, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetCoexUnsafeChannelsReqChipParam param;
    bool msgParseResult = WifiChipParseSetCoexUnsafeChannelsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(unsafeChannels, param.unsafeChannels, "unsafeChannels (vector<IWifiChip::CoexUnsafeChannel>)"));
    VERIFY(COMPARE_BUF(&restrictions, &param.restrictions, sizeof(int32_t), "restrictions (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_SET_COEX_UNSAFE_CHANNELS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetCoexUnsafeChannelsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetCoexUnsafeChannelsCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SET_COUNTRY_CODE_REQ()
{
    array<uint8_t, 2> code;
    for (int index = 0; index < 2; index++)
    {
        code[index] = 0x3e;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetCountryCodeReq(code, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    array<uint8_t, 2> param;
    bool msgParseResult = WifiChipParseSetCountryCodeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_ARY(code, param, "code (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_CHIP_SET_COUNTRY_CODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetCountryCodeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetCountryCodeCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SET_LATENCY_MODE_REQ()
{
    IWifiChip::LatencyMode mode = IWifiChip::LatencyMode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetLatencyModeReq(mode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IWifiChip::LatencyMode param;
    bool msgParseResult = WifiChipParseSetLatencyModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&mode, &param, sizeof(IWifiChip::LatencyMode), "mode (IWifiChip::LatencyMode)"));
    return true;
}

static bool T_WIFI_CHIP_SET_LATENCY_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetLatencyModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetLatencyModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SET_MULTI_STA_PRIMARY_CONNECTION_REQ()
{
    string ifName = "FkSvLk";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMultiStaPrimaryConnectionReq(ifName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = WifiChipParseSetMultiStaPrimaryConnectionReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifName, param, "ifName (string)"));
    return true;
}

static bool T_WIFI_CHIP_SET_MULTI_STA_PRIMARY_CONNECTION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMultiStaPrimaryConnectionCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetMultiStaPrimaryConnectionCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SET_MULTI_STA_USE_CASE_REQ()
{
    IWifiChip::MultiStaUseCase useCase = IWifiChip::MultiStaUseCase(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMultiStaUseCaseReq(useCase, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IWifiChip::MultiStaUseCase param;
    bool msgParseResult = WifiChipParseSetMultiStaUseCaseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&useCase, &param, sizeof(IWifiChip::MultiStaUseCase), "useCase (IWifiChip::MultiStaUseCase)"));
    return true;
}

static bool T_WIFI_CHIP_SET_MULTI_STA_USE_CASE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMultiStaUseCaseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetMultiStaUseCaseCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_START_LOGGING_TO_DEBUG_RING_BUFFER_REQ()
{
    string ringName = "FvfU9yLM4";
    WifiDebugRingBufferVerboseLevel verboseLevel = WifiDebugRingBufferVerboseLevel(1);
    int32_t maxIntervalInSec = 28;
    int32_t minDataSizeInBytes = 102;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeStartLoggingToDebugRingBufferReq(ringName, verboseLevel, maxIntervalInSec, minDataSizeInBytes, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartLoggingToDebugRingBufferReqChipParam param;
    bool msgParseResult = WifiChipParseStartLoggingToDebugRingBufferReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ringName, param.ringName, "ringName (string)"));
    VERIFY(COMPARE_BUF(&verboseLevel, &param.verboseLevel, sizeof(WifiDebugRingBufferVerboseLevel), "verboseLevel (WifiDebugRingBufferVerboseLevel)"));
    VERIFY(COMPARE_BUF(&maxIntervalInSec, &param.maxIntervalInSec, sizeof(int32_t), "maxIntervalInSec (int32_t)"));
    VERIFY(COMPARE_BUF(&minDataSizeInBytes, &param.minDataSizeInBytes, sizeof(int32_t), "minDataSizeInBytes (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_START_LOGGING_TO_DEBUG_RING_BUFFER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeStartLoggingToDebugRingBufferCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseStartLoggingToDebugRingBufferCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_STOP_LOGGING_TO_DEBUG_RING_BUFFER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeStopLoggingToDebugRingBufferReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseStopLoggingToDebugRingBufferReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_STOP_LOGGING_TO_DEBUG_RING_BUFFER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeStopLoggingToDebugRingBufferCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseStopLoggingToDebugRingBufferCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_TRIGGER_SUBSYSTEM_RESTART_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeTriggerSubsystemRestartReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseTriggerSubsystemRestartReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_TRIGGER_SUBSYSTEM_RESTART_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeTriggerSubsystemRestartCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseTriggerSubsystemRestartCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_ENABLE_STA_CHANNEL_FOR_PEER_NETWORK_REQ()
{
    int32_t channelCategoryEnableFlag = 37;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeEnableStaChannelForPeerNetworkReq(channelCategoryEnableFlag, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiChipParseEnableStaChannelForPeerNetworkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&channelCategoryEnableFlag, &param, sizeof(int32_t), "channelCategoryEnableFlag (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_ENABLE_STA_CHANNEL_FOR_PEER_NETWORK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeEnableStaChannelForPeerNetworkCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseEnableStaChannelForPeerNetworkCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_SET_MLO_MODE_REQ()
{
    IWifiChip::ChipMloMode mode = IWifiChip::ChipMloMode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMloModeReq(mode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IWifiChip::ChipMloMode param;
    bool msgParseResult = WifiChipParseSetMloModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&mode, &param, sizeof(IWifiChip::ChipMloMode), "mode (IWifiChip::ChipMloMode)"));
    return true;
}

static bool T_WIFI_CHIP_SET_MLO_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeSetMloModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiChipParseSetMloModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_CHIP_ON_CHIP_RECONFIGURE_FAILURE_IND()
{
    WifiStatusCode status = WifiStatusCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnChipReconfigureFailureInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    WifiStatusCode param;
    bool msgParseResult = WifiChipParseOnChipReconfigureFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param, sizeof(WifiStatusCode), "status (WifiStatusCode)"));
    return true;
}

static bool T_WIFI_CHIP_ON_CHIP_RECONFIGURED_IND()
{
    int32_t modeId = 20;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnChipReconfiguredInd(modeId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiChipParseOnChipReconfiguredInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&modeId, &param, sizeof(int32_t), "modeId (int32_t)"));
    return true;
}

static bool T_WIFI_CHIP_ON_DEBUG_ERROR_ALERT_IND()
{
    int32_t errorCode = 122;
    vector<uint8_t> debugData;
    debugData.resize(4);
    for (int index = 0; index < 4; index++)
    {
        debugData[index] = 0xdd;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnDebugErrorAlertInd(errorCode, debugData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDebugErrorAlertIndChipParam param;
    bool msgParseResult = WifiChipParseOnDebugErrorAlertInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&errorCode, &param.errorCode, sizeof(int32_t), "errorCode (int32_t)"));
    VERIFY(COMPARE_VEC(debugData, param.debugData, "debugData (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_CHIP_ON_DEBUG_RING_BUFFER_DATA_AVAILABLE_IND()
{
    WifiDebugRingBufferStatus status;
    INIT_WIFI_DEBUG_RING_BUFFER_STATUS(status);
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0x48;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnDebugRingBufferDataAvailableInd(status, data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDebugRingBufferDataAvailableIndChipParam param;
    bool msgParseResult = WifiChipParseOnDebugRingBufferDataAvailableInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(status, param.status, "status (WifiDebugRingBufferStatus)"));
    VERIFY(COMPARE_VEC(data, param.data, "data (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_CHIP_ON_IFACE_ADDED_IND()
{
    IfaceType type = IfaceType(1);
    string name = "lORKytxra3AoZX";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnIfaceAddedInd(type, name, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnIfaceAddedIndChipParam param;
    bool msgParseResult = WifiChipParseOnIfaceAddedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&type, &param.type, sizeof(IfaceType), "type (IfaceType)"));
    VERIFY(COMPARE_STR(name, param.name, "name (string)"));
    return true;
}

static bool T_WIFI_CHIP_ON_IFACE_REMOVED_IND()
{
    IfaceType type = IfaceType(1);
    string name = "FEgGsF79Cw";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnIfaceRemovedInd(type, name, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnIfaceRemovedIndChipParam param;
    bool msgParseResult = WifiChipParseOnIfaceRemovedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&type, &param.type, sizeof(IfaceType), "type (IfaceType)"));
    VERIFY(COMPARE_STR(name, param.name, "name (string)"));
    return true;
}

static void INIT_IFACE_INFO(IWifiChipEventCallback::IfaceInfo& ifaceInfos)
{
    ifaceInfos.name = "i4A";
    ifaceInfos.channel = 110;
}

static void INIT_RADIO_MODE_INFO(IWifiChipEventCallback::RadioModeInfo& radioModeInfos)
{
    radioModeInfos.radioId = 35;
    radioModeInfos.bandInfo = WifiBand(1);
    radioModeInfos.ifaceInfos.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_IFACE_INFO(radioModeInfos.ifaceInfos[index]);
    }
}

static bool T_WIFI_CHIP_ON_RADIO_MODE_CHANGE_IND()
{
    vector<IWifiChipEventCallback::RadioModeInfo> radioModeInfos;
    radioModeInfos.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_RADIO_MODE_INFO(radioModeInfos[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiChipSerializeOnRadioModeChangeInd(radioModeInfos, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<IWifiChipEventCallback::RadioModeInfo> param;
    bool msgParseResult = WifiChipParseOnRadioModeChangeInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(radioModeInfos, param, "radioModeInfos (vector<IWifiChipEventCallback::RadioModeInfo>)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "hrZFk7BVahFkk";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static void INIT_NAN_BAND_SPECIFIC_CONFIG(NanBandSpecificConfig& bandSpecificConfig)
{
    bandSpecificConfig.rssiClose = 0xb0;
    bandSpecificConfig.rssiMiddle = 0x1a;
    bandSpecificConfig.rssiCloseProximity = 0xbe;
    bandSpecificConfig.dwellTimeMs = 'G';
    bandSpecificConfig.scanPeriodSec = 'U';
    bandSpecificConfig.validDiscoveryWindowIntervalVal = false;
    bandSpecificConfig.discoveryWindowIntervalVal = 0x2d;
}

static void INIT_NAN_CONFIG_REQUEST(NanConfigRequest& msg1)
{
    msg1.masterPref = 0x51;
    msg1.disableDiscoveryAddressChangeIndication = true;
    msg1.disableStartedClusterIndication = false;
    msg1.disableJoinedClusterIndication = false;
    msg1.includePublishServiceIdsInBeacon = true;
    msg1.numberOfPublishServiceIdsInBeacon = 0x70;
    msg1.includeSubscribeServiceIdsInBeacon = false;
    msg1.numberOfSubscribeServiceIdsInBeacon = 0xef;
    msg1.rssiWindowSize = 'L';
    msg1.macAddressRandomizationIntervalSec = 86;
    for (int index = 0; index < 3; index++)
    {
        INIT_NAN_BAND_SPECIFIC_CONFIG(msg1.bandSpecificConfig[index]);
    }
}

static void INIT_NAN_CONFIG_REQUEST_SUPPLEMENTAL(NanConfigRequestSupplemental& msg2)
{
    msg2.discoveryBeaconIntervalMs = 100;
    msg2.numberOfSpatialStreamsInDiscovery = 61;
    msg2.enableDiscoveryWindowEarlyTermination = false;
    msg2.enableRanging = true;
    msg2.enableInstantCommunicationMode = false;
    msg2.instantModeChannel = 111;
    msg2.clusterId = 13;
}

static bool T_WIFI_NAN_IFACE_CONFIG_REQUEST_REQ()
{
    char16_t cmdId = 'V';
    NanConfigRequest msg1;
    INIT_NAN_CONFIG_REQUEST(msg1);
    NanConfigRequestSupplemental msg2;
    INIT_NAN_CONFIG_REQUEST_SUPPLEMENTAL(msg2);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeConfigRequestReq(cmdId, msg1, msg2, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ConfigRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseConfigRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg1, param.msg1, "msg1 (NanConfigRequest)"));
    VERIFY(COMPARE_OBJ(msg2, param.msg2, "msg2 (NanConfigRequestSupplemental)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_CONFIG_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeConfigRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseConfigRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_CREATE_DATA_INTERFACE_REQUEST_REQ()
{
    char16_t cmdId = 'O';
    string ifaceName = "DYM6OH6hGIbi";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeCreateDataInterfaceRequestReq(cmdId, ifaceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateDataInterfaceRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseCreateDataInterfaceRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_CREATE_DATA_INTERFACE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeCreateDataInterfaceRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseCreateDataInterfaceRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_DELETE_DATA_INTERFACE_REQUEST_REQ()
{
    char16_t cmdId = 'z';
    string ifaceName = "Ed8mFQv56ji";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeDeleteDataInterfaceRequestReq(cmdId, ifaceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DeleteDataInterfaceRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseDeleteDataInterfaceRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_DELETE_DATA_INTERFACE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeDeleteDataInterfaceRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseDeleteDataInterfaceRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_DISABLE_REQUEST_REQ()
{
    char16_t cmdId = 'b';
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeDisableRequestReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    char16_t param;
    bool msgParseResult = WifiNanIfaceParseDisableRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(char16_t), "cmdId (char16_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_DISABLE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeDisableRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseDisableRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_DEBUG_CONFIG(NanDebugConfig& debugConfigs)
{
    debugConfigs.validClusterIdVals = false;
    debugConfigs.clusterIdBottomRangeVal = '3';
    debugConfigs.clusterIdTopRangeVal = '4';
    debugConfigs.validIntfAddrVal = false;
    for (int index = 0; index < 6; index++)
    {
        debugConfigs.intfAddrVal[index] = 0x9b;
    }
    debugConfigs.validOuiVal = false;
    debugConfigs.ouiVal = 82;
    debugConfigs.validRandomFactorForceVal = false;
    debugConfigs.randomFactorForceVal = 0x50;
    debugConfigs.validHopCountForceVal = true;
    debugConfigs.hopCountForceVal = 0xcb;
    debugConfigs.validDiscoveryChannelVal = true;
    for (int index = 0; index < 3; index++)
    {
        debugConfigs.discoveryChannelMhzVal[index] = 53;
    }
    debugConfigs.validUseBeaconsInBandVal = true;
    for (int index = 0; index < 3; index++)
    {
        debugConfigs.useBeaconsInBandVal[index] = true;
    }
    debugConfigs.validUseSdfInBandVal = true;
    for (int index = 0; index < 3; index++)
    {
        debugConfigs.useSdfInBandVal[index] = false;
    }
}

static void INIT_NAN_ENABLE_REQUEST(NanEnableRequest& msg1)
{
    for (int index = 0; index < 3; index++)
    {
        msg1.operateInBand[index] = false;
    }
    msg1.hopCountMax = 0xa5;
    INIT_NAN_CONFIG_REQUEST(msg1.configParams);
    INIT_NAN_DEBUG_CONFIG(msg1.debugConfigs);
}

static bool T_WIFI_NAN_IFACE_ENABLE_REQUEST_REQ()
{
    char16_t cmdId = 'k';
    NanEnableRequest msg1;
    INIT_NAN_ENABLE_REQUEST(msg1);
    NanConfigRequestSupplemental msg2;
    INIT_NAN_CONFIG_REQUEST_SUPPLEMENTAL(msg2);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEnableRequestReq(cmdId, msg1, msg2, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EnableRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseEnableRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg1, param.msg1, "msg1 (NanEnableRequest)"));
    VERIFY(COMPARE_OBJ(msg2, param.msg2, "msg2 (NanConfigRequestSupplemental)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_ENABLE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEnableRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseEnableRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_GET_CAPABILITIES_REQUEST_REQ()
{
    char16_t cmdId = 'Y';
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeGetCapabilitiesRequestReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    char16_t param;
    bool msgParseResult = WifiNanIfaceParseGetCapabilitiesRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(char16_t), "cmdId (char16_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_GET_CAPABILITIES_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeGetCapabilitiesRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseGetCapabilitiesRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_DATA_PATH_SECURITY_CONFIG(NanDataPathSecurityConfig& securityConfig)
{
    securityConfig.securityType = NanDataPathSecurityType(1);
    securityConfig.cipherType = NanCipherSuiteType(1);
    for (int index = 0; index < 32; index++)
    {
        securityConfig.pmk[index] = 0x44;
    }
    securityConfig.passphrase.resize(4);
    for (int index = 0; index < 4; index++)
    {
        securityConfig.passphrase[index] = 0x49;
    }
    for (int index = 0; index < 16; index++)
    {
        securityConfig.scid[index] = 0x17;
    }
}

static void INIT_NAN_INITIATE_DATA_PATH_REQUEST(NanInitiateDataPathRequest& msg)
{
    msg.peerId = 94;
    for (int index = 0; index < 6; index++)
    {
        msg.peerDiscMacAddr[index] = 0x44;
    }
    msg.channelRequestType = NanDataPathChannelCfg(1);
    msg.channel = 113;
    msg.ifaceName = "ze16cdabtL2keK4";
    INIT_NAN_DATA_PATH_SECURITY_CONFIG(msg.securityConfig);
    msg.appInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.appInfo[index] = 0x22;
    }
    msg.serviceNameOutOfBand.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.serviceNameOutOfBand[index] = 0x6f;
    }
    msg.discoverySessionId = 0x5c;
}

static bool T_WIFI_NAN_IFACE_INITIATE_DATA_PATH_REQUEST_REQ()
{
    char16_t cmdId = 'E';
    NanInitiateDataPathRequest msg;
    INIT_NAN_INITIATE_DATA_PATH_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiateDataPathRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InitiateDataPathRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseInitiateDataPathRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanInitiateDataPathRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_INITIATE_DATA_PATH_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiateDataPathRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseInitiateDataPathRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_REGISTER_EVENT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRegisterEventCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseRegisterEventCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_REGISTER_EVENT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRegisterEventCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseRegisterEventCallbackCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_RESPOND_TO_DATA_PATH_INDICATION_REQUEST(NanRespondToDataPathIndicationRequest& msg)
{
    msg.acceptRequest = true;
    msg.ndpInstanceId = 11;
    msg.ifaceName = "sQhIuJhm7PMj";
    INIT_NAN_DATA_PATH_SECURITY_CONFIG(msg.securityConfig);
    msg.appInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.appInfo[index] = 0xfe;
    }
    msg.serviceNameOutOfBand.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.serviceNameOutOfBand[index] = 0xf1;
    }
    msg.discoverySessionId = 0xbc;
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_DATA_PATH_INDICATION_REQUEST_REQ()
{
    char16_t cmdId = 'W';
    NanRespondToDataPathIndicationRequest msg;
    INIT_NAN_RESPOND_TO_DATA_PATH_INDICATION_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToDataPathIndicationRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RespondToDataPathIndicationRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseRespondToDataPathIndicationRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanRespondToDataPathIndicationRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_DATA_PATH_INDICATION_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToDataPathIndicationRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseRespondToDataPathIndicationRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_DISCOVERY_COMMON_CONFIG(NanDiscoveryCommonConfig& baseConfigs)
{
    baseConfigs.sessionId = 0xc7;
    baseConfigs.ttlSec = 't';
    baseConfigs.discoveryWindowPeriod = 54;
    baseConfigs.discoveryCount = 0xfb;
    baseConfigs.serviceName.resize(4);
    for (int index = 0; index < 4; index++)
    {
        baseConfigs.serviceName[index] = 0x90;
    }
    baseConfigs.discoveryMatchIndicator = NanMatchAlg(1);
    baseConfigs.serviceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        baseConfigs.serviceSpecificInfo[index] = 0xc2;
    }
    baseConfigs.extendedServiceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        baseConfigs.extendedServiceSpecificInfo[index] = 0xd0;
    }
    baseConfigs.rxMatchFilter.resize(4);
    for (int index = 0; index < 4; index++)
    {
        baseConfigs.rxMatchFilter[index] = 0x1d;
    }
    baseConfigs.txMatchFilter.resize(4);
    for (int index = 0; index < 4; index++)
    {
        baseConfigs.txMatchFilter[index] = 0xa5;
    }
    baseConfigs.useRssiThreshold = false;
    baseConfigs.disableDiscoveryTerminationIndication = true;
    baseConfigs.disableMatchExpirationIndication = false;
    baseConfigs.disableFollowupReceivedIndication = false;
    INIT_NAN_DATA_PATH_SECURITY_CONFIG(baseConfigs.securityConfig);
    baseConfigs.rangingRequired = false;
    baseConfigs.rangingIntervalMs = 109;
    baseConfigs.configRangingIndications = 70;
    baseConfigs.distanceIngressCm = 'c';
    baseConfigs.distanceEgressCm = 'v';
    baseConfigs.enableSessionSuspendability = true;
}

static void INIT_NAN_PAIRING_CONFIG(NanPairingConfig& pairingConfig)
{
    pairingConfig.enablePairingSetup = true;
    pairingConfig.enablePairingCache = true;
    pairingConfig.enablePairingVerification = false;
    pairingConfig.supportedBootstrappingMethods = 65;
}

static void INIT_NAN_PUBLISH_REQUEST(NanPublishRequest& msg)
{
    INIT_NAN_DISCOVERY_COMMON_CONFIG(msg.baseConfigs);
    msg.publishType = NanPublishType(1);
    msg.txType = NanTxType(1);
    msg.autoAcceptDataPathRequests = true;
    INIT_NAN_PAIRING_CONFIG(msg.pairingConfig);
    for (int index = 0; index < 16; index++)
    {
        msg.identityKey[index] = 0xe5;
    }
}

static bool T_WIFI_NAN_IFACE_START_PUBLISH_REQUEST_REQ()
{
    char16_t cmdId = 't';
    NanPublishRequest msg;
    INIT_NAN_PUBLISH_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStartPublishRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartPublishRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseStartPublishRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanPublishRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_START_PUBLISH_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStartPublishRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseStartPublishRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_MAC_ADDRESS(MacAddress& intfAddr)
{
    for (int index = 0; index < 6; index++)
    {
        intfAddr.data[index] = 0xbd;
    }
}

static void INIT_NAN_SUBSCRIBE_REQUEST(NanSubscribeRequest& msg)
{
    INIT_NAN_DISCOVERY_COMMON_CONFIG(msg.baseConfigs);
    msg.subscribeType = NanSubscribeType(1);
    msg.srfType = NanSrfType(1);
    msg.srfRespondIfInAddressSet = false;
    msg.shouldUseSrf = true;
    msg.isSsiRequiredForMatch = false;
    msg.intfAddr.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MAC_ADDRESS(msg.intfAddr[index]);
    }
    INIT_NAN_PAIRING_CONFIG(msg.pairingConfig);
    for (int index = 0; index < 16; index++)
    {
        msg.identityKey[index] = 0x84;
    }
}

static bool T_WIFI_NAN_IFACE_START_SUBSCRIBE_REQUEST_REQ()
{
    char16_t cmdId = 'G';
    NanSubscribeRequest msg;
    INIT_NAN_SUBSCRIBE_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStartSubscribeRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartSubscribeRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseStartSubscribeRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanSubscribeRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_START_SUBSCRIBE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStartSubscribeRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseStartSubscribeRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_STOP_PUBLISH_REQUEST_REQ()
{
    char16_t cmdId = 'p';
    int8_t sessionId = 0x2a;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStopPublishRequestReq(cmdId, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StopPublishRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseStopPublishRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_STOP_PUBLISH_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStopPublishRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseStopPublishRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_STOP_SUBSCRIBE_REQUEST_REQ()
{
    char16_t cmdId = 'j';
    int8_t sessionId = 0x69;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStopSubscribeRequestReq(cmdId, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StopSubscribeRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseStopSubscribeRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_STOP_SUBSCRIBE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeStopSubscribeRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseStopSubscribeRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_TERMINATE_DATA_PATH_REQUEST_REQ()
{
    char16_t cmdId = 'o';
    int32_t ndpInstanceId = 41;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTerminateDataPathRequestReq(cmdId, ndpInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    TerminateDataPathRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseTerminateDataPathRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&ndpInstanceId, &param.ndpInstanceId, sizeof(int32_t), "ndpInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_TERMINATE_DATA_PATH_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTerminateDataPathRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseTerminateDataPathRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_SUSPEND_REQUEST_REQ()
{
    char16_t cmdId = 'd';
    int8_t sessionId = 0xc4;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeSuspendRequestReq(cmdId, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SuspendRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseSuspendRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_SUSPEND_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeSuspendRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseSuspendRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_RESUME_REQUEST_REQ()
{
    char16_t cmdId = 'Y';
    int8_t sessionId = 0xaf;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeResumeRequestReq(cmdId, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ResumeRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseResumeRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_RESUME_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeResumeRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseResumeRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_TRANSMIT_FOLLOWUP_REQUEST(NanTransmitFollowupRequest& msg)
{
    msg.discoverySessionId = 0x66;
    msg.peerId = 50;
    for (int index = 0; index < 6; index++)
    {
        msg.addr[index] = 0x92;
    }
    msg.isHighPriority = true;
    msg.shouldUseDiscoveryWindow = false;
    msg.serviceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.serviceSpecificInfo[index] = 0x6e;
    }
    msg.extendedServiceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.extendedServiceSpecificInfo[index] = 0xf6;
    }
    msg.disableFollowupResultIndication = false;
}

static bool T_WIFI_NAN_IFACE_TRANSMIT_FOLLOWUP_REQUEST_REQ()
{
    char16_t cmdId = 's';
    NanTransmitFollowupRequest msg;
    INIT_NAN_TRANSMIT_FOLLOWUP_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTransmitFollowupRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    TransmitFollowupRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseTransmitFollowupRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanTransmitFollowupRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_TRANSMIT_FOLLOWUP_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTransmitFollowupRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseTransmitFollowupRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_PAIRING_SECURITY_CONFIG(NanPairingSecurityConfig& securityConfig)
{
    securityConfig.securityType = NanPairingSecurityType(1);
    for (int index = 0; index < 32; index++)
    {
        securityConfig.pmk[index] = 0xd1;
    }
    securityConfig.passphrase.resize(4);
    for (int index = 0; index < 4; index++)
    {
        securityConfig.passphrase[index] = 0xb8;
    }
    securityConfig.akm = NanPairingAkm(1);
    securityConfig.cipherType = NanCipherSuiteType(1);
}

static void INIT_NAN_PAIRING_REQUEST(NanPairingRequest& msg)
{
    msg.peerId = 61;
    for (int index = 0; index < 6; index++)
    {
        msg.peerDiscMacAddr[index] = 0x35;
    }
    msg.requestType = NanPairingRequestType(1);
    msg.enablePairingCache = true;
    for (int index = 0; index < 16; index++)
    {
        msg.pairingIdentityKey[index] = 0x15;
    }
    INIT_NAN_PAIRING_SECURITY_CONFIG(msg.securityConfig);
}

static bool T_WIFI_NAN_IFACE_INITIATE_PAIRING_REQUEST_REQ()
{
    char16_t cmdId = 'o';
    NanPairingRequest msg;
    INIT_NAN_PAIRING_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiatePairingRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InitiatePairingRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseInitiatePairingRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanPairingRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_INITIATE_PAIRING_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiatePairingRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseInitiatePairingRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_RESPOND_TO_PAIRING_INDICATION_REQUEST(NanRespondToPairingIndicationRequest& msg)
{
    msg.acceptRequest = false;
    msg.pairingInstanceId = 119;
    msg.requestType = NanPairingRequestType(1);
    msg.enablePairingCache = false;
    for (int index = 0; index < 16; index++)
    {
        msg.pairingIdentityKey[index] = 0x65;
    }
    INIT_NAN_PAIRING_SECURITY_CONFIG(msg.securityConfig);
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_PAIRING_INDICATION_REQUEST_REQ()
{
    char16_t cmdId = 'D';
    NanRespondToPairingIndicationRequest msg;
    INIT_NAN_RESPOND_TO_PAIRING_INDICATION_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToPairingIndicationRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RespondToPairingIndicationRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseRespondToPairingIndicationRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanRespondToPairingIndicationRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_PAIRING_INDICATION_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToPairingIndicationRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseRespondToPairingIndicationRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_BOOTSTRAPPING_REQUEST(NanBootstrappingRequest& msg)
{
    msg.peerId = 13;
    for (int index = 0; index < 6; index++)
    {
        msg.peerDiscMacAddr[index] = 0x7b;
    }
    msg.requestBootstrappingMethod = NanBootstrappingMethod(1);
    msg.cookie.resize(4);
    for (int index = 0; index < 4; index++)
    {
        msg.cookie[index] = 0x3c;
    }
}

static bool T_WIFI_NAN_IFACE_INITIATE_BOOTSTRAPPING_REQUEST_REQ()
{
    char16_t cmdId = 'm';
    NanBootstrappingRequest msg;
    INIT_NAN_BOOTSTRAPPING_REQUEST(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiateBootstrappingRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InitiateBootstrappingRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseInitiateBootstrappingRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanBootstrappingRequest)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_INITIATE_BOOTSTRAPPING_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeInitiateBootstrappingRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseInitiateBootstrappingRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_BOOTSTRAPPING_RESPONSE(NanBootstrappingResponse& msg)
{
    msg.bootstrappingInstanceId = 75;
    msg.acceptRequest = true;
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_BOOTSTRAPPING_INDICATION_REQUEST_REQ()
{
    char16_t cmdId = '5';
    NanBootstrappingResponse msg;
    INIT_NAN_BOOTSTRAPPING_RESPONSE(msg);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestReq(cmdId, msg, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RespondToBootstrappingIndicationRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseRespondToBootstrappingIndicationRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_OBJ(msg, param.msg, "msg (NanBootstrappingResponse)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_RESPOND_TO_BOOTSTRAPPING_INDICATION_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseRespondToBootstrappingIndicationRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_NAN_IFACE_TERMINATE_PAIRING_REQUEST_REQ()
{
    char16_t cmdId = '6';
    int32_t pairingInstanceId = 52;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTerminatePairingRequestReq(cmdId, pairingInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    TerminatePairingRequestReqNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseTerminatePairingRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(char16_t), "cmdId (char16_t)"));
    VERIFY(COMPARE_BUF(&pairingInstanceId, &param.pairingInstanceId, sizeof(int32_t), "pairingInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_TERMINATE_PAIRING_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeTerminatePairingRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiNanIfaceParseTerminatePairingRequestCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NAN_CLUSTER_EVENT_IND(NanClusterEventInd& event)
{
    event.eventType = NanClusterEventType(1);
    for (int index = 0; index < 6; index++)
    {
        event.addr[index] = 0xf9;
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_CLUSTER_EVENT_IND()
{
    NanClusterEventInd event;
    INIT_NAN_CLUSTER_EVENT_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventClusterEventInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanClusterEventInd param;
    bool msgParseResult = WifiNanIfaceParseEventClusterEventInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanClusterEventInd)"));
    return true;
}

static void INIT_NAN_STATUS(NanStatus& status)
{
    status.status = NanStatusCode(1);
    status.description = "ZYemP";
}

static void INIT_NAN_DATA_PATH_CHANNEL_INFO(NanDataPathChannelInfo& channelInfo)
{
    channelInfo.channelFreq = 38;
    channelInfo.channelBandwidth = WifiChannelWidthInMhz(1);
    channelInfo.numSpatialStreams = 50;
}

static void INIT_NAN_DATA_PATH_CONFIRM_IND(NanDataPathConfirmInd& event)
{
    event.ndpInstanceId = 20;
    event.dataPathSetupSuccess = false;
    for (int index = 0; index < 6; index++)
    {
        event.peerNdiMacAddr[index] = 0x7a;
    }
    event.appInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.appInfo[index] = 0x40;
    }
    INIT_NAN_STATUS(event.status);
    event.channelInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_NAN_DATA_PATH_CHANNEL_INFO(event.channelInfo[index]);
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_DATA_PATH_CONFIRM_IND()
{
    NanDataPathConfirmInd event;
    INIT_NAN_DATA_PATH_CONFIRM_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventDataPathConfirmInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanDataPathConfirmInd param;
    bool msgParseResult = WifiNanIfaceParseEventDataPathConfirmInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanDataPathConfirmInd)"));
    return true;
}

static void INIT_NAN_DATA_PATH_REQUEST_IND(NanDataPathRequestInd& event)
{
    event.discoverySessionId = 0x06;
    for (int index = 0; index < 6; index++)
    {
        event.peerDiscMacAddr[index] = 0x01;
    }
    event.ndpInstanceId = 96;
    event.securityRequired = true;
    event.appInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.appInfo[index] = 0x8d;
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_DATA_PATH_REQUEST_IND()
{
    NanDataPathRequestInd event;
    INIT_NAN_DATA_PATH_REQUEST_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventDataPathRequestInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanDataPathRequestInd param;
    bool msgParseResult = WifiNanIfaceParseEventDataPathRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanDataPathRequestInd)"));
    return true;
}

static void INIT_NAN_DATA_PATH_SCHEDULE_UPDATE_IND(NanDataPathScheduleUpdateInd& event)
{
    for (int index = 0; index < 6; index++)
    {
        event.peerDiscoveryAddress[index] = 0xe4;
    }
    event.channelInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_NAN_DATA_PATH_CHANNEL_INFO(event.channelInfo[index]);
    }
    event.ndpInstanceIds.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.ndpInstanceIds[index] = 58;
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_DATA_PATH_SCHEDULE_UPDATE_IND()
{
    NanDataPathScheduleUpdateInd event;
    INIT_NAN_DATA_PATH_SCHEDULE_UPDATE_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventDataPathScheduleUpdateInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanDataPathScheduleUpdateInd param;
    bool msgParseResult = WifiNanIfaceParseEventDataPathScheduleUpdateInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanDataPathScheduleUpdateInd)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_DATA_PATH_TERMINATED_IND()
{
    int32_t ndpInstanceId = 118;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventDataPathTerminatedInd(ndpInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiNanIfaceParseEventDataPathTerminatedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&ndpInstanceId, &param, sizeof(int32_t), "ndpInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_DISABLED_IND()
{
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventDisabledInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanStatus param;
    bool msgParseResult = WifiNanIfaceParseEventDisabledInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(status, param, "status (NanStatus)"));
    return true;
}

static void INIT_NAN_FOLLOWUP_RECEIVED_IND(NanFollowupReceivedInd& event)
{
    event.discoverySessionId = 0xb2;
    event.peerId = 20;
    for (int index = 0; index < 6; index++)
    {
        event.addr[index] = 0xcf;
    }
    event.receivedInFaw = true;
    event.serviceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.serviceSpecificInfo[index] = 0xe9;
    }
    event.extendedServiceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.extendedServiceSpecificInfo[index] = 0xcb;
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_FOLLOWUP_RECEIVED_IND()
{
    NanFollowupReceivedInd event;
    INIT_NAN_FOLLOWUP_RECEIVED_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventFollowupReceivedInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanFollowupReceivedInd param;
    bool msgParseResult = WifiNanIfaceParseEventFollowupReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanFollowupReceivedInd)"));
    return true;
}

static void INIT_NAN_IDENTITY_RESOLUTION_ATTRIBUTE(NanIdentityResolutionAttribute& peerNira)
{
    for (int index = 0; index < 8; index++)
    {
        peerNira.nonce[index] = 0x44;
    }
    for (int index = 0; index < 8; index++)
    {
        peerNira.tag[index] = 0x34;
    }
}

static void INIT_NAN_MATCH_IND(NanMatchInd& event)
{
    event.discoverySessionId = 0x72;
    event.peerId = 37;
    for (int index = 0; index < 6; index++)
    {
        event.addr[index] = 0x4d;
    }
    event.serviceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.serviceSpecificInfo[index] = 0xd7;
    }
    event.extendedServiceSpecificInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.extendedServiceSpecificInfo[index] = 0xf2;
    }
    event.matchFilter.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.matchFilter[index] = 0xda;
    }
    event.matchOccurredInBeaconFlag = false;
    event.outOfResourceFlag = true;
    event.rssiValue = 0x1a;
    event.peerCipherType = NanCipherSuiteType(1);
    event.peerRequiresSecurityEnabledInNdp = true;
    event.peerRequiresRanging = false;
    event.rangingMeasurementInMm = 36;
    event.rangingIndicationType = 93;
    event.scid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.scid[index] = 0xfb;
    }
    INIT_NAN_PAIRING_CONFIG(event.peerPairingConfig);
    INIT_NAN_IDENTITY_RESOLUTION_ATTRIBUTE(event.peerNira);
}

static bool T_WIFI_NAN_IFACE_EVENT_MATCH_IND()
{
    NanMatchInd event;
    INIT_NAN_MATCH_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventMatchInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanMatchInd param;
    bool msgParseResult = WifiNanIfaceParseEventMatchInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanMatchInd)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_MATCH_EXPIRED_IND()
{
    int8_t discoverySessionId = 0x6c;
    int32_t peerId = 100;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventMatchExpiredInd(discoverySessionId, peerId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EventMatchExpiredIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseEventMatchExpiredInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&discoverySessionId, &param.discoverySessionId, sizeof(int8_t), "discoverySessionId (int8_t)"));
    VERIFY(COMPARE_BUF(&peerId, &param.peerId, sizeof(int32_t), "peerId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_PUBLISH_TERMINATED_IND()
{
    int8_t sessionId = 0x80;
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventPublishTerminatedInd(sessionId, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EventPublishTerminatedIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseEventPublishTerminatedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_SUBSCRIBE_TERMINATED_IND()
{
    int8_t sessionId = 0x7c;
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventSubscribeTerminatedInd(sessionId, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EventSubscribeTerminatedIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseEventSubscribeTerminatedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_EVENT_TRANSMIT_FOLLOWUP_IND()
{
    char16_t id = 'n';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventTransmitFollowupInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EventTransmitFollowupIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseEventTransmitFollowupInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static void INIT_NAN_SUSPENSION_MODE_CHANGE_IND(NanSuspensionModeChangeInd& event)
{
    event.isSuspended = false;
}

static bool T_WIFI_NAN_IFACE_EVENT_SUSPENSION_MODE_CHANGED_IND()
{
    NanSuspensionModeChangeInd event;
    INIT_NAN_SUSPENSION_MODE_CHANGE_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventSuspensionModeChangedInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanSuspensionModeChangeInd param;
    bool msgParseResult = WifiNanIfaceParseEventSuspensionModeChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanSuspensionModeChangeInd)"));
    return true;
}

static void INIT_NAN_CAPABILITIES(NanCapabilities& capabilities)
{
    capabilities.maxConcurrentClusters = 48;
    capabilities.maxPublishes = 31;
    capabilities.maxSubscribes = 78;
    capabilities.maxServiceNameLen = 107;
    capabilities.maxMatchFilterLen = 13;
    capabilities.maxTotalMatchFilterLen = 111;
    capabilities.maxServiceSpecificInfoLen = 127;
    capabilities.maxExtendedServiceSpecificInfoLen = 52;
    capabilities.maxNdiInterfaces = 79;
    capabilities.maxNdpSessions = 51;
    capabilities.maxAppInfoLen = 117;
    capabilities.maxQueuedTransmitFollowupMsgs = 104;
    capabilities.maxSubscribeInterfaceAddresses = 127;
    capabilities.supportedCipherSuites = 96;
    capabilities.instantCommunicationModeSupportFlag = true;
    capabilities.supports6g = false;
    capabilities.supportsHe = true;
    capabilities.supportsPairing = false;
    capabilities.supportsSetClusterId = false;
    capabilities.supportsSuspension = false;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_CAPABILITIES_RESPONSE_IND()
{
    char16_t id = 'm';
    NanStatus status;
    INIT_NAN_STATUS(status);
    NanCapabilities capabilities;
    INIT_NAN_CAPABILITIES(capabilities);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyCapabilitiesResponseInd(id, status, capabilities, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyCapabilitiesResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyCapabilitiesResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_OBJ(capabilities, param.capabilities, "capabilities (NanCapabilities)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_CONFIG_RESPONSE_IND()
{
    char16_t id = 'J';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyConfigResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyConfigResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyConfigResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_CREATE_DATA_INTERFACE_RESPONSE_IND()
{
    char16_t id = '0';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyCreateDataInterfaceResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyCreateDataInterfaceResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyCreateDataInterfaceResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_DELETE_DATA_INTERFACE_RESPONSE_IND()
{
    char16_t id = 'b';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyDeleteDataInterfaceResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyDeleteDataInterfaceResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyDeleteDataInterfaceResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_DISABLE_RESPONSE_IND()
{
    char16_t id = 'h';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyDisableResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyDisableResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyDisableResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_ENABLE_RESPONSE_IND()
{
    char16_t id = 'e';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyEnableResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyEnableResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyEnableResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_INITIATE_DATA_PATH_RESPONSE_IND()
{
    char16_t id = '8';
    NanStatus status;
    INIT_NAN_STATUS(status);
    int32_t ndpInstanceId = 63;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyInitiateDataPathResponseInd(id, status, ndpInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyInitiateDataPathResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyInitiateDataPathResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_BUF(&ndpInstanceId, &param.ndpInstanceId, sizeof(int32_t), "ndpInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_DATA_PATH_INDICATION_RESPONSE_IND()
{
    char16_t id = 'W';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyRespondToDataPathIndicationResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyRespondToDataPathIndicationResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyRespondToDataPathIndicationResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_START_PUBLISH_RESPONSE_IND()
{
    char16_t id = 'Q';
    NanStatus status;
    INIT_NAN_STATUS(status);
    int8_t sessionId = 0x10;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyStartPublishResponseInd(id, status, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyStartPublishResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyStartPublishResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_START_SUBSCRIBE_RESPONSE_IND()
{
    char16_t id = 'H';
    NanStatus status;
    INIT_NAN_STATUS(status);
    int8_t sessionId = 0xbd;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyStartSubscribeResponseInd(id, status, sessionId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyStartSubscribeResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyStartSubscribeResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_BUF(&sessionId, &param.sessionId, sizeof(int8_t), "sessionId (int8_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_STOP_PUBLISH_RESPONSE_IND()
{
    char16_t id = 'C';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyStopPublishResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyStopPublishResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyStopPublishResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_STOP_SUBSCRIBE_RESPONSE_IND()
{
    char16_t id = 'A';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyStopSubscribeResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyStopSubscribeResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyStopSubscribeResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_TERMINATE_DATA_PATH_RESPONSE_IND()
{
    char16_t id = 'D';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyTerminateDataPathResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyTerminateDataPathResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyTerminateDataPathResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_SUSPEND_RESPONSE_IND()
{
    char16_t id = 'l';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifySuspendResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifySuspendResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifySuspendResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_RESUME_RESPONSE_IND()
{
    char16_t id = '9';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyResumeResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyResumeResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyResumeResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_TRANSMIT_FOLLOWUP_RESPONSE_IND()
{
    char16_t id = 'S';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyTransmitFollowupResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyTransmitFollowupResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyTransmitFollowupResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static void INIT_NAN_PAIRING_REQUEST_IND(NanPairingRequestInd& event)
{
    event.discoverySessionId = 0x96;
    event.peerId = 118;
    for (int index = 0; index < 6; index++)
    {
        event.peerDiscMacAddr[index] = 0x7c;
    }
    event.pairingInstanceId = 91;
    event.requestType = NanPairingRequestType(1);
    event.enablePairingCache = true;
    INIT_NAN_IDENTITY_RESOLUTION_ATTRIBUTE(event.peerNira);
}

static bool T_WIFI_NAN_IFACE_EVENT_PAIRING_REQUEST_IND()
{
    NanPairingRequestInd event;
    INIT_NAN_PAIRING_REQUEST_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventPairingRequestInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanPairingRequestInd param;
    bool msgParseResult = WifiNanIfaceParseEventPairingRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanPairingRequestInd)"));
    return true;
}

static void INIT_NPK_SECURITY_ASSOCIATION(NpkSecurityAssociation& npksa)
{
    for (int index = 0; index < 16; index++)
    {
        npksa.peerNanIdentityKey[index] = 0x96;
    }
    for (int index = 0; index < 16; index++)
    {
        npksa.localNanIdentityKey[index] = 0x4b;
    }
    for (int index = 0; index < 32; index++)
    {
        npksa.npk[index] = 0x0e;
    }
    npksa.akm = NanPairingAkm(1);
    npksa.cipherType = NanCipherSuiteType(1);
}

static void INIT_NAN_PAIRING_CONFIRM_IND(NanPairingConfirmInd& event)
{
    event.pairingInstanceId = 71;
    event.pairingSuccess = true;
    INIT_NAN_STATUS(event.status);
    event.requestType = NanPairingRequestType(1);
    event.enablePairingCache = false;
    INIT_NPK_SECURITY_ASSOCIATION(event.npksa);
}

static bool T_WIFI_NAN_IFACE_EVENT_PAIRING_CONFIRM_IND()
{
    NanPairingConfirmInd event;
    INIT_NAN_PAIRING_CONFIRM_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventPairingConfirmInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanPairingConfirmInd param;
    bool msgParseResult = WifiNanIfaceParseEventPairingConfirmInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanPairingConfirmInd)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_INITIATE_PAIRING_RESPONSE_IND()
{
    char16_t id = 'b';
    NanStatus status;
    INIT_NAN_STATUS(status);
    int32_t pairingInstanceId = 7;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyInitiatePairingResponseInd(id, status, pairingInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyInitiatePairingResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyInitiatePairingResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_BUF(&pairingInstanceId, &param.pairingInstanceId, sizeof(int32_t), "pairingInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_PAIRING_INDICATION_RESPONSE_IND()
{
    char16_t id = 'f';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyRespondToPairingIndicationResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyRespondToPairingIndicationResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyRespondToPairingIndicationResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static void INIT_NAN_BOOTSTRAPPING_REQUEST_IND(NanBootstrappingRequestInd& event)
{
    event.discoverySessionId = 0x40;
    event.peerId = 49;
    for (int index = 0; index < 6; index++)
    {
        event.peerDiscMacAddr[index] = 0x30;
    }
    event.bootstrappingInstanceId = 106;
    event.requestBootstrappingMethod = NanBootstrappingMethod(1);
}

static bool T_WIFI_NAN_IFACE_EVENT_BOOTSTRAPPING_REQUEST_IND()
{
    NanBootstrappingRequestInd event;
    INIT_NAN_BOOTSTRAPPING_REQUEST_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventBootstrappingRequestInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanBootstrappingRequestInd param;
    bool msgParseResult = WifiNanIfaceParseEventBootstrappingRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanBootstrappingRequestInd)"));
    return true;
}

static void INIT_NAN_BOOTSTRAPPING_CONFIRM_IND(NanBootstrappingConfirmInd& event)
{
    event.bootstrappingInstanceId = 39;
    event.responseCode = NanBootstrappingResponseCode(1);
    INIT_NAN_STATUS(event.reasonCode);
    event.comeBackDelay = 39;
    event.cookie.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event.cookie[index] = 0x97;
    }
}

static bool T_WIFI_NAN_IFACE_EVENT_BOOTSTRAPPING_CONFIRM_IND()
{
    NanBootstrappingConfirmInd event;
    INIT_NAN_BOOTSTRAPPING_CONFIRM_IND(event);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeEventBootstrappingConfirmInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NanBootstrappingConfirmInd param;
    bool msgParseResult = WifiNanIfaceParseEventBootstrappingConfirmInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(event, param, "event (NanBootstrappingConfirmInd)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_INITIATE_BOOTSTRAPPING_RESPONSE_IND()
{
    char16_t id = 'i';
    NanStatus status;
    INIT_NAN_STATUS(status);
    int32_t bootstrappingInstanceId = 21;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyInitiateBootstrappingResponseInd(id, status, bootstrappingInstanceId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyInitiateBootstrappingResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyInitiateBootstrappingResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    VERIFY(COMPARE_BUF(&bootstrappingInstanceId, &param.bootstrappingInstanceId, sizeof(int32_t), "bootstrappingInstanceId (int32_t)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_BOOTSTRAPPING_INDICATION_RESPONSE_IND()
{
    char16_t id = 'l';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyRespondToBootstrappingIndicationResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyRespondToBootstrappingIndicationResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyRespondToBootstrappingIndicationResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_NAN_IFACE_NOTIFY_TERMINATE_PAIRING_RESPONSE_IND()
{
    char16_t id = 'y';
    NanStatus status;
    INIT_NAN_STATUS(status);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiNanIfaceSerializeNotifyTerminatePairingResponseInd(id, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NotifyTerminatePairingResponseIndNanIfaceParam param;
    bool msgParseResult = WifiNanIfaceParseNotifyTerminatePairingResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(char16_t), "id (char16_t)"));
    VERIFY(COMPARE_OBJ(status, param.status, "status (NanStatus)"));
    return true;
}

static bool T_WIFI_P2P_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiP2pIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiP2pIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_P2P_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "E";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiP2pIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmP2pIfaceParam param;
    bool msgParseResult = WifiP2pIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_DISABLE_RESPONDER_REQ()
{
    int32_t cmdId = 95;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeDisableResponderReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiRttControllerParseDisableResponderReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(int32_t), "cmdId (int32_t)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_DISABLE_RESPONDER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeDisableResponderCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseDisableResponderCfm(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_CHANNEL_INFO(WifiChannelInfo& channelHint)
{
    channelHint.width = WifiChannelWidthInMhz(1);
    channelHint.centerFreq = 114;
    channelHint.centerFreq0 = 62;
    channelHint.centerFreq1 = 39;
}

static void INIT_RTT_RESPONDER(RttResponder& info)
{
    INIT_WIFI_CHANNEL_INFO(info.channel);
    info.preamble = RttPreamble(1);
}

static bool T_WIFI_RTT_CONTROLLER_ENABLE_RESPONDER_REQ()
{
    int32_t cmdId = 98;
    WifiChannelInfo channelHint;
    INIT_WIFI_CHANNEL_INFO(channelHint);
    int32_t maxDurationInSeconds = 70;
    RttResponder info;
    INIT_RTT_RESPONDER(info);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeEnableResponderReq(cmdId, channelHint, maxDurationInSeconds, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EnableResponderReqRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseEnableResponderReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_OBJ(channelHint, param.channelHint, "channelHint (WifiChannelInfo)"));
    VERIFY(COMPARE_BUF(&maxDurationInSeconds, &param.maxDurationInSeconds, sizeof(int32_t), "maxDurationInSeconds (int32_t)"));
    VERIFY(COMPARE_OBJ(info, param.info, "info (RttResponder)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_ENABLE_RESPONDER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeEnableResponderCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseEnableResponderCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_GET_BOUND_IFACE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetBoundIfaceReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseGetBoundIfaceReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_GET_BOUND_IFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 42;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetBoundIfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetBoundIfaceCfmRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseGetBoundIfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_GET_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseGetCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_RTT_CAPABILITIES(RttCapabilities& result)
{
    result.rttOneSidedSupported = true;
    result.rttFtmSupported = false;
    result.lciSupported = false;
    result.lcrSupported = false;
    result.responderSupported = false;
    result.preambleSupport = RttPreamble(1);
    result.bwSupport = RttBw(1);
    result.mcVersion = 0x08;
}

static bool T_WIFI_RTT_CONTROLLER_GET_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    RttCapabilities result;
    INIT_RTT_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetCapabilitiesCfmRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseGetCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (RttCapabilities)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_GET_RESPONDER_INFO_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetResponderInfoReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseGetResponderInfoReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_GET_RESPONDER_INFO_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    RttResponder result;
    INIT_RTT_RESPONDER(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeGetResponderInfoCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetResponderInfoCfmRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseGetResponderInfoCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (RttResponder)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_RANGE_CANCEL_REQ()
{
    int32_t cmdId = 97;
    vector<MacAddress> addrs;
    addrs.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MAC_ADDRESS(addrs[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRangeCancelReq(cmdId, addrs, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RangeCancelReqRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseRangeCancelReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_VEC(addrs, param.addrs, "addrs (vector<MacAddress>)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_RANGE_CANCEL_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRangeCancelCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseRangeCancelCfm(payload.data(), payload.size());
    return true;
}

static void INIT_RTT_CONFIG(RttConfig& rttConfigs)
{
    for (int index = 0; index < 6; index++)
    {
        rttConfigs.addr[index] = 0x3f;
    }
    rttConfigs.type = RttType(1);
    rttConfigs.peer = RttPeerType(1);
    INIT_WIFI_CHANNEL_INFO(rttConfigs.channel);
    rttConfigs.burstPeriod = 25;
    rttConfigs.numBurst = 74;
    rttConfigs.numFramesPerBurst = 41;
    rttConfigs.numRetriesPerRttFrame = 22;
    rttConfigs.numRetriesPerFtmr = 72;
    rttConfigs.mustRequestLci = false;
    rttConfigs.mustRequestLcr = true;
    rttConfigs.burstDuration = 62;
    rttConfigs.preamble = RttPreamble(1);
    rttConfigs.bw = RttBw(1);
}

static bool T_WIFI_RTT_CONTROLLER_RANGE_REQUEST_REQ()
{
    int32_t cmdId = 4;
    vector<RttConfig> rttConfigs;
    rttConfigs.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_RTT_CONFIG(rttConfigs[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRangeRequestReq(cmdId, rttConfigs, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RangeRequestReqRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseRangeRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_VEC(rttConfigs, param.rttConfigs, "rttConfigs (vector<RttConfig>)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_RANGE_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRangeRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseRangeRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_REGISTER_EVENT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRegisterEventCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseRegisterEventCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_REGISTER_EVENT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeRegisterEventCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseRegisterEventCallbackCfm(payload.data(), payload.size());
    return true;
}

static void INIT_RTT_LCI_INFORMATION(RttLciInformation& lci)
{
    lci.latitude = 7280102887592706048;
    lci.longitude = 5836270271374447351;
    lci.altitude = 95;
    lci.latitudeUnc = 0x0a;
    lci.longitudeUnc = 0xfa;
    lci.altitudeUnc = 0x4e;
    lci.motionPattern = RttMotionPattern(1);
    lci.floor = 88;
    lci.heightAboveFloor = 69;
    lci.heightUnc = 29;
}

static bool T_WIFI_RTT_CONTROLLER_SET_LCI_REQ()
{
    int32_t cmdId = 114;
    RttLciInformation lci;
    INIT_RTT_LCI_INFORMATION(lci);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeSetLciReq(cmdId, lci, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetLciReqRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseSetLciReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_OBJ(lci, param.lci, "lci (RttLciInformation)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_SET_LCI_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeSetLciCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseSetLciCfm(payload.data(), payload.size());
    return true;
}

static void INIT_RTT_LCR_INFORMATION(RttLcrInformation& lcr)
{
    for (int index = 0; index < 2; index++)
    {
        lcr.countryCode[index] = 0x49;
    }
    lcr.civicInfo = "muhADoSOR";
}

static bool T_WIFI_RTT_CONTROLLER_SET_LCR_REQ()
{
    int32_t cmdId = 15;
    RttLcrInformation lcr;
    INIT_RTT_LCR_INFORMATION(lcr);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeSetLcrReq(cmdId, lcr, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetLcrReqRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseSetLcrReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_OBJ(lcr, param.lcr, "lcr (RttLcrInformation)"));
    return true;
}

static bool T_WIFI_RTT_CONTROLLER_SET_LCR_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeSetLcrCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiRttControllerParseSetLcrCfm(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_RATE_INFO(WifiRateInfo& txRate)
{
    txRate.preamble = WifiRatePreamble(1);
    txRate.nss = WifiRateNss(1);
    txRate.bw = WifiChannelWidthInMhz(1);
    txRate.rateMcsIdx = 0xa3;
    txRate.bitRateInKbps = 50;
}

static void INIT_WIFI_INFORMATION_ELEMENT(WifiInformationElement& lci)
{
    lci.id = 0x97;
    lci.data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        lci.data[index] = 0xb8;
    }
}

static void INIT_RTT_RESULT(RttResult& results)
{
    for (int index = 0; index < 6; index++)
    {
        results.addr[index] = 0xff;
    }
    results.burstNum = 125;
    results.measurementNumber = 46;
    results.successNumber = 27;
    results.numberPerBurstPeer = 0x58;
    results.status = RttStatus(1);
    results.retryAfterDuration = 0xcd;
    results.type = RttType(1);
    results.rssi = 108;
    results.rssiSpread = 116;
    INIT_WIFI_RATE_INFO(results.txRate);
    INIT_WIFI_RATE_INFO(results.rxRate);
    results.rtt = 2696418530764368590;
    results.rttSd = 5572842175697724305;
    results.rttSpread = 321569699358732081;
    results.distanceInMm = 119;
    results.distanceSdInMm = 58;
    results.distanceSpreadInMm = 96;
    results.timeStampInUs = 4267586589022713900;
    results.burstDurationInMs = 119;
    results.negotiatedBurstNum = 3;
    INIT_WIFI_INFORMATION_ELEMENT(results.lci);
    INIT_WIFI_INFORMATION_ELEMENT(results.lcr);
    results.channelFreqMHz = 119;
    results.packetBw = RttBw(1);
}

static bool T_WIFI_RTT_CONTROLLER_ON_RESULTS_IND()
{
    int32_t cmdId = 0;
    vector<RttResult> results;
    results.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_RTT_RESULT(results[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiRttControllerSerializeOnResultsInd(cmdId, results, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnResultsIndRttControllerParam param;
    bool msgParseResult = WifiRttControllerParseOnResultsInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_VEC(results, param.results, "results (vector<RttResult>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "G";
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static void INIT_SSID(Ssid& ssidAllowlist)
{
    for (int index = 0; index < 32; index++)
    {
        ssidAllowlist.data[index] = 0x47;
    }
}

static void INIT_STA_ROAMING_CONFIG(StaRoamingConfig& config)
{
    config.bssidBlocklist.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MAC_ADDRESS(config.bssidBlocklist[index]);
    }
    config.ssidAllowlist.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_SSID(config.ssidAllowlist[index]);
    }
}

static bool T_WIFI_STA_IFACE_CONFIGURE_ROAMING_REQ()
{
    StaRoamingConfig config;
    INIT_STA_ROAMING_CONFIG(config);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeConfigureRoamingReq(config, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StaRoamingConfig param;
    bool msgParseResult = WifiStaIfaceParseConfigureRoamingReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(config, param, "config (StaRoamingConfig)"));
    return true;
}

static bool T_WIFI_STA_IFACE_CONFIGURE_ROAMING_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeConfigureRoamingCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseConfigureRoamingCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_DISABLE_LINK_LAYER_STATS_COLLECTION_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeDisableLinkLayerStatsCollectionReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseDisableLinkLayerStatsCollectionReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_DISABLE_LINK_LAYER_STATS_COLLECTION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeDisableLinkLayerStatsCollectionCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseDisableLinkLayerStatsCollectionCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_ENABLE_LINK_LAYER_STATS_COLLECTION_REQ()
{
    bool debug = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeEnableLinkLayerStatsCollectionReq(debug, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = WifiStaIfaceParseEnableLinkLayerStatsCollectionReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&debug, &param, sizeof(bool), "debug (bool)"));
    return true;
}

static bool T_WIFI_STA_IFACE_ENABLE_LINK_LAYER_STATS_COLLECTION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeEnableLinkLayerStatsCollectionCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseEnableLinkLayerStatsCollectionCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_ENABLE_ND_OFFLOAD_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeEnableNdOffloadReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = WifiStaIfaceParseEnableNdOffloadReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_WIFI_STA_IFACE_ENABLE_ND_OFFLOAD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeEnableNdOffloadCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseEnableNdOffloadCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_GET_APF_PACKET_FILTER_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetApfPacketFilterCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_STA_APF_PACKET_FILTER_CAPABILITIES(StaApfPacketFilterCapabilities& result)
{
    result.version = 83;
    result.maxLength = 69;
}

static bool T_WIFI_STA_IFACE_GET_APF_PACKET_FILTER_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    StaApfPacketFilterCapabilities result;
    INIT_STA_APF_PACKET_FILTER_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetApfPacketFilterCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetApfPacketFilterCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (StaApfPacketFilterCapabilities)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_BACKGROUND_SCAN_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetBackgroundScanCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetBackgroundScanCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_STA_BACKGROUND_SCAN_CAPABILITIES(StaBackgroundScanCapabilities& result)
{
    result.maxCacheSize = 99;
    result.maxBuckets = 118;
    result.maxApCachePerScan = 13;
    result.maxReportingThreshold = 92;
}

static bool T_WIFI_STA_IFACE_GET_BACKGROUND_SCAN_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    StaBackgroundScanCapabilities result;
    INIT_STA_BACKGROUND_SCAN_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetBackgroundScanCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetBackgroundScanCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetBackgroundScanCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (StaBackgroundScanCapabilities)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_FEATURE_SET_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetFeatureSetReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetFeatureSetReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_GET_FEATURE_SET_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 12;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetFeatureSetCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetFeatureSetCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetFeatureSetCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_DEBUG_RX_PACKET_FATES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetDebugRxPacketFatesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetDebugRxPacketFatesReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_DEBUG_PACKET_FATE_FRAME_INFO(WifiDebugPacketFateFrameInfo& frameInfo)
{
    frameInfo.frameType = WifiDebugPacketFateFrameType(1);
    frameInfo.frameLen = 7903224334166733001;
    frameInfo.driverTimestampUsec = 2934842859594070518;
    frameInfo.firmwareTimestampUsec = 7400072161902979513;
    frameInfo.frameContent.resize(4);
    for (int index = 0; index < 4; index++)
    {
        frameInfo.frameContent[index] = 0x88;
    }
}

static void INIT_WIFI_DEBUG_RX_PACKET_FATE_REPORT(WifiDebugRxPacketFateReport& result)
{
    result.fate = WifiDebugRxPacketFate(1);
    INIT_WIFI_DEBUG_PACKET_FATE_FRAME_INFO(result.frameInfo);
}

static bool T_WIFI_STA_IFACE_GET_DEBUG_RX_PACKET_FATES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<WifiDebugRxPacketFateReport> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_DEBUG_RX_PACKET_FATE_REPORT(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetDebugRxPacketFatesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDebugRxPacketFatesCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetDebugRxPacketFatesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<WifiDebugRxPacketFateReport>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_DEBUG_TX_PACKET_FATES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetDebugTxPacketFatesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetDebugTxPacketFatesReq(payload.data(), payload.size());
    return true;
}

static void INIT_WIFI_DEBUG_TX_PACKET_FATE_REPORT(WifiDebugTxPacketFateReport& result)
{
    result.fate = WifiDebugTxPacketFate(1);
    INIT_WIFI_DEBUG_PACKET_FATE_FRAME_INFO(result.frameInfo);
}

static bool T_WIFI_STA_IFACE_GET_DEBUG_TX_PACKET_FATES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<WifiDebugTxPacketFateReport> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_DEBUG_TX_PACKET_FATE_REPORT(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetDebugTxPacketFatesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDebugTxPacketFatesCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetDebugTxPacketFatesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<WifiDebugTxPacketFateReport>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_FACTORY_MAC_ADDRESS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetFactoryMacAddressReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetFactoryMacAddressReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_GET_FACTORY_MAC_ADDRESS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    array<uint8_t, 6> result;
    for (int index = 0; index < 6; index++)
    {
        result[index] = 0x83;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetFactoryMacAddressCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetFactoryMacAddressCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetFactoryMacAddressCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_ARY(result, param.result, "result (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_LINK_LAYER_STATS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetLinkLayerStatsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetLinkLayerStatsReq(payload.data(), payload.size());
    return true;
}

static void INIT_STA_LINK_LAYER_IFACE_PACKET_STATS(StaLinkLayerIfacePacketStats& wmeBePktStats)
{
    wmeBePktStats.rxMpdu = 8976640641088401297;
    wmeBePktStats.txMpdu = 489428722128303387;
    wmeBePktStats.lostMpdu = 1758631679263490112;
    wmeBePktStats.retries = 3605596528059901472;
}

static void INIT_STA_LINK_LAYER_IFACE_CONTENTION_TIME_STATS(StaLinkLayerIfaceContentionTimeStats& wmeBeContentionTimeStats)
{
    wmeBeContentionTimeStats.contentionTimeMinInUsec = 61;
    wmeBeContentionTimeStats.contentionTimeMaxInUsec = 122;
    wmeBeContentionTimeStats.contentionTimeAvgInUsec = 44;
    wmeBeContentionTimeStats.contentionNumSamples = 104;
}

static void INIT_STA_RATE_STAT(StaRateStat& rateStats)
{
    INIT_WIFI_RATE_INFO(rateStats.rateInfo);
    rateStats.txMpdu = 65;
    rateStats.rxMpdu = 38;
    rateStats.mpduLost = 14;
    rateStats.retries = 22;
}

static void INIT_STA_PEER_INFO(StaPeerInfo& peers)
{
    peers.staCount = 'q';
    peers.chanUtil = 'Q';
    peers.rateStats.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_RATE_STAT(peers.rateStats[index]);
    }
}

static void INIT_STA_LINK_LAYER_LINK_STATS(StaLinkLayerLinkStats& links)
{
    links.linkId = 83;
    links.radioId = 73;
    links.frequencyMhz = 127;
    links.beaconRx = 125;
    links.avgRssiMgmt = 29;
    INIT_STA_LINK_LAYER_IFACE_PACKET_STATS(links.wmeBePktStats);
    INIT_STA_LINK_LAYER_IFACE_PACKET_STATS(links.wmeBkPktStats);
    INIT_STA_LINK_LAYER_IFACE_PACKET_STATS(links.wmeViPktStats);
    INIT_STA_LINK_LAYER_IFACE_PACKET_STATS(links.wmeVoPktStats);
    links.timeSliceDutyCycleInPercent = 0x2d;
    INIT_STA_LINK_LAYER_IFACE_CONTENTION_TIME_STATS(links.wmeBeContentionTimeStats);
    INIT_STA_LINK_LAYER_IFACE_CONTENTION_TIME_STATS(links.wmeBkContentionTimeStats);
    INIT_STA_LINK_LAYER_IFACE_CONTENTION_TIME_STATS(links.wmeViContentionTimeStats);
    INIT_STA_LINK_LAYER_IFACE_CONTENTION_TIME_STATS(links.wmeVoContentionTimeStats);
    links.peers.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_PEER_INFO(links.peers[index]);
    }
    links.state = StaLinkLayerLinkStats::StaLinkState(1);
}

static void INIT_STA_LINK_LAYER_IFACE_STATS(StaLinkLayerIfaceStats& iface)
{
    iface.links.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_LINK_LAYER_LINK_STATS(iface.links[index]);
    }
}

static void INIT_WIFI_CHANNEL_STATS(WifiChannelStats& channelStats)
{
    INIT_WIFI_CHANNEL_INFO(channelStats.channel);
    channelStats.onTimeInMs = 64;
    channelStats.ccaBusyTimeInMs = 36;
}

static void INIT_STA_LINK_LAYER_RADIO_STATS(StaLinkLayerRadioStats& radios)
{
    radios.onTimeInMs = 81;
    radios.txTimeInMs = 67;
    radios.txTimeInMsPerLevel.resize(4);
    for (int index = 0; index < 4; index++)
    {
        radios.txTimeInMsPerLevel[index] = 32;
    }
    radios.rxTimeInMs = 127;
    radios.onTimeInMsForScan = 85;
    radios.onTimeInMsForNanScan = 43;
    radios.onTimeInMsForBgScan = 63;
    radios.onTimeInMsForRoamScan = 83;
    radios.onTimeInMsForPnoScan = 15;
    radios.onTimeInMsForHs20Scan = 62;
    radios.channelStats.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_CHANNEL_STATS(radios.channelStats[index]);
    }
    radios.radioId = 26;
}

static void INIT_STA_LINK_LAYER_STATS(StaLinkLayerStats& result)
{
    INIT_STA_LINK_LAYER_IFACE_STATS(result.iface);
    result.radios.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_LINK_LAYER_RADIO_STATS(result.radios[index]);
    }
    result.timeStampInMs = 7776558438560847435;
}

static bool T_WIFI_STA_IFACE_GET_LINK_LAYER_STATS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    StaLinkLayerStats result;
    INIT_STA_LINK_LAYER_STATS(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetLinkLayerStatsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetLinkLayerStatsCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetLinkLayerStatsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (StaLinkLayerStats)"));
    return true;
}

static bool T_WIFI_STA_IFACE_GET_ROAMING_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetRoamingCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseGetRoamingCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_STA_ROAMING_CAPABILITIES(StaRoamingCapabilities& result)
{
    result.maxBlocklistSize = 101;
    result.maxAllowlistSize = 62;
}

static bool T_WIFI_STA_IFACE_GET_ROAMING_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    StaRoamingCapabilities result;
    INIT_STA_ROAMING_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeGetRoamingCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetRoamingCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseGetRoamingCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (StaRoamingCapabilities)"));
    return true;
}

static bool T_WIFI_STA_IFACE_INSTALL_APF_PACKET_FILTER_REQ()
{
    vector<uint8_t> program;
    program.resize(4);
    for (int index = 0; index < 4; index++)
    {
        program[index] = 0x13;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeInstallApfPacketFilterReq(program, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = WifiStaIfaceParseInstallApfPacketFilterReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(program, param, "program (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_INSTALL_APF_PACKET_FILTER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeInstallApfPacketFilterCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseInstallApfPacketFilterCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_READ_APF_PACKET_FILTER_DATA_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeReadApfPacketFilterDataReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseReadApfPacketFilterDataReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_READ_APF_PACKET_FILTER_DATA_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xfe;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeReadApfPacketFilterDataCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ReadApfPacketFilterDataCfmStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseReadApfPacketFilterDataCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_REGISTER_EVENT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeRegisterEventCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseRegisterEventCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_REGISTER_EVENT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeRegisterEventCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseRegisterEventCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_SET_MAC_ADDRESS_REQ()
{
    array<uint8_t, 6> mac;
    for (int index = 0; index < 6; index++)
    {
        mac[index] = 0x46;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetMacAddressReq(mac, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    array<uint8_t, 6> param;
    bool msgParseResult = WifiStaIfaceParseSetMacAddressReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_ARY(mac, param, "mac (array<uint8_t>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_SET_MAC_ADDRESS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetMacAddressCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseSetMacAddressCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_SET_ROAMING_STATE_REQ()
{
    StaRoamingState state = StaRoamingState(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetRoamingStateReq(state, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StaRoamingState param;
    bool msgParseResult = WifiStaIfaceParseSetRoamingStateReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&state, &param, sizeof(StaRoamingState), "state (StaRoamingState)"));
    return true;
}

static bool T_WIFI_STA_IFACE_SET_ROAMING_STATE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetRoamingStateCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseSetRoamingStateCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_SET_SCAN_MODE_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetScanModeReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = WifiStaIfaceParseSetScanModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_WIFI_STA_IFACE_SET_SCAN_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetScanModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseSetScanModeCfm(payload.data(), payload.size());
    return true;
}

static void INIT_STA_BACKGROUND_SCAN_BUCKET_PARAMETERS(StaBackgroundScanBucketParameters& buckets)
{
    buckets.bucketIdx = 35;
    buckets.band = WifiBand(1);
    buckets.frequencies.resize(4);
    for (int index = 0; index < 4; index++)
    {
        buckets.frequencies[index] = 52;
    }
    buckets.periodInMs = 11;
    buckets.eventReportScheme = 42;
    buckets.exponentialMaxPeriodInMs = 104;
    buckets.exponentialBase = 119;
    buckets.exponentialStepCount = 72;
}

static void INIT_STA_BACKGROUND_SCAN_PARAMETERS(StaBackgroundScanParameters& params)
{
    params.basePeriodInMs = 116;
    params.maxApPerScan = 112;
    params.reportThresholdPercent = 53;
    params.reportThresholdNumScans = 111;
    params.buckets.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_BACKGROUND_SCAN_BUCKET_PARAMETERS(params.buckets[index]);
    }
}

static bool T_WIFI_STA_IFACE_START_BACKGROUND_SCAN_REQ()
{
    int32_t cmdId = 98;
    StaBackgroundScanParameters params;
    INIT_STA_BACKGROUND_SCAN_PARAMETERS(params);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartBackgroundScanReq(cmdId, params, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartBackgroundScanReqStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseStartBackgroundScanReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_OBJ(params, param.params, "params (StaBackgroundScanParameters)"));
    return true;
}

static bool T_WIFI_STA_IFACE_START_BACKGROUND_SCAN_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartBackgroundScanCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStartBackgroundScanCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_START_DEBUG_PACKET_FATE_MONITORING_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartDebugPacketFateMonitoringReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStartDebugPacketFateMonitoringReq(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_START_DEBUG_PACKET_FATE_MONITORING_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartDebugPacketFateMonitoringCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStartDebugPacketFateMonitoringCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_START_RSSI_MONITORING_REQ()
{
    int32_t cmdId = 43;
    int32_t maxRssi = 88;
    int32_t minRssi = 99;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartRssiMonitoringReq(cmdId, maxRssi, minRssi, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartRssiMonitoringReqStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseStartRssiMonitoringReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_BUF(&maxRssi, &param.maxRssi, sizeof(int32_t), "maxRssi (int32_t)"));
    VERIFY(COMPARE_BUF(&minRssi, &param.minRssi, sizeof(int32_t), "minRssi (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_START_RSSI_MONITORING_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartRssiMonitoringCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStartRssiMonitoringCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_START_SENDING_KEEP_ALIVE_PACKETS_REQ()
{
    int32_t cmdId = 99;
    vector<uint8_t> ipPacketData;
    ipPacketData.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ipPacketData[index] = 0x3f;
    }
    int32_t etherType = 39;
    array<uint8_t, 6> srcAddress;
    for (int index = 0; index < 6; index++)
    {
        srcAddress[index] = 0x88;
    }
    array<uint8_t, 6> dstAddress;
    for (int index = 0; index < 6; index++)
    {
        dstAddress[index] = 0xb4;
    }
    int32_t periodInMs = 5;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartSendingKeepAlivePacketsReq(cmdId, ipPacketData, etherType, srcAddress, dstAddress, periodInMs, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartSendingKeepAlivePacketsReqStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseStartSendingKeepAlivePacketsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_VEC(ipPacketData, param.ipPacketData, "ipPacketData (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&etherType, &param.etherType, sizeof(int32_t), "etherType (int32_t)"));
    VERIFY(COMPARE_ARY(srcAddress, param.srcAddress, "srcAddress (array<uint8_t>)"));
    VERIFY(COMPARE_ARY(dstAddress, param.dstAddress, "dstAddress (array<uint8_t>)"));
    VERIFY(COMPARE_BUF(&periodInMs, &param.periodInMs, sizeof(int32_t), "periodInMs (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_START_SENDING_KEEP_ALIVE_PACKETS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStartSendingKeepAlivePacketsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStartSendingKeepAlivePacketsCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_BACKGROUND_SCAN_REQ()
{
    int32_t cmdId = 87;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopBackgroundScanReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiStaIfaceParseStopBackgroundScanReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(int32_t), "cmdId (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_BACKGROUND_SCAN_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopBackgroundScanCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStopBackgroundScanCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_RSSI_MONITORING_REQ()
{
    int32_t cmdId = 57;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopRssiMonitoringReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiStaIfaceParseStopRssiMonitoringReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(int32_t), "cmdId (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_RSSI_MONITORING_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopRssiMonitoringCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStopRssiMonitoringCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_SENDING_KEEP_ALIVE_PACKETS_REQ()
{
    int32_t cmdId = 112;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopSendingKeepAlivePacketsReq(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiStaIfaceParseStopSendingKeepAlivePacketsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(int32_t), "cmdId (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_STOP_SENDING_KEEP_ALIVE_PACKETS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeStopSendingKeepAlivePacketsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseStopSendingKeepAlivePacketsCfm(payload.data(), payload.size());
    return true;
}

static bool T_WIFI_STA_IFACE_SET_DTIM_MULTIPLIER_REQ()
{
    int32_t multiplier = 62;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetDtimMultiplierReq(multiplier, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiStaIfaceParseSetDtimMultiplierReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&multiplier, &param, sizeof(int32_t), "multiplier (int32_t)"));
    return true;
}

static bool T_WIFI_STA_IFACE_SET_DTIM_MULTIPLIER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeSetDtimMultiplierCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = WifiStaIfaceParseSetDtimMultiplierCfm(payload.data(), payload.size());
    return true;
}

static void INIT_STA_SCAN_RESULT(StaScanResult& result)
{
    result.timeStampInUs = 255086232245417066;
    result.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result.ssid[index] = 0xf9;
    }
    for (int index = 0; index < 6; index++)
    {
        result.bssid[index] = 0xc3;
    }
    result.rssi = 19;
    result.frequency = 18;
    result.beaconPeriodInMs = 'f';
    result.capability = 'q';
    result.informationElements.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_WIFI_INFORMATION_ELEMENT(result.informationElements[index]);
    }
}

static bool T_WIFI_STA_IFACE_ON_BACKGROUND_FULL_SCAN_RESULT_IND()
{
    int32_t cmdId = 27;
    int32_t bucketsScanned = 56;
    StaScanResult result;
    INIT_STA_SCAN_RESULT(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeOnBackgroundFullScanResultInd(cmdId, bucketsScanned, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnBackgroundFullScanResultIndStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseOnBackgroundFullScanResultInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_BUF(&bucketsScanned, &param.bucketsScanned, sizeof(int32_t), "bucketsScanned (int32_t)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (StaScanResult)"));
    return true;
}

static bool T_WIFI_STA_IFACE_ON_BACKGROUND_SCAN_FAILURE_IND()
{
    int32_t cmdId = 41;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeOnBackgroundScanFailureInd(cmdId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = WifiStaIfaceParseOnBackgroundScanFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param, sizeof(int32_t), "cmdId (int32_t)"));
    return true;
}

static void INIT_STA_SCAN_DATA(StaScanData& scanDatas)
{
    scanDatas.flags = 6;
    scanDatas.bucketsScanned = 4;
    scanDatas.results.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_SCAN_RESULT(scanDatas.results[index]);
    }
}

static bool T_WIFI_STA_IFACE_ON_BACKGROUND_SCAN_RESULTS_IND()
{
    int32_t cmdId = 62;
    vector<StaScanData> scanDatas;
    scanDatas.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_STA_SCAN_DATA(scanDatas[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeOnBackgroundScanResultsInd(cmdId, scanDatas, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnBackgroundScanResultsIndStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseOnBackgroundScanResultsInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_VEC(scanDatas, param.scanDatas, "scanDatas (vector<StaScanData>)"));
    return true;
}

static bool T_WIFI_STA_IFACE_ON_RSSI_THRESHOLD_BREACHED_IND()
{
    int32_t cmdId = 16;
    array<uint8_t, 6> currBssid;
    for (int index = 0; index < 6; index++)
    {
        currBssid[index] = 0x17;
    }
    int32_t currRssi = 115;
    vector<uint8_t> payload;
    bool msgSerializeResult = WifiStaIfaceSerializeOnRssiThresholdBreachedInd(cmdId, currBssid, currRssi, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnRssiThresholdBreachedIndStaIfaceParam param;
    bool msgParseResult = WifiStaIfaceParseOnRssiThresholdBreachedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&cmdId, &param.cmdId, sizeof(int32_t), "cmdId (int32_t)"));
    VERIFY(COMPARE_ARY(currBssid, param.currBssid, "currBssid (array<uint8_t>)"));
    VERIFY(COMPARE_BUF(&currRssi, &param.currRssi, sizeof(int32_t), "currRssi (int32_t)"));
    return true;
}

static bool T_WIFI_HAL_STATUS()
{
    int32_t status = 26;
    string info = "6t";
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
    VERIFY(T_WIFI_GET_CHIP_REQ());
    VERIFY(T_WIFI_GET_CHIP_CFM());
    VERIFY(T_WIFI_GET_CHIP_IDS_REQ());
    VERIFY(T_WIFI_GET_CHIP_IDS_CFM());
    VERIFY(T_WIFI_IS_STARTED_REQ());
    VERIFY(T_WIFI_IS_STARTED_CFM());
    VERIFY(T_WIFI_REGISTER_EVENT_CALLBACK_REQ());
    VERIFY(T_WIFI_REGISTER_EVENT_CALLBACK_CFM());
    VERIFY(T_WIFI_START_REQ());
    VERIFY(T_WIFI_START_CFM());
    VERIFY(T_WIFI_STOP_REQ());
    VERIFY(T_WIFI_STOP_CFM());
    VERIFY(T_WIFI_ON_FAILURE_IND());
    VERIFY(T_WIFI_ON_START_IND());
    VERIFY(T_WIFI_ON_STOP_IND());
    VERIFY(T_WIFI_ON_SUBSYSTEM_RESTART_IND());
    VERIFY(T_WIFI_AP_IFACE_GET_NAME_REQ());
    VERIFY(T_WIFI_AP_IFACE_GET_NAME_CFM());
    VERIFY(T_WIFI_AP_IFACE_GET_BRIDGED_INSTANCES_REQ());
    VERIFY(T_WIFI_AP_IFACE_GET_BRIDGED_INSTANCES_CFM());
    VERIFY(T_WIFI_AP_IFACE_GET_FACTORY_MAC_ADDRESS_REQ());
    VERIFY(T_WIFI_AP_IFACE_GET_FACTORY_MAC_ADDRESS_CFM());
    VERIFY(T_WIFI_AP_IFACE_SET_COUNTRY_CODE_REQ());
    VERIFY(T_WIFI_AP_IFACE_SET_COUNTRY_CODE_CFM());
    VERIFY(T_WIFI_AP_IFACE_RESET_TO_FACTORY_MAC_ADDRESS_REQ());
    VERIFY(T_WIFI_AP_IFACE_RESET_TO_FACTORY_MAC_ADDRESS_CFM());
    VERIFY(T_WIFI_AP_IFACE_SET_MAC_ADDRESS_REQ());
    VERIFY(T_WIFI_AP_IFACE_SET_MAC_ADDRESS_CFM());
    VERIFY(T_WIFI_CHIP_CONFIGURE_CHIP_REQ());
    VERIFY(T_WIFI_CHIP_CONFIGURE_CHIP_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_AP_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_AP_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_BRIDGED_AP_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_BRIDGED_AP_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_NAN_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_NAN_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_P2P_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_P2P_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_RTT_CONTROLLER_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_RTT_CONTROLLER_CFM());
    VERIFY(T_WIFI_CHIP_CREATE_STA_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_CREATE_STA_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_ENABLE_DEBUG_ERROR_ALERTS_REQ());
    VERIFY(T_WIFI_CHIP_ENABLE_DEBUG_ERROR_ALERTS_CFM());
    VERIFY(T_WIFI_CHIP_FLUSH_RING_BUFFER_TO_FILE_REQ());
    VERIFY(T_WIFI_CHIP_FLUSH_RING_BUFFER_TO_FILE_CFM());
    VERIFY(T_WIFI_CHIP_FORCE_DUMP_TO_DEBUG_RING_BUFFER_REQ());
    VERIFY(T_WIFI_CHIP_FORCE_DUMP_TO_DEBUG_RING_BUFFER_CFM());
    VERIFY(T_WIFI_CHIP_GET_AP_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_GET_AP_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_GET_AP_IFACE_NAMES_REQ());
    VERIFY(T_WIFI_CHIP_GET_AP_IFACE_NAMES_CFM());
    VERIFY(T_WIFI_CHIP_GET_AVAILABLE_MODES_REQ());
    VERIFY(T_WIFI_CHIP_GET_AVAILABLE_MODES_CFM());
    VERIFY(T_WIFI_CHIP_GET_FEATURE_SET_REQ());
    VERIFY(T_WIFI_CHIP_GET_FEATURE_SET_CFM());
    VERIFY(T_WIFI_CHIP_GET_DEBUG_HOST_WAKE_REASON_STATS_REQ());
    VERIFY(T_WIFI_CHIP_GET_DEBUG_HOST_WAKE_REASON_STATS_CFM());
    VERIFY(T_WIFI_CHIP_GET_DEBUG_RING_BUFFERS_STATUS_REQ());
    VERIFY(T_WIFI_CHIP_GET_DEBUG_RING_BUFFERS_STATUS_CFM());
    VERIFY(T_WIFI_CHIP_GET_ID_REQ());
    VERIFY(T_WIFI_CHIP_GET_ID_CFM());
    VERIFY(T_WIFI_CHIP_GET_MODE_REQ());
    VERIFY(T_WIFI_CHIP_GET_MODE_CFM());
    VERIFY(T_WIFI_CHIP_GET_NAN_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_GET_NAN_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_GET_NAN_IFACE_NAMES_REQ());
    VERIFY(T_WIFI_CHIP_GET_NAN_IFACE_NAMES_CFM());
    VERIFY(T_WIFI_CHIP_GET_P2P_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_GET_P2P_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_GET_P2P_IFACE_NAMES_REQ());
    VERIFY(T_WIFI_CHIP_GET_P2P_IFACE_NAMES_CFM());
    VERIFY(T_WIFI_CHIP_GET_STA_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_GET_STA_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_GET_STA_IFACE_NAMES_REQ());
    VERIFY(T_WIFI_CHIP_GET_STA_IFACE_NAMES_CFM());
    VERIFY(T_WIFI_CHIP_GET_SUPPORTED_RADIO_COMBINATIONS_REQ());
    VERIFY(T_WIFI_CHIP_GET_SUPPORTED_RADIO_COMBINATIONS_CFM());
    VERIFY(T_WIFI_CHIP_GET_WIFI_CHIP_CAPABILITIES_REQ());
    VERIFY(T_WIFI_CHIP_GET_WIFI_CHIP_CAPABILITIES_CFM());
    VERIFY(T_WIFI_CHIP_GET_USABLE_CHANNELS_REQ());
    VERIFY(T_WIFI_CHIP_GET_USABLE_CHANNELS_CFM());
    VERIFY(T_WIFI_CHIP_SET_AFC_CHANNEL_ALLOWANCE_REQ());
    VERIFY(T_WIFI_CHIP_SET_AFC_CHANNEL_ALLOWANCE_CFM());
    VERIFY(T_WIFI_CHIP_REGISTER_EVENT_CALLBACK_REQ());
    VERIFY(T_WIFI_CHIP_REGISTER_EVENT_CALLBACK_CFM());
    VERIFY(T_WIFI_CHIP_REMOVE_AP_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_REMOVE_AP_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_REMOVE_IFACE_INSTANCE_FROM_BRIDGED_AP_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_REMOVE_IFACE_INSTANCE_FROM_BRIDGED_AP_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_REMOVE_NAN_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_REMOVE_NAN_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_REMOVE_P2P_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_REMOVE_P2P_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_REMOVE_STA_IFACE_REQ());
    VERIFY(T_WIFI_CHIP_REMOVE_STA_IFACE_CFM());
    VERIFY(T_WIFI_CHIP_REQUEST_CHIP_DEBUG_INFO_REQ());
    VERIFY(T_WIFI_CHIP_REQUEST_CHIP_DEBUG_INFO_CFM());
    VERIFY(T_WIFI_CHIP_REQUEST_DRIVER_DEBUG_DUMP_REQ());
    VERIFY(T_WIFI_CHIP_REQUEST_DRIVER_DEBUG_DUMP_CFM());
    VERIFY(T_WIFI_CHIP_REQUEST_FIRMWARE_DEBUG_DUMP_REQ());
    VERIFY(T_WIFI_CHIP_REQUEST_FIRMWARE_DEBUG_DUMP_CFM());
    VERIFY(T_WIFI_CHIP_RESET_TX_POWER_SCENARIO_REQ());
    VERIFY(T_WIFI_CHIP_RESET_TX_POWER_SCENARIO_CFM());
    VERIFY(T_WIFI_CHIP_SELECT_TX_POWER_SCENARIO_REQ());
    VERIFY(T_WIFI_CHIP_SELECT_TX_POWER_SCENARIO_CFM());
    VERIFY(T_WIFI_CHIP_SET_COEX_UNSAFE_CHANNELS_REQ());
    VERIFY(T_WIFI_CHIP_SET_COEX_UNSAFE_CHANNELS_CFM());
    VERIFY(T_WIFI_CHIP_SET_COUNTRY_CODE_REQ());
    VERIFY(T_WIFI_CHIP_SET_COUNTRY_CODE_CFM());
    VERIFY(T_WIFI_CHIP_SET_LATENCY_MODE_REQ());
    VERIFY(T_WIFI_CHIP_SET_LATENCY_MODE_CFM());
    VERIFY(T_WIFI_CHIP_SET_MULTI_STA_PRIMARY_CONNECTION_REQ());
    VERIFY(T_WIFI_CHIP_SET_MULTI_STA_PRIMARY_CONNECTION_CFM());
    VERIFY(T_WIFI_CHIP_SET_MULTI_STA_USE_CASE_REQ());
    VERIFY(T_WIFI_CHIP_SET_MULTI_STA_USE_CASE_CFM());
    VERIFY(T_WIFI_CHIP_START_LOGGING_TO_DEBUG_RING_BUFFER_REQ());
    VERIFY(T_WIFI_CHIP_START_LOGGING_TO_DEBUG_RING_BUFFER_CFM());
    VERIFY(T_WIFI_CHIP_STOP_LOGGING_TO_DEBUG_RING_BUFFER_REQ());
    VERIFY(T_WIFI_CHIP_STOP_LOGGING_TO_DEBUG_RING_BUFFER_CFM());
    VERIFY(T_WIFI_CHIP_TRIGGER_SUBSYSTEM_RESTART_REQ());
    VERIFY(T_WIFI_CHIP_TRIGGER_SUBSYSTEM_RESTART_CFM());
    VERIFY(T_WIFI_CHIP_ENABLE_STA_CHANNEL_FOR_PEER_NETWORK_REQ());
    VERIFY(T_WIFI_CHIP_ENABLE_STA_CHANNEL_FOR_PEER_NETWORK_CFM());
    VERIFY(T_WIFI_CHIP_SET_MLO_MODE_REQ());
    VERIFY(T_WIFI_CHIP_SET_MLO_MODE_CFM());
    VERIFY(T_WIFI_CHIP_ON_CHIP_RECONFIGURE_FAILURE_IND());
    VERIFY(T_WIFI_CHIP_ON_CHIP_RECONFIGURED_IND());
    VERIFY(T_WIFI_CHIP_ON_DEBUG_ERROR_ALERT_IND());
    VERIFY(T_WIFI_CHIP_ON_DEBUG_RING_BUFFER_DATA_AVAILABLE_IND());
    VERIFY(T_WIFI_CHIP_ON_IFACE_ADDED_IND());
    VERIFY(T_WIFI_CHIP_ON_IFACE_REMOVED_IND());
    VERIFY(T_WIFI_CHIP_ON_RADIO_MODE_CHANGE_IND());
    VERIFY(T_WIFI_NAN_IFACE_GET_NAME_REQ());
    VERIFY(T_WIFI_NAN_IFACE_GET_NAME_CFM());
    VERIFY(T_WIFI_NAN_IFACE_CONFIG_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_CONFIG_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_CREATE_DATA_INTERFACE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_CREATE_DATA_INTERFACE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_DELETE_DATA_INTERFACE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_DELETE_DATA_INTERFACE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_DISABLE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_DISABLE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_ENABLE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_ENABLE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_GET_CAPABILITIES_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_GET_CAPABILITIES_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_DATA_PATH_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_DATA_PATH_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_REGISTER_EVENT_CALLBACK_REQ());
    VERIFY(T_WIFI_NAN_IFACE_REGISTER_EVENT_CALLBACK_CFM());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_DATA_PATH_INDICATION_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_DATA_PATH_INDICATION_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_START_PUBLISH_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_START_PUBLISH_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_START_SUBSCRIBE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_START_SUBSCRIBE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_STOP_PUBLISH_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_STOP_PUBLISH_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_STOP_SUBSCRIBE_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_STOP_SUBSCRIBE_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_TERMINATE_DATA_PATH_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_TERMINATE_DATA_PATH_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_SUSPEND_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_SUSPEND_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_RESUME_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_RESUME_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_TRANSMIT_FOLLOWUP_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_TRANSMIT_FOLLOWUP_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_PAIRING_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_PAIRING_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_PAIRING_INDICATION_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_PAIRING_INDICATION_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_BOOTSTRAPPING_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_INITIATE_BOOTSTRAPPING_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_BOOTSTRAPPING_INDICATION_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_RESPOND_TO_BOOTSTRAPPING_INDICATION_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_TERMINATE_PAIRING_REQUEST_REQ());
    VERIFY(T_WIFI_NAN_IFACE_TERMINATE_PAIRING_REQUEST_CFM());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_CLUSTER_EVENT_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_DATA_PATH_CONFIRM_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_DATA_PATH_REQUEST_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_DATA_PATH_SCHEDULE_UPDATE_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_DATA_PATH_TERMINATED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_DISABLED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_FOLLOWUP_RECEIVED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_MATCH_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_MATCH_EXPIRED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_PUBLISH_TERMINATED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_SUBSCRIBE_TERMINATED_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_TRANSMIT_FOLLOWUP_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_SUSPENSION_MODE_CHANGED_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_CAPABILITIES_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_CONFIG_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_CREATE_DATA_INTERFACE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_DELETE_DATA_INTERFACE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_DISABLE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_ENABLE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_INITIATE_DATA_PATH_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_DATA_PATH_INDICATION_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_START_PUBLISH_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_START_SUBSCRIBE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_STOP_PUBLISH_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_STOP_SUBSCRIBE_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_TERMINATE_DATA_PATH_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_SUSPEND_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_RESUME_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_TRANSMIT_FOLLOWUP_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_PAIRING_REQUEST_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_PAIRING_CONFIRM_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_INITIATE_PAIRING_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_PAIRING_INDICATION_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_BOOTSTRAPPING_REQUEST_IND());
    VERIFY(T_WIFI_NAN_IFACE_EVENT_BOOTSTRAPPING_CONFIRM_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_INITIATE_BOOTSTRAPPING_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_RESPOND_TO_BOOTSTRAPPING_INDICATION_RESPONSE_IND());
    VERIFY(T_WIFI_NAN_IFACE_NOTIFY_TERMINATE_PAIRING_RESPONSE_IND());
    VERIFY(T_WIFI_P2P_IFACE_GET_NAME_REQ());
    VERIFY(T_WIFI_P2P_IFACE_GET_NAME_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_DISABLE_RESPONDER_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_DISABLE_RESPONDER_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_ENABLE_RESPONDER_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_ENABLE_RESPONDER_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_BOUND_IFACE_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_BOUND_IFACE_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_CAPABILITIES_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_CAPABILITIES_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_RESPONDER_INFO_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_GET_RESPONDER_INFO_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_RANGE_CANCEL_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_RANGE_CANCEL_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_RANGE_REQUEST_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_RANGE_REQUEST_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_REGISTER_EVENT_CALLBACK_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_REGISTER_EVENT_CALLBACK_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_SET_LCI_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_SET_LCI_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_SET_LCR_REQ());
    VERIFY(T_WIFI_RTT_CONTROLLER_SET_LCR_CFM());
    VERIFY(T_WIFI_RTT_CONTROLLER_ON_RESULTS_IND());
    VERIFY(T_WIFI_STA_IFACE_GET_NAME_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_NAME_CFM());
    VERIFY(T_WIFI_STA_IFACE_CONFIGURE_ROAMING_REQ());
    VERIFY(T_WIFI_STA_IFACE_CONFIGURE_ROAMING_CFM());
    VERIFY(T_WIFI_STA_IFACE_DISABLE_LINK_LAYER_STATS_COLLECTION_REQ());
    VERIFY(T_WIFI_STA_IFACE_DISABLE_LINK_LAYER_STATS_COLLECTION_CFM());
    VERIFY(T_WIFI_STA_IFACE_ENABLE_LINK_LAYER_STATS_COLLECTION_REQ());
    VERIFY(T_WIFI_STA_IFACE_ENABLE_LINK_LAYER_STATS_COLLECTION_CFM());
    VERIFY(T_WIFI_STA_IFACE_ENABLE_ND_OFFLOAD_REQ());
    VERIFY(T_WIFI_STA_IFACE_ENABLE_ND_OFFLOAD_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_APF_PACKET_FILTER_CAPABILITIES_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_APF_PACKET_FILTER_CAPABILITIES_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_BACKGROUND_SCAN_CAPABILITIES_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_BACKGROUND_SCAN_CAPABILITIES_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_FEATURE_SET_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_FEATURE_SET_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_DEBUG_RX_PACKET_FATES_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_DEBUG_RX_PACKET_FATES_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_DEBUG_TX_PACKET_FATES_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_DEBUG_TX_PACKET_FATES_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_FACTORY_MAC_ADDRESS_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_FACTORY_MAC_ADDRESS_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_LINK_LAYER_STATS_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_LINK_LAYER_STATS_CFM());
    VERIFY(T_WIFI_STA_IFACE_GET_ROAMING_CAPABILITIES_REQ());
    VERIFY(T_WIFI_STA_IFACE_GET_ROAMING_CAPABILITIES_CFM());
    VERIFY(T_WIFI_STA_IFACE_INSTALL_APF_PACKET_FILTER_REQ());
    VERIFY(T_WIFI_STA_IFACE_INSTALL_APF_PACKET_FILTER_CFM());
    VERIFY(T_WIFI_STA_IFACE_READ_APF_PACKET_FILTER_DATA_REQ());
    VERIFY(T_WIFI_STA_IFACE_READ_APF_PACKET_FILTER_DATA_CFM());
    VERIFY(T_WIFI_STA_IFACE_REGISTER_EVENT_CALLBACK_REQ());
    VERIFY(T_WIFI_STA_IFACE_REGISTER_EVENT_CALLBACK_CFM());
    VERIFY(T_WIFI_STA_IFACE_SET_MAC_ADDRESS_REQ());
    VERIFY(T_WIFI_STA_IFACE_SET_MAC_ADDRESS_CFM());
    VERIFY(T_WIFI_STA_IFACE_SET_ROAMING_STATE_REQ());
    VERIFY(T_WIFI_STA_IFACE_SET_ROAMING_STATE_CFM());
    VERIFY(T_WIFI_STA_IFACE_SET_SCAN_MODE_REQ());
    VERIFY(T_WIFI_STA_IFACE_SET_SCAN_MODE_CFM());
    VERIFY(T_WIFI_STA_IFACE_START_BACKGROUND_SCAN_REQ());
    VERIFY(T_WIFI_STA_IFACE_START_BACKGROUND_SCAN_CFM());
    VERIFY(T_WIFI_STA_IFACE_START_DEBUG_PACKET_FATE_MONITORING_REQ());
    VERIFY(T_WIFI_STA_IFACE_START_DEBUG_PACKET_FATE_MONITORING_CFM());
    VERIFY(T_WIFI_STA_IFACE_START_RSSI_MONITORING_REQ());
    VERIFY(T_WIFI_STA_IFACE_START_RSSI_MONITORING_CFM());
    VERIFY(T_WIFI_STA_IFACE_START_SENDING_KEEP_ALIVE_PACKETS_REQ());
    VERIFY(T_WIFI_STA_IFACE_START_SENDING_KEEP_ALIVE_PACKETS_CFM());
    VERIFY(T_WIFI_STA_IFACE_STOP_BACKGROUND_SCAN_REQ());
    VERIFY(T_WIFI_STA_IFACE_STOP_BACKGROUND_SCAN_CFM());
    VERIFY(T_WIFI_STA_IFACE_STOP_RSSI_MONITORING_REQ());
    VERIFY(T_WIFI_STA_IFACE_STOP_RSSI_MONITORING_CFM());
    VERIFY(T_WIFI_STA_IFACE_STOP_SENDING_KEEP_ALIVE_PACKETS_REQ());
    VERIFY(T_WIFI_STA_IFACE_STOP_SENDING_KEEP_ALIVE_PACKETS_CFM());
    VERIFY(T_WIFI_STA_IFACE_SET_DTIM_MULTIPLIER_REQ());
    VERIFY(T_WIFI_STA_IFACE_SET_DTIM_MULTIPLIER_CFM());
    VERIFY(T_WIFI_STA_IFACE_ON_BACKGROUND_FULL_SCAN_RESULT_IND());
    VERIFY(T_WIFI_STA_IFACE_ON_BACKGROUND_SCAN_FAILURE_IND());
    VERIFY(T_WIFI_STA_IFACE_ON_BACKGROUND_SCAN_RESULTS_IND());
    VERIFY(T_WIFI_STA_IFACE_ON_RSSI_THRESHOLD_BREACHED_IND());
    VERIFY(T_WIFI_HAL_STATUS());
    ALOGD("ALL 299 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("wifi-msg-test starts");
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
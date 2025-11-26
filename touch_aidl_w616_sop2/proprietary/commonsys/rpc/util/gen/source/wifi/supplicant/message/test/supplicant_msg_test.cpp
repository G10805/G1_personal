/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "non_standard_cert_msg.h"
#include "supplicant_msg.h"
#include "supplicant_msg_common.h"
#include "supplicant_p2p_iface_msg.h"
#include "supplicant_p2p_network_msg.h"
#include "supplicant_sta_iface_msg.h"
#include "supplicant_sta_network_msg.h"

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
    status.status = 29;
    status.info = "7WQ";
}

static bool T_SUPPLICANT_ADD_P2P_INTERFACE_REQ()
{
    string ifName = "r5QjF1sLz";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeAddP2pInterfaceReq(ifName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseAddP2pInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifName, param, "ifName (string)"));
    return true;
}

static bool T_SUPPLICANT_ADD_P2P_INTERFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 73;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeAddP2pInterfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddP2pInterfaceCfmParam param;
    bool msgParseResult = SupplicantParseAddP2pInterfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_ADD_STA_INTERFACE_REQ()
{
    string ifName = "0";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeAddStaInterfaceReq(ifName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseAddStaInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifName, param, "ifName (string)"));
    return true;
}

static bool T_SUPPLICANT_ADD_STA_INTERFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 79;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeAddStaInterfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddStaInterfaceCfmParam param;
    bool msgParseResult = SupplicantParseAddStaInterfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_GET_DEBUG_LEVEL_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetDebugLevelReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseGetDebugLevelReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_GET_DEBUG_LEVEL_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    DebugLevel result = DebugLevel(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetDebugLevelCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDebugLevelCfmParam param;
    bool msgParseResult = SupplicantParseGetDebugLevelCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(DebugLevel), "result (DebugLevel)"));
    return true;
}

static bool T_SUPPLICANT_GET_P2P_INTERFACE_REQ()
{
    string ifName = "u0";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetP2pInterfaceReq(ifName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseGetP2pInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifName, param, "ifName (string)"));
    return true;
}

static bool T_SUPPLICANT_GET_P2P_INTERFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 55;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetP2pInterfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetP2pInterfaceCfmParam param;
    bool msgParseResult = SupplicantParseGetP2pInterfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_GET_STA_INTERFACE_REQ()
{
    string ifName = "k8qvm";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetStaInterfaceReq(ifName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseGetStaInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifName, param, "ifName (string)"));
    return true;
}

static bool T_SUPPLICANT_GET_STA_INTERFACE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 86;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeGetStaInterfaceCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetStaInterfaceCfmParam param;
    bool msgParseResult = SupplicantParseGetStaInterfaceCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_IS_DEBUG_SHOW_KEYS_ENABLED_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeIsDebugShowKeysEnabledReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseIsDebugShowKeysEnabledReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_IS_DEBUG_SHOW_KEYS_ENABLED_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeIsDebugShowKeysEnabledCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsDebugShowKeysEnabledCfmParam param;
    bool msgParseResult = SupplicantParseIsDebugShowKeysEnabledCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_IS_DEBUG_SHOW_TIMESTAMP_ENABLED_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeIsDebugShowTimestampEnabledReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseIsDebugShowTimestampEnabledReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_IS_DEBUG_SHOW_TIMESTAMP_ENABLED_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeIsDebugShowTimestampEnabledCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsDebugShowTimestampEnabledCfmParam param;
    bool msgParseResult = SupplicantParseIsDebugShowTimestampEnabledCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_LIST_INTERFACES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeListInterfacesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseListInterfacesReq(payload.data(), payload.size());
    return true;
}

static void INIT_IFACE_INFO(IfaceInfo& result)
{
    result.type = IfaceType(1);
    result.name = "pCCuj0sJZL";
}

static bool T_SUPPLICANT_LIST_INTERFACES_CFM()
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
    bool msgSerializeResult = SupplicantSerializeListInterfacesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListInterfacesCfmParam param;
    bool msgParseResult = SupplicantParseListInterfacesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<IfaceInfo>)"));
    return true;
}

static bool T_SUPPLICANT_REGISTER_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRegisterCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseRegisterCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_REGISTER_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRegisterCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseRegisterCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_REMOVE_INTERFACE_REQ()
{
    IfaceInfo ifaceInfo;
    INIT_IFACE_INFO(ifaceInfo);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRemoveInterfaceReq(ifaceInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IfaceInfo param;
    bool msgParseResult = SupplicantParseRemoveInterfaceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(ifaceInfo, param, "ifaceInfo (IfaceInfo)"));
    return true;
}

static bool T_SUPPLICANT_REMOVE_INTERFACE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRemoveInterfaceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseRemoveInterfaceCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_SET_CONCURRENCY_PRIORITY_REQ()
{
    IfaceType type = IfaceType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeSetConcurrencyPriorityReq(type, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IfaceType param;
    bool msgParseResult = SupplicantParseSetConcurrencyPriorityReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&type, &param, sizeof(IfaceType), "type (IfaceType)"));
    return true;
}

static bool T_SUPPLICANT_SET_CONCURRENCY_PRIORITY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeSetConcurrencyPriorityCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseSetConcurrencyPriorityCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_SET_DEBUG_PARAMS_REQ()
{
    DebugLevel level = DebugLevel(1);
    bool showTimestamp = false;
    bool showKeys = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeSetDebugParamsReq(level, showTimestamp, showKeys, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetDebugParamsReqParam param;
    bool msgParseResult = SupplicantParseSetDebugParamsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&level, &param.level, sizeof(DebugLevel), "level (DebugLevel)"));
    VERIFY(COMPARE_BUF(&showTimestamp, &param.showTimestamp, sizeof(bool), "showTimestamp (bool)"));
    VERIFY(COMPARE_BUF(&showKeys, &param.showKeys, sizeof(bool), "showKeys (bool)"));
    return true;
}

static bool T_SUPPLICANT_SET_DEBUG_PARAMS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeSetDebugParamsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseSetDebugParamsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_TERMINATE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeTerminateReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseTerminateReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_TERMINATE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeTerminateCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseTerminateCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_REGISTER_NON_STANDARD_CERT_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRegisterNonStandardCertCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseRegisterNonStandardCertCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_REGISTER_NON_STANDARD_CERT_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeRegisterNonStandardCertCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantParseRegisterNonStandardCertCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_ON_INTERFACE_CREATED_IND()
{
    string ifaceName = "A1BfQ6B";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeOnInterfaceCreatedInd(ifaceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseOnInterfaceCreatedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param, "ifaceName (string)"));
    return true;
}

static bool T_SUPPLICANT_ON_INTERFACE_REMOVED_IND()
{
    string ifaceName = "CIlgfUIPC";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeOnInterfaceRemovedInd(ifaceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantParseOnInterfaceRemovedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param, "ifaceName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_BONJOUR_SERVICE_REQ()
{
    vector<uint8_t> query;
    query.resize(4);
    for (int index = 0; index < 4; index++)
    {
        query[index] = 0xfd;
    }
    vector<uint8_t> response;
    response.resize(4);
    for (int index = 0; index < 4; index++)
    {
        response[index] = 0x56;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddBonjourServiceReq(query, response, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddBonjourServiceReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseAddBonjourServiceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(query, param.query, "query (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(response, param.response, "response (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_BONJOUR_SERVICE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddBonjourServiceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseAddBonjourServiceCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_GROUP_REQ()
{
    bool persistent = true;
    int32_t persistentNetworkId = 15;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddGroupReq(persistent, persistentNetworkId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddGroupReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseAddGroupReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&persistent, &param.persistent, sizeof(bool), "persistent (bool)"));
    VERIFY(COMPARE_BUF(&persistentNetworkId, &param.persistentNetworkId, sizeof(int32_t), "persistentNetworkId (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_GROUP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddGroupCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseAddGroupCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_GROUP_WITH_CONFIG_REQ()
{
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0xe0;
    }
    string pskPassphrase = "v8";
    bool persistent = true;
    int32_t freq = 54;
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x5f;
    }
    bool joinExistingGroup = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddGroupWithConfigReq(ssid, pskPassphrase, persistent, freq, peerAddress, joinExistingGroup, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddGroupWithConfigReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseAddGroupWithConfigReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(ssid, param.ssid, "ssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(pskPassphrase, param.pskPassphrase, "pskPassphrase (string)"));
    VERIFY(COMPARE_BUF(&persistent, &param.persistent, sizeof(bool), "persistent (bool)"));
    VERIFY(COMPARE_BUF(&freq, &param.freq, sizeof(int32_t), "freq (int32_t)"));
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&joinExistingGroup, &param.joinExistingGroup, sizeof(bool), "joinExistingGroup (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_GROUP_WITH_CONFIG_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddGroupWithConfigCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseAddGroupWithConfigCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_NETWORK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddNetworkReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseAddNetworkReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_NETWORK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 84;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddNetworkCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddNetworkCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseAddNetworkCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_UPNP_SERVICE_REQ()
{
    int32_t version = 31;
    string serviceName = "i6mnkRs8exG4";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddUpnpServiceReq(version, serviceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddUpnpServiceReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseAddUpnpServiceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&version, &param.version, sizeof(int32_t), "version (int32_t)"));
    VERIFY(COMPARE_STR(serviceName, param.serviceName, "serviceName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ADD_UPNP_SERVICE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeAddUpnpServiceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseAddUpnpServiceCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_CONNECT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelConnectReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCancelConnectReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_CONNECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelConnectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCancelConnectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_SERVICE_DISCOVERY_REQ()
{
    int64_t identifier = 8815961425872026720;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelServiceDiscoveryReq(identifier, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int64_t param;
    bool msgParseResult = SupplicantP2pIfaceParseCancelServiceDiscoveryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&identifier, &param, sizeof(int64_t), "identifier (int64_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_SERVICE_DISCOVERY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelServiceDiscoveryCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCancelServiceDiscoveryCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_WPS_REQ()
{
    string groupIfName = "lpjF";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelWpsReq(groupIfName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseCancelWpsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param, "groupIfName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CANCEL_WPS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCancelWpsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCancelWpsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONFIGURE_EXT_LISTEN_REQ()
{
    int32_t periodInMillis = 96;
    int32_t intervalInMillis = 44;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConfigureExtListenReq(periodInMillis, intervalInMillis, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ConfigureExtListenReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseConfigureExtListenReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&periodInMillis, &param.periodInMillis, sizeof(int32_t), "periodInMillis (int32_t)"));
    VERIFY(COMPARE_BUF(&intervalInMillis, &param.intervalInMillis, sizeof(int32_t), "intervalInMillis (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONFIGURE_EXT_LISTEN_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConfigureExtListenCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseConfigureExtListenCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONNECT_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x08;
    }
    WpsProvisionMethod provisionMethod = WpsProvisionMethod(1);
    string preSelectedPin = "6ovg";
    bool joinExistingGroup = true;
    bool persistent = false;
    int32_t goIntent = 103;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConnectReq(peerAddress, provisionMethod, preSelectedPin, joinExistingGroup, persistent, goIntent, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ConnectReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseConnectReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&provisionMethod, &param.provisionMethod, sizeof(WpsProvisionMethod), "provisionMethod (WpsProvisionMethod)"));
    VERIFY(COMPARE_STR(preSelectedPin, param.preSelectedPin, "preSelectedPin (string)"));
    VERIFY(COMPARE_BUF(&joinExistingGroup, &param.joinExistingGroup, sizeof(bool), "joinExistingGroup (bool)"));
    VERIFY(COMPARE_BUF(&persistent, &param.persistent, sizeof(bool), "persistent (bool)"));
    VERIFY(COMPARE_BUF(&goIntent, &param.goIntent, sizeof(int32_t), "goIntent (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONNECT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "XsykPyiHU8KUzKGx";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConnectCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ConnectCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseConnectCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_REQUEST_MESSAGE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_REQUEST_MESSAGE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x20;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateNfcHandoverRequestMessageCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_SELECT_MESSAGE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_SELECT_MESSAGE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x41;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    CreateNfcHandoverSelectMessageCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ENABLE_WFD_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeEnableWfdReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantP2pIfaceParseEnableWfdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ENABLE_WFD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeEnableWfdCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseEnableWfdCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_REQ()
{
    int32_t timeoutInSec = 120;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindReq(timeoutInSec, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantP2pIfaceParseFindReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&timeoutInSec, &param, sizeof(int32_t), "timeoutInSec (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFindCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FLUSH_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFlushReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFlushReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FLUSH_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFlushCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFlushCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FLUSH_SERVICES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFlushServicesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFlushServicesReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FLUSH_SERVICES_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFlushServicesCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFlushServicesCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_DEVICE_ADDRESS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetDeviceAddressReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseGetDeviceAddressReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_DEVICE_ADDRESS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x4e;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetDeviceAddressCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetDeviceAddressCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetDeviceAddressCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_EDMG_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetEdmgReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseGetEdmgReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_EDMG_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetEdmgCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEdmgCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetEdmgCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_GROUP_CAPABILITY_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x9f;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetGroupCapabilityReq(peerAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseGetGroupCapabilityReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param, "peerAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_GROUP_CAPABILITY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    P2pGroupCapabilityMask result = P2pGroupCapabilityMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetGroupCapabilityCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetGroupCapabilityCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetGroupCapabilityCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(P2pGroupCapabilityMask), "result (P2pGroupCapabilityMask)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "h6drXFrzciWw1wN";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_NETWORK_REQ()
{
    int32_t id = 90;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetNetworkReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantP2pIfaceParseGetNetworkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_NETWORK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 34;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetNetworkCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNetworkCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetNetworkCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_SSID_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x38;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetSsidReq(peerAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseGetSsidReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param, "peerAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_SSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xf8;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetSsidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSsidCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetSsidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_TYPE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetTypeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseGetTypeReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_GET_TYPE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    IfaceType result = IfaceType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeGetTypeCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetTypeCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseGetTypeCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(IfaceType), "result (IfaceType)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_INVITE_REQ()
{
    string groupIfName = "2m2EdQ9";
    vector<uint8_t> goDeviceAddress;
    goDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        goDeviceAddress[index] = 0xeb;
    }
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0xf0;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeInviteReq(groupIfName, goDeviceAddress, peerAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InviteReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseInviteReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_VEC(goDeviceAddress, param.goDeviceAddress, "goDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_INVITE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeInviteCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseInviteCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_LIST_NETWORKS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeListNetworksReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseListNetworksReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_LIST_NETWORKS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<int32_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 35;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeListNetworksCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListNetworksCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseListNetworksCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<int32>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_PROVISION_DISCOVERY_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x18;
    }
    WpsProvisionMethod provisionMethod = WpsProvisionMethod(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeProvisionDiscoveryReq(peerAddress, provisionMethod, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ProvisionDiscoveryReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseProvisionDiscoveryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&provisionMethod, &param.provisionMethod, sizeof(WpsProvisionMethod), "provisionMethod (WpsProvisionMethod)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_PROVISION_DISCOVERY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeProvisionDiscoveryCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseProvisionDiscoveryCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REGISTER_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRegisterCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRegisterCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REGISTER_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRegisterCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRegisterCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REINVOKE_REQ()
{
    int32_t persistentNetworkId = 71;
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x24;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReinvokeReq(persistentNetworkId, peerAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ReinvokeReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseReinvokeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&persistentNetworkId, &param.persistentNetworkId, sizeof(int32_t), "persistentNetworkId (int32_t)"));
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REINVOKE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReinvokeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseReinvokeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REJECT_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0xef;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRejectReq(peerAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseRejectReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param, "peerAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REJECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRejectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRejectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_BONJOUR_SERVICE_REQ()
{
    vector<uint8_t> query;
    query.resize(4);
    for (int index = 0; index < 4; index++)
    {
        query[index] = 0x48;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveBonjourServiceReq(query, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseRemoveBonjourServiceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(query, param, "query (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_BONJOUR_SERVICE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveBonjourServiceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRemoveBonjourServiceCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_GROUP_REQ()
{
    string groupIfName = "pTr2";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveGroupReq(groupIfName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseRemoveGroupReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param, "groupIfName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_GROUP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveGroupCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRemoveGroupCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_NETWORK_REQ()
{
    int32_t id = 7;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveNetworkReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantP2pIfaceParseRemoveNetworkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_NETWORK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveNetworkCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRemoveNetworkCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_UPNP_SERVICE_REQ()
{
    int32_t version = 59;
    string serviceName = "vAQ831Djw";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveUpnpServiceReq(version, serviceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RemoveUpnpServiceReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseRemoveUpnpServiceReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&version, &param.version, sizeof(int32_t), "version (int32_t)"));
    VERIFY(COMPARE_STR(serviceName, param.serviceName, "serviceName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_UPNP_SERVICE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveUpnpServiceCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRemoveUpnpServiceCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_INITIATION_REQ()
{
    vector<uint8_t> select;
    select.resize(4);
    for (int index = 0; index < 4; index++)
    {
        select[index] = 0xb7;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReportNfcHandoverInitiationReq(select, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseReportNfcHandoverInitiationReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(select, param, "select (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_INITIATION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReportNfcHandoverInitiationCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseReportNfcHandoverInitiationCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_RESPONSE_REQ()
{
    vector<uint8_t> request;
    request.resize(4);
    for (int index = 0; index < 4; index++)
    {
        request[index] = 0xfe;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReportNfcHandoverResponseReq(request, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseReportNfcHandoverResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(request, param, "request (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeReportNfcHandoverResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseReportNfcHandoverResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REQUEST_SERVICE_DISCOVERY_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x7f;
    }
    vector<uint8_t> query;
    query.resize(4);
    for (int index = 0; index < 4; index++)
    {
        query[index] = 0x07;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRequestServiceDiscoveryReq(peerAddress, query, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RequestServiceDiscoveryReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseRequestServiceDiscoveryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(query, param.query, "query (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REQUEST_SERVICE_DISCOVERY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int64_t result = 1458356273926372279;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRequestServiceDiscoveryCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RequestServiceDiscoveryCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseRequestServiceDiscoveryCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int64_t), "result (int64_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SAVE_CONFIG_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSaveConfigReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSaveConfigReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SAVE_CONFIG_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSaveConfigCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSaveConfigCfm(payload.data(), payload.size());
    return true;
}

static void INIT_FREQ_RANGE(FreqRange& ranges)
{
    ranges.min = 22;
    ranges.max = 67;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_DISALLOWED_FREQUENCIES_REQ()
{
    vector<FreqRange> ranges;
    ranges.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_FREQ_RANGE(ranges[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetDisallowedFrequenciesReq(ranges, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<FreqRange> param;
    bool msgParseResult = SupplicantP2pIfaceParseSetDisallowedFrequenciesReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(ranges, param, "ranges (vector<FreqRange>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_DISALLOWED_FREQUENCIES_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetDisallowedFrequenciesCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetDisallowedFrequenciesCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_EDMG_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetEdmgReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantP2pIfaceParseSetEdmgReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_EDMG_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetEdmgCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetEdmgCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_GROUP_IDLE_REQ()
{
    string groupIfName = "X9x0PCXdYz";
    int32_t timeoutInSec = 66;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetGroupIdleReq(groupIfName, timeoutInSec, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetGroupIdleReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseSetGroupIdleReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_BUF(&timeoutInSec, &param.timeoutInSec, sizeof(int32_t), "timeoutInSec (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_GROUP_IDLE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetGroupIdleCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetGroupIdleCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_LISTEN_CHANNEL_REQ()
{
    int32_t channel = 121;
    int32_t operatingClass = 123;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetListenChannelReq(channel, operatingClass, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetListenChannelReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseSetListenChannelReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&channel, &param.channel, sizeof(int32_t), "channel (int32_t)"));
    VERIFY(COMPARE_BUF(&operatingClass, &param.operatingClass, sizeof(int32_t), "operatingClass (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_LISTEN_CHANNEL_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetListenChannelCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetListenChannelCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_MAC_RANDOMIZATION_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetMacRandomizationReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantP2pIfaceParseSetMacRandomizationReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_MAC_RANDOMIZATION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetMacRandomizationCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetMacRandomizationCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_MIRACAST_MODE_REQ()
{
    MiracastMode mode = MiracastMode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetMiracastModeReq(mode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    MiracastMode param;
    bool msgParseResult = SupplicantP2pIfaceParseSetMiracastModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&mode, &param, sizeof(MiracastMode), "mode (MiracastMode)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_MIRACAST_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetMiracastModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetMiracastModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_POWER_SAVE_REQ()
{
    string groupIfName = "Bj5ub8";
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetPowerSaveReq(groupIfName, enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetPowerSaveReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseSetPowerSaveReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_BUF(&enable, &param.enable, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_POWER_SAVE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetPowerSaveCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetPowerSaveCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_SSID_POSTFIX_REQ()
{
    vector<uint8_t> postfix;
    postfix.resize(4);
    for (int index = 0; index < 4; index++)
    {
        postfix[index] = 0x67;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetSsidPostfixReq(postfix, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseSetSsidPostfixReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(postfix, param, "postfix (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_SSID_POSTFIX_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetSsidPostfixCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetSsidPostfixCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WFD_DEVICE_INFO_REQ()
{
    vector<uint8_t> info;
    info.resize(4);
    for (int index = 0; index < 4; index++)
    {
        info[index] = 0x16;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWfdDeviceInfoReq(info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWfdDeviceInfoReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(info, param, "info (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WFD_DEVICE_INFO_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWfdDeviceInfoCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWfdDeviceInfoCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WFD_R2_DEVICE_INFO_REQ()
{
    vector<uint8_t> info;
    info.resize(4);
    for (int index = 0; index < 4; index++)
    {
        info[index] = 0xd3;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoReq(info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWfdR2DeviceInfoReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(info, param, "info (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WFD_R2_DEVICE_INFO_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWfdR2DeviceInfoCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_CLIENT_REQ()
{
    vector<uint8_t> peerAddress;
    peerAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        peerAddress[index] = 0x98;
    }
    bool isLegacyClient = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveClientReq(peerAddress, isLegacyClient, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RemoveClientReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseRemoveClientReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(peerAddress, param.peerAddress, "peerAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&isLegacyClient, &param.isLegacyClient, sizeof(bool), "isLegacyClient (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_REMOVE_CLIENT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeRemoveClientCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseRemoveClientCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_CONFIG_METHODS_REQ()
{
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsConfigMethodsReq(configMethods, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    WpsConfigMethods param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsConfigMethodsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&configMethods, &param, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_CONFIG_METHODS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsConfigMethodsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsConfigMethodsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_NAME_REQ()
{
    string name = "E8ijk";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsDeviceNameReq(name, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsDeviceNameReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(name, param, "name (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_NAME_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsDeviceNameCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsDeviceNameCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_TYPE_REQ()
{
    vector<uint8_t> type;
    type.resize(4);
    for (int index = 0; index < 4; index++)
    {
        type[index] = 0xa2;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsDeviceTypeReq(type, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsDeviceTypeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(type, param, "type (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_TYPE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsDeviceTypeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsDeviceTypeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MANUFACTURER_REQ()
{
    string manufacturer = "Aa62lFxper2S";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsManufacturerReq(manufacturer, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsManufacturerReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(manufacturer, param, "manufacturer (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MANUFACTURER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsManufacturerCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsManufacturerCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NAME_REQ()
{
    string modelName = "n8gGyA";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsModelNameReq(modelName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsModelNameReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(modelName, param, "modelName (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NAME_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsModelNameCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsModelNameCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NUMBER_REQ()
{
    string modelNumber = "Zz1BD";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsModelNumberReq(modelNumber, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsModelNumberReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(modelNumber, param, "modelNumber (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NUMBER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsModelNumberCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsModelNumberCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_SERIAL_NUMBER_REQ()
{
    string serialNumber = "3gsA32WxXFk";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsSerialNumberReq(serialNumber, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsSerialNumberReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(serialNumber, param, "serialNumber (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_WPS_SERIAL_NUMBER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetWpsSerialNumberCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetWpsSerialNumberCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PBC_REQ()
{
    string groupIfName = "HOa";
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x50;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPbcReq(groupIfName, bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsPbcReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPbcReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PBC_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPbcCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPbcCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_DISPLAY_REQ()
{
    string groupIfName = "Wt";
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xa6;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPinDisplayReq(groupIfName, bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsPinDisplayReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPinDisplayReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_DISPLAY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "7";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPinDisplayCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsPinDisplayCfmP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPinDisplayCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_KEYPAD_REQ()
{
    string groupIfName = "URV0zeeAQ2v8x";
    string pin = "1TXhSvlzA";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPinKeypadReq(groupIfName, pin, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsPinKeypadReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPinKeypadReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfName, param.groupIfName, "groupIfName (string)"));
    VERIFY(COMPARE_STR(pin, param.pin, "pin (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_KEYPAD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStartWpsPinKeypadCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseStartWpsPinKeypadCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_STOP_FIND_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStopFindReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseStopFindReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_STOP_FIND_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeStopFindCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseStopFindCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_ON_SOCIAL_CHANNELS_REQ()
{
    int32_t timeoutInSec = 86;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindOnSocialChannelsReq(timeoutInSec, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantP2pIfaceParseFindOnSocialChannelsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&timeoutInSec, &param, sizeof(int32_t), "timeoutInSec (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_ON_SOCIAL_CHANNELS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindOnSocialChannelsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFindOnSocialChannelsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_ON_SPECIFIC_FREQUENCY_REQ()
{
    int32_t freqInHz = 95;
    int32_t timeoutInSec = 114;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindOnSpecificFrequencyReq(freqInHz, timeoutInSec, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    FindOnSpecificFrequencyReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseFindOnSpecificFrequencyReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&freqInHz, &param.freqInHz, sizeof(int32_t), "freqInHz (int32_t)"));
    VERIFY(COMPARE_BUF(&timeoutInSec, &param.timeoutInSec, sizeof(int32_t), "timeoutInSec (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_FIND_ON_SPECIFIC_FREQUENCY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeFindOnSpecificFrequencyCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseFindOnSpecificFrequencyCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_VENDOR_ELEMENTS_REQ()
{
    P2pFrameTypeMask frameTypeMask = P2pFrameTypeMask(1);
    vector<uint8_t> vendorElemBytes;
    vendorElemBytes.resize(4);
    for (int index = 0; index < 4; index++)
    {
        vendorElemBytes[index] = 0xab;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetVendorElementsReq(frameTypeMask, vendorElemBytes, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetVendorElementsReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseSetVendorElementsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&frameTypeMask, &param.frameTypeMask, sizeof(P2pFrameTypeMask), "frameTypeMask (P2pFrameTypeMask)"));
    VERIFY(COMPARE_VEC(vendorElemBytes, param.vendorElemBytes, "vendorElemBytes (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_SET_VENDOR_ELEMENTS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeSetVendorElementsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseSetVendorElementsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONFIGURE_EAPOL_IP_ADDRESS_ALLOCATION_PARAMS_REQ()
{
    int32_t ipAddressGo = 107;
    int32_t ipAddressMask = 16;
    int32_t ipAddressStart = 36;
    int32_t ipAddressEnd = 94;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsReq(ipAddressGo, ipAddressMask, ipAddressStart, ipAddressEnd, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ConfigureEapolIpAddressAllocationParamsReqP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&ipAddressGo, &param.ipAddressGo, sizeof(int32_t), "ipAddressGo (int32_t)"));
    VERIFY(COMPARE_BUF(&ipAddressMask, &param.ipAddressMask, sizeof(int32_t), "ipAddressMask (int32_t)"));
    VERIFY(COMPARE_BUF(&ipAddressStart, &param.ipAddressStart, sizeof(int32_t), "ipAddressStart (int32_t)"));
    VERIFY(COMPARE_BUF(&ipAddressEnd, &param.ipAddressEnd, sizeof(int32_t), "ipAddressEnd (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_CONFIGURE_EAPOL_IP_ADDRESS_ALLOCATION_PARAMS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_DEVICE_FOUND_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0xa1;
    }
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0x37;
    }
    vector<uint8_t> primaryDeviceType;
    primaryDeviceType.resize(4);
    for (int index = 0; index < 4; index++)
    {
        primaryDeviceType[index] = 0x07;
    }
    string deviceName = "X9IGx2";
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    int8_t deviceCapabilities = 0x19;
    P2pGroupCapabilityMask groupCapabilities = P2pGroupCapabilityMask(1);
    vector<uint8_t> wfdDeviceInfo;
    wfdDeviceInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wfdDeviceInfo[index] = 0x9b;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnDeviceFoundInd(srcAddress, p2pDeviceAddress, primaryDeviceType, deviceName, configMethods, deviceCapabilities, groupCapabilities, wfdDeviceInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDeviceFoundIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnDeviceFoundInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(primaryDeviceType, param.primaryDeviceType, "primaryDeviceType (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(deviceName, param.deviceName, "deviceName (string)"));
    VERIFY(COMPARE_BUF(&configMethods, &param.configMethods, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    VERIFY(COMPARE_BUF(&deviceCapabilities, &param.deviceCapabilities, sizeof(int8_t), "deviceCapabilities (int8_t)"));
    VERIFY(COMPARE_BUF(&groupCapabilities, &param.groupCapabilities, sizeof(P2pGroupCapabilityMask), "groupCapabilities (P2pGroupCapabilityMask)"));
    VERIFY(COMPARE_VEC(wfdDeviceInfo, param.wfdDeviceInfo, "wfdDeviceInfo (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_DEVICE_LOST_IND()
{
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0xb8;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnDeviceLostInd(p2pDeviceAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantP2pIfaceParseOnDeviceLostInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param, "p2pDeviceAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_FIND_STOPPED_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnFindStoppedInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseOnFindStoppedInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GO_NEGOTIATION_COMPLETED_IND()
{
    P2pStatusCode status = P2pStatusCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGoNegotiationCompletedInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    P2pStatusCode param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGoNegotiationCompletedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param, sizeof(P2pStatusCode), "status (P2pStatusCode)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GO_NEGOTIATION_REQUEST_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0x96;
    }
    WpsDevPasswordId passwordId = WpsDevPasswordId(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGoNegotiationRequestInd(srcAddress, passwordId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnGoNegotiationRequestIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGoNegotiationRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&passwordId, &param.passwordId, sizeof(WpsDevPasswordId), "passwordId (WpsDevPasswordId)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_FORMATION_FAILURE_IND()
{
    string failureReason = "N6XbR3iu5O2to";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupFormationFailureInd(failureReason, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupFormationFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(failureReason, param, "failureReason (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_FORMATION_SUCCESS_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupFormationSuccessInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupFormationSuccessInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_REMOVED_IND()
{
    string groupIfname = "O7";
    bool isGroupOwner = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupRemovedInd(groupIfname, isGroupOwner, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnGroupRemovedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupRemovedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfname, param.groupIfname, "groupIfname (string)"));
    VERIFY(COMPARE_BUF(&isGroupOwner, &param.isGroupOwner, sizeof(bool), "isGroupOwner (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_STARTED_IND()
{
    string groupIfname = "nQ5xVYu";
    bool isGroupOwner = true;
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0xd0;
    }
    int32_t frequency = 95;
    vector<uint8_t> psk;
    psk.resize(4);
    for (int index = 0; index < 4; index++)
    {
        psk[index] = 0x4e;
    }
    string passphrase = "nVN";
    vector<uint8_t> goDeviceAddress;
    goDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        goDeviceAddress[index] = 0x74;
    }
    bool isPersistent = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupStartedInd(groupIfname, isGroupOwner, ssid, frequency, psk, passphrase, goDeviceAddress, isPersistent, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnGroupStartedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupStartedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfname, param.groupIfname, "groupIfname (string)"));
    VERIFY(COMPARE_BUF(&isGroupOwner, &param.isGroupOwner, sizeof(bool), "isGroupOwner (bool)"));
    VERIFY(COMPARE_VEC(ssid, param.ssid, "ssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&frequency, &param.frequency, sizeof(int32_t), "frequency (int32_t)"));
    VERIFY(COMPARE_VEC(psk, param.psk, "psk (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(passphrase, param.passphrase, "passphrase (string)"));
    VERIFY(COMPARE_VEC(goDeviceAddress, param.goDeviceAddress, "goDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&isPersistent, &param.isPersistent, sizeof(bool), "isPersistent (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_INVITATION_RECEIVED_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0xd1;
    }
    vector<uint8_t> goDeviceAddress;
    goDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        goDeviceAddress[index] = 0x16;
    }
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xa9;
    }
    int32_t persistentNetworkId = 68;
    int32_t operatingFrequency = 67;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnInvitationReceivedInd(srcAddress, goDeviceAddress, bssid, persistentNetworkId, operatingFrequency, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnInvitationReceivedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnInvitationReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(goDeviceAddress, param.goDeviceAddress, "goDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&persistentNetworkId, &param.persistentNetworkId, sizeof(int32_t), "persistentNetworkId (int32_t)"));
    VERIFY(COMPARE_BUF(&operatingFrequency, &param.operatingFrequency, sizeof(int32_t), "operatingFrequency (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_INVITATION_RESULT_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xa5;
    }
    P2pStatusCode status = P2pStatusCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnInvitationResultInd(bssid, status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnInvitationResultIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnInvitationResultInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(P2pStatusCode), "status (P2pStatusCode)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_PROVISION_DISCOVERY_COMPLETED_IND()
{
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0xf5;
    }
    bool isRequest = false;
    P2pProvDiscStatusCode status = P2pProvDiscStatusCode(1);
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    string generatedPin = "8Q";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnProvisionDiscoveryCompletedInd(p2pDeviceAddress, isRequest, status, configMethods, generatedPin, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnProvisionDiscoveryCompletedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnProvisionDiscoveryCompletedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&isRequest, &param.isRequest, sizeof(bool), "isRequest (bool)"));
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(P2pProvDiscStatusCode), "status (P2pProvDiscStatusCode)"));
    VERIFY(COMPARE_BUF(&configMethods, &param.configMethods, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    VERIFY(COMPARE_STR(generatedPin, param.generatedPin, "generatedPin (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_R2_DEVICE_FOUND_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0x53;
    }
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0xe0;
    }
    vector<uint8_t> primaryDeviceType;
    primaryDeviceType.resize(4);
    for (int index = 0; index < 4; index++)
    {
        primaryDeviceType[index] = 0x0f;
    }
    string deviceName = "6A35";
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    int8_t deviceCapabilities = 0xf5;
    P2pGroupCapabilityMask groupCapabilities = P2pGroupCapabilityMask(1);
    vector<uint8_t> wfdDeviceInfo;
    wfdDeviceInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wfdDeviceInfo[index] = 0x30;
    }
    vector<uint8_t> wfdR2DeviceInfo;
    wfdR2DeviceInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wfdR2DeviceInfo[index] = 0x4c;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnR2DeviceFoundInd(srcAddress, p2pDeviceAddress, primaryDeviceType, deviceName, configMethods, deviceCapabilities, groupCapabilities, wfdDeviceInfo, wfdR2DeviceInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnR2DeviceFoundIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnR2DeviceFoundInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(primaryDeviceType, param.primaryDeviceType, "primaryDeviceType (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(deviceName, param.deviceName, "deviceName (string)"));
    VERIFY(COMPARE_BUF(&configMethods, &param.configMethods, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    VERIFY(COMPARE_BUF(&deviceCapabilities, &param.deviceCapabilities, sizeof(int8_t), "deviceCapabilities (int8_t)"));
    VERIFY(COMPARE_BUF(&groupCapabilities, &param.groupCapabilities, sizeof(P2pGroupCapabilityMask), "groupCapabilities (P2pGroupCapabilityMask)"));
    VERIFY(COMPARE_VEC(wfdDeviceInfo, param.wfdDeviceInfo, "wfdDeviceInfo (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(wfdR2DeviceInfo, param.wfdR2DeviceInfo, "wfdR2DeviceInfo (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_SERVICE_DISCOVERY_RESPONSE_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0xd0;
    }
    char16_t updateIndicator = 'J';
    vector<uint8_t> tlvs;
    tlvs.resize(4);
    for (int index = 0; index < 4; index++)
    {
        tlvs[index] = 0x66;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnServiceDiscoveryResponseInd(srcAddress, updateIndicator, tlvs, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnServiceDiscoveryResponseIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnServiceDiscoveryResponseInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&updateIndicator, &param.updateIndicator, sizeof(char16_t), "updateIndicator (char16_t)"));
    VERIFY(COMPARE_VEC(tlvs, param.tlvs, "tlvs (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_STA_AUTHORIZED_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0xb9;
    }
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0xe9;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnStaAuthorizedInd(srcAddress, p2pDeviceAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnStaAuthorizedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnStaAuthorizedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_STA_DEAUTHORIZED_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0x71;
    }
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0x99;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnStaDeauthorizedInd(srcAddress, p2pDeviceAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnStaDeauthorizedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnStaDeauthorizedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_FREQUENCY_CHANGED_IND()
{
    string groupIfname = "4rSNmzMZbzA";
    int32_t frequency = 30;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupFrequencyChangedInd(groupIfname, frequency, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnGroupFrequencyChangedIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupFrequencyChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(groupIfname, param.groupIfname, "groupIfname (string)"));
    VERIFY(COMPARE_BUF(&frequency, &param.frequency, sizeof(int32_t), "frequency (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_IFACE_ON_DEVICE_FOUND_WITH_VENDOR_ELEMENTS_IND()
{
    vector<uint8_t> srcAddress;
    srcAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        srcAddress[index] = 0x0c;
    }
    vector<uint8_t> p2pDeviceAddress;
    p2pDeviceAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        p2pDeviceAddress[index] = 0x9c;
    }
    vector<uint8_t> primaryDeviceType;
    primaryDeviceType.resize(4);
    for (int index = 0; index < 4; index++)
    {
        primaryDeviceType[index] = 0x7b;
    }
    string deviceName = "FeIsIh";
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    int8_t deviceCapabilities = 0x81;
    P2pGroupCapabilityMask groupCapabilities = P2pGroupCapabilityMask(1);
    vector<uint8_t> wfdDeviceInfo;
    wfdDeviceInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wfdDeviceInfo[index] = 0x16;
    }
    vector<uint8_t> wfdR2DeviceInfo;
    wfdR2DeviceInfo.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wfdR2DeviceInfo[index] = 0x91;
    }
    vector<uint8_t> vendorElemBytes;
    vendorElemBytes.resize(4);
    for (int index = 0; index < 4; index++)
    {
        vendorElemBytes[index] = 0x2a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnDeviceFoundWithVendorElementsInd(srcAddress, p2pDeviceAddress, primaryDeviceType, deviceName, configMethods, deviceCapabilities, groupCapabilities, wfdDeviceInfo, wfdR2DeviceInfo, vendorElemBytes, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDeviceFoundWithVendorElementsIndP2pIfaceParam param;
    bool msgParseResult = SupplicantP2pIfaceParseOnDeviceFoundWithVendorElementsInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(srcAddress, param.srcAddress, "srcAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(p2pDeviceAddress, param.p2pDeviceAddress, "p2pDeviceAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(primaryDeviceType, param.primaryDeviceType, "primaryDeviceType (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(deviceName, param.deviceName, "deviceName (string)"));
    VERIFY(COMPARE_BUF(&configMethods, &param.configMethods, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    VERIFY(COMPARE_BUF(&deviceCapabilities, &param.deviceCapabilities, sizeof(int8_t), "deviceCapabilities (int8_t)"));
    VERIFY(COMPARE_BUF(&groupCapabilities, &param.groupCapabilities, sizeof(P2pGroupCapabilityMask), "groupCapabilities (P2pGroupCapabilityMask)"));
    VERIFY(COMPARE_VEC(wfdDeviceInfo, param.wfdDeviceInfo, "wfdDeviceInfo (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(wfdR2DeviceInfo, param.wfdR2DeviceInfo, "wfdR2DeviceInfo (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(vendorElemBytes, param.vendorElemBytes, "vendorElemBytes (vector<uint8_t>)"));
    return true;
}

static void INIT_P2P_CLIENT_EAPOL_IP_ADDRESS_INFO(P2pClientEapolIpAddressInfo& p2pClientIpInfo)
{
    p2pClientIpInfo.ipAddressClient = 115;
    p2pClientIpInfo.ipAddressMask = 3;
    p2pClientIpInfo.ipAddressGo = 73;
}

static void INIT_P2P_GROUP_STARTED_EVENT_PARAMS(P2pGroupStartedEventParams& groupStartedEventParams)
{
    groupStartedEventParams.groupInterfaceName = "Ol2aG";
    groupStartedEventParams.isGroupOwner = true;
    groupStartedEventParams.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        groupStartedEventParams.ssid[index] = 0x99;
    }
    groupStartedEventParams.frequencyMHz = 5;
    groupStartedEventParams.psk.resize(4);
    for (int index = 0; index < 4; index++)
    {
        groupStartedEventParams.psk[index] = 0xcf;
    }
    groupStartedEventParams.passphrase = "HUe2hXwms2oks";
    groupStartedEventParams.isPersistent = false;
    for (int index = 0; index < 6; index++)
    {
        groupStartedEventParams.goDeviceAddress[index] = 0x2f;
    }
    for (int index = 0; index < 6; index++)
    {
        groupStartedEventParams.goInterfaceAddress[index] = 0x8e;
    }
    groupStartedEventParams.isP2pClientEapolIpAddressInfoPresent = false;
    INIT_P2P_CLIENT_EAPOL_IP_ADDRESS_INFO(groupStartedEventParams.p2pClientIpInfo);
}

static bool T_SUPPLICANT_P2P_IFACE_ON_GROUP_STARTED_WITH_PARAMS_IND()
{
    P2pGroupStartedEventParams groupStartedEventParams;
    INIT_P2P_GROUP_STARTED_EVENT_PARAMS(groupStartedEventParams);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pIfaceSerializeOnGroupStartedWithParamsInd(groupStartedEventParams, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    P2pGroupStartedEventParams param;
    bool msgParseResult = SupplicantP2pIfaceParseOnGroupStartedWithParamsInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(groupStartedEventParams, param, "groupStartedEventParams (P2pGroupStartedEventParams)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_BSSID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetBssidReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetBssidReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_BSSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x8a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetBssidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetBssidCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetBssidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_CLIENT_LIST_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetClientListReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetClientListReq(payload.data(), payload.size());
    return true;
}

static void INIT_MAC_ADDRESS(MacAddress& result)
{
    result.data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result.data[index] = 0x60;
    }
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_CLIENT_LIST_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<MacAddress> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MAC_ADDRESS(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetClientListCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetClientListCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetClientListCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<MacAddress>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetIdReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 35;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetIdCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_INTERFACE_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetInterfaceNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetInterfaceNameReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_INTERFACE_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "7WDis";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetInterfaceNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetInterfaceNameCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetInterfaceNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_SSID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetSsidReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetSsidReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_SSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xc2;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetSsidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSsidCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetSsidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_TYPE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetTypeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseGetTypeReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_GET_TYPE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    IfaceType result = IfaceType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeGetTypeCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetTypeCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseGetTypeCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(IfaceType), "result (IfaceType)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_CURRENT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsCurrentReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseIsCurrentReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_CURRENT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsCurrentCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsCurrentCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseIsCurrentCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_GROUP_OWNER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsGroupOwnerReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseIsGroupOwnerReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_GROUP_OWNER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsGroupOwnerCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsGroupOwnerCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseIsGroupOwnerCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_PERSISTENT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsPersistentReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseIsPersistentReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_IS_PERSISTENT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeIsPersistentCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    IsPersistentCfmP2pNetworkParam param;
    bool msgParseResult = SupplicantP2pNetworkParseIsPersistentCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_SET_CLIENT_LIST_REQ()
{
    vector<MacAddress> clients;
    clients.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MAC_ADDRESS(clients[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeSetClientListReq(clients, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<MacAddress> param;
    bool msgParseResult = SupplicantP2pNetworkParseSetClientListReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(clients, param, "clients (vector<MacAddress>)"));
    return true;
}

static bool T_SUPPLICANT_P2P_NETWORK_SET_CLIENT_LIST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantP2pNetworkSerializeSetClientListCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantP2pNetworkParseSetClientListCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_DPP_PEER_URI_REQ()
{
    string uri = "7tsaMN";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddDppPeerUriReq(uri, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseAddDppPeerUriReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(uri, param, "uri (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_DPP_PEER_URI_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 112;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddDppPeerUriCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddDppPeerUriCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseAddDppPeerUriCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_EXT_RADIO_WORK_REQ()
{
    string name = "jbVqcGOm";
    int32_t freqInMhz = 85;
    int32_t timeoutInSec = 25;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddExtRadioWorkReq(name, freqInMhz, timeoutInSec, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddExtRadioWorkReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseAddExtRadioWorkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(name, param.name, "name (string)"));
    VERIFY(COMPARE_BUF(&freqInMhz, &param.freqInMhz, sizeof(int32_t), "freqInMhz (int32_t)"));
    VERIFY(COMPARE_BUF(&timeoutInSec, &param.timeoutInSec, sizeof(int32_t), "timeoutInSec (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_EXT_RADIO_WORK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 116;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddExtRadioWorkCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddExtRadioWorkCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseAddExtRadioWorkCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_NETWORK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddNetworkReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseAddNetworkReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_NETWORK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 63;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddNetworkCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddNetworkCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseAddNetworkCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_RX_FILTER_REQ()
{
    RxFilterType type = RxFilterType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddRxFilterReq(type, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RxFilterType param;
    bool msgParseResult = SupplicantStaIfaceParseAddRxFilterReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&type, &param, sizeof(RxFilterType), "type (RxFilterType)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ADD_RX_FILTER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddRxFilterCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseAddRxFilterCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_CANCEL_WPS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeCancelWpsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseCancelWpsReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_CANCEL_WPS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeCancelWpsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseCancelWpsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_DISCONNECT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeDisconnectReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseDisconnectReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_DISCONNECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeDisconnectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseDisconnectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ENABLE_AUTO_RECONNECT_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeEnableAutoReconnectReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseEnableAutoReconnectReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ENABLE_AUTO_RECONNECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeEnableAutoReconnectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseEnableAutoReconnectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_FILS_HLP_ADD_REQUEST_REQ()
{
    vector<uint8_t> dst_mac;
    dst_mac.resize(4);
    for (int index = 0; index < 4; index++)
    {
        dst_mac[index] = 0x35;
    }
    vector<uint8_t> pkt;
    pkt.resize(4);
    for (int index = 0; index < 4; index++)
    {
        pkt[index] = 0x77;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeFilsHlpAddRequestReq(dst_mac, pkt, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    FilsHlpAddRequestReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseFilsHlpAddRequestReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(dst_mac, param.dst_mac, "dst_mac (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(pkt, param.pkt, "pkt (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_FILS_HLP_ADD_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeFilsHlpAddRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseFilsHlpAddRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_FILS_HLP_FLUSH_REQUEST_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeFilsHlpFlushRequestReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseFilsHlpFlushRequestReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_FILS_HLP_FLUSH_REQUEST_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeFilsHlpFlushRequestCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseFilsHlpFlushRequestCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GENERATE_DPP_BOOTSTRAP_INFO_FOR_RESPONDER_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0xb1;
    }
    string deviceInfo = "PzfyqXb";
    DppCurve curve = DppCurve(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderReq(macAddress, deviceInfo, curve, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GenerateDppBootstrapInfoForResponderReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param.macAddress, "macAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(deviceInfo, param.deviceInfo, "deviceInfo (string)"));
    VERIFY(COMPARE_BUF(&curve, &param.curve, sizeof(DppCurve), "curve (DppCurve)"));
    return true;
}

static void INIT_DPP_RESPONDER_BOOTSTRAP_INFO(DppResponderBootstrapInfo& result)
{
    result.bootstrapId = 34;
    result.listenChannel = 40;
    result.uri = "q8gkdkRD9xYBV";
}

static bool T_SUPPLICANT_STA_IFACE_GENERATE_DPP_BOOTSTRAP_INFO_FOR_RESPONDER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    DppResponderBootstrapInfo result;
    INIT_DPP_RESPONDER_BOOTSTRAP_INFO(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GenerateDppBootstrapInfoForResponderCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (DppResponderBootstrapInfo)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GENERATE_SELF_DPP_CONFIGURATION_REQ()
{
    string ssid = "7b";
    vector<uint8_t> privEcKey;
    privEcKey.resize(4);
    for (int index = 0; index < 4; index++)
    {
        privEcKey[index] = 0x4f;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGenerateSelfDppConfigurationReq(ssid, privEcKey, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GenerateSelfDppConfigurationReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGenerateSelfDppConfigurationReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ssid, param.ssid, "ssid (string)"));
    VERIFY(COMPARE_VEC(privEcKey, param.privEcKey, "privEcKey (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GENERATE_SELF_DPP_CONFIGURATION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGenerateSelfDppConfigurationCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGenerateSelfDppConfigurationCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_CONNECTION_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetConnectionCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetConnectionCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static void INIT_CONNECTION_CAPABILITIES(ConnectionCapabilities& result)
{
    result.technology = WifiTechnology(1);
    result.channelBandwidth = 47;
    result.maxNumberTxSpatialStreams = 99;
    result.maxNumberRxSpatialStreams = 86;
    result.legacyMode = LegacyMode(1);
    result.apTidToLinkMapNegotiationSupported = true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_CONNECTION_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    ConnectionCapabilities result;
    INIT_CONNECTION_CAPABILITIES(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetConnectionCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetConnectionCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetConnectionCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (ConnectionCapabilities)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_CONNECTION_MLO_LINKS_INFO_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetConnectionMloLinksInfoReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetConnectionMloLinksInfoReq(payload.data(), payload.size());
    return true;
}

static void INIT_MLO_LINK(MloLink& links)
{
    links.linkId = 0xe5;
    links.staLinkMacAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        links.staLinkMacAddress[index] = 0xb7;
    }
    links.tidsUplinkMap = 0x82;
    links.tidsDownlinkMap = 0x5c;
    array<uint8_t, 6> param;
    for (int index = 0; index < 6; index++)
    {
        param[index] = 0x26;
    }
    links.apLinkMacAddress = param;
    links.frequencyMHz = 51;
}

static void INIT_MLO_LINKS_INFO(MloLinksInfo& result)
{
    result.links.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_MLO_LINK(result.links[index]);
    }
    result.apMloLinkId = 4;
    array<uint8_t, 6> param;
    for (int index = 0; index < 6; index++)
    {
        param[index] = 0x4f;
    }
    result.apMldMacAddress = param;
}

static bool T_SUPPLICANT_STA_IFACE_GET_CONNECTION_MLO_LINKS_INFO_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    MloLinksInfo result;
    INIT_MLO_LINKS_INFO(result);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetConnectionMloLinksInfoCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetConnectionMloLinksInfoCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetConnectionMloLinksInfoCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_OBJ(result, param.result, "result (MloLinksInfo)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_KEY_MGMT_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetKeyMgmtCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_KEY_MGMT_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    KeyMgmtMask result = KeyMgmtMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetKeyMgmtCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetKeyMgmtCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(KeyMgmtMask), "result (KeyMgmtMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_MAC_ADDRESS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetMacAddressReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetMacAddressReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_MAC_ADDRESS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x49;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetMacAddressCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetMacAddressCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetMacAddressCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetNameReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "D43qaINL3BxLo";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNameCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_NETWORK_REQ()
{
    int32_t id = 8;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetNetworkReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseGetNetworkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_NETWORK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 87;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetNetworkCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetNetworkCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetNetworkCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_TYPE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetTypeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetTypeReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_TYPE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    IfaceType result = IfaceType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetTypeCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetTypeCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetTypeCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(IfaceType), "result (IfaceType)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_WPA_DRIVER_CAPABILITIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetWpaDriverCapabilitiesReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_WPA_DRIVER_CAPABILITIES_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    WpaDriverCapabilitiesMask result = WpaDriverCapabilitiesMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWpaDriverCapabilitiesCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetWpaDriverCapabilitiesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(WpaDriverCapabilitiesMask), "result (WpaDriverCapabilitiesMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_ANQP_QUERY_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0x91;
    }
    vector<AnqpInfoId> infoElements;
    infoElements.resize(4);
    for (int index = 0; index < 4; index++)
    {
        infoElements[index] = AnqpInfoId(1);
    }
    vector<Hs20AnqpSubtypes> subTypes;
    subTypes.resize(4);
    for (int index = 0; index < 4; index++)
    {
        subTypes[index] = Hs20AnqpSubtypes(1);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateAnqpQueryReq(macAddress, infoElements, subTypes, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InitiateAnqpQueryReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateAnqpQueryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param.macAddress, "macAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(infoElements, param.infoElements, "infoElements (vector<AnqpInfoId>)"));
    VERIFY(COMPARE_VEC(subTypes, param.subTypes, "subTypes (vector<Hs20AnqpSubtypes>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_ANQP_QUERY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateAnqpQueryCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateAnqpQueryCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_HS20_ICON_QUERY_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0x6a;
    }
    string fileName = "lnM";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateHs20IconQueryReq(macAddress, fileName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    InitiateHs20IconQueryReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateHs20IconQueryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param.macAddress, "macAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(fileName, param.fileName, "fileName (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_HS20_ICON_QUERY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateHs20IconQueryCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateHs20IconQueryCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_DISCOVER_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0x11;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsDiscoverReq(macAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsDiscoverReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param, "macAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_DISCOVER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsDiscoverCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsDiscoverCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_SETUP_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0x7b;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsSetupReq(macAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsSetupReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param, "macAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_SETUP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsSetupCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsSetupCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_TEARDOWN_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0xa9;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsTeardownReq(macAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsTeardownReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param, "macAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_TEARDOWN_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateTdlsTeardownCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateTdlsTeardownCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_VENUE_URL_ANQP_QUERY_REQ()
{
    vector<uint8_t> macAddress;
    macAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        macAddress[index] = 0x7f;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryReq(macAddress, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(macAddress, param, "macAddress (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_INITIATE_VENUE_URL_ANQP_QUERY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_LIST_NETWORKS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeListNetworksReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseListNetworksReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_LIST_NETWORKS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<int32_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 12;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeListNetworksCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ListNetworksCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseListNetworksCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<int32>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REASSOCIATE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeReassociateReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseReassociateReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REASSOCIATE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeReassociateCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseReassociateCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_RECONNECT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeReconnectReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseReconnectReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_RECONNECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeReconnectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseReconnectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REGISTER_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRegisterCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRegisterCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REGISTER_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRegisterCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRegisterCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_QOS_POLICY_FEATURE_ENABLED_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetQosPolicyFeatureEnabledReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_QOS_POLICY_FEATURE_ENABLED_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetQosPolicyFeatureEnabledCfm(payload.data(), payload.size());
    return true;
}

static void INIT_QOS_POLICY_STATUS(QosPolicyStatus& qosPolicyStatusList)
{
    qosPolicyStatusList.policyId = 0x94;
    qosPolicyStatusList.status = QosPolicyStatusCode(1);
}

static bool T_SUPPLICANT_STA_IFACE_SEND_QOS_POLICY_RESPONSE_REQ()
{
    int32_t qosPolicyRequestId = 0;
    bool morePolicies = true;
    vector<QosPolicyStatus> qosPolicyStatusList;
    qosPolicyStatusList.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_STATUS(qosPolicyStatusList[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSendQosPolicyResponseReq(qosPolicyRequestId, morePolicies, qosPolicyStatusList, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SendQosPolicyResponseReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseSendQosPolicyResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&qosPolicyRequestId, &param.qosPolicyRequestId, sizeof(int32_t), "qosPolicyRequestId (int32_t)"));
    VERIFY(COMPARE_BUF(&morePolicies, &param.morePolicies, sizeof(bool), "morePolicies (bool)"));
    VERIFY(COMPARE_VEC(qosPolicyStatusList, param.qosPolicyStatusList, "qosPolicyStatusList (vector<QosPolicyStatus>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SEND_QOS_POLICY_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSendQosPolicyResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSendQosPolicyResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_ALL_QOS_POLICIES_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveAllQosPoliciesReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveAllQosPoliciesReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_ALL_QOS_POLICIES_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveAllQosPoliciesCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveAllQosPoliciesCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_DPP_URI_REQ()
{
    int32_t id = 110;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveDppUriReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveDppUriReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_DPP_URI_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveDppUriCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveDppUriCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_EXT_RADIO_WORK_REQ()
{
    int32_t id = 40;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveExtRadioWorkReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveExtRadioWorkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_EXT_RADIO_WORK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveExtRadioWorkCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveExtRadioWorkCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_NETWORK_REQ()
{
    int32_t id = 8;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveNetworkReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveNetworkReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_NETWORK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveNetworkCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveNetworkCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_RX_FILTER_REQ()
{
    RxFilterType type = RxFilterType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveRxFilterReq(type, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RxFilterType param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveRxFilterReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&type, &param, sizeof(RxFilterType), "type (RxFilterType)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_RX_FILTER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveRxFilterCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseRemoveRxFilterCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_MODE_REQ()
{
    BtCoexistenceMode mode = BtCoexistenceMode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetBtCoexistenceModeReq(mode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    BtCoexistenceMode param;
    bool msgParseResult = SupplicantStaIfaceParseSetBtCoexistenceModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&mode, &param, sizeof(BtCoexistenceMode), "mode (BtCoexistenceMode)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetBtCoexistenceModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetBtCoexistenceModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_SCAN_MODE_ENABLED_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_SCAN_MODE_ENABLED_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_COUNTRY_CODE_REQ()
{
    vector<uint8_t> code;
    code.resize(4);
    for (int index = 0; index < 4; index++)
    {
        code[index] = 0x9b;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetCountryCodeReq(code, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseSetCountryCodeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(code, param, "code (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_COUNTRY_CODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetCountryCodeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetCountryCodeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_EXTERNAL_SIM_REQ()
{
    bool useExternalSim = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetExternalSimReq(useExternalSim, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetExternalSimReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&useExternalSim, &param, sizeof(bool), "useExternalSim (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_EXTERNAL_SIM_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetExternalSimCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetExternalSimCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_MBO_CELLULAR_DATA_STATUS_REQ()
{
    bool available = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetMboCellularDataStatusReq(available, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetMboCellularDataStatusReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&available, &param, sizeof(bool), "available (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_MBO_CELLULAR_DATA_STATUS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetMboCellularDataStatusCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetMboCellularDataStatusCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_POWER_SAVE_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetPowerSaveReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetPowerSaveReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_POWER_SAVE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetPowerSaveCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetPowerSaveCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_SUSPEND_MODE_ENABLED_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetSuspendModeEnabledReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaIfaceParseSetSuspendModeEnabledReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_SUSPEND_MODE_ENABLED_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetSuspendModeEnabledCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetSuspendModeEnabledCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_CONFIG_METHODS_REQ()
{
    WpsConfigMethods configMethods = WpsConfigMethods(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsConfigMethodsReq(configMethods, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    WpsConfigMethods param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsConfigMethodsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&configMethods, &param, sizeof(WpsConfigMethods), "configMethods (WpsConfigMethods)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_CONFIG_METHODS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsConfigMethodsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsConfigMethodsCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_NAME_REQ()
{
    string name = "hB";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsDeviceNameReq(name, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsDeviceNameReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(name, param, "name (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_NAME_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsDeviceNameCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsDeviceNameCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_TYPE_REQ()
{
    vector<uint8_t> type;
    type.resize(4);
    for (int index = 0; index < 4; index++)
    {
        type[index] = 0x53;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsDeviceTypeReq(type, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsDeviceTypeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(type, param, "type (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_TYPE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsDeviceTypeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsDeviceTypeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MANUFACTURER_REQ()
{
    string manufacturer = "xuJrGl6b";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsManufacturerReq(manufacturer, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsManufacturerReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(manufacturer, param, "manufacturer (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MANUFACTURER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsManufacturerCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsManufacturerCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NAME_REQ()
{
    string modelName = "Guzxx2sln03gOa";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsModelNameReq(modelName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsModelNameReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(modelName, param, "modelName (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NAME_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsModelNameCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsModelNameCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NUMBER_REQ()
{
    string modelNumber = "hvybPkrKFJWsxEq";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsModelNumberReq(modelNumber, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsModelNumberReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(modelNumber, param, "modelNumber (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NUMBER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsModelNumberCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsModelNumberCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_SERIAL_NUMBER_REQ()
{
    string serialNumber = "nM8331TY3";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsSerialNumberReq(serialNumber, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseSetWpsSerialNumberReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(serialNumber, param, "serialNumber (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_SET_WPS_SERIAL_NUMBER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeSetWpsSerialNumberCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseSetWpsSerialNumberCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_CONFIGURATOR_INITIATOR_REQ()
{
    int32_t peerBootstrapId = 56;
    int32_t ownBootstrapId = 67;
    string ssid = "gSrHvBnrJ1lge";
    string password = "Ngcp4Q6pzy";
    string psk = "QQlU1uvcU";
    DppNetRole netRole = DppNetRole(1);
    DppAkm securityAkm = DppAkm(1);
    vector<uint8_t> privEcKey;
    privEcKey.resize(4);
    for (int index = 0; index < 4; index++)
    {
        privEcKey[index] = 0x5a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorReq(peerBootstrapId, ownBootstrapId, ssid, password, psk, netRole, securityAkm, privEcKey, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartDppConfiguratorInitiatorReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseStartDppConfiguratorInitiatorReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&peerBootstrapId, &param.peerBootstrapId, sizeof(int32_t), "peerBootstrapId (int32_t)"));
    VERIFY(COMPARE_BUF(&ownBootstrapId, &param.ownBootstrapId, sizeof(int32_t), "ownBootstrapId (int32_t)"));
    VERIFY(COMPARE_STR(ssid, param.ssid, "ssid (string)"));
    VERIFY(COMPARE_STR(password, param.password, "password (string)"));
    VERIFY(COMPARE_STR(psk, param.psk, "psk (string)"));
    VERIFY(COMPARE_BUF(&netRole, &param.netRole, sizeof(DppNetRole), "netRole (DppNetRole)"));
    VERIFY(COMPARE_BUF(&securityAkm, &param.securityAkm, sizeof(DppAkm), "securityAkm (DppAkm)"));
    VERIFY(COMPARE_VEC(privEcKey, param.privEcKey, "privEcKey (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_CONFIGURATOR_INITIATOR_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x6a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartDppConfiguratorInitiatorCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseStartDppConfiguratorInitiatorCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_INITIATOR_REQ()
{
    int32_t peerBootstrapId = 60;
    int32_t ownBootstrapId = 60;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorReq(peerBootstrapId, ownBootstrapId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartDppEnrolleeInitiatorReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseStartDppEnrolleeInitiatorReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&peerBootstrapId, &param.peerBootstrapId, sizeof(int32_t), "peerBootstrapId (int32_t)"));
    VERIFY(COMPARE_BUF(&ownBootstrapId, &param.ownBootstrapId, sizeof(int32_t), "ownBootstrapId (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_INITIATOR_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartDppEnrolleeInitiatorCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_RESPONDER_REQ()
{
    int32_t listenChannel = 118;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppEnrolleeResponderReq(listenChannel, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseStartDppEnrolleeResponderReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&listenChannel, &param, sizeof(int32_t), "listenChannel (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_RESPONDER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartDppEnrolleeResponderCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartDppEnrolleeResponderCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_RX_FILTER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartRxFilterReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartRxFilterReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_RX_FILTER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartRxFilterCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartRxFilterCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PBC_REQ()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xec;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPbcReq(bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPbcReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PBC_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPbcCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPbcCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PIN_DISPLAY_REQ()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x77;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPinDisplayReq(bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPinDisplayReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PIN_DISPLAY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "1T";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPinDisplayCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsPinDisplayCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPinDisplayCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PIN_KEYPAD_REQ()
{
    string pin = "eks";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPinKeypadReq(pin, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPinKeypadReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(pin, param, "pin (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_PIN_KEYPAD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsPinKeypadCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartWpsPinKeypadCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_REGISTRAR_REQ()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x97;
    }
    string pin = "90RpXeb";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsRegistrarReq(bssid, pin, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    StartWpsRegistrarReqStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseStartWpsRegistrarReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(pin, param.pin, "pin (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_START_WPS_REGISTRAR_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStartWpsRegistrarCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStartWpsRegistrarCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_DPP_INITIATOR_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopDppInitiatorReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStopDppInitiatorReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_DPP_INITIATOR_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopDppInitiatorCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStopDppInitiatorCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_DPP_RESPONDER_REQ()
{
    int32_t ownBootstrapId = 91;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopDppResponderReq(ownBootstrapId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseStopDppResponderReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&ownBootstrapId, &param, sizeof(int32_t), "ownBootstrapId (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_DPP_RESPONDER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopDppResponderCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStopDppResponderCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_RX_FILTER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopRxFilterReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStopRxFilterReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_STOP_RX_FILTER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeStopRxFilterCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseStopRxFilterCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_GET_SIGNAL_POLL_RESULTS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetSignalPollResultsReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseGetSignalPollResultsReq(payload.data(), payload.size());
    return true;
}

static void INIT_SIGNAL_POLL_RESULT(SignalPollResult& result)
{
    result.linkId = 37;
    result.currentRssiDbm = 109;
    result.txBitrateMbps = 11;
    result.rxBitrateMbps = 110;
    result.frequencyMhz = 19;
}

static bool T_SUPPLICANT_STA_IFACE_GET_SIGNAL_POLL_RESULTS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<SignalPollResult> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_SIGNAL_POLL_RESULT(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeGetSignalPollResultsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSignalPollResultsCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseGetSignalPollResultsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<SignalPollResult>)"));
    return true;
}

static void INIT_PORT_RANGE(PortRange& dstPortRange)
{
    dstPortRange.startPort = 73;
    dstPortRange.endPort = 59;
}

static void INIT_QOS_POLICY_CLASSIFIER_PARAMS(QosPolicyClassifierParams& classifierParams)
{
    classifierParams.ipVersion = IpVersion(1);
    classifierParams.classifierParamMask = QosPolicyClassifierParamsMask(1);
    classifierParams.srcIp.resize(4);
    for (int index = 0; index < 4; index++)
    {
        classifierParams.srcIp[index] = 0xe9;
    }
    classifierParams.dstIp.resize(4);
    for (int index = 0; index < 4; index++)
    {
        classifierParams.dstIp[index] = 0x68;
    }
    classifierParams.srcPort = 78;
    INIT_PORT_RANGE(classifierParams.dstPortRange);
    classifierParams.protocolNextHdr = ProtocolNextHeader(1);
    classifierParams.flowLabelIpv6.resize(4);
    for (int index = 0; index < 4; index++)
    {
        classifierParams.flowLabelIpv6[index] = 0xa7;
    }
    classifierParams.domainName = "qz0QXn6";
    classifierParams.dscp = 0x3a;
}

static void INIT_QOS_POLICY_SCS_DATA(QosPolicyScsData& qosPolicyData)
{
    qosPolicyData.policyId = 0x70;
    qosPolicyData.userPriority = 0x96;
    INIT_QOS_POLICY_CLASSIFIER_PARAMS(qosPolicyData.classifierParams);
}

static bool T_SUPPLICANT_STA_IFACE_ADD_QOS_POLICY_REQUEST_FOR_SCS_REQ()
{
    vector<QosPolicyScsData> qosPolicyData;
    qosPolicyData.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_SCS_DATA(qosPolicyData[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddQosPolicyRequestForScsReq(qosPolicyData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<QosPolicyScsData> param;
    bool msgParseResult = SupplicantStaIfaceParseAddQosPolicyRequestForScsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(qosPolicyData, param, "qosPolicyData (vector<QosPolicyScsData>)"));
    return true;
}

static void INIT_QOS_POLICY_SCS_REQUEST_STATUS(QosPolicyScsRequestStatus& result)
{
    result.policyId = 0x1e;
    result.qosPolicyScsRequestStatusCode = QosPolicyScsRequestStatusCode(1);
}

static bool T_SUPPLICANT_STA_IFACE_ADD_QOS_POLICY_REQUEST_FOR_SCS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<QosPolicyScsRequestStatus> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_SCS_REQUEST_STATUS(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeAddQosPolicyRequestForScsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddQosPolicyRequestForScsCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseAddQosPolicyRequestForScsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<QosPolicyScsRequestStatus>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_QOS_POLICY_FOR_SCS_REQ()
{
    vector<uint8_t> scsPolicyIds;
    scsPolicyIds.resize(4);
    for (int index = 0; index < 4; index++)
    {
        scsPolicyIds[index] = 0x55;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveQosPolicyForScsReq(scsPolicyIds, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveQosPolicyForScsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(scsPolicyIds, param, "scsPolicyIds (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_REMOVE_QOS_POLICY_FOR_SCS_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<QosPolicyScsRequestStatus> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_SCS_REQUEST_STATUS(result[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeRemoveQosPolicyForScsCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    RemoveQosPolicyForScsCfmStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseRemoveQosPolicyForScsCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<QosPolicyScsRequestStatus>)"));
    return true;
}

static void INIT_ANQP_DATA(AnqpData& data)
{
    data.venueName.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.venueName[index] = 0x2a;
    }
    data.roamingConsortium.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.roamingConsortium[index] = 0x3c;
    }
    data.ipAddrTypeAvailability.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.ipAddrTypeAvailability[index] = 0x72;
    }
    data.naiRealm.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.naiRealm[index] = 0x7a;
    }
    data.anqp3gppCellularNetwork.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.anqp3gppCellularNetwork[index] = 0x89;
    }
    data.domainName.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.domainName[index] = 0x2a;
    }
    data.venueUrl.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data.venueUrl[index] = 0x62;
    }
}

static void INIT_HS20_ANQP_DATA(Hs20AnqpData& hs20Data)
{
    hs20Data.operatorFriendlyName.resize(4);
    for (int index = 0; index < 4; index++)
    {
        hs20Data.operatorFriendlyName[index] = 0x88;
    }
    hs20Data.wanMetrics.resize(4);
    for (int index = 0; index < 4; index++)
    {
        hs20Data.wanMetrics[index] = 0x44;
    }
    hs20Data.connectionCapability.resize(4);
    for (int index = 0; index < 4; index++)
    {
        hs20Data.connectionCapability[index] = 0x59;
    }
    hs20Data.osuProvidersList.resize(4);
    for (int index = 0; index < 4; index++)
    {
        hs20Data.osuProvidersList[index] = 0x2d;
    }
}

static bool T_SUPPLICANT_STA_IFACE_ON_ANQP_QUERY_DONE_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xb8;
    }
    AnqpData data;
    INIT_ANQP_DATA(data);
    Hs20AnqpData hs20Data;
    INIT_HS20_ANQP_DATA(hs20Data);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnAnqpQueryDoneInd(bssid, data, hs20Data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnAnqpQueryDoneIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnAnqpQueryDoneInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_OBJ(data, param.data, "data (AnqpData)"));
    VERIFY(COMPARE_OBJ(hs20Data, param.hs20Data, "hs20Data (Hs20AnqpData)"));
    return true;
}

static void INIT_OCE_RSSI_BASED_ASSOC_REJECT_ATTR(OceRssiBasedAssocRejectAttr& oceRssiBasedAssocRejectData)
{
    oceRssiBasedAssocRejectData.deltaRssi = 102;
    oceRssiBasedAssocRejectData.retryDelayS = 56;
}

static void INIT_ASSOCIATION_REJECTION_DATA(AssociationRejectionData& assocRejectData)
{
    assocRejectData.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        assocRejectData.ssid[index] = 0x3d;
    }
    assocRejectData.bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        assocRejectData.bssid[index] = 0xe7;
    }
    assocRejectData.statusCode = StaIfaceStatusCode(1);
    assocRejectData.timedOut = true;
    assocRejectData.isMboAssocDisallowedReasonCodePresent = true;
    assocRejectData.mboAssocDisallowedReason = MboAssocDisallowedReasonCode(1);
    assocRejectData.isOceRssiBasedAssocRejectAttrPresent = true;
    INIT_OCE_RSSI_BASED_ASSOC_REJECT_ATTR(assocRejectData.oceRssiBasedAssocRejectData);
}

static bool T_SUPPLICANT_STA_IFACE_ON_ASSOCIATION_REJECTED_IND()
{
    AssociationRejectionData assocRejectData;
    INIT_ASSOCIATION_REJECTION_DATA(assocRejectData);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnAssociationRejectedInd(assocRejectData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AssociationRejectionData param;
    bool msgParseResult = SupplicantStaIfaceParseOnAssociationRejectedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(assocRejectData, param, "assocRejectData (AssociationRejectionData)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_AUTHENTICATION_TIMEOUT_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x18;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnAuthenticationTimeoutInd(bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseOnAuthenticationTimeoutInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_AUXILIARY_SUPPLICANT_EVENT_IND()
{
    AuxiliarySupplicantEventCode eventCode = AuxiliarySupplicantEventCode(1);
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x14;
    }
    string reasonString = "eRN52Z0xQhI";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnAuxiliarySupplicantEventInd(eventCode, bssid, reasonString, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnAuxiliarySupplicantEventIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnAuxiliarySupplicantEventInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&eventCode, &param.eventCode, sizeof(AuxiliarySupplicantEventCode), "eventCode (AuxiliarySupplicantEventCode)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(reasonString, param.reasonString, "reasonString (string)"));
    return true;
}

static void INIT_BSS_TM_DATA(BssTmData& tmData)
{
    tmData.status = BssTmStatusCode(1);
    tmData.flags = BssTmDataFlagsMask(1);
    tmData.assocRetryDelayMs = 74;
    tmData.mboTransitionReason = MboTransitionReasonCode(1);
    tmData.mboCellPreference = MboCellularDataConnectionPrefValue(1);
}

static bool T_SUPPLICANT_STA_IFACE_ON_BSS_TM_HANDLING_DONE_IND()
{
    BssTmData tmData;
    INIT_BSS_TM_DATA(tmData);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnBssTmHandlingDoneInd(tmData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    BssTmData param;
    bool msgParseResult = SupplicantStaIfaceParseOnBssTmHandlingDoneInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(tmData, param, "tmData (BssTmData)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_BSSID_CHANGED_IND()
{
    BssidChangeReason reason = BssidChangeReason(1);
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x2b;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnBssidChangedInd(reason, bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnBssidChangedIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnBssidChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&reason, &param.reason, sizeof(BssidChangeReason), "reason (BssidChangeReason)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DISCONNECTED_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x24;
    }
    bool locallyGenerated = true;
    StaIfaceReasonCode reasonCode = StaIfaceReasonCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDisconnectedInd(bssid, locallyGenerated, reasonCode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDisconnectedIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnDisconnectedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&locallyGenerated, &param.locallyGenerated, sizeof(bool), "locallyGenerated (bool)"));
    VERIFY(COMPARE_BUF(&reasonCode, &param.reasonCode, sizeof(StaIfaceReasonCode), "reasonCode (StaIfaceReasonCode)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_FAILURE_IND()
{
    DppFailureCode code = DppFailureCode(1);
    string ssid = "11X1YC2Q";
    string channelList = "YvS4cvI";
    vector<char16_t> bandList;
    bandList.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bandList[index] = 'L';
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppFailureInd(code, ssid, channelList, bandList, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDppFailureIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&code, &param.code, sizeof(DppFailureCode), "code (DppFailureCode)"));
    VERIFY(COMPARE_STR(ssid, param.ssid, "ssid (string)"));
    VERIFY(COMPARE_STR(channelList, param.channelList, "channelList (string)"));
    VERIFY(COMPARE_VEC(bandList, param.bandList, "bandList (vector<char>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_PROGRESS_IND()
{
    DppProgressCode code = DppProgressCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppProgressInd(code, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DppProgressCode param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppProgressInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&code, &param, sizeof(DppProgressCode), "code (DppProgressCode)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_IND()
{
    DppEventType event = DppEventType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppSuccessInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DppEventType param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppSuccessInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&event, &param, sizeof(DppEventType), "event (DppEventType)"));
    return true;
}

static void INIT_DPP_CONNECTION_KEYS(DppConnectionKeys& dppConnectionKeys)
{
    dppConnectionKeys.connector.resize(4);
    for (int index = 0; index < 4; index++)
    {
        dppConnectionKeys.connector[index] = 0x5e;
    }
    dppConnectionKeys.cSign.resize(4);
    for (int index = 0; index < 4; index++)
    {
        dppConnectionKeys.cSign[index] = 0xfc;
    }
    dppConnectionKeys.netAccessKey.resize(4);
    for (int index = 0; index < 4; index++)
    {
        dppConnectionKeys.netAccessKey[index] = 0x2a;
    }
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_CONFIG_RECEIVED_IND()
{
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0x52;
    }
    string password = "w8ECOM8LWba3ln8";
    vector<uint8_t> psk;
    psk.resize(4);
    for (int index = 0; index < 4; index++)
    {
        psk[index] = 0x4c;
    }
    DppAkm securityAkm = DppAkm(1);
    DppConnectionKeys dppConnectionKeys;
    INIT_DPP_CONNECTION_KEYS(dppConnectionKeys);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppSuccessConfigReceivedInd(ssid, password, psk, securityAkm, dppConnectionKeys, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnDppSuccessConfigReceivedIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppSuccessConfigReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(ssid, param.ssid, "ssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(password, param.password, "password (string)"));
    VERIFY(COMPARE_VEC(psk, param.psk, "psk (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&securityAkm, &param.securityAkm, sizeof(DppAkm), "securityAkm (DppAkm)"));
    VERIFY(COMPARE_OBJ(dppConnectionKeys, param.dppConnectionKeys, "dppConnectionKeys (DppConnectionKeys)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_CONFIG_SENT_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppSuccessConfigSentInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseOnDppSuccessConfigSentInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_EAP_FAILURE_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x37;
    }
    int32_t errorCode = 61;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnEapFailureInd(bssid, errorCode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnEapFailureIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnEapFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&errorCode, &param.errorCode, sizeof(int32_t), "errorCode (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_EXT_RADIO_WORK_START_IND()
{
    int32_t id = 13;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnExtRadioWorkStartInd(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseOnExtRadioWorkStartInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_EXT_RADIO_WORK_TIMEOUT_IND()
{
    int32_t id = 26;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnExtRadioWorkTimeoutInd(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseOnExtRadioWorkTimeoutInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_HS20_DEAUTH_IMMINENT_NOTICE_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x9b;
    }
    int32_t reasonCode = 22;
    int32_t reAuthDelayInSec = 46;
    string url = "5wqx0";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnHs20DeauthImminentNoticeInd(bssid, reasonCode, reAuthDelayInSec, url, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnHs20DeauthImminentNoticeIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnHs20DeauthImminentNoticeInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&reasonCode, &param.reasonCode, sizeof(int32_t), "reasonCode (int32_t)"));
    VERIFY(COMPARE_BUF(&reAuthDelayInSec, &param.reAuthDelayInSec, sizeof(int32_t), "reAuthDelayInSec (int32_t)"));
    VERIFY(COMPARE_STR(url, param.url, "url (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_HS20_ICON_QUERY_DONE_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x73;
    }
    string fileName = "EVVnKj7f7EGRrdCy";
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0xeb;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnHs20IconQueryDoneInd(bssid, fileName, data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnHs20IconQueryDoneIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnHs20IconQueryDoneInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(fileName, param.fileName, "fileName (string)"));
    VERIFY(COMPARE_VEC(data, param.data, "data (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_HS20_SUBSCRIPTION_REMEDIATION_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xfa;
    }
    OsuMethod osuMethod = OsuMethod(1);
    string url = "FjdnDVvFhUNH";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnHs20SubscriptionRemediationInd(bssid, osuMethod, url, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnHs20SubscriptionRemediationIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnHs20SubscriptionRemediationInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&osuMethod, &param.osuMethod, sizeof(OsuMethod), "osuMethod (OsuMethod)"));
    VERIFY(COMPARE_STR(url, param.url, "url (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_HS20_TERMS_AND_CONDITIONS_ACCEPTANCE_REQUESTED_NOTIFICATION_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x59;
    }
    string url = "WdCK";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(bssid, url, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnHs20TermsAndConditionsAcceptanceRequestedNotificationIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_STR(url, param.url, "url (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_NETWORK_ADDED_IND()
{
    int32_t id = 36;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnNetworkAddedInd(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseOnNetworkAddedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_NETWORK_NOT_FOUND_IND()
{
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0x5a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnNetworkNotFoundInd(ssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaIfaceParseOnNetworkNotFoundInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(ssid, param, "ssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_NETWORK_REMOVED_IND()
{
    int32_t id = 90;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnNetworkRemovedInd(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseOnNetworkRemovedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_PMK_CACHE_ADDED_IND()
{
    int64_t expirationTimeInSec = 3521331697280749314;
    vector<uint8_t> serializedEntry;
    serializedEntry.resize(4);
    for (int index = 0; index < 4; index++)
    {
        serializedEntry[index] = 0xc9;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnPmkCacheAddedInd(expirationTimeInSec, serializedEntry, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnPmkCacheAddedIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnPmkCacheAddedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&expirationTimeInSec, &param.expirationTimeInSec, sizeof(int64_t), "expirationTimeInSec (int64_t)"));
    VERIFY(COMPARE_VEC(serializedEntry, param.serializedEntry, "serializedEntry (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_STATE_CHANGED_IND()
{
    StaIfaceCallbackState newState = StaIfaceCallbackState(1);
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x6e;
    }
    int32_t id = 5;
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0x60;
    }
    bool filsHlpSent = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnStateChangedInd(newState, bssid, id, ssid, filsHlpSent, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnStateChangedIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnStateChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&newState, &param.newState, sizeof(StaIfaceCallbackState), "newState (StaIfaceCallbackState)"));
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&id, &param.id, sizeof(int32_t), "id (int32_t)"));
    VERIFY(COMPARE_VEC(ssid, param.ssid, "ssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&filsHlpSent, &param.filsHlpSent, sizeof(bool), "filsHlpSent (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_FAIL_IND()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0xda;
    }
    WpsConfigError configError = WpsConfigError(1);
    WpsErrorIndication errorInd = WpsErrorIndication(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnWpsEventFailInd(bssid, configError, errorInd, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnWpsEventFailIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnWpsEventFailInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param.bssid, "bssid (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&configError, &param.configError, sizeof(WpsConfigError), "configError (WpsConfigError)"));
    VERIFY(COMPARE_BUF(&errorInd, &param.errorInd, sizeof(WpsErrorIndication), "errorInd (WpsErrorIndication)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_PBC_OVERLAP_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnWpsEventPbcOverlapInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseOnWpsEventPbcOverlapInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_SUCCESS_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnWpsEventSuccessInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseOnWpsEventSuccessInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_RESET_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnQosPolicyResetInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaIfaceParseOnQosPolicyResetInd(payload.data(), payload.size());
    return true;
}

static void INIT_QOS_POLICY_DATA(QosPolicyData& qosPolicyData)
{
    qosPolicyData.policyId = 0xdb;
    qosPolicyData.requestType = QosPolicyRequestType(1);
    qosPolicyData.dscp = 0xa9;
    INIT_QOS_POLICY_CLASSIFIER_PARAMS(qosPolicyData.classifierParams);
}

static bool T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_REQUEST_IND()
{
    int32_t qosPolicyRequestId = 123;
    vector<QosPolicyData> qosPolicyData;
    qosPolicyData.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_DATA(qosPolicyData[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnQosPolicyRequestInd(qosPolicyRequestId, qosPolicyData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnQosPolicyRequestIndStaIfaceParam param;
    bool msgParseResult = SupplicantStaIfaceParseOnQosPolicyRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&qosPolicyRequestId, &param.qosPolicyRequestId, sizeof(int32_t), "qosPolicyRequestId (int32_t)"));
    VERIFY(COMPARE_VEC(qosPolicyData, param.qosPolicyData, "qosPolicyData (vector<QosPolicyData>)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_MLO_LINKS_INFO_CHANGED_IND()
{
    ISupplicantStaIfaceCallback::MloLinkInfoChangeReason reason = ISupplicantStaIfaceCallback::MloLinkInfoChangeReason(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnMloLinksInfoChangedInd(reason, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ISupplicantStaIfaceCallback::MloLinkInfoChangeReason param;
    bool msgParseResult = SupplicantStaIfaceParseOnMloLinksInfoChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&reason, &param, sizeof(ISupplicantStaIfaceCallback::MloLinkInfoChangeReason), "reason (ISupplicantStaIfaceCallback::MloLinkInfoChangeReason)"));
    return true;
}

static void INIT_DPP_CONFIGURATION_DATA(DppConfigurationData& configData)
{
    configData.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        configData.ssid[index] = 0xd5;
    }
    configData.password = "ZZ";
    configData.psk.resize(4);
    for (int index = 0; index < 4; index++)
    {
        configData.psk[index] = 0xdb;
    }
    configData.securityAkm = DppAkm(1);
    INIT_DPP_CONNECTION_KEYS(configData.dppConnectionKeys);
    configData.connStatusRequested = false;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_CONFIG_RECEIVED_IND()
{
    DppConfigurationData configData;
    INIT_DPP_CONFIGURATION_DATA(configData);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppConfigReceivedInd(configData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DppConfigurationData param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppConfigReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(configData, param, "configData (DppConfigurationData)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_DPP_CONNECTION_STATUS_RESULT_SENT_IND()
{
    DppStatusErrorCode code = DppStatusErrorCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnDppConnectionStatusResultSentInd(code, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DppStatusErrorCode param;
    bool msgParseResult = SupplicantStaIfaceParseOnDppConnectionStatusResultSentInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&code, &param, sizeof(DppStatusErrorCode), "code (DppStatusErrorCode)"));
    return true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_BSS_FREQUENCY_CHANGED_IND()
{
    int32_t frequencyMhz = 84;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnBssFrequencyChangedInd(frequencyMhz, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaIfaceParseOnBssFrequencyChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&frequencyMhz, &param, sizeof(int32_t), "frequencyMhz (int32_t)"));
    return true;
}

static void INIT_SUPPLICANT_STATE_CHANGE_DATA(SupplicantStateChangeData& stateChangeData)
{
    stateChangeData.newState = StaIfaceCallbackState(1);
    stateChangeData.id = 78;
    stateChangeData.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        stateChangeData.ssid[index] = 0xae;
    }
    for (int index = 0; index < 6; index++)
    {
        stateChangeData.bssid[index] = 0x35;
    }
    stateChangeData.keyMgmtMask = KeyMgmtMask(1);
    stateChangeData.frequencyMhz = 117;
    stateChangeData.filsHlpSent = true;
}

static bool T_SUPPLICANT_STA_IFACE_ON_SUPPLICANT_STATE_CHANGED_IND()
{
    SupplicantStateChangeData stateChangeData;
    INIT_SUPPLICANT_STATE_CHANGE_DATA(stateChangeData);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnSupplicantStateChangedInd(stateChangeData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SupplicantStateChangeData param;
    bool msgParseResult = SupplicantStaIfaceParseOnSupplicantStateChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(stateChangeData, param, "stateChangeData (SupplicantStateChangeData)"));
    return true;
}

static void INIT_QOS_POLICY_SCS_RESPONSE_STATUS(QosPolicyScsResponseStatus& qosPolicyScsResponseStatus)
{
    qosPolicyScsResponseStatus.policyId = 0xae;
    qosPolicyScsResponseStatus.qosPolicyScsResponseStatusCode = QosPolicyScsResponseStatusCode(1);
}

static bool T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_RESPONSE_FOR_SCS_IND()
{
    vector<QosPolicyScsResponseStatus> qosPolicyScsResponseStatus;
    qosPolicyScsResponseStatus.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_QOS_POLICY_SCS_RESPONSE_STATUS(qosPolicyScsResponseStatus[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnQosPolicyResponseForScsInd(qosPolicyScsResponseStatus, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<QosPolicyScsResponseStatus> param;
    bool msgParseResult = SupplicantStaIfaceParseOnQosPolicyResponseForScsInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(qosPolicyScsResponseStatus, param, "qosPolicyScsResponseStatus (vector<QosPolicyScsResponseStatus>)"));
    return true;
}

static void INIT_PMK_SA_CACHE_DATA(PmkSaCacheData& pmkSaData)
{
    for (int index = 0; index < 6; index++)
    {
        pmkSaData.bssid[index] = 0xfe;
    }
    pmkSaData.expirationTimeInSec = 7627837889405662363;
    pmkSaData.serializedEntry.resize(4);
    for (int index = 0; index < 4; index++)
    {
        pmkSaData.serializedEntry[index] = 0xd7;
    }
}

static bool T_SUPPLICANT_STA_IFACE_ON_PMK_SA_CACHE_ADDED_IND()
{
    PmkSaCacheData pmkSaData;
    INIT_PMK_SA_CACHE_DATA(pmkSaData);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaIfaceSerializeOnPmkSaCacheAddedInd(pmkSaData, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    PmkSaCacheData param;
    bool msgParseResult = SupplicantStaIfaceParseOnPmkSaCacheAddedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(pmkSaData, param, "pmkSaData (PmkSaCacheData)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_DISABLE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeDisableReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseDisableReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_DISABLE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeDisableCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseDisableCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_REQ()
{
    bool noConnect = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableReq(noConnect, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseEnableReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&noConnect, &param, sizeof(bool), "noConnect (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseEnableCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_SAE_PK_ONLY_MODE_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableSaePkOnlyModeReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseEnableSaePkOnlyModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_SAE_PK_ONLY_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableSaePkOnlyModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseEnableSaePkOnlyModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_SUITE_B_EAP_OPEN_SSL_CIPHERS_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_SUITE_B_EAP_OPEN_SSL_CIPHERS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_TLS_SUITE_B_EAP_PHASE1_PARAM_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ENABLE_TLS_SUITE_B_EAP_PHASE1_PARAM_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_AUTH_ALG_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetAuthAlgReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetAuthAlgReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_AUTH_ALG_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    AuthAlgMask result = AuthAlgMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetAuthAlgCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetAuthAlgCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetAuthAlgCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(AuthAlgMask), "result (AuthAlgMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_BSSID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetBssidReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetBssidReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_BSSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x4b;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetBssidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetBssidCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetBssidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ALT_SUBJECT_MATCH_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapAltSubjectMatchReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapAltSubjectMatchReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ALT_SUBJECT_MATCH_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "7v6Fxk4B";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapAltSubjectMatchCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapAltSubjectMatchCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapAltSubjectMatchCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ANONYMOUS_IDENTITY_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapAnonymousIdentityReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapAnonymousIdentityReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ANONYMOUS_IDENTITY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xc4;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapAnonymousIdentityCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapAnonymousIdentityCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapAnonymousIdentityCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_CERT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapCACertReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapCACertReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_CERT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "Hu";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapCACertCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapCACertCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapCACertCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_PATH_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapCAPathReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapCAPathReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_PATH_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "29rYBIw";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapCAPathCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapCAPathCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapCAPathCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_CLIENT_CERT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapClientCertReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapClientCertReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_CLIENT_CERT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "67iiFHta7";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapClientCertCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapClientCertCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapClientCertCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_DOMAIN_SUFFIX_MATCH_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapDomainSuffixMatchReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapDomainSuffixMatchReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_DOMAIN_SUFFIX_MATCH_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "1W5bVAjIlW7IMyU";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapDomainSuffixMatchCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapDomainSuffixMatchCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapDomainSuffixMatchCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapEngineReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapEngineReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapEngineCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapEngineCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapEngineCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapEngineIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapEngineIdReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "4Kjyk";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapEngineIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapEngineIdCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapEngineIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_IDENTITY_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapIdentityReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapIdentityReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_IDENTITY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xe0;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapIdentityCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapIdentityCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapIdentityCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_METHOD_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapMethodReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapMethodReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_METHOD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    EapMethod result = EapMethod(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapMethodCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapMethodCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapMethodCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(EapMethod), "result (EapMethod)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PASSWORD_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPasswordReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapPasswordReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PASSWORD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x79;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPasswordCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapPasswordCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapPasswordCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PHASE2_METHOD_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPhase2MethodReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapPhase2MethodReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PHASE2_METHOD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    EapPhase2Method result = EapPhase2Method(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPhase2MethodCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapPhase2MethodCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapPhase2MethodCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(EapPhase2Method), "result (EapPhase2Method)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PRIVATE_KEY_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPrivateKeyIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapPrivateKeyIdReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_PRIVATE_KEY_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "txRZLyF9";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapPrivateKeyIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapPrivateKeyIdCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapPrivateKeyIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_SUBJECT_MATCH_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapSubjectMatchReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEapSubjectMatchReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EAP_SUBJECT_MATCH_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "GTSBwxJ";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEapSubjectMatchCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEapSubjectMatchCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEapSubjectMatchCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EDMG_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEdmgReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetEdmgReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_EDMG_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetEdmgCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetEdmgCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetEdmgCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_GROUP_CIPHER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetGroupCipherReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetGroupCipherReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_GROUP_CIPHER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    GroupCipherMask result = GroupCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetGroupCipherCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetGroupCipherCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetGroupCipherCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(GroupCipherMask), "result (GroupCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_GROUP_MGMT_CIPHER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetGroupMgmtCipherReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetGroupMgmtCipherReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_GROUP_MGMT_CIPHER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    GroupMgmtCipherMask result = GroupMgmtCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetGroupMgmtCipherCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetGroupMgmtCipherCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetGroupMgmtCipherCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(GroupMgmtCipherMask), "result (GroupMgmtCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetIdReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 7;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetIdCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_ID_STR_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetIdStrReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetIdStrReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_ID_STR_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "iM9TEuHTqxrXzy5K";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetIdStrCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetIdStrCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetIdStrCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_INTERFACE_NAME_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetInterfaceNameReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetInterfaceNameReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_INTERFACE_NAME_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "KqPe6nK";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetInterfaceNameCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetInterfaceNameCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetInterfaceNameCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_KEY_MGMT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetKeyMgmtReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetKeyMgmtReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_KEY_MGMT_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    KeyMgmtMask result = KeyMgmtMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetKeyMgmtCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetKeyMgmtCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetKeyMgmtCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(KeyMgmtMask), "result (KeyMgmtMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_OCSP_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetOcspReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetOcspReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_OCSP_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    OcspType result = OcspType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetOcspCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetOcspCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetOcspCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(OcspType), "result (OcspType)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PAIRWISE_CIPHER_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPairwiseCipherReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetPairwiseCipherReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PAIRWISE_CIPHER_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    PairwiseCipherMask result = PairwiseCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPairwiseCipherCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetPairwiseCipherCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetPairwiseCipherCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(PairwiseCipherMask), "result (PairwiseCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PROTO_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetProtoReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetProtoReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PROTO_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    ProtoMask result = ProtoMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetProtoCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetProtoCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetProtoCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(ProtoMask), "result (ProtoMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PSK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPskReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetPskReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PSK_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x29;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPskCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetPskCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetPskCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PSK_PASSPHRASE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPskPassphraseReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetPskPassphraseReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_PSK_PASSPHRASE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "FiYOqYRDY";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetPskPassphraseCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetPskPassphraseCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetPskPassphraseCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_REQUIRE_PMF_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetRequirePmfReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetRequirePmfReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_REQUIRE_PMF_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetRequirePmfCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetRequirePmfCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetRequirePmfCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSaePasswordReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetSaePasswordReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "1zE";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSaePasswordCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSaePasswordCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetSaePasswordCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_ID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSaePasswordIdReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetSaePasswordIdReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_ID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "upBuc4gxuy";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSaePasswordIdCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSaePasswordIdCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetSaePasswordIdCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SCAN_SSID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetScanSsidReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetScanSsidReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SCAN_SSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    bool result = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetScanSsidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetScanSsidCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetScanSsidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(bool), "result (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SSID_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSsidReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetSsidReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_SSID_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xed;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetSsidCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetSsidCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetSsidCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_TYPE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetTypeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetTypeReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_TYPE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    IfaceType result = IfaceType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetTypeCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetTypeCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetTypeCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(IfaceType), "result (IfaceType)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WAPI_CERT_SUITE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWapiCertSuiteReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetWapiCertSuiteReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WAPI_CERT_SUITE_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    string result = "ma1fc7SsC0";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWapiCertSuiteCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWapiCertSuiteCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetWapiCertSuiteCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_STR(result, param.result, "result (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WEP_KEY_REQ()
{
    int32_t keyIdx = 79;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWepKeyReq(keyIdx, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaNetworkParseGetWepKeyReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&keyIdx, &param, sizeof(int32_t), "keyIdx (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WEP_KEY_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0xcf;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWepKeyCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWepKeyCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetWepKeyCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WEP_TX_KEY_IDX_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWepTxKeyIdxReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetWepTxKeyIdxReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WEP_TX_KEY_IDX_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    int32_t result = 43;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWepTxKeyIdxCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWepTxKeyIdxCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetWepTxKeyIdxCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_BUF(&result, &param.result, sizeof(int32_t), "result (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WPS_NFC_CONFIGURATION_TOKEN_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseGetWpsNfcConfigurationTokenReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_GET_WPS_NFC_CONFIGURATION_TOKEN_CFM()
{
    HalStatusParam status;
    INIT_HAL_STATUS(status);
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x8c;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenCfm(status, result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GetWpsNfcConfigurationTokenCfmStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseGetWpsNfcConfigurationTokenCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(HalStatusParam), "status (HalStatusParam)"));
    VERIFY(COMPARE_VEC(result, param.result, "result (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_REGISTER_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeRegisterCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseRegisterCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_REGISTER_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeRegisterCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseRegisterCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SELECT_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSelectReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSelectReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SELECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSelectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSelectCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_IDENTITY_RESPONSE_REQ()
{
    vector<uint8_t> identity;
    identity.resize(4);
    for (int index = 0; index < 4; index++)
    {
        identity[index] = 0x0e;
    }
    vector<uint8_t> encryptedIdentity;
    encryptedIdentity.resize(4);
    for (int index = 0; index < 4; index++)
    {
        encryptedIdentity[index] = 0xc7;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseReq(identity, encryptedIdentity, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SendNetworkEapIdentityResponseReqStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapIdentityResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(identity, param.identity, "identity (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(encryptedIdentity, param.encryptedIdentity, "encryptedIdentity (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_IDENTITY_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapIdentityResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_FAILURE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_FAILURE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NETWORK_RESPONSE_EAP_SIM_GSM_AUTH_PARAMS(NetworkResponseEapSimGsmAuthParams& params)
{
    params.kc.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.kc[index] = 0xc7;
    }
    params.sres.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.sres[index] = 0x39;
    }
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_RESPONSE_REQ()
{
    vector<NetworkResponseEapSimGsmAuthParams> params;
    params.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_NETWORK_RESPONSE_EAP_SIM_GSM_AUTH_PARAMS(params[index]);
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseReq(params, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<NetworkResponseEapSimGsmAuthParams> param;
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(params, param, "params (vector<NetworkResponseEapSimGsmAuthParams>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_FAILURE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureReq(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_FAILURE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureCfm(payload.data(), payload.size());
    return true;
}

static void INIT_NETWORK_RESPONSE_EAP_SIM_UMTS_AUTH_PARAMS(NetworkResponseEapSimUmtsAuthParams& params)
{
    params.res.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.res[index] = 0x17;
    }
    params.ik.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.ik[index] = 0x02;
    }
    params.ck.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.ck[index] = 0xf5;
    }
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_RESPONSE_REQ()
{
    NetworkResponseEapSimUmtsAuthParams params;
    INIT_NETWORK_RESPONSE_EAP_SIM_UMTS_AUTH_PARAMS(params);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseReq(params, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NetworkResponseEapSimUmtsAuthParams param;
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(params, param, "params (NetworkResponseEapSimUmtsAuthParams)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTS_RESPONSE_REQ()
{
    vector<uint8_t> auts;
    auts.resize(4);
    for (int index = 0; index < 4; index++)
    {
        auts[index] = 0xa9;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseReq(auts, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(auts, param, "auts (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTS_RESPONSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_AUTH_ALG_REQ()
{
    AuthAlgMask authAlgMask = AuthAlgMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetAuthAlgReq(authAlgMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AuthAlgMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetAuthAlgReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&authAlgMask, &param, sizeof(AuthAlgMask), "authAlgMask (AuthAlgMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_AUTH_ALG_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetAuthAlgCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetAuthAlgCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_BSSID_REQ()
{
    vector<uint8_t> bssid;
    bssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        bssid[index] = 0x3e;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetBssidReq(bssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetBssidReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(bssid, param, "bssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_BSSID_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetBssidCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetBssidCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_DPP_KEYS_REQ()
{
    DppConnectionKeys keys;
    INIT_DPP_CONNECTION_KEYS(keys);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetDppKeysReq(keys, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DppConnectionKeys param;
    bool msgParseResult = SupplicantStaNetworkParseSetDppKeysReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(keys, param, "keys (DppConnectionKeys)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_DPP_KEYS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetDppKeysCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetDppKeysCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ALT_SUBJECT_MATCH_REQ()
{
    string match = "j537MGT";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapAltSubjectMatchReq(match, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapAltSubjectMatchReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(match, param, "match (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ALT_SUBJECT_MATCH_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapAltSubjectMatchCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapAltSubjectMatchCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ANONYMOUS_IDENTITY_REQ()
{
    vector<uint8_t> identity;
    identity.resize(4);
    for (int index = 0; index < 4; index++)
    {
        identity[index] = 0x57;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapAnonymousIdentityReq(identity, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapAnonymousIdentityReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(identity, param, "identity (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ANONYMOUS_IDENTITY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapAnonymousIdentityCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapAnonymousIdentityCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_CERT_REQ()
{
    string path = "mj3tvlK";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapCACertReq(path, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapCACertReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(path, param, "path (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_CERT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapCACertCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapCACertCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_PATH_REQ()
{
    string path = "9jVVnXf";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapCAPathReq(path, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapCAPathReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(path, param, "path (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_PATH_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapCAPathCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapCAPathCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_CLIENT_CERT_REQ()
{
    string path = "K7qzhy";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapClientCertReq(path, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapClientCertReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(path, param, "path (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_CLIENT_CERT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapClientCertCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapClientCertCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_DOMAIN_SUFFIX_MATCH_REQ()
{
    string match = "gUJR5T0g5ltLztBB";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapDomainSuffixMatchReq(match, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapDomainSuffixMatchReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(match, param, "match (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_DOMAIN_SUFFIX_MATCH_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapDomainSuffixMatchCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapDomainSuffixMatchCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENCRYPTED_IMSI_IDENTITY_REQ()
{
    vector<uint8_t> identity;
    identity.resize(4);
    for (int index = 0; index < 4; index++)
    {
        identity[index] = 0x96;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityReq(identity, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapEncryptedImsiIdentityReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(identity, param, "identity (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENCRYPTED_IMSI_IDENTITY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapEncryptedImsiIdentityCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEngineReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapEngineReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEngineCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapEngineCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_I_D_REQ()
{
    string id = "hPbfWnzZUuG1IojU";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEngineIDReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapEngineIDReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(id, param, "id (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_I_D_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapEngineIDCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapEngineIDCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ERP_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapErpReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapErpReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_ERP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapErpCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapErpCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_IDENTITY_REQ()
{
    vector<uint8_t> identity;
    identity.resize(4);
    for (int index = 0; index < 4; index++)
    {
        identity[index] = 0xd2;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapIdentityReq(identity, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapIdentityReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(identity, param, "identity (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_IDENTITY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapIdentityCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapIdentityCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_METHOD_REQ()
{
    EapMethod method = EapMethod(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapMethodReq(method, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EapMethod param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapMethodReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&method, &param, sizeof(EapMethod), "method (EapMethod)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_METHOD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapMethodCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapMethodCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PASSWORD_REQ()
{
    vector<uint8_t> password;
    password.resize(4);
    for (int index = 0; index < 4; index++)
    {
        password[index] = 0x38;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPasswordReq(password, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapPasswordReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(password, param, "password (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PASSWORD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPasswordCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapPasswordCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PHASE2_METHOD_REQ()
{
    EapPhase2Method method = EapPhase2Method(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPhase2MethodReq(method, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    EapPhase2Method param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapPhase2MethodReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&method, &param, sizeof(EapPhase2Method), "method (EapPhase2Method)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PHASE2_METHOD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPhase2MethodCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapPhase2MethodCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PRIVATE_KEY_ID_REQ()
{
    string id = "IPnHqUs4";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPrivateKeyIdReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapPrivateKeyIdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(id, param, "id (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_PRIVATE_KEY_ID_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapPrivateKeyIdCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapPrivateKeyIdCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_SUBJECT_MATCH_REQ()
{
    string match = "Z4O";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapSubjectMatchReq(match, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetEapSubjectMatchReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(match, param, "match (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EAP_SUBJECT_MATCH_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEapSubjectMatchCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEapSubjectMatchCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EDMG_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEdmgReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetEdmgReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_EDMG_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetEdmgCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetEdmgCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_GROUP_CIPHER_REQ()
{
    GroupCipherMask groupCipherMask = GroupCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetGroupCipherReq(groupCipherMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GroupCipherMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetGroupCipherReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&groupCipherMask, &param, sizeof(GroupCipherMask), "groupCipherMask (GroupCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_GROUP_CIPHER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetGroupCipherCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetGroupCipherCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_GROUP_MGMT_CIPHER_REQ()
{
    GroupMgmtCipherMask groupMgmtCipherMask = GroupMgmtCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetGroupMgmtCipherReq(groupMgmtCipherMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    GroupMgmtCipherMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetGroupMgmtCipherReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&groupMgmtCipherMask, &param, sizeof(GroupMgmtCipherMask), "groupMgmtCipherMask (GroupMgmtCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_GROUP_MGMT_CIPHER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetGroupMgmtCipherCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetGroupMgmtCipherCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_ID_STR_REQ()
{
    string idStr = "5";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetIdStrReq(idStr, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetIdStrReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(idStr, param, "idStr (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_ID_STR_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetIdStrCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetIdStrCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_KEY_MGMT_REQ()
{
    KeyMgmtMask keyMgmtMask = KeyMgmtMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetKeyMgmtReq(keyMgmtMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    KeyMgmtMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetKeyMgmtReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&keyMgmtMask, &param, sizeof(KeyMgmtMask), "keyMgmtMask (KeyMgmtMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_KEY_MGMT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetKeyMgmtCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetKeyMgmtCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_OCSP_REQ()
{
    OcspType ocspType = OcspType(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetOcspReq(ocspType, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OcspType param;
    bool msgParseResult = SupplicantStaNetworkParseSetOcspReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&ocspType, &param, sizeof(OcspType), "ocspType (OcspType)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_OCSP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetOcspCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetOcspCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PAIRWISE_CIPHER_REQ()
{
    PairwiseCipherMask pairwiseCipherMask = PairwiseCipherMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPairwiseCipherReq(pairwiseCipherMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    PairwiseCipherMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetPairwiseCipherReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&pairwiseCipherMask, &param, sizeof(PairwiseCipherMask), "pairwiseCipherMask (PairwiseCipherMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PAIRWISE_CIPHER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPairwiseCipherCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetPairwiseCipherCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PMK_CACHE_REQ()
{
    vector<uint8_t> serializedEntry;
    serializedEntry.resize(4);
    for (int index = 0; index < 4; index++)
    {
        serializedEntry[index] = 0xdb;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPmkCacheReq(serializedEntry, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetPmkCacheReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(serializedEntry, param, "serializedEntry (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PMK_CACHE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPmkCacheCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetPmkCacheCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PROACTIVE_KEY_CACHING_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetProactiveKeyCachingReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetProactiveKeyCachingReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PROACTIVE_KEY_CACHING_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetProactiveKeyCachingCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetProactiveKeyCachingCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PROTO_REQ()
{
    ProtoMask protoMask = ProtoMask(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetProtoReq(protoMask, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ProtoMask param;
    bool msgParseResult = SupplicantStaNetworkParseSetProtoReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&protoMask, &param, sizeof(ProtoMask), "protoMask (ProtoMask)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PROTO_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetProtoCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetProtoCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PSK_REQ()
{
    vector<uint8_t> psk;
    psk.resize(4);
    for (int index = 0; index < 4; index++)
    {
        psk[index] = 0x3c;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPskReq(psk, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetPskReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(psk, param, "psk (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PSK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPskCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetPskCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PSK_PASSPHRASE_REQ()
{
    string psk = "gEe4JPhkE";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPskPassphraseReq(psk, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetPskPassphraseReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(psk, param, "psk (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_PSK_PASSPHRASE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetPskPassphraseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetPskPassphraseCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_REQUIRE_PMF_REQ()
{
    bool enable = true;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetRequirePmfReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetRequirePmfReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_REQUIRE_PMF_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetRequirePmfCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetRequirePmfCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_H2E_MODE_REQ()
{
    SaeH2eMode mode = SaeH2eMode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaeH2eModeReq(mode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SaeH2eMode param;
    bool msgParseResult = SupplicantStaNetworkParseSetSaeH2eModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&mode, &param, sizeof(SaeH2eMode), "mode (SaeH2eMode)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_H2E_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaeH2eModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetSaeH2eModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_REQ()
{
    string saePassword = "NIfGq2P4Q";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaePasswordReq(saePassword, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetSaePasswordReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(saePassword, param, "saePassword (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaePasswordCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetSaePasswordCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_ID_REQ()
{
    string saePasswordId = "G";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaePasswordIdReq(saePasswordId, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetSaePasswordIdReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(saePasswordId, param, "saePasswordId (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_ID_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSaePasswordIdCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetSaePasswordIdCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SCAN_SSID_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetScanSsidReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetScanSsidReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SCAN_SSID_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetScanSsidCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetScanSsidCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SSID_REQ()
{
    vector<uint8_t> ssid;
    ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        ssid[index] = 0xe4;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSsidReq(ssid, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetSsidReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(ssid, param, "ssid (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_SSID_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetSsidCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetSsidCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_UPDATE_IDENTIFIER_REQ()
{
    int32_t id = 14;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetUpdateIdentifierReq(id, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaNetworkParseSetUpdateIdentifierReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&id, &param, sizeof(int32_t), "id (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_UPDATE_IDENTIFIER_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetUpdateIdentifierCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetUpdateIdentifierCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WAPI_CERT_SUITE_REQ()
{
    string suite = "trqGXHHe3MlaI";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWapiCertSuiteReq(suite, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = SupplicantStaNetworkParseSetWapiCertSuiteReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(suite, param, "suite (string)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WAPI_CERT_SUITE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWapiCertSuiteCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetWapiCertSuiteCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WEP_KEY_REQ()
{
    int32_t keyIdx = 19;
    vector<uint8_t> wepKey;
    wepKey.resize(4);
    for (int index = 0; index < 4; index++)
    {
        wepKey[index] = 0x43;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWepKeyReq(keyIdx, wepKey, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    SetWepKeyReqStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseSetWepKeyReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&keyIdx, &param.keyIdx, sizeof(int32_t), "keyIdx (int32_t)"));
    VERIFY(COMPARE_VEC(wepKey, param.wepKey, "wepKey (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WEP_KEY_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWepKeyCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetWepKeyCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WEP_TX_KEY_IDX_REQ()
{
    int32_t keyIdx = 66;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWepTxKeyIdxReq(keyIdx, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    int32_t param;
    bool msgParseResult = SupplicantStaNetworkParseSetWepTxKeyIdxReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&keyIdx, &param, sizeof(int32_t), "keyIdx (int32_t)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_WEP_TX_KEY_IDX_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetWepTxKeyIdxCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetWepTxKeyIdxCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_ROAMING_CONSORTIUM_SELECTION_REQ()
{
    vector<uint8_t> selectedRcoi;
    selectedRcoi.resize(4);
    for (int index = 0; index < 4; index++)
    {
        selectedRcoi[index] = 0x72;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionReq(selectedRcoi, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = SupplicantStaNetworkParseSetRoamingConsortiumSelectionReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(selectedRcoi, param, "selectedRcoi (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_ROAMING_CONSORTIUM_SELECTION_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetRoamingConsortiumSelectionCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_MINIMUM_TLS_VERSION_EAP_PHASE1_PARAM_REQ()
{
    TlsVersion tlsVersion = TlsVersion(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamReq(tlsVersion, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    TlsVersion param;
    bool msgParseResult = SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&tlsVersion, &param, sizeof(TlsVersion), "tlsVersion (TlsVersion)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_MINIMUM_TLS_VERSION_EAP_PHASE1_PARAM_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_STRICT_CONSERVATIVE_PEER_MODE_REQ()
{
    bool enable = false;
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetStrictConservativePeerModeReq(enable, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    bool param;
    bool msgParseResult = SupplicantStaNetworkParseSetStrictConservativePeerModeReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&enable, &param, sizeof(bool), "enable (bool)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_SET_STRICT_CONSERVATIVE_PEER_MODE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeSetStrictConservativePeerModeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseSetStrictConservativePeerModeCfm(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_IDENTITY_REQUEST_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnNetworkEapIdentityRequestInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseOnNetworkEapIdentityRequestInd(payload.data(), payload.size());
    return true;
}

static void INIT_GSM_RAND(GsmRand& rands)
{
    rands.data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        rands.data[index] = 0xe8;
    }
}

static void INIT_NETWORK_REQUEST_EAP_SIM_GSM_AUTH_PARAMS(NetworkRequestEapSimGsmAuthParams& params)
{
    params.rands.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_GSM_RAND(params.rands[index]);
    }
}

static bool T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_SIM_GSM_AUTH_REQUEST_IND()
{
    NetworkRequestEapSimGsmAuthParams params;
    INIT_NETWORK_REQUEST_EAP_SIM_GSM_AUTH_PARAMS(params);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnNetworkEapSimGsmAuthRequestInd(params, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NetworkRequestEapSimGsmAuthParams param;
    bool msgParseResult = SupplicantStaNetworkParseOnNetworkEapSimGsmAuthRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(params, param, "params (NetworkRequestEapSimGsmAuthParams)"));
    return true;
}

static void INIT_NETWORK_REQUEST_EAP_SIM_UMTS_AUTH_PARAMS(NetworkRequestEapSimUmtsAuthParams& params)
{
    params.rand.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.rand[index] = 0x86;
    }
    params.autn.resize(4);
    for (int index = 0; index < 4; index++)
    {
        params.autn[index] = 0xcb;
    }
}

static bool T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_SIM_UMTS_AUTH_REQUEST_IND()
{
    NetworkRequestEapSimUmtsAuthParams params;
    INIT_NETWORK_REQUEST_EAP_SIM_UMTS_AUTH_PARAMS(params);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnNetworkEapSimUmtsAuthRequestInd(params, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    NetworkRequestEapSimUmtsAuthParams param;
    bool msgParseResult = SupplicantStaNetworkParseOnNetworkEapSimUmtsAuthRequestInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(params, param, "params (NetworkRequestEapSimUmtsAuthParams)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ON_TRANSITION_DISABLE_IND()
{
    TransitionDisableIndication ind = TransitionDisableIndication(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnTransitionDisableInd(ind, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    TransitionDisableIndication param;
    bool msgParseResult = SupplicantStaNetworkParseOnTransitionDisableInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&ind, &param, sizeof(TransitionDisableIndication), "ind (TransitionDisableIndication)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ON_SERVER_CERTIFICATE_AVAILABLE_IND()
{
    int32_t depth = 12;
    vector<uint8_t> subject;
    subject.resize(4);
    for (int index = 0; index < 4; index++)
    {
        subject[index] = 0xf7;
    }
    vector<uint8_t> certHash;
    certHash.resize(4);
    for (int index = 0; index < 4; index++)
    {
        certHash[index] = 0x39;
    }
    vector<uint8_t> certBlob;
    certBlob.resize(4);
    for (int index = 0; index < 4; index++)
    {
        certBlob[index] = 0xb8;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnServerCertificateAvailableInd(depth, subject, certHash, certBlob, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnServerCertificateAvailableIndStaNetworkParam param;
    bool msgParseResult = SupplicantStaNetworkParseOnServerCertificateAvailableInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&depth, &param.depth, sizeof(int32_t), "depth (int32_t)"));
    VERIFY(COMPARE_VEC(subject, param.subject, "subject (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(certHash, param.certHash, "certHash (vector<uint8_t>)"));
    VERIFY(COMPARE_VEC(certBlob, param.certBlob, "certBlob (vector<uint8_t>)"));
    return true;
}

static bool T_SUPPLICANT_STA_NETWORK_ON_PERMANENT_ID_REQ_DENIED_IND()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantStaNetworkSerializeOnPermanentIdReqDeniedInd(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = SupplicantStaNetworkParseOnPermanentIdReqDeniedInd(payload.data(), payload.size());
    return true;
}

static bool T_SUPPLICANT_HAL_STATUS()
{
    int32_t status = 69;
    string info = "SN9F6G1q";
    vector<uint8_t> payload;
    bool msgSerializeResult = SupplicantSerializeHalStatus(status, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HalStatusParam param;
    bool msgParseResult = SupplicantParseHalStatus(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(int32_t), "status (int32_t)"));
    VERIFY(COMPARE_STR(info, param.info, "info (string)"));
    return true;
}

static bool T_NON_STANDARD_CERT_GET_BLOB_REQ()
{
    string alias = "p6sIW";
    vector<uint8_t> payload;
    bool msgSerializeResult = NonStandardCertSerializeGetBlobReq(alias, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = NonStandardCertParseGetBlobReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(alias, param, "alias (string)"));
    return true;
}

static bool T_NON_STANDARD_CERT_GET_BLOB_CFM()
{
    vector<uint8_t> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = 0x44;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = NonStandardCertSerializeGetBlobCfm(result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = NonStandardCertParseGetBlobCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(result, param, "result (vector<uint8_t>)"));
    return true;
}

static bool T_NON_STANDARD_CERT_LIST_ALIASES_REQ()
{
    string prefix = "p1W3x";
    vector<uint8_t> payload;
    bool msgSerializeResult = NonStandardCertSerializeListAliasesReq(prefix, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = NonStandardCertParseListAliasesReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(prefix, param, "prefix (string)"));
    return true;
}

static bool T_NON_STANDARD_CERT_LIST_ALIASES_CFM()
{
    vector<string> result;
    result.resize(4);
    for (int index = 0; index < 4; index++)
    {
        result[index] = "Rt";
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = NonStandardCertSerializeListAliasesCfm(result, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<string> param;
    bool msgParseResult = NonStandardCertParseListAliasesCfm(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(result, param, "result (vector<string>)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_SUPPLICANT_ADD_P2P_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_ADD_P2P_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_ADD_STA_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_ADD_STA_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_GET_DEBUG_LEVEL_REQ());
    VERIFY(T_SUPPLICANT_GET_DEBUG_LEVEL_CFM());
    VERIFY(T_SUPPLICANT_GET_P2P_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_GET_P2P_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_GET_STA_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_GET_STA_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_IS_DEBUG_SHOW_KEYS_ENABLED_REQ());
    VERIFY(T_SUPPLICANT_IS_DEBUG_SHOW_KEYS_ENABLED_CFM());
    VERIFY(T_SUPPLICANT_IS_DEBUG_SHOW_TIMESTAMP_ENABLED_REQ());
    VERIFY(T_SUPPLICANT_IS_DEBUG_SHOW_TIMESTAMP_ENABLED_CFM());
    VERIFY(T_SUPPLICANT_LIST_INTERFACES_REQ());
    VERIFY(T_SUPPLICANT_LIST_INTERFACES_CFM());
    VERIFY(T_SUPPLICANT_REGISTER_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_REGISTER_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_REMOVE_INTERFACE_REQ());
    VERIFY(T_SUPPLICANT_REMOVE_INTERFACE_CFM());
    VERIFY(T_SUPPLICANT_SET_CONCURRENCY_PRIORITY_REQ());
    VERIFY(T_SUPPLICANT_SET_CONCURRENCY_PRIORITY_CFM());
    VERIFY(T_SUPPLICANT_SET_DEBUG_PARAMS_REQ());
    VERIFY(T_SUPPLICANT_SET_DEBUG_PARAMS_CFM());
    VERIFY(T_SUPPLICANT_TERMINATE_REQ());
    VERIFY(T_SUPPLICANT_TERMINATE_CFM());
    VERIFY(T_SUPPLICANT_REGISTER_NON_STANDARD_CERT_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_REGISTER_NON_STANDARD_CERT_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_ON_INTERFACE_CREATED_IND());
    VERIFY(T_SUPPLICANT_ON_INTERFACE_REMOVED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_BONJOUR_SERVICE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_BONJOUR_SERVICE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_GROUP_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_GROUP_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_GROUP_WITH_CONFIG_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_GROUP_WITH_CONFIG_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_UPNP_SERVICE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ADD_UPNP_SERVICE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_CONNECT_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_CONNECT_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_SERVICE_DISCOVERY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_SERVICE_DISCOVERY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_WPS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CANCEL_WPS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONFIGURE_EXT_LISTEN_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONFIGURE_EXT_LISTEN_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONNECT_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONNECT_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_REQUEST_MESSAGE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_REQUEST_MESSAGE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_SELECT_MESSAGE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CREATE_NFC_HANDOVER_SELECT_MESSAGE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ENABLE_WFD_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ENABLE_WFD_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FLUSH_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FLUSH_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FLUSH_SERVICES_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FLUSH_SERVICES_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_DEVICE_ADDRESS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_DEVICE_ADDRESS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_EDMG_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_EDMG_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_GROUP_CAPABILITY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_GROUP_CAPABILITY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_NAME_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_NAME_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_SSID_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_SSID_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_TYPE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_GET_TYPE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_INVITE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_INVITE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_LIST_NETWORKS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_LIST_NETWORKS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_PROVISION_DISCOVERY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_PROVISION_DISCOVERY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REGISTER_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REGISTER_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REINVOKE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REINVOKE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REJECT_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REJECT_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_BONJOUR_SERVICE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_BONJOUR_SERVICE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_GROUP_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_GROUP_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_UPNP_SERVICE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_UPNP_SERVICE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_INITIATION_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_INITIATION_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REPORT_NFC_HANDOVER_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REQUEST_SERVICE_DISCOVERY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REQUEST_SERVICE_DISCOVERY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SAVE_CONFIG_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SAVE_CONFIG_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_DISALLOWED_FREQUENCIES_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_DISALLOWED_FREQUENCIES_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_EDMG_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_EDMG_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_GROUP_IDLE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_GROUP_IDLE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_LISTEN_CHANNEL_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_LISTEN_CHANNEL_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_MAC_RANDOMIZATION_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_MAC_RANDOMIZATION_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_MIRACAST_MODE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_MIRACAST_MODE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_POWER_SAVE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_POWER_SAVE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_SSID_POSTFIX_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_SSID_POSTFIX_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WFD_DEVICE_INFO_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WFD_DEVICE_INFO_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WFD_R2_DEVICE_INFO_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WFD_R2_DEVICE_INFO_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_CLIENT_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_REMOVE_CLIENT_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_CONFIG_METHODS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_CONFIG_METHODS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_NAME_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_NAME_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_TYPE_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_DEVICE_TYPE_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MANUFACTURER_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MANUFACTURER_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NAME_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NAME_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NUMBER_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_MODEL_NUMBER_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_SERIAL_NUMBER_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_WPS_SERIAL_NUMBER_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PBC_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PBC_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_DISPLAY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_DISPLAY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_KEYPAD_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_START_WPS_PIN_KEYPAD_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_STOP_FIND_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_STOP_FIND_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_ON_SOCIAL_CHANNELS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_ON_SOCIAL_CHANNELS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_ON_SPECIFIC_FREQUENCY_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_FIND_ON_SPECIFIC_FREQUENCY_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_VENDOR_ELEMENTS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_SET_VENDOR_ELEMENTS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONFIGURE_EAPOL_IP_ADDRESS_ALLOCATION_PARAMS_REQ());
    VERIFY(T_SUPPLICANT_P2P_IFACE_CONFIGURE_EAPOL_IP_ADDRESS_ALLOCATION_PARAMS_CFM());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_DEVICE_FOUND_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_DEVICE_LOST_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_FIND_STOPPED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GO_NEGOTIATION_COMPLETED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GO_NEGOTIATION_REQUEST_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_FORMATION_FAILURE_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_FORMATION_SUCCESS_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_REMOVED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_STARTED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_INVITATION_RECEIVED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_INVITATION_RESULT_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_PROVISION_DISCOVERY_COMPLETED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_R2_DEVICE_FOUND_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_SERVICE_DISCOVERY_RESPONSE_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_STA_AUTHORIZED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_STA_DEAUTHORIZED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_FREQUENCY_CHANGED_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_DEVICE_FOUND_WITH_VENDOR_ELEMENTS_IND());
    VERIFY(T_SUPPLICANT_P2P_IFACE_ON_GROUP_STARTED_WITH_PARAMS_IND());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_BSSID_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_BSSID_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_CLIENT_LIST_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_CLIENT_LIST_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_ID_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_ID_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_INTERFACE_NAME_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_INTERFACE_NAME_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_SSID_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_SSID_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_TYPE_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_GET_TYPE_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_CURRENT_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_CURRENT_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_GROUP_OWNER_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_GROUP_OWNER_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_PERSISTENT_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_IS_PERSISTENT_CFM());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_SET_CLIENT_LIST_REQ());
    VERIFY(T_SUPPLICANT_P2P_NETWORK_SET_CLIENT_LIST_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_DPP_PEER_URI_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_DPP_PEER_URI_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_EXT_RADIO_WORK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_EXT_RADIO_WORK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_RX_FILTER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_RX_FILTER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_CANCEL_WPS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_CANCEL_WPS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_DISCONNECT_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_DISCONNECT_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ENABLE_AUTO_RECONNECT_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ENABLE_AUTO_RECONNECT_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_FILS_HLP_ADD_REQUEST_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_FILS_HLP_ADD_REQUEST_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_FILS_HLP_FLUSH_REQUEST_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_FILS_HLP_FLUSH_REQUEST_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GENERATE_DPP_BOOTSTRAP_INFO_FOR_RESPONDER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GENERATE_DPP_BOOTSTRAP_INFO_FOR_RESPONDER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GENERATE_SELF_DPP_CONFIGURATION_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GENERATE_SELF_DPP_CONFIGURATION_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_CONNECTION_CAPABILITIES_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_CONNECTION_CAPABILITIES_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_CONNECTION_MLO_LINKS_INFO_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_CONNECTION_MLO_LINKS_INFO_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_KEY_MGMT_CAPABILITIES_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_KEY_MGMT_CAPABILITIES_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_MAC_ADDRESS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_MAC_ADDRESS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_NAME_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_NAME_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_TYPE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_TYPE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_WPA_DRIVER_CAPABILITIES_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_WPA_DRIVER_CAPABILITIES_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_ANQP_QUERY_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_ANQP_QUERY_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_HS20_ICON_QUERY_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_HS20_ICON_QUERY_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_DISCOVER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_DISCOVER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_SETUP_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_SETUP_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_TEARDOWN_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_TDLS_TEARDOWN_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_VENUE_URL_ANQP_QUERY_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_INITIATE_VENUE_URL_ANQP_QUERY_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_LIST_NETWORKS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_LIST_NETWORKS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REASSOCIATE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REASSOCIATE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_RECONNECT_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_RECONNECT_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REGISTER_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REGISTER_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_QOS_POLICY_FEATURE_ENABLED_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_QOS_POLICY_FEATURE_ENABLED_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SEND_QOS_POLICY_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SEND_QOS_POLICY_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_ALL_QOS_POLICIES_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_ALL_QOS_POLICIES_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_DPP_URI_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_DPP_URI_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_EXT_RADIO_WORK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_EXT_RADIO_WORK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_NETWORK_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_NETWORK_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_RX_FILTER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_RX_FILTER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_MODE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_MODE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_SCAN_MODE_ENABLED_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_BT_COEXISTENCE_SCAN_MODE_ENABLED_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_COUNTRY_CODE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_COUNTRY_CODE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_EXTERNAL_SIM_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_EXTERNAL_SIM_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_MBO_CELLULAR_DATA_STATUS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_MBO_CELLULAR_DATA_STATUS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_POWER_SAVE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_POWER_SAVE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_SUSPEND_MODE_ENABLED_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_SUSPEND_MODE_ENABLED_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_CONFIG_METHODS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_CONFIG_METHODS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_NAME_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_NAME_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_TYPE_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_DEVICE_TYPE_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MANUFACTURER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MANUFACTURER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NAME_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NAME_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NUMBER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_MODEL_NUMBER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_SERIAL_NUMBER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_SET_WPS_SERIAL_NUMBER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_CONFIGURATOR_INITIATOR_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_CONFIGURATOR_INITIATOR_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_INITIATOR_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_INITIATOR_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_RESPONDER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_DPP_ENROLLEE_RESPONDER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_RX_FILTER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_RX_FILTER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PBC_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PBC_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PIN_DISPLAY_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PIN_DISPLAY_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PIN_KEYPAD_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_PIN_KEYPAD_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_REGISTRAR_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_START_WPS_REGISTRAR_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_DPP_INITIATOR_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_DPP_INITIATOR_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_DPP_RESPONDER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_DPP_RESPONDER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_RX_FILTER_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_STOP_RX_FILTER_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_SIGNAL_POLL_RESULTS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_GET_SIGNAL_POLL_RESULTS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_QOS_POLICY_REQUEST_FOR_SCS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_ADD_QOS_POLICY_REQUEST_FOR_SCS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_QOS_POLICY_FOR_SCS_REQ());
    VERIFY(T_SUPPLICANT_STA_IFACE_REMOVE_QOS_POLICY_FOR_SCS_CFM());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_ANQP_QUERY_DONE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_ASSOCIATION_REJECTED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_AUTHENTICATION_TIMEOUT_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_AUXILIARY_SUPPLICANT_EVENT_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_BSS_TM_HANDLING_DONE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_BSSID_CHANGED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DISCONNECTED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_FAILURE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_PROGRESS_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_CONFIG_RECEIVED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_SUCCESS_CONFIG_SENT_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_EAP_FAILURE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_EXT_RADIO_WORK_START_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_EXT_RADIO_WORK_TIMEOUT_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_HS20_DEAUTH_IMMINENT_NOTICE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_HS20_ICON_QUERY_DONE_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_HS20_SUBSCRIPTION_REMEDIATION_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_HS20_TERMS_AND_CONDITIONS_ACCEPTANCE_REQUESTED_NOTIFICATION_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_NETWORK_ADDED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_NETWORK_NOT_FOUND_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_NETWORK_REMOVED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_PMK_CACHE_ADDED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_STATE_CHANGED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_FAIL_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_PBC_OVERLAP_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_WPS_EVENT_SUCCESS_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_RESET_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_REQUEST_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_MLO_LINKS_INFO_CHANGED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_CONFIG_RECEIVED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_DPP_CONNECTION_STATUS_RESULT_SENT_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_BSS_FREQUENCY_CHANGED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_SUPPLICANT_STATE_CHANGED_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_QOS_POLICY_RESPONSE_FOR_SCS_IND());
    VERIFY(T_SUPPLICANT_STA_IFACE_ON_PMK_SA_CACHE_ADDED_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_DISABLE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_DISABLE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_SAE_PK_ONLY_MODE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_SAE_PK_ONLY_MODE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_SUITE_B_EAP_OPEN_SSL_CIPHERS_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_SUITE_B_EAP_OPEN_SSL_CIPHERS_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_TLS_SUITE_B_EAP_PHASE1_PARAM_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ENABLE_TLS_SUITE_B_EAP_PHASE1_PARAM_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_AUTH_ALG_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_AUTH_ALG_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_BSSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_BSSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ALT_SUBJECT_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ALT_SUBJECT_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ANONYMOUS_IDENTITY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ANONYMOUS_IDENTITY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_CERT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_CERT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_PATH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_C_A_PATH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_CLIENT_CERT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_CLIENT_CERT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_DOMAIN_SUFFIX_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_DOMAIN_SUFFIX_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_ENGINE_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_IDENTITY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_IDENTITY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_METHOD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_METHOD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PASSWORD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PASSWORD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PHASE2_METHOD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PHASE2_METHOD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PRIVATE_KEY_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_PRIVATE_KEY_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_SUBJECT_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EAP_SUBJECT_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EDMG_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_EDMG_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_GROUP_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_GROUP_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_GROUP_MGMT_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_GROUP_MGMT_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_ID_STR_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_ID_STR_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_INTERFACE_NAME_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_INTERFACE_NAME_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_KEY_MGMT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_KEY_MGMT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_OCSP_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_OCSP_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PAIRWISE_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PAIRWISE_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PROTO_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PROTO_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PSK_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PSK_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PSK_PASSPHRASE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_PSK_PASSPHRASE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_REQUIRE_PMF_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_REQUIRE_PMF_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SAE_PASSWORD_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SCAN_SSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SCAN_SSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_SSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_TYPE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_TYPE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WAPI_CERT_SUITE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WAPI_CERT_SUITE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WEP_KEY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WEP_KEY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WEP_TX_KEY_IDX_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WEP_TX_KEY_IDX_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WPS_NFC_CONFIGURATION_TOKEN_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_GET_WPS_NFC_CONFIGURATION_TOKEN_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_REGISTER_CALLBACK_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_REGISTER_CALLBACK_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SELECT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SELECT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_IDENTITY_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_IDENTITY_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_FAILURE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_FAILURE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_GSM_AUTH_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_FAILURE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_FAILURE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTH_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTS_RESPONSE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SEND_NETWORK_EAP_SIM_UMTS_AUTS_RESPONSE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_AUTH_ALG_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_AUTH_ALG_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_BSSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_BSSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_DPP_KEYS_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_DPP_KEYS_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ALT_SUBJECT_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ALT_SUBJECT_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ANONYMOUS_IDENTITY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ANONYMOUS_IDENTITY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_CERT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_CERT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_PATH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_C_A_PATH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_CLIENT_CERT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_CLIENT_CERT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_DOMAIN_SUFFIX_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_DOMAIN_SUFFIX_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENCRYPTED_IMSI_IDENTITY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENCRYPTED_IMSI_IDENTITY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_I_D_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ENGINE_I_D_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ERP_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_ERP_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_IDENTITY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_IDENTITY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_METHOD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_METHOD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PASSWORD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PASSWORD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PHASE2_METHOD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PHASE2_METHOD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PRIVATE_KEY_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_PRIVATE_KEY_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_SUBJECT_MATCH_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EAP_SUBJECT_MATCH_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EDMG_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_EDMG_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_GROUP_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_GROUP_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_GROUP_MGMT_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_GROUP_MGMT_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_ID_STR_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_ID_STR_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_KEY_MGMT_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_KEY_MGMT_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_OCSP_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_OCSP_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PAIRWISE_CIPHER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PAIRWISE_CIPHER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PMK_CACHE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PMK_CACHE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PROACTIVE_KEY_CACHING_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PROACTIVE_KEY_CACHING_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PROTO_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PROTO_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PSK_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PSK_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PSK_PASSPHRASE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_PSK_PASSPHRASE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_REQUIRE_PMF_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_REQUIRE_PMF_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_H2E_MODE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_H2E_MODE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_ID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SAE_PASSWORD_ID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SCAN_SSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SCAN_SSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SSID_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_SSID_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_UPDATE_IDENTIFIER_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_UPDATE_IDENTIFIER_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WAPI_CERT_SUITE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WAPI_CERT_SUITE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WEP_KEY_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WEP_KEY_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WEP_TX_KEY_IDX_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_WEP_TX_KEY_IDX_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_ROAMING_CONSORTIUM_SELECTION_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_ROAMING_CONSORTIUM_SELECTION_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_MINIMUM_TLS_VERSION_EAP_PHASE1_PARAM_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_MINIMUM_TLS_VERSION_EAP_PHASE1_PARAM_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_STRICT_CONSERVATIVE_PEER_MODE_REQ());
    VERIFY(T_SUPPLICANT_STA_NETWORK_SET_STRICT_CONSERVATIVE_PEER_MODE_CFM());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_IDENTITY_REQUEST_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_SIM_GSM_AUTH_REQUEST_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_NETWORK_EAP_SIM_UMTS_AUTH_REQUEST_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_TRANSITION_DISABLE_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_SERVER_CERTIFICATE_AVAILABLE_IND());
    VERIFY(T_SUPPLICANT_STA_NETWORK_ON_PERMANENT_ID_REQ_DENIED_IND());
    VERIFY(T_SUPPLICANT_HAL_STATUS());
    VERIFY(T_NON_STANDARD_CERT_GET_BLOB_REQ());
    VERIFY(T_NON_STANDARD_CERT_GET_BLOB_CFM());
    VERIFY(T_NON_STANDARD_CERT_LIST_ALIASES_REQ());
    VERIFY(T_NON_STANDARD_CERT_LIST_ALIASES_CFM());
    ALOGD("ALL 560 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("supplicant-msg-test starts");
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
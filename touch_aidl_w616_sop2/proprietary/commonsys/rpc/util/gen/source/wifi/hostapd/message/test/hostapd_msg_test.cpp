/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "hostapd_msg.h"
#include "hostapd_msg_common.h"

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
    status.status = 21;
    status.info = "aG";
}

static void INIT_HW_MODE_PARAMS(HwModeParams& hwModeParams)
{
    hwModeParams.enable80211N = true;
    hwModeParams.enable80211AC = false;
    hwModeParams.enable80211AX = true;
    hwModeParams.enable6GhzBand = true;
    hwModeParams.enableHeSingleUserBeamformer = true;
    hwModeParams.enableHeSingleUserBeamformee = false;
    hwModeParams.enableHeMultiUserBeamformer = true;
    hwModeParams.enableHeTargetWakeTime = true;
    hwModeParams.enableEdmg = true;
    hwModeParams.enable80211BE = true;
    hwModeParams.maximumChannelBandwidth = ChannelBandwidth(1);
}

static void INIT_FREQUENCY_RANGE(FrequencyRange& acsChannelFreqRangesMhz)
{
    acsChannelFreqRangesMhz.startMhz = 63;
    acsChannelFreqRangesMhz.endMhz = 52;
}

static void INIT_CHANNEL_PARAMS(ChannelParams& channelParams)
{
    channelParams.bandMask = BandMask(1);
    channelParams.acsChannelFreqRangesMhz.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_FREQUENCY_RANGE(channelParams.acsChannelFreqRangesMhz[index]);
    }
    channelParams.enableAcs = true;
    channelParams.acsShouldExcludeDfs = true;
    channelParams.channel = 55;
}

static void INIT_IFACE_PARAMS(IfaceParams& ifaceParams)
{
    ifaceParams.name = "mAc558AP4lYfecW4";
    INIT_HW_MODE_PARAMS(ifaceParams.hwModeParams);
    ifaceParams.channelParams.resize(4);
    for (int index = 0; index < 4; index++)
    {
        INIT_CHANNEL_PARAMS(ifaceParams.channelParams[index]);
    }
}

static void INIT_NETWORK_PARAMS(NetworkParams& nwParams)
{
    nwParams.ssid.resize(4);
    for (int index = 0; index < 4; index++)
    {
        nwParams.ssid[index] = 0xcc;
    }
    nwParams.isHidden = true;
    nwParams.encryptionType = EncryptionType(1);
    nwParams.passphrase = "wsMyo";
    nwParams.isMetered = true;
    nwParams.vendorElements.resize(4);
    for (int index = 0; index < 4; index++)
    {
        nwParams.vendorElements[index] = 0x8f;
    }
}

static bool T_HOSTAPD_ADD_ACCESS_POINT_REQ()
{
    IfaceParams ifaceParams;
    INIT_IFACE_PARAMS(ifaceParams);
    NetworkParams nwParams;
    INIT_NETWORK_PARAMS(nwParams);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeAddAccessPointReq(ifaceParams, nwParams, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    AddAccessPointReqParam param;
    bool msgParseResult = HostapdParseAddAccessPointReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(ifaceParams, param.ifaceParams, "ifaceParams (IfaceParams)"));
    VERIFY(COMPARE_OBJ(nwParams, param.nwParams, "nwParams (NetworkParams)"));
    return true;
}

static bool T_HOSTAPD_ADD_ACCESS_POINT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeAddAccessPointCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseAddAccessPointCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_FORCE_CLIENT_DISCONNECT_REQ()
{
    string ifaceName = "Q7zBVBZG";
    vector<uint8_t> clientAddress;
    clientAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        clientAddress[index] = 0x2d;
    }
    Ieee80211ReasonCode reasonCode = Ieee80211ReasonCode(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeForceClientDisconnectReq(ifaceName, clientAddress, reasonCode, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ForceClientDisconnectReqParam param;
    bool msgParseResult = HostapdParseForceClientDisconnectReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    VERIFY(COMPARE_VEC(clientAddress, param.clientAddress, "clientAddress (vector<uint8_t>)"));
    VERIFY(COMPARE_BUF(&reasonCode, &param.reasonCode, sizeof(Ieee80211ReasonCode), "reasonCode (Ieee80211ReasonCode)"));
    return true;
}

static bool T_HOSTAPD_FORCE_CLIENT_DISCONNECT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeForceClientDisconnectCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseForceClientDisconnectCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_REGISTER_CALLBACK_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeRegisterCallbackReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseRegisterCallbackReq(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_REGISTER_CALLBACK_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeRegisterCallbackCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseRegisterCallbackCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_REMOVE_ACCESS_POINT_REQ()
{
    string ifaceName = "6QKM";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeRemoveAccessPointReq(ifaceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    string param;
    bool msgParseResult = HostapdParseRemoveAccessPointReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param, "ifaceName (string)"));
    return true;
}

static bool T_HOSTAPD_REMOVE_ACCESS_POINT_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeRemoveAccessPointCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseRemoveAccessPointCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_SET_DEBUG_PARAMS_REQ()
{
    DebugLevel level = DebugLevel(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeSetDebugParamsReq(level, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    DebugLevel param;
    bool msgParseResult = HostapdParseSetDebugParamsReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&level, &param, sizeof(DebugLevel), "level (DebugLevel)"));
    return true;
}

static bool T_HOSTAPD_SET_DEBUG_PARAMS_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeSetDebugParamsCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseSetDebugParamsCfm(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_TERMINATE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeTerminateReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseTerminateReq(payload.data(), payload.size());
    return true;
}

static bool T_HOSTAPD_TERMINATE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeTerminateCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = HostapdParseTerminateCfm(payload.data(), payload.size());
    return true;
}

static void INIT_AP_INFO(ApInfo& apInfo)
{
    apInfo.ifaceName = "C9h8EYIP";
    apInfo.apIfaceInstance = "4Knnf";
    apInfo.freqMhz = 45;
    apInfo.channelBandwidth = ChannelBandwidth(1);
    apInfo.generation = Generation(1);
    apInfo.apIfaceInstanceMacAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        apInfo.apIfaceInstanceMacAddress[index] = 0x4f;
    }
}

static bool T_HOSTAPD_ON_AP_INSTANCE_INFO_CHANGED_IND()
{
    ApInfo apInfo;
    INIT_AP_INFO(apInfo);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeOnApInstanceInfoChangedInd(apInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ApInfo param;
    bool msgParseResult = HostapdParseOnApInstanceInfoChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(apInfo, param, "apInfo (ApInfo)"));
    return true;
}

static void INIT_CLIENT_INFO(ClientInfo& clientInfo)
{
    clientInfo.ifaceName = "zhL6VVL8UQ";
    clientInfo.apIfaceInstance = "JKpSqIJVkjtBNd2";
    clientInfo.clientAddress.resize(4);
    for (int index = 0; index < 4; index++)
    {
        clientInfo.clientAddress[index] = 0xd0;
    }
    clientInfo.isConnected = false;
}

static bool T_HOSTAPD_ON_CONNECTED_CLIENTS_CHANGED_IND()
{
    ClientInfo clientInfo;
    INIT_CLIENT_INFO(clientInfo);
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeOnConnectedClientsChangedInd(clientInfo, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    ClientInfo param;
    bool msgParseResult = HostapdParseOnConnectedClientsChangedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(clientInfo, param, "clientInfo (ClientInfo)"));
    return true;
}

static bool T_HOSTAPD_ON_FAILURE_IND()
{
    string ifaceName = "jdERXP";
    string instanceName = "Y";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeOnFailureInd(ifaceName, instanceName, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    OnFailureIndParam param;
    bool msgParseResult = HostapdParseOnFailureInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_STR(ifaceName, param.ifaceName, "ifaceName (string)"));
    VERIFY(COMPARE_STR(instanceName, param.instanceName, "instanceName (string)"));
    return true;
}

static bool T_HOSTAPD_HAL_STATUS()
{
    int32_t status = 49;
    string info = "HAQiP";
    vector<uint8_t> payload;
    bool msgSerializeResult = HostapdSerializeHalStatus(status, info, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HalStatusParam param;
    bool msgParseResult = HostapdParseHalStatus(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param.status, sizeof(int32_t), "status (int32_t)"));
    VERIFY(COMPARE_STR(info, param.info, "info (string)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_HOSTAPD_ADD_ACCESS_POINT_REQ());
    VERIFY(T_HOSTAPD_ADD_ACCESS_POINT_CFM());
    VERIFY(T_HOSTAPD_FORCE_CLIENT_DISCONNECT_REQ());
    VERIFY(T_HOSTAPD_FORCE_CLIENT_DISCONNECT_CFM());
    VERIFY(T_HOSTAPD_REGISTER_CALLBACK_REQ());
    VERIFY(T_HOSTAPD_REGISTER_CALLBACK_CFM());
    VERIFY(T_HOSTAPD_REMOVE_ACCESS_POINT_REQ());
    VERIFY(T_HOSTAPD_REMOVE_ACCESS_POINT_CFM());
    VERIFY(T_HOSTAPD_SET_DEBUG_PARAMS_REQ());
    VERIFY(T_HOSTAPD_SET_DEBUG_PARAMS_CFM());
    VERIFY(T_HOSTAPD_TERMINATE_REQ());
    VERIFY(T_HOSTAPD_TERMINATE_CFM());
    VERIFY(T_HOSTAPD_ON_AP_INSTANCE_INFO_CHANGED_IND());
    VERIFY(T_HOSTAPD_ON_CONNECTED_CLIENTS_CHANGED_IND());
    VERIFY(T_HOSTAPD_ON_FAILURE_IND());
    VERIFY(T_HOSTAPD_HAL_STATUS());
    ALOGD("ALL 16 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("hostapd-msg-test starts");
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
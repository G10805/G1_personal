/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "hostapd_msg.h"

#include "ApInfo.pb.h"
#include "BandMask.pb.h"
#include "ChannelBandwidth.pb.h"
#include "ChannelParams.pb.h"
#include "ClientInfo.pb.h"
#include "DebugLevel.pb.h"
#include "EncryptionType.pb.h"
#include "FrequencyRange.pb.h"
#include "Generation.pb.h"
#include "HalStatus.pb.h"
#include "HwModeParams.pb.h"
#include "IHostapd.pb.h"
#include "IHostapdCallback.pb.h"
#include "Ieee80211ReasonCode.pb.h"
#include "IfaceParams.pb.h"
#include "NetworkParams.pb.h"

using AddAccessPointReq = ::wifi::hostapd::IHostapd_AddAccessPointReq;
using ForceClientDisconnectReq = ::wifi::hostapd::IHostapd_ForceClientDisconnectReq;
using OnApInstanceInfoChangedInd = ::wifi::hostapd::IHostapdCallback_OnApInstanceInfoChangedInd;
using OnConnectedClientsChangedInd = ::wifi::hostapd::IHostapdCallback_OnConnectedClientsChangedInd;
using OnFailureInd = ::wifi::hostapd::IHostapdCallback_OnFailureInd;
using RemoveAccessPointReq = ::wifi::hostapd::IHostapd_RemoveAccessPointReq;
using SetDebugParamsReq = ::wifi::hostapd::IHostapd_SetDebugParamsReq;

using ApInfo_P = ::wifi::hostapd::ApInfo;
using BandMask_P = ::wifi::hostapd::BandMask;
using ChannelBandwidth_P = ::wifi::hostapd::ChannelBandwidth;
using ChannelParams_P = ::wifi::hostapd::ChannelParams;
using ClientInfo_P = ::wifi::hostapd::ClientInfo;
using DebugLevel_P = ::wifi::hostapd::DebugLevel;
using EncryptionType_P = ::wifi::hostapd::EncryptionType;
using FrequencyRange_P = ::wifi::hostapd::FrequencyRange;
using Generation_P = ::wifi::hostapd::Generation;
using HalStatus_P = ::wifi::hostapd::HalStatus;
using HwModeParams_P = ::wifi::hostapd::HwModeParams;
using Ieee80211ReasonCode_P = ::wifi::hostapd::Ieee80211ReasonCode;
using IfaceParams_P = ::wifi::hostapd::IfaceParams;
using NetworkParams_P = ::wifi::hostapd::NetworkParams;


static void HwModeParams2Message(const HwModeParams& hwModeParams, HwModeParams_P& msgHostapd)
{
    msgHostapd.set_enable80211n(hwModeParams.enable80211N);
    msgHostapd.set_enable80211ac(hwModeParams.enable80211AC);
    msgHostapd.set_enable80211ax(hwModeParams.enable80211AX);
    msgHostapd.set_enable6ghzband(hwModeParams.enable6GhzBand);
    msgHostapd.set_enablehesingleuserbeamformer(hwModeParams.enableHeSingleUserBeamformer);
    msgHostapd.set_enablehesingleuserbeamformee(hwModeParams.enableHeSingleUserBeamformee);
    msgHostapd.set_enablehemultiuserbeamformer(hwModeParams.enableHeMultiUserBeamformer);
    msgHostapd.set_enablehetargetwaketime(hwModeParams.enableHeTargetWakeTime);
    msgHostapd.set_enableedmg(hwModeParams.enableEdmg);
    msgHostapd.set_enable80211be(hwModeParams.enable80211BE);
    msgHostapd.set_maximumchannelbandwidth((ChannelBandwidth_P) hwModeParams.maximumChannelBandwidth);
}

static void FrequencyRange2Message(const FrequencyRange& acsChannelFreqRangesMhz, FrequencyRange_P& msgHostapd)
{
    msgHostapd.set_startmhz(acsChannelFreqRangesMhz.startMhz);
    msgHostapd.set_endmhz(acsChannelFreqRangesMhz.endMhz);
}

static void ChannelParams2Message(const ChannelParams& channelParams, ChannelParams_P& msgHostapd)
{
    msgHostapd.set_bandmask((BandMask_P) channelParams.bandMask);
    int size_acschannelfreqrangesmhz = channelParams.acsChannelFreqRangesMhz.size();
    if (0 < size_acschannelfreqrangesmhz)
    {
        for (int index = 0; index < size_acschannelfreqrangesmhz; index++)
        {
            FrequencyRange_P *acsChannelFreqRangesMhz_p = msgHostapd.add_acschannelfreqrangesmhz();
            FrequencyRange2Message(channelParams.acsChannelFreqRangesMhz[index], *acsChannelFreqRangesMhz_p);
        }
    }
    msgHostapd.set_enableacs(channelParams.enableAcs);
    msgHostapd.set_acsshouldexcludedfs(channelParams.acsShouldExcludeDfs);
    msgHostapd.set_channel(channelParams.channel);
}

static void IfaceParams2Message(const IfaceParams& ifaceParams, IfaceParams_P& msgHostapd)
{
    msgHostapd.set_name(ifaceParams.name);
    HwModeParams_P* hwModeParams_p = msgHostapd.mutable_hwmodeparams();
    HwModeParams2Message(ifaceParams.hwModeParams, *hwModeParams_p);
    int size_channelparams = ifaceParams.channelParams.size();
    if (0 < size_channelparams)
    {
        for (int index = 0; index < size_channelparams; index++)
        {
            ChannelParams_P *channelParams_p = msgHostapd.add_channelparams();
            ChannelParams2Message(ifaceParams.channelParams[index], *channelParams_p);
        }
    }
}

static void NetworkParams2Message(const NetworkParams& nwParams, NetworkParams_P& msgHostapd)
{
    string ssid;
    Vector2String(nwParams.ssid, ssid);
    msgHostapd.set_ssid(ssid);
    msgHostapd.set_ishidden(nwParams.isHidden);
    msgHostapd.set_encryptiontype((EncryptionType_P) nwParams.encryptionType);
    msgHostapd.set_passphrase(nwParams.passphrase);
    msgHostapd.set_ismetered(nwParams.isMetered);
    string vendorElements;
    Vector2String(nwParams.vendorElements, vendorElements);
    msgHostapd.set_vendorelements(vendorElements);
}

bool HostapdSerializeAddAccessPointReq(const IfaceParams& ifaceParams, const NetworkParams& nwParams, vector<uint8_t>& payload)
{
    AddAccessPointReq msgHostapd;
    IfaceParams_P* ifaceParams_p = msgHostapd.mutable_ifaceparams();
    IfaceParams2Message(ifaceParams, *ifaceParams_p);
    NetworkParams_P* nwParams_p = msgHostapd.mutable_nwparams();
    NetworkParams2Message(nwParams, *nwParams_p);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgHostapd)
{
    msgHostapd.set_status(status.status);
    msgHostapd.set_info(status.info);
}

bool HostapdSerializeAddAccessPointCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeForceClientDisconnectReq(const string& ifaceName, const vector<uint8_t>& clientAddress, Ieee80211ReasonCode reasonCode, vector<uint8_t>& payload)
{
    ForceClientDisconnectReq msgHostapd;
    msgHostapd.set_ifacename(ifaceName);
    string clientAddressStr;
    Vector2String(clientAddress, clientAddressStr);
    msgHostapd.set_clientaddress(clientAddressStr);
    msgHostapd.set_reasoncode((Ieee80211ReasonCode_P) reasonCode);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

bool HostapdSerializeForceClientDisconnectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeRegisterCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeRegisterCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeRemoveAccessPointReq(const string& ifaceName, vector<uint8_t>& payload)
{
    RemoveAccessPointReq msgHostapd;
    msgHostapd.set_ifacename(ifaceName);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

bool HostapdSerializeRemoveAccessPointCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeSetDebugParamsReq(DebugLevel level, vector<uint8_t>& payload)
{
    SetDebugParamsReq msgHostapd;
    msgHostapd.set_level((DebugLevel_P) level);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

bool HostapdSerializeSetDebugParamsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeTerminateReq(vector<uint8_t>& payload)
{
    return true;
}

bool HostapdSerializeTerminateCfm(vector<uint8_t>& payload)
{
    return true;
}

static void ApInfo2Message(const ApInfo& apInfo, ApInfo_P& msgHostapd)
{
    msgHostapd.set_ifacename(apInfo.ifaceName);
    msgHostapd.set_apifaceinstance(apInfo.apIfaceInstance);
    msgHostapd.set_freqmhz(apInfo.freqMhz);
    msgHostapd.set_channelbandwidth((ChannelBandwidth_P) apInfo.channelBandwidth);
    msgHostapd.set_generation((Generation_P) apInfo.generation);
    string apIfaceInstanceMacAddress;
    Vector2String(apInfo.apIfaceInstanceMacAddress, apIfaceInstanceMacAddress);
    msgHostapd.set_apifaceinstancemacaddress(apIfaceInstanceMacAddress);
}

bool HostapdSerializeOnApInstanceInfoChangedInd(const ApInfo& apInfo, vector<uint8_t>& payload)
{
    OnApInstanceInfoChangedInd msgHostapd;
    ApInfo_P* apInfo_p = msgHostapd.mutable_apinfo();
    ApInfo2Message(apInfo, *apInfo_p);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

static void ClientInfo2Message(const ClientInfo& clientInfo, ClientInfo_P& msgHostapd)
{
    msgHostapd.set_ifacename(clientInfo.ifaceName);
    msgHostapd.set_apifaceinstance(clientInfo.apIfaceInstance);
    string clientAddress;
    Vector2String(clientInfo.clientAddress, clientAddress);
    msgHostapd.set_clientaddress(clientAddress);
    msgHostapd.set_isconnected(clientInfo.isConnected);
}

bool HostapdSerializeOnConnectedClientsChangedInd(const ClientInfo& clientInfo, vector<uint8_t>& payload)
{
    OnConnectedClientsChangedInd msgHostapd;
    ClientInfo_P* clientInfo_p = msgHostapd.mutable_clientinfo();
    ClientInfo2Message(clientInfo, *clientInfo_p);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

bool HostapdSerializeOnFailureInd(const string& ifaceName, const string& instanceName, vector<uint8_t>& payload)
{
    OnFailureInd msgHostapd;
    msgHostapd.set_ifacename(ifaceName);
    msgHostapd.set_instancename(instanceName);
    return ProtoMessageSerialize(&msgHostapd, payload);
}

static void Message2HwModeParams(const HwModeParams_P& msgHostapd, HwModeParams& hwModeParams)
{
    hwModeParams.enable80211N = msgHostapd.enable80211n();
    hwModeParams.enable80211AC = msgHostapd.enable80211ac();
    hwModeParams.enable80211AX = msgHostapd.enable80211ax();
    hwModeParams.enable6GhzBand = msgHostapd.enable6ghzband();
    hwModeParams.enableHeSingleUserBeamformer = msgHostapd.enablehesingleuserbeamformer();
    hwModeParams.enableHeSingleUserBeamformee = msgHostapd.enablehesingleuserbeamformee();
    hwModeParams.enableHeMultiUserBeamformer = msgHostapd.enablehemultiuserbeamformer();
    hwModeParams.enableHeTargetWakeTime = msgHostapd.enablehetargetwaketime();
    hwModeParams.enableEdmg = msgHostapd.enableedmg();
    hwModeParams.enable80211BE = msgHostapd.enable80211be();
    hwModeParams.maximumChannelBandwidth = (ChannelBandwidth) msgHostapd.maximumchannelbandwidth();
}

static void Message2FrequencyRange(const FrequencyRange_P& msgHostapd, FrequencyRange& acsChannelFreqRangesMhz)
{
    acsChannelFreqRangesMhz.startMhz = msgHostapd.startmhz();
    acsChannelFreqRangesMhz.endMhz = msgHostapd.endmhz();
}

static void Message2ChannelParams(const ChannelParams_P& msgHostapd, ChannelParams& channelParams)
{
    channelParams.bandMask = (BandMask) msgHostapd.bandmask();
    int size_acschannelfreqrangesmhz = msgHostapd.acschannelfreqrangesmhz_size();
    if (0 < size_acschannelfreqrangesmhz)
    {
        channelParams.acsChannelFreqRangesMhz.resize(size_acschannelfreqrangesmhz);
        for (int index = 0; index < size_acschannelfreqrangesmhz; index++)
        {
            Message2FrequencyRange(msgHostapd.acschannelfreqrangesmhz(index), channelParams.acsChannelFreqRangesMhz[index]);
        }
    }
    channelParams.enableAcs = msgHostapd.enableacs();
    channelParams.acsShouldExcludeDfs = msgHostapd.acsshouldexcludedfs();
    channelParams.channel = msgHostapd.channel();
}

static void Message2IfaceParams(const IfaceParams_P& msgHostapd, IfaceParams& ifaceParams)
{
    ifaceParams.name = msgHostapd.name();
    Message2HwModeParams(msgHostapd.hwmodeparams(), ifaceParams.hwModeParams);
    int size_channelparams = msgHostapd.channelparams_size();
    if (0 < size_channelparams)
    {
        ifaceParams.channelParams.resize(size_channelparams);
        for (int index = 0; index < size_channelparams; index++)
        {
            Message2ChannelParams(msgHostapd.channelparams(index), ifaceParams.channelParams[index]);
        }
    }
}

static void Message2NetworkParams(const NetworkParams_P& msgHostapd, NetworkParams& nwParams)
{
    String2Vector(msgHostapd.ssid(), nwParams.ssid);
    nwParams.isHidden = msgHostapd.ishidden();
    nwParams.encryptionType = (EncryptionType) msgHostapd.encryptiontype();
    nwParams.passphrase = msgHostapd.passphrase();
    nwParams.isMetered = msgHostapd.ismetered();
    String2Vector(msgHostapd.vendorelements(), nwParams.vendorElements);
}

bool HostapdParseAddAccessPointReq(const uint8_t* data, size_t length, AddAccessPointReqParam& param)
{
    AddAccessPointReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2IfaceParams(msg.ifaceparams(), param.ifaceParams);
        Message2NetworkParams(msg.nwparams(), param.nwParams);
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgHostapd, HalStatusParam& status)
{
    status.status = msgHostapd.status();
    status.info = msgHostapd.info();
}

bool HostapdParseAddAccessPointCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseForceClientDisconnectReq(const uint8_t* data, size_t length, ForceClientDisconnectReqParam& param)
{
    ForceClientDisconnectReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ifaceName = msg.ifacename();
        String2Vector(msg.clientaddress(), param.clientAddress);
        param.reasonCode = (Ieee80211ReasonCode) msg.reasoncode();
        return true;
    }
    return false;
}

bool HostapdParseForceClientDisconnectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseRegisterCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseRegisterCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseRemoveAccessPointReq(const uint8_t* data, size_t length, string& param)
{
    RemoveAccessPointReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifacename();
        return true;
    }
    return false;
}

bool HostapdParseRemoveAccessPointCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseSetDebugParamsReq(const uint8_t* data, size_t length, DebugLevel& param)
{
    SetDebugParamsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (DebugLevel) msg.level();
        return true;
    }
    return false;
}

bool HostapdParseSetDebugParamsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseTerminateReq(const uint8_t* data, size_t length)
{
    return true;
}

bool HostapdParseTerminateCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ApInfo(const ApInfo_P& msgHostapd, ApInfo& apInfo)
{
    apInfo.ifaceName = msgHostapd.ifacename();
    apInfo.apIfaceInstance = msgHostapd.apifaceinstance();
    apInfo.freqMhz = msgHostapd.freqmhz();
    apInfo.channelBandwidth = (ChannelBandwidth) msgHostapd.channelbandwidth();
    apInfo.generation = (Generation) msgHostapd.generation();
    String2Vector(msgHostapd.apifaceinstancemacaddress(), apInfo.apIfaceInstanceMacAddress);
}

bool HostapdParseOnApInstanceInfoChangedInd(const uint8_t* data, size_t length, ApInfo& param)
{
    OnApInstanceInfoChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2ApInfo(msg.apinfo(), param);
        return true;
    }
    return false;
}

static void Message2ClientInfo(const ClientInfo_P& msgHostapd, ClientInfo& clientInfo)
{
    clientInfo.ifaceName = msgHostapd.ifacename();
    clientInfo.apIfaceInstance = msgHostapd.apifaceinstance();
    String2Vector(msgHostapd.clientaddress(), clientInfo.clientAddress);
    clientInfo.isConnected = msgHostapd.isconnected();
}

bool HostapdParseOnConnectedClientsChangedInd(const uint8_t* data, size_t length, ClientInfo& param)
{
    OnConnectedClientsChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2ClientInfo(msg.clientinfo(), param);
        return true;
    }
    return false;
}

bool HostapdParseOnFailureInd(const uint8_t* data, size_t length, OnFailureIndParam& param)
{
    OnFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ifaceName = msg.ifacename();
        param.instanceName = msg.instancename();
        return true;
    }
    return false;
}
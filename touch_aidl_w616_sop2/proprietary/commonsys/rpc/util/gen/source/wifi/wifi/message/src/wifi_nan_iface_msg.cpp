/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_nan_iface_msg.h"

#include "HalStatus.pb.h"
#include "IWifiNanIface.pb.h"
#include "IWifiNanIfaceEventCallback.pb.h"
#include "MacAddress.pb.h"
#include "NanBandSpecificConfig.pb.h"
#include "NanBootstrappingConfirmInd.pb.h"
#include "NanBootstrappingMethod.pb.h"
#include "NanBootstrappingRequest.pb.h"
#include "NanBootstrappingRequestInd.pb.h"
#include "NanBootstrappingResponse.pb.h"
#include "NanBootstrappingResponseCode.pb.h"
#include "NanCapabilities.pb.h"
#include "NanCipherSuiteType.pb.h"
#include "NanClusterEventInd.pb.h"
#include "NanClusterEventType.pb.h"
#include "NanConfigRequest.pb.h"
#include "NanConfigRequestSupplemental.pb.h"
#include "NanDataPathChannelCfg.pb.h"
#include "NanDataPathChannelInfo.pb.h"
#include "NanDataPathConfirmInd.pb.h"
#include "NanDataPathRequestInd.pb.h"
#include "NanDataPathScheduleUpdateInd.pb.h"
#include "NanDataPathSecurityConfig.pb.h"
#include "NanDataPathSecurityType.pb.h"
#include "NanDebugConfig.pb.h"
#include "NanDiscoveryCommonConfig.pb.h"
#include "NanEnableRequest.pb.h"
#include "NanFollowupReceivedInd.pb.h"
#include "NanIdentityResolutionAttribute.pb.h"
#include "NanInitiateDataPathRequest.pb.h"
#include "NanMatchAlg.pb.h"
#include "NanMatchInd.pb.h"
#include "NanPairingAkm.pb.h"
#include "NanPairingConfig.pb.h"
#include "NanPairingConfirmInd.pb.h"
#include "NanPairingRequest.pb.h"
#include "NanPairingRequestInd.pb.h"
#include "NanPairingRequestType.pb.h"
#include "NanPairingSecurityConfig.pb.h"
#include "NanPairingSecurityType.pb.h"
#include "NanPublishRequest.pb.h"
#include "NanPublishType.pb.h"
#include "NanRespondToDataPathIndicationRequest.pb.h"
#include "NanRespondToPairingIndicationRequest.pb.h"
#include "NanSrfType.pb.h"
#include "NanStatus.pb.h"
#include "NanStatusCode.pb.h"
#include "NanSubscribeRequest.pb.h"
#include "NanSubscribeType.pb.h"
#include "NanSuspensionModeChangeInd.pb.h"
#include "NanTransmitFollowupRequest.pb.h"
#include "NanTxType.pb.h"
#include "NpkSecurityAssociation.pb.h"
#include "WifiChannelWidthInMhz.pb.h"

using ConfigRequestReq = ::wifi::IWifiNanIface_ConfigRequestReq;
using CreateDataInterfaceRequestReq = ::wifi::IWifiNanIface_CreateDataInterfaceRequestReq;
using DeleteDataInterfaceRequestReq = ::wifi::IWifiNanIface_DeleteDataInterfaceRequestReq;
using DisableRequestReq = ::wifi::IWifiNanIface_DisableRequestReq;
using EnableRequestReq = ::wifi::IWifiNanIface_EnableRequestReq;
using EventBootstrappingConfirmInd = ::wifi::IWifiNanIfaceEventCallback_EventBootstrappingConfirmInd;
using EventBootstrappingRequestInd = ::wifi::IWifiNanIfaceEventCallback_EventBootstrappingRequestInd;
using EventClusterEventInd = ::wifi::IWifiNanIfaceEventCallback_EventClusterEventInd;
using EventDataPathConfirmInd = ::wifi::IWifiNanIfaceEventCallback_EventDataPathConfirmInd;
using EventDataPathRequestInd = ::wifi::IWifiNanIfaceEventCallback_EventDataPathRequestInd;
using EventDataPathScheduleUpdateInd = ::wifi::IWifiNanIfaceEventCallback_EventDataPathScheduleUpdateInd;
using EventDataPathTerminatedInd = ::wifi::IWifiNanIfaceEventCallback_EventDataPathTerminatedInd;
using EventDisabledInd = ::wifi::IWifiNanIfaceEventCallback_EventDisabledInd;
using EventFollowupReceivedInd = ::wifi::IWifiNanIfaceEventCallback_EventFollowupReceivedInd;
using EventMatchExpiredInd = ::wifi::IWifiNanIfaceEventCallback_EventMatchExpiredInd;
using EventMatchInd = ::wifi::IWifiNanIfaceEventCallback_EventMatchInd;
using EventPairingConfirmInd = ::wifi::IWifiNanIfaceEventCallback_EventPairingConfirmInd;
using EventPairingRequestInd = ::wifi::IWifiNanIfaceEventCallback_EventPairingRequestInd;
using EventPublishTerminatedInd = ::wifi::IWifiNanIfaceEventCallback_EventPublishTerminatedInd;
using EventSubscribeTerminatedInd = ::wifi::IWifiNanIfaceEventCallback_EventSubscribeTerminatedInd;
using EventSuspensionModeChangedInd = ::wifi::IWifiNanIfaceEventCallback_EventSuspensionModeChangedInd;
using EventTransmitFollowupInd = ::wifi::IWifiNanIfaceEventCallback_EventTransmitFollowupInd;
using GetCapabilitiesRequestReq = ::wifi::IWifiNanIface_GetCapabilitiesRequestReq;
using GetNameCfm = ::wifi::IWifiNanIface_GetNameCfm;
using InitiateBootstrappingRequestReq = ::wifi::IWifiNanIface_InitiateBootstrappingRequestReq;
using InitiateDataPathRequestReq = ::wifi::IWifiNanIface_InitiateDataPathRequestReq;
using InitiatePairingRequestReq = ::wifi::IWifiNanIface_InitiatePairingRequestReq;
using NotifyCapabilitiesResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyCapabilitiesResponseInd;
using NotifyConfigResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyConfigResponseInd;
using NotifyCreateDataInterfaceResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyCreateDataInterfaceResponseInd;
using NotifyDeleteDataInterfaceResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyDeleteDataInterfaceResponseInd;
using NotifyDisableResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyDisableResponseInd;
using NotifyEnableResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyEnableResponseInd;
using NotifyInitiateBootstrappingResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyInitiateBootstrappingResponseInd;
using NotifyInitiateDataPathResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyInitiateDataPathResponseInd;
using NotifyInitiatePairingResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyInitiatePairingResponseInd;
using NotifyRespondToBootstrappingIndicationResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyRespondToBootstrappingIndicationResponseInd;
using NotifyRespondToDataPathIndicationResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyRespondToDataPathIndicationResponseInd;
using NotifyRespondToPairingIndicationResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyRespondToPairingIndicationResponseInd;
using NotifyResumeResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyResumeResponseInd;
using NotifyStartPublishResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyStartPublishResponseInd;
using NotifyStartSubscribeResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyStartSubscribeResponseInd;
using NotifyStopPublishResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyStopPublishResponseInd;
using NotifyStopSubscribeResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyStopSubscribeResponseInd;
using NotifySuspendResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifySuspendResponseInd;
using NotifyTerminateDataPathResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyTerminateDataPathResponseInd;
using NotifyTerminatePairingResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyTerminatePairingResponseInd;
using NotifyTransmitFollowupResponseInd = ::wifi::IWifiNanIfaceEventCallback_NotifyTransmitFollowupResponseInd;
using RespondToBootstrappingIndicationRequestReq = ::wifi::IWifiNanIface_RespondToBootstrappingIndicationRequestReq;
using RespondToDataPathIndicationRequestReq = ::wifi::IWifiNanIface_RespondToDataPathIndicationRequestReq;
using RespondToPairingIndicationRequestReq = ::wifi::IWifiNanIface_RespondToPairingIndicationRequestReq;
using ResumeRequestReq = ::wifi::IWifiNanIface_ResumeRequestReq;
using StartPublishRequestReq = ::wifi::IWifiNanIface_StartPublishRequestReq;
using StartSubscribeRequestReq = ::wifi::IWifiNanIface_StartSubscribeRequestReq;
using StopPublishRequestReq = ::wifi::IWifiNanIface_StopPublishRequestReq;
using StopSubscribeRequestReq = ::wifi::IWifiNanIface_StopSubscribeRequestReq;
using SuspendRequestReq = ::wifi::IWifiNanIface_SuspendRequestReq;
using TerminateDataPathRequestReq = ::wifi::IWifiNanIface_TerminateDataPathRequestReq;
using TerminatePairingRequestReq = ::wifi::IWifiNanIface_TerminatePairingRequestReq;
using TransmitFollowupRequestReq = ::wifi::IWifiNanIface_TransmitFollowupRequestReq;

using HalStatus_P = ::wifi::HalStatus;
using MacAddress_P = ::wifi::MacAddress;
using NanBandSpecificConfig_P = ::wifi::NanBandSpecificConfig;
using NanBootstrappingConfirmInd_P = ::wifi::NanBootstrappingConfirmInd;
using NanBootstrappingMethod_P = ::wifi::NanBootstrappingMethod;
using NanBootstrappingRequestInd_P = ::wifi::NanBootstrappingRequestInd;
using NanBootstrappingRequest_P = ::wifi::NanBootstrappingRequest;
using NanBootstrappingResponseCode_P = ::wifi::NanBootstrappingResponseCode;
using NanBootstrappingResponse_P = ::wifi::NanBootstrappingResponse;
using NanCapabilities_P = ::wifi::NanCapabilities;
using NanCipherSuiteType_P = ::wifi::ncst::NanCipherSuiteType;
using NanClusterEventInd_P = ::wifi::NanClusterEventInd;
using NanClusterEventType_P = ::wifi::NanClusterEventType;
using NanConfigRequestSupplemental_P = ::wifi::NanConfigRequestSupplemental;
using NanConfigRequest_P = ::wifi::NanConfigRequest;
using NanDataPathChannelCfg_P = ::wifi::NanDataPathChannelCfg;
using NanDataPathChannelInfo_P = ::wifi::NanDataPathChannelInfo;
using NanDataPathConfirmInd_P = ::wifi::NanDataPathConfirmInd;
using NanDataPathRequestInd_P = ::wifi::NanDataPathRequestInd;
using NanDataPathScheduleUpdateInd_P = ::wifi::NanDataPathScheduleUpdateInd;
using NanDataPathSecurityConfig_P = ::wifi::NanDataPathSecurityConfig;
using NanDataPathSecurityType_P = ::wifi::ndpst::NanDataPathSecurityType;
using NanDebugConfig_P = ::wifi::NanDebugConfig;
using NanDiscoveryCommonConfig_P = ::wifi::NanDiscoveryCommonConfig;
using NanEnableRequest_P = ::wifi::NanEnableRequest;
using NanFollowupReceivedInd_P = ::wifi::NanFollowupReceivedInd;
using NanIdentityResolutionAttribute_P = ::wifi::NanIdentityResolutionAttribute;
using NanInitiateDataPathRequest_P = ::wifi::NanInitiateDataPathRequest;
using NanMatchAlg_P = ::wifi::NanMatchAlg;
using NanMatchInd_P = ::wifi::NanMatchInd;
using NanPairingAkm_P = ::wifi::NanPairingAkm;
using NanPairingConfig_P = ::wifi::NanPairingConfig;
using NanPairingConfirmInd_P = ::wifi::NanPairingConfirmInd;
using NanPairingRequestInd_P = ::wifi::NanPairingRequestInd;
using NanPairingRequestType_P = ::wifi::NanPairingRequestType;
using NanPairingRequest_P = ::wifi::NanPairingRequest;
using NanPairingSecurityConfig_P = ::wifi::NanPairingSecurityConfig;
using NanPairingSecurityType_P = ::wifi::npst::NanPairingSecurityType;
using NanPublishRequest_P = ::wifi::NanPublishRequest;
using NanPublishType_P = ::wifi::NanPublishType;
using NanRespondToDataPathIndicationRequest_P = ::wifi::NanRespondToDataPathIndicationRequest;
using NanRespondToPairingIndicationRequest_P = ::wifi::NanRespondToPairingIndicationRequest;
using NanSrfType_P = ::wifi::NanSrfType;
using NanStatusCode_P = ::wifi::nsc::NanStatusCode;
using NanStatus_P = ::wifi::NanStatus;
using NanSubscribeRequest_P = ::wifi::NanSubscribeRequest;
using NanSubscribeType_P = ::wifi::NanSubscribeType;
using NanSuspensionModeChangeInd_P = ::wifi::NanSuspensionModeChangeInd;
using NanTransmitFollowupRequest_P = ::wifi::NanTransmitFollowupRequest;
using NanTxType_P = ::wifi::NanTxType;
using NpkSecurityAssociation_P = ::wifi::NpkSecurityAssociation;
using WifiChannelWidthInMhz_P = ::wifi::WifiChannelWidthInMhz;


bool WifiNanIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiNanIface)
{
    msgWifiNanIface.set_status(status.status);
    msgWifiNanIface.set_info(status.info);
}

bool WifiNanIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgWifiNanIface;
    HalStatus_P* status_p = msgWifiNanIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiNanIface.set_result(result);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanBandSpecificConfig2Message(const NanBandSpecificConfig& bandSpecificConfig, NanBandSpecificConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_rssiclose(bandSpecificConfig.rssiClose);
    msgWifiNanIface.set_rssimiddle(bandSpecificConfig.rssiMiddle);
    msgWifiNanIface.set_rssicloseproximity(bandSpecificConfig.rssiCloseProximity);
    msgWifiNanIface.set_dwelltimems(bandSpecificConfig.dwellTimeMs);
    msgWifiNanIface.set_scanperiodsec(bandSpecificConfig.scanPeriodSec);
    msgWifiNanIface.set_validdiscoverywindowintervalval(bandSpecificConfig.validDiscoveryWindowIntervalVal);
    msgWifiNanIface.set_discoverywindowintervalval(bandSpecificConfig.discoveryWindowIntervalVal);
}

static void NanConfigRequest2Message(const NanConfigRequest& msg1, NanConfigRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_masterpref(msg1.masterPref);
    msgWifiNanIface.set_disablediscoveryaddresschangeindication(msg1.disableDiscoveryAddressChangeIndication);
    msgWifiNanIface.set_disablestartedclusterindication(msg1.disableStartedClusterIndication);
    msgWifiNanIface.set_disablejoinedclusterindication(msg1.disableJoinedClusterIndication);
    msgWifiNanIface.set_includepublishserviceidsinbeacon(msg1.includePublishServiceIdsInBeacon);
    msgWifiNanIface.set_numberofpublishserviceidsinbeacon(msg1.numberOfPublishServiceIdsInBeacon);
    msgWifiNanIface.set_includesubscribeserviceidsinbeacon(msg1.includeSubscribeServiceIdsInBeacon);
    msgWifiNanIface.set_numberofsubscribeserviceidsinbeacon(msg1.numberOfSubscribeServiceIdsInBeacon);
    msgWifiNanIface.set_rssiwindowsize(msg1.rssiWindowSize);
    msgWifiNanIface.set_macaddressrandomizationintervalsec(msg1.macAddressRandomizationIntervalSec);
    int size_bandspecificconfig = msg1.bandSpecificConfig.size();
    if (0 < size_bandspecificconfig)
    {
        for (int index = 0; index < size_bandspecificconfig; index++)
        {
            NanBandSpecificConfig_P *bandSpecificConfig_p = msgWifiNanIface.add_bandspecificconfig();
            NanBandSpecificConfig2Message(msg1.bandSpecificConfig[index], *bandSpecificConfig_p);
        }
    }
}

static void NanConfigRequestSupplemental2Message(const NanConfigRequestSupplemental& msg2, NanConfigRequestSupplemental_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverybeaconintervalms(msg2.discoveryBeaconIntervalMs);
    msgWifiNanIface.set_numberofspatialstreamsindiscovery(msg2.numberOfSpatialStreamsInDiscovery);
    msgWifiNanIface.set_enablediscoverywindowearlytermination(msg2.enableDiscoveryWindowEarlyTermination);
    msgWifiNanIface.set_enableranging(msg2.enableRanging);
    msgWifiNanIface.set_enableinstantcommunicationmode(msg2.enableInstantCommunicationMode);
    msgWifiNanIface.set_instantmodechannel(msg2.instantModeChannel);
    msgWifiNanIface.set_clusterid(msg2.clusterId);
}

bool WifiNanIfaceSerializeConfigRequestReq(char16_t cmdId, const NanConfigRequest& msg1, const NanConfigRequestSupplemental& msg2, vector<uint8_t>& payload)
{
    ConfigRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanConfigRequest_P* msg1_p = msgWifiNanIface.mutable_msg1();
    NanConfigRequest2Message(msg1, *msg1_p);
    NanConfigRequestSupplemental_P* msg2_p = msgWifiNanIface.mutable_msg2();
    NanConfigRequestSupplemental2Message(msg2, *msg2_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeConfigRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeCreateDataInterfaceRequestReq(char16_t cmdId, const string& ifaceName, vector<uint8_t>& payload)
{
    CreateDataInterfaceRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_ifacename(ifaceName);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeCreateDataInterfaceRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeDeleteDataInterfaceRequestReq(char16_t cmdId, const string& ifaceName, vector<uint8_t>& payload)
{
    DeleteDataInterfaceRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_ifacename(ifaceName);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeDeleteDataInterfaceRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeDisableRequestReq(char16_t cmdId, vector<uint8_t>& payload)
{
    DisableRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeDisableRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanDebugConfig2Message(const NanDebugConfig& debugConfigs, NanDebugConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_validclusteridvals(debugConfigs.validClusterIdVals);
    msgWifiNanIface.set_clusteridbottomrangeval(debugConfigs.clusterIdBottomRangeVal);
    msgWifiNanIface.set_clusteridtoprangeval(debugConfigs.clusterIdTopRangeVal);
    msgWifiNanIface.set_validintfaddrval(debugConfigs.validIntfAddrVal);
    string intfAddrVal;
    Array2String(debugConfigs.intfAddrVal, intfAddrVal);
    msgWifiNanIface.set_intfaddrval(intfAddrVal);
    msgWifiNanIface.set_validouival(debugConfigs.validOuiVal);
    msgWifiNanIface.set_ouival(debugConfigs.ouiVal);
    msgWifiNanIface.set_validrandomfactorforceval(debugConfigs.validRandomFactorForceVal);
    msgWifiNanIface.set_randomfactorforceval(debugConfigs.randomFactorForceVal);
    msgWifiNanIface.set_validhopcountforceval(debugConfigs.validHopCountForceVal);
    msgWifiNanIface.set_hopcountforceval(debugConfigs.hopCountForceVal);
    msgWifiNanIface.set_validdiscoverychannelval(debugConfigs.validDiscoveryChannelVal);
    int size_discoverychannelmhzval = debugConfigs.discoveryChannelMhzVal.size();
    if (0 < size_discoverychannelmhzval)
    {
        for (int index = 0; index < size_discoverychannelmhzval; index++)
        {
            msgWifiNanIface.add_discoverychannelmhzval(debugConfigs.discoveryChannelMhzVal[index]);
        }
    }
    msgWifiNanIface.set_validusebeaconsinbandval(debugConfigs.validUseBeaconsInBandVal);
    int size_usebeaconsinbandval = debugConfigs.useBeaconsInBandVal.size();
    if (0 < size_usebeaconsinbandval)
    {
        for (int index = 0; index < size_usebeaconsinbandval; index++)
        {
            msgWifiNanIface.add_usebeaconsinbandval(debugConfigs.useBeaconsInBandVal[index]);
        }
    }
    msgWifiNanIface.set_validusesdfinbandval(debugConfigs.validUseSdfInBandVal);
    int size_usesdfinbandval = debugConfigs.useSdfInBandVal.size();
    if (0 < size_usesdfinbandval)
    {
        for (int index = 0; index < size_usesdfinbandval; index++)
        {
            msgWifiNanIface.add_usesdfinbandval(debugConfigs.useSdfInBandVal[index]);
        }
    }
}

static void NanEnableRequest2Message(const NanEnableRequest& msg1, NanEnableRequest_P& msgWifiNanIface)
{
    int size_operateinband = msg1.operateInBand.size();
    if (0 < size_operateinband)
    {
        for (int index = 0; index < size_operateinband; index++)
        {
            msgWifiNanIface.add_operateinband(msg1.operateInBand[index]);
        }
    }
    msgWifiNanIface.set_hopcountmax(msg1.hopCountMax);
    NanConfigRequest_P* configParams_p = msgWifiNanIface.mutable_configparams();
    NanConfigRequest2Message(msg1.configParams, *configParams_p);
    NanDebugConfig_P* debugConfigs_p = msgWifiNanIface.mutable_debugconfigs();
    NanDebugConfig2Message(msg1.debugConfigs, *debugConfigs_p);
}

bool WifiNanIfaceSerializeEnableRequestReq(char16_t cmdId, const NanEnableRequest& msg1, const NanConfigRequestSupplemental& msg2, vector<uint8_t>& payload)
{
    EnableRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanEnableRequest_P* msg1_p = msgWifiNanIface.mutable_msg1();
    NanEnableRequest2Message(msg1, *msg1_p);
    NanConfigRequestSupplemental_P* msg2_p = msgWifiNanIface.mutable_msg2();
    NanConfigRequestSupplemental2Message(msg2, *msg2_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEnableRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeGetCapabilitiesRequestReq(char16_t cmdId, vector<uint8_t>& payload)
{
    GetCapabilitiesRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeGetCapabilitiesRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanDataPathSecurityConfig2Message(const NanDataPathSecurityConfig& securityConfig, NanDataPathSecurityConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_securitytype((NanDataPathSecurityType_P) securityConfig.securityType);
    msgWifiNanIface.set_ciphertype((NanCipherSuiteType_P) securityConfig.cipherType);
    string pmk;
    Array2String(securityConfig.pmk, pmk);
    msgWifiNanIface.set_pmk(pmk);
    string passphrase;
    Vector2String(securityConfig.passphrase, passphrase);
    msgWifiNanIface.set_passphrase(passphrase);
    string scid;
    Array2String(securityConfig.scid, scid);
    msgWifiNanIface.set_scid(scid);
}

static void NanInitiateDataPathRequest2Message(const NanInitiateDataPathRequest& msg, NanInitiateDataPathRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_peerid(msg.peerId);
    string peerDiscMacAddr;
    Array2String(msg.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_channelrequesttype((NanDataPathChannelCfg_P) msg.channelRequestType);
    msgWifiNanIface.set_channel(msg.channel);
    msgWifiNanIface.set_ifacename(msg.ifaceName);
    NanDataPathSecurityConfig_P* securityConfig_p = msgWifiNanIface.mutable_securityconfig();
    NanDataPathSecurityConfig2Message(msg.securityConfig, *securityConfig_p);
    string appInfo;
    Vector2String(msg.appInfo, appInfo);
    msgWifiNanIface.set_appinfo(appInfo);
    string serviceNameOutOfBand;
    Vector2String(msg.serviceNameOutOfBand, serviceNameOutOfBand);
    msgWifiNanIface.set_servicenameoutofband(serviceNameOutOfBand);
    msgWifiNanIface.set_discoverysessionid(msg.discoverySessionId);
}

bool WifiNanIfaceSerializeInitiateDataPathRequestReq(char16_t cmdId, const NanInitiateDataPathRequest& msg, vector<uint8_t>& payload)
{
    InitiateDataPathRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanInitiateDataPathRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanInitiateDataPathRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeInitiateDataPathRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeRegisterEventCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanRespondToDataPathIndicationRequest2Message(const NanRespondToDataPathIndicationRequest& msg, NanRespondToDataPathIndicationRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_acceptrequest(msg.acceptRequest);
    msgWifiNanIface.set_ndpinstanceid(msg.ndpInstanceId);
    msgWifiNanIface.set_ifacename(msg.ifaceName);
    NanDataPathSecurityConfig_P* securityConfig_p = msgWifiNanIface.mutable_securityconfig();
    NanDataPathSecurityConfig2Message(msg.securityConfig, *securityConfig_p);
    string appInfo;
    Vector2String(msg.appInfo, appInfo);
    msgWifiNanIface.set_appinfo(appInfo);
    string serviceNameOutOfBand;
    Vector2String(msg.serviceNameOutOfBand, serviceNameOutOfBand);
    msgWifiNanIface.set_servicenameoutofband(serviceNameOutOfBand);
    msgWifiNanIface.set_discoverysessionid(msg.discoverySessionId);
}

bool WifiNanIfaceSerializeRespondToDataPathIndicationRequestReq(char16_t cmdId, const NanRespondToDataPathIndicationRequest& msg, vector<uint8_t>& payload)
{
    RespondToDataPathIndicationRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanRespondToDataPathIndicationRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanRespondToDataPathIndicationRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeRespondToDataPathIndicationRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanDiscoveryCommonConfig2Message(const NanDiscoveryCommonConfig& baseConfigs, NanDiscoveryCommonConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_sessionid(baseConfigs.sessionId);
    msgWifiNanIface.set_ttlsec(baseConfigs.ttlSec);
    msgWifiNanIface.set_discoverywindowperiod(baseConfigs.discoveryWindowPeriod);
    msgWifiNanIface.set_discoverycount(baseConfigs.discoveryCount);
    string serviceName;
    Vector2String(baseConfigs.serviceName, serviceName);
    msgWifiNanIface.set_servicename(serviceName);
    msgWifiNanIface.set_discoverymatchindicator((NanMatchAlg_P) baseConfigs.discoveryMatchIndicator);
    string serviceSpecificInfo;
    Vector2String(baseConfigs.serviceSpecificInfo, serviceSpecificInfo);
    msgWifiNanIface.set_servicespecificinfo(serviceSpecificInfo);
    string extendedServiceSpecificInfo;
    Vector2String(baseConfigs.extendedServiceSpecificInfo, extendedServiceSpecificInfo);
    msgWifiNanIface.set_extendedservicespecificinfo(extendedServiceSpecificInfo);
    string rxMatchFilter;
    Vector2String(baseConfigs.rxMatchFilter, rxMatchFilter);
    msgWifiNanIface.set_rxmatchfilter(rxMatchFilter);
    string txMatchFilter;
    Vector2String(baseConfigs.txMatchFilter, txMatchFilter);
    msgWifiNanIface.set_txmatchfilter(txMatchFilter);
    msgWifiNanIface.set_userssithreshold(baseConfigs.useRssiThreshold);
    msgWifiNanIface.set_disablediscoveryterminationindication(baseConfigs.disableDiscoveryTerminationIndication);
    msgWifiNanIface.set_disablematchexpirationindication(baseConfigs.disableMatchExpirationIndication);
    msgWifiNanIface.set_disablefollowupreceivedindication(baseConfigs.disableFollowupReceivedIndication);
    NanDataPathSecurityConfig_P* securityConfig_p = msgWifiNanIface.mutable_securityconfig();
    NanDataPathSecurityConfig2Message(baseConfigs.securityConfig, *securityConfig_p);
    msgWifiNanIface.set_rangingrequired(baseConfigs.rangingRequired);
    msgWifiNanIface.set_rangingintervalms(baseConfigs.rangingIntervalMs);
    msgWifiNanIface.set_configrangingindications(baseConfigs.configRangingIndications);
    msgWifiNanIface.set_distanceingresscm(baseConfigs.distanceIngressCm);
    msgWifiNanIface.set_distanceegresscm(baseConfigs.distanceEgressCm);
    msgWifiNanIface.set_enablesessionsuspendability(baseConfigs.enableSessionSuspendability);
}

static void NanPairingConfig2Message(const NanPairingConfig& pairingConfig, NanPairingConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_enablepairingsetup(pairingConfig.enablePairingSetup);
    msgWifiNanIface.set_enablepairingcache(pairingConfig.enablePairingCache);
    msgWifiNanIface.set_enablepairingverification(pairingConfig.enablePairingVerification);
    msgWifiNanIface.set_supportedbootstrappingmethods(pairingConfig.supportedBootstrappingMethods);
}

static void NanPublishRequest2Message(const NanPublishRequest& msg, NanPublishRequest_P& msgWifiNanIface)
{
    NanDiscoveryCommonConfig_P* baseConfigs_p = msgWifiNanIface.mutable_baseconfigs();
    NanDiscoveryCommonConfig2Message(msg.baseConfigs, *baseConfigs_p);
    msgWifiNanIface.set_publishtype((NanPublishType_P) msg.publishType);
    msgWifiNanIface.set_txtype((NanTxType_P) msg.txType);
    msgWifiNanIface.set_autoacceptdatapathrequests(msg.autoAcceptDataPathRequests);
    NanPairingConfig_P* pairingConfig_p = msgWifiNanIface.mutable_pairingconfig();
    NanPairingConfig2Message(msg.pairingConfig, *pairingConfig_p);
    string identityKey;
    Array2String(msg.identityKey, identityKey);
    msgWifiNanIface.set_identitykey(identityKey);
}

bool WifiNanIfaceSerializeStartPublishRequestReq(char16_t cmdId, const NanPublishRequest& msg, vector<uint8_t>& payload)
{
    StartPublishRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanPublishRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanPublishRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeStartPublishRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void MacAddress2Message(const MacAddress& intfAddr, MacAddress_P& msgWifiNanIface)
{
    string data;
    Array2String(intfAddr.data, data);
    msgWifiNanIface.set_data(data);
}

static void NanSubscribeRequest2Message(const NanSubscribeRequest& msg, NanSubscribeRequest_P& msgWifiNanIface)
{
    NanDiscoveryCommonConfig_P* baseConfigs_p = msgWifiNanIface.mutable_baseconfigs();
    NanDiscoveryCommonConfig2Message(msg.baseConfigs, *baseConfigs_p);
    msgWifiNanIface.set_subscribetype((NanSubscribeType_P) msg.subscribeType);
    msgWifiNanIface.set_srftype((NanSrfType_P) msg.srfType);
    msgWifiNanIface.set_srfrespondifinaddressset(msg.srfRespondIfInAddressSet);
    msgWifiNanIface.set_shouldusesrf(msg.shouldUseSrf);
    msgWifiNanIface.set_isssirequiredformatch(msg.isSsiRequiredForMatch);
    int size_intfaddr = msg.intfAddr.size();
    if (0 < size_intfaddr)
    {
        for (int index = 0; index < size_intfaddr; index++)
        {
            MacAddress_P *intfAddr_p = msgWifiNanIface.add_intfaddr();
            MacAddress2Message(msg.intfAddr[index], *intfAddr_p);
        }
    }
    NanPairingConfig_P* pairingConfig_p = msgWifiNanIface.mutable_pairingconfig();
    NanPairingConfig2Message(msg.pairingConfig, *pairingConfig_p);
    string identityKey;
    Array2String(msg.identityKey, identityKey);
    msgWifiNanIface.set_identitykey(identityKey);
}

bool WifiNanIfaceSerializeStartSubscribeRequestReq(char16_t cmdId, const NanSubscribeRequest& msg, vector<uint8_t>& payload)
{
    StartSubscribeRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanSubscribeRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanSubscribeRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeStartSubscribeRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeStopPublishRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload)
{
    StopPublishRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeStopPublishRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeStopSubscribeRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload)
{
    StopSubscribeRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeStopSubscribeRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeTerminateDataPathRequestReq(char16_t cmdId, int32_t ndpInstanceId, vector<uint8_t>& payload)
{
    TerminateDataPathRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_ndpinstanceid(ndpInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeTerminateDataPathRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeSuspendRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload)
{
    SuspendRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeSuspendRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeResumeRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload)
{
    ResumeRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeResumeRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanTransmitFollowupRequest2Message(const NanTransmitFollowupRequest& msg, NanTransmitFollowupRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(msg.discoverySessionId);
    msgWifiNanIface.set_peerid(msg.peerId);
    string addr;
    Array2String(msg.addr, addr);
    msgWifiNanIface.set_addr(addr);
    msgWifiNanIface.set_ishighpriority(msg.isHighPriority);
    msgWifiNanIface.set_shouldusediscoverywindow(msg.shouldUseDiscoveryWindow);
    string serviceSpecificInfo;
    Vector2String(msg.serviceSpecificInfo, serviceSpecificInfo);
    msgWifiNanIface.set_servicespecificinfo(serviceSpecificInfo);
    string extendedServiceSpecificInfo;
    Vector2String(msg.extendedServiceSpecificInfo, extendedServiceSpecificInfo);
    msgWifiNanIface.set_extendedservicespecificinfo(extendedServiceSpecificInfo);
    msgWifiNanIface.set_disablefollowupresultindication(msg.disableFollowupResultIndication);
}

bool WifiNanIfaceSerializeTransmitFollowupRequestReq(char16_t cmdId, const NanTransmitFollowupRequest& msg, vector<uint8_t>& payload)
{
    TransmitFollowupRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanTransmitFollowupRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanTransmitFollowupRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeTransmitFollowupRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanPairingSecurityConfig2Message(const NanPairingSecurityConfig& securityConfig, NanPairingSecurityConfig_P& msgWifiNanIface)
{
    msgWifiNanIface.set_securitytype((NanPairingSecurityType_P) securityConfig.securityType);
    string pmk;
    Array2String(securityConfig.pmk, pmk);
    msgWifiNanIface.set_pmk(pmk);
    string passphrase;
    Vector2String(securityConfig.passphrase, passphrase);
    msgWifiNanIface.set_passphrase(passphrase);
    msgWifiNanIface.set_akm((NanPairingAkm_P) securityConfig.akm);
    msgWifiNanIface.set_ciphertype((NanCipherSuiteType_P) securityConfig.cipherType);
}

static void NanPairingRequest2Message(const NanPairingRequest& msg, NanPairingRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_peerid(msg.peerId);
    string peerDiscMacAddr;
    Array2String(msg.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_requesttype((NanPairingRequestType_P) msg.requestType);
    msgWifiNanIface.set_enablepairingcache(msg.enablePairingCache);
    string pairingIdentityKey;
    Array2String(msg.pairingIdentityKey, pairingIdentityKey);
    msgWifiNanIface.set_pairingidentitykey(pairingIdentityKey);
    NanPairingSecurityConfig_P* securityConfig_p = msgWifiNanIface.mutable_securityconfig();
    NanPairingSecurityConfig2Message(msg.securityConfig, *securityConfig_p);
}

bool WifiNanIfaceSerializeInitiatePairingRequestReq(char16_t cmdId, const NanPairingRequest& msg, vector<uint8_t>& payload)
{
    InitiatePairingRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanPairingRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanPairingRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeInitiatePairingRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanRespondToPairingIndicationRequest2Message(const NanRespondToPairingIndicationRequest& msg, NanRespondToPairingIndicationRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_acceptrequest(msg.acceptRequest);
    msgWifiNanIface.set_pairinginstanceid(msg.pairingInstanceId);
    msgWifiNanIface.set_requesttype((NanPairingRequestType_P) msg.requestType);
    msgWifiNanIface.set_enablepairingcache(msg.enablePairingCache);
    string pairingIdentityKey;
    Array2String(msg.pairingIdentityKey, pairingIdentityKey);
    msgWifiNanIface.set_pairingidentitykey(pairingIdentityKey);
    NanPairingSecurityConfig_P* securityConfig_p = msgWifiNanIface.mutable_securityconfig();
    NanPairingSecurityConfig2Message(msg.securityConfig, *securityConfig_p);
}

bool WifiNanIfaceSerializeRespondToPairingIndicationRequestReq(char16_t cmdId, const NanRespondToPairingIndicationRequest& msg, vector<uint8_t>& payload)
{
    RespondToPairingIndicationRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanRespondToPairingIndicationRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanRespondToPairingIndicationRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeRespondToPairingIndicationRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanBootstrappingRequest2Message(const NanBootstrappingRequest& msg, NanBootstrappingRequest_P& msgWifiNanIface)
{
    msgWifiNanIface.set_peerid(msg.peerId);
    string peerDiscMacAddr;
    Array2String(msg.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_requestbootstrappingmethod((NanBootstrappingMethod_P) msg.requestBootstrappingMethod);
    string cookie;
    Vector2String(msg.cookie, cookie);
    msgWifiNanIface.set_cookie(cookie);
}

bool WifiNanIfaceSerializeInitiateBootstrappingRequestReq(char16_t cmdId, const NanBootstrappingRequest& msg, vector<uint8_t>& payload)
{
    InitiateBootstrappingRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanBootstrappingRequest_P* msg_p = msgWifiNanIface.mutable_msg();
    NanBootstrappingRequest2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeInitiateBootstrappingRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanBootstrappingResponse2Message(const NanBootstrappingResponse& msg, NanBootstrappingResponse_P& msgWifiNanIface)
{
    msgWifiNanIface.set_bootstrappinginstanceid(msg.bootstrappingInstanceId);
    msgWifiNanIface.set_acceptrequest(msg.acceptRequest);
}

bool WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestReq(char16_t cmdId, const NanBootstrappingResponse& msg, vector<uint8_t>& payload)
{
    RespondToBootstrappingIndicationRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    NanBootstrappingResponse_P* msg_p = msgWifiNanIface.mutable_msg();
    NanBootstrappingResponse2Message(msg, *msg_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiNanIfaceSerializeTerminatePairingRequestReq(char16_t cmdId, int32_t pairingInstanceId, vector<uint8_t>& payload)
{
    TerminatePairingRequestReq msgWifiNanIface;
    msgWifiNanIface.set_cmdid(cmdId);
    msgWifiNanIface.set_pairinginstanceid(pairingInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeTerminatePairingRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NanClusterEventInd2Message(const NanClusterEventInd& event, NanClusterEventInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_eventtype((NanClusterEventType_P) event.eventType);
    string addr;
    Array2String(event.addr, addr);
    msgWifiNanIface.set_addr(addr);
}

bool WifiNanIfaceSerializeEventClusterEventInd(const NanClusterEventInd& event, vector<uint8_t>& payload)
{
    EventClusterEventInd msgWifiNanIface;
    NanClusterEventInd_P* event_p = msgWifiNanIface.mutable_event();
    NanClusterEventInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanStatus2Message(const NanStatus& status, NanStatus_P& msgWifiNanIface)
{
    msgWifiNanIface.set_status((NanStatusCode_P) status.status);
    msgWifiNanIface.set_description(status.description);
}

static void NanDataPathChannelInfo2Message(const NanDataPathChannelInfo& channelInfo, NanDataPathChannelInfo_P& msgWifiNanIface)
{
    msgWifiNanIface.set_channelfreq(channelInfo.channelFreq);
    msgWifiNanIface.set_channelbandwidth((WifiChannelWidthInMhz_P) channelInfo.channelBandwidth);
    msgWifiNanIface.set_numspatialstreams(channelInfo.numSpatialStreams);
}

static void NanDataPathConfirmInd2Message(const NanDataPathConfirmInd& event, NanDataPathConfirmInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_ndpinstanceid(event.ndpInstanceId);
    msgWifiNanIface.set_datapathsetupsuccess(event.dataPathSetupSuccess);
    string peerNdiMacAddr;
    Array2String(event.peerNdiMacAddr, peerNdiMacAddr);
    msgWifiNanIface.set_peerndimacaddr(peerNdiMacAddr);
    string appInfo;
    Vector2String(event.appInfo, appInfo);
    msgWifiNanIface.set_appinfo(appInfo);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(event.status, *status_p);
    int size_channelinfo = event.channelInfo.size();
    if (0 < size_channelinfo)
    {
        for (int index = 0; index < size_channelinfo; index++)
        {
            NanDataPathChannelInfo_P *channelInfo_p = msgWifiNanIface.add_channelinfo();
            NanDataPathChannelInfo2Message(event.channelInfo[index], *channelInfo_p);
        }
    }
}

bool WifiNanIfaceSerializeEventDataPathConfirmInd(const NanDataPathConfirmInd& event, vector<uint8_t>& payload)
{
    EventDataPathConfirmInd msgWifiNanIface;
    NanDataPathConfirmInd_P* event_p = msgWifiNanIface.mutable_event();
    NanDataPathConfirmInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanDataPathRequestInd2Message(const NanDataPathRequestInd& event, NanDataPathRequestInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(event.discoverySessionId);
    string peerDiscMacAddr;
    Array2String(event.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_ndpinstanceid(event.ndpInstanceId);
    msgWifiNanIface.set_securityrequired(event.securityRequired);
    string appInfo;
    Vector2String(event.appInfo, appInfo);
    msgWifiNanIface.set_appinfo(appInfo);
}

bool WifiNanIfaceSerializeEventDataPathRequestInd(const NanDataPathRequestInd& event, vector<uint8_t>& payload)
{
    EventDataPathRequestInd msgWifiNanIface;
    NanDataPathRequestInd_P* event_p = msgWifiNanIface.mutable_event();
    NanDataPathRequestInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanDataPathScheduleUpdateInd2Message(const NanDataPathScheduleUpdateInd& event, NanDataPathScheduleUpdateInd_P& msgWifiNanIface)
{
    string peerDiscoveryAddress;
    Array2String(event.peerDiscoveryAddress, peerDiscoveryAddress);
    msgWifiNanIface.set_peerdiscoveryaddress(peerDiscoveryAddress);
    int size_channelinfo = event.channelInfo.size();
    if (0 < size_channelinfo)
    {
        for (int index = 0; index < size_channelinfo; index++)
        {
            NanDataPathChannelInfo_P *channelInfo_p = msgWifiNanIface.add_channelinfo();
            NanDataPathChannelInfo2Message(event.channelInfo[index], *channelInfo_p);
        }
    }
    int size_ndpinstanceids = event.ndpInstanceIds.size();
    if (0 < size_ndpinstanceids)
    {
        for (int index = 0; index < size_ndpinstanceids; index++)
        {
            msgWifiNanIface.add_ndpinstanceids(event.ndpInstanceIds[index]);
        }
    }
}

bool WifiNanIfaceSerializeEventDataPathScheduleUpdateInd(const NanDataPathScheduleUpdateInd& event, vector<uint8_t>& payload)
{
    EventDataPathScheduleUpdateInd msgWifiNanIface;
    NanDataPathScheduleUpdateInd_P* event_p = msgWifiNanIface.mutable_event();
    NanDataPathScheduleUpdateInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventDataPathTerminatedInd(int32_t ndpInstanceId, vector<uint8_t>& payload)
{
    EventDataPathTerminatedInd msgWifiNanIface;
    msgWifiNanIface.set_ndpinstanceid(ndpInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventDisabledInd(const NanStatus& status, vector<uint8_t>& payload)
{
    EventDisabledInd msgWifiNanIface;
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanFollowupReceivedInd2Message(const NanFollowupReceivedInd& event, NanFollowupReceivedInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(event.discoverySessionId);
    msgWifiNanIface.set_peerid(event.peerId);
    string addr;
    Array2String(event.addr, addr);
    msgWifiNanIface.set_addr(addr);
    msgWifiNanIface.set_receivedinfaw(event.receivedInFaw);
    string serviceSpecificInfo;
    Vector2String(event.serviceSpecificInfo, serviceSpecificInfo);
    msgWifiNanIface.set_servicespecificinfo(serviceSpecificInfo);
    string extendedServiceSpecificInfo;
    Vector2String(event.extendedServiceSpecificInfo, extendedServiceSpecificInfo);
    msgWifiNanIface.set_extendedservicespecificinfo(extendedServiceSpecificInfo);
}

bool WifiNanIfaceSerializeEventFollowupReceivedInd(const NanFollowupReceivedInd& event, vector<uint8_t>& payload)
{
    EventFollowupReceivedInd msgWifiNanIface;
    NanFollowupReceivedInd_P* event_p = msgWifiNanIface.mutable_event();
    NanFollowupReceivedInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanIdentityResolutionAttribute2Message(const NanIdentityResolutionAttribute& peerNira, NanIdentityResolutionAttribute_P& msgWifiNanIface)
{
    string nonce;
    Array2String(peerNira.nonce, nonce);
    msgWifiNanIface.set_nonce(nonce);
    string tag;
    Array2String(peerNira.tag, tag);
    msgWifiNanIface.set_tag(tag);
}

static void NanMatchInd2Message(const NanMatchInd& event, NanMatchInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(event.discoverySessionId);
    msgWifiNanIface.set_peerid(event.peerId);
    string addr;
    Array2String(event.addr, addr);
    msgWifiNanIface.set_addr(addr);
    string serviceSpecificInfo;
    Vector2String(event.serviceSpecificInfo, serviceSpecificInfo);
    msgWifiNanIface.set_servicespecificinfo(serviceSpecificInfo);
    string extendedServiceSpecificInfo;
    Vector2String(event.extendedServiceSpecificInfo, extendedServiceSpecificInfo);
    msgWifiNanIface.set_extendedservicespecificinfo(extendedServiceSpecificInfo);
    string matchFilter;
    Vector2String(event.matchFilter, matchFilter);
    msgWifiNanIface.set_matchfilter(matchFilter);
    msgWifiNanIface.set_matchoccurredinbeaconflag(event.matchOccurredInBeaconFlag);
    msgWifiNanIface.set_outofresourceflag(event.outOfResourceFlag);
    msgWifiNanIface.set_rssivalue(event.rssiValue);
    msgWifiNanIface.set_peerciphertype((NanCipherSuiteType_P) event.peerCipherType);
    msgWifiNanIface.set_peerrequiressecurityenabledinndp(event.peerRequiresSecurityEnabledInNdp);
    msgWifiNanIface.set_peerrequiresranging(event.peerRequiresRanging);
    msgWifiNanIface.set_rangingmeasurementinmm(event.rangingMeasurementInMm);
    msgWifiNanIface.set_rangingindicationtype(event.rangingIndicationType);
    string scid;
    Vector2String(event.scid, scid);
    msgWifiNanIface.set_scid(scid);
    NanPairingConfig_P* peerPairingConfig_p = msgWifiNanIface.mutable_peerpairingconfig();
    NanPairingConfig2Message(event.peerPairingConfig, *peerPairingConfig_p);
    NanIdentityResolutionAttribute_P* peerNira_p = msgWifiNanIface.mutable_peernira();
    NanIdentityResolutionAttribute2Message(event.peerNira, *peerNira_p);
}

bool WifiNanIfaceSerializeEventMatchInd(const NanMatchInd& event, vector<uint8_t>& payload)
{
    EventMatchInd msgWifiNanIface;
    NanMatchInd_P* event_p = msgWifiNanIface.mutable_event();
    NanMatchInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventMatchExpiredInd(int8_t discoverySessionId, int32_t peerId, vector<uint8_t>& payload)
{
    EventMatchExpiredInd msgWifiNanIface;
    msgWifiNanIface.set_discoverysessionid(discoverySessionId);
    msgWifiNanIface.set_peerid(peerId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventPublishTerminatedInd(int8_t sessionId, const NanStatus& status, vector<uint8_t>& payload)
{
    EventPublishTerminatedInd msgWifiNanIface;
    msgWifiNanIface.set_sessionid(sessionId);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventSubscribeTerminatedInd(int8_t sessionId, const NanStatus& status, vector<uint8_t>& payload)
{
    EventSubscribeTerminatedInd msgWifiNanIface;
    msgWifiNanIface.set_sessionid(sessionId);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeEventTransmitFollowupInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    EventTransmitFollowupInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanSuspensionModeChangeInd2Message(const NanSuspensionModeChangeInd& event, NanSuspensionModeChangeInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_issuspended(event.isSuspended);
}

bool WifiNanIfaceSerializeEventSuspensionModeChangedInd(const NanSuspensionModeChangeInd& event, vector<uint8_t>& payload)
{
    EventSuspensionModeChangedInd msgWifiNanIface;
    NanSuspensionModeChangeInd_P* event_p = msgWifiNanIface.mutable_event();
    NanSuspensionModeChangeInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanCapabilities2Message(const NanCapabilities& capabilities, NanCapabilities_P& msgWifiNanIface)
{
    msgWifiNanIface.set_maxconcurrentclusters(capabilities.maxConcurrentClusters);
    msgWifiNanIface.set_maxpublishes(capabilities.maxPublishes);
    msgWifiNanIface.set_maxsubscribes(capabilities.maxSubscribes);
    msgWifiNanIface.set_maxservicenamelen(capabilities.maxServiceNameLen);
    msgWifiNanIface.set_maxmatchfilterlen(capabilities.maxMatchFilterLen);
    msgWifiNanIface.set_maxtotalmatchfilterlen(capabilities.maxTotalMatchFilterLen);
    msgWifiNanIface.set_maxservicespecificinfolen(capabilities.maxServiceSpecificInfoLen);
    msgWifiNanIface.set_maxextendedservicespecificinfolen(capabilities.maxExtendedServiceSpecificInfoLen);
    msgWifiNanIface.set_maxndiinterfaces(capabilities.maxNdiInterfaces);
    msgWifiNanIface.set_maxndpsessions(capabilities.maxNdpSessions);
    msgWifiNanIface.set_maxappinfolen(capabilities.maxAppInfoLen);
    msgWifiNanIface.set_maxqueuedtransmitfollowupmsgs(capabilities.maxQueuedTransmitFollowupMsgs);
    msgWifiNanIface.set_maxsubscribeinterfaceaddresses(capabilities.maxSubscribeInterfaceAddresses);
    msgWifiNanIface.set_supportedciphersuites(capabilities.supportedCipherSuites);
    msgWifiNanIface.set_instantcommunicationmodesupportflag(capabilities.instantCommunicationModeSupportFlag);
    msgWifiNanIface.set_supports6g(capabilities.supports6g);
    msgWifiNanIface.set_supportshe(capabilities.supportsHe);
    msgWifiNanIface.set_supportspairing(capabilities.supportsPairing);
    msgWifiNanIface.set_supportssetclusterid(capabilities.supportsSetClusterId);
    msgWifiNanIface.set_supportssuspension(capabilities.supportsSuspension);
}

bool WifiNanIfaceSerializeNotifyCapabilitiesResponseInd(char16_t id, const NanStatus& status, const NanCapabilities& capabilities, vector<uint8_t>& payload)
{
    NotifyCapabilitiesResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    NanCapabilities_P* capabilities_p = msgWifiNanIface.mutable_capabilities();
    NanCapabilities2Message(capabilities, *capabilities_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyConfigResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyConfigResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyCreateDataInterfaceResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyCreateDataInterfaceResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyDeleteDataInterfaceResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyDeleteDataInterfaceResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyDisableResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyDisableResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyEnableResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyEnableResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyInitiateDataPathResponseInd(char16_t id, const NanStatus& status, int32_t ndpInstanceId, vector<uint8_t>& payload)
{
    NotifyInitiateDataPathResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    msgWifiNanIface.set_ndpinstanceid(ndpInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyRespondToDataPathIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyRespondToDataPathIndicationResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyStartPublishResponseInd(char16_t id, const NanStatus& status, int8_t sessionId, vector<uint8_t>& payload)
{
    NotifyStartPublishResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyStartSubscribeResponseInd(char16_t id, const NanStatus& status, int8_t sessionId, vector<uint8_t>& payload)
{
    NotifyStartSubscribeResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    msgWifiNanIface.set_sessionid(sessionId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyStopPublishResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyStopPublishResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyStopSubscribeResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyStopSubscribeResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyTerminateDataPathResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyTerminateDataPathResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifySuspendResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifySuspendResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyResumeResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyResumeResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyTransmitFollowupResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyTransmitFollowupResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanPairingRequestInd2Message(const NanPairingRequestInd& event, NanPairingRequestInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(event.discoverySessionId);
    msgWifiNanIface.set_peerid(event.peerId);
    string peerDiscMacAddr;
    Array2String(event.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_pairinginstanceid(event.pairingInstanceId);
    msgWifiNanIface.set_requesttype((NanPairingRequestType_P) event.requestType);
    msgWifiNanIface.set_enablepairingcache(event.enablePairingCache);
    NanIdentityResolutionAttribute_P* peerNira_p = msgWifiNanIface.mutable_peernira();
    NanIdentityResolutionAttribute2Message(event.peerNira, *peerNira_p);
}

bool WifiNanIfaceSerializeEventPairingRequestInd(const NanPairingRequestInd& event, vector<uint8_t>& payload)
{
    EventPairingRequestInd msgWifiNanIface;
    NanPairingRequestInd_P* event_p = msgWifiNanIface.mutable_event();
    NanPairingRequestInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NpkSecurityAssociation2Message(const NpkSecurityAssociation& npksa, NpkSecurityAssociation_P& msgWifiNanIface)
{
    string peerNanIdentityKey;
    Array2String(npksa.peerNanIdentityKey, peerNanIdentityKey);
    msgWifiNanIface.set_peernanidentitykey(peerNanIdentityKey);
    string localNanIdentityKey;
    Array2String(npksa.localNanIdentityKey, localNanIdentityKey);
    msgWifiNanIface.set_localnanidentitykey(localNanIdentityKey);
    string npk;
    Array2String(npksa.npk, npk);
    msgWifiNanIface.set_npk(npk);
    msgWifiNanIface.set_akm((NanPairingAkm_P) npksa.akm);
    msgWifiNanIface.set_ciphertype((NanCipherSuiteType_P) npksa.cipherType);
}

static void NanPairingConfirmInd2Message(const NanPairingConfirmInd& event, NanPairingConfirmInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_pairinginstanceid(event.pairingInstanceId);
    msgWifiNanIface.set_pairingsuccess(event.pairingSuccess);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(event.status, *status_p);
    msgWifiNanIface.set_requesttype((NanPairingRequestType_P) event.requestType);
    msgWifiNanIface.set_enablepairingcache(event.enablePairingCache);
    NpkSecurityAssociation_P* npksa_p = msgWifiNanIface.mutable_npksa();
    NpkSecurityAssociation2Message(event.npksa, *npksa_p);
}

bool WifiNanIfaceSerializeEventPairingConfirmInd(const NanPairingConfirmInd& event, vector<uint8_t>& payload)
{
    EventPairingConfirmInd msgWifiNanIface;
    NanPairingConfirmInd_P* event_p = msgWifiNanIface.mutable_event();
    NanPairingConfirmInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyInitiatePairingResponseInd(char16_t id, const NanStatus& status, int32_t pairingInstanceId, vector<uint8_t>& payload)
{
    NotifyInitiatePairingResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    msgWifiNanIface.set_pairinginstanceid(pairingInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyRespondToPairingIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyRespondToPairingIndicationResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanBootstrappingRequestInd2Message(const NanBootstrappingRequestInd& event, NanBootstrappingRequestInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_discoverysessionid(event.discoverySessionId);
    msgWifiNanIface.set_peerid(event.peerId);
    string peerDiscMacAddr;
    Array2String(event.peerDiscMacAddr, peerDiscMacAddr);
    msgWifiNanIface.set_peerdiscmacaddr(peerDiscMacAddr);
    msgWifiNanIface.set_bootstrappinginstanceid(event.bootstrappingInstanceId);
    msgWifiNanIface.set_requestbootstrappingmethod((NanBootstrappingMethod_P) event.requestBootstrappingMethod);
}

bool WifiNanIfaceSerializeEventBootstrappingRequestInd(const NanBootstrappingRequestInd& event, vector<uint8_t>& payload)
{
    EventBootstrappingRequestInd msgWifiNanIface;
    NanBootstrappingRequestInd_P* event_p = msgWifiNanIface.mutable_event();
    NanBootstrappingRequestInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

static void NanBootstrappingConfirmInd2Message(const NanBootstrappingConfirmInd& event, NanBootstrappingConfirmInd_P& msgWifiNanIface)
{
    msgWifiNanIface.set_bootstrappinginstanceid(event.bootstrappingInstanceId);
    msgWifiNanIface.set_responsecode((NanBootstrappingResponseCode_P) event.responseCode);
    NanStatus_P* reasonCode_p = msgWifiNanIface.mutable_reasoncode();
    NanStatus2Message(event.reasonCode, *reasonCode_p);
    msgWifiNanIface.set_comebackdelay(event.comeBackDelay);
    string cookie;
    Vector2String(event.cookie, cookie);
    msgWifiNanIface.set_cookie(cookie);
}

bool WifiNanIfaceSerializeEventBootstrappingConfirmInd(const NanBootstrappingConfirmInd& event, vector<uint8_t>& payload)
{
    EventBootstrappingConfirmInd msgWifiNanIface;
    NanBootstrappingConfirmInd_P* event_p = msgWifiNanIface.mutable_event();
    NanBootstrappingConfirmInd2Message(event, *event_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyInitiateBootstrappingResponseInd(char16_t id, const NanStatus& status, int32_t bootstrappingInstanceId, vector<uint8_t>& payload)
{
    NotifyInitiateBootstrappingResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    msgWifiNanIface.set_bootstrappinginstanceid(bootstrappingInstanceId);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyRespondToBootstrappingIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyRespondToBootstrappingIndicationResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceSerializeNotifyTerminatePairingResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload)
{
    NotifyTerminatePairingResponseInd msgWifiNanIface;
    msgWifiNanIface.set_id(id);
    NanStatus_P* status_p = msgWifiNanIface.mutable_status();
    NanStatus2Message(status, *status_p);
    return ProtoMessageSerialize(&msgWifiNanIface, payload);
}

bool WifiNanIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgWifiNanIface, HalStatusParam& status)
{
    status.status = msgWifiNanIface.status();
    status.info = msgWifiNanIface.info();
}

bool WifiNanIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmNanIfaceParam& param)
{
    GetNameCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

static void Message2NanBandSpecificConfig(const NanBandSpecificConfig_P& msgWifiNanIface, NanBandSpecificConfig& bandSpecificConfig)
{
    bandSpecificConfig.rssiClose = msgWifiNanIface.rssiclose();
    bandSpecificConfig.rssiMiddle = msgWifiNanIface.rssimiddle();
    bandSpecificConfig.rssiCloseProximity = msgWifiNanIface.rssicloseproximity();
    bandSpecificConfig.dwellTimeMs = msgWifiNanIface.dwelltimems();
    bandSpecificConfig.scanPeriodSec = msgWifiNanIface.scanperiodsec();
    bandSpecificConfig.validDiscoveryWindowIntervalVal = msgWifiNanIface.validdiscoverywindowintervalval();
    bandSpecificConfig.discoveryWindowIntervalVal = msgWifiNanIface.discoverywindowintervalval();
}

static void Message2NanConfigRequest(const NanConfigRequest_P& msgWifiNanIface, NanConfigRequest& msg1)
{
    msg1.masterPref = msgWifiNanIface.masterpref();
    msg1.disableDiscoveryAddressChangeIndication = msgWifiNanIface.disablediscoveryaddresschangeindication();
    msg1.disableStartedClusterIndication = msgWifiNanIface.disablestartedclusterindication();
    msg1.disableJoinedClusterIndication = msgWifiNanIface.disablejoinedclusterindication();
    msg1.includePublishServiceIdsInBeacon = msgWifiNanIface.includepublishserviceidsinbeacon();
    msg1.numberOfPublishServiceIdsInBeacon = msgWifiNanIface.numberofpublishserviceidsinbeacon();
    msg1.includeSubscribeServiceIdsInBeacon = msgWifiNanIface.includesubscribeserviceidsinbeacon();
    msg1.numberOfSubscribeServiceIdsInBeacon = msgWifiNanIface.numberofsubscribeserviceidsinbeacon();
    msg1.rssiWindowSize = msgWifiNanIface.rssiwindowsize();
    msg1.macAddressRandomizationIntervalSec = msgWifiNanIface.macaddressrandomizationintervalsec();
    int size_bandspecificconfig = msgWifiNanIface.bandspecificconfig_size();
    if (0 < size_bandspecificconfig)
    {
        for (int index = 0; index < size_bandspecificconfig; index++)
        {
            Message2NanBandSpecificConfig(msgWifiNanIface.bandspecificconfig(index), msg1.bandSpecificConfig[index]);
        }
    }
}

static void Message2NanConfigRequestSupplemental(const NanConfigRequestSupplemental_P& msgWifiNanIface, NanConfigRequestSupplemental& msg2)
{
    msg2.discoveryBeaconIntervalMs = msgWifiNanIface.discoverybeaconintervalms();
    msg2.numberOfSpatialStreamsInDiscovery = msgWifiNanIface.numberofspatialstreamsindiscovery();
    msg2.enableDiscoveryWindowEarlyTermination = msgWifiNanIface.enablediscoverywindowearlytermination();
    msg2.enableRanging = msgWifiNanIface.enableranging();
    msg2.enableInstantCommunicationMode = msgWifiNanIface.enableinstantcommunicationmode();
    msg2.instantModeChannel = msgWifiNanIface.instantmodechannel();
    msg2.clusterId = msgWifiNanIface.clusterid();
}

bool WifiNanIfaceParseConfigRequestReq(const uint8_t* data, size_t length, ConfigRequestReqNanIfaceParam& param)
{
    ConfigRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanConfigRequest(msg.msg1(), param.msg1);
        Message2NanConfigRequestSupplemental(msg.msg2(), param.msg2);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseConfigRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseCreateDataInterfaceRequestReq(const uint8_t* data, size_t length, CreateDataInterfaceRequestReqNanIfaceParam& param)
{
    CreateDataInterfaceRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.ifaceName = msg.ifacename();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseCreateDataInterfaceRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseDeleteDataInterfaceRequestReq(const uint8_t* data, size_t length, DeleteDataInterfaceRequestReqNanIfaceParam& param)
{
    DeleteDataInterfaceRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.ifaceName = msg.ifacename();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseDeleteDataInterfaceRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseDisableRequestReq(const uint8_t* data, size_t length, char16_t& param)
{
    DisableRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseDisableRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanDebugConfig(const NanDebugConfig_P& msgWifiNanIface, NanDebugConfig& debugConfigs)
{
    debugConfigs.validClusterIdVals = msgWifiNanIface.validclusteridvals();
    debugConfigs.clusterIdBottomRangeVal = msgWifiNanIface.clusteridbottomrangeval();
    debugConfigs.clusterIdTopRangeVal = msgWifiNanIface.clusteridtoprangeval();
    debugConfigs.validIntfAddrVal = msgWifiNanIface.validintfaddrval();
    String2Array(msgWifiNanIface.intfaddrval(), debugConfigs.intfAddrVal);
    debugConfigs.validOuiVal = msgWifiNanIface.validouival();
    debugConfigs.ouiVal = msgWifiNanIface.ouival();
    debugConfigs.validRandomFactorForceVal = msgWifiNanIface.validrandomfactorforceval();
    debugConfigs.randomFactorForceVal = msgWifiNanIface.randomfactorforceval();
    debugConfigs.validHopCountForceVal = msgWifiNanIface.validhopcountforceval();
    debugConfigs.hopCountForceVal = msgWifiNanIface.hopcountforceval();
    debugConfigs.validDiscoveryChannelVal = msgWifiNanIface.validdiscoverychannelval();
    int size_discoverychannelmhzval = msgWifiNanIface.discoverychannelmhzval_size();
    if (0 < size_discoverychannelmhzval)
    {
        for (int index = 0; index < size_discoverychannelmhzval; index++)
        {
            debugConfigs.discoveryChannelMhzVal[index] = msgWifiNanIface.discoverychannelmhzval(index);
        }
    }
    debugConfigs.validUseBeaconsInBandVal = msgWifiNanIface.validusebeaconsinbandval();
    int size_usebeaconsinbandval = msgWifiNanIface.usebeaconsinbandval_size();
    if (0 < size_usebeaconsinbandval)
    {
        for (int index = 0; index < size_usebeaconsinbandval; index++)
        {
            debugConfigs.useBeaconsInBandVal[index] = msgWifiNanIface.usebeaconsinbandval(index);
        }
    }
    debugConfigs.validUseSdfInBandVal = msgWifiNanIface.validusesdfinbandval();
    int size_usesdfinbandval = msgWifiNanIface.usesdfinbandval_size();
    if (0 < size_usesdfinbandval)
    {
        for (int index = 0; index < size_usesdfinbandval; index++)
        {
            debugConfigs.useSdfInBandVal[index] = msgWifiNanIface.usesdfinbandval(index);
        }
    }
}

static void Message2NanEnableRequest(const NanEnableRequest_P& msgWifiNanIface, NanEnableRequest& msg1)
{
    int size_operateinband = msgWifiNanIface.operateinband_size();
    if (0 < size_operateinband)
    {
        for (int index = 0; index < size_operateinband; index++)
        {
            msg1.operateInBand[index] = msgWifiNanIface.operateinband(index);
        }
    }
    msg1.hopCountMax = msgWifiNanIface.hopcountmax();
    Message2NanConfigRequest(msgWifiNanIface.configparams(), msg1.configParams);
    Message2NanDebugConfig(msgWifiNanIface.debugconfigs(), msg1.debugConfigs);
}

bool WifiNanIfaceParseEnableRequestReq(const uint8_t* data, size_t length, EnableRequestReqNanIfaceParam& param)
{
    EnableRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanEnableRequest(msg.msg1(), param.msg1);
        Message2NanConfigRequestSupplemental(msg.msg2(), param.msg2);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEnableRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseGetCapabilitiesRequestReq(const uint8_t* data, size_t length, char16_t& param)
{
    GetCapabilitiesRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseGetCapabilitiesRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanDataPathSecurityConfig(const NanDataPathSecurityConfig_P& msgWifiNanIface, NanDataPathSecurityConfig& securityConfig)
{
    securityConfig.securityType = (NanDataPathSecurityType) msgWifiNanIface.securitytype();
    securityConfig.cipherType = (NanCipherSuiteType) msgWifiNanIface.ciphertype();
    String2Array(msgWifiNanIface.pmk(), securityConfig.pmk);
    String2Vector(msgWifiNanIface.passphrase(), securityConfig.passphrase);
    String2Array(msgWifiNanIface.scid(), securityConfig.scid);
}

static void Message2NanInitiateDataPathRequest(const NanInitiateDataPathRequest_P& msgWifiNanIface, NanInitiateDataPathRequest& msg)
{
    msg.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), msg.peerDiscMacAddr);
    msg.channelRequestType = (NanDataPathChannelCfg) msgWifiNanIface.channelrequesttype();
    msg.channel = msgWifiNanIface.channel();
    msg.ifaceName = msgWifiNanIface.ifacename();
    Message2NanDataPathSecurityConfig(msgWifiNanIface.securityconfig(), msg.securityConfig);
    String2Vector(msgWifiNanIface.appinfo(), msg.appInfo);
    String2Vector(msgWifiNanIface.servicenameoutofband(), msg.serviceNameOutOfBand);
    msg.discoverySessionId = msgWifiNanIface.discoverysessionid();
}

bool WifiNanIfaceParseInitiateDataPathRequestReq(const uint8_t* data, size_t length, InitiateDataPathRequestReqNanIfaceParam& param)
{
    InitiateDataPathRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanInitiateDataPathRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseInitiateDataPathRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseRegisterEventCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseRegisterEventCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanRespondToDataPathIndicationRequest(const NanRespondToDataPathIndicationRequest_P& msgWifiNanIface, NanRespondToDataPathIndicationRequest& msg)
{
    msg.acceptRequest = msgWifiNanIface.acceptrequest();
    msg.ndpInstanceId = msgWifiNanIface.ndpinstanceid();
    msg.ifaceName = msgWifiNanIface.ifacename();
    Message2NanDataPathSecurityConfig(msgWifiNanIface.securityconfig(), msg.securityConfig);
    String2Vector(msgWifiNanIface.appinfo(), msg.appInfo);
    String2Vector(msgWifiNanIface.servicenameoutofband(), msg.serviceNameOutOfBand);
    msg.discoverySessionId = msgWifiNanIface.discoverysessionid();
}

bool WifiNanIfaceParseRespondToDataPathIndicationRequestReq(const uint8_t* data, size_t length, RespondToDataPathIndicationRequestReqNanIfaceParam& param)
{
    RespondToDataPathIndicationRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanRespondToDataPathIndicationRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseRespondToDataPathIndicationRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanDiscoveryCommonConfig(const NanDiscoveryCommonConfig_P& msgWifiNanIface, NanDiscoveryCommonConfig& baseConfigs)
{
    baseConfigs.sessionId = msgWifiNanIface.sessionid();
    baseConfigs.ttlSec = msgWifiNanIface.ttlsec();
    baseConfigs.discoveryWindowPeriod = msgWifiNanIface.discoverywindowperiod();
    baseConfigs.discoveryCount = msgWifiNanIface.discoverycount();
    String2Vector(msgWifiNanIface.servicename(), baseConfigs.serviceName);
    baseConfigs.discoveryMatchIndicator = (NanMatchAlg) msgWifiNanIface.discoverymatchindicator();
    String2Vector(msgWifiNanIface.servicespecificinfo(), baseConfigs.serviceSpecificInfo);
    String2Vector(msgWifiNanIface.extendedservicespecificinfo(), baseConfigs.extendedServiceSpecificInfo);
    String2Vector(msgWifiNanIface.rxmatchfilter(), baseConfigs.rxMatchFilter);
    String2Vector(msgWifiNanIface.txmatchfilter(), baseConfigs.txMatchFilter);
    baseConfigs.useRssiThreshold = msgWifiNanIface.userssithreshold();
    baseConfigs.disableDiscoveryTerminationIndication = msgWifiNanIface.disablediscoveryterminationindication();
    baseConfigs.disableMatchExpirationIndication = msgWifiNanIface.disablematchexpirationindication();
    baseConfigs.disableFollowupReceivedIndication = msgWifiNanIface.disablefollowupreceivedindication();
    Message2NanDataPathSecurityConfig(msgWifiNanIface.securityconfig(), baseConfigs.securityConfig);
    baseConfigs.rangingRequired = msgWifiNanIface.rangingrequired();
    baseConfigs.rangingIntervalMs = msgWifiNanIface.rangingintervalms();
    baseConfigs.configRangingIndications = msgWifiNanIface.configrangingindications();
    baseConfigs.distanceIngressCm = msgWifiNanIface.distanceingresscm();
    baseConfigs.distanceEgressCm = msgWifiNanIface.distanceegresscm();
    baseConfigs.enableSessionSuspendability = msgWifiNanIface.enablesessionsuspendability();
}

static void Message2NanPairingConfig(const NanPairingConfig_P& msgWifiNanIface, NanPairingConfig& pairingConfig)
{
    pairingConfig.enablePairingSetup = msgWifiNanIface.enablepairingsetup();
    pairingConfig.enablePairingCache = msgWifiNanIface.enablepairingcache();
    pairingConfig.enablePairingVerification = msgWifiNanIface.enablepairingverification();
    pairingConfig.supportedBootstrappingMethods = msgWifiNanIface.supportedbootstrappingmethods();
}

static void Message2NanPublishRequest(const NanPublishRequest_P& msgWifiNanIface, NanPublishRequest& msg)
{
    Message2NanDiscoveryCommonConfig(msgWifiNanIface.baseconfigs(), msg.baseConfigs);
    msg.publishType = (NanPublishType) msgWifiNanIface.publishtype();
    msg.txType = (NanTxType) msgWifiNanIface.txtype();
    msg.autoAcceptDataPathRequests = msgWifiNanIface.autoacceptdatapathrequests();
    Message2NanPairingConfig(msgWifiNanIface.pairingconfig(), msg.pairingConfig);
    String2Array(msgWifiNanIface.identitykey(), msg.identityKey);
}

bool WifiNanIfaceParseStartPublishRequestReq(const uint8_t* data, size_t length, StartPublishRequestReqNanIfaceParam& param)
{
    StartPublishRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanPublishRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseStartPublishRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2MacAddress(const MacAddress_P& msgWifiNanIface, MacAddress& intfAddr)
{
    String2Array(msgWifiNanIface.data(), intfAddr.data);
}

static void Message2NanSubscribeRequest(const NanSubscribeRequest_P& msgWifiNanIface, NanSubscribeRequest& msg)
{
    Message2NanDiscoveryCommonConfig(msgWifiNanIface.baseconfigs(), msg.baseConfigs);
    msg.subscribeType = (NanSubscribeType) msgWifiNanIface.subscribetype();
    msg.srfType = (NanSrfType) msgWifiNanIface.srftype();
    msg.srfRespondIfInAddressSet = msgWifiNanIface.srfrespondifinaddressset();
    msg.shouldUseSrf = msgWifiNanIface.shouldusesrf();
    msg.isSsiRequiredForMatch = msgWifiNanIface.isssirequiredformatch();
    int size_intfaddr = msgWifiNanIface.intfaddr_size();
    if (0 < size_intfaddr)
    {
        msg.intfAddr.resize(size_intfaddr);
        for (int index = 0; index < size_intfaddr; index++)
        {
            Message2MacAddress(msgWifiNanIface.intfaddr(index), msg.intfAddr[index]);
        }
    }
    Message2NanPairingConfig(msgWifiNanIface.pairingconfig(), msg.pairingConfig);
    String2Array(msgWifiNanIface.identitykey(), msg.identityKey);
}

bool WifiNanIfaceParseStartSubscribeRequestReq(const uint8_t* data, size_t length, StartSubscribeRequestReqNanIfaceParam& param)
{
    StartSubscribeRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanSubscribeRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseStartSubscribeRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseStopPublishRequestReq(const uint8_t* data, size_t length, StopPublishRequestReqNanIfaceParam& param)
{
    StopPublishRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseStopPublishRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseStopSubscribeRequestReq(const uint8_t* data, size_t length, StopSubscribeRequestReqNanIfaceParam& param)
{
    StopSubscribeRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseStopSubscribeRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseTerminateDataPathRequestReq(const uint8_t* data, size_t length, TerminateDataPathRequestReqNanIfaceParam& param)
{
    TerminateDataPathRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.ndpInstanceId = msg.ndpinstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseTerminateDataPathRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseSuspendRequestReq(const uint8_t* data, size_t length, SuspendRequestReqNanIfaceParam& param)
{
    SuspendRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseSuspendRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseResumeRequestReq(const uint8_t* data, size_t length, ResumeRequestReqNanIfaceParam& param)
{
    ResumeRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseResumeRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanTransmitFollowupRequest(const NanTransmitFollowupRequest_P& msgWifiNanIface, NanTransmitFollowupRequest& msg)
{
    msg.discoverySessionId = msgWifiNanIface.discoverysessionid();
    msg.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.addr(), msg.addr);
    msg.isHighPriority = msgWifiNanIface.ishighpriority();
    msg.shouldUseDiscoveryWindow = msgWifiNanIface.shouldusediscoverywindow();
    String2Vector(msgWifiNanIface.servicespecificinfo(), msg.serviceSpecificInfo);
    String2Vector(msgWifiNanIface.extendedservicespecificinfo(), msg.extendedServiceSpecificInfo);
    msg.disableFollowupResultIndication = msgWifiNanIface.disablefollowupresultindication();
}

bool WifiNanIfaceParseTransmitFollowupRequestReq(const uint8_t* data, size_t length, TransmitFollowupRequestReqNanIfaceParam& param)
{
    TransmitFollowupRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanTransmitFollowupRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseTransmitFollowupRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanPairingSecurityConfig(const NanPairingSecurityConfig_P& msgWifiNanIface, NanPairingSecurityConfig& securityConfig)
{
    securityConfig.securityType = (NanPairingSecurityType) msgWifiNanIface.securitytype();
    String2Array(msgWifiNanIface.pmk(), securityConfig.pmk);
    String2Vector(msgWifiNanIface.passphrase(), securityConfig.passphrase);
    securityConfig.akm = (NanPairingAkm) msgWifiNanIface.akm();
    securityConfig.cipherType = (NanCipherSuiteType) msgWifiNanIface.ciphertype();
}

static void Message2NanPairingRequest(const NanPairingRequest_P& msgWifiNanIface, NanPairingRequest& msg)
{
    msg.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), msg.peerDiscMacAddr);
    msg.requestType = (NanPairingRequestType) msgWifiNanIface.requesttype();
    msg.enablePairingCache = msgWifiNanIface.enablepairingcache();
    String2Array(msgWifiNanIface.pairingidentitykey(), msg.pairingIdentityKey);
    Message2NanPairingSecurityConfig(msgWifiNanIface.securityconfig(), msg.securityConfig);
}

bool WifiNanIfaceParseInitiatePairingRequestReq(const uint8_t* data, size_t length, InitiatePairingRequestReqNanIfaceParam& param)
{
    InitiatePairingRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanPairingRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseInitiatePairingRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanRespondToPairingIndicationRequest(const NanRespondToPairingIndicationRequest_P& msgWifiNanIface, NanRespondToPairingIndicationRequest& msg)
{
    msg.acceptRequest = msgWifiNanIface.acceptrequest();
    msg.pairingInstanceId = msgWifiNanIface.pairinginstanceid();
    msg.requestType = (NanPairingRequestType) msgWifiNanIface.requesttype();
    msg.enablePairingCache = msgWifiNanIface.enablepairingcache();
    String2Array(msgWifiNanIface.pairingidentitykey(), msg.pairingIdentityKey);
    Message2NanPairingSecurityConfig(msgWifiNanIface.securityconfig(), msg.securityConfig);
}

bool WifiNanIfaceParseRespondToPairingIndicationRequestReq(const uint8_t* data, size_t length, RespondToPairingIndicationRequestReqNanIfaceParam& param)
{
    RespondToPairingIndicationRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanRespondToPairingIndicationRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseRespondToPairingIndicationRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanBootstrappingRequest(const NanBootstrappingRequest_P& msgWifiNanIface, NanBootstrappingRequest& msg)
{
    msg.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), msg.peerDiscMacAddr);
    msg.requestBootstrappingMethod = (NanBootstrappingMethod) msgWifiNanIface.requestbootstrappingmethod();
    String2Vector(msgWifiNanIface.cookie(), msg.cookie);
}

bool WifiNanIfaceParseInitiateBootstrappingRequestReq(const uint8_t* data, size_t length, InitiateBootstrappingRequestReqNanIfaceParam& param)
{
    InitiateBootstrappingRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanBootstrappingRequest(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseInitiateBootstrappingRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanBootstrappingResponse(const NanBootstrappingResponse_P& msgWifiNanIface, NanBootstrappingResponse& msg)
{
    msg.bootstrappingInstanceId = msgWifiNanIface.bootstrappinginstanceid();
    msg.acceptRequest = msgWifiNanIface.acceptrequest();
}

bool WifiNanIfaceParseRespondToBootstrappingIndicationRequestReq(const uint8_t* data, size_t length, RespondToBootstrappingIndicationRequestReqNanIfaceParam& param)
{
    RespondToBootstrappingIndicationRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2NanBootstrappingResponse(msg.msg(), param.msg);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseRespondToBootstrappingIndicationRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiNanIfaceParseTerminatePairingRequestReq(const uint8_t* data, size_t length, TerminatePairingRequestReqNanIfaceParam& param)
{
    TerminatePairingRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.pairingInstanceId = msg.pairinginstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseTerminatePairingRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NanClusterEventInd(const NanClusterEventInd_P& msgWifiNanIface, NanClusterEventInd& event)
{
    event.eventType = (NanClusterEventType) msgWifiNanIface.eventtype();
    String2Array(msgWifiNanIface.addr(), event.addr);
}

bool WifiNanIfaceParseEventClusterEventInd(const uint8_t* data, size_t length, NanClusterEventInd& param)
{
    EventClusterEventInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanClusterEventInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanStatus(const NanStatus_P& msgWifiNanIface, NanStatus& status)
{
    status.status = (NanStatusCode) msgWifiNanIface.status();
    status.description = msgWifiNanIface.description();
}

static void Message2NanDataPathChannelInfo(const NanDataPathChannelInfo_P& msgWifiNanIface, NanDataPathChannelInfo& channelInfo)
{
    channelInfo.channelFreq = msgWifiNanIface.channelfreq();
    channelInfo.channelBandwidth = (WifiChannelWidthInMhz) msgWifiNanIface.channelbandwidth();
    channelInfo.numSpatialStreams = msgWifiNanIface.numspatialstreams();
}

static void Message2NanDataPathConfirmInd(const NanDataPathConfirmInd_P& msgWifiNanIface, NanDataPathConfirmInd& event)
{
    event.ndpInstanceId = msgWifiNanIface.ndpinstanceid();
    event.dataPathSetupSuccess = msgWifiNanIface.datapathsetupsuccess();
    String2Array(msgWifiNanIface.peerndimacaddr(), event.peerNdiMacAddr);
    String2Vector(msgWifiNanIface.appinfo(), event.appInfo);
    Message2NanStatus(msgWifiNanIface.status(), event.status);
    int size_channelinfo = msgWifiNanIface.channelinfo_size();
    if (0 < size_channelinfo)
    {
        event.channelInfo.resize(size_channelinfo);
        for (int index = 0; index < size_channelinfo; index++)
        {
            Message2NanDataPathChannelInfo(msgWifiNanIface.channelinfo(index), event.channelInfo[index]);
        }
    }
}

bool WifiNanIfaceParseEventDataPathConfirmInd(const uint8_t* data, size_t length, NanDataPathConfirmInd& param)
{
    EventDataPathConfirmInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanDataPathConfirmInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanDataPathRequestInd(const NanDataPathRequestInd_P& msgWifiNanIface, NanDataPathRequestInd& event)
{
    event.discoverySessionId = msgWifiNanIface.discoverysessionid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), event.peerDiscMacAddr);
    event.ndpInstanceId = msgWifiNanIface.ndpinstanceid();
    event.securityRequired = msgWifiNanIface.securityrequired();
    String2Vector(msgWifiNanIface.appinfo(), event.appInfo);
}

bool WifiNanIfaceParseEventDataPathRequestInd(const uint8_t* data, size_t length, NanDataPathRequestInd& param)
{
    EventDataPathRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanDataPathRequestInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanDataPathScheduleUpdateInd(const NanDataPathScheduleUpdateInd_P& msgWifiNanIface, NanDataPathScheduleUpdateInd& event)
{
    String2Array(msgWifiNanIface.peerdiscoveryaddress(), event.peerDiscoveryAddress);
    int size_channelinfo = msgWifiNanIface.channelinfo_size();
    if (0 < size_channelinfo)
    {
        event.channelInfo.resize(size_channelinfo);
        for (int index = 0; index < size_channelinfo; index++)
        {
            Message2NanDataPathChannelInfo(msgWifiNanIface.channelinfo(index), event.channelInfo[index]);
        }
    }
    int size_ndpinstanceids = msgWifiNanIface.ndpinstanceids_size();
    if (0 < size_ndpinstanceids)
    {
        event.ndpInstanceIds.resize(size_ndpinstanceids);
        for (int index = 0; index < size_ndpinstanceids; index++)
        {
            event.ndpInstanceIds[index] = msgWifiNanIface.ndpinstanceids(index);
        }
    }
}

bool WifiNanIfaceParseEventDataPathScheduleUpdateInd(const uint8_t* data, size_t length, NanDataPathScheduleUpdateInd& param)
{
    EventDataPathScheduleUpdateInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanDataPathScheduleUpdateInd(msg.event(), param);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventDataPathTerminatedInd(const uint8_t* data, size_t length, int32_t& param)
{
    EventDataPathTerminatedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ndpinstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventDisabledInd(const uint8_t* data, size_t length, NanStatus& param)
{
    EventDisabledInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanStatus(msg.status(), param);
        return true;
    }
    return false;
}

static void Message2NanFollowupReceivedInd(const NanFollowupReceivedInd_P& msgWifiNanIface, NanFollowupReceivedInd& event)
{
    event.discoverySessionId = msgWifiNanIface.discoverysessionid();
    event.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.addr(), event.addr);
    event.receivedInFaw = msgWifiNanIface.receivedinfaw();
    String2Vector(msgWifiNanIface.servicespecificinfo(), event.serviceSpecificInfo);
    String2Vector(msgWifiNanIface.extendedservicespecificinfo(), event.extendedServiceSpecificInfo);
}

bool WifiNanIfaceParseEventFollowupReceivedInd(const uint8_t* data, size_t length, NanFollowupReceivedInd& param)
{
    EventFollowupReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanFollowupReceivedInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanIdentityResolutionAttribute(const NanIdentityResolutionAttribute_P& msgWifiNanIface, NanIdentityResolutionAttribute& peerNira)
{
    String2Array(msgWifiNanIface.nonce(), peerNira.nonce);
    String2Array(msgWifiNanIface.tag(), peerNira.tag);
}

static void Message2NanMatchInd(const NanMatchInd_P& msgWifiNanIface, NanMatchInd& event)
{
    event.discoverySessionId = msgWifiNanIface.discoverysessionid();
    event.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.addr(), event.addr);
    String2Vector(msgWifiNanIface.servicespecificinfo(), event.serviceSpecificInfo);
    String2Vector(msgWifiNanIface.extendedservicespecificinfo(), event.extendedServiceSpecificInfo);
    String2Vector(msgWifiNanIface.matchfilter(), event.matchFilter);
    event.matchOccurredInBeaconFlag = msgWifiNanIface.matchoccurredinbeaconflag();
    event.outOfResourceFlag = msgWifiNanIface.outofresourceflag();
    event.rssiValue = msgWifiNanIface.rssivalue();
    event.peerCipherType = (NanCipherSuiteType) msgWifiNanIface.peerciphertype();
    event.peerRequiresSecurityEnabledInNdp = msgWifiNanIface.peerrequiressecurityenabledinndp();
    event.peerRequiresRanging = msgWifiNanIface.peerrequiresranging();
    event.rangingMeasurementInMm = msgWifiNanIface.rangingmeasurementinmm();
    event.rangingIndicationType = msgWifiNanIface.rangingindicationtype();
    String2Vector(msgWifiNanIface.scid(), event.scid);
    Message2NanPairingConfig(msgWifiNanIface.peerpairingconfig(), event.peerPairingConfig);
    Message2NanIdentityResolutionAttribute(msgWifiNanIface.peernira(), event.peerNira);
}

bool WifiNanIfaceParseEventMatchInd(const uint8_t* data, size_t length, NanMatchInd& param)
{
    EventMatchInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanMatchInd(msg.event(), param);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventMatchExpiredInd(const uint8_t* data, size_t length, EventMatchExpiredIndNanIfaceParam& param)
{
    EventMatchExpiredInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.discoverySessionId = msg.discoverysessionid();
        param.peerId = msg.peerid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventPublishTerminatedInd(const uint8_t* data, size_t length, EventPublishTerminatedIndNanIfaceParam& param)
{
    EventPublishTerminatedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.sessionId = msg.sessionid();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventSubscribeTerminatedInd(const uint8_t* data, size_t length, EventSubscribeTerminatedIndNanIfaceParam& param)
{
    EventSubscribeTerminatedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.sessionId = msg.sessionid();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseEventTransmitFollowupInd(const uint8_t* data, size_t length, EventTransmitFollowupIndNanIfaceParam& param)
{
    EventTransmitFollowupInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

static void Message2NanSuspensionModeChangeInd(const NanSuspensionModeChangeInd_P& msgWifiNanIface, NanSuspensionModeChangeInd& event)
{
    event.isSuspended = msgWifiNanIface.issuspended();
}

bool WifiNanIfaceParseEventSuspensionModeChangedInd(const uint8_t* data, size_t length, NanSuspensionModeChangeInd& param)
{
    EventSuspensionModeChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanSuspensionModeChangeInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanCapabilities(const NanCapabilities_P& msgWifiNanIface, NanCapabilities& capabilities)
{
    capabilities.maxConcurrentClusters = msgWifiNanIface.maxconcurrentclusters();
    capabilities.maxPublishes = msgWifiNanIface.maxpublishes();
    capabilities.maxSubscribes = msgWifiNanIface.maxsubscribes();
    capabilities.maxServiceNameLen = msgWifiNanIface.maxservicenamelen();
    capabilities.maxMatchFilterLen = msgWifiNanIface.maxmatchfilterlen();
    capabilities.maxTotalMatchFilterLen = msgWifiNanIface.maxtotalmatchfilterlen();
    capabilities.maxServiceSpecificInfoLen = msgWifiNanIface.maxservicespecificinfolen();
    capabilities.maxExtendedServiceSpecificInfoLen = msgWifiNanIface.maxextendedservicespecificinfolen();
    capabilities.maxNdiInterfaces = msgWifiNanIface.maxndiinterfaces();
    capabilities.maxNdpSessions = msgWifiNanIface.maxndpsessions();
    capabilities.maxAppInfoLen = msgWifiNanIface.maxappinfolen();
    capabilities.maxQueuedTransmitFollowupMsgs = msgWifiNanIface.maxqueuedtransmitfollowupmsgs();
    capabilities.maxSubscribeInterfaceAddresses = msgWifiNanIface.maxsubscribeinterfaceaddresses();
    capabilities.supportedCipherSuites = msgWifiNanIface.supportedciphersuites();
    capabilities.instantCommunicationModeSupportFlag = msgWifiNanIface.instantcommunicationmodesupportflag();
    capabilities.supports6g = msgWifiNanIface.supports6g();
    capabilities.supportsHe = msgWifiNanIface.supportshe();
    capabilities.supportsPairing = msgWifiNanIface.supportspairing();
    capabilities.supportsSetClusterId = msgWifiNanIface.supportssetclusterid();
    capabilities.supportsSuspension = msgWifiNanIface.supportssuspension();
}

bool WifiNanIfaceParseNotifyCapabilitiesResponseInd(const uint8_t* data, size_t length, NotifyCapabilitiesResponseIndNanIfaceParam& param)
{
    NotifyCapabilitiesResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        Message2NanCapabilities(msg.capabilities(), param.capabilities);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyConfigResponseInd(const uint8_t* data, size_t length, NotifyConfigResponseIndNanIfaceParam& param)
{
    NotifyConfigResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyCreateDataInterfaceResponseInd(const uint8_t* data, size_t length, NotifyCreateDataInterfaceResponseIndNanIfaceParam& param)
{
    NotifyCreateDataInterfaceResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyDeleteDataInterfaceResponseInd(const uint8_t* data, size_t length, NotifyDeleteDataInterfaceResponseIndNanIfaceParam& param)
{
    NotifyDeleteDataInterfaceResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyDisableResponseInd(const uint8_t* data, size_t length, NotifyDisableResponseIndNanIfaceParam& param)
{
    NotifyDisableResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyEnableResponseInd(const uint8_t* data, size_t length, NotifyEnableResponseIndNanIfaceParam& param)
{
    NotifyEnableResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyInitiateDataPathResponseInd(const uint8_t* data, size_t length, NotifyInitiateDataPathResponseIndNanIfaceParam& param)
{
    NotifyInitiateDataPathResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        param.ndpInstanceId = msg.ndpinstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyRespondToDataPathIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToDataPathIndicationResponseIndNanIfaceParam& param)
{
    NotifyRespondToDataPathIndicationResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyStartPublishResponseInd(const uint8_t* data, size_t length, NotifyStartPublishResponseIndNanIfaceParam& param)
{
    NotifyStartPublishResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyStartSubscribeResponseInd(const uint8_t* data, size_t length, NotifyStartSubscribeResponseIndNanIfaceParam& param)
{
    NotifyStartSubscribeResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        param.sessionId = msg.sessionid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyStopPublishResponseInd(const uint8_t* data, size_t length, NotifyStopPublishResponseIndNanIfaceParam& param)
{
    NotifyStopPublishResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyStopSubscribeResponseInd(const uint8_t* data, size_t length, NotifyStopSubscribeResponseIndNanIfaceParam& param)
{
    NotifyStopSubscribeResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyTerminateDataPathResponseInd(const uint8_t* data, size_t length, NotifyTerminateDataPathResponseIndNanIfaceParam& param)
{
    NotifyTerminateDataPathResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifySuspendResponseInd(const uint8_t* data, size_t length, NotifySuspendResponseIndNanIfaceParam& param)
{
    NotifySuspendResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyResumeResponseInd(const uint8_t* data, size_t length, NotifyResumeResponseIndNanIfaceParam& param)
{
    NotifyResumeResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyTransmitFollowupResponseInd(const uint8_t* data, size_t length, NotifyTransmitFollowupResponseIndNanIfaceParam& param)
{
    NotifyTransmitFollowupResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

static void Message2NanPairingRequestInd(const NanPairingRequestInd_P& msgWifiNanIface, NanPairingRequestInd& event)
{
    event.discoverySessionId = msgWifiNanIface.discoverysessionid();
    event.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), event.peerDiscMacAddr);
    event.pairingInstanceId = msgWifiNanIface.pairinginstanceid();
    event.requestType = (NanPairingRequestType) msgWifiNanIface.requesttype();
    event.enablePairingCache = msgWifiNanIface.enablepairingcache();
    Message2NanIdentityResolutionAttribute(msgWifiNanIface.peernira(), event.peerNira);
}

bool WifiNanIfaceParseEventPairingRequestInd(const uint8_t* data, size_t length, NanPairingRequestInd& param)
{
    EventPairingRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanPairingRequestInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NpkSecurityAssociation(const NpkSecurityAssociation_P& msgWifiNanIface, NpkSecurityAssociation& npksa)
{
    String2Array(msgWifiNanIface.peernanidentitykey(), npksa.peerNanIdentityKey);
    String2Array(msgWifiNanIface.localnanidentitykey(), npksa.localNanIdentityKey);
    String2Array(msgWifiNanIface.npk(), npksa.npk);
    npksa.akm = (NanPairingAkm) msgWifiNanIface.akm();
    npksa.cipherType = (NanCipherSuiteType) msgWifiNanIface.ciphertype();
}

static void Message2NanPairingConfirmInd(const NanPairingConfirmInd_P& msgWifiNanIface, NanPairingConfirmInd& event)
{
    event.pairingInstanceId = msgWifiNanIface.pairinginstanceid();
    event.pairingSuccess = msgWifiNanIface.pairingsuccess();
    Message2NanStatus(msgWifiNanIface.status(), event.status);
    event.requestType = (NanPairingRequestType) msgWifiNanIface.requesttype();
    event.enablePairingCache = msgWifiNanIface.enablepairingcache();
    Message2NpkSecurityAssociation(msgWifiNanIface.npksa(), event.npksa);
}

bool WifiNanIfaceParseEventPairingConfirmInd(const uint8_t* data, size_t length, NanPairingConfirmInd& param)
{
    EventPairingConfirmInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanPairingConfirmInd(msg.event(), param);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyInitiatePairingResponseInd(const uint8_t* data, size_t length, NotifyInitiatePairingResponseIndNanIfaceParam& param)
{
    NotifyInitiatePairingResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        param.pairingInstanceId = msg.pairinginstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyRespondToPairingIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToPairingIndicationResponseIndNanIfaceParam& param)
{
    NotifyRespondToPairingIndicationResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

static void Message2NanBootstrappingRequestInd(const NanBootstrappingRequestInd_P& msgWifiNanIface, NanBootstrappingRequestInd& event)
{
    event.discoverySessionId = msgWifiNanIface.discoverysessionid();
    event.peerId = msgWifiNanIface.peerid();
    String2Array(msgWifiNanIface.peerdiscmacaddr(), event.peerDiscMacAddr);
    event.bootstrappingInstanceId = msgWifiNanIface.bootstrappinginstanceid();
    event.requestBootstrappingMethod = (NanBootstrappingMethod) msgWifiNanIface.requestbootstrappingmethod();
}

bool WifiNanIfaceParseEventBootstrappingRequestInd(const uint8_t* data, size_t length, NanBootstrappingRequestInd& param)
{
    EventBootstrappingRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanBootstrappingRequestInd(msg.event(), param);
        return true;
    }
    return false;
}

static void Message2NanBootstrappingConfirmInd(const NanBootstrappingConfirmInd_P& msgWifiNanIface, NanBootstrappingConfirmInd& event)
{
    event.bootstrappingInstanceId = msgWifiNanIface.bootstrappinginstanceid();
    event.responseCode = (NanBootstrappingResponseCode) msgWifiNanIface.responsecode();
    Message2NanStatus(msgWifiNanIface.reasoncode(), event.reasonCode);
    event.comeBackDelay = msgWifiNanIface.comebackdelay();
    String2Vector(msgWifiNanIface.cookie(), event.cookie);
}

bool WifiNanIfaceParseEventBootstrappingConfirmInd(const uint8_t* data, size_t length, NanBootstrappingConfirmInd& param)
{
    EventBootstrappingConfirmInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NanBootstrappingConfirmInd(msg.event(), param);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyInitiateBootstrappingResponseInd(const uint8_t* data, size_t length, NotifyInitiateBootstrappingResponseIndNanIfaceParam& param)
{
    NotifyInitiateBootstrappingResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        param.bootstrappingInstanceId = msg.bootstrappinginstanceid();
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyRespondToBootstrappingIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToBootstrappingIndicationResponseIndNanIfaceParam& param)
{
    NotifyRespondToBootstrappingIndicationResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}

bool WifiNanIfaceParseNotifyTerminatePairingResponseInd(const uint8_t* data, size_t length, NotifyTerminatePairingResponseIndNanIfaceParam& param)
{
    NotifyTerminatePairingResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.id = msg.id();
        Message2NanStatus(msg.status(), param.status);
        return true;
    }
    return false;
}
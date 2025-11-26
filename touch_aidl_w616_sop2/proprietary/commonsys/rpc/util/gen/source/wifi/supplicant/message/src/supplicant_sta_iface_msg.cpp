/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_sta_iface_msg.h"

#include "AnqpData.pb.h"
#include "AnqpInfoId.pb.h"
#include "AssociationRejectionData.pb.h"
#include "AuxiliarySupplicantEventCode.pb.h"
#include "BssTmData.pb.h"
#include "BssTmDataFlagsMask.pb.h"
#include "BssTmStatusCode.pb.h"
#include "BssidChangeReason.pb.h"
#include "BtCoexistenceMode.pb.h"
#include "ConnectionCapabilities.pb.h"
#include "DppAkm.pb.h"
#include "DppConfigurationData.pb.h"
#include "DppConnectionKeys.pb.h"
#include "DppCurve.pb.h"
#include "DppEventType.pb.h"
#include "DppFailureCode.pb.h"
#include "DppNetRole.pb.h"
#include "DppProgressCode.pb.h"
#include "DppResponderBootstrapInfo.pb.h"
#include "DppStatusErrorCode.pb.h"
#include "HalStatus.pb.h"
#include "Hs20AnqpData.pb.h"
#include "Hs20AnqpSubtypes.pb.h"
#include "ISupplicantStaIface.pb.h"
#include "ISupplicantStaIfaceCallback.pb.h"
#include "ISupplicantStaNetwork.pb.h"
#include "IfaceType.pb.h"
#include "IpVersion.pb.h"
#include "KeyMgmtMask.pb.h"
#include "LegacyMode.pb.h"
#include "MboAssocDisallowedReasonCode.pb.h"
#include "MboCellularDataConnectionPrefValue.pb.h"
#include "MboTransitionReasonCode.pb.h"
#include "MloLink.pb.h"
#include "MloLinksInfo.pb.h"
#include "OceRssiBasedAssocRejectAttr.pb.h"
#include "OsuMethod.pb.h"
#include "PmkSaCacheData.pb.h"
#include "PortRange.pb.h"
#include "ProtocolNextHeader.pb.h"
#include "QosPolicyClassifierParams.pb.h"
#include "QosPolicyClassifierParamsMask.pb.h"
#include "QosPolicyData.pb.h"
#include "QosPolicyRequestType.pb.h"
#include "QosPolicyScsData.pb.h"
#include "QosPolicyScsRequestStatus.pb.h"
#include "QosPolicyScsRequestStatusCode.pb.h"
#include "QosPolicyScsResponseStatus.pb.h"
#include "QosPolicyScsResponseStatusCode.pb.h"
#include "QosPolicyStatus.pb.h"
#include "QosPolicyStatusCode.pb.h"
#include "RxFilterType.pb.h"
#include "SignalPollResult.pb.h"
#include "StaIfaceCallbackState.pb.h"
#include "StaIfaceReasonCode.pb.h"
#include "StaIfaceStatusCode.pb.h"
#include "SupplicantStateChangeData.pb.h"
#include "WifiTechnology.pb.h"
#include "WpaDriverCapabilitiesMask.pb.h"
#include "WpsConfigError.pb.h"
#include "WpsConfigMethods.pb.h"
#include "WpsErrorIndication.pb.h"

using AddDppPeerUriCfm = ::wifi::supplicant::ISupplicantStaIface_AddDppPeerUriCfm;
using AddDppPeerUriReq = ::wifi::supplicant::ISupplicantStaIface_AddDppPeerUriReq;
using AddExtRadioWorkCfm = ::wifi::supplicant::ISupplicantStaIface_AddExtRadioWorkCfm;
using AddExtRadioWorkReq = ::wifi::supplicant::ISupplicantStaIface_AddExtRadioWorkReq;
using AddNetworkCfm = ::wifi::supplicant::ISupplicantStaIface_AddNetworkCfm;
using AddQosPolicyRequestForScsCfm = ::wifi::supplicant::ISupplicantStaIface_AddQosPolicyRequestForScsCfm;
using AddQosPolicyRequestForScsReq = ::wifi::supplicant::ISupplicantStaIface_AddQosPolicyRequestForScsReq;
using AddRxFilterReq = ::wifi::supplicant::ISupplicantStaIface_AddRxFilterReq;
using EnableAutoReconnectReq = ::wifi::supplicant::ISupplicantStaIface_EnableAutoReconnectReq;
using FilsHlpAddRequestReq = ::wifi::supplicant::ISupplicantStaIface_FilsHlpAddRequestReq;
using GenerateDppBootstrapInfoForResponderCfm = ::wifi::supplicant::ISupplicantStaIface_GenerateDppBootstrapInfoForResponderCfm;
using GenerateDppBootstrapInfoForResponderReq = ::wifi::supplicant::ISupplicantStaIface_GenerateDppBootstrapInfoForResponderReq;
using GenerateSelfDppConfigurationReq = ::wifi::supplicant::ISupplicantStaIface_GenerateSelfDppConfigurationReq;
using GetConnectionCapabilitiesCfm = ::wifi::supplicant::ISupplicantStaIface_GetConnectionCapabilitiesCfm;
using GetConnectionMloLinksInfoCfm = ::wifi::supplicant::ISupplicantStaIface_GetConnectionMloLinksInfoCfm;
using GetKeyMgmtCapabilitiesCfm = ::wifi::supplicant::ISupplicantStaIface_GetKeyMgmtCapabilitiesCfm;
using GetMacAddressCfm = ::wifi::supplicant::ISupplicantStaIface_GetMacAddressCfm;
using GetNameCfm = ::wifi::supplicant::ISupplicantStaIface_GetNameCfm;
using GetNetworkCfm = ::wifi::supplicant::ISupplicantStaIface_GetNetworkCfm;
using GetNetworkReq = ::wifi::supplicant::ISupplicantStaIface_GetNetworkReq;
using GetSignalPollResultsCfm = ::wifi::supplicant::ISupplicantStaIface_GetSignalPollResultsCfm;
using GetTypeCfm = ::wifi::supplicant::ISupplicantStaIface_GetTypeCfm;
using GetWpaDriverCapabilitiesCfm = ::wifi::supplicant::ISupplicantStaIface_GetWpaDriverCapabilitiesCfm;
using InitiateAnqpQueryReq = ::wifi::supplicant::ISupplicantStaIface_InitiateAnqpQueryReq;
using InitiateHs20IconQueryReq = ::wifi::supplicant::ISupplicantStaIface_InitiateHs20IconQueryReq;
using InitiateTdlsDiscoverReq = ::wifi::supplicant::ISupplicantStaIface_InitiateTdlsDiscoverReq;
using InitiateTdlsSetupReq = ::wifi::supplicant::ISupplicantStaIface_InitiateTdlsSetupReq;
using InitiateTdlsTeardownReq = ::wifi::supplicant::ISupplicantStaIface_InitiateTdlsTeardownReq;
using InitiateVenueUrlAnqpQueryReq = ::wifi::supplicant::ISupplicantStaIface_InitiateVenueUrlAnqpQueryReq;
using ListNetworksCfm = ::wifi::supplicant::ISupplicantStaIface_ListNetworksCfm;
using OnAnqpQueryDoneInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnAnqpQueryDoneInd;
using OnAssociationRejectedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnAssociationRejectedInd;
using OnAuthenticationTimeoutInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnAuthenticationTimeoutInd;
using OnAuxiliarySupplicantEventInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnAuxiliarySupplicantEventInd;
using OnBssFrequencyChangedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnBssFrequencyChangedInd;
using OnBssTmHandlingDoneInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnBssTmHandlingDoneInd;
using OnBssidChangedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnBssidChangedInd;
using OnDisconnectedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDisconnectedInd;
using OnDppConfigReceivedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppConfigReceivedInd;
using OnDppConnectionStatusResultSentInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppConnectionStatusResultSentInd;
using OnDppFailureInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppFailureInd;
using OnDppProgressInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppProgressInd;
using OnDppSuccessConfigReceivedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppSuccessConfigReceivedInd;
using OnDppSuccessInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnDppSuccessInd;
using OnEapFailureInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnEapFailureInd;
using OnExtRadioWorkStartInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnExtRadioWorkStartInd;
using OnExtRadioWorkTimeoutInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnExtRadioWorkTimeoutInd;
using OnHs20DeauthImminentNoticeInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnHs20DeauthImminentNoticeInd;
using OnHs20IconQueryDoneInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnHs20IconQueryDoneInd;
using OnHs20SubscriptionRemediationInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnHs20SubscriptionRemediationInd;
using OnHs20TermsAndConditionsAcceptanceRequestedNotificationInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnHs20TermsAndConditionsAcceptanceRequestedNotificationInd;
using OnMloLinksInfoChangedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnMloLinksInfoChangedInd;
using OnNetworkAddedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnNetworkAddedInd;
using OnNetworkNotFoundInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnNetworkNotFoundInd;
using OnNetworkRemovedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnNetworkRemovedInd;
using OnPmkCacheAddedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnPmkCacheAddedInd;
using OnPmkSaCacheAddedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnPmkSaCacheAddedInd;
using OnQosPolicyRequestInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnQosPolicyRequestInd;
using OnQosPolicyResponseForScsInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnQosPolicyResponseForScsInd;
using OnStateChangedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnStateChangedInd;
using OnSupplicantStateChangedInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnSupplicantStateChangedInd;
using OnWpsEventFailInd = ::wifi::supplicant::ISupplicantStaIfaceCallback_OnWpsEventFailInd;
using RemoveDppUriReq = ::wifi::supplicant::ISupplicantStaIface_RemoveDppUriReq;
using RemoveExtRadioWorkReq = ::wifi::supplicant::ISupplicantStaIface_RemoveExtRadioWorkReq;
using RemoveNetworkReq = ::wifi::supplicant::ISupplicantStaIface_RemoveNetworkReq;
using RemoveQosPolicyForScsCfm = ::wifi::supplicant::ISupplicantStaIface_RemoveQosPolicyForScsCfm;
using RemoveQosPolicyForScsReq = ::wifi::supplicant::ISupplicantStaIface_RemoveQosPolicyForScsReq;
using RemoveRxFilterReq = ::wifi::supplicant::ISupplicantStaIface_RemoveRxFilterReq;
using SendQosPolicyResponseReq = ::wifi::supplicant::ISupplicantStaIface_SendQosPolicyResponseReq;
using SetBtCoexistenceModeReq = ::wifi::supplicant::ISupplicantStaIface_SetBtCoexistenceModeReq;
using SetBtCoexistenceScanModeEnabledReq = ::wifi::supplicant::ISupplicantStaIface_SetBtCoexistenceScanModeEnabledReq;
using SetCountryCodeReq = ::wifi::supplicant::ISupplicantStaIface_SetCountryCodeReq;
using SetExternalSimReq = ::wifi::supplicant::ISupplicantStaIface_SetExternalSimReq;
using SetMboCellularDataStatusReq = ::wifi::supplicant::ISupplicantStaIface_SetMboCellularDataStatusReq;
using SetPowerSaveReq = ::wifi::supplicant::ISupplicantStaIface_SetPowerSaveReq;
using SetQosPolicyFeatureEnabledReq = ::wifi::supplicant::ISupplicantStaIface_SetQosPolicyFeatureEnabledReq;
using SetSuspendModeEnabledReq = ::wifi::supplicant::ISupplicantStaIface_SetSuspendModeEnabledReq;
using SetWpsConfigMethodsReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsConfigMethodsReq;
using SetWpsDeviceNameReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsDeviceNameReq;
using SetWpsDeviceTypeReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsDeviceTypeReq;
using SetWpsManufacturerReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsManufacturerReq;
using SetWpsModelNameReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsModelNameReq;
using SetWpsModelNumberReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsModelNumberReq;
using SetWpsSerialNumberReq = ::wifi::supplicant::ISupplicantStaIface_SetWpsSerialNumberReq;
using StartDppConfiguratorInitiatorCfm = ::wifi::supplicant::ISupplicantStaIface_StartDppConfiguratorInitiatorCfm;
using StartDppConfiguratorInitiatorReq = ::wifi::supplicant::ISupplicantStaIface_StartDppConfiguratorInitiatorReq;
using StartDppEnrolleeInitiatorReq = ::wifi::supplicant::ISupplicantStaIface_StartDppEnrolleeInitiatorReq;
using StartDppEnrolleeResponderReq = ::wifi::supplicant::ISupplicantStaIface_StartDppEnrolleeResponderReq;
using StartWpsPbcReq = ::wifi::supplicant::ISupplicantStaIface_StartWpsPbcReq;
using StartWpsPinDisplayCfm = ::wifi::supplicant::ISupplicantStaIface_StartWpsPinDisplayCfm;
using StartWpsPinDisplayReq = ::wifi::supplicant::ISupplicantStaIface_StartWpsPinDisplayReq;
using StartWpsPinKeypadReq = ::wifi::supplicant::ISupplicantStaIface_StartWpsPinKeypadReq;
using StartWpsRegistrarReq = ::wifi::supplicant::ISupplicantStaIface_StartWpsRegistrarReq;
using StopDppResponderReq = ::wifi::supplicant::ISupplicantStaIface_StopDppResponderReq;

using AnqpData_P = ::wifi::supplicant::AnqpData;
using AnqpInfoId_P = ::wifi::supplicant::aii::AnqpInfoId;
using AssociationRejectionData_P = ::wifi::supplicant::AssociationRejectionData;
using AuxiliarySupplicantEventCode_P = ::wifi::supplicant::AuxiliarySupplicantEventCode;
using BssTmDataFlagsMask_P = ::wifi::supplicant::BssTmDataFlagsMask;
using BssTmData_P = ::wifi::supplicant::BssTmData;
using BssTmStatusCode_P = ::wifi::supplicant::BssTmStatusCode;
using BssidChangeReason_P = ::wifi::supplicant::BssidChangeReason;
using BtCoexistenceMode_P = ::wifi::supplicant::bcm::BtCoexistenceMode;
using ConnectionCapabilities_P = ::wifi::supplicant::ConnectionCapabilities;
using DppAkm_P = ::wifi::supplicant::da::DppAkm;
using DppConfigurationData_P = ::wifi::supplicant::DppConfigurationData;
using DppConnectionKeys_P = ::wifi::supplicant::DppConnectionKeys;
using DppCurve_P = ::wifi::supplicant::DppCurve;
using DppEventType_P = ::wifi::supplicant::DppEventType;
using DppFailureCode_P = ::wifi::supplicant::dfc::DppFailureCode;
using DppNetRole_P = ::wifi::supplicant::dnr::DppNetRole;
using DppProgressCode_P = ::wifi::supplicant::dpc::DppProgressCode;
using DppResponderBootstrapInfo_P = ::wifi::supplicant::DppResponderBootstrapInfo;
using DppStatusErrorCode_P = ::wifi::supplicant::dsec::DppStatusErrorCode;
using HalStatus_P = ::wifi::supplicant::HalStatus;
using Hs20AnqpData_P = ::wifi::supplicant::Hs20AnqpData;
using Hs20AnqpSubtypes_P = ::wifi::supplicant::Hs20AnqpSubtypes;
using ISupplicantStaNetwork_P = ::wifi::supplicant::ISupplicantStaNetwork;
using IfaceType_P = ::wifi::supplicant::it::IfaceType;
using IpVersion_P = ::wifi::supplicant::IpVersion;
using KeyMgmtMask_P = ::wifi::supplicant::kmm::KeyMgmtMask;
using LegacyMode_P = ::wifi::supplicant::lm::LegacyMode;
using MboAssocDisallowedReasonCode_P = ::wifi::supplicant::madrc::MboAssocDisallowedReasonCode;
using MboCellularDataConnectionPrefValue_P = ::wifi::supplicant::MboCellularDataConnectionPrefValue;
using MboTransitionReasonCode_P = ::wifi::supplicant::mtrc::MboTransitionReasonCode;
using MloLinkInfoChangeReason_P = ::wifi::supplicant::ISupplicantStaIfaceCallback_MloLinkInfoChangeReason;
using MloLink_P = ::wifi::supplicant::MloLink;
using MloLinksInfo_P = ::wifi::supplicant::MloLinksInfo;
using OceRssiBasedAssocRejectAttr_P = ::wifi::supplicant::OceRssiBasedAssocRejectAttr;
using OsuMethod_P = ::wifi::supplicant::OsuMethod;
using PmkSaCacheData_P = ::wifi::supplicant::PmkSaCacheData;
using PortRange_P = ::wifi::supplicant::PortRange;
using ProtocolNextHeader_P = ::wifi::supplicant::ProtocolNextHeader;
using QosPolicyClassifierParamsMask_P = ::wifi::supplicant::qpcpm::QosPolicyClassifierParamsMask;
using QosPolicyClassifierParams_P = ::wifi::supplicant::QosPolicyClassifierParams;
using QosPolicyData_P = ::wifi::supplicant::QosPolicyData;
using QosPolicyRequestType_P = ::wifi::supplicant::QosPolicyRequestType;
using QosPolicyScsData_P = ::wifi::supplicant::QosPolicyScsData;
using QosPolicyScsRequestStatusCode_P = ::wifi::supplicant::QosPolicyScsRequestStatusCode;
using QosPolicyScsRequestStatus_P = ::wifi::supplicant::QosPolicyScsRequestStatus;
using QosPolicyScsResponseStatusCode_P = ::wifi::supplicant::qpsrsc::QosPolicyScsResponseStatusCode;
using QosPolicyScsResponseStatus_P = ::wifi::supplicant::QosPolicyScsResponseStatus;
using QosPolicyStatusCode_P = ::wifi::supplicant::QosPolicyStatusCode;
using QosPolicyStatus_P = ::wifi::supplicant::QosPolicyStatus;
using RxFilterType_P = ::wifi::supplicant::RxFilterType;
using SignalPollResult_P = ::wifi::supplicant::SignalPollResult;
using StaIfaceCallbackState_P = ::wifi::supplicant::StaIfaceCallbackState;
using StaIfaceReasonCode_P = ::wifi::supplicant::sirc::StaIfaceReasonCode;
using StaIfaceStatusCode_P = ::wifi::supplicant::sisc::StaIfaceStatusCode;
using SupplicantStateChangeData_P = ::wifi::supplicant::SupplicantStateChangeData;
using WifiTechnology_P = ::wifi::supplicant::wt::WifiTechnology;
using WpaDriverCapabilitiesMask_P = ::wifi::supplicant::wdcm::WpaDriverCapabilitiesMask;
using WpsConfigError_P = ::wifi::supplicant::wce::WpsConfigError;
using WpsConfigMethods_P = ::wifi::supplicant::wcm::WpsConfigMethods;
using WpsErrorIndication_P = ::wifi::supplicant::wei::WpsErrorIndication;


bool SupplicantStaIfaceSerializeAddDppPeerUriReq(const string& uri, vector<uint8_t>& payload)
{
    AddDppPeerUriReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_uri(uri);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_status(status.status);
    msgSupplicantStaIface.set_info(status.info);
}

bool SupplicantStaIfaceSerializeAddDppPeerUriCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddDppPeerUriCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeAddExtRadioWorkReq(const string& name, int32_t freqInMhz, int32_t timeoutInSec, vector<uint8_t>& payload)
{
    AddExtRadioWorkReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_name(name);
    msgSupplicantStaIface.set_freqinmhz(freqInMhz);
    msgSupplicantStaIface.set_timeoutinsec(timeoutInSec);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeAddExtRadioWorkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddExtRadioWorkCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeAddNetworkReq(vector<uint8_t>& payload)
{
    return true;
}

static void ISupplicantStaNetwork2Message(int32_t result, ISupplicantStaNetwork_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_instanceid(result);
}

bool SupplicantStaIfaceSerializeAddNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddNetworkCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantStaNetwork_P* result_p = msgSupplicantStaIface.mutable_result();
    ISupplicantStaNetwork2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeAddRxFilterReq(RxFilterType type, vector<uint8_t>& payload)
{
    AddRxFilterReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_type((RxFilterType_P) type);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeAddRxFilterCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeCancelWpsReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeCancelWpsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeDisconnectReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeDisconnectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeEnableAutoReconnectReq(bool enable, vector<uint8_t>& payload)
{
    EnableAutoReconnectReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeEnableAutoReconnectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeFilsHlpAddRequestReq(const vector<uint8_t>& dst_mac, const vector<uint8_t>& pkt, vector<uint8_t>& payload)
{
    FilsHlpAddRequestReq msgSupplicantStaIface;
    string dst_macStr;
    Vector2String(dst_mac, dst_macStr);
    msgSupplicantStaIface.set_dst_mac(dst_macStr);
    string pktStr;
    Vector2String(pkt, pktStr);
    msgSupplicantStaIface.set_pkt(pktStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeFilsHlpAddRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeFilsHlpFlushRequestReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeFilsHlpFlushRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderReq(const vector<uint8_t>& macAddress, const string& deviceInfo, DppCurve curve, vector<uint8_t>& payload)
{
    GenerateDppBootstrapInfoForResponderReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    msgSupplicantStaIface.set_deviceinfo(deviceInfo);
    msgSupplicantStaIface.set_curve((DppCurve_P) curve);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void DppResponderBootstrapInfo2Message(const DppResponderBootstrapInfo& result, DppResponderBootstrapInfo_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_bootstrapid(result.bootstrapId);
    msgSupplicantStaIface.set_listenchannel(result.listenChannel);
    msgSupplicantStaIface.set_uri(result.uri);
}

bool SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderCfm(const HalStatusParam& status, const DppResponderBootstrapInfo& result, vector<uint8_t>& payload)
{
    GenerateDppBootstrapInfoForResponderCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    DppResponderBootstrapInfo_P* result_p = msgSupplicantStaIface.mutable_result();
    DppResponderBootstrapInfo2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGenerateSelfDppConfigurationReq(const string& ssid, const vector<uint8_t>& privEcKey, vector<uint8_t>& payload)
{
    GenerateSelfDppConfigurationReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_ssid(ssid);
    string privEcKeyStr;
    Vector2String(privEcKey, privEcKeyStr);
    msgSupplicantStaIface.set_priveckey(privEcKeyStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGenerateSelfDppConfigurationCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetConnectionCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void ConnectionCapabilities2Message(const ConnectionCapabilities& result, ConnectionCapabilities_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_technology((WifiTechnology_P) result.technology);
    msgSupplicantStaIface.set_channelbandwidth(result.channelBandwidth);
    msgSupplicantStaIface.set_maxnumbertxspatialstreams(result.maxNumberTxSpatialStreams);
    msgSupplicantStaIface.set_maxnumberrxspatialstreams(result.maxNumberRxSpatialStreams);
    msgSupplicantStaIface.set_legacymode((LegacyMode_P) result.legacyMode);
    msgSupplicantStaIface.set_aptidtolinkmapnegotiationsupported(result.apTidToLinkMapNegotiationSupported);
}

bool SupplicantStaIfaceSerializeGetConnectionCapabilitiesCfm(const HalStatusParam& status, const ConnectionCapabilities& result, vector<uint8_t>& payload)
{
    GetConnectionCapabilitiesCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    ConnectionCapabilities_P* result_p = msgSupplicantStaIface.mutable_result();
    ConnectionCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetConnectionMloLinksInfoReq(vector<uint8_t>& payload)
{
    return true;
}

static void MloLink2Message(const MloLink& links, MloLink_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_linkid(links.linkId);
    string staLinkMacAddress;
    Vector2String(links.staLinkMacAddress, staLinkMacAddress);
    msgSupplicantStaIface.set_stalinkmacaddress(staLinkMacAddress);
    msgSupplicantStaIface.set_tidsuplinkmap(links.tidsUplinkMap);
    msgSupplicantStaIface.set_tidsdownlinkmap(links.tidsDownlinkMap);
    if (links.apLinkMacAddress.has_value())
    {
        string apLinkMacAddress;
        Array2String(links.apLinkMacAddress.value(), apLinkMacAddress);
        msgSupplicantStaIface.set_aplinkmacaddress(apLinkMacAddress);
    }
    msgSupplicantStaIface.set_frequencymhz(links.frequencyMHz);
}

static void MloLinksInfo2Message(const MloLinksInfo& result, MloLinksInfo_P& msgSupplicantStaIface)
{
    int size_links = result.links.size();
    if (0 < size_links)
    {
        for (int index = 0; index < size_links; index++)
        {
            MloLink_P *links_p = msgSupplicantStaIface.add_links();
            MloLink2Message(result.links[index], *links_p);
        }
    }
    msgSupplicantStaIface.set_apmlolinkid(result.apMloLinkId);
    if (result.apMldMacAddress.has_value())
    {
        string apMldMacAddress;
        Array2String(result.apMldMacAddress.value(), apMldMacAddress);
        msgSupplicantStaIface.set_apmldmacaddress(apMldMacAddress);
    }
}

bool SupplicantStaIfaceSerializeGetConnectionMloLinksInfoCfm(const HalStatusParam& status, const MloLinksInfo& result, vector<uint8_t>& payload)
{
    GetConnectionMloLinksInfoCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    MloLinksInfo_P* result_p = msgSupplicantStaIface.mutable_result();
    MloLinksInfo2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesCfm(const HalStatusParam& status, KeyMgmtMask result, vector<uint8_t>& payload)
{
    GetKeyMgmtCapabilitiesCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result((KeyMgmtMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetMacAddressReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetMacAddressCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetMacAddressCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetNetworkReq(int32_t id, vector<uint8_t>& payload)
{
    GetNetworkReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetNetworkCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantStaNetwork_P* result_p = msgSupplicantStaIface.mutable_result();
    ISupplicantStaNetwork2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetTypeReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload)
{
    GetTypeCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result((IfaceType_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesCfm(const HalStatusParam& status, WpaDriverCapabilitiesMask result, vector<uint8_t>& payload)
{
    GetWpaDriverCapabilitiesCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result((WpaDriverCapabilitiesMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateAnqpQueryReq(const vector<uint8_t>& macAddress, const vector<AnqpInfoId>& infoElements, const vector<Hs20AnqpSubtypes>& subTypes, vector<uint8_t>& payload)
{
    InitiateAnqpQueryReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    int size_infoelements = infoElements.size();
    if (0 < size_infoelements)
    {
        for (int index = 0; index < size_infoelements; index++)
        {
            msgSupplicantStaIface.add_infoelements((AnqpInfoId_P) infoElements[index]);
        }
    }
    int size_subtypes = subTypes.size();
    if (0 < size_subtypes)
    {
        for (int index = 0; index < size_subtypes; index++)
        {
            msgSupplicantStaIface.add_subtypes((Hs20AnqpSubtypes_P) subTypes[index]);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateAnqpQueryCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeInitiateHs20IconQueryReq(const vector<uint8_t>& macAddress, const string& fileName, vector<uint8_t>& payload)
{
    InitiateHs20IconQueryReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    msgSupplicantStaIface.set_filename(fileName);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateHs20IconQueryCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeInitiateTdlsDiscoverReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload)
{
    InitiateTdlsDiscoverReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateTdlsDiscoverCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeInitiateTdlsSetupReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload)
{
    InitiateTdlsSetupReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateTdlsSetupCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeInitiateTdlsTeardownReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload)
{
    InitiateTdlsTeardownReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateTdlsTeardownCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload)
{
    InitiateVenueUrlAnqpQueryReq msgSupplicantStaIface;
    string macAddressStr;
    Vector2String(macAddress, macAddressStr);
    msgSupplicantStaIface.set_macaddress(macAddressStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeListNetworksReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeListNetworksCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload)
{
    ListNetworksCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgSupplicantStaIface.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeReassociateReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeReassociateCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeReconnectReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeReconnectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRegisterCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRegisterCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledReq(bool enable, vector<uint8_t>& payload)
{
    SetQosPolicyFeatureEnabledReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledCfm(vector<uint8_t>& payload)
{
    return true;
}

static void QosPolicyStatus2Message(const QosPolicyStatus& qosPolicyStatusList, QosPolicyStatus_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_policyid(qosPolicyStatusList.policyId);
    msgSupplicantStaIface.set_status((QosPolicyStatusCode_P) qosPolicyStatusList.status);
}

bool SupplicantStaIfaceSerializeSendQosPolicyResponseReq(int32_t qosPolicyRequestId, bool morePolicies, const vector<QosPolicyStatus>& qosPolicyStatusList, vector<uint8_t>& payload)
{
    SendQosPolicyResponseReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_qospolicyrequestid(qosPolicyRequestId);
    msgSupplicantStaIface.set_morepolicies(morePolicies);
    int size_qospolicystatuslist = qosPolicyStatusList.size();
    if (0 < size_qospolicystatuslist)
    {
        for (int index = 0; index < size_qospolicystatuslist; index++)
        {
            QosPolicyStatus_P *qosPolicyStatusList_p = msgSupplicantStaIface.add_qospolicystatuslist();
            QosPolicyStatus2Message(qosPolicyStatusList[index], *qosPolicyStatusList_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSendQosPolicyResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveAllQosPoliciesReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveAllQosPoliciesCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveDppUriReq(int32_t id, vector<uint8_t>& payload)
{
    RemoveDppUriReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveDppUriCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveExtRadioWorkReq(int32_t id, vector<uint8_t>& payload)
{
    RemoveExtRadioWorkReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveExtRadioWorkCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveNetworkReq(int32_t id, vector<uint8_t>& payload)
{
    RemoveNetworkReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveNetworkCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeRemoveRxFilterReq(RxFilterType type, vector<uint8_t>& payload)
{
    RemoveRxFilterReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_type((RxFilterType_P) type);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveRxFilterCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetBtCoexistenceModeReq(BtCoexistenceMode mode, vector<uint8_t>& payload)
{
    SetBtCoexistenceModeReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_mode((BtCoexistenceMode_P) mode);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetBtCoexistenceModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledReq(bool enable, vector<uint8_t>& payload)
{
    SetBtCoexistenceScanModeEnabledReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetCountryCodeReq(const vector<uint8_t>& code, vector<uint8_t>& payload)
{
    SetCountryCodeReq msgSupplicantStaIface;
    string codeStr;
    Vector2String(code, codeStr);
    msgSupplicantStaIface.set_code(codeStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetCountryCodeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetExternalSimReq(bool useExternalSim, vector<uint8_t>& payload)
{
    SetExternalSimReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_useexternalsim(useExternalSim);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetExternalSimCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetMboCellularDataStatusReq(bool available, vector<uint8_t>& payload)
{
    SetMboCellularDataStatusReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_available(available);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetMboCellularDataStatusCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetPowerSaveReq(bool enable, vector<uint8_t>& payload)
{
    SetPowerSaveReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetPowerSaveCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetSuspendModeEnabledReq(bool enable, vector<uint8_t>& payload)
{
    SetSuspendModeEnabledReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetSuspendModeEnabledCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsConfigMethodsReq(WpsConfigMethods configMethods, vector<uint8_t>& payload)
{
    SetWpsConfigMethodsReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_configmethods((WpsConfigMethods_P) configMethods);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsConfigMethodsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsDeviceNameReq(const string& name, vector<uint8_t>& payload)
{
    SetWpsDeviceNameReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_name(name);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsDeviceNameCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsDeviceTypeReq(const vector<uint8_t>& type, vector<uint8_t>& payload)
{
    SetWpsDeviceTypeReq msgSupplicantStaIface;
    string typeStr;
    Vector2String(type, typeStr);
    msgSupplicantStaIface.set_type(typeStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsDeviceTypeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsManufacturerReq(const string& manufacturer, vector<uint8_t>& payload)
{
    SetWpsManufacturerReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_manufacturer(manufacturer);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsManufacturerCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsModelNameReq(const string& modelName, vector<uint8_t>& payload)
{
    SetWpsModelNameReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_modelname(modelName);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsModelNameCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsModelNumberReq(const string& modelNumber, vector<uint8_t>& payload)
{
    SetWpsModelNumberReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_modelnumber(modelNumber);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsModelNumberCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeSetWpsSerialNumberReq(const string& serialNumber, vector<uint8_t>& payload)
{
    SetWpsSerialNumberReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_serialnumber(serialNumber);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeSetWpsSerialNumberCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorReq(int32_t peerBootstrapId, int32_t ownBootstrapId, const string& ssid, const string& password, const string& psk, DppNetRole netRole, DppAkm securityAkm, const vector<uint8_t>& privEcKey, vector<uint8_t>& payload)
{
    StartDppConfiguratorInitiatorReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_peerbootstrapid(peerBootstrapId);
    msgSupplicantStaIface.set_ownbootstrapid(ownBootstrapId);
    msgSupplicantStaIface.set_ssid(ssid);
    msgSupplicantStaIface.set_password(password);
    msgSupplicantStaIface.set_psk(psk);
    msgSupplicantStaIface.set_netrole((DppNetRole_P) netRole);
    msgSupplicantStaIface.set_securityakm((DppAkm_P) securityAkm);
    string privEcKeyStr;
    Vector2String(privEcKey, privEcKeyStr);
    msgSupplicantStaIface.set_priveckey(privEcKeyStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    StartDppConfiguratorInitiatorCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorReq(int32_t peerBootstrapId, int32_t ownBootstrapId, vector<uint8_t>& payload)
{
    StartDppEnrolleeInitiatorReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_peerbootstrapid(peerBootstrapId);
    msgSupplicantStaIface.set_ownbootstrapid(ownBootstrapId);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartDppEnrolleeResponderReq(int32_t listenChannel, vector<uint8_t>& payload)
{
    StartDppEnrolleeResponderReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_listenchannel(listenChannel);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartDppEnrolleeResponderCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartRxFilterReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartRxFilterCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartWpsPbcReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    StartWpsPbcReq msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartWpsPbcCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartWpsPinDisplayReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    StartWpsPinDisplayReq msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartWpsPinDisplayCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    StartWpsPinDisplayCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartWpsPinKeypadReq(const string& pin, vector<uint8_t>& payload)
{
    StartWpsPinKeypadReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_pin(pin);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartWpsPinKeypadCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStartWpsRegistrarReq(const vector<uint8_t>& bssid, const string& pin, vector<uint8_t>& payload)
{
    StartWpsRegistrarReq msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_pin(pin);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStartWpsRegistrarCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStopDppInitiatorReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStopDppInitiatorCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStopDppResponderReq(int32_t ownBootstrapId, vector<uint8_t>& payload)
{
    StopDppResponderReq msgSupplicantStaIface;
    msgSupplicantStaIface.set_ownbootstrapid(ownBootstrapId);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeStopDppResponderCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStopRxFilterReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeStopRxFilterCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeGetSignalPollResultsReq(vector<uint8_t>& payload)
{
    return true;
}

static void SignalPollResult2Message(const SignalPollResult& result, SignalPollResult_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_linkid(result.linkId);
    msgSupplicantStaIface.set_currentrssidbm(result.currentRssiDbm);
    msgSupplicantStaIface.set_txbitratembps(result.txBitrateMbps);
    msgSupplicantStaIface.set_rxbitratembps(result.rxBitrateMbps);
    msgSupplicantStaIface.set_frequencymhz(result.frequencyMhz);
}

bool SupplicantStaIfaceSerializeGetSignalPollResultsCfm(const HalStatusParam& status, const vector<SignalPollResult>& result, vector<uint8_t>& payload)
{
    GetSignalPollResultsCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            SignalPollResult_P *result_p = msgSupplicantStaIface.add_result();
            SignalPollResult2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void PortRange2Message(const PortRange& dstPortRange, PortRange_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_startport(dstPortRange.startPort);
    msgSupplicantStaIface.set_endport(dstPortRange.endPort);
}

static void QosPolicyClassifierParams2Message(const QosPolicyClassifierParams& classifierParams, QosPolicyClassifierParams_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_ipversion((IpVersion_P) classifierParams.ipVersion);
    msgSupplicantStaIface.set_classifierparammask((QosPolicyClassifierParamsMask_P) classifierParams.classifierParamMask);
    string srcIp;
    Vector2String(classifierParams.srcIp, srcIp);
    msgSupplicantStaIface.set_srcip(srcIp);
    string dstIp;
    Vector2String(classifierParams.dstIp, dstIp);
    msgSupplicantStaIface.set_dstip(dstIp);
    msgSupplicantStaIface.set_srcport(classifierParams.srcPort);
    PortRange_P* dstPortRange_p = msgSupplicantStaIface.mutable_dstportrange();
    PortRange2Message(classifierParams.dstPortRange, *dstPortRange_p);
    msgSupplicantStaIface.set_protocolnexthdr((ProtocolNextHeader_P) classifierParams.protocolNextHdr);
    string flowLabelIpv6;
    Vector2String(classifierParams.flowLabelIpv6, flowLabelIpv6);
    msgSupplicantStaIface.set_flowlabelipv6(flowLabelIpv6);
    msgSupplicantStaIface.set_domainname(classifierParams.domainName);
    msgSupplicantStaIface.set_dscp(classifierParams.dscp);
}

static void QosPolicyScsData2Message(const QosPolicyScsData& qosPolicyData, QosPolicyScsData_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_policyid(qosPolicyData.policyId);
    msgSupplicantStaIface.set_userpriority(qosPolicyData.userPriority);
    QosPolicyClassifierParams_P* classifierParams_p = msgSupplicantStaIface.mutable_classifierparams();
    QosPolicyClassifierParams2Message(qosPolicyData.classifierParams, *classifierParams_p);
}

bool SupplicantStaIfaceSerializeAddQosPolicyRequestForScsReq(const vector<QosPolicyScsData>& qosPolicyData, vector<uint8_t>& payload)
{
    AddQosPolicyRequestForScsReq msgSupplicantStaIface;
    int size_qospolicydata = qosPolicyData.size();
    if (0 < size_qospolicydata)
    {
        for (int index = 0; index < size_qospolicydata; index++)
        {
            QosPolicyScsData_P *qosPolicyData_p = msgSupplicantStaIface.add_qospolicydata();
            QosPolicyScsData2Message(qosPolicyData[index], *qosPolicyData_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void QosPolicyScsRequestStatus2Message(const QosPolicyScsRequestStatus& result, QosPolicyScsRequestStatus_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_policyid(result.policyId);
    msgSupplicantStaIface.set_qospolicyscsrequeststatuscode((QosPolicyScsRequestStatusCode_P) result.qosPolicyScsRequestStatusCode);
}

bool SupplicantStaIfaceSerializeAddQosPolicyRequestForScsCfm(const HalStatusParam& status, const vector<QosPolicyScsRequestStatus>& result, vector<uint8_t>& payload)
{
    AddQosPolicyRequestForScsCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            QosPolicyScsRequestStatus_P *result_p = msgSupplicantStaIface.add_result();
            QosPolicyScsRequestStatus2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveQosPolicyForScsReq(const vector<uint8_t>& scsPolicyIds, vector<uint8_t>& payload)
{
    RemoveQosPolicyForScsReq msgSupplicantStaIface;
    string scsPolicyIdsStr;
    Vector2String(scsPolicyIds, scsPolicyIdsStr);
    msgSupplicantStaIface.set_scspolicyids(scsPolicyIdsStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeRemoveQosPolicyForScsCfm(const HalStatusParam& status, const vector<QosPolicyScsRequestStatus>& result, vector<uint8_t>& payload)
{
    RemoveQosPolicyForScsCfm msgSupplicantStaIface;
    HalStatus_P* status_p = msgSupplicantStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            QosPolicyScsRequestStatus_P *result_p = msgSupplicantStaIface.add_result();
            QosPolicyScsRequestStatus2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void AnqpData2Message(const AnqpData& data, AnqpData_P& msgSupplicantStaIface)
{
    string venueName;
    Vector2String(data.venueName, venueName);
    msgSupplicantStaIface.set_venuename(venueName);
    string roamingConsortium;
    Vector2String(data.roamingConsortium, roamingConsortium);
    msgSupplicantStaIface.set_roamingconsortium(roamingConsortium);
    string ipAddrTypeAvailability;
    Vector2String(data.ipAddrTypeAvailability, ipAddrTypeAvailability);
    msgSupplicantStaIface.set_ipaddrtypeavailability(ipAddrTypeAvailability);
    string naiRealm;
    Vector2String(data.naiRealm, naiRealm);
    msgSupplicantStaIface.set_nairealm(naiRealm);
    string anqp3gppCellularNetwork;
    Vector2String(data.anqp3gppCellularNetwork, anqp3gppCellularNetwork);
    msgSupplicantStaIface.set_anqp3gppcellularnetwork(anqp3gppCellularNetwork);
    string domainName;
    Vector2String(data.domainName, domainName);
    msgSupplicantStaIface.set_domainname(domainName);
    string venueUrl;
    Vector2String(data.venueUrl, venueUrl);
    msgSupplicantStaIface.set_venueurl(venueUrl);
}

static void Hs20AnqpData2Message(const Hs20AnqpData& hs20Data, Hs20AnqpData_P& msgSupplicantStaIface)
{
    string operatorFriendlyName;
    Vector2String(hs20Data.operatorFriendlyName, operatorFriendlyName);
    msgSupplicantStaIface.set_operatorfriendlyname(operatorFriendlyName);
    string wanMetrics;
    Vector2String(hs20Data.wanMetrics, wanMetrics);
    msgSupplicantStaIface.set_wanmetrics(wanMetrics);
    string connectionCapability;
    Vector2String(hs20Data.connectionCapability, connectionCapability);
    msgSupplicantStaIface.set_connectioncapability(connectionCapability);
    string osuProvidersList;
    Vector2String(hs20Data.osuProvidersList, osuProvidersList);
    msgSupplicantStaIface.set_osuproviderslist(osuProvidersList);
}

bool SupplicantStaIfaceSerializeOnAnqpQueryDoneInd(const vector<uint8_t>& bssid, const AnqpData& data, const Hs20AnqpData& hs20Data, vector<uint8_t>& payload)
{
    OnAnqpQueryDoneInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    AnqpData_P* data_p = msgSupplicantStaIface.mutable_data();
    AnqpData2Message(data, *data_p);
    Hs20AnqpData_P* hs20Data_p = msgSupplicantStaIface.mutable_hs20data();
    Hs20AnqpData2Message(hs20Data, *hs20Data_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void OceRssiBasedAssocRejectAttr2Message(const OceRssiBasedAssocRejectAttr& oceRssiBasedAssocRejectData, OceRssiBasedAssocRejectAttr_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_deltarssi(oceRssiBasedAssocRejectData.deltaRssi);
    msgSupplicantStaIface.set_retrydelays(oceRssiBasedAssocRejectData.retryDelayS);
}

static void AssociationRejectionData2Message(const AssociationRejectionData& assocRejectData, AssociationRejectionData_P& msgSupplicantStaIface)
{
    string ssid;
    Vector2String(assocRejectData.ssid, ssid);
    msgSupplicantStaIface.set_ssid(ssid);
    string bssid;
    Vector2String(assocRejectData.bssid, bssid);
    msgSupplicantStaIface.set_bssid(bssid);
    msgSupplicantStaIface.set_statuscode((StaIfaceStatusCode_P) assocRejectData.statusCode);
    msgSupplicantStaIface.set_timedout(assocRejectData.timedOut);
    msgSupplicantStaIface.set_ismboassocdisallowedreasoncodepresent(assocRejectData.isMboAssocDisallowedReasonCodePresent);
    msgSupplicantStaIface.set_mboassocdisallowedreason((MboAssocDisallowedReasonCode_P) assocRejectData.mboAssocDisallowedReason);
    msgSupplicantStaIface.set_isocerssibasedassocrejectattrpresent(assocRejectData.isOceRssiBasedAssocRejectAttrPresent);
    OceRssiBasedAssocRejectAttr_P* oceRssiBasedAssocRejectData_p = msgSupplicantStaIface.mutable_ocerssibasedassocrejectdata();
    OceRssiBasedAssocRejectAttr2Message(assocRejectData.oceRssiBasedAssocRejectData, *oceRssiBasedAssocRejectData_p);
}

bool SupplicantStaIfaceSerializeOnAssociationRejectedInd(const AssociationRejectionData& assocRejectData, vector<uint8_t>& payload)
{
    OnAssociationRejectedInd msgSupplicantStaIface;
    AssociationRejectionData_P* assocRejectData_p = msgSupplicantStaIface.mutable_assocrejectdata();
    AssociationRejectionData2Message(assocRejectData, *assocRejectData_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnAuthenticationTimeoutInd(const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    OnAuthenticationTimeoutInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnAuxiliarySupplicantEventInd(AuxiliarySupplicantEventCode eventCode, const vector<uint8_t>& bssid, const string& reasonString, vector<uint8_t>& payload)
{
    OnAuxiliarySupplicantEventInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_eventcode((AuxiliarySupplicantEventCode_P) eventCode);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_reasonstring(reasonString);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void BssTmData2Message(const BssTmData& tmData, BssTmData_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_status((BssTmStatusCode_P) tmData.status);
    msgSupplicantStaIface.set_flags((BssTmDataFlagsMask_P) tmData.flags);
    msgSupplicantStaIface.set_assocretrydelayms(tmData.assocRetryDelayMs);
    msgSupplicantStaIface.set_mbotransitionreason((MboTransitionReasonCode_P) tmData.mboTransitionReason);
    msgSupplicantStaIface.set_mbocellpreference((MboCellularDataConnectionPrefValue_P) tmData.mboCellPreference);
}

bool SupplicantStaIfaceSerializeOnBssTmHandlingDoneInd(const BssTmData& tmData, vector<uint8_t>& payload)
{
    OnBssTmHandlingDoneInd msgSupplicantStaIface;
    BssTmData_P* tmData_p = msgSupplicantStaIface.mutable_tmdata();
    BssTmData2Message(tmData, *tmData_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnBssidChangedInd(BssidChangeReason reason, const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    OnBssidChangedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_reason((BssidChangeReason_P) reason);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDisconnectedInd(const vector<uint8_t>& bssid, bool locallyGenerated, StaIfaceReasonCode reasonCode, vector<uint8_t>& payload)
{
    OnDisconnectedInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_locallygenerated(locallyGenerated);
    msgSupplicantStaIface.set_reasoncode((StaIfaceReasonCode_P) reasonCode);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDppFailureInd(DppFailureCode code, const string& ssid, const string& channelList, const vector<char16_t>& bandList, vector<uint8_t>& payload)
{
    OnDppFailureInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_code((DppFailureCode_P) code);
    msgSupplicantStaIface.set_ssid(ssid);
    msgSupplicantStaIface.set_channellist(channelList);
    int size_bandlist = bandList.size();
    if (0 < size_bandlist)
    {
        for (int index = 0; index < size_bandlist; index++)
        {
            msgSupplicantStaIface.add_bandlist(bandList[index]);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDppProgressInd(DppProgressCode code, vector<uint8_t>& payload)
{
    OnDppProgressInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_code((DppProgressCode_P) code);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDppSuccessInd(DppEventType event, vector<uint8_t>& payload)
{
    OnDppSuccessInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_event((DppEventType_P) event);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void DppConnectionKeys2Message(const DppConnectionKeys& dppConnectionKeys, DppConnectionKeys_P& msgSupplicantStaIface)
{
    string connector;
    Vector2String(dppConnectionKeys.connector, connector);
    msgSupplicantStaIface.set_connector(connector);
    string cSign;
    Vector2String(dppConnectionKeys.cSign, cSign);
    msgSupplicantStaIface.set_csign(cSign);
    string netAccessKey;
    Vector2String(dppConnectionKeys.netAccessKey, netAccessKey);
    msgSupplicantStaIface.set_netaccesskey(netAccessKey);
}

bool SupplicantStaIfaceSerializeOnDppSuccessConfigReceivedInd(const vector<uint8_t>& ssid, const string& password, const vector<uint8_t>& psk, DppAkm securityAkm, const DppConnectionKeys& dppConnectionKeys, vector<uint8_t>& payload)
{
    OnDppSuccessConfigReceivedInd msgSupplicantStaIface;
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantStaIface.set_ssid(ssidStr);
    msgSupplicantStaIface.set_password(password);
    string pskStr;
    Vector2String(psk, pskStr);
    msgSupplicantStaIface.set_psk(pskStr);
    msgSupplicantStaIface.set_securityakm((DppAkm_P) securityAkm);
    DppConnectionKeys_P* dppConnectionKeys_p = msgSupplicantStaIface.mutable_dppconnectionkeys();
    DppConnectionKeys2Message(dppConnectionKeys, *dppConnectionKeys_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDppSuccessConfigSentInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeOnEapFailureInd(const vector<uint8_t>& bssid, int32_t errorCode, vector<uint8_t>& payload)
{
    OnEapFailureInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_errorcode(errorCode);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnExtRadioWorkStartInd(int32_t id, vector<uint8_t>& payload)
{
    OnExtRadioWorkStartInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnExtRadioWorkTimeoutInd(int32_t id, vector<uint8_t>& payload)
{
    OnExtRadioWorkTimeoutInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnHs20DeauthImminentNoticeInd(const vector<uint8_t>& bssid, int32_t reasonCode, int32_t reAuthDelayInSec, const string& url, vector<uint8_t>& payload)
{
    OnHs20DeauthImminentNoticeInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_reasoncode(reasonCode);
    msgSupplicantStaIface.set_reauthdelayinsec(reAuthDelayInSec);
    msgSupplicantStaIface.set_url(url);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnHs20IconQueryDoneInd(const vector<uint8_t>& bssid, const string& fileName, const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    OnHs20IconQueryDoneInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_filename(fileName);
    string dataStr;
    Vector2String(data, dataStr);
    msgSupplicantStaIface.set_data(dataStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnHs20SubscriptionRemediationInd(const vector<uint8_t>& bssid, OsuMethod osuMethod, const string& url, vector<uint8_t>& payload)
{
    OnHs20SubscriptionRemediationInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_osumethod((OsuMethod_P) osuMethod);
    msgSupplicantStaIface.set_url(url);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(const vector<uint8_t>& bssid, const string& url, vector<uint8_t>& payload)
{
    OnHs20TermsAndConditionsAcceptanceRequestedNotificationInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_url(url);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnNetworkAddedInd(int32_t id, vector<uint8_t>& payload)
{
    OnNetworkAddedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnNetworkNotFoundInd(const vector<uint8_t>& ssid, vector<uint8_t>& payload)
{
    OnNetworkNotFoundInd msgSupplicantStaIface;
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantStaIface.set_ssid(ssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnNetworkRemovedInd(int32_t id, vector<uint8_t>& payload)
{
    OnNetworkRemovedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnPmkCacheAddedInd(int64_t expirationTimeInSec, const vector<uint8_t>& serializedEntry, vector<uint8_t>& payload)
{
    OnPmkCacheAddedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_expirationtimeinsec(expirationTimeInSec);
    string serializedEntryStr;
    Vector2String(serializedEntry, serializedEntryStr);
    msgSupplicantStaIface.set_serializedentry(serializedEntryStr);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnStateChangedInd(StaIfaceCallbackState newState, const vector<uint8_t>& bssid, int32_t id, const vector<uint8_t>& ssid, bool filsHlpSent, vector<uint8_t>& payload)
{
    OnStateChangedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_newstate((StaIfaceCallbackState_P) newState);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_id(id);
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantStaIface.set_ssid(ssidStr);
    msgSupplicantStaIface.set_filshlpsent(filsHlpSent);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnWpsEventFailInd(const vector<uint8_t>& bssid, WpsConfigError configError, WpsErrorIndication errorInd, vector<uint8_t>& payload)
{
    OnWpsEventFailInd msgSupplicantStaIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaIface.set_bssid(bssidStr);
    msgSupplicantStaIface.set_configerror((WpsConfigError_P) configError);
    msgSupplicantStaIface.set_errorind((WpsErrorIndication_P) errorInd);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnWpsEventPbcOverlapInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeOnWpsEventSuccessInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaIfaceSerializeOnQosPolicyResetInd(vector<uint8_t>& payload)
{
    return true;
}

static void QosPolicyData2Message(const QosPolicyData& qosPolicyData, QosPolicyData_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_policyid(qosPolicyData.policyId);
    msgSupplicantStaIface.set_requesttype((QosPolicyRequestType_P) qosPolicyData.requestType);
    msgSupplicantStaIface.set_dscp(qosPolicyData.dscp);
    QosPolicyClassifierParams_P* classifierParams_p = msgSupplicantStaIface.mutable_classifierparams();
    QosPolicyClassifierParams2Message(qosPolicyData.classifierParams, *classifierParams_p);
}

bool SupplicantStaIfaceSerializeOnQosPolicyRequestInd(int32_t qosPolicyRequestId, const vector<QosPolicyData>& qosPolicyData, vector<uint8_t>& payload)
{
    OnQosPolicyRequestInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_qospolicyrequestid(qosPolicyRequestId);
    int size_qospolicydata = qosPolicyData.size();
    if (0 < size_qospolicydata)
    {
        for (int index = 0; index < size_qospolicydata; index++)
        {
            QosPolicyData_P *qosPolicyData_p = msgSupplicantStaIface.add_qospolicydata();
            QosPolicyData2Message(qosPolicyData[index], *qosPolicyData_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnMloLinksInfoChangedInd(ISupplicantStaIfaceCallback::MloLinkInfoChangeReason reason, vector<uint8_t>& payload)
{
    OnMloLinksInfoChangedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_reason((MloLinkInfoChangeReason_P) reason);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void DppConfigurationData2Message(const DppConfigurationData& configData, DppConfigurationData_P& msgSupplicantStaIface)
{
    string ssid;
    Vector2String(configData.ssid, ssid);
    msgSupplicantStaIface.set_ssid(ssid);
    msgSupplicantStaIface.set_password(configData.password);
    string psk;
    Vector2String(configData.psk, psk);
    msgSupplicantStaIface.set_psk(psk);
    msgSupplicantStaIface.set_securityakm((DppAkm_P) configData.securityAkm);
    DppConnectionKeys_P* dppConnectionKeys_p = msgSupplicantStaIface.mutable_dppconnectionkeys();
    DppConnectionKeys2Message(configData.dppConnectionKeys, *dppConnectionKeys_p);
    msgSupplicantStaIface.set_connstatusrequested(configData.connStatusRequested);
}

bool SupplicantStaIfaceSerializeOnDppConfigReceivedInd(const DppConfigurationData& configData, vector<uint8_t>& payload)
{
    OnDppConfigReceivedInd msgSupplicantStaIface;
    DppConfigurationData_P* configData_p = msgSupplicantStaIface.mutable_configdata();
    DppConfigurationData2Message(configData, *configData_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnDppConnectionStatusResultSentInd(DppStatusErrorCode code, vector<uint8_t>& payload)
{
    OnDppConnectionStatusResultSentInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_code((DppStatusErrorCode_P) code);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceSerializeOnBssFrequencyChangedInd(int32_t frequencyMhz, vector<uint8_t>& payload)
{
    OnBssFrequencyChangedInd msgSupplicantStaIface;
    msgSupplicantStaIface.set_frequencymhz(frequencyMhz);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void SupplicantStateChangeData2Message(const SupplicantStateChangeData& stateChangeData, SupplicantStateChangeData_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_newstate((StaIfaceCallbackState_P) stateChangeData.newState);
    msgSupplicantStaIface.set_id(stateChangeData.id);
    string ssid;
    Vector2String(stateChangeData.ssid, ssid);
    msgSupplicantStaIface.set_ssid(ssid);
    string bssid;
    Array2String(stateChangeData.bssid, bssid);
    msgSupplicantStaIface.set_bssid(bssid);
    msgSupplicantStaIface.set_keymgmtmask((KeyMgmtMask_P) stateChangeData.keyMgmtMask);
    msgSupplicantStaIface.set_frequencymhz(stateChangeData.frequencyMhz);
    msgSupplicantStaIface.set_filshlpsent(stateChangeData.filsHlpSent);
}

bool SupplicantStaIfaceSerializeOnSupplicantStateChangedInd(const SupplicantStateChangeData& stateChangeData, vector<uint8_t>& payload)
{
    OnSupplicantStateChangedInd msgSupplicantStaIface;
    SupplicantStateChangeData_P* stateChangeData_p = msgSupplicantStaIface.mutable_statechangedata();
    SupplicantStateChangeData2Message(stateChangeData, *stateChangeData_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void QosPolicyScsResponseStatus2Message(const QosPolicyScsResponseStatus& qosPolicyScsResponseStatus, QosPolicyScsResponseStatus_P& msgSupplicantStaIface)
{
    msgSupplicantStaIface.set_policyid(qosPolicyScsResponseStatus.policyId);
    msgSupplicantStaIface.set_qospolicyscsresponsestatuscode((QosPolicyScsResponseStatusCode_P) qosPolicyScsResponseStatus.qosPolicyScsResponseStatusCode);
}

bool SupplicantStaIfaceSerializeOnQosPolicyResponseForScsInd(const vector<QosPolicyScsResponseStatus>& qosPolicyScsResponseStatus, vector<uint8_t>& payload)
{
    OnQosPolicyResponseForScsInd msgSupplicantStaIface;
    int size_qospolicyscsresponsestatus = qosPolicyScsResponseStatus.size();
    if (0 < size_qospolicyscsresponsestatus)
    {
        for (int index = 0; index < size_qospolicyscsresponsestatus; index++)
        {
            QosPolicyScsResponseStatus_P *qosPolicyScsResponseStatus_p = msgSupplicantStaIface.add_qospolicyscsresponsestatus();
            QosPolicyScsResponseStatus2Message(qosPolicyScsResponseStatus[index], *qosPolicyScsResponseStatus_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

static void PmkSaCacheData2Message(const PmkSaCacheData& pmkSaData, PmkSaCacheData_P& msgSupplicantStaIface)
{
    string bssid;
    Array2String(pmkSaData.bssid, bssid);
    msgSupplicantStaIface.set_bssid(bssid);
    msgSupplicantStaIface.set_expirationtimeinsec(pmkSaData.expirationTimeInSec);
    string serializedEntry;
    Vector2String(pmkSaData.serializedEntry, serializedEntry);
    msgSupplicantStaIface.set_serializedentry(serializedEntry);
}

bool SupplicantStaIfaceSerializeOnPmkSaCacheAddedInd(const PmkSaCacheData& pmkSaData, vector<uint8_t>& payload)
{
    OnPmkSaCacheAddedInd msgSupplicantStaIface;
    PmkSaCacheData_P* pmkSaData_p = msgSupplicantStaIface.mutable_pmksadata();
    PmkSaCacheData2Message(pmkSaData, *pmkSaData_p);
    return ProtoMessageSerialize(&msgSupplicantStaIface, payload);
}

bool SupplicantStaIfaceParseAddDppPeerUriReq(const uint8_t* data, size_t length, string& param)
{
    AddDppPeerUriReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.uri();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantStaIface, HalStatusParam& status)
{
    status.status = msgSupplicantStaIface.status();
    status.info = msgSupplicantStaIface.info();
}

bool SupplicantStaIfaceParseAddDppPeerUriCfm(const uint8_t* data, size_t length, AddDppPeerUriCfmStaIfaceParam& param)
{
    AddDppPeerUriCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseAddExtRadioWorkReq(const uint8_t* data, size_t length, AddExtRadioWorkReqStaIfaceParam& param)
{
    AddExtRadioWorkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.name = msg.name();
        param.freqInMhz = msg.freqinmhz();
        param.timeoutInSec = msg.timeoutinsec();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseAddExtRadioWorkCfm(const uint8_t* data, size_t length, AddExtRadioWorkCfmStaIfaceParam& param)
{
    AddExtRadioWorkCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseAddNetworkReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ISupplicantStaNetwork(const ISupplicantStaNetwork_P& msgSupplicantStaIface, int32_t& result)
{
    result = msgSupplicantStaIface.instanceid();
}

bool SupplicantStaIfaceParseAddNetworkCfm(const uint8_t* data, size_t length, AddNetworkCfmStaIfaceParam& param)
{
    AddNetworkCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantStaNetwork(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseAddRxFilterReq(const uint8_t* data, size_t length, RxFilterType& param)
{
    AddRxFilterReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (RxFilterType) msg.type();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseAddRxFilterCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseCancelWpsReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseCancelWpsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseDisconnectReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseDisconnectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseEnableAutoReconnectReq(const uint8_t* data, size_t length, bool& param)
{
    EnableAutoReconnectReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseEnableAutoReconnectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseFilsHlpAddRequestReq(const uint8_t* data, size_t length, FilsHlpAddRequestReqStaIfaceParam& param)
{
    FilsHlpAddRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.dst_mac(), param.dst_mac);
        String2Vector(msg.pkt(), param.pkt);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseFilsHlpAddRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseFilsHlpFlushRequestReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseFilsHlpFlushRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderReq(const uint8_t* data, size_t length, GenerateDppBootstrapInfoForResponderReqStaIfaceParam& param)
{
    GenerateDppBootstrapInfoForResponderReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param.macAddress);
        param.deviceInfo = msg.deviceinfo();
        param.curve = (DppCurve) msg.curve();
        return true;
    }
    return false;
}

static void Message2DppResponderBootstrapInfo(const DppResponderBootstrapInfo_P& msgSupplicantStaIface, DppResponderBootstrapInfo& result)
{
    result.bootstrapId = msgSupplicantStaIface.bootstrapid();
    result.listenChannel = msgSupplicantStaIface.listenchannel();
    result.uri = msgSupplicantStaIface.uri();
}

bool SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderCfm(const uint8_t* data, size_t length, GenerateDppBootstrapInfoForResponderCfmStaIfaceParam& param)
{
    GenerateDppBootstrapInfoForResponderCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2DppResponderBootstrapInfo(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGenerateSelfDppConfigurationReq(const uint8_t* data, size_t length, GenerateSelfDppConfigurationReqStaIfaceParam& param)
{
    GenerateSelfDppConfigurationReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ssid = msg.ssid();
        String2Vector(msg.priveckey(), param.privEcKey);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGenerateSelfDppConfigurationCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetConnectionCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ConnectionCapabilities(const ConnectionCapabilities_P& msgSupplicantStaIface, ConnectionCapabilities& result)
{
    result.technology = (WifiTechnology) msgSupplicantStaIface.technology();
    result.channelBandwidth = msgSupplicantStaIface.channelbandwidth();
    result.maxNumberTxSpatialStreams = msgSupplicantStaIface.maxnumbertxspatialstreams();
    result.maxNumberRxSpatialStreams = msgSupplicantStaIface.maxnumberrxspatialstreams();
    result.legacyMode = (LegacyMode) msgSupplicantStaIface.legacymode();
    result.apTidToLinkMapNegotiationSupported = msgSupplicantStaIface.aptidtolinkmapnegotiationsupported();
}

bool SupplicantStaIfaceParseGetConnectionCapabilitiesCfm(const uint8_t* data, size_t length, GetConnectionCapabilitiesCfmStaIfaceParam& param)
{
    GetConnectionCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ConnectionCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetConnectionMloLinksInfoReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2MloLink(const MloLink_P& msgSupplicantStaIface, MloLink& links)
{
    links.linkId = msgSupplicantStaIface.linkid();
    String2Vector(msgSupplicantStaIface.stalinkmacaddress(), links.staLinkMacAddress);
    links.tidsUplinkMap = msgSupplicantStaIface.tidsuplinkmap();
    links.tidsDownlinkMap = msgSupplicantStaIface.tidsdownlinkmap();
    array<uint8_t, 6> param;
    String2Array(msgSupplicantStaIface.aplinkmacaddress(), param);
    links.apLinkMacAddress = param;
    links.frequencyMHz = msgSupplicantStaIface.frequencymhz();
}

static void Message2MloLinksInfo(const MloLinksInfo_P& msgSupplicantStaIface, MloLinksInfo& result)
{
    int size_links = msgSupplicantStaIface.links_size();
    if (0 < size_links)
    {
        result.links.resize(size_links);
        for (int index = 0; index < size_links; index++)
        {
            Message2MloLink(msgSupplicantStaIface.links(index), result.links[index]);
        }
    }
    result.apMloLinkId = msgSupplicantStaIface.apmlolinkid();
    array<uint8_t, 6> param;
    String2Array(msgSupplicantStaIface.apmldmacaddress(), param);
    result.apMldMacAddress = param;
}

bool SupplicantStaIfaceParseGetConnectionMloLinksInfoCfm(const uint8_t* data, size_t length, GetConnectionMloLinksInfoCfmStaIfaceParam& param)
{
    GetConnectionMloLinksInfoCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2MloLinksInfo(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetKeyMgmtCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetKeyMgmtCapabilitiesCfm(const uint8_t* data, size_t length, GetKeyMgmtCapabilitiesCfmStaIfaceParam& param)
{
    GetKeyMgmtCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (KeyMgmtMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetMacAddressReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetMacAddressCfm(const uint8_t* data, size_t length, GetMacAddressCfmStaIfaceParam& param)
{
    GetMacAddressCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmStaIfaceParam& param)
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

bool SupplicantStaIfaceParseGetNetworkReq(const uint8_t* data, size_t length, int32_t& param)
{
    GetNetworkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetNetworkCfm(const uint8_t* data, size_t length, GetNetworkCfmStaIfaceParam& param)
{
    GetNetworkCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantStaNetwork(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetTypeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmStaIfaceParam& param)
{
    GetTypeCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (IfaceType) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseGetWpaDriverCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetWpaDriverCapabilitiesCfm(const uint8_t* data, size_t length, GetWpaDriverCapabilitiesCfmStaIfaceParam& param)
{
    GetWpaDriverCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (WpaDriverCapabilitiesMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateAnqpQueryReq(const uint8_t* data, size_t length, InitiateAnqpQueryReqStaIfaceParam& param)
{
    InitiateAnqpQueryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param.macAddress);
        int size_infoelements = msg.infoelements_size();
        if (0 < size_infoelements)
        {
            param.infoElements.resize(size_infoelements);
            for (int index = 0; index < size_infoelements; index++)
            {
                param.infoElements[index] = (AnqpInfoId) msg.infoelements(index);
            }
        }
        int size_subtypes = msg.subtypes_size();
        if (0 < size_subtypes)
        {
            param.subTypes.resize(size_subtypes);
            for (int index = 0; index < size_subtypes; index++)
            {
                param.subTypes[index] = (Hs20AnqpSubtypes) msg.subtypes(index);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateAnqpQueryCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseInitiateHs20IconQueryReq(const uint8_t* data, size_t length, InitiateHs20IconQueryReqStaIfaceParam& param)
{
    InitiateHs20IconQueryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param.macAddress);
        param.fileName = msg.filename();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateHs20IconQueryCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseInitiateTdlsDiscoverReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    InitiateTdlsDiscoverReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateTdlsDiscoverCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseInitiateTdlsSetupReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    InitiateTdlsSetupReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateTdlsSetupCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseInitiateTdlsTeardownReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    InitiateTdlsTeardownReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateTdlsTeardownCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    InitiateVenueUrlAnqpQueryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.macaddress(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseListNetworksReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseListNetworksCfm(const uint8_t* data, size_t length, ListNetworksCfmStaIfaceParam& param)
{
    ListNetworksCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseReassociateReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseReassociateCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseReconnectReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseReconnectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRegisterCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRegisterCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetQosPolicyFeatureEnabledReq(const uint8_t* data, size_t length, bool& param)
{
    SetQosPolicyFeatureEnabledReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetQosPolicyFeatureEnabledCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2QosPolicyStatus(const QosPolicyStatus_P& msgSupplicantStaIface, QosPolicyStatus& qosPolicyStatusList)
{
    qosPolicyStatusList.policyId = msgSupplicantStaIface.policyid();
    qosPolicyStatusList.status = (QosPolicyStatusCode) msgSupplicantStaIface.status();
}

bool SupplicantStaIfaceParseSendQosPolicyResponseReq(const uint8_t* data, size_t length, SendQosPolicyResponseReqStaIfaceParam& param)
{
    SendQosPolicyResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.qosPolicyRequestId = msg.qospolicyrequestid();
        param.morePolicies = msg.morepolicies();
        int size_qospolicystatuslist = msg.qospolicystatuslist_size();
        if (0 < size_qospolicystatuslist)
        {
            param.qosPolicyStatusList.resize(size_qospolicystatuslist);
            for (int index = 0; index < size_qospolicystatuslist; index++)
            {
                Message2QosPolicyStatus(msg.qospolicystatuslist(index), param.qosPolicyStatusList[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSendQosPolicyResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveAllQosPoliciesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveAllQosPoliciesCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveDppUriReq(const uint8_t* data, size_t length, int32_t& param)
{
    RemoveDppUriReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveDppUriCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveExtRadioWorkReq(const uint8_t* data, size_t length, int32_t& param)
{
    RemoveExtRadioWorkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveExtRadioWorkCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveNetworkReq(const uint8_t* data, size_t length, int32_t& param)
{
    RemoveNetworkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveNetworkCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseRemoveRxFilterReq(const uint8_t* data, size_t length, RxFilterType& param)
{
    RemoveRxFilterReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (RxFilterType) msg.type();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveRxFilterCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetBtCoexistenceModeReq(const uint8_t* data, size_t length, BtCoexistenceMode& param)
{
    SetBtCoexistenceModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (BtCoexistenceMode) msg.mode();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetBtCoexistenceModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledReq(const uint8_t* data, size_t length, bool& param)
{
    SetBtCoexistenceScanModeEnabledReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetCountryCodeReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetCountryCodeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.code(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetCountryCodeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetExternalSimReq(const uint8_t* data, size_t length, bool& param)
{
    SetExternalSimReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.useexternalsim();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetExternalSimCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetMboCellularDataStatusReq(const uint8_t* data, size_t length, bool& param)
{
    SetMboCellularDataStatusReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.available();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetMboCellularDataStatusCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetPowerSaveReq(const uint8_t* data, size_t length, bool& param)
{
    SetPowerSaveReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetPowerSaveCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetSuspendModeEnabledReq(const uint8_t* data, size_t length, bool& param)
{
    SetSuspendModeEnabledReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetSuspendModeEnabledCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsConfigMethodsReq(const uint8_t* data, size_t length, WpsConfigMethods& param)
{
    SetWpsConfigMethodsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (WpsConfigMethods) msg.configmethods();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsConfigMethodsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsDeviceNameReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsDeviceNameReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.name();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsDeviceNameCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsDeviceTypeReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetWpsDeviceTypeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.type(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsDeviceTypeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsManufacturerReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsManufacturerReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.manufacturer();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsManufacturerCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsModelNameReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsModelNameReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modelname();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsModelNameCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsModelNumberReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsModelNumberReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modelnumber();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsModelNumberCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseSetWpsSerialNumberReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsSerialNumberReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.serialnumber();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseSetWpsSerialNumberCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartDppConfiguratorInitiatorReq(const uint8_t* data, size_t length, StartDppConfiguratorInitiatorReqStaIfaceParam& param)
{
    StartDppConfiguratorInitiatorReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.peerBootstrapId = msg.peerbootstrapid();
        param.ownBootstrapId = msg.ownbootstrapid();
        param.ssid = msg.ssid();
        param.password = msg.password();
        param.psk = msg.psk();
        param.netRole = (DppNetRole) msg.netrole();
        param.securityAkm = (DppAkm) msg.securityakm();
        String2Vector(msg.priveckey(), param.privEcKey);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartDppConfiguratorInitiatorCfm(const uint8_t* data, size_t length, StartDppConfiguratorInitiatorCfmStaIfaceParam& param)
{
    StartDppConfiguratorInitiatorCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartDppEnrolleeInitiatorReq(const uint8_t* data, size_t length, StartDppEnrolleeInitiatorReqStaIfaceParam& param)
{
    StartDppEnrolleeInitiatorReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.peerBootstrapId = msg.peerbootstrapid();
        param.ownBootstrapId = msg.ownbootstrapid();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartDppEnrolleeInitiatorCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartDppEnrolleeResponderReq(const uint8_t* data, size_t length, int32_t& param)
{
    StartDppEnrolleeResponderReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.listenchannel();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartDppEnrolleeResponderCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartRxFilterReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartRxFilterCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartWpsPbcReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    StartWpsPbcReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartWpsPbcCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartWpsPinDisplayReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    StartWpsPinDisplayReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartWpsPinDisplayCfm(const uint8_t* data, size_t length, StartWpsPinDisplayCfmStaIfaceParam& param)
{
    StartWpsPinDisplayCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartWpsPinKeypadReq(const uint8_t* data, size_t length, string& param)
{
    StartWpsPinKeypadReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.pin();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartWpsPinKeypadCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStartWpsRegistrarReq(const uint8_t* data, size_t length, StartWpsRegistrarReqStaIfaceParam& param)
{
    StartWpsRegistrarReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.pin = msg.pin();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStartWpsRegistrarCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStopDppInitiatorReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStopDppInitiatorCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStopDppResponderReq(const uint8_t* data, size_t length, int32_t& param)
{
    StopDppResponderReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ownbootstrapid();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseStopDppResponderCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStopRxFilterReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseStopRxFilterCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseGetSignalPollResultsReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2SignalPollResult(const SignalPollResult_P& msgSupplicantStaIface, SignalPollResult& result)
{
    result.linkId = msgSupplicantStaIface.linkid();
    result.currentRssiDbm = msgSupplicantStaIface.currentrssidbm();
    result.txBitrateMbps = msgSupplicantStaIface.txbitratembps();
    result.rxBitrateMbps = msgSupplicantStaIface.rxbitratembps();
    result.frequencyMhz = msgSupplicantStaIface.frequencymhz();
}

bool SupplicantStaIfaceParseGetSignalPollResultsCfm(const uint8_t* data, size_t length, GetSignalPollResultsCfmStaIfaceParam& param)
{
    GetSignalPollResultsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2SignalPollResult(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

static void Message2PortRange(const PortRange_P& msgSupplicantStaIface, PortRange& dstPortRange)
{
    dstPortRange.startPort = msgSupplicantStaIface.startport();
    dstPortRange.endPort = msgSupplicantStaIface.endport();
}

static void Message2QosPolicyClassifierParams(const QosPolicyClassifierParams_P& msgSupplicantStaIface, QosPolicyClassifierParams& classifierParams)
{
    classifierParams.ipVersion = (IpVersion) msgSupplicantStaIface.ipversion();
    classifierParams.classifierParamMask = (QosPolicyClassifierParamsMask) msgSupplicantStaIface.classifierparammask();
    String2Vector(msgSupplicantStaIface.srcip(), classifierParams.srcIp);
    String2Vector(msgSupplicantStaIface.dstip(), classifierParams.dstIp);
    classifierParams.srcPort = msgSupplicantStaIface.srcport();
    Message2PortRange(msgSupplicantStaIface.dstportrange(), classifierParams.dstPortRange);
    classifierParams.protocolNextHdr = (ProtocolNextHeader) msgSupplicantStaIface.protocolnexthdr();
    String2Vector(msgSupplicantStaIface.flowlabelipv6(), classifierParams.flowLabelIpv6);
    classifierParams.domainName = msgSupplicantStaIface.domainname();
    classifierParams.dscp = msgSupplicantStaIface.dscp();
}

static void Message2QosPolicyScsData(const QosPolicyScsData_P& msgSupplicantStaIface, QosPolicyScsData& qosPolicyData)
{
    qosPolicyData.policyId = msgSupplicantStaIface.policyid();
    qosPolicyData.userPriority = msgSupplicantStaIface.userpriority();
    Message2QosPolicyClassifierParams(msgSupplicantStaIface.classifierparams(), qosPolicyData.classifierParams);
}

bool SupplicantStaIfaceParseAddQosPolicyRequestForScsReq(const uint8_t* data, size_t length, vector<QosPolicyScsData>& param)
{
    AddQosPolicyRequestForScsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_qospolicydata = msg.qospolicydata_size();
        if (0 < size_qospolicydata)
        {
            param.resize(size_qospolicydata);
            for (int index = 0; index < size_qospolicydata; index++)
            {
                Message2QosPolicyScsData(msg.qospolicydata(index), param[index]);
            }
        }
        return true;
    }
    return false;
}

static void Message2QosPolicyScsRequestStatus(const QosPolicyScsRequestStatus_P& msgSupplicantStaIface, QosPolicyScsRequestStatus& result)
{
    result.policyId = msgSupplicantStaIface.policyid();
    result.qosPolicyScsRequestStatusCode = (QosPolicyScsRequestStatusCode) msgSupplicantStaIface.qospolicyscsrequeststatuscode();
}

bool SupplicantStaIfaceParseAddQosPolicyRequestForScsCfm(const uint8_t* data, size_t length, AddQosPolicyRequestForScsCfmStaIfaceParam& param)
{
    AddQosPolicyRequestForScsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2QosPolicyScsRequestStatus(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveQosPolicyForScsReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    RemoveQosPolicyForScsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.scspolicyids(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseRemoveQosPolicyForScsCfm(const uint8_t* data, size_t length, RemoveQosPolicyForScsCfmStaIfaceParam& param)
{
    RemoveQosPolicyForScsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2QosPolicyScsRequestStatus(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

static void Message2AnqpData(const AnqpData_P& msgSupplicantStaIface, AnqpData& data)
{
    String2Vector(msgSupplicantStaIface.venuename(), data.venueName);
    String2Vector(msgSupplicantStaIface.roamingconsortium(), data.roamingConsortium);
    String2Vector(msgSupplicantStaIface.ipaddrtypeavailability(), data.ipAddrTypeAvailability);
    String2Vector(msgSupplicantStaIface.nairealm(), data.naiRealm);
    String2Vector(msgSupplicantStaIface.anqp3gppcellularnetwork(), data.anqp3gppCellularNetwork);
    String2Vector(msgSupplicantStaIface.domainname(), data.domainName);
    String2Vector(msgSupplicantStaIface.venueurl(), data.venueUrl);
}

static void Message2Hs20AnqpData(const Hs20AnqpData_P& msgSupplicantStaIface, Hs20AnqpData& hs20Data)
{
    String2Vector(msgSupplicantStaIface.operatorfriendlyname(), hs20Data.operatorFriendlyName);
    String2Vector(msgSupplicantStaIface.wanmetrics(), hs20Data.wanMetrics);
    String2Vector(msgSupplicantStaIface.connectioncapability(), hs20Data.connectionCapability);
    String2Vector(msgSupplicantStaIface.osuproviderslist(), hs20Data.osuProvidersList);
}

bool SupplicantStaIfaceParseOnAnqpQueryDoneInd(const uint8_t* data, size_t length, OnAnqpQueryDoneIndStaIfaceParam& param)
{
    OnAnqpQueryDoneInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        Message2AnqpData(msg.data(), param.data);
        Message2Hs20AnqpData(msg.hs20data(), param.hs20Data);
        return true;
    }
    return false;
}

static void Message2OceRssiBasedAssocRejectAttr(const OceRssiBasedAssocRejectAttr_P& msgSupplicantStaIface, OceRssiBasedAssocRejectAttr& oceRssiBasedAssocRejectData)
{
    oceRssiBasedAssocRejectData.deltaRssi = msgSupplicantStaIface.deltarssi();
    oceRssiBasedAssocRejectData.retryDelayS = msgSupplicantStaIface.retrydelays();
}

static void Message2AssociationRejectionData(const AssociationRejectionData_P& msgSupplicantStaIface, AssociationRejectionData& assocRejectData)
{
    String2Vector(msgSupplicantStaIface.ssid(), assocRejectData.ssid);
    String2Vector(msgSupplicantStaIface.bssid(), assocRejectData.bssid);
    assocRejectData.statusCode = (StaIfaceStatusCode) msgSupplicantStaIface.statuscode();
    assocRejectData.timedOut = msgSupplicantStaIface.timedout();
    assocRejectData.isMboAssocDisallowedReasonCodePresent = msgSupplicantStaIface.ismboassocdisallowedreasoncodepresent();
    assocRejectData.mboAssocDisallowedReason = (MboAssocDisallowedReasonCode) msgSupplicantStaIface.mboassocdisallowedreason();
    assocRejectData.isOceRssiBasedAssocRejectAttrPresent = msgSupplicantStaIface.isocerssibasedassocrejectattrpresent();
    Message2OceRssiBasedAssocRejectAttr(msgSupplicantStaIface.ocerssibasedassocrejectdata(), assocRejectData.oceRssiBasedAssocRejectData);
}

bool SupplicantStaIfaceParseOnAssociationRejectedInd(const uint8_t* data, size_t length, AssociationRejectionData& param)
{
    OnAssociationRejectedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2AssociationRejectionData(msg.assocrejectdata(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnAuthenticationTimeoutInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    OnAuthenticationTimeoutInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnAuxiliarySupplicantEventInd(const uint8_t* data, size_t length, OnAuxiliarySupplicantEventIndStaIfaceParam& param)
{
    OnAuxiliarySupplicantEventInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.eventCode = (AuxiliarySupplicantEventCode) msg.eventcode();
        String2Vector(msg.bssid(), param.bssid);
        param.reasonString = msg.reasonstring();
        return true;
    }
    return false;
}

static void Message2BssTmData(const BssTmData_P& msgSupplicantStaIface, BssTmData& tmData)
{
    tmData.status = (BssTmStatusCode) msgSupplicantStaIface.status();
    tmData.flags = (BssTmDataFlagsMask) msgSupplicantStaIface.flags();
    tmData.assocRetryDelayMs = msgSupplicantStaIface.assocretrydelayms();
    tmData.mboTransitionReason = (MboTransitionReasonCode) msgSupplicantStaIface.mbotransitionreason();
    tmData.mboCellPreference = (MboCellularDataConnectionPrefValue) msgSupplicantStaIface.mbocellpreference();
}

bool SupplicantStaIfaceParseOnBssTmHandlingDoneInd(const uint8_t* data, size_t length, BssTmData& param)
{
    OnBssTmHandlingDoneInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2BssTmData(msg.tmdata(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnBssidChangedInd(const uint8_t* data, size_t length, OnBssidChangedIndStaIfaceParam& param)
{
    OnBssidChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.reason = (BssidChangeReason) msg.reason();
        String2Vector(msg.bssid(), param.bssid);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDisconnectedInd(const uint8_t* data, size_t length, OnDisconnectedIndStaIfaceParam& param)
{
    OnDisconnectedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.locallyGenerated = msg.locallygenerated();
        param.reasonCode = (StaIfaceReasonCode) msg.reasoncode();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDppFailureInd(const uint8_t* data, size_t length, OnDppFailureIndStaIfaceParam& param)
{
    OnDppFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.code = (DppFailureCode) msg.code();
        param.ssid = msg.ssid();
        param.channelList = msg.channellist();
        int size_bandlist = msg.bandlist_size();
        if (0 < size_bandlist)
        {
            param.bandList.resize(size_bandlist);
            for (int index = 0; index < size_bandlist; index++)
            {
                param.bandList[index] = msg.bandlist(index);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDppProgressInd(const uint8_t* data, size_t length, DppProgressCode& param)
{
    OnDppProgressInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (DppProgressCode) msg.code();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDppSuccessInd(const uint8_t* data, size_t length, DppEventType& param)
{
    OnDppSuccessInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (DppEventType) msg.event();
        return true;
    }
    return false;
}

static void Message2DppConnectionKeys(const DppConnectionKeys_P& msgSupplicantStaIface, DppConnectionKeys& dppConnectionKeys)
{
    String2Vector(msgSupplicantStaIface.connector(), dppConnectionKeys.connector);
    String2Vector(msgSupplicantStaIface.csign(), dppConnectionKeys.cSign);
    String2Vector(msgSupplicantStaIface.netaccesskey(), dppConnectionKeys.netAccessKey);
}

bool SupplicantStaIfaceParseOnDppSuccessConfigReceivedInd(const uint8_t* data, size_t length, OnDppSuccessConfigReceivedIndStaIfaceParam& param)
{
    OnDppSuccessConfigReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.ssid(), param.ssid);
        param.password = msg.password();
        String2Vector(msg.psk(), param.psk);
        param.securityAkm = (DppAkm) msg.securityakm();
        Message2DppConnectionKeys(msg.dppconnectionkeys(), param.dppConnectionKeys);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDppSuccessConfigSentInd(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseOnEapFailureInd(const uint8_t* data, size_t length, OnEapFailureIndStaIfaceParam& param)
{
    OnEapFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.errorCode = msg.errorcode();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnExtRadioWorkStartInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnExtRadioWorkStartInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnExtRadioWorkTimeoutInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnExtRadioWorkTimeoutInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnHs20DeauthImminentNoticeInd(const uint8_t* data, size_t length, OnHs20DeauthImminentNoticeIndStaIfaceParam& param)
{
    OnHs20DeauthImminentNoticeInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.reasonCode = msg.reasoncode();
        param.reAuthDelayInSec = msg.reauthdelayinsec();
        param.url = msg.url();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnHs20IconQueryDoneInd(const uint8_t* data, size_t length, OnHs20IconQueryDoneIndStaIfaceParam& param)
{
    OnHs20IconQueryDoneInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.fileName = msg.filename();
        String2Vector(msg.data(), param.data);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnHs20SubscriptionRemediationInd(const uint8_t* data, size_t length, OnHs20SubscriptionRemediationIndStaIfaceParam& param)
{
    OnHs20SubscriptionRemediationInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.osuMethod = (OsuMethod) msg.osumethod();
        param.url = msg.url();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(const uint8_t* data, size_t length, OnHs20TermsAndConditionsAcceptanceRequestedNotificationIndStaIfaceParam& param)
{
    OnHs20TermsAndConditionsAcceptanceRequestedNotificationInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.url = msg.url();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnNetworkAddedInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnNetworkAddedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnNetworkNotFoundInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    OnNetworkNotFoundInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.ssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnNetworkRemovedInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnNetworkRemovedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnPmkCacheAddedInd(const uint8_t* data, size_t length, OnPmkCacheAddedIndStaIfaceParam& param)
{
    OnPmkCacheAddedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.expirationTimeInSec = msg.expirationtimeinsec();
        String2Vector(msg.serializedentry(), param.serializedEntry);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnStateChangedInd(const uint8_t* data, size_t length, OnStateChangedIndStaIfaceParam& param)
{
    OnStateChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.newState = (StaIfaceCallbackState) msg.newstate();
        String2Vector(msg.bssid(), param.bssid);
        param.id = msg.id();
        String2Vector(msg.ssid(), param.ssid);
        param.filsHlpSent = msg.filshlpsent();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnWpsEventFailInd(const uint8_t* data, size_t length, OnWpsEventFailIndStaIfaceParam& param)
{
    OnWpsEventFailInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.configError = (WpsConfigError) msg.configerror();
        param.errorInd = (WpsErrorIndication) msg.errorind();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnWpsEventPbcOverlapInd(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseOnWpsEventSuccessInd(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaIfaceParseOnQosPolicyResetInd(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2QosPolicyData(const QosPolicyData_P& msgSupplicantStaIface, QosPolicyData& qosPolicyData)
{
    qosPolicyData.policyId = msgSupplicantStaIface.policyid();
    qosPolicyData.requestType = (QosPolicyRequestType) msgSupplicantStaIface.requesttype();
    qosPolicyData.dscp = msgSupplicantStaIface.dscp();
    Message2QosPolicyClassifierParams(msgSupplicantStaIface.classifierparams(), qosPolicyData.classifierParams);
}

bool SupplicantStaIfaceParseOnQosPolicyRequestInd(const uint8_t* data, size_t length, OnQosPolicyRequestIndStaIfaceParam& param)
{
    OnQosPolicyRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.qosPolicyRequestId = msg.qospolicyrequestid();
        int size_qospolicydata = msg.qospolicydata_size();
        if (0 < size_qospolicydata)
        {
            param.qosPolicyData.resize(size_qospolicydata);
            for (int index = 0; index < size_qospolicydata; index++)
            {
                Message2QosPolicyData(msg.qospolicydata(index), param.qosPolicyData[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnMloLinksInfoChangedInd(const uint8_t* data, size_t length, ISupplicantStaIfaceCallback::MloLinkInfoChangeReason& param)
{
    OnMloLinksInfoChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (ISupplicantStaIfaceCallback::MloLinkInfoChangeReason) msg.reason();
        return true;
    }
    return false;
}

static void Message2DppConfigurationData(const DppConfigurationData_P& msgSupplicantStaIface, DppConfigurationData& configData)
{
    String2Vector(msgSupplicantStaIface.ssid(), configData.ssid);
    configData.password = msgSupplicantStaIface.password();
    String2Vector(msgSupplicantStaIface.psk(), configData.psk);
    configData.securityAkm = (DppAkm) msgSupplicantStaIface.securityakm();
    Message2DppConnectionKeys(msgSupplicantStaIface.dppconnectionkeys(), configData.dppConnectionKeys);
    configData.connStatusRequested = msgSupplicantStaIface.connstatusrequested();
}

bool SupplicantStaIfaceParseOnDppConfigReceivedInd(const uint8_t* data, size_t length, DppConfigurationData& param)
{
    OnDppConfigReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2DppConfigurationData(msg.configdata(), param);
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnDppConnectionStatusResultSentInd(const uint8_t* data, size_t length, DppStatusErrorCode& param)
{
    OnDppConnectionStatusResultSentInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (DppStatusErrorCode) msg.code();
        return true;
    }
    return false;
}

bool SupplicantStaIfaceParseOnBssFrequencyChangedInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnBssFrequencyChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.frequencymhz();
        return true;
    }
    return false;
}

static void Message2SupplicantStateChangeData(const SupplicantStateChangeData_P& msgSupplicantStaIface, SupplicantStateChangeData& stateChangeData)
{
    stateChangeData.newState = (StaIfaceCallbackState) msgSupplicantStaIface.newstate();
    stateChangeData.id = msgSupplicantStaIface.id();
    String2Vector(msgSupplicantStaIface.ssid(), stateChangeData.ssid);
    String2Array(msgSupplicantStaIface.bssid(), stateChangeData.bssid);
    stateChangeData.keyMgmtMask = (KeyMgmtMask) msgSupplicantStaIface.keymgmtmask();
    stateChangeData.frequencyMhz = msgSupplicantStaIface.frequencymhz();
    stateChangeData.filsHlpSent = msgSupplicantStaIface.filshlpsent();
}

bool SupplicantStaIfaceParseOnSupplicantStateChangedInd(const uint8_t* data, size_t length, SupplicantStateChangeData& param)
{
    OnSupplicantStateChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2SupplicantStateChangeData(msg.statechangedata(), param);
        return true;
    }
    return false;
}

static void Message2QosPolicyScsResponseStatus(const QosPolicyScsResponseStatus_P& msgSupplicantStaIface, QosPolicyScsResponseStatus& qosPolicyScsResponseStatus)
{
    qosPolicyScsResponseStatus.policyId = msgSupplicantStaIface.policyid();
    qosPolicyScsResponseStatus.qosPolicyScsResponseStatusCode = (QosPolicyScsResponseStatusCode) msgSupplicantStaIface.qospolicyscsresponsestatuscode();
}

bool SupplicantStaIfaceParseOnQosPolicyResponseForScsInd(const uint8_t* data, size_t length, vector<QosPolicyScsResponseStatus>& param)
{
    OnQosPolicyResponseForScsInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_qospolicyscsresponsestatus = msg.qospolicyscsresponsestatus_size();
        if (0 < size_qospolicyscsresponsestatus)
        {
            param.resize(size_qospolicyscsresponsestatus);
            for (int index = 0; index < size_qospolicyscsresponsestatus; index++)
            {
                Message2QosPolicyScsResponseStatus(msg.qospolicyscsresponsestatus(index), param[index]);
            }
        }
        return true;
    }
    return false;
}

static void Message2PmkSaCacheData(const PmkSaCacheData_P& msgSupplicantStaIface, PmkSaCacheData& pmkSaData)
{
    String2Array(msgSupplicantStaIface.bssid(), pmkSaData.bssid);
    pmkSaData.expirationTimeInSec = msgSupplicantStaIface.expirationtimeinsec();
    String2Vector(msgSupplicantStaIface.serializedentry(), pmkSaData.serializedEntry);
}

bool SupplicantStaIfaceParseOnPmkSaCacheAddedInd(const uint8_t* data, size_t length, PmkSaCacheData& param)
{
    OnPmkSaCacheAddedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2PmkSaCacheData(msg.pmksadata(), param);
        return true;
    }
    return false;
}
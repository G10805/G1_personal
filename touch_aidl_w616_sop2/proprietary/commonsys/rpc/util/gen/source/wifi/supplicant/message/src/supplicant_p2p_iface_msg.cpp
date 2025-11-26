/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_p2p_iface_msg.h"

#include "FreqRange.pb.h"
#include "HalStatus.pb.h"
#include "ISupplicantP2pIface.pb.h"
#include "ISupplicantP2pIfaceCallback.pb.h"
#include "ISupplicantP2pNetwork.pb.h"
#include "IfaceType.pb.h"
#include "MiracastMode.pb.h"
#include "P2pClientEapolIpAddressInfo.pb.h"
#include "P2pFrameTypeMask.pb.h"
#include "P2pGroupCapabilityMask.pb.h"
#include "P2pGroupStartedEventParams.pb.h"
#include "P2pProvDiscStatusCode.pb.h"
#include "P2pStatusCode.pb.h"
#include "WpsConfigMethods.pb.h"
#include "WpsDevPasswordId.pb.h"
#include "WpsProvisionMethod.pb.h"

using AddBonjourServiceReq = ::wifi::supplicant::ISupplicantP2pIface_AddBonjourServiceReq;
using AddGroupReq = ::wifi::supplicant::ISupplicantP2pIface_AddGroupReq;
using AddGroupWithConfigReq = ::wifi::supplicant::ISupplicantP2pIface_AddGroupWithConfigReq;
using AddNetworkCfm = ::wifi::supplicant::ISupplicantP2pIface_AddNetworkCfm;
using AddUpnpServiceReq = ::wifi::supplicant::ISupplicantP2pIface_AddUpnpServiceReq;
using CancelServiceDiscoveryReq = ::wifi::supplicant::ISupplicantP2pIface_CancelServiceDiscoveryReq;
using CancelWpsReq = ::wifi::supplicant::ISupplicantP2pIface_CancelWpsReq;
using ConfigureEapolIpAddressAllocationParamsReq = ::wifi::supplicant::ISupplicantP2pIface_ConfigureEapolIpAddressAllocationParamsReq;
using ConfigureExtListenReq = ::wifi::supplicant::ISupplicantP2pIface_ConfigureExtListenReq;
using ConnectCfm = ::wifi::supplicant::ISupplicantP2pIface_ConnectCfm;
using ConnectReq = ::wifi::supplicant::ISupplicantP2pIface_ConnectReq;
using CreateNfcHandoverRequestMessageCfm = ::wifi::supplicant::ISupplicantP2pIface_CreateNfcHandoverRequestMessageCfm;
using CreateNfcHandoverSelectMessageCfm = ::wifi::supplicant::ISupplicantP2pIface_CreateNfcHandoverSelectMessageCfm;
using EnableWfdReq = ::wifi::supplicant::ISupplicantP2pIface_EnableWfdReq;
using FindOnSocialChannelsReq = ::wifi::supplicant::ISupplicantP2pIface_FindOnSocialChannelsReq;
using FindOnSpecificFrequencyReq = ::wifi::supplicant::ISupplicantP2pIface_FindOnSpecificFrequencyReq;
using FindReq = ::wifi::supplicant::ISupplicantP2pIface_FindReq;
using GetDeviceAddressCfm = ::wifi::supplicant::ISupplicantP2pIface_GetDeviceAddressCfm;
using GetEdmgCfm = ::wifi::supplicant::ISupplicantP2pIface_GetEdmgCfm;
using GetGroupCapabilityCfm = ::wifi::supplicant::ISupplicantP2pIface_GetGroupCapabilityCfm;
using GetGroupCapabilityReq = ::wifi::supplicant::ISupplicantP2pIface_GetGroupCapabilityReq;
using GetNameCfm = ::wifi::supplicant::ISupplicantP2pIface_GetNameCfm;
using GetNetworkCfm = ::wifi::supplicant::ISupplicantP2pIface_GetNetworkCfm;
using GetNetworkReq = ::wifi::supplicant::ISupplicantP2pIface_GetNetworkReq;
using GetSsidCfm = ::wifi::supplicant::ISupplicantP2pIface_GetSsidCfm;
using GetSsidReq = ::wifi::supplicant::ISupplicantP2pIface_GetSsidReq;
using GetTypeCfm = ::wifi::supplicant::ISupplicantP2pIface_GetTypeCfm;
using InviteReq = ::wifi::supplicant::ISupplicantP2pIface_InviteReq;
using ListNetworksCfm = ::wifi::supplicant::ISupplicantP2pIface_ListNetworksCfm;
using OnDeviceFoundInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnDeviceFoundInd;
using OnDeviceFoundWithVendorElementsInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnDeviceFoundWithVendorElementsInd;
using OnDeviceLostInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnDeviceLostInd;
using OnGoNegotiationCompletedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGoNegotiationCompletedInd;
using OnGoNegotiationRequestInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGoNegotiationRequestInd;
using OnGroupFormationFailureInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGroupFormationFailureInd;
using OnGroupFrequencyChangedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGroupFrequencyChangedInd;
using OnGroupRemovedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGroupRemovedInd;
using OnGroupStartedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGroupStartedInd;
using OnGroupStartedWithParamsInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnGroupStartedWithParamsInd;
using OnInvitationReceivedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnInvitationReceivedInd;
using OnInvitationResultInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnInvitationResultInd;
using OnProvisionDiscoveryCompletedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnProvisionDiscoveryCompletedInd;
using OnR2DeviceFoundInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnR2DeviceFoundInd;
using OnServiceDiscoveryResponseInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnServiceDiscoveryResponseInd;
using OnStaAuthorizedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnStaAuthorizedInd;
using OnStaDeauthorizedInd = ::wifi::supplicant::ISupplicantP2pIfaceCallback_OnStaDeauthorizedInd;
using ProvisionDiscoveryReq = ::wifi::supplicant::ISupplicantP2pIface_ProvisionDiscoveryReq;
using ReinvokeReq = ::wifi::supplicant::ISupplicantP2pIface_ReinvokeReq;
using RejectReq = ::wifi::supplicant::ISupplicantP2pIface_RejectReq;
using RemoveBonjourServiceReq = ::wifi::supplicant::ISupplicantP2pIface_RemoveBonjourServiceReq;
using RemoveClientReq = ::wifi::supplicant::ISupplicantP2pIface_RemoveClientReq;
using RemoveGroupReq = ::wifi::supplicant::ISupplicantP2pIface_RemoveGroupReq;
using RemoveNetworkReq = ::wifi::supplicant::ISupplicantP2pIface_RemoveNetworkReq;
using RemoveUpnpServiceReq = ::wifi::supplicant::ISupplicantP2pIface_RemoveUpnpServiceReq;
using ReportNfcHandoverInitiationReq = ::wifi::supplicant::ISupplicantP2pIface_ReportNfcHandoverInitiationReq;
using ReportNfcHandoverResponseReq = ::wifi::supplicant::ISupplicantP2pIface_ReportNfcHandoverResponseReq;
using RequestServiceDiscoveryCfm = ::wifi::supplicant::ISupplicantP2pIface_RequestServiceDiscoveryCfm;
using RequestServiceDiscoveryReq = ::wifi::supplicant::ISupplicantP2pIface_RequestServiceDiscoveryReq;
using SetDisallowedFrequenciesReq = ::wifi::supplicant::ISupplicantP2pIface_SetDisallowedFrequenciesReq;
using SetEdmgReq = ::wifi::supplicant::ISupplicantP2pIface_SetEdmgReq;
using SetGroupIdleReq = ::wifi::supplicant::ISupplicantP2pIface_SetGroupIdleReq;
using SetListenChannelReq = ::wifi::supplicant::ISupplicantP2pIface_SetListenChannelReq;
using SetMacRandomizationReq = ::wifi::supplicant::ISupplicantP2pIface_SetMacRandomizationReq;
using SetMiracastModeReq = ::wifi::supplicant::ISupplicantP2pIface_SetMiracastModeReq;
using SetPowerSaveReq = ::wifi::supplicant::ISupplicantP2pIface_SetPowerSaveReq;
using SetSsidPostfixReq = ::wifi::supplicant::ISupplicantP2pIface_SetSsidPostfixReq;
using SetVendorElementsReq = ::wifi::supplicant::ISupplicantP2pIface_SetVendorElementsReq;
using SetWfdDeviceInfoReq = ::wifi::supplicant::ISupplicantP2pIface_SetWfdDeviceInfoReq;
using SetWfdR2DeviceInfoReq = ::wifi::supplicant::ISupplicantP2pIface_SetWfdR2DeviceInfoReq;
using SetWpsConfigMethodsReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsConfigMethodsReq;
using SetWpsDeviceNameReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsDeviceNameReq;
using SetWpsDeviceTypeReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsDeviceTypeReq;
using SetWpsManufacturerReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsManufacturerReq;
using SetWpsModelNameReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsModelNameReq;
using SetWpsModelNumberReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsModelNumberReq;
using SetWpsSerialNumberReq = ::wifi::supplicant::ISupplicantP2pIface_SetWpsSerialNumberReq;
using StartWpsPbcReq = ::wifi::supplicant::ISupplicantP2pIface_StartWpsPbcReq;
using StartWpsPinDisplayCfm = ::wifi::supplicant::ISupplicantP2pIface_StartWpsPinDisplayCfm;
using StartWpsPinDisplayReq = ::wifi::supplicant::ISupplicantP2pIface_StartWpsPinDisplayReq;
using StartWpsPinKeypadReq = ::wifi::supplicant::ISupplicantP2pIface_StartWpsPinKeypadReq;

using FreqRange_P = ::wifi::supplicant::FreqRange;
using HalStatus_P = ::wifi::supplicant::HalStatus;
using ISupplicantP2pNetwork_P = ::wifi::supplicant::ISupplicantP2pNetwork;
using IfaceType_P = ::wifi::supplicant::it::IfaceType;
using MiracastMode_P = ::wifi::supplicant::mm::MiracastMode;
using P2pClientEapolIpAddressInfo_P = ::wifi::supplicant::P2pClientEapolIpAddressInfo;
using P2pFrameTypeMask_P = ::wifi::supplicant::P2pFrameTypeMask;
using P2pGroupCapabilityMask_P = ::wifi::supplicant::P2pGroupCapabilityMask;
using P2pGroupStartedEventParams_P = ::wifi::supplicant::P2pGroupStartedEventParams;
using P2pProvDiscStatusCode_P = ::wifi::supplicant::ppdsc::P2pProvDiscStatusCode;
using P2pStatusCode_P = ::wifi::supplicant::psc::P2pStatusCode;
using WpsConfigMethods_P = ::wifi::supplicant::wcm::WpsConfigMethods;
using WpsDevPasswordId_P = ::wifi::supplicant::wdpi::WpsDevPasswordId;
using WpsProvisionMethod_P = ::wifi::supplicant::wpm::WpsProvisionMethod;


bool SupplicantP2pIfaceSerializeAddBonjourServiceReq(const vector<uint8_t>& query, const vector<uint8_t>& response, vector<uint8_t>& payload)
{
    AddBonjourServiceReq msgSupplicantP2pIface;
    string queryStr;
    Vector2String(query, queryStr);
    msgSupplicantP2pIface.set_query(queryStr);
    string responseStr;
    Vector2String(response, responseStr);
    msgSupplicantP2pIface.set_response(responseStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantP2pIface)
{
    msgSupplicantP2pIface.set_status(status.status);
    msgSupplicantP2pIface.set_info(status.info);
}

bool SupplicantP2pIfaceSerializeAddBonjourServiceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeAddGroupReq(bool persistent, int32_t persistentNetworkId, vector<uint8_t>& payload)
{
    AddGroupReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_persistent(persistent);
    msgSupplicantP2pIface.set_persistentnetworkid(persistentNetworkId);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeAddGroupCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeAddGroupWithConfigReq(const vector<uint8_t>& ssid, const string& pskPassphrase, bool persistent, int32_t freq, const vector<uint8_t>& peerAddress, bool joinExistingGroup, vector<uint8_t>& payload)
{
    AddGroupWithConfigReq msgSupplicantP2pIface;
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantP2pIface.set_ssid(ssidStr);
    msgSupplicantP2pIface.set_pskpassphrase(pskPassphrase);
    msgSupplicantP2pIface.set_persistent(persistent);
    msgSupplicantP2pIface.set_freq(freq);
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    msgSupplicantP2pIface.set_joinexistinggroup(joinExistingGroup);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeAddGroupWithConfigCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeAddNetworkReq(vector<uint8_t>& payload)
{
    return true;
}

static void ISupplicantP2pNetwork2Message(int32_t result, ISupplicantP2pNetwork_P& msgSupplicantP2pIface)
{
    msgSupplicantP2pIface.set_instanceid(result);
}

bool SupplicantP2pIfaceSerializeAddNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    AddNetworkCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantP2pNetwork_P* result_p = msgSupplicantP2pIface.mutable_result();
    ISupplicantP2pNetwork2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeAddUpnpServiceReq(int32_t version, const string& serviceName, vector<uint8_t>& payload)
{
    AddUpnpServiceReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_version(version);
    msgSupplicantP2pIface.set_servicename(serviceName);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeAddUpnpServiceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCancelConnectReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCancelConnectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCancelServiceDiscoveryReq(int64_t identifier, vector<uint8_t>& payload)
{
    CancelServiceDiscoveryReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_identifier(identifier);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeCancelServiceDiscoveryCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCancelWpsReq(const string& groupIfName, vector<uint8_t>& payload)
{
    CancelWpsReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeCancelWpsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeConfigureExtListenReq(int32_t periodInMillis, int32_t intervalInMillis, vector<uint8_t>& payload)
{
    ConfigureExtListenReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_periodinmillis(periodInMillis);
    msgSupplicantP2pIface.set_intervalinmillis(intervalInMillis);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeConfigureExtListenCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeConnectReq(const vector<uint8_t>& peerAddress, WpsProvisionMethod provisionMethod, const string& preSelectedPin, bool joinExistingGroup, bool persistent, int32_t goIntent, vector<uint8_t>& payload)
{
    ConnectReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    msgSupplicantP2pIface.set_provisionmethod((WpsProvisionMethod_P) provisionMethod);
    msgSupplicantP2pIface.set_preselectedpin(preSelectedPin);
    msgSupplicantP2pIface.set_joinexistinggroup(joinExistingGroup);
    msgSupplicantP2pIface.set_persistent(persistent);
    msgSupplicantP2pIface.set_gointent(goIntent);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeConnectCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    ConnectCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    CreateNfcHandoverRequestMessageCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    CreateNfcHandoverSelectMessageCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeEnableWfdReq(bool enable, vector<uint8_t>& payload)
{
    EnableWfdReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeEnableWfdCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFindReq(int32_t timeoutInSec, vector<uint8_t>& payload)
{
    FindReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_timeoutinsec(timeoutInSec);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeFindCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFlushReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFlushCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFlushServicesReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFlushServicesCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeGetDeviceAddressReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeGetDeviceAddressCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetDeviceAddressCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetEdmgReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeGetEdmgCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    GetEdmgCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetGroupCapabilityReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload)
{
    GetGroupCapabilityReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetGroupCapabilityCfm(const HalStatusParam& status, P2pGroupCapabilityMask result, vector<uint8_t>& payload)
{
    GetGroupCapabilityCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result((P2pGroupCapabilityMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetNetworkReq(int32_t id, vector<uint8_t>& payload)
{
    GetNetworkReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetNetworkCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    ISupplicantP2pNetwork_P* result_p = msgSupplicantP2pIface.mutable_result();
    ISupplicantP2pNetwork2Message(result, *result_p);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetSsidReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload)
{
    GetSsidReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetSsidCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantP2pIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeGetTypeReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload)
{
    GetTypeCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result((IfaceType_P) result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeInviteReq(const string& groupIfName, const vector<uint8_t>& goDeviceAddress, const vector<uint8_t>& peerAddress, vector<uint8_t>& payload)
{
    InviteReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    string goDeviceAddressStr;
    Vector2String(goDeviceAddress, goDeviceAddressStr);
    msgSupplicantP2pIface.set_godeviceaddress(goDeviceAddressStr);
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeInviteCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeListNetworksReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeListNetworksCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload)
{
    ListNetworksCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgSupplicantP2pIface.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeProvisionDiscoveryReq(const vector<uint8_t>& peerAddress, WpsProvisionMethod provisionMethod, vector<uint8_t>& payload)
{
    ProvisionDiscoveryReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    msgSupplicantP2pIface.set_provisionmethod((WpsProvisionMethod_P) provisionMethod);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeProvisionDiscoveryCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRegisterCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRegisterCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeReinvokeReq(int32_t persistentNetworkId, const vector<uint8_t>& peerAddress, vector<uint8_t>& payload)
{
    ReinvokeReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_persistentnetworkid(persistentNetworkId);
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeReinvokeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRejectReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload)
{
    RejectReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRejectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRemoveBonjourServiceReq(const vector<uint8_t>& query, vector<uint8_t>& payload)
{
    RemoveBonjourServiceReq msgSupplicantP2pIface;
    string queryStr;
    Vector2String(query, queryStr);
    msgSupplicantP2pIface.set_query(queryStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRemoveBonjourServiceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRemoveGroupReq(const string& groupIfName, vector<uint8_t>& payload)
{
    RemoveGroupReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRemoveGroupCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRemoveNetworkReq(int32_t id, vector<uint8_t>& payload)
{
    RemoveNetworkReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRemoveNetworkCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRemoveUpnpServiceReq(int32_t version, const string& serviceName, vector<uint8_t>& payload)
{
    RemoveUpnpServiceReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_version(version);
    msgSupplicantP2pIface.set_servicename(serviceName);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRemoveUpnpServiceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeReportNfcHandoverInitiationReq(const vector<uint8_t>& select, vector<uint8_t>& payload)
{
    ReportNfcHandoverInitiationReq msgSupplicantP2pIface;
    string selectStr;
    Vector2String(select, selectStr);
    msgSupplicantP2pIface.set_select(selectStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeReportNfcHandoverInitiationCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeReportNfcHandoverResponseReq(const vector<uint8_t>& request, vector<uint8_t>& payload)
{
    ReportNfcHandoverResponseReq msgSupplicantP2pIface;
    string requestStr;
    Vector2String(request, requestStr);
    msgSupplicantP2pIface.set_request(requestStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeReportNfcHandoverResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRequestServiceDiscoveryReq(const vector<uint8_t>& peerAddress, const vector<uint8_t>& query, vector<uint8_t>& payload)
{
    RequestServiceDiscoveryReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    string queryStr;
    Vector2String(query, queryStr);
    msgSupplicantP2pIface.set_query(queryStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRequestServiceDiscoveryCfm(const HalStatusParam& status, int64_t result, vector<uint8_t>& payload)
{
    RequestServiceDiscoveryCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSaveConfigReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSaveConfigCfm(vector<uint8_t>& payload)
{
    return true;
}

static void FreqRange2Message(const FreqRange& ranges, FreqRange_P& msgSupplicantP2pIface)
{
    msgSupplicantP2pIface.set_min(ranges.min);
    msgSupplicantP2pIface.set_max(ranges.max);
}

bool SupplicantP2pIfaceSerializeSetDisallowedFrequenciesReq(const vector<FreqRange>& ranges, vector<uint8_t>& payload)
{
    SetDisallowedFrequenciesReq msgSupplicantP2pIface;
    int size_ranges = ranges.size();
    if (0 < size_ranges)
    {
        for (int index = 0; index < size_ranges; index++)
        {
            FreqRange_P *ranges_p = msgSupplicantP2pIface.add_ranges();
            FreqRange2Message(ranges[index], *ranges_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetDisallowedFrequenciesCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetEdmgReq(bool enable, vector<uint8_t>& payload)
{
    SetEdmgReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetEdmgCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetGroupIdleReq(const string& groupIfName, int32_t timeoutInSec, vector<uint8_t>& payload)
{
    SetGroupIdleReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    msgSupplicantP2pIface.set_timeoutinsec(timeoutInSec);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetGroupIdleCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetListenChannelReq(int32_t channel, int32_t operatingClass, vector<uint8_t>& payload)
{
    SetListenChannelReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_channel(channel);
    msgSupplicantP2pIface.set_operatingclass(operatingClass);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetListenChannelCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetMacRandomizationReq(bool enable, vector<uint8_t>& payload)
{
    SetMacRandomizationReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetMacRandomizationCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetMiracastModeReq(MiracastMode mode, vector<uint8_t>& payload)
{
    SetMiracastModeReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_mode((MiracastMode_P) mode);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetMiracastModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetPowerSaveReq(const string& groupIfName, bool enable, vector<uint8_t>& payload)
{
    SetPowerSaveReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    msgSupplicantP2pIface.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetPowerSaveCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetSsidPostfixReq(const vector<uint8_t>& postfix, vector<uint8_t>& payload)
{
    SetSsidPostfixReq msgSupplicantP2pIface;
    string postfixStr;
    Vector2String(postfix, postfixStr);
    msgSupplicantP2pIface.set_postfix(postfixStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetSsidPostfixCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWfdDeviceInfoReq(const vector<uint8_t>& info, vector<uint8_t>& payload)
{
    SetWfdDeviceInfoReq msgSupplicantP2pIface;
    string infoStr;
    Vector2String(info, infoStr);
    msgSupplicantP2pIface.set_info(infoStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWfdDeviceInfoCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoReq(const vector<uint8_t>& info, vector<uint8_t>& payload)
{
    SetWfdR2DeviceInfoReq msgSupplicantP2pIface;
    string infoStr;
    Vector2String(info, infoStr);
    msgSupplicantP2pIface.set_info(infoStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeRemoveClientReq(const vector<uint8_t>& peerAddress, bool isLegacyClient, vector<uint8_t>& payload)
{
    RemoveClientReq msgSupplicantP2pIface;
    string peerAddressStr;
    Vector2String(peerAddress, peerAddressStr);
    msgSupplicantP2pIface.set_peeraddress(peerAddressStr);
    msgSupplicantP2pIface.set_islegacyclient(isLegacyClient);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeRemoveClientCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsConfigMethodsReq(WpsConfigMethods configMethods, vector<uint8_t>& payload)
{
    SetWpsConfigMethodsReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_configmethods((WpsConfigMethods_P) configMethods);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsConfigMethodsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsDeviceNameReq(const string& name, vector<uint8_t>& payload)
{
    SetWpsDeviceNameReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_name(name);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsDeviceNameCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsDeviceTypeReq(const vector<uint8_t>& type, vector<uint8_t>& payload)
{
    SetWpsDeviceTypeReq msgSupplicantP2pIface;
    string typeStr;
    Vector2String(type, typeStr);
    msgSupplicantP2pIface.set_type(typeStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsDeviceTypeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsManufacturerReq(const string& manufacturer, vector<uint8_t>& payload)
{
    SetWpsManufacturerReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_manufacturer(manufacturer);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsManufacturerCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsModelNameReq(const string& modelName, vector<uint8_t>& payload)
{
    SetWpsModelNameReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_modelname(modelName);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsModelNameCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsModelNumberReq(const string& modelNumber, vector<uint8_t>& payload)
{
    SetWpsModelNumberReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_modelnumber(modelNumber);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsModelNumberCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetWpsSerialNumberReq(const string& serialNumber, vector<uint8_t>& payload)
{
    SetWpsSerialNumberReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_serialnumber(serialNumber);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetWpsSerialNumberCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeStartWpsPbcReq(const string& groupIfName, const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    StartWpsPbcReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantP2pIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeStartWpsPbcCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeStartWpsPinDisplayReq(const string& groupIfName, const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    StartWpsPinDisplayReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantP2pIface.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeStartWpsPinDisplayCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    StartWpsPinDisplayCfm msgSupplicantP2pIface;
    HalStatus_P* status_p = msgSupplicantP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeStartWpsPinKeypadReq(const string& groupIfName, const string& pin, vector<uint8_t>& payload)
{
    StartWpsPinKeypadReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfName);
    msgSupplicantP2pIface.set_pin(pin);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeStartWpsPinKeypadCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeStopFindReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeStopFindCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFindOnSocialChannelsReq(int32_t timeoutInSec, vector<uint8_t>& payload)
{
    FindOnSocialChannelsReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_timeoutinsec(timeoutInSec);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeFindOnSocialChannelsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeFindOnSpecificFrequencyReq(int32_t freqInHz, int32_t timeoutInSec, vector<uint8_t>& payload)
{
    FindOnSpecificFrequencyReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_freqinhz(freqInHz);
    msgSupplicantP2pIface.set_timeoutinsec(timeoutInSec);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeFindOnSpecificFrequencyCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeSetVendorElementsReq(P2pFrameTypeMask frameTypeMask, const vector<uint8_t>& vendorElemBytes, vector<uint8_t>& payload)
{
    SetVendorElementsReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_frametypemask((P2pFrameTypeMask_P) frameTypeMask);
    string vendorElemBytesStr;
    Vector2String(vendorElemBytes, vendorElemBytesStr);
    msgSupplicantP2pIface.set_vendorelembytes(vendorElemBytesStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeSetVendorElementsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsReq(int32_t ipAddressGo, int32_t ipAddressMask, int32_t ipAddressStart, int32_t ipAddressEnd, vector<uint8_t>& payload)
{
    ConfigureEapolIpAddressAllocationParamsReq msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_ipaddressgo(ipAddressGo);
    msgSupplicantP2pIface.set_ipaddressmask(ipAddressMask);
    msgSupplicantP2pIface.set_ipaddressstart(ipAddressStart);
    msgSupplicantP2pIface.set_ipaddressend(ipAddressEnd);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeOnDeviceFoundInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, vector<uint8_t>& payload)
{
    OnDeviceFoundInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    string primaryDeviceTypeStr;
    Vector2String(primaryDeviceType, primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_primarydevicetype(primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_devicename(deviceName);
    msgSupplicantP2pIface.set_configmethods((WpsConfigMethods_P) configMethods);
    msgSupplicantP2pIface.set_devicecapabilities(deviceCapabilities);
    msgSupplicantP2pIface.set_groupcapabilities((P2pGroupCapabilityMask_P) groupCapabilities);
    string wfdDeviceInfoStr;
    Vector2String(wfdDeviceInfo, wfdDeviceInfoStr);
    msgSupplicantP2pIface.set_wfddeviceinfo(wfdDeviceInfoStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnDeviceLostInd(const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload)
{
    OnDeviceLostInd msgSupplicantP2pIface;
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnFindStoppedInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeOnGoNegotiationCompletedInd(P2pStatusCode status, vector<uint8_t>& payload)
{
    OnGoNegotiationCompletedInd msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_status((P2pStatusCode_P) status);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnGoNegotiationRequestInd(const vector<uint8_t>& srcAddress, WpsDevPasswordId passwordId, vector<uint8_t>& payload)
{
    OnGoNegotiationRequestInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    msgSupplicantP2pIface.set_passwordid((WpsDevPasswordId_P) passwordId);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnGroupFormationFailureInd(const string& failureReason, vector<uint8_t>& payload)
{
    OnGroupFormationFailureInd msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_failurereason(failureReason);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnGroupFormationSuccessInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantP2pIfaceSerializeOnGroupRemovedInd(const string& groupIfname, bool isGroupOwner, vector<uint8_t>& payload)
{
    OnGroupRemovedInd msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfname);
    msgSupplicantP2pIface.set_isgroupowner(isGroupOwner);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnGroupStartedInd(const string& groupIfname, bool isGroupOwner, const vector<uint8_t>& ssid, int32_t frequency, const vector<uint8_t>& psk, const string& passphrase, const vector<uint8_t>& goDeviceAddress, bool isPersistent, vector<uint8_t>& payload)
{
    OnGroupStartedInd msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfname);
    msgSupplicantP2pIface.set_isgroupowner(isGroupOwner);
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantP2pIface.set_ssid(ssidStr);
    msgSupplicantP2pIface.set_frequency(frequency);
    string pskStr;
    Vector2String(psk, pskStr);
    msgSupplicantP2pIface.set_psk(pskStr);
    msgSupplicantP2pIface.set_passphrase(passphrase);
    string goDeviceAddressStr;
    Vector2String(goDeviceAddress, goDeviceAddressStr);
    msgSupplicantP2pIface.set_godeviceaddress(goDeviceAddressStr);
    msgSupplicantP2pIface.set_ispersistent(isPersistent);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnInvitationReceivedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& goDeviceAddress, const vector<uint8_t>& bssid, int32_t persistentNetworkId, int32_t operatingFrequency, vector<uint8_t>& payload)
{
    OnInvitationReceivedInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string goDeviceAddressStr;
    Vector2String(goDeviceAddress, goDeviceAddressStr);
    msgSupplicantP2pIface.set_godeviceaddress(goDeviceAddressStr);
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantP2pIface.set_bssid(bssidStr);
    msgSupplicantP2pIface.set_persistentnetworkid(persistentNetworkId);
    msgSupplicantP2pIface.set_operatingfrequency(operatingFrequency);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnInvitationResultInd(const vector<uint8_t>& bssid, P2pStatusCode status, vector<uint8_t>& payload)
{
    OnInvitationResultInd msgSupplicantP2pIface;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantP2pIface.set_bssid(bssidStr);
    msgSupplicantP2pIface.set_status((P2pStatusCode_P) status);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnProvisionDiscoveryCompletedInd(const vector<uint8_t>& p2pDeviceAddress, bool isRequest, P2pProvDiscStatusCode status, WpsConfigMethods configMethods, const string& generatedPin, vector<uint8_t>& payload)
{
    OnProvisionDiscoveryCompletedInd msgSupplicantP2pIface;
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_isrequest(isRequest);
    msgSupplicantP2pIface.set_status((P2pProvDiscStatusCode_P) status);
    msgSupplicantP2pIface.set_configmethods((WpsConfigMethods_P) configMethods);
    msgSupplicantP2pIface.set_generatedpin(generatedPin);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnR2DeviceFoundInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, const vector<uint8_t>& wfdR2DeviceInfo, vector<uint8_t>& payload)
{
    OnR2DeviceFoundInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    string primaryDeviceTypeStr;
    Vector2String(primaryDeviceType, primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_primarydevicetype(primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_devicename(deviceName);
    msgSupplicantP2pIface.set_configmethods((WpsConfigMethods_P) configMethods);
    msgSupplicantP2pIface.set_devicecapabilities(deviceCapabilities);
    msgSupplicantP2pIface.set_groupcapabilities((P2pGroupCapabilityMask_P) groupCapabilities);
    string wfdDeviceInfoStr;
    Vector2String(wfdDeviceInfo, wfdDeviceInfoStr);
    msgSupplicantP2pIface.set_wfddeviceinfo(wfdDeviceInfoStr);
    string wfdR2DeviceInfoStr;
    Vector2String(wfdR2DeviceInfo, wfdR2DeviceInfoStr);
    msgSupplicantP2pIface.set_wfdr2deviceinfo(wfdR2DeviceInfoStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnServiceDiscoveryResponseInd(const vector<uint8_t>& srcAddress, char16_t updateIndicator, const vector<uint8_t>& tlvs, vector<uint8_t>& payload)
{
    OnServiceDiscoveryResponseInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    msgSupplicantP2pIface.set_updateindicator(updateIndicator);
    string tlvsStr;
    Vector2String(tlvs, tlvsStr);
    msgSupplicantP2pIface.set_tlvs(tlvsStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnStaAuthorizedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload)
{
    OnStaAuthorizedInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnStaDeauthorizedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload)
{
    OnStaDeauthorizedInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnGroupFrequencyChangedInd(const string& groupIfname, int32_t frequency, vector<uint8_t>& payload)
{
    OnGroupFrequencyChangedInd msgSupplicantP2pIface;
    msgSupplicantP2pIface.set_groupifname(groupIfname);
    msgSupplicantP2pIface.set_frequency(frequency);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceSerializeOnDeviceFoundWithVendorElementsInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, const vector<uint8_t>& wfdR2DeviceInfo, const vector<uint8_t>& vendorElemBytes, vector<uint8_t>& payload)
{
    OnDeviceFoundWithVendorElementsInd msgSupplicantP2pIface;
    string srcAddressStr;
    Vector2String(srcAddress, srcAddressStr);
    msgSupplicantP2pIface.set_srcaddress(srcAddressStr);
    string p2pDeviceAddressStr;
    Vector2String(p2pDeviceAddress, p2pDeviceAddressStr);
    msgSupplicantP2pIface.set_p2pdeviceaddress(p2pDeviceAddressStr);
    string primaryDeviceTypeStr;
    Vector2String(primaryDeviceType, primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_primarydevicetype(primaryDeviceTypeStr);
    msgSupplicantP2pIface.set_devicename(deviceName);
    msgSupplicantP2pIface.set_configmethods((WpsConfigMethods_P) configMethods);
    msgSupplicantP2pIface.set_devicecapabilities(deviceCapabilities);
    msgSupplicantP2pIface.set_groupcapabilities((P2pGroupCapabilityMask_P) groupCapabilities);
    string wfdDeviceInfoStr;
    Vector2String(wfdDeviceInfo, wfdDeviceInfoStr);
    msgSupplicantP2pIface.set_wfddeviceinfo(wfdDeviceInfoStr);
    string wfdR2DeviceInfoStr;
    Vector2String(wfdR2DeviceInfo, wfdR2DeviceInfoStr);
    msgSupplicantP2pIface.set_wfdr2deviceinfo(wfdR2DeviceInfoStr);
    string vendorElemBytesStr;
    Vector2String(vendorElemBytes, vendorElemBytesStr);
    msgSupplicantP2pIface.set_vendorelembytes(vendorElemBytesStr);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

static void P2pClientEapolIpAddressInfo2Message(const P2pClientEapolIpAddressInfo& p2pClientIpInfo, P2pClientEapolIpAddressInfo_P& msgSupplicantP2pIface)
{
    msgSupplicantP2pIface.set_ipaddressclient(p2pClientIpInfo.ipAddressClient);
    msgSupplicantP2pIface.set_ipaddressmask(p2pClientIpInfo.ipAddressMask);
    msgSupplicantP2pIface.set_ipaddressgo(p2pClientIpInfo.ipAddressGo);
}

static void P2pGroupStartedEventParams2Message(const P2pGroupStartedEventParams& groupStartedEventParams, P2pGroupStartedEventParams_P& msgSupplicantP2pIface)
{
    msgSupplicantP2pIface.set_groupinterfacename(groupStartedEventParams.groupInterfaceName);
    msgSupplicantP2pIface.set_isgroupowner(groupStartedEventParams.isGroupOwner);
    string ssid;
    Vector2String(groupStartedEventParams.ssid, ssid);
    msgSupplicantP2pIface.set_ssid(ssid);
    msgSupplicantP2pIface.set_frequencymhz(groupStartedEventParams.frequencyMHz);
    string psk;
    Vector2String(groupStartedEventParams.psk, psk);
    msgSupplicantP2pIface.set_psk(psk);
    msgSupplicantP2pIface.set_passphrase(groupStartedEventParams.passphrase);
    msgSupplicantP2pIface.set_ispersistent(groupStartedEventParams.isPersistent);
    string goDeviceAddress;
    Array2String(groupStartedEventParams.goDeviceAddress, goDeviceAddress);
    msgSupplicantP2pIface.set_godeviceaddress(goDeviceAddress);
    string goInterfaceAddress;
    Array2String(groupStartedEventParams.goInterfaceAddress, goInterfaceAddress);
    msgSupplicantP2pIface.set_gointerfaceaddress(goInterfaceAddress);
    msgSupplicantP2pIface.set_isp2pclienteapolipaddressinfopresent(groupStartedEventParams.isP2pClientEapolIpAddressInfoPresent);
    P2pClientEapolIpAddressInfo_P* p2pClientIpInfo_p = msgSupplicantP2pIface.mutable_p2pclientipinfo();
    P2pClientEapolIpAddressInfo2Message(groupStartedEventParams.p2pClientIpInfo, *p2pClientIpInfo_p);
}

bool SupplicantP2pIfaceSerializeOnGroupStartedWithParamsInd(const P2pGroupStartedEventParams& groupStartedEventParams, vector<uint8_t>& payload)
{
    OnGroupStartedWithParamsInd msgSupplicantP2pIface;
    P2pGroupStartedEventParams_P* groupStartedEventParams_p = msgSupplicantP2pIface.mutable_groupstartedeventparams();
    P2pGroupStartedEventParams2Message(groupStartedEventParams, *groupStartedEventParams_p);
    return ProtoMessageSerialize(&msgSupplicantP2pIface, payload);
}

bool SupplicantP2pIfaceParseAddBonjourServiceReq(const uint8_t* data, size_t length, AddBonjourServiceReqP2pIfaceParam& param)
{
    AddBonjourServiceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.query(), param.query);
        String2Vector(msg.response(), param.response);
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantP2pIface, HalStatusParam& status)
{
    status.status = msgSupplicantP2pIface.status();
    status.info = msgSupplicantP2pIface.info();
}

bool SupplicantP2pIfaceParseAddBonjourServiceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseAddGroupReq(const uint8_t* data, size_t length, AddGroupReqP2pIfaceParam& param)
{
    AddGroupReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.persistent = msg.persistent();
        param.persistentNetworkId = msg.persistentnetworkid();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseAddGroupCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseAddGroupWithConfigReq(const uint8_t* data, size_t length, AddGroupWithConfigReqP2pIfaceParam& param)
{
    AddGroupWithConfigReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.ssid(), param.ssid);
        param.pskPassphrase = msg.pskpassphrase();
        param.persistent = msg.persistent();
        param.freq = msg.freq();
        String2Vector(msg.peeraddress(), param.peerAddress);
        param.joinExistingGroup = msg.joinexistinggroup();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseAddGroupWithConfigCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseAddNetworkReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ISupplicantP2pNetwork(const ISupplicantP2pNetwork_P& msgSupplicantP2pIface, int32_t& result)
{
    result = msgSupplicantP2pIface.instanceid();
}

bool SupplicantP2pIfaceParseAddNetworkCfm(const uint8_t* data, size_t length, AddNetworkCfmP2pIfaceParam& param)
{
    AddNetworkCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantP2pNetwork(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseAddUpnpServiceReq(const uint8_t* data, size_t length, AddUpnpServiceReqP2pIfaceParam& param)
{
    AddUpnpServiceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.version = msg.version();
        param.serviceName = msg.servicename();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseAddUpnpServiceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCancelConnectReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCancelConnectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCancelServiceDiscoveryReq(const uint8_t* data, size_t length, int64_t& param)
{
    CancelServiceDiscoveryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.identifier();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseCancelServiceDiscoveryCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCancelWpsReq(const uint8_t* data, size_t length, string& param)
{
    CancelWpsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.groupifname();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseCancelWpsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseConfigureExtListenReq(const uint8_t* data, size_t length, ConfigureExtListenReqP2pIfaceParam& param)
{
    ConfigureExtListenReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.periodInMillis = msg.periodinmillis();
        param.intervalInMillis = msg.intervalinmillis();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseConfigureExtListenCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseConnectReq(const uint8_t* data, size_t length, ConnectReqP2pIfaceParam& param)
{
    ConnectReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param.peerAddress);
        param.provisionMethod = (WpsProvisionMethod) msg.provisionmethod();
        param.preSelectedPin = msg.preselectedpin();
        param.joinExistingGroup = msg.joinexistinggroup();
        param.persistent = msg.persistent();
        param.goIntent = msg.gointent();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseConnectCfm(const uint8_t* data, size_t length, ConnectCfmP2pIfaceParam& param)
{
    ConnectCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageCfm(const uint8_t* data, size_t length, CreateNfcHandoverRequestMessageCfmP2pIfaceParam& param)
{
    CreateNfcHandoverRequestMessageCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageCfm(const uint8_t* data, size_t length, CreateNfcHandoverSelectMessageCfmP2pIfaceParam& param)
{
    CreateNfcHandoverSelectMessageCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseEnableWfdReq(const uint8_t* data, size_t length, bool& param)
{
    EnableWfdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseEnableWfdCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFindReq(const uint8_t* data, size_t length, int32_t& param)
{
    FindReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.timeoutinsec();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseFindCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFlushReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFlushCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFlushServicesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFlushServicesCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseGetDeviceAddressReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseGetDeviceAddressCfm(const uint8_t* data, size_t length, GetDeviceAddressCfmP2pIfaceParam& param)
{
    GetDeviceAddressCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetEdmgReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseGetEdmgCfm(const uint8_t* data, size_t length, GetEdmgCfmP2pIfaceParam& param)
{
    GetEdmgCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetGroupCapabilityReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    GetGroupCapabilityReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetGroupCapabilityCfm(const uint8_t* data, size_t length, GetGroupCapabilityCfmP2pIfaceParam& param)
{
    GetGroupCapabilityCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (P2pGroupCapabilityMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmP2pIfaceParam& param)
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

bool SupplicantP2pIfaceParseGetNetworkReq(const uint8_t* data, size_t length, int32_t& param)
{
    GetNetworkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetNetworkCfm(const uint8_t* data, size_t length, GetNetworkCfmP2pIfaceParam& param)
{
    GetNetworkCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ISupplicantP2pNetwork(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetSsidReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    GetSsidReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmP2pIfaceParam& param)
{
    GetSsidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseGetTypeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmP2pIfaceParam& param)
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

bool SupplicantP2pIfaceParseInviteReq(const uint8_t* data, size_t length, InviteReqP2pIfaceParam& param)
{
    InviteReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        String2Vector(msg.godeviceaddress(), param.goDeviceAddress);
        String2Vector(msg.peeraddress(), param.peerAddress);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseInviteCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseListNetworksReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseListNetworksCfm(const uint8_t* data, size_t length, ListNetworksCfmP2pIfaceParam& param)
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

bool SupplicantP2pIfaceParseProvisionDiscoveryReq(const uint8_t* data, size_t length, ProvisionDiscoveryReqP2pIfaceParam& param)
{
    ProvisionDiscoveryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param.peerAddress);
        param.provisionMethod = (WpsProvisionMethod) msg.provisionmethod();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseProvisionDiscoveryCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRegisterCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRegisterCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseReinvokeReq(const uint8_t* data, size_t length, ReinvokeReqP2pIfaceParam& param)
{
    ReinvokeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.persistentNetworkId = msg.persistentnetworkid();
        String2Vector(msg.peeraddress(), param.peerAddress);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseReinvokeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRejectReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    RejectReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRejectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRemoveBonjourServiceReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    RemoveBonjourServiceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.query(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRemoveBonjourServiceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRemoveGroupReq(const uint8_t* data, size_t length, string& param)
{
    RemoveGroupReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.groupifname();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRemoveGroupCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRemoveNetworkReq(const uint8_t* data, size_t length, int32_t& param)
{
    RemoveNetworkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRemoveNetworkCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRemoveUpnpServiceReq(const uint8_t* data, size_t length, RemoveUpnpServiceReqP2pIfaceParam& param)
{
    RemoveUpnpServiceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.version = msg.version();
        param.serviceName = msg.servicename();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRemoveUpnpServiceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseReportNfcHandoverInitiationReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    ReportNfcHandoverInitiationReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.select(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseReportNfcHandoverInitiationCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseReportNfcHandoverResponseReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    ReportNfcHandoverResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.request(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseReportNfcHandoverResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRequestServiceDiscoveryReq(const uint8_t* data, size_t length, RequestServiceDiscoveryReqP2pIfaceParam& param)
{
    RequestServiceDiscoveryReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param.peerAddress);
        String2Vector(msg.query(), param.query);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRequestServiceDiscoveryCfm(const uint8_t* data, size_t length, RequestServiceDiscoveryCfmP2pIfaceParam& param)
{
    RequestServiceDiscoveryCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSaveConfigReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSaveConfigCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2FreqRange(const FreqRange_P& msgSupplicantP2pIface, FreqRange& ranges)
{
    ranges.min = msgSupplicantP2pIface.min();
    ranges.max = msgSupplicantP2pIface.max();
}

bool SupplicantP2pIfaceParseSetDisallowedFrequenciesReq(const uint8_t* data, size_t length, vector<FreqRange>& param)
{
    SetDisallowedFrequenciesReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_ranges = msg.ranges_size();
        if (0 < size_ranges)
        {
            param.resize(size_ranges);
            for (int index = 0; index < size_ranges; index++)
            {
                Message2FreqRange(msg.ranges(index), param[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetDisallowedFrequenciesCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetEdmgReq(const uint8_t* data, size_t length, bool& param)
{
    SetEdmgReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetEdmgCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetGroupIdleReq(const uint8_t* data, size_t length, SetGroupIdleReqP2pIfaceParam& param)
{
    SetGroupIdleReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        param.timeoutInSec = msg.timeoutinsec();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetGroupIdleCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetListenChannelReq(const uint8_t* data, size_t length, SetListenChannelReqP2pIfaceParam& param)
{
    SetListenChannelReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.channel = msg.channel();
        param.operatingClass = msg.operatingclass();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetListenChannelCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetMacRandomizationReq(const uint8_t* data, size_t length, bool& param)
{
    SetMacRandomizationReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetMacRandomizationCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetMiracastModeReq(const uint8_t* data, size_t length, MiracastMode& param)
{
    SetMiracastModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (MiracastMode) msg.mode();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetMiracastModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetPowerSaveReq(const uint8_t* data, size_t length, SetPowerSaveReqP2pIfaceParam& param)
{
    SetPowerSaveReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        param.enable = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetPowerSaveCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetSsidPostfixReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetSsidPostfixReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.postfix(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetSsidPostfixCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWfdDeviceInfoReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetWfdDeviceInfoReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.info(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWfdDeviceInfoCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWfdR2DeviceInfoReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetWfdR2DeviceInfoReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.info(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWfdR2DeviceInfoCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseRemoveClientReq(const uint8_t* data, size_t length, RemoveClientReqP2pIfaceParam& param)
{
    RemoveClientReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.peeraddress(), param.peerAddress);
        param.isLegacyClient = msg.islegacyclient();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseRemoveClientCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsConfigMethodsReq(const uint8_t* data, size_t length, WpsConfigMethods& param)
{
    SetWpsConfigMethodsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (WpsConfigMethods) msg.configmethods();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsConfigMethodsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsDeviceNameReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsDeviceNameReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.name();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsDeviceNameCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsDeviceTypeReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetWpsDeviceTypeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.type(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsDeviceTypeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsManufacturerReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsManufacturerReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.manufacturer();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsManufacturerCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsModelNameReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsModelNameReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modelname();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsModelNameCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsModelNumberReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsModelNumberReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modelnumber();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsModelNumberCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetWpsSerialNumberReq(const uint8_t* data, size_t length, string& param)
{
    SetWpsSerialNumberReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.serialnumber();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetWpsSerialNumberCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseStartWpsPbcReq(const uint8_t* data, size_t length, StartWpsPbcReqP2pIfaceParam& param)
{
    StartWpsPbcReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        String2Vector(msg.bssid(), param.bssid);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseStartWpsPbcCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseStartWpsPinDisplayReq(const uint8_t* data, size_t length, StartWpsPinDisplayReqP2pIfaceParam& param)
{
    StartWpsPinDisplayReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        String2Vector(msg.bssid(), param.bssid);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseStartWpsPinDisplayCfm(const uint8_t* data, size_t length, StartWpsPinDisplayCfmP2pIfaceParam& param)
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

bool SupplicantP2pIfaceParseStartWpsPinKeypadReq(const uint8_t* data, size_t length, StartWpsPinKeypadReqP2pIfaceParam& param)
{
    StartWpsPinKeypadReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfName = msg.groupifname();
        param.pin = msg.pin();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseStartWpsPinKeypadCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseStopFindReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseStopFindCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFindOnSocialChannelsReq(const uint8_t* data, size_t length, int32_t& param)
{
    FindOnSocialChannelsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.timeoutinsec();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseFindOnSocialChannelsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseFindOnSpecificFrequencyReq(const uint8_t* data, size_t length, FindOnSpecificFrequencyReqP2pIfaceParam& param)
{
    FindOnSpecificFrequencyReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.freqInHz = msg.freqinhz();
        param.timeoutInSec = msg.timeoutinsec();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseFindOnSpecificFrequencyCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseSetVendorElementsReq(const uint8_t* data, size_t length, SetVendorElementsReqP2pIfaceParam& param)
{
    SetVendorElementsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.frameTypeMask = (P2pFrameTypeMask) msg.frametypemask();
        String2Vector(msg.vendorelembytes(), param.vendorElemBytes);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseSetVendorElementsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsReq(const uint8_t* data, size_t length, ConfigureEapolIpAddressAllocationParamsReqP2pIfaceParam& param)
{
    ConfigureEapolIpAddressAllocationParamsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ipAddressGo = msg.ipaddressgo();
        param.ipAddressMask = msg.ipaddressmask();
        param.ipAddressStart = msg.ipaddressstart();
        param.ipAddressEnd = msg.ipaddressend();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseOnDeviceFoundInd(const uint8_t* data, size_t length, OnDeviceFoundIndP2pIfaceParam& param)
{
    OnDeviceFoundInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        String2Vector(msg.primarydevicetype(), param.primaryDeviceType);
        param.deviceName = msg.devicename();
        param.configMethods = (WpsConfigMethods) msg.configmethods();
        param.deviceCapabilities = msg.devicecapabilities();
        param.groupCapabilities = (P2pGroupCapabilityMask) msg.groupcapabilities();
        String2Vector(msg.wfddeviceinfo(), param.wfdDeviceInfo);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnDeviceLostInd(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    OnDeviceLostInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.p2pdeviceaddress(), param);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnFindStoppedInd(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseOnGoNegotiationCompletedInd(const uint8_t* data, size_t length, P2pStatusCode& param)
{
    OnGoNegotiationCompletedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (P2pStatusCode) msg.status();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnGoNegotiationRequestInd(const uint8_t* data, size_t length, OnGoNegotiationRequestIndP2pIfaceParam& param)
{
    OnGoNegotiationRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        param.passwordId = (WpsDevPasswordId) msg.passwordid();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnGroupFormationFailureInd(const uint8_t* data, size_t length, string& param)
{
    OnGroupFormationFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.failurereason();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnGroupFormationSuccessInd(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantP2pIfaceParseOnGroupRemovedInd(const uint8_t* data, size_t length, OnGroupRemovedIndP2pIfaceParam& param)
{
    OnGroupRemovedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfname = msg.groupifname();
        param.isGroupOwner = msg.isgroupowner();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnGroupStartedInd(const uint8_t* data, size_t length, OnGroupStartedIndP2pIfaceParam& param)
{
    OnGroupStartedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfname = msg.groupifname();
        param.isGroupOwner = msg.isgroupowner();
        String2Vector(msg.ssid(), param.ssid);
        param.frequency = msg.frequency();
        String2Vector(msg.psk(), param.psk);
        param.passphrase = msg.passphrase();
        String2Vector(msg.godeviceaddress(), param.goDeviceAddress);
        param.isPersistent = msg.ispersistent();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnInvitationReceivedInd(const uint8_t* data, size_t length, OnInvitationReceivedIndP2pIfaceParam& param)
{
    OnInvitationReceivedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.godeviceaddress(), param.goDeviceAddress);
        String2Vector(msg.bssid(), param.bssid);
        param.persistentNetworkId = msg.persistentnetworkid();
        param.operatingFrequency = msg.operatingfrequency();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnInvitationResultInd(const uint8_t* data, size_t length, OnInvitationResultIndP2pIfaceParam& param)
{
    OnInvitationResultInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param.bssid);
        param.status = (P2pStatusCode) msg.status();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnProvisionDiscoveryCompletedInd(const uint8_t* data, size_t length, OnProvisionDiscoveryCompletedIndP2pIfaceParam& param)
{
    OnProvisionDiscoveryCompletedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        param.isRequest = msg.isrequest();
        param.status = (P2pProvDiscStatusCode) msg.status();
        param.configMethods = (WpsConfigMethods) msg.configmethods();
        param.generatedPin = msg.generatedpin();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnR2DeviceFoundInd(const uint8_t* data, size_t length, OnR2DeviceFoundIndP2pIfaceParam& param)
{
    OnR2DeviceFoundInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        String2Vector(msg.primarydevicetype(), param.primaryDeviceType);
        param.deviceName = msg.devicename();
        param.configMethods = (WpsConfigMethods) msg.configmethods();
        param.deviceCapabilities = msg.devicecapabilities();
        param.groupCapabilities = (P2pGroupCapabilityMask) msg.groupcapabilities();
        String2Vector(msg.wfddeviceinfo(), param.wfdDeviceInfo);
        String2Vector(msg.wfdr2deviceinfo(), param.wfdR2DeviceInfo);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnServiceDiscoveryResponseInd(const uint8_t* data, size_t length, OnServiceDiscoveryResponseIndP2pIfaceParam& param)
{
    OnServiceDiscoveryResponseInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        param.updateIndicator = msg.updateindicator();
        String2Vector(msg.tlvs(), param.tlvs);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnStaAuthorizedInd(const uint8_t* data, size_t length, OnStaAuthorizedIndP2pIfaceParam& param)
{
    OnStaAuthorizedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnStaDeauthorizedInd(const uint8_t* data, size_t length, OnStaDeauthorizedIndP2pIfaceParam& param)
{
    OnStaDeauthorizedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnGroupFrequencyChangedInd(const uint8_t* data, size_t length, OnGroupFrequencyChangedIndP2pIfaceParam& param)
{
    OnGroupFrequencyChangedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.groupIfname = msg.groupifname();
        param.frequency = msg.frequency();
        return true;
    }
    return false;
}

bool SupplicantP2pIfaceParseOnDeviceFoundWithVendorElementsInd(const uint8_t* data, size_t length, OnDeviceFoundWithVendorElementsIndP2pIfaceParam& param)
{
    OnDeviceFoundWithVendorElementsInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.srcaddress(), param.srcAddress);
        String2Vector(msg.p2pdeviceaddress(), param.p2pDeviceAddress);
        String2Vector(msg.primarydevicetype(), param.primaryDeviceType);
        param.deviceName = msg.devicename();
        param.configMethods = (WpsConfigMethods) msg.configmethods();
        param.deviceCapabilities = msg.devicecapabilities();
        param.groupCapabilities = (P2pGroupCapabilityMask) msg.groupcapabilities();
        String2Vector(msg.wfddeviceinfo(), param.wfdDeviceInfo);
        String2Vector(msg.wfdr2deviceinfo(), param.wfdR2DeviceInfo);
        String2Vector(msg.vendorelembytes(), param.vendorElemBytes);
        return true;
    }
    return false;
}

static void Message2P2pClientEapolIpAddressInfo(const P2pClientEapolIpAddressInfo_P& msgSupplicantP2pIface, P2pClientEapolIpAddressInfo& p2pClientIpInfo)
{
    p2pClientIpInfo.ipAddressClient = msgSupplicantP2pIface.ipaddressclient();
    p2pClientIpInfo.ipAddressMask = msgSupplicantP2pIface.ipaddressmask();
    p2pClientIpInfo.ipAddressGo = msgSupplicantP2pIface.ipaddressgo();
}

static void Message2P2pGroupStartedEventParams(const P2pGroupStartedEventParams_P& msgSupplicantP2pIface, P2pGroupStartedEventParams& groupStartedEventParams)
{
    groupStartedEventParams.groupInterfaceName = msgSupplicantP2pIface.groupinterfacename();
    groupStartedEventParams.isGroupOwner = msgSupplicantP2pIface.isgroupowner();
    String2Vector(msgSupplicantP2pIface.ssid(), groupStartedEventParams.ssid);
    groupStartedEventParams.frequencyMHz = msgSupplicantP2pIface.frequencymhz();
    String2Vector(msgSupplicantP2pIface.psk(), groupStartedEventParams.psk);
    groupStartedEventParams.passphrase = msgSupplicantP2pIface.passphrase();
    groupStartedEventParams.isPersistent = msgSupplicantP2pIface.ispersistent();
    String2Array(msgSupplicantP2pIface.godeviceaddress(), groupStartedEventParams.goDeviceAddress);
    String2Array(msgSupplicantP2pIface.gointerfaceaddress(), groupStartedEventParams.goInterfaceAddress);
    groupStartedEventParams.isP2pClientEapolIpAddressInfoPresent = msgSupplicantP2pIface.isp2pclienteapolipaddressinfopresent();
    Message2P2pClientEapolIpAddressInfo(msgSupplicantP2pIface.p2pclientipinfo(), groupStartedEventParams.p2pClientIpInfo);
}

bool SupplicantP2pIfaceParseOnGroupStartedWithParamsInd(const uint8_t* data, size_t length, P2pGroupStartedEventParams& param)
{
    OnGroupStartedWithParamsInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2P2pGroupStartedEventParams(msg.groupstartedeventparams(), param);
        return true;
    }
    return false;
}
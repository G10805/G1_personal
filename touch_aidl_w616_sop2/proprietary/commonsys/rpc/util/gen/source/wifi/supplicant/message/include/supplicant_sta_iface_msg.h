/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/ISupplicantStaIface.h>
#include <aidl/android/hardware/wifi/supplicant/ISupplicantStaIfaceCallback.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddDppPeerUriCfmStaIfaceParam;

typedef struct
{
    string name;
    int32_t freqInMhz;
    int32_t timeoutInSec;
} AddExtRadioWorkReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddExtRadioWorkCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddNetworkCfmStaIfaceParam;

typedef struct
{
    vector<uint8_t> dst_mac;
    vector<uint8_t> pkt;
} FilsHlpAddRequestReqStaIfaceParam;

typedef struct
{
    vector<uint8_t> macAddress;
    string deviceInfo;
    DppCurve curve;
} GenerateDppBootstrapInfoForResponderReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    DppResponderBootstrapInfo result;
} GenerateDppBootstrapInfoForResponderCfmStaIfaceParam;

typedef struct
{
    string ssid;
    vector<uint8_t> privEcKey;
} GenerateSelfDppConfigurationReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    ConnectionCapabilities result;
} GetConnectionCapabilitiesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    MloLinksInfo result;
} GetConnectionMloLinksInfoCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    KeyMgmtMask result;
} GetKeyMgmtCapabilitiesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetMacAddressCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetNetworkCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    IfaceType result;
} GetTypeCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    WpaDriverCapabilitiesMask result;
} GetWpaDriverCapabilitiesCfmStaIfaceParam;

typedef struct
{
    vector<uint8_t> macAddress;
    vector<AnqpInfoId> infoElements;
    vector<Hs20AnqpSubtypes> subTypes;
} InitiateAnqpQueryReqStaIfaceParam;

typedef struct
{
    vector<uint8_t> macAddress;
    string fileName;
} InitiateHs20IconQueryReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<int32_t> result;
} ListNetworksCfmStaIfaceParam;

typedef struct
{
    int32_t qosPolicyRequestId;
    bool morePolicies;
    vector<QosPolicyStatus> qosPolicyStatusList;
} SendQosPolicyResponseReqStaIfaceParam;

typedef struct
{
    int32_t peerBootstrapId;
    int32_t ownBootstrapId;
    string ssid;
    string password;
    string psk;
    DppNetRole netRole;
    DppAkm securityAkm;
    vector<uint8_t> privEcKey;
} StartDppConfiguratorInitiatorReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} StartDppConfiguratorInitiatorCfmStaIfaceParam;

typedef struct
{
    int32_t peerBootstrapId;
    int32_t ownBootstrapId;
} StartDppEnrolleeInitiatorReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    string result;
} StartWpsPinDisplayCfmStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    string pin;
} StartWpsRegistrarReqStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<SignalPollResult> result;
} GetSignalPollResultsCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<QosPolicyScsRequestStatus> result;
} AddQosPolicyRequestForScsCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<QosPolicyScsRequestStatus> result;
} RemoveQosPolicyForScsCfmStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    AnqpData data;
    Hs20AnqpData hs20Data;
} OnAnqpQueryDoneIndStaIfaceParam;

typedef struct
{
    AuxiliarySupplicantEventCode eventCode;
    vector<uint8_t> bssid;
    string reasonString;
} OnAuxiliarySupplicantEventIndStaIfaceParam;

typedef struct
{
    BssidChangeReason reason;
    vector<uint8_t> bssid;
} OnBssidChangedIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    bool locallyGenerated;
    StaIfaceReasonCode reasonCode;
} OnDisconnectedIndStaIfaceParam;

typedef struct
{
    DppFailureCode code;
    string ssid;
    string channelList;
    vector<char16_t> bandList;
} OnDppFailureIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> ssid;
    string password;
    vector<uint8_t> psk;
    DppAkm securityAkm;
    DppConnectionKeys dppConnectionKeys;
} OnDppSuccessConfigReceivedIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    int32_t errorCode;
} OnEapFailureIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    int32_t reasonCode;
    int32_t reAuthDelayInSec;
    string url;
} OnHs20DeauthImminentNoticeIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    string fileName;
    vector<uint8_t> data;
} OnHs20IconQueryDoneIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    OsuMethod osuMethod;
    string url;
} OnHs20SubscriptionRemediationIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    string url;
} OnHs20TermsAndConditionsAcceptanceRequestedNotificationIndStaIfaceParam;

typedef struct
{
    int64_t expirationTimeInSec;
    vector<uint8_t> serializedEntry;
} OnPmkCacheAddedIndStaIfaceParam;

typedef struct
{
    StaIfaceCallbackState newState;
    vector<uint8_t> bssid;
    int32_t id;
    vector<uint8_t> ssid;
    bool filsHlpSent;
} OnStateChangedIndStaIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    WpsConfigError configError;
    WpsErrorIndication errorInd;
} OnWpsEventFailIndStaIfaceParam;

typedef struct
{
    int32_t qosPolicyRequestId;
    vector<QosPolicyData> qosPolicyData;
} OnQosPolicyRequestIndStaIfaceParam;

bool SupplicantStaIfaceSerializeAddDppPeerUriReq(const string& uri, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddDppPeerUriCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddExtRadioWorkReq(const string& name, int32_t freqInMhz, int32_t timeoutInSec, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddExtRadioWorkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddNetworkReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddRxFilterReq(RxFilterType type, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddRxFilterCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeCancelWpsReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeCancelWpsCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeDisconnectReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeDisconnectCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeEnableAutoReconnectReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeEnableAutoReconnectCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeFilsHlpAddRequestReq(const vector<uint8_t>& dst_mac, const vector<uint8_t>& pkt, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeFilsHlpAddRequestCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeFilsHlpFlushRequestReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeFilsHlpFlushRequestCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderReq(const vector<uint8_t>& macAddress, const string& deviceInfo, DppCurve curve, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGenerateDppBootstrapInfoForResponderCfm(const HalStatusParam& status, const DppResponderBootstrapInfo& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGenerateSelfDppConfigurationReq(const string& ssid, const vector<uint8_t>& privEcKey, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGenerateSelfDppConfigurationCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetConnectionCapabilitiesReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetConnectionCapabilitiesCfm(const HalStatusParam& status, const ConnectionCapabilities& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetConnectionMloLinksInfoReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetConnectionMloLinksInfoCfm(const HalStatusParam& status, const MloLinksInfo& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetKeyMgmtCapabilitiesCfm(const HalStatusParam& status, KeyMgmtMask result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetMacAddressReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetMacAddressCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetNetworkReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetTypeReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetWpaDriverCapabilitiesCfm(const HalStatusParam& status, WpaDriverCapabilitiesMask result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateAnqpQueryReq(const vector<uint8_t>& macAddress, const vector<AnqpInfoId>& infoElements, const vector<Hs20AnqpSubtypes>& subTypes, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateAnqpQueryCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateHs20IconQueryReq(const vector<uint8_t>& macAddress, const string& fileName, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateHs20IconQueryCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsDiscoverReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsDiscoverCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsSetupReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsSetupCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsTeardownReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateTdlsTeardownCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryReq(const vector<uint8_t>& macAddress, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeInitiateVenueUrlAnqpQueryCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeListNetworksReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeListNetworksCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeReassociateReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeReassociateCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeReconnectReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeReconnectCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRegisterCallbackReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRegisterCallbackCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetQosPolicyFeatureEnabledCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSendQosPolicyResponseReq(int32_t qosPolicyRequestId, bool morePolicies, const vector<QosPolicyStatus>& qosPolicyStatusList, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSendQosPolicyResponseCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveAllQosPoliciesReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveAllQosPoliciesCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveDppUriReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveDppUriCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveExtRadioWorkReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveExtRadioWorkCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveNetworkReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveNetworkCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveRxFilterReq(RxFilterType type, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveRxFilterCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetBtCoexistenceModeReq(BtCoexistenceMode mode, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetBtCoexistenceModeCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetBtCoexistenceScanModeEnabledCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetCountryCodeReq(const vector<uint8_t>& code, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetCountryCodeCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetExternalSimReq(bool useExternalSim, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetExternalSimCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetMboCellularDataStatusReq(bool available, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetMboCellularDataStatusCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetPowerSaveReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetPowerSaveCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetSuspendModeEnabledReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetSuspendModeEnabledCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsConfigMethodsReq(WpsConfigMethods configMethods, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsConfigMethodsCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsDeviceNameReq(const string& name, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsDeviceNameCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsDeviceTypeReq(const vector<uint8_t>& type, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsDeviceTypeCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsManufacturerReq(const string& manufacturer, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsManufacturerCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsModelNameReq(const string& modelName, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsModelNameCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsModelNumberReq(const string& modelNumber, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsModelNumberCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsSerialNumberReq(const string& serialNumber, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeSetWpsSerialNumberCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorReq(int32_t peerBootstrapId, int32_t ownBootstrapId, const string& ssid, const string& password, const string& psk, DppNetRole netRole, DppAkm securityAkm, const vector<uint8_t>& privEcKey, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppConfiguratorInitiatorCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorReq(int32_t peerBootstrapId, int32_t ownBootstrapId, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppEnrolleeInitiatorCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppEnrolleeResponderReq(int32_t listenChannel, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartDppEnrolleeResponderCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartRxFilterReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartRxFilterCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPbcReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPbcCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPinDisplayReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPinDisplayCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPinKeypadReq(const string& pin, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsPinKeypadCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsRegistrarReq(const vector<uint8_t>& bssid, const string& pin, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStartWpsRegistrarCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopDppInitiatorReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopDppInitiatorCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopDppResponderReq(int32_t ownBootstrapId, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopDppResponderCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopRxFilterReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeStopRxFilterCfm(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetSignalPollResultsReq(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeGetSignalPollResultsCfm(const HalStatusParam& status, const vector<SignalPollResult>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddQosPolicyRequestForScsReq(const vector<QosPolicyScsData>& qosPolicyData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeAddQosPolicyRequestForScsCfm(const HalStatusParam& status, const vector<QosPolicyScsRequestStatus>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveQosPolicyForScsReq(const vector<uint8_t>& scsPolicyIds, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeRemoveQosPolicyForScsCfm(const HalStatusParam& status, const vector<QosPolicyScsRequestStatus>& result, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnAnqpQueryDoneInd(const vector<uint8_t>& bssid, const AnqpData& data, const Hs20AnqpData& hs20Data, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnAssociationRejectedInd(const AssociationRejectionData& assocRejectData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnAuthenticationTimeoutInd(const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnAuxiliarySupplicantEventInd(AuxiliarySupplicantEventCode eventCode, const vector<uint8_t>& bssid, const string& reasonString, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnBssTmHandlingDoneInd(const BssTmData& tmData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnBssidChangedInd(BssidChangeReason reason, const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDisconnectedInd(const vector<uint8_t>& bssid, bool locallyGenerated, StaIfaceReasonCode reasonCode, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppFailureInd(DppFailureCode code, const string& ssid, const string& channelList, const vector<char16_t>& bandList, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppProgressInd(DppProgressCode code, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppSuccessInd(DppEventType event, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppSuccessConfigReceivedInd(const vector<uint8_t>& ssid, const string& password, const vector<uint8_t>& psk, DppAkm securityAkm, const DppConnectionKeys& dppConnectionKeys, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppSuccessConfigSentInd(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnEapFailureInd(const vector<uint8_t>& bssid, int32_t errorCode, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnExtRadioWorkStartInd(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnExtRadioWorkTimeoutInd(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnHs20DeauthImminentNoticeInd(const vector<uint8_t>& bssid, int32_t reasonCode, int32_t reAuthDelayInSec, const string& url, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnHs20IconQueryDoneInd(const vector<uint8_t>& bssid, const string& fileName, const vector<uint8_t>& data, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnHs20SubscriptionRemediationInd(const vector<uint8_t>& bssid, OsuMethod osuMethod, const string& url, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(const vector<uint8_t>& bssid, const string& url, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnNetworkAddedInd(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnNetworkNotFoundInd(const vector<uint8_t>& ssid, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnNetworkRemovedInd(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnPmkCacheAddedInd(int64_t expirationTimeInSec, const vector<uint8_t>& serializedEntry, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnStateChangedInd(StaIfaceCallbackState newState, const vector<uint8_t>& bssid, int32_t id, const vector<uint8_t>& ssid, bool filsHlpSent, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnWpsEventFailInd(const vector<uint8_t>& bssid, WpsConfigError configError, WpsErrorIndication errorInd, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnWpsEventPbcOverlapInd(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnWpsEventSuccessInd(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnQosPolicyResetInd(vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnQosPolicyRequestInd(int32_t qosPolicyRequestId, const vector<QosPolicyData>& qosPolicyData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnMloLinksInfoChangedInd(ISupplicantStaIfaceCallback::MloLinkInfoChangeReason reason, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppConfigReceivedInd(const DppConfigurationData& configData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnDppConnectionStatusResultSentInd(DppStatusErrorCode code, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnBssFrequencyChangedInd(int32_t frequencyMhz, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnSupplicantStateChangedInd(const SupplicantStateChangeData& stateChangeData, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnQosPolicyResponseForScsInd(const vector<QosPolicyScsResponseStatus>& qosPolicyScsResponseStatus, vector<uint8_t>& payload);

bool SupplicantStaIfaceSerializeOnPmkSaCacheAddedInd(const PmkSaCacheData& pmkSaData, vector<uint8_t>& payload);

bool SupplicantStaIfaceParseAddDppPeerUriReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseAddDppPeerUriCfm(const uint8_t* data, size_t length, AddDppPeerUriCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseAddExtRadioWorkReq(const uint8_t* data, size_t length, AddExtRadioWorkReqStaIfaceParam& param);

bool SupplicantStaIfaceParseAddExtRadioWorkCfm(const uint8_t* data, size_t length, AddExtRadioWorkCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseAddNetworkReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseAddNetworkCfm(const uint8_t* data, size_t length, AddNetworkCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseAddRxFilterReq(const uint8_t* data, size_t length, RxFilterType& param);

bool SupplicantStaIfaceParseAddRxFilterCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseCancelWpsReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseCancelWpsCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseDisconnectReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseDisconnectCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseEnableAutoReconnectReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseEnableAutoReconnectCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseFilsHlpAddRequestReq(const uint8_t* data, size_t length, FilsHlpAddRequestReqStaIfaceParam& param);

bool SupplicantStaIfaceParseFilsHlpAddRequestCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseFilsHlpFlushRequestReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseFilsHlpFlushRequestCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderReq(const uint8_t* data, size_t length, GenerateDppBootstrapInfoForResponderReqStaIfaceParam& param);

bool SupplicantStaIfaceParseGenerateDppBootstrapInfoForResponderCfm(const uint8_t* data, size_t length, GenerateDppBootstrapInfoForResponderCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGenerateSelfDppConfigurationReq(const uint8_t* data, size_t length, GenerateSelfDppConfigurationReqStaIfaceParam& param);

bool SupplicantStaIfaceParseGenerateSelfDppConfigurationCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetConnectionCapabilitiesReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetConnectionCapabilitiesCfm(const uint8_t* data, size_t length, GetConnectionCapabilitiesCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetConnectionMloLinksInfoReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetConnectionMloLinksInfoCfm(const uint8_t* data, size_t length, GetConnectionMloLinksInfoCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetKeyMgmtCapabilitiesReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetKeyMgmtCapabilitiesCfm(const uint8_t* data, size_t length, GetKeyMgmtCapabilitiesCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetMacAddressReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetMacAddressCfm(const uint8_t* data, size_t length, GetMacAddressCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetNetworkReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseGetNetworkCfm(const uint8_t* data, size_t length, GetNetworkCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetTypeReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseGetWpaDriverCapabilitiesReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetWpaDriverCapabilitiesCfm(const uint8_t* data, size_t length, GetWpaDriverCapabilitiesCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseInitiateAnqpQueryReq(const uint8_t* data, size_t length, InitiateAnqpQueryReqStaIfaceParam& param);

bool SupplicantStaIfaceParseInitiateAnqpQueryCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseInitiateHs20IconQueryReq(const uint8_t* data, size_t length, InitiateHs20IconQueryReqStaIfaceParam& param);

bool SupplicantStaIfaceParseInitiateHs20IconQueryCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseInitiateTdlsDiscoverReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseInitiateTdlsDiscoverCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseInitiateTdlsSetupReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseInitiateTdlsSetupCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseInitiateTdlsTeardownReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseInitiateTdlsTeardownCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseInitiateVenueUrlAnqpQueryCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseListNetworksReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseListNetworksCfm(const uint8_t* data, size_t length, ListNetworksCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseReassociateReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseReassociateCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseReconnectReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseReconnectCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRegisterCallbackReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRegisterCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetQosPolicyFeatureEnabledReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetQosPolicyFeatureEnabledCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSendQosPolicyResponseReq(const uint8_t* data, size_t length, SendQosPolicyResponseReqStaIfaceParam& param);

bool SupplicantStaIfaceParseSendQosPolicyResponseCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveAllQosPoliciesReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveAllQosPoliciesCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveDppUriReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseRemoveDppUriCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveExtRadioWorkReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseRemoveExtRadioWorkCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveNetworkReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseRemoveNetworkCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseRemoveRxFilterReq(const uint8_t* data, size_t length, RxFilterType& param);

bool SupplicantStaIfaceParseRemoveRxFilterCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetBtCoexistenceModeReq(const uint8_t* data, size_t length, BtCoexistenceMode& param);

bool SupplicantStaIfaceParseSetBtCoexistenceModeCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetBtCoexistenceScanModeEnabledCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetCountryCodeReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseSetCountryCodeCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetExternalSimReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetExternalSimCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetMboCellularDataStatusReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetMboCellularDataStatusCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetPowerSaveReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetPowerSaveCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetSuspendModeEnabledReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaIfaceParseSetSuspendModeEnabledCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsConfigMethodsReq(const uint8_t* data, size_t length, WpsConfigMethods& param);

bool SupplicantStaIfaceParseSetWpsConfigMethodsCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsDeviceNameReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseSetWpsDeviceNameCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsDeviceTypeReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseSetWpsDeviceTypeCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsManufacturerReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseSetWpsManufacturerCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsModelNameReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseSetWpsModelNameCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsModelNumberReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseSetWpsModelNumberCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseSetWpsSerialNumberReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseSetWpsSerialNumberCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartDppConfiguratorInitiatorReq(const uint8_t* data, size_t length, StartDppConfiguratorInitiatorReqStaIfaceParam& param);

bool SupplicantStaIfaceParseStartDppConfiguratorInitiatorCfm(const uint8_t* data, size_t length, StartDppConfiguratorInitiatorCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseStartDppEnrolleeInitiatorReq(const uint8_t* data, size_t length, StartDppEnrolleeInitiatorReqStaIfaceParam& param);

bool SupplicantStaIfaceParseStartDppEnrolleeInitiatorCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartDppEnrolleeResponderReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseStartDppEnrolleeResponderCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartRxFilterReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartRxFilterCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartWpsPbcReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseStartWpsPbcCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartWpsPinDisplayReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseStartWpsPinDisplayCfm(const uint8_t* data, size_t length, StartWpsPinDisplayCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseStartWpsPinKeypadReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaIfaceParseStartWpsPinKeypadCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStartWpsRegistrarReq(const uint8_t* data, size_t length, StartWpsRegistrarReqStaIfaceParam& param);

bool SupplicantStaIfaceParseStartWpsRegistrarCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStopDppInitiatorReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStopDppInitiatorCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStopDppResponderReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseStopDppResponderCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStopRxFilterReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseStopRxFilterCfm(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetSignalPollResultsReq(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseGetSignalPollResultsCfm(const uint8_t* data, size_t length, GetSignalPollResultsCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseAddQosPolicyRequestForScsReq(const uint8_t* data, size_t length, vector<QosPolicyScsData>& param);

bool SupplicantStaIfaceParseAddQosPolicyRequestForScsCfm(const uint8_t* data, size_t length, AddQosPolicyRequestForScsCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseRemoveQosPolicyForScsReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseRemoveQosPolicyForScsCfm(const uint8_t* data, size_t length, RemoveQosPolicyForScsCfmStaIfaceParam& param);

bool SupplicantStaIfaceParseOnAnqpQueryDoneInd(const uint8_t* data, size_t length, OnAnqpQueryDoneIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnAssociationRejectedInd(const uint8_t* data, size_t length, AssociationRejectionData& param);

bool SupplicantStaIfaceParseOnAuthenticationTimeoutInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseOnAuxiliarySupplicantEventInd(const uint8_t* data, size_t length, OnAuxiliarySupplicantEventIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnBssTmHandlingDoneInd(const uint8_t* data, size_t length, BssTmData& param);

bool SupplicantStaIfaceParseOnBssidChangedInd(const uint8_t* data, size_t length, OnBssidChangedIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnDisconnectedInd(const uint8_t* data, size_t length, OnDisconnectedIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnDppFailureInd(const uint8_t* data, size_t length, OnDppFailureIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnDppProgressInd(const uint8_t* data, size_t length, DppProgressCode& param);

bool SupplicantStaIfaceParseOnDppSuccessInd(const uint8_t* data, size_t length, DppEventType& param);

bool SupplicantStaIfaceParseOnDppSuccessConfigReceivedInd(const uint8_t* data, size_t length, OnDppSuccessConfigReceivedIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnDppSuccessConfigSentInd(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseOnEapFailureInd(const uint8_t* data, size_t length, OnEapFailureIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnExtRadioWorkStartInd(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseOnExtRadioWorkTimeoutInd(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseOnHs20DeauthImminentNoticeInd(const uint8_t* data, size_t length, OnHs20DeauthImminentNoticeIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnHs20IconQueryDoneInd(const uint8_t* data, size_t length, OnHs20IconQueryDoneIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnHs20SubscriptionRemediationInd(const uint8_t* data, size_t length, OnHs20SubscriptionRemediationIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnHs20TermsAndConditionsAcceptanceRequestedNotificationInd(const uint8_t* data, size_t length, OnHs20TermsAndConditionsAcceptanceRequestedNotificationIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnNetworkAddedInd(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseOnNetworkNotFoundInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaIfaceParseOnNetworkRemovedInd(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseOnPmkCacheAddedInd(const uint8_t* data, size_t length, OnPmkCacheAddedIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnStateChangedInd(const uint8_t* data, size_t length, OnStateChangedIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnWpsEventFailInd(const uint8_t* data, size_t length, OnWpsEventFailIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnWpsEventPbcOverlapInd(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseOnWpsEventSuccessInd(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseOnQosPolicyResetInd(const uint8_t* data, size_t length);

bool SupplicantStaIfaceParseOnQosPolicyRequestInd(const uint8_t* data, size_t length, OnQosPolicyRequestIndStaIfaceParam& param);

bool SupplicantStaIfaceParseOnMloLinksInfoChangedInd(const uint8_t* data, size_t length, ISupplicantStaIfaceCallback::MloLinkInfoChangeReason& param);

bool SupplicantStaIfaceParseOnDppConfigReceivedInd(const uint8_t* data, size_t length, DppConfigurationData& param);

bool SupplicantStaIfaceParseOnDppConnectionStatusResultSentInd(const uint8_t* data, size_t length, DppStatusErrorCode& param);

bool SupplicantStaIfaceParseOnBssFrequencyChangedInd(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaIfaceParseOnSupplicantStateChangedInd(const uint8_t* data, size_t length, SupplicantStateChangeData& param);

bool SupplicantStaIfaceParseOnQosPolicyResponseForScsInd(const uint8_t* data, size_t length, vector<QosPolicyScsResponseStatus>& param);

bool SupplicantStaIfaceParseOnPmkSaCacheAddedInd(const uint8_t* data, size_t length, PmkSaCacheData& param);
/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/ISupplicantP2pIface.h>
#include <aidl/android/hardware/wifi/supplicant/ISupplicantP2pIfaceCallback.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


typedef struct
{
    vector<uint8_t> query;
    vector<uint8_t> response;
} AddBonjourServiceReqP2pIfaceParam;

typedef struct
{
    bool persistent;
    int32_t persistentNetworkId;
} AddGroupReqP2pIfaceParam;

typedef struct
{
    vector<uint8_t> ssid;
    string pskPassphrase;
    bool persistent;
    int32_t freq;
    vector<uint8_t> peerAddress;
    bool joinExistingGroup;
} AddGroupWithConfigReqP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddNetworkCfmP2pIfaceParam;

typedef struct
{
    int32_t version;
    string serviceName;
} AddUpnpServiceReqP2pIfaceParam;

typedef struct
{
    int32_t periodInMillis;
    int32_t intervalInMillis;
} ConfigureExtListenReqP2pIfaceParam;

typedef struct
{
    vector<uint8_t> peerAddress;
    WpsProvisionMethod provisionMethod;
    string preSelectedPin;
    bool joinExistingGroup;
    bool persistent;
    int32_t goIntent;
} ConnectReqP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    string result;
} ConnectCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} CreateNfcHandoverRequestMessageCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} CreateNfcHandoverSelectMessageCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetDeviceAddressCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} GetEdmgCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    P2pGroupCapabilityMask result;
} GetGroupCapabilityCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetNetworkCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetSsidCfmP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    IfaceType result;
} GetTypeCfmP2pIfaceParam;

typedef struct
{
    string groupIfName;
    vector<uint8_t> goDeviceAddress;
    vector<uint8_t> peerAddress;
} InviteReqP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<int32_t> result;
} ListNetworksCfmP2pIfaceParam;

typedef struct
{
    vector<uint8_t> peerAddress;
    WpsProvisionMethod provisionMethod;
} ProvisionDiscoveryReqP2pIfaceParam;

typedef struct
{
    int32_t persistentNetworkId;
    vector<uint8_t> peerAddress;
} ReinvokeReqP2pIfaceParam;

typedef struct
{
    int32_t version;
    string serviceName;
} RemoveUpnpServiceReqP2pIfaceParam;

typedef struct
{
    vector<uint8_t> peerAddress;
    vector<uint8_t> query;
} RequestServiceDiscoveryReqP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    int64_t result;
} RequestServiceDiscoveryCfmP2pIfaceParam;

typedef struct
{
    string groupIfName;
    int32_t timeoutInSec;
} SetGroupIdleReqP2pIfaceParam;

typedef struct
{
    int32_t channel;
    int32_t operatingClass;
} SetListenChannelReqP2pIfaceParam;

typedef struct
{
    string groupIfName;
    bool enable;
} SetPowerSaveReqP2pIfaceParam;

typedef struct
{
    vector<uint8_t> peerAddress;
    bool isLegacyClient;
} RemoveClientReqP2pIfaceParam;

typedef struct
{
    string groupIfName;
    vector<uint8_t> bssid;
} StartWpsPbcReqP2pIfaceParam;

typedef struct
{
    string groupIfName;
    vector<uint8_t> bssid;
} StartWpsPinDisplayReqP2pIfaceParam;

typedef struct
{
    HalStatusParam status;
    string result;
} StartWpsPinDisplayCfmP2pIfaceParam;

typedef struct
{
    string groupIfName;
    string pin;
} StartWpsPinKeypadReqP2pIfaceParam;

typedef struct
{
    int32_t freqInHz;
    int32_t timeoutInSec;
} FindOnSpecificFrequencyReqP2pIfaceParam;

typedef struct
{
    P2pFrameTypeMask frameTypeMask;
    vector<uint8_t> vendorElemBytes;
} SetVendorElementsReqP2pIfaceParam;

typedef struct
{
    int32_t ipAddressGo;
    int32_t ipAddressMask;
    int32_t ipAddressStart;
    int32_t ipAddressEnd;
} ConfigureEapolIpAddressAllocationParamsReqP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> p2pDeviceAddress;
    vector<uint8_t> primaryDeviceType;
    string deviceName;
    WpsConfigMethods configMethods;
    int8_t deviceCapabilities;
    P2pGroupCapabilityMask groupCapabilities;
    vector<uint8_t> wfdDeviceInfo;
} OnDeviceFoundIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    WpsDevPasswordId passwordId;
} OnGoNegotiationRequestIndP2pIfaceParam;

typedef struct
{
    string groupIfname;
    bool isGroupOwner;
} OnGroupRemovedIndP2pIfaceParam;

typedef struct
{
    string groupIfname;
    bool isGroupOwner;
    vector<uint8_t> ssid;
    int32_t frequency;
    vector<uint8_t> psk;
    string passphrase;
    vector<uint8_t> goDeviceAddress;
    bool isPersistent;
} OnGroupStartedIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> goDeviceAddress;
    vector<uint8_t> bssid;
    int32_t persistentNetworkId;
    int32_t operatingFrequency;
} OnInvitationReceivedIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> bssid;
    P2pStatusCode status;
} OnInvitationResultIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> p2pDeviceAddress;
    bool isRequest;
    P2pProvDiscStatusCode status;
    WpsConfigMethods configMethods;
    string generatedPin;
} OnProvisionDiscoveryCompletedIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> p2pDeviceAddress;
    vector<uint8_t> primaryDeviceType;
    string deviceName;
    WpsConfigMethods configMethods;
    int8_t deviceCapabilities;
    P2pGroupCapabilityMask groupCapabilities;
    vector<uint8_t> wfdDeviceInfo;
    vector<uint8_t> wfdR2DeviceInfo;
} OnR2DeviceFoundIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    char16_t updateIndicator;
    vector<uint8_t> tlvs;
} OnServiceDiscoveryResponseIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> p2pDeviceAddress;
} OnStaAuthorizedIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> p2pDeviceAddress;
} OnStaDeauthorizedIndP2pIfaceParam;

typedef struct
{
    string groupIfname;
    int32_t frequency;
} OnGroupFrequencyChangedIndP2pIfaceParam;

typedef struct
{
    vector<uint8_t> srcAddress;
    vector<uint8_t> p2pDeviceAddress;
    vector<uint8_t> primaryDeviceType;
    string deviceName;
    WpsConfigMethods configMethods;
    int8_t deviceCapabilities;
    P2pGroupCapabilityMask groupCapabilities;
    vector<uint8_t> wfdDeviceInfo;
    vector<uint8_t> wfdR2DeviceInfo;
    vector<uint8_t> vendorElemBytes;
} OnDeviceFoundWithVendorElementsIndP2pIfaceParam;

bool SupplicantP2pIfaceSerializeAddBonjourServiceReq(const vector<uint8_t>& query, const vector<uint8_t>& response, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddBonjourServiceCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddGroupReq(bool persistent, int32_t persistentNetworkId, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddGroupCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddGroupWithConfigReq(const vector<uint8_t>& ssid, const string& pskPassphrase, bool persistent, int32_t freq, const vector<uint8_t>& peerAddress, bool joinExistingGroup, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddGroupWithConfigCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddNetworkReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddUpnpServiceReq(int32_t version, const string& serviceName, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeAddUpnpServiceCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelConnectReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelConnectCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelServiceDiscoveryReq(int64_t identifier, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelServiceDiscoveryCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelWpsReq(const string& groupIfName, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCancelWpsCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConfigureExtListenReq(int32_t periodInMillis, int32_t intervalInMillis, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConfigureExtListenCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConnectReq(const vector<uint8_t>& peerAddress, WpsProvisionMethod provisionMethod, const string& preSelectedPin, bool joinExistingGroup, bool persistent, int32_t goIntent, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConnectCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCreateNfcHandoverRequestMessageCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeCreateNfcHandoverSelectMessageCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeEnableWfdReq(bool enable, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeEnableWfdCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindReq(int32_t timeoutInSec, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFlushReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFlushCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFlushServicesReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFlushServicesCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetDeviceAddressReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetDeviceAddressCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetEdmgReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetEdmgCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetGroupCapabilityReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetGroupCapabilityCfm(const HalStatusParam& status, P2pGroupCapabilityMask result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetNetworkReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetNetworkCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetSsidReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetTypeReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeInviteReq(const string& groupIfName, const vector<uint8_t>& goDeviceAddress, const vector<uint8_t>& peerAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeInviteCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeListNetworksReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeListNetworksCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeProvisionDiscoveryReq(const vector<uint8_t>& peerAddress, WpsProvisionMethod provisionMethod, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeProvisionDiscoveryCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRegisterCallbackReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRegisterCallbackCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReinvokeReq(int32_t persistentNetworkId, const vector<uint8_t>& peerAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReinvokeCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRejectReq(const vector<uint8_t>& peerAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRejectCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveBonjourServiceReq(const vector<uint8_t>& query, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveBonjourServiceCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveGroupReq(const string& groupIfName, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveGroupCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveNetworkReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveNetworkCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveUpnpServiceReq(int32_t version, const string& serviceName, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveUpnpServiceCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReportNfcHandoverInitiationReq(const vector<uint8_t>& select, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReportNfcHandoverInitiationCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReportNfcHandoverResponseReq(const vector<uint8_t>& request, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeReportNfcHandoverResponseCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRequestServiceDiscoveryReq(const vector<uint8_t>& peerAddress, const vector<uint8_t>& query, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRequestServiceDiscoveryCfm(const HalStatusParam& status, int64_t result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSaveConfigReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSaveConfigCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetDisallowedFrequenciesReq(const vector<FreqRange>& ranges, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetDisallowedFrequenciesCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetEdmgReq(bool enable, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetEdmgCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetGroupIdleReq(const string& groupIfName, int32_t timeoutInSec, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetGroupIdleCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetListenChannelReq(int32_t channel, int32_t operatingClass, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetListenChannelCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetMacRandomizationReq(bool enable, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetMacRandomizationCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetMiracastModeReq(MiracastMode mode, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetMiracastModeCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetPowerSaveReq(const string& groupIfName, bool enable, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetPowerSaveCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetSsidPostfixReq(const vector<uint8_t>& postfix, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetSsidPostfixCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWfdDeviceInfoReq(const vector<uint8_t>& info, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWfdDeviceInfoCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoReq(const vector<uint8_t>& info, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWfdR2DeviceInfoCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveClientReq(const vector<uint8_t>& peerAddress, bool isLegacyClient, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeRemoveClientCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsConfigMethodsReq(WpsConfigMethods configMethods, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsConfigMethodsCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsDeviceNameReq(const string& name, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsDeviceNameCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsDeviceTypeReq(const vector<uint8_t>& type, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsDeviceTypeCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsManufacturerReq(const string& manufacturer, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsManufacturerCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsModelNameReq(const string& modelName, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsModelNameCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsModelNumberReq(const string& modelNumber, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsModelNumberCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsSerialNumberReq(const string& serialNumber, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetWpsSerialNumberCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPbcReq(const string& groupIfName, const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPbcCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPinDisplayReq(const string& groupIfName, const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPinDisplayCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPinKeypadReq(const string& groupIfName, const string& pin, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStartWpsPinKeypadCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStopFindReq(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeStopFindCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindOnSocialChannelsReq(int32_t timeoutInSec, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindOnSocialChannelsCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindOnSpecificFrequencyReq(int32_t freqInHz, int32_t timeoutInSec, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeFindOnSpecificFrequencyCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetVendorElementsReq(P2pFrameTypeMask frameTypeMask, const vector<uint8_t>& vendorElemBytes, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeSetVendorElementsCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsReq(int32_t ipAddressGo, int32_t ipAddressMask, int32_t ipAddressStart, int32_t ipAddressEnd, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeConfigureEapolIpAddressAllocationParamsCfm(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnDeviceFoundInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnDeviceLostInd(const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnFindStoppedInd(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGoNegotiationCompletedInd(P2pStatusCode status, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGoNegotiationRequestInd(const vector<uint8_t>& srcAddress, WpsDevPasswordId passwordId, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupFormationFailureInd(const string& failureReason, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupFormationSuccessInd(vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupRemovedInd(const string& groupIfname, bool isGroupOwner, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupStartedInd(const string& groupIfname, bool isGroupOwner, const vector<uint8_t>& ssid, int32_t frequency, const vector<uint8_t>& psk, const string& passphrase, const vector<uint8_t>& goDeviceAddress, bool isPersistent, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnInvitationReceivedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& goDeviceAddress, const vector<uint8_t>& bssid, int32_t persistentNetworkId, int32_t operatingFrequency, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnInvitationResultInd(const vector<uint8_t>& bssid, P2pStatusCode status, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnProvisionDiscoveryCompletedInd(const vector<uint8_t>& p2pDeviceAddress, bool isRequest, P2pProvDiscStatusCode status, WpsConfigMethods configMethods, const string& generatedPin, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnR2DeviceFoundInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, const vector<uint8_t>& wfdR2DeviceInfo, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnServiceDiscoveryResponseInd(const vector<uint8_t>& srcAddress, char16_t updateIndicator, const vector<uint8_t>& tlvs, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnStaAuthorizedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnStaDeauthorizedInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupFrequencyChangedInd(const string& groupIfname, int32_t frequency, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnDeviceFoundWithVendorElementsInd(const vector<uint8_t>& srcAddress, const vector<uint8_t>& p2pDeviceAddress, const vector<uint8_t>& primaryDeviceType, const string& deviceName, WpsConfigMethods configMethods, int8_t deviceCapabilities, P2pGroupCapabilityMask groupCapabilities, const vector<uint8_t>& wfdDeviceInfo, const vector<uint8_t>& wfdR2DeviceInfo, const vector<uint8_t>& vendorElemBytes, vector<uint8_t>& payload);

bool SupplicantP2pIfaceSerializeOnGroupStartedWithParamsInd(const P2pGroupStartedEventParams& groupStartedEventParams, vector<uint8_t>& payload);

bool SupplicantP2pIfaceParseAddBonjourServiceReq(const uint8_t* data, size_t length, AddBonjourServiceReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseAddBonjourServiceCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseAddGroupReq(const uint8_t* data, size_t length, AddGroupReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseAddGroupCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseAddGroupWithConfigReq(const uint8_t* data, size_t length, AddGroupWithConfigReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseAddGroupWithConfigCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseAddNetworkReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseAddNetworkCfm(const uint8_t* data, size_t length, AddNetworkCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseAddUpnpServiceReq(const uint8_t* data, size_t length, AddUpnpServiceReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseAddUpnpServiceCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCancelConnectReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCancelConnectCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCancelServiceDiscoveryReq(const uint8_t* data, size_t length, int64_t& param);

bool SupplicantP2pIfaceParseCancelServiceDiscoveryCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCancelWpsReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseCancelWpsCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseConfigureExtListenReq(const uint8_t* data, size_t length, ConfigureExtListenReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseConfigureExtListenCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseConnectReq(const uint8_t* data, size_t length, ConnectReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseConnectCfm(const uint8_t* data, size_t length, ConnectCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCreateNfcHandoverRequestMessageCfm(const uint8_t* data, size_t length, CreateNfcHandoverRequestMessageCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseCreateNfcHandoverSelectMessageCfm(const uint8_t* data, size_t length, CreateNfcHandoverSelectMessageCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseEnableWfdReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantP2pIfaceParseEnableWfdCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFindReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantP2pIfaceParseFindCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFlushReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFlushCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFlushServicesReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFlushServicesCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseGetDeviceAddressReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseGetDeviceAddressCfm(const uint8_t* data, size_t length, GetDeviceAddressCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetEdmgReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseGetEdmgCfm(const uint8_t* data, size_t length, GetEdmgCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetGroupCapabilityReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseGetGroupCapabilityCfm(const uint8_t* data, size_t length, GetGroupCapabilityCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetNetworkReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantP2pIfaceParseGetNetworkCfm(const uint8_t* data, size_t length, GetNetworkCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetSsidReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseGetTypeReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseInviteReq(const uint8_t* data, size_t length, InviteReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseInviteCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseListNetworksReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseListNetworksCfm(const uint8_t* data, size_t length, ListNetworksCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseProvisionDiscoveryReq(const uint8_t* data, size_t length, ProvisionDiscoveryReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseProvisionDiscoveryCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRegisterCallbackReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRegisterCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseReinvokeReq(const uint8_t* data, size_t length, ReinvokeReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseReinvokeCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRejectReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseRejectCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRemoveBonjourServiceReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseRemoveBonjourServiceCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRemoveGroupReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseRemoveGroupCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRemoveNetworkReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantP2pIfaceParseRemoveNetworkCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRemoveUpnpServiceReq(const uint8_t* data, size_t length, RemoveUpnpServiceReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseRemoveUpnpServiceCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseReportNfcHandoverInitiationReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseReportNfcHandoverInitiationCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseReportNfcHandoverResponseReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseReportNfcHandoverResponseCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRequestServiceDiscoveryReq(const uint8_t* data, size_t length, RequestServiceDiscoveryReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseRequestServiceDiscoveryCfm(const uint8_t* data, size_t length, RequestServiceDiscoveryCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseSaveConfigReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSaveConfigCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetDisallowedFrequenciesReq(const uint8_t* data, size_t length, vector<FreqRange>& param);

bool SupplicantP2pIfaceParseSetDisallowedFrequenciesCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetEdmgReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantP2pIfaceParseSetEdmgCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetGroupIdleReq(const uint8_t* data, size_t length, SetGroupIdleReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseSetGroupIdleCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetListenChannelReq(const uint8_t* data, size_t length, SetListenChannelReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseSetListenChannelCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetMacRandomizationReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantP2pIfaceParseSetMacRandomizationCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetMiracastModeReq(const uint8_t* data, size_t length, MiracastMode& param);

bool SupplicantP2pIfaceParseSetMiracastModeCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetPowerSaveReq(const uint8_t* data, size_t length, SetPowerSaveReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseSetPowerSaveCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetSsidPostfixReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseSetSsidPostfixCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWfdDeviceInfoReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseSetWfdDeviceInfoCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWfdR2DeviceInfoReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseSetWfdR2DeviceInfoCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseRemoveClientReq(const uint8_t* data, size_t length, RemoveClientReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseRemoveClientCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsConfigMethodsReq(const uint8_t* data, size_t length, WpsConfigMethods& param);

bool SupplicantP2pIfaceParseSetWpsConfigMethodsCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsDeviceNameReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseSetWpsDeviceNameCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsDeviceTypeReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseSetWpsDeviceTypeCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsManufacturerReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseSetWpsManufacturerCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsModelNameReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseSetWpsModelNameCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsModelNumberReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseSetWpsModelNumberCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetWpsSerialNumberReq(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseSetWpsSerialNumberCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseStartWpsPbcReq(const uint8_t* data, size_t length, StartWpsPbcReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseStartWpsPbcCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseStartWpsPinDisplayReq(const uint8_t* data, size_t length, StartWpsPinDisplayReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseStartWpsPinDisplayCfm(const uint8_t* data, size_t length, StartWpsPinDisplayCfmP2pIfaceParam& param);

bool SupplicantP2pIfaceParseStartWpsPinKeypadReq(const uint8_t* data, size_t length, StartWpsPinKeypadReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseStartWpsPinKeypadCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseStopFindReq(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseStopFindCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFindOnSocialChannelsReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantP2pIfaceParseFindOnSocialChannelsCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseFindOnSpecificFrequencyReq(const uint8_t* data, size_t length, FindOnSpecificFrequencyReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseFindOnSpecificFrequencyCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseSetVendorElementsReq(const uint8_t* data, size_t length, SetVendorElementsReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseSetVendorElementsCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsReq(const uint8_t* data, size_t length, ConfigureEapolIpAddressAllocationParamsReqP2pIfaceParam& param);

bool SupplicantP2pIfaceParseConfigureEapolIpAddressAllocationParamsCfm(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseOnDeviceFoundInd(const uint8_t* data, size_t length, OnDeviceFoundIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnDeviceLostInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantP2pIfaceParseOnFindStoppedInd(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseOnGoNegotiationCompletedInd(const uint8_t* data, size_t length, P2pStatusCode& param);

bool SupplicantP2pIfaceParseOnGoNegotiationRequestInd(const uint8_t* data, size_t length, OnGoNegotiationRequestIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnGroupFormationFailureInd(const uint8_t* data, size_t length, string& param);

bool SupplicantP2pIfaceParseOnGroupFormationSuccessInd(const uint8_t* data, size_t length);

bool SupplicantP2pIfaceParseOnGroupRemovedInd(const uint8_t* data, size_t length, OnGroupRemovedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnGroupStartedInd(const uint8_t* data, size_t length, OnGroupStartedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnInvitationReceivedInd(const uint8_t* data, size_t length, OnInvitationReceivedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnInvitationResultInd(const uint8_t* data, size_t length, OnInvitationResultIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnProvisionDiscoveryCompletedInd(const uint8_t* data, size_t length, OnProvisionDiscoveryCompletedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnR2DeviceFoundInd(const uint8_t* data, size_t length, OnR2DeviceFoundIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnServiceDiscoveryResponseInd(const uint8_t* data, size_t length, OnServiceDiscoveryResponseIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnStaAuthorizedInd(const uint8_t* data, size_t length, OnStaAuthorizedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnStaDeauthorizedInd(const uint8_t* data, size_t length, OnStaDeauthorizedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnGroupFrequencyChangedInd(const uint8_t* data, size_t length, OnGroupFrequencyChangedIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnDeviceFoundWithVendorElementsInd(const uint8_t* data, size_t length, OnDeviceFoundWithVendorElementsIndP2pIfaceParam& param);

bool SupplicantP2pIfaceParseOnGroupStartedWithParamsInd(const uint8_t* data, size_t length, P2pGroupStartedEventParams& param);
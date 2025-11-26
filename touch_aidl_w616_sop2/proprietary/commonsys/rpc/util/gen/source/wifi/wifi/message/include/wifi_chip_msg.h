/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiChip.h>
#include <aidl/android/hardware/wifi/IWifiChipEventCallback.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateApIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateBridgedApIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateNanIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateP2pIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateRttControllerCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} CreateStaIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetApIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<string> result;
} GetApIfaceNamesCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<IWifiChip::ChipMode> result;
} GetAvailableModesCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetFeatureSetCfmChipParam;

typedef struct
{
    HalStatusParam status;
    WifiDebugHostWakeReasonStats result;
} GetDebugHostWakeReasonStatsCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<WifiDebugRingBufferStatus> result;
} GetDebugRingBuffersStatusCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetIdCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetModeCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetNanIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<string> result;
} GetNanIfaceNamesCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetP2pIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<string> result;
} GetP2pIfaceNamesCfmChipParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetStaIfaceCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<string> result;
} GetStaIfaceNamesCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<WifiRadioCombination> result;
} GetSupportedRadioCombinationsCfmChipParam;

typedef struct
{
    HalStatusParam status;
    WifiChipCapabilities result;
} GetWifiChipCapabilitiesCfmChipParam;

typedef struct
{
    WifiBand band;
    int32_t ifaceModeMask;
    int32_t filterMask;
} GetUsableChannelsReqChipParam;

typedef struct
{
    HalStatusParam status;
    vector<WifiUsableChannel> result;
} GetUsableChannelsCfmChipParam;

typedef struct
{
    string brIfaceName;
    string ifaceInstanceName;
} RemoveIfaceInstanceFromBridgedApIfaceReqChipParam;

typedef struct
{
    HalStatusParam status;
    IWifiChip::ChipDebugInfo result;
} RequestChipDebugInfoCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} RequestDriverDebugDumpCfmChipParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} RequestFirmwareDebugDumpCfmChipParam;

typedef struct
{
    vector<IWifiChip::CoexUnsafeChannel> unsafeChannels;
    int32_t restrictions;
} SetCoexUnsafeChannelsReqChipParam;

typedef struct
{
    string ringName;
    WifiDebugRingBufferVerboseLevel verboseLevel;
    int32_t maxIntervalInSec;
    int32_t minDataSizeInBytes;
} StartLoggingToDebugRingBufferReqChipParam;

typedef struct
{
    int32_t errorCode;
    vector<uint8_t> debugData;
} OnDebugErrorAlertIndChipParam;

typedef struct
{
    WifiDebugRingBufferStatus status;
    vector<uint8_t> data;
} OnDebugRingBufferDataAvailableIndChipParam;

typedef struct
{
    IfaceType type;
    string name;
} OnIfaceAddedIndChipParam;

typedef struct
{
    IfaceType type;
    string name;
} OnIfaceRemovedIndChipParam;

bool WifiChipSerializeConfigureChipReq(int32_t modeId, vector<uint8_t>& payload);

bool WifiChipSerializeConfigureChipCfm(vector<uint8_t>& payload);

bool WifiChipSerializeCreateApIfaceReq(vector<uint8_t>& payload);

bool WifiChipSerializeCreateApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeCreateBridgedApIfaceReq(vector<uint8_t>& payload);

bool WifiChipSerializeCreateBridgedApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeCreateNanIfaceReq(vector<uint8_t>& payload);

bool WifiChipSerializeCreateNanIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeCreateP2pIfaceReq(vector<uint8_t>& payload);

bool WifiChipSerializeCreateP2pIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeCreateRttControllerReq(int32_t boundIface, vector<uint8_t>& payload);

bool WifiChipSerializeCreateRttControllerCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeCreateStaIfaceReq(vector<uint8_t>& payload);

bool WifiChipSerializeCreateStaIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeEnableDebugErrorAlertsReq(bool enable, vector<uint8_t>& payload);

bool WifiChipSerializeEnableDebugErrorAlertsCfm(vector<uint8_t>& payload);

bool WifiChipSerializeFlushRingBufferToFileReq(vector<uint8_t>& payload);

bool WifiChipSerializeFlushRingBufferToFileCfm(vector<uint8_t>& payload);

bool WifiChipSerializeForceDumpToDebugRingBufferReq(const string& ringName, vector<uint8_t>& payload);

bool WifiChipSerializeForceDumpToDebugRingBufferCfm(vector<uint8_t>& payload);

bool WifiChipSerializeGetApIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeGetApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetApIfaceNamesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetApIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetAvailableModesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetAvailableModesCfm(const HalStatusParam& status, const vector<IWifiChip::ChipMode>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetFeatureSetReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetFeatureSetCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetDebugHostWakeReasonStatsReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetDebugHostWakeReasonStatsCfm(const HalStatusParam& status, const WifiDebugHostWakeReasonStats& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetDebugRingBuffersStatusReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetDebugRingBuffersStatusCfm(const HalStatusParam& status, const vector<WifiDebugRingBufferStatus>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetIdReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetModeReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetModeCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetNanIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeGetNanIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetNanIfaceNamesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetNanIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetP2pIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeGetP2pIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetP2pIfaceNamesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetP2pIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetStaIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeGetStaIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiChipSerializeGetStaIfaceNamesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetStaIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetSupportedRadioCombinationsReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetSupportedRadioCombinationsCfm(const HalStatusParam& status, const vector<WifiRadioCombination>& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetWifiChipCapabilitiesReq(vector<uint8_t>& payload);

bool WifiChipSerializeGetWifiChipCapabilitiesCfm(const HalStatusParam& status, const WifiChipCapabilities& result, vector<uint8_t>& payload);

bool WifiChipSerializeGetUsableChannelsReq(WifiBand band, int32_t ifaceModeMask, int32_t filterMask, vector<uint8_t>& payload);

bool WifiChipSerializeGetUsableChannelsCfm(const HalStatusParam& status, const vector<WifiUsableChannel>& result, vector<uint8_t>& payload);

bool WifiChipSerializeSetAfcChannelAllowanceReq(const AfcChannelAllowance& afcChannelAllowance, vector<uint8_t>& payload);

bool WifiChipSerializeSetAfcChannelAllowanceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRegisterEventCallbackReq(vector<uint8_t>& payload);

bool WifiChipSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRemoveApIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeRemoveApIfaceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceReq(const string& brIfaceName, const string& ifaceInstanceName, vector<uint8_t>& payload);

bool WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRemoveNanIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeRemoveNanIfaceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRemoveP2pIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeRemoveP2pIfaceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRemoveStaIfaceReq(const string& ifname, vector<uint8_t>& payload);

bool WifiChipSerializeRemoveStaIfaceCfm(vector<uint8_t>& payload);

bool WifiChipSerializeRequestChipDebugInfoReq(vector<uint8_t>& payload);

bool WifiChipSerializeRequestChipDebugInfoCfm(const HalStatusParam& status, const IWifiChip::ChipDebugInfo& result, vector<uint8_t>& payload);

bool WifiChipSerializeRequestDriverDebugDumpReq(vector<uint8_t>& payload);

bool WifiChipSerializeRequestDriverDebugDumpCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool WifiChipSerializeRequestFirmwareDebugDumpReq(vector<uint8_t>& payload);

bool WifiChipSerializeRequestFirmwareDebugDumpCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool WifiChipSerializeResetTxPowerScenarioReq(vector<uint8_t>& payload);

bool WifiChipSerializeResetTxPowerScenarioCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSelectTxPowerScenarioReq(IWifiChip::TxPowerScenario scenario, vector<uint8_t>& payload);

bool WifiChipSerializeSelectTxPowerScenarioCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetCoexUnsafeChannelsReq(const vector<IWifiChip::CoexUnsafeChannel>& unsafeChannels, int32_t restrictions, vector<uint8_t>& payload);

bool WifiChipSerializeSetCoexUnsafeChannelsCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetCountryCodeReq(const array<uint8_t, 2>& code, vector<uint8_t>& payload);

bool WifiChipSerializeSetCountryCodeCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetLatencyModeReq(IWifiChip::LatencyMode mode, vector<uint8_t>& payload);

bool WifiChipSerializeSetLatencyModeCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetMultiStaPrimaryConnectionReq(const string& ifName, vector<uint8_t>& payload);

bool WifiChipSerializeSetMultiStaPrimaryConnectionCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetMultiStaUseCaseReq(IWifiChip::MultiStaUseCase useCase, vector<uint8_t>& payload);

bool WifiChipSerializeSetMultiStaUseCaseCfm(vector<uint8_t>& payload);

bool WifiChipSerializeStartLoggingToDebugRingBufferReq(const string& ringName, WifiDebugRingBufferVerboseLevel verboseLevel, int32_t maxIntervalInSec, int32_t minDataSizeInBytes, vector<uint8_t>& payload);

bool WifiChipSerializeStartLoggingToDebugRingBufferCfm(vector<uint8_t>& payload);

bool WifiChipSerializeStopLoggingToDebugRingBufferReq(vector<uint8_t>& payload);

bool WifiChipSerializeStopLoggingToDebugRingBufferCfm(vector<uint8_t>& payload);

bool WifiChipSerializeTriggerSubsystemRestartReq(vector<uint8_t>& payload);

bool WifiChipSerializeTriggerSubsystemRestartCfm(vector<uint8_t>& payload);

bool WifiChipSerializeEnableStaChannelForPeerNetworkReq(int32_t channelCategoryEnableFlag, vector<uint8_t>& payload);

bool WifiChipSerializeEnableStaChannelForPeerNetworkCfm(vector<uint8_t>& payload);

bool WifiChipSerializeSetMloModeReq(IWifiChip::ChipMloMode mode, vector<uint8_t>& payload);

bool WifiChipSerializeSetMloModeCfm(vector<uint8_t>& payload);

bool WifiChipSerializeOnChipReconfigureFailureInd(WifiStatusCode status, vector<uint8_t>& payload);

bool WifiChipSerializeOnChipReconfiguredInd(int32_t modeId, vector<uint8_t>& payload);

bool WifiChipSerializeOnDebugErrorAlertInd(int32_t errorCode, const vector<uint8_t>& debugData, vector<uint8_t>& payload);

bool WifiChipSerializeOnDebugRingBufferDataAvailableInd(const WifiDebugRingBufferStatus& status, const vector<uint8_t>& data, vector<uint8_t>& payload);

bool WifiChipSerializeOnIfaceAddedInd(IfaceType type, const string& name, vector<uint8_t>& payload);

bool WifiChipSerializeOnIfaceRemovedInd(IfaceType type, const string& name, vector<uint8_t>& payload);

bool WifiChipSerializeOnRadioModeChangeInd(const vector<IWifiChipEventCallback::RadioModeInfo>& radioModeInfos, vector<uint8_t>& payload);

bool WifiChipParseConfigureChipReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiChipParseConfigureChipCfm(const uint8_t* data, size_t length);

bool WifiChipParseCreateApIfaceReq(const uint8_t* data, size_t length);

bool WifiChipParseCreateApIfaceCfm(const uint8_t* data, size_t length, CreateApIfaceCfmChipParam& param);

bool WifiChipParseCreateBridgedApIfaceReq(const uint8_t* data, size_t length);

bool WifiChipParseCreateBridgedApIfaceCfm(const uint8_t* data, size_t length, CreateBridgedApIfaceCfmChipParam& param);

bool WifiChipParseCreateNanIfaceReq(const uint8_t* data, size_t length);

bool WifiChipParseCreateNanIfaceCfm(const uint8_t* data, size_t length, CreateNanIfaceCfmChipParam& param);

bool WifiChipParseCreateP2pIfaceReq(const uint8_t* data, size_t length);

bool WifiChipParseCreateP2pIfaceCfm(const uint8_t* data, size_t length, CreateP2pIfaceCfmChipParam& param);

bool WifiChipParseCreateRttControllerReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiChipParseCreateRttControllerCfm(const uint8_t* data, size_t length, CreateRttControllerCfmChipParam& param);

bool WifiChipParseCreateStaIfaceReq(const uint8_t* data, size_t length);

bool WifiChipParseCreateStaIfaceCfm(const uint8_t* data, size_t length, CreateStaIfaceCfmChipParam& param);

bool WifiChipParseEnableDebugErrorAlertsReq(const uint8_t* data, size_t length, bool& param);

bool WifiChipParseEnableDebugErrorAlertsCfm(const uint8_t* data, size_t length);

bool WifiChipParseFlushRingBufferToFileReq(const uint8_t* data, size_t length);

bool WifiChipParseFlushRingBufferToFileCfm(const uint8_t* data, size_t length);

bool WifiChipParseForceDumpToDebugRingBufferReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseForceDumpToDebugRingBufferCfm(const uint8_t* data, size_t length);

bool WifiChipParseGetApIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseGetApIfaceCfm(const uint8_t* data, size_t length, GetApIfaceCfmChipParam& param);

bool WifiChipParseGetApIfaceNamesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetApIfaceNamesCfm(const uint8_t* data, size_t length, GetApIfaceNamesCfmChipParam& param);

bool WifiChipParseGetAvailableModesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetAvailableModesCfm(const uint8_t* data, size_t length, GetAvailableModesCfmChipParam& param);

bool WifiChipParseGetFeatureSetReq(const uint8_t* data, size_t length);

bool WifiChipParseGetFeatureSetCfm(const uint8_t* data, size_t length, GetFeatureSetCfmChipParam& param);

bool WifiChipParseGetDebugHostWakeReasonStatsReq(const uint8_t* data, size_t length);

bool WifiChipParseGetDebugHostWakeReasonStatsCfm(const uint8_t* data, size_t length, GetDebugHostWakeReasonStatsCfmChipParam& param);

bool WifiChipParseGetDebugRingBuffersStatusReq(const uint8_t* data, size_t length);

bool WifiChipParseGetDebugRingBuffersStatusCfm(const uint8_t* data, size_t length, GetDebugRingBuffersStatusCfmChipParam& param);

bool WifiChipParseGetIdReq(const uint8_t* data, size_t length);

bool WifiChipParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmChipParam& param);

bool WifiChipParseGetModeReq(const uint8_t* data, size_t length);

bool WifiChipParseGetModeCfm(const uint8_t* data, size_t length, GetModeCfmChipParam& param);

bool WifiChipParseGetNanIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseGetNanIfaceCfm(const uint8_t* data, size_t length, GetNanIfaceCfmChipParam& param);

bool WifiChipParseGetNanIfaceNamesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetNanIfaceNamesCfm(const uint8_t* data, size_t length, GetNanIfaceNamesCfmChipParam& param);

bool WifiChipParseGetP2pIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseGetP2pIfaceCfm(const uint8_t* data, size_t length, GetP2pIfaceCfmChipParam& param);

bool WifiChipParseGetP2pIfaceNamesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetP2pIfaceNamesCfm(const uint8_t* data, size_t length, GetP2pIfaceNamesCfmChipParam& param);

bool WifiChipParseGetStaIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseGetStaIfaceCfm(const uint8_t* data, size_t length, GetStaIfaceCfmChipParam& param);

bool WifiChipParseGetStaIfaceNamesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetStaIfaceNamesCfm(const uint8_t* data, size_t length, GetStaIfaceNamesCfmChipParam& param);

bool WifiChipParseGetSupportedRadioCombinationsReq(const uint8_t* data, size_t length);

bool WifiChipParseGetSupportedRadioCombinationsCfm(const uint8_t* data, size_t length, GetSupportedRadioCombinationsCfmChipParam& param);

bool WifiChipParseGetWifiChipCapabilitiesReq(const uint8_t* data, size_t length);

bool WifiChipParseGetWifiChipCapabilitiesCfm(const uint8_t* data, size_t length, GetWifiChipCapabilitiesCfmChipParam& param);

bool WifiChipParseGetUsableChannelsReq(const uint8_t* data, size_t length, GetUsableChannelsReqChipParam& param);

bool WifiChipParseGetUsableChannelsCfm(const uint8_t* data, size_t length, GetUsableChannelsCfmChipParam& param);

bool WifiChipParseSetAfcChannelAllowanceReq(const uint8_t* data, size_t length, AfcChannelAllowance& param);

bool WifiChipParseSetAfcChannelAllowanceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRegisterEventCallbackReq(const uint8_t* data, size_t length);

bool WifiChipParseRegisterEventCallbackCfm(const uint8_t* data, size_t length);

bool WifiChipParseRemoveApIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseRemoveApIfaceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceReq(const uint8_t* data, size_t length, RemoveIfaceInstanceFromBridgedApIfaceReqChipParam& param);

bool WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRemoveNanIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseRemoveNanIfaceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRemoveP2pIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseRemoveP2pIfaceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRemoveStaIfaceReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseRemoveStaIfaceCfm(const uint8_t* data, size_t length);

bool WifiChipParseRequestChipDebugInfoReq(const uint8_t* data, size_t length);

bool WifiChipParseRequestChipDebugInfoCfm(const uint8_t* data, size_t length, RequestChipDebugInfoCfmChipParam& param);

bool WifiChipParseRequestDriverDebugDumpReq(const uint8_t* data, size_t length);

bool WifiChipParseRequestDriverDebugDumpCfm(const uint8_t* data, size_t length, RequestDriverDebugDumpCfmChipParam& param);

bool WifiChipParseRequestFirmwareDebugDumpReq(const uint8_t* data, size_t length);

bool WifiChipParseRequestFirmwareDebugDumpCfm(const uint8_t* data, size_t length, RequestFirmwareDebugDumpCfmChipParam& param);

bool WifiChipParseResetTxPowerScenarioReq(const uint8_t* data, size_t length);

bool WifiChipParseResetTxPowerScenarioCfm(const uint8_t* data, size_t length);

bool WifiChipParseSelectTxPowerScenarioReq(const uint8_t* data, size_t length, IWifiChip::TxPowerScenario& param);

bool WifiChipParseSelectTxPowerScenarioCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetCoexUnsafeChannelsReq(const uint8_t* data, size_t length, SetCoexUnsafeChannelsReqChipParam& param);

bool WifiChipParseSetCoexUnsafeChannelsCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetCountryCodeReq(const uint8_t* data, size_t length, array<uint8_t, 2>& param);

bool WifiChipParseSetCountryCodeCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetLatencyModeReq(const uint8_t* data, size_t length, IWifiChip::LatencyMode& param);

bool WifiChipParseSetLatencyModeCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetMultiStaPrimaryConnectionReq(const uint8_t* data, size_t length, string& param);

bool WifiChipParseSetMultiStaPrimaryConnectionCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetMultiStaUseCaseReq(const uint8_t* data, size_t length, IWifiChip::MultiStaUseCase& param);

bool WifiChipParseSetMultiStaUseCaseCfm(const uint8_t* data, size_t length);

bool WifiChipParseStartLoggingToDebugRingBufferReq(const uint8_t* data, size_t length, StartLoggingToDebugRingBufferReqChipParam& param);

bool WifiChipParseStartLoggingToDebugRingBufferCfm(const uint8_t* data, size_t length);

bool WifiChipParseStopLoggingToDebugRingBufferReq(const uint8_t* data, size_t length);

bool WifiChipParseStopLoggingToDebugRingBufferCfm(const uint8_t* data, size_t length);

bool WifiChipParseTriggerSubsystemRestartReq(const uint8_t* data, size_t length);

bool WifiChipParseTriggerSubsystemRestartCfm(const uint8_t* data, size_t length);

bool WifiChipParseEnableStaChannelForPeerNetworkReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiChipParseEnableStaChannelForPeerNetworkCfm(const uint8_t* data, size_t length);

bool WifiChipParseSetMloModeReq(const uint8_t* data, size_t length, IWifiChip::ChipMloMode& param);

bool WifiChipParseSetMloModeCfm(const uint8_t* data, size_t length);

bool WifiChipParseOnChipReconfigureFailureInd(const uint8_t* data, size_t length, WifiStatusCode& param);

bool WifiChipParseOnChipReconfiguredInd(const uint8_t* data, size_t length, int32_t& param);

bool WifiChipParseOnDebugErrorAlertInd(const uint8_t* data, size_t length, OnDebugErrorAlertIndChipParam& param);

bool WifiChipParseOnDebugRingBufferDataAvailableInd(const uint8_t* data, size_t length, OnDebugRingBufferDataAvailableIndChipParam& param);

bool WifiChipParseOnIfaceAddedInd(const uint8_t* data, size_t length, OnIfaceAddedIndChipParam& param);

bool WifiChipParseOnIfaceRemovedInd(const uint8_t* data, size_t length, OnIfaceRemovedIndChipParam& param);

bool WifiChipParseOnRadioModeChangeInd(const uint8_t* data, size_t length, vector<IWifiChipEventCallback::RadioModeInfo>& param);
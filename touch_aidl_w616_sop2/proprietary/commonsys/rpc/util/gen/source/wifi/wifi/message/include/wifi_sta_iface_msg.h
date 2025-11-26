/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiStaIface.h>
#include <aidl/android/hardware/wifi/IWifiStaIfaceEventCallback.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    StaApfPacketFilterCapabilities result;
} GetApfPacketFilterCapabilitiesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    StaBackgroundScanCapabilities result;
} GetBackgroundScanCapabilitiesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetFeatureSetCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<WifiDebugRxPacketFateReport> result;
} GetDebugRxPacketFatesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<WifiDebugTxPacketFateReport> result;
} GetDebugTxPacketFatesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    array<uint8_t, 6> result;
} GetFactoryMacAddressCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    StaLinkLayerStats result;
} GetLinkLayerStatsCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    StaRoamingCapabilities result;
} GetRoamingCapabilitiesCfmStaIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} ReadApfPacketFilterDataCfmStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    StaBackgroundScanParameters params;
} StartBackgroundScanReqStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    int32_t maxRssi;
    int32_t minRssi;
} StartRssiMonitoringReqStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    vector<uint8_t> ipPacketData;
    int32_t etherType;
    array<uint8_t, 6> srcAddress;
    array<uint8_t, 6> dstAddress;
    int32_t periodInMs;
} StartSendingKeepAlivePacketsReqStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    int32_t bucketsScanned;
    StaScanResult result;
} OnBackgroundFullScanResultIndStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    vector<StaScanData> scanDatas;
} OnBackgroundScanResultsIndStaIfaceParam;

typedef struct
{
    int32_t cmdId;
    array<uint8_t, 6> currBssid;
    int32_t currRssi;
} OnRssiThresholdBreachedIndStaIfaceParam;

bool WifiStaIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeConfigureRoamingReq(const StaRoamingConfig& config, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeConfigureRoamingCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeDisableLinkLayerStatsCollectionReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeDisableLinkLayerStatsCollectionCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeEnableLinkLayerStatsCollectionReq(bool debug, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeEnableLinkLayerStatsCollectionCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeEnableNdOffloadReq(bool enable, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeEnableNdOffloadCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesCfm(const HalStatusParam& status, const StaApfPacketFilterCapabilities& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetBackgroundScanCapabilitiesReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetBackgroundScanCapabilitiesCfm(const HalStatusParam& status, const StaBackgroundScanCapabilities& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetFeatureSetReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetFeatureSetCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetDebugRxPacketFatesReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetDebugRxPacketFatesCfm(const HalStatusParam& status, const vector<WifiDebugRxPacketFateReport>& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetDebugTxPacketFatesReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetDebugTxPacketFatesCfm(const HalStatusParam& status, const vector<WifiDebugTxPacketFateReport>& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetFactoryMacAddressReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetFactoryMacAddressCfm(const HalStatusParam& status, const array<uint8_t, 6>& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetLinkLayerStatsReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetLinkLayerStatsCfm(const HalStatusParam& status, const StaLinkLayerStats& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetRoamingCapabilitiesReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeGetRoamingCapabilitiesCfm(const HalStatusParam& status, const StaRoamingCapabilities& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeInstallApfPacketFilterReq(const vector<uint8_t>& program, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeInstallApfPacketFilterCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeReadApfPacketFilterDataReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeReadApfPacketFilterDataCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeRegisterEventCallbackReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetMacAddressReq(const array<uint8_t, 6>& mac, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetMacAddressCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetRoamingStateReq(StaRoamingState state, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetRoamingStateCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetScanModeReq(bool enable, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetScanModeCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartBackgroundScanReq(int32_t cmdId, const StaBackgroundScanParameters& params, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartBackgroundScanCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartDebugPacketFateMonitoringReq(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartDebugPacketFateMonitoringCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartRssiMonitoringReq(int32_t cmdId, int32_t maxRssi, int32_t minRssi, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartRssiMonitoringCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartSendingKeepAlivePacketsReq(int32_t cmdId, const vector<uint8_t>& ipPacketData, int32_t etherType, const array<uint8_t, 6>& srcAddress, const array<uint8_t, 6>& dstAddress, int32_t periodInMs, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStartSendingKeepAlivePacketsCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopBackgroundScanReq(int32_t cmdId, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopBackgroundScanCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopRssiMonitoringReq(int32_t cmdId, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopRssiMonitoringCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopSendingKeepAlivePacketsReq(int32_t cmdId, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeStopSendingKeepAlivePacketsCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetDtimMultiplierReq(int32_t multiplier, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeSetDtimMultiplierCfm(vector<uint8_t>& payload);

bool WifiStaIfaceSerializeOnBackgroundFullScanResultInd(int32_t cmdId, int32_t bucketsScanned, const StaScanResult& result, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeOnBackgroundScanFailureInd(int32_t cmdId, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeOnBackgroundScanResultsInd(int32_t cmdId, const vector<StaScanData>& scanDatas, vector<uint8_t>& payload);

bool WifiStaIfaceSerializeOnRssiThresholdBreachedInd(int32_t cmdId, const array<uint8_t, 6>& currBssid, int32_t currRssi, vector<uint8_t>& payload);

bool WifiStaIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmStaIfaceParam& param);

bool WifiStaIfaceParseConfigureRoamingReq(const uint8_t* data, size_t length, StaRoamingConfig& param);

bool WifiStaIfaceParseConfigureRoamingCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseDisableLinkLayerStatsCollectionReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseDisableLinkLayerStatsCollectionCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseEnableLinkLayerStatsCollectionReq(const uint8_t* data, size_t length, bool& param);

bool WifiStaIfaceParseEnableLinkLayerStatsCollectionCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseEnableNdOffloadReq(const uint8_t* data, size_t length, bool& param);

bool WifiStaIfaceParseEnableNdOffloadCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetApfPacketFilterCapabilitiesReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetApfPacketFilterCapabilitiesCfm(const uint8_t* data, size_t length, GetApfPacketFilterCapabilitiesCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetBackgroundScanCapabilitiesReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetBackgroundScanCapabilitiesCfm(const uint8_t* data, size_t length, GetBackgroundScanCapabilitiesCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetFeatureSetReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetFeatureSetCfm(const uint8_t* data, size_t length, GetFeatureSetCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetDebugRxPacketFatesReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetDebugRxPacketFatesCfm(const uint8_t* data, size_t length, GetDebugRxPacketFatesCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetDebugTxPacketFatesReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetDebugTxPacketFatesCfm(const uint8_t* data, size_t length, GetDebugTxPacketFatesCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetFactoryMacAddressReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetFactoryMacAddressCfm(const uint8_t* data, size_t length, GetFactoryMacAddressCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetLinkLayerStatsReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetLinkLayerStatsCfm(const uint8_t* data, size_t length, GetLinkLayerStatsCfmStaIfaceParam& param);

bool WifiStaIfaceParseGetRoamingCapabilitiesReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseGetRoamingCapabilitiesCfm(const uint8_t* data, size_t length, GetRoamingCapabilitiesCfmStaIfaceParam& param);

bool WifiStaIfaceParseInstallApfPacketFilterReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool WifiStaIfaceParseInstallApfPacketFilterCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseReadApfPacketFilterDataReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseReadApfPacketFilterDataCfm(const uint8_t* data, size_t length, ReadApfPacketFilterDataCfmStaIfaceParam& param);

bool WifiStaIfaceParseRegisterEventCallbackReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseRegisterEventCallbackCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseSetMacAddressReq(const uint8_t* data, size_t length, array<uint8_t, 6>& param);

bool WifiStaIfaceParseSetMacAddressCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseSetRoamingStateReq(const uint8_t* data, size_t length, StaRoamingState& param);

bool WifiStaIfaceParseSetRoamingStateCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseSetScanModeReq(const uint8_t* data, size_t length, bool& param);

bool WifiStaIfaceParseSetScanModeCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStartBackgroundScanReq(const uint8_t* data, size_t length, StartBackgroundScanReqStaIfaceParam& param);

bool WifiStaIfaceParseStartBackgroundScanCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStartDebugPacketFateMonitoringReq(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStartDebugPacketFateMonitoringCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStartRssiMonitoringReq(const uint8_t* data, size_t length, StartRssiMonitoringReqStaIfaceParam& param);

bool WifiStaIfaceParseStartRssiMonitoringCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStartSendingKeepAlivePacketsReq(const uint8_t* data, size_t length, StartSendingKeepAlivePacketsReqStaIfaceParam& param);

bool WifiStaIfaceParseStartSendingKeepAlivePacketsCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStopBackgroundScanReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiStaIfaceParseStopBackgroundScanCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStopRssiMonitoringReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiStaIfaceParseStopRssiMonitoringCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseStopSendingKeepAlivePacketsReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiStaIfaceParseStopSendingKeepAlivePacketsCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseSetDtimMultiplierReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiStaIfaceParseSetDtimMultiplierCfm(const uint8_t* data, size_t length);

bool WifiStaIfaceParseOnBackgroundFullScanResultInd(const uint8_t* data, size_t length, OnBackgroundFullScanResultIndStaIfaceParam& param);

bool WifiStaIfaceParseOnBackgroundScanFailureInd(const uint8_t* data, size_t length, int32_t& param);

bool WifiStaIfaceParseOnBackgroundScanResultsInd(const uint8_t* data, size_t length, OnBackgroundScanResultsIndStaIfaceParam& param);

bool WifiStaIfaceParseOnRssiThresholdBreachedInd(const uint8_t* data, size_t length, OnRssiThresholdBreachedIndStaIfaceParam& param);
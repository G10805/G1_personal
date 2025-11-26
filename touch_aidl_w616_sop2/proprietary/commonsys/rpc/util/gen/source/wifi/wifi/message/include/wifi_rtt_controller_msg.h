/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiRttController.h>
#include <aidl/android/hardware/wifi/IWifiRttControllerEventCallback.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    int32_t cmdId;
    WifiChannelInfo channelHint;
    int32_t maxDurationInSeconds;
    RttResponder info;
} EnableResponderReqRttControllerParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetBoundIfaceCfmRttControllerParam;

typedef struct
{
    HalStatusParam status;
    RttCapabilities result;
} GetCapabilitiesCfmRttControllerParam;

typedef struct
{
    HalStatusParam status;
    RttResponder result;
} GetResponderInfoCfmRttControllerParam;

typedef struct
{
    int32_t cmdId;
    vector<MacAddress> addrs;
} RangeCancelReqRttControllerParam;

typedef struct
{
    int32_t cmdId;
    vector<RttConfig> rttConfigs;
} RangeRequestReqRttControllerParam;

typedef struct
{
    int32_t cmdId;
    RttLciInformation lci;
} SetLciReqRttControllerParam;

typedef struct
{
    int32_t cmdId;
    RttLcrInformation lcr;
} SetLcrReqRttControllerParam;

typedef struct
{
    int32_t cmdId;
    vector<RttResult> results;
} OnResultsIndRttControllerParam;

bool WifiRttControllerSerializeDisableResponderReq(int32_t cmdId, vector<uint8_t>& payload);

bool WifiRttControllerSerializeDisableResponderCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeEnableResponderReq(int32_t cmdId, const WifiChannelInfo& channelHint, int32_t maxDurationInSeconds, const RttResponder& info, vector<uint8_t>& payload);

bool WifiRttControllerSerializeEnableResponderCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetBoundIfaceReq(vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetBoundIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetCapabilitiesReq(vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetCapabilitiesCfm(const HalStatusParam& status, const RttCapabilities& result, vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetResponderInfoReq(vector<uint8_t>& payload);

bool WifiRttControllerSerializeGetResponderInfoCfm(const HalStatusParam& status, const RttResponder& result, vector<uint8_t>& payload);

bool WifiRttControllerSerializeRangeCancelReq(int32_t cmdId, const vector<MacAddress>& addrs, vector<uint8_t>& payload);

bool WifiRttControllerSerializeRangeCancelCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeRangeRequestReq(int32_t cmdId, const vector<RttConfig>& rttConfigs, vector<uint8_t>& payload);

bool WifiRttControllerSerializeRangeRequestCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeRegisterEventCallbackReq(vector<uint8_t>& payload);

bool WifiRttControllerSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeSetLciReq(int32_t cmdId, const RttLciInformation& lci, vector<uint8_t>& payload);

bool WifiRttControllerSerializeSetLciCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeSetLcrReq(int32_t cmdId, const RttLcrInformation& lcr, vector<uint8_t>& payload);

bool WifiRttControllerSerializeSetLcrCfm(vector<uint8_t>& payload);

bool WifiRttControllerSerializeOnResultsInd(int32_t cmdId, const vector<RttResult>& results, vector<uint8_t>& payload);

bool WifiRttControllerParseDisableResponderReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiRttControllerParseDisableResponderCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseEnableResponderReq(const uint8_t* data, size_t length, EnableResponderReqRttControllerParam& param);

bool WifiRttControllerParseEnableResponderCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseGetBoundIfaceReq(const uint8_t* data, size_t length);

bool WifiRttControllerParseGetBoundIfaceCfm(const uint8_t* data, size_t length, GetBoundIfaceCfmRttControllerParam& param);

bool WifiRttControllerParseGetCapabilitiesReq(const uint8_t* data, size_t length);

bool WifiRttControllerParseGetCapabilitiesCfm(const uint8_t* data, size_t length, GetCapabilitiesCfmRttControllerParam& param);

bool WifiRttControllerParseGetResponderInfoReq(const uint8_t* data, size_t length);

bool WifiRttControllerParseGetResponderInfoCfm(const uint8_t* data, size_t length, GetResponderInfoCfmRttControllerParam& param);

bool WifiRttControllerParseRangeCancelReq(const uint8_t* data, size_t length, RangeCancelReqRttControllerParam& param);

bool WifiRttControllerParseRangeCancelCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseRangeRequestReq(const uint8_t* data, size_t length, RangeRequestReqRttControllerParam& param);

bool WifiRttControllerParseRangeRequestCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseRegisterEventCallbackReq(const uint8_t* data, size_t length);

bool WifiRttControllerParseRegisterEventCallbackCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseSetLciReq(const uint8_t* data, size_t length, SetLciReqRttControllerParam& param);

bool WifiRttControllerParseSetLciCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseSetLcrReq(const uint8_t* data, size_t length, SetLcrReqRttControllerParam& param);

bool WifiRttControllerParseSetLcrCfm(const uint8_t* data, size_t length);

bool WifiRttControllerParseOnResultsInd(const uint8_t* data, size_t length, OnResultsIndRttControllerParam& param);
/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifi.h>
#include <aidl/android/hardware/wifi/IWifiEventCallback.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetChipCfmParam;

typedef struct
{
    HalStatusParam status;
    vector<int32_t> result;
} GetChipIdsCfmParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsStartedCfmParam;

bool WifiSerializeGetChipReq(int32_t chipId, vector<uint8_t>& payload);

bool WifiSerializeGetChipCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool WifiSerializeGetChipIdsReq(vector<uint8_t>& payload);

bool WifiSerializeGetChipIdsCfm(const HalStatusParam& status, const vector<int32_t>& result, vector<uint8_t>& payload);

bool WifiSerializeIsStartedReq(vector<uint8_t>& payload);

bool WifiSerializeIsStartedCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool WifiSerializeRegisterEventCallbackReq(vector<uint8_t>& payload);

bool WifiSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload);

bool WifiSerializeStartReq(vector<uint8_t>& payload);

bool WifiSerializeStartCfm(vector<uint8_t>& payload);

bool WifiSerializeStopReq(vector<uint8_t>& payload);

bool WifiSerializeStopCfm(vector<uint8_t>& payload);

bool WifiSerializeOnFailureInd(WifiStatusCode status, vector<uint8_t>& payload);

bool WifiSerializeOnStartInd(vector<uint8_t>& payload);

bool WifiSerializeOnStopInd(vector<uint8_t>& payload);

bool WifiSerializeOnSubsystemRestartInd(WifiStatusCode status, vector<uint8_t>& payload);

bool WifiParseGetChipReq(const uint8_t* data, size_t length, int32_t& param);

bool WifiParseGetChipCfm(const uint8_t* data, size_t length, GetChipCfmParam& param);

bool WifiParseGetChipIdsReq(const uint8_t* data, size_t length);

bool WifiParseGetChipIdsCfm(const uint8_t* data, size_t length, GetChipIdsCfmParam& param);

bool WifiParseIsStartedReq(const uint8_t* data, size_t length);

bool WifiParseIsStartedCfm(const uint8_t* data, size_t length, IsStartedCfmParam& param);

bool WifiParseRegisterEventCallbackReq(const uint8_t* data, size_t length);

bool WifiParseRegisterEventCallbackCfm(const uint8_t* data, size_t length);

bool WifiParseStartReq(const uint8_t* data, size_t length);

bool WifiParseStartCfm(const uint8_t* data, size_t length);

bool WifiParseStopReq(const uint8_t* data, size_t length);

bool WifiParseStopCfm(const uint8_t* data, size_t length);

bool WifiParseOnFailureInd(const uint8_t* data, size_t length, WifiStatusCode& param);

bool WifiParseOnStartInd(const uint8_t* data, size_t length);

bool WifiParseOnStopInd(const uint8_t* data, size_t length);

bool WifiParseOnSubsystemRestartInd(const uint8_t* data, size_t length, WifiStatusCode& param);
/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/ISupplicant.h>
#include <aidl/android/hardware/wifi/supplicant/ISupplicantCallback.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddP2pInterfaceCfmParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} AddStaInterfaceCfmParam;

typedef struct
{
    HalStatusParam status;
    DebugLevel result;
} GetDebugLevelCfmParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetP2pInterfaceCfmParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetStaInterfaceCfmParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsDebugShowKeysEnabledCfmParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsDebugShowTimestampEnabledCfmParam;

typedef struct
{
    HalStatusParam status;
    vector<IfaceInfo> result;
} ListInterfacesCfmParam;

typedef struct
{
    DebugLevel level;
    bool showTimestamp;
    bool showKeys;
} SetDebugParamsReqParam;

bool SupplicantSerializeAddP2pInterfaceReq(const string& ifName, vector<uint8_t>& payload);

bool SupplicantSerializeAddP2pInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantSerializeAddStaInterfaceReq(const string& ifName, vector<uint8_t>& payload);

bool SupplicantSerializeAddStaInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantSerializeGetDebugLevelReq(vector<uint8_t>& payload);

bool SupplicantSerializeGetDebugLevelCfm(const HalStatusParam& status, DebugLevel result, vector<uint8_t>& payload);

bool SupplicantSerializeGetP2pInterfaceReq(const string& ifName, vector<uint8_t>& payload);

bool SupplicantSerializeGetP2pInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantSerializeGetStaInterfaceReq(const string& ifName, vector<uint8_t>& payload);

bool SupplicantSerializeGetStaInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantSerializeIsDebugShowKeysEnabledReq(vector<uint8_t>& payload);

bool SupplicantSerializeIsDebugShowKeysEnabledCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantSerializeIsDebugShowTimestampEnabledReq(vector<uint8_t>& payload);

bool SupplicantSerializeIsDebugShowTimestampEnabledCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantSerializeListInterfacesReq(vector<uint8_t>& payload);

bool SupplicantSerializeListInterfacesCfm(const HalStatusParam& status, const vector<IfaceInfo>& result, vector<uint8_t>& payload);

bool SupplicantSerializeRegisterCallbackReq(vector<uint8_t>& payload);

bool SupplicantSerializeRegisterCallbackCfm(vector<uint8_t>& payload);

bool SupplicantSerializeRemoveInterfaceReq(const IfaceInfo& ifaceInfo, vector<uint8_t>& payload);

bool SupplicantSerializeRemoveInterfaceCfm(vector<uint8_t>& payload);

bool SupplicantSerializeSetConcurrencyPriorityReq(IfaceType type, vector<uint8_t>& payload);

bool SupplicantSerializeSetConcurrencyPriorityCfm(vector<uint8_t>& payload);

bool SupplicantSerializeSetDebugParamsReq(DebugLevel level, bool showTimestamp, bool showKeys, vector<uint8_t>& payload);

bool SupplicantSerializeSetDebugParamsCfm(vector<uint8_t>& payload);

bool SupplicantSerializeTerminateReq(vector<uint8_t>& payload);

bool SupplicantSerializeTerminateCfm(vector<uint8_t>& payload);

bool SupplicantSerializeRegisterNonStandardCertCallbackReq(vector<uint8_t>& payload);

bool SupplicantSerializeRegisterNonStandardCertCallbackCfm(vector<uint8_t>& payload);

bool SupplicantSerializeOnInterfaceCreatedInd(const string& ifaceName, vector<uint8_t>& payload);

bool SupplicantSerializeOnInterfaceRemovedInd(const string& ifaceName, vector<uint8_t>& payload);

bool SupplicantParseAddP2pInterfaceReq(const uint8_t* data, size_t length, string& param);

bool SupplicantParseAddP2pInterfaceCfm(const uint8_t* data, size_t length, AddP2pInterfaceCfmParam& param);

bool SupplicantParseAddStaInterfaceReq(const uint8_t* data, size_t length, string& param);

bool SupplicantParseAddStaInterfaceCfm(const uint8_t* data, size_t length, AddStaInterfaceCfmParam& param);

bool SupplicantParseGetDebugLevelReq(const uint8_t* data, size_t length);

bool SupplicantParseGetDebugLevelCfm(const uint8_t* data, size_t length, GetDebugLevelCfmParam& param);

bool SupplicantParseGetP2pInterfaceReq(const uint8_t* data, size_t length, string& param);

bool SupplicantParseGetP2pInterfaceCfm(const uint8_t* data, size_t length, GetP2pInterfaceCfmParam& param);

bool SupplicantParseGetStaInterfaceReq(const uint8_t* data, size_t length, string& param);

bool SupplicantParseGetStaInterfaceCfm(const uint8_t* data, size_t length, GetStaInterfaceCfmParam& param);

bool SupplicantParseIsDebugShowKeysEnabledReq(const uint8_t* data, size_t length);

bool SupplicantParseIsDebugShowKeysEnabledCfm(const uint8_t* data, size_t length, IsDebugShowKeysEnabledCfmParam& param);

bool SupplicantParseIsDebugShowTimestampEnabledReq(const uint8_t* data, size_t length);

bool SupplicantParseIsDebugShowTimestampEnabledCfm(const uint8_t* data, size_t length, IsDebugShowTimestampEnabledCfmParam& param);

bool SupplicantParseListInterfacesReq(const uint8_t* data, size_t length);

bool SupplicantParseListInterfacesCfm(const uint8_t* data, size_t length, ListInterfacesCfmParam& param);

bool SupplicantParseRegisterCallbackReq(const uint8_t* data, size_t length);

bool SupplicantParseRegisterCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantParseRemoveInterfaceReq(const uint8_t* data, size_t length, IfaceInfo& param);

bool SupplicantParseRemoveInterfaceCfm(const uint8_t* data, size_t length);

bool SupplicantParseSetConcurrencyPriorityReq(const uint8_t* data, size_t length, IfaceType& param);

bool SupplicantParseSetConcurrencyPriorityCfm(const uint8_t* data, size_t length);

bool SupplicantParseSetDebugParamsReq(const uint8_t* data, size_t length, SetDebugParamsReqParam& param);

bool SupplicantParseSetDebugParamsCfm(const uint8_t* data, size_t length);

bool SupplicantParseTerminateReq(const uint8_t* data, size_t length);

bool SupplicantParseTerminateCfm(const uint8_t* data, size_t length);

bool SupplicantParseRegisterNonStandardCertCallbackReq(const uint8_t* data, size_t length);

bool SupplicantParseRegisterNonStandardCertCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantParseOnInterfaceCreatedInd(const uint8_t* data, size_t length, string& param);

bool SupplicantParseOnInterfaceRemovedInd(const uint8_t* data, size_t length, string& param);
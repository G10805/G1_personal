/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/hostapd/IHostapd.h>
#include <aidl/android/hardware/wifi/hostapd/IHostapdCallback.h>

#include "hostapd_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::hostapd;


typedef struct
{
    IfaceParams ifaceParams;
    NetworkParams nwParams;
} AddAccessPointReqParam;

typedef struct
{
    string ifaceName;
    vector<uint8_t> clientAddress;
    Ieee80211ReasonCode reasonCode;
} ForceClientDisconnectReqParam;

typedef struct
{
    string ifaceName;
    string instanceName;
} OnFailureIndParam;

bool HostapdSerializeAddAccessPointReq(const IfaceParams& ifaceParams, const NetworkParams& nwParams, vector<uint8_t>& payload);

bool HostapdSerializeAddAccessPointCfm(vector<uint8_t>& payload);

bool HostapdSerializeForceClientDisconnectReq(const string& ifaceName, const vector<uint8_t>& clientAddress, Ieee80211ReasonCode reasonCode, vector<uint8_t>& payload);

bool HostapdSerializeForceClientDisconnectCfm(vector<uint8_t>& payload);

bool HostapdSerializeRegisterCallbackReq(vector<uint8_t>& payload);

bool HostapdSerializeRegisterCallbackCfm(vector<uint8_t>& payload);

bool HostapdSerializeRemoveAccessPointReq(const string& ifaceName, vector<uint8_t>& payload);

bool HostapdSerializeRemoveAccessPointCfm(vector<uint8_t>& payload);

bool HostapdSerializeSetDebugParamsReq(DebugLevel level, vector<uint8_t>& payload);

bool HostapdSerializeSetDebugParamsCfm(vector<uint8_t>& payload);

bool HostapdSerializeTerminateReq(vector<uint8_t>& payload);

bool HostapdSerializeTerminateCfm(vector<uint8_t>& payload);

bool HostapdSerializeOnApInstanceInfoChangedInd(const ApInfo& apInfo, vector<uint8_t>& payload);

bool HostapdSerializeOnConnectedClientsChangedInd(const ClientInfo& clientInfo, vector<uint8_t>& payload);

bool HostapdSerializeOnFailureInd(const string& ifaceName, const string& instanceName, vector<uint8_t>& payload);

bool HostapdParseAddAccessPointReq(const uint8_t* data, size_t length, AddAccessPointReqParam& param);

bool HostapdParseAddAccessPointCfm(const uint8_t* data, size_t length);

bool HostapdParseForceClientDisconnectReq(const uint8_t* data, size_t length, ForceClientDisconnectReqParam& param);

bool HostapdParseForceClientDisconnectCfm(const uint8_t* data, size_t length);

bool HostapdParseRegisterCallbackReq(const uint8_t* data, size_t length);

bool HostapdParseRegisterCallbackCfm(const uint8_t* data, size_t length);

bool HostapdParseRemoveAccessPointReq(const uint8_t* data, size_t length, string& param);

bool HostapdParseRemoveAccessPointCfm(const uint8_t* data, size_t length);

bool HostapdParseSetDebugParamsReq(const uint8_t* data, size_t length, DebugLevel& param);

bool HostapdParseSetDebugParamsCfm(const uint8_t* data, size_t length);

bool HostapdParseTerminateReq(const uint8_t* data, size_t length);

bool HostapdParseTerminateCfm(const uint8_t* data, size_t length);

bool HostapdParseOnApInstanceInfoChangedInd(const uint8_t* data, size_t length, ApInfo& param);

bool HostapdParseOnConnectedClientsChangedInd(const uint8_t* data, size_t length, ClientInfo& param);

bool HostapdParseOnFailureInd(const uint8_t* data, size_t length, OnFailureIndParam& param);
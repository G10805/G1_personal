/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/ISupplicantP2pNetwork.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetBssidCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<MacAddress> result;
} GetClientListCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetIdCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetInterfaceNameCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetSsidCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    IfaceType result;
} GetTypeCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsCurrentCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsGroupOwnerCfmP2pNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} IsPersistentCfmP2pNetworkParam;

bool SupplicantP2pNetworkSerializeGetBssidReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetBssidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetClientListReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetClientListCfm(const HalStatusParam& status, const vector<MacAddress>& result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetIdReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetInterfaceNameReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetInterfaceNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetSsidReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetTypeReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsCurrentReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsCurrentCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsGroupOwnerReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsGroupOwnerCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsPersistentReq(vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeIsPersistentCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeSetClientListReq(const vector<MacAddress>& clients, vector<uint8_t>& payload);

bool SupplicantP2pNetworkSerializeSetClientListCfm(vector<uint8_t>& payload);

bool SupplicantP2pNetworkParseGetBssidReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetBssidCfm(const uint8_t* data, size_t length, GetBssidCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseGetClientListReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetClientListCfm(const uint8_t* data, size_t length, GetClientListCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseGetIdReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseGetInterfaceNameReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetInterfaceNameCfm(const uint8_t* data, size_t length, GetInterfaceNameCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseGetSsidReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseGetTypeReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseIsCurrentReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseIsCurrentCfm(const uint8_t* data, size_t length, IsCurrentCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseIsGroupOwnerReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseIsGroupOwnerCfm(const uint8_t* data, size_t length, IsGroupOwnerCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseIsPersistentReq(const uint8_t* data, size_t length);

bool SupplicantP2pNetworkParseIsPersistentCfm(const uint8_t* data, size_t length, IsPersistentCfmP2pNetworkParam& param);

bool SupplicantP2pNetworkParseSetClientListReq(const uint8_t* data, size_t length, vector<MacAddress>& param);

bool SupplicantP2pNetworkParseSetClientListCfm(const uint8_t* data, size_t length);
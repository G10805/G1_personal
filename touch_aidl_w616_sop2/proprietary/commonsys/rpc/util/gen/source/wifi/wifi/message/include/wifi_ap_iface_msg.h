/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiApIface.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmApIfaceParam;

typedef struct
{
    HalStatusParam status;
    vector<string> result;
} GetBridgedInstancesCfmApIfaceParam;

typedef struct
{
    HalStatusParam status;
    array<uint8_t, 6> result;
} GetFactoryMacAddressCfmApIfaceParam;

bool WifiApIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool WifiApIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool WifiApIfaceSerializeGetBridgedInstancesReq(vector<uint8_t>& payload);

bool WifiApIfaceSerializeGetBridgedInstancesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool WifiApIfaceSerializeGetFactoryMacAddressReq(vector<uint8_t>& payload);

bool WifiApIfaceSerializeGetFactoryMacAddressCfm(const HalStatusParam& status, const array<uint8_t, 6>& result, vector<uint8_t>& payload);

bool WifiApIfaceSerializeSetCountryCodeReq(const array<uint8_t, 2>& code, vector<uint8_t>& payload);

bool WifiApIfaceSerializeSetCountryCodeCfm(vector<uint8_t>& payload);

bool WifiApIfaceSerializeResetToFactoryMacAddressReq(vector<uint8_t>& payload);

bool WifiApIfaceSerializeResetToFactoryMacAddressCfm(vector<uint8_t>& payload);

bool WifiApIfaceSerializeSetMacAddressReq(const array<uint8_t, 6>& mac, vector<uint8_t>& payload);

bool WifiApIfaceSerializeSetMacAddressCfm(vector<uint8_t>& payload);

bool WifiApIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool WifiApIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmApIfaceParam& param);

bool WifiApIfaceParseGetBridgedInstancesReq(const uint8_t* data, size_t length);

bool WifiApIfaceParseGetBridgedInstancesCfm(const uint8_t* data, size_t length, GetBridgedInstancesCfmApIfaceParam& param);

bool WifiApIfaceParseGetFactoryMacAddressReq(const uint8_t* data, size_t length);

bool WifiApIfaceParseGetFactoryMacAddressCfm(const uint8_t* data, size_t length, GetFactoryMacAddressCfmApIfaceParam& param);

bool WifiApIfaceParseSetCountryCodeReq(const uint8_t* data, size_t length, array<uint8_t, 2>& param);

bool WifiApIfaceParseSetCountryCodeCfm(const uint8_t* data, size_t length);

bool WifiApIfaceParseResetToFactoryMacAddressReq(const uint8_t* data, size_t length);

bool WifiApIfaceParseResetToFactoryMacAddressCfm(const uint8_t* data, size_t length);

bool WifiApIfaceParseSetMacAddressReq(const uint8_t* data, size_t length, array<uint8_t, 6>& param);

bool WifiApIfaceParseSetMacAddressCfm(const uint8_t* data, size_t length);
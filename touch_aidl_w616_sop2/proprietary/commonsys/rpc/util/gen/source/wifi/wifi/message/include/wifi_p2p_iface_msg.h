/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiP2pIface.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmP2pIfaceParam;

bool WifiP2pIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool WifiP2pIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool WifiP2pIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool WifiP2pIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmP2pIfaceParam& param);
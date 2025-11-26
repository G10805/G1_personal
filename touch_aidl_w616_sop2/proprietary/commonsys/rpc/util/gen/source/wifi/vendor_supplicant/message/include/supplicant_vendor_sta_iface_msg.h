/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendorStaIface.h>
#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendorStaIfaceCallback.h>

#include "supplicant_vendor_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::vendor::qti::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    string result;
} DoDriverCmdCfmParam;

typedef struct
{
    string ifaceName;
    string eventStr;
} OnCtrlEventIndParam;

bool SupplicantVendorStaIfaceSerializeDoDriverCmdReq(const string& command, vector<uint8_t>& payload);

bool SupplicantVendorStaIfaceSerializeDoDriverCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackReq(vector<uint8_t>& payload);

bool SupplicantVendorStaIfaceSerializeRegisterSupplicantVendorStaIfaceCallbackCfm(vector<uint8_t>& payload);

bool SupplicantVendorStaIfaceSerializeOnCtrlEventInd(const string& ifaceName, const string& eventStr, vector<uint8_t>& payload);

bool SupplicantVendorStaIfaceParseDoDriverCmdReq(const uint8_t* data, size_t length, string& param);

bool SupplicantVendorStaIfaceParseDoDriverCmdCfm(const uint8_t* data, size_t length, DoDriverCmdCfmParam& param);

bool SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackReq(const uint8_t* data, size_t length);

bool SupplicantVendorStaIfaceParseRegisterSupplicantVendorStaIfaceCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantVendorStaIfaceParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param);
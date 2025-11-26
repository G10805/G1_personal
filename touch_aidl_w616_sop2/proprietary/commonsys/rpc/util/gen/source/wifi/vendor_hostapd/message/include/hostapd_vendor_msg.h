/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/vendor/qti/hardware/wifi/hostapd/IHostapdVendor.h>
#include <aidl/vendor/qti/hardware/wifi/hostapd/IHostapdVendorCallback.h>

#include "hostapd_vendor_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::vendor::qti::hardware::wifi::hostapd;


typedef struct
{
    HalStatusParam status;
    vector<string> result;
} ListVendorInterfacesCfmParam;

typedef struct
{
    string iface;
    string cmd;
} DoDriverCmdReqParam;

typedef struct
{
    HalStatusParam status;
    string result;
} DoDriverCmdCfmParam;

typedef struct
{
    string ifaceName;
    string event_str;
} OnCtrlEventIndParam;

typedef struct
{
    string ifname;
    string instanceName;
} OnFailureIndParam;

bool HostapdVendorSerializeListVendorInterfacesReq(vector<uint8_t>& payload);

bool HostapdVendorSerializeListVendorInterfacesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload);

bool HostapdVendorSerializeRegisterHostapdVendorCallbackReq(vector<uint8_t>& payload);

bool HostapdVendorSerializeRegisterHostapdVendorCallbackCfm(vector<uint8_t>& payload);

bool HostapdVendorSerializeDoDriverCmdReq(const string& iface, const string& cmd, vector<uint8_t>& payload);

bool HostapdVendorSerializeDoDriverCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool HostapdVendorSerializeOnCtrlEventInd(const string& ifaceName, const string& event_str, vector<uint8_t>& payload);

bool HostapdVendorSerializeOnApInstanceInfoChangedInd(const VendorApInfo& apInfo, vector<uint8_t>& payload);

bool HostapdVendorSerializeOnFailureInd(const string& ifname, const string& instanceName, vector<uint8_t>& payload);

bool HostapdVendorParseListVendorInterfacesReq(const uint8_t* data, size_t length);

bool HostapdVendorParseListVendorInterfacesCfm(const uint8_t* data, size_t length, ListVendorInterfacesCfmParam& param);

bool HostapdVendorParseRegisterHostapdVendorCallbackReq(const uint8_t* data, size_t length);

bool HostapdVendorParseRegisterHostapdVendorCallbackCfm(const uint8_t* data, size_t length);

bool HostapdVendorParseDoDriverCmdReq(const uint8_t* data, size_t length, DoDriverCmdReqParam& param);

bool HostapdVendorParseDoDriverCmdCfm(const uint8_t* data, size_t length, DoDriverCmdCfmParam& param);

bool HostapdVendorParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param);

bool HostapdVendorParseOnApInstanceInfoChangedInd(const uint8_t* data, size_t length, VendorApInfo& param);

bool HostapdVendorParseOnFailureInd(const uint8_t* data, size_t length, OnFailureIndParam& param);
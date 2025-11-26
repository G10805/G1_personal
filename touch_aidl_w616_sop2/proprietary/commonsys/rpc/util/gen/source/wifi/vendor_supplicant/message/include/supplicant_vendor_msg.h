/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/vendor/qti/hardware/wifi/supplicant/ISupplicantVendor.h>

#include "supplicant_vendor_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::vendor::qti::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetVendorInterfaceCfmParam;

typedef struct
{
    HalStatusParam status;
    vector<IVendorIfaceInfo> result;
} ListVendorInterfacesCfmParam;

bool SupplicantVendorSerializeGetVendorInterfaceReq(const IVendorIfaceInfo& ifaceInfo, vector<uint8_t>& payload);

bool SupplicantVendorSerializeGetVendorInterfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantVendorSerializeListVendorInterfacesReq(vector<uint8_t>& payload);

bool SupplicantVendorSerializeListVendorInterfacesCfm(const HalStatusParam& status, const vector<IVendorIfaceInfo>& result, vector<uint8_t>& payload);

bool SupplicantVendorParseGetVendorInterfaceReq(const uint8_t* data, size_t length, IVendorIfaceInfo& param);

bool SupplicantVendorParseGetVendorInterfaceCfm(const uint8_t* data, size_t length, GetVendorInterfaceCfmParam& param);

bool SupplicantVendorParseListVendorInterfacesReq(const uint8_t* data, size_t length);

bool SupplicantVendorParseListVendorInterfacesCfm(const uint8_t* data, size_t length, ListVendorInterfacesCfmParam& param);
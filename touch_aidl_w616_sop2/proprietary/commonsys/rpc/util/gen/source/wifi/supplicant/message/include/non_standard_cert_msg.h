/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/INonStandardCertCallback.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


bool NonStandardCertSerializeGetBlobReq(const string& alias, vector<uint8_t>& payload);

bool NonStandardCertSerializeGetBlobCfm(const vector<uint8_t>& result, vector<uint8_t>& payload);

bool NonStandardCertSerializeListAliasesReq(const string& prefix, vector<uint8_t>& payload);

bool NonStandardCertSerializeListAliasesCfm(const vector<string>& result, vector<uint8_t>& payload);

bool NonStandardCertParseGetBlobReq(const uint8_t* data, size_t length, string& param);

bool NonStandardCertParseGetBlobCfm(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool NonStandardCertParseListAliasesReq(const uint8_t* data, size_t length, string& param);

bool NonStandardCertParseListAliasesCfm(const uint8_t* data, size_t length, vector<string>& param);
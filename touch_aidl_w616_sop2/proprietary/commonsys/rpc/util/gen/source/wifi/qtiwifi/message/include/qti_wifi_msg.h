/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/vendor/qti/hardware/wifi/qtiwifi/IQtiWifi.h>
#include <aidl/vendor/qti/hardware/wifi/qtiwifi/IQtiWifiCallback.h>

#include "qti_wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::vendor::qti::hardware::wifi::qtiwifi;


typedef struct
{
    HalStatusParam status;
    vector<IfaceInfo> result;
} ListAvailableInterfacesCfmParam;

typedef struct
{
    string iface;
    string cmd;
} DoQtiWifiCmdReqParam;

typedef struct
{
    HalStatusParam status;
    string result;
} DoQtiWifiCmdCfmParam;

typedef struct
{
    string iface;
    string event;
} OnCtrlEventIndParam;

bool QtiWifiSerializeListAvailableInterfacesReq(vector<uint8_t>& payload);

bool QtiWifiSerializeListAvailableInterfacesCfm(const HalStatusParam& status, const vector<IfaceInfo>& result, vector<uint8_t>& payload);

bool QtiWifiSerializeRegisterQtiWifiCallbackReq(vector<uint8_t>& payload);

bool QtiWifiSerializeRegisterQtiWifiCallbackCfm(vector<uint8_t>& payload);

bool QtiWifiSerializeDoQtiWifiCmdReq(const string& iface, const string& cmd, vector<uint8_t>& payload);

bool QtiWifiSerializeDoQtiWifiCmdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool QtiWifiSerializeOnCtrlEventInd(const string& iface, const string& event, vector<uint8_t>& payload);

bool QtiWifiParseListAvailableInterfacesReq(const uint8_t* data, size_t length);

bool QtiWifiParseListAvailableInterfacesCfm(const uint8_t* data, size_t length, ListAvailableInterfacesCfmParam& param);

bool QtiWifiParseRegisterQtiWifiCallbackReq(const uint8_t* data, size_t length);

bool QtiWifiParseRegisterQtiWifiCallbackCfm(const uint8_t* data, size_t length);

bool QtiWifiParseDoQtiWifiCmdReq(const uint8_t* data, size_t length, DoQtiWifiCmdReqParam& param);

bool QtiWifiParseDoQtiWifiCmdCfm(const uint8_t* data, size_t length, DoQtiWifiCmdCfmParam& param);

bool QtiWifiParseOnCtrlEventInd(const uint8_t* data, size_t length, OnCtrlEventIndParam& param);
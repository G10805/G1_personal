/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/bluetooth/IBluetoothHci.h>
#include <aidl/android/hardware/bluetooth/IBluetoothHciCallbacks.h>

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::bluetooth;


bool BluetoothHciSerializeCloseReq(vector<uint8_t>& payload);

bool BluetoothHciSerializeCloseCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeInitializeReq(vector<uint8_t>& payload);

bool BluetoothHciSerializeInitializeCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeSendAclDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciSerializeSendAclDataCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeSendHciCommandReq(const vector<uint8_t>& command, vector<uint8_t>& payload);

bool BluetoothHciSerializeSendHciCommandCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeSendIsoDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciSerializeSendIsoDataCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeSendScoDataReq(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciSerializeSendScoDataCfm(vector<uint8_t>& payload);

bool BluetoothHciSerializeAclDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciSerializeHciEventReceivedInd(const vector<uint8_t>& event, vector<uint8_t>& payload);

bool BluetoothHciSerializeInitializationCompleteInd(Status status, vector<uint8_t>& payload);

bool BluetoothHciSerializeIsoDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciSerializeScoDataReceivedInd(const vector<uint8_t>& data, vector<uint8_t>& payload);

bool BluetoothHciParseCloseReq(const uint8_t* data, size_t length);

bool BluetoothHciParseCloseCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseInitializeReq(const uint8_t* data, size_t length);

bool BluetoothHciParseInitializeCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseSendAclDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseSendAclDataCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseSendHciCommandReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseSendHciCommandCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseSendIsoDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseSendIsoDataCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseSendScoDataReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseSendScoDataCfm(const uint8_t* data, size_t length);

bool BluetoothHciParseAclDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseHciEventReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseInitializationCompleteInd(const uint8_t* data, size_t length, Status& param);

bool BluetoothHciParseIsoDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool BluetoothHciParseScoDataReceivedInd(const uint8_t* data, size_t length, vector<uint8_t>& param);
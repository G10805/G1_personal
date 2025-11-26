/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/IBluetoothHandsfreeAudio.h>

using std::array;
using std::string;
using std::vector;

using namespace aidl::vendor::qti::hardware::bluetooth::handsfree_audio;


bool BluetoothHandsfreeAudioSerializeStartReq(const HandsfreeAudioParameter& data, vector<uint8_t>& payload);

bool BluetoothHandsfreeAudioSerializeStartCfm(vector<uint8_t>& payload);

bool BluetoothHandsfreeAudioSerializeStopReq(vector<uint8_t>& payload);

bool BluetoothHandsfreeAudioSerializeStopCfm(vector<uint8_t>& payload);

bool BluetoothHandsfreeAudioParseStartReq(const uint8_t* data, size_t length, HandsfreeAudioParameter& param);

bool BluetoothHandsfreeAudioParseStartCfm(const uint8_t* data, size_t length);

bool BluetoothHandsfreeAudioParseStopReq(const uint8_t* data, size_t length);

bool BluetoothHandsfreeAudioParseStopCfm(const uint8_t* data, size_t length);
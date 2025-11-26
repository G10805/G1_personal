/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "bluetooth_handsfree_audio_msg.h"

#include "HalStatus.pb.h"
#include "HandsfreeAudioParameter.pb.h"
#include "IBluetoothHandsfreeAudio.pb.h"

using StartReq = ::bluetooth::handsfree::audio::IBluetoothHandsfreeAudio_StartReq;

using HalStatus_P = ::bluetooth::handsfree::audio::HalStatus;
using HandsfreeAudioParameter_P = ::bluetooth::handsfree::audio::HandsfreeAudioParameter;


static void HandsfreeAudioParameter2Message(const HandsfreeAudioParameter& data, HandsfreeAudioParameter_P& msgBluetoothHandsfreeAudio)
{
    msgBluetoothHandsfreeAudio.set_samplingrate(data.samplingRate);
    msgBluetoothHandsfreeAudio.set_volume(data.volume);
}

bool BluetoothHandsfreeAudioSerializeStartReq(const HandsfreeAudioParameter& data, vector<uint8_t>& payload)
{
    StartReq msgBluetoothHandsfreeAudio;
    HandsfreeAudioParameter_P* data_p = msgBluetoothHandsfreeAudio.mutable_data();
    HandsfreeAudioParameter2Message(data, *data_p);
    return ProtoMessageSerialize(&msgBluetoothHandsfreeAudio, payload);
}

bool BluetoothHandsfreeAudioSerializeStartCfm(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHandsfreeAudioSerializeStopReq(vector<uint8_t>& payload)
{
    return true;
}

bool BluetoothHandsfreeAudioSerializeStopCfm(vector<uint8_t>& payload)
{
    return true;
}

static void Message2HandsfreeAudioParameter(const HandsfreeAudioParameter_P& msgBluetoothHandsfreeAudio, HandsfreeAudioParameter& data)
{
    data.samplingRate = msgBluetoothHandsfreeAudio.samplingrate();
    data.volume = msgBluetoothHandsfreeAudio.volume();
}

bool BluetoothHandsfreeAudioParseStartReq(const uint8_t* data, size_t length, HandsfreeAudioParameter& param)
{
    StartReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HandsfreeAudioParameter(msg.data(), param);
        return true;
    }
    return false;
}

bool BluetoothHandsfreeAudioParseStartCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHandsfreeAudioParseStopReq(const uint8_t* data, size_t length)
{
    return true;
}

bool BluetoothHandsfreeAudioParseStopCfm(const uint8_t* data, size_t length)
{
    return true;
}
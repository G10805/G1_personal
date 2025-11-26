/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "bluetooth_handsfree_audio_msg.h"

using std::array;
using std::string;
using std::vector;

#define VERIFY(f)   { \
                        if (!(f)) \
                            { \
                                ALOGE("%s: %s fail, return false", __func__, #f); \
                                return false; \
                            } \
                    }

static bool COMPARE_STR(string& str1, string& str2, const char *output)
{
    ALOGD("%s: %s: str1 size=%d, str2 size=%d", __func__, output, str1.size(), str2.size());
    if (!str1.compare(str2))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename VectorType>
static bool COMPARE_VEC(vector<VectorType>& vec1, vector<VectorType>& vec2, const char *output)
{
    ALOGD("%s: %s: vec1 size=%d, vec2 size=%d", __func__, output, vec1.size(), vec2.size());
    if (vec1 == vec2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename ArrayType, size_t N>
static bool COMPARE_ARY(array<ArrayType, N>& array1, array<ArrayType, N>& array2, const char *output)
{
    ALOGD("%s: %s: array1 size=%d, array2 size=%d", __func__, output, array1.size(), array2.size());
    if (array1 == array2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

static bool COMPARE_BUF(void *buf1, void *buf2, size_t length, const char *output)
{
    ALOGD("%s: size: %d: %s", __func__, length, output);
    if (!memcmp(buf1, buf2, length))
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

template<typename ObjType>
static bool COMPARE_OBJ(ObjType &obj1, ObjType &obj2, const char *output)
{
    ALOGD("%s: %s: obj1 size=%d, obj2 size=%d", __func__, output, sizeof(obj1), sizeof(obj2));
    if (obj1 == obj2)
    {
        ALOGD("%s: same %s", __func__, output);
        return true;
    }
    else
    {
        ALOGE("%s: mis-matched %s", __func__, output);
        return false;
    }
}

static void INIT_HANDSFREE_AUDIO_PARAMETER(HandsfreeAudioParameter& data)
{
    data.samplingRate = 59;
    data.volume = 106;
}

static bool T_BLUETOOTH_HANDSFREE_AUDIO_START_REQ()
{
    HandsfreeAudioParameter data;
    INIT_HANDSFREE_AUDIO_PARAMETER(data);
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHandsfreeAudioSerializeStartReq(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    HandsfreeAudioParameter param;
    bool msgParseResult = BluetoothHandsfreeAudioParseStartReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_OBJ(data, param, "data (HandsfreeAudioParameter)"));
    return true;
}

static bool T_BLUETOOTH_HANDSFREE_AUDIO_START_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHandsfreeAudioSerializeStartCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHandsfreeAudioParseStartCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHandsfreeAudioSerializeStopReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHandsfreeAudioParseStopReq(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HANDSFREE_AUDIO_STOP_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHandsfreeAudioSerializeStopCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHandsfreeAudioParseStopCfm(payload.data(), payload.size());
    return true;
}

static bool T_ALL()
{
    VERIFY(T_BLUETOOTH_HANDSFREE_AUDIO_START_REQ());
    VERIFY(T_BLUETOOTH_HANDSFREE_AUDIO_START_CFM());
    VERIFY(T_BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ());
    VERIFY(T_BLUETOOTH_HANDSFREE_AUDIO_STOP_CFM());
    ALOGD("ALL 4 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("bluetooth-handsfree-audio-msg-test starts");
    int count = 1;
    if (1 < argc)
    {
        count = atoi(argv[1]);
    }
    for (int i = 0; i < count; i++)
    {
        if (T_ALL())
        {
            ALOGD("LOOP #%d FINISHED AND PASSED", i + 1);
        }
        else
        {
            ALOGE("LOOP #%d FAILED", i + 1);
        }
    }
    return 0;
}
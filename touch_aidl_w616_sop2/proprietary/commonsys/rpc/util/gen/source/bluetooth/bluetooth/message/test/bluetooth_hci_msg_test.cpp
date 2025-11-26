/* This is an auto-generated source file. */

#include <cstdint>
#include <sys/stat.h>
#include <utils/Log.h>

#include "common_util.h"
#include "proto_common.h"

#include "bluetooth_hci_msg.h"

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

static bool T_BLUETOOTH_HCI_CLOSE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeCloseReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseCloseReq(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_CLOSE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeCloseCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseCloseCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_INITIALIZE_REQ()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeInitializeReq(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseInitializeReq(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_INITIALIZE_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeInitializeCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseInitializeCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_ACL_DATA_REQ()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0x4e;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendAclDataReq(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseSendAclDataReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_ACL_DATA_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendAclDataCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseSendAclDataCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_HCI_COMMAND_REQ()
{
    vector<uint8_t> command;
    command.resize(4);
    for (int index = 0; index < 4; index++)
    {
        command[index] = 0xb1;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendHciCommandReq(command, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseSendHciCommandReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(command, param, "command (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_HCI_COMMAND_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendHciCommandCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseSendHciCommandCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_ISO_DATA_REQ()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0xcd;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendIsoDataReq(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseSendIsoDataReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_ISO_DATA_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendIsoDataCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseSendIsoDataCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_SCO_DATA_REQ()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0xb8;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendScoDataReq(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseSendScoDataReq(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_SEND_SCO_DATA_CFM()
{
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeSendScoDataCfm(payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult);
    bool msgParseResult = BluetoothHciParseSendScoDataCfm(payload.data(), payload.size());
    return true;
}

static bool T_BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0xd7;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeAclDataReceivedInd(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseAclDataReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_HCI_EVENT_RECEIVED_IND()
{
    vector<uint8_t> event;
    event.resize(4);
    for (int index = 0; index < 4; index++)
    {
        event[index] = 0x95;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeHciEventReceivedInd(event, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseHciEventReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(event, param, "event (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_INITIALIZATION_COMPLETE_IND()
{
    Status status = Status(1);
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeInitializationCompleteInd(status, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    Status param;
    bool msgParseResult = BluetoothHciParseInitializationCompleteInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_BUF(&status, &param, sizeof(Status), "status (Status)"));
    return true;
}

static bool T_BLUETOOTH_HCI_ISO_DATA_RECEIVED_IND()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0x1a;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeIsoDataReceivedInd(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseIsoDataReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND()
{
    vector<uint8_t> data;
    data.resize(4);
    for (int index = 0; index < 4; index++)
    {
        data[index] = 0xc5;
    }
    vector<uint8_t> payload;
    bool msgSerializeResult = BluetoothHciSerializeScoDataReceivedInd(data, payload);
    ALOGD("%s: msgSerializeResult : %d,  output size: %d", __func__, msgSerializeResult, payload.size());

    vector<uint8_t> param;
    bool msgParseResult = BluetoothHciParseScoDataReceivedInd(payload.data(), payload.size(), param);
    VERIFY(COMPARE_VEC(data, param, "data (vector<uint8_t>)"));
    return true;
}

static bool T_ALL()
{
    VERIFY(T_BLUETOOTH_HCI_CLOSE_REQ());
    VERIFY(T_BLUETOOTH_HCI_CLOSE_CFM());
    VERIFY(T_BLUETOOTH_HCI_INITIALIZE_REQ());
    VERIFY(T_BLUETOOTH_HCI_INITIALIZE_CFM());
    VERIFY(T_BLUETOOTH_HCI_SEND_ACL_DATA_REQ());
    VERIFY(T_BLUETOOTH_HCI_SEND_ACL_DATA_CFM());
    VERIFY(T_BLUETOOTH_HCI_SEND_HCI_COMMAND_REQ());
    VERIFY(T_BLUETOOTH_HCI_SEND_HCI_COMMAND_CFM());
    VERIFY(T_BLUETOOTH_HCI_SEND_ISO_DATA_REQ());
    VERIFY(T_BLUETOOTH_HCI_SEND_ISO_DATA_CFM());
    VERIFY(T_BLUETOOTH_HCI_SEND_SCO_DATA_REQ());
    VERIFY(T_BLUETOOTH_HCI_SEND_SCO_DATA_CFM());
    VERIFY(T_BLUETOOTH_HCI_ACL_DATA_RECEIVED_IND());
    VERIFY(T_BLUETOOTH_HCI_HCI_EVENT_RECEIVED_IND());
    VERIFY(T_BLUETOOTH_HCI_INITIALIZATION_COMPLETE_IND());
    VERIFY(T_BLUETOOTH_HCI_ISO_DATA_RECEIVED_IND());
    VERIFY(T_BLUETOOTH_HCI_SCO_DATA_RECEIVED_IND());
    ALOGD("ALL 17 TESTS PASSED");
    return true;
}

int main(int argc, char** argv)
{
    ALOGD("bluetooth-hci-msg-test starts");
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
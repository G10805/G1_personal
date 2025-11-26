/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_p2p_iface_msg.h"

#include "HalStatus.pb.h"
#include "IWifiP2pIface.pb.h"

using GetNameCfm = ::wifi::IWifiP2pIface_GetNameCfm;

using HalStatus_P = ::wifi::HalStatus;


bool WifiP2pIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiP2pIface)
{
    msgWifiP2pIface.set_status(status.status);
    msgWifiP2pIface.set_info(status.info);
}

bool WifiP2pIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgWifiP2pIface;
    HalStatus_P* status_p = msgWifiP2pIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiP2pIface.set_result(result);
    return ProtoMessageSerialize(&msgWifiP2pIface, payload);
}

bool WifiP2pIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgWifiP2pIface, HalStatusParam& status)
{
    status.status = msgWifiP2pIface.status();
    status.info = msgWifiP2pIface.info();
}

bool WifiP2pIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmP2pIfaceParam& param)
{
    GetNameCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}
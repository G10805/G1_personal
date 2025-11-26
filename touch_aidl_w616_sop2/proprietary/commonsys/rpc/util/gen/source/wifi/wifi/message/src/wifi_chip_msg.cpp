/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_chip_msg.h"

#include "AfcChannelAllowance.pb.h"
#include "AvailableAfcChannelInfo.pb.h"
#include "AvailableAfcFrequencyInfo.pb.h"
#include "HalStatus.pb.h"
#include "IWifiApIface.pb.h"
#include "IWifiChip.pb.h"
#include "IWifiChipEventCallback.pb.h"
#include "IWifiNanIface.pb.h"
#include "IWifiP2pIface.pb.h"
#include "IWifiRttController.pb.h"
#include "IWifiStaIface.pb.h"
#include "IfaceConcurrencyType.pb.h"
#include "IfaceType.pb.h"
#include "WifiAntennaMode.pb.h"
#include "WifiBand.pb.h"
#include "WifiChannelWidthInMhz.pb.h"
#include "WifiChipCapabilities.pb.h"
#include "WifiDebugHostWakeReasonRxIcmpPacketDetails.pb.h"
#include "WifiDebugHostWakeReasonRxMulticastPacketDetails.pb.h"
#include "WifiDebugHostWakeReasonRxPacketDetails.pb.h"
#include "WifiDebugHostWakeReasonStats.pb.h"
#include "WifiDebugRingBufferStatus.pb.h"
#include "WifiDebugRingBufferVerboseLevel.pb.h"
#include "WifiRadioCombination.pb.h"
#include "WifiRadioConfiguration.pb.h"
#include "WifiStatusCode.pb.h"
#include "WifiUsableChannel.pb.h"

using ConfigureChipReq = ::wifi::IWifiChip_ConfigureChipReq;
using CreateApIfaceCfm = ::wifi::IWifiChip_CreateApIfaceCfm;
using CreateBridgedApIfaceCfm = ::wifi::IWifiChip_CreateBridgedApIfaceCfm;
using CreateNanIfaceCfm = ::wifi::IWifiChip_CreateNanIfaceCfm;
using CreateP2pIfaceCfm = ::wifi::IWifiChip_CreateP2pIfaceCfm;
using CreateRttControllerCfm = ::wifi::IWifiChip_CreateRttControllerCfm;
using CreateRttControllerReq = ::wifi::IWifiChip_CreateRttControllerReq;
using CreateStaIfaceCfm = ::wifi::IWifiChip_CreateStaIfaceCfm;
using EnableDebugErrorAlertsReq = ::wifi::IWifiChip_EnableDebugErrorAlertsReq;
using EnableStaChannelForPeerNetworkReq = ::wifi::IWifiChip_EnableStaChannelForPeerNetworkReq;
using ForceDumpToDebugRingBufferReq = ::wifi::IWifiChip_ForceDumpToDebugRingBufferReq;
using GetApIfaceCfm = ::wifi::IWifiChip_GetApIfaceCfm;
using GetApIfaceNamesCfm = ::wifi::IWifiChip_GetApIfaceNamesCfm;
using GetApIfaceReq = ::wifi::IWifiChip_GetApIfaceReq;
using GetAvailableModesCfm = ::wifi::IWifiChip_GetAvailableModesCfm;
using GetDebugHostWakeReasonStatsCfm = ::wifi::IWifiChip_GetDebugHostWakeReasonStatsCfm;
using GetDebugRingBuffersStatusCfm = ::wifi::IWifiChip_GetDebugRingBuffersStatusCfm;
using GetFeatureSetCfm = ::wifi::IWifiChip_GetFeatureSetCfm;
using GetIdCfm = ::wifi::IWifiChip_GetIdCfm;
using GetModeCfm = ::wifi::IWifiChip_GetModeCfm;
using GetNanIfaceCfm = ::wifi::IWifiChip_GetNanIfaceCfm;
using GetNanIfaceNamesCfm = ::wifi::IWifiChip_GetNanIfaceNamesCfm;
using GetNanIfaceReq = ::wifi::IWifiChip_GetNanIfaceReq;
using GetP2pIfaceCfm = ::wifi::IWifiChip_GetP2pIfaceCfm;
using GetP2pIfaceNamesCfm = ::wifi::IWifiChip_GetP2pIfaceNamesCfm;
using GetP2pIfaceReq = ::wifi::IWifiChip_GetP2pIfaceReq;
using GetStaIfaceCfm = ::wifi::IWifiChip_GetStaIfaceCfm;
using GetStaIfaceNamesCfm = ::wifi::IWifiChip_GetStaIfaceNamesCfm;
using GetStaIfaceReq = ::wifi::IWifiChip_GetStaIfaceReq;
using GetSupportedRadioCombinationsCfm = ::wifi::IWifiChip_GetSupportedRadioCombinationsCfm;
using GetUsableChannelsCfm = ::wifi::IWifiChip_GetUsableChannelsCfm;
using GetUsableChannelsReq = ::wifi::IWifiChip_GetUsableChannelsReq;
using GetWifiChipCapabilitiesCfm = ::wifi::IWifiChip_GetWifiChipCapabilitiesCfm;
using OnChipReconfigureFailureInd = ::wifi::IWifiChipEventCallback_OnChipReconfigureFailureInd;
using OnChipReconfiguredInd = ::wifi::IWifiChipEventCallback_OnChipReconfiguredInd;
using OnDebugErrorAlertInd = ::wifi::IWifiChipEventCallback_OnDebugErrorAlertInd;
using OnDebugRingBufferDataAvailableInd = ::wifi::IWifiChipEventCallback_OnDebugRingBufferDataAvailableInd;
using OnIfaceAddedInd = ::wifi::IWifiChipEventCallback_OnIfaceAddedInd;
using OnIfaceRemovedInd = ::wifi::IWifiChipEventCallback_OnIfaceRemovedInd;
using OnRadioModeChangeInd = ::wifi::IWifiChipEventCallback_OnRadioModeChangeInd;
using RemoveApIfaceReq = ::wifi::IWifiChip_RemoveApIfaceReq;
using RemoveIfaceInstanceFromBridgedApIfaceReq = ::wifi::IWifiChip_RemoveIfaceInstanceFromBridgedApIfaceReq;
using RemoveNanIfaceReq = ::wifi::IWifiChip_RemoveNanIfaceReq;
using RemoveP2pIfaceReq = ::wifi::IWifiChip_RemoveP2pIfaceReq;
using RemoveStaIfaceReq = ::wifi::IWifiChip_RemoveStaIfaceReq;
using RequestChipDebugInfoCfm = ::wifi::IWifiChip_RequestChipDebugInfoCfm;
using RequestDriverDebugDumpCfm = ::wifi::IWifiChip_RequestDriverDebugDumpCfm;
using RequestFirmwareDebugDumpCfm = ::wifi::IWifiChip_RequestFirmwareDebugDumpCfm;
using SelectTxPowerScenarioReq = ::wifi::IWifiChip_SelectTxPowerScenarioReq;
using SetAfcChannelAllowanceReq = ::wifi::IWifiChip_SetAfcChannelAllowanceReq;
using SetCoexUnsafeChannelsReq = ::wifi::IWifiChip_SetCoexUnsafeChannelsReq;
using SetCountryCodeReq = ::wifi::IWifiChip_SetCountryCodeReq;
using SetLatencyModeReq = ::wifi::IWifiChip_SetLatencyModeReq;
using SetMloModeReq = ::wifi::IWifiChip_SetMloModeReq;
using SetMultiStaPrimaryConnectionReq = ::wifi::IWifiChip_SetMultiStaPrimaryConnectionReq;
using SetMultiStaUseCaseReq = ::wifi::IWifiChip_SetMultiStaUseCaseReq;
using StartLoggingToDebugRingBufferReq = ::wifi::IWifiChip_StartLoggingToDebugRingBufferReq;

using AfcChannelAllowance_P = ::wifi::AfcChannelAllowance;
using AvailableAfcChannelInfo_P = ::wifi::AvailableAfcChannelInfo;
using AvailableAfcFrequencyInfo_P = ::wifi::AvailableAfcFrequencyInfo;
using ChipConcurrencyCombinationLimit_P = ::wifi::IWifiChip_ChipConcurrencyCombinationLimit;
using ChipConcurrencyCombination_P = ::wifi::IWifiChip_ChipConcurrencyCombination;
using ChipDebugInfo_P = ::wifi::IWifiChip_ChipDebugInfo;
using ChipMloMode_P = ::wifi::IWifiChip_ChipMloMode;
using ChipMode_P = ::wifi::IWifiChip_ChipMode;
using CoexUnsafeChannel_P = ::wifi::IWifiChip_CoexUnsafeChannel;
using HalStatus_P = ::wifi::HalStatus;
using IWifiApIface_P = ::wifi::IWifiApIface;
using IWifiNanIface_P = ::wifi::IWifiNanIface;
using IWifiP2pIface_P = ::wifi::IWifiP2pIface;
using IWifiRttController_P = ::wifi::IWifiRttController;
using IWifiStaIface_P = ::wifi::IWifiStaIface;
using IfaceConcurrencyType_P = ::wifi::ict::IfaceConcurrencyType;
using IfaceInfo_P = ::wifi::IWifiChipEventCallback_IfaceInfo;
using IfaceType_P = ::wifi::it::IfaceType;
using LatencyMode_P = ::wifi::IWifiChip_LatencyMode;
using MultiStaUseCase_P = ::wifi::IWifiChip_MultiStaUseCase;
using RadioModeInfo_P = ::wifi::IWifiChipEventCallback_RadioModeInfo;
using TxPowerScenario_P = ::wifi::IWifiChip_TxPowerScenario;
using WifiAntennaMode_P = ::wifi::WifiAntennaMode;
using WifiBand_P = ::wifi::WifiBand;
using WifiChannelWidthInMhz_P = ::wifi::WifiChannelWidthInMhz;
using WifiChipCapabilities_P = ::wifi::WifiChipCapabilities;
using WifiDebugHostWakeReasonRxIcmpPacketDetails_P = ::wifi::WifiDebugHostWakeReasonRxIcmpPacketDetails;
using WifiDebugHostWakeReasonRxMulticastPacketDetails_P = ::wifi::WifiDebugHostWakeReasonRxMulticastPacketDetails;
using WifiDebugHostWakeReasonRxPacketDetails_P = ::wifi::WifiDebugHostWakeReasonRxPacketDetails;
using WifiDebugHostWakeReasonStats_P = ::wifi::WifiDebugHostWakeReasonStats;
using WifiDebugRingBufferStatus_P = ::wifi::WifiDebugRingBufferStatus;
using WifiDebugRingBufferVerboseLevel_P = ::wifi::wdrbvl::WifiDebugRingBufferVerboseLevel;
using WifiRadioCombination_P = ::wifi::WifiRadioCombination;
using WifiRadioConfiguration_P = ::wifi::WifiRadioConfiguration;
using WifiStatusCode_P = ::wifi::wsc::WifiStatusCode;
using WifiUsableChannel_P = ::wifi::WifiUsableChannel;


bool WifiChipSerializeConfigureChipReq(int32_t modeId, vector<uint8_t>& payload)
{
    ConfigureChipReq msgWifiChip;
    msgWifiChip.set_modeid(modeId);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiChip)
{
    msgWifiChip.set_status(status.status);
    msgWifiChip.set_info(status.info);
}

bool WifiChipSerializeConfigureChipCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeCreateApIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

static void IWifiApIface2Message(int32_t result, IWifiApIface_P& msgWifiChip)
{
    msgWifiChip.set_instanceid(result);
}

bool WifiChipSerializeCreateApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateApIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiApIface_P* result_p = msgWifiChip.mutable_result();
    IWifiApIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeCreateBridgedApIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeCreateBridgedApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateBridgedApIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiApIface_P* result_p = msgWifiChip.mutable_result();
    IWifiApIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeCreateNanIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

static void IWifiNanIface2Message(int32_t result, IWifiNanIface_P& msgWifiChip)
{
    msgWifiChip.set_instanceid(result);
}

bool WifiChipSerializeCreateNanIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateNanIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiNanIface_P* result_p = msgWifiChip.mutable_result();
    IWifiNanIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeCreateP2pIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

static void IWifiP2pIface2Message(int32_t result, IWifiP2pIface_P& msgWifiChip)
{
    msgWifiChip.set_instanceid(result);
}

bool WifiChipSerializeCreateP2pIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateP2pIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiP2pIface_P* result_p = msgWifiChip.mutable_result();
    IWifiP2pIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void IWifiStaIface2Message(int32_t boundIface, IWifiStaIface_P& msgWifiChip)
{
    msgWifiChip.set_instanceid(boundIface);
}

bool WifiChipSerializeCreateRttControllerReq(int32_t boundIface, vector<uint8_t>& payload)
{
    CreateRttControllerReq msgWifiChip;
    IWifiStaIface_P* boundIface_p = msgWifiChip.mutable_boundiface();
    IWifiStaIface2Message(boundIface, *boundIface_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void IWifiRttController2Message(int32_t result, IWifiRttController_P& msgWifiChip)
{
    msgWifiChip.set_instanceid(result);
}

bool WifiChipSerializeCreateRttControllerCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateRttControllerCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiRttController_P* result_p = msgWifiChip.mutable_result();
    IWifiRttController2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeCreateStaIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeCreateStaIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    CreateStaIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiStaIface_P* result_p = msgWifiChip.mutable_result();
    IWifiStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeEnableDebugErrorAlertsReq(bool enable, vector<uint8_t>& payload)
{
    EnableDebugErrorAlertsReq msgWifiChip;
    msgWifiChip.set_enable(enable);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeEnableDebugErrorAlertsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeFlushRingBufferToFileReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeFlushRingBufferToFileCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeForceDumpToDebugRingBufferReq(const string& ringName, vector<uint8_t>& payload)
{
    ForceDumpToDebugRingBufferReq msgWifiChip;
    msgWifiChip.set_ringname(ringName);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeForceDumpToDebugRingBufferCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetApIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    GetApIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetApIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetApIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiApIface_P* result_p = msgWifiChip.mutable_result();
    IWifiApIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetApIfaceNamesReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetApIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    GetApIfaceNamesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifiChip.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetAvailableModesReq(vector<uint8_t>& payload)
{
    return true;
}

static void ChipConcurrencyCombinationLimit2Message(const IWifiChip::ChipConcurrencyCombinationLimit& limits, ChipConcurrencyCombinationLimit_P& msgWifiChip)
{
    int size_types = limits.types.size();
    if (0 < size_types)
    {
        for (int index = 0; index < size_types; index++)
        {
            msgWifiChip.add_types((IfaceConcurrencyType_P) limits.types[index]);
        }
    }
    msgWifiChip.set_maxifaces(limits.maxIfaces);
}

static void ChipConcurrencyCombination2Message(const IWifiChip::ChipConcurrencyCombination& availableCombinations, ChipConcurrencyCombination_P& msgWifiChip)
{
    int size_limits = availableCombinations.limits.size();
    if (0 < size_limits)
    {
        for (int index = 0; index < size_limits; index++)
        {
            ChipConcurrencyCombinationLimit_P *limits_p = msgWifiChip.add_limits();
            ChipConcurrencyCombinationLimit2Message(availableCombinations.limits[index], *limits_p);
        }
    }
}

static void ChipMode2Message(const IWifiChip::ChipMode& result, ChipMode_P& msgWifiChip)
{
    msgWifiChip.set_id(result.id);
    int size_availablecombinations = result.availableCombinations.size();
    if (0 < size_availablecombinations)
    {
        for (int index = 0; index < size_availablecombinations; index++)
        {
            ChipConcurrencyCombination_P *availableCombinations_p = msgWifiChip.add_availablecombinations();
            ChipConcurrencyCombination2Message(result.availableCombinations[index], *availableCombinations_p);
        }
    }
}

bool WifiChipSerializeGetAvailableModesCfm(const HalStatusParam& status, const vector<IWifiChip::ChipMode>& result, vector<uint8_t>& payload)
{
    GetAvailableModesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            ChipMode_P *result_p = msgWifiChip.add_result();
            ChipMode2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetFeatureSetReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetFeatureSetCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetFeatureSetCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiChip.set_result(result);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetDebugHostWakeReasonStatsReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiDebugHostWakeReasonRxPacketDetails2Message(const WifiDebugHostWakeReasonRxPacketDetails& rxPktWakeDetails, WifiDebugHostWakeReasonRxPacketDetails_P& msgWifiChip)
{
    msgWifiChip.set_rxunicastcnt(rxPktWakeDetails.rxUnicastCnt);
    msgWifiChip.set_rxmulticastcnt(rxPktWakeDetails.rxMulticastCnt);
    msgWifiChip.set_rxbroadcastcnt(rxPktWakeDetails.rxBroadcastCnt);
}

static void WifiDebugHostWakeReasonRxMulticastPacketDetails2Message(const WifiDebugHostWakeReasonRxMulticastPacketDetails& rxMulticastPkWakeDetails, WifiDebugHostWakeReasonRxMulticastPacketDetails_P& msgWifiChip)
{
    msgWifiChip.set_ipv4rxmulticastaddrcnt(rxMulticastPkWakeDetails.ipv4RxMulticastAddrCnt);
    msgWifiChip.set_ipv6rxmulticastaddrcnt(rxMulticastPkWakeDetails.ipv6RxMulticastAddrCnt);
    msgWifiChip.set_otherrxmulticastaddrcnt(rxMulticastPkWakeDetails.otherRxMulticastAddrCnt);
}

static void WifiDebugHostWakeReasonRxIcmpPacketDetails2Message(const WifiDebugHostWakeReasonRxIcmpPacketDetails& rxIcmpPkWakeDetails, WifiDebugHostWakeReasonRxIcmpPacketDetails_P& msgWifiChip)
{
    msgWifiChip.set_icmppkt(rxIcmpPkWakeDetails.icmpPkt);
    msgWifiChip.set_icmp6pkt(rxIcmpPkWakeDetails.icmp6Pkt);
    msgWifiChip.set_icmp6ra(rxIcmpPkWakeDetails.icmp6Ra);
    msgWifiChip.set_icmp6na(rxIcmpPkWakeDetails.icmp6Na);
    msgWifiChip.set_icmp6ns(rxIcmpPkWakeDetails.icmp6Ns);
}

static void WifiDebugHostWakeReasonStats2Message(const WifiDebugHostWakeReasonStats& result, WifiDebugHostWakeReasonStats_P& msgWifiChip)
{
    msgWifiChip.set_totalcmdeventwakecnt(result.totalCmdEventWakeCnt);
    int size_cmdeventwakecntpertype = result.cmdEventWakeCntPerType.size();
    if (0 < size_cmdeventwakecntpertype)
    {
        for (int index = 0; index < size_cmdeventwakecntpertype; index++)
        {
            msgWifiChip.add_cmdeventwakecntpertype(result.cmdEventWakeCntPerType[index]);
        }
    }
    msgWifiChip.set_totaldriverfwlocalwakecnt(result.totalDriverFwLocalWakeCnt);
    int size_driverfwlocalwakecntpertype = result.driverFwLocalWakeCntPerType.size();
    if (0 < size_driverfwlocalwakecntpertype)
    {
        for (int index = 0; index < size_driverfwlocalwakecntpertype; index++)
        {
            msgWifiChip.add_driverfwlocalwakecntpertype(result.driverFwLocalWakeCntPerType[index]);
        }
    }
    msgWifiChip.set_totalrxpacketwakecnt(result.totalRxPacketWakeCnt);
    WifiDebugHostWakeReasonRxPacketDetails_P* rxPktWakeDetails_p = msgWifiChip.mutable_rxpktwakedetails();
    WifiDebugHostWakeReasonRxPacketDetails2Message(result.rxPktWakeDetails, *rxPktWakeDetails_p);
    WifiDebugHostWakeReasonRxMulticastPacketDetails_P* rxMulticastPkWakeDetails_p = msgWifiChip.mutable_rxmulticastpkwakedetails();
    WifiDebugHostWakeReasonRxMulticastPacketDetails2Message(result.rxMulticastPkWakeDetails, *rxMulticastPkWakeDetails_p);
    WifiDebugHostWakeReasonRxIcmpPacketDetails_P* rxIcmpPkWakeDetails_p = msgWifiChip.mutable_rxicmppkwakedetails();
    WifiDebugHostWakeReasonRxIcmpPacketDetails2Message(result.rxIcmpPkWakeDetails, *rxIcmpPkWakeDetails_p);
}

bool WifiChipSerializeGetDebugHostWakeReasonStatsCfm(const HalStatusParam& status, const WifiDebugHostWakeReasonStats& result, vector<uint8_t>& payload)
{
    GetDebugHostWakeReasonStatsCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    WifiDebugHostWakeReasonStats_P* result_p = msgWifiChip.mutable_result();
    WifiDebugHostWakeReasonStats2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetDebugRingBuffersStatusReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiDebugRingBufferStatus2Message(const WifiDebugRingBufferStatus& result, WifiDebugRingBufferStatus_P& msgWifiChip)
{
    msgWifiChip.set_ringname(result.ringName);
    msgWifiChip.set_flags(result.flags);
    msgWifiChip.set_ringid(result.ringId);
    msgWifiChip.set_sizeinbytes(result.sizeInBytes);
    msgWifiChip.set_freesizeinbytes(result.freeSizeInBytes);
    msgWifiChip.set_verboselevel(result.verboseLevel);
}

bool WifiChipSerializeGetDebugRingBuffersStatusCfm(const HalStatusParam& status, const vector<WifiDebugRingBufferStatus>& result, vector<uint8_t>& payload)
{
    GetDebugRingBuffersStatusCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            WifiDebugRingBufferStatus_P *result_p = msgWifiChip.add_result();
            WifiDebugRingBufferStatus2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetIdCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiChip.set_result(result);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetModeReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetModeCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetModeCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiChip.set_result(result);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetNanIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    GetNanIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetNanIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetNanIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiNanIface_P* result_p = msgWifiChip.mutable_result();
    IWifiNanIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetNanIfaceNamesReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetNanIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    GetNanIfaceNamesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifiChip.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetP2pIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    GetP2pIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetP2pIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetP2pIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiP2pIface_P* result_p = msgWifiChip.mutable_result();
    IWifiP2pIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetP2pIfaceNamesReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetP2pIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    GetP2pIfaceNamesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifiChip.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetStaIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    GetStaIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetStaIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetStaIfaceCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiStaIface_P* result_p = msgWifiChip.mutable_result();
    IWifiStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetStaIfaceNamesReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeGetStaIfaceNamesCfm(const HalStatusParam& status, const vector<string>& result, vector<uint8_t>& payload)
{
    GetStaIfaceNamesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgWifiChip.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetSupportedRadioCombinationsReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiRadioConfiguration2Message(const WifiRadioConfiguration& radioConfigurations, WifiRadioConfiguration_P& msgWifiChip)
{
    msgWifiChip.set_bandinfo((WifiBand_P) radioConfigurations.bandInfo);
    msgWifiChip.set_antennamode((WifiAntennaMode_P) radioConfigurations.antennaMode);
}

static void WifiRadioCombination2Message(const WifiRadioCombination& result, WifiRadioCombination_P& msgWifiChip)
{
    int size_radioconfigurations = result.radioConfigurations.size();
    if (0 < size_radioconfigurations)
    {
        for (int index = 0; index < size_radioconfigurations; index++)
        {
            WifiRadioConfiguration_P *radioConfigurations_p = msgWifiChip.add_radioconfigurations();
            WifiRadioConfiguration2Message(result.radioConfigurations[index], *radioConfigurations_p);
        }
    }
}

bool WifiChipSerializeGetSupportedRadioCombinationsCfm(const HalStatusParam& status, const vector<WifiRadioCombination>& result, vector<uint8_t>& payload)
{
    GetSupportedRadioCombinationsCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            WifiRadioCombination_P *result_p = msgWifiChip.add_result();
            WifiRadioCombination2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetWifiChipCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiChipCapabilities2Message(const WifiChipCapabilities& result, WifiChipCapabilities_P& msgWifiChip)
{
    msgWifiChip.set_maxmloassociationlinkcount(result.maxMloAssociationLinkCount);
    msgWifiChip.set_maxmlostrlinkcount(result.maxMloStrLinkCount);
    msgWifiChip.set_maxconcurrenttdlssessioncount(result.maxConcurrentTdlsSessionCount);
}

bool WifiChipSerializeGetWifiChipCapabilitiesCfm(const HalStatusParam& status, const WifiChipCapabilities& result, vector<uint8_t>& payload)
{
    GetWifiChipCapabilitiesCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    WifiChipCapabilities_P* result_p = msgWifiChip.mutable_result();
    WifiChipCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeGetUsableChannelsReq(WifiBand band, int32_t ifaceModeMask, int32_t filterMask, vector<uint8_t>& payload)
{
    GetUsableChannelsReq msgWifiChip;
    msgWifiChip.set_band((WifiBand_P) band);
    msgWifiChip.set_ifacemodemask(ifaceModeMask);
    msgWifiChip.set_filtermask(filterMask);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void WifiUsableChannel2Message(const WifiUsableChannel& result, WifiUsableChannel_P& msgWifiChip)
{
    msgWifiChip.set_channel(result.channel);
    msgWifiChip.set_channelbandwidth((WifiChannelWidthInMhz_P) result.channelBandwidth);
    msgWifiChip.set_ifacemodemask(result.ifaceModeMask);
}

bool WifiChipSerializeGetUsableChannelsCfm(const HalStatusParam& status, const vector<WifiUsableChannel>& result, vector<uint8_t>& payload)
{
    GetUsableChannelsCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            WifiUsableChannel_P *result_p = msgWifiChip.add_result();
            WifiUsableChannel2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void AvailableAfcFrequencyInfo2Message(const AvailableAfcFrequencyInfo& availableAfcFrequencyInfos, AvailableAfcFrequencyInfo_P& msgWifiChip)
{
    msgWifiChip.set_startfrequencymhz(availableAfcFrequencyInfos.startFrequencyMhz);
    msgWifiChip.set_endfrequencymhz(availableAfcFrequencyInfos.endFrequencyMhz);
    msgWifiChip.set_maxpsd(availableAfcFrequencyInfos.maxPsd);
}

static void AvailableAfcChannelInfo2Message(const AvailableAfcChannelInfo& availableAfcChannelInfos, AvailableAfcChannelInfo_P& msgWifiChip)
{
    msgWifiChip.set_globaloperatingclass(availableAfcChannelInfos.globalOperatingClass);
    msgWifiChip.set_channelcfi(availableAfcChannelInfos.channelCfi);
    msgWifiChip.set_maxeirpdbm(availableAfcChannelInfos.maxEirpDbm);
}

static void AfcChannelAllowance2Message(const AfcChannelAllowance& afcChannelAllowance, AfcChannelAllowance_P& msgWifiChip)
{
    int size_availableafcfrequencyinfos = afcChannelAllowance.availableAfcFrequencyInfos.size();
    if (0 < size_availableafcfrequencyinfos)
    {
        for (int index = 0; index < size_availableafcfrequencyinfos; index++)
        {
            AvailableAfcFrequencyInfo_P *availableAfcFrequencyInfos_p = msgWifiChip.add_availableafcfrequencyinfos();
            AvailableAfcFrequencyInfo2Message(afcChannelAllowance.availableAfcFrequencyInfos[index], *availableAfcFrequencyInfos_p);
        }
    }
    int size_availableafcchannelinfos = afcChannelAllowance.availableAfcChannelInfos.size();
    if (0 < size_availableafcchannelinfos)
    {
        for (int index = 0; index < size_availableafcchannelinfos; index++)
        {
            AvailableAfcChannelInfo_P *availableAfcChannelInfos_p = msgWifiChip.add_availableafcchannelinfos();
            AvailableAfcChannelInfo2Message(afcChannelAllowance.availableAfcChannelInfos[index], *availableAfcChannelInfos_p);
        }
    }
    msgWifiChip.set_availabilityexpiretimems(afcChannelAllowance.availabilityExpireTimeMs);
}

bool WifiChipSerializeSetAfcChannelAllowanceReq(const AfcChannelAllowance& afcChannelAllowance, vector<uint8_t>& payload)
{
    SetAfcChannelAllowanceReq msgWifiChip;
    AfcChannelAllowance_P* afcChannelAllowance_p = msgWifiChip.mutable_afcchannelallowance();
    AfcChannelAllowance2Message(afcChannelAllowance, *afcChannelAllowance_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetAfcChannelAllowanceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRegisterEventCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRemoveApIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    RemoveApIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRemoveApIfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceReq(const string& brIfaceName, const string& ifaceInstanceName, vector<uint8_t>& payload)
{
    RemoveIfaceInstanceFromBridgedApIfaceReq msgWifiChip;
    msgWifiChip.set_brifacename(brIfaceName);
    msgWifiChip.set_ifaceinstancename(ifaceInstanceName);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRemoveIfaceInstanceFromBridgedApIfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRemoveNanIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    RemoveNanIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRemoveNanIfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRemoveP2pIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    RemoveP2pIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRemoveP2pIfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRemoveStaIfaceReq(const string& ifname, vector<uint8_t>& payload)
{
    RemoveStaIfaceReq msgWifiChip;
    msgWifiChip.set_ifname(ifname);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRemoveStaIfaceCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRequestChipDebugInfoReq(vector<uint8_t>& payload)
{
    return true;
}

static void ChipDebugInfo2Message(const IWifiChip::ChipDebugInfo& result, ChipDebugInfo_P& msgWifiChip)
{
    msgWifiChip.set_driverdescription(result.driverDescription);
    msgWifiChip.set_firmwaredescription(result.firmwareDescription);
}

bool WifiChipSerializeRequestChipDebugInfoCfm(const HalStatusParam& status, const IWifiChip::ChipDebugInfo& result, vector<uint8_t>& payload)
{
    RequestChipDebugInfoCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    ChipDebugInfo_P* result_p = msgWifiChip.mutable_result();
    ChipDebugInfo2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRequestDriverDebugDumpReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRequestDriverDebugDumpCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    RequestDriverDebugDumpCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgWifiChip.set_result(resultStr);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeRequestFirmwareDebugDumpReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeRequestFirmwareDebugDumpCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    RequestFirmwareDebugDumpCfm msgWifiChip;
    HalStatus_P* status_p = msgWifiChip.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgWifiChip.set_result(resultStr);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeResetTxPowerScenarioReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeResetTxPowerScenarioCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSelectTxPowerScenarioReq(IWifiChip::TxPowerScenario scenario, vector<uint8_t>& payload)
{
    SelectTxPowerScenarioReq msgWifiChip;
    msgWifiChip.set_scenario((TxPowerScenario_P) scenario);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSelectTxPowerScenarioCfm(vector<uint8_t>& payload)
{
    return true;
}

static void CoexUnsafeChannel2Message(const IWifiChip::CoexUnsafeChannel& unsafeChannels, CoexUnsafeChannel_P& msgWifiChip)
{
    msgWifiChip.set_band((WifiBand_P) unsafeChannels.band);
    msgWifiChip.set_channel(unsafeChannels.channel);
    msgWifiChip.set_powercapdbm(unsafeChannels.powerCapDbm);
}

bool WifiChipSerializeSetCoexUnsafeChannelsReq(const vector<IWifiChip::CoexUnsafeChannel>& unsafeChannels, int32_t restrictions, vector<uint8_t>& payload)
{
    SetCoexUnsafeChannelsReq msgWifiChip;
    int size_unsafechannels = unsafeChannels.size();
    if (0 < size_unsafechannels)
    {
        for (int index = 0; index < size_unsafechannels; index++)
        {
            CoexUnsafeChannel_P *unsafeChannels_p = msgWifiChip.add_unsafechannels();
            CoexUnsafeChannel2Message(unsafeChannels[index], *unsafeChannels_p);
        }
    }
    msgWifiChip.set_restrictions(restrictions);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetCoexUnsafeChannelsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSetCountryCodeReq(const array<uint8_t, 2>& code, vector<uint8_t>& payload)
{
    SetCountryCodeReq msgWifiChip;
    string codeStr;
    Array2String(code, codeStr);
    msgWifiChip.set_code(codeStr);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetCountryCodeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSetLatencyModeReq(IWifiChip::LatencyMode mode, vector<uint8_t>& payload)
{
    SetLatencyModeReq msgWifiChip;
    msgWifiChip.set_mode((LatencyMode_P) mode);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetLatencyModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSetMultiStaPrimaryConnectionReq(const string& ifName, vector<uint8_t>& payload)
{
    SetMultiStaPrimaryConnectionReq msgWifiChip;
    msgWifiChip.set_ifname(ifName);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetMultiStaPrimaryConnectionCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSetMultiStaUseCaseReq(IWifiChip::MultiStaUseCase useCase, vector<uint8_t>& payload)
{
    SetMultiStaUseCaseReq msgWifiChip;
    msgWifiChip.set_usecase((MultiStaUseCase_P) useCase);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetMultiStaUseCaseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeStartLoggingToDebugRingBufferReq(const string& ringName, WifiDebugRingBufferVerboseLevel verboseLevel, int32_t maxIntervalInSec, int32_t minDataSizeInBytes, vector<uint8_t>& payload)
{
    StartLoggingToDebugRingBufferReq msgWifiChip;
    msgWifiChip.set_ringname(ringName);
    msgWifiChip.set_verboselevel((WifiDebugRingBufferVerboseLevel_P) verboseLevel);
    msgWifiChip.set_maxintervalinsec(maxIntervalInSec);
    msgWifiChip.set_mindatasizeinbytes(minDataSizeInBytes);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeStartLoggingToDebugRingBufferCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeStopLoggingToDebugRingBufferReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeStopLoggingToDebugRingBufferCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeTriggerSubsystemRestartReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeTriggerSubsystemRestartCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeEnableStaChannelForPeerNetworkReq(int32_t channelCategoryEnableFlag, vector<uint8_t>& payload)
{
    EnableStaChannelForPeerNetworkReq msgWifiChip;
    msgWifiChip.set_channelcategoryenableflag(channelCategoryEnableFlag);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeEnableStaChannelForPeerNetworkCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeSetMloModeReq(IWifiChip::ChipMloMode mode, vector<uint8_t>& payload)
{
    SetMloModeReq msgWifiChip;
    msgWifiChip.set_mode((ChipMloMode_P) mode);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeSetMloModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiChipSerializeOnChipReconfigureFailureInd(WifiStatusCode status, vector<uint8_t>& payload)
{
    OnChipReconfigureFailureInd msgWifiChip;
    msgWifiChip.set_status((WifiStatusCode_P) status);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeOnChipReconfiguredInd(int32_t modeId, vector<uint8_t>& payload)
{
    OnChipReconfiguredInd msgWifiChip;
    msgWifiChip.set_modeid(modeId);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeOnDebugErrorAlertInd(int32_t errorCode, const vector<uint8_t>& debugData, vector<uint8_t>& payload)
{
    OnDebugErrorAlertInd msgWifiChip;
    msgWifiChip.set_errorcode(errorCode);
    string debugDataStr;
    Vector2String(debugData, debugDataStr);
    msgWifiChip.set_debugdata(debugDataStr);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeOnDebugRingBufferDataAvailableInd(const WifiDebugRingBufferStatus& status, const vector<uint8_t>& data, vector<uint8_t>& payload)
{
    OnDebugRingBufferDataAvailableInd msgWifiChip;
    WifiDebugRingBufferStatus_P* status_p = msgWifiChip.mutable_status();
    WifiDebugRingBufferStatus2Message(status, *status_p);
    string dataStr;
    Vector2String(data, dataStr);
    msgWifiChip.set_data(dataStr);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeOnIfaceAddedInd(IfaceType type, const string& name, vector<uint8_t>& payload)
{
    OnIfaceAddedInd msgWifiChip;
    msgWifiChip.set_type((IfaceType_P) type);
    msgWifiChip.set_name(name);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipSerializeOnIfaceRemovedInd(IfaceType type, const string& name, vector<uint8_t>& payload)
{
    OnIfaceRemovedInd msgWifiChip;
    msgWifiChip.set_type((IfaceType_P) type);
    msgWifiChip.set_name(name);
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

static void IfaceInfo2Message(const IWifiChipEventCallback::IfaceInfo& ifaceInfos, IfaceInfo_P& msgWifiChip)
{
    msgWifiChip.set_name(ifaceInfos.name);
    msgWifiChip.set_channel(ifaceInfos.channel);
}

static void RadioModeInfo2Message(const IWifiChipEventCallback::RadioModeInfo& radioModeInfos, RadioModeInfo_P& msgWifiChip)
{
    msgWifiChip.set_radioid(radioModeInfos.radioId);
    msgWifiChip.set_bandinfo((WifiBand_P) radioModeInfos.bandInfo);
    int size_ifaceinfos = radioModeInfos.ifaceInfos.size();
    if (0 < size_ifaceinfos)
    {
        for (int index = 0; index < size_ifaceinfos; index++)
        {
            IfaceInfo_P *ifaceInfos_p = msgWifiChip.add_ifaceinfos();
            IfaceInfo2Message(radioModeInfos.ifaceInfos[index], *ifaceInfos_p);
        }
    }
}

bool WifiChipSerializeOnRadioModeChangeInd(const vector<IWifiChipEventCallback::RadioModeInfo>& radioModeInfos, vector<uint8_t>& payload)
{
    OnRadioModeChangeInd msgWifiChip;
    int size_radiomodeinfos = radioModeInfos.size();
    if (0 < size_radiomodeinfos)
    {
        for (int index = 0; index < size_radiomodeinfos; index++)
        {
            RadioModeInfo_P *radioModeInfos_p = msgWifiChip.add_radiomodeinfos();
            RadioModeInfo2Message(radioModeInfos[index], *radioModeInfos_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiChip, payload);
}

bool WifiChipParseConfigureChipReq(const uint8_t* data, size_t length, int32_t& param)
{
    ConfigureChipReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modeid();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgWifiChip, HalStatusParam& status)
{
    status.status = msgWifiChip.status();
    status.info = msgWifiChip.info();
}

bool WifiChipParseConfigureChipCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseCreateApIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2IWifiApIface(const IWifiApIface_P& msgWifiChip, int32_t& result)
{
    result = msgWifiChip.instanceid();
}

bool WifiChipParseCreateApIfaceCfm(const uint8_t* data, size_t length, CreateApIfaceCfmChipParam& param)
{
    CreateApIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiApIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseCreateBridgedApIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseCreateBridgedApIfaceCfm(const uint8_t* data, size_t length, CreateBridgedApIfaceCfmChipParam& param)
{
    CreateBridgedApIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiApIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseCreateNanIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2IWifiNanIface(const IWifiNanIface_P& msgWifiChip, int32_t& result)
{
    result = msgWifiChip.instanceid();
}

bool WifiChipParseCreateNanIfaceCfm(const uint8_t* data, size_t length, CreateNanIfaceCfmChipParam& param)
{
    CreateNanIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiNanIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseCreateP2pIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2IWifiP2pIface(const IWifiP2pIface_P& msgWifiChip, int32_t& result)
{
    result = msgWifiChip.instanceid();
}

bool WifiChipParseCreateP2pIfaceCfm(const uint8_t* data, size_t length, CreateP2pIfaceCfmChipParam& param)
{
    CreateP2pIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiP2pIface(msg.result(), param.result);
        return true;
    }
    return false;
}

static void Message2IWifiStaIface(const IWifiStaIface_P& msgWifiChip, int32_t& boundIface)
{
    boundIface = msgWifiChip.instanceid();
}

bool WifiChipParseCreateRttControllerReq(const uint8_t* data, size_t length, int32_t& param)
{
    CreateRttControllerReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2IWifiStaIface(msg.boundiface(), param);
        return true;
    }
    return false;
}

static void Message2IWifiRttController(const IWifiRttController_P& msgWifiChip, int32_t& result)
{
    result = msgWifiChip.instanceid();
}

bool WifiChipParseCreateRttControllerCfm(const uint8_t* data, size_t length, CreateRttControllerCfmChipParam& param)
{
    CreateRttControllerCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiRttController(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseCreateStaIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseCreateStaIfaceCfm(const uint8_t* data, size_t length, CreateStaIfaceCfmChipParam& param)
{
    CreateStaIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseEnableDebugErrorAlertsReq(const uint8_t* data, size_t length, bool& param)
{
    EnableDebugErrorAlertsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool WifiChipParseEnableDebugErrorAlertsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseFlushRingBufferToFileReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseFlushRingBufferToFileCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseForceDumpToDebugRingBufferReq(const uint8_t* data, size_t length, string& param)
{
    ForceDumpToDebugRingBufferReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ringname();
        return true;
    }
    return false;
}

bool WifiChipParseForceDumpToDebugRingBufferCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetApIfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetApIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseGetApIfaceCfm(const uint8_t* data, size_t length, GetApIfaceCfmChipParam& param)
{
    GetApIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiApIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetApIfaceNamesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetApIfaceNamesCfm(const uint8_t* data, size_t length, GetApIfaceNamesCfmChipParam& param)
{
    GetApIfaceNamesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetAvailableModesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ChipConcurrencyCombinationLimit(const ChipConcurrencyCombinationLimit_P& msgWifiChip, IWifiChip::ChipConcurrencyCombinationLimit& limits)
{
    int size_types = msgWifiChip.types_size();
    if (0 < size_types)
    {
        limits.types.resize(size_types);
        for (int index = 0; index < size_types; index++)
        {
            limits.types[index] = (IfaceConcurrencyType) msgWifiChip.types(index);
        }
    }
    limits.maxIfaces = msgWifiChip.maxifaces();
}

static void Message2ChipConcurrencyCombination(const ChipConcurrencyCombination_P& msgWifiChip, IWifiChip::ChipConcurrencyCombination& availableCombinations)
{
    int size_limits = msgWifiChip.limits_size();
    if (0 < size_limits)
    {
        availableCombinations.limits.resize(size_limits);
        for (int index = 0; index < size_limits; index++)
        {
            Message2ChipConcurrencyCombinationLimit(msgWifiChip.limits(index), availableCombinations.limits[index]);
        }
    }
}

static void Message2ChipMode(const ChipMode_P& msgWifiChip, IWifiChip::ChipMode& result)
{
    result.id = msgWifiChip.id();
    int size_availablecombinations = msgWifiChip.availablecombinations_size();
    if (0 < size_availablecombinations)
    {
        result.availableCombinations.resize(size_availablecombinations);
        for (int index = 0; index < size_availablecombinations; index++)
        {
            Message2ChipConcurrencyCombination(msgWifiChip.availablecombinations(index), result.availableCombinations[index]);
        }
    }
}

bool WifiChipParseGetAvailableModesCfm(const uint8_t* data, size_t length, GetAvailableModesCfmChipParam& param)
{
    GetAvailableModesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2ChipMode(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetFeatureSetReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetFeatureSetCfm(const uint8_t* data, size_t length, GetFeatureSetCfmChipParam& param)
{
    GetFeatureSetCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool WifiChipParseGetDebugHostWakeReasonStatsReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiDebugHostWakeReasonRxPacketDetails(const WifiDebugHostWakeReasonRxPacketDetails_P& msgWifiChip, WifiDebugHostWakeReasonRxPacketDetails& rxPktWakeDetails)
{
    rxPktWakeDetails.rxUnicastCnt = msgWifiChip.rxunicastcnt();
    rxPktWakeDetails.rxMulticastCnt = msgWifiChip.rxmulticastcnt();
    rxPktWakeDetails.rxBroadcastCnt = msgWifiChip.rxbroadcastcnt();
}

static void Message2WifiDebugHostWakeReasonRxMulticastPacketDetails(const WifiDebugHostWakeReasonRxMulticastPacketDetails_P& msgWifiChip, WifiDebugHostWakeReasonRxMulticastPacketDetails& rxMulticastPkWakeDetails)
{
    rxMulticastPkWakeDetails.ipv4RxMulticastAddrCnt = msgWifiChip.ipv4rxmulticastaddrcnt();
    rxMulticastPkWakeDetails.ipv6RxMulticastAddrCnt = msgWifiChip.ipv6rxmulticastaddrcnt();
    rxMulticastPkWakeDetails.otherRxMulticastAddrCnt = msgWifiChip.otherrxmulticastaddrcnt();
}

static void Message2WifiDebugHostWakeReasonRxIcmpPacketDetails(const WifiDebugHostWakeReasonRxIcmpPacketDetails_P& msgWifiChip, WifiDebugHostWakeReasonRxIcmpPacketDetails& rxIcmpPkWakeDetails)
{
    rxIcmpPkWakeDetails.icmpPkt = msgWifiChip.icmppkt();
    rxIcmpPkWakeDetails.icmp6Pkt = msgWifiChip.icmp6pkt();
    rxIcmpPkWakeDetails.icmp6Ra = msgWifiChip.icmp6ra();
    rxIcmpPkWakeDetails.icmp6Na = msgWifiChip.icmp6na();
    rxIcmpPkWakeDetails.icmp6Ns = msgWifiChip.icmp6ns();
}

static void Message2WifiDebugHostWakeReasonStats(const WifiDebugHostWakeReasonStats_P& msgWifiChip, WifiDebugHostWakeReasonStats& result)
{
    result.totalCmdEventWakeCnt = msgWifiChip.totalcmdeventwakecnt();
    int size_cmdeventwakecntpertype = msgWifiChip.cmdeventwakecntpertype_size();
    if (0 < size_cmdeventwakecntpertype)
    {
        result.cmdEventWakeCntPerType.resize(size_cmdeventwakecntpertype);
        for (int index = 0; index < size_cmdeventwakecntpertype; index++)
        {
            result.cmdEventWakeCntPerType[index] = msgWifiChip.cmdeventwakecntpertype(index);
        }
    }
    result.totalDriverFwLocalWakeCnt = msgWifiChip.totaldriverfwlocalwakecnt();
    int size_driverfwlocalwakecntpertype = msgWifiChip.driverfwlocalwakecntpertype_size();
    if (0 < size_driverfwlocalwakecntpertype)
    {
        result.driverFwLocalWakeCntPerType.resize(size_driverfwlocalwakecntpertype);
        for (int index = 0; index < size_driverfwlocalwakecntpertype; index++)
        {
            result.driverFwLocalWakeCntPerType[index] = msgWifiChip.driverfwlocalwakecntpertype(index);
        }
    }
    result.totalRxPacketWakeCnt = msgWifiChip.totalrxpacketwakecnt();
    Message2WifiDebugHostWakeReasonRxPacketDetails(msgWifiChip.rxpktwakedetails(), result.rxPktWakeDetails);
    Message2WifiDebugHostWakeReasonRxMulticastPacketDetails(msgWifiChip.rxmulticastpkwakedetails(), result.rxMulticastPkWakeDetails);
    Message2WifiDebugHostWakeReasonRxIcmpPacketDetails(msgWifiChip.rxicmppkwakedetails(), result.rxIcmpPkWakeDetails);
}

bool WifiChipParseGetDebugHostWakeReasonStatsCfm(const uint8_t* data, size_t length, GetDebugHostWakeReasonStatsCfmChipParam& param)
{
    GetDebugHostWakeReasonStatsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2WifiDebugHostWakeReasonStats(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetDebugRingBuffersStatusReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiDebugRingBufferStatus(const WifiDebugRingBufferStatus_P& msgWifiChip, WifiDebugRingBufferStatus& result)
{
    result.ringName = msgWifiChip.ringname();
    result.flags = msgWifiChip.flags();
    result.ringId = msgWifiChip.ringid();
    result.sizeInBytes = msgWifiChip.sizeinbytes();
    result.freeSizeInBytes = msgWifiChip.freesizeinbytes();
    result.verboseLevel = msgWifiChip.verboselevel();
}

bool WifiChipParseGetDebugRingBuffersStatusCfm(const uint8_t* data, size_t length, GetDebugRingBuffersStatusCfmChipParam& param)
{
    GetDebugRingBuffersStatusCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2WifiDebugRingBufferStatus(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmChipParam& param)
{
    GetIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool WifiChipParseGetModeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetModeCfm(const uint8_t* data, size_t length, GetModeCfmChipParam& param)
{
    GetModeCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool WifiChipParseGetNanIfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetNanIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseGetNanIfaceCfm(const uint8_t* data, size_t length, GetNanIfaceCfmChipParam& param)
{
    GetNanIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiNanIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetNanIfaceNamesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetNanIfaceNamesCfm(const uint8_t* data, size_t length, GetNanIfaceNamesCfmChipParam& param)
{
    GetNanIfaceNamesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetP2pIfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetP2pIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseGetP2pIfaceCfm(const uint8_t* data, size_t length, GetP2pIfaceCfmChipParam& param)
{
    GetP2pIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiP2pIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetP2pIfaceNamesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetP2pIfaceNamesCfm(const uint8_t* data, size_t length, GetP2pIfaceNamesCfmChipParam& param)
{
    GetP2pIfaceNamesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetStaIfaceReq(const uint8_t* data, size_t length, string& param)
{
    GetStaIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseGetStaIfaceCfm(const uint8_t* data, size_t length, GetStaIfaceCfmChipParam& param)
{
    GetStaIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetStaIfaceNamesReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseGetStaIfaceNamesCfm(const uint8_t* data, size_t length, GetStaIfaceNamesCfmChipParam& param)
{
    GetStaIfaceNamesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param.result[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetSupportedRadioCombinationsReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiRadioConfiguration(const WifiRadioConfiguration_P& msgWifiChip, WifiRadioConfiguration& radioConfigurations)
{
    radioConfigurations.bandInfo = (WifiBand) msgWifiChip.bandinfo();
    radioConfigurations.antennaMode = (WifiAntennaMode) msgWifiChip.antennamode();
}

static void Message2WifiRadioCombination(const WifiRadioCombination_P& msgWifiChip, WifiRadioCombination& result)
{
    int size_radioconfigurations = msgWifiChip.radioconfigurations_size();
    if (0 < size_radioconfigurations)
    {
        result.radioConfigurations.resize(size_radioconfigurations);
        for (int index = 0; index < size_radioconfigurations; index++)
        {
            Message2WifiRadioConfiguration(msgWifiChip.radioconfigurations(index), result.radioConfigurations[index]);
        }
    }
}

bool WifiChipParseGetSupportedRadioCombinationsCfm(const uint8_t* data, size_t length, GetSupportedRadioCombinationsCfmChipParam& param)
{
    GetSupportedRadioCombinationsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2WifiRadioCombination(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiChipParseGetWifiChipCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiChipCapabilities(const WifiChipCapabilities_P& msgWifiChip, WifiChipCapabilities& result)
{
    result.maxMloAssociationLinkCount = msgWifiChip.maxmloassociationlinkcount();
    result.maxMloStrLinkCount = msgWifiChip.maxmlostrlinkcount();
    result.maxConcurrentTdlsSessionCount = msgWifiChip.maxconcurrenttdlssessioncount();
}

bool WifiChipParseGetWifiChipCapabilitiesCfm(const uint8_t* data, size_t length, GetWifiChipCapabilitiesCfmChipParam& param)
{
    GetWifiChipCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2WifiChipCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseGetUsableChannelsReq(const uint8_t* data, size_t length, GetUsableChannelsReqChipParam& param)
{
    GetUsableChannelsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.band = (WifiBand) msg.band();
        param.ifaceModeMask = msg.ifacemodemask();
        param.filterMask = msg.filtermask();
        return true;
    }
    return false;
}

static void Message2WifiUsableChannel(const WifiUsableChannel_P& msgWifiChip, WifiUsableChannel& result)
{
    result.channel = msgWifiChip.channel();
    result.channelBandwidth = (WifiChannelWidthInMhz) msgWifiChip.channelbandwidth();
    result.ifaceModeMask = msgWifiChip.ifacemodemask();
}

bool WifiChipParseGetUsableChannelsCfm(const uint8_t* data, size_t length, GetUsableChannelsCfmChipParam& param)
{
    GetUsableChannelsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2WifiUsableChannel(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

static void Message2AvailableAfcFrequencyInfo(const AvailableAfcFrequencyInfo_P& msgWifiChip, AvailableAfcFrequencyInfo& availableAfcFrequencyInfos)
{
    availableAfcFrequencyInfos.startFrequencyMhz = msgWifiChip.startfrequencymhz();
    availableAfcFrequencyInfos.endFrequencyMhz = msgWifiChip.endfrequencymhz();
    availableAfcFrequencyInfos.maxPsd = msgWifiChip.maxpsd();
}

static void Message2AvailableAfcChannelInfo(const AvailableAfcChannelInfo_P& msgWifiChip, AvailableAfcChannelInfo& availableAfcChannelInfos)
{
    availableAfcChannelInfos.globalOperatingClass = msgWifiChip.globaloperatingclass();
    availableAfcChannelInfos.channelCfi = msgWifiChip.channelcfi();
    availableAfcChannelInfos.maxEirpDbm = msgWifiChip.maxeirpdbm();
}

static void Message2AfcChannelAllowance(const AfcChannelAllowance_P& msgWifiChip, AfcChannelAllowance& afcChannelAllowance)
{
    int size_availableafcfrequencyinfos = msgWifiChip.availableafcfrequencyinfos_size();
    if (0 < size_availableafcfrequencyinfos)
    {
        afcChannelAllowance.availableAfcFrequencyInfos.resize(size_availableafcfrequencyinfos);
        for (int index = 0; index < size_availableafcfrequencyinfos; index++)
        {
            Message2AvailableAfcFrequencyInfo(msgWifiChip.availableafcfrequencyinfos(index), afcChannelAllowance.availableAfcFrequencyInfos[index]);
        }
    }
    int size_availableafcchannelinfos = msgWifiChip.availableafcchannelinfos_size();
    if (0 < size_availableafcchannelinfos)
    {
        afcChannelAllowance.availableAfcChannelInfos.resize(size_availableafcchannelinfos);
        for (int index = 0; index < size_availableafcchannelinfos; index++)
        {
            Message2AvailableAfcChannelInfo(msgWifiChip.availableafcchannelinfos(index), afcChannelAllowance.availableAfcChannelInfos[index]);
        }
    }
    afcChannelAllowance.availabilityExpireTimeMs = msgWifiChip.availabilityexpiretimems();
}

bool WifiChipParseSetAfcChannelAllowanceReq(const uint8_t* data, size_t length, AfcChannelAllowance& param)
{
    SetAfcChannelAllowanceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2AfcChannelAllowance(msg.afcchannelallowance(), param);
        return true;
    }
    return false;
}

bool WifiChipParseSetAfcChannelAllowanceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRegisterEventCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRegisterEventCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRemoveApIfaceReq(const uint8_t* data, size_t length, string& param)
{
    RemoveApIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseRemoveApIfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceReq(const uint8_t* data, size_t length, RemoveIfaceInstanceFromBridgedApIfaceReqChipParam& param)
{
    RemoveIfaceInstanceFromBridgedApIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.brIfaceName = msg.brifacename();
        param.ifaceInstanceName = msg.ifaceinstancename();
        return true;
    }
    return false;
}

bool WifiChipParseRemoveIfaceInstanceFromBridgedApIfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRemoveNanIfaceReq(const uint8_t* data, size_t length, string& param)
{
    RemoveNanIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseRemoveNanIfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRemoveP2pIfaceReq(const uint8_t* data, size_t length, string& param)
{
    RemoveP2pIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseRemoveP2pIfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRemoveStaIfaceReq(const uint8_t* data, size_t length, string& param)
{
    RemoveStaIfaceReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseRemoveStaIfaceCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRequestChipDebugInfoReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2ChipDebugInfo(const ChipDebugInfo_P& msgWifiChip, IWifiChip::ChipDebugInfo& result)
{
    result.driverDescription = msgWifiChip.driverdescription();
    result.firmwareDescription = msgWifiChip.firmwaredescription();
}

bool WifiChipParseRequestChipDebugInfoCfm(const uint8_t* data, size_t length, RequestChipDebugInfoCfmChipParam& param)
{
    RequestChipDebugInfoCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2ChipDebugInfo(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseRequestDriverDebugDumpReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRequestDriverDebugDumpCfm(const uint8_t* data, size_t length, RequestDriverDebugDumpCfmChipParam& param)
{
    RequestDriverDebugDumpCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseRequestFirmwareDebugDumpReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseRequestFirmwareDebugDumpCfm(const uint8_t* data, size_t length, RequestFirmwareDebugDumpCfmChipParam& param)
{
    RequestFirmwareDebugDumpCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiChipParseResetTxPowerScenarioReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseResetTxPowerScenarioCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSelectTxPowerScenarioReq(const uint8_t* data, size_t length, IWifiChip::TxPowerScenario& param)
{
    SelectTxPowerScenarioReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (IWifiChip::TxPowerScenario) msg.scenario();
        return true;
    }
    return false;
}

bool WifiChipParseSelectTxPowerScenarioCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2CoexUnsafeChannel(const CoexUnsafeChannel_P& msgWifiChip, IWifiChip::CoexUnsafeChannel& unsafeChannels)
{
    unsafeChannels.band = (WifiBand) msgWifiChip.band();
    unsafeChannels.channel = msgWifiChip.channel();
    unsafeChannels.powerCapDbm = msgWifiChip.powercapdbm();
}

bool WifiChipParseSetCoexUnsafeChannelsReq(const uint8_t* data, size_t length, SetCoexUnsafeChannelsReqChipParam& param)
{
    SetCoexUnsafeChannelsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_unsafechannels = msg.unsafechannels_size();
        if (0 < size_unsafechannels)
        {
            param.unsafeChannels.resize(size_unsafechannels);
            for (int index = 0; index < size_unsafechannels; index++)
            {
                Message2CoexUnsafeChannel(msg.unsafechannels(index), param.unsafeChannels[index]);
            }
        }
        param.restrictions = msg.restrictions();
        return true;
    }
    return false;
}

bool WifiChipParseSetCoexUnsafeChannelsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSetCountryCodeReq(const uint8_t* data, size_t length, array<uint8_t, 2>& param)
{
    SetCountryCodeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Array(msg.code(), param);
        return true;
    }
    return false;
}

bool WifiChipParseSetCountryCodeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSetLatencyModeReq(const uint8_t* data, size_t length, IWifiChip::LatencyMode& param)
{
    SetLatencyModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (IWifiChip::LatencyMode) msg.mode();
        return true;
    }
    return false;
}

bool WifiChipParseSetLatencyModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSetMultiStaPrimaryConnectionReq(const uint8_t* data, size_t length, string& param)
{
    SetMultiStaPrimaryConnectionReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.ifname();
        return true;
    }
    return false;
}

bool WifiChipParseSetMultiStaPrimaryConnectionCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSetMultiStaUseCaseReq(const uint8_t* data, size_t length, IWifiChip::MultiStaUseCase& param)
{
    SetMultiStaUseCaseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (IWifiChip::MultiStaUseCase) msg.usecase();
        return true;
    }
    return false;
}

bool WifiChipParseSetMultiStaUseCaseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseStartLoggingToDebugRingBufferReq(const uint8_t* data, size_t length, StartLoggingToDebugRingBufferReqChipParam& param)
{
    StartLoggingToDebugRingBufferReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.ringName = msg.ringname();
        param.verboseLevel = (WifiDebugRingBufferVerboseLevel) msg.verboselevel();
        param.maxIntervalInSec = msg.maxintervalinsec();
        param.minDataSizeInBytes = msg.mindatasizeinbytes();
        return true;
    }
    return false;
}

bool WifiChipParseStartLoggingToDebugRingBufferCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseStopLoggingToDebugRingBufferReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseStopLoggingToDebugRingBufferCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseTriggerSubsystemRestartReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseTriggerSubsystemRestartCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseEnableStaChannelForPeerNetworkReq(const uint8_t* data, size_t length, int32_t& param)
{
    EnableStaChannelForPeerNetworkReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.channelcategoryenableflag();
        return true;
    }
    return false;
}

bool WifiChipParseEnableStaChannelForPeerNetworkCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseSetMloModeReq(const uint8_t* data, size_t length, IWifiChip::ChipMloMode& param)
{
    SetMloModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (IWifiChip::ChipMloMode) msg.mode();
        return true;
    }
    return false;
}

bool WifiChipParseSetMloModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiChipParseOnChipReconfigureFailureInd(const uint8_t* data, size_t length, WifiStatusCode& param)
{
    OnChipReconfigureFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (WifiStatusCode) msg.status();
        return true;
    }
    return false;
}

bool WifiChipParseOnChipReconfiguredInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnChipReconfiguredInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.modeid();
        return true;
    }
    return false;
}

bool WifiChipParseOnDebugErrorAlertInd(const uint8_t* data, size_t length, OnDebugErrorAlertIndChipParam& param)
{
    OnDebugErrorAlertInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.errorCode = msg.errorcode();
        String2Vector(msg.debugdata(), param.debugData);
        return true;
    }
    return false;
}

bool WifiChipParseOnDebugRingBufferDataAvailableInd(const uint8_t* data, size_t length, OnDebugRingBufferDataAvailableIndChipParam& param)
{
    OnDebugRingBufferDataAvailableInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2WifiDebugRingBufferStatus(msg.status(), param.status);
        String2Vector(msg.data(), param.data);
        return true;
    }
    return false;
}

bool WifiChipParseOnIfaceAddedInd(const uint8_t* data, size_t length, OnIfaceAddedIndChipParam& param)
{
    OnIfaceAddedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.type = (IfaceType) msg.type();
        param.name = msg.name();
        return true;
    }
    return false;
}

bool WifiChipParseOnIfaceRemovedInd(const uint8_t* data, size_t length, OnIfaceRemovedIndChipParam& param)
{
    OnIfaceRemovedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.type = (IfaceType) msg.type();
        param.name = msg.name();
        return true;
    }
    return false;
}

static void Message2IfaceInfo(const IfaceInfo_P& msgWifiChip, IWifiChipEventCallback::IfaceInfo& ifaceInfos)
{
    ifaceInfos.name = msgWifiChip.name();
    ifaceInfos.channel = msgWifiChip.channel();
}

static void Message2RadioModeInfo(const RadioModeInfo_P& msgWifiChip, IWifiChipEventCallback::RadioModeInfo& radioModeInfos)
{
    radioModeInfos.radioId = msgWifiChip.radioid();
    radioModeInfos.bandInfo = (WifiBand) msgWifiChip.bandinfo();
    int size_ifaceinfos = msgWifiChip.ifaceinfos_size();
    if (0 < size_ifaceinfos)
    {
        radioModeInfos.ifaceInfos.resize(size_ifaceinfos);
        for (int index = 0; index < size_ifaceinfos; index++)
        {
            Message2IfaceInfo(msgWifiChip.ifaceinfos(index), radioModeInfos.ifaceInfos[index]);
        }
    }
}

bool WifiChipParseOnRadioModeChangeInd(const uint8_t* data, size_t length, vector<IWifiChipEventCallback::RadioModeInfo>& param)
{
    OnRadioModeChangeInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_radiomodeinfos = msg.radiomodeinfos_size();
        if (0 < size_radiomodeinfos)
        {
            param.resize(size_radiomodeinfos);
            for (int index = 0; index < size_radiomodeinfos; index++)
            {
                Message2RadioModeInfo(msg.radiomodeinfos(index), param[index]);
            }
        }
        return true;
    }
    return false;
}
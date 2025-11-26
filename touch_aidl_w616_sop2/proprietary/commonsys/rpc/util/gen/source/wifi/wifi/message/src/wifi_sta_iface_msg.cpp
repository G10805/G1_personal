/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_sta_iface_msg.h"

#include "HalStatus.pb.h"
#include "IWifiStaIface.pb.h"
#include "IWifiStaIfaceEventCallback.pb.h"
#include "MacAddress.pb.h"
#include "Ssid.pb.h"
#include "StaApfPacketFilterCapabilities.pb.h"
#include "StaBackgroundScanBucketParameters.pb.h"
#include "StaBackgroundScanCapabilities.pb.h"
#include "StaBackgroundScanParameters.pb.h"
#include "StaLinkLayerIfaceContentionTimeStats.pb.h"
#include "StaLinkLayerIfacePacketStats.pb.h"
#include "StaLinkLayerIfaceStats.pb.h"
#include "StaLinkLayerLinkStats.pb.h"
#include "StaLinkLayerRadioStats.pb.h"
#include "StaLinkLayerStats.pb.h"
#include "StaPeerInfo.pb.h"
#include "StaRateStat.pb.h"
#include "StaRoamingCapabilities.pb.h"
#include "StaRoamingConfig.pb.h"
#include "StaRoamingState.pb.h"
#include "StaScanData.pb.h"
#include "StaScanResult.pb.h"
#include "WifiBand.pb.h"
#include "WifiChannelInfo.pb.h"
#include "WifiChannelStats.pb.h"
#include "WifiChannelWidthInMhz.pb.h"
#include "WifiDebugPacketFateFrameInfo.pb.h"
#include "WifiDebugPacketFateFrameType.pb.h"
#include "WifiDebugRxPacketFate.pb.h"
#include "WifiDebugRxPacketFateReport.pb.h"
#include "WifiDebugTxPacketFate.pb.h"
#include "WifiDebugTxPacketFateReport.pb.h"
#include "WifiInformationElement.pb.h"
#include "WifiRateInfo.pb.h"
#include "WifiRateNss.pb.h"
#include "WifiRatePreamble.pb.h"

using ConfigureRoamingReq = ::wifi::IWifiStaIface_ConfigureRoamingReq;
using EnableLinkLayerStatsCollectionReq = ::wifi::IWifiStaIface_EnableLinkLayerStatsCollectionReq;
using EnableNdOffloadReq = ::wifi::IWifiStaIface_EnableNdOffloadReq;
using GetApfPacketFilterCapabilitiesCfm = ::wifi::IWifiStaIface_GetApfPacketFilterCapabilitiesCfm;
using GetBackgroundScanCapabilitiesCfm = ::wifi::IWifiStaIface_GetBackgroundScanCapabilitiesCfm;
using GetDebugRxPacketFatesCfm = ::wifi::IWifiStaIface_GetDebugRxPacketFatesCfm;
using GetDebugTxPacketFatesCfm = ::wifi::IWifiStaIface_GetDebugTxPacketFatesCfm;
using GetFactoryMacAddressCfm = ::wifi::IWifiStaIface_GetFactoryMacAddressCfm;
using GetFeatureSetCfm = ::wifi::IWifiStaIface_GetFeatureSetCfm;
using GetLinkLayerStatsCfm = ::wifi::IWifiStaIface_GetLinkLayerStatsCfm;
using GetNameCfm = ::wifi::IWifiStaIface_GetNameCfm;
using GetRoamingCapabilitiesCfm = ::wifi::IWifiStaIface_GetRoamingCapabilitiesCfm;
using InstallApfPacketFilterReq = ::wifi::IWifiStaIface_InstallApfPacketFilterReq;
using OnBackgroundFullScanResultInd = ::wifi::IWifiStaIfaceEventCallback_OnBackgroundFullScanResultInd;
using OnBackgroundScanFailureInd = ::wifi::IWifiStaIfaceEventCallback_OnBackgroundScanFailureInd;
using OnBackgroundScanResultsInd = ::wifi::IWifiStaIfaceEventCallback_OnBackgroundScanResultsInd;
using OnRssiThresholdBreachedInd = ::wifi::IWifiStaIfaceEventCallback_OnRssiThresholdBreachedInd;
using ReadApfPacketFilterDataCfm = ::wifi::IWifiStaIface_ReadApfPacketFilterDataCfm;
using SetDtimMultiplierReq = ::wifi::IWifiStaIface_SetDtimMultiplierReq;
using SetMacAddressReq = ::wifi::IWifiStaIface_SetMacAddressReq;
using SetRoamingStateReq = ::wifi::IWifiStaIface_SetRoamingStateReq;
using SetScanModeReq = ::wifi::IWifiStaIface_SetScanModeReq;
using StartBackgroundScanReq = ::wifi::IWifiStaIface_StartBackgroundScanReq;
using StartRssiMonitoringReq = ::wifi::IWifiStaIface_StartRssiMonitoringReq;
using StartSendingKeepAlivePacketsReq = ::wifi::IWifiStaIface_StartSendingKeepAlivePacketsReq;
using StopBackgroundScanReq = ::wifi::IWifiStaIface_StopBackgroundScanReq;
using StopRssiMonitoringReq = ::wifi::IWifiStaIface_StopRssiMonitoringReq;
using StopSendingKeepAlivePacketsReq = ::wifi::IWifiStaIface_StopSendingKeepAlivePacketsReq;

using HalStatus_P = ::wifi::HalStatus;
using MacAddress_P = ::wifi::MacAddress;
using Ssid_P = ::wifi::Ssid;
using StaApfPacketFilterCapabilities_P = ::wifi::StaApfPacketFilterCapabilities;
using StaBackgroundScanBucketParameters_P = ::wifi::StaBackgroundScanBucketParameters;
using StaBackgroundScanCapabilities_P = ::wifi::StaBackgroundScanCapabilities;
using StaBackgroundScanParameters_P = ::wifi::StaBackgroundScanParameters;
using StaLinkLayerIfaceContentionTimeStats_P = ::wifi::StaLinkLayerIfaceContentionTimeStats;
using StaLinkLayerIfacePacketStats_P = ::wifi::StaLinkLayerIfacePacketStats;
using StaLinkLayerIfaceStats_P = ::wifi::StaLinkLayerIfaceStats;
using StaLinkLayerLinkStats_P = ::wifi::StaLinkLayerLinkStats;
using StaLinkLayerRadioStats_P = ::wifi::StaLinkLayerRadioStats;
using StaLinkLayerStats_P = ::wifi::StaLinkLayerStats;
using StaLinkState_P = ::wifi::StaLinkLayerLinkStats_StaLinkState;
using StaPeerInfo_P = ::wifi::StaPeerInfo;
using StaRateStat_P = ::wifi::StaRateStat;
using StaRoamingCapabilities_P = ::wifi::StaRoamingCapabilities;
using StaRoamingConfig_P = ::wifi::StaRoamingConfig;
using StaRoamingState_P = ::wifi::StaRoamingState;
using StaScanData_P = ::wifi::StaScanData;
using StaScanResult_P = ::wifi::StaScanResult;
using WifiBand_P = ::wifi::WifiBand;
using WifiChannelInfo_P = ::wifi::WifiChannelInfo;
using WifiChannelStats_P = ::wifi::WifiChannelStats;
using WifiChannelWidthInMhz_P = ::wifi::WifiChannelWidthInMhz;
using WifiDebugPacketFateFrameInfo_P = ::wifi::WifiDebugPacketFateFrameInfo;
using WifiDebugPacketFateFrameType_P = ::wifi::wdpfft::WifiDebugPacketFateFrameType;
using WifiDebugRxPacketFateReport_P = ::wifi::WifiDebugRxPacketFateReport;
using WifiDebugRxPacketFate_P = ::wifi::wdrpf::WifiDebugRxPacketFate;
using WifiDebugTxPacketFateReport_P = ::wifi::WifiDebugTxPacketFateReport;
using WifiDebugTxPacketFate_P = ::wifi::wdtpf::WifiDebugTxPacketFate;
using WifiInformationElement_P = ::wifi::WifiInformationElement;
using WifiRateInfo_P = ::wifi::WifiRateInfo;
using WifiRateNss_P = ::wifi::WifiRateNss;
using WifiRatePreamble_P = ::wifi::wrp::WifiRatePreamble;


bool WifiStaIfaceSerializeGetNameReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiStaIface)
{
    msgWifiStaIface.set_status(status.status);
    msgWifiStaIface.set_info(status.info);
}

bool WifiStaIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetNameCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiStaIface.set_result(result);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

static void MacAddress2Message(const MacAddress& bssidBlocklist, MacAddress_P& msgWifiStaIface)
{
    string data;
    Array2String(bssidBlocklist.data, data);
    msgWifiStaIface.set_data(data);
}

static void Ssid2Message(const Ssid& ssidAllowlist, Ssid_P& msgWifiStaIface)
{
    string data;
    Array2String(ssidAllowlist.data, data);
    msgWifiStaIface.set_data(data);
}

static void StaRoamingConfig2Message(const StaRoamingConfig& config, StaRoamingConfig_P& msgWifiStaIface)
{
    int size_bssidblocklist = config.bssidBlocklist.size();
    if (0 < size_bssidblocklist)
    {
        for (int index = 0; index < size_bssidblocklist; index++)
        {
            MacAddress_P *bssidBlocklist_p = msgWifiStaIface.add_bssidblocklist();
            MacAddress2Message(config.bssidBlocklist[index], *bssidBlocklist_p);
        }
    }
    int size_ssidallowlist = config.ssidAllowlist.size();
    if (0 < size_ssidallowlist)
    {
        for (int index = 0; index < size_ssidallowlist; index++)
        {
            Ssid_P *ssidAllowlist_p = msgWifiStaIface.add_ssidallowlist();
            Ssid2Message(config.ssidAllowlist[index], *ssidAllowlist_p);
        }
    }
}

bool WifiStaIfaceSerializeConfigureRoamingReq(const StaRoamingConfig& config, vector<uint8_t>& payload)
{
    ConfigureRoamingReq msgWifiStaIface;
    StaRoamingConfig_P* config_p = msgWifiStaIface.mutable_config();
    StaRoamingConfig2Message(config, *config_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeConfigureRoamingCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeDisableLinkLayerStatsCollectionReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeDisableLinkLayerStatsCollectionCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeEnableLinkLayerStatsCollectionReq(bool debug, vector<uint8_t>& payload)
{
    EnableLinkLayerStatsCollectionReq msgWifiStaIface;
    msgWifiStaIface.set_debug(debug);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeEnableLinkLayerStatsCollectionCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeEnableNdOffloadReq(bool enable, vector<uint8_t>& payload)
{
    EnableNdOffloadReq msgWifiStaIface;
    msgWifiStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeEnableNdOffloadCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void StaApfPacketFilterCapabilities2Message(const StaApfPacketFilterCapabilities& result, StaApfPacketFilterCapabilities_P& msgWifiStaIface)
{
    msgWifiStaIface.set_version(result.version);
    msgWifiStaIface.set_maxlength(result.maxLength);
}

bool WifiStaIfaceSerializeGetApfPacketFilterCapabilitiesCfm(const HalStatusParam& status, const StaApfPacketFilterCapabilities& result, vector<uint8_t>& payload)
{
    GetApfPacketFilterCapabilitiesCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    StaApfPacketFilterCapabilities_P* result_p = msgWifiStaIface.mutable_result();
    StaApfPacketFilterCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetBackgroundScanCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void StaBackgroundScanCapabilities2Message(const StaBackgroundScanCapabilities& result, StaBackgroundScanCapabilities_P& msgWifiStaIface)
{
    msgWifiStaIface.set_maxcachesize(result.maxCacheSize);
    msgWifiStaIface.set_maxbuckets(result.maxBuckets);
    msgWifiStaIface.set_maxapcacheperscan(result.maxApCachePerScan);
    msgWifiStaIface.set_maxreportingthreshold(result.maxReportingThreshold);
}

bool WifiStaIfaceSerializeGetBackgroundScanCapabilitiesCfm(const HalStatusParam& status, const StaBackgroundScanCapabilities& result, vector<uint8_t>& payload)
{
    GetBackgroundScanCapabilitiesCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    StaBackgroundScanCapabilities_P* result_p = msgWifiStaIface.mutable_result();
    StaBackgroundScanCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetFeatureSetReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeGetFeatureSetCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetFeatureSetCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    msgWifiStaIface.set_result(result);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetDebugRxPacketFatesReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiDebugPacketFateFrameInfo2Message(const WifiDebugPacketFateFrameInfo& frameInfo, WifiDebugPacketFateFrameInfo_P& msgWifiStaIface)
{
    msgWifiStaIface.set_frametype((WifiDebugPacketFateFrameType_P) frameInfo.frameType);
    msgWifiStaIface.set_framelen(frameInfo.frameLen);
    msgWifiStaIface.set_drivertimestampusec(frameInfo.driverTimestampUsec);
    msgWifiStaIface.set_firmwaretimestampusec(frameInfo.firmwareTimestampUsec);
    string frameContent;
    Vector2String(frameInfo.frameContent, frameContent);
    msgWifiStaIface.set_framecontent(frameContent);
}

static void WifiDebugRxPacketFateReport2Message(const WifiDebugRxPacketFateReport& result, WifiDebugRxPacketFateReport_P& msgWifiStaIface)
{
    msgWifiStaIface.set_fate((WifiDebugRxPacketFate_P) result.fate);
    WifiDebugPacketFateFrameInfo_P* frameInfo_p = msgWifiStaIface.mutable_frameinfo();
    WifiDebugPacketFateFrameInfo2Message(result.frameInfo, *frameInfo_p);
}

bool WifiStaIfaceSerializeGetDebugRxPacketFatesCfm(const HalStatusParam& status, const vector<WifiDebugRxPacketFateReport>& result, vector<uint8_t>& payload)
{
    GetDebugRxPacketFatesCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            WifiDebugRxPacketFateReport_P *result_p = msgWifiStaIface.add_result();
            WifiDebugRxPacketFateReport2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetDebugTxPacketFatesReq(vector<uint8_t>& payload)
{
    return true;
}

static void WifiDebugTxPacketFateReport2Message(const WifiDebugTxPacketFateReport& result, WifiDebugTxPacketFateReport_P& msgWifiStaIface)
{
    msgWifiStaIface.set_fate((WifiDebugTxPacketFate_P) result.fate);
    WifiDebugPacketFateFrameInfo_P* frameInfo_p = msgWifiStaIface.mutable_frameinfo();
    WifiDebugPacketFateFrameInfo2Message(result.frameInfo, *frameInfo_p);
}

bool WifiStaIfaceSerializeGetDebugTxPacketFatesCfm(const HalStatusParam& status, const vector<WifiDebugTxPacketFateReport>& result, vector<uint8_t>& payload)
{
    GetDebugTxPacketFatesCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            WifiDebugTxPacketFateReport_P *result_p = msgWifiStaIface.add_result();
            WifiDebugTxPacketFateReport2Message(result[index], *result_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetFactoryMacAddressReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeGetFactoryMacAddressCfm(const HalStatusParam& status, const array<uint8_t, 6>& result, vector<uint8_t>& payload)
{
    GetFactoryMacAddressCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Array2String(result, resultStr);
    msgWifiStaIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetLinkLayerStatsReq(vector<uint8_t>& payload)
{
    return true;
}

static void StaLinkLayerIfacePacketStats2Message(const StaLinkLayerIfacePacketStats& wmeBePktStats, StaLinkLayerIfacePacketStats_P& msgWifiStaIface)
{
    msgWifiStaIface.set_rxmpdu(wmeBePktStats.rxMpdu);
    msgWifiStaIface.set_txmpdu(wmeBePktStats.txMpdu);
    msgWifiStaIface.set_lostmpdu(wmeBePktStats.lostMpdu);
    msgWifiStaIface.set_retries(wmeBePktStats.retries);
}

static void StaLinkLayerIfaceContentionTimeStats2Message(const StaLinkLayerIfaceContentionTimeStats& wmeBeContentionTimeStats, StaLinkLayerIfaceContentionTimeStats_P& msgWifiStaIface)
{
    msgWifiStaIface.set_contentiontimemininusec(wmeBeContentionTimeStats.contentionTimeMinInUsec);
    msgWifiStaIface.set_contentiontimemaxinusec(wmeBeContentionTimeStats.contentionTimeMaxInUsec);
    msgWifiStaIface.set_contentiontimeavginusec(wmeBeContentionTimeStats.contentionTimeAvgInUsec);
    msgWifiStaIface.set_contentionnumsamples(wmeBeContentionTimeStats.contentionNumSamples);
}

static void WifiRateInfo2Message(const WifiRateInfo& rateInfo, WifiRateInfo_P& msgWifiStaIface)
{
    msgWifiStaIface.set_preamble((WifiRatePreamble_P) rateInfo.preamble);
    msgWifiStaIface.set_nss((WifiRateNss_P) rateInfo.nss);
    msgWifiStaIface.set_bw((WifiChannelWidthInMhz_P) rateInfo.bw);
    msgWifiStaIface.set_ratemcsidx(rateInfo.rateMcsIdx);
    msgWifiStaIface.set_bitrateinkbps(rateInfo.bitRateInKbps);
}

static void StaRateStat2Message(const StaRateStat& rateStats, StaRateStat_P& msgWifiStaIface)
{
    WifiRateInfo_P* rateInfo_p = msgWifiStaIface.mutable_rateinfo();
    WifiRateInfo2Message(rateStats.rateInfo, *rateInfo_p);
    msgWifiStaIface.set_txmpdu(rateStats.txMpdu);
    msgWifiStaIface.set_rxmpdu(rateStats.rxMpdu);
    msgWifiStaIface.set_mpdulost(rateStats.mpduLost);
    msgWifiStaIface.set_retries(rateStats.retries);
}

static void StaPeerInfo2Message(const StaPeerInfo& peers, StaPeerInfo_P& msgWifiStaIface)
{
    msgWifiStaIface.set_stacount(peers.staCount);
    msgWifiStaIface.set_chanutil(peers.chanUtil);
    int size_ratestats = peers.rateStats.size();
    if (0 < size_ratestats)
    {
        for (int index = 0; index < size_ratestats; index++)
        {
            StaRateStat_P *rateStats_p = msgWifiStaIface.add_ratestats();
            StaRateStat2Message(peers.rateStats[index], *rateStats_p);
        }
    }
}

static void StaLinkLayerLinkStats2Message(const StaLinkLayerLinkStats& links, StaLinkLayerLinkStats_P& msgWifiStaIface)
{
    msgWifiStaIface.set_linkid(links.linkId);
    msgWifiStaIface.set_radioid(links.radioId);
    msgWifiStaIface.set_frequencymhz(links.frequencyMhz);
    msgWifiStaIface.set_beaconrx(links.beaconRx);
    msgWifiStaIface.set_avgrssimgmt(links.avgRssiMgmt);
    StaLinkLayerIfacePacketStats_P* wmeBePktStats_p = msgWifiStaIface.mutable_wmebepktstats();
    StaLinkLayerIfacePacketStats2Message(links.wmeBePktStats, *wmeBePktStats_p);
    StaLinkLayerIfacePacketStats_P* wmeBkPktStats_p = msgWifiStaIface.mutable_wmebkpktstats();
    StaLinkLayerIfacePacketStats2Message(links.wmeBkPktStats, *wmeBkPktStats_p);
    StaLinkLayerIfacePacketStats_P* wmeViPktStats_p = msgWifiStaIface.mutable_wmevipktstats();
    StaLinkLayerIfacePacketStats2Message(links.wmeViPktStats, *wmeViPktStats_p);
    StaLinkLayerIfacePacketStats_P* wmeVoPktStats_p = msgWifiStaIface.mutable_wmevopktstats();
    StaLinkLayerIfacePacketStats2Message(links.wmeVoPktStats, *wmeVoPktStats_p);
    msgWifiStaIface.set_timeslicedutycycleinpercent(links.timeSliceDutyCycleInPercent);
    StaLinkLayerIfaceContentionTimeStats_P* wmeBeContentionTimeStats_p = msgWifiStaIface.mutable_wmebecontentiontimestats();
    StaLinkLayerIfaceContentionTimeStats2Message(links.wmeBeContentionTimeStats, *wmeBeContentionTimeStats_p);
    StaLinkLayerIfaceContentionTimeStats_P* wmeBkContentionTimeStats_p = msgWifiStaIface.mutable_wmebkcontentiontimestats();
    StaLinkLayerIfaceContentionTimeStats2Message(links.wmeBkContentionTimeStats, *wmeBkContentionTimeStats_p);
    StaLinkLayerIfaceContentionTimeStats_P* wmeViContentionTimeStats_p = msgWifiStaIface.mutable_wmevicontentiontimestats();
    StaLinkLayerIfaceContentionTimeStats2Message(links.wmeViContentionTimeStats, *wmeViContentionTimeStats_p);
    StaLinkLayerIfaceContentionTimeStats_P* wmeVoContentionTimeStats_p = msgWifiStaIface.mutable_wmevocontentiontimestats();
    StaLinkLayerIfaceContentionTimeStats2Message(links.wmeVoContentionTimeStats, *wmeVoContentionTimeStats_p);
    int size_peers = links.peers.size();
    if (0 < size_peers)
    {
        for (int index = 0; index < size_peers; index++)
        {
            StaPeerInfo_P *peers_p = msgWifiStaIface.add_peers();
            StaPeerInfo2Message(links.peers[index], *peers_p);
        }
    }
    msgWifiStaIface.set_state((StaLinkState_P) links.state);
}

static void StaLinkLayerIfaceStats2Message(const StaLinkLayerIfaceStats& iface, StaLinkLayerIfaceStats_P& msgWifiStaIface)
{
    int size_links = iface.links.size();
    if (0 < size_links)
    {
        for (int index = 0; index < size_links; index++)
        {
            StaLinkLayerLinkStats_P *links_p = msgWifiStaIface.add_links();
            StaLinkLayerLinkStats2Message(iface.links[index], *links_p);
        }
    }
}

static void WifiChannelInfo2Message(const WifiChannelInfo& channel, WifiChannelInfo_P& msgWifiStaIface)
{
    msgWifiStaIface.set_width((WifiChannelWidthInMhz_P) channel.width);
    msgWifiStaIface.set_centerfreq(channel.centerFreq);
    msgWifiStaIface.set_centerfreq0(channel.centerFreq0);
    msgWifiStaIface.set_centerfreq1(channel.centerFreq1);
}

static void WifiChannelStats2Message(const WifiChannelStats& channelStats, WifiChannelStats_P& msgWifiStaIface)
{
    WifiChannelInfo_P* channel_p = msgWifiStaIface.mutable_channel();
    WifiChannelInfo2Message(channelStats.channel, *channel_p);
    msgWifiStaIface.set_ontimeinms(channelStats.onTimeInMs);
    msgWifiStaIface.set_ccabusytimeinms(channelStats.ccaBusyTimeInMs);
}

static void StaLinkLayerRadioStats2Message(const StaLinkLayerRadioStats& radios, StaLinkLayerRadioStats_P& msgWifiStaIface)
{
    msgWifiStaIface.set_ontimeinms(radios.onTimeInMs);
    msgWifiStaIface.set_txtimeinms(radios.txTimeInMs);
    int size_txtimeinmsperlevel = radios.txTimeInMsPerLevel.size();
    if (0 < size_txtimeinmsperlevel)
    {
        for (int index = 0; index < size_txtimeinmsperlevel; index++)
        {
            msgWifiStaIface.add_txtimeinmsperlevel(radios.txTimeInMsPerLevel[index]);
        }
    }
    msgWifiStaIface.set_rxtimeinms(radios.rxTimeInMs);
    msgWifiStaIface.set_ontimeinmsforscan(radios.onTimeInMsForScan);
    msgWifiStaIface.set_ontimeinmsfornanscan(radios.onTimeInMsForNanScan);
    msgWifiStaIface.set_ontimeinmsforbgscan(radios.onTimeInMsForBgScan);
    msgWifiStaIface.set_ontimeinmsforroamscan(radios.onTimeInMsForRoamScan);
    msgWifiStaIface.set_ontimeinmsforpnoscan(radios.onTimeInMsForPnoScan);
    msgWifiStaIface.set_ontimeinmsforhs20scan(radios.onTimeInMsForHs20Scan);
    int size_channelstats = radios.channelStats.size();
    if (0 < size_channelstats)
    {
        for (int index = 0; index < size_channelstats; index++)
        {
            WifiChannelStats_P *channelStats_p = msgWifiStaIface.add_channelstats();
            WifiChannelStats2Message(radios.channelStats[index], *channelStats_p);
        }
    }
    msgWifiStaIface.set_radioid(radios.radioId);
}

static void StaLinkLayerStats2Message(const StaLinkLayerStats& result, StaLinkLayerStats_P& msgWifiStaIface)
{
    StaLinkLayerIfaceStats_P* iface_p = msgWifiStaIface.mutable_iface();
    StaLinkLayerIfaceStats2Message(result.iface, *iface_p);
    int size_radios = result.radios.size();
    if (0 < size_radios)
    {
        for (int index = 0; index < size_radios; index++)
        {
            StaLinkLayerRadioStats_P *radios_p = msgWifiStaIface.add_radios();
            StaLinkLayerRadioStats2Message(result.radios[index], *radios_p);
        }
    }
    msgWifiStaIface.set_timestampinms(result.timeStampInMs);
}

bool WifiStaIfaceSerializeGetLinkLayerStatsCfm(const HalStatusParam& status, const StaLinkLayerStats& result, vector<uint8_t>& payload)
{
    GetLinkLayerStatsCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    StaLinkLayerStats_P* result_p = msgWifiStaIface.mutable_result();
    StaLinkLayerStats2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeGetRoamingCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void StaRoamingCapabilities2Message(const StaRoamingCapabilities& result, StaRoamingCapabilities_P& msgWifiStaIface)
{
    msgWifiStaIface.set_maxblocklistsize(result.maxBlocklistSize);
    msgWifiStaIface.set_maxallowlistsize(result.maxAllowlistSize);
}

bool WifiStaIfaceSerializeGetRoamingCapabilitiesCfm(const HalStatusParam& status, const StaRoamingCapabilities& result, vector<uint8_t>& payload)
{
    GetRoamingCapabilitiesCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    StaRoamingCapabilities_P* result_p = msgWifiStaIface.mutable_result();
    StaRoamingCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeInstallApfPacketFilterReq(const vector<uint8_t>& program, vector<uint8_t>& payload)
{
    InstallApfPacketFilterReq msgWifiStaIface;
    string programStr;
    Vector2String(program, programStr);
    msgWifiStaIface.set_program(programStr);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeInstallApfPacketFilterCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeReadApfPacketFilterDataReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeReadApfPacketFilterDataCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    ReadApfPacketFilterDataCfm msgWifiStaIface;
    HalStatus_P* status_p = msgWifiStaIface.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgWifiStaIface.set_result(resultStr);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeRegisterEventCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeSetMacAddressReq(const array<uint8_t, 6>& mac, vector<uint8_t>& payload)
{
    SetMacAddressReq msgWifiStaIface;
    string macStr;
    Array2String(mac, macStr);
    msgWifiStaIface.set_mac(macStr);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeSetMacAddressCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeSetRoamingStateReq(StaRoamingState state, vector<uint8_t>& payload)
{
    SetRoamingStateReq msgWifiStaIface;
    msgWifiStaIface.set_state((StaRoamingState_P) state);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeSetRoamingStateCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeSetScanModeReq(bool enable, vector<uint8_t>& payload)
{
    SetScanModeReq msgWifiStaIface;
    msgWifiStaIface.set_enable(enable);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeSetScanModeCfm(vector<uint8_t>& payload)
{
    return true;
}

static void StaBackgroundScanBucketParameters2Message(const StaBackgroundScanBucketParameters& buckets, StaBackgroundScanBucketParameters_P& msgWifiStaIface)
{
    msgWifiStaIface.set_bucketidx(buckets.bucketIdx);
    msgWifiStaIface.set_band((WifiBand_P) buckets.band);
    int size_frequencies = buckets.frequencies.size();
    if (0 < size_frequencies)
    {
        for (int index = 0; index < size_frequencies; index++)
        {
            msgWifiStaIface.add_frequencies(buckets.frequencies[index]);
        }
    }
    msgWifiStaIface.set_periodinms(buckets.periodInMs);
    msgWifiStaIface.set_eventreportscheme(buckets.eventReportScheme);
    msgWifiStaIface.set_exponentialmaxperiodinms(buckets.exponentialMaxPeriodInMs);
    msgWifiStaIface.set_exponentialbase(buckets.exponentialBase);
    msgWifiStaIface.set_exponentialstepcount(buckets.exponentialStepCount);
}

static void StaBackgroundScanParameters2Message(const StaBackgroundScanParameters& params, StaBackgroundScanParameters_P& msgWifiStaIface)
{
    msgWifiStaIface.set_baseperiodinms(params.basePeriodInMs);
    msgWifiStaIface.set_maxapperscan(params.maxApPerScan);
    msgWifiStaIface.set_reportthresholdpercent(params.reportThresholdPercent);
    msgWifiStaIface.set_reportthresholdnumscans(params.reportThresholdNumScans);
    int size_buckets = params.buckets.size();
    if (0 < size_buckets)
    {
        for (int index = 0; index < size_buckets; index++)
        {
            StaBackgroundScanBucketParameters_P *buckets_p = msgWifiStaIface.add_buckets();
            StaBackgroundScanBucketParameters2Message(params.buckets[index], *buckets_p);
        }
    }
}

bool WifiStaIfaceSerializeStartBackgroundScanReq(int32_t cmdId, const StaBackgroundScanParameters& params, vector<uint8_t>& payload)
{
    StartBackgroundScanReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    StaBackgroundScanParameters_P* params_p = msgWifiStaIface.mutable_params();
    StaBackgroundScanParameters2Message(params, *params_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStartBackgroundScanCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStartDebugPacketFateMonitoringReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStartDebugPacketFateMonitoringCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStartRssiMonitoringReq(int32_t cmdId, int32_t maxRssi, int32_t minRssi, vector<uint8_t>& payload)
{
    StartRssiMonitoringReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    msgWifiStaIface.set_maxrssi(maxRssi);
    msgWifiStaIface.set_minrssi(minRssi);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStartRssiMonitoringCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStartSendingKeepAlivePacketsReq(int32_t cmdId, const vector<uint8_t>& ipPacketData, int32_t etherType, const array<uint8_t, 6>& srcAddress, const array<uint8_t, 6>& dstAddress, int32_t periodInMs, vector<uint8_t>& payload)
{
    StartSendingKeepAlivePacketsReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    string ipPacketDataStr;
    Vector2String(ipPacketData, ipPacketDataStr);
    msgWifiStaIface.set_ippacketdata(ipPacketDataStr);
    msgWifiStaIface.set_ethertype(etherType);
    string srcAddressStr;
    Array2String(srcAddress, srcAddressStr);
    msgWifiStaIface.set_srcaddress(srcAddressStr);
    string dstAddressStr;
    Array2String(dstAddress, dstAddressStr);
    msgWifiStaIface.set_dstaddress(dstAddressStr);
    msgWifiStaIface.set_periodinms(periodInMs);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStartSendingKeepAlivePacketsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStopBackgroundScanReq(int32_t cmdId, vector<uint8_t>& payload)
{
    StopBackgroundScanReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStopBackgroundScanCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStopRssiMonitoringReq(int32_t cmdId, vector<uint8_t>& payload)
{
    StopRssiMonitoringReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStopRssiMonitoringCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeStopSendingKeepAlivePacketsReq(int32_t cmdId, vector<uint8_t>& payload)
{
    StopSendingKeepAlivePacketsReq msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeStopSendingKeepAlivePacketsCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiStaIfaceSerializeSetDtimMultiplierReq(int32_t multiplier, vector<uint8_t>& payload)
{
    SetDtimMultiplierReq msgWifiStaIface;
    msgWifiStaIface.set_multiplier(multiplier);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeSetDtimMultiplierCfm(vector<uint8_t>& payload)
{
    return true;
}

static void WifiInformationElement2Message(const WifiInformationElement& informationElements, WifiInformationElement_P& msgWifiStaIface)
{
    msgWifiStaIface.set_id(informationElements.id);
    string data;
    Vector2String(informationElements.data, data);
    msgWifiStaIface.set_data(data);
}

static void StaScanResult2Message(const StaScanResult& result, StaScanResult_P& msgWifiStaIface)
{
    msgWifiStaIface.set_timestampinus(result.timeStampInUs);
    string ssid;
    Vector2String(result.ssid, ssid);
    msgWifiStaIface.set_ssid(ssid);
    string bssid;
    Array2String(result.bssid, bssid);
    msgWifiStaIface.set_bssid(bssid);
    msgWifiStaIface.set_rssi(result.rssi);
    msgWifiStaIface.set_frequency(result.frequency);
    msgWifiStaIface.set_beaconperiodinms(result.beaconPeriodInMs);
    msgWifiStaIface.set_capability(result.capability);
    int size_informationelements = result.informationElements.size();
    if (0 < size_informationelements)
    {
        for (int index = 0; index < size_informationelements; index++)
        {
            WifiInformationElement_P *informationElements_p = msgWifiStaIface.add_informationelements();
            WifiInformationElement2Message(result.informationElements[index], *informationElements_p);
        }
    }
}

bool WifiStaIfaceSerializeOnBackgroundFullScanResultInd(int32_t cmdId, int32_t bucketsScanned, const StaScanResult& result, vector<uint8_t>& payload)
{
    OnBackgroundFullScanResultInd msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    msgWifiStaIface.set_bucketsscanned(bucketsScanned);
    StaScanResult_P* result_p = msgWifiStaIface.mutable_result();
    StaScanResult2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeOnBackgroundScanFailureInd(int32_t cmdId, vector<uint8_t>& payload)
{
    OnBackgroundScanFailureInd msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

static void StaScanData2Message(const StaScanData& scanDatas, StaScanData_P& msgWifiStaIface)
{
    msgWifiStaIface.set_flags(scanDatas.flags);
    msgWifiStaIface.set_bucketsscanned(scanDatas.bucketsScanned);
    int size_results = scanDatas.results.size();
    if (0 < size_results)
    {
        for (int index = 0; index < size_results; index++)
        {
            StaScanResult_P *results_p = msgWifiStaIface.add_results();
            StaScanResult2Message(scanDatas.results[index], *results_p);
        }
    }
}

bool WifiStaIfaceSerializeOnBackgroundScanResultsInd(int32_t cmdId, const vector<StaScanData>& scanDatas, vector<uint8_t>& payload)
{
    OnBackgroundScanResultsInd msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    int size_scandatas = scanDatas.size();
    if (0 < size_scandatas)
    {
        for (int index = 0; index < size_scandatas; index++)
        {
            StaScanData_P *scanDatas_p = msgWifiStaIface.add_scandatas();
            StaScanData2Message(scanDatas[index], *scanDatas_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceSerializeOnRssiThresholdBreachedInd(int32_t cmdId, const array<uint8_t, 6>& currBssid, int32_t currRssi, vector<uint8_t>& payload)
{
    OnRssiThresholdBreachedInd msgWifiStaIface;
    msgWifiStaIface.set_cmdid(cmdId);
    string currBssidStr;
    Array2String(currBssid, currBssidStr);
    msgWifiStaIface.set_currbssid(currBssidStr);
    msgWifiStaIface.set_currrssi(currRssi);
    return ProtoMessageSerialize(&msgWifiStaIface, payload);
}

bool WifiStaIfaceParseGetNameReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgWifiStaIface, HalStatusParam& status)
{
    status.status = msgWifiStaIface.status();
    status.info = msgWifiStaIface.info();
}

bool WifiStaIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmStaIfaceParam& param)
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

static void Message2MacAddress(const MacAddress_P& msgWifiStaIface, MacAddress& bssidBlocklist)
{
    String2Array(msgWifiStaIface.data(), bssidBlocklist.data);
}

static void Message2Ssid(const Ssid_P& msgWifiStaIface, Ssid& ssidAllowlist)
{
    String2Array(msgWifiStaIface.data(), ssidAllowlist.data);
}

static void Message2StaRoamingConfig(const StaRoamingConfig_P& msgWifiStaIface, StaRoamingConfig& config)
{
    int size_bssidblocklist = msgWifiStaIface.bssidblocklist_size();
    if (0 < size_bssidblocklist)
    {
        config.bssidBlocklist.resize(size_bssidblocklist);
        for (int index = 0; index < size_bssidblocklist; index++)
        {
            Message2MacAddress(msgWifiStaIface.bssidblocklist(index), config.bssidBlocklist[index]);
        }
    }
    int size_ssidallowlist = msgWifiStaIface.ssidallowlist_size();
    if (0 < size_ssidallowlist)
    {
        config.ssidAllowlist.resize(size_ssidallowlist);
        for (int index = 0; index < size_ssidallowlist; index++)
        {
            Message2Ssid(msgWifiStaIface.ssidallowlist(index), config.ssidAllowlist[index]);
        }
    }
}

bool WifiStaIfaceParseConfigureRoamingReq(const uint8_t* data, size_t length, StaRoamingConfig& param)
{
    ConfigureRoamingReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2StaRoamingConfig(msg.config(), param);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseConfigureRoamingCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseDisableLinkLayerStatsCollectionReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseDisableLinkLayerStatsCollectionCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseEnableLinkLayerStatsCollectionReq(const uint8_t* data, size_t length, bool& param)
{
    EnableLinkLayerStatsCollectionReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.debug();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseEnableLinkLayerStatsCollectionCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseEnableNdOffloadReq(const uint8_t* data, size_t length, bool& param)
{
    EnableNdOffloadReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseEnableNdOffloadCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseGetApfPacketFilterCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2StaApfPacketFilterCapabilities(const StaApfPacketFilterCapabilities_P& msgWifiStaIface, StaApfPacketFilterCapabilities& result)
{
    result.version = msgWifiStaIface.version();
    result.maxLength = msgWifiStaIface.maxlength();
}

bool WifiStaIfaceParseGetApfPacketFilterCapabilitiesCfm(const uint8_t* data, size_t length, GetApfPacketFilterCapabilitiesCfmStaIfaceParam& param)
{
    GetApfPacketFilterCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2StaApfPacketFilterCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetBackgroundScanCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2StaBackgroundScanCapabilities(const StaBackgroundScanCapabilities_P& msgWifiStaIface, StaBackgroundScanCapabilities& result)
{
    result.maxCacheSize = msgWifiStaIface.maxcachesize();
    result.maxBuckets = msgWifiStaIface.maxbuckets();
    result.maxApCachePerScan = msgWifiStaIface.maxapcacheperscan();
    result.maxReportingThreshold = msgWifiStaIface.maxreportingthreshold();
}

bool WifiStaIfaceParseGetBackgroundScanCapabilitiesCfm(const uint8_t* data, size_t length, GetBackgroundScanCapabilitiesCfmStaIfaceParam& param)
{
    GetBackgroundScanCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2StaBackgroundScanCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetFeatureSetReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseGetFeatureSetCfm(const uint8_t* data, size_t length, GetFeatureSetCfmStaIfaceParam& param)
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

bool WifiStaIfaceParseGetDebugRxPacketFatesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiDebugPacketFateFrameInfo(const WifiDebugPacketFateFrameInfo_P& msgWifiStaIface, WifiDebugPacketFateFrameInfo& frameInfo)
{
    frameInfo.frameType = (WifiDebugPacketFateFrameType) msgWifiStaIface.frametype();
    frameInfo.frameLen = msgWifiStaIface.framelen();
    frameInfo.driverTimestampUsec = msgWifiStaIface.drivertimestampusec();
    frameInfo.firmwareTimestampUsec = msgWifiStaIface.firmwaretimestampusec();
    String2Vector(msgWifiStaIface.framecontent(), frameInfo.frameContent);
}

static void Message2WifiDebugRxPacketFateReport(const WifiDebugRxPacketFateReport_P& msgWifiStaIface, WifiDebugRxPacketFateReport& result)
{
    result.fate = (WifiDebugRxPacketFate) msgWifiStaIface.fate();
    Message2WifiDebugPacketFateFrameInfo(msgWifiStaIface.frameinfo(), result.frameInfo);
}

bool WifiStaIfaceParseGetDebugRxPacketFatesCfm(const uint8_t* data, size_t length, GetDebugRxPacketFatesCfmStaIfaceParam& param)
{
    GetDebugRxPacketFatesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2WifiDebugRxPacketFateReport(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetDebugTxPacketFatesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiDebugTxPacketFateReport(const WifiDebugTxPacketFateReport_P& msgWifiStaIface, WifiDebugTxPacketFateReport& result)
{
    result.fate = (WifiDebugTxPacketFate) msgWifiStaIface.fate();
    Message2WifiDebugPacketFateFrameInfo(msgWifiStaIface.frameinfo(), result.frameInfo);
}

bool WifiStaIfaceParseGetDebugTxPacketFatesCfm(const uint8_t* data, size_t length, GetDebugTxPacketFatesCfmStaIfaceParam& param)
{
    GetDebugTxPacketFatesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.result.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                Message2WifiDebugTxPacketFateReport(msg.result(index), param.result[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetFactoryMacAddressReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseGetFactoryMacAddressCfm(const uint8_t* data, size_t length, GetFactoryMacAddressCfmStaIfaceParam& param)
{
    GetFactoryMacAddressCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Array(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetLinkLayerStatsReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2StaLinkLayerIfacePacketStats(const StaLinkLayerIfacePacketStats_P& msgWifiStaIface, StaLinkLayerIfacePacketStats& wmeBePktStats)
{
    wmeBePktStats.rxMpdu = msgWifiStaIface.rxmpdu();
    wmeBePktStats.txMpdu = msgWifiStaIface.txmpdu();
    wmeBePktStats.lostMpdu = msgWifiStaIface.lostmpdu();
    wmeBePktStats.retries = msgWifiStaIface.retries();
}

static void Message2StaLinkLayerIfaceContentionTimeStats(const StaLinkLayerIfaceContentionTimeStats_P& msgWifiStaIface, StaLinkLayerIfaceContentionTimeStats& wmeBeContentionTimeStats)
{
    wmeBeContentionTimeStats.contentionTimeMinInUsec = msgWifiStaIface.contentiontimemininusec();
    wmeBeContentionTimeStats.contentionTimeMaxInUsec = msgWifiStaIface.contentiontimemaxinusec();
    wmeBeContentionTimeStats.contentionTimeAvgInUsec = msgWifiStaIface.contentiontimeavginusec();
    wmeBeContentionTimeStats.contentionNumSamples = msgWifiStaIface.contentionnumsamples();
}

static void Message2WifiRateInfo(const WifiRateInfo_P& msgWifiStaIface, WifiRateInfo& rateInfo)
{
    rateInfo.preamble = (WifiRatePreamble) msgWifiStaIface.preamble();
    rateInfo.nss = (WifiRateNss) msgWifiStaIface.nss();
    rateInfo.bw = (WifiChannelWidthInMhz) msgWifiStaIface.bw();
    rateInfo.rateMcsIdx = msgWifiStaIface.ratemcsidx();
    rateInfo.bitRateInKbps = msgWifiStaIface.bitrateinkbps();
}

static void Message2StaRateStat(const StaRateStat_P& msgWifiStaIface, StaRateStat& rateStats)
{
    Message2WifiRateInfo(msgWifiStaIface.rateinfo(), rateStats.rateInfo);
    rateStats.txMpdu = msgWifiStaIface.txmpdu();
    rateStats.rxMpdu = msgWifiStaIface.rxmpdu();
    rateStats.mpduLost = msgWifiStaIface.mpdulost();
    rateStats.retries = msgWifiStaIface.retries();
}

static void Message2StaPeerInfo(const StaPeerInfo_P& msgWifiStaIface, StaPeerInfo& peers)
{
    peers.staCount = msgWifiStaIface.stacount();
    peers.chanUtil = msgWifiStaIface.chanutil();
    int size_ratestats = msgWifiStaIface.ratestats_size();
    if (0 < size_ratestats)
    {
        peers.rateStats.resize(size_ratestats);
        for (int index = 0; index < size_ratestats; index++)
        {
            Message2StaRateStat(msgWifiStaIface.ratestats(index), peers.rateStats[index]);
        }
    }
}

static void Message2StaLinkLayerLinkStats(const StaLinkLayerLinkStats_P& msgWifiStaIface, StaLinkLayerLinkStats& links)
{
    links.linkId = msgWifiStaIface.linkid();
    links.radioId = msgWifiStaIface.radioid();
    links.frequencyMhz = msgWifiStaIface.frequencymhz();
    links.beaconRx = msgWifiStaIface.beaconrx();
    links.avgRssiMgmt = msgWifiStaIface.avgrssimgmt();
    Message2StaLinkLayerIfacePacketStats(msgWifiStaIface.wmebepktstats(), links.wmeBePktStats);
    Message2StaLinkLayerIfacePacketStats(msgWifiStaIface.wmebkpktstats(), links.wmeBkPktStats);
    Message2StaLinkLayerIfacePacketStats(msgWifiStaIface.wmevipktstats(), links.wmeViPktStats);
    Message2StaLinkLayerIfacePacketStats(msgWifiStaIface.wmevopktstats(), links.wmeVoPktStats);
    links.timeSliceDutyCycleInPercent = msgWifiStaIface.timeslicedutycycleinpercent();
    Message2StaLinkLayerIfaceContentionTimeStats(msgWifiStaIface.wmebecontentiontimestats(), links.wmeBeContentionTimeStats);
    Message2StaLinkLayerIfaceContentionTimeStats(msgWifiStaIface.wmebkcontentiontimestats(), links.wmeBkContentionTimeStats);
    Message2StaLinkLayerIfaceContentionTimeStats(msgWifiStaIface.wmevicontentiontimestats(), links.wmeViContentionTimeStats);
    Message2StaLinkLayerIfaceContentionTimeStats(msgWifiStaIface.wmevocontentiontimestats(), links.wmeVoContentionTimeStats);
    int size_peers = msgWifiStaIface.peers_size();
    if (0 < size_peers)
    {
        links.peers.resize(size_peers);
        for (int index = 0; index < size_peers; index++)
        {
            Message2StaPeerInfo(msgWifiStaIface.peers(index), links.peers[index]);
        }
    }
    links.state = (StaLinkLayerLinkStats::StaLinkState) msgWifiStaIface.state();
}

static void Message2StaLinkLayerIfaceStats(const StaLinkLayerIfaceStats_P& msgWifiStaIface, StaLinkLayerIfaceStats& iface)
{
    int size_links = msgWifiStaIface.links_size();
    if (0 < size_links)
    {
        iface.links.resize(size_links);
        for (int index = 0; index < size_links; index++)
        {
            Message2StaLinkLayerLinkStats(msgWifiStaIface.links(index), iface.links[index]);
        }
    }
}

static void Message2WifiChannelInfo(const WifiChannelInfo_P& msgWifiStaIface, WifiChannelInfo& channel)
{
    channel.width = (WifiChannelWidthInMhz) msgWifiStaIface.width();
    channel.centerFreq = msgWifiStaIface.centerfreq();
    channel.centerFreq0 = msgWifiStaIface.centerfreq0();
    channel.centerFreq1 = msgWifiStaIface.centerfreq1();
}

static void Message2WifiChannelStats(const WifiChannelStats_P& msgWifiStaIface, WifiChannelStats& channelStats)
{
    Message2WifiChannelInfo(msgWifiStaIface.channel(), channelStats.channel);
    channelStats.onTimeInMs = msgWifiStaIface.ontimeinms();
    channelStats.ccaBusyTimeInMs = msgWifiStaIface.ccabusytimeinms();
}

static void Message2StaLinkLayerRadioStats(const StaLinkLayerRadioStats_P& msgWifiStaIface, StaLinkLayerRadioStats& radios)
{
    radios.onTimeInMs = msgWifiStaIface.ontimeinms();
    radios.txTimeInMs = msgWifiStaIface.txtimeinms();
    int size_txtimeinmsperlevel = msgWifiStaIface.txtimeinmsperlevel_size();
    if (0 < size_txtimeinmsperlevel)
    {
        radios.txTimeInMsPerLevel.resize(size_txtimeinmsperlevel);
        for (int index = 0; index < size_txtimeinmsperlevel; index++)
        {
            radios.txTimeInMsPerLevel[index] = msgWifiStaIface.txtimeinmsperlevel(index);
        }
    }
    radios.rxTimeInMs = msgWifiStaIface.rxtimeinms();
    radios.onTimeInMsForScan = msgWifiStaIface.ontimeinmsforscan();
    radios.onTimeInMsForNanScan = msgWifiStaIface.ontimeinmsfornanscan();
    radios.onTimeInMsForBgScan = msgWifiStaIface.ontimeinmsforbgscan();
    radios.onTimeInMsForRoamScan = msgWifiStaIface.ontimeinmsforroamscan();
    radios.onTimeInMsForPnoScan = msgWifiStaIface.ontimeinmsforpnoscan();
    radios.onTimeInMsForHs20Scan = msgWifiStaIface.ontimeinmsforhs20scan();
    int size_channelstats = msgWifiStaIface.channelstats_size();
    if (0 < size_channelstats)
    {
        radios.channelStats.resize(size_channelstats);
        for (int index = 0; index < size_channelstats; index++)
        {
            Message2WifiChannelStats(msgWifiStaIface.channelstats(index), radios.channelStats[index]);
        }
    }
    radios.radioId = msgWifiStaIface.radioid();
}

static void Message2StaLinkLayerStats(const StaLinkLayerStats_P& msgWifiStaIface, StaLinkLayerStats& result)
{
    Message2StaLinkLayerIfaceStats(msgWifiStaIface.iface(), result.iface);
    int size_radios = msgWifiStaIface.radios_size();
    if (0 < size_radios)
    {
        result.radios.resize(size_radios);
        for (int index = 0; index < size_radios; index++)
        {
            Message2StaLinkLayerRadioStats(msgWifiStaIface.radios(index), result.radios[index]);
        }
    }
    result.timeStampInMs = msgWifiStaIface.timestampinms();
}

bool WifiStaIfaceParseGetLinkLayerStatsCfm(const uint8_t* data, size_t length, GetLinkLayerStatsCfmStaIfaceParam& param)
{
    GetLinkLayerStatsCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2StaLinkLayerStats(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseGetRoamingCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2StaRoamingCapabilities(const StaRoamingCapabilities_P& msgWifiStaIface, StaRoamingCapabilities& result)
{
    result.maxBlocklistSize = msgWifiStaIface.maxblocklistsize();
    result.maxAllowlistSize = msgWifiStaIface.maxallowlistsize();
}

bool WifiStaIfaceParseGetRoamingCapabilitiesCfm(const uint8_t* data, size_t length, GetRoamingCapabilitiesCfmStaIfaceParam& param)
{
    GetRoamingCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2StaRoamingCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseInstallApfPacketFilterReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    InstallApfPacketFilterReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.program(), param);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseInstallApfPacketFilterCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseReadApfPacketFilterDataReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseReadApfPacketFilterDataCfm(const uint8_t* data, size_t length, ReadApfPacketFilterDataCfmStaIfaceParam& param)
{
    ReadApfPacketFilterDataCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseRegisterEventCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseRegisterEventCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseSetMacAddressReq(const uint8_t* data, size_t length, array<uint8_t, 6>& param)
{
    SetMacAddressReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Array(msg.mac(), param);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseSetMacAddressCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseSetRoamingStateReq(const uint8_t* data, size_t length, StaRoamingState& param)
{
    SetRoamingStateReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (StaRoamingState) msg.state();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseSetRoamingStateCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseSetScanModeReq(const uint8_t* data, size_t length, bool& param)
{
    SetScanModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseSetScanModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2StaBackgroundScanBucketParameters(const StaBackgroundScanBucketParameters_P& msgWifiStaIface, StaBackgroundScanBucketParameters& buckets)
{
    buckets.bucketIdx = msgWifiStaIface.bucketidx();
    buckets.band = (WifiBand) msgWifiStaIface.band();
    int size_frequencies = msgWifiStaIface.frequencies_size();
    if (0 < size_frequencies)
    {
        buckets.frequencies.resize(size_frequencies);
        for (int index = 0; index < size_frequencies; index++)
        {
            buckets.frequencies[index] = msgWifiStaIface.frequencies(index);
        }
    }
    buckets.periodInMs = msgWifiStaIface.periodinms();
    buckets.eventReportScheme = msgWifiStaIface.eventreportscheme();
    buckets.exponentialMaxPeriodInMs = msgWifiStaIface.exponentialmaxperiodinms();
    buckets.exponentialBase = msgWifiStaIface.exponentialbase();
    buckets.exponentialStepCount = msgWifiStaIface.exponentialstepcount();
}

static void Message2StaBackgroundScanParameters(const StaBackgroundScanParameters_P& msgWifiStaIface, StaBackgroundScanParameters& params)
{
    params.basePeriodInMs = msgWifiStaIface.baseperiodinms();
    params.maxApPerScan = msgWifiStaIface.maxapperscan();
    params.reportThresholdPercent = msgWifiStaIface.reportthresholdpercent();
    params.reportThresholdNumScans = msgWifiStaIface.reportthresholdnumscans();
    int size_buckets = msgWifiStaIface.buckets_size();
    if (0 < size_buckets)
    {
        params.buckets.resize(size_buckets);
        for (int index = 0; index < size_buckets; index++)
        {
            Message2StaBackgroundScanBucketParameters(msgWifiStaIface.buckets(index), params.buckets[index]);
        }
    }
}

bool WifiStaIfaceParseStartBackgroundScanReq(const uint8_t* data, size_t length, StartBackgroundScanReqStaIfaceParam& param)
{
    StartBackgroundScanReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2StaBackgroundScanParameters(msg.params(), param.params);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStartBackgroundScanCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStartDebugPacketFateMonitoringReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStartDebugPacketFateMonitoringCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStartRssiMonitoringReq(const uint8_t* data, size_t length, StartRssiMonitoringReqStaIfaceParam& param)
{
    StartRssiMonitoringReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.maxRssi = msg.maxrssi();
        param.minRssi = msg.minrssi();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStartRssiMonitoringCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStartSendingKeepAlivePacketsReq(const uint8_t* data, size_t length, StartSendingKeepAlivePacketsReqStaIfaceParam& param)
{
    StartSendingKeepAlivePacketsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        String2Vector(msg.ippacketdata(), param.ipPacketData);
        param.etherType = msg.ethertype();
        String2Array(msg.srcaddress(), param.srcAddress);
        String2Array(msg.dstaddress(), param.dstAddress);
        param.periodInMs = msg.periodinms();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStartSendingKeepAlivePacketsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStopBackgroundScanReq(const uint8_t* data, size_t length, int32_t& param)
{
    StopBackgroundScanReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStopBackgroundScanCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStopRssiMonitoringReq(const uint8_t* data, size_t length, int32_t& param)
{
    StopRssiMonitoringReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStopRssiMonitoringCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseStopSendingKeepAlivePacketsReq(const uint8_t* data, size_t length, int32_t& param)
{
    StopSendingKeepAlivePacketsReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseStopSendingKeepAlivePacketsCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiStaIfaceParseSetDtimMultiplierReq(const uint8_t* data, size_t length, int32_t& param)
{
    SetDtimMultiplierReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.multiplier();
        return true;
    }
    return false;
}

bool WifiStaIfaceParseSetDtimMultiplierCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiInformationElement(const WifiInformationElement_P& msgWifiStaIface, WifiInformationElement& informationElements)
{
    informationElements.id = msgWifiStaIface.id();
    String2Vector(msgWifiStaIface.data(), informationElements.data);
}

static void Message2StaScanResult(const StaScanResult_P& msgWifiStaIface, StaScanResult& result)
{
    result.timeStampInUs = msgWifiStaIface.timestampinus();
    String2Vector(msgWifiStaIface.ssid(), result.ssid);
    String2Array(msgWifiStaIface.bssid(), result.bssid);
    result.rssi = msgWifiStaIface.rssi();
    result.frequency = msgWifiStaIface.frequency();
    result.beaconPeriodInMs = msgWifiStaIface.beaconperiodinms();
    result.capability = msgWifiStaIface.capability();
    int size_informationelements = msgWifiStaIface.informationelements_size();
    if (0 < size_informationelements)
    {
        result.informationElements.resize(size_informationelements);
        for (int index = 0; index < size_informationelements; index++)
        {
            Message2WifiInformationElement(msgWifiStaIface.informationelements(index), result.informationElements[index]);
        }
    }
}

bool WifiStaIfaceParseOnBackgroundFullScanResultInd(const uint8_t* data, size_t length, OnBackgroundFullScanResultIndStaIfaceParam& param)
{
    OnBackgroundFullScanResultInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        param.bucketsScanned = msg.bucketsscanned();
        Message2StaScanResult(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiStaIfaceParseOnBackgroundScanFailureInd(const uint8_t* data, size_t length, int32_t& param)
{
    OnBackgroundScanFailureInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

static void Message2StaScanData(const StaScanData_P& msgWifiStaIface, StaScanData& scanDatas)
{
    scanDatas.flags = msgWifiStaIface.flags();
    scanDatas.bucketsScanned = msgWifiStaIface.bucketsscanned();
    int size_results = msgWifiStaIface.results_size();
    if (0 < size_results)
    {
        scanDatas.results.resize(size_results);
        for (int index = 0; index < size_results; index++)
        {
            Message2StaScanResult(msgWifiStaIface.results(index), scanDatas.results[index]);
        }
    }
}

bool WifiStaIfaceParseOnBackgroundScanResultsInd(const uint8_t* data, size_t length, OnBackgroundScanResultsIndStaIfaceParam& param)
{
    OnBackgroundScanResultsInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        int size_scandatas = msg.scandatas_size();
        if (0 < size_scandatas)
        {
            param.scanDatas.resize(size_scandatas);
            for (int index = 0; index < size_scandatas; index++)
            {
                Message2StaScanData(msg.scandatas(index), param.scanDatas[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiStaIfaceParseOnRssiThresholdBreachedInd(const uint8_t* data, size_t length, OnRssiThresholdBreachedIndStaIfaceParam& param)
{
    OnRssiThresholdBreachedInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        String2Array(msg.currbssid(), param.currBssid);
        param.currRssi = msg.currrssi();
        return true;
    }
    return false;
}
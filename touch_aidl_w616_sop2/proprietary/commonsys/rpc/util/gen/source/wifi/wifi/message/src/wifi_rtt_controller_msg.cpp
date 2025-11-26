/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "wifi_rtt_controller_msg.h"

#include "HalStatus.pb.h"
#include "IWifiRttController.pb.h"
#include "IWifiRttControllerEventCallback.pb.h"
#include "IWifiStaIface.pb.h"
#include "MacAddress.pb.h"
#include "RttBw.pb.h"
#include "RttCapabilities.pb.h"
#include "RttConfig.pb.h"
#include "RttLciInformation.pb.h"
#include "RttLcrInformation.pb.h"
#include "RttMotionPattern.pb.h"
#include "RttPeerType.pb.h"
#include "RttPreamble.pb.h"
#include "RttResponder.pb.h"
#include "RttResult.pb.h"
#include "RttStatus.pb.h"
#include "RttType.pb.h"
#include "WifiChannelInfo.pb.h"
#include "WifiChannelWidthInMhz.pb.h"
#include "WifiInformationElement.pb.h"
#include "WifiRateInfo.pb.h"
#include "WifiRateNss.pb.h"
#include "WifiRatePreamble.pb.h"

using DisableResponderReq = ::wifi::IWifiRttController_DisableResponderReq;
using EnableResponderReq = ::wifi::IWifiRttController_EnableResponderReq;
using GetBoundIfaceCfm = ::wifi::IWifiRttController_GetBoundIfaceCfm;
using GetCapabilitiesCfm = ::wifi::IWifiRttController_GetCapabilitiesCfm;
using GetResponderInfoCfm = ::wifi::IWifiRttController_GetResponderInfoCfm;
using OnResultsInd = ::wifi::IWifiRttControllerEventCallback_OnResultsInd;
using RangeCancelReq = ::wifi::IWifiRttController_RangeCancelReq;
using RangeRequestReq = ::wifi::IWifiRttController_RangeRequestReq;
using SetLciReq = ::wifi::IWifiRttController_SetLciReq;
using SetLcrReq = ::wifi::IWifiRttController_SetLcrReq;

using HalStatus_P = ::wifi::HalStatus;
using IWifiStaIface_P = ::wifi::IWifiStaIface;
using MacAddress_P = ::wifi::MacAddress;
using RttBw_P = ::wifi::RttBw;
using RttCapabilities_P = ::wifi::RttCapabilities;
using RttConfig_P = ::wifi::RttConfig;
using RttLciInformation_P = ::wifi::RttLciInformation;
using RttLcrInformation_P = ::wifi::RttLcrInformation;
using RttMotionPattern_P = ::wifi::rmp::RttMotionPattern;
using RttPeerType_P = ::wifi::rpt::RttPeerType;
using RttPreamble_P = ::wifi::rp::RttPreamble;
using RttResponder_P = ::wifi::RttResponder;
using RttResult_P = ::wifi::RttResult;
using RttStatus_P = ::wifi::rs::RttStatus;
using RttType_P = ::wifi::RttType;
using WifiChannelInfo_P = ::wifi::WifiChannelInfo;
using WifiChannelWidthInMhz_P = ::wifi::WifiChannelWidthInMhz;
using WifiInformationElement_P = ::wifi::WifiInformationElement;
using WifiRateInfo_P = ::wifi::WifiRateInfo;
using WifiRateNss_P = ::wifi::WifiRateNss;
using WifiRatePreamble_P = ::wifi::wrp::WifiRatePreamble;


bool WifiRttControllerSerializeDisableResponderReq(int32_t cmdId, vector<uint8_t>& payload)
{
    DisableResponderReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgWifiRttController)
{
    msgWifiRttController.set_status(status.status);
    msgWifiRttController.set_info(status.info);
}

bool WifiRttControllerSerializeDisableResponderCfm(vector<uint8_t>& payload)
{
    return true;
}

static void WifiChannelInfo2Message(const WifiChannelInfo& channelHint, WifiChannelInfo_P& msgWifiRttController)
{
    msgWifiRttController.set_width((WifiChannelWidthInMhz_P) channelHint.width);
    msgWifiRttController.set_centerfreq(channelHint.centerFreq);
    msgWifiRttController.set_centerfreq0(channelHint.centerFreq0);
    msgWifiRttController.set_centerfreq1(channelHint.centerFreq1);
}

static void RttResponder2Message(const RttResponder& info, RttResponder_P& msgWifiRttController)
{
    WifiChannelInfo_P* channel_p = msgWifiRttController.mutable_channel();
    WifiChannelInfo2Message(info.channel, *channel_p);
    msgWifiRttController.set_preamble((RttPreamble_P) info.preamble);
}

bool WifiRttControllerSerializeEnableResponderReq(int32_t cmdId, const WifiChannelInfo& channelHint, int32_t maxDurationInSeconds, const RttResponder& info, vector<uint8_t>& payload)
{
    EnableResponderReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    WifiChannelInfo_P* channelHint_p = msgWifiRttController.mutable_channelhint();
    WifiChannelInfo2Message(channelHint, *channelHint_p);
    msgWifiRttController.set_maxdurationinseconds(maxDurationInSeconds);
    RttResponder_P* info_p = msgWifiRttController.mutable_info();
    RttResponder2Message(info, *info_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeEnableResponderCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiRttControllerSerializeGetBoundIfaceReq(vector<uint8_t>& payload)
{
    return true;
}

static void IWifiStaIface2Message(int32_t result, IWifiStaIface_P& msgWifiRttController)
{
    msgWifiRttController.set_instanceid(result);
}

bool WifiRttControllerSerializeGetBoundIfaceCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetBoundIfaceCfm msgWifiRttController;
    HalStatus_P* status_p = msgWifiRttController.mutable_status();
    HalStatus2Message(status, *status_p);
    IWifiStaIface_P* result_p = msgWifiRttController.mutable_result();
    IWifiStaIface2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeGetCapabilitiesReq(vector<uint8_t>& payload)
{
    return true;
}

static void RttCapabilities2Message(const RttCapabilities& result, RttCapabilities_P& msgWifiRttController)
{
    msgWifiRttController.set_rttonesidedsupported(result.rttOneSidedSupported);
    msgWifiRttController.set_rttftmsupported(result.rttFtmSupported);
    msgWifiRttController.set_lcisupported(result.lciSupported);
    msgWifiRttController.set_lcrsupported(result.lcrSupported);
    msgWifiRttController.set_respondersupported(result.responderSupported);
    msgWifiRttController.set_preamblesupport((RttPreamble_P) result.preambleSupport);
    msgWifiRttController.set_bwsupport((RttBw_P) result.bwSupport);
    msgWifiRttController.set_mcversion(result.mcVersion);
}

bool WifiRttControllerSerializeGetCapabilitiesCfm(const HalStatusParam& status, const RttCapabilities& result, vector<uint8_t>& payload)
{
    GetCapabilitiesCfm msgWifiRttController;
    HalStatus_P* status_p = msgWifiRttController.mutable_status();
    HalStatus2Message(status, *status_p);
    RttCapabilities_P* result_p = msgWifiRttController.mutable_result();
    RttCapabilities2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeGetResponderInfoReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiRttControllerSerializeGetResponderInfoCfm(const HalStatusParam& status, const RttResponder& result, vector<uint8_t>& payload)
{
    GetResponderInfoCfm msgWifiRttController;
    HalStatus_P* status_p = msgWifiRttController.mutable_status();
    HalStatus2Message(status, *status_p);
    RttResponder_P* result_p = msgWifiRttController.mutable_result();
    RttResponder2Message(result, *result_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

static void MacAddress2Message(const MacAddress& addrs, MacAddress_P& msgWifiRttController)
{
    string data;
    Array2String(addrs.data, data);
    msgWifiRttController.set_data(data);
}

bool WifiRttControllerSerializeRangeCancelReq(int32_t cmdId, const vector<MacAddress>& addrs, vector<uint8_t>& payload)
{
    RangeCancelReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    int size_addrs = addrs.size();
    if (0 < size_addrs)
    {
        for (int index = 0; index < size_addrs; index++)
        {
            MacAddress_P *addrs_p = msgWifiRttController.add_addrs();
            MacAddress2Message(addrs[index], *addrs_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeRangeCancelCfm(vector<uint8_t>& payload)
{
    return true;
}

static void RttConfig2Message(const RttConfig& rttConfigs, RttConfig_P& msgWifiRttController)
{
    string addr;
    Array2String(rttConfigs.addr, addr);
    msgWifiRttController.set_addr(addr);
    msgWifiRttController.set_type((RttType_P) rttConfigs.type);
    msgWifiRttController.set_peer((RttPeerType_P) rttConfigs.peer);
    WifiChannelInfo_P* channel_p = msgWifiRttController.mutable_channel();
    WifiChannelInfo2Message(rttConfigs.channel, *channel_p);
    msgWifiRttController.set_burstperiod(rttConfigs.burstPeriod);
    msgWifiRttController.set_numburst(rttConfigs.numBurst);
    msgWifiRttController.set_numframesperburst(rttConfigs.numFramesPerBurst);
    msgWifiRttController.set_numretriesperrttframe(rttConfigs.numRetriesPerRttFrame);
    msgWifiRttController.set_numretriesperftmr(rttConfigs.numRetriesPerFtmr);
    msgWifiRttController.set_mustrequestlci(rttConfigs.mustRequestLci);
    msgWifiRttController.set_mustrequestlcr(rttConfigs.mustRequestLcr);
    msgWifiRttController.set_burstduration(rttConfigs.burstDuration);
    msgWifiRttController.set_preamble((RttPreamble_P) rttConfigs.preamble);
    msgWifiRttController.set_bw((RttBw_P) rttConfigs.bw);
}

bool WifiRttControllerSerializeRangeRequestReq(int32_t cmdId, const vector<RttConfig>& rttConfigs, vector<uint8_t>& payload)
{
    RangeRequestReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    int size_rttconfigs = rttConfigs.size();
    if (0 < size_rttconfigs)
    {
        for (int index = 0; index < size_rttconfigs; index++)
        {
            RttConfig_P *rttConfigs_p = msgWifiRttController.add_rttconfigs();
            RttConfig2Message(rttConfigs[index], *rttConfigs_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeRangeRequestCfm(vector<uint8_t>& payload)
{
    return true;
}

bool WifiRttControllerSerializeRegisterEventCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool WifiRttControllerSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

static void RttLciInformation2Message(const RttLciInformation& lci, RttLciInformation_P& msgWifiRttController)
{
    msgWifiRttController.set_latitude(lci.latitude);
    msgWifiRttController.set_longitude(lci.longitude);
    msgWifiRttController.set_altitude(lci.altitude);
    msgWifiRttController.set_latitudeunc(lci.latitudeUnc);
    msgWifiRttController.set_longitudeunc(lci.longitudeUnc);
    msgWifiRttController.set_altitudeunc(lci.altitudeUnc);
    msgWifiRttController.set_motionpattern((RttMotionPattern_P) lci.motionPattern);
    msgWifiRttController.set_floor(lci.floor);
    msgWifiRttController.set_heightabovefloor(lci.heightAboveFloor);
    msgWifiRttController.set_heightunc(lci.heightUnc);
}

bool WifiRttControllerSerializeSetLciReq(int32_t cmdId, const RttLciInformation& lci, vector<uint8_t>& payload)
{
    SetLciReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    RttLciInformation_P* lci_p = msgWifiRttController.mutable_lci();
    RttLciInformation2Message(lci, *lci_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeSetLciCfm(vector<uint8_t>& payload)
{
    return true;
}

static void RttLcrInformation2Message(const RttLcrInformation& lcr, RttLcrInformation_P& msgWifiRttController)
{
    string countryCode;
    Array2String(lcr.countryCode, countryCode);
    msgWifiRttController.set_countrycode(countryCode);
    msgWifiRttController.set_civicinfo(lcr.civicInfo);
}

bool WifiRttControllerSerializeSetLcrReq(int32_t cmdId, const RttLcrInformation& lcr, vector<uint8_t>& payload)
{
    SetLcrReq msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    RttLcrInformation_P* lcr_p = msgWifiRttController.mutable_lcr();
    RttLcrInformation2Message(lcr, *lcr_p);
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerSerializeSetLcrCfm(vector<uint8_t>& payload)
{
    return true;
}

static void WifiRateInfo2Message(const WifiRateInfo& txRate, WifiRateInfo_P& msgWifiRttController)
{
    msgWifiRttController.set_preamble((WifiRatePreamble_P) txRate.preamble);
    msgWifiRttController.set_nss((WifiRateNss_P) txRate.nss);
    msgWifiRttController.set_bw((WifiChannelWidthInMhz_P) txRate.bw);
    msgWifiRttController.set_ratemcsidx(txRate.rateMcsIdx);
    msgWifiRttController.set_bitrateinkbps(txRate.bitRateInKbps);
}

static void WifiInformationElement2Message(const WifiInformationElement& lci, WifiInformationElement_P& msgWifiRttController)
{
    msgWifiRttController.set_id(lci.id);
    string data;
    Vector2String(lci.data, data);
    msgWifiRttController.set_data(data);
}

static void RttResult2Message(const RttResult& results, RttResult_P& msgWifiRttController)
{
    string addr;
    Array2String(results.addr, addr);
    msgWifiRttController.set_addr(addr);
    msgWifiRttController.set_burstnum(results.burstNum);
    msgWifiRttController.set_measurementnumber(results.measurementNumber);
    msgWifiRttController.set_successnumber(results.successNumber);
    msgWifiRttController.set_numberperburstpeer(results.numberPerBurstPeer);
    msgWifiRttController.set_status((RttStatus_P) results.status);
    msgWifiRttController.set_retryafterduration(results.retryAfterDuration);
    msgWifiRttController.set_type((RttType_P) results.type);
    msgWifiRttController.set_rssi(results.rssi);
    msgWifiRttController.set_rssispread(results.rssiSpread);
    WifiRateInfo_P* txRate_p = msgWifiRttController.mutable_txrate();
    WifiRateInfo2Message(results.txRate, *txRate_p);
    WifiRateInfo_P* rxRate_p = msgWifiRttController.mutable_rxrate();
    WifiRateInfo2Message(results.rxRate, *rxRate_p);
    msgWifiRttController.set_rtt(results.rtt);
    msgWifiRttController.set_rttsd(results.rttSd);
    msgWifiRttController.set_rttspread(results.rttSpread);
    msgWifiRttController.set_distanceinmm(results.distanceInMm);
    msgWifiRttController.set_distancesdinmm(results.distanceSdInMm);
    msgWifiRttController.set_distancespreadinmm(results.distanceSpreadInMm);
    msgWifiRttController.set_timestampinus(results.timeStampInUs);
    msgWifiRttController.set_burstdurationinms(results.burstDurationInMs);
    msgWifiRttController.set_negotiatedburstnum(results.negotiatedBurstNum);
    WifiInformationElement_P* lci_p = msgWifiRttController.mutable_lci();
    WifiInformationElement2Message(results.lci, *lci_p);
    WifiInformationElement_P* lcr_p = msgWifiRttController.mutable_lcr();
    WifiInformationElement2Message(results.lcr, *lcr_p);
    msgWifiRttController.set_channelfreqmhz(results.channelFreqMHz);
    msgWifiRttController.set_packetbw((RttBw_P) results.packetBw);
}

bool WifiRttControllerSerializeOnResultsInd(int32_t cmdId, const vector<RttResult>& results, vector<uint8_t>& payload)
{
    OnResultsInd msgWifiRttController;
    msgWifiRttController.set_cmdid(cmdId);
    int size_results = results.size();
    if (0 < size_results)
    {
        for (int index = 0; index < size_results; index++)
        {
            RttResult_P *results_p = msgWifiRttController.add_results();
            RttResult2Message(results[index], *results_p);
        }
    }
    return ProtoMessageSerialize(&msgWifiRttController, payload);
}

bool WifiRttControllerParseDisableResponderReq(const uint8_t* data, size_t length, int32_t& param)
{
    DisableResponderReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.cmdid();
        return true;
    }
    return false;
}

static void Message2HalStatus(const HalStatus_P& msgWifiRttController, HalStatusParam& status)
{
    status.status = msgWifiRttController.status();
    status.info = msgWifiRttController.info();
}

bool WifiRttControllerParseDisableResponderCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiChannelInfo(const WifiChannelInfo_P& msgWifiRttController, WifiChannelInfo& channelHint)
{
    channelHint.width = (WifiChannelWidthInMhz) msgWifiRttController.width();
    channelHint.centerFreq = msgWifiRttController.centerfreq();
    channelHint.centerFreq0 = msgWifiRttController.centerfreq0();
    channelHint.centerFreq1 = msgWifiRttController.centerfreq1();
}

static void Message2RttResponder(const RttResponder_P& msgWifiRttController, RttResponder& info)
{
    Message2WifiChannelInfo(msgWifiRttController.channel(), info.channel);
    info.preamble = (RttPreamble) msgWifiRttController.preamble();
}

bool WifiRttControllerParseEnableResponderReq(const uint8_t* data, size_t length, EnableResponderReqRttControllerParam& param)
{
    EnableResponderReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2WifiChannelInfo(msg.channelhint(), param.channelHint);
        param.maxDurationInSeconds = msg.maxdurationinseconds();
        Message2RttResponder(msg.info(), param.info);
        return true;
    }
    return false;
}

bool WifiRttControllerParseEnableResponderCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiRttControllerParseGetBoundIfaceReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2IWifiStaIface(const IWifiStaIface_P& msgWifiRttController, int32_t& result)
{
    result = msgWifiRttController.instanceid();
}

bool WifiRttControllerParseGetBoundIfaceCfm(const uint8_t* data, size_t length, GetBoundIfaceCfmRttControllerParam& param)
{
    GetBoundIfaceCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2IWifiStaIface(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiRttControllerParseGetCapabilitiesReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2RttCapabilities(const RttCapabilities_P& msgWifiRttController, RttCapabilities& result)
{
    result.rttOneSidedSupported = msgWifiRttController.rttonesidedsupported();
    result.rttFtmSupported = msgWifiRttController.rttftmsupported();
    result.lciSupported = msgWifiRttController.lcisupported();
    result.lcrSupported = msgWifiRttController.lcrsupported();
    result.responderSupported = msgWifiRttController.respondersupported();
    result.preambleSupport = (RttPreamble) msgWifiRttController.preamblesupport();
    result.bwSupport = (RttBw) msgWifiRttController.bwsupport();
    result.mcVersion = msgWifiRttController.mcversion();
}

bool WifiRttControllerParseGetCapabilitiesCfm(const uint8_t* data, size_t length, GetCapabilitiesCfmRttControllerParam& param)
{
    GetCapabilitiesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2RttCapabilities(msg.result(), param.result);
        return true;
    }
    return false;
}

bool WifiRttControllerParseGetResponderInfoReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiRttControllerParseGetResponderInfoCfm(const uint8_t* data, size_t length, GetResponderInfoCfmRttControllerParam& param)
{
    GetResponderInfoCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        Message2RttResponder(msg.result(), param.result);
        return true;
    }
    return false;
}

static void Message2MacAddress(const MacAddress_P& msgWifiRttController, MacAddress& addrs)
{
    String2Array(msgWifiRttController.data(), addrs.data);
}

bool WifiRttControllerParseRangeCancelReq(const uint8_t* data, size_t length, RangeCancelReqRttControllerParam& param)
{
    RangeCancelReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        int size_addrs = msg.addrs_size();
        if (0 < size_addrs)
        {
            param.addrs.resize(size_addrs);
            for (int index = 0; index < size_addrs; index++)
            {
                Message2MacAddress(msg.addrs(index), param.addrs[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiRttControllerParseRangeCancelCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2RttConfig(const RttConfig_P& msgWifiRttController, RttConfig& rttConfigs)
{
    String2Array(msgWifiRttController.addr(), rttConfigs.addr);
    rttConfigs.type = (RttType) msgWifiRttController.type();
    rttConfigs.peer = (RttPeerType) msgWifiRttController.peer();
    Message2WifiChannelInfo(msgWifiRttController.channel(), rttConfigs.channel);
    rttConfigs.burstPeriod = msgWifiRttController.burstperiod();
    rttConfigs.numBurst = msgWifiRttController.numburst();
    rttConfigs.numFramesPerBurst = msgWifiRttController.numframesperburst();
    rttConfigs.numRetriesPerRttFrame = msgWifiRttController.numretriesperrttframe();
    rttConfigs.numRetriesPerFtmr = msgWifiRttController.numretriesperftmr();
    rttConfigs.mustRequestLci = msgWifiRttController.mustrequestlci();
    rttConfigs.mustRequestLcr = msgWifiRttController.mustrequestlcr();
    rttConfigs.burstDuration = msgWifiRttController.burstduration();
    rttConfigs.preamble = (RttPreamble) msgWifiRttController.preamble();
    rttConfigs.bw = (RttBw) msgWifiRttController.bw();
}

bool WifiRttControllerParseRangeRequestReq(const uint8_t* data, size_t length, RangeRequestReqRttControllerParam& param)
{
    RangeRequestReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        int size_rttconfigs = msg.rttconfigs_size();
        if (0 < size_rttconfigs)
        {
            param.rttConfigs.resize(size_rttconfigs);
            for (int index = 0; index < size_rttconfigs; index++)
            {
                Message2RttConfig(msg.rttconfigs(index), param.rttConfigs[index]);
            }
        }
        return true;
    }
    return false;
}

bool WifiRttControllerParseRangeRequestCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiRttControllerParseRegisterEventCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool WifiRttControllerParseRegisterEventCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2RttLciInformation(const RttLciInformation_P& msgWifiRttController, RttLciInformation& lci)
{
    lci.latitude = msgWifiRttController.latitude();
    lci.longitude = msgWifiRttController.longitude();
    lci.altitude = msgWifiRttController.altitude();
    lci.latitudeUnc = msgWifiRttController.latitudeunc();
    lci.longitudeUnc = msgWifiRttController.longitudeunc();
    lci.altitudeUnc = msgWifiRttController.altitudeunc();
    lci.motionPattern = (RttMotionPattern) msgWifiRttController.motionpattern();
    lci.floor = msgWifiRttController.floor();
    lci.heightAboveFloor = msgWifiRttController.heightabovefloor();
    lci.heightUnc = msgWifiRttController.heightunc();
}

bool WifiRttControllerParseSetLciReq(const uint8_t* data, size_t length, SetLciReqRttControllerParam& param)
{
    SetLciReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2RttLciInformation(msg.lci(), param.lci);
        return true;
    }
    return false;
}

bool WifiRttControllerParseSetLciCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2RttLcrInformation(const RttLcrInformation_P& msgWifiRttController, RttLcrInformation& lcr)
{
    String2Array(msgWifiRttController.countrycode(), lcr.countryCode);
    lcr.civicInfo = msgWifiRttController.civicinfo();
}

bool WifiRttControllerParseSetLcrReq(const uint8_t* data, size_t length, SetLcrReqRttControllerParam& param)
{
    SetLcrReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        Message2RttLcrInformation(msg.lcr(), param.lcr);
        return true;
    }
    return false;
}

bool WifiRttControllerParseSetLcrCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2WifiRateInfo(const WifiRateInfo_P& msgWifiRttController, WifiRateInfo& txRate)
{
    txRate.preamble = (WifiRatePreamble) msgWifiRttController.preamble();
    txRate.nss = (WifiRateNss) msgWifiRttController.nss();
    txRate.bw = (WifiChannelWidthInMhz) msgWifiRttController.bw();
    txRate.rateMcsIdx = msgWifiRttController.ratemcsidx();
    txRate.bitRateInKbps = msgWifiRttController.bitrateinkbps();
}

static void Message2WifiInformationElement(const WifiInformationElement_P& msgWifiRttController, WifiInformationElement& lci)
{
    lci.id = msgWifiRttController.id();
    String2Vector(msgWifiRttController.data(), lci.data);
}

static void Message2RttResult(const RttResult_P& msgWifiRttController, RttResult& results)
{
    String2Array(msgWifiRttController.addr(), results.addr);
    results.burstNum = msgWifiRttController.burstnum();
    results.measurementNumber = msgWifiRttController.measurementnumber();
    results.successNumber = msgWifiRttController.successnumber();
    results.numberPerBurstPeer = msgWifiRttController.numberperburstpeer();
    results.status = (RttStatus) msgWifiRttController.status();
    results.retryAfterDuration = msgWifiRttController.retryafterduration();
    results.type = (RttType) msgWifiRttController.type();
    results.rssi = msgWifiRttController.rssi();
    results.rssiSpread = msgWifiRttController.rssispread();
    Message2WifiRateInfo(msgWifiRttController.txrate(), results.txRate);
    Message2WifiRateInfo(msgWifiRttController.rxrate(), results.rxRate);
    results.rtt = msgWifiRttController.rtt();
    results.rttSd = msgWifiRttController.rttsd();
    results.rttSpread = msgWifiRttController.rttspread();
    results.distanceInMm = msgWifiRttController.distanceinmm();
    results.distanceSdInMm = msgWifiRttController.distancesdinmm();
    results.distanceSpreadInMm = msgWifiRttController.distancespreadinmm();
    results.timeStampInUs = msgWifiRttController.timestampinus();
    results.burstDurationInMs = msgWifiRttController.burstdurationinms();
    results.negotiatedBurstNum = msgWifiRttController.negotiatedburstnum();
    Message2WifiInformationElement(msgWifiRttController.lci(), results.lci);
    Message2WifiInformationElement(msgWifiRttController.lcr(), results.lcr);
    results.channelFreqMHz = msgWifiRttController.channelfreqmhz();
    results.packetBw = (RttBw) msgWifiRttController.packetbw();
}

bool WifiRttControllerParseOnResultsInd(const uint8_t* data, size_t length, OnResultsIndRttControllerParam& param)
{
    OnResultsInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.cmdId = msg.cmdid();
        int size_results = msg.results_size();
        if (0 < size_results)
        {
            param.results.resize(size_results);
            for (int index = 0; index < size_results; index++)
            {
                Message2RttResult(msg.results(index), param.results[index]);
            }
        }
        return true;
    }
    return false;
}
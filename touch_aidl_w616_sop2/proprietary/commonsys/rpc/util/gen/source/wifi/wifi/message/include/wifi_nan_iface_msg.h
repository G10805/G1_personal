/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/IWifiNanIface.h>
#include <aidl/android/hardware/wifi/IWifiNanIfaceEventCallback.h>

#include "wifi_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi;


typedef struct
{
    HalStatusParam status;
    string result;
} GetNameCfmNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanConfigRequest msg1;
    NanConfigRequestSupplemental msg2;
} ConfigRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    string ifaceName;
} CreateDataInterfaceRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    string ifaceName;
} DeleteDataInterfaceRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanEnableRequest msg1;
    NanConfigRequestSupplemental msg2;
} EnableRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanInitiateDataPathRequest msg;
} InitiateDataPathRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanRespondToDataPathIndicationRequest msg;
} RespondToDataPathIndicationRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanPublishRequest msg;
} StartPublishRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanSubscribeRequest msg;
} StartSubscribeRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int8_t sessionId;
} StopPublishRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int8_t sessionId;
} StopSubscribeRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int32_t ndpInstanceId;
} TerminateDataPathRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int8_t sessionId;
} SuspendRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int8_t sessionId;
} ResumeRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanTransmitFollowupRequest msg;
} TransmitFollowupRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanPairingRequest msg;
} InitiatePairingRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanRespondToPairingIndicationRequest msg;
} RespondToPairingIndicationRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanBootstrappingRequest msg;
} InitiateBootstrappingRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    NanBootstrappingResponse msg;
} RespondToBootstrappingIndicationRequestReqNanIfaceParam;

typedef struct
{
    char16_t cmdId;
    int32_t pairingInstanceId;
} TerminatePairingRequestReqNanIfaceParam;

typedef struct
{
    int8_t discoverySessionId;
    int32_t peerId;
} EventMatchExpiredIndNanIfaceParam;

typedef struct
{
    int8_t sessionId;
    NanStatus status;
} EventPublishTerminatedIndNanIfaceParam;

typedef struct
{
    int8_t sessionId;
    NanStatus status;
} EventSubscribeTerminatedIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} EventTransmitFollowupIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    NanCapabilities capabilities;
} NotifyCapabilitiesResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyConfigResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyCreateDataInterfaceResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyDeleteDataInterfaceResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyDisableResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyEnableResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    int32_t ndpInstanceId;
} NotifyInitiateDataPathResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyRespondToDataPathIndicationResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    int8_t sessionId;
} NotifyStartPublishResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    int8_t sessionId;
} NotifyStartSubscribeResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyStopPublishResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyStopSubscribeResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyTerminateDataPathResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifySuspendResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyResumeResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyTransmitFollowupResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    int32_t pairingInstanceId;
} NotifyInitiatePairingResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyRespondToPairingIndicationResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
    int32_t bootstrappingInstanceId;
} NotifyInitiateBootstrappingResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyRespondToBootstrappingIndicationResponseIndNanIfaceParam;

typedef struct
{
    char16_t id;
    NanStatus status;
} NotifyTerminatePairingResponseIndNanIfaceParam;

bool WifiNanIfaceSerializeGetNameReq(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeGetNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeConfigRequestReq(char16_t cmdId, const NanConfigRequest& msg1, const NanConfigRequestSupplemental& msg2, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeConfigRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeCreateDataInterfaceRequestReq(char16_t cmdId, const string& ifaceName, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeCreateDataInterfaceRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeDeleteDataInterfaceRequestReq(char16_t cmdId, const string& ifaceName, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeDeleteDataInterfaceRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeDisableRequestReq(char16_t cmdId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeDisableRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEnableRequestReq(char16_t cmdId, const NanEnableRequest& msg1, const NanConfigRequestSupplemental& msg2, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEnableRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeGetCapabilitiesRequestReq(char16_t cmdId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeGetCapabilitiesRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiateDataPathRequestReq(char16_t cmdId, const NanInitiateDataPathRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiateDataPathRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRegisterEventCallbackReq(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRegisterEventCallbackCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToDataPathIndicationRequestReq(char16_t cmdId, const NanRespondToDataPathIndicationRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToDataPathIndicationRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStartPublishRequestReq(char16_t cmdId, const NanPublishRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStartPublishRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStartSubscribeRequestReq(char16_t cmdId, const NanSubscribeRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStartSubscribeRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStopPublishRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStopPublishRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStopSubscribeRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeStopSubscribeRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTerminateDataPathRequestReq(char16_t cmdId, int32_t ndpInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTerminateDataPathRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeSuspendRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeSuspendRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeResumeRequestReq(char16_t cmdId, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeResumeRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTransmitFollowupRequestReq(char16_t cmdId, const NanTransmitFollowupRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTransmitFollowupRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiatePairingRequestReq(char16_t cmdId, const NanPairingRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiatePairingRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToPairingIndicationRequestReq(char16_t cmdId, const NanRespondToPairingIndicationRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToPairingIndicationRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiateBootstrappingRequestReq(char16_t cmdId, const NanBootstrappingRequest& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeInitiateBootstrappingRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestReq(char16_t cmdId, const NanBootstrappingResponse& msg, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeRespondToBootstrappingIndicationRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTerminatePairingRequestReq(char16_t cmdId, int32_t pairingInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeTerminatePairingRequestCfm(vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventClusterEventInd(const NanClusterEventInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventDataPathConfirmInd(const NanDataPathConfirmInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventDataPathRequestInd(const NanDataPathRequestInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventDataPathScheduleUpdateInd(const NanDataPathScheduleUpdateInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventDataPathTerminatedInd(int32_t ndpInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventDisabledInd(const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventFollowupReceivedInd(const NanFollowupReceivedInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventMatchInd(const NanMatchInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventMatchExpiredInd(int8_t discoverySessionId, int32_t peerId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventPublishTerminatedInd(int8_t sessionId, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventSubscribeTerminatedInd(int8_t sessionId, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventTransmitFollowupInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventSuspensionModeChangedInd(const NanSuspensionModeChangeInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyCapabilitiesResponseInd(char16_t id, const NanStatus& status, const NanCapabilities& capabilities, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyConfigResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyCreateDataInterfaceResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyDeleteDataInterfaceResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyDisableResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyEnableResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyInitiateDataPathResponseInd(char16_t id, const NanStatus& status, int32_t ndpInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyRespondToDataPathIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyStartPublishResponseInd(char16_t id, const NanStatus& status, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyStartSubscribeResponseInd(char16_t id, const NanStatus& status, int8_t sessionId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyStopPublishResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyStopSubscribeResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyTerminateDataPathResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifySuspendResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyResumeResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyTransmitFollowupResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventPairingRequestInd(const NanPairingRequestInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventPairingConfirmInd(const NanPairingConfirmInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyInitiatePairingResponseInd(char16_t id, const NanStatus& status, int32_t pairingInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyRespondToPairingIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventBootstrappingRequestInd(const NanBootstrappingRequestInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeEventBootstrappingConfirmInd(const NanBootstrappingConfirmInd& event, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyInitiateBootstrappingResponseInd(char16_t id, const NanStatus& status, int32_t bootstrappingInstanceId, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyRespondToBootstrappingIndicationResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceSerializeNotifyTerminatePairingResponseInd(char16_t id, const NanStatus& status, vector<uint8_t>& payload);

bool WifiNanIfaceParseGetNameReq(const uint8_t* data, size_t length);

bool WifiNanIfaceParseGetNameCfm(const uint8_t* data, size_t length, GetNameCfmNanIfaceParam& param);

bool WifiNanIfaceParseConfigRequestReq(const uint8_t* data, size_t length, ConfigRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseConfigRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseCreateDataInterfaceRequestReq(const uint8_t* data, size_t length, CreateDataInterfaceRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseCreateDataInterfaceRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseDeleteDataInterfaceRequestReq(const uint8_t* data, size_t length, DeleteDataInterfaceRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseDeleteDataInterfaceRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseDisableRequestReq(const uint8_t* data, size_t length, char16_t& param);

bool WifiNanIfaceParseDisableRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseEnableRequestReq(const uint8_t* data, size_t length, EnableRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseEnableRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseGetCapabilitiesRequestReq(const uint8_t* data, size_t length, char16_t& param);

bool WifiNanIfaceParseGetCapabilitiesRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseInitiateDataPathRequestReq(const uint8_t* data, size_t length, InitiateDataPathRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseInitiateDataPathRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseRegisterEventCallbackReq(const uint8_t* data, size_t length);

bool WifiNanIfaceParseRegisterEventCallbackCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseRespondToDataPathIndicationRequestReq(const uint8_t* data, size_t length, RespondToDataPathIndicationRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseRespondToDataPathIndicationRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseStartPublishRequestReq(const uint8_t* data, size_t length, StartPublishRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseStartPublishRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseStartSubscribeRequestReq(const uint8_t* data, size_t length, StartSubscribeRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseStartSubscribeRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseStopPublishRequestReq(const uint8_t* data, size_t length, StopPublishRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseStopPublishRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseStopSubscribeRequestReq(const uint8_t* data, size_t length, StopSubscribeRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseStopSubscribeRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseTerminateDataPathRequestReq(const uint8_t* data, size_t length, TerminateDataPathRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseTerminateDataPathRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseSuspendRequestReq(const uint8_t* data, size_t length, SuspendRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseSuspendRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseResumeRequestReq(const uint8_t* data, size_t length, ResumeRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseResumeRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseTransmitFollowupRequestReq(const uint8_t* data, size_t length, TransmitFollowupRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseTransmitFollowupRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseInitiatePairingRequestReq(const uint8_t* data, size_t length, InitiatePairingRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseInitiatePairingRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseRespondToPairingIndicationRequestReq(const uint8_t* data, size_t length, RespondToPairingIndicationRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseRespondToPairingIndicationRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseInitiateBootstrappingRequestReq(const uint8_t* data, size_t length, InitiateBootstrappingRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseInitiateBootstrappingRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseRespondToBootstrappingIndicationRequestReq(const uint8_t* data, size_t length, RespondToBootstrappingIndicationRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseRespondToBootstrappingIndicationRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseTerminatePairingRequestReq(const uint8_t* data, size_t length, TerminatePairingRequestReqNanIfaceParam& param);

bool WifiNanIfaceParseTerminatePairingRequestCfm(const uint8_t* data, size_t length);

bool WifiNanIfaceParseEventClusterEventInd(const uint8_t* data, size_t length, NanClusterEventInd& param);

bool WifiNanIfaceParseEventDataPathConfirmInd(const uint8_t* data, size_t length, NanDataPathConfirmInd& param);

bool WifiNanIfaceParseEventDataPathRequestInd(const uint8_t* data, size_t length, NanDataPathRequestInd& param);

bool WifiNanIfaceParseEventDataPathScheduleUpdateInd(const uint8_t* data, size_t length, NanDataPathScheduleUpdateInd& param);

bool WifiNanIfaceParseEventDataPathTerminatedInd(const uint8_t* data, size_t length, int32_t& param);

bool WifiNanIfaceParseEventDisabledInd(const uint8_t* data, size_t length, NanStatus& param);

bool WifiNanIfaceParseEventFollowupReceivedInd(const uint8_t* data, size_t length, NanFollowupReceivedInd& param);

bool WifiNanIfaceParseEventMatchInd(const uint8_t* data, size_t length, NanMatchInd& param);

bool WifiNanIfaceParseEventMatchExpiredInd(const uint8_t* data, size_t length, EventMatchExpiredIndNanIfaceParam& param);

bool WifiNanIfaceParseEventPublishTerminatedInd(const uint8_t* data, size_t length, EventPublishTerminatedIndNanIfaceParam& param);

bool WifiNanIfaceParseEventSubscribeTerminatedInd(const uint8_t* data, size_t length, EventSubscribeTerminatedIndNanIfaceParam& param);

bool WifiNanIfaceParseEventTransmitFollowupInd(const uint8_t* data, size_t length, EventTransmitFollowupIndNanIfaceParam& param);

bool WifiNanIfaceParseEventSuspensionModeChangedInd(const uint8_t* data, size_t length, NanSuspensionModeChangeInd& param);

bool WifiNanIfaceParseNotifyCapabilitiesResponseInd(const uint8_t* data, size_t length, NotifyCapabilitiesResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyConfigResponseInd(const uint8_t* data, size_t length, NotifyConfigResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyCreateDataInterfaceResponseInd(const uint8_t* data, size_t length, NotifyCreateDataInterfaceResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyDeleteDataInterfaceResponseInd(const uint8_t* data, size_t length, NotifyDeleteDataInterfaceResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyDisableResponseInd(const uint8_t* data, size_t length, NotifyDisableResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyEnableResponseInd(const uint8_t* data, size_t length, NotifyEnableResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyInitiateDataPathResponseInd(const uint8_t* data, size_t length, NotifyInitiateDataPathResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyRespondToDataPathIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToDataPathIndicationResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyStartPublishResponseInd(const uint8_t* data, size_t length, NotifyStartPublishResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyStartSubscribeResponseInd(const uint8_t* data, size_t length, NotifyStartSubscribeResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyStopPublishResponseInd(const uint8_t* data, size_t length, NotifyStopPublishResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyStopSubscribeResponseInd(const uint8_t* data, size_t length, NotifyStopSubscribeResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyTerminateDataPathResponseInd(const uint8_t* data, size_t length, NotifyTerminateDataPathResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifySuspendResponseInd(const uint8_t* data, size_t length, NotifySuspendResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyResumeResponseInd(const uint8_t* data, size_t length, NotifyResumeResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyTransmitFollowupResponseInd(const uint8_t* data, size_t length, NotifyTransmitFollowupResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseEventPairingRequestInd(const uint8_t* data, size_t length, NanPairingRequestInd& param);

bool WifiNanIfaceParseEventPairingConfirmInd(const uint8_t* data, size_t length, NanPairingConfirmInd& param);

bool WifiNanIfaceParseNotifyInitiatePairingResponseInd(const uint8_t* data, size_t length, NotifyInitiatePairingResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyRespondToPairingIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToPairingIndicationResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseEventBootstrappingRequestInd(const uint8_t* data, size_t length, NanBootstrappingRequestInd& param);

bool WifiNanIfaceParseEventBootstrappingConfirmInd(const uint8_t* data, size_t length, NanBootstrappingConfirmInd& param);

bool WifiNanIfaceParseNotifyInitiateBootstrappingResponseInd(const uint8_t* data, size_t length, NotifyInitiateBootstrappingResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyRespondToBootstrappingIndicationResponseInd(const uint8_t* data, size_t length, NotifyRespondToBootstrappingIndicationResponseIndNanIfaceParam& param);

bool WifiNanIfaceParseNotifyTerminatePairingResponseInd(const uint8_t* data, size_t length, NotifyTerminatePairingResponseIndNanIfaceParam& param);
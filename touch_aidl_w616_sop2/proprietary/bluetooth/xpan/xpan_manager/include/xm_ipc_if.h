/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef XM_IPC_IF_H
#define XM_IPC_IF_H

#pragma once

#include <stdint.h>
#include <string.h>
#include "xpan_utils.h"

namespace xpan {
namespace implementation {

typedef enum {
  XM_IPC_MSG_BASE = 1000,
  /* This message is used to send CoP version from DH to XM*/
  DH_XM_COP_VER_IND,
  /* This message is used to send CP log level*/
  XM_CP_LOG_LVL,
  XM_XP_CREATE_SAP_INF_STATUS,
  /* This message is used to notify that remote support XPAN or not*/
  QHCI_XM_REMOTE_SUPPORT_XPAN,
  XP_XM_TRANSPORT_ENABLED,
  XP_XM_BONDED_DEVICES_LIST,
  /* QHCI asks to prepare audio bearer for given transport*/
  QHCI_XM_PREPARE_AUDIO_BEARER_REQ,
  QHCI_XM_USECASE_UPDATE,
  XP_XM_PREPARE_AUDIO_BEARER_RSP,
  KP_XM_PREPARE_AUDIO_BEARER_RSP,
  CP_XM_PREPARE_AUDIO_BEARER_RSP,
  XM_QHCI_PREPARE_AUDIO_BEARER_RSP,
  XP_XM_AUDIO_BEARER_SWITCHED,
  /* QHCI Asks to unprepare audio bearer for given transport */
  QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ,
  XM_PREPARE_AUDIO_BEARER_TIMEOUT,
  XP_XM_ENABLE_ACS,
  WIFI_XM_ACS_RESULTS,
  WIFI_XM_TWT_EVENT,
  XP_XM_HOST_PARAMETERS,
  XP_XM_SAP_POWER_STATE,
  XP_XM_SAP_CURRENT_STATE,
  XP_XM_CREATE_SAP_INF,
  WIFI_XM_SAP_POWER_SAVE_STATE_RSP,
  XP_XM_TWT_SESSION_EST,
  XP_XM_BEARER_PREFERENCE_IND,
  CP_XM_DELAY_REPORTING,
  CP_XM_TRANSPORT_UPDATE,
  /* Triggered to send the usecase to clients during seamless transtions */
  XM_WIFI_USECASE_UPDATE,
  XM_WIFI_ENABLE_STATS,
} XmIpcEventId;

typedef struct {
  XmIpcEventId eventId;
  uint8_t len;
  uint8_t *data; 
} __attribute__((packed)) DhXmCopVerInd;

typedef struct {
  XmIpcEventId eventId;
  uint8_t len;
  uint8_t data; 
} __attribute__((packed)) XmCpLogLvl;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  bool is_supported; 
} __attribute__((packed)) QhciXmRemoteSupportXpan;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  TransportType type;
} __attribute__((packed)) QhciXmPrepareAudioBearerReq;

typedef struct {
  XmIpcEventId eventId;
  UseCaseType usecase;
} __attribute__((packed)) QhciXmUseCase;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  TransportType type;
} __attribute__((packed)) QhciXmUnPrepareAudioBearerReq;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  bool status;
} __attribute__((packed)) XmQhciUnPrepareAudioBearerRsp;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  RspStatus status;
} __attribute__((packed)) XmPrepareAudioBearerRsp;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  TransportType type;
  bool status;
  uint8_t reason;
} __attribute__((packed)) XmXpTransportEnabled;

typedef struct {
  XmIpcEventId eventId;
  uint8_t current_transport;
  uint8_t status;
} __attribute__((packed)) XmKpAudioBearerRsp;

typedef struct {
  XmIpcEventId eventId;
  uint8_t current_transport;
  uint8_t status;
} __attribute__((packed)) XmCpAudioBearerRsp;


typedef struct {
  XmIpcEventId eventId;
} __attribute__((packed)) XmTimeout;

typedef struct {
  XmIpcEventId eventId;
  bdaddr_t bdaddr;
  RspStatus status;
} __attribute__((packed)) XpXmBearerSwitch;

typedef struct {
  XmIpcEventId eventId;
  int *freq_list;
  int freq_list_size;
} __attribute__((packed)) XpEnableAcs;

typedef struct {
  XmIpcEventId eventId;
  uint8_t twt_event;
  int32_t wake_dur_us;
  int32_t wake_int_us;
  macaddr_t addr;
} __attribute__((packed)) XpTwtEvent;

typedef struct {
  XmIpcEventId eventId;
  int freq;
  uint8_t status;
} __attribute__((packed)) XpAcsResults;

typedef struct {
  XmIpcEventId eventId;
  macaddr_t macaddr;
  uint16_t Ethertype;
} __attribute__((packed)) XpHostParameters;

typedef struct {
  XmIpcEventId eventId;
  uint8_t state;
} __attribute__((packed)) XpCreateSapInf;

typedef struct {
  XmIpcEventId eventId;
  uint8_t req_state;
  uint8_t status;
} __attribute__((packed)) XpCreateSapInfStatus;

typedef struct {
  XmIpcEventId eventId;
  uint8_t dialog_id;
  uint8_t state;
} __attribute__((packed)) XpSapPowerStateParams;

typedef struct {
  XmIpcEventId eventId;
  uint16_t state;
} __attribute__((packed)) XpSapStatus;

typedef struct {
  XmIpcEventId eventId;
  uint8_t dialog_id;
  uint16_t power_save_bi_multiplier;
  uint64_t next_tsf;
} __attribute__((packed)) WiFiSapPowerStateParams;

typedef struct {
  macaddr_t mac_addr;
  uint32_t interval;
  uint32_t peroid;
  uint32_t location;
} __attribute__((packed)) XPANTwtSessionParams;

typedef struct {
  XmIpcEventId eventId;
  uint8_t num_devices;
  XPANTwtSessionParams *params;
} __attribute__((packed)) XpTwtParameters;

typedef struct {
  XmIpcEventId eventId;
  uint8_t transport;
} __attribute__((packed)) XpBearerPreference;

typedef struct {
  XmIpcEventId eventId;
  uint32_t delay_reporting;
} __attribute__((packed)) CpDelayReporting;

typedef struct {
  XmIpcEventId eventId;
  bool enable;
  int interval;
} __attribute__((packed)) XmEnableStats;

typedef struct {
  XmIpcEventId eventId;
  TransportType transport;
} __attribute__((packed)) CpTransportUpdate;

typedef struct {
  XmIpcEventId eventId;
  uint8_t numOfDevices;
  bdaddr_t *bdaddr;
} __attribute__((packed)) BondedDeviceList;

typedef union {
  XmIpcEventId eventId;
  /* This entry holds CoP version indication data */
  DhXmCopVerInd CoPInd;
  XmCpLogLvl Loglvl;
  QhciXmRemoteSupportXpan SupportsXpan;
  QhciXmPrepareAudioBearerReq AudioBearerReq;
  QhciXmUseCase UseCase;
  QhciXmUnPrepareAudioBearerReq UnPrepareAudioBearerReq;
  XmQhciUnPrepareAudioBearerRsp UnPrepareAudioBearerRsp;
  XmPrepareAudioBearerRsp AudioBearerRsp;
  XmXpTransportEnabled TransportEnabled;
  XmKpAudioBearerRsp KpAudioBearerRsp;
  XmCpAudioBearerRsp CpAudioBearerRsp;
  XpXmBearerSwitch   XpBearerSwitchInd;
  XmTimeout AudioBearerTimeout;
  XpEnableAcs Acslist;
  XpAcsResults AcsResults;
  XpTwtEvent TwtEvent;
  XpHostParameters HostParams;
  XpCreateSapInf CreateSapInfParams;
  XpCreateSapInfStatus CreateSapInfStatusParams;
  XpSapPowerStateParams SapPowerStateParams;
  XpSapStatus SapState;
  WiFiSapPowerStateParams SapPowerStateRsp;
  XpTwtParameters TwtParams;
  XpBearerPreference BearerPreference;
  CpDelayReporting DelayReporting;
  XmEnableStats EnableStats;
  CpTransportUpdate TransportUpdate;
  BondedDeviceList BondedDevies;
} xm_ipc_msg_t;

typedef struct {
  XmIpcEventId eventId;
  ApiDirection orginator;
  uint8_t waiting_for_rsp;
  bdaddr_t bdaddr;
  bool rx_bearer_ind;
  bool stats_enabled;
} __attribute__((packed)) XmAudioBearerReq;

#define XM_IPC_MSG_SIZE sizeof(xm_ipc_msg_t)

#define XM_XP				0
#define XM_CP 				1
#define XM_KP				2
#define XM_QHCI				3
#define XM_ORG_INVALID                  4
#define XM_AUDIO_BEARER_TIMEOUT		(10000 * 5)

static inline char * ConvertMsgtoString(XmIpcEventId eventId)
{ 
  if (eventId == DH_XM_COP_VER_IND)
    return "DH_XM_COP_VER_IND";
  else if(eventId == XP_XM_HOST_PARAMETERS)
    return "XP_XM_HOST_PARAMETERS";
  else if (eventId == QHCI_XM_REMOTE_SUPPORT_XPAN)
    return "QHCI_XM_REMOTE_SUPPORT_XPAN";
  else if (eventId == XP_XM_TRANSPORT_ENABLED)
    return "XP_XM_TRANSPORT_ENABLED";
  else if (eventId == XP_XM_ENABLE_ACS)
    return "XP_XM_ENABLE_ACS";
  else if (eventId == WIFI_XM_ACS_RESULTS)
    return "WIFI_XM_ACS_RESULTS";
  else if (eventId == QHCI_XM_PREPARE_AUDIO_BEARER_REQ)
    return "QHCI_XM_PREPARE_AUDIO_BEARER_REQ";
  else if(eventId == QHCI_XM_USECASE_UPDATE)
    return "QHCI_XM_USECASE_UPDATE";
  else if (eventId == XP_XM_PREPARE_AUDIO_BEARER_RSP)
    return "XP_XM_PREPARE_AUDIO_BEARER_RSP";
  else if (eventId == KP_XM_PREPARE_AUDIO_BEARER_RSP)
    return "KP_XM_PREPARE_AUDIO_BEARER_RSP";
  else if (eventId == CP_XM_PREPARE_AUDIO_BEARER_RSP)
    return "CP_XM_PREPARE_AUDIO_BEARER_RSP";
  else if (eventId == XM_PREPARE_AUDIO_BEARER_TIMEOUT)
    return "XM_PREPARE_AUDIO_BEARER_TIMEOUT";
  else if (eventId == XP_XM_AUDIO_BEARER_SWITCHED)
    return "XP_XM_AUDIO_BEARER_SWITCHED";
  else if (eventId == XM_CP_LOG_LVL)
    return "XM_CP_LOG_LVL";
  else if (eventId == QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ)
    return "QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ";
  else if (eventId == XM_WIFI_ENABLE_STATS)
    return "XM_WIFI_ENABLE_STATS";
  else if (eventId == XM_WIFI_USECASE_UPDATE)
    return "XM_WIFI_USECASE_UPDATE";
  else if (eventId == CP_XM_TRANSPORT_UPDATE)
    return "CP_XM_TRANSPORT_UPDATE";
  else if (eventId == XP_XM_SAP_CURRENT_STATE)
    return "XP_XM_SAP_CURRENT_STATE";
  else if (eventId == XP_XM_SAP_POWER_STATE)
    return "XP_XM_SAP_POWER_STATE";
  else if (eventId == WIFI_XM_TWT_EVENT)
    return "WIFI_XM_TWT_EVENT";
  else if (eventId == XP_XM_TWT_SESSION_EST)
    return "XP_XM_TWT_SESSION_EST";
  else if (eventId == XP_XM_CREATE_SAP_INF)
    return "XP_XM_CREATE_SAP_INF";
  else if (eventId == WIFI_XM_SAP_POWER_SAVE_STATE_RSP)
    return "WIFI_XM_SAP_POWER_SAVE_STATE_RSP";
  else if (eventId == XM_XP_CREATE_SAP_INF_STATUS)
    return "XM_XP_CREATE_SAP_INF_STATUS";
  else
    return "INVALID Event ID";
}

} // namespace implementation
} // namespace xpan
#endif

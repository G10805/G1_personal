/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <stdint.h>
#include <hidl/HidlSupport.h>
#include "hci_transport.h"
#include "qhci_main.h"
#include "qhci_packetizer.h"
#include "qhci_xm_if.h"
#include "xm_qhci_if.h"
#include "xpan_utils.h"
#include <utils/Log.h>

#define LOG_TAG "vendor.qti.bluetooth@1.0-xpan_qhci_intf"

namespace xpan {
namespace implementation {
XMQhciIf xm_qhci_if;

QHciXmIntf::QHciXmIntf() {
}

QHciXmIntf::~QHciXmIntf() {
}

/******************************************************************
 *
 * Function       RemoteSupportXpanToXm
 *
 * Description    Inform Xpan Manager that remote is supporting
 *                Xpan
 *
 * Arguments      remote bluetooth addres - bd_addr_t
 *                xpan enable or not      - bool
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::RemoteSupportXpanToXm(bdaddr_t bd_addr, bool enable) {
  ALOGD("%s  BD Address %s enable %d ", __func__,
    ConvertRawBdaddress(bd_addr), enable);
  //call XM APi from here
  xm_qhci_if.RemoteSupportsXpan(bd_addr, enable);
  return;
}

/******************************************************************
 *
 * Function       PrepareAudioBearerReqToXm
 *
 * Description    Send the prepare bearer request to the XM
 *
 * Arguments      remote bluetooth addres - bd_addr_t
 *                transport type : 0 - BT
 *                                 1 - XPAN
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::PrepareAudioBearerReqToXm(bdaddr_t bd_addr,
                                                     TransportType transport_type) {
  ALOGD("%s BD Address %s transport_type %s ", __func__,
    ConvertRawBdaddress(bd_addr), TransportTypeToString(transport_type));
  //call XM APi from here
  xm_qhci_if.PrepareAudioBearerReq(bd_addr, transport_type, QHCI_To_XM);

  return;
}

/******************************************************************
 *
 * Function       PrepareAudioBearerReqFromXm
 *
 * Description    Process prepare Audio bearer request from Xm and
 *                send it to the rx queue.
 *
 * Arguments      remote bluetooth addres - bdaddr_t
 *                transport type : 0 - BT
 *                                 1 - XPAN
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::PrepareAudioBearerReqFromXm(bdaddr_t bd_addr,
                                                     TransportType transport_type) {
  ALOGD("%s  BD Address %s TransportType %s ", __func__,
          ConvertRawBdaddress(bd_addr), TransportTypeToString(transport_type));

  //process Prepare Audio Bearer Request from XM
    qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE + 
                                            sizeof(bd_addr));
  if (transport_type == QHCI_BT_TRANSPORT) {
    msg->eventId = QHCI_XM_PREPARE_REQ_BT;
    msg->PreBearerReq.eventId = QHCI_XM_PREPARE_REQ_BT;
  } else {
    msg->eventId = QHCI_XM_PREPARE_REQ;
    msg->PreBearerReq.eventId = QHCI_XM_PREPARE_REQ;
  }
  msg->PreBearerReq.transport_type = transport_type;

  for (int i = 0; i < 6; i++)
    msg->PreBearerReq.bd_addr.b[i] = bd_addr.b[i];

  QHci::Get()->PostMessage(msg);

}

/******************************************************************
 *
 * Function       PrepareAudioBearerRspToXm
 *
 * Description    send prepare Audio bearer response to Xm
 *
 * Arguments      remote bluetooth addres - bdaddr_t
 *                status : false  - Failure
 *                         true   - Success
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::PrepareAudioBearerRspToXm(bdaddr_t bd_addr,
                                                     RspStatus status) {
  ALOGD("%s enable %d ", __func__, status);

  if (status) {
    xm_qhci_if.PrepareAudioBearerRsp(bd_addr, XM_SUCCESS, QHCI_To_XM);
  } else {
    xm_qhci_if.PrepareAudioBearerRsp(bd_addr,
                                     status,
                                     QHCI_To_XM);
  }

  return;
}

/******************************************************************
 *
 * Function       PrepareAudioBearerRspFromXm
 *
 * Description    Process prepare Audio bearer response from Xm
 *                add it to the queue.
 *
 * Arguments      remote bluetooth addres - bd_addr_t
 *                status : false  - Failure
 *                         true   - Success
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::PrepareAudioBearerRspFromXm(bdaddr_t bd_addr,
                                                     RspStatus status) {
  ALOGD("%s enable %d ", __func__, status);

  //process Prepare Audio Bearer response from XM
  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_RSP_SIZE + 
                                            sizeof(bd_addr));

  msg->eventId = QHCI_XM_PREPARE_RSP;
  msg->PreBearerRsp.eventId = QHCI_XM_PREPARE_RSP;
  msg->PreBearerRsp.status = status;

  for (int i = 0; i < 6; i++)
    msg->PreBearerRsp.bd_addr.b[i] = bd_addr.b[i];

  QHci::Get()->PostMessage(msg);

}

/******************************************************************
 *
 * Function       UseCaseUpdateToXm
 *
 * Description    Send Usecase Update to the XM
 *
 *
 * Arguments      usecase- uint8_t
 *
 * return         true/false
 ******************************************************************/
bool QHciXmIntf::UseCaseUpdateToXm(UseCaseType usecase) {
     ALOGE("%s enable %d ", __func__, usecase);
     //call XM APi from here
     usecase = USECASE_XPAN_LOSSLESS;
     xm_qhci_if.UseCaseUpdate(usecase);
     return true;
}

/******************************************************************
 *
 * Function       UseCaseUpdateConfirmFromXm
 *
 * Description    Process usecase update confirmation from XM
 *
 * Arguments      status- true - Success
 *                        false -Failure
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UseCaseUpdateConfirmFromXm(uint8_t usecase, bool status) {
     ALOGE("%s enable %d ", __func__, status);

     return;
}

/******************************************************************
 *
 * Function       UnPrepareAudioBearerReqToXm
 *
 * Description    Send the unprepare bearer audio request to the XM
 *
 * Arguments      remote bluetooth addres - bdaddr_t
 *                transport type : 0 - BT
 *                                 1 - XPAN
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UnPrepareAudioBearerReqToXm(bdaddr_t bd_addr,
                                             TransportType transport_type) {
  ALOGD("%s transport_type %s ", __func__,
          TransportTypeToString(transport_type));
  TransportType next_active_transport = NONE;

  if (transport_type == XPAN)
    next_active_transport = BT;
  else if (transport_type == BT)
    next_active_transport = XPAN;
  //call XM APi from here
  xm_qhci_if.UnPrepareAudioBearerReq(bd_addr, next_active_transport);
  return;
}

/******************************************************************
 *
 * Function       UnPrepareAudioBearerRspFromXm
 *
 * Description    Process UnPrepareAudioBearerRspFromXm
 *
 * Arguments      remote bluetooth addres - bd_addr_t
 *                bool status
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UnPrepareAudioBearerRspFromXm(bdaddr_t bd_addr,
                                                          bool status) {
  ALOGD("%s status %d ", __func__, status);

  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE +
                                            sizeof(bd_addr));
  msg->eventId = QHCI_XM_UNPREPARE_RSP;
  msg->UnPreBearerRsp.eventId = QHCI_XM_UNPREPARE_RSP;
  msg->UnPreBearerRsp.status = status;

  for (int i = 0; i < 6; i++)
    msg->UnPreBearerRsp.bd_addr.b[i] = bd_addr.b[i];

  QHci::Get()->PostMessage(msg);

}

/******************************************************************
 *
 * Function       UnPrepareAudioBearerReqFromXm
 *
 * Description    Process UnPrepareAudioBearerReqFromXm
 *
 * Arguments      remote bluetooth addres - bd_addr_t
 *                transport type : 0 - BT
 *                                 1 - XPAN
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UnPrepareAudioBearerReqFromXm(bdaddr_t bd_addr,
                                             TransportType transport_type) {
  ALOGD("%s transport_type %s ", __func__,
          TransportTypeToString(transport_type));
  //process UnPrepare Audio Bearer Request from XM
    qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE + 
                                            sizeof(bd_addr));

  msg->eventId = QHCI_XM_UNPREPARE_REQ;
  msg->PreBearerReq.eventId = QHCI_XM_UNPREPARE_REQ;
  msg->PreBearerReq.transport_type = transport_type;

  for (int i = 0; i < 6; i++)
    msg->PreBearerReq.bd_addr.b[i] = bd_addr.b[i];

  QHci::Get()->PostMessage(msg);
  
}

/******************************************************************
 *
 * Function       TransportEnabledFromXm
 *
 * Description    Process transportEnable  from XM 
 *
 * Arguments      remote bluetooth addres - bdaddr_t
 *                status- true - Success
 *                        false -Failure
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::TransportEnabledFromXm(bdaddr_t bd_addr, TransportType type,
                                                bool status, uint8_t reason) {
  ALOGD("%s status %d Transport Type %s Reason %d", __func__, status,
                TransportTypeToString(type), reason);
  //process Transport enabled from XM
    qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE + 
                                            sizeof(bd_addr));

  msg->eventId = QHCI_TRANSPORT_ENABLE;
  msg->TransportEnabled.eventId = QHCI_TRANSPORT_ENABLE;
  msg->TransportEnabled.status = reason;

  for (int i = 0; i < 6; i++)
    msg->TransportEnabled.bd_addr.b[i] = bd_addr.b[i];

  QHci::Get()->PostMessage(msg);
}

/******************************************************************
 *
 * Function       UpdateTransportFromXm
 *
 * Description    Process update transport  from XM 
 *
 * Arguments      remote bluetooth addres - bdaddr_t
 *                transport type : 0 - BT
 *                                 1 - XPAN

 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UpdateTransportFromXm(TransportType transport_type) {
  ALOGD("%s transport_type %s ", __func__,
          TransportTypeToString(transport_type));
  //process Update Transport  from XM
  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE);

  msg->eventId = QHCI_UPDATE_TRANSPORT;
  msg->UpdateTransport.eventId = QHCI_UPDATE_TRANSPORT;
  msg->UpdateTransport.transport_type = transport_type;

  QHci::Get()->PostMessage(msg);
}

/******************************************************************
 *
 * Function       DelayReporting
 *
 * Description    Process DelayReporting  from XM 
 *
 * Arguments      delay value
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::DelayReporting(uint32_t delay) {
  ALOGD("%s Delay value %d ", __func__, delay);
  //process Update Transport  from XM
  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE);

  msg->eventId = QHCI_DELAY_REPORT_EVT;
  msg->DelayReport.eventId = QHCI_DELAY_REPORT_EVT;
  msg->DelayReport.delay_report = delay;

  QHci::Get()->PostMessage(msg);

}

/******************************************************************
 *
 * Function       UpdateXpanBondedDevices
 *
 * Description    Process Bonded list of Xpan Devices
 *
 * Arguments      list of bdaddr_t
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::UpdateXpanBondedDevices(uint8_t num_devices, bdaddr_t *) {
  ALOGD("%s ", __func__);
  //Need to reuse the functionality to avoid QLL everytime

}

/******************************************************************
 *
 * Function       BearerSwitchInd
 *
 * Description    Process BearerSwitchInd from XM
 *
 * Arguments      bdaddr_t
 *                uint8_t status
 *
 * return         none
 ******************************************************************/
void QHciXmIntf::BearerSwitchInd(bdaddr_t addr, uint8_t status) {
  ALOGD("%s  status %d", __func__, status);

  //process Update Transport  from XM
  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_AUDIO_BEARER_REQ_SIZE);

  msg->eventId = QHCI_BEARER_SWITCH_IND;
  msg->BearerSwitchInd.eventId = QHCI_BEARER_SWITCH_IND;
  msg->BearerSwitchInd.ind_status = status;

  for (int i = 0; i < 6; i++)
    msg->BearerSwitchInd.bd_addr.b[i] = addr.b[i];

  QHci::Get()->PostMessage(msg);

}

} // namespace implementation
} // namespace xpan



/* 
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <signal.h>
#include "data_handler.h"
#include "qhci_main.h"
#include "qhci_packetizer.h"
#include "qhci_xm_if.h"
#include "xpan_utils.h"
#include <hidl/HidlSupport.h>
#include "hci_transport.h"
//#include "logger.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif

//using ::xpan::implementation::QHci;
#define LOG_TAG "vendor.qti.bluetooth@1.0-xpan_qhci"
using android::hardware::bluetooth::V1_0::implementation::DataHandler;

namespace xpan {
namespace implementation {

std::shared_ptr<QHci> QHci::qhci_main_instance = nullptr;

QHciXmIntf qhci_xm_intf;

QHci::QHci() {
  ALOGD("%s", __func__);
  //logger_ = Logger::Get();
}

QHci::~QHci() {
  ALOGD("%s", __func__);
}

std::shared_ptr<QHci> QHci::Get() {
  if (!qhci_main_instance)
    qhci_main_instance.reset(new QHci());
  return qhci_main_instance;
}

void QHci::QHciMainThreadRoutine() {
  DataHandler *data_handler = DataHandler::Get();

  ALOGD("%s", __func__);
  if (data_handler) {
    //data_handler->QHciIntialized(true);
  } else {
    ALOGE("%s: DataHandler Not initialized", __func__);
    return;
  }

  if (std::atomic_exchange(&main_thread_running, true)) return;

  while (main_thread_running) {

    std::atomic_exchange(&is_main_thread_busy, false);
    std::unique_lock<std::mutex> qhci_lock(qhci_wq_mtx);

    qhci_wq_notifier.wait(qhci_lock);
    qhci_lock.unlock();
    std::atomic_exchange(&is_main_thread_busy, true);

    while(1) {
      qhci_wq_mtx.lock();

      if (qhci_workqueue.empty()) {
        qhci_wq_mtx.unlock();
        break;
      } else {
        qhci_msg_t *data = qhci_workqueue.front();
        qhci_workqueue.pop();
        qhci_wq_mtx.unlock();
        QHciMsgHandler(data);

      }
    }
  }

  ALOGI("%s: is stopped", __func__);

}

void QHci::curr_usr_handler (int) {
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

int QHci::Init() {
  qhci_main_thread = std::thread([this]() {
                      struct sigaction prev_sig, curr_sig;
                      memset(&curr_sig, 0, sizeof(curr_sig));
                      curr_sig.sa_handler = curr_usr_handler;
                      sigaction(SIGUSR1, &curr_sig, &prev_sig);
                      ALOGD("%s: Starting QHCI main thread", __func__);
                      QHciMainThreadRoutine();
                     });
  if (!qhci_main_thread.joinable()) return -1;

  char value_prop[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bt.mtp_mora_testing", value_prop , "false");
  if (strcmp(value_prop, "true") == 0) {
    dbg_mtp_mora_prop = true;
    ALOGD("%s: dbg_mtp_mora_prop %d", __func__, dbg_mtp_mora_prop);
  }

  property_get("persist.vendor.service.bt.mtp_mtp_testing", value_prop , "false");
  if (strcmp(value_prop, "true") == 0) {
    dbg_mtp_mtp_prop = true;
    ALOGD("%s: dbg_mtp_mtp_prop %d", __func__, dbg_mtp_mtp_prop);
  }

  property_get("persist.vendor.service.bt.mtp_mora_nut_testing", value_prop , "false");
  if (strcmp(value_prop, "true") == 0) {
    dbg_mora_nut_prop = true;
    ALOGD("%s: dbg_mora_nut_prop %d", __func__, dbg_mora_nut_prop);
  }
  qhci_set_host_bit = false;

  return 0;
}

int QHci::DeInit() {

  if (!std::atomic_exchange(&main_thread_running, false)) {
    ALOGW("%s: main thread already stopped", __func__);
  }
  ALOGI("%s clearing out pending message", __func__);
  /* Unqueue all the pending messages */
  qhci_wq_mtx.lock();
  while(!qhci_workqueue.empty()) {
    qhci_msg_t *msg = qhci_workqueue.front();
    qhci_workqueue.pop();
  }
  qhci_wq_mtx.unlock();
  {
    std::unique_lock<std::mutex> qhci_lock(qhci_wq_mtx);
    qhci_wq_notifier.notify_all();
  }
  ALOGI("%s cleared pending message", __func__);

  if (qhci_main_thread.joinable()) {
    ALOGD("%s: sending SIGUSR1 signal", __func__);
    pthread_kill(qhci_main_thread.native_handle(), SIGUSR1);
    ALOGD("%s: joining QHCI main thread", __func__);
    qhci_main_thread.join();
    ALOGI("%s: joined QHCI main thread", __func__);
  }

  qhci_set_host_bit = false;

  if (qhci_main_instance) {
    qhci_main_instance.reset();
    qhci_main_instance = NULL;
  }

 return 0;
}

void QHci::PostMessage(qhci_msg_t * msg) {
  ALOGE("%s ", __func__);
  GetMainThreadState();
  if (!main_thread_running) {
    ALOGW("%s: Main worker thread is not ready to process this message: %d",
          __func__, msg->eventId);
    free(msg);
    return;
  }

  qhci_wq_mtx.lock();
  qhci_workqueue.push(msg);
  qhci_wq_mtx.unlock();
  /* Main thread anyways get the data from queue.
   * if it is idle due to no data notify it that 
   * it has data to process in its queue.
   */
  if (!is_main_thread_busy) {
    std::unique_lock<std::mutex> qhci_lock(qhci_wq_mtx);
    qhci_wq_notifier.notify_all();
  }
}

char* QHci::ConvertEventToString(uint8_t eventId) {
  switch(eventId) {
    case QHCI_CSM_LE_CONN_CMPL_EVT:
      return "QHCI_CSM_LE_CONN_CMPL_EVT";
      break;
    case QHCI_CSM_LE_CLOSE_EVT:
      return "QHCI_CSM_LE_CLOSE_EVT";
      break;
    case QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT:
      return "QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT";
      break;
    case QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT:
      return "QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT";
      break;
    case QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT:
      return "QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT";
      break;
    case QHCI_CSM_XPAN_CONN_FAILED_EVT:
      return "QHCI_CSM_XPAN_CONN_FAILED_EVT";
      break;
    case QHCI_CSM_CIS_DISCONNECT_EVT:
      return "QHCI_CSM_CIS_DISCONNECT_EVT";
      break;
    case QHCI_CSM_USECASE_XPAN_TRANS_ENABLE_EVT:
      return "QHCI_CSM_USECASE_XPAN_TRANS_ENABLE_EVT";
      break;
    case QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT:
      return "QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT";
      break;
    case QHCI_CSM_UPDATE_TRANS_XPAN_EVT:
      return "QHCI_CSM_UPDATE_TRANS_XPAN_EVT";
      break;
    case QHCI_CSM_PREPARE_BEARER_BT:
      return "QHCI_CSM_PREPARE_BEARER_BT";
      break;
    case QHCI_CSM_PREPARE_BEARER_XPAN:
      return "QHCI_CSM_PREPARE_BEARER_XPAN";
      break;
    default:
      ALOGW("%s: Invalid EVENT ID", __func__);
  }
  return NULL;
}

char* QHci::ConvertMsgtoString(uint8_t state) {
  switch(state) {
    case QHCI_IDLE_STATE:
      return "QHCI_IDLE_STATE";
      break;
    case QHCI_BT_CLOSE_XPAN_CLOSE:
      return "QHCI_BT_CLOSE_XPAN_CLOSE";
      break;
    case QHCI_BT_CLOSE_XPAN_CONNECTING:
      return "QHCI_BT_CLOSE_XPAN_CONNECTING";
      break;
    case QHCI_BT_CLOSE_XPAN_OPEN:
      return "QHCI_BT_CLOSE_XPAN_OPEN";
      break;
    case QHCI_BT_OPEN_XPAN_CLOSE:
      return "QHCI_BT_OPEN_XPAN_CLOSE";
      break;
    case QHCI_BT_OPEN_XPAN_CONNECTING:
      return "QHCI_BT_OPEN_XPAN_CONNECTING";
      break;
    case QHCI_BT_OPEN_XPAN_OPEN:
      return "QHCI_BT_OPEN_XPAN_OPEN";
      break;
    /* TODO Fill Remaining */
    default:
      ALOGW("%s: Invalid State %d", __func__, state);
  }
  return NULL;
}

bool QHci::ProcessRxLogger(qhci_msg_t * msg) {
  return true;
}

void QHci::SendRxPktToHost(qhci_msg_t *msg) {
  DataHandler *data_handler = DataHandler::Get();
  ALOGD("%s  ", __func__);
  if (data_handler) {
#if 0
    if (!ProcessRxLogger(msg)) {
      free(msg->RxEvtPkt.hidl_data);
      return;
    }
#endif
    hidl_vec<uint8_t> *hidl_data = new hidl_vec(*msg->RxEvtPkt.hidl_data);
#ifdef XPAN_SUPPORTED
    data_handler->OnPacketReadyFromQHci(msg->RxEvtPkt.type,
                                        hidl_data, false);
#endif
    free(msg->RxEvtPkt.hidl_data);
    ALOGD("%s After free", __func__);
  }
}

void QHci::QHciProcessConnCmplEvt(qhci_msg_t *msg) {
  ALOGD("%s  ", __func__);
  qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(msg->ConnCmpl.handle);
  if (rem_info) {
    QHciSmExecute(rem_info, QHCI_CSM_LE_CONN_CMPL_EVT);
  } else {
    ALOGD("%s  Not a valid ConnCmplEvt", __func__);
  }
}

void QHci::QHciSendVndrQllEvtMask() {
    ALOGD("%s  ", __func__);

    uint8_t qhci_evt_pkt[7]
      = {0xe, 0x5, 0x1, 0x51, 0xfc, 0x00, 0xf};

    DataHandler *data_handler = DataHandler::Get();
    if (data_handler) {
#ifdef XPAN_SUPPORTED
      hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
      hidl_data->resize(7);
      memcpy(hidl_data->data(), qhci_evt_pkt, 7);
      data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                          hidl_data, false);
#endif
    }

}

void QHci::QHciProcessRxPktEvt(qhci_msg_t *msg) {
  ALOGD("%s msg->RxEvtPkt.hidl_data->size() %d ", __func__, msg->RxEvtPkt.hidl_data->size());
  const uint8_t* data = msg->RxEvtPkt.hidl_data->data();

    if (data[0] == QHCI_COMMAND_COMPLETE_EVENT) {
    uint16_t opcode = ((data[4] << 8) | (data[3]));
    if (opcode == 0xFC51) {
      if (data[6] == 0x0F) {
        ALOGD("%s HCI_LE_VENDOR_SPECIFIC_EVENT Params ", __func__);
        QHciSetHostSupportedBit();
        return;
      }
      if (data[6] == 0x14) {
        ALOGD("%s HCI_VS_QBCE_QLE_SET_HOST_FEATURE status %d", __func__, data[5]);
        return;
      }
    }
  }

  if (data[0] == HCI_ENCRYPT_CHANGE_EVT) {
    //If its xpan supported version
    uint16_t handle = (data[4] << 8 | data[3]);
    ALOGD("%s HCI_ENCRYPT_CHANGE_EVT handle %d", __func__, handle);
    //Store the key in the map
    encrypt_event_pending = true;
  }
  if (data[0] == HCI_READ_REMOTE_VERSION_CMPL_EVT) {
    //Add the blocket call for this
    if (qhci_wait_for_qll_event) {
      ALOGD("%s Sending QLL", __func__);
      memcpy(&qhci_remote_version_data, msg->RxEvtPkt.hidl_data->data(),
              msg->RxEvtPkt.hidl_data->size());
      for (int i = 0; i < msg->RxEvtPkt.hidl_data->size(); i++)
        ALOGD("%s 0x%2x", __func__, qhci_remote_version_data[i]);
      uint16_t handle = (data[4] << 8 | data[3]);
      //QHciProcessQllReq(handle);
      return;
    }
  }

  if (data[0] == HCI_DISCONNECT_CMPL_EVT) {
    ALOGD("%s HCI_DISCONNECT_CMPL_EVT ", __func__);
    //parse the handle
    uint16_t handle = ((data[4] << 8) | (data[3]));
    qhci_dev_cb_t* rem_info = GetQHciRemoteDeviceInfo(handle);

    if (is_cis_handle_disc_pending) {
      is_cis_handle_disc_pending = false;
      if (rem_info) {
        ALOGD("%s CIS HANDLE DISCONNECTED state %s", __func__,
              ConvertMsgtoString(rem_info->state));
        rem_info->is_cis_disconnect_pending_from_soc = false;
        return;
      }
    }

    std::vector<uint16_t>::iterator it2;
    it2 = std::find(active_cis_handles.begin(), active_cis_handles.end(),
                   handle);
    if (it2 != active_cis_handles.end()) {
      ALOGD("%s CIS HANDLE DISCONNECT_CMPL_EVT is for XPAN ACTIVE DEVICE",
        __func__);
      active_cis_handles.erase(it2);
      QHciSmExecute(rem_info, QHCI_CSM_CIS_DISCONNECT_EVT);
    }
  }

  if (data[0] == HCI_LE_EVT) {
    if (data[2] == HCI_LE_CIS_ESTABLISHED_EVT) {
      ALOGD("%s HCI_LE_CIS_ESTABLISHED_EVT from SOC ", __func__);
      if (pending_cis_est_evt == 2) {
        pending_cis_est_evt = 0;
        if (data[3] == QHCI_BT_SUCCESS) {
          uint16_t acl_handle = QHciBDAddrToHandleMap(pending_bd_addr_cis);
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(acl_handle);
          if (rem_info) {
            ALOGD("%s HCI_LE_CIS_ESTABLISHED_EVT QHCI state %s ", __func__, 
                    ConvertMsgtoString(rem_info->state));
            rem_info->state = QHCI_BT_OPEN_XPAN_OPEN;
            qhci_xm_intf.PrepareAudioBearerRspToXm(pending_bd_addr_cis, XM_SUCCESS);
          }
        } else {
          qhci_xm_intf.PrepareAudioBearerRspToXm(pending_bd_addr_cis, XM_FAILED);
          qhci_bearer_switch_pending = false;
        }
      }
    }
  }
}

void QHci::QHciSendDisconnectCmdStatus() {
  ALOGE("%s  ", __func__);

  uint8_t qhci_evt_pkt[6]
    = {0x0f, 0x04, 0x00, 0x01, 0x06, 0x04};

  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(6);
    memcpy(hidl_data->data(), qhci_evt_pkt, 6);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  }
}

void QHci::QHciSendDisconnectCmplt(uint16_t handle) {
  ALOGE("%s  ", __func__);

  uint8_t qhci_evt_pkt[6]
    = {0x5, 0x4, 0x0, 0x0, 0x0, 0x16};

  qhci_evt_pkt[3] = 0x00FF & handle;
  qhci_evt_pkt[4] = 0xFF00 & handle;
  
  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(6);
    memcpy(hidl_data->data(), qhci_evt_pkt, 6);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  }
}

void QHci::QHciUnprepareAudioBearerRspfromXm(bdaddr_t bd_addr,
                                                          bool status) {
  ALOGD("%s  ", __func__);
  prep_bearer_active = false;
  QHciSendDisconnectCmplt(cig_params.cis_handles[0]);
  QHciSendDisconnectCmplt(cig_params.cis_handles[1]);
}


void QHci::QHciSendUsecaseUpdateCfm(uint8_t usecase) {
  ALOGE("%s  ", __func__);

  uint8_t vs_qhci_evt_pkt[VSC_QHCI_VENDOR_USECASE_EVT_LEN]
    = {0xe, 0x7, 0x1, 0x0a, 0xfc, 0x15, 0xFF, 0xFF, 0xFF};
  if ((usecase == (uint8_t) XPAN_WIFI_HQ_PROFILE)
    || (usecase == (uint8_t) XPAN_WIFI_GAMING_PROFILE)) {
    is_xpan_supported = true;
  } else {
    is_xpan_supported = false;
  }

  vs_qhci_evt_pkt[8] = usecase;
  
  ALOGD("%s %d DBGG usecase ", __func__, vs_qhci_evt_pkt[8]);
  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(VSC_QHCI_VENDOR_USECASE_EVT_LEN);
    memcpy(hidl_data->data(), vs_qhci_evt_pkt, VSC_QHCI_VENDOR_USECASE_EVT_LEN);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  }
}

void QHci::QHciUseCaseUpdateEvt(uint8_t usecase) {
  ALOGE("%s: ++++ Updating usecase ", __func__);

  uint8_t vs_qhci_evt_pkt[QHCI_DELAY_REPORT_EVT_LEN]
    = {0xFF, 0x4, 0x12, 0xFF, 0xFF, 0xFF};

  vs_qhci_evt_pkt[5] = usecase;

  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(6);
    memcpy(hidl_data->data(), vs_qhci_evt_pkt, 6);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  }
}


void QHci::QHciDelayReportingEvt(qhci_msg_t *msg) {
  ALOGE("%s: ++++ Sending Delay reporting ", __func__);

  uint8_t vs_qhci_evt_pkt[QHCI_DELAY_REPORT_EVT_LEN]
    = {0xFF, 0x4, 0x12, 0xFF, 0xFF, 0xFF};

  vs_qhci_evt_pkt[3] = msg->DelayReport.delay_report & 0x00FF;
  vs_qhci_evt_pkt[4] = msg->DelayReport.delay_report & 0xFF00;

  ALOGE("%s: ++++ Sending Delay reporting value %d", __func__,
    ((vs_qhci_evt_pkt[4] << 8) | vs_qhci_evt_pkt[3]));
  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(6);
    memcpy(hidl_data->data(), vs_qhci_evt_pkt, 6);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  }
}

void QHci::GetMainThreadState(void)
{
  ALOGD("%s: main_thread_running :%d ", __func__, main_thread_running.load());
  ALOGD("%s:  qhci_workqueue:%p qhci_wq_mtx :%p qhci_wq_notifier %p", __func__, &
qhci_workqueue, &qhci_wq_mtx, &qhci_wq_notifier);
}


/******************************************************************
 *
 * Function       QHciParseUsecaseUpdateCmd
 *
 * Description    process the usecase update cmd and send it to the
 *                XM Manager
 *
 *
 * Arguments      msg- Process the message based on eventId
 *
 * return         none
 ******************************************************************/
void QHci::QHciParseUsecaseUpdateCmd(qhci_msg_t *msg) {

  UseCaseType usecase_update_to_xm = USECASE_XPAN_NONE;
  uint8_t usecase_update_to_stack = 0;
  uint8_t recv_context_type = msg->TxUsecaseRcvd.context_type;

  if (xpan_active_devices_.size() > 0) {
    uint16_t handle = QHciBDAddrToHandleMap(curr_bd_addr);
    qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
    if (rem_info && rem_info->transport_enable) {
      if (recv_context_type == GAME_USECASE) {
        usecase_update_to_stack = LE_UNICAST_GAMING_PROFILE;
        usecase_update_to_xm = USECASE_XPAN_NONE;
      } else if (recv_context_type == HD_USECASE) {
        usecase_update_to_stack = XPAN_WIFI_HQ_PROFILE;
        usecase_update_to_xm = USECASE_XPAN_LOSSLESS;
      }
      ALOGW("%s Transport_enabled: usecase_update_to_stack %d"
            "usecase_update_to_xm %d", __func__, usecase_update_to_stack,
            usecase_update_to_xm);
    } else {
      if (recv_context_type == GAME_USECASE) {
        usecase_update_to_stack = LE_UNICAST_GAMING_PROFILE;
        usecase_update_to_xm = USECASE_XPAN_NONE;
      } else if (recv_context_type == HD_USECASE) {
        usecase_update_to_stack = LE_UNICAST_HQ_PROFILE;
        usecase_update_to_xm = USECASE_XPAN_NONE;
      }
    }
  } else {
    if (recv_context_type == GAME_USECASE) {
      usecase_update_to_stack = LE_UNICAST_GAMING_PROFILE;
      usecase_update_to_xm = USECASE_XPAN_NONE;
    } else if (recv_context_type == HD_USECASE) {
      usecase_update_to_stack = LE_UNICAST_HQ_PROFILE;
      usecase_update_to_xm = USECASE_XPAN_NONE;
    }
  }
  if (dbg_mtp_mtp_prop) {
    usecase_update_to_stack = XPAN_WIFI_HQ_PROFILE;
    usecase_update_to_xm = USECASE_XPAN_LOSSLESS;
  }
  ALOGW("%s usecase_update_to_stack %d usecase_update_to_xm %d", __func__,
         usecase_update_to_stack, usecase_update_to_xm);

  QHciSendUsecaseUpdateCfm(usecase_update_to_stack);
  QHciUseCaseUpdateEvt(usecase_update_to_stack);

  if (usecase_type_to_xm != usecase_update_to_xm) {
    usecase_type_to_xm = usecase_update_to_xm;
    //Send to XPAN Manager about usecase update
    qhci_xm_intf.UseCaseUpdateToXm(usecase_update_to_xm);
  }

}

void QHci::QHciProcessQllReq(uint16_t handle) {
  ALOGD("%s ", __func__);

  uint8_t  pkt[QHCI_VS_QLL_CMD_REQ_LEN] = {0};
  //Opcode
  pkt[0] = 0x51;
  pkt[1] = 0xFC;

  pkt[2] = 3; //length
  pkt[3] = QHCI_VS_QBCE_READ_REMOTE_QLL_SUPPORTED_FEATURES;

  pkt[4] = (handle & 0x00FF);
  pkt[5] = (handle & 0xFF00);

  qhci_qll_req_sent = true;
  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
    if (data_handler->GetControllerRef() != nullptr)
      ALOGD("%s  Sending data to the soc", __func__);
      data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                 pkt, QHCI_VS_QLL_CMD_REQ_LEN);
  }

}

/******************************************************************
 *
 * Function       QHciMsgHandler
 *
 * Description    prasing the message which was scheduled in qhci
 *                main thread
 *
 *
 * Arguments      msg- Process the message based on eventId
 *
 * return         none
 ******************************************************************/
void QHci::QHciMsgHandler(qhci_msg_t *msg) {
  QHciEventId eventId = msg->eventId;
  ALOGI("%s: processing event ", __func__);

  switch (eventId) {
    case QHCI_QLL_CMD_REQ:
      {
        ALOGI("%s: QHCI_QLL_CMD_REQ ", __func__);
        QHciProcessQllReq(msg->QllCmd.handle);
        break;
      }
    case QHCI_CIS_CONN_CMD:
      {
        qhci_packetizer.ProcessMessage(msg);
        break;
      }
    case QHCI_CIS_CONN_EVT:
      {
        qhci_packetizer.ProcessMessage(msg);
        break;
      }
    case QHCI_LE_CONN_CMPL_EVT:
      {
        QHciProcessConnCmplEvt(msg);
        break;
      }
    case QHCI_ACL_DISCONNECT:
      {
        QHciProcessRxPktEvt(msg);
        break;
      }
    case QHCI_XM_PREPARE_REQ:
      {
        uint16_t handle = QHciBDAddrToHandleMap(msg->PreBearerReq.bd_addr);
        qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
        ALOGD("%s: Event QHCI_XM_PREPARE_REQ handle %d", __func__, handle);
        if (rem_info) {
          ALOGD("%s: is_cis_established %d qhci_state == %s ", __func__,
                 rem_info->is_cis_established,
                 ConvertMsgtoString(rem_info->state));
          if (rem_info->state == QHCI_BT_CLOSE_XPAN_CLOSE) {
            QHciSmExecute(rem_info, QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT);
          } else if (rem_info->state == QHCI_BT_OPEN_XPAN_CLOSE){
            QHciSmExecute(rem_info, QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT);
          } else {
            ALOGE("%s: Prepare transport Request received at wrong time ", __func__);
            qhci_xm_intf.PrepareAudioBearerRspToXm(msg->PreBearerReq.bd_addr,
                                                  XM_FAILED_WRONG_TRANSPORT_TYPE_REQUESTED);
          }
        }
        break;
      }
    case QHCI_XM_PREPARE_REQ_BT:
      {
        uint16_t handle = QHciBDAddrToHandleMap(msg->PreBearerReq.bd_addr);
        qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
        if (rem_info) {
          if (GetQHciState(rem_info) == QHCI_BT_CLOSE_XPAN_OPEN) {
            pending_bd_addr_cis = msg->PreBearerReq.bd_addr;
            ALOGI("%s: Event QHCI_CSM_PREPARE_BEARER_BT ", __func__);
            QHciSmExecute(rem_info, QHCI_CSM_PREPARE_BEARER_BT);
          } else if (GetQHciState(rem_info) == QHCI_BT_OPEN_XPAN_CLOSE) {
            if (rem_info->current_transport == BT) {
              qhci_xm_intf.PrepareAudioBearerRspToXm(msg->PreBearerReq.bd_addr,
                                XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT);
            } else {
              ALOGE("%s: QHCI in wrong state with wrong transport ", __func__);
            }
          } else {
            ALOGE("%s: QHCI SM is in wrong state ", __func__);
          }
        } else {
          ALOGE("%s: QHCI BAD State for  QHCI_XM_PREPARE_REQ_BT ", __func__);
        }
        break;
      }
    case QHCI_XM_PREPARE_RSP:
      {
        uint16_t handle = QHciBDAddrToHandleMap(msg->PreBearerRsp.bd_addr);
        ALOGD("%s: QHCI_XM_PREPARE_RSP handle %d", __func__, handle);
        if (handle != 0) {
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
          if (rem_info) {
            if (msg->PreBearerRsp.status == QHCI_BT_SUCCESS) {
              prep_bearer_active = true;
              // sending cis established event to stack.
              ALOGD("%s: QHCI_XM_PREPARE_RSP SUCCESS %d", __func__, handle);
              rem_info->state = QHCI_BT_CLOSE_XPAN_OPEN;
              QHciSendCisEstablishedEvt(rem_info, QHCI_BT_SUCCESS, 0);
              QHciSendCisEstablishedEvt(rem_info, QHCI_BT_SUCCESS, 1);
              if (dbg_mtp_mora_prop) {
                ALOGD("%s: DBG MORA PROP ", __func__);
                uint16_t dbg_handle = QHciBDAddrToHandleMap(curr_bd_addr);
                qhci_dev_cb_t *dbg_rem_info
                    = GetQHciRemoteDeviceInfo(dbg_handle);
                if (dbg_rem_info) {
                  dbg_rem_info->state = QHCI_BT_CLOSE_XPAN_OPEN;
                } else {
                  ALOGE("%s: DBG MORA is not correctly connected ", __func__);
                }
              }
            } else {
              ALOGE("%s: QHCI_XM_PREPARE_RSP FAILED %d", __func__, handle);
              QHciSmExecute(rem_info, QHCI_CSM_XPAN_CONN_FAILED_EVT);
            }
          }
        }
        break;
      }
    case QHCI_PROCESS_RX_PKT_EVT:
      {
        QHciProcessRxPktEvt(msg);
        break;
      }
    case QHCI_USECASE_UPDATE_CFM:
      {
        QHciSendUsecaseUpdateCfm(3);
        break;
      }
    case QHCI_XM_UNPREPARE_REQ:
      {
        qhci_packetizer.ProcessMessage(msg);
        break;
      }
    case QHCI_XM_UNPREPARE_RSP:
      {
        QHciUnprepareAudioBearerRspfromXm(msg->UnPreBearerRsp.bd_addr,
                                                msg->UnPreBearerRsp.status);
        break;
      }
    case QHCI_TRANSPORT_ENABLE:
      {
        uint16_t handle = QHciBDAddrToHandleMap(msg->TransportEnabled.bd_addr);
        ALOGE("%s: QHCI_TRANSPORT_ENABLE handle %d", __func__, handle);
        if (handle != 0) {
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
          if (rem_info) {
            if (msg->TransportEnabled.status == QHCI_BT_SUCCESS) {
              rem_info->transport_enable = true;
              if (GetQHciState(rem_info) != QHCI_BT_CLOSE_XPAN_CLOSE) {
                if (!rem_info->is_cis_established) {
                  QHciSmExecute(rem_info,
                                QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT);
                } else {
                  ALOGE("%s: QHCI_TRANSPORT_ENABLE Wrong state to handle",
                    __func__);
                  //TODO Currently its below event not handling
                  //QHciSmExecute(rem_info, QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT);
                }
              }
            }
          }
        }
        break;
      }
    case QHCI_UPDATE_TRANSPORT:
      {
        uint16_t handle = QHciBDAddrToHandleMap(curr_bd_addr);
        ALOGE("%s: QHCI_UPDATE_TRANSPORT handle %d", __func__, handle);
        if (handle != 0) {
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
          if (msg->UpdateTransport.transport_type == QHCI_XPAN_TRANSPORT) {
            rem_info->update_transport = QHCI_XPAN_TRANSPORT;
            if ((GetQHciState(rem_info) == QHCI_BT_CLOSE_XPAN_CONNECTING)
               || (GetQHciState(rem_info) == QHCI_BT_OPEN_XPAN_CONNECTING)) {
              QHciSmExecute(rem_info, QHCI_CSM_UPDATE_TRANS_XPAN_EVT);
            } else {
              ALOGE("%s: Update transport received in wrong state ", __func__);
            }
          } else {
            if ((GetQHciState(rem_info) == QHCI_BT_OPEN_XPAN_OPEN)) {
              QHciSmExecute(rem_info, QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT);
            }
          }
        }
        break;
      }
    case QHCI_DELAY_REPORT_EVT:
      {
        ALOGE("%s: ++++ Sending Delay reporting ", __func__);
        QHciDelayReportingEvt(msg);
        break;
      }
    case QHCI_USECASE_UPDATE_CMD:
      {
        QHciParseUsecaseUpdateCmd(msg);
        break;
      }
    case QHCI_BEARER_SWITCH_IND:
      {
        uint16_t handle = QHciBDAddrToHandleMap(msg->BearerSwitchInd.bd_addr);
        if (handle != 0) {
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(handle);
          if (rem_info) {
            ALOGI("%s: QHCI_BEARER_SWITCH_IND QHci state %s", __func__,
                   ConvertMsgtoString(rem_info->state));
            if (msg->BearerSwitchInd.ind_status == XM_SUCCESS) {
              switch (rem_info->state) {
                case QHCI_BT_OPEN_XPAN_OPEN:
                {
                  QHciSmExecute(rem_info,
                                QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT);
                }
                  break;
                case QHCI_BT_OPEN_XPAN_CONNECTING:
                {
                  QHciSmExecute(rem_info,
                                QHCI_CSM_UPDATE_TRANS_XPAN_EVT);
                }
                  break;
                default:
                  ALOGI("%s: Its not valid for QHCI_BEARER_SWITCH_IND in the"
                         "state %s", __func__,
                         ConvertMsgtoString(rem_info->state));
              }
            } else {
              ALOGE("%s: QHCI_BEARER_SWITCH_IND Failed and stay in current"
                   "transport", __func__, rem_info->current_transport);
              switch (rem_info->state) {
                case QHCI_BT_OPEN_XPAN_OPEN:
                {
                  rem_info->state = QHCI_BT_CLOSE_XPAN_OPEN;
                  rem_info->is_prepare_bearer_triggered = false;
                  qhci_bearer_switch_pending = false;
                }
                  break;
                case QHCI_BT_OPEN_XPAN_CONNECTING:
                {
                  QHciSmExecute(rem_info,
                                QHCI_CSM_XPAN_CONN_FAILED_EVT);
                }
                  break;
                default:
                  ALOGD("%s: Its not valid for QHCI_BEARER_SWITCH_IND in the"
                         "state %s", __func__,
                         ConvertMsgtoString(rem_info->state));
              }
            }
          }
        } else {
          ALOGE("%s: QHCI_BEARER_SWITCH_IND Wrong handle value", __func__);
        }
      }
      break;
    default:
      ALOGD("%s: UnKnown Event ", __func__);
  }
  free(msg);

}

void QHci::FromDataHandler() {
  ALOGD("%s: Inside QHCI data handling ", __func__);
  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_PKT_MESSAGE_LENGTH);
  msg->eventId = QHCI_DELAY_REPORT_EVT;
  msg->DelayReport.eventId = QHCI_DELAY_REPORT_EVT;
  PostMessage(msg);
}

void QHci::QHciSendCmdStatusForCis() {
  ALOGD("%s ", __func__);

  uint8_t qhci_cmd_status[QHCI_CMD_STATUS_CIS_LEN]
      = {0x0f, 0x04, 0x00, 0x01, 0x64, 0x20};

  ALOGD("%s Sending Status", __func__);

  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
#ifdef XPAN_SUPPORTED
    ALOGD("%s Sending Cis Command status to stack ", __func__);
    hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
    hidl_data->resize(QHCI_CMD_STATUS_CIS_LEN);
    memcpy(hidl_data->data(), qhci_cmd_status, QHCI_CMD_STATUS_CIS_LEN);
    data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                        hidl_data, false);
#endif
  } else {
    ALOGE("%s Command Status couldnt send data_handler is null", __func__);
  }
}

/******************************************************************
 *
 * Function       IsQhciTxPkt
 *
 * Description    prasing the hci TX pkt and checks whether it needs
 *                QHci module interaction or not?
 *
 *
 * Arguments      *data- contains the tx packet
 *                length - size of the packet
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQhciTxPkt(const uint8_t *data, size_t length) {
  uint16_t opcode = (data[1] << 8 | data[0]);

  uint8_t status = false;

  switch (opcode) {
    case HCI_VENDOR_USECASE_UPDATE:
      if (data[3] == VSC_QHCI_VENDOR_OPCODE) {
        status = true;
      } else {
        status = false;
      }
      break;
    case HCI_LE_SET_CIG_PARAMETERS:
      break;
    case HCI_LE_SETUP_ISO_DATA_PATH:
      break;
    case HCI_LE_CREATE_CIS:
      {
        ALOGD("%s HCI_LE_CREATE_CIS ", __func__);
        if (is_cis_conn_prog) {
          ALOGD("%s ALready CIS IS GOING ON IGNORE HCI_LE_CREATE_CIS ", __func__);
          status = false;
          break;
        }
           if (is_xpan_supported) {
           ALOGD("%s HCI_LE_CREATE_CIS  CIS Count %d ", __func__, data[3]);
           uint16_t acl_handle = 0;
           is_cis_conn_prog = true;
          // for one cis
          if (data[3] == 1) {
            acl_handle = (data[7] << 8 | data[6]);
            if (IsQHciXpanSupportedDevice(acl_handle)) {
              status = true;
              qhci_progress_cis_cmd = true;
            }
          } else if (data[3] == 2) {
              acl_handle = (data[7] << 8 | data[6]);
              cig_params.cis_handles[0] = (data[5] << 8 | data[4]);
              cig_params.cis_handles[1] = (data[9] << 8 | data[8]);
              cig_params.cis_count = 2;
              ALOGD("%s HCI_LE_CREATE_CIS  ACL HANDLE %d ", __func__, acl_handle);
              ALOGD("%s HCI_LE_CREATE_CIS  CIS Handles %d %d ", __func__,
                      cig_params.cis_handles[0], cig_params.cis_handles[1]);
              if (IsQHciXpanSupportedDevice(acl_handle)) {
                status = true;
                qhci_progress_cis_cmd = true;
              }
          }
        }
      }
      break;
    case HCI_DISCONNECT:
      {
      ALOGD("%s HCI_DISCONNECT ", __func__);
      uint16_t handle = (data[4] << 8 | data[3]);
      if (prep_bearer_active) {
         if ((cig_params.cis_handles[0] == handle) ||
              (cig_params.cis_handles[1] == handle)) {
            status = true;
            QHciSendDisconnectCmdStatus();
         }
      } else {
        if ((cig_params.cis_handles[0] == handle) ||
            (cig_params.cis_handles[1] == handle)) {
            uint16_t acl_handle = QHciGetMappingAclHandle(handle);
            qhci_dev_cb_t* rem_info = GetQHciRemoteDeviceInfo(acl_handle);
            if (rem_info) {
              rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
            }
        }
      }
      is_cis_conn_prog = false;
      }
      break;
    case HCI_LE_REMOVE_CIG:
      status = false;
      break;
    default:
      status = false;
  }

  return status;

}

void QHci::QHciClearRemoteDeviceInfo(uint16_t handle) {
  ALOGD("%s ", __func__);

  for (auto i = qhci_xpan_dev_db.begin(); i != qhci_xpan_dev_db.end(); ++i) {
    if (handle == i->handle) {
      ALOGD("%s Clearing the db for handle %d", __func__, handle);
      qhci_xpan_dev_db.erase(i);
      i--;
    }
  }
}
qhci_dev_cb_t* QHci::GetQHciRemoteDeviceInfo(uint16_t handle) {
  //Get Remote info from the active XPAN devices

  ALOGD("%s ", __func__);
  for (int i = 0; i < qhci_xpan_dev_db.size(); i++) {
    if (handle == qhci_xpan_dev_db[i].handle) {
      ALOGD("%s Remote device matched in database ", __func__);
      return &qhci_xpan_dev_db[i];
    }
  }
  return NULL;
}

/******************************************************************
 *
 * Function       ProcessTxPktCmd
 *
 * Description    Processing the TX pkt which are needed in qhci
 *                main thread.
 *
 *
 * Arguments      *data- contains the tx packet
 *                length - size of the packet
 *
 * return         none
 ******************************************************************/
void QHci::ProcessTxPktCmd(const uint8_t *data, size_t length) {

  ALOGD("%s ", __func__);
  uint16_t opcode = (data[1] << 8 | data[0]);

  if (opcode == HCI_VENDOR_USECASE_UPDATE) {
    if (data[3] == VSC_QHCI_VENDOR_OPCODE) {
      ALOGE("%s Usecase Update length %d", __func__, length);
      qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_PKT_MESSAGE_LENGTH);
      msg->eventId = QHCI_USECASE_UPDATE_CMD;
      msg->TxUsecaseRcvd.eventId = QHCI_USECASE_UPDATE_CMD;
      msg->TxUsecaseRcvd.length = data[2];
      msg->TxUsecaseRcvd.opcode = data[3];
      msg->TxUsecaseRcvd.context_type = (UseCaseType)data[4];
      PostMessage(msg);
      return;
    }
  }

  if (opcode == HCI_DISCONNECT) {
    uint16_t handle = (data[4] << 8 | data[3]);
    ALOGD("%s HCI_DISCONNECT %d", __func__, handle);
    if ((cig_params.cis_handles[0] == handle) ||
         (cig_params.cis_handles[1] == handle)) {
         ALOGD("%s CIS_DISCONNECT %d", __func__, handle);
         uint16_t acl_handle = QHciBDAddrToHandleMap(curr_bd_addr);
         qhci_dev_cb_t *rem_qhci_dev = GetQHciRemoteDeviceInfo(acl_handle);
         cig_params.cis_count--;
         if (cig_params.cis_count <= 0) {
            if (rem_qhci_dev) {
              QHciSmExecute(rem_qhci_dev, QHCI_CSM_CIS_DISCONNECT_EVT);
            } else {
              ALOGD("%s QHCI DB is bad state. Incorrect handle %d",
                __func__, acl_handle);
            }
         }
         //QHciSendDisconnectCmplt(handle);
    }
  }

  if (opcode == HCI_LE_CREATE_CIS) {
    ALOGD("%s HCI_LE_CREATE_CIS %d", __func__, qhci_progress_cis_cmd);
    if (qhci_progress_cis_cmd) {
      qhci_progress_cis_cmd = false;
      uint16_t acl_handle = 0;
      uint8_t cis_count = data[3];
      uint16_t cis_handle[2] = {0};
      if (cis_count == 1) {
        acl_handle = (data[7] << 8 | data[6]);
        cis_handle[0] = (data[5] << 8 | data[4]);
        cis_acl_handle_map.insert({cis_handle[0], acl_handle});
      } else {
        cis_handle[0] = (data[5] << 8 | data[4]);
        acl_handle = (data[7] << 8 | data[6]);
        cis_handle[1] = (data[9] << 8 | data[8]);
        cis_acl_handle_map.insert({cis_handle[0], acl_handle});
        cis_acl_handle_map.insert({cis_handle[1], acl_handle});
      }
      ALOGD("%s HCI_LE_CREATE_CIS CIS count %d ACL HANDLE %d", __func__, 
             cis_count, acl_handle);

      //TODO Remove this code when testing with actual earbud
      acl_handle = QHciBDAddrToHandleMap(curr_bd_addr);
      qhci_dev_cb_t *rem_qhci_dev = GetQHciRemoteDeviceInfo(acl_handle);
      if (rem_qhci_dev) {
        QHCI_CSM_STATE qhci_state = GetQHciState(rem_qhci_dev);
        ALOGD("%s HCI_LE_CREATE_CIS  GetQHciState %s transport_enable %d",
                __func__, ConvertMsgtoString(qhci_state), rem_qhci_dev->transport_enable);
        if (dbg_mtp_mora_prop) {
          bool dbg_st = true;
          for (int i = 0; i < 6; i++) {
            if (curr_bd_addr.b[i] != dbg_curr_bd_addr[1].b[i]) {
              dbg_st = false;
              break;
            }
          }
          if (dbg_st) {
            ALOGD("%s HCI_LE_CREATE_CIS ON Mora", __func__);
            rem_qhci_dev->transport_enable = true;
          }
        }
        if (rem_qhci_dev->transport_enable) {
          QHciSendCmdStatusForCis();
          rem_qhci_dev->active_cis_handle = (data[5] << 8 | data[4]);
          rem_qhci_dev->is_create_cis_from_stack = true;
          rem_qhci_dev->current_transport = XPAN;
          if (qhci_state == QHCI_BT_CLOSE_XPAN_CLOSE)
            QHciSmExecute(rem_qhci_dev, QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT);
          else
            ALOGE("%s qhci_state wrong state, state %s", __func__,
                    ConvertMsgtoString(qhci_state));
        } else {
          rem_qhci_dev->active_cis_handle = (data[5] << 8 | data[4]);
          rem_qhci_dev->is_create_cis_from_stack = true;
          rem_qhci_dev->current_transport = BT;
          rem_qhci_dev->is_create_cis_from_stack = true;
          DataHandler *data_handler = DataHandler::Get();

          if (data_handler) {
            if (data_handler->GetControllerRef() != nullptr) {
              ALOGE("%s Sending cis req to the soc", __func__);
              data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                   data, length);
            }
          }
          if (qhci_state == QHCI_BT_CLOSE_XPAN_CLOSE)
            QHciSmExecute(rem_qhci_dev, QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT);
          else
            ALOGE("%s qhci_state wrong state, state %s", __func__,
                    ConvertMsgtoString(qhci_state));
        }
      } else {

        ALOGE("%s QHCI Is in BAD STATE. NO REMINFO for CIS", __func__);
        DataHandler *data_handler = DataHandler::Get();

        if (data_handler) {
          if (data_handler->GetControllerRef() != nullptr) {
            ALOGE("%s Sending cis req to the soc", __func__);
            data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                   data, length);
          }
        }
      }
    }else {
      DataHandler *data_handler = DataHandler::Get();

      if (data_handler) {
        if (data_handler->GetControllerRef() != nullptr) {
          ALOGE("%s Sending data to the soc", __func__);
          data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                   data, length);
        }
      }
    }
  }
}

/******************************************************************
 *
 * Function       IsQHciSupportVersion
 *
 * Description    Checking the Remote Version is QHCI Supported
                  version or not
 *
 *
 * Arguments      version- uint8_t
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQHciSupportVersion (uint8_t version) {
  ALOGD("%s version %d", __func__, version);

  if (version >= QHCI_BT_MIN_VERSION_SUPPORT) {
    return true;
  }

  return false;
}

/******************************************************************
 *
 * Function       IsQHciSupportLmpVersion
 *
 * Description    Checking the remote LMP Version is QHCI Supported
                  version or not
 *
 *
 * Arguments      lmp version- uint16_t
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQHciSupportLmpVersion (uint16_t subversion) {
  ALOGD("%s ", __func__);

  if (subversion >= QHCI_BT_MIN_LMP_VERSION_SUPPORT) {
    return true;
  }

  return true;
}

/******************************************************************
 *
 * Function       IsQHciSupportManuFacture
 *
 * Description    Checking the remote Manufacute is QHCI Supported
                  manufacture id or not
 *
 *
 * Arguments      manufacture_id- uint16_t
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQHciSupportManuFacture (uint16_t manufacture_id) {
  ALOGD("%s ", __func__);

  if (manufacture_id == QHCI_BT_MANUFACTURE_SUPPORT_ID) {
    return true;
  }

  return false;
}

/******************************************************************
 *
 * Function       IsQHciXpanSupportedDevice
 *
 * Description    Checking the handle is in supported XPAN devices
 *                or not?
 *
 *
 * Arguments      hidl_data- vector with uint8_t
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQHciXpanSupportedDevice(uint16_t handle) {
  ALOGD("%s ", __func__);

  std::map<uint16_t, bdaddr_t>::iterator it;
  it = xpan_active_devices_.find(handle);
  if (it != xpan_active_devices_.end()) {

    ALOGD("%s XPAN DEVICE IS SUPPORTED", __func__);
    return true;
  }
  //return false;
  return true;
}

/******************************************************************
 *
 * Function       QHciGetMappingAclHandle
 *
 * Description    Checking the handle from CIS Handle
 *
 *
 * Arguments      uint16_t
 *
 * return         uint16_t
 ******************************************************************/
uint16_t QHci::QHciGetMappingAclHandle(uint16_t cis_handle) {
  ALOGD("%s ", __func__);
  uint16_t acl_handle = 0;
  if (cis_acl_handle_map.find(cis_handle) != cis_acl_handle_map.end()) {
    acl_handle = cis_acl_handle_map[cis_handle];
    ALOGD("%s Cis Handle mappaed to acl_handle %d", __func__, acl_handle);
  }

  return acl_handle;
}

bool QHci::QHciCmpBDAddrs(bdaddr_t bd_addr1, bdaddr_t bd_addr2) {
  ALOGD("%s Addr %s Addr2  %s", __func__,
         ConvertRawBdaddress(bd_addr1), ConvertRawBdaddress(bd_addr2));

  for (int i = 0; i < 6; i++) {
    if (bd_addr1.b[i] != bd_addr2.b[i]) return false;
  }
  return true;
}
uint16_t QHci::QHciBDAddrToHandleMap(bdaddr_t bd_addr) {
  ALOGD("%s ", __func__);

  for (auto& it : handle_bdaddr_map_) {
      // If mapped value is K,
      // then print the key value
      if (QHciCmpBDAddrs(it.second, bd_addr)) {
          ALOGD("%s Addr %s Handle %d", __func__,
                 ConvertRawBdaddress(bd_addr), it.first);
          return (it.first);
      }
  }
  return 0;
}

/******************************************************************
 *
 * Function       IsQhciRxPkt
 *
 * Description    Checking the received HCI RX Event should be
 *                parsed by QHCI or not?
 *
 *
 * Arguments      hidl_data- vector with uint8_t
 *
 * return         true/false
 ******************************************************************/
bool QHci::IsQhciRxPkt (const hidl_vec <uint8_t> *hidl_data) {
  const uint8_t* data = hidl_data->data();

  if (data[0] == QHCI_COMMAND_COMPLETE_EVENT) {
    uint16_t opcode = ((data[4] << 8) | (data[3]));
    if (opcode == 0xFC51) {
      if (data[6] == 0x0F) {
        if (!qhci_set_host_bit) {
          ALOGD("%s HCI_LE_VENDOR_SPECIFIC_EVENT Params ", __func__);
          return true;
        } else {
          return false;
        }
      }
    }
    if (opcode == 0xFC51) {
      if (data[6] == 0x14) {
        ALOGD("%s HCI_VS_QBCE_QLE_SET_HOST_FEATURE status %d", __func__, data[5]);
        QHciSendVndrQllEvtMask();
        return true;
      }
    }
  }

  if (data[0] == QHCI_COMMAND_COMPLETE_EVENT) {
    if (qhci_wait_for_cmd_status_from_soc) {
      uint8_t length = data[1];
      uint8_t num_of_cmds = data[2];
      if (num_of_cmds == 1) {
        uint16_t opcode = ((data[4] << 8) | (data[3]));
        switch (opcode) {
          case HCI_LE_SET_CIG_PARAMETERS:
            {
              if (data[5] == QHCI_BT_SUCCESS) {
                ALOGD("%s HCI_LE_SET_CIG_PARAMETERS ", __func__);
                if (data[6] == cig_params.cig_id) {
                  if (cig_params.cis_count == 1) {
                    cig_params.cis_handles[0] = ((data[9] << 8) | (data[8]));
                    cig_params.cis_count = 1;
                  } else {
                    cig_params.cis_count = 2;
                    cig_params.cis_handles[0] = ((data[9] << 8) | (data[8]));
                    cig_params.cis_handles[1] = ((data[11] << 8) | (data[10]));
                    ALOGD("%s HCI_LE_SET_CIG_PARAMETERS CIS HANDLES %d %d ",
                            __func__, cig_params.cis_handles[0],
                            cig_params.cis_handles[1]);
                  }
                }
              }
            }
            break;
          case HCI_LE_REMOVE_CIG:
            {
              if (data[5] == QHCI_BT_SUCCESS) {
                ALOGD("%s HCI_LE_REMOVE_CIG ", __func__);
                cig_params.cig_id = 0;
                cig_params.cis_handles[0] = 0;
                cig_params.cis_handles[1] = 0;
              }
            }
            break;
          case HCI_DISCONNECT:
            {
              if (prep_bearer_active) {
                prep_bearer_active = false;
                if (!dbg_mtp_mora_prop) {
                  qhci_xm_intf.UnPrepareAudioBearerReqToXm(curr_bd_addr, NONE);
                } else {
                  qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0], NONE);
                }
                return true;
              }
              if (is_cis_handle_disc_pending)
                return true;
            }
            break;
          default:
            ALOGD("%s Wrong QHCI_COMMAND_COMPLETE_EVENT", __func__);
        }
      }
    }
    return false;
  }

  if (data[0] == HCI_VENDOR_EVT) {
    if (data[2] == QHCI_QBCE_SUBEVENT_CODE) {
      if (data[3] == READ_REMOTE_QLL_FEATURE_CMPL_EVT) {
        uint16_t acl_handle = (data[6] << 8 | data[5]);
        uint8_t is_xpan_feature_set = data[14] & 0x20;
        ALOGD("%s QHCI_QBCE_SUBEVENT_CODE Acl handle %d  "
          "is_XPAN_feature_set %d ", __func__, acl_handle, is_xpan_feature_set);

        qhci_dev_cb_t* rem_info = GetQHciRemoteDeviceInfo(acl_handle);
        if (encrypt_event_pending) {
          encrypt_event_pending = false;
          uint8_t evt_pkt[6] = {0x08, 0x04, 0x00, 0x02, 0x00, 0x01};
          DataHandler *data_handler = DataHandler::Get();
          if (data_handler) {
#ifdef XPAN_SUPPORTED
                hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
                hidl_data->resize(HCI_ENCRYPT_CHANGE_EVT_LEN);
                memcpy(hidl_data->data(), evt_pkt,
                  HCI_ENCRYPT_CHANGE_EVT_LEN);
                data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                                    hidl_data, false);
#endif
          }
        }
        if (is_xpan_feature_set) {

          rem_info->is_xpan_device = true;

          std::map<uint16_t, bdaddr_t>::iterator it;
          it = handle_bdaddr_map_.find(acl_handle);
          if (it != handle_bdaddr_map_.end()) {
            bdaddr_t bd_addr = it->second;
            curr_bd_addr = bd_addr;
            xpan_active_devices_[acl_handle] = it->second;
            qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(acl_handle);
            if (rem_info) {
              ALOGD("%s handle state is QHCI_BT_CLOSE_XPAN_CLOSE %d ", __func__,
                    acl_handle);
            }
            qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_PKT_MESSAGE_LENGTH);
            msg->eventId = QHCI_LE_CONN_CMPL_EVT;
            msg->ConnCmpl.eventId = QHCI_LE_CONN_CMPL_EVT;
            msg->ConnCmpl.bd_addr = bd_addr;
            msg->ConnCmpl.handle = acl_handle;
            PostMessage(msg);
            return false;
          }
        }
      }
    }
  }

  if (data[0] == HCI_ENCRYPT_CHANGE_EVT) {
    //if its xpan supported version
    return false;
  }


  if ((data[0] != HCI_LE_EVT) && (data[0] != HCI_DISCONNECT_CMPL_EVT) &&
    (data[0] != HCI_READ_REMOTE_VERSION_CMPL_EVT)) {
    return false;
  }

  if (data[0] == HCI_READ_REMOTE_VERSION_CMPL_EVT) {
    uint8_t event_status = data[2];

    if (event_status != QHCI_BT_SUCCESS) {
      return false;
    }
    uint8_t version = data[5];
    uint16_t manufacture_id = (data[7] << 8 | data[6]);
    uint16_t lmp_version = (data[9] << 8 | data[8]);
    uint16_t le_handle = (data[4] <<8 | data[3]);

    if ((IsQHciSupportVersion(version)) &&
       (IsQHciSupportManuFacture(manufacture_id)) &&
       (IsQHciSupportLmpVersion(lmp_version))) {

       qhci_wait_for_qll_event = true;
       return false;

    } else {
      //Not Supported
      if (dbg_mtp_mora_prop || dbg_mtp_mtp_prop || dbg_mora_nut_prop) {
        std::map<uint16_t, bdaddr_t>::iterator it;
        it = handle_bdaddr_map_.find(le_handle);
        if (it != handle_bdaddr_map_.end()) {
          bdaddr_t bd_addr = it->second;
          //qhci_xm_intf.RemoteSupportXpanToXm(bd_addr, true);
          curr_bd_addr = bd_addr;
          dbg_curr_bd_addr[dbg_cnt_bd_addr] = bd_addr;
          dbg_cnt_bd_addr++;
          dbg_cnt_bd_addr = dbg_cnt_bd_addr%2;
          xpan_active_devices_[le_handle] = it->second;
          qhci_dev_cb_t *rem_info = GetQHciRemoteDeviceInfo(le_handle);
          if (rem_info) {
            ALOGD("%s handle state is QHCI_BT_CLOSE_XPAN_CLOSE %d ", __func__,
                  le_handle);
          }
          qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_PKT_MESSAGE_LENGTH);
          msg->eventId = QHCI_LE_CONN_CMPL_EVT;
          msg->ConnCmpl.eventId = QHCI_LE_CONN_CMPL_EVT;
          msg->ConnCmpl.bd_addr = bd_addr;
          msg->ConnCmpl.handle = le_handle;
          PostMessage(msg);

          //qhci_xm_intf.UseCaseUpdateToXm(USECASE_XPAN_LOSSLESS);
          //QHciSmExecute(rem_info, QHCI_CSM_LE_CONN_CMPL_EVT);
          ALOGD("%s handle for remote version %d ", __func__, le_handle);
        }
      }
      return false;
    }

    return false;
  }

  if (data[0] == HCI_DISCONNECT_CMPL_EVT) {
    //parse the handle
    is_cis_conn_prog = false;
    uint16_t handle = ((data[4] << 8) | (data[3]));

    //check the handle is present in XPAN active device map
    std::map<uint16_t, bdaddr_t>::iterator it;
    it = xpan_active_devices_.find(handle);
    if (it != xpan_active_devices_.end()) {
      ALOGD("%s HCI_DISCONNECT_CMPL_EVT is for XPAN ACTIVE DEVICE", __func__);
          //check the handle is present in XPAN active device map
      qhci_dev_cb_t* rem_info = GetQHciRemoteDeviceInfo(handle);

      if (rem_info) {
        QHciSmExecute(rem_info, QHCI_CSM_LE_CLOSE_EVT);
        xpan_active_devices_.erase(handle);
        handle_bdaddr_map_.erase(handle);
        //reset rem_info if needed
        QHciClearRemoteDeviceInfo(handle);
      }
      return false;
    }

     if ((cig_params.cis_handles[0] == handle) ||
            (cig_params.cis_handles[1] == handle)) {
      ALOGD("%s CIS HANDLE DISCONNECT_CMPL_EVT is for XPAN- handle %d",
              __func__, handle);
      return true;
    } else {
      ALOGD("%s HCI_DISCONNECT_CMPL_EVT Non XPAN ACTIVE DEVICE - handle %d",
        __func__, handle);
      return false;
    }
  }

  uint8_t le_evt_sub_code = data[2];

  if (le_evt_sub_code == HCI_LE_EXT_ADV_EVT) return false;

  bool status = false;
  switch (le_evt_sub_code) {
    case HCI_LE_CONN_CMPL_EVT:
    {
      ALOGD ("%s Parsing LE CONN CMPL EVT ", __func__);
      qhci_dev_cb_t rem_dev_info;
      //store the Handle
      rem_dev_info.handle = ((data[5] << 8) | (data[4]));
      //Initial state will be IDLE state
      rem_dev_info.state = QHCI_IDLE_STATE;

      //parsing BD addr type
      rem_dev_info.addr_type = data[7];

      //parsing BD addr
      for (int i = 0; i < 6; i++) {
        rem_dev_info.rem_addr.b[i] = data[8+i];
      }
      handle_bdaddr_map_[rem_dev_info.handle] = rem_dev_info.rem_addr;

      //inserting remote device details into xpan device db
      qhci_xpan_dev_db.push_back(rem_dev_info);

      ALOGD ("%s bdaddr %s",__func__, 
              ConvertRawBdaddress(rem_dev_info.rem_addr));

      /* return false so that it can go to the BT Stack and
         QHCI wont process the command */
      status = false;
    }
      break;
    case HCI_LE_READ_REMOTE_FEAT_CMPL_EVT:
    {
      //Store the details for future purpose
      status = false;
    }
     break;
    case HCI_LE_CIS_ESTABLISHED_EVT:
    {
      // If Remote supports XPAN,
      // Process the CIS Established evt
      // Else -- if remote not supports XPAN
      ALOGD("%s HCI_LE_CIS_ESTABLISHED_EVT qhci_bearer_switch_pending %d",
      __func__, qhci_bearer_switch_pending);
      if (qhci_bearer_switch_pending) {
        pending_cis_est_evt++;
        status = true;
      } else {
        status = false;
      }
    }
      break;
    default:
      status = false;
  }

  // parse the Command complete for CREATE CIG and SET ISO path
  // Check the status
  // Update the status

  return status;
}

void QHci::ProcessRxPktEvent(HciPacketType type,
                                    const hidl_vec < uint8_t > * hidl_data_t) {
  ALOGD("%s ", __func__);

  qhci_msg_t *msg = (qhci_msg_t *) malloc(QHCI_PKT_MESSAGE_LENGTH
                    + hidl_data_t->size());
  msg->RxEvtPkt.hidl_data = new hidl_vec(*hidl_data_t);
  msg->RxEvtPkt.type = type;
  const uint8_t* data = hidl_data_t->data();
  const uint8_t* data2 = msg->RxEvtPkt.hidl_data->data();
  uint16_t len = hidl_data_t->size();
  uint16_t len2 = msg->RxEvtPkt.hidl_data->size();
  msg->eventId = QHCI_PROCESS_RX_PKT_EVT;
  msg->RxEvtPkt.eventId = QHCI_PROCESS_RX_PKT_EVT;

  PostMessage(msg);

  return;
}

void QHci::QHciHandleIdleState(qhci_dev_cb_t *rem_info, uint8_t event) {
  //Handle the parameters in Idle state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch (event)
  {
    case QHCI_CSM_LE_CONN_CMPL_EVT:
      {
        rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
        rem_info->is_xpan_device = true;
        is_xpan_supported = true;
        //rem_info->transport_enable = true;
        qhci_xm_intf.RemoteSupportXpanToXm(rem_info->rem_addr,
                                           rem_info->is_xpan_device);
        //qhci_xm_intf.UseCaseUpdateToXm(USECASE_XPAN_LOSSLESS);
      }
      break;
    default:
      ALOGE("%s: Invalid Event in this state", __func__);
  }
}

QHCI_CSM_STATE QHci::GetQHciState(qhci_dev_cb_t *rem_info) {

  if (rem_info) {
    ALOGD("%s:  state  %s", __func__, ConvertMsgtoString(rem_info->state));
    return rem_info->state;
  }

  return QHCI_IDLE_STATE;

}


/******************************************************************
 *
 * Function       QHciSetHostSupportedBit
 *
 * Description    Sending Set XPAN Host Supported Bit
 *
 * Arguments      none
 *
 * return         none
 ******************************************************************/
void QHci::QHciSetHostSupportedBit() {
  ALOGE("%s ", __func__);

  uint16_t length = QHCI_SET_HOST_SUPPORTED_BIT_COMMAND_LEN;

  uint8_t *data = (uint8_t *)malloc(length * sizeof(uint8_t));

  data[0] = 0x51;
  data[1] = 0xFC;
  data[2] = 3;
  data[3] = HCI_SUB_OPCODE_QLE_SET_HOST_FEATURE;
  //BIT Position
  data[4] = 0x3D;
  data[5] = 0x01;

  qhci_set_host_bit = true;
  DataHandler *data_handler = DataHandler::Get();

  if (data_handler) {
    if (data_handler->GetControllerRef() != nullptr) {
      ALOGE("%s Sending data to the soc", __func__);
      data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                   data,
                                                   QHCI_SET_HOST_SUPPORTED_BIT_COMMAND_LEN);
      }
  }
  return;
}

/******************************************************************
 *
 * Function       QHciSendCisEstablishedEvt
 *
 * Description    Sending Fake CIS Established Evt to Stack
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *
 * return         none
 ******************************************************************/
void QHci::QHciSendCisEstablishedEvt(qhci_dev_cb_t *rem_info,
                                               uint8_t status,
                                               uint8_t handle_num) {

  ALOGD("%s ", __func__);

  if (rem_info) {
    //Move this logic to other thread _if there any crashes
    uint16_t length = QHCI_CIS_ESTABLISHED_EVT_SIZE;
    uint8_t data[QHCI_CIS_ESTABLISHED_EVT_SIZE] = {0};

    //Event Code
    data[0] = HCI_LE_EVT;
    //length 
    data[1] = length - 2;
    //SubEvent
    data[2] = HCI_LE_CIS_ESTABLISHED_EVT;
    //Status
    data[3] = status;
    //CIS Handle
    data[4] = cig_params.cis_handles[handle_num] & 0x00FF;
    data[5] = cig_params.cis_handles[handle_num] & 0xFF00;
    //CIG SYNC Delay
    data[6] = rem_info->qhci_cis_establish_evt_data.cig_sync_delay[0];
    data[7] = rem_info->qhci_cis_establish_evt_data.cig_sync_delay[1];
    data[8] = rem_info->qhci_cis_establish_evt_data.cig_sync_delay[2];
    //CIS Sync Delay
    data[9] = rem_info->qhci_cis_establish_evt_data.cis_sync_delay[0];
    data[10] = rem_info->qhci_cis_establish_evt_data.cis_sync_delay[1];
    data[11] = rem_info->qhci_cis_establish_evt_data.cis_sync_delay[2];
    //Transport latency M to S
    data[12] = rem_info->qhci_cis_establish_evt_data.trans_latency_m_s[0];
    data[13] = rem_info->qhci_cis_establish_evt_data.trans_latency_m_s[1];
    data[14] = rem_info->qhci_cis_establish_evt_data.trans_latency_m_s[2];
    //Transport latency S to M
    data[15] = rem_info->qhci_cis_establish_evt_data.trans_latency_s_m[0];
    data[16] = rem_info->qhci_cis_establish_evt_data.trans_latency_s_m[1];
    data[17] = rem_info->qhci_cis_establish_evt_data.trans_latency_s_m[2];
    //phy M to S
    data[18] = rem_info->qhci_cis_establish_evt_data.phy_m_s;
    //phy S to M
    data[19] = rem_info->qhci_cis_establish_evt_data.phy_s_m;
    // No of Sub events
    data[20] = rem_info->qhci_cis_establish_evt_data.num_sub_events;
    //burst num M to S
    data[21] = rem_info->qhci_cis_establish_evt_data.burst_num_m_s;
    //burst num S to M
    data[22] = rem_info->qhci_cis_establish_evt_data.burst_num_s_m;
    //Flush timeout M to S
    data[23] = rem_info->qhci_cis_establish_evt_data.flush_timeout_m_s;
    //Flush timeout S to M
    data[24] = rem_info->qhci_cis_establish_evt_data.flush_timeout_s_m;
    //Max PDU Size M to S
    data[25] = rem_info->qhci_cis_establish_evt_data.max_pdu_size_m_s[0];
    data[26] = rem_info->qhci_cis_establish_evt_data.max_pdu_size_m_s[1];
    //Max PDU Size S to M
    data[27] = rem_info->qhci_cis_establish_evt_data.max_pdu_size_m_s[0];
    data[28] = rem_info->qhci_cis_establish_evt_data.max_pdu_size_m_s[1];
    //Iso Interval
    data[29] = rem_info->qhci_cis_establish_evt_data.iso_interval[0];
    data[30] = rem_info->qhci_cis_establish_evt_data.iso_interval[1];

    DataHandler *data_handler = DataHandler::Get();
    if (data_handler) {
#ifdef XPAN_SUPPORTED
      hidl_vec<uint8_t> *hidl_data = new hidl_vec<uint8_t>;
      hidl_data->resize(QHCI_CIS_ESTABLISHED_EVT_SIZE);
      memcpy(hidl_data->data(), data, QHCI_CIS_ESTABLISHED_EVT_SIZE);

      data_handler->OnPacketReadyFromQHci(HCI_PACKET_TYPE_EVENT,
                                          hidl_data, false);
#endif
    }
  }

}


/******************************************************************
 *
 * Function       SendCreateCisToSoc
 *
 * Description    Sending Create CIS to SOC
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *
 * return         none
 ******************************************************************/
void QHci::SendCreateCisToSoc(qhci_dev_cb_t *rem_info) {

  ALOGD("%s ", __func__);
  uint16_t handle;

  if (rem_info) {
    handle = rem_info->handle;
  } else {
    ALOGE("%s Enter Valid Handle ", __func__);
    handle = 0;
  }

  //Move this logic to other thread _if there any crashes
  uint16_t length = 12;
  uint8_t *data = (uint8_t *)malloc(length);

  //Opcode
  data[0] = 0x64;
  data[1] = 0x20;

  //length
  data[2] = 9;

  //cis_count
  data[3] = 2;


  //Currently create cis going each handle separately
  data[4] = cig_params.cis_handles[0] & 0x00FF;
  data[5] = cig_params.cis_handles[0] & 0xFF00;

  //ACL Handle
  data[6] = handle & 0x00FF;
  data[7] = handle & 0xFF00;

  data[8] = cig_params.cis_handles[1] & 0x00FF;
  data[9] = cig_params.cis_handles[1] & 0xFF00;

  //ACL Handle
  data[10] = handle & 0x00FF;
  data[11] = handle & 0xFF00;

#ifdef XPAN_SUPPORTED
  DataHandler *data_handler = DataHandler::Get();
  if (data_handler) {
    if (data_handler->GetControllerRef() != nullptr)
      ALOGD("%s Sending data to the soc", __func__);
      data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                   data, length);
  }
#endif

}

/******************************************************************
 *
 * Function       QHciLeConnCleanup
 *
 * Description    cleaning up the remote device data
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *
 * return         none
 ******************************************************************/
void QHci::QHciLeConnCleanup(qhci_dev_cb_t *rem_info) {
  //Doing Memset
  memset((void *)rem_info, 0, sizeof(qhci_dev_cb_t));
}

/******************************************************************
 *
 * Function       QHciHandleBtCloseXpanClose
 *
 * Description    Handling the QHCI_BT_CLOSE_XPAN_CLOSE
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtCloseXpanClose(qhci_dev_cb_t *rem_info, uint8_t event) {
  //Handle the parameters in QHciHandleBtCloseXpanClose state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch (event)
  {
    case QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT:
      {
      
        ALOGD("%s:  QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT ", __func__);
        rem_info->state = QHCI_BT_OPEN_XPAN_CLOSE;
        //Create CIS is triggered from the stack
        rem_info->is_create_cis_from_stack = false;
        if (rem_info->is_create_cis_from_stack) {
          //Update the params if anything needed
        } else {
          ALOGE("%s:  Invalid call from QHCI module", __func__);
        }
      }
      break;
    case QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT:
      {
        ALOGD("%s:  QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT ", __func__);
        rem_info->state = QHCI_BT_CLOSE_XPAN_CONNECTING;
        //Sending prepare Bearer Request to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.PrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                  XPAN);
        } else {
          qhci_xm_intf.PrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                  XPAN);
        }
      }
      break;
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        QHciLeConnCleanup(rem_info);
      }
      break;
    default:
      ALOGE("%s: Invalid Event in this state", __func__);
  }

}

/******************************************************************
 *
 * Function       QHciHandleBtCloseXpanConnecting
 *
 * Description    Handling the QHCI_BT_CLOSE_XPAN_CONNECTING
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtCloseXpanConnecting(qhci_dev_cb_t *rem_info,
                                                       uint8_t event) {
  //Handle the parameters in QHciHandleBtCloseXpanConnecting state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch(event)
  {
    case QHCI_CSM_UPDATE_TRANS_XPAN_EVT:
      {
        rem_info->state = QHCI_BT_CLOSE_XPAN_OPEN;
        rem_info->current_transport = XPAN;

        //Sending Fake CIS Establishment to the Stack
        QHciSendCisEstablishedEvt(rem_info, QHCI_BT_SUCCESS, 0);
        QHciSendCisEstablishedEvt(rem_info, QHCI_BT_SUCCESS, 1);
      }
      break;
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }

        //clear any other pending things related to control blocks.
        //Ideally those will be over written
        QHciLeConnCleanup(rem_info);
        //Check with XM whether it need disconnect info or not?
      }
      break;
    case QHCI_CSM_XPAN_CONN_FAILED_EVT:
      {
        //TODO Remove below api call once testing with EB
        QHciSendCisEstablishedEvt(rem_info, 13, 0);
        QHciSendCisEstablishedEvt(rem_info, 13, 1);
        rem_info->state = QHCI_BT_OPEN_XPAN_CLOSE;
        //Create CIS is triggered from the stack
        if (rem_info->is_create_cis_from_stack) {
          SendCreateCisToSoc(rem_info);
        } else {
          ALOGE("%s:  Invalid call from QHCI moduele", __func__);
        }
      }
      break;
    default:
      ALOGE("%s:  Invalid Event  %d in this state", __func__, event);
  }
}

/******************************************************************
 *
 * Function       QHciHandleBtCloseXpanOpen
 *
 * Description    Handling the QHCI_BT_CLOSE_XPAN_OPEN
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtCloseXpanOpen(qhci_dev_cb_t *rem_info,
                                               uint8_t event) {
  //Handle the parameters in QHciHandleBtCloseXpanOpen state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch(event)
  {
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }

        //clear any other pending things related to control blocks.
        //Ideally those will be over written
        QHciLeConnCleanup(rem_info);
        //Check with XM whether it need disconnect info or not?
      }
      break;
    case QHCI_CSM_CIS_DISCONNECT_EVT:
      {
        //This Event will be triggered when the stream is stopped
        rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }

        //clear any other pending things related to control blocks.
        //Ideally those will be over written
      }
      break;
    case QHCI_CSM_PREPARE_BEARER_BT:
      {
        //This Event will be triggered when preparea bearer req from XM
        rem_info->state = QHCI_BT_OPEN_XPAN_OPEN;
        rem_info->is_create_cis_from_qhci_to_soc = true;
        qhci_bearer_switch_pending = true;
        //Send Create CIS to Soc
        ALOGE("%s:  QHCI_CSM_PREPARE_BEARER_BT qhci_bearer_switch_pending  %d",
                __func__, qhci_bearer_switch_pending);
        SendCreateCisToSoc(rem_info);
      }
      break;
    default:
      ALOGE("%s:  Invalid Event  %d in this state", __func__, event);
  }
}

/******************************************************************
 *
 * Function       QHciHandleBtOpenXpanClose
 *
 * Description    Handling the QHCI_BT_OPEN_XPAN_CLOSE
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtOpenXpanClose(qhci_dev_cb_t *rem_info,
                                               uint8_t event) {
  //Handle the parameters in QHciHandleBtOpenXpanClose state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch(event)
  {
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }
        //clear any other pending things related to control blocks.
        //Ideally those will be over written
        QHciLeConnCleanup(rem_info);
        //Check with XM whether it need disconnect info or not?
      }
      break;
    case QHCI_CSM_CIS_DISCONNECT_EVT:
      {
        //This Event will be triggered when the stream is stopped
        rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
        //No Need to update the XPAN Manager.
        //Here Transport is BT. Change current active_transport to NONE
        rem_info->current_transport = BT;
      }
      break;
    case QHCI_CSM_USECASE_XPAN_TRANS_ENABLE_EVT:
      {
        //Move the Bearer to XPAN.
        rem_info->state = QHCI_BT_OPEN_XPAN_CONNECTING;

        //Send the prepareBearer request to the XM
        rem_info->is_prepare_bearer_triggered = true;
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.PrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                 rem_info->current_transport);
        } else {
          qhci_xm_intf.PrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                 rem_info->current_transport);
        }
      }
      break;
    case QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT:
      {
        // Respond to prepare Bearer Request to XPAN.
        rem_info->state = QHCI_BT_OPEN_XPAN_CONNECTING;
        qhci_bearer_switch_pending = true;
        rem_info->is_prepare_bearer_triggered = true;

        qhci_xm_intf.PrepareAudioBearerRspToXm(rem_info->rem_addr, XM_SUCCESS);
      }
      break;
    default:
      ALOGE("%s:  Invalid Event  %d in this state", __func__, event);
  }
}

/******************************************************************
 *
 * Function       QHciPrepareAndSendHciDisconnect
 *
 * Description    Sending HCI Disconnect  to SCO
 *
 * Arguments      acl handle
 *
 * return         none
 ******************************************************************/
void QHci::QHciPrepareAndSendHciDisconnect(uint16_t handle) {
  ALOGD("%s:  handle %d", __func__, handle);

  if (handle > 0) {
     uint16_t length = QHCI_DISCONNECT_COMMAND_LEN;

     uint8_t *data = (uint8_t *)malloc(length);
   
     data[0] = 0x06;
     data[1] = 0x04;
     data[2] = 0x03;
     data[3] = handle & 0x00FF;
     data[4] = handle & 0xFF00;
     data[5] = 0x13; //Remote user terminated connection status code

     is_cis_handle_disc_pending = true;
     DataHandler *data_handler = DataHandler::Get();
     if (data_handler) {
       if (data_handler->GetControllerRef() != nullptr)
         ALOGD("%s Sending data to the soc", __func__);
         data_handler->GetControllerRef()->SendPacket(HCI_PACKET_TYPE_COMMAND,
                                                      data, length);
     }
  }
  return;
}

/******************************************************************
 *
 * Function       QHciSendDisconnectCisToSoc
 *
 * Description    Sending HCI Disconnect to CIS handles to SCO
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *
 * return         none
 ******************************************************************/
void QHci::QHciSendDisconnectCisToSoc(qhci_dev_cb_t *rem_info) {
  ALOGD("%s:  ", __func__);

  if (rem_info) {
    rem_info->is_cis_disconnect_pending_from_soc =  true;
    //currently only one cis will created with this remote
    // need to enhance to csip devices also

    QHciPrepareAndSendHciDisconnect(cig_params.cis_handles[0]);
    QHciPrepareAndSendHciDisconnect(cig_params.cis_handles[1]);
  } else {
    ALOGE("%s:  QHCI Rem_info Database is Null", __func__);
  }

}

/******************************************************************
 *
 * Function       QHciHandleBtOpenXpanConnecting
 *
 * Description    Handling the QHCI_BT_OPEN_XPAN_CONNECTING
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtOpenXpanConnecting(qhci_dev_cb_t *rem_info,
                                                      uint8_t event) {
  //Handle the parameters in QHciHandleBtOpenXpanConnecting state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch(event) {
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }
        //clear any other pending things related to control blocks.
        //Ideally those will be over written
        QHciLeConnCleanup(rem_info);
        //Check with XM whether it need disconnect info or not?
      }
      break;
    case QHCI_CSM_CIS_DISCONNECT_EVT:
      {
        //This Event will be triggered when the stream is stopped
        rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }

        //Here Transport is BT. Change current active_transport to NONE
        rem_info->current_transport = BT;
      }
      break;
    case QHCI_CSM_XPAN_CONN_FAILED_EVT:
      {
        rem_info->state = QHCI_BT_OPEN_XPAN_CLOSE;
        rem_info->current_transport = BT;
        rem_info->is_prepare_bearer_triggered = false;
      }
      break;
    case QHCI_CSM_UPDATE_TRANS_XPAN_EVT:
      {
        rem_info->state = QHCI_BT_CLOSE_XPAN_OPEN;
        rem_info->current_transport = XPAN;
        rem_info->is_prepare_bearer_triggered = false;
        rem_info->is_create_cis_from_qhci_to_soc = true;
        //disconnect the CIS handles with the Soc
        QHciSendDisconnectCisToSoc(rem_info);
      }
      break;
    default:
      ALOGE("%s:  Invalid Event  %d in this state", __func__, event);
  }
}

/******************************************************************
 *
 * Function       QHciHandleBtOpenXpanOpen
 *
 * Description    Handling the QHCI_BT_OPEN_XPAN_OPEN
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciHandleBtOpenXpanOpen(qhci_dev_cb_t *rem_info,
                                              uint8_t event) {
  //Handle the parameters in QHciHandleBtOpenXpanOpen state
  ALOGD("%s:  Event  %s", __func__, ConvertEventToString(event));

  switch(event)
  {
    case QHCI_CSM_LE_CLOSE_EVT:
      {
        rem_info->state = QHCI_IDLE_STATE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }
        //clear any other pending things related to control blocks.
        //Ideally those will be over written
        QHciLeConnCleanup(rem_info);
        //Check with XM whether it need disconnect info or not?
      }
      break;
    case QHCI_CSM_CIS_DISCONNECT_EVT:
      {
        //This Event will be triggered when the stream is stopped
        rem_info->state = QHCI_BT_CLOSE_XPAN_CLOSE;
        //Sending Unprepare Audio Bearer to XPAN Manager
        if (!dbg_mtp_mora_prop) {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(rem_info->rem_addr,
                                                   NONE);
        } else {
          qhci_xm_intf.UnPrepareAudioBearerReqToXm(dbg_curr_bd_addr[0],
                                                   NONE);
        }

        //Here Transport is BT. Change current active_transport to NONE
        rem_info->current_transport = BT;
      }
      break;
    case QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT:
      {
        rem_info->state = QHCI_BT_OPEN_XPAN_CLOSE;
        rem_info->current_transport = BT;
        rem_info->is_prepare_bearer_triggered = false;
        qhci_bearer_switch_pending = false;
      }
      break;
    default:
      ALOGE("%s:  Invalid Event  %d in this state", __func__, event);
  }
}

/******************************************************************
 *
 * Function       QHciSmExecute
 *
 * Description    Execute the state machine based on the state
 *                for that remote device
 *
 * Arguments      qhci_dev_cb_t -QHCI control block for that
 *                remote
 *                event - type of event which needs to handle in
 *                this state
 *
 * return         none
 ******************************************************************/
void QHci::QHciSmExecute(qhci_dev_cb_t *rem_info, uint8_t event) {
  ALOGD("%s state %s Event %s ", __func__, ConvertMsgtoString(rem_info->state),
         ConvertEventToString(event));

  switch (rem_info->state) {
    case QHCI_IDLE_STATE:
        QHciHandleIdleState(rem_info, event);
        break;
      case QHCI_BT_CLOSE_XPAN_CLOSE:
        QHciHandleBtCloseXpanClose(rem_info, event);
        break;
      case QHCI_BT_CLOSE_XPAN_CONNECTING:
        QHciHandleBtCloseXpanConnecting(rem_info, event);
        break;
      case QHCI_BT_CLOSE_XPAN_OPEN:
        QHciHandleBtCloseXpanOpen(rem_info, event);
        break;
      case QHCI_BT_OPEN_XPAN_CLOSE:
        QHciHandleBtOpenXpanClose(rem_info, event);
        break;
      case QHCI_BT_OPEN_XPAN_CONNECTING:
        QHciHandleBtOpenXpanConnecting(rem_info, event);
        break;
      case QHCI_BT_OPEN_XPAN_OPEN:
        QHciHandleBtOpenXpanOpen(rem_info, event);
        break;
      default:
        ALOGD(" %s Not valid event = %d", event);
        break;
    }
}


void QHci::QHciCmdTimeOut(union sigval sig) {
  //Handle when command was timeout
}

void QHci::QHciSetCmdTimerState(QHciTimerState timer_state) {
  qhci_cmd_timer_.timer_state = timer_state;
}

QHciTimerState QHci::QHciGetCmdTimerState() {
  return qhci_cmd_timer_.timer_state;
}

void QHci::QHciStopCmdTimer() {
  struct itimerspec ts;
  QHciTimerState cmd_timer_state;

  cmd_timer_state = QHciGetCmdTimerState();

  if (cmd_timer_state == QHCI_TIMER_NOT_CREATED) {
      ALOGD("%s: CmdTimer already stopped", __func__);
      return;
  } else {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    if (timer_settime(qhci_cmd_timer_.timer_id, 0, &ts, 0) == -1) {
      ALOGE("%s:Failed to stop cmd thread timer", __func__);
    }
    timer_delete(qhci_cmd_timer_.timer_id);
    QHciSetCmdTimerState(QHCI_TIMER_NOT_CREATED);
    ALOGD("%s: CmdTimer Stopped", __func__);
    return;
  }
}

void QHci::QHciStartCommandTimer() {
  struct itimerspec ts;
  struct sigevent se;
  uint32_t timeout;
  int prop_val = 0;

  ALOGV("%s", __func__);
  if (qhci_cmd_timer_.timer_state == QHCI_TIMER_NOT_CREATED) {
    se.sigev_notify_function = (void (*)(union sigval))QHciCmdTimeOut;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = &qhci_cmd_timer_.timer_id;
    se.sigev_notify_attributes = NULL;

    if (!timer_create(CLOCK_MONOTONIC, &se, &qhci_cmd_timer_.timer_id))
      QHciSetCmdTimerState(QHCI_TIMER_CREATED);
    else
      ALOGE("%s: Failed to create QHCICmdTimer", __func__);
  }

  if (QHciGetCmdTimerState() == QHCI_TIMER_CREATED) {

    //Currently timeout is 1 sec
    timeout = 1000;
    ts.it_value.tv_sec = (timeout / 1000);
    ts.it_value.tv_nsec = 1000000 * (timeout % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    if ((timer_settime(qhci_cmd_timer_.timer_id, 0, &ts, 0)) == -1) {
      ALOGE("%s: Failed to start Init timer", __func__);
      return;
    } else {
      QHciSetCmdTimerState(QHCI_TIMER_ACTIVE);
      ALOGD("%s: Command timer started", __func__);
    }
  }
}

} // namespace implementation
} // namespace xpan


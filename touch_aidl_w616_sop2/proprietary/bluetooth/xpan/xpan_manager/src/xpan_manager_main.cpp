/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <utils/Log.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include "xpan_manager_main.h"
#include "data_handler.h"
#include "xm_xprofile_if.h"
#include "xm_qhci_if.h"
#include "xm_wifi_if.h"
#include "xm_packetizer.h"
#include <cutils/properties.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.xpan@1.0-xpanmanager"

using android::hardware::bluetooth::V1_0::implementation::DataHandler;

namespace xpan {
namespace implementation {

XMPacketizer packetizer;

#ifdef XPAN_ENABLED
XMXprofileIf profile_if;
XMQhciIf qhci_if;
XMWifiIf wifi_if;
char xpan_value[PROPERTY_VALUE_MAX] = {'\0'};
#endif

std::shared_ptr<XpanManager> XpanManager::main_instance = nullptr;

XpanManager::XpanManager()
{
}

XpanManager::~XpanManager()
{
  ALOGD("%s", __func__);
}

std::shared_ptr<XpanManager> XpanManager::Get()
{
  if (!main_instance)
    main_instance.reset(new XpanManager());
  return main_instance;
}

#ifdef XPAN_ENABLED
/* This callback is fired either during audio bearer req
 * or unprepare audio bearer rsp.
 */ 
void XpanManager::AudioBearerTimeOut(union sigval sig)
{
  ALOGI("%s: triggered", __func__);
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->AudioBearerTimeout.eventId = XM_PREPARE_AUDIO_BEARER_TIMEOUT;
  if (main_instance)
    main_instance->PostMessage(msg);
}
#endif

void XpanManager::XpanManagerMainThreadRoutine(bool is_xpan_supported)
{
  DataHandler *data_handler = DataHandler::Get();
  bool status = false;
  memset(&xm_state, 0, sizeof(struct xm_state_machine));

  /* Initialize Glink Channel */
  glink_transport = GlinkTransport::Get();
  if (!glink_transport) {
    goto update_status;
  } else {
    glink_fd = glink_transport->OpenGlinkChannel(TYPE_GLINK_CC);
    if (glink_fd < 0) {
      goto update_status;
    }
  }

  /* Initialize KPTransport */
  kp_transport = KernelProxyTransport::Get();
  if (!kp_transport) {
    goto update_status;
  } else {
    kp_fd = kp_transport->OpenKpTransport();
    if (kp_fd < 0) {
      goto update_status;
    }
  }

#ifdef XPAN_ENABLED
  if(is_xpan_supported) {
    wifi_if.InitWifiIf();

    audio_bearer = create_timer("audio bearer", AudioBearerTimeOut);
    if (audio_bearer == NULL)
      goto update_status;

    xm_state.current_state = IDLE;
    xm_state.prev_state = IDLE;
    /* Initialize XPAN provider AIDL */
    profile_if.Initialize();
  } else {
    ALOGD("%s: XPAN prop not enabled, proceeding without initialzing wifi vendor lib",
        __func__);
  }
#endif

  status = true;

update_status:
  if (data_handler)
    data_handler->XMInitialized(status);
  else {
  ALOGE("%s: DataHandler is nullptr, returning from here", __func__);	  
  }

  /* Terminate main thread if status is set to false */
  if (!status)
    return;

  fd_watcher_.WatchFdForNonBlockingReads(glink_fd,
  [](int fd) {packetizer. OnDataReady(fd); });

  fd_watcher_.WatchFdForNonBlockingReads(kp_fd,
  [](int fd) { packetizer.OnDataReady(fd); });

  if (std::atomic_exchange(&main_thread_running, true)) return;
  NotifyLoglvl();

  while (main_thread_running) {
    ALOGI("waiting for event");
    std::atomic_exchange(&is_main_thread_busy, false);
    std::unique_lock<std::mutex> lck(xm_wq_mtx);
    xm_wq_notifier.wait(lck);
    lck.unlock();
    std::atomic_exchange(&is_main_thread_busy, true);
    ALOGI("event received");
    while(1)
    {
      xm_wq_mtx.lock();
      if(xm_workqueue.empty())
      {
        xm_wq_mtx.unlock();
        break;
      } else {
        xm_ipc_msg_t *msg = xm_workqueue.front();
        xm_workqueue.pop();
        xm_wq_mtx.unlock();
        XmIpcMsgHandler(msg);
      }
    }
  }

  ALOGI("%s: is stopped", __func__);
}

void XpanManager::usr_handler(int /* s */)
{
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

int XpanManager::Deinitialize(bool is_xpan_supported)
{
  fd_watcher_.StopWatchingFileDescriptors();

  if (!std::atomic_exchange(&main_thread_running, false)) {
    ALOGW("%s: main thread already stopped", __func__);
  }
  ALOGI("%s clearing out pending message", __func__);
  /* Unqueue all the pending messages */
  xm_wq_mtx.lock();
  while(!xm_workqueue.empty()) {
    xm_ipc_msg_t *msg = xm_workqueue.front();
    xm_workqueue.pop();
  }
  xm_wq_mtx.unlock();
  {
    std::unique_lock<std::mutex> lck(xm_wq_mtx);
    xm_wq_notifier.notify_all();
  }
  ALOGI("%s cleared pending message", __func__);

  if (main_thread.joinable()) {
   ALOGD("%s: sending SIGUSR1 signal", __func__);
   pthread_kill(main_thread.native_handle(), SIGUSR1);
   ALOGD("%s: joining main thread", __func__);
   main_thread.join();
   ALOGI("%s: joined main thread", __func__);
  }

#ifdef XPAN_ENABLED
  if (is_xpan_supported) {
    wifi_if.DeInitWifiIf();
    profile_if.Deinitialize();
  }
#endif

  if (glink_transport) {
    glink_transport->CloseGlinkChannel(glink_fd);
    delete glink_transport;
  }

  if (kp_transport) {
    kp_transport->CloseKpTransport(kp_fd);
    delete kp_transport;
  }

  if (main_instance) {
    main_instance.reset();
    main_instance = NULL;
  }
 return 0;
}

int XpanManager::Initialize(bool is_xpan_supported)
{
  main_thread = std::thread([this, is_xpan_supported]() {
  struct sigaction old_sa, sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = usr_handler;
  sigaction(SIGUSR1, &sa, &old_sa);
  ALOGD("%s: Started XPAN manager main thread", __func__);
  XpanManagerMainThreadRoutine(is_xpan_supported);
  });
  if (!main_thread.joinable()) return -1;
  return 0;
}

void XpanManager::NotifyLoglvl(void)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  uint8_t val;
  ALOGI("%s", __func__);
  property_get("persist.vendor.service.cpdata.loglvl", value, XM_CP_DEFAULT_LOG_LVL);
  ALOGI("%s: CP Data log level set to %s", __func__, value);
  val = (uint8_t)strtoul (value, NULL, 16);
  msg->Loglvl.eventId = XM_CP_LOG_LVL;
  msg->Loglvl.len = 1;
  msg->Loglvl.data = val;
  PostMessage(msg);
}

void XpanManager::NotifyCoPVer(uint8_t len, uint8_t *data)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGI("%s Rx CoP ver", __func__);
  msg->CoPInd.eventId = DH_XM_COP_VER_IND;
  msg->CoPInd.len = len;
  msg->CoPInd.data = data;
  PostMessage(msg);
}

void XpanManager::GetMainThreadState(void)
{
  ALOGD("%s: main_thread_running :%d ", __func__, main_thread_running.load());
  ALOGD("%s:  xm_workqueue:%p xm_wq_mtx :%p xm_wq_notifier %p", __func__, &xm_workqueue, &xm_wq_mtx, &xm_wq_notifier);
}

bool XpanManager::PostMessage(xm_ipc_msg_t * msg)
{
  if (!main_thread_running) {
    ALOGW("%s: Main worker thread is not ready to process this message: %s",
          __func__, ConvertMsgtoString(msg->eventId));
    free(msg);
    return false;
  }

  xm_wq_mtx.lock();
  xm_workqueue.push(msg);
  xm_wq_mtx.unlock();
 /* Main thread anyways get the data from queue.
  * if it is idle due to no data notify it that
  * it has data to process in its queue.
  */
 if (!is_main_thread_busy) {
   std::unique_lock<std::mutex> lck(xm_wq_mtx);
   xm_wq_notifier.notify_all();
 }

 return true;
}

#ifdef XPAN_ENABLED

void XpanManager::QhciPrepareAudioBearerReq(xm_ipc_msg_t *msg)
{
  XmIpcEventId eventId = msg->eventId;
  QhciXmPrepareAudioBearerReq AudioBearerReq = msg->AudioBearerReq;
  TransportType type = AudioBearerReq.type;
  xm_sm_state current_state = xm_sm_get_current_state(&xm_state);

  if (type == BT && (current_state == BT_Connected ||
      current_state == BT_Connecting)) {
    ALOGW("%s: Requested transport by QHCI is %s and current state is %s",
	  __func__, TransportTypeToString(type),
	  StateToString(current_state));
    qhci_if.PrepareAudioBearerRsp(AudioBearerReq.bdaddr,
            XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT, XM_To_QHCI);
    return;
  } else if (type == XPAN && (current_state == XPAN_Connected ||
             current_state== XPAN_Connecting)) {
    ALOGW("%s: Requested transport by QHCI is %s and current state is %s",
          __func__, TransportTypeToString(type),
          StateToString(current_state));
    qhci_if.PrepareAudioBearerRsp(AudioBearerReq.bdaddr,
	    XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT, XM_To_QHCI);
    return;
  }

  ALOGI("%s: Req transport by QHCI is %s current_state is %s",
	__func__, TransportTypeToString(type),
	StateToString(current_state));
  if (type == BT)
    xm_sm_set_current_state(&xm_state, BT_Connecting);
  else
    xm_sm_set_current_state(&xm_state, XPAN_Connecting);

  /* As of now there is no request can be sent from QHCI to prepare
   * Audio bearer req for BT. If this is added handle to send a delayed
   * message to XP.
   */
  AudioBearerData.eventId = QHCI_XM_PREPARE_AUDIO_BEARER_REQ;
  AudioBearerData.orginator = QHCI_To_XM;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_XP;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_CP;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_KP;
  AudioBearerData.rx_bearer_ind = false;
  memcpy(&AudioBearerData.bdaddr, &AudioBearerReq.bdaddr, sizeof(bdaddr_t));
  start_timer(audio_bearer, XM_AUDIO_BEARER_TIMEOUT);
  profile_if.ProcessMessage(eventId, msg);
  packetizer.ProcessMessage(eventId, msg);
}

void XpanManager::ConvertToXmStatus(uint8_t pkt_from, RspStatus *status)
{
  if (pkt_from == XM_KP) {
   if (*status > XM_KP_MSG_INVALID)
     *status = (RspStatus) XM_KP_MSG_INVALID;
   else
     *status = (RspStatus)(XM_KP_ERROR_REASON_OFFSET + (*status));  
  }
}

int XpanManager::GetStatsInterval(void)
{
  int interval;
  if (usecase == USECASE_XPAN_LOSSLESS)
    interval = 100;
  else if (usecase == USECASE_XPAN_GAMING)
    interval = 100;
  else if (usecase == USECASE_XPAN_VBC)
    interval = 100;
  else {
    ALOGE("%s:%s", UseCaseToString(usecase));
    interval = 0;
  }
  return interval;
}

void XpanManager::UpdateStats(bool enable)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  msg->EnableStats.eventId = XM_WIFI_ENABLE_STATS;
  msg->EnableStats.enable = enable;
  msg->EnableStats.interval = enable ? GetStatsInterval(): 0;
  PostMessage(msg);
}

void XpanManager::PrepareAudioBearerRsp(XmIpcEventId eventId, xm_ipc_msg_t *msg)
{
  RspStatus status = XM_SUCCESS;
  uint8_t pkt_from = XM_ORG_INVALID; 
  bdaddr_t bdaddr = ACTIVE_BDADDR;
  xm_sm_state current_state  = xm_sm_get_current_state(&xm_state);

  memcpy(&bdaddr, &AudioBearerData.bdaddr, sizeof(bdaddr_t));
  if (current_state == BT_Connected || current_state == XPAN_Connected ||
      current_state == IDLE) {
    ALOGW("%s: discarding this event %s as state is :%s", __func__,
	  ConvertMsgtoString(eventId), StateToString(current_state));
    return;
  }
  
  if (eventId == XP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = msg->AudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from profile with status:%s",
	  __func__, StatusToString(status));
    if (status == XM_SUCCESS) {
      if (!strncmp((const char *)&bdaddr, (const char *)&msg->AudioBearerReq.bdaddr, sizeof(bdaddr_t))) {
        ALOGI("%s bd address matched:%s", __func__, ConvertRawBdaddress(bdaddr));  
      } else {
        ALOGE("%s bdaddress mis-matched req generated on %s and req performed %s",
	      __func__, ConvertRawBdaddress(bdaddr),
	      ConvertRawBdaddress(AudioBearerData.bdaddr));
        status = XM_XP_PERFORMED_ON_WRONG_BD_ADDRESS;	
      }
    }
    pkt_from = XM_XP;
  } else if (eventId == KP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = (RspStatus)msg->KpAudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from kp with status:%s",
	  __func__, StatusToString(status));
    pkt_from = XM_KP;
  } else if (eventId == CP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = (RspStatus)msg->CpAudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from cp with status:%s",
	  __func__, StatusToString(status));
    pkt_from = XM_CP;
    if (status == XM_SUCCESS) {
      AudioBearerData.stats_enabled  = true;
      //UpdateStats(true);
    }
  } else if (eventId == XM_PREPARE_AUDIO_BEARER_TIMEOUT) {
    ALOGI("%s: Recevied audio bearer timeout", __func__);
    status = XM_FAILED_DUE_TO_AUDIO_BEARER_TIMEOUT;
  }

  if (status == XM_SUCCESS) {
    AudioBearerData.waiting_for_rsp = (AudioBearerData.waiting_for_rsp &
	                              (~(0x01 << pkt_from)));
    if (!AudioBearerData.waiting_for_rsp) {
      ALOGI("%s: Rsp recevied from all modules", __func__);
      if (AudioBearerData.orginator == QHCI_To_XM) {
        qhci_if.PrepareAudioBearerRsp(bdaddr, status, XM_To_QHCI);
      } else {
        ALOGE("%s: No orginator...", __func__);
      }
    }
  } else {
    ConvertToXmStatus(pkt_from, &status);
    ALOGE("%s: Recevied prepare audio bearer rsp with failure status :%s",
	  __func__, StatusToString(status));
    xm_sm_revert_current_state(&xm_state);
    stop_timer(audio_bearer);
    if (AudioBearerData.stats_enabled)
      UpdateStats(false);
    uint8_t waiting_for_rsp = AudioBearerData.waiting_for_rsp;
    /* waiting_for_rsp stands zero if prepare audio bearer rsp is recevied
     * from all the clients and XM notified to orginator, but Bearer ind
     * is not recevied from the XP. In that case update bearer ind to
     * orginator with failed status
     */
    if (waiting_for_rsp == 0) {
      ALOGE("%s: may be this is a timeout due to bearer indication", __func__);
      if (AudioBearerData.orginator == QHCI_To_XM) {
        qhci_if.BearerSwitchInd(bdaddr, status);
      }
      packetizer.KpBearerSwitchInd(status);
      packetizer.CpBearerSwitchInd(status);
      profile_if.XmXpBearerSwitchInd(bdaddr, status);
    } else if (AudioBearerData.orginator == QHCI_To_XM) {
      qhci_if.PrepareAudioBearerRsp(bdaddr, status, XM_To_QHCI);
      packetizer.KpBearerSwitchInd(status);
      packetizer.CpBearerSwitchInd(status);
      profile_if.XmXpBearerSwitchInd(bdaddr, status);
    } else {
      ALOGE("%s: No orginator...", __func__);
    }
 
    ALOGI("Updated failure status %s to all clients", __func__);
    memset(&AudioBearerData, 0, sizeof(XmAudioBearerReq));
    return;
  }

  /* we reach below block if profile has responded with audio bearer indications
   * and prepare audio bearer rsp is delayed from other clients. If respone
   * recevied before timeout. Update AudioBearer ind clients with success
   * status as we reach here if only profile notified audio bearer ind with
   * status success.
   */
  if (!AudioBearerData.waiting_for_rsp && AudioBearerData.rx_bearer_ind) {
    ALOGI("%s: Rx audio bearer rsp from all the waiting clients", __func__);
    ALOGI("%s: Rx AudioBearer indcation too, so updating now to all clients",
	  __func__);
    packetizer.KpBearerSwitchInd(XM_SUCCESS);
    packetizer.CpBearerSwitchInd(XM_SUCCESS);
    qhci_if.BearerSwitchInd(bdaddr, XM_SUCCESS);
    memset(&AudioBearerData, 0, sizeof(XmAudioBearerReq));
    stop_timer(audio_bearer);
    xm_sm_move_to_next_state(&xm_state);
  }
}

void XpanManager::BearerSwitchInd(xm_ipc_msg_t *msg)
{
  RspStatus status = msg->XpBearerSwitchInd.status;
  bdaddr_t bdaddr = ACTIVE_BDADDR;
  xm_sm_state current_state  = xm_sm_get_current_state(&xm_state);
  uint8_t waiting_for_rsp = AudioBearerData.waiting_for_rsp;

  if (current_state == BT_Connected || current_state == XPAN_Connected ||
      current_state == IDLE) {
    ALOGW("%s: discarding this event %s as state is :%s", __func__,
	  ConvertMsgtoString(msg->eventId), StateToString(current_state));
    return;
  }

  ALOGI("%s: profile notified with status :%s", __func__, StatusToString(status));
  
  AudioBearerData.rx_bearer_ind = true;
  if (status == XM_SUCCESS) {
    if (!waiting_for_rsp) {
      ALOGI("%s: stoping audio bearer timer as all modules notified with rsp",
	    __func__);
      stop_timer(audio_bearer);
      xm_sm_move_to_next_state(&xm_state);
    } else {
      if (IS_BIT_SET(waiting_for_rsp, XM_CP))
        ALOGW("%s: Waiting for prepare audio bearer rsp from CP", __func__);
      if (IS_BIT_SET(waiting_for_rsp, XM_KP))
        ALOGW("%s: Waiting for prepare audio bearer rsp from KP", __func__);
      if (IS_BIT_SET(waiting_for_rsp, XM_QHCI))
        ALOGW("%s: Waiting for prepare audio bearer rsp from QHCI", __func__);
      return;
    }
  } else {
    ALOGI("%s: stoping audio bearer timer", __func__);
    stop_timer(audio_bearer);
    xm_sm_revert_current_state(&xm_state);
    if (AudioBearerData.stats_enabled)
      UpdateStats(false);
  }

  packetizer.KpBearerSwitchInd(status);
  packetizer.CpBearerSwitchInd(status);
  qhci_if.BearerSwitchInd(bdaddr, status);
  memset(&AudioBearerData, 0, sizeof(XmAudioBearerReq));
  return;
}

void XpanManager::QhciUnPrepareAudioBearerReq(xm_ipc_msg_t *msg)
{
  XmIpcEventId eventId = msg->eventId;
  QhciXmUnPrepareAudioBearerReq UnPrepareAudioBearerReq = msg->UnPrepareAudioBearerReq;
  TransportType type = UnPrepareAudioBearerReq.type;
  xm_sm_state current_state = xm_sm_get_current_state(&xm_state);

  ALOGW("%s: Requested transport by QHCI is %s and current state is %s",
        __func__, TransportTypeToString(type), StateToString(current_state));

  if (type != NONE) {
    qhci_if.UnPrepareAudioBearerRsp(UnPrepareAudioBearerReq.bdaddr,
            XM_FAILED_REQUSTED_WRONG_TRANSPORT_TYPE);
    return;
  } else if (current_state == IDLE) {
    qhci_if.UnPrepareAudioBearerRsp(UnPrepareAudioBearerReq.bdaddr,
            XM_NOTALLOWING_UNPREPARE_AS_STATE_IS_IDLE);
    return;
  }

  /* Move to Disconnecting state */
  if (current_state == XPAN_Connected || current_state == XPAN_Connecting)
    xm_sm_set_current_state(&xm_state, XPAN_Disconnecting);
  if (current_state == BT_Connected || current_state == BT_Connecting)
    xm_sm_set_current_state(&xm_state, BT_Disconnecting);

  AudioBearerData.eventId = QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ;
  AudioBearerData.orginator = QHCI_To_XM;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_XP;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_CP;
  AudioBearerData.waiting_for_rsp |= 0x01 << XM_KP;
  /* Don't wait for bearer indications */
  AudioBearerData.rx_bearer_ind = false;
  memcpy(&AudioBearerData.bdaddr, &UnPrepareAudioBearerReq.bdaddr, sizeof(bdaddr_t));
  start_timer(audio_bearer, XM_AUDIO_BEARER_TIMEOUT);
  profile_if.ProcessMessage(eventId, msg);
  packetizer.ProcessMessage(eventId, msg);
  UpdateStats(false);
  XmWifiUseCaseUpdate(usecase = USECASE_XPAN_NONE);
}

void XpanManager::UnPrepareAudioBearerRsp(XmIpcEventId eventId, xm_ipc_msg_t *msg)
{
  RspStatus status = XM_SUCCESS;
  uint8_t pkt_from;
  bdaddr_t bdaddr = ACTIVE_BDADDR;
  xm_sm_state current_state  = xm_sm_get_current_state(&xm_state);

  memcpy(&bdaddr, &AudioBearerData.bdaddr, sizeof(bdaddr_t));
  if (current_state != BT_Disconnecting && current_state != XPAN_Disconnecting) {
    ALOGW("%s: discarding this event %s as state is :%s", __func__,
	  ConvertMsgtoString(eventId), StateToString(current_state));
    return;
  }
  
  if (eventId == XP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = msg->AudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from profile with status:%s",
	  __func__, StatusToString(status));
    if (status == XM_SUCCESS) {
      if (!strncmp((const char *)&bdaddr, (const char *)&msg->AudioBearerReq.bdaddr, sizeof(bdaddr_t))) {
        ALOGI("%s bd address matched:%s", __func__, ConvertRawBdaddress(bdaddr));  
      } else {
        ALOGE("%s bdaddress mis-matched req generated on %s and req performed %s",
	      __func__, ConvertRawBdaddress(bdaddr),
	      ConvertRawBdaddress(AudioBearerData.bdaddr));
        status = XM_XP_PERFORMED_ON_WRONG_BD_ADDRESS;	
      }
    }
    pkt_from = XM_XP;
  } else if (eventId == KP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = (RspStatus)msg->KpAudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from kp with status:%s",
	  __func__, StatusToString(status));
    pkt_from = XM_KP;
  } else if (eventId == CP_XM_PREPARE_AUDIO_BEARER_RSP) {
    status = (RspStatus)msg->CpAudioBearerRsp.status;
    ALOGI("%s: received prepare audio bearer rsp from cp with status:%s",
	  __func__, StatusToString(status));
    pkt_from = XM_CP;
  } else if (eventId == XM_AUDIO_BEARER_TIMEOUT) {
    ALOGI("%s: Recevied audio bearer timeout", __func__);
    status = XM_FAILED_DUE_TO_UNPREPARE_AUDIO_BEARER_TIMEOUT;
  }

  if (status == XM_SUCCESS) {
    AudioBearerData.waiting_for_rsp = (AudioBearerData.waiting_for_rsp &
	                              (~(0x01 << pkt_from)));
    if (AudioBearerData.waiting_for_rsp)
      return;
    else
      ALOGI("%s: Rsp recevied for all other modules", __func__);
  } else {
    ALOGE("%s: Recevied prepare audio bearer rsp with failure status",
	  __func__);
  }

  xm_sm_set_current_state(&xm_state, IDLE);
  stop_timer(audio_bearer);
  qhci_if.UnPrepareAudioBearerRsp(bdaddr, status);
  memset(&AudioBearerData, 0, sizeof(XmAudioBearerReq));
}

void XpanManager::XmWifiUseCaseUpdate(UseCaseType usecase)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  ALOGI("%s usecase: %s", __func__, UseCaseToString(usecase));
  msg->UseCase.eventId = XM_WIFI_USECASE_UPDATE;
  msg->UseCase.usecase = usecase;
  PostMessage(msg);
}

void XpanManager::GetUseCase(xm_ipc_msg_t *msg)
{
  usecase = msg->UseCase.usecase;
  ALOGI("%s: current usecase set to :%s", __func__, UseCaseToString(usecase));
}
#endif

void XpanManager::XmIpcMsgHandler(xm_ipc_msg_t *msg)
{
  XmIpcEventId eventId = msg->eventId;
  ALOGI("%s: processing event %s with address %p", __func__, ConvertMsgtoString(eventId), msg);
  ALOGI("%s: eventId:%04x", __func__, eventId);

  switch (eventId) {
    case DH_XM_COP_VER_IND:
    case XM_CP_LOG_LVL: {
      packetizer.ProcessMessage(eventId ,msg);
      break;
    }
#ifdef XPAN_ENABLED
    case QHCI_XM_REMOTE_SUPPORT_XPAN:
    case WIFI_XM_ACS_RESULTS:
    case WIFI_XM_SAP_POWER_SAVE_STATE_RSP:
    case XM_XP_CREATE_SAP_INF_STATUS:
    case WIFI_XM_TWT_EVENT: {
      profile_if.ProcessMessage(eventId, msg);
      break;
    } case XP_XM_TRANSPORT_ENABLED:
      case XP_XM_BONDED_DEVICES_LIST: {
      qhci_if.ProcessMessage(eventId, msg);
      break;
    } case QHCI_XM_USECASE_UPDATE: {
      packetizer.ProcessMessage(eventId ,msg);
      profile_if.ProcessMessage(eventId, msg);
      wifi_if.PostMessage(msg);
      GetUseCase(msg);
      break;
    } case QHCI_XM_PREPARE_AUDIO_BEARER_REQ: {
      QhciPrepareAudioBearerReq(msg);
      break;
    } case KP_XM_PREPARE_AUDIO_BEARER_RSP: 
      case CP_XM_PREPARE_AUDIO_BEARER_RSP: 
      case XP_XM_PREPARE_AUDIO_BEARER_RSP:
      case XM_PREPARE_AUDIO_BEARER_TIMEOUT: {
      if(AudioBearerData.eventId == QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ)
	UnPrepareAudioBearerRsp(eventId, msg);
      else if (AudioBearerData.eventId == QHCI_XM_PREPARE_AUDIO_BEARER_REQ)
        PrepareAudioBearerRsp(eventId, msg);
      else
        ALOGW("%s: AudioBearerData might have cleared", __func__);
      break;
    } case XP_XM_AUDIO_BEARER_SWITCHED: {
      BearerSwitchInd(msg);
      break;
    } case XP_XM_ENABLE_ACS:
      case XM_WIFI_ENABLE_STATS :
      case XP_XM_SAP_POWER_STATE :
      case XP_XM_SAP_CURRENT_STATE :
      case XP_XM_CREATE_SAP_INF :
      case XM_WIFI_USECASE_UPDATE: {
      wifi_if.PostMessage(msg);
      break;
    } case XP_XM_HOST_PARAMETERS:
      case XP_XM_TWT_SESSION_EST:
      case XP_XM_BEARER_PREFERENCE_IND: {
      packetizer.ProcessMessage(eventId ,msg);
      if (eventId == XP_XM_TWT_SESSION_EST) {
        bool enableStats = false;
        if (msg->TwtParams.num_devices)
          enableStats = true;
        UpdateStats(enableStats);
      }
      break;
    } case QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ: {
      QhciUnPrepareAudioBearerReq(msg);
      break;
    } case CP_XM_DELAY_REPORTING : {
      qhci_if.DelayReporting(msg);
      break;
    } case CP_XM_TRANSPORT_UPDATE: {
      qhci_if.TransportUpdate(msg);
      profile_if.ProcessMessage(eventId, msg);
    } 
#endif
  }
  free(msg);
}

} // namespace xpan
} // namaespace implementation;

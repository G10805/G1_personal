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
#include "xm_wifi_if.h"
#include "xpan_wifi_hal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.xpan@1.0-xmwifiif"

namespace xpan {
namespace implementation {

std::map<uint8_t, uint64_t> dialog_id_mapper;
std::mutex dialog_id_mtk;

static inline char * WiFiStatus(xpan_wifi_status type)
{
  if (type == XPAN_WIFI_STATUS_SUCCESS)
    return "XPAN_WIFI_STATUS_SUCCESS";
  else if (type == XPAN_WIFI_STATUS_FAILURE)
    return "XPAN_WIFI_STATUS_FAILURE";
  else
    return "INVALID STATE";
}

static inline char * TwtEventType(wifi_twt_oper_type type)
{
  if (type == TWT_OPER_EVENT_SETUP)
    return "TWT_OPER_EVENT_SETUP";
  else if (type == TWT_OPER_EVENT_TERMINATE)
    return "TWT_OPER_EVENT_TERMINATE";
  else if (type == TWT_OPER_EVENT_SUSPEND)
    return "TWT_OPER_EVENT_SUSPEND";
  else if (type == TWT_OPER_EVENT_RESUME)
    return "TWT_OPER_EVENT_RESUME";
  else
    return "INVALID EVENT";
}

static inline char *AcsEventType(uint8_t status)
{
  if (status == ACS_STARTED)
    return "ACS_STARTED";
  else if (status == ACS_FAILURE)
    return "ACS_FAILURE";
  else if (status == ACS_COMPLETED)
    return "ACS_COMPLETED";
  else
    return "INVALID EVENT";
}

static inline char *SapCurrentState(xpan_sap_state state)
{
  if (state == XPAN_SAP_STATE_DISABLING)
    return "XPAN_SAP_STATE_DISABLING";
  else if (state == XPAN_SAP_STATE_DISABLED)
    return "XPAN_SAP_STATE_DISABLED";
  else if (state == XPAN_SAP_STATE_ENABLING)
    return "XPAN_SAP_STATE_ENABLING";
  else if (state == XPAN_SAP_STATE_ENABLED)
    return "XPAN_SAP_STATE_ENABLED";
  else if (state == XPAN_SAP_STATE_FAILED)
    return "XPAN_SAP_STATE_FAILED";
  else
    return "INVALID EVENT";
}

void cb_wifi_acs_results(int, acs_status);
void cb_wifi_twt_event(twt_event_response);
void cb_ap_power_save_event(ap_power_save_data);

struct wifi_drv_ops *callback;
wifi_handle handle;
struct xpan_wifi_data *wifi_if_data;
struct wifi_callbacks xm_callback = {cb_wifi_acs_results,
	                             cb_wifi_twt_event,
                                     cb_ap_power_save_event};

XMWifiIf::XMWifiIf()
{
  dialog_id_mapper.clear();
}

XMWifiIf::~XMWifiIf()
{
}

void cb_wifi_acs_results(int freq, acs_status status)
{
  XMWifiIf::CbWiFiAcsResults(freq, uint8_t (status));
}

void cb_wifi_twt_event(twt_event_response response)
{
  XMWifiIf::CbWiFiTwtEvent((void *)&response);
}

void cb_ap_power_save_event(ap_power_save_data data)
{
  XMWifiIf::CbSapPowerState((void *)&data);
}

static inline uint8_t getDialogId(uint64_t cookie)
{
  std::unique_lock<std::mutex> lck(dialog_id_mtk);
  uint8_t dialog_id = 0;
  for (auto& it : dialog_id_mapper) {
     if (cookie == it.second) {
       dialog_id = it.first;
       ALOGD("%s: cookie %u matched with dialog id :%d", __func__, cookie, dialog_id);
       dialog_id_mapper.erase(dialog_id);
       break;
     }
  }

  return dialog_id;
}

static inline void map_and_add_dialogId_to_cookie(uint8_t dialog_id, uint64_t *cookie)
{
  std::unique_lock<std::mutex> lck(dialog_id_mtk);
  for (auto& it : dialog_id_mapper) {
     if (dialog_id == it.first) {
       ALOGE("%s dialog_id %d is already part of list with cookie %u",
             __func__, it.first, it.second);
       dialog_id_mapper.erase(it.first);
       break;
     }
  }
  dialog_id_mapper.insert(std::pair{dialog_id, *cookie});
}

void XMWifiIf:: CbSapPowerState(void *data)
{
  ap_power_save_data *rsp = (ap_power_save_data *)data;
  ALOGI("%s cookie:%u power_save_bi_multiplier :%d next_tsf %u",
	__func__, rsp->cookie, rsp->power_save_bi_multiplier, rsp->next_tsf);

  uint64_t cookie = rsp->cookie;
  uint8_t dialog_id = getDialogId(cookie);
  if (!dialog_id) {
    ALOGE("%s: dialog_id is missing for cookie %u.discarding here even here",
	  __func__, cookie);
    return;
  }

  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  msg->eventId = WIFI_XM_SAP_POWER_SAVE_STATE_RSP;
  msg->SapPowerStateRsp.dialog_id = dialog_id;
  msg->SapPowerStateRsp.power_save_bi_multiplier = rsp->power_save_bi_multiplier;
  msg->SapPowerStateRsp.next_tsf = rsp->next_tsf;
  XpanManager::Get()->PostMessage(msg);
}

void XMWifiIf:: CbWiFiAcsResults(int freq, uint8_t status)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGI("%s: with frequency %d and id %d and status: %s", __func__, freq,
	status, AcsEventType(status));
  msg->eventId = WIFI_XM_ACS_RESULTS;
  msg->AcsResults.freq = freq;
  msg->AcsResults.status = status;
  XpanManager::Get()->PostMessage(msg);
}

void XMWifiIf:: CbWiFiTwtEvent(void *rsp)
{
  twt_event_response *response = (twt_event_response *)rsp;
  ALOGI("%s", __func__);

  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->eventId = WIFI_XM_TWT_EVENT;
  msg->TwtEvent.twt_event  = response->event_type;
  msg->TwtEvent.wake_dur_us = response->wake_dur_us;
  msg->TwtEvent.wake_int_us = response->wake_int_us;

  memcpy(&msg->TwtEvent.addr, response->addr, sizeof(macaddr_t));

  ALOGI("%s: event type:%s for Mac Address %s with wake dur:%d wake interval:%d",
        __func__, TwtEventType(response->event_type),
        ConvertRawMacaddress(msg->TwtEvent.addr), response->wake_dur_us,
	response->wake_int_us);
  XpanManager::Get()->PostMessage(msg);
}

void XMWifiIf::usr_handler1(int /* s */)
{
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

void XMWifiIf::usr_handler(int /* s */)
{
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

void XMWifiIf::XMWiFiThreadRoutine()
{
  if (std::atomic_exchange(&xm_wifi_if_thread_running, true)) return;

  while (xm_wifi_if_thread_running) {
    ALOGI("waiting for event");
    std::atomic_exchange(&is_wifi_if_thread_busy, false);
    std::unique_lock<std::mutex> lck(xm_wifi_if_mtx);
    xm_wifi_if_wq_notifier.wait(lck);
    lck.unlock();
    std::atomic_exchange(&is_wifi_if_thread_busy, true);
    ALOGI("event received");
    while(1)
    {
      xm_wifi_if_mtx.lock();
      if(xm_wifi_if_workqueue.empty())
      {
        xm_wifi_if_mtx.unlock();
        break;
      } else {
        xm_ipc_msg_t *msg = xm_wifi_if_workqueue.front();
        xm_wifi_if_workqueue.pop();
        xm_wifi_if_mtx.unlock();
        WiFiIpcMsgHandler(msg);
      }
    }
  }

  ALOGI("%s: is stopped", __func__);
}

/******************************************************************
 *
 * Function         InitWifiIf
 *
 * Description      This API is notify profile when a new remote
 *                  is connected.
 * Parameters:      bdaddr_t - Bluetooth address of the remote.
 *                  enable   - add a place holder always set to true.
 *
 * Return:          bool - return the status wether the message is
 *                  queued to processed by AIDL.
 ******************************************************************/
bool XMWifiIf::InitWifiIf()
{
  xpan_wifi_status status;

  ALOGI("%s: initializing wifi vendor lib", __func__);

  callback = (struct wifi_drv_ops *)malloc(sizeof(struct wifi_drv_ops));
  if (callback == nullptr) {
    ALOGE("%s: failed to allocate memory for wifi drv ops", __func__);
    return false;
  }

  status = init_xpan_wifi_lib_function_table(callback);
  ALOGI("%s: registered wifi vendor lib with status %s", __func__,
        WiFiStatus(status));
  if (status == XPAN_WIFI_STATUS_FAILURE) {
    free(callback);
    callback = nullptr;
    return false;
  }

  if (callback->init_xpan_wifi_lib) {
    void* data = (xpan_wifi_data *)callback->init_xpan_wifi_lib(&handle);
    if (data == nullptr) {
      ALOGI("%s: Failed to initialize wifi vendor lib", __func__);
      free(callback);
      callback = nullptr;
      return false;
    }
    wifi_if_data = (xpan_wifi_data*)data ;
  }

  callback->register_callbacks(wifi_if_data, &xm_callback);
  /* Not sure whether BT was closed properly. So deleting interface
   * if available, WiFi vendor lib will delete it.
   */
  callback->wifi_delete_ap_iface(wifi_if_data);

  ALOGI("%s: creating event handler thread", __func__);
  /* A dedicated thread is needed for WiFi vendor lib to
   * send events to XM.
   */
  wifi_event_handling_thread = std::thread([]() {
    struct sigaction old_sa, sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = usr_handler;
    sigaction(SIGUSR1, &sa, &old_sa);
    ALOGD("%s: Started WiFi event handling thread", __func__);
    callback->xpan_wifi_event_loop(wifi_if_data);
  });

  if (!wifi_event_handling_thread.joinable()) {
    ALOGE("%s: failed to create wifi event handling thread", __func__);
    if (callback) {
      callback->deregister_callbacks(wifi_if_data);
      /* This will ensure that event thread will be stopped */
      callback ->deinit_xpan_wifi_lib(wifi_if_data);
    }
    free(callback);
    callback = nullptr;
    return false;
  }

  /* This thread is used to send the packet to WiFi vendor
   * lib. The same thread is used by wifi to interacts with
   * WiFi FW.
   */
  xm_wifi_if_thread = std::thread([this]() {
    struct sigaction old_sa, sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = usr_handler1;
    sigaction(SIGUSR1, &sa, &old_sa);
    ALOGD("%s: Started XM WiFi interface thread", __func__);
    XMWiFiThreadRoutine();
  });

  if (!xm_wifi_if_thread.joinable()) {
    ALOGE("%s: failed to create XM WiFi interface thread", __func__);
    if (wifi_event_handling_thread.joinable()) {
     ALOGD("%s: sending SIGUSR1 signal to WiFi Event thread", __func__);
     pthread_kill(wifi_event_handling_thread.native_handle(), SIGUSR1);
     ALOGD("%s: joining to WiFi Event thread", __func__);
     wifi_event_handling_thread.join();
     ALOGI("%s: joined to WiFi Event thread", __func__);
    }
    if (callback) {
      callback->deregister_callbacks(wifi_if_data);
      /* This will ensure that event thread will be stopped */
      callback ->deinit_xpan_wifi_lib(wifi_if_data);
    }
    free(callback);
    callback = nullptr;
    return false;
  }

end:
  return true;
}

bool XMWifiIf::DeInitWifiIf()
{
  if (!std::atomic_exchange(&xm_wifi_if_thread_running, false)) {
    ALOGW("%s: XM WiFi interface thread already stopped", __func__);
  }

  ALOGI("%s clearing out pending message", __func__);
  /* Unqueue all the pending messages */
  xm_wifi_if_mtx.lock();
  while(!xm_wifi_if_workqueue.empty()) {
    xm_ipc_msg_t *msg = xm_wifi_if_workqueue.front();
    xm_wifi_if_workqueue.pop();
  }
  xm_wifi_if_mtx.unlock();
  {
    std::unique_lock<std::mutex> lck(xm_wifi_if_mtx);
    xm_wifi_if_wq_notifier.notify_all();
  }
  ALOGI("%s cleared pending message", __func__);

  if (xm_wifi_if_thread.joinable()) {
   ALOGD("%s: sending SIGUSR1 signal to XM WiFI If", __func__);
   pthread_kill(xm_wifi_if_thread.native_handle(), SIGUSR1);
   ALOGD("%s: joining XM WiFI If thread", __func__);
   xm_wifi_if_thread.join();
   ALOGI("%s: joined XM WiFI If thread", __func__);
  }

  if (wifi_event_handling_thread.joinable()) {
   ALOGD("%s: sending SIGUSR1 signal to WiFi Event thread", __func__);
   pthread_kill(wifi_event_handling_thread.native_handle(), SIGUSR1);
   ALOGD("%s: joining to WiFi Event thread", __func__);
   wifi_event_handling_thread.join();
   ALOGI("%s: joined to WiFi Event thread", __func__);
  }

  if (callback) {
    /* Not sure whether AP interface created, Trigger delete API
     * So if created it may deleted it or else it return false
     */
    callback->wifi_delete_ap_iface(wifi_if_data);
    callback->deregister_callbacks(wifi_if_data);
    /* This will ensure that event thread will be stopped */
    callback ->deinit_xpan_wifi_lib(wifi_if_data);
    free(callback);
    callback = nullptr;
  }

  return true;
}

bool XMWifiIf::PostMessage(xm_ipc_msg_t * msg)
{
  xm_ipc_msg_t *wifi_if_msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  XmIpcEventId eventId = msg->eventId;

  if (wifi_if_msg == nullptr) {
    ALOGW("%s: Failed to allocate message", __func__);
    return false;
  }

  if (!xm_wifi_if_thread_running) {
    ALOGW("%s: WiFi vendor interface is not ready to process this message, trying again: %s",
          __func__, ConvertMsgtoString(msg->eventId));
    // Try 1 more time to check if by now Wifi interface is update
    if (!InitWifiIf()) {
      ALOGW("%s: WiFi vendor interface is still not ready to process this message: %s",
            __func__, ConvertMsgtoString(msg->eventId));
      free(wifi_if_msg);
      return false;
    }

    if (!xm_wifi_if_thread.joinable() || !wifi_event_handling_thread.joinable()) {
      ALOGW("%s: WiFi vendor interface is not ready to process this message: %s",
            __func__, ConvertMsgtoString(msg->eventId));
      free(wifi_if_msg);
      return false;
    }
  }

  /* Copy separately for every message */
  switch (eventId) {
    case QHCI_XM_USECASE_UPDATE:
    case XM_WIFI_USECASE_UPDATE: {
      wifi_if_msg->UseCase.eventId = eventId;
      wifi_if_msg->UseCase.usecase = msg->UseCase.usecase;
      break;
    } case XP_XM_ENABLE_ACS: {
      wifi_if_msg->Acslist.eventId = eventId;
      wifi_if_msg->Acslist.freq_list_size = msg->Acslist.freq_list_size;
      wifi_if_msg->Acslist.freq_list =
	      (int *)malloc(sizeof(int) * msg->Acslist.freq_list_size);
      for (int i = 0 ; i < msg->Acslist.freq_list_size; i++)
        wifi_if_msg->Acslist.freq_list[i] = msg->Acslist.freq_list[i];
      /* Clear previoulsy allocated memory */
      free(msg->Acslist.freq_list);
      break;
    } case XM_WIFI_ENABLE_STATS : {
      wifi_if_msg->EnableStats.eventId = eventId;
      wifi_if_msg->EnableStats.enable = msg->EnableStats.enable;
      wifi_if_msg->EnableStats.interval = msg->EnableStats.interval;
      break;
    } case XP_XM_SAP_POWER_STATE: {
      wifi_if_msg->SapPowerStateParams.eventId = eventId;
      wifi_if_msg->SapPowerStateParams.dialog_id =
	      msg->SapPowerStateParams.dialog_id;
      wifi_if_msg->SapPowerStateParams.state =
	      msg->SapPowerStateParams.state;
      break;
    } case XP_XM_SAP_CURRENT_STATE: {
      wifi_if_msg->SapState.eventId = msg->SapState.eventId;
      wifi_if_msg->SapState.state = msg->SapState.state;
      break;
    } case XP_XM_CREATE_SAP_INF: {
      wifi_if_msg->CreateSapInfParams.eventId = msg->CreateSapInfParams.eventId;
      wifi_if_msg->CreateSapInfParams.state = msg->CreateSapInfParams.state;
      break;
    } default :{
      ALOGW("%s: unhandled event: %s", __func__, ConvertMsgtoString(msg->eventId));
      free(wifi_if_msg);
      return false;
    }
  }

  xm_wifi_if_mtx.lock();
  xm_wifi_if_workqueue.push(wifi_if_msg);
  xm_wifi_if_mtx.unlock();
 /* Wifi vendor if anyways get the data from queue.
  * if it is idle due to no data notify it that
  * it has data to process in its queue.
  */
 if (!is_wifi_if_thread_busy) {
   std::unique_lock<std::mutex> lck(xm_wifi_if_mtx);
   xm_wifi_if_wq_notifier.notify_all();
 }

 return true;
}

void XMWifiIf::UseCaseUpdate(xm_ipc_msg_t *msg)
{
  struct xpan_usecase_params params;
  xpan_wifi_status status;
  QhciXmUseCase UseCase = msg->UseCase;
  params.mode = (xpan_usecase_type)UseCase.usecase;

  if(callback) {
    status = callback->set_xpan_usecase_params(wifi_if_data, &params);
    ALOGI("%s: updated usecase :%d to wifi vendor lib with status %s",
           __func__, params.mode, WiFiStatus(status));
  } else {
      ALOGE("%s: Failed to update usecase to wifi vendor lib", __func__);
  }

  return;
}

void XMWifiIf::EnableAcs(xm_ipc_msg_t * msg)
{
  int *freq_list = msg->Acslist.freq_list;
  xpan_wifi_status status = XPAN_WIFI_STATUS_FAILURE;
  int freq_list_size = msg->Acslist.freq_list_size;

  ALOGE("%s: freq_list_size:%d", __func__, freq_list_size);
  for (int i = 0; i < freq_list_size ; i++) {
    ALOGE("freq_list[%d]= %d ", i, freq_list[i]);
  }

  status = callback->wifi_do_acs(wifi_if_data, freq_list, freq_list_size);
  ALOGI("%s: requested wifi vendor do acs with status %s", __func__,
	WiFiStatus(status));

  /* Let Profile know on this failure */
  if (status == XPAN_WIFI_STATUS_FAILURE) {
    CbWiFiAcsResults(0, XM_FAILED_TO_ESTABLISH_ACS); 
  }
}

void XMWifiIf::EnableStats(xm_ipc_msg_t *msg)
{
  bool enable = msg->EnableStats.enable;
  int interval = msg->EnableStats.interval;
  ALOGI("%s:%s",__func__,  enable ? "enable": "disable");
  if(callback) {
   if (callback->set_xpan_wifi_stats_enabled(wifi_if_data, enable, interval) ==
	XPAN_WIFI_STATUS_FAILURE)
     ALOGE("%s: failed to %s stats", __func__,  enable ? "enable": "disable");
  }
}

void XMWifiIf::SapPowerState(xm_ipc_msg_t *msg)
{
  bool enable = (msg->SapPowerStateParams.state & 0x01);
  uint8_t dialog_id  = msg->SapPowerStateParams.dialog_id;
  uint64_t *cookie = (uint64_t *)&msg->SapPowerStateParams.dialog_id;
  ALOGI("%s with dialog_id %d and state: %s",__func__,  dialog_id,
	enable ? "enable": "disable");
  if(callback) {
   if (callback->set_xpan_ap_power_save(wifi_if_data, enable, cookie) ==
	XPAN_WIFI_STATUS_FAILURE) {
     ALOGE("%s failed to update SAP power state with dialog_id %d and state: %s",
	   __func__,  dialog_id,  enable ? "enable": "disable");
   } else {
     ALOGI("%s: dialog_id %d and its cookie :%u", __func__, dialog_id, *cookie);
     map_and_add_dialogId_to_cookie(dialog_id, cookie);   
   }
  }
}

void XMWifiIf::SapState(xm_ipc_msg_t *msg)
{
  enum xpan_sap_state state = (xpan_sap_state)(msg->SapState.state);
  ALOGI("%s Sap current state id %d :%s ",__func__, state, SapCurrentState(state));
  if (callback) {
   if (callback->set_softap_state(state) == XPAN_WIFI_STATUS_FAILURE)
     ALOGI("%s Sap current state:%s",__func__,  SapCurrentState(state));
  }
}

void XMWifiIf::CreateSapInterface(xm_ipc_msg_t *msg)
{
  uint8_t state = msg->CreateSapInfParams.state;
  xpan_wifi_status status = XPAN_WIFI_STATUS_FAILURE;
  ALOGI(" %s: with state: %s", __func__, state? "enable": "disable");

  xm_ipc_msg_t *rsp = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  rsp->CreateSapInfStatusParams.eventId = XM_XP_CREATE_SAP_INF_STATUS;
  rsp->CreateSapInfStatusParams.req_state = state;

  if(callback && state) {
    ALOGI("%s: Creating AP interface", __func__);
    status = callback->wifi_create_ap_iface(wifi_if_data) ;
    if (status == XPAN_WIFI_STATUS_SUCCESS) {
      ALOGI("%s: successfully created %s interface", __func__, wifi_if_data->ifname);
    } else {
      ALOGE("%s: failed to create interface", __func__);
    }
  } else if (callback) {
    ALOGI("%s deleting interface", __func__);
    status = callback->wifi_delete_ap_iface(wifi_if_data);
    if (status == XPAN_WIFI_STATUS_SUCCESS) {
      ALOGI("%s: successfully deleted interface", __func__);
    } else {
      ALOGE("%s: failed to delete interface", __func__);
    }
  }

  rsp->CreateSapInfStatusParams.status = (uint8_t)status;
  XpanManager::Get()->PostMessage(rsp);
}

void XMWifiIf::WiFiIpcMsgHandler(xm_ipc_msg_t *msg)
{
  XmIpcEventId eventId = msg->eventId;
  ALOGI("%s: processing event :%s", __func__, ConvertMsgtoString(eventId));

  switch (eventId) {
    case QHCI_XM_USECASE_UPDATE:
    case XM_WIFI_USECASE_UPDATE: {
      UseCaseUpdate(msg);
      break;
    } case XP_XM_ENABLE_ACS: {
      EnableAcs(msg);
      break;
    } case XM_WIFI_ENABLE_STATS : {
      EnableStats(msg);
      break;
    } case XP_XM_SAP_POWER_STATE: {
      SapPowerState(msg);
      break;
    } case XP_XM_SAP_CURRENT_STATE: {
      SapState(msg);
      break;
    } case XP_XM_CREATE_SAP_INF: {
      CreateSapInterface(msg);
      break;
    }
  }

  free(msg);
}

} //namespace implementation
} //namespace xpan

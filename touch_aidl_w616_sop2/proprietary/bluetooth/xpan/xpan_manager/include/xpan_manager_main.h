/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <mutex>
#include <atomic>
#include <queue>
#include "xpan_glink_transport.h"
#include "xpan_kp_transport.h"
#include "xm_async_fd_watcher.h"
#include "xm_ipc_if.h"
#include "xm_state_machine.h"
#include "timer.h"

#define XM_CP_DEFAULT_LOG_LVL      "0x11"
namespace xpan {
namespace implementation {

class XpanManager
{
  public:
   XpanManager();
   ~XpanManager();
   int Initialize(bool);
   static std::shared_ptr <XpanManager> Get(void);
   int Deinitialize(bool);
   void NotifyCoPVer(uint8_t, uint8_t *);
   bool PostMessage(xm_ipc_msg_t *);
   void GetMainThreadState(void);

  private:
   void XpanManagerMainThreadRoutine(bool);
   static void usr_handler(int);
   void XmIpcMsgHandler(xm_ipc_msg_t *);
   static void AudioBearerTimeOut(union sigval);
   void QhciPrepareAudioBearerReq(xm_ipc_msg_t *);
   void PrepareAudioBearerRsp(XmIpcEventId, xm_ipc_msg_t *);
   void BearerSwitchInd(xm_ipc_msg_t *);
   void QhciUnPrepareAudioBearerReq(xm_ipc_msg_t *);
   void UnPrepareAudioBearerRsp(XmIpcEventId, xm_ipc_msg_t *);
   void ConvertToXmStatus(uint8_t, RspStatus*);
   void NotifyLoglvl(void);
   void GetUseCase(xm_ipc_msg_t *);
   void XmWifiUseCaseUpdate(UseCaseType);
   void UpdateStats(bool);
   int GetStatsInterval(void);

   /* Declared below as static to avoid mutiple references */
   std::mutex xm_wq_mtx;
   std::queue <xm_ipc_msg_t *> xm_workqueue;
   std::condition_variable xm_wq_notifier;
   std::atomic_bool is_main_thread_busy;
   std::atomic_bool main_thread_running;
   static std::shared_ptr<XpanManager> main_instance;
   struct alarm_t *audio_bearer;
   struct xm_state_machine xm_state;

   GlinkTransport *glink_transport;
   KernelProxyTransport *kp_transport;
   std::thread main_thread;
   int glink_fd;
   int kp_fd;
   XMAsyncFdWatcher fd_watcher_;
   XmAudioBearerReq AudioBearerData;
   UseCaseType usecase;
};

} // namespace implementation
} // namespace xpan

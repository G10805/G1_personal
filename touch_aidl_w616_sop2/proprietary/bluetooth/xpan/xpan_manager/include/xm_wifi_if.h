/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#ifndef XM_WIFI_IF_H
#define XM_WIFI_IF_H

#pragma once

#include <stdint.h>
#include "xpan_utils.h"
#include "xpan_manager_main.h"

namespace xpan {
namespace implementation {

class XMWifiIf
{
  public:
   XMWifiIf();
   ~XMWifiIf();
   bool InitWifiIf();
   bool DeInitWifiIf();
   bool PostMessage(xm_ipc_msg_t *);
   void WiFiIpcMsgHandler(xm_ipc_msg_t *);
   static void CbWiFiAcsResults(int freq, uint8_t status);
   static void CbWiFiTwtEvent(void*);
   static void CbSapPowerState(void *);

  private:
   std::thread wifi_event_handling_thread;
   std::thread xm_wifi_if_thread;
   void XMWiFiThreadRoutine(void);
   static void usr_handler(int);
   static void usr_handler1(int);
   void UseCaseUpdate(xm_ipc_msg_t *);
   void EnableAcs(xm_ipc_msg_t *);
   void EnableStats(xm_ipc_msg_t *);
   bool HostMacAddress(bdaddr_t);
   void SapPowerState(xm_ipc_msg_t *);
   void SapState(xm_ipc_msg_t *);
   void CreateSapInterface(xm_ipc_msg_t *);

   std::mutex xm_wifi_if_mtx;
   std::queue <xm_ipc_msg_t *> xm_wifi_if_workqueue;
   std::condition_variable xm_wifi_if_wq_notifier;
   std::atomic_bool xm_wifi_if_thread_running{false};
   std::atomic_bool is_wifi_if_thread_busy{false};
   static std::mutex dialog_id_mtk;
};

} // namespace implementation
} // namespace xpan

#endif

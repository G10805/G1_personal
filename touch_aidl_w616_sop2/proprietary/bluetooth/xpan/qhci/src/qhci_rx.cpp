/* 
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "data_handler.h"
#include "xpan/qhci/include/qhci_main.h"
#include "xpan/qhci/include/qhci_packetizer.h"
#include <xpan/qhci/include/qhci_xm_if.h>
#include <hidl/HidlSupport.h>
#include "hci_transport.h"
//#include "logger.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "xpan_qhci"
using android::hardware::bluetooth::V1_0::implementation::DataHandler;

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace xpan {
namespace implementation {

QHciRx *QHciRx::qhci_rx_instance = nullptr;

QHciRx::QHciRx() {
  ALOGD("%s", __func__);
}

QHciRx::~QHciRx() {
  ALOGD("%s", __func__);
}

QHciRx *QHciRx::Get() {
  return qhci_rx_instance;
}


void QHciRx::QHciRxThreadRoutine() {
  DataHandler *data_handler = DataHandler::Get();

  ALOGD("%s  ", __func__);
  if (data_handler) {
    //TODO
    //data_handler->QHciIntialized(true);
  } else {
    ALOGE("%s: DataHandler Not initialized", __func__);
    return;
  }

  if (std::atomic_exchange(&qhci_rx_thread_running, true)) return;

  while (qhci_rx_thread_running) {
    ALOGE("waiting for event");

    std::atomic_exchange(&is_qhci_rx_thread_busy, false);
    std::unique_lock<std::mutex> qhci_rx_lock(qhci_rx_wq_mtx);

    ALOGE("waiting for event inside before_wait_call");
    qhci_rx_wq_notifier.wait(qhci_rx_lock);
    ALOGE("waiting for event inside after qhci_rx_lock");
    qhci_rx_lock.unlock();
    std::atomic_exchange(&is_qhci_rx_thread_busy, true);

    while(1) {
      ALOGE("waiting for event inside while (1)");
      qhci_rx_wq_mtx.lock();

      if (qhci_rx_workqueue.empty()) {
        qhci_rx_wq_mtx.unlock();
        break;
      } else {
        qhci_msg_t *data = qhci_rx_workqueue.front();
        qhci_rx_workqueue.pop();
        qhci_rx_wq_mtx.unlock();
        QHciRxMsgHandler(data);
      }
    }
  }

  ALOGI("%s: is stopped", __func__);

}

void QHciRx::curr_usr_qhci_rx_handler (int) {
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

int QHciRx::Init() {
  if (!qhci_rx_instance) {
    qhci_rx_instance = new QHci();
    qhci_rx_thread = std::thread([this]() {
                       struct sigaction prev_sig, curr_sig;
                       memset(&curr_sig, 0, sizeof(curr_sig));
                       curr_sig.sa_handler = curr_usr_qhci_rx_handler;
                       sigaction(SIGUSR1, &curr_sig, &prev_sig);
                       ALOGD("%s: Starting QHCI TX thread", __func__);
                       QHciRxThreadRoutine();
                     });
    if (!qhci_rx_thread.joinable()) return -1;
  }
  return 0;
}

} // namespace implementation
} // namespace xpan
}
}
}
}


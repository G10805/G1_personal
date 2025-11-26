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

QHciTx *QHciTx::qhci_tx_instance = nullptr;

QHciTx::QHciTx() {
  ALOGD("%s", __func__);
  //logger_ = Logger::Get();
}

QHciTx::~QHciTx() {
  ALOGD("%s", __func__);
}

QHciTx *QHciTx::Get() {
  return qhci_tx_instance;
}


void QHciTx::QHciTxThreadRoutine() {
  DataHandler *data_handler = DataHandler::Get();

  ALOGD("%s  ", __func__);
  if (data_handler) {
    //TODO
    //data_handler->QHciIntialized(true);
  } else {
    ALOGE("%s: DataHandler Not initialized", __func__);
    return;
  }

  if (std::atomic_exchange(&qhci_tx_thread_running, true)) return;

  while (qhci_tx_thread_running) {
    ALOGE("waiting for event");

    std::atomic_exchange(&is_qhci_tx_thread_busy, false);
    std::unique_lock<std::mutex> qhci_tx_lock(qhci_tx_wq_mtx);

    ALOGE("waiting for event inside before_wait_call");
    qhci_tx_wq_notifier.wait(qhci_tx_lock);
    ALOGE("waiting for event inside after qhci_tx_lock");
    qhci_tx_lock.unlock();
    std::atomic_exchange(&is_qhci_tx_thread_busy, true);

    while(1) {
      ALOGE("waiting for event inside while (1)");
      qhci_tx_wq_mtx.lock();

      if (qhci_tx_workqueue.empty()) {
        qhci_tx_wq_mtx.unlock();
        break;
      } else {
        qhci_msg_t *data = qhci_tx_workqueue.front();
        qhci_tx_workqueue.pop();
        qhci_tx_wq_mtx.unlock();
        QHciTxMsgHandler(data);
      }
    }
  }

  ALOGI("%s: is stopped", __func__);

}

void QHciTx::curr_usr_qhci_tx_handler (int) {
  bool status = true;

  ALOGI("%s: exit\n", __func__);
  pthread_exit(&status);
}

int QHciTx::Init() {
  if (!qhci_tx_instance) {
    qhci_tx_instance = new QHci();
    qhci_tx_thread = std::thread([this]() {
                       struct sigaction prev_sig, curr_sig;
                       memset(&curr_sig, 0, sizeof(curr_sig));
                       curr_sig.sa_handler = curr_usr_qhci_tx_handler;
                       sigaction(SIGUSR1, &curr_sig, &prev_sig);
                       ALOGD("%s: Starting QHCI TX thread", __func__);
                       QHciTxThreadRoutine();
                     });
    if (!qhci_tx_thread.joinable()) return -1;
  }
  return 0;
}

} // namespace implementation
} // namespace xpan
}
}
}
}


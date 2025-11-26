/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <unistd.h>
#include <sstream>
#include <math.h>
#include "bttpi.h"
#include <utils/Log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "vendor.qti.hardware.bttpi-impl"
#define HCI_GRP_VENDOR_SPECIFIC (0x3F << 10) /* 0xFC00 */
#define HCI_VS_LINK_POWER_CTRL_REQ_OPCODE (0x00DA | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_VS_SET_TPI_STATE_SUBOPCODE (0x0020)
#define HCI_VS_CMD_STATUS_OFFSET (0x05)
std::mutex bttpi_timer_mutex;
bool BtTpi::isCmdPending;
bool BtTpi::is_timer_created = false;
std::shared_ptr<IBtTpiStatusCb> BtTpi::statusCb;
std::shared_ptr<IBtTpiEventsCb> BtTpi::eventCb;
uint8_t BtTpi::pendingReq;
uint8_t BtTpi::nextSeqNum;
uint16_t BtTpi::remainingLen;
uint8_t BtTpi::totalFragments;
std::vector<uint8_t> BtTpi::reqParams;
timer_t BtTpi::bttpi_timer;
AIBinder_DeathRecipient* BtTpi::deathObj;

BtTpi::BtTpi() {
  isCmdPending = false;
  nextSeqNum = 0;
  pendingReq = 0;
  remainingLen = 0;
  totalFragments = 0;
  statusCb = nullptr;
  eventCb = nullptr;
}

BtTpi::~BtTpi() {
}

::ndk::ScopedAStatus BtTpi::setTpiState(BtTpiState in_state,
                             const std::shared_ptr<IBtTpiStatusCb>& in_statusCb,
                             int8_t* _aidl_return) {
  ALOGD("%s: state:%d", __func__, in_state);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  *_aidl_return = TPI_CMD_STATUS_SUCCESS;

  // return error if a command is already pending
  if (isCmdPending) {
    ALOGE("%s: New command received while previous command is pending:%d",
           __func__);
    *_aidl_return = TPI_CMD_STATUS_BUSY_TRY_LATER;
    return ::ndk::ScopedAStatus::ok();
  }

  statusCb = in_statusCb;
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(2);//length of remaning  data
  data.push_back((uint8_t)HCI_VS_STX_BT_ENABLE_IND);//sub opcode
  data.push_back((uint8_t)in_state);

  pendingReq   = HCI_VS_STX_BT_ENABLE_IND;
  isCmdPending = true;
  nextSeqNum = 0;
  sendDataToController(data, _aidl_return);

  return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BtTpi::setActiveDSI(int32_t in_dsi,
                             const std::shared_ptr<IBtTpiStatusCb>& in_statusCb,
                             int8_t* _aidl_return) {
  ALOGD("%s: state:%d", __func__, in_dsi);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  *_aidl_return = TPI_CMD_STATUS_SUCCESS;

  // return error if a command is already pending
  if (isCmdPending) {
    ALOGE("%s: New command received while previous command is pending:%d",
           __func__);
    *_aidl_return = TPI_CMD_STATUS_BUSY_TRY_LATER;
    return ::ndk::ScopedAStatus::ok();
  }

  statusCb = in_statusCb;
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(2);//length of remaning  data
  data.push_back((uint8_t)HCI_VS_STX_ACTIVE_DSI_IND);//sub opcode
  data.push_back((uint8_t)in_dsi);

  pendingReq   = HCI_VS_STX_ACTIVE_DSI_IND;
  isCmdPending = true;
  nextSeqNum = 0;
  sendDataToController(data, _aidl_return);

  return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BtTpi::setAirplaneMode(bool in_airPlaneMode,
                             const std::shared_ptr<IBtTpiStatusCb>& in_statusCb,
                             int8_t* _aidl_return) {
  ALOGD("%s: state:%d", __func__, in_airPlaneMode);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  *_aidl_return = TPI_CMD_STATUS_SUCCESS;

  // return error if a command is already pending
  if (isCmdPending) {
    ALOGE("%s: New command received while previous command is pending:%d",
           __func__);
    *_aidl_return = TPI_CMD_STATUS_BUSY_TRY_LATER;
    return ::ndk::ScopedAStatus::ok();
  }

  statusCb = in_statusCb;
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(2);//length of remaning  data
  data.push_back((uint8_t)HCI_VS_STX_AIRPLANE_MODE_STATUS);//sub opcode
  data.push_back((uint8_t)in_airPlaneMode);

  pendingReq   = HCI_VS_STX_AIRPLANE_MODE_STATUS;
  isCmdPending = true;
  nextSeqNum = 0;
  sendDataToController(data, _aidl_return);

  return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BtTpi::setWiFiState(bool in_wifiState,
                             const std::shared_ptr<IBtTpiStatusCb>& in_statusCb,
                             int8_t* _aidl_return) {
  ALOGD("%s: state:%d", __func__, in_wifiState);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  *_aidl_return = TPI_CMD_STATUS_SUCCESS;

  // return error if a command is already pending
  if (isCmdPending) {
    ALOGD("%s: New command received while previous command is pending:%d",
           __func__);
    *_aidl_return = TPI_CMD_STATUS_BUSY_TRY_LATER;
    return ::ndk::ScopedAStatus::ok();
  }

  statusCb = in_statusCb;
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(2);//length of remaning  data
  data.push_back((uint8_t)HCI_VS_STX_WLAN_STATE_IND);//sub opcode
  data.push_back((uint8_t)in_wifiState);

  pendingReq   = HCI_VS_STX_WLAN_STATE_IND;
  isCmdPending = true;
  nextSeqNum = 0;
  sendDataToController(data, _aidl_return);

  return ::ndk::ScopedAStatus::ok();
}

void BtTpi::sendFragment(int8_t* retValPtr) {
  ALOGD("%s: pendingReq(%d), nextSeqNum(%d), remainingLen(%d)", __func__, pendingReq, nextSeqNum, remainingLen);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    return;
  }

  if (remainingLen == 0)
    return;

  uint16_t currLen = 0;
  currLen = (nextSeqNum == 0) ? remainingLen : TPI_CMD_MAX_BLOB_SIZE;
  remainingLen -= currLen;

  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back((uint8_t)(HCI_STX_FRAGMENT_HDR_SIZE + currLen)); //length of rem params
  data.push_back((uint8_t)pendingReq);//sub opcode
  data.push_back((uint8_t)nextSeqNum);
  data.push_back((uint8_t)remainingLen);
  data.push_back((uint8_t)(remainingLen >> 8));
  data.push_back((uint8_t)currLen);
  data.insert(data.end(),
     reqParams.begin() + (totalFragments - nextSeqNum - 1) * TPI_CMD_MAX_BLOB_SIZE,
     reqParams.begin() + (totalFragments - nextSeqNum - 1) * TPI_CMD_MAX_BLOB_SIZE + currLen);

  if (nextSeqNum > 0)
    nextSeqNum--;
  sendDataToController(data, retValPtr);

  return;
}

::ndk::ScopedAStatus BtTpi::setTpiParams(int32_t in_reqType,
                        const std::vector<uint8_t>& in_reqParams,
                        const std::shared_ptr<IBtTpiStatusCb>& in_statusCb,
                        int8_t* _aidl_return) {
  ALOGD("%s: reqType:%d, size:%d", __func__, in_reqType, in_reqParams.size());

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  *_aidl_return = TPI_CMD_STATUS_SUCCESS;

  // return error if a command is already pending
  if (isCmdPending) {
    ALOGE("%s: New command received while previous command is pending:%d",
           __func__, in_reqType);
    *_aidl_return = TPI_CMD_STATUS_BUSY_TRY_LATER;
    return ::ndk::ScopedAStatus::ok();
  }

  statusCb = in_statusCb;
  remainingLen = in_reqParams.size();
  // return error if no params received
  if (remainingLen == 0) {
    ALOGD("%s: Empty array of parameters received:%d",
           __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  isCmdPending = true;
  pendingReq = (uint8_t)in_reqType;
  totalFragments = ceil(float(remainingLen) / float(TPI_CMD_MAX_BLOB_SIZE));
  nextSeqNum = totalFragments - 1;
  reqParams.assign(in_reqParams.begin(), in_reqParams.end());
  sendFragment(_aidl_return);

  return ::ndk::ScopedAStatus::ok();
}

void BtTpi::sendDataToController(std::vector<uint8_t>& data, int8_t* retValPtr) {
  size_t send_data_status;
  DataHandler *data_handler = DataHandler::Get();
  ALOGD("%s:",__func__);
  if (data_handler != nullptr) {
    ALOGD("%s: sending VS cmd, starting timer", __func__);
    BtTpiTimerStart();
    send_data_status = data_handler->SendBtTpiData(
      &data[0],
      data.size(),
      [this](HciPacketType type,
             const hidl_vec<uint8_t> *packet) {
        int8_t cmdStatus = (int8_t)(packet->data())[HCI_VS_CMD_STATUS_OFFSET];
        switch (type) {
          case HCI_PACKET_TYPE_EVENT:
            ALOGD("%s: HCI event received. Status(%d)", __func__, cmdStatus);
            BtTpiTimerStop();

            if (isCmdPending && statusCb != nullptr) {
              if (remainingLen > 0) {
                if (cmdStatus != TPI_CMD_STATUS_SUCCESS) {
                  ALOGI("%s: Abort sending of remaining fragments", __func__);
                  isCmdPending = false;
                  nextSeqNum = 0;
                  remainingLen = 0;
                  totalFragments = 0;
                  reqParams.clear();
                  // abort sending of remaining packets
                  statusCb->setStatus(pendingReq, cmdStatus);
                  pendingReq = 0;
                  return;
                }
                // send next fragment to SoC
                sendFragment(nullptr);
              } else {
                isCmdPending = false;
                nextSeqNum = 0;
                remainingLen = 0;
                // send status for the pending commands
                statusCb->setStatus(pendingReq, cmdStatus);
              }
            }
            break;
          default:
            {
              ALOGE("%s Unexpected event type %d", __func__, type);
              break;
            }
        };
      });


    if (!send_data_status) {
      ALOGE("BtTpi is down");
      BtTpiTimerStop();
      if (retValPtr)
       *retValPtr = TPI_CMD_STATUS_FAILURE;
    }
    else
      ALOGD("%s: Command is sent",__func__);
  } else {
    ALOGE("%s: Data handler is null",__func__);
    if (retValPtr)
      *retValPtr = TPI_CMD_STATUS_FAILURE;
  }
}

void BtTpi::BtTpiTimerExpired() {
  std::lock_guard<std::mutex> guard(bttpi_timer_mutex);
  ALOGE("%s:",__func__);
  if (isCmdPending && (statusCb != nullptr)) {
    isCmdPending = false;
    nextSeqNum = 0;
    totalFragments = 0;
    remainingLen = 0;
    reqParams.clear();
    // send failure for the pending commands
    if (statusCb)
      statusCb->setStatus(pendingReq, TPI_CMD_STATUS_BTSOC_TIMEOUT);
    pendingReq = 0;
  }
}

void BtTpi::BtTpiTimerStart() {
    int status;
    struct itimerspec ts;
    struct sigevent se;
    uint32_t timeout_ms;
    ALOGD("%s: is_timer_created=%d", __func__, is_timer_created);
    std::lock_guard<std::mutex> guard(bttpi_timer_mutex);
    if (is_timer_created == false) {
      se.sigev_notify_function = (void (*)(union sigval))BtTpiTimerExpired;
      se.sigev_notify = SIGEV_THREAD;
      se.sigev_value.sival_ptr = &bttpi_timer;
      se.sigev_notify_attributes = NULL;

      status = timer_create(CLOCK_MONOTONIC, &se, &bttpi_timer);
      if (status == 0)
      {
        ALOGD("%s: BT TPI timer created",__func__);
        is_timer_created = true;
      } else {
        ALOGE("%s: Error creating BT TPI timer %d\n", __func__, status);
      }
    }
    if (is_timer_created == true) {
      timeout_ms = BTTPI_TIMER_VALUE_MS;
      ts.it_value.tv_sec = timeout_ms/1000;
      ts.it_value.tv_nsec = 1000000*(timeout_ms%1000);
      ts.it_interval.tv_sec = 0;
      ts.it_interval.tv_nsec = 0;

      status = timer_settime(bttpi_timer, 0, &ts, 0);
      if (status < 0)
        ALOGE("%s:Failed to set BT TPI timer: %d",__func__, status);
    }
}

void BtTpi::BtTpiTimerStop() {
  int status;
  struct itimerspec ts;
  ALOGD("%s: is_timer_created= %d",__func__, is_timer_created);

  std::lock_guard<std::mutex> guard(bttpi_timer_mutex);
  if (is_timer_created == true) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    status = timer_settime(bttpi_timer, 0, &ts, 0);
    if (status == -1)
      ALOGE("%s:Failed to stop BT TPI timer",__func__);
  }
}

void BtTpi::BtTpiTimerCleanup() {
  ALOGI("%s", __func__);
  if (is_timer_created == true) {
    BtTpiTimerStop();
    timer_delete(bttpi_timer);
    is_timer_created = false;
    bttpi_timer = NULL;
  }
}

::ndk::ScopedAStatus BtTpi::registerTpiEventsCallback(
                        const std::shared_ptr<IBtTpiEventsCb>& in_eventCb,
                        int8_t* _aidl_return) {
  ALOGD("%s", __func__);

  DataHandler *data_handler = DataHandler::Get();
  if (data_handler == nullptr) {
    ALOGE("%s: BT not initialized yet", __func__);
    *_aidl_return = -1;
    return ::ndk::ScopedAStatus::ok();
  }

  eventCb = in_eventCb;

  deathObj = AIBinder_DeathRecipient_new(&deathRecipient);
  if (deathObj != nullptr) {
    auto ret = AIBinder_linkToDeath(in_eventCb->asBinder().get(), deathObj, reinterpret_cast<void*>(this));
    if (ret != STATUS_OK) {
      // ignore it
      ALOGE("%s: linkToDeath failed. error(%d)", __func__, ret);
    }
  } else {
    ALOGE("%s: deatObj is NULL", __func__);
  }

  data_handler->registerTpiAsyncEventCb(
      [this](HciPacketType type,
             const hidl_vec<uint8_t> *packet) {

        int32_t dataLen = packet->size();
        const uint8_t* data = packet->data();
        uint8_t eventType = data[6];
        std::vector<uint8_t> asyncData(&data[HCI_STX_ASYNC_EVT_DATA_OFFSET],
             &data[HCI_STX_ASYNC_EVT_DATA_OFFSET] + dataLen - HCI_STX_ASYNC_EVT_DATA_OFFSET);

        switch ((uint8_t)eventType) {
          case HCI_VS_COEX_STX_BT_PWR_REPORT_IND:
            ALOGD("%s: TPI Async Pwr Report, eventLen-%d", __func__, dataLen);
            if (eventCb)
              eventCb->sendTpiEvent(eventType, asyncData);
            break;
          case HCI_VS_COEX_STX_BT_MAX_PWR_IND:
            ALOGD("%s: TPI Async Pwr Report, eventLen-%d", __func__, dataLen);
            if (eventCb)
              eventCb->sendTpiEvent(eventType, asyncData);
            break;
          default:
            {
              ALOGE("%s Unexpected Async event type %d", __func__, eventType);
              break;
            }
        };
      });

  *_aidl_return = 0;
  return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus BtTpi::UnRegisterTpiEventsCallback(int8_t* _aidl_return) {
  ALOGI("%s", __func__);

  // return error if init not done
  if (eventCb == nullptr) {
    ALOGE("%s: Init not done. Returning", __func__);
    *_aidl_return = TPI_CMD_STATUS_FAILURE;
    return ::ndk::ScopedAStatus::ok();
  }

  // Its like de-init function
  BtTpiTimerCleanup();
  if (deathObj != nullptr) {
    ALOGI("%s: Unlink Deathrecipient", __func__);
    AIBinder_unlinkToDeath(eventCb->asBinder().get(), deathObj, reinterpret_cast<void*>(this));
    deathObj = nullptr;
  }

  isCmdPending = false;;
  pendingReq = 0;
  nextSeqNum = 0;
  remainingLen = 0;
  totalFragments = 0;
  reqParams.clear();
  statusCb = nullptr;
  eventCb = nullptr;

  *_aidl_return = 0;
  return ::ndk::ScopedAStatus::ok();
}

void BtTpi::deathRecipient(void* cookie) {
  ALOGI("%s: BtTpi Client died", __func__);
  BtTpi* self = reinterpret_cast<BtTpi*>(cookie);
  BtTpiTimerCleanup();
  isCmdPending = false;;
  pendingReq = 0;
  nextSeqNum = 0;
  remainingLen = 0;
  totalFragments = 0;
  reqParams.clear();
  statusCb = nullptr;
  eventCb = nullptr;
  deathObj = nullptr;
}

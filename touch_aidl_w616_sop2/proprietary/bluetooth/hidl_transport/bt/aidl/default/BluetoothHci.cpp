/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */
/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <utils/Log.h>
#include <thread>
#include <future>
#include <chrono>
#include <cutils/properties.h>
#include "BluetoothHci.h"
#include "bluetooth_address.h"
#include "data_handler.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth-bluetooth_hci"

#define HCI_CLOSE_TIMEOUT_VALUE (10)

using ::android::hardware::hidl_vec;
using aidl::android::hardware::bluetooth::Status;
using android::hardware::bluetooth::V1_0::implementation::DataHandler;
using android::hardware::bluetooth::V1_0::implementation::Logger;
using android::hardware::bluetooth::V1_0::implementation::RxThreadEventType;

namespace aidl {
namespace android {
namespace hardware {
namespace bluetooth {
namespace implementation {

void OnDeath(void* cookie);

class BluetoothDeathRecipient {
 public:
  BluetoothDeathRecipient(BluetoothHci* hci) : mHci(hci) {}

  void LinkToDeath(const std::shared_ptr<IBluetoothHciCallbacks>& cb) {
    mCb = cb;
    clientDeathRecipient_ = AIBinder_DeathRecipient_new(OnDeath);
    auto linkToDeathReturnStatus = AIBinder_linkToDeath(
        mCb->asBinder().get(), clientDeathRecipient_, this /* cookie */);
    if (linkToDeathReturnStatus != STATUS_OK) {
      ALOGE("%s: Unable to link to death recipient(%d)", __func__,
            linkToDeathReturnStatus);
    }
  }

  void UnlinkToDeath(const std::shared_ptr<IBluetoothHciCallbacks>& cb) {
    if (cb != mCb) {
      ALOGE("Unable to unlink mismatched pointers");
    }
  }

  void serviceDied() {
    if (mCb != nullptr && !AIBinder_isAlive(mCb->asBinder().get())) {
      ALOGE("Bluetooth remote service has died");
    } else {
      ALOGE("BluetoothDeathRecipient::serviceDied called but service not dead");
      return;
    }
    {
      std::lock_guard<std::mutex> guard(mHasDiedMutex);
      has_died_ = true;
    }
    mHci->close();
  }
  BluetoothHci* mHci;
  std::shared_ptr<IBluetoothHciCallbacks> mCb;
  AIBinder_DeathRecipient* clientDeathRecipient_;
  bool getHasDied() {
    std::lock_guard<std::mutex> guard(mHasDiedMutex);
    return has_died_;
  }

 private:
  std::mutex mHasDiedMutex;
  bool has_died_{false};
};

void OnDeath(void* cookie) {
  auto* death_recipient = static_cast<BluetoothDeathRecipient*>(cookie);
  death_recipient->serviceDied();
}

BluetoothHci::BluetoothHci()
  : deathRecipient(std::make_shared<BluetoothDeathRecipient>(this))
{
}

ndk::ScopedAStatus BluetoothHci::initialize(
  const std::shared_ptr<IBluetoothHciCallbacks>& cb)
{
  ALOGW("BluetoothHci::initialize()");
  if (cb == nullptr) {
    ALOGE("%s: Received NULL callback from BT client", __func__);
    return ndk::ScopedAStatus::fromServiceSpecificError(STATUS_BAD_VALUE);
  }
  std::shared_ptr<IBluetoothHciCallbacks> event_cb_tmp;
  event_cb_tmp = cb;
  bool rc = DataHandler::Init( TYPE_BT,
    [this, event_cb_tmp](bool status) {
      if (event_cb_tmp != nullptr) {
        ALOGI("%s: Set callbacks received from BT client inorder "
               "to provide status and data through them", __func__);
        event_cb_ = event_cb_tmp;
      }
      if (event_cb_ != nullptr) {
        auto init_status = event_cb_->initializationComplete(
          status ? Status::SUCCESS : Status::HARDWARE_INITIALIZATION_ERROR);
        if(!init_status.isOk()) {
          ALOGE("Client dead, callback initializationComplete failed");
        }
      }
    },
    /* Remove unused lamda capture 'this' to avoid compile warning */
    [event_cb_tmp](HciPacketType type, const hidl_vec<uint8_t> *packet) {
      DataHandler *data_handler = DataHandler::Get();
      if (event_cb_tmp == nullptr) {
        ALOGE("BluetoothHci: event_cb_tmp is null");
        if (data_handler)
          data_handler->SetClientStatus(false, TYPE_BT);
        return;
      }
      /* Skip calling client callback when client is dead */
      if(data_handler && (data_handler->GetClientStatus(TYPE_BT) == false)) {
        ALOGV("%s: Skip calling client callback when client is dead", __func__);
        return;
      }
      Logger::Get()->UpdateRxTimeStamp();
      switch (type) {
        case HCI_PACKET_TYPE_EVENT:
        {
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_PRE_STACK_EVT_CALL_BACK);
#endif
          auto status = event_cb_tmp->hciEventReceived(*packet);
          if(!status.isOk()) {
            ALOGE("Client dead, callback hciEventReceived failed");
            if (data_handler)
              data_handler->SetClientStatus(false, TYPE_BT);
          }
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_POST_STACK_EVT_CALL_BACK);
#endif
        }
        break;
        case HCI_PACKET_TYPE_ACL_DATA:
        {
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_PRE_STACK_ACL_CALL_BACK);
#endif
          auto status = event_cb_tmp->aclDataReceived(*packet);
          if(!status.isOk()) {
            ALOGE("Client dead, callback aclDataReceived failed");
            if (data_handler)
              data_handler->SetClientStatus(false, TYPE_BT);
          }
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_POST_STACK_ACL_CALL_BACK);
#endif
        }
        break;
        case HCI_PACKET_TYPE_SCO_DATA:
        {
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_PRE_STACK_SCO_CALL_BACK);
#endif
          auto status = event_cb_tmp->scoDataReceived(*packet);
          if(!status.isOk()) {
            ALOGE("Client dead, callback aclDataReceived failed");
            if (data_handler)
              data_handler->SetClientStatus(false, TYPE_BT);
          }
#ifdef DUMP_RINGBUF_LOG
          Logger::Get()->UpdateRxEventTag(RxThreadEventType::RX_POST_STACK_SCO_CALL_BACK);
#endif
        }
        break;
        default:
          ALOGE("%s Unexpected event type %d", __func__, type);
          break;
      }
    });
  if (!rc && (cb != nullptr)) {
    Status init_status = Status::HARDWARE_INITIALIZATION_ERROR;
    DataHandler *data_handler = DataHandler::Get();
    if(data_handler->isProtocolAdded(TYPE_BT)) {
      init_status = Status::ALREADY_INITIALIZED;
    }
    auto status = cb->initializationComplete(init_status);
    if(!status.isOk()) {
      ALOGE("Client dead, callback initializationComplete failed");
    }
    ALOGE("BluetoothHci: error %d", init_status);
  } else if (rc && (cb != nullptr)) {
     ALOGI("%s: linking to deathRecipient", __func__);
     deathRecipient->LinkToDeath(cb);
  }

  return ndk::ScopedAStatus::ok();
}

void CloseTask(std::shared_ptr<IBluetoothHciCallbacks> event_cb_,
    std::shared_ptr<BluetoothDeathRecipient> deathRecipient)
{
  ALOGW("%s enter", __func__);
  if (deathRecipient != nullptr) deathRecipient->UnlinkToDeath(event_cb_);
  DataHandler::CleanUp(TYPE_BT);
  ALOGW("%s leave", __func__);
}

ndk::ScopedAStatus BluetoothHci::close()
{
  ALOGW("BluetoothHci::close()");
  auto future = std::async(std::launch::async, CloseTask,
                           event_cb_, deathRecipient);

  int closeTimeout= property_get_int32("persist.vendor.bluetooth.close.timeout",
          HCI_CLOSE_TIMEOUT_VALUE);
  if (future.wait_for(std::chrono::seconds(closeTimeout)) ==
          std::future_status::timeout) {
    ALOGE("Terminate BT HAL due to timeout during closing process!");
    kill(getpid(), SIGKILL);
  }

  ALOGW("BluetoothHci::close, finish cleanup");
  return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus BluetoothHci::sendHciCommand(const std::vector<uint8_t>& command)
{
  sendDataToController(HCI_PACKET_TYPE_COMMAND, command);
  return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus BluetoothHci::sendAclData(const std::vector<uint8_t>& data)
{
  sendDataToController(HCI_PACKET_TYPE_ACL_DATA, data);
  return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus BluetoothHci::sendScoData(const std::vector<uint8_t>& data)
{
  sendDataToController(HCI_PACKET_TYPE_SCO_DATA, data);
  return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus BluetoothHci::sendIsoData(const std::vector<uint8_t>& data)
{
  sendDataToController(HCI_PACKET_TYPE_ISO_DATA, data);
  return ndk::ScopedAStatus::ok();
}

void BluetoothHci::sendDataToController(HciPacketType type,
                                        const std::vector<uint8_t>& data)
{
  DataHandler *data_handler = DataHandler::Get();
  if(data_handler != nullptr) {
    data_handler->SendData(TYPE_BT, type, data.data(), data.size());
  }
}

}  // namespace implementation
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
}  // namespace aidl

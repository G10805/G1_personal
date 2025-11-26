/******************************************************************************
  Copyright (c) 2017,2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_5_H_
#define _QTI_RADIO_SERVICE_2_5_H_

#include <vendor/qti/hardware/radio/qtiradio/2.5/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.5/IQtiRadioResponse.h>

#include "hidl_impl/2.4/qti_radio_service_2_4.h"
#include "hidl_impl/2.5/qti_radio_service_utils_2_5.h"

#include "interfaces/nas/RilRequestSetNrConfigMessage.h"
#include "interfaces/nas/RilRequestQueryNrConfigMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_5 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V2_4::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_5::IQtiRadioResponse> mResponseCbV2_5;

  ::android::sp<V2_5::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_5;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_5::setCallback_nolock");
    mResponseCbV2_5 = nullptr;
    V2_4::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_5::setCallback_nolock");
    mResponseCbV2_5 = V2_5::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    V2_4::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForSetNrConfig(int32_t serial, RIL_Errno errorCode) {
    ::android::sp<V2_5::IQtiRadioResponse> respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("setNrConfigResponse: serial=%d", serial);
      auto ret = respCb->setNrConfigResponse(serial, static_cast<uint32_t>(errorCode),
                                             V2_5::Status::SUCCESS);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQueryNrConfig(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryNrConfigResult_t> payload) {
    V2_5::NrConfig config = V2_5::NrConfig::NR_CONFIG_INVALID;
    if (errorCode == RIL_E_SUCCESS && payload) {
      config = utils::convert_nr_disable_mode(payload->mode);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onNrBearerAllocationResponse: serial=%d", serial);
      auto ret = respCb->onNrConfigResponse(serial, static_cast<uint32_t>(errorCode), config);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

 public:
  static const HalServiceImplVersion& getVersion();

  ::android::hardware::Return<void> setCallback(
      const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
      const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_5::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> setNrConfig(
      int32_t serial, const ::vendor::qti::hardware::radio::qtiradio::V2_5::NrConfig config) {
    QCRIL_LOG_DEBUG("setNrConfig: serial=%d", serial);
    auto msg = std::make_shared<RilRequestSetNrConfigMessage>(this->getContext(serial),
                                                              utils::convert_nr_config(config));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            if (resp) {
              errorCode = resp->errorCode;
            }
            this->sendResponseForSetNrConfig(serial, errorCode);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForSetNrConfig(serial, RIL_E_NO_MEMORY);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> queryNrConfig(int32_t serial) {
    QCRIL_LOG_DEBUG("queryNrConfig: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrConfigMessage>(this->getContext(serial));
    if (msg != nullptr) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQueryNrConfigResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload =
                  std::static_pointer_cast<qcril::interfaces::RilQueryNrConfigResult_t>(resp->data);
            }
            this->sendResponseForQueryNrConfig(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryNrConfig(serial, RIL_E_NO_MEMORY, nullptr);
    }
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_5::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_5
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_5_H_

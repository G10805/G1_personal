/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_1_0_H_
#define _QTI_RADIO_SERVICE_1_0_H_

#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadioResponse.h>

#include "HalServiceImplFactory.h"
#include "hidl_impl/qti_radio_service_base.h"
#include "interfaces/uim/UimGetAtrRequestMsg.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public T, public QtiRadioServiceBase {
 private:
  ::android::sp<V1_0::IQtiRadioResponse> mResponseCb;
  ::android::sp<V1_0::IQtiRadioIndication> mIndicationCb;

  ::android::sp<V1_0::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCb;
  }
  ::android::sp<V1_0::IQtiRadioIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCb;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_0::setCallback_nolock");
    mIndicationCb = nullptr;
    mResponseCb = nullptr;
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V1_0::setCallback_nolock");
    if (mResponseCb != nullptr) {
      mResponseCb->unlinkToDeath(this);
    }
    mIndicationCb = indCb;
    mResponseCb = respCb;
    if (mResponseCb != nullptr) {
      mResponseCb->linkToDeath(this, 0);
    }
  }

  // Response APIs
  void sendResponseForGetAtr(int serial, RIL_Errno errorCode, std::string atr) {
    ::android::sp<V1_0::IQtiRadioResponse> respCb = getResponseCallback();
    if (respCb) {
      QtiRadioResponseInfo responseInfo{};
      responseInfo.serial = serial;
      responseInfo.type = QtiRadioResponseType::SOLICITED;
      responseInfo.error = static_cast<QtiRadioError>(errorCode);
      QCRIL_LOG_DEBUG("IQtiRadio: getAtrResponse: serial=%d", serial);
      auto ret = respCb->getAtrResponse(responseInfo, atr);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("respCb NULL");
    }
  }

 public:
  static const HalServiceImplVersion& getVersion();

  bool registerService(qcril_instance_id_e_type instId) {
    this->setInstanceId(instId);

    std::string serviceName = "slot" + std::to_string(this->getInstanceId() + 1);
    android::status_t ret = T::registerAsService(serviceName);
    Log::getInstance().d(std::string("[QtiRadio]: ") + T::descriptor +
                         (ret == android::OK ? " registered" : " failed to register"));
    return (ret == android::OK);
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> setCallback(
      const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
      const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> getAtr(int32_t serial) {
    QCRIL_LOG_DEBUG("IQtiRadio: getAtr: serial=%d", serial);
    auto req = std::make_shared<UimGetAtrRequestMsg>(this->getInstanceId());
    if (req) {
      GenericCallback<UimAtrRspParam> cb(
          ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<UimAtrRspParam> resp) -> void {
            RIL_Errno errorCode = RIL_E_INTERNAL_ERR;
            std::string atr;
            if (msg && resp && status == Message::Callback::Status::SUCCESS &&
                resp->err == RIL_UIM_E_SUCCESS) {
              errorCode = RIL_E_SUCCESS;
              atr = resp->atr;
            }
            this->sendResponseForGetAtr(serial, errorCode, atr);
          }));
      req->setCallback(&cb);
      req->dispatch();
    } else {
      this->sendResponseForGetAtr(serial, RIL_E_NO_MEMORY, "");
    }
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V1_0::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V1_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_1_0_H_

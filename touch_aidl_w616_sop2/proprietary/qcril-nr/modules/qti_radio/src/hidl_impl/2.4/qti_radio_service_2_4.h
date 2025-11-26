/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_4_H_
#define _QTI_RADIO_SERVICE_2_4_H_

#include <vendor/qti/hardware/radio/qtiradio/2.4/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.4/IQtiRadioResponse.h>

#include "hidl_impl/2.3/qti_radio_service_2_3.h"

#include "SetCarrierInfoImsiEncryptionMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_4 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V2_3::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_4::IQtiRadioResponse> mResponseCbV2_4;

  ::android::sp<V2_4::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_4;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_4::setCallback_nolock");
    mResponseCbV2_4 = nullptr;
    V2_3::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_4::setCallback_nolock");
    mResponseCbV2_4 = V2_4::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    V2_3::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForSetCarrierInfoForImsiEncryption(int32_t serial,
                                                              V1_0::QtiRadioError errorCode) {
    V1_0::QtiRadioResponseInfo responseInfo = { V1_0::QtiRadioResponseType::SOLICITED, serial,
                                                errorCode };
    ::android::sp<V2_4::IQtiRadioResponse> respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("setCarrierInfoForImsiEncryptionResponse: serial=%d", serial);
      auto ret = respCb->setCarrierInfoForImsiEncryptionResponse(responseInfo);
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
    QCRIL_LOG_DEBUG("V2_4::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> setCarrierInfoForImsiEncryption(
      int32_t serial, const V2_4::ImsiEncryptionInfo& data) {
    QCRIL_LOG_DEBUG("setCarrierInfoForImsiEncryption: serial=%d", serial);
    rildata::ImsiEncryptionInfo_t imsiEncryption;
    imsiEncryption.mcc = data.base.mcc;
    imsiEncryption.mnc = data.base.mnc;
    imsiEncryption.carrierKey = data.base.carrierKey;
    imsiEncryption.keyIdentifier = data.base.keyIdentifier;
    imsiEncryption.expiryTime = data.base.expirationTime;
    imsiEncryption.keyType = (rildata::PublicKeyType_t)data.keyType;

    auto msg = std::make_shared<rildata::SetCarrierInfoImsiEncryptionMessage>(imsiEncryption);
    if (msg) {
      GenericCallback<RIL_Errno> cb([this, serial](std::shared_ptr<Message> /*msg*/,
                                                   Message::Callback::Status status,
                                                   std::shared_ptr<RIL_Errno> resp) -> void {
        V1_0::QtiRadioError errorCode = V1_0::QtiRadioError::GENERIC_FAILURE;
        QCRIL_LOG_DEBUG("Message::Callback::Status : %d", status);
        if (status == Message::Callback::Status::SUCCESS) {
          if (resp) {
            errorCode = (V1_0::QtiRadioError)(*resp);
          } else {
            errorCode = V1_0::QtiRadioError::GENERIC_FAILURE;
          }
        }
        this->sendResponseForSetCarrierInfoForImsiEncryption(serial, errorCode);
      });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      QCRIL_LOG_ERROR("Unable to create msg SetCarrierInfoImsiEncryptionMessage");
      this->sendResponseForSetCarrierInfoForImsiEncryption(serial,
                                                           V1_0::QtiRadioError::GENERIC_FAILURE);
    }
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_4::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_4
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_4_H_

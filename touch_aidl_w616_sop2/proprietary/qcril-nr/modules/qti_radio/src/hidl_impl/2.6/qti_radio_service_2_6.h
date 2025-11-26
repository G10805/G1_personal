/******************************************************************************
  Copyright (c) 2017,2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_6_H_
#define _QTI_RADIO_SERVICE_2_6_H_

#include <vendor/qti/hardware/radio/qtiradio/2.6/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.6/IQtiRadioResponse.h>

#include "hidl_impl/2.5/qti_radio_service_2_5.h"

#include "interfaces/nas/RilRequestGetEnhancedRadioCapabilityMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_6 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V2_5::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_6::IQtiRadioResponse> mResponseCbV2_6;

  ::android::sp<V2_6::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_6;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_6::setCallback_nolock");
    mResponseCbV2_6 = nullptr;
    V2_5::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_6::setCallback_nolock");
    mResponseCbV2_6 = V2_6::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    V2_5::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForGetQtiRadioCapability(
      ::android::hardware::radio::V1_0::RadioResponseInfo respInfo,
      std::shared_ptr<qcril::interfaces::RilGetRadioAccessFamilyResult_t> payload) {
    ::android::hardware::hidl_bitfield<V2_6::RadioAccessFamily> raf =
      static_cast<::android::hardware::hidl_bitfield<V2_6::RadioAccessFamily>>
          (V2_6::RadioAccessFamily::UNKNOWN);
    if (payload) {
      raf = static_cast<::android::hardware::hidl_bitfield<V2_6::RadioAccessFamily>>
          (payload->radioAccessFamily);
    }
    ::android::sp<V2_6::IQtiRadioResponse> respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("getQtiRadioCapabilityResponse: serial=%d", respInfo.serial);
      auto ret = respCb->getQtiRadioCapabilityResponse(respInfo, raf);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

 public:
  static const HalServiceImplVersion& getVersion();

  ::android::hardware::Return<void> setCallback(
      const android::sp<V1_0::IQtiRadioResponse>& respCb,
      const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_6::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> getQtiRadioCapability(int32_t serial) {
    QCRIL_LOG_DEBUG("QtiRadioImpl::getQtiRadioCapability, serial=%d", serial);
    auto msg = std::make_shared<RilRequestGetEnhancedRadioCapabilityMessage>(
        this->getContext(serial));
    if (msg != nullptr) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        ::android::hardware::radio::V1_0::RadioResponseInfo resp_info{
            ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
            ::android::hardware::radio::V1_0::RadioError::NO_MEMORY};
        std::shared_ptr<qcril::interfaces::RilGetRadioAccessFamilyResult_t> payload = nullptr;
        (void) msg;
        (void) status;
        ::android::hardware::Return<void> ret;
        if (resp != nullptr) {
          resp_info.error = (::android::hardware::radio::V1_0::RadioError)resp->errorCode;
          payload = std::static_pointer_cast<qcril::interfaces::RilGetRadioAccessFamilyResult_t>(
              resp->data);
        }
        this->sendResponseForGetQtiRadioCapability(resp_info, payload);
      });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
        ::android::hardware::radio::V1_0::RadioResponseInfo respInfo{
            ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
            ::android::hardware::radio::V1_0::RadioError::NO_MEMORY};
        this->sendResponseForGetQtiRadioCapability(respInfo, nullptr);
    }
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_6::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_6
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_6_H_

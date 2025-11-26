/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_2_H_
#define _QTI_RADIO_SERVICE_2_2_H_

#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioResponse.h>

#include "hidl_impl/2.1/qti_radio_service_2_1.h"
#include "hidl_impl/2.2/qti_radio_service_utils_2_2.h"

#ifndef QMI_RIL_UTF
#include "request/GetDataNrIconTypeMessage.h"
#endif

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_2 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V2_1::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_2::IQtiRadioResponse> mResponseCbV2_2;
  ::android::sp<V2_2::IQtiRadioIndication> mIndicationCbV2_2;

  ::android::sp<V2_2::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_2;
  }
  ::android::sp<V2_2::IQtiRadioIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCbV2_2;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_2::setCallback_nolock");
    mIndicationCbV2_2 = nullptr;
    mResponseCbV2_2 = nullptr;
    V2_1::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_2::setCallback_nolock");
    mResponseCbV2_2 = V2_2::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    mIndicationCbV2_2 = V2_2::IQtiRadioIndication::castFrom(indCb).withDefault(nullptr);
    V2_1::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForQueryNrIconType(int32_t serial, RIL_Errno errorCode,
                                              five_g_icon_type iconType) {
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onNrIconTypeResponse: serial=%d, errorCode=%d, iconType=%d", serial,
                      errorCode, iconType);
      auto ret = respCb->onNrIconTypeResponse(serial, static_cast<uint32_t>(errorCode),
                                              utils::convert_five_g_icon_type(iconType));
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
    QCRIL_LOG_DEBUG("V2_2::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> queryNrIconType(int32_t serial) {
    QCRIL_LOG_DEBUG("queryNrIconType: serial=%d", serial);
#ifndef QMI_RIL_UTF
    auto msg = std::make_shared<rildata::GetDataNrIconTypeMessage>();
    if (msg) {
      GenericCallback<rildata::NrIconType_t> cb(
          ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                          std::shared_ptr<rildata::NrIconType_t> resp) -> void {
            RIL_Errno errorCode = RIL_E_INTERNAL_ERR;
            five_g_icon_type rilIconType = FIVE_G_ICON_TYPE_INVALID;
            if (status == Message::Callback::Status::SUCCESS) {
              if (resp) {
                if (resp->isNone()) {
                  rilIconType = FIVE_G_ICON_TYPE_NONE;
                } else if (resp->isBasic()) {
                  rilIconType = FIVE_G_ICON_TYPE_BASIC;
                } else if (resp->isUwb()) {
                  rilIconType = FIVE_G_ICON_TYPE_UWB;
                } else {
                  rilIconType = FIVE_G_ICON_TYPE_INVALID;
                }
              }
              errorCode = RIL_E_SUCCESS;
            }
            this->sendResponseForQueryNrIconType(serial, errorCode, rilIconType);
          }));
      msg->setCallback(&cb);
      msg->dispatch();
    }
#endif
    return ::android::hardware::Void();
  }

  /**
   * Notifies onNrIconTypeChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_2::IQtiRadioIndication::onNrIconTypeChange
   */
  void notifyOnNrIconTypeChange(five_g_icon_type iconType) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onNrIconTypeChange");
      auto ret = indCb->onNrIconTypeChange(utils::convert_five_g_icon_type(iconType));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_2::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_2
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_2_H_

/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_1_3_H_
#define __RADIO_CONFIG_SERVICE_1_3_H_

#include <android/hardware/radio/config/1.3/IRadioConfig.h>
#include <android/hardware/radio/config/1.3/IRadioConfigResponse.h>

#include "hidl_impl/1.2/radio_config_service_1_2.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_3 {
namespace implementation {

template <typename T>
class RadioConfigServiceImpl : public V1_2::implementation::RadioConfigServiceImpl<T> {
 private:
  ::android::sp<V1_3::IRadioConfigResponse> mResponseCbV1_3;

  ::android::sp<V1_3::IRadioConfigResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV1_3;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_3::clearCallbacks_nolock");
    mResponseCbV1_3 = nullptr;
    V1_2::implementation::RadioConfigServiceImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setResponseFunctions_nolock(const ::android::sp<V1_0::IRadioConfigResponse>& respCb,
                                   const ::android::sp<V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("V1_3::setResponseFunctions_nolock");
    mResponseCbV1_3 = V1_3::IRadioConfigResponse::castFrom(respCb).withDefault(nullptr);
    V1_2::implementation::RadioConfigServiceImpl<T>::setResponseFunctions_nolock(respCb, indCb);
  }

  virtual void sendResponseForGetHalDeviceCapabilities(int32_t serial,
        RIL_Errno errorCode, bool reducedFeature) {
    ::android::hardware::radio::V1_6::RadioResponseInfo rsp_info {
                         ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
                         static_cast<::android::hardware::radio::V1_6::RadioError>(errorCode) };
    auto respCb = this->getResponseCallback();
    if (!respCb) return;
    auto ret = respCb->getHalDeviceCapabilitiesResponse(rsp_info, reducedFeature);
    if (!ret.isOk()) {
       QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }

 public:
  static const HalServiceImplVersion& getVersion();

  ::android::hardware::Return<void> setResponseFunctions(
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& respCb,
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("setResponseFunctions");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setResponseFunctions_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Implemented APIs
  ::android::hardware::Return<void> getHalDeviceCapabilities(int32_t serial) {
    QCRIL_LOG_DEBUG("IRadiConfig: getHalDeviceCapabilities: serial=%d", serial);
    this->sendResponseForGetHalDeviceCapabilities(serial, RIL_E_SUCCESS, true);
    return ::android::hardware::Void();
  }
};


template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_3::IRadioConfig>::getVersion();

}  // namespace implementation
}  // namespace V1_3
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_1_3_H_

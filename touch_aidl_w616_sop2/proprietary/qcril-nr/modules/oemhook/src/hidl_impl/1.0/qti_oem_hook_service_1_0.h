/******************************************************************************
#  Copyright (c) 2017,2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef __OEMHOOK_SERVICE_1_0_H__
#define __OEMHOOK_SERVICE_1_0_H__

#include "vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHook.h"
#include "telephony/ril.h"
#include "hidl_impl/qti_oem_hook_service_base.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {
namespace implementation {

// V1_0::IQtiOemHook implementation
template <typename T>
class OemHookServiceImpl : public T, public OemHookServiceBase {
 private:
  ::android::sp<V1_0::IQtiOemHookResponse> mResponseCb;
  ::android::sp<V1_0::IQtiOemHookIndication> mIndicationCb;

  ::android::sp<V1_0::IQtiOemHookResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCb;
  }

  ::android::sp<V1_0::IQtiOemHookIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCb;
  }

 protected:
  void clearCallbacks_nolock() {
    mResponseCb = nullptr;
    mIndicationCb = nullptr;
  }
  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }
  void setCallback_nolock(const ::android::sp<V1_0::IQtiOemHookResponse>& respCb,
                          const ::android::sp<V1_0::IQtiOemHookIndication>& indCb) {
    if (mResponseCb) {
      mResponseCb->unlinkToDeath(this);
    }
    mResponseCb = respCb;
    mIndicationCb = indCb;
    if (mResponseCb != nullptr) {
      mResponseCb->linkToDeath(this, 0);
    }
  }

 public:
  OemHookServiceImpl() = default;

  virtual ~OemHookServiceImpl() = default;

  static const HalServiceImplVersion& getVersion();

  bool registerService(qcril_instance_id_e_type instId) {
    this->setInstanceId(instId);

    std::string serviceName = "oemhook" + std::to_string(this->getInstanceId());
    android::status_t ret = T::registerAsService(serviceName);
    Log::getInstance().d(std::string("[IQtiOemHook]: ") + T::descriptor +
                         (ret == android::OK ? " registered" : " failed to register"));
    return (ret == android::OK);
  }

  //===========================================================================
  // setCallback
  //===========================================================================
  android::hardware::Return<void> setCallback(
      const ::android::sp<V1_0::IQtiOemHookResponse>& respCb,
      const ::android::sp<V1_0::IQtiOemHookIndication>& indCb) {
    {
      std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
      this->setCallback_nolock(respCb, indCb);
    }
    return android::hardware::Void();
  }

  //===========================================================================
  // oemHookRawRequest
  //===========================================================================
  ::android::hardware::Return<void> oemHookRawRequest(
      int32_t serial, const ::android::hardware::hidl_vec<uint8_t>& data) {
    processOemHookRawRequest(serial, data);
    return ::android::hardware::Void();
  }

  void sendResponse(int32_t serial, RadioError errorCode,
                    const ::android::hardware::hidl_vec<uint8_t>& respData) {
    QCRIL_LOG_INFO("sendResponse");
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_INFO("oemHookRawResponse serial=%d error=%d length=%d", serial, errorCode,
                     respData.size());
      auto ret = respCb->oemHookRawResponse(serial, errorCode, respData);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  void sendResponse(int32_t serial, RadioError errorCode) {
    ::android::hardware::hidl_vec<uint8_t> respData{};
    this->sendResponse(serial, errorCode, respData);
  }

  void sendResponse(int serial, RIL_Errno errorCode, uint8_t* buf, size_t bufLen) {
    ::android::hardware::hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen);
    this->sendResponse(serial, static_cast<RadioError>(errorCode), data);
  }

  void sendIndication(const ::android::hardware::hidl_vec<uint8_t>& respData) {
    QCRIL_LOG_INFO("sendIndication");
    auto indCb = this->getIndicationCallback();
    if (indCb) {
      auto ret = indCb->oemHookRawIndication(respData);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    }
  }

  void sendIndication(uint8_t* buf, size_t bufLen) {
    ::android::hardware::hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen);
    this->sendIndication(data);
  }
};

template <>
const HalServiceImplVersion& OemHookServiceImpl<V1_0::IQtiOemHook>::getVersion();

}  // namespace implementation
}  // namespace V1_0
}  // namespace qcrilhook
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // __OEMHOOK_SERVICE_1_0_H__

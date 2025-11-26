/******************************************************************************
  Copyright (c) 2017,2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_1_0_H_
#define _QTI_RADIO_SERVICE_1_0_H_

#include <utils/String8.h>
#include <vendor/qti/hardware/radio/am/1.0/IQcRilAudio.h>
#include <vendor/qti/hardware/radio/am/1.0/types.h>
#include <utils/StrongPointer.h>
#include <hidl/HidlSupport.h>
#include <string.h>
#include <string>

#include "HalServiceImplFactory.h"
#include "hidl_impl/qti_audio_service_base.h"
#include "interfaces/voice/QcRilRequestSetAudioServiceStatusMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace am {
namespace V1_0 {
namespace implementation {

template <typename T>
class QtiAudioImpl : public T, public QtiAudioServiceBase {
 private:
  ::android::sp<V1_0::IQcRilAudioCallback> mQcRilAudioCallback;

  ::android::sp<V1_0::IQcRilAudioCallback> getQcRilAudioCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mQcRilAudioCallback;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_0::clearCallbacks_nolock");
    mQcRilAudioCallback = nullptr;
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQcRilAudioCallback>& respCb) {
    QCRIL_LOG_DEBUG("V1_0::setCallback_nolock");
    if (mQcRilAudioCallback != nullptr) {
      mQcRilAudioCallback->unlinkToDeath(this);
    }
    mQcRilAudioCallback = respCb;
    if (mQcRilAudioCallback != nullptr) {
      mQcRilAudioCallback->linkToDeath(this, 0);
    }
  }

  // QcRilAudioCallback APIs
  /**
   * getParameters
   * Invokes V1_0::IQcRilAudioCallback::getParameters
   */
  ::android::String8 getParameters(::android::String8 param) override {
    QCRIL_LOG_INFO("V1_0::getParameters");
    android::String8 keyValPairs;
    auto callback = getQcRilAudioCallback();
    if (callback) {
      ::android::hardware::hidl_string str{ param.string() };
      auto ret = callback->getParameters(str, [&](const android::hardware::hidl_string& results) {
        keyValPairs = android::String8(results.c_str());
      });
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to getParameters. Exception : %s", ret.description().c_str());
      }
    }
    return keyValPairs;
  }

  /**
   * setParameters
   * Invokes V1_0::IQcRilAudioCallback::setParameters
   */
  int setParameters(::android::String8 param) override {
    QCRIL_LOG_INFO("V1_0::setParameters");
    int result = -1;
    auto callback = getQcRilAudioCallback();
    if (callback) {
      ::android::hardware::hidl_string str{ param.string() };
      auto ret = callback->setParameters(str);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to setParameters. Exception : %s", ret.description().c_str());
      } else {
        result = ret;
      }
    } else {
      QCRIL_LOG_ERROR("mQcRilAudioCallback == NULL");
    }

    return result;
  }

 public:
  static const HalServiceImplVersion& getVersion();

  bool registerService(qcril_instance_id_e_type instId) {
    this->setInstanceId(instId);

    std::string serviceName = "slot" + std::to_string(this->getInstanceId() + 1);
    ::android::status_t ret = T::registerAsService(serviceName);
    Log::getInstance().d(std::string("[QcRilAudio]: ") + T::descriptor +
                         (ret == ::android::OK ? " registered" : " failed to register"));
    return (ret == ::android::OK);
  }

  // Functions from IQcRilAudio
  /**
   * setCallback
   */
  ::android::hardware::Return<void> setCallback(
      const ::android::sp<V1_0::IQcRilAudioCallback>& respCb) {
    QCRIL_LOG_DEBUG("V1_0::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb);
    return ::android::hardware::Void();
  }

  /**
   * setError
   */
  ::android::hardware::Return<void> setError(
      ::vendor::qti::hardware::radio::am::V1_0::AudioError errorCode) {
    QCRIL_LOG_INFO("V1_0::setError: %d", errorCode);
    if (errorCode == ::vendor::qti::hardware::radio::am::V1_0::AudioError::AUDIO_STATUS_OK) {
      auto msg = std::make_shared<QcRilRequestSetAudioServiceStatusMessage>(this->getContext(0));
      if (msg) {
        msg->setIsReady(true);
        msg->dispatch();
      }
    }
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiAudioImpl<V1_0::IQcRilAudio>::getVersion();

}  // namespace implementation
}  // namespace V1_0
}  // namespace am
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_1_0_H_

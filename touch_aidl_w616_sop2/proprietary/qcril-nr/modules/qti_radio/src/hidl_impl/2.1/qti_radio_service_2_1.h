/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_1_H_
#define _QTI_RADIO_SERVICE_2_1_H_

#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/2.1/IQtiRadioResponse.h>

#include "hidl_impl/2.0/qti_radio_service_2_0.h"
#include "hidl_impl/2.1/qti_radio_service_utils_2_1.h"

#include "interfaces/nas/RilRequestQuery5gConfigInfoMessage.h"
#include "interfaces/nas/RilRequestQueryUpperLayerIndInfoMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_1 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V2_0::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_1::IQtiRadioResponse> mResponseCbV2_1;
  ::android::sp<V2_1::IQtiRadioIndication> mIndicationCbV2_1;

  ::android::sp<V2_1::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_1;
  }
  ::android::sp<V2_1::IQtiRadioIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCbV2_1;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_0::setCallback_nolock");
    mIndicationCbV2_1 = nullptr;
    mResponseCbV2_1 = nullptr;
    V2_0::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_1::setCallback_nolock");
    mResponseCbV2_1 = V2_1::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    mIndicationCbV2_1 = V2_1::IQtiRadioIndication::castFrom(indCb).withDefault(nullptr);
    V2_0::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForQueryNrBearerAllocation(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryNrBearAllocResult_t> payload) {
    auto respCb = getResponseCallback();
    if (!respCb) {
      // Fallback to 2.0
      V2_0::implementation::QtiRadioImpl<T>::sendResponseForQueryNrBearerAllocation(
          serial, errorCode, payload);
      return;
    }
    V2_1::BearerStatus bearStatus = V2_1::BearerStatus::INVALID;
    if (errorCode == RIL_E_SUCCESS && payload != nullptr) {
      bearStatus = utils::convert_five_g_bearer_status_2_1(payload->status);
    }
    QCRIL_LOG_DEBUG("onNrBearerAllocationResponse_2_1: serial=%d", serial);
    auto ret = respCb->onNrBearerAllocationResponse_2_1(serial, static_cast<uint32_t>(errorCode),
                                                        bearStatus);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }

  virtual void sendResponseForQueryUpperLayerIndInfo(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryUpperLayerIndInfoResult_t> payload) {
    V2_1::UpperLayerIndInfo upliInfo;
    utils::initialize(upliInfo);
    if (errorCode == RIL_E_SUCCESS && payload != nullptr) {
      upliInfo = utils::convert_five_g_upper_layer_ind_info(payload->upli_info);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onUpperLayerIndInfoResponse: serial=%d", serial);
      auto ret =
          respCb->onUpperLayerIndInfoResponse(serial, static_cast<uint32_t>(errorCode), upliInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQuery5gConfigInfo(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQuery5gConfigInfoResult_t> payload) {
    V2_1::ConfigType configInfo = V2_1::ConfigType::INVALID;
    if (errorCode == RIL_E_SUCCESS && payload != nullptr) {
      configInfo = utils::convert_five_g_config_info(payload->config_info);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("on5gConfigInfoResponse: serial=%d", serial);
      auto ret =
          respCb->on5gConfigInfoResponse(serial, static_cast<uint32_t>(errorCode), configInfo);
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
    QCRIL_LOG_DEBUG("V2_1::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> queryUpperLayerIndInfo(int32_t serial) {
    QCRIL_LOG_DEBUG("queryUpperLayerIndInfo: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryUpperLayerIndInfoMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQueryUpperLayerIndInfoResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload =
                  std::static_pointer_cast<qcril::interfaces::RilQueryUpperLayerIndInfoResult_t>(
                      resp->data);
            }
            this->sendResponseForQueryUpperLayerIndInfo(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryUpperLayerIndInfo(serial, RIL_E_NO_MEMORY, nullptr);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> query5gConfigInfo(int32_t serial) {
    QCRIL_LOG_DEBUG("query5gConfigInfo: serial=%d", serial);
    auto msg = std::make_shared<RilRequestQuery5gConfigInfoMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQuery5gConfigInfoResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload = std::static_pointer_cast<qcril::interfaces::RilQuery5gConfigInfoResult_t>(
                  resp->data);
            }
            this->sendResponseForQuery5gConfigInfo(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQuery5gConfigInfo(serial, RIL_E_NO_MEMORY, nullptr);
    }

    return ::android::hardware::Void();
  }

  // Indication APIs
  /**
   * Notifies onUpperLayerIndInfoChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_1::IQtiRadioIndication::onUpperLayerIndInfoChange
   */
  void notifyOnUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onUpperLayerIndInfoChange");
      auto ret =
          indCb->onUpperLayerIndInfoChange(utils::convert_five_g_upper_layer_ind_info(upli_info));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }

  /**
   * Notifies onNrBearerAllocationChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onNrBearerAllocationChange
   *   V2_1::IQtiRadioIndication::onNrBearerAllocationChange_2_1
   */
  void notifyOnNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onNrBearerAllocationChange_2_1");
      ::android::hardware::Return<void> ret = indCb->onNrBearerAllocationChange_2_1(
          utils::convert_five_g_bearer_status_2_1(bearerStatus));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      // Fallback to old
      V2_0::implementation::QtiRadioImpl<T>::notifyOnNrBearerAllocationChange(bearerStatus);
    }
  }

  /**
   * Notifies on5gConfigInfoChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_1::IQtiRadioIndication::on5gConfigInfoChange
   */
  void notifyOn5gConfigInfoChange(five_g_config_type config) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("on5gConfigInfoChange");
      ::android::hardware::Return<void> ret =
          indCb->on5gConfigInfoChange(utils::convert_five_g_config_info(config));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_1::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_1
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_1_H_

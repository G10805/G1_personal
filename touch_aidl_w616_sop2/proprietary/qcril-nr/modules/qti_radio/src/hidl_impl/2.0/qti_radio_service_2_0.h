/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_0_H_
#define _QTI_RADIO_SERVICE_2_0_H_

#include <android/hardware/radio/1.0/types.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadioResponse.h>

#include "hidl_impl/1.0/qti_radio_service_1_0.h"
#include "hidl_impl/2.0/qti_radio_service_utils_2_0.h"

#include "telephony/ril.h"
#include "interfaces/sms/RilRequestCdmaSendSmsMessage.h"
#include "interfaces/nas/RilRequestQuery5GStatusMessage.h"
#include "interfaces/nas/RilRequestQueryNrBearAllocationMessage.h"
#include "interfaces/nas/RilRequestQueryNrDcParamMessage.h"
#include "interfaces/nas/RilRequestQueryNrSignalStrengthMessage.h"
#include "interfaces/nas/RilRequestSet5GStatusMessage.h"
#include "interfaces/nas/nas_types.h"

#undef TAG
#define TAG "RILQ"

namespace android::hardware::radio::utils {
void constructCdmaSms(RIL_CDMA_SMS_Message& rcsm, const V1_0::CdmaSmsMessage& sms);
}

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_0 {
namespace implementation {

template <typename T>
class QtiRadioImpl : public V1_0::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_0::IQtiRadioResponse> mResponseCbV2_0;
  ::android::sp<V2_0::IQtiRadioIndication> mIndicationCbV2_0;

  ::android::sp<V2_0::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_0;
  }
  ::android::sp<V2_0::IQtiRadioIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCbV2_0;
  }

 protected:

  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_0::setCallback_nolock");
    mIndicationCbV2_0 = nullptr;
    mResponseCbV2_0 = nullptr;
    V1_0::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_0::setCallback_nolock");
    mResponseCbV2_0 = V2_0::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    mIndicationCbV2_0 = V2_0::IQtiRadioIndication::castFrom(indCb).withDefault(nullptr);
    V1_0::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForQueryNrBearerAllocation(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryNrBearAllocResult_t> payload) {
    V2_0::BearerStatus bearStatus = V2_0::BearerStatus::INVALID;
    if (errorCode == RIL_E_SUCCESS && payload) {
      bearStatus = utils::convert_five_g_bearer_status(payload->status);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onNrBearerAllocationResponse: serial=%d", serial);
      auto ret = respCb->onNrBearerAllocationResponse(serial, static_cast<uint32_t>(errorCode),
                                                      bearStatus);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForEnable5g(int32_t serial, RIL_Errno errorCode, Status status) {
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onEnable5gResponse: serial=%d", serial);
      auto ret = respCb->onEnable5gResponse(serial, static_cast<uint32_t>(errorCode), status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForDisable5g(int32_t serial, RIL_Errno errorCode, Status status) {
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onDisable5gResponse: serial=%d", serial);
      auto ret = respCb->onDisable5gResponse(serial, static_cast<uint32_t>(errorCode), status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForEnable5gOnly(int32_t serial, RIL_Errno errorCode, Status status) {
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onEnable5gOnlyResponse: serial=%d", serial);
      auto ret = respCb->onEnable5gOnlyResponse(serial, static_cast<uint32_t>(errorCode), status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQuery5gStatus(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQuery5GStatusResult_t> payload) {
    V2_0::EnableStatus status = V2_0::EnableStatus::INVALID;
    if (errorCode == RIL_E_SUCCESS && payload) {
      status = (payload->status == FIVE_G_STATUS_DISABLED) ? V2_0::EnableStatus::DISABLED
                                                           : V2_0::EnableStatus::ENABLED;
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("on5gStatusResponse: serial=%d", serial);
      auto ret = respCb->on5gStatusResponse(serial, static_cast<uint32_t>(errorCode), status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQueryNrDcParam(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryNrDcParamResult_t> payload) {
    V2_0::DcParam params;
    utils::initialize(params);

    if (errorCode == RIL_E_SUCCESS && payload) {
      params = utils::convert_five_g_endc_dcnr(payload->dc);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onNrDcParamResponse: serial=%d", serial);
      auto ret = respCb->onNrDcParamResponse(serial, static_cast<uint32_t>(errorCode), params);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQueryNrSignalStrength(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryNrSignalStrengthResult_t> payload) {
    V2_0::SignalStrength signalStrength;
    utils::initialize(signalStrength);

    if (errorCode == RIL_E_SUCCESS && payload) {
      signalStrength = utils::convert_five_g_signal_strength(payload->signal);
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onSignalStrengthResponse: serial=%d", serial);
      auto ret = respCb->onSignalStrengthResponse(serial, static_cast<uint32_t>(errorCode),
                                                  signalStrength);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForSendCdmaSms(int32_t serial, RIL_Errno errorCode,
                                          std::shared_ptr<RilSendSmsResult_t> payload) {
    android::hardware::radio::V1_0::SendSmsResult result = { -1, ::android::hardware::hidl_string(),
                                                             -1 };
    if (errorCode == RIL_E_SUCCESS && payload) {
      result.messageRef = payload->messageRef;
      result.ackPDU = payload->ackPDU;
      result.errorCode = payload->errorCode;
    }

    V1_0::QtiRadioResponseInfo responseInfo = {};
    responseInfo.serial = serial;
    responseInfo.type = V1_0::QtiRadioResponseType::SOLICITED;
    responseInfo.error = static_cast<V1_0::QtiRadioError>(errorCode);

    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("sendCdmaSmsResponse: serial=%d", serial);
      auto ret = respCb->sendCdmaSmsResponse(responseInfo, result);
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
    QCRIL_LOG_DEBUG("V2_0::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> enable5g(int32_t serial) {
    QCRIL_LOG_DEBUG("enable5g: serial=%d", serial);
    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(this->getContext(serial),
                                                              FIVE_G_MODE_INCLUSIVE);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            if (resp) {
              errorCode = resp->errorCode;
            }
            this->sendResponseForEnable5g(serial, errorCode, Status::SUCCESS);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForEnable5g(serial, RIL_E_NO_MEMORY, Status::SUCCESS);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> disable5g(int32_t serial) {
    QCRIL_LOG_DEBUG("disable5g: serial=%d", serial);

    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(this->getContext(serial),
                                                              FIVE_G_MODE_DISABLED);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            if (resp) {
              errorCode = resp->errorCode;
            }
            this->sendResponseForDisable5g(serial, errorCode, Status::SUCCESS);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForDisable5g(serial, RIL_E_NO_MEMORY, Status::SUCCESS);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> enable5gOnly(int32_t serial) {
    QCRIL_LOG_DEBUG("enable5gOnly: serial=%d", serial);

    auto msg = std::make_shared<RilRequestSet5GStatusMessage>(this->getContext(serial),
                                                              FIVE_G_MODE_EXCLUSIVE);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            if (resp) {
              errorCode = resp->errorCode;
            }
            this->sendResponseForEnable5gOnly(serial, errorCode, Status::SUCCESS);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForEnable5gOnly(serial, RIL_E_NO_MEMORY, Status::SUCCESS);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> query5gStatus(int32_t serial) {
    QCRIL_LOG_DEBUG("query5gStatus: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQuery5GStatusMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQuery5GStatusResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload =
                  std::static_pointer_cast<qcril::interfaces::RilQuery5GStatusResult_t>(resp->data);
            }
            this->sendResponseForQuery5gStatus(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQuery5gStatus(serial, RIL_E_NO_MEMORY, nullptr);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> queryNrDcParam(int32_t serial) {
    QCRIL_LOG_DEBUG("queryNrDcParam: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrDcParamMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQueryNrDcParamResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload =
                  std::static_pointer_cast<qcril::interfaces::RilQueryNrDcParamResult_t>(resp->data);
            }
            this->sendResponseForQueryNrDcParam(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryNrDcParam(serial, RIL_E_NO_MEMORY, nullptr);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> queryNrBearerAllocation(int32_t serial) {
    QCRIL_LOG_DEBUG("queryNrBearerAllocation: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrBearAllocationMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQueryNrBearAllocResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload = std::static_pointer_cast<qcril::interfaces::RilQueryNrBearAllocResult_t>(
                  resp->data);
            }
            this->sendResponseForQueryNrBearerAllocation(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryNrBearerAllocation(serial, RIL_E_NO_MEMORY, nullptr);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> queryNrSignalStrength(int32_t serial) {
    QCRIL_LOG_DEBUG("queryNrSignalStrength: serial=%d", serial);

    auto msg = std::make_shared<RilRequestQueryNrSignalStrengthMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status /*status*/,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<qcril::interfaces::RilQueryNrSignalStrengthResult_t> payload;
            if (resp) {
              errorCode = resp->errorCode;
              payload =
                  std::static_pointer_cast<qcril::interfaces::RilQueryNrSignalStrengthResult_t>(
                      resp->data);
            }
            this->sendResponseForQueryNrSignalStrength(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryNrSignalStrength(serial, RIL_E_NO_MEMORY, nullptr);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> sendCdmaSms(
      int32_t /*serial*/, const android::hardware::radio::V1_0::CdmaSmsMessage& /*sms*/, bool /*expectMore*/) {
    /*
    QCRIL_LOG_DEBUG("sendCdmaSms: serial=%d expectMore=%d", serial, expectMore);
    RIL_CDMA_SMS_Message rcsm{};
    android::hardware::radio::utils::constructCdmaSms(rcsm, sms);
    if (expectMore) {
      rcsm.expectMore = 1;
    }
    auto msg = std::make_shared<RilRequestCdmaSendSmsMessage>(this->getContext(serial), rcsm);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          ([this, serial]([[maybe_unused]] std::shared_ptr<Message> msg,
                          Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_NO_MEMORY;
            std::shared_ptr<RilSendSmsResult_t> payload;
            if (status == Message::Callback::Status::SUCCESS && resp) {
              errorCode = resp->errorCode;
              payload = std::static_pointer_cast<RilSendSmsResult_t>(resp->data);
            }
            this->sendResponseForSendCdmaSms(serial, errorCode, payload);
          }));
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForSendCdmaSms(serial, RIL_E_NO_MEMORY, nullptr);
    }
    */
    return ::android::hardware::Void();
  }

  // Indication APIs
  /**
   * Notifies on5gStatusChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::on5gStatusChange
   */
  void notifyOn5gStatusChange(five_g_status status) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("on5gStatusChange");
      auto ret = indCb->on5gStatusChange(utils::convert_five_g_status(status));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }

  /**
   * Notifies onNrDcParamChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onNrDcParamChange
   */
  void notifyOnNrDcParamChange(five_g_endc_dcnr dcParam) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onNrDcParamChange");
      auto ret = indCb->onNrDcParamChange(utils::convert_five_g_endc_dcnr(dcParam));
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
   */
  void notifyOnNrBearerAllocationChange(five_g_bearer_status bearerStatus) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onNrBearerAllocationChange");
      auto ret =
          indCb->onNrBearerAllocationChange(utils::convert_five_g_bearer_status(bearerStatus));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }

  /**
   * Notifies onSignalStrengthChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onSignalStrengthChange
   */
  void notifyOnSignalStrengthChange(five_g_signal_strength signalStrength) {
    auto indCb = getIndicationCallback();
    if (indCb != nullptr) {
      QCRIL_LOG_DEBUG("onSignalStrengthChange");
      auto ret =
          indCb->onSignalStrengthChange(utils::convert_five_g_signal_strength(signalStrength));
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } else {
      QCRIL_LOG_ERROR("indCb NULL");
    }
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_0::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_0_H_

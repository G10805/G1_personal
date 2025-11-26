/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_1_1_H_
#define __RADIO_CONFIG_SERVICE_1_1_H_

#include <android/hardware/radio/config/1.1/IRadioConfig.h>
#include <android/hardware/radio/config/1.1/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.1/IRadioConfigResponse.h>
#include "UnSolMessages/RadioConfigClientConnectedMessage.h"
#include "request/SetPreferredDataModemRequestMessage.h"
#include "interfaces/nas/RilRequestGetPhoneCapabilityMessage.h"
#include "interfaces/nas/nas_types.h"
#include "interfaces/mbn/QcRilRequestGetModemsConfigMessage.h"
#include "interfaces/mbn/QcRilRequestSetModemsConfigMessage.h"
#include "interfaces/mbn/mbn.h"

#include "hidl_impl/1.0/radio_config_service_1_0.h"
#include "hidl_impl/1.1/radio_config_service_utils_1_1.h"

#undef TAG
#define TAG "RILQ"


namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_1 {
namespace implementation {

template <typename T>
class RadioConfigServiceImpl : public V1_0::implementation::RadioConfigServiceImpl<T> {
 private:
  ::android::sp<V1_1::IRadioConfigResponse> mResponseCbV1_1;
  ::android::sp<V1_1::IRadioConfigIndication> mIndicationCbV1_1;

  ::android::sp<V1_1::IRadioConfigResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV1_1;
  }
  ::android::sp<V1_1::IRadioConfigIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCbV1_1;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_1::clearCallbacks_nolock");
    mIndicationCbV1_1 = nullptr;
    mResponseCbV1_1 = nullptr;
    V1_0::implementation::RadioConfigServiceImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setResponseFunctions_nolock(const ::android::sp<V1_0::IRadioConfigResponse>& respCb,
                                   const ::android::sp<V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("V1_1::setResponseFunctions_nolock");
    mResponseCbV1_1 = V1_1::IRadioConfigResponse::castFrom(respCb).withDefault(nullptr);
    mIndicationCbV1_1 = V1_1::IRadioConfigIndication::castFrom(indCb).withDefault(nullptr);
    auto msg = std::make_shared<rildata::RadioConfigClientConnectedMessage>();
    if (msg != nullptr) {
        QCRIL_LOG_DEBUG("RadioConfigImpl_1_1: broadcasting client connected");
        msg->broadcast();
    } else {
        QCRIL_LOG_ERROR("RadioConfigImpl_1_1:: failed to allocate RadioConfigClientConnectedMessage");
    }
    V1_0::implementation::RadioConfigServiceImpl<T>::setResponseFunctions_nolock(respCb, indCb);
  }

  virtual void sendResponseForSetModemsConfig(int32_t serial, RIL_Errno errorCode) {
    using namespace ::android::hardware::radio::V1_0;
    auto respCb = this->getResponseCallback();
    if (respCb) {
        QCRIL_LOG_DEBUG("setModemsConfigResponse: serial=%d, error=%d", serial, errorCode);
        RadioResponseInfo info = {
            .type = RadioResponseType::SOLICITED,
            .serial = serial,
            .error = static_cast<RadioError>(errorCode),
        };
        auto ret = respCb->setModemsConfigResponse(info);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }
  }

  virtual void sendResponseForGetModemsConfig(int32_t serial, RIL_Errno errorCode,
        std::shared_ptr<qcril::interfaces::ModemsConfigResp> result) {
    using namespace ::android::hardware::radio::V1_0;
    auto respCb = getResponseCallback();
    if (respCb != nullptr) {
         RadioResponseInfo info = {
             .type = RadioResponseType::SOLICITED,
             .serial = serial,
             .error = static_cast<RadioError>(errorCode),
         };
         config::V1_1::ModemsConfig modemsConfig = {};
         if (errorCode == RIL_E_SUCCESS && result != nullptr) {
             modemsConfig.numOfLiveModems = result->numOfModems;
         }
         Return<void> ret = respCb->getModemsConfigResponse(info, modemsConfig);
         if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
         }
    }
  }

  virtual void setPreferredDataModemResponse(
        int32_t serial,
        std::shared_ptr<rildata::SetPreferredDataModemResponse_t> response)
  {
    using namespace ::android::hardware::radio::V1_0;
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:: setPreferredDataModemResponse ENTRY");
    auto respCb = this->getResponseCallback();
    if (respCb != nullptr)
    {
        RadioError error = RadioError::INTERNAL_ERR;
        if (response != nullptr) {
            utils::convertErrorToHidl(response->toResponseError(), error);
        }
        RadioResponseInfo info = {
        .type = RadioResponseType::SOLICITED,
        .serial = serial,
        .error = error
        };
        auto ret = respCb->setPreferredDataModemResponse(info);
        if (!ret.isOk())
        {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    } else {
        QCRIL_LOG_INFO("RadioConfigImpl_1_1:: callback is null");
    }
  }

  virtual void sendResponseForGetPhoneCapability(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilPhoneCapabilityResult_t> payload) {
    radio::V1_0::RadioResponseInfo responseInfo{ radio::V1_0::RadioResponseType::SOLICITED, serial,
                                                 static_cast<radio::V1_0::RadioError>(errorCode) };
    config::V1_1::PhoneCapability phoneCapability = {};
    if (errorCode == RIL_E_SUCCESS && payload) {
      phoneCapability = config::utils::convertPhoneCapabilityToHal(payload->phoneCap);
    }
    auto respCb = this->getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("getPhoneCapabilityResponse: serial=%d, error=%d", serial, errorCode);
      auto ret = respCb->getPhoneCapabilityResponse(responseInfo, phoneCapability);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
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

  ::android::hardware::Return<void> getPhoneCapability(int32_t serial) {
    QCRIL_LOG_DEBUG("IRadiConfig: getPhoneCapability: serial=%d", serial);
    auto msg = std::make_shared<RilRequestGetPhoneCapabilityMessage>();
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_INTERNAL_ERR;
            std::shared_ptr<qcril::interfaces::RilPhoneCapabilityResult_t> phoneCapability{};
            if (status == Message::Callback::Status::SUCCESS && resp) {
              errorCode = resp->errorCode;
              phoneCapability =
                  std::static_pointer_cast<qcril::interfaces::RilPhoneCapabilityResult_t>(
                      resp->data);
            }
            this->sendResponseForGetPhoneCapability(serial, errorCode, phoneCapability);
          }));
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForGetPhoneCapability(serial, RIL_E_NO_MEMORY, nullptr);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> setPreferredDataModem(int32_t serial, uint8_t modemId) {
    QCRIL_LOG_DEBUG("IRadiConfig: setPreferredDataModem: serial=%d", serial);
    QCRIL_LOG_INFO("RadioConfigImpl_1_1:: setPreferredDataModem serial=%d modemId=%d", serial, modemId);
    auto msg = std::make_shared<rildata::SetPreferredDataModemRequestMessage>(modemId);
    auto cb = std::bind(&V1_1::implementation::RadioConfigServiceImpl<T>::setPreferredDataModemResponse,
                        this, serial, std::placeholders::_3);
    GenericCallback<rildata::SetPreferredDataModemResponse_t> responseCb(cb);
    if (msg != nullptr) {
        msg->setCallback(&responseCb);
        msg->dispatch();
    } else {
        QCRIL_LOG_ERROR("RadioConfigImpl_1_1:: failed to allocate setPreferredDataModem message");
        rildata::SetPreferredDataModemResponse_t errorResponse = {
        .errCode = rildata::SetPreferredDataModemResult_t::DDS_SWITCH_FAILED
        };
        this->setPreferredDataModemResponse(serial,
                std::make_shared<rildata::SetPreferredDataModemResponse_t>(errorResponse));
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> setModemsConfig(
      int32_t serial, const ::android::hardware::radio::config::V1_1::ModemsConfig& modemsConfig) {

    uint8_t numOfLiveModems = modemsConfig.numOfLiveModems;

    QCRIL_LOG_DEBUG("IRadiConfig: setModemsConfig: serial=%d, numOfLiveModems=%d",
            serial, numOfLiveModems);

    // Currently, we only support switch to single/multi sim mode i.e live modems = 1 or 2
    if (numOfLiveModems != 1 && numOfLiveModems != 2) {
        QCRIL_LOG_DEBUG("Invalid liveModems arg passed");
        sendResponseForSetModemsConfig(serial, RIL_E_INVALID_ARGUMENTS);
        return ::android::hardware::Void();
    }

    std::shared_ptr<RadioConfigContext> ctx = this->getContext(serial);
    auto msg = std::make_shared<QcRilRequestSetModemsConfigMessage>(
            ctx, numOfLiveModems);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> /* msg */, Message::Callback::Status status,
                           std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
                RIL_Errno errorCode = RIL_E_INTERNAL_ERR;
                if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                    errorCode = resp->errorCode;
                }
                sendResponseForSetModemsConfig(serial, errorCode);
            });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        sendResponseForSetModemsConfig(serial, RIL_E_NO_MEMORY);
    }

    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> getModemsConfig(int32_t serial) {
    QCRIL_LOG_DEBUG("IRadiConfig: getModemsConfig: serial=%d", serial);

    std::shared_ptr<RadioConfigContext> ctx = this->getContext(serial);
    auto msg = std::make_shared<QcRilRequestGetModemsConfigMessage>(ctx);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> /* msg */, Message::Callback::Status status,
                           std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
                RIL_Errno errorCode = RIL_E_INTERNAL_ERR;
                std::shared_ptr<qcril::interfaces::ModemsConfigResp> result = nullptr;
                if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                    errorCode = resp->errorCode;
                    result = std::static_pointer_cast<qcril::interfaces::ModemsConfigResp>(resp->data);
                }
                sendResponseForGetModemsConfig(serial, errorCode, result);
            });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        sendResponseForGetModemsConfig(serial, RIL_E_NO_MEMORY, nullptr);
    }
    return ::android::hardware::Void();
  }

  // Indication APIs
};

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_1::IRadioConfig>::getVersion();

}  // namespace implementation
}  // namespace V1_1
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_1_1_H_

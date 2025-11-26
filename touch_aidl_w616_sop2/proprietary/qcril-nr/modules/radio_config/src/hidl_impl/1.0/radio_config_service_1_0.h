/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_1_0_H_
#define __RADIO_CONFIG_SERVICE_1_0_H_

#include <android/hardware/radio/config/1.0/IRadioConfig.h>
#include <android/hardware/radio/config/1.0/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.0/IRadioConfigResponse.h>

#include "HalServiceImplFactory.h"
#include "hidl_impl/radio_config_service_base.h"
#include "interfaces/uim/UimGetSlotStatusRequestMsg.h"
#include "interfaces/uim/UimSwitchSlotRequestMsg.h"
#include <android/hardware/radio/config/1.0/types.h>
#include <interfaces/uim/UimSlotStatusInd.h>
#include "hidl_impl/1.0/radio_config_service_utils_1_0.h"
#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {
namespace implementation {

template <typename T>
class RadioConfigServiceImpl : public T, public RadioConfigServiceBase {
 private:
  ::android::sp<V1_0::IRadioConfigResponse> mResponseCb;
  ::android::sp<V1_0::IRadioConfigIndication> mIndicationCb;

  ::android::sp<V1_0::IRadioConfigResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCb;
  }

  ::android::sp<V1_0::IRadioConfigIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCb;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_0::clearCallbacks_nolock");
    mIndicationCb = nullptr;
    mResponseCb = nullptr;
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setResponseFunctions_nolock(
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& respCb,
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("V1_0::setResponseFunctions_nolock");
    if (mResponseCb != nullptr) {
      mResponseCb->unlinkToDeath(this);
    }
    mIndicationCb = indCb;
    mResponseCb = respCb;
    if (mResponseCb != nullptr) {
      mResponseCb->linkToDeath(this, 0);
    }
  }

  virtual void sendResponseForGetSimSlotsStatus(int32_t serial, ::android::hardware::radio::V1_0::RadioError errorCode,
                                                   std::shared_ptr<RIL_UIM_SlotsStatusInfo> responseDataPtr ) {
   ::android::hardware::radio::V1_0::RadioResponseInfo responseInfo{
                        ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
                                          static_cast<::android::hardware::radio::V1_0::RadioError>(errorCode) };
    std::vector<SimSlotStatus> slot_status = {};
    if (responseInfo.error == ::android::hardware::radio::V1_0::RadioError::NONE) {
      slot_status.resize(responseDataPtr->slot_status.size());

      for (uint8_t index = 0; index < responseDataPtr->slot_status.size(); index++)
      {
        config::utils::convertUimSlotStatusToHal(responseDataPtr->slot_status.at(index), slot_status[index]);
      }
    }
    auto respCb = this->getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("getSimSlotsStatusResponse: serial=%d, error=%d", serial, errorCode);
      auto ret = respCb->getSimSlotsStatusResponse(responseInfo, slot_status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForSetSimSlotsMapping(int32_t serial,::android::hardware::radio::V1_0::RadioError errorCode) {
    ::android::hardware::radio::V1_0::RadioResponseInfo responseInfo{
                        ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
                                          static_cast<::android::hardware::radio::V1_0::RadioError>(errorCode) };
    auto respCb = this->getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("setSimSlotsMappingResponse: serial=%d, error=%d", serial, errorCode);
      auto ret = respCb->setSimSlotsMappingResponse(responseInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

 public:
  static const HalServiceImplVersion& getVersion();

  bool registerService(qcril_instance_id_e_type instId) {
    this->setInstanceId(instId);

    if (instId == QCRIL_DEFAULT_INSTANCE_ID) {
      std::string serviceName {"default"};
      android::status_t ret = T::registerAsService(serviceName);
      Log::getInstance().d(std::string("[RadioConfig]: ") + T::descriptor +
                           (ret == android::OK ? " registered" : " failed to register"));
      return (ret == android::OK);
    }
    return false;
  }

  // Functions from IRadioConfig
  ::android::hardware::Return<void> setResponseFunctions(
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& respCb,
      const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("setResponseFunctions");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setResponseFunctions_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> getSimSlotsStatus(int32_t serial) {
    QCRIL_LOG_DEBUG("IRadioConfig: getSimSlotsStatus: serial=%d", serial);
    auto msg = std::make_shared<UimGetSlotStatusRequestMsg>();
    if (msg)
    {
      GenericCallback<RIL_UIM_SlotsStatusInfo> cb(
          ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                          std::shared_ptr<RIL_UIM_SlotsStatusInfo> responseDataPtr) -> void
      {
          ::android::hardware::radio::V1_0::RadioError errorCode{
                         ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR };
          if (solicitedMsg && responseDataPtr &&
              status == Message::Callback::Status::SUCCESS)
          {
            errorCode = static_cast<::android::hardware::radio::V1_0::RadioError>(responseDataPtr->err);
          }
          this->sendResponseForGetSimSlotsStatus(serial,errorCode,responseDataPtr);
      }));
      msg->setCallback(&cb);
      msg->dispatch();
    }
    else
    {
        this->sendResponseForGetSimSlotsStatus(serial,
              ::android::hardware::radio::V1_0::RadioError::NO_MEMORY,nullptr);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> setSimSlotsMapping(
      int32_t serial, const ::android::hardware::hidl_vec<uint32_t>& slotMap) {
    QCRIL_LOG_DEBUG("IRadioConfig: setSimSlotsMapping: serial=%d", serial);
    auto msg = std::make_shared<UimSwitchSlotRequestMsg>(slotMap);
    if (msg) {
        GenericCallback<RIL_UIM_Errno> cb(
             ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
       {
         ::android::hardware::radio::V1_0::RadioError errorCode{
                   ::android::hardware::radio::V1_0::RadioError::INTERNAL_ERR };
           if (solicitedMsg && responseDataPtr &&
                status == Message::Callback::Status::SUCCESS)
           {
             errorCode = static_cast<::android::hardware::radio::V1_0::RadioError>(*(responseDataPtr.get()));
           }
           this->sendResponseForSetSimSlotsMapping(serial,errorCode);
       }));
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
           this->sendResponseForSetSimSlotsMapping(serial,
                    ::android::hardware::radio::V1_0::RadioError::NO_MEMORY);
    }
    return ::android::hardware::Void();
  }
  // Indication APIs

  /**
   * Notifies on5gStatusChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V1_0::IRadioConfigIndication::simSlotsStatusChanged
   */
  void sendSlotStatusIndication(const std::shared_ptr<UimSlotStatusInd> msg) {
    std::vector<SimSlotStatus>   slotStatus = {};
    auto indCb = this->getIndicationCallback();
    QCRIL_LOG_INFO("indCb: %s", indCb ? "valid" : "invalid");
    if (indCb && msg) {
      ::android::hardware::hidl_vec<V1_0::SimSlotStatus> slotStatus{};
      slotStatus.resize(msg->get_status().size());

      for (uint8_t index = 0; index < slotStatus.size(); index++)
      {
        config::utils::convertUimSlotStatusToHal(msg->get_status().at(index), slotStatus[index]);
      }

      Return<void> ret = indCb->simSlotsStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED, slotStatus);

      if (!ret.isOk())
      {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
  }

};

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_0::IRadioConfig>::getVersion();

}  // namespace implementation
}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_1_0_H_

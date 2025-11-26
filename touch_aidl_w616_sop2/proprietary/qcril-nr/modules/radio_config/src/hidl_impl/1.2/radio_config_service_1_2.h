/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_1_2_H_
#define __RADIO_CONFIG_SERVICE_1_2_H_

#include <android/hardware/radio/config/1.1/IRadioConfig.h>
#include <android/hardware/radio/config/1.2/IRadioConfigIndication.h>
#include <android/hardware/radio/config/1.2/IRadioConfigResponse.h>

#include "hidl_impl/1.1/radio_config_service_1_1.h"
#include "hidl_impl/1.0/radio_config_service_utils_1_0.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_2 {
namespace implementation {

template <typename T>
class RadioConfigServiceImpl : public V1_1::implementation::RadioConfigServiceImpl<T> {
 private:
  ::android::sp<V1_2::IRadioConfigResponse> mResponseCbV1_2;
  ::android::sp<V1_2::IRadioConfigIndication> mIndicationCbV1_2;

  ::android::sp<V1_2::IRadioConfigResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV1_2;
  }
  ::android::sp<V1_2::IRadioConfigIndication> getIndicationCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mIndicationCbV1_2;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V1_2::clearCallbacks_nolock");
    mIndicationCbV1_2 = nullptr;
    mResponseCbV1_2 = nullptr;
    V1_1::implementation::RadioConfigServiceImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setResponseFunctions_nolock(const ::android::sp<V1_0::IRadioConfigResponse>& respCb,
                                   const ::android::sp<V1_0::IRadioConfigIndication>& indCb) {
    QCRIL_LOG_DEBUG("V1_2::setResponseFunctions_nolock");
    mResponseCbV1_2 = V1_2::IRadioConfigResponse::castFrom(respCb).withDefault(nullptr);
    mIndicationCbV1_2 = V1_2::IRadioConfigIndication::castFrom(indCb).withDefault(nullptr);
    V1_1::implementation::RadioConfigServiceImpl<T>::setResponseFunctions_nolock(respCb, indCb);
  }

  virtual void sendResponseForGetSimSlotsStatus(int32_t serial, ::android::hardware::radio::V1_0::RadioError errorCode,
                                                std::shared_ptr<RIL_UIM_SlotsStatusInfo> responseDataPtr) {
    ::android::hardware::radio::V1_0::RadioResponseInfo rsp_info {
                         ::android::hardware::radio::V1_0::RadioResponseType::SOLICITED, serial,
                                          static_cast<::android::hardware::radio::V1_0::RadioError>(errorCode) };
    auto respCb = this->getResponseCallback();
    if (!respCb) {
      V1_1::implementation::RadioConfigServiceImpl<T>::sendResponseForGetSimSlotsStatus(
          serial, errorCode, responseDataPtr);
      return;
    }
     std::vector<android::hardware::radio::config::V1_2::SimSlotStatus> slot_status;
     if (rsp_info.error == ::android::hardware::radio::V1_0::RadioError::NONE)
     {
           slot_status.resize(responseDataPtr->slot_status.size());
           for (int index = 0; index < responseDataPtr->slot_status.size(); index++)
           {
             hidl_string    hidl_eid = {};
             char         * eid      = nullptr;

             config::utils::convertUimSlotStatusToHal(responseDataPtr->slot_status.at(index),
                                       slot_status[index].base);

             if(responseDataPtr->slot_status.at(index).eid.size() != 0)
             {
               eid = utils::radio_config_bin_to_hexstring
                       (responseDataPtr->slot_status.at(index).eid.data(),
                        responseDataPtr->slot_status.at(index).eid.size());
             }

             if(eid != nullptr)
             {
               hidl_eid.setToExternal(eid, strlen(eid));
             }

             slot_status[index].eid = hidl_eid;

             if(eid != nullptr)
             {
                delete[] eid;
              }
            }
      }
      auto ret = respCb->getSimSlotsStatusResponse_1_2(rsp_info, slot_status);
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

  // Indication APIs

  /**
   * Notifies on5gStatusChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V1_0::IRadioConfigIndication::simSlotsStatusChanged
   *   V1_2::IRadioConfigIndication::simSlotsStatusChanged_1_2
   */
  void sendSlotStatusIndication(const std::shared_ptr<UimSlotStatusInd> msg) {
    std::vector<android::hardware::radio::config::V1_2::SimSlotStatus> slotStatus = {};
    if (!msg) {
      QCRIL_LOG_ERROR("msg is nullptr");
      return;
    }
    auto indCb = this->getIndicationCallback();
    QCRIL_LOG_INFO("indCb: %s", indCb ? "valid" : "invalid");
    if (!indCb) {
      V1_1::implementation::RadioConfigServiceImpl<T>::sendSlotStatusIndication(msg);
      return;
    }
    slotStatus.resize(msg->get_status().size());

    for (int index = 0; index < slotStatus.size(); index++)
    {
      hidl_string    hidl_eid = {};
      char         * eid      = nullptr;

      config::utils::convertUimSlotStatusToHal(msg->get_status().at(index), slotStatus[index].base);

      if(msg->get_status().at(index).eid.size() != 0)
      {
        eid = utils::radio_config_bin_to_hexstring(msg->get_status().at(index).eid.data(),
                                            msg->get_status().at(index).eid.size());
      }

      if(eid != nullptr)
      {
        hidl_eid.setToExternal(eid, strlen(eid));
      }

      slotStatus[index].eid = hidl_eid;

      if(eid != nullptr)
      {
        delete[] eid;
      }
    }
    Return<void> ret = indCb->simSlotsStatusChanged_1_2(::android::hardware::radio::V1_0::RadioIndicationType::UNSOLICITED, slotStatus);

    if (!ret.isOk())
    {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
};

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_1::IRadioConfig>::getVersion();

}  // namespace implementation
}  // namespace V1_2
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_1_2_H_

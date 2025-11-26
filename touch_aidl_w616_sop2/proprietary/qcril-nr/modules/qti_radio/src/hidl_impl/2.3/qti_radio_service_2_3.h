/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_2_3_H_
#define _QTI_RADIO_SERVICE_2_3_H_

#include <vendor/qti/hardware/radio/qtiradio/2.3/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.3/IQtiRadioResponse.h>

#include "hidl_impl/2.2/qti_radio_service_2_2.h"

#include <cutils/properties.h>
#include "qcril_config.h"
#include "qcril_other.h"
#include "qcril_legacy_apis.h"

#include "interfaces/nas/RilRequestEnableEndcMessage.h"
#include "interfaces/nas/RilRequestQueryEndcStatusMessage.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_3 {
namespace implementation {

property_id_type getPropertyId(std::string prop);

template <typename T>
class QtiRadioImpl : public V2_2::implementation::QtiRadioImpl<T> {
 private:
  ::android::sp<V2_3::IQtiRadioResponse> mResponseCbV2_3;

  ::android::sp<V2_3::IQtiRadioResponse> getResponseCallback() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    return mResponseCbV2_3;
  }

 protected:
  void clearCallbacks_nolock() {
    QCRIL_LOG_DEBUG("V2_3::setCallback_nolock");
    mResponseCbV2_3 = nullptr;
    V2_2::implementation::QtiRadioImpl<T>::clearCallbacks_nolock();
  }

  virtual void clearCallbacks() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    clearCallbacks_nolock();
  }

  void setCallback_nolock(const ::android::sp<V1_0::IQtiRadioResponse>& respCb,
                          const ::android::sp<V1_0::IQtiRadioIndication>& indCb) {
    QCRIL_LOG_DEBUG("V2_3::setCallback_nolock");
    mResponseCbV2_3 = V2_3::IQtiRadioResponse::castFrom(respCb).withDefault(nullptr);
    V2_2::implementation::QtiRadioImpl<T>::setCallback_nolock(respCb, indCb);
  }

  virtual void sendResponseForEnableEndc(int32_t serial, RIL_Errno errorCode, V2_0::Status status) {
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onEnableEndcResponse: serial=%d", serial);
      auto ret = respCb->onEnableEndcResponse(serial, static_cast<uint32_t>(errorCode), status);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }

  virtual void sendResponseForQueryEndcStatus(
      int32_t serial, RIL_Errno errorCode,
      std::shared_ptr<qcril::interfaces::RilQueryEndcStatusResult_t> payload) {
    V2_3::EndcStatus endcStatus = V2_3::EndcStatus::INVALID;
    if (errorCode == RIL_E_SUCCESS && payload != nullptr) {
      endcStatus = (payload->status == ENDC_STATUS_DISABLED) ? V2_3::EndcStatus::DISABLED
                                                             : V2_3::EndcStatus::ENABLED;
    }
    auto respCb = getResponseCallback();
    if (respCb) {
      QCRIL_LOG_DEBUG("onEndcStatusResponse: serial=%d", serial);
      auto ret = respCb->onEndcStatusResponse(serial, static_cast<uint32_t>(errorCode), endcStatus);
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
    QCRIL_LOG_DEBUG("V2_3::setCallback");
    std::unique_lock<qtimutex::QtiSharedMutex> lock(this->mCallbackLock);
    setCallback_nolock(respCb, indCb);
    return ::android::hardware::Void();
  }

  // Functions from IQtiRadio
  ::android::hardware::Return<void> enableEndc(int32_t serial, bool enable) {
    QCRIL_LOG_DEBUG("enableEndc: serial=%d enable:%d", serial, enable);
    auto msg = std::make_shared<RilRequestEnableEndcMessage>(this->getContext(serial), enable);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_SYSTEM_ERR;
            if (status == Message::Callback::Status::SUCCESS) {
              if (resp) {
                errorCode = resp->errorCode;
              } else {
                errorCode = RIL_E_NO_MEMORY;
              }
            }
            this->sendResponseForEnableEndc(serial, errorCode, V2_0::Status::SUCCESS);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForEnableEndc(serial, RIL_E_NO_MEMORY, V2_0::Status::FAILURE);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<void> queryEndcStatus(int32_t serial) {
    QCRIL_LOG_DEBUG("queryEndcStatus: serial=%d", serial);
    auto msg = std::make_shared<RilRequestQueryEndcStatusMessage>(this->getContext(serial));
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          [this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                         std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_SYSTEM_ERR;
            std::shared_ptr<qcril::interfaces::RilQueryEndcStatusResult_t> payload;
            if (status == Message::Callback::Status::SUCCESS) {
              if (resp) {
                errorCode = resp->errorCode;
                payload = std::static_pointer_cast<qcril::interfaces::RilQueryEndcStatusResult_t>(
                    resp->data);
              } else {
                errorCode = RIL_E_NO_MEMORY;
              }
            }
            this->sendResponseForQueryEndcStatus(serial, errorCode, payload);
          });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      this->sendResponseForQueryEndcStatus(serial, RIL_E_NO_MEMORY, nullptr);
    }
    return ::android::hardware::Void();
  }

  ::android::hardware::Return<int32_t> getPropertyValueInt(
      const ::android::hardware::hidl_string& prop, int32_t def) {
    int prop_val = def;
    if (!prop.empty()) {
      property_id_type prop_id = getPropertyId(prop);
      QCRIL_LOG_DEBUG("Get property from %s", (prop_id != PROPERTY_ID_MAX ? "QCRILDB" : "ADB"));
      if (prop_id != PROPERTY_ID_MAX) {
        int config_value;
        if (qcril_config_get(prop_id, config_value) == E_SUCCESS) {
          prop_val = config_value;
        }
      } else {
        qmi_ril_get_property_value_from_integer(prop.c_str(), &prop_val, def);
      }
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %d", prop.c_str(), prop_val);
    return prop_val;
  }

  ::android::hardware::Return<bool> getPropertyValueBool(const ::android::hardware::hidl_string& prop,
                                                         bool def) {
    bool prop_val = def;
    if (!prop.empty()) {
      property_id_type prop_id = getPropertyId(prop);
      QCRIL_LOG_DEBUG("Get property from %s", (prop_id != PROPERTY_ID_MAX ? "QCRILDB" : "ADB"));
      if (prop_id != PROPERTY_ID_MAX) {
        bool config_value;
        if (qcril_config_get(prop_id, config_value) == E_SUCCESS) {
          prop_val = config_value;
        }
      } else {
        boolean boolean_val;
        qmi_ril_get_property_value_from_boolean(prop.c_str(), &boolean_val, (boolean)def);
        prop_val = boolean_val;
      }
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %s", prop.c_str(), (prop_val ? "true" : "false"));
    return prop_val;
  }

  ::android::hardware::Return<void> getPropertyValueString(
      const ::android::hardware::hidl_string& prop, const ::android::hardware::hidl_string& def,
      V2_3::IQtiRadio::getPropertyValueString_cb _hidl_cb) {
    ::android::hardware::hidl_string prop_val = def;
    if (!prop.empty()) {
      property_id_type prop_id = getPropertyId(prop);
      QCRIL_LOG_DEBUG("Get property from %s", (prop_id != PROPERTY_ID_MAX ? "QCRILDB" : "ADB"));
      if (prop_id != PROPERTY_ID_MAX) {
        std::string config_value;
        if (qcril_config_get(prop_id, config_value) == E_SUCCESS) {
          prop_val = config_value;
        }
      } else {
        char string_val[PROPERTY_VALUE_MAX]{0};
        qmi_ril_get_property_value_from_string(prop.c_str(), string_val, def.c_str());
        prop_val = string_val;
      }
    }
    QCRIL_LOG_DEBUG("Property: %s, Value: %s", prop.c_str(), prop_val.c_str());
    _hidl_cb(prop_val);
    return ::android::hardware::Void();
  }
};

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_3::IQtiRadio>::getVersion();

}  // namespace implementation
}  // namespace V2_3
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_2_3_H_

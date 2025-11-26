/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RILQ"

#include "interfaces/nas/RilRequestSetMsimPreferenceMessage.h"

#include "framework/Log.h"
#include "qti_radio_config_aidl_service.h"

qcril_instance_id_e_type IQtiRadioConfigImpl::getInstanceId() {
  return mInstanceId;
}

std::shared_ptr<IQtiRadioConfigContext> IQtiRadioConfigImpl::getContext(uint32_t serial) {
  return std::make_shared<IQtiRadioConfigContext>(mInstanceId, serial);
}

std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse> IQtiRadioConfigImpl::getResponseCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
}

std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication> IQtiRadioConfigImpl::getIndicationCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
}

void IQtiRadioConfigImpl::clearCallbacks_nolock() {
  mIndicationCb = nullptr;
  mResponseCb = nullptr;
  mDeathRecipient = nullptr;
}

void IQtiRadioConfigImpl::clearCallbacks() {
  QCRIL_LOG_FUNC_ENTRY("enter");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    clearCallbacks_nolock();
  }
  QCRIL_LOG_FUNC_ENTRY("exit");
}

void IQtiRadioConfigImpl::deathNotifier(void* /*cookie*/) {
  QCRIL_LOG_DEBUG("IQtiRadioConfig::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

void deathRecpCallback(void* cookie) {
  IQtiRadioConfigImpl* iQtiRadioConfigImpl = static_cast<IQtiRadioConfigImpl*>(cookie);
  if (iQtiRadioConfigImpl != nullptr) {
    iQtiRadioConfigImpl->deathNotifier(cookie);
  }
}

IQtiRadioConfigImpl::IQtiRadioConfigImpl(qcril_instance_id_e_type instance): mInstanceId(instance) {}

IQtiRadioConfigImpl::~IQtiRadioConfigImpl(){}

void IQtiRadioConfigImpl::setResponseFunctions_nolock(
    const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse>& in_qtiRadioConfigResponse,
    const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication>& in_qtiRadioConfigIndication) {
  QCRIL_LOG_DEBUG("IQtiRadioConfig::setResponseFunctions_nolock");
  mResponseCb = in_qtiRadioConfigResponse;
  mIndicationCb = in_qtiRadioConfigIndication;
}

::ndk::ScopedAStatus IQtiRadioConfigImpl::setCallbacks(
    const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse>& in_qtiRadioConfigResponse,
    const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication>& in_qtiRadioConfigIndication) {
  QCRIL_LOG_DEBUG("IQtiRadioConfig::setResponseFunctions");
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr) {
    AIBinder_unlinkToDeath(mResponseCb->asBinder().get(), mDeathRecipient,
                           reinterpret_cast<void*>(this));
  }
  setResponseFunctions_nolock(in_qtiRadioConfigResponse, in_qtiRadioConfigIndication);
  if (mResponseCb != nullptr) {
    if (mDeathRecipient == nullptr) {
      mDeathRecipient = AIBinder_DeathRecipient_new(&deathRecpCallback);
    }
    AIBinder_linkToDeath(mResponseCb->asBinder().get(), mDeathRecipient,
                         reinterpret_cast<void*>(this));
  }
  return ndk::ScopedAStatus::ok();
}

// Send Response Functions

/**
 * @param status : True indicates device is in secure mode
 */

void IQtiRadioConfigImpl::sendResponseForGetSecureModeStatus(int32_t in_serial,
    RIL_Errno errorCode, bool status) {

  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse> respCb = getResponseCallback();
  if (respCb == nullptr) {
    QCRIL_LOG_DEBUG("IQtiRadioConfig: getResponseCallback Failed");
    return;
  }

  QCRIL_LOG_DEBUG("getSecureModeStatusResponse: serial=%d, error=%d, status= %s ",
      in_serial, errorCode, status ? "true":"false");
  auto ret = respCb->getSecureModeStatusResponse(in_serial, static_cast<int32_t>(errorCode), status);
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.getDescription().c_str());
  }
}

void IQtiRadioConfigImpl::sendResponseForSetMsimPreference(int32_t in_serial,
    RIL_Errno errorCode) {

  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse> respCb = getResponseCallback();
  if (respCb == nullptr) {
    QCRIL_LOG_DEBUG("IQtiRadioConfig: getResponseCallback Failed");
    return;
  }

  QCRIL_LOG_DEBUG("setMsimPreferenceResponse: serial=%d, error=%d",
      in_serial, errorCode);
  auto ret = respCb->setMsimPreferenceResponse(in_serial, static_cast<int32_t>(errorCode));
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.getDescription().c_str());
  }
}

/**
 * Get device secure mode status.
 * @param serial Serial number of request
 */

::ndk::ScopedAStatus IQtiRadioConfigImpl::getSecureModeStatus(int32_t in_serial) {
  QCRIL_LOG_DEBUG("IQtiRadioConfig: getSecureModeStatus: serial=%d", in_serial);
  sendResponseForGetSecureModeStatus(in_serial, RIL_E_REQUEST_NOT_SUPPORTED, false);
  return ndk::ScopedAStatus::ok();
}

/**
 * Set MSIM preference to either DSDA or DSDS
 * @param serial Serial number of request
 * @param pref Pereferece to set either DSDA or DSDS
 */
::ndk::ScopedAStatus IQtiRadioConfigImpl::setMsimPreference(int32_t in_serial,
    aidlqtiradioconfig::MsimPreference pref) {
  QCRIL_LOG_DEBUG("IQtiRadioConfig: setMsimPreference: serial=%d MsimPreference:%d",
      in_serial, pref);
    auto msg = std::make_shared<RilRequestSetMsimPreferenceMessage>(this->getContext(in_serial),
        aidlqtiradioconfig::utils::convert_msim_preference(pref));
    if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, in_serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno rilErr = RIL_E_SYSTEM_ERR;
          if(status != Message::Callback::Status::SUCCESS) {
            QCRIL_LOG_ERROR("Message::Callback::Status : %d", status);
          } else {
            rilErr = (resp == nullptr) ? RIL_E_NO_MEMORY : resp->errorCode;
          }
          sendResponseForSetMsimPreference(in_serial, rilErr);
      });
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      sendResponseForSetMsimPreference(in_serial, RIL_E_NO_MEMORY);
    }
    return ndk::ScopedAStatus::ok();
}



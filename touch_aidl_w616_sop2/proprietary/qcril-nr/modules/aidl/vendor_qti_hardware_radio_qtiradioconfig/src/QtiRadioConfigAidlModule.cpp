/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RILQ"

#include "QtiRadioConfigAidlModule.h"
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cstring>
#include <framework/Log.h>

#ifdef QMI_RIL_UTF
#include <binder/Binder.h>
#include <ibinder_internal.h>
#include "ril_utf_service_manager.h"

using android::sp;
using android::String16;
using android::status_t;
using android::ServiceManager;
#endif

static load_module<QtiRadioConfigAidlModule> sQtiRadioConfigAidlModule;

QtiRadioConfigAidlModule* getQtiRadioConfigAidlModule() {
  return &(sQtiRadioConfigAidlModule.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
QtiRadioConfigAidlModule::QtiRadioConfigAidlModule() {
  mName = "RadioQtiRadioConfigAidlModule";

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, QtiRadioConfigAidlModule::handleQcrilInit),
  };
}

/* Follow RAII.
 */
QtiRadioConfigAidlModule::~QtiRadioConfigAidlModule() {
}

/*
 * Module specific initialization that does not belong to RAII .
 */
void QtiRadioConfigAidlModule::init() {
  Module::init();
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void QtiRadioConfigAidlModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName + "]: get_instance_id = " +
                       std::to_string(msg->get_instance_id()));
  /* Register AIDL service. */
  registerAidlService(msg->get_instance_id());
}

void QtiRadioConfigAidlModule::registerAidlService(qcril_instance_id_e_type instance_id) {
    // Register Stable AIDL Interface.
  if (instance_id == QCRIL_DEFAULT_INSTANCE_ID) {
    if (mIQtiRadioConfigAidlImpl == nullptr) {
      mIQtiRadioConfigAidlImpl = ndk::SharedRefBase::make<IQtiRadioConfigImpl>(instance_id);
      const std::string instance =
          std::string(IQtiRadioConfigImpl::descriptor) + "/default";
      Log::getInstance().d("instance=" + instance);
#ifndef QMI_RIL_UTF
      binder_status_t status =
          AServiceManager_addService(mIQtiRadioConfigAidlImpl->asBinder().get(), instance.c_str());
#else
      status_t status = getServiceManager()->addService(String16(instance.c_str()),
          mIQtiRadioConfigAidlImpl->asBinder().get()->getBinder());
#endif
      QCRIL_LOG_INFO("IQtiRadioConfig Stable AIDL addService, status= %d", status);
      if (status != STATUS_OK) {
        mIQtiRadioConfigAidlImpl = nullptr;
        QCRIL_LOG_INFO("Error registering service %s %d",
                       IQtiRadioConfigImpl::descriptor,
                       instance_id);
      }
    }
  }
}



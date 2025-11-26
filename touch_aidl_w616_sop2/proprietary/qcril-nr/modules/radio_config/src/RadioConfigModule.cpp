/******************************************************************************
#  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef QMI_RIL_UTF

#include <cstring>

#include "hidl_impl/1.0/radio_config_service_1_0.h"
#include "hidl_impl/1.1/radio_config_service_1_1.h"
#include "hidl_impl/1.2/radio_config_service_1_2.h"

#include "framework/Log.h"
#include "RadioConfigModule.h"

static load_module<RadioConfigModule> sRadioConfigModule;

RadioConfigModule* getRadioConfigModule() {
  return &(sRadioConfigModule.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
RadioConfigModule::RadioConfigModule() {
  mName = "RadioConfigModule";

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, RadioConfigModule::handleQcrilInit),
    HANDLER(UimSlotStatusInd, RadioConfigModule::handleSlotStatusIndiaction),
  };
}

/* Follow RAII.
 */
RadioConfigModule::~RadioConfigModule() {
}

/*
 * Module specific initialization that does not belong to RAII .
 */
void RadioConfigModule::init() {
  Module::init();
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void RadioConfigModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName +
                       "]: get_instance_id = " + std::to_string(msg->get_instance_id()));

  if (msg->get_instance_id() == QCRIL_DEFAULT_INSTANCE_ID) {
    if (!mRadioConfigService) {
      for (auto svcImpl : getHalServiceImplFactory<RadioConfigServiceBase>()) {
        bool result = svcImpl->registerService(msg->get_instance_id());
        if (result) {
          Log::getInstance().d("[" + mName + "]: Registered!");
          mRadioConfigService = svcImpl;
          break;
        }
      }
    }
  }
}

void RadioConfigModule::handleSlotStatusIndiaction(std::shared_ptr<UimSlotStatusInd> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mRadioConfigService) {
    mRadioConfigService->sendSlotStatusIndication(msg);
  }
}

#ifdef QMI_RIL_UTF
void qcril_qmi_radio_config_service_init(int instanceId) {
  QCRIL_LOG_DEBUG("qcril_qmi_radio_config_service_init %d", instanceId);
  auto msg = std::make_shared<QcrilInitMessage>((qcril_instance_id_e_type)instanceId);
  getRadioConfigModule()->dispatchSync(msg);
}
#endif

#endif

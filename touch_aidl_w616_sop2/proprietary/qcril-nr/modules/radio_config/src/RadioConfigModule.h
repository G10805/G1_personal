/******************************************************************************
#  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef __HAL_RADIO_CONFIG_MODULE_H_
#define __HAL_RADIO_CONFIG_MODULE_H_

#include "hidl_impl/radio_config_service_base.h"
#include "framework/Module.h"
#include "framework/QcrilInitMessage.h"
#include "interfaces/uim/UimSlotStatusInd.h"

class RadioConfigModule : public Module {
 public:
  RadioConfigModule();
  ~RadioConfigModule();
  void init();

 private:
  void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
  void handleSlotStatusIndiaction(std::shared_ptr<UimSlotStatusInd> msg);

  ::android::sp<RadioConfigServiceBase> mRadioConfigService = nullptr;
};

#ifdef QMI_RIL_UTF
void qcril_qmi_radio_config_service_init(int instanceId);
#endif

#endif  // __HAL_RADIO_CONFIG_MODULE_H_

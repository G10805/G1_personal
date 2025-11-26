/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include "qti_radio_config_aidl_service.h"

class QtiRadioConfigAidlModule : public Module {
 public:
  QtiRadioConfigAidlModule();
  ~QtiRadioConfigAidlModule();
  void init();

 private:
  void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
  void registerAidlService(qcril_instance_id_e_type instance_id);
  std::shared_ptr<IQtiRadioConfigImpl> mIQtiRadioConfigAidlImpl{nullptr};
};

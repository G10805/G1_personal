/******************************************************************************
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/Log.h>
#include <framework/MessageContext.h>
#include <utils/StrongPointer.h>

class RadioConfigContext : public MessageContext {
 private:
 public:
  RadioConfigContext(qcril_instance_id_e_type instance_id, uint32_t serial)
      : MessageContext(instance_id, serial) {}

  ~RadioConfigContext() { }

  std::string toString() {
    return std::string("IRadioConfig(") + std::to_string(serial) + std::string(")");
  }
};

/******************************************************************************
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "framework/Log.h"
#include "radio_config_service_base.h"

#undef TAG
#define TAG "RILQ"

/**
 * Handler function for hidl death notification.
 * From hidl_death_recipient
 */
void RadioConfigServiceBase::serviceDied(uint64_t,
                                         const ::android::wp<::android::hidl::base::V1_0::IBase>&) {
  QCRIL_LOG_INFO("serviceDied");
  clearCallbacks();
}

  /**
   * Generate RadioConfigContext
   */
std::shared_ptr<RadioConfigContext> RadioConfigServiceBase::getContext(uint32_t serial) {
    return std::make_shared<RadioConfigContext>(mInstanceId, serial);
}

/**
 * Set instance id
 */
void RadioConfigServiceBase::setInstanceId(qcril_instance_id_e_type instId) {
  mInstanceId = instId;
}

/**
 * Returns the instance id
 */
qcril_instance_id_e_type RadioConfigServiceBase::getInstanceId() {
  return mInstanceId;
}

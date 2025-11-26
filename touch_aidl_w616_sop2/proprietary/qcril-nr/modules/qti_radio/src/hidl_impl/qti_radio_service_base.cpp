/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
 ******************************************************************************/

#include "framework/Log.h"
#include "hidl_impl/qti_radio_service_base.h"

#undef TAG
#define TAG "RILQ"

/**
 * Handler function for hidl death notification.
 * From hidl_death_recipient
 */
void QtiRadioServiceBase::serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase>&) {
  QCRIL_LOG_DEBUG("serviceDied");
  clearCallbacks();
}

// Utils APIs

/**
 * Generate QtiRadioContext
 */
std::shared_ptr<QtiRadioContext> QtiRadioServiceBase::getContext(uint32_t serial) {
  return std::make_shared<QtiRadioContext>(mInstanceId, serial);
}

/**
 * Set instance id
 */
void QtiRadioServiceBase::setInstanceId(qcril_instance_id_e_type instId) {
  mInstanceId = instId;
}

/**
 * Returns the instance id
 */
qcril_instance_id_e_type QtiRadioServiceBase::getInstanceId() {
  return mInstanceId;
}

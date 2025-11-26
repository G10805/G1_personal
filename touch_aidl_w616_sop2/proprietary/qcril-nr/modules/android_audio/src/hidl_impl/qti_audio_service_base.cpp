/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
 ******************************************************************************/

#include "framework/Log.h"
#include "hidl_impl/qti_audio_service_base.h"

#undef TAG
#define TAG "RILQ"

/**
 * Handler function for hidl death notification.
 * From hidl_death_recipient
 */
void QtiAudioServiceBase::serviceDied(uint64_t,
                                      const ::android::wp<::android::hidl::base::V1_0::IBase>&) {
  QCRIL_LOG_DEBUG("serviceDied");
  clearCallbacks();
}

// Utils APIs

/**
 * Generate QcRilAudioContext
 */
std::shared_ptr<QcRilAudioContext> QtiAudioServiceBase::getContext(uint32_t serial) {
  return std::make_shared<QcRilAudioContext>(mInstanceId, serial);
}

/**
 * Set instance id
 */
void QtiAudioServiceBase::setInstanceId(qcril_instance_id_e_type instId) {
  mInstanceId = instId;
}

/**
 * Returns the instance id
 */
qcril_instance_id_e_type QtiAudioServiceBase::getInstanceId() {
  return mInstanceId;
}

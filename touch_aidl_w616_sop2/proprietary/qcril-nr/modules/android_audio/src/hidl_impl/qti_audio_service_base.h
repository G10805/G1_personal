/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __QTI_AUDIO_SERVICE_BASE_H_
#define __QTI_AUDIO_SERVICE_BASE_H_

#include <QtiMutex.h>
#include <framework/legacy.h>
#include <hidl/HidlSupport.h>
#include <utils/String8.h>

#include "HalServiceImplFactory.h"
#include "QcRilAudioContext.h"

class QtiAudioServiceBase : public ::android::hardware::hidl_death_recipient {
 private:
  qcril_instance_id_e_type mInstanceId;

 protected:
  qtimutex::QtiSharedMutex mCallbackLock;

  /**
   * Clean up the callback handlers.
   */
  virtual void clearCallbacks() = 0;

  /**
   * Handler function for hidl death notification.
   * From hidl_death_recipient
   */
  void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase>&);

  // Utils APIs

  /**
   * Generate QtiAudioService
   */
  std::shared_ptr<QcRilAudioContext> getContext(uint32_t serial);

  /**
   * Set instance id
   */
  void setInstanceId(qcril_instance_id_e_type instId);

  /**
   * Returns the instance id
   */
  qcril_instance_id_e_type getInstanceId();

 public:
  QtiAudioServiceBase() = default;

  virtual ~QtiAudioServiceBase() = default;

  /**
   * Register the latest version of the service.
   */
  virtual bool registerService(qcril_instance_id_e_type instId) = 0;

  /**
   * getParameters
   * Invokes V1_0::IQcRilAudioCallback::getParameters
   */
  virtual ::android::String8 getParameters(::android::String8 /*param*/) {
    return ::android::String8{};
  }

  /**
   * setParameters
   * Invokes V1_0::IQcRilAudioCallback::setParameters
   */
  virtual int setParameters(::android::String8 /*param*/) {
    return 0;
  }
};

#endif  // __QTI_AUDIO_SERVICE_BASE_H_

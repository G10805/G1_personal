/******************************************************************************
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_BASE_H_
#define __RADIO_CONFIG_SERVICE_BASE_H_

#include <QtiMutex.h>
#include <framework/legacy.h>
#include <hidl/HidlSupport.h>

#include "HalServiceImplFactory.h"
#include "RadioConfigContext.h"

#include "interfaces/uim/UimSlotStatusInd.h"

class RadioConfigServiceBase : public ::android::hardware::hidl_death_recipient {
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
   * Generate RadioConfigContext
   */
  std::shared_ptr<RadioConfigContext> getContext(uint32_t serial);

  /**
   * Set instance id
   */
  void setInstanceId(qcril_instance_id_e_type instId);

  /**
   * Returns the instance id
   */
  qcril_instance_id_e_type getInstanceId();

 public:
  RadioConfigServiceBase() = default;

  virtual ~RadioConfigServiceBase() = default;

  /**
   * Register the latest version of the service.
   */
  virtual bool registerService(qcril_instance_id_e_type instId) = 0;

  // Indicaiton APIs

  /**
   * Notifies on5gStatusChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V1_0::IRadioConfigIndication::simSlotsStatusChanged
   *   V1_2::IRadioConfigIndication::simSlotsStatusChanged_1_2
   */
  virtual void sendSlotStatusIndication(const std::shared_ptr<UimSlotStatusInd> /*msg*/) {
  }
};

#endif  // __RADIO_CONFIG_SERVICE_BASE_H_

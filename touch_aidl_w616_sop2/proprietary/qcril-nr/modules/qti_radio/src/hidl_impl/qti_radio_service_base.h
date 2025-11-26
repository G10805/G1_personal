/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QCRIL_QMI_QTI_RADIO_SERVICE_BASE_H_
#define _QCRIL_QMI_QTI_RADIO_SERVICE_BASE_H_

#include <QtiMutex.h>
#include <framework/legacy.h>
#include <hidl/HidlSupport.h>

#include "HalServiceImplFactory.h"
#include "QtiRadioContext.h"

class QtiRadioServiceBase : public ::android::hardware::hidl_death_recipient {
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
   * Generate QtiRadioContext
   */
  std::shared_ptr<QtiRadioContext> getContext(uint32_t serial);

  /**
   * Set instance id
   */
  void setInstanceId(qcril_instance_id_e_type instId);

  /**
   * Returns the instance id
   */
  qcril_instance_id_e_type getInstanceId();

 public:

  QtiRadioServiceBase() = default;

  virtual ~QtiRadioServiceBase() = default;

  /**
   * Register the latest version of the service.
   */
  virtual bool registerService(qcril_instance_id_e_type instId) = 0;

  // Indicaiton APIs

  /**
   * Notifies on5gStatusChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::on5gStatusChange
   */
  virtual void notifyOn5gStatusChange(five_g_status /*status*/) {
  }

  /**
   * Notifies onNrDcParamChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onNrDcParamChange
   */
  virtual void notifyOnNrDcParamChange(five_g_endc_dcnr /*dcParam*/) {
  }

  /**
   * Notifies onNrBearerAllocationChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onNrBearerAllocationChange
   *   V2_1::IQtiRadioIndication::onNrBearerAllocationChange_2_1
   */
  virtual void notifyOnNrBearerAllocationChange(five_g_bearer_status /*bearerStatus*/) {
  }

  /**
   * Notifies onSignalStrengthChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_0::IQtiRadioIndication::onSignalStrengthChange
   */
  virtual void notifyOnSignalStrengthChange(five_g_signal_strength /*signalStrength*/) {
  }

  /**
   * Notifies onUpperLayerIndInfoChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_1::IQtiRadioIndication::onUpperLayerIndInfoChange
   */
  virtual void notifyOnUpperLayerIndInfoChange(five_g_upper_layer_ind_info /*upli_info*/) {
  }

  /**
   * Notifies on5gConfigInfoChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_1::IQtiRadioIndication::on5gConfigInfoChange
   */
  virtual void notifyOn5gConfigInfoChange(five_g_config_type /*config*/) {
  }

  /**
   * Notifies onNrIconTypeChange indication.
   * The implementation will invoke the latest version of the below the indication APIs based on
   * the version of the indication callback object set by the client.
   *   V2_2::IQtiRadioIndication::onNrIconTypeChange
   */
  virtual void notifyOnNrIconTypeChange(five_g_icon_type /*iconType*/) {
  }
};

#endif  // _QCRIL_QMI_QTI_RADIO_SERVICE_BASE_H_

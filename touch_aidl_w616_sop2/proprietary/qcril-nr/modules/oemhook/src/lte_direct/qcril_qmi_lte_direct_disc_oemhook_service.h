/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2017,2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef __QCRIL_QMI_LTE_DIRECT_DISC_OEMHOOK_SERVICE_H__
#define __QCRIL_QMI_LTE_DIRECT_DISC_OEMHOOK_SERVICE_H__

#include "hidl/HidlSupport.h"
#include "OemHookContext.h"

#include <vendor/qti/hardware/radio/qcrilhook/1.0/types.h>

#ifdef FEATURE_QCRIL_LTE_DIRECT
#include <interfaces/lte_direct/QcRilUnsolAuthorizationResultMessage.h>
#include <interfaces/lte_direct/QcRilUnsolDeviceCapabilityChangedMessage.h>
#include <interfaces/lte_direct/QcRilUnsolExpressionStatusMessage.h>
#include <interfaces/lte_direct/QcRilUnsolMatchEventMessage.h>
#include <interfaces/lte_direct/QcRilUnsolPskExpirtedMessage.h>
#include <interfaces/lte_direct/QcRilUnsolReceptionStatusMessage.h>
#include <interfaces/lte_direct/QcRilUnsolServiceStatusMessage.h>
#include <interfaces/lte_direct/QcRilUnsolTransmissionStatusMessage.h>
#include "LteDirectDiscovery.pb.h"
#endif

namespace oemhook {
namespace lte_direct {

class LteDirectImpl {
 public:
#ifndef FEATURE_QCRIL_LTE_DIRECT
  bool processLteDirectDiscRequest(int32_t /*serial*/, uint8_t* /*data*/, uint32_t /*dataLen*/) {
    return false;
  }
#else
  bool processLteDirectDiscRequest(int32_t serial, uint8_t* data, uint32_t dataLen);
  virtual void sendResponse(int32_t serial,
                            ::vendor::qti::hardware::radio::qcrilhook::V1_0::RadioError errorCode,
                            const ::android::hardware::hidl_vec<uint8_t> &respData) = 0;
  virtual void sendIndication(const ::android::hardware::hidl_vec<uint8_t> &respData) = 0;

  void lteDirectInit();

  // LTE Direct Discovery
  void lteDirectDiscoveryInitializeRequest(int32_t serial);
  void lteDirectDiscoveryGetDeviceCapabilityRequest(int32_t serial);
  void lteDirectDiscoveryTerminateRequest(int32_t serial, LteDirectDiscovery_Terminate* inData);
  void lteDirectDiscoveryGetServiceStatusRequest(int32_t serial);
  void lteDirectDiscoveryPublishRequest(int32_t serial, LteDirectDiscovery_Publish* inData);
  void lteDirectDiscoveryCancelPublishRequest(int32_t serial,
                                              LteDirectDiscovery_CancelPublish* inData);
  void lteDirectDiscoverySubscribeRequest(int32_t serial, LteDirectDiscovery_Subscribe* inData);
  void lteDirectDiscoveryCancelSubscribeRequest(int32_t serial,
                                                LteDirectDiscovery_CancelSubscribe* inData);

  void sendLteDirectUnsolAuthorizationResult(
      std::shared_ptr<QcRilUnsolAuthorizationResultMessage> msg);
  void sendLteDirectUnsolDeviceCapabilityChanged(
      std::shared_ptr<QcRilUnsolDeviceCapabilityChangedMessage> msg);
  void sendLteDirectUnsolExpressionStatus(std::shared_ptr<QcRilUnsolExpressionStatusMessage> msg);
  void sendLteDirectUnsolMatchEvent(std::shared_ptr<QcRilUnsolMatchEventMessage> msg);
  void sendLteDirectUnsolPskExpirted(std::shared_ptr<QcRilUnsolPskExpirtedMessage> msg);
  void sendLteDirectUnsolTransmissionStatus(std::shared_ptr<QcRilUnsolTransmissionStatusMessage> msg);
  void sendLteDirectUnsolReceptionStatus(std::shared_ptr<QcRilUnsolReceptionStatusMessage> msg);
  void sendLteDirectUnsolServiceStatus(std::shared_ptr<QcRilUnsolServiceStatusMessage> msg);

 protected:
  /**
   * Generate OemHookContext
   */
  virtual std::shared_ptr<OemHookContext> getContext(uint32_t serial) = 0;
#endif  // FEATURE_QCRIL_LTE_DIRECT
};

}  // namespace lte_direct
}  // namespace oemhook

#endif  // __QCRIL_QMI_LTE_DIRECT_DISC_OEMHOOK_SERVICE_H__

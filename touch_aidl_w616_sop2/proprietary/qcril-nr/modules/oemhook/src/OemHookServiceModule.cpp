/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "OemHookServiceModule.h"

#ifndef RIL_FOR_LOW_RAM
#include "UnSolMessages/SaiListChangeIndMessage.h"
#include "UnSolMessages/TMGIListChangeIndMessage.h"
#include "UnSolMessages/TMGIListOOSWarningIndMessage.h"
#include "UnSolMessages/TMGIAvailableIndMessage.h"
#include "UnSolMessages/ContentDescUpdateIndMessage.h"
#include "UnSolMessages/InterestedTMGIListIndMessage.h"
#endif
#include "qcril_legacy_apis.h"

#undef  TAG
#define TAG "RILQ"

static load_module<OemHookServiceModule> sOemHookModule;

OemHookServiceModule& getOemHookServiceModule() {
  return (sOemHookModule.get_module());
}

OemHookServiceModule::OemHookServiceModule() {
    mName = "OemHookServiceModule";

    using std::placeholders::_1;
    mMessageHandler = {
        HANDLER(QcrilInitMessage, OemHookServiceModule::handleQcrilInit),
        HANDLER(QcRilUnsolAdnRecordsOnSimMessage, OemHookServiceModule::handleAdnRecordsOnSimMessage),
        HANDLER(QcRilUnsolAdnInitDoneMessage, OemHookServiceModule::handleAdnInitDoneMessage),
        HANDLER(QcRilUnsolMbnConfigResultMessage, OemHookServiceModule::handleMbnConfigResultMessage),
        HANDLER(QcRilUnsolMbnConfigClearedMessage, OemHookServiceModule::handleMbnConfigClearedMessage),
        HANDLER(QcRilUnsolMbnValidateDumpedMessage, OemHookServiceModule::handleMbnValidateDumpedMessage),
        HANDLER(QcRilUnsolMbnConfigListMessage, OemHookServiceModule::handleMbnConfigListMessage),
        HANDLER(QcRilUnsolMbnValidateConfigMessage, OemHookServiceModule::handleMbnValidateConfigMessage),
        HANDLER(UimSimlockTempUnlockExpireInd, OemHookServiceModule::handleUimSimlockTempUnlockExpireInd),
        HANDLER(UimCardStateChangeInd, OemHookServiceModule::handleUimCardStateChangeInd),
        HANDLER(UimSlotStatusInd, OemHookServiceModule::handleUimSlotStatusInd),
        HANDLER(UimSimRefreshIndication, OemHookServiceModule::handleUimSimRefreshIndication),
        HANDLER(UimVoltageStatusInd, OemHookServiceModule::handleUimVoltageStatusInd),
        HANDLER(UimRmtRemoteSimStatusIndMsg, OemHookServiceModule::handleUimRmtRemoteSimStatusIndMsg),
        HANDLER(QcRilUnsolDtmfMessage, OemHookServiceModule::handleQcRilUnsolDtmfMessage),
        HANDLER(QcRilUnsolExtBurstIntlMessage, OemHookServiceModule::handleQcRilUnsolExtBurstIntlMessage),
        HANDLER(QcRilUnsolNssReleaseMessage, OemHookServiceModule::handleQcRilUnsolNssReleaseMessage),
        HANDLER(QcRilUnsolSuppSvcErrorCodeMessage, OemHookServiceModule::handleQcRilUnsolSuppSvcErrorCodeMessage),
        HANDLER(QcRilUnsolSpeechCodecInfoMessage, OemHookServiceModule::handleQcRilUnsolSpeechCodecInfoMessage),
        HANDLER(QcRilUnsolAudioStateChangedMessage, OemHookServiceModule::handleQcRilUnsolAudioStateChangedMessage),
        HANDLER(QcRilUnsolWmsReadyMessage, OemHookServiceModule::handleQcRilUnsolWmsReadyMessage),
#ifdef FEATURE_ENHANCED_OEMHOOK_IMPLN
        HANDLER(RilUnsolMaxActiveDataSubsChangedMessage,
                OemHookServiceModule::handleRilUnsolMaxActiveDataSubsChangedMessage),
        HANDLER(RilUnsolCsgIdChangedMessage, OemHookServiceModule::handleRilUnsolCsgIdChangedMessage),
        HANDLER(RilUnsolEngineerModeMessage, OemHookServiceModule::handleRilUnsolEngineerModeMessage),
        HANDLER(RilUnsolOemNetworkScanMessage,
                OemHookServiceModule::handleRilUnsolOemNetworkScanMessage),
        HANDLER(RilUnsolSubProvisioningStatusMessage,
                OemHookServiceModule::handleRilUnsolSubProvisionStatusMessage),
#endif

#ifdef FEATURE_QCRIL_LTE_DIRECT
        HANDLER(QcRilUnsolAuthorizationResultMessage, OemHookServiceModule::handleQcRilUnsolAuthorizationResultMessage),
        HANDLER(QcRilUnsolDeviceCapabilityChangedMessage, OemHookServiceModule::handleQcRilUnsolDeviceCapabilityChangedMessage),
        HANDLER(QcRilUnsolExpressionStatusMessage, OemHookServiceModule::handleQcRilUnsolExpressionStatusMessage),
        HANDLER(QcRilUnsolMatchEventMessage, OemHookServiceModule::handleQcRilUnsolMatchEventMessage),
        HANDLER(QcRilUnsolPskExpirtedMessage, OemHookServiceModule::handleQcRilUnsolPskExpirtedMessage),
        HANDLER(QcRilUnsolReceptionStatusMessage, OemHookServiceModule::handleQcRilUnsolReceptionStatusMessage),
        HANDLER(QcRilUnsolServiceStatusMessage, OemHookServiceModule::handleQcRilUnsolServiceStatusMessage),
        HANDLER(QcRilUnsolTransmissionStatusMessage, OemHookServiceModule::handleQcRilUnsolTransmissionStatusMessage),
#endif
#ifndef RIL_FOR_LOW_RAM
        HANDLER(rildata::SaiListChangeIndMessage, OemHookServiceModule::handleSaiListChangeIndMessage),
        HANDLER(rildata::TMGIListChangeIndMessage, OemHookServiceModule::handleTMGIListChangeIndMessage),
        HANDLER(rildata::TMGIListOOSWarningIndMessage, OemHookServiceModule::handleTMGIListOOSWarningIndMessage),
        HANDLER(rildata::TMGIAvailableIndMessage, OemHookServiceModule::handleTMGIAvailableIndMessage),
        HANDLER(rildata::ContentDescUpdateIndMessage, OemHookServiceModule::handleContentDescUpdateIndMessage),
        HANDLER(rildata::InterestedTMGIListIndMessage, OemHookServiceModule::handleInterestedTMGIListIndMessage),
#endif
    };
}

void OemHookServiceModule::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName +
                       "]: get_instance_id = " + std::to_string(msg->get_instance_id()));

  if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_OEM_SOCKET)) {
    if (!mOemHookService) {
      for (auto svcImpl : getHalServiceImplFactory<OemHookServiceBase>()) {
        bool result = svcImpl->registerService(msg->get_instance_id());
        if (result) {
          Log::getInstance().d("[" + mName + "]: Registered!");
          mOemHookService = svcImpl;
#ifdef FEATURE_QCRIL_LTE_DIRECT
          mOemHookService->lteDirectInit();
#endif
          break;
        }
      }
    }
  }
}

#ifndef RIL_FOR_LOW_RAM
void OemHookServiceModule::handleInterestedTMGIListIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendInterestedTMGIListChange(msg);
    }
}
void OemHookServiceModule::handleContentDescUpdateIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendContentDescUpdateChange(msg);
    }
}
void OemHookServiceModule::handleTMGIAvailableIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendTMGIAvailableChange(msg);
    }
}
void OemHookServiceModule::handleTMGIListOOSWarningIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendTMGIListOOSWarningChange(msg);
    }
}
void OemHookServiceModule::handleTMGIListChangeIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendTMGIListChange(msg);
    }
}
void OemHookServiceModule::handleSaiListChangeIndMessage(std::shared_ptr<Message> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendSaiListChange(msg);
    }
}
#endif

void OemHookServiceModule::handleAdnRecordsOnSimMessage(
        std::shared_ptr<QcRilUnsolAdnRecordsOnSimMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendAdnRecords(msg);
    }
}

void OemHookServiceModule::handleAdnInitDoneMessage(
        std::shared_ptr<QcRilUnsolAdnInitDoneMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendAdnInitDone(msg);
    }
}

void OemHookServiceModule::handleQcRilUnsolDtmfMessage(
        std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
      switch (msg->getDtmfEvent()) {
        case qcril::interfaces::DtmfEvent::FWD_BURST:
          mOemHookService->sendUnsolCdmaBurstDtmf(msg);
          break;
        case qcril::interfaces::DtmfEvent::FWD_START_CONT:
          mOemHookService->sendUnsolCdmaContDtmfStart(msg);
          break;
        case qcril::interfaces::DtmfEvent::FWD_STOP_CONT:
          mOemHookService->sendUnsolCdmaContDtmfStop(msg);
          break;
        case qcril::interfaces::DtmfEvent::UNKNOWN:
        default:
          break;
      }
    }
}

void OemHookServiceModule::handleQcRilUnsolExtBurstIntlMessage(
        std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolExtendedDbmIntl(msg);
    }
}

void OemHookServiceModule::handleQcRilUnsolNssReleaseMessage(
        std::shared_ptr<QcRilUnsolNssReleaseMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolNssRelease(msg);
    }
}

void OemHookServiceModule::handleQcRilUnsolSuppSvcErrorCodeMessage(
        std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolSsErrorCode(msg);
    }
}

void OemHookServiceModule::handleQcRilUnsolSpeechCodecInfoMessage(
        std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolSpeechCodecInfo(msg);
    }
}

void OemHookServiceModule::handleMbnConfigResultMessage(
        std::shared_ptr<QcRilUnsolMbnConfigResultMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendMbnConfigResult(msg);
    }
}

void OemHookServiceModule::handleMbnConfigClearedMessage(
        std::shared_ptr<QcRilUnsolMbnConfigClearedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendMbnConfigCleared(msg);
    }
}

void OemHookServiceModule::handleMbnValidateDumpedMessage(
        std::shared_ptr<QcRilUnsolMbnValidateDumpedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendMbnValidateDumped(msg);
    }
}

void OemHookServiceModule::handleMbnConfigListMessage(
        std::shared_ptr<QcRilUnsolMbnConfigListMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendMbnConfigList(msg);
    }
}

void OemHookServiceModule::handleMbnValidateConfigMessage(
        std::shared_ptr<QcRilUnsolMbnValidateConfigMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendMbnValidateConfig(msg);
    }
}

void OemHookServiceModule::handleUimSimlockTempUnlockExpireInd(
        std::shared_ptr<UimSimlockTempUnlockExpireInd> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimSimlockTempUnlockExpireInd(msg);
    }
}

void OemHookServiceModule::handleUimCardStateChangeInd(
        std::shared_ptr<UimCardStateChangeInd> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimCardStateChangeInd(msg);
    }
}

void OemHookServiceModule::handleUimSlotStatusInd(
        std::shared_ptr<UimSlotStatusInd> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimSlotStatusInd(msg);
    }
}

void OemHookServiceModule::handleUimSimRefreshIndication(
        std::shared_ptr<UimSimRefreshIndication> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimSimRefreshIndication(msg);
    }
}

void OemHookServiceModule::handleUimVoltageStatusInd(
        std::shared_ptr<UimVoltageStatusInd> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimVoltageStatusInd(msg);
    }
}

void OemHookServiceModule::handleUimRmtRemoteSimStatusIndMsg(
        std::shared_ptr<UimRmtRemoteSimStatusIndMsg> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->uimRmtRemoteSimStatusIndMsg(msg);
    }
}

#ifdef FEATURE_QCRIL_LTE_DIRECT
void OemHookServiceModule::handleQcRilUnsolAuthorizationResultMessage(
    std::shared_ptr<QcRilUnsolAuthorizationResultMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolAuthorizationResult(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolDeviceCapabilityChangedMessage(
    std::shared_ptr<QcRilUnsolDeviceCapabilityChangedMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolDeviceCapabilityChanged(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolExpressionStatusMessage(
    std::shared_ptr<QcRilUnsolExpressionStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolExpressionStatus(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolMatchEventMessage(
    std::shared_ptr<QcRilUnsolMatchEventMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolMatchEvent(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolPskExpirtedMessage(
    std::shared_ptr<QcRilUnsolPskExpirtedMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolPskExpirted(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolReceptionStatusMessage(
    std::shared_ptr<QcRilUnsolReceptionStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolReceptionStatus(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolServiceStatusMessage(
    std::shared_ptr<QcRilUnsolServiceStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolServiceStatus(msg);
  }
}

void OemHookServiceModule::handleQcRilUnsolTransmissionStatusMessage(
    std::shared_ptr<QcRilUnsolTransmissionStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendLteDirectUnsolTransmissionStatus(msg);
  }
}
#endif

void OemHookServiceModule::handleQcRilUnsolAudioStateChangedMessage(
     std::shared_ptr<QcRilUnsolAudioStateChangedMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolAudioStateChanged(msg);
    }
}

void OemHookServiceModule::handleQcRilUnsolWmsReadyMessage(
        std::shared_ptr<QcRilUnsolWmsReadyMessage> msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());

    if (mOemHookService) {
        mOemHookService->sendUnsolWmsReady(msg);
    }
}

void OemHookServiceModule::handleRilUnsolMaxActiveDataSubsChangedMessage(
    std::shared_ptr<RilUnsolMaxActiveDataSubsChangedMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendUnsolMaxActiveDataSubsChanged(msg);
  }
}

void OemHookServiceModule::handleRilUnsolCsgIdChangedMessage(
    std::shared_ptr<RilUnsolCsgIdChangedMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendUnsolCsgIdChanged(msg);
  }
}

void OemHookServiceModule::handleRilUnsolEngineerModeMessage(
    std::shared_ptr<RilUnsolEngineerModeMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendUnsolEngineerMode(msg);
  }
}

void OemHookServiceModule::handleRilUnsolOemNetworkScanMessage(
  std::shared_ptr<RilUnsolOemNetworkScanMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendUnsolNetworkScanResult(msg);
  }
}

void OemHookServiceModule::handleRilUnsolSubProvisionStatusMessage(
  std::shared_ptr<RilUnsolSubProvisioningStatusMessage> msg) {
  QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  if (mOemHookService) {
    mOemHookService->sendUnsolSubProvisionStatusChanged(msg);
  }
}

void OemHookServiceModule::sendIndication(uint8_t* buf, size_t bufLen) {
  QCRIL_LOG_INFO("sendIndication");
  if (mOemHookService) {
    mOemHookService->sendIndication(buf, bufLen);
  }
}
void OemHookServiceModule::sendResponse(int serial, RIL_Errno errorCode, uint8_t* buf,
                                        size_t bufLen) {
  QCRIL_LOG_INFO("sendResponse");
  if (mOemHookService) {
    mOemHookService->sendResponse(serial, errorCode, buf, bufLen);
  }
}

void sendOemhookIndication(qcril_instance_id_e_type /*instId*/, uint8_t* buf,
                           size_t bufLen) {
  QCRIL_LOG_INFO("sendOemhookIndication");
  getOemHookServiceModule().sendIndication(buf, bufLen);
}


void sendOemhookResponse(qcril_instance_id_e_type /*instId*/, int serial, RIL_Errno errorCode,
                         uint8_t* buf, size_t bufLen) {
  QCRIL_LOG_INFO("sendOemhookResponse");
  getOemHookServiceModule().sendResponse(serial, errorCode, buf, bufLen);
}


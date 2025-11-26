/******************************************************************************
#  Copyright (c) 2017,2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "hidl_impl/qti_oem_hook_service_base.h"

#include "qcril_qmi_oem_reqlist.h"
#include "qcril_qmi_oemhook_utils.h"
#include "qcril_qmi_oemhook_agent.h"
#include <qmi_idl_lib.h>
#include <qmi_client.h>

#include <interfaces/uim/UimVoltageStatusRequestMsg.h>
#include <interfaces/uim/UimCardPowerReqMsg.h>
#include <interfaces/uim/UimPersoRequestMsg.h>
#include <interfaces/uim/UimGetPersoStatusRequestMsg.h>
#include <interfaces/uim/UimEnterDePersoRequestMsg.h>
#include <interfaces/uim/UimGetCardStatusRequestMsg.h>
#include <interfaces/uim/UimGetAtrRequestMsg.h>
#include <interfaces/uim/UimTransmitAPDURequestMsg.h>
#include <interfaces/uim/UimSwitchSlotRequestMsg.h>
#include <interfaces/uim/UimGetPhysicalSlotMappingRequestMsg.h>
#include <interfaces/uim/UimSetAPDUBehaviorRequestMsg.h>
#include <interfaces/uim/UimGetSlotStatusRequestMsg.h>
#include <interfaces/uim/UimGetLPATermialCapabilitesRequestMsg.h>
#include <interfaces/uim/UimSetLPATermialCapabilitesRequestMsg.h>
#include <interfaces/voice/QcRilRequestRejectIncomingCallMessage.h>
#include <interfaces/voice/QcRilRequestGetCurrentSetupCallsMessage.h>
#include <interfaces/voice/QcRilRequestSetupAnswerMessage.h>
#include "interfaces/pbm/QcRilRequestGetAdnRecordMessage.h"
#include "interfaces/pbm/QcRilRequestUpdateAdnRecordMessage.h"
#include "interfaces/pbm/QcRilUnsolAdnRecordsOnSimMessage.h"
#include "interfaces/pbm/QcRilUnsolAdnInitDoneMessage.h"
#include "interfaces/nas/RilRequestStartNetworkScanMessage.h"
#include "interfaces/nas/RilRequestStopNetworkScanMessage.h"
#include "interfaces/nas/RilRequestGetLPlusLFeatureSupportStatusMessage.h"
#include "interfaces/nas/RilRequestGetCsgIdMessage.h"
#include "interfaces/nas/RilRequestSetUiStatusMessage.h"
#include "interfaces/nas/RilRequestGetPreferredNeworkBandPrefMessage.h"
#include "interfaces/nas/RilRequestSetPreferredNeworkBandPrefMessage.h"
#include "interfaces/nas/RilRequestEnableEngineerModeMessage.h"
#include "interfaces/nas/RilRequestCsgPerformNetworkScanMessage.h"
#include "interfaces/nas/RilRequestCsgSetSysSelPrefMessage.h"
#include "interfaces/dms/RilRequestGetMaxDataAllowedMessage.h"
#include "interfaces/nv/RilRequestNvReadMessage.h"
#include "interfaces/nv/RilRequestNvWriteMessage.h"
#include "interfaces/rfrpe/RilRequestSetRfmScenarioMessage.h"
#include "interfaces/rfrpe/RilRequestGetRfmScenarioMessage.h"
#include "interfaces/rfrpe/RilRequestGetProvTableRevMessage.h"
#include "interfaces/sar/RilRequestGetSarRevKeyMessage.h"
#include "interfaces/sar/RilRequestSetTransmitPowerMessage.h"

#ifndef RIL_FOR_LOW_RAM
#include "request/RilRequestEmbmsActivateTmgiMessage.h"
#include "request/RilRequestEmbmsDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsActivateDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsContentDescUpdateMessage.h"
#include "request/RilRequestEmbmsGetActiveTmgiMessage.h"
#include "request/RilRequestEmbmsGetAvailTmgiMessage.h"
#include "request/RilRequestEmbmsSendIntTmgiListMessage.h"
#include "UnSolMessages/SaiListChangeIndMessage.h"
#include "UnSolMessages/TMGIListChangeIndMessage.h"
#include "UnSolMessages/TMGIListOOSWarningIndMessage.h"
#include "UnSolMessages/TMGIAvailableIndMessage.h"
#include "UnSolMessages/ContentDescUpdateIndMessage.h"
#include "UnSolMessages/InterestedTMGIListIndMessage.h"
#endif

#include "modules/mbn/qcril_qmi_pdc.h"
#include "interfaces/dms/dms_types.h"
#include "interfaces/mbn/mbn.h"
#include "interfaces/nas/nas_types.h"
#include "qcril_memory_management.h"
#include "qcril_otheri.h"

#undef TAG
#define TAG "RILQ"

using ::android::hardware::hidl_vec;
using ::vendor::qti::hardware::radio::qcrilhook::V1_0::RadioError;

void processIncomingOemhookMessage(qcril_instance_id_e_type oemSocketInstanceId, int serial,
                                   unsigned char* data, size_t dataLen);

#define QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN 256

typedef struct {
  int index;                                        /* Connection Index for use with, eg, AT+CHLD */
  int toa;                                          /* type of address, eg 145 = intl */
  char als;                                         /* ALS line indicator if available
                                                       (0 = line 1) */
  char isVoice;                                     /* nonzero if this is is a voice call */
  char number[QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN]; /* Remote party number */
  int numberPresentation; /* 0=Allowed, 1=Restricted, 2=Not Specified/Unknown 3=Payphone */
  char name[QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN]; /* Remote party name */
  int namePresentation; /* 0=Allowed, 1=Restricted, 2=Not Specified/Unknown 3=Payphone */
} qcril_qmi_voice_setup_call_info;

typedef struct {
  boolean rejection;
} qcril_qmi_voice_setup_answer_data_type;

/* OEM HOOK DTMF forward burst payload length (72 bytes)
 * on_length   : 4 bytes
 * off_length  : 4 bytes
 * dtmf_digits: 64 bytes */
#define QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH 72

#define QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH 5
/**
 * Handler function for hidl death notification.
 * From hidl_death_recipient
 */
void OemHookServiceBase::serviceDied(uint64_t,
                                     const ::android::wp<::android::hidl::base::V1_0::IBase>&) {
  QCRIL_LOG_INFO("serviceDied");
  clearCallbacks();
}

/**
 * Generate OemHookContext
 */
std::shared_ptr<OemHookContext> OemHookServiceBase::getContext(uint32_t serial) {
  return std::make_shared<OemHookContext>(mInstanceId, serial);
}

/**
 * Set instance id
 */
void OemHookServiceBase::setInstanceId(qcril_instance_id_e_type instId) {
  mInstanceId = instId;
}

qcril_instance_id_e_type OemHookServiceBase::getInstanceId() {
  return mInstanceId;
}

//===========================================================================
// OemHookServiceBase::processOemHookRawRequest
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the oemhook request sent by client

    Oemhook Request has the following message format
    [OEMNAME (8 bytes MANDATORY)]
    [COMMAND ID (4 bytes MANDATORY)]
    [REQ DATA LENGTH (4 bytes OPTIONAL)]
    [REQ DATA (REQ DATA LENGTH bytes OPTIONAL)]

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::processOemHookRawRequest(int32_t serial, const hidl_vec<uint8_t>& data) {
  uint32_t cmd_id = 0;
  uint32_t reqLen = 0;
  uint8_t* reqData = nullptr;

  size_t dataLen = data.size();
  const uint8_t* dataPtr = data.data();
  QCRIL_LOG_INFO("processOemHookRawRequest serial=%d length=%d", serial, dataLen);

  if (!dataPtr) {
    QCRIL_LOG_ERROR("Invalid parameters; data is null");
    sendResponse(serial, RadioError::REQUEST_NOT_SUPPORTED);
    return;
  }

  // [OEMNAME (8 bytes MANDATORY)]
  if (dataLen < QCRIL_OTHER_OEM_NAME_LENGTH) {
    QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }
  dataLen -= QCRIL_OTHER_OEM_NAME_LENGTH;
  dataPtr += QCRIL_OTHER_OEM_NAME_LENGTH;

  // [COMMAND ID (4 bytes MANDATORY)]
  if (dataLen < QCRIL_OTHER_OEM_REQUEST_ID_LEN) {
    QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }
  memcpy(&cmd_id, dataPtr, QCRIL_OTHER_OEM_REQUEST_ID_LEN);
  dataLen -= QCRIL_OTHER_OEM_REQUEST_ID_LEN;
  dataPtr += QCRIL_OTHER_OEM_REQUEST_ID_LEN;

  // [REQ DATA LENGTH (4 bytes OPTIONAL)]
  if (dataLen > QCRIL_OTHER_OEM_REQUEST_DATA_LEN) {
    memcpy(&reqLen, dataPtr, QCRIL_OTHER_OEM_REQUEST_DATA_LEN);
    dataLen -= QCRIL_OTHER_OEM_REQUEST_DATA_LEN;
    dataPtr += QCRIL_OTHER_OEM_REQUEST_DATA_LEN;
    // [REQ DATA (REQ DATA LENGTH bytes OPTIONAL)]
    reqData = const_cast<uint8_t *>(dataPtr);
  }

  // Return failure if REQ DATA LENGTH is larger than the actual data length.
  if (reqLen > dataLen) {
    QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  switch (cmd_id) {
    case QCRIL_REQ_HOOK_GET_ADN_RECORD:
      getAdnRecord(serial);
      break;

    case QCRIL_REQ_HOOK_UPDATE_ADN_RECORD:
      updateAdnRecord(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SET_MODEM_TEST_MODE:
      setMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_QUERY_MODEM_TEST_MODE:
      getMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_AVAILABLE_CONFIGS:
      getAvailableMbnConfig(serial);
      break;

    case QCRIL_REQ_HOOK_CLEANUP_LOADED_CONFIGS:
      cleanupMbnConfig(serial);
      break;

    case QCRIL_REQ_HOOK_SEL_CONFIG:
      selectMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_META_INFO:
      getMetaInfo(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_DEACTIVATE_CONFIGS:
      deactivateMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_QC_VERSION_OF_FILE:
      getQcVersionOfFile(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_VALIDATE_CONFIG:
      validateMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_QC_VERSION_OF_CONFIGID:
      getQcVersionOfConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_OEM_VERSION_OF_FILE:
      getOemVersionOfFile(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_OEM_VERSION_OF_CONFIGID:
      getOemVersionOfConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_ACTIVATE_CONFIGS:
      activateMbnConfig(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_ENABLE_MODEM_UPDATE:
      enableModemUpdate(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ:
      uimGetVoltageStatus(serial);
      break;

    case QCRIL_REQ_HOOK_UICC_POWER_REQ:
    case QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ:
      uimSendCardPowerReq(serial, cmd_id, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_PERSONALIZATION_REACTIVATE_REQ:
      uimSendPersoReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_PERSONALIZATION_STATUS_REQ:
      uimGetPersoStatus(serial);
      break;

    case QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ:
      uimSendDePersoReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SLOT_GET_CARD_STATE_REQ:
      uimGetCardStateReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SLOT_GET_ATR_REQ:
      uimGetCardAtrReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SLOT_SEND_APDU_REQ:
      uimSendApduReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SWITCH_SLOT_REQ:
      uimSwitchSlotReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_PHYSICAL_SLOT_MAPPING_REQ:
      uimGetSlotMapping(serial);
      break;

    case QCRIL_REQ_HOOK_SLOT_SET_APDU_BEHAVIOR_REQ:
      uimSetApduBehaviorReq(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_SLOTS_STATUS_REQ:
      uimGetSlotStatus(serial);
      break;

    case QCRIL_REQ_HOOK_GET_TERMINAL_CAPABILITY_LPA_TLV_REQ:
      uimGetLPATC(serial);
      break;

    case QCRIL_REQ_HOOK_SET_TERMINAL_CAPABILITY_LPA_TLV_REQ:
      uimSetLPATC(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21:
      rejectIncomingCallWithCause21(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS:
      getCurrentSetupCalls(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER:
      requestSetupAnswer(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_PERFORM_INCREMENTAL_NW_SCAN:
      startIncrementalScanRequest(serial);
      break;

    case QCRIL_REQ_HOOK_CANCEL_QUERY_AVAILABLE_NETWORK:
      cancelIncrementalScanRequest(serial);
      break;

#ifdef FEATURE_ENHANCED_OEMHOOK_IMPLN
    case QCRIL_REQ_HOOK_GET_MAX_DATA_ALLOWED_REQ:
      getMaxDataAllowed(serial);
      break;

    case QCRIL_REQ_HOOK_GET_L_PLUS_L_FEATURE_SUPPORT_STATUS_REQ:
      getLPlusLFeatureSupportStatus(serial);
      break;

    case QCRIL_REQ_HOOK_SET_ATEL_UI_STATUS:
      setUiStatus(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_PREFERRED_NETWORK_BAND_PREF:
      getPreferredNetworkBandPref(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_SET_PREFERRED_NETWORK_BAND_PREF:
      setPreferredNetworkBandPref(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_CSG_ID:
      getCsgId(serial);
      break;

    case QCRIL_REQ_HOOK_ENABLE_ENGINEER_MODE:
      enableEngineerMode(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_GET_SAR_REV_KEY:
      getSarRevKey(serial);
      break;

    case QCRIL_REQ_HOOK_SET_TRANSMIT_POWER:
      setSarTransmitPower(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_CSG_PERFORM_NW_SCAN:
      performCsgNeworkScan(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_CSG_SET_SYS_SEL_PREF:
      setCsgSystemSelectionPreference(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_NV_READ:
      nvRead(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_NV_WRITE:
      nvWrite(serial, reqData, reqLen);
      break;
#endif

    case QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ:
      processLteDirectDiscRequest(serial, reqData, reqLen);
      break;

    case QCRIL_REQ_HOOK_REQ_GENERIC:
      if (genericMessageHandler(serial, reqData, reqLen)) {
        break;
      }
      // fallthrough; to process the request further if there is no specific handler
      [[fallthrough]];

    default:
      processIncomingOemhookMessage(mInstanceId, serial, const_cast<unsigned char*>(data.data()),
                                    data.size());
      break;
  }
}

//===========================================================================
// OemHookServiceBase::getAdnRecord
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_ADN_RECORD request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getAdnRecord(int32_t serial) {
  QCRIL_LOG_INFO("getAdnRecord: serial=%d", serial);
  auto msg = std::make_shared<QcRilRequestGetAdnRecordMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              qcril::interfaces::qcril_pbm_adn_count_info adnCountInfo =
                  std::static_pointer_cast<qcril::interfaces::AdnCountInfoResp>(
                      responseDataPtr->data)
                      ->getAdnCountInfo();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)&adnCountInfo, sizeof(adnCountInfo));
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

qcril::interfaces::AdnRecordInfo* constructAdnRecord(
    uint8_t* data, uint32_t data_len, qcril::interfaces::AdnRecordInfo* record_data_ptr) {
  uint16_t data_index, email_index, anr_index, email_elements, anr_elements;
  uint16_t name_length, number_length, email_length, anr_length;

  if ((NULL != data) && (1 < data_len)) {
    data_index = 0;

    if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
      return NULL;
    }

    record_data_ptr->record_id = data[data_index++];
    record_data_ptr->record_id += data[data_index++] << 8;
    QCRIL_LOG_INFO("record_id is %d", record_data_ptr->record_id);

    if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
      return NULL;
    }

    name_length = data[data_index++];
    name_length += data[data_index++] << 8;
    QCRIL_LOG_INFO("name_length is %d", name_length);

    if (name_length > 0) {
      record_data_ptr->name = std::string(static_cast<const char*>((char*)data + data_index));
      name_length++;
    }

    data_index += name_length;
    QCRIL_LOG_INFO("name is %s", record_data_ptr->name.c_str());

    if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
      return NULL;
    }
    number_length = data[data_index++];
    number_length += data[data_index++] << 8;
    QCRIL_LOG_INFO("number_length is %d", number_length);

    if (number_length > 0) {
      record_data_ptr->number = std::string(static_cast<const char*>((char*)data + data_index));
      number_length++;
    }

    data_index += number_length;
    QCRIL_LOG_INFO("number is %s", record_data_ptr->number.c_str());

    if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
      return NULL;
    }
    email_elements = data[data_index++];
    email_elements += data[data_index++] << 8;
    if(email_elements > RIL_MAX_NUM_EMAIL_COUNT) {
      QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
      return NULL;
    }
    record_data_ptr->email_elements = email_elements;
    QCRIL_LOG_INFO("email_elements is %d", email_elements);
    for (email_index = 0; email_index < email_elements; email_index++) {
      if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
        return NULL;
      }
      email_length = data[data_index++];
      email_length += data[data_index++] << 8;

      record_data_ptr->email[email_index] =
          std::string(static_cast<const char*>((char*)data + data_index));
      data_index += email_length + 1;
    }

    if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
      return NULL;
    }
    anr_elements = data[data_index++];
    anr_elements += data[data_index++] << 8;
    if(anr_elements > RIL_MAX_NUM_AD_COUNT) {
      QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
      return NULL;
    }
    record_data_ptr->anr_elements = anr_elements;
    QCRIL_LOG_INFO("anr_elements is %d", anr_elements);
    for (anr_index = 0; anr_index < anr_elements; anr_index++) {
      if (!oemhook::utils::dataLengthCheck(data_index, data_len)) {
        return NULL;
      }
      anr_length = data[data_index++];
      anr_length += data[data_index++] << 8;

      record_data_ptr->ad_number[anr_index] =
          std::string(static_cast<const char*>((char*)data + data_index));
      data_index += anr_length + 1;
    }

    return record_data_ptr;
  } else {
    QCRIL_LOG_ERROR("parameter error");
    return NULL;
  }
}

//===========================================================================
// OemHookServiceBase::updateAdnRecord
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_UPDATE_ADN_RECORD request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::updateAdnRecord(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("updateAdnRecord: serial=%d", serial);

  qcril::interfaces::AdnRecordInfo recordData;

  if (constructAdnRecord(data, dataLen, &recordData) == NULL) {
    QCRIL_LOG_ERROR("%s", " Invalid input for data \n");

    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg = std::make_shared<QcRilRequestUpdateAdnRecordMessage>(getContext(serial), recordData);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int32_t recordIndex =
                  std::static_pointer_cast<qcril::interfaces::AdnRecordUpdatedResp>(
                      responseDataPtr->data)
                      ->getRecordIndex();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)&recordIndex, sizeof(recordIndex));
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, NULL);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::setMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::setMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;
  char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];
  size_t fileLen;

  QCRIL_LOG_INFO("setMbnConfig: serial=%d", serial);

  if (NULL == data || dataLen < (1 + sizeof(qcril::interfaces::qcril_pdc_mbn_type))) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    return;
  }

  // fetch sub ID
  uint32_t subId = data[dataIndex];
  dataIndex++;

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType =
      (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
  dataIndex += sizeof(mbnType);
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  // fetch mbn file path
  fileLen = strlcpy(mbnFilePath, (const char*)(data + dataIndex),
                    std::min((int)(dataLen - dataIndex), (int)(sizeof(mbnFilePath))));
  dataIndex += fileLen + 1;

  QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

  if (fileLen >= QCRIL_MBN_FILE_PATH_LEN) {
    QCRIL_LOG_ERROR("invalid parameter: file name length too long");

    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }
  if (dataIndex >= dataLen) {
    QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data + dataIndex, data + dataLen - 1);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg = std::make_shared<QcRilRequestSetMbnConfigMessage>(
      getContext(serial), subId, mbnType, std::string(static_cast<const char*>((char*)mbnFilePath)),
      configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;

  QCRIL_LOG_INFO("getMbnConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch sub ID
  uint32_t subId = data[dataIndex];
  dataIndex++;

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType =
      (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
  dataIndex += sizeof(mbnType);
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  auto msg = std::make_shared<QcRilRequestGetMbnConfigMessage>(getContext(serial), subId, mbnType);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int32_t configLen =
                  std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)
                      ->getLength();
              std::vector<uint8_t> configId =
                  std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)
                      ->getConfig();
              QCRIL_LOG_INFO("Sending response with config ID is %s", configId.data());
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)configId.data(), configLen);
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getAvailableMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_AVAILABLE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getAvailableMbnConfig(int32_t serial) {
  QCRIL_LOG_INFO("getAvailableMbnConfig: serial=%d", serial);

  auto msg = std::make_shared<QcRilRequestGetAvlMbnConfigMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::cleanupMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_CLEANUP_LOADED_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::cleanupMbnConfig(int32_t serial) {
  QCRIL_LOG_INFO("cleanupMbnConfig: serial=%d", serial);

  auto msg = std::make_shared<QcRilRequestCleanupMbnConfigMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::selectMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_SEL_CONFIG request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::selectMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;

  QCRIL_LOG_INFO("selectMbnConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch sub ID
  uint32_t subId = data[dataIndex];
  dataIndex++;

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType =
      (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
  dataIndex += sizeof(mbnType);
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  if (dataIndex > dataLen) {
    QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg = std::make_shared<QcRilRequestSelectMbnConfigMessage>(getContext(serial), subId,
                                                                  mbnType, configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getMetaInfo
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_META_INFO request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getMetaInfo(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;

  QCRIL_LOG_INFO("getMetaInfo: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType =
      (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
  dataIndex += sizeof(mbnType);
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  if (dataIndex > dataLen) {
    QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg = std::make_shared<QcRilRequestGetMetaInfoMessage>(getContext(serial), mbnType, configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int32_t configLen =
                  std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)
                      ->getLength();
              std::vector<uint8_t> configInfo =
                  std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)
                      ->getConfig();
              QCRIL_LOG_INFO("Sending response with config info is %s", configInfo.data());
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)configInfo.data(), configLen);
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::deactivateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::deactivateMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("deactivateMbnConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType = (qcril::interfaces::qcril_pdc_mbn_type)*data;
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  auto msg = std::make_shared<QcRilRequestDeactivateMbnConfigMessage>(getContext(serial), mbnType);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getQcVersionOfFile
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getQcVersionOfFile(int32_t serial, uint8_t* data, uint32_t dataLen) {
  char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];

  QCRIL_LOG_INFO("getQcVersionOfFile: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  if ((dataLen > QCRIL_MBN_FILE_PATH_LEN) || (strlen((char*)data) >= QCRIL_MBN_FILE_PATH_LEN)) {
    QCRIL_LOG_ERROR("invalid parameter: file name length too long");

    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch mbn file path
  memset(mbnFilePath, 0, sizeof(mbnFilePath));
  strlcpy(mbnFilePath, (const char*)data, std::min((int)(dataLen + 1), (int)(sizeof(mbnFilePath))));

  QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

  auto msg =
      std::make_shared<QcRilRequestGetQcVersionOfFileMessage>(getContext(serial), mbnFilePath);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int32_t versionLen =
                  std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)
                      ->getLength();
              std::vector<uint8_t> fileVersion =
                  std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)
                      ->getVersion();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)fileVersion.data(), versionLen);
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::validateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_VALIDATE_CONFIG request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::validateMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;

  QCRIL_LOG_INFO("validateMbnConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch sub ID
  uint32_t subId = data[dataIndex];
  dataIndex++;

  if (dataIndex > dataLen) {
    QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg =
      std::make_shared<QcRilRequestValidateMbnConfigMessage>(getContext(serial), subId, configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getQcVersionOfConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getQcVersionOfConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("getQcVersionOfConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data, data + dataLen);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid parameter: config ID length too long");

    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg = std::make_shared<QcRilRequestGetQcVersionOfConfigMessage>(getContext(serial), configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int configVersion = std::static_pointer_cast<qcril::interfaces::ConfigVersionResp>(
                                      responseDataPtr->data)
                                      ->getVersion();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)&(configVersion), sizeof(configVersion));
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getOemVersionOfFile
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getOemVersionOfFile(int32_t serial, uint8_t* data, uint32_t dataLen) {
  char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];

  QCRIL_LOG_INFO("getOemVersionOfFile: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  if ((dataLen > QCRIL_MBN_FILE_PATH_LEN) || (strlen((char*)data) >= QCRIL_MBN_FILE_PATH_LEN)) {
    QCRIL_LOG_ERROR("invalid parameter: file name length too long");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch mbn file path
  memset(mbnFilePath, 0, sizeof(mbnFilePath));
  strlcpy(mbnFilePath, (char*)data, sizeof(mbnFilePath));

  QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

  auto msg =
      std::make_shared<QcRilRequestGetOemVersionOfFileMessage>(getContext(serial), mbnFilePath);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int32_t versionLen =
                  std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)
                      ->getLength();
              std::vector<uint8_t> fileVersion =
                  std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)
                      ->getVersion();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)fileVersion.data(), versionLen);
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getOemVersionOfConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getOemVersionOfConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("getOemVersionOfConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch config ID
  std::vector<uint8_t> configId(data, data + dataLen);
  configId.insert(configId.end(), '\0');
  QCRIL_LOG_INFO("config ID is %s", configId.data());

  if (configId.size() > QCRIL_MBN_CONFIG_ID_LEN) {
    QCRIL_LOG_ERROR("invalid parameter: config ID length too long");

    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  auto msg =
      std::make_shared<QcRilRequestGetOemVersionOfConfigMessage>(getContext(serial), configId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            if (responseDataPtr->data) {
              int configVersion = std::static_pointer_cast<qcril::interfaces::ConfigVersionResp>(
                                      responseDataPtr->data)
                                      ->getVersion();
              hidl_vec<uint8_t> data;
              data.setToExternal((uint8_t*)&(configVersion), sizeof(configVersion));
              sendResponse(serial, (RadioError)responseDataPtr->errorCode, data);
            } else {
              sendResponse(serial, (RadioError)responseDataPtr->errorCode);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::activateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_ACTIVATE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::activateMbnConfig(int32_t serial, uint8_t* data, uint32_t dataLen) {
  uint16_t dataIndex = 0;
  QCRIL_LOG_INFO("activateMbnConfig: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch sub ID
  uint32_t subId = data[dataIndex];
  dataIndex++;

  // fetch mbn type
  qcril::interfaces::qcril_pdc_mbn_type mbnType =
      (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
  dataIndex += sizeof(mbnType);
  QCRIL_LOG_INFO("mbn type is %d", mbnType);

  auto msg =
      std::make_shared<QcRilRequestActivateMbnConfigMessage>(getContext(serial), subId, mbnType);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::enableModemUpdate
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_ENABLE_MODEM_UPDATE request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::enableModemUpdate(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("enableModemUpdate: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  // fetch sub ID
  uint32_t subId = data[0];

  auto msg = std::make_shared<QcRilRequestEnableModemUpdateMessage>(getContext(serial), subId);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            sendResponse(serial, (RadioError)responseDataPtr->errorCode);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetVoltageStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetVoltageStatus(int32_t serial) {
  QCRIL_LOG_INFO("uimGetVoltageStatus: serial=%d", serial);

  auto msg = std::make_shared<UimVoltageStatusRequestMsg>(mInstanceId);
  if (msg) {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            sendResponse(serial, (RadioError)*responseDataPtr);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSendCardPowerReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_POWER_REQ and QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ
    request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSendCardPowerReq(int32_t serial, uint32_t cmd_id, uint8_t* data,
                                             uint32_t dataLen) {
  RIL_UIM_CardPowerState card_power = RIL_UIM_CARD_POWER_DOWN;
  uint8_t slot = mInstanceId;

  QCRIL_LOG_INFO("uimSendCardPowerReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  if (cmd_id == QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ) {
    RIL_SlotCardPower* in_ptr = (RIL_SlotCardPower*)data;

    card_power = (RIL_UIM_CardPowerState)(in_ptr->card_power);
    slot = in_ptr->slot_id;
  } else {
    card_power = (RIL_UIM_CardPowerState)*data;
  }

  auto msg = std::make_shared<UimCardPowerReqMsg>(slot, card_power);
  if (msg) {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            sendResponse(serial, (RadioError)*responseDataPtr);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSendPersoReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_PERSONALIZATION_REACTIVATE_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSendPersoReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_UIM_PersonalizationReq mPersoData = {};
  uint32_t ctrl_key_len = 0;

  QCRIL_LOG_INFO("uimSendPersoReq: serial=%d", serial);

  if (data == NULL || dataLen <= (strlen((char*)data) + 1)) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  ctrl_key_len = strlen((char*)data) + 1;

  mPersoData.controlKey = (char*)data;
  mPersoData.persoType = (RIL_UIM_PersoSubstate) * ((uint8_t*)data + ctrl_key_len);

  auto msg = std::make_shared<UimPersoRequestMsg>(mPersoData);
  if (msg) {
    GenericCallback<RIL_UIM_PersoResponse> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersoResponse> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            int retries = responseDataPtr->no_of_retries;
            hidl_vec<uint8_t> data = {};

            data.setToExternal((uint8_t*)&retries, sizeof(retries), false);

            sendResponse(serial, (RadioError)responseDataPtr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetPersoStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_PERSONALIZATION_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetPersoStatus(int32_t serial) {
  QCRIL_LOG_INFO("uimGetPersoStatus: serial=%d", serial);

  auto msg = std::make_shared<UimGetPersoStatusRequestMsg>();
  if (msg) {
    GenericCallback<RIL_UIM_PersonalizationStatusResp> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersonalizationStatusResp> rsp_ptr) -> void {
          if (solicitedMsg && rsp_ptr && status == Message::Callback::Status::SUCCESS) {
            RIL_UIM_PersonalizationStatusResp rsp = {};
            hidl_vec<uint8_t> data = {};

            if (rsp_ptr->has_gwNWPersoStatus) {
              rsp.has_gwNWPersoStatus = TRUE;
              rsp.gwNWPersoStatus.verifyAttempts = rsp_ptr->gwNWPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwNWSubsetPersoStatus) {
              rsp.has_gwNWSubsetPersoStatus = TRUE;
              rsp.gwNWSubsetPersoStatus.verifyAttempts =
                  rsp_ptr->gwNWSubsetPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwSPPersoStatus) {
              rsp.has_gwSPPersoStatus = TRUE;
              rsp.gwSPPersoStatus.verifyAttempts = rsp_ptr->gwSPPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwCPPersoStatus) {
              rsp.has_gwCPPersoStatus = TRUE;
              rsp.gwCPPersoStatus.verifyAttempts = rsp_ptr->gwCPPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwSPNPersoStatus) {
              rsp.has_gwSPNPersoStatus = TRUE;
              rsp.gwSPNPersoStatus.verifyAttempts = rsp_ptr->gwSPNPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwSPEhplmnPersoStatus) {
              rsp.has_gwSPEhplmnPersoStatus = TRUE;
              rsp.gwSPEhplmnPersoStatus.verifyAttempts =
                  rsp_ptr->gwSPEhplmnPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwICCIDPersoStatus) {
              rsp.has_gwICCIDPersoStatus = TRUE;
              rsp.gwICCIDPersoStatus.verifyAttempts = rsp_ptr->gwICCIDPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwIMPIPersoStatus) {
              rsp.has_gwIMPIPersoStatus = TRUE;
              rsp.gwIMPIPersoStatus.verifyAttempts = rsp_ptr->gwIMPIPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwSIMPersoStatus) {
              rsp.has_gwSIMPersoStatus = TRUE;
              rsp.gwSIMPersoStatus.verifyAttempts = rsp_ptr->gwSIMPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gwNSSPPersoStatus) {
              rsp.has_gwNSSPPersoStatus = TRUE;
              rsp.gwNSSPPersoStatus.verifyAttempts = rsp_ptr->gwNSSPPersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gw1xNWType1PersoStatus) {
              rsp.has_gw1xNWType1PersoStatus = TRUE;
              rsp.gw1xNWType1PersoStatus.verifyAttempts =
                  rsp_ptr->gw1xNWType1PersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gw1xNWType2PersoStatus) {
              rsp.has_gw1xNWType2PersoStatus = TRUE;
              rsp.gw1xNWType2PersoStatus.verifyAttempts =
                  rsp_ptr->gw1xNWType2PersoStatus.verifyAttempts;
            } else if (rsp_ptr->has_gw1xRUIMPersoStatus) {
              rsp.has_gw1xRUIMPersoStatus = TRUE;
              rsp.gw1xRUIMPersoStatus.verifyAttempts = rsp_ptr->gw1xRUIMPersoStatus.verifyAttempts;
            }

            data.setToExternal((uint8_t*)&rsp, sizeof(rsp), false);

            sendResponse(serial, (RadioError)rsp_ptr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSendDePersoReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSendDePersoReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_UIM_PersoSubstate perso_state = {};
  uint32_t code_len = 0;

  QCRIL_LOG_INFO("uimSendDePersoReq: serial=%d", serial);

  if (data == NULL || dataLen <= (strlen((char*)data) + 1)) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  code_len = strlen((char*)data) + 1;

  perso_state = (RIL_UIM_PersoSubstate)(atoi((char*)data));
  data += code_len;

  if (data == NULL || dataLen <= (strlen((char*)data) + 1)) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  auto msg = std::make_shared<UimEnterDePersoRequestMsg>((char*)data, perso_state);
  if (msg) {
    GenericCallback<RIL_UIM_PersoResponse> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersoResponse> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            int retries = responseDataPtr->no_of_retries;
            hidl_vec<uint8_t> data = {};

            data.setToExternal((uint8_t*)&retries, sizeof(retries), false);

            sendResponse(serial, (RadioError)responseDataPtr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetCardStateReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_GET_CARD_STATE_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetCardStateReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("uimGetCardStateReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  auto msg = std::make_shared<UimGetCardStatusRequestMsg>((uint8_t)(*((int*)data)));
  if (msg) {
    GenericCallback<RIL_UIM_CardStatus> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_CardStatus> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            RIL_UIM_CardState card_state = responseDataPtr->card_state;
            hidl_vec<uint8_t> data = {};

            data.setToExternal((uint8_t*)&card_state, sizeof(card_state), false);

            sendResponse(serial, (RadioError)responseDataPtr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetCardAtrReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_GET_ATR_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetCardAtrReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("uimGetCardAtrReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  auto msg = std::make_shared<UimGetAtrRequestMsg>((uint8_t)(*((int*)data)));
  if (msg) {
    GenericCallback<UimAtrRspParam> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<UimAtrRspParam> rsp_ptr) -> void {
          if (solicitedMsg && rsp_ptr && status == Message::Callback::Status::SUCCESS) {
            hidl_vec<uint8_t> data = {};

            data.setToExternal((uint8_t*)(rsp_ptr->atr.data()), rsp_ptr->atr.length(), false);

            sendResponse(serial, (RadioError)rsp_ptr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSendApduReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_SEND_APDU_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSendApduReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_SlotStreamApduReq* apdu_ptr = NULL;
  RIL_UIM_SIM_APDU apdu_data = {};

  QCRIL_LOG_INFO("uimSendApduReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  apdu_ptr = (RIL_SlotStreamApduReq*)data;

  apdu_data.sessionid = apdu_ptr->apdu_data.sessionid;
  apdu_data.cla = apdu_ptr->apdu_data.cla;
  apdu_data.instruction = apdu_ptr->apdu_data.instruction;
  apdu_data.p1 = apdu_ptr->apdu_data.p1;
  apdu_data.p2 = apdu_ptr->apdu_data.p2;
  apdu_data.p3 = apdu_ptr->apdu_data.p3;
  apdu_data.data = apdu_ptr->apdu_data.data;

  auto msg = std::make_shared<UimTransmitAPDURequestMsg>(apdu_ptr->slot_id, false, false, apdu_data);
  if (msg) {
    GenericCallback<RIL_UIM_SIM_IO_Response> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr) -> void {
          if (solicitedMsg && rsp_ptr && status == Message::Callback::Status::SUCCESS) {
            RIL_SlotSendApduResp apdu_rsp = {};
            hidl_vec<uint8_t> data = {};

            apdu_rsp.slot_id = rsp_ptr->slot_id;
            apdu_rsp.ril_err = (RIL_Errno)rsp_ptr->err;
            apdu_rsp.sw1 = rsp_ptr->sw1;
            apdu_rsp.sw2 = rsp_ptr->sw2;
            if (rsp_ptr->simResponse.length() < sizeof(apdu_rsp.simResponse)) {
              rsp_ptr->simResponse.copy(apdu_rsp.simResponse, rsp_ptr->simResponse.length());
            }
            data.setToExternal((uint8_t*)&apdu_rsp, sizeof(apdu_rsp), false);

            sendResponse(serial, (RadioError)rsp_ptr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSwitchSlotReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SWITCH_SLOT_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSwitchSlotReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_PhySlotMap* slot_map_ptr = NULL;
  std::vector<uint32_t> mSlotMap = {};

  QCRIL_LOG_INFO("uimSwitchSlotReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  slot_map_ptr = (RIL_PhySlotMap*)data;
  if (slot_map_ptr->array_len > UIM_OEM_HOOK_MAX_CARD_COUNT) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    QCRIL_LOG_FUNC_RETURN();
    return;
  }
  mSlotMap.assign(slot_map_ptr->slot_map, slot_map_ptr->slot_map + slot_map_ptr->array_len);
  auto msg = std::make_shared<UimSwitchSlotRequestMsg>(mSlotMap);
  if (msg) {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            sendResponse(serial, (RadioError)*responseDataPtr);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetSlotMapping
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_PHYSICAL_SLOT_MAPPING_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetSlotMapping(int32_t serial) {
  QCRIL_LOG_INFO("uimGetSlotMapping: serial=%d", serial);

  auto msg = std::make_shared<UimGetPhysicalSlotMappingRequestMsg>();
  if (msg) {
    GenericCallback<RIL_UIM_PhySlotMap> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PhySlotMap> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            RIL_PhySlotMap slot_map = {};
            hidl_vec<uint8_t> data = {};

            if (responseDataPtr->slotMap.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT) {
              slot_map.array_len = responseDataPtr->slotMap.size();
              for (uint32_t i = 0; i < slot_map.array_len; i++) {
                slot_map.slot_map[i] = responseDataPtr->slotMap[i];
              }
            }

            data.setToExternal((uint8_t*)&slot_map, sizeof(slot_map), false);
            sendResponse(serial, (RadioError)responseDataPtr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSetApduBehaviorReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_SET_APDU_BEHAVIOR_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSetApduBehaviorReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_SetAPDUBehavior* data_ptr = NULL;
  RIL_UIM_SetAPDUBehavior apdu_behav = {};

  QCRIL_LOG_INFO("uimSetApduBehaviorReq: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  data_ptr = (RIL_SetAPDUBehavior*)data;
  apdu_behav.channel_id = data_ptr->channel_id;
  apdu_behav.apdu_behavior = (RIL_UIM_APDU_Behavior)data_ptr->apdu_behavior;

  auto msg = std::make_shared<UimSetAPDUBehaviorRequestMsg>(mInstanceId, apdu_behav);
  if (msg) {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            sendResponse(serial, (RadioError)*responseDataPtr);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetSlotStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_SLOTS_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetSlotStatus(int32_t serial) {
  QCRIL_LOG_INFO("uimGetSlotStatus: serial=%d", serial);

  auto msg = std::make_shared<UimGetSlotStatusRequestMsg>();
  if (msg) {
    GenericCallback<RIL_UIM_SlotsStatusInfo> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_SlotsStatusInfo> resp_ptr) -> void {
          if (solicitedMsg && resp_ptr && status == Message::Callback::Status::SUCCESS) {
            RIL_SlotsStatus status = {};
            hidl_vec<uint8_t> data = {};

            if (resp_ptr->slot_status.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT) {
              status.no_of_slots = resp_ptr->slot_status.size();

              for (uint8_t i = 0; i < status.no_of_slots && i < UIM_OEM_HOOK_MAX_CARD_COUNT; i++) {
                status.slot_status[i].card_state =
                    (Ril_CardPresenceState)resp_ptr->slot_status[i].card_state;
                status.slot_status[i].slot_state =
                    (Ril_SlotState)resp_ptr->slot_status[i].slot_state;
                status.slot_status[i].logical_slot = (uint32_t)resp_ptr->slot_status[i].logical_slot;

                if (resp_ptr->slot_status[i].iccid.size() <= UIM_OEM_HOOK_MAX_ICCID_LEN) {
                  status.slot_status[i].iccid_len = resp_ptr->slot_status[i].iccid.size();
                  memcpy(status.slot_status[i].iccid, resp_ptr->slot_status[i].iccid.data(),
                         status.slot_status[i].iccid_len);
                }
              }
            }
            data.setToExternal((uint8_t*)&status, sizeof(status), false);
            sendResponse(serial, (RadioError)resp_ptr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimGetLPATC
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_TERMINAL_CAPABILITY_LPA_TLV_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimGetLPATC(int32_t serial) {
  QCRIL_LOG_INFO("uimGetLPATC: serial=%d", serial);

  auto msg = std::make_shared<UimGetLPATermialCapabilitesRequestMsg>(mInstanceId);
  if (msg) {
    GenericCallback<RIL_UIM_TerminalCapabilityLPATLV> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_TerminalCapabilityLPATLV> resp_ptr) -> void {
          if (solicitedMsg && resp_ptr && status == Message::Callback::Status::SUCCESS) {
            RIL_TerminalCapabilityLPATLV rsp = {};
            hidl_vec<uint8_t> data = {};

            rsp.tlv_present = resp_ptr->tlv_present;
            rsp.value = resp_ptr->value;

            data.setToExternal((uint8_t*)&rsp, sizeof(rsp), false);
            sendResponse(serial, (RadioError)resp_ptr->err, data);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::uimSetLPATC
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::uimSetLPATC(int32_t serial, uint8_t* data, uint32_t dataLen) {
  RIL_TerminalCapabilityLPATLV* data_ptr = NULL;

  QCRIL_LOG_INFO("uimSetLPATC: serial=%d", serial);

  if (data == NULL || dataLen == 0) {
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);

    QCRIL_LOG_FUNC_RETURN();
    return;
  }

  data_ptr = (RIL_TerminalCapabilityLPATLV*)data;

  auto msg = std::make_shared<UimSetLPATermialCapabilitesRequestMsg>(
      mInstanceId, data_ptr->tlv_present, data_ptr->value);
  if (msg) {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void {
          if (solicitedMsg && responseDataPtr && status == Message::Callback::Status::SUCCESS) {
            sendResponse(serial, (RadioError)*responseDataPtr);
          } else if (solicitedMsg) {
            sendResponse(serial, RadioError::INTERNAL_ERR);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::rejectIncomingCallWithCause21
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21 request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::rejectIncomingCallWithCause21(int32_t serial, uint8_t* /*data*/,
                                                       uint32_t /*dataLen*/) {
  QCRIL_LOG_INFO("rejectIncomingCallWithCause21: serial=%d", serial);

  auto msg = std::make_shared<QcRilRequestRejectIncomingCallMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::getCurrentSetupCalls
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::getCurrentSetupCalls(int32_t serial, uint8_t* /*data*/,
                                              uint32_t /*dataLen*/) {
  QCRIL_LOG_INFO("getCurrentSetupCalls: serial=%d", serial);

  auto msg = std::make_shared<QcRilRequestGetCurrentSetupCallsMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto rilCurrentCalls =
                std::static_pointer_cast<qcril::interfaces::GetCurrentSetupCallsRespData>(
                    resp->data);
            if (rilCurrentCalls) {
              qcril_qmi_voice_setup_call_info callInfo = {};
              if (rilCurrentCalls->hasIndex()) {
                callInfo.index = rilCurrentCalls->getIndex();
              }
              if (rilCurrentCalls->hasToa()) {
                callInfo.toa = rilCurrentCalls->getToa();
              }
              if (rilCurrentCalls->hasAls()) {
                callInfo.als = rilCurrentCalls->getAls();
              }
              if (rilCurrentCalls->hasIsVoice()) {
                callInfo.isVoice = rilCurrentCalls->getIsVoice();
              }
              if (rilCurrentCalls->hasNumber() && !rilCurrentCalls->getNumber().empty()) {
                strlcpy(callInfo.number, (const char*)rilCurrentCalls->getNumber().c_str(),
                        sizeof(callInfo.number));
              }
              if (rilCurrentCalls->hasNumberPresentation()) {
                callInfo.numberPresentation = rilCurrentCalls->getNumberPresentation();
              }
              if (rilCurrentCalls->hasName() && !rilCurrentCalls->getName().empty()) {
                strlcpy(callInfo.name, (const char*)rilCurrentCalls->getName().c_str(),
                        sizeof(callInfo.name));
              }
              if (rilCurrentCalls->hasNamePresentation()) {
                callInfo.namePresentation = rilCurrentCalls->getNamePresentation();
              }

              size_t send_buffer_len = sizeof(callInfo);
              unsigned char* send_buffer = new unsigned char[send_buffer_len];
              if (send_buffer) {
                memcpy(send_buffer, &callInfo, send_buffer_len);
              }
              data.setToExternal((uint8_t*)send_buffer, send_buffer_len,
                                 true); /* Send true to allow hidl_vec to delete memory*/
            }
          }
          sendResponse(serial, (RadioError)errorCode, data);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// OemHookServiceBase::requestSetupAnswer
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER request sent by client

    @return
    None
*/
/*=========================================================================*/
void OemHookServiceBase::requestSetupAnswer(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("requestSetupAnswer: serial=%d", serial);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  qcril_qmi_voice_setup_answer_data_type* setupAnswerReq =
      (qcril_qmi_voice_setup_answer_data_type*)data;

  auto msg = std::make_shared<QcRilRequestSetupAnswerMessage>(getContext(serial));
  if (msg) {
    msg->setRejectSetup(setupAnswerReq->rejection ? true : false);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::startIncrementalScanRequest(int32_t serial) {
  QCRIL_LOG_INFO("startIncrementalScanRequest: serial %d ", serial);

  auto msg = std::make_shared<RilRequestStartNetworkScanMessage>(getContext(serial));
  msg->setRequestSource(RilRequestStartNetworkScanMessage::RequestSource::OEM);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::cancelIncrementalScanRequest(int32_t serial) {
  QCRIL_LOG_INFO("cancelIncrementalScanRequest: serial %d ", serial);

  auto msg = std::make_shared<RilRequestStopNetworkScanMessage>(getContext(serial));

  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getMaxDataAllowed(int32_t serial) {
  QCRIL_LOG_INFO("getMaxDataAllowed: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetMaxDataAllowedMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          uint8_t maxDataAllowed = 1;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::MaxDataAllowedResult_t>(resp->data);
            if (respData) {
              maxDataAllowed = respData->maxDataSubscriptions;
            }
          }
          data.setToExternal((uint8_t*)&maxDataAllowed, sizeof(maxDataAllowed), false);
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getLPlusLFeatureSupportStatus(int32_t serial) {
  QCRIL_LOG_INFO("getLPlusLFeatureSupportStatus: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetLPlusLFeatureSupportStatusMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          uint8_t lPlusLStatus = FALSE;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::RilLPlusLFeatureSupportStatus_t>(
                    resp->data);
            if (respData) {
              if (respData->mSupported) {
                lPlusLStatus = TRUE;
              }
            }
          }
          data.setToExternal((uint8_t*)&lPlusLStatus, sizeof(lPlusLStatus), false);
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::setUiStatus(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("setUiStatus: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    if (!data || !dataLen) {
      QCRIL_LOG_ERROR("invalid parameter: data is empty");
      errorCode = RIL_E_INVALID_ARGUMENTS;
      break;
    }
    uint8_t uiReady = 0;
    if (dataLen >= sizeof(uiReady)) {
      memcpy(&uiReady, data, sizeof(uiReady));
    }
    auto msg =
        std::make_shared<RilRequestSetUiStatusMessage>(getContext(serial), uiReady);
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getPreferredNetworkBandPref(int32_t serial, uint8_t* data,
                                                     uint32_t dataLen) {
  QCRIL_LOG_INFO("getPreferredNetworkBandPref: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    if (!data || !dataLen) {
      QCRIL_LOG_ERROR("invalid parameter: data is empty");
      errorCode = RIL_E_INVALID_ARGUMENTS;
      break;
    }
    uint32_t ratBandType = 0;
    if (dataLen >= 4) {
      memcpy(&ratBandType, data, 4);
    }
    auto msg = std::make_shared<RilRequestGetPreferredNeworkBandPrefMessage>(
        getContext(serial), static_cast<qcril::interfaces::RilRatBandType>(ratBandType));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          uint32_t ratBandMap = 0;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::GetPreferredNeworkBandPrefResult_t>(
                    resp->data);
            if (respData) {
              ratBandMap = static_cast<uint32_t>(respData->bandPrefMap);
            }
          }
          data.setToExternal((uint8_t*)&ratBandMap, sizeof(ratBandMap), false);
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::setPreferredNetworkBandPref(int32_t serial, uint8_t* data,
                                                     uint32_t dataLen) {
  QCRIL_LOG_INFO("setPreferredNetworkBandPref: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    if (!data || !dataLen) {
      QCRIL_LOG_ERROR("invalid parameter: data is empty");
      errorCode = RIL_E_INVALID_ARGUMENTS;
      break;
    }
    uint32_t bandPrefMap = 0;
    if (dataLen >= 4) {
      memcpy(&bandPrefMap, data, 4);
    }
    auto msg = std::make_shared<RilRequestSetPreferredNeworkBandPrefMessage>(
        getContext(serial), static_cast<qcril::interfaces::RilBandPrefType>(bandPrefMap));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::enableEngineerMode(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("enableEngineerMode: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    if (!data || !dataLen) {
      QCRIL_LOG_ERROR("invalid parameter: data is empty");
      errorCode = RIL_E_INVALID_ARGUMENTS;
      break;
    }
    auto msg = std::make_shared<RilRequestEnableEngineerModeMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getSarRevKey(int32_t serial) {
  QCRIL_LOG_INFO("getSarRevKey: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetSarRevKeyMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          uint32_t key = 0;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::GetSarRevKeyResult>(resp->data);
            if (respData) {
              key = respData->key;
            }
          }
          data.setToExternal((uint8_t*)&key, sizeof(key), false);
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::setSarTransmitPower(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("setSarTransmitPower: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    if (!data || !dataLen) {
      QCRIL_LOG_ERROR("invalid parameter: data is empty");
      errorCode = RIL_E_INVALID_ARGUMENTS;
      break;
    }
    auto msg = std::make_shared<RilRequestSetTransmitPowerMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getCsgId(int32_t serial) {
  QCRIL_LOG_INFO("getCsgId: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetCsgIdMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          uint32_t csgId = 0xFFFFFFFF;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::GetCsgIdResult_t>(resp->data);
            if (respData) {
              csgId = respData->mCsgId;
            }
          }
          data.setToExternal((uint8_t*)&csgId, sizeof(csgId), false);
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::performCsgNeworkScan(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("performCsgNeworkScan: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    auto msg = std::make_shared<RilRequestCsgPerformNetworkScanMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            auto respData =
                std::static_pointer_cast<qcril::interfaces::CsgPerformNwScanResult_t>(resp->data);
            if (respData) {
              data = respData->toTlv();
            }
          }
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}
void OemHookServiceBase::setCsgSystemSelectionPreference(int32_t serial, uint8_t* data,
                                                         uint32_t dataLen) {
  QCRIL_LOG_INFO("setCsgSystemSelectionPreference: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    auto msg = std::make_shared<RilRequestCsgSetSysSelPrefMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::nvRead(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("nvRead: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    auto msg = std::make_shared<RilRequestNvReadMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            auto respData = std::static_pointer_cast<qcril::interfaces::NvReadResult_t>(resp->data);
            if (respData) {
              if (respData->nvItemValueSize) {
                data.setToExternal((uint8_t*)&(respData->nvItemValue), respData->nvItemValueSize,
                                   false);
              }
            }
          }
          sendResponse(serial, (RadioError)errorCode, data);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::nvWrite(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("nvWrite: serial %d ", serial);
  RIL_Errno errorCode = RIL_E_SUCCESS;
  do {
    auto msg = std::make_shared<RilRequestNvWriteMessage>(getContext(serial));
    if (!msg) {
      errorCode = RIL_E_NO_MEMORY;
      break;
    }
    errorCode = msg->setData(data, dataLen);
    if (errorCode != RIL_E_SUCCESS) {
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
          }
          sendResponse(serial, (RadioError)errorCode);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } while (0);

  if (errorCode != RIL_E_SUCCESS) {
    sendResponse(serial, (RadioError)errorCode);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::setRfmScenarioReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("setRfmScenarioReq: serial %d ", serial);
  if (data && dataLen) {
    Qtuner_set_scenario_req_v01* req = (Qtuner_set_scenario_req_v01*)data;
    auto msg = std::make_shared<RilRequestSetRfmScenarioMessage>(getContext(serial), *req);
    if (msg) {
      GenericCallback<QcRilRequestMessageCallbackPayload> cb(
          ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
            if (status == Message::Callback::Status::SUCCESS && resp) {
              errorCode = resp->errorCode;
            }
            hidl_vec<uint8_t> data;
            uint8_t* encodedResp = nullptr;
            uint32_t encodedRespLen = 0;
            if (oemhook::utils::encodeGenericResponsePayLoad(
                    nullptr, 0, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER,
                    QMI_Qtuner_SET_RFM_SCENARIO_RESP_V01, errorCode, encodedResp, encodedRespLen)) {
              errorCode = RIL_E_SUCCESS;
              data.setToExternal(encodedResp, encodedRespLen, false);
            }
            sendResponse(serial, (RadioError)errorCode, data);
            if (encodedResp) {
              free(encodedResp);
            }
          }));
      msg->setCallback(&cb);
      msg->dispatch();
    } else {
      sendResponse(serial, RadioError::NO_MEMORY);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getRfmScenarioReq(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("getRfmScenarioReq: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetRfmScenarioMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::GetRfmScenarioResult> respData;
          uint8_t* rawResp = nullptr;
          uint32_t rawRespLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            respData = std::static_pointer_cast<qcril::interfaces::GetRfmScenarioResult>(resp->data);
            if (respData) {
              rawResp = (uint8_t*)&(respData->response);
              rawRespLen = sizeof(respData->response);
            }
          }
          hidl_vec<uint8_t> data;
          uint8_t* encodedResp = nullptr;
          uint32_t encodedRespLen = 0;
          if (oemhook::utils::encodeGenericResponsePayLoad(
                  rawResp, rawRespLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER,
                  QMI_Qtuner_GET_RFM_SCENARIO_RESP_V01, errorCode, encodedResp, encodedRespLen)) {
            errorCode = RIL_E_SUCCESS;
            data.setToExternal(encodedResp, encodedRespLen, false);
          }
          sendResponse(serial, (RadioError)errorCode, data);
          if (encodedResp) {
            free(encodedResp);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::getProvisionedTableRevisionReq(int32_t serial, uint8_t* data,
                                                        uint32_t dataLen) {
  QCRIL_LOG_INFO("getProvisionedTableRevisionReq: serial %d ", serial);
  auto msg = std::make_shared<RilRequestGetProvTableRevMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::GetProvTableRevResult> respData;
          uint8_t* rawResp = nullptr;
          uint32_t rawRespLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp) {
            errorCode = resp->errorCode;
            respData =
                std::static_pointer_cast<qcril::interfaces::GetProvTableRevResult>(resp->data);
            if (respData) {
              rawResp = (uint8_t*)&(respData->response);
              rawRespLen = sizeof(respData->response);
            }
          }
          hidl_vec<uint8_t> data;
          uint8_t* encodedResp = nullptr;
          uint32_t encodedRespLen = 0;
          if (oemhook::utils::encodeGenericResponsePayLoad(
                  rawResp, rawRespLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER,
                  QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_RESP_V01, errorCode, encodedResp,
                  encodedRespLen)) {
            errorCode = RIL_E_SUCCESS;
            data.setToExternal(encodedResp, encodedRespLen, false);
          }
          sendResponse(serial, (RadioError)errorCode, data);
          if (encodedResp) {
            free(encodedResp);
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}

bool encodeGenericResponsePayLoad(uint8_t*& rawData, uint32_t& rawLen, uint16_t service_id,
                                  uint16_t message_id) {
  /* Response oemhook has following message format
     [Message Id (4) ] [Payload Length (4) ] [Payload]
     In case of EMBMS|VT|Presence OemHook Service
     Payload :
     [SRV_ID(2)][MSG_ID(2)][Error Code(2)][Actual Payload]
   */
  QCRIL_LOG_DEBUG("Generic Response Message Encode");
  if (rawData == nullptr || rawLen < 1) {
    QCRIL_LOG_DEBUG("Invalid Message received for encoding");
    return false;
  }
  auto srv_obj = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object(
      (qmi_ril_oem_hook_qmi_tunneling_service_id_type)service_id);
  if (srv_obj) {
    uint32_t tlv_stream_len = 0;
    auto idl_err =
        qmi_idl_get_max_message_len(srv_obj, QMI_IDL_RESPONSE, message_id, &tlv_stream_len);
    if (QMI_NO_ERR == idl_err) {
      QCRIL_LOG_DEBUG("max length = %d, msg_id = %d", tlv_stream_len, message_id);
      char* payload =
          (char*)qcril_malloc(tlv_stream_len + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE);
      if (payload) {
        uint32_t encoded_fact = QMI_RIL_ZERO;
        idl_err = qmi_idl_message_encode(srv_obj, QMI_IDL_RESPONSE, message_id, rawData, rawLen,
                                         payload + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE,
                                         tlv_stream_len, &encoded_fact);

        if (QMI_NO_ERR == idl_err) {
          /* for VT/PRESENCE service, skip the result part, as RIL<-->Telphony interface does not
           * expect result field */
          if (service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT ||
              service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE) {
            payload += OEM_HOOK_QMI_TUNNELING_RESULT_SIZE;
            encoded_fact -= OEM_HOOK_QMI_TUNNELING_RESULT_SIZE;
          }
          // complete the oem hook tunneling header
          char* ptr = payload;
          // request_id
          uint32_t request_id = QCRIL_REQ_HOOK_REQ_GENERIC;
          memcpy(payload, &request_id, OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE);
          ptr += OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE;
          // payload_size
          uint32_t payload_size = encoded_fact + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +
                                  OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE +
                                  OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE;
          memcpy(ptr, &payload_size, OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE);
          ptr += OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE;
          // service_id
          memcpy(ptr, &service_id, OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE);
          ptr += OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE;
          // message_id
          memcpy(ptr, &message_id, OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE);
          ptr += OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;
          // error_code
          memcpy(ptr, &idl_err, OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE);
          rawData = (uint8_t*)payload;
          rawLen = encoded_fact + OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE;
        } else {
          QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
          qcril_free(payload);
          return false;
        }
      } else {
        QCRIL_LOG_DEBUG("Memory Allocation Failed");
        return false;
      }
    } else {
      QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
      return false;
    }
  } else {
    QCRIL_LOG_DEBUG("Could not create required srv obj");
    return false;
  }
  return true;
}

bool encodeGenericUnsolPayload(uint8_t*& rawData, uint32_t& rawLen, uint16_t service_id,
                               uint16_t message_id) {
  /* Unsolicited oemhook message has following message format
     [RIL_UNSOL_OEM_HOOK_RAW (4)]
     [OEM_NAME(8)] [Message Id (4) ] [Payload Length (4) ] [Payload]
     In case of EMBMS|VT|Presence OemHook Service
     Payload :
     [SRV_ID(2)][MSG_ID(2)][Actual Payload]
   */
  QCRIL_LOG_DEBUG("Generic Unsol Message Encode");
  if (rawData == nullptr || rawLen < 1) {
    QCRIL_LOG_DEBUG("Invalid Message received for encoding");
    return false;
  }
  auto srv_obj = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object(
      (qmi_ril_oem_hook_qmi_tunneling_service_id_type)service_id);
  if (srv_obj) {
    uint32_t tlv_stream_len = 0;
    auto idl_err =
        qmi_idl_get_max_message_len(srv_obj, QMI_IDL_INDICATION, message_id, &tlv_stream_len);
    if (QMI_NO_ERR == idl_err) {
      QCRIL_LOG_DEBUG("max length = %d, msg_id = %d", tlv_stream_len, message_id);
      uint32_t SRV_MSG_ID_LEN =
          OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;
      char* payload = (char*)qcril_malloc(tlv_stream_len + SRV_MSG_ID_LEN);
      if (payload) {
        uint32_t encoded_fact = QMI_RIL_ZERO;
        idl_err = qmi_idl_message_encode(srv_obj, QMI_IDL_INDICATION, message_id, rawData, rawLen,
                                         payload + SRV_MSG_ID_LEN, tlv_stream_len, &encoded_fact);

        if (QMI_NO_ERR == idl_err) {
          // complete the oem hook tunneling header
          memcpy(payload, &service_id, OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE);
          memcpy(payload + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE, &message_id,
                 OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE);
          rawData = (uint8_t*)payload;
          rawLen = encoded_fact + SRV_MSG_ID_LEN;
          uint32_t bufLen = 0;
          uint8_t* buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_GENERIC,
                                                             rawData, rawLen);
          if (buf) {
            rawData = buf;
            rawLen = bufLen;
          }
        } else {
          QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
          qcril_free(payload);
          return false;
        }
      } else {
        QCRIL_LOG_DEBUG("Memory Allocation Failed");
        return false;
      }
    } else {
      QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
      return false;
    }
  } else {
    QCRIL_LOG_DEBUG("Could not create required srv obj");
    return false;
  }
  return true;
}

#ifndef FEATURE_ENHANCED_OEMHOOK_IMPLN
bool genericMessageDecode(uint8_t*& reqData, uint16_t& service_id, uint16_t& message_id,
                          uint32_t& reqLen) {
  /* OEMHOOK request format
     [8 byte OemIdentifier][4 bytes RequestID][4bytes PayloadSize][PayloadSize bytes Payload]
     QCRIL_REQ_HOOK_REQ_GENERIC is used for EMBMS, VT and Presence.
     Actual message id need to be extracted from the payload
     Payload format is
     [8bytes ModemID][2bytes ServiceID][2bytes MessageID][ActualPayload]
  */
  QCRIL_LOG_DEBUG("Generic Message Decode");
  // ignore the modem id size
  reqData = reqData + OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE;
  // extract service_id
  memcpy(&service_id, reqData, OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE);
  reqData = reqData + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE;
  // extract message_id
  memcpy(&message_id, reqData, OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE);
  reqData = reqData + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;
  reqLen = reqLen - OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE - OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE -
           OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE;
  auto srv_obj = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object(
      (qmi_ril_oem_hook_qmi_tunneling_service_id_type)service_id);
  if (srv_obj) {
    uint32_t substituted_data_len;
    char* substituted_data = nullptr;
    auto idl_err = qmi_idl_get_message_c_struct_len(srv_obj, QMI_IDL_REQUEST, message_id,
                                                    &substituted_data_len);
    if (service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT &&
        message_id == IMS_VT_GET_CALL_INFO_REQ_V01) {
      QCRIL_LOG_DEBUG("change max length to 4 due to  idl issue");
      substituted_data_len = 4;
    }
    if (idl_err == QMI_NO_ERR && substituted_data_len > 0) {
      substituted_data = (char*)qcril_malloc(substituted_data_len);
      if (substituted_data) {
        idl_err = qmi_idl_message_decode(srv_obj, QMI_IDL_REQUEST, message_id, reqData, reqLen,
                                         substituted_data, substituted_data_len);
        if (idl_err == QMI_NO_ERR) {
          reqData = (uint8_t*)substituted_data;
          reqLen = substituted_data_len;
        } else {
          QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
          qcril_free(substituted_data);
          return false;
        }
      } else {
        QCRIL_LOG_DEBUG("Memory Allocation Failed");
        return false;
      }
    } else {
      QCRIL_LOG_ERROR("QMI IDL - request decode error %d", (int)idl_err);
      return false;
    }
  } else {
    QCRIL_LOG_DEBUG("Could not create required srv obj");
    return false;
  }
  return true;
}

bool OemHookServiceBase::genericMessageHandler(int32_t serial, uint8_t* reqData, uint32_t reqLen) {
  QCRIL_LOG_DEBUG("Generic Message Handler");
  uint16_t service_id = 0;
  uint16_t message_id = 0;
  if (genericMessageDecode(reqData, service_id, message_id, reqLen)) {
    bool generic_processed = true;
    if (service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS) {
      switch (message_id) {
#ifndef RIL_FOR_LOW_RAM
        case QMI_EMBMS_ACTIVATE_TMGI_REQ_V01:
          processActiveTMGI(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01:
          processDeActiveTMGI(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01:
          processGetAvailableTMGI(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01:
          processGetActiveTMGI(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01:
          processActiveDeactivateTMGI(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01:
          processContentDescUpdate(serial, reqData, reqLen);
          break;
        case QMI_EMBMS_GET_INTERESTED_TMGI_LIST_RESP_V01:
          processSendInterestedListTMGI(serial, reqData, reqLen);
          break;
#endif
        default:
          generic_processed = false;
      }
    } else {
      generic_processed = false;
    }
    if (reqData) {
      free(reqData);
    }
    return generic_processed;
  } else {
    return false;
  }
  return true;
}
#else

bool OemHookServiceBase::genericMessageHandler(int32_t serial, const uint8_t* reqData,
                                               uint32_t reqDataLen) {
  QCRIL_LOG_FUNC_ENTRY();
  uint16_t service_id = 0;
  uint16_t message_id = 0;
  uint8_t* msgData = nullptr;
  uint32_t msgDataLen = 0;

  bool result = oemhook::utils::decodeGenericRequest(reqData, reqDataLen, service_id, message_id,
                                                     msgData, msgDataLen);
  if (result) {
    QCRIL_LOG_DEBUG("serviceId = %d, messageId = 0x%X", service_id, message_id);
    if (service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS) {
      switch (message_id) {
#ifndef RIL_FOR_LOW_RAM
        case QMI_EMBMS_ACTIVATE_TMGI_REQ_V01:
          processActiveTMGI(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01:
          processDeActiveTMGI(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01:
          processGetAvailableTMGI(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01:
          processGetActiveTMGI(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01:
          processActiveDeactivateTMGI(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_UPDATE_CONTENT_DESC_REQ_V01:
          processContentDescUpdate(serial, msgData, msgDataLen);
          break;
        case QMI_EMBMS_GET_INTERESTED_TMGI_LIST_RESP_V01:
          processSendInterestedListTMGI(serial, msgData, msgDataLen);
          break;
#endif
        default:
          result = false;
          break;
      }
    } else if (service_id == QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER) {
      switch (message_id) {
        case QMI_Qtuner_SET_RFM_SCENARIO_REQ_V01:
          setRfmScenarioReq(serial, msgData, msgDataLen);
          break;
        case QMI_Qtuner_GET_RFM_SCENARIO_REQ_V01:
          getRfmScenarioReq(serial, msgData, msgDataLen);
          break;
        case QMI_Qtuner_GET_PROVISIONED_TABLE_REVISION_REQ_V01:
          getProvisionedTableRevisionReq(serial, msgData, msgDataLen);
          break;
        default:
          result = false;
          break;
      }
    } else {
      result = false;
    }
  }
  if (msgData) {
    free(msgData);
  }
  QCRIL_LOG_FUNC_RETURN("result = %d", result);
  return result;
}
#endif

#ifndef RIL_FOR_LOW_RAM
/*
 * QCRIL_REQ_HOOK_EMBMS_SEND_INTERESTED_TMGI_LIST is handled by this API
 */
void OemHookServiceBase::processSendInterestedListTMGI(int32_t serial, uint8_t* data,
                                                       uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processSendInterestedListTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_get_interested_tmgi_list_resp_msg_v01* payload =
      (embms_get_interested_tmgi_list_resp_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsSendIntTmgiListMessage>(*payload);
  if (msg) {
    GenericCallback<embms_get_interested_tmgi_list_resp_ack_msg_v01> cb((
        [this, serial](
            std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
            std::shared_ptr<embms_get_interested_tmgi_list_resp_ack_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_GET_INTERESTED_TMGI_LIST_RESP_ACK_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_CONTENT_DESC_UPDATE is handled by this API
 */
void OemHookServiceBase::processContentDescUpdate(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processContentDescUpdate on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_update_content_desc_req_msg_v01* payload = (embms_update_content_desc_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsContentDescUpdateMessage>(*payload);
  if (msg) {
    GenericCallback<embms_update_content_desc_resp_msg_v01> cb(
        ([this, serial](
             std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
             std::shared_ptr<embms_update_content_desc_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_UPDATE_CONTENT_DESC_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI is handled by this API
 */
void OemHookServiceBase::processActiveDeactivateTMGI(int32_t serial, uint8_t* data,
                                                     uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processActiveDeactivateTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_activate_deactivate_tmgi_req_msg_v01* payload =
      (embms_activate_deactivate_tmgi_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsActivateDeactivateTmgiMessage>(*payload);
  if (msg) {
    GenericCallback<embms_activate_deactivate_tmgi_resp_msg_v01> cb(
        ([this, serial](
             std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
             std::shared_ptr<embms_activate_deactivate_tmgi_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_GET_ACTIVE_TMGI is handled by this API
 */
void OemHookServiceBase::processGetActiveTMGI(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processGetActiveTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_get_active_tmgi_req_msg_v01* payload = (embms_get_active_tmgi_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsGetActiveTmgiMessage>(*payload);
  if (msg) {
    GenericCallback<embms_get_active_tmgi_resp_msg_v01> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<embms_get_active_tmgi_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_GET_AVAILABLE_TMGI is handled by this API
 */
void OemHookServiceBase::processGetAvailableTMGI(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processGetAvailableTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_get_available_tmgi_req_msg_v01* payload = (embms_get_available_tmgi_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsGetAvailTmgiMessage>(*payload);
  if (msg) {
    GenericCallback<embms_get_available_tmgi_resp_msg_v01> cb(
        ([this, serial](
             std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
             std::shared_ptr<embms_get_available_tmgi_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_DEACTIVATE_TMGI is handled by this API
 */
void OemHookServiceBase::processDeActiveTMGI(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processDeActiveTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_deactivate_tmgi_req_msg_v01* payload = (embms_deactivate_tmgi_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsDeactivateTmgiMessage>(*payload);
  if (msg) {
    GenericCallback<embms_deactivate_tmgi_resp_msg_v01> cb((
        [this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<embms_deactivate_tmgi_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}
/*
 * QCRIL_REQ_HOOK_EMBMS_ACTIVATE_TMGI is handled by this API
 */
void OemHookServiceBase::processActiveTMGI(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookServiceBase::processActiveTMGI on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    sendResponse(serial, RadioError::INVALID_ARGUMENTS);
    return;
  }

  embms_activate_tmgi_req_msg_v01* payload = (embms_activate_tmgi_req_msg_v01*)data;
  auto msg = std::make_shared<rildata::RilRequestEmbmsActivateTmgiMessage>(*payload);
  if (msg) {
    GenericCallback<embms_activate_tmgi_resp_msg_v01> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<embms_activate_tmgi_resp_msg_v01> responseDataPtr) -> void {
          QCRIL_NOTUSED(status);
          if (solicitedMsg && responseDataPtr) {
            hidl_vec<uint8_t> data;
            uint32_t rawLen = sizeof(*responseDataPtr);
            auto rawData = (uint8_t*)responseDataPtr.get();
            if (encodeGenericResponsePayLoad(rawData, rawLen,
                                             QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                             QMI_EMBMS_ACTIVATE_TMGI_RESP_V01)) {
              data.setToExternal(rawData, rawLen);
              sendResponse(serial, RadioError::NONE, data);
              if (rawData) {
                free(rawData);
              }
            } else {
              sendResponse(serial, RadioError::GENERIC_FAILURE);
            }
          }
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY);
  }
  QCRIL_LOG_FUNC_RETURN();
}

void OemHookServiceBase::sendInterestedTMGIListChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::InterestedTMGIListIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_UNSOL_GET_INTERESTED_TMGI_LIST_REQ_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
void OemHookServiceBase::sendContentDescUpdateChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::ContentDescUpdateIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_UNSOL_CONTENT_DESC_UPDATE_PER_OBJ_IND_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
void OemHookServiceBase::sendTMGIAvailableChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::TMGIAvailableIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_AVAILABLE_TMGI_IND_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
void OemHookServiceBase::sendTMGIListOOSWarningChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::TMGIListOOSWarningIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_OOS_WARNING_IND_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
void OemHookServiceBase::sendTMGIListChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::TMGIListChangeIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_ACTIVE_TMGI_IND_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
void OemHookServiceBase::sendSaiListChange(std::shared_ptr<Message> msg) {
  if (msg) {
    auto m = std::static_pointer_cast<rildata::SaiListChangeIndMessage>(msg);
    if (m == nullptr) {
      return;
    }
    auto records = m->getParam();
    uint8_t* rawData = (uint8_t*)(void*)(&records);
    uint32_t rawLen = sizeof(records);
    if (encodeGenericUnsolPayload(rawData, rawLen, QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS,
                                  QMI_EMBMS_SAI_IND_V01)) {
      hidl_vec<uint8_t> data;
      data.setToExternal(rawData, rawLen, true);
      sendIndication(data);
    }
  }
}
#endif

uint8_t* convertAdnRecords(uint32_t* length, const qcril::interfaces::AdnRecords* adn_record_ptr) {
  uint8_t* hook_response;
  uint16_t element_index, response_index = 0;
  uint16_t name_length, number_length, email_length, anr_length;
  uint16_t email_elements, email_index, anr_elements, anr_index;
  const uint32_t INT_SIZE = 2;
  uint32_t response_length = 0;

  response_length = INT_SIZE;  // record_elements
  for (element_index = 0; element_index < adn_record_ptr->record_elements; element_index++) {
    const qcril::interfaces::AdnRecordInfo* adn_resp_ptr =
        &adn_record_ptr->adn_record_info[element_index];

    response_length += INT_SIZE;  // record_id
    response_length += INT_SIZE;  // name_length
    if (!adn_resp_ptr->name.empty()) {
      response_length += strlen(adn_resp_ptr->name.c_str());  // name
    }
    response_length += INT_SIZE;  // number_length
    if (!adn_resp_ptr->number.empty()) {
      response_length += strlen(adn_resp_ptr->number.c_str());  // number
    }
    response_length += INT_SIZE;  // email_elements
    for (email_index = 0; email_index < adn_resp_ptr->email_elements; email_index++) {
      response_length += INT_SIZE;                                          // email_length
      response_length += strlen(adn_resp_ptr->email[email_index].c_str());  // email
    }
    response_length += INT_SIZE;  // anr_elements
    for (anr_index = 0; anr_index < adn_resp_ptr->anr_elements; anr_index++) {
      response_length += INT_SIZE;                                            // anr_length
      response_length += strlen(adn_resp_ptr->ad_number[anr_index].c_str());  // anr
    }
    response_length++;
  }

  QCRIL_LOG_INFO("Response length is: %d", response_length);

  hook_response = (uint8_t*)qcril_malloc(response_length);
  if (!hook_response) {
    QCRIL_LOG_ERROR("Failed to alloc memory for oem hook payload");
    return NULL;
  }

  // record_elements
  response_index = 0;
  hook_response[response_index++] = adn_record_ptr->record_elements & 0x00FF;
  hook_response[response_index++] = (adn_record_ptr->record_elements & 0xFF00) >> 8;

  for (element_index = 0; element_index < adn_record_ptr->record_elements; element_index++) {
    const qcril::interfaces::AdnRecordInfo* adn_resp_ptr =
        &adn_record_ptr->adn_record_info[element_index];

    // record_id
    hook_response[response_index++] = adn_resp_ptr->record_id & 0x00FF;
    hook_response[response_index++] = (adn_resp_ptr->record_id & 0xFF00) >> 8;

    // name_length
    if (!adn_resp_ptr->name.empty()) {
      name_length = strlen(adn_resp_ptr->name.c_str());
    } else {
      name_length = 0;
    }
    hook_response[response_index++] = name_length & 0x00FF;
    hook_response[response_index++] = (name_length & 0xFF00) >> 8;

    // name
    if (!adn_resp_ptr->name.empty()) {
      memcpy(hook_response + response_index, adn_resp_ptr->name.c_str(), name_length);
    }
    response_index += name_length;

    // number_length
    if (!adn_resp_ptr->number.empty()) {
      number_length = strlen(adn_resp_ptr->number.c_str());
    } else {
      number_length = 0;
    }
    hook_response[response_index++] = number_length & 0x00FF;
    hook_response[response_index++] = (number_length & 0xFF00) >> 8;

    // number
    if (!adn_resp_ptr->number.empty()) {
      memcpy(hook_response + response_index, adn_resp_ptr->number.c_str(), number_length);
    }
    response_index += number_length;

    // email_elements
    email_elements = adn_resp_ptr->email_elements;
    hook_response[response_index++] = email_elements & 0x00FF;
    hook_response[response_index++] = (email_elements & 0xFF00) >> 8;

    for (email_index = 0; email_index < email_elements; email_index++) {
      // email_length
      email_length = strlen(adn_resp_ptr->email[email_index].c_str());
      hook_response[response_index++] = email_length & 0x00FF;
      hook_response[response_index++] = (email_length & 0xFF00) >> 8;
      // email
      memcpy(hook_response + response_index, adn_resp_ptr->email[email_index].c_str(), email_length);
      response_index += email_length;
    }

    // anr_elements
    anr_elements = adn_resp_ptr->anr_elements;
    hook_response[response_index++] = anr_elements & 0x00FF;
    hook_response[response_index++] = (anr_elements & 0xFF00) >> 8;

    for (anr_index = 0; anr_index < anr_elements; anr_index++) {
      // anr_length
      anr_length = strlen(adn_resp_ptr->ad_number[anr_index].c_str());
      hook_response[response_index++] = anr_length & 0x00FF;
      hook_response[response_index++] = (anr_length & 0xFF00) >> 8;
      // anr
      memcpy(hook_response + response_index, adn_resp_ptr->ad_number[anr_index].c_str(), anr_length);
      response_index += anr_length;
    }
  }
  hook_response[response_index] = '\0';

  *length = response_length;
  return hook_response;
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_ADN_RECORDS_IND to telephony via callback
 */
void OemHookServiceBase::sendAdnRecords(std::shared_ptr<QcRilUnsolAdnRecordsOnSimMessage> msg) {
  if (msg) {
    uint32_t recordsLen = 0;
    uint8_t* records = convertAdnRecords(&recordsLen, msg->getAdnRecords());

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_ADN_RECORDS_IND, records, recordsLen);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_ADN_INIT_DONE to telephony via callback
 */
void OemHookServiceBase::sendAdnInitDone(std::shared_ptr<QcRilUnsolAdnInitDoneMessage> msg) {
  if (msg) {
    QCRIL_NOTUSED(msg);

    uint32_t bufLen = 0;
    uint8_t* buf =
        oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_ADN_INIT_DONE, NULL, 0);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG to telephony via callback
 */
void OemHookServiceBase::sendMbnConfigResult(std::shared_ptr<QcRilUnsolMbnConfigResultMessage> msg) {
  if (msg) {
    char result = msg->getResult();
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_CONFIG,
                                                       (uint8_t*)&result, sizeof(result));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_CLEAR_CONFIGS to telephony via callback
 */
void OemHookServiceBase::sendMbnConfigCleared(std::shared_ptr<QcRilUnsolMbnConfigClearedMessage> msg) {
  if (msg) {
    char result = msg->getResult();
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_CLEAR_CONFIGS, (uint8_t*)&result, sizeof(result));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_VALIDATE_DUMPED to telephony via callback
 */
void OemHookServiceBase::sendMbnValidateDumped(
    std::shared_ptr<QcRilUnsolMbnValidateDumpedMessage> msg) {
  if (msg) {
    std::string dumpFile = msg->getDumpFile();
    uint32_t bufLen = 0;
    uint8_t* buf =
        oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_VALIDATE_DUMPED,
                                            (uint8_t*)dumpFile.c_str(), dumpFile.length());

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_LIST_CONFIG to telephony via callback
 */
void OemHookServiceBase::sendMbnConfigList(std::shared_ptr<QcRilUnsolMbnConfigListMessage> msg) {
  if (msg) {
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_LIST_CONFIG, (uint8_t*)msg->getResult().data(),
        msg->getResult().size());

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_VALIDATE_CONFIGS to telephony via callback
 */
void OemHookServiceBase::sendMbnValidateConfig(
    std::shared_ptr<QcRilUnsolMbnValidateConfigMessage> msg) {
  if (msg) {
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_VALIDATE_CONFIGS, (uint8_t*)msg->getMbnDiff().data(),
        msg->getMbnDiff().size());

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED to telephony via callback
 */
void OemHookServiceBase::uimSimlockTempUnlockExpireInd(
    std::shared_ptr<UimSimlockTempUnlockExpireInd> msg) {
  if (msg) {
    QCRIL_NOTUSED(msg);

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED, NULL, 0);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND to telephony via callback
 */
void OemHookServiceBase::uimCardStateChangeInd(std::shared_ptr<UimCardStateChangeInd> msg) {
  if (msg) {
    RIL_SlotCardState card_state = {};
    uint32_t bufLen = 0;
    hidl_vec<uint8_t> data = {};
    uint8_t* buf = NULL;

    card_state.slot_id = (uint32_t)msg->get_slot();
    card_state.card_state = (RIL_CardState)msg->get_card_state();

    buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND,
                                              (uint8_t*)&card_state, sizeof(card_state));

    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SLOT_STATUS_CHANGE_IND to telephony via callback
 */
void OemHookServiceBase::uimSlotStatusInd(std::shared_ptr<UimSlotStatusInd> msg) {
  if (msg) {
    RIL_SlotsStatus status = {};
    uint32_t bufLen = 0;
    hidl_vec<uint8_t> data = {};
    uint8_t* buf = NULL;
    std::vector<RIL_UIM_SlotStatus> ind = msg->get_status();

    if (ind.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT) {
      status.no_of_slots = ind.size();

      for (uint8_t i = 0; i < status.no_of_slots && i < UIM_OEM_HOOK_MAX_CARD_COUNT; i++) {
        status.slot_status[i].card_state = (Ril_CardPresenceState)ind[i].card_state;
        status.slot_status[i].slot_state = (Ril_SlotState)ind[i].slot_state;
        status.slot_status[i].logical_slot = (uint32_t)ind[i].logical_slot;

        if (ind[i].iccid.size() <= UIM_OEM_HOOK_MAX_ICCID_LEN) {
          status.slot_status[i].iccid_len = ind[i].iccid.size();
          memcpy(status.slot_status[i].iccid, ind[i].iccid.data(), status.slot_status[i].iccid_len);
        }
      }
    }

    buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SLOT_STATUS_CHANGE_IND,
                                              (uint8_t*)&status, sizeof(status));

    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH to telephony via callback
 */
void OemHookServiceBase::uimSimRefreshIndication(std::shared_ptr<UimSimRefreshIndication> msg) {
  if (msg) {
    RIL_Hook_SimRefreshResponse refresh = {};
    uint32_t bufLen = 0;
    hidl_vec<uint8_t> data = {};
    uint8_t* buf = NULL;
    RIL_UIM_SIM_RefreshIndication ind = msg->get_refresh_ind();

    refresh.result = (RIL_SimRefreshResult)ind.result;
    refresh.ef_id = ind.ef_id;
    refresh.app_type = (RIL_AppType)ind.app_type;

    if (ind.aid.length() <= (UIM_OEM_HOOK_MAX_AID_SIZE * 2)) {
      refresh.aid_len = ind.aid.length();
      ind.aid.copy(refresh.aid, refresh.aid_len);
    }

    buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH,
                                              (uint8_t*)&refresh, sizeof(refresh));

    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_UICC_VOLTAGE_STATUS to telephony via callback
 */
void OemHookServiceBase::uimVoltageStatusInd(std::shared_ptr<UimVoltageStatusInd> msg) {
  if (msg) {
    RIL_UiccVoltageStatus status = (RIL_UiccVoltageStatus)msg->get_status();
    uint32_t bufLen = 0;
    hidl_vec<uint8_t> data = {};
    uint8_t* buf = NULL;

    buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_UICC_VOLTAGE_STATUS,
                                              (uint8_t*)status, sizeof(status));

    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_REMOTE_SIM_STATUS_IND to telephony via callback
 */
void OemHookServiceBase::uimRmtRemoteSimStatusIndMsg(
    std::shared_ptr<UimRmtRemoteSimStatusIndMsg> msg) {
  if (msg) {
    RIL_HookRemoteSimInfo status = {};
    uint32_t bufLen = 0;
    hidl_vec<uint8_t> data = {};
    uint8_t* buf = NULL;
    RIL_UIM_RMT_RemoteSIMInfo ind = msg->get_status();

    status.isEnable = ind.isEnable;
    status.transportType = (RIL_TransportType)ind.transportType;
    status.usage = (RIL_Usage)ind.usage;
    status.remoteEvent = (RIL_remoteEvent)ind.remoteEvent;

    buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_REMOTE_SIM_STATUS_IND,
                                              (uint8_t*)&status, sizeof(status));

    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*=========================================================================
  HELPER FUNCTION: on_length_enum_to_str
===========================================================================*/
void on_length_enum_to_str(qcril::interfaces::DtmfOnLength on_enum, char* str, int len) {
  if (len >= 4) {
    switch (on_enum) {
      case qcril::interfaces::DtmfOnLength::ONLENGTH_95MS:
        strlcpy(str, "95", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_150MS:
        strlcpy(str, "150", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_200MS:
        strlcpy(str, "200", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_250MS:
        strlcpy(str, "250", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_300MS:
        strlcpy(str, "300", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_350MS:
        strlcpy(str, "350", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_SMS:
        strlcpy(str, "SMS", len);
        break;
      default:
        break;
    }
  }
}

/*=========================================================================
  HELPER FUNCTION: off_length_enum_to_str
===========================================================================*/
void off_length_enum_to_str(qcril::interfaces::DtmfOffLength off_enum, char* str, int len) {
  if (len >= 4) {
    switch (off_enum) {
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_60MS:
        strlcpy(str, "60", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_100MS:
        strlcpy(str, "100", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_150MS:
        strlcpy(str, "150", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_200MS:
        strlcpy(str, "200", len);
        break;
      default:
        break;
    }
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF to telephony via callback
 */
void OemHookServiceBase::sendUnsolCdmaBurstDtmf(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  if (msg) {
    char payload[QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH];
    char on_length[4];
    char off_length[4];
    int digit_buf_len = 0;

    memset(payload, 0, sizeof(payload));
    memset(on_length, 0, sizeof(on_length));
    memset(off_length, 0, sizeof(off_length));

    if (msg->hasOnLength()) {
      on_length_enum_to_str(msg->getOnLength(), on_length, sizeof(on_length));
    }
    if (msg->hasOffLength()) {
      off_length_enum_to_str(msg->getOffLength(), off_length, sizeof(off_length));
    }

    if (!msg->getDigitBuffer().empty()) {
      digit_buf_len = std::min(msg->getDigitBuffer().size(),
                               (sizeof(payload) - (sizeof(on_length) + sizeof(off_length))));
    }
    memcpy(payload, on_length, sizeof(on_length));
    memcpy(payload + sizeof(on_length), off_length, sizeof(off_length));
    if (digit_buf_len) {
      memcpy(payload + sizeof(on_length) + sizeof(off_length), (void*)msg->getDigitBuffer().c_str(),
             digit_buf_len);
    }

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF, (uint8_t*)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START to telephony via callback
 */
void OemHookServiceBase::sendUnsolCdmaContDtmfStart(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  if (msg) {
    char payload = 0;
    if (!msg->getDigitBuffer().empty()) {
      payload = msg->getDigitBuffer()[0];
    }

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START, (uint8_t*)&payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP to telephony via callback
 */
void OemHookServiceBase::sendUnsolCdmaContDtmfStop(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  if (msg) {
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP, NULL, 0);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL to telephony via callback
 */
void OemHookServiceBase::sendUnsolExtendedDbmIntl(std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg) {
  if (msg) {
    int payload[QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH];
    memset(payload, 0, sizeof(payload));

    payload[0] = msg->getMcc();
    payload[1] = msg->getDataBurstSubType();
    payload[2] = msg->getChargeInd();
    payload[3] = msg->getSubUnit();
    payload[4] = msg->getUnit();

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL, (uint8_t*)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE to telephony via callback
 */
void OemHookServiceBase::sendUnsolNssRelease(std::shared_ptr<QcRilUnsolNssReleaseMessage> msg) {
  if (msg) {
    uint8_t callId = msg->getCallId();
    voice_nss_release_enum_v02 nssRelease = msg->getNssRelease();

    size_t nss_release_size = sizeof(nssRelease);
    size_t call_id_size = sizeof(callId);
    char payload[(nss_release_size + call_id_size)];
    memcpy(payload, &nssRelease, nss_release_size);
    memcpy((payload + nss_release_size), &callId, call_id_size);

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE,
                                                       (uint8_t*)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE to telephony via callback
 */
void OemHookServiceBase::sendUnsolSsErrorCode(std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg) {
  if (msg) {
    int payload[2] = { 0 };

    payload[0] = msg->getCallId();
    payload[1] = msg->getFailureCause();

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE,
                                                       (uint8_t*)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO to telephony via callback
 */
void OemHookServiceBase::sendUnsolSpeechCodecInfo(
    std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg) {
  if (msg) {
    int payload[3] = { 0 };

    payload[0] = msg->getCallId();
    payload[1] = msg->getSpeechCodec();
    payload[2] = msg->getNetworkMode();

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO, (uint8_t*)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_AUDIO_STATE_CHANGED to telephony via callback
 */
void OemHookServiceBase::sendUnsolAudioStateChanged(
    std::shared_ptr<QcRilUnsolAudioStateChangedMessage> msg) {
  if (msg) {
    auto payload = msg->getAudioParams();
    uint32_t bufLen = 0;
    uint8_t* buf =
        oemhook::utils::constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_AUDIO_STATE_CHANGED,
                                            (uint8_t*)payload.c_str(), payload.size() + 1);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_WMS_READY to telephony via callback
 */
void OemHookServiceBase::sendUnsolWmsReady(
    std::shared_ptr<QcRilUnsolWmsReadyMessage> msg) {
  if (msg) {
    auto payload = msg->getStatus();
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_WMS_READY, (uint8_t*)&payload,
        sizeof(payload));
    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_MAX_DATA_CHANGE_IND to telephony via callback
 */
void OemHookServiceBase::sendUnsolMaxActiveDataSubsChanged(
    std::shared_ptr<RilUnsolMaxActiveDataSubsChangedMessage> msg) {
  if (msg) {
    auto payload = msg->getMaxActiveDataSubscriptions();
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_MAX_DATA_CHANGE_IND, (uint8_t*)&payload, sizeof(payload));
    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CSG_ID_CHANGE_IND to telephony via callback
 */
void OemHookServiceBase::sendUnsolCsgIdChanged(std::shared_ptr<RilUnsolCsgIdChangedMessage> msg) {
  if (msg) {
    auto payload = msg->getCsgId();
    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
        &bufLen, QCRIL_REQ_HOOK_UNSOL_CSG_ID_CHANGE_IND, (uint8_t*)&payload, sizeof(payload));
    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
    sendIndication(data);
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_ENGINEER_MODE to telephony via callback
 */
void OemHookServiceBase::sendUnsolEngineerMode(std::shared_ptr<RilUnsolEngineerModeMessage> msg) {
  if (msg) {
    uint8_t *payload = nullptr;
    size_t payload_len = 0;
    if (msg->isCdmaFtmDataValid()) {
      cdma_ftm_data &data = msg->getCdmaFtmData();
      payload = (uint8_t *)&data;
      payload_len = sizeof(data);
    }
    if (msg->isGsmFtmDataValid()) {
      gsm_ftm_data &data = msg->getGsmFtmData();
      payload = (uint8_t *)&data;
      payload_len = sizeof(data);
    }
    if (msg->isWcdmaFtmDataValid()) {
      wcdma_ftm_data &data = msg->getWcdmaFtmData();
      payload = (uint8_t *)&data;
      payload_len = sizeof(data);
    }

    if (payload && payload_len) {
      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_ENGINEER_MODE, payload, payload_len);
      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
      sendIndication(data);
    }
  }
}

void OemHookServiceBase::sendUnsolNetworkScanResult(
    std::shared_ptr<RilUnsolOemNetworkScanMessage> msg) {
  if (msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
    const char *respData = msg->getData();
    uint16_t respDataLen = msg->getDataLen();

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
      &bufLen, QCRIL_REQ_HOOK_UNSOL_INCREMENTAL_NW_SCAN_IND, (uint8_t *)respData, respDataLen);
    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
    sendIndication(data);
  }
}

void OemHookServiceBase::sendUnsolSubProvisionStatusChanged(
   std::shared_ptr<RilUnsolSubProvisioningStatusMessage> msg) {
  if (msg) {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
    qcril::interfaces::RILSubProvStatus_t prefResp = msg->getStatus();
    int32_t resp[2];
    resp[0] = prefResp.user_preference;
    resp[1] = prefResp.current_sub_preference;

    uint32_t bufLen = 0;
    uint8_t* buf = oemhook::utils::constructOemHookRaw(
      &bufLen, QCRIL_REQ_HOOK_UNSOL_SUB_PROVISION_STATUS, (uint8_t *)resp, sizeof(resp));
    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete   memory */
    sendIndication(data);
  }
}

/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2017,2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifdef FEATURE_QCRIL_LTE_DIRECT
#define TAG "RILQ"

#include "lte_direct/qcril_qmi_lte_direct_disc_oemhook_service.h"

#include <interfaces/lte_direct/QcRilRequestCancelPublishMessage.h>
#include <interfaces/lte_direct/QcRilRequestCancelSubscribeMessage.h>
#include <interfaces/lte_direct/QcRilRequestGetDeviceCapabilityMessage.h>
#include <interfaces/lte_direct/QcRilRequestGetServiceStatusMessage.h>
#include <interfaces/lte_direct/QcRilRequestLteDirectInitializeMessage.h>
#include <interfaces/lte_direct/QcRilRequestPublishMessage.h>
#include <interfaces/lte_direct/QcRilRequestSubscribeMessage.h>
#include <interfaces/lte_direct/QcRilRequestTerminateMessage.h>
#include <interfaces/lte_direct/lte_direct.h>
#include "lte_direct/qcril_qmi_lte_direct_disc_packing.h"
#include "lte_direct/qcril_qmi_lte_direct_disc_msg_meta.h"

#include "qcril_qmi_oemhook_utils.h"
#include "qcril_qmi_oem_reqlist.h"

using ::vendor::qti::hardware::radio::qcrilhook::V1_0::RadioError;
using ::android::hardware::hidl_vec;

namespace oemhook {
namespace lte_direct {

bool LteDirectImpl::processLteDirectDiscRequest(int32_t serial, uint8_t* data, uint32_t dataLen) {
  QCRIL_LOG_FUNC_ENTRY();

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty: data = %p, dataLen = %d", data, dataLen);
    sendResponse(serial, RadioError::INVALID_ARGUMENTS, hidl_vec<uint8_t>{});
    return true;
  }

  size_t unpacked_msg_size = 0;
  LteDirectDiscovery_MsgHeader* msg_tag_ptr = NULL;
  void* msg_data_ptr = NULL;

  QCRIL_LOG_INFO("unpacking Message tag");

  msg_tag_ptr = qcril_qmi_lte_direct_disc_unpack_msg_tag(data, dataLen);

  if (msg_tag_ptr) {
    QCRIL_LOG_INFO("msg: %s, type: %d, message_id: %d, error: %d",
                   qcril_qmi_lte_direct_disc_get_msg_log_str(msg_tag_ptr->id, msg_tag_ptr->type),
                   msg_tag_ptr->type, msg_tag_ptr->id, msg_tag_ptr->error);

    qcril_qmi_lte_direct_disc_parse_packed_msg(msg_tag_ptr->type, msg_tag_ptr->id,
                                               msg_tag_ptr->payload, dataLen, &msg_data_ptr,
                                               &unpacked_msg_size);

    switch (msg_tag_ptr->id) {
      case LteDirectDiscovery_MsgId_REQUEST_INITIALIZE:
        lteDirectDiscoveryInitializeRequest(serial);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY:
        lteDirectDiscoveryGetDeviceCapabilityRequest(serial);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_TERMINATE:
        lteDirectDiscoveryTerminateRequest(serial, (LteDirectDiscovery_Terminate*)msg_data_ptr);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS:
        lteDirectDiscoveryGetServiceStatusRequest(serial);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_PUBLISH:
        lteDirectDiscoveryPublishRequest(serial, (LteDirectDiscovery_Publish*)msg_data_ptr);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH:
        lteDirectDiscoveryCancelPublishRequest(serial,
                                               (LteDirectDiscovery_CancelPublish*)msg_data_ptr);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE:
        lteDirectDiscoverySubscribeRequest(serial, (LteDirectDiscovery_Subscribe*)msg_data_ptr);
        break;
      case LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE:
        lteDirectDiscoveryCancelSubscribeRequest(serial,
                                                 (LteDirectDiscovery_CancelSubscribe*)msg_data_ptr);
        break;
      default:
        break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
  return true;
}

void LteDirectImpl::lteDirectInit() {
  qcril_qmi_lted_init_msg_meta();
}

static qcril::interfaces::lte_direct::DiscoveryType convertDiscoveryType(
    LteDirectDiscovery_DiscoveryType in) {
  switch (in) {
    case LteDirectDiscovery_DiscoveryType_INVALID:
      return qcril::interfaces::lte_direct::DiscoveryType::UNKNOWN;
    case LteDirectDiscovery_DiscoveryType_OPEN:
      return qcril::interfaces::lte_direct::DiscoveryType::OPEN;
    case LteDirectDiscovery_DiscoveryType_RESTRICTED:
      return qcril::interfaces::lte_direct::DiscoveryType::RESTRICTED;
  }
  return qcril::interfaces::lte_direct::DiscoveryType::UNKNOWN;
}
static LteDirectDiscovery_Result convertLteDirectResult(qcril::interfaces::lte_direct::Result in) {
  switch (in) {
    case qcril::interfaces::lte_direct::Result::SUCCESS:
      return LteDirectDiscovery_Result_SUCCESS;
    case qcril::interfaces::lte_direct::Result::GENERIC_FAILURE:
      return LteDirectDiscovery_Result_GENERIC_FAILURE;
    case qcril::interfaces::lte_direct::Result::IN_PROGRESS:
      return LteDirectDiscovery_Result_IN_PROGRESS;
    case qcril::interfaces::lte_direct::Result::INVALID_EXPRESSION_SCOPE:
      return LteDirectDiscovery_Result_INVALID_EXPRESSION_SCOPE;
    case qcril::interfaces::lte_direct::Result::UNKNOWN_EXPRESSION:
      return LteDirectDiscovery_Result_UNKNOWN_EXPRESSION;
    case qcril::interfaces::lte_direct::Result::INVALID_DISCOVERY_TYPE:
      return LteDirectDiscovery_Result_INVALID_DISCOVERY_TYPE;
    case qcril::interfaces::lte_direct::Result::SERVICE_NOT_AVAILABLE:
      return LteDirectDiscovery_Result_SERVICE_NOT_AVAILABLE;
    case qcril::interfaces::lte_direct::Result::APP_AUTH_FAILURE:
      return LteDirectDiscovery_Result_APP_AUTH_FAILURE;
    case qcril::interfaces::lte_direct::Result::NOT_SUPPORTED:
    case qcril::interfaces::lte_direct::Result::UNKNOWN:
      return LteDirectDiscovery_Result_NOT_SUPPORTED;
  }
  return LteDirectDiscovery_Result_NOT_SUPPORTED;
}

void LteDirectImpl::lteDirectDiscoveryInitializeRequest(int32_t serial) {
  QCRIL_LOG_INFO("serial %d ", serial);

  auto msg = std::make_shared<QcRilRequestLteDirectInitializeMessage>(getContext(serial));

  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_INITIALIZE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> respData;
          respData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                 false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, respData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryGetDeviceCapabilityRequest(int32_t serial) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestGetDeviceCapabilityMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          LteDirectDiscovery_DeviceCapability outData = { .has_capability = FALSE };
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          void* respData = nullptr;
          size_t respDataLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            if (errorCode == RIL_E_SUCCESS) {
              auto rilData =
                  std::static_pointer_cast<qcril::interfaces::lte_direct::DeviceCapability>(
                      resp->data);
              if (rilData) {
                if (rilData->hasCapability()) {
                  outData.has_capability = TRUE;
                  outData.capability = rilData->getCapability();
                }
                respData = &outData;
                respDataLen = sizeof(outData);
              }
            }
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              respData, respDataLen, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY,
              LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryTerminateRequest(int32_t serial,
                                                       LteDirectDiscovery_Terminate* inData) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestTerminateMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char*)inData->osAppId.arg);
    } else {
      QCRIL_LOG_ERROR("Invalid OsAppId");
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_TERMINATE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryGetServiceStatusRequest(int32_t serial) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestGetServiceStatusMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          LteDirectDiscovery_ServiceStatus outData = { .has_publishAllowed = FALSE,
                                                       .has_subscribeAllowed = FALSE };
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          void* respData = nullptr;
          size_t respDataLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            if (errorCode == RIL_E_SUCCESS) {
              auto rilData =
                  std::static_pointer_cast<qcril::interfaces::lte_direct::ServiceStatus>(resp->data);
              if (rilData) {
                if (rilData->hasPublishAllowed()) {
                  outData.has_publishAllowed = TRUE;
                  outData.publishAllowed = rilData->getPublishAllowed();
                }
                if (rilData->hasSubscribeAllowed()) {
                  outData.has_subscribeAllowed = TRUE;
                  outData.subscribeAllowed = rilData->getSubscribeAllowed();
                }
                respData = &outData;
                respDataLen = sizeof(outData);
              }
            }
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              respData, respDataLen, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS,
              LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryPublishRequest(int32_t serial,
                                                     LteDirectDiscovery_Publish* inData) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestPublishMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char*)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char*)inData->expression.arg);
    }
    if (inData->has_expressionValidityTime) {
      msg->setExpressionValidityTime(inData->expressionValidityTime);
    }
    if (inData->metaData.arg) {
      msg->setMetaData((char*)inData->metaData.arg);
    }
    if (inData->has_discoveryType) {
      msg->setDiscoveryType(convertDiscoveryType(inData->discoveryType));
    }
    if (inData->has_duration) {
      msg->setDuration(inData->duration);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_PUBLISH, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryCancelPublishRequest(int32_t serial,
                                                           LteDirectDiscovery_CancelPublish* inData) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestCancelPublishMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char*)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char*)inData->expression.arg);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoverySubscribeRequest(int32_t serial,
                                                       LteDirectDiscovery_Subscribe* inData) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestSubscribeMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char*)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char*)inData->expression.arg);
    }
    if (inData->has_expressionValidityTime) {
      msg->setExpressionValidityTime(inData->expressionValidityTime);
    }
    if (inData->has_discoveryType) {
      msg->setDiscoveryType(convertDiscoveryType(inData->discoveryType));
    }
    if (inData->has_duration) {
      msg->setDuration(inData->duration);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::lteDirectDiscoveryCancelSubscribeRequest(
    int32_t serial, LteDirectDiscovery_CancelSubscribe* inData) {
  QCRIL_LOG_INFO("serial %d ", serial);
  auto msg = std::make_shared<QcRilRequestCancelSubscribeMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char*)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char*)inData->expression.arg);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = { 0 };
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t*)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          sendResponse(serial, (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    sendResponse(serial, RadioError::NO_MEMORY, hidl_vec<uint8_t>{});
  }

  QCRIL_LOG_FUNC_RETURN();
}

void LteDirectImpl::sendLteDirectUnsolAuthorizationResult(
    std::shared_ptr<QcRilUnsolAuthorizationResultMessage> msg) {
  if (msg) {
    LteDirectDiscovery_AuthorizationResult outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void*)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasResult()) {
        outData.has_result = TRUE;
        outData.result = convertLteDirectResult(msg->getResult());
      } else {
        QCRIL_LOG_ERROR("Result is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
  }
}
void LteDirectImpl::sendLteDirectUnsolDeviceCapabilityChanged(
    std::shared_ptr<QcRilUnsolDeviceCapabilityChangedMessage> msg) {
  if (msg) {
    LteDirectDiscovery_DeviceCapability outData = {};

    do {
      if (msg->hasCapability()) {
        outData.has_capability = TRUE;
        outData.capability = msg->getCapability();
      } else {
        QCRIL_LOG_ERROR("Capability is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);
  }
}

void LteDirectImpl::sendLteDirectUnsolExpressionStatus(
    std::shared_ptr<QcRilUnsolExpressionStatusMessage> msg) {
  if (msg) {
    LteDirectDiscovery_ExpressionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void*)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void*)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasResult()) {
        outData.has_result = TRUE;
        outData.result = convertLteDirectResult(msg->getResult());
      } else {
        QCRIL_LOG_ERROR("Result is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);
    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void LteDirectImpl::sendLteDirectUnsolMatchEvent(std::shared_ptr<QcRilUnsolMatchEventMessage> msg) {
  if (msg) {
    LteDirectDiscovery_MatchEvent outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void*)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void*)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasMatchedExpression() && msg->getMatchedExpression().size()) {
        outData.matchedExpression.arg = (void*)strdup(msg->getMatchedExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("MatchedExpression is not present");
        break;
      }
      if (msg->hasState()) {
        outData.has_state = TRUE;
        outData.state = msg->getState();
      } else {
        QCRIL_LOG_ERROR("state is not present");
        break;
      }
      if (msg->hasMetaDataIndex()) {
        outData.has_metaDataIndex = TRUE;
        outData.metaDataIndex = msg->getMetaDataIndex();
      } else {
        QCRIL_LOG_ERROR("MetaDataIndex is not present");
        break;
      }
      if (msg->hasMetaData() && msg->getMetaData().size()) {
        outData.metaData.arg = (void*)strdup(msg->getMetaData().c_str());
      } else {
        QCRIL_LOG_ERROR("MetaData is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT, LteDirectDiscovery_Error_E_SUCCESS,
          send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
    if (outData.matchedExpression.arg) {
      free(outData.matchedExpression.arg);
    }
    if (outData.metaData.arg) {
      free(outData.metaData.arg);
    }
  }
}

void LteDirectImpl::sendLteDirectUnsolPskExpirted(
    std::shared_ptr<QcRilUnsolPskExpirtedMessage> /*msg*/) {
  QCRIL_LOG_INFO("dummy implementation");
}

void LteDirectImpl::sendLteDirectUnsolTransmissionStatus(
    std::shared_ptr<QcRilUnsolTransmissionStatusMessage> msg) {
  if (msg) {
    LteDirectDiscovery_TransmissionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void*)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void*)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasStatus()) {
        outData.has_status = TRUE;
        outData.status = msg->getStatus();
      } else {
        QCRIL_LOG_ERROR("Status is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void LteDirectImpl::sendLteDirectUnsolReceptionStatus(
    std::shared_ptr<QcRilUnsolReceptionStatusMessage> msg) {
  if (msg) {
    LteDirectDiscovery_ReceptionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void*)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void*)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasStatus()) {
        outData.has_status = TRUE;
        outData.status = msg->getStatus();
      } else {
        QCRIL_LOG_ERROR("Status is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void LteDirectImpl::sendLteDirectUnsolServiceStatus(
    std::shared_ptr<QcRilUnsolServiceStatusMessage> msg) {
  if (msg) {
    LteDirectDiscovery_ServiceStatus outData = {};

    do {
      if (msg->hasPublishAllowed()) {
        outData.has_publishAllowed = TRUE;
        outData.publishAllowed = msg->getPublishAllowed();
      } else {
        QCRIL_LOG_ERROR("PublishAllowed is not present");
        break;
      }
      if (msg->hasSubscribeAllowed()) {
        outData.has_subscribeAllowed = TRUE;
        outData.subscribeAllowed = msg->getSubscribeAllowed();
      } else {
        QCRIL_LOG_ERROR("SubscribeAllowed is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = { 0 };
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10,  // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t* buf = oemhook::utils::constructOemHookRaw(
          &bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC, (uint8_t*)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      sendIndication(data);
    } while (FALSE);
  }
}

}  // namespace lte_direct
}  // namespace oemhook
#endif  // FEATURE_QCRIL_LTE_DIRECT

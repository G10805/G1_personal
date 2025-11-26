/******************************************************************************
#  Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "WDSModemEndPoint.h"
#include "sync/GetAttachListMessage.h"
#include "sync/SetAttachListSyncMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "request/GetPdnThrottleTimeMessage.h"
#include "sync/RegisterForKeepAliveSyncMessage.h"
#include "UnSolMessages/SetCapabilitiesMessage.h"
#include "UnSolMessages/GetPdnThrottleTimeResponseInd.h"
#include <sstream>

using std::to_string;
using namespace qdp;

constexpr const char *WDSModemEndPoint::NAME;

void WDSModemEndPoint::requestSetup(string clientToken, qcril_instance_id_e_type id,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, id, callback);
  mModule->dispatch(shared_setupMsg);
}

Message::Callback::Status WDSModemEndPoint::getAttachListSync
(
  std::shared_ptr<std::list<uint16_t>>& attach_list
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync] "
      "dispatching message GetAttachListMessage");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetAttachListMessage>(nullptr);
    Message::Callback::Status apiStatus = Message::Callback::Status::FAILURE;
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(attach_list);
    }
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListSync]"
      " Failed to send message GetAttachListMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::getLteAttachParams(wds_get_lte_attach_params_resp_msg_v01 *attach_param)
{
  int rc;
  Message::Callback::Status ret = Message::Callback::Status::FAILURE;
  wds_get_lte_attach_params_req_msg_v01 req;

  do
  {
    if (!attach_param)
    {
     Log::getInstance().d("[WDSModemEndPointModule]: BAD input");
      break;
    }

    memset(&req, 0, sizeof(req));
    memset(attach_param, 0, sizeof(wds_get_lte_attach_params_resp_msg_v01));

    rc = sendRawSync(QMI_WDS_GET_LTE_ATTACH_PARAMS_REQ_V01,
                     (void *)&req, sizeof(req),
                     (void*)attach_param, sizeof(*attach_param),
                     DEFAULT_SYNC_TIMEOUT);

    if( rc != QMI_NO_ERR ||(attach_param->resp.result != QMI_RESULT_SUCCESS_V01))
    {
      Log::getInstance().d("[WDSModemEndPointModule]: failed with rc=" + std::to_string(rc) +
      ", qmi_err=" + std::to_string(attach_param->resp.error));
    }

    if (rc == QMI_NO_ERR)
    {
      ret = Message::Callback::Status::SUCCESS;
    }

  } while(0);
  return ret;
}

Message::Callback::Status WDSModemEndPoint::setAttachListSync
(
  const std::shared_ptr<std::list<uint16_t>>& attach_list,
  const SetAttachListSyncMessage::AttachListAction action
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "dispatching message SetAttachListSyncMessage");
    // Note that callback is not required for 'dispatchSync' calls.
    auto msg =
        std::make_shared<SetAttachListSyncMessage>(nullptr);
    if(attach_list && msg != nullptr)
    {
      msg->setParams(*attach_list, action);
    } else {
      Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "attach_list is NULL.Returning FAILURE");
      return Message::Callback::Status::FAILURE;
    }

    Message::Callback::Status apiStatus;
    auto r = std::make_shared<int>();
    apiStatus = msg->dispatchSync(r);
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::setAttachListSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::setAttachListSync]"
      "Failed to send message SetAttachListSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::getAttachListCapabilitySync
(
  std::shared_ptr<AttachListCap>& cap
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      " dispatching message GetAttachListCapabilitySyncMessage");

    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<GetAttachListCapabilitySyncMessage>(nullptr);
    Message::Callback::Status apiStatus = Message::Callback::Status::FAILURE;
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(cap);
    }
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getAttachListCapabilitySync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;

  } else
  {
    Log::getInstance().d("[WDSModemEndPoint::getAttachListCapabilitySync]"
      "Failed to send message GetAttachListCapabilitySyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

/**
 * @brief Posts GetCallBringUpCapabilitySyncMessage to query the modem
 *
 * @return Success if message is posted succesfully, Failure otherwise
 **/
Message::Callback::Status WDSModemEndPoint::getCallBringUpCapabilitySync
(
  std::shared_ptr<BringUpCapability>& callBringUpCapability
)
{
  // Allow any QMI IDL API invocation only if we are in OPERATIONAL state.
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::getCallBringUpCapabilitySync] "
      "dispatching message GetCallBringUpCapabilitySyncMessage");
    // Note that callback is not required for sync calls.
    auto msg =
        std::make_shared<GetCallBringUpCapabilitySyncMessage>(nullptr);
    Message::Callback::Status apiStatus = Message::Callback::Status::FAILURE;
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(callBringUpCapability);
    }
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::getCallBringUpCapabilitySync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getCallBringUpCapabilitySync]"
     " Failed to send message GetCallBringUpCapabilitySyncMessage");
    return Message::Callback::Status::FAILURE;
  }
} /* WDSModemEndPoint::getCallBringUpCapabilitySync */

Message::Callback::Status WDSModemEndPoint::setV2Capabilities(bool nswo) {
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    auto msg = std::make_shared<SetCapabilitiesMessage>();
    msg->setWdsPdnThrottleV2(true);
    if (nswo) {
      msg->setWdsNswo(true);
    }
    msg->broadcast();
    return Message::Callback::Status::SUCCESS;
  }

  Log::getInstance().d("[WDSModemEndPoint::setV2Capabilities]"
                       "Failed to send message SetCapabilitiesMessage");
  return Message::Callback::Status::FAILURE;
}

void WDSModemEndPoint::getPdnThrottleTime(std::string apn, DataProfileInfoType_t techType, std::string ipType, int cid) {
  auto msg = std::make_shared<GetPdnThrottleTimeMessage>(apn, techType, ipType);
  GenericCallback<int64_t> cb([cid](std::shared_ptr<Message>, Message::Callback::Status status,
                          std::shared_ptr<int64_t> r) -> void {
    if (status == Message::Callback::Status::SUCCESS && r != nullptr) {
      auto indMsg = std::make_shared<GetPdnThrottleTimeResponseInd>(cid, *r);
      indMsg->broadcast();
    } else {
      Log::getInstance().d("[WDSModemEndPoint::getPdnThrottleTime] Failed to get throttle time");
      auto indMsg = std::make_shared<GetPdnThrottleTimeResponseInd>(cid, -1);
      indMsg->broadcast();
    }
  });
  msg->setCallback(&cb);
  msg->dispatch();
}

Message::Callback::Status WDSModemEndPoint::registerForKeepAliveInd
(
  bool toRegister
)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[WDSModemEndPoint::registerForKeepAliveInd]"
                         "dispatching message RegisterForKeepAliveSyncMessage register" + std::to_string(toRegister));
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<RegisterForKeepAliveSyncMessage>(nullptr);
    Message::Callback::Status apiStatus = Message::Callback::Status::FAILURE;
    auto r = std::make_shared<int>();
    if (msg != nullptr) {
        msg->setParams(toRegister);
        apiStatus = msg->dispatchSync(r);
    }
    std::ostringstream ss;
    ss << "[WDSModemEndPoint::RegisterForKeepAliveIndMessage] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
     Log::getInstance().d("[WDSModemEndPoint::RegisterForKeepAliveIndMessage]"
                           "Failed to send message RegisterForKeepAliveSyncMessage");
     return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status WDSModemEndPoint::setDefaultProfileNum(TechType techType, ProfileId index)
{
  Log::getInstance().d("[WDSModemEndPoint::setDefaultProfileNum] = "+std::to_string(index));
  /* set supplied profile as default */
  wds_set_default_profile_num_req_msg_v01 ps_req;
  wds_set_default_profile_num_resp_msg_v01 ps_resp;

  memset(&ps_req, 0, sizeof(wds_set_default_profile_num_req_msg_v01));
  memset(&ps_resp, 0, sizeof(wds_set_default_profile_num_resp_msg_v01));
  ps_req.profile_identifier.profile_family = WDS_PROFILE_FAMILY_EMBEDDED_V01;
  switch (techType) {
    case TechType::THREE_GPP:
      ps_req.profile_identifier.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
      break;
    case TechType::THREE_GPP2:
      ps_req.profile_identifier.profile_type = WDS_PROFILE_TYPE_3GPP2_V01;
      break;
    default:
      Log::getInstance().d("[WDSModemEndPoint::setDefaultProfileNum] = invalid tech type");
      break;
  }
  ps_req.profile_identifier.profile_index = (uint8_t)index;
  auto rc = sendRawSync(QMI_WDS_SET_DEFAULT_PROFILE_NUM_REQ_V01,
                  (void*)&ps_req, sizeof(ps_req),
                  (void*)&ps_resp, sizeof(ps_resp),
                  DEFAULT_SYNC_TIMEOUT);

  if ( QMI_NO_ERR != rc ||
       QMI_RESULT_SUCCESS_V01 != ps_resp.resp.result) {
    Log::getInstance().d("[WDSModemEndPoint] Failed to send"
      "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile_index:" \
        +std::to_string(ps_req.profile_identifier.profile_index)+"with rc ="+std::to_string(rc));
    return Message::Callback::Status::FAILURE;
  } else {
    Log::getInstance().d("[WDSModemEndPoint] setProfileDefault::Successfully sent"
      "QMI_WDS_SET_DEFAULT_PROFILE_NUM_V01 for profile index =" \
      + std::to_string(ps_req.profile_identifier.profile_index));
  }

  return Message::Callback::Status::SUCCESS;
}

bool WDSModemEndPoint::getReportingStatus()
{
  return mIsReportDataRegistrationRejectCause;
}

bool WDSModemEndPoint::getDataRegistrationState()
{
  return mIsDataRegistered;
}

void WDSModemEndPoint::updateDataRegistrationState(bool registered)
{
  mIsDataRegistered = registered;
  Log::getInstance().d("[WDSModemEndPoint::updateDataRegistrationState] = "+
                        std::to_string(mIsDataRegistered));
}

Message::Callback::Status WDSModemEndPoint::registerDataRegistrationRejectCause()
{
  return registerDataRegistrationRejectCause(mIsReportDataRegistrationRejectCause);
}

void WDSModemEndPoint::registerforTdInfoInd()
{
  Log::getInstance().d("WDSModemEndPoint::registerforIndications");
  wds_indication_register_req_msg_v01 req;
  wds_indication_register_resp_msg_v01 resp;

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  req.report_td_info_valid = TRUE;
  req.report_td_info = 1;

  auto rc = sendRawSync(QMI_WDS_INDICATION_REGISTER_REQ_V01,
                        (void*)&req, sizeof(req),
                        (void*)&resp, sizeof(resp),
                        DEFAULT_SYNC_TIMEOUT);

  if (rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    Log::getInstance().d("RegisterforTdInfoInd: failed with rc="+ std::to_string(rc) +", qmi_err=" + std::to_string(resp.resp.error));
  }
}

Message::Callback::Status WDSModemEndPoint::registerDataRegistrationRejectCause(bool enable)
{
  mIsReportDataRegistrationRejectCause = enable;
  Log::getInstance().d("[WDSModemEndPoint::registerDataRegistrationRejectCause] = "+
                        std::to_string(mIsReportDataRegistrationRejectCause));

  wds_indication_register_req_msg_v01 req;
  wds_indication_register_resp_msg_v01 resp;

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));
  req.report_lte_attach_failure_info_valid = true;
  req.report_lte_attach_failure_info = mIsReportDataRegistrationRejectCause;
  auto rc = sendRawSync(QMI_WDS_INDICATION_REGISTER_REQ_V01,
                        (void*)&req, sizeof(req),
                        (void*)&resp, sizeof(resp),
                        DEFAULT_SYNC_TIMEOUT);
  if ( QMI_NO_ERR != rc ||
       QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    return Message::Callback::Status::FAILURE;
  }

  return Message::Callback::Status::SUCCESS;
}

void WDSModemEndPoint::registerForPdnThrottle()
{
  Log::getInstance().d("[WDSModemEndPoint::registerForPdnThrottle]");

  wds_indication_register_req_msg_v01 req;
  wds_indication_register_resp_msg_v01 resp;

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));
  req.report_pdn_throttle_info_valid = true;
  req.report_pdn_throttle_info = 1;
  auto rc = sendRawSync(QMI_WDS_INDICATION_REGISTER_REQ_V01,
                        (void*)&req, sizeof(req),
                        (void*)&resp, sizeof(resp),
                        DEFAULT_SYNC_TIMEOUT);
  if ( QMI_NO_ERR != rc ||
       QMI_RESULT_SUCCESS_V01 != resp.resp.result) {
    Log::getInstance().d("[WDSModemEndPoint::registerForPdnThrottle] failed to register");
  }
}

Message::Callback::Status WDSModemEndPoint::getPduSessionParamLookup(uint16_t txId, ApnTypes_t apnType,
                                                                     std::optional<TrafficDescriptor_t> td,
                                                                     bool matchAllRuleAllowed)
{
  Log::getInstance().d("[WDSModemEndPoint]: getPduSessionParamLookup() txId="+std::to_string(txId));

  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    wds_pdu_session_param_lookup_req_msg_v01 req;
    wds_pdu_session_param_lookup_resp_msg_v01 resp;
    memset(&req, 0, sizeof(req));
    memset(&resp, 0, sizeof(resp));

    req.tx_id = txId;
    req.tx_td_info.total_td_frag_num = 1;
    req.tx_td_info.td_frag_index = 0;
    req.apn_type_mask_valid = 1;
    req.apn_type_mask = WDSModemEndPointModule::convertToApnTypeMask((int32_t)apnType);
    req.allow_match_all_valid = 1;
    req.allow_match_all = matchAllRuleAllowed;

    if(td.has_value()) {
      if (td.value().dnn.has_value()) {
        Log::getInstance().d("dnn="+td.value().dnn.value());
        req.dnn_valid = 1;
        memcpy(req.dnn, td.value().dnn.value().c_str(), td.value().dnn.value().length());
      }
      else {
        Log::getInstance().d("dnn invalid");
      }
      if (td.value().osAppId.has_value() && td.value().osAppId.value().size() > 17) {
        req.app_id_list_valid = 1;
        req.app_id_list_len = 1;
        // 16 bytes for OSId + 1 byte for OSAppId length + up to 255 bytes for OSAppId
        req.app_id_list[0].os_id_valid = 1;
        std::copy(td.value().osAppId.value().begin(),
                  td.value().osAppId.value().begin()+16,
                  req.app_id_list[0].os_id);
        req.app_id_list[0].os_app_id_len = td.value().osAppId.value()[16];
        std::copy(td.value().osAppId.value().begin()+17,
                  td.value().osAppId.value().begin()+17+td.value().osAppId.value()[16],
                  req.app_id_list[0].os_app_id);
      }
      else {
        Log::getInstance().d("osAppId invalid");
      }
    }

    int rc = sendRawSync(QMI_WDS_PDU_SESSION_PARAM_LOOKUP_REQ_V01,
                         (void *)&req, sizeof(req),
                         (void *)&resp, sizeof(resp),
                         QCRIL_DATA_QMI_TIMEOUT);
    if ((rc != QMI_NO_ERR) || (resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[WDSModemEndPoint] Failed to send QMI_WDS_PDU_SESSION_PARAM_LOOKUP_REQ_V01,"
                           "error="+ std::to_string(resp.resp.error));
    } else {
      Log::getInstance().d("[WDSModemEndPoint] QMI_WDS_PDU_SESSION_PARAM_LOOKUP_REQ_V01::Successfully sent");
      return Message::Callback::Status::SUCCESS;
    }
  } else {
    Log::getInstance().d("[WDSModemEndPoint::getPduSessionParamLookup]"
                         " not operational");
  }
  return Message::Callback::Status::FAILURE;
}

void WDSModemEndPoint::getSlicingConfigRequest(std::shared_ptr<GetSlicingConfigRequestMessage> m)
{
  Log::getInstance().d("[WDSModemEndPoint]: handleGetSlicingConfigRequest");
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    auto msg = std::make_shared<GetSlicingConfigInternalMessage>(m);
    msg->broadcast();
  }
  else
  {
    rildata::GetSlicingConfigResp_t resp;
    resp.respErr = ResponseError_t::INTERNAL_ERROR;
    m->sendResponse(m, Message::Callback::Status::FAILURE, std::make_shared<rildata::GetSlicingConfigResp_t>(resp));
    Log::getInstance().d("[WDSModemEndPoint::getSlicingConfigrequest]"
                         " not operational");
  }
  //release wakeLock
  if(m->getAcknowlegeRequestCb() != nullptr) {
    auto cb = *(m->getAcknowlegeRequestCb().get());
    cb(m->getSerial());
  }
}

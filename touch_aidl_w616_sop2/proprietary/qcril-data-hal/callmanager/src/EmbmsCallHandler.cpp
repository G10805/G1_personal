/*===========================================================================

  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <cstring>
#include <string.h>
#include <framework/Log.h>
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"

#include "MessageCommon.h"
#include "DataModule.h"
#include "CallInfo.h"
#include "EmbmsCallHandler.h"
#include "CallManager.h"

#include "request/RilRequestEmbmsActivateTmgiMessage.h"
#include "request/RilRequestEmbmsDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsActivateDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsContentDescUpdateMessage.h"
#include "request/RilRequestEmbmsGetActiveTmgiMessage.h"
#include "request/RilRequestEmbmsGetAvailTmgiMessage.h"
#include "request/RilRequestEmbmsSendIntTmgiListMessage.h"
#include "UnSolMessages/TMGIListChangeIndMessage.h"
#include "UnSolMessages/TMGIListOOSWarningIndMessage.h"
#include "UnSolMessages/ContentDescUpdateIndMessage.h"
#include "UnSolMessages/TMGIAvailableIndMessage.h"
#include "UnSolMessages/SaiListChangeIndMessage.h"
#include "UnSolMessages/InterestedTMGIListIndMessage.h"
#include "DataCommon.h"

using namespace rildata;

unordered_map<int, EmbmsDataCallFailCause_t> EmbmsCallEndReason::dsiCallEndReasonMap =
{
  {WDS_TMGI_ACTIVATE_FAILURE_UNKNOWN_V01,                           EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_RADIO_CONFIG,           EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_CHANNEL_UNAVAIL,        EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_EMBMS_NOT_ENABLED,      EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_OUT_OF_COVERAGE,        EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {WDS_TMGI_ACTIVATE_SUCCESS_DUPLICATE_V01,                         EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},         /*2*/
  {WDS_TMGI_ACTIVATE_FAILURE_NOT_ALLOWED_V01,                       EmbmsDataCallFailCause_t::ERROR_NOT_ALLOWED},          /*3*/
  {WDS_TMGI_ACTIVATE_FAILURE_MISSING_CONTROL_INFO_V01,              EmbmsDataCallFailCause_t::ERROR_MISSING_CONTROL_INFO}, /*4*/
  {WDS_TMGI_ACTIVATE_FAILURE_MISSING_TMGI_V01,                      EmbmsDataCallFailCause_t::ERROR_MISSING_TMGI},         /*5*/
  {WDS_TMGI_ACTIVATE_FAILURE_MCAST_OOS_V01,                         EmbmsDataCallFailCause_t::ERROR_MCAST_OOC},            /*6*/
  {WDS_TMGI_ACTIVATE_FAILURE_UCAST_OOS_V01,                         EmbmsDataCallFailCause_t::ERROR_UCAST_OOS},            /*7*/
  {WDS_TMGI_ACTIVATE_FAILURE_CAMPED_ON_OTHER_FREQ_V01,              EmbmsDataCallFailCause_t::ERROR_FREQUENCY_CONFLICT},   /*8*/
  {WDS_EMBMS_ECC_TMGI_ACTIVATION_IN_PROGRESS_V01,                   EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},         /*2*/
  {WDS_EMBMS_EEC_TMGI_DEACTIVATION_IN_PROGRESS_V01,                 EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {WDS_EMBMS_EEC_TMGI_NOT_SUPPORTED_V01,                            EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_EMBMS_EEC_TMGI_INVALID_V01,                                  EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},
  {WDS_EMBMS_EEC_TMGI_DEACTIVATION_IN_PROGRESS_V01,                 EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_TMGI_ACTIVATE_FAILURE_SAI_MISMATCH_V01,                      EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_TMGI_ACTIVATION_FAILURE_MAX_TMGI_ALREADY_ACTIVE_V01,         EmbmsDataCallFailCause_t::ERROR_MAX_TMGI_ALREADY_ACTIVE},
  {WDS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE_V01,                   EmbmsDataCallFailCause_t::SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE},
  {WDS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE_V01,                   EmbmsDataCallFailCause_t::SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED}
};

EmbmsDataCallFailCause_t EmbmsCallEndReason::getFailCause(int dsiReason) {
  if (dsiCallEndReasonMap.find(dsiReason)== dsiCallEndReasonMap.end()) {
     Log::getInstance().d("[EmbmsCallEndReason]: getCallEndReason code unknown");
     return EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  }
  return dsiCallEndReasonMap[dsiReason];
}

EmbmsCallHandler::EmbmsCallHandler(LocalLogBuffer& setLogBuffer): logBuffer(setLogBuffer) {
  Log::getInstance().d("[EmbmsCallHandler]: EmbmsCallHandler");
  dsiHandle = 0;
}

EmbmsCallHandler::~EmbmsCallHandler() {
  Log::getInstance().d("[EmbmsCallHandler]: ~EmbmsCallHandler");
  txnList.clear();
}

void EmbmsCallHandler::sendActivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                      uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendActivateTmgiResponse");
  embms_activate_tmgi_resp_msg_v01 pRespPacket= {};
  pRespPacket.call_id_valid      = TRUE;
  pRespPacket.call_id            = call_id;
  pRespPacket.resp_code          = (int32_t)failCause;
  pRespPacket.dbg_trace_id       = dbg_trace_id;
  pRespPacket.tmgi_info_valid    = TRUE;
  pRespPacket.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
  memcpy(&pRespPacket.tmgi_info.tmgi[0], &tmgi_info->tmgi[0], TMGI_LENGTH_MAX_V01);
  auto m = std::static_pointer_cast<RilRequestEmbmsActivateTmgiMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_activate_tmgi_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendDeactivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                    uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendDeactivateTmgiResponse");
  embms_deactivate_tmgi_resp_msg_v01 pRespPacket = {};
  pRespPacket.call_id_valid      = TRUE;
  pRespPacket.call_id            = call_id;;
  pRespPacket.resp_code          = (int32_t)failCause;
  pRespPacket.dbg_trace_id       = dbg_trace_id;
  pRespPacket.tmgi_info_valid    = TRUE;
  pRespPacket.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
  memcpy(&pRespPacket.tmgi_info.tmgi[0],
         &tmgi_info->tmgi[0],
         TMGI_LENGTH_MAX_V01);
  auto m = std::static_pointer_cast<RilRequestEmbmsDeactivateTmgiMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_deactivate_tmgi_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendActivateDeactivateTmgiResponse(embms_tmgi_type_v01 *act_tmgi_info, embms_tmgi_type_v01 *deact_tmgi_info,
        int32_t dbg_trace_id, uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendActivateDeactivateTmgiResponse");
  embms_activate_deactivate_tmgi_resp_msg_v01 pRespPacket= {};
  pRespPacket.call_id_valid      = TRUE;
  pRespPacket.call_id            = call_id;;
  pRespPacket.act_resp_code      = (uint16_t)failCause;
  pRespPacket.deact_resp_code    = (uint16_t)failCause;
  pRespPacket.dbg_trace_id       = dbg_trace_id;
  pRespPacket.act_tmgi_info_valid    = TRUE;
  pRespPacket.act_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
  memcpy(&pRespPacket.act_tmgi_info.tmgi[0],
         &act_tmgi_info[0],
         TMGI_LENGTH_MAX_V01);
  pRespPacket.deact_tmgi_info_valid    = TRUE;
  pRespPacket.deact_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
  memcpy(&pRespPacket.deact_tmgi_info.tmgi[0],
         &deact_tmgi_info[0],
         TMGI_LENGTH_MAX_V01);
  auto m = std::static_pointer_cast<RilRequestEmbmsActivateDeactivateTmgiMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_activate_deactivate_tmgi_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendContentDescUpdateResponse(int32_t dbg_trace_id, EmbmsDataCallFailCause_t failCause,
                                                    std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendContentDescUpdateResponse");
  embms_update_content_desc_resp_msg_v01 pRespPacket = {};
  pRespPacket.resp_code          = (int32_t)failCause;
  pRespPacket.dbg_trace_id       = dbg_trace_id;
  auto m = std::static_pointer_cast<RilRequestEmbmsContentDescUpdateMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_update_content_desc_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendGetActiveTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                                 EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendGetActiveTmgiResponse");
  embms_get_active_tmgi_resp_msg_v01 pRespPacket= {};
  pRespPacket.resp_code          = (int32_t)failCause;
  pRespPacket.dbg_trace_id       = dbg_trace_id;
  if (EmbmsDataCallFailCause_t::ERROR_NONE == failCause) {
    pRespPacket.tmgi_info_valid = TRUE;

    /* validate length of tmgi_list */
    pRespPacket.tmgi_info_len =
       (embms_tmgi->tmgi_list.tmgi_list_len > NUMBER_MAX_V01) ?
       NUMBER_MAX_V01 : embms_tmgi->tmgi_list.tmgi_list_len;

    for (int j = 0; j < pRespPacket.tmgi_info_len; j++)
    {
      pRespPacket.tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&(pRespPacket.tmgi_info[j].tmgi[0]),
             &(embms_tmgi->tmgi_list.tmgi_list[j].tmgi[0]),
             TMGI_LENGTH_MAX_V01);
    }
  } else {
    pRespPacket.tmgi_info_valid = FALSE;
  }
  auto m = std::static_pointer_cast<RilRequestEmbmsGetActiveTmgiMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_get_active_tmgi_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendInterestedListResponse(int32_t dbg_trace_id, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendInterestedList");
  embms_get_interested_tmgi_list_resp_ack_msg_v01 pRespPacket = {};
  pRespPacket.dbg_trace_id = dbg_trace_id;
  auto m = std::static_pointer_cast<RilRequestEmbmsSendIntTmgiListMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_get_interested_tmgi_list_resp_ack_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::sendGetAvailableTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                    EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendGetAvailableTmgiResponse");
  embms_get_available_tmgi_resp_msg_v01 pRespPacket = {};
  pRespPacket.resp_code = (int32_t)failCause;
  pRespPacket.dbg_trace_id = dbg_trace_id;
  if (EmbmsDataCallFailCause_t::ERROR_NONE == failCause) {
    pRespPacket.tmgi_info_valid = TRUE;

    /* validate length of tmgi_list */
    pRespPacket.tmgi_info_len = (embms_tmgi->tmgi_list.tmgi_list_len > NUMBER_MAX_V01) ?
                                   NUMBER_MAX_V01 : embms_tmgi->tmgi_list.tmgi_list_len;

    for (int j = 0; j < pRespPacket.tmgi_info_len; j++)
    {
      pRespPacket.tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&(pRespPacket.tmgi_info[j].tmgi[0]),
             &(embms_tmgi->tmgi_list.tmgi_list[j].tmgi[0]),
             TMGI_LENGTH_MAX_V01);
    }
  } else {
    pRespPacket.tmgi_info_valid = FALSE;
  }
  auto m = std::static_pointer_cast<RilRequestEmbmsGetAvailTmgiMessage>(msg);
  if (m) {
    auto resp = std::make_shared<embms_get_available_tmgi_resp_msg_v01>(pRespPacket);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }
}

void EmbmsCallHandler::activateTmgi(embms_activate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: activateTmgi");
  int tmgiDeactivateReason = 0;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;

  std::shared_ptr<rildata::tmgiRequestType> tmgiInfo = std::make_shared<rildata::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }

  tmgiInfo->cId = req.call_id;
  tmgiInfo->pend_req = requestType::EMBMS_ACTIVATE_TMGI;
  tmgiInfo->message = msg;
  memcpy((char *)(&tmgiInfo->tmgi.tmgi),
         (char *)(&req.tmgi_info.tmgi),
         EMBMS_TMGI_LEN );

  if (DSI_SUCCESS != dsiTmgiActivate(dsiHandle,
                                     (char *)(&req.tmgi_info.tmgi[0]),
                                     (unsigned int*)&req.earfcnlist[0],
                                     req.earfcnlist_len,
                                     (unsigned long)req.preemption_priority,
                                     (unsigned int*)&req.saiList[0],
                                     (unsigned char)req.saiList_len,
                                     req.dbg_trace_id)) {
     Log::getInstance().d("unable to activate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_ACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS"+std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }
     sendActivateTmgiResponse(&req.tmgi_info, req.dbg_trace_id, req.call_id, failCause, msg);
  }
  txnList.push_back(tmgiInfo);
}

void EmbmsCallHandler::deactivateTmgi(embms_deactivate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: deactivateTmgi");
  int tmgiDeactivateReason = 0;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  std::shared_ptr<rildata::tmgiRequestType> tmgiInfo = std::make_shared<rildata::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }

  tmgiInfo->cId = req.call_id;
  tmgiInfo->pend_req = requestType::EMBMS_DEACTIVATE_TMGI;
  tmgiInfo->message = msg;
  memcpy((char *)(&tmgiInfo->deact_tmgi.tmgi),
         (char *)(&req.tmgi_info.tmgi),
         EMBMS_TMGI_LEN );


  if (DSI_SUCCESS != dsiTmgiDeactivate(dsiHandle,
                                       (char *)(&req.tmgi_info.tmgi[0]),
                                       req.dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_DEACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS:"+ std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }
     sendDeactivateTmgiResponse(&req.tmgi_info, req.dbg_trace_id, req.call_id, failCause, msg);
  }
  txnList.push_back(tmgiInfo);
}

void EmbmsCallHandler::activateDeactivateTmgi(embms_activate_deactivate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: activateDeactivateTmgi");
  int tmgiDeactivateReason = 0;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  std::shared_ptr<rildata::tmgiRequestType> tmgiInfo = std::make_shared<rildata::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }
  tmgiInfo->cId = req.call_id;
  tmgiInfo->pend_req = requestType::EMBMS_ACTIVATE_DEACTIVATE_TMGI;
  tmgiInfo->message = msg;

  memcpy((char *)(&tmgiInfo->tmgi.tmgi),
         (char *)(&req.act_tmgi_info.tmgi),
         EMBMS_TMGI_LEN);
  memcpy((char *)(&tmgiInfo->deact_tmgi.tmgi),
         (char *)(&req.deact_tmgi_info.tmgi),
         EMBMS_TMGI_LEN);

  if (DSI_SUCCESS != dsiTmgiActivateDeactivate(dsiHandle,
                                     (char *)(&req.act_tmgi_info.tmgi[0]),
                                     (unsigned int*)&req.earfcnlist[0],
                                     req.earfcnlist_len,
                                     (unsigned long)req.preemption_priority,
                                     (char *)(&req.deact_tmgi_info.tmgi[0]),
                                     (unsigned int*)&req.saiList[0],
                                     (unsigned char)req.saiList_len,
                                     req.dbg_trace_id)) {
     Log::getInstance().d("unable to activate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_ACTIVATE_DEACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS:" + std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }
     sendActivateDeactivateTmgiResponse(&req.act_tmgi_info, &req.deact_tmgi_info, req.dbg_trace_id, req.call_id, failCause, msg);
  }
  txnList.push_back(tmgiInfo);
}

void EmbmsCallHandler::contentDescUpdate(embms_update_content_desc_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: contentDescUpdate");
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;

  if (DSI_SUCCESS != dsiContentDescUpdate(dsiHandle,
                                          (char *)(&req.tmgi_info.tmgi[0]),
                                          (unsigned char)req.content_desc_valid,
                                          req.content_desc_len,
                                          (dsi_embms_content_desc_type *)req.content_desc,
                                          req.dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  }
  sendContentDescUpdateResponse(req.dbg_trace_id, failCause, msg);
}

void EmbmsCallHandler::getActiveTmgi(embms_get_active_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: getActiveTmgi");
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;

  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));

  if (DSI_SUCCESS !=  dsiGetEmbmsTmgiListquery(dsiHandle,
                                                WDS_EMBMS_TMGI_LIST_ACTIVE_V01,
                                                &embms_tmgi,
                                                req.dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  }
  sendGetActiveTmgiResponse(req.dbg_trace_id, &embms_tmgi, failCause, msg);
}

void EmbmsCallHandler::getAvailableTmgi(embms_get_available_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: getAvailableTmgi");
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;
  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));
  if (DSI_SUCCESS !=  dsiGetEmbmsTmgiListquery(dsiHandle,
                                                WDS_EMBMS_TMGI_LIST_AVAILABLE_V01,
                                                &embms_tmgi,
                                                req.dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  }
  sendGetAvailableTmgiResponse(req.dbg_trace_id, &embms_tmgi, failCause, msg);
}

void EmbmsCallHandler::sendInterestedList(embms_get_interested_tmgi_list_resp_msg_v01 req, std::shared_ptr<Message> msg) {
  Log::getInstance().d("[EmbmsCallHandler]: sendInterestedList");
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  char **tmgi_ptr_list = NULL;

  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));

  if(req.tmgi_info_len > 0)
  {
    tmgi_ptr_list = (char **)malloc(req.tmgi_info_len * sizeof(char *));
    if(tmgi_ptr_list == NULL)
    {
      Log::getInstance().d( "failed to allocate memory" );
      return;
    }

    /* Send the tmgi list as a ptr to an array of tmgi ptrs */
    memset(tmgi_ptr_list, 0, req.tmgi_info_len * sizeof(char *));
    for(int j = 0; j < req.tmgi_info_len; j++)
    {
      tmgi_ptr_list[j] = (char *)&req.tmgi_info[j].tmgi[0];
    }
  }

  if (DSI_SUCCESS != dsiEmbmsSvcInterestList(dsiHandle,
                                             tmgi_ptr_list,
                                             req.tmgi_info_len,
                                             req.dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
  }
  sendInterestedListResponse(req.dbg_trace_id, msg);
}

int EmbmsCallHandler::isEmbmsActive(void) {
  Log::getInstance().d("[EmbmsCallHandler]: isEmbmsActive");
  dsi_call_tech_type callTech;

  if ((DSI_SUCCESS == dsi_get_call_tech(dsiHandle, &callTech)) &&
      (callTech == DSI_EXT_TECH_EMBMS)) {
        return TRUE;
  }
  return FALSE;
}

void EmbmsCallHandler::processTmgiActivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
  Log::getInstance().d("[EmbmsCallHandler]: processTmgiActivatedEvent");
  std::list<std::shared_ptr<rildata::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<rildata::tmgiRequestType> req = *it;

    if (req->pend_req == requestType::EMBMS_ACTIVATE_TMGI) {
       if (memcmp(req->tmgi.tmgi,
                 embms_payload->embms_act_info.tmgi.tmgi,
                 sizeof(req->tmgi.tmgi)) == 0) {
          Log::getInstance().d("ACTIVATED EVENT, TMGI match occured");
          embms_tmgi_type_v01 tmgi_val;
          memcpy(&tmgi_val.tmgi, embms_payload->embms_act_info.tmgi.tmgi, EMBMS_TMGI_LEN);
          tmgi_val.tmgi_len = sizeof(tmgi_val.tmgi);
          sendActivateTmgiResponse(&tmgi_val, embms_payload->embms_act_info.tranx_id,
                        req->cId, EmbmsDataCallFailCause_t::ERROR_NONE, req->message);
          it = txnList.erase(it);
          return;
       } else {
         it++;
       }
    } else if (req->pend_req == requestType::EMBMS_DEACTIVATE_TMGI) {
        if ((memcmp(req->deact_tmgi.tmgi,
                   embms_payload->embms_deact_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
          Log::getInstance().d( "Activated received when expecting a "
                           "deactivated indication, Dropping Indication");
          return;
        } else {
          it++;
        }
    } else {
      it++;
    }
  }
}


void EmbmsCallHandler::processTmgiDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
  Log::getInstance().d("[EmbmsCallHandler]: processTmgiDeactivatedEvent");
  std::list<std::shared_ptr<rildata::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<rildata::tmgiRequestType> req = *it;

    if (req->pend_req == requestType::EMBMS_DEACTIVATE_TMGI) {
       Log::getInstance().d("DEACTIVATED EVENT, TMGI match occured");
       if ((memcmp(req->deact_tmgi.tmgi,
                   embms_payload->embms_deact_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
            Log::getInstance().d("sending response");
            sendDeactivateTmgiResponse(&req->deact_tmgi, embms_payload->embms_deact_info.tranx_id,
                                        req->cId, EmbmsDataCallFailCause_t::ERROR_NONE, req->message);
          it = txnList.erase(it);
          return;
       } else {
         it++;
       }
    } else if (req->pend_req == requestType::EMBMS_ACTIVATE_TMGI) {
        Log::getInstance().d("Found a Tmgi Activated Event");
        if ((memcmp(req->tmgi.tmgi,
                   embms_payload->embms_act_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
          EmbmsDataCallFailCause_t failCause = EmbmsCallEndReason::getFailCause(embms_payload->embms_act_info.activate_status);
          Log::getInstance().d("sending response");
          sendActivateTmgiResponse(&req->tmgi, embms_payload->embms_act_info.tranx_id,
                                                     req->cId, failCause, req->message);
          it = txnList.erase(it);
          return;
        } else {
          it++;
        }
    } else {
      it++;
    }
  }
}

void EmbmsCallHandler::processTmgiActivatedDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
   Log::getInstance().d("[EmbmsCallHandler]: processTmgiActivatedDeactivatedEvent");
  std::list<std::shared_ptr<rildata::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<rildata::tmgiRequestType> req = *it;

    if (req->pend_req == requestType::EMBMS_ACTIVATE_DEACTIVATE_TMGI) {
       Log::getInstance().d("ACTIVATED_DEACTIVATED EVENT, TMGI match occured");
       EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
       if ( ((memcmp(req->deact_tmgi.tmgi,
                     embms_payload->embms_act_deact_info.deact_tmgi.tmgi,
                     EMBMS_TMGI_LEN)) == 0) &&
              ((memcmp(req->tmgi.tmgi,
                     embms_payload->embms_act_deact_info.act_tmgi.tmgi,
                     EMBMS_TMGI_LEN)) == 0) ) {
           RIL_Errno err_no;
           if((WDS_TMGI_ACTIVATE_SUCCESS_V01 ==
               embms_payload->embms_act_deact_info.act_status) &&
              (WDS_TMGI_DEACTIVATE_SUCCESS_V01 ==
               embms_payload->embms_act_deact_info.deact_status)
              )
           {
             err_no = RIL_E_SUCCESS;
           }
           else
           {
             failCause = EmbmsCallEndReason::getFailCause(embms_payload->embms_act_deact_info.act_status);
             err_no = RIL_E_GENERIC_FAILURE;
           }
           sendActivateDeactivateTmgiResponse(&req->tmgi, &req->deact_tmgi,
                    embms_payload->embms_deact_info.tranx_id, req->cId, failCause, req->message);
           it = txnList.erase(it);
           return;
       } else {
          it++;
       }
    } else {
      it ++;
    }
  }
}


void EmbmsCallHandler::processEvents(dsi_net_evt_t evt, dsi_embms_tmgi_info_type *embmsPayload) {
   Log::getInstance().d("[EmbmsCallHandler]: processEvents");
  embms_unsol_active_tmgi_ind_msg_v01 active_indication;

   switch(evt)
   {
     case DSI_NET_SAI_LIST_CHANGED:
     {
       embms_unsol_sai_ind_msg_v01  sai_indication;
       unsigned int list_index;
       unsigned int freq_index;
       unsigned int camped_index;
       unsigned int avail_index;
       unsigned short  list_len;
       unsigned char   sai_size = 0;
       unsigned short  camped_sai_list_size = 0;
       unsigned short  available_sai_list_size = 0;

       Log::getInstance().d(">>>DSI_NET_SAI_LIST_CHANGED: START>>>");

       if(embmsPayload->embms_sai_info.freq_sai_list_valid != TRUE &&
          embmsPayload->embms_sai_info.freq_sai_list_ex_valid != TRUE)
       {
         Log::getInstance().d("invalid sai_list, Drop SAI_LIST indication");
         return;
       }

       list_len = (embmsPayload->embms_sai_info.freq_sai_list_ex_valid)?
                   embmsPayload->embms_sai_info.freq_sai_list_ex_len:
                   embmsPayload->embms_sai_info.freq_sai_list_len;

       /* validate number of freqencies */
       list_len = (unsigned short)((list_len > QMI_WDS_EMBMS_FREQ_SAI_MAX_V01)?
                                   QMI_WDS_EMBMS_FREQ_SAI_MAX_V01: list_len);

       /* calculate the size for arrays */
       for (list_index = 0; list_index < list_len; list_index ++)
       {
         sai_size = (embmsPayload->embms_sai_info.freq_sai_list_ex_valid)?
                     embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].sai_list_len:
                     embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list_len;
         /* validate sai_size */
         if (sai_size > QMI_WDS_EMBMS_SAI_LIST_MAX_V01)
         {
           Log::getInstance().d("sai_list_len_per_freq is invalid, Drop SAI_LIST indication: " + std::to_string(sai_size));
           return;
         }

         available_sai_list_size = (unsigned short) (available_sai_list_size + sai_size);
         if ((embmsPayload->embms_sai_info.freq_sai_list_ex_valid &&
              TRUE == embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].is_serving_frequency) ||
             (embmsPayload->embms_sai_info.freq_sai_list_valid &&
              TRUE == embmsPayload->embms_sai_info.freq_sai_list[list_index].is_serving_frequency))
         {
           camped_sai_list_size = (unsigned short) (camped_sai_list_size + sai_size);
         }
       }

       memset(&sai_indication, 0, sizeof(sai_indication));
       sai_indication.available_sai_list_len = available_sai_list_size;
       sai_indication.camped_sai_list_len = camped_sai_list_size;
       sai_indication.num_of_sai_per_group_len = list_len;

       /* validate available_sai_list_len */
       if (sai_indication.available_sai_list_len > SAI_MAX_V01)
       {
         Log::getInstance().d(" Oversize available_sai_list_len, Drop SAI_LIST indication: "+
                         std::to_string(sai_indication.available_sai_list_len));
         return;
       }
       /* validate camped_sai_list_len */
       if (sai_indication.camped_sai_list_len > SAI_PER_FREQ_MAX_V01)
       {
         Log::getInstance().d(" Oversize camped_sai_list_len, Drop SAI_LIST indication: " +
                         std::to_string(sai_indication.camped_sai_list_len));
         return;
       }
       /* validate num_of_sai_per_group_len */
       if (sai_indication.num_of_sai_per_group_len > FREQ_MAX_V01)
       {
         Log::getInstance().d(" Oversize num_of_sai_per_group_len, Drop SAI_LIST indication: " +
                         std::to_string(sai_indication.num_of_sai_per_group_len));
         return;
       }

       camped_index = avail_index = 0;
       for (list_index = 0; list_index < sai_indication.num_of_sai_per_group_len; list_index ++)
       {
         sai_size = (embmsPayload->embms_sai_info.freq_sai_list_ex_valid)?
                     embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].sai_list_len:
                     embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list_len;
         for(freq_index = 0; freq_index < sai_size; freq_index++)
         {
           if(embmsPayload->embms_sai_info.freq_sai_list_ex_valid)
           {
             sai_indication.available_sai_list[ avail_index ++ ] =
               embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].sai_list[freq_index];
             if(TRUE == embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].is_serving_frequency)
             {
               sai_indication.camped_sai_list[ camped_index ++ ] =
                 embmsPayload->embms_sai_info.freq_sai_list_ex[list_index].sai_list[freq_index];
             }
           }
           else
           {
             //embmsPayload->embms_sai_info.freq_sai_list_valid
             sai_indication.available_sai_list[ avail_index ++ ] =
               embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list[freq_index];
             if(TRUE == embmsPayload->embms_sai_info.freq_sai_list[list_index].is_serving_frequency)
             {
               sai_indication.camped_sai_list[ camped_index ++ ] =
                 embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list[freq_index];
             }
           }
         }
         sai_indication.num_of_sai_per_group[list_index] = sai_size;
       }

       if(embmsPayload->embms_sai_info.tranx_id_valid)
       {
         sai_indication.dbg_trace_id = embmsPayload->embms_sai_info.tranx_id;
       }

       Log::getInstance().d( "EMBMS SAI list indication, available_sai_list_len: " + std::to_string(sai_indication.available_sai_list_len) +
                       "camped_sai_list_len: " + std::to_string(sai_indication.camped_sai_list_len) +
                       "num_of_sai_per_group_len: " + std::to_string(sai_indication.num_of_sai_per_group_len) +
                       "dbg_trace_id: " + std::to_string(sai_indication.dbg_trace_id));

       unsigned int i;
       for (i = 0; i < sai_indication.available_sai_list_len; i++)
       {
          Log::getInstance().d("available_sai_list: " + std::to_string(sai_indication.available_sai_list[i]));
       }
       for (i = 0; i < sai_indication.camped_sai_list_len; i++)
       {
          Log::getInstance().d("camped_sai_list: " + std::to_string(sai_indication.camped_sai_list[i]));
       }
       for (i = 0; i < sai_indication.num_of_sai_per_group_len; i++)
       {
          Log::getInstance().d("num_of_sai_per_group: " + std::to_string(sai_indication.num_of_sai_per_group[i]));
       }
       auto msg = std::make_shared<SaiListChangeIndMessage>(sai_indication);
       if (msg){
           msg->broadcast();
       }
     }
     break;

     case DSI_NET_TMGI_LIST_CHANGED:
     {
       Log::getInstance().d(">>>DSI_NET_TMGI_LIST_CHANGED: START>>> list indication type :" +
                            std::to_string(embmsPayload->embms_list_info.tmgi_list.list_type));

       if(embmsPayload->embms_list_info.tmgi_list_valid != TRUE)
       {
         Log::getInstance().d( "invalid tmgi_list, Dropping TMGI list indication" );
         return;
       }

       switch(embmsPayload->embms_list_info.tmgi_list.list_type)
       {
         case WDS_EMBMS_TMGI_LIST_ACTIVE_V01:
           {
             int index = 0;
             memset(&active_indication, 0, sizeof(active_indication));
             active_indication.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(active_indication.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication: " +
                                  std::to_string(active_indication.tmgi_info_len));
               return;
             }

             for (index = 0; index < (int)active_indication.tmgi_info_len; index++)
             {
               active_indication.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(active_indication.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               active_indication.dbg_trace_id =
                 embmsPayload->embms_list_info.tranx_id;
             }

             Log::getInstance().d( "EMBMS ACTIVE list ind debug_id" + std::to_string(active_indication.dbg_trace_id) +
                                   "num TMGI's " + std::to_string(active_indication.tmgi_info_len));

             auto msg = std::make_shared<TMGIListChangeIndMessage>(active_indication);
             if (msg) {
                 msg->broadcast();
             }
           }
           break;

         case WDS_EMBMS_TMGI_LIST_OOS_WARNING_V01:
           {
             int index = 0;
             embms_unsol_oos_warning_ind_msg_v01   oos_warning_ind;
             memset(&oos_warning_ind, 0, sizeof(oos_warning_ind));
             oos_warning_ind.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(oos_warning_ind.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication:" +
                                  std::to_string(oos_warning_ind.tmgi_info_len) );
               return;
             }

             for (index = 0; index < (int)oos_warning_ind.tmgi_info_len; index++)
             {
               oos_warning_ind.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(oos_warning_ind.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               oos_warning_ind.dbg_trace_id =
                 embmsPayload->embms_list_info.tranx_id;
             }
             oos_warning_ind.reason = embmsPayload->embms_list_info.warn_reason;

             Log::getInstance().d( "EMBMS OOS warning ind " + std::to_string(oos_warning_ind.reason) +
                                   "debug_id " + std::to_string(oos_warning_ind.dbg_trace_id) +
                                  "num TMGI's " + std::to_string(oos_warning_ind.tmgi_info_len));
             auto msg = std::make_shared<TMGIListOOSWarningIndMessage>(oos_warning_ind);
             if (msg) {
                msg->broadcast();
             }
           }
           break;

         case WDS_EMBMS_TMGI_LIST_AVAILABLE_V01:
           {
             embms_unsol_available_tmgi_ind_msg_v01 available_tmgi_list;
             int index = 0;

             memset(&available_tmgi_list, 0, sizeof(available_tmgi_list));
             available_tmgi_list.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(available_tmgi_list.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication:" +
                                  std::to_string(available_tmgi_list.tmgi_info_len));
               return;
             }

             for (index = 0; index < (int)available_tmgi_list.tmgi_info_len; index++)
             {
               available_tmgi_list.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(available_tmgi_list.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               available_tmgi_list.dbg_trace_id = embmsPayload->embms_list_info.tranx_id;
             }

             Log::getInstance().d( "EMBMS AVAILABLE list ind debug_id" + std::to_string(available_tmgi_list.dbg_trace_id) +
                                   "num TMGI's" + std::to_string(available_tmgi_list.tmgi_info_len));

             auto msg = std::make_shared<TMGIAvailableIndMessage>(available_tmgi_list);
             if (msg) {
                msg->broadcast();
             }
           }
           break;

         default:
           break;
       }
     }
     break;

     case DSI_NET_TMGI_DEACTIVATED:
       processTmgiDeactivatedEvent(embmsPayload);
       break;

     case DSI_NET_TMGI_ACTIVATED_DEACTIVATED:
       processTmgiActivatedDeactivatedEvent(embmsPayload);
       break;

     case DSI_NET_TMGI_ACTIVATED:
       processTmgiActivatedEvent(embmsPayload);
       break;

     case DSI_NET_CONTENT_DESC_CONTROL:
     {
       embms_unsol_content_desc_update_per_obj_ind_msg_v01 content_desc_ctrl_ind;
       Log::getInstance().d(">>>DSI_NET_CONTENT_DESC_CONTROL: START>>>");

       memset(&content_desc_ctrl_ind, 0, sizeof(content_desc_ctrl_ind));
       memcpy(&(content_desc_ctrl_ind.tmgi_info.tmgi),
              &(embmsPayload->embms_content_desc_info.tmgi.tmgi),
              TMGI_LENGTH_MAX_V01);

       content_desc_ctrl_ind.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
       if(embmsPayload->embms_content_desc_info.tranx_id_valid)
       {
         content_desc_ctrl_ind.dbg_trace_id = embmsPayload->embms_content_desc_info.tranx_id;
       }
       if (embmsPayload->embms_content_desc_info.content_control_valid)
       {
         content_desc_ctrl_ind.per_object_content_ctrl_valid = TRUE;
         content_desc_ctrl_ind.per_object_content_ctrl =
             embmsPayload->embms_content_desc_info.content_control;
       }
       if (embmsPayload->embms_content_desc_info.status_control_valid)
       {
         content_desc_ctrl_ind.per_object_status_ctrl_valid = TRUE;
         content_desc_ctrl_ind.per_object_status_ctrl =
             embmsPayload->embms_content_desc_info.status_control;
       }
       Log::getInstance().d( "EMBMS content desc control indication status_control: " + std::to_string(content_desc_ctrl_ind.per_object_status_ctrl) +
                       "valid: " + std::to_string(content_desc_ctrl_ind.per_object_status_ctrl_valid) +
                       "content_control: " + std::to_string(content_desc_ctrl_ind.per_object_content_ctrl) +
                       "valid: " + std::to_string(content_desc_ctrl_ind.per_object_content_ctrl_valid) +
                       "dbg_trace_id: " + std::to_string(content_desc_ctrl_ind.dbg_trace_id) +
                       "tmgi_list len " + std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi_len) +
                       "tmgi_list :" + std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[0]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[1]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[2]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[3]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[4]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[5]));

       auto msg = std::make_shared<ContentDescUpdateIndMessage>(content_desc_ctrl_ind);
       if (msg) {
         msg->broadcast();
       }
     }
     break;

     case DSI_NET_TMGI_SERVICE_INTERESTED:
     {
       embms_unsol_get_interested_tmgi_list_req_msg_v01 get_interested_tmgi_list;
       Log::getInstance().d(">>>DSI_NET_TMGI_SERVICE_INTERESTED: START>>>");
       memset(&get_interested_tmgi_list, 0, sizeof(get_interested_tmgi_list));

       if(embmsPayload->embms_svc_interest_info.tranx_id_valid)
       {
         get_interested_tmgi_list.dbg_trace_id =
             embmsPayload->embms_svc_interest_info.tranx_id;
       }

       Log::getInstance().d("EMBMS get interested TMGI list, dbg_trace_id: "+
                             std::to_string(get_interested_tmgi_list.dbg_trace_id));

       auto msg = std::make_shared<InterestedTMGIListIndMessage>(get_interested_tmgi_list);
       if (msg) {
         msg->broadcast();
       }
     }
     break;

     default:
       Log::getInstance().d("Unknown embms event received: " + std::to_string(evt));
       break;
   } /*switch(evt)*/
}

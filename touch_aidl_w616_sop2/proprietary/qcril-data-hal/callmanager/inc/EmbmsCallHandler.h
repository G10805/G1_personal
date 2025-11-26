/*===========================================================================

  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef EMBMSCALLHANDLER
#define EMBMSCALLHANDLER

#include <stdint.h>
#include <set>
#include <unordered_map>
#include "framework/Message.h"
#include "qcril_data.h"
#include "event/RilEventDataCallback.h"

#include "LocalLogBuffer.h"
#include "DsiWrapper.h"

#include "qmi_embms_v01.h"


#define EMBMS_TMGI_LEN 6
/**
 * @brief EMBMS information class
 * @details EMBMS information class
 */
namespace rildata {

typedef enum
{
  EMBMS_ACTIVATE_TMGI,
  EMBMS_DEACTIVATE_TMGI,
  EMBMS_ACTIVATE_DEACTIVATE_TMGI,
  EMBMS_GET_AVAILABLE_TMGI,
  EMBMS_GET_ACTIVE_TMGI,
  EMBMS_CONTENT_DESC_UPDATE,
  EMBMS_SEND_INTERESTED_TMGI_LIST,
}requestType;

typedef struct
{
  int                      cId;
  requestType              pend_req;
  embms_tmgi_type_v01      tmgi;
  embms_tmgi_type_v01      deact_tmgi;
  std::shared_ptr<Message> message;
} tmgiRequestType;

}

class EmbmsCallEndReason {
public:
  static EmbmsDataCallFailCause_t getFailCause(int dsiReason);
private:
  static std::unordered_map<int, EmbmsDataCallFailCause_t> dsiCallEndReasonMap;
};

class EmbmsCallHandler {
public:
  EmbmsCallHandler(LocalLogBuffer& logBuffer);
  ~EmbmsCallHandler();

  void activateTmgi(embms_activate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg);
  void deactivateTmgi(embms_deactivate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg);
  void activateDeactivateTmgi(embms_activate_deactivate_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg);
  void contentDescUpdate(embms_update_content_desc_req_msg_v01 req, std::shared_ptr<Message> msg);
  void sendInterestedList(embms_get_interested_tmgi_list_resp_msg_v01 req, std::shared_ptr<Message> msg);
  void getActiveTmgi(embms_get_active_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg);
  void getAvailableTmgi(embms_get_available_tmgi_req_msg_v01 req, std::shared_ptr<Message> msg);
  void checkIsActive();
  void processEvents(dsi_net_evt_t evt, dsi_embms_tmgi_info_type *embms_payload);
  void setDsiHandle(dsi_hndl_t handle){dsiHandle = handle;}
  int isEmbmsActive(void);

private:
  LocalLogBuffer& logBuffer;
  dsi_hndl_t dsiHandle;
  int instanceId;
  std::list<std::shared_ptr<rildata::tmgiRequestType>> txnList;
  void processTmgiDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
  void processTmgiActivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
  void processTmgiActivatedDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload);
  void sendActivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
        uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendDeactivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
        uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendActivateDeactivateTmgiResponse(embms_tmgi_type_v01 *act_tmgi_info, embms_tmgi_type_v01 *deact_tmgi_info,
        int32_t dbg_trace_id, uint8_t call_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendContentDescUpdateResponse(int32_t dbg_trace_id, EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendGetActiveTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                 EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendGetAvailableTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                    EmbmsDataCallFailCause_t failCause, std::shared_ptr<Message> msg);
  void sendInterestedListResponse(int32_t dbg_trace_id, std::shared_ptr<Message> msg);
};

#endif
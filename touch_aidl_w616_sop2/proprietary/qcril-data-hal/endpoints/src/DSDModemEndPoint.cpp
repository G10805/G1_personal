/******************************************************************************
#  Copyright (c) 2018-2019, 2021-2023 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "DSDModemEndPoint.h"
#include "sync/RegisterForSystemStatusSyncMessage.h"
#include "sync/TurnOnAPAssistIWLANSyncMessage.h"
#include "sync/RegisterForRoamingStatusChangedMessage.h"
#include "sync/TriggerDDSSwitchSyncMessage.h"
#include "request/GetDsdSystemStatusMessage.h"
#include "request/GetDataNrIconTypeMessage.h"
#include "UnSolMessages/DDSSwitchResultIndMessage.h"
#include "UnSolMessages/DsdSystemStatusMessage.h"
#include "UnSolMessages/DsdSystemStatusPerApnMessage.h"
#include "UnSolMessages/DataNrIconTypeIndMessage.h"
#include "UnSolMessages/SetCapabilitiesMessage.h"
#include "UnSolMessages/AutoDdsSwitchControlIndMessage.h"
#include "UnSolMessages/DataCapabilityChangeIndMessage.h"
#include "MessageCommon.h"
#include <sstream>

using std::to_string;
using namespace rildata;
constexpr const char *DSDModemEndPoint::NAME;
bool DSDModemEndPoint::mUserDdsSwitchSelection = false;

void DSDModemEndPoint::requestSetup(string clientToken, qcril_instance_id_e_type id,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, id, callback);
  mModule->dispatch(shared_setupMsg);
  mUserDdsSwitchSelection = false;
}

boolean DSDModemEndPoint::isApnTypeFound
(
  const ApnTypes_t &apn_type, const ApnTypes_t &apn_types_mask
)
{
  if (((int32_t)apn_type & (int32_t)apn_types_mask) == (int32_t)apn_type)
  {
    std::stringstream ss;
    ss << "apn type " << apn_type << " was enabled";
    Log::getInstance().d(ss.str());
    return TRUE;
  }
  return FALSE;
}

void DSDModemEndPoint::sendApnInfoQmi
(
  const std::string &apn_name,
  dsd_apn_type_enum_v01 apn_type
)
{
  Log::getInstance().d("DSDModemEndPoint::sendApnInfoQmi ENTRY");
  dsd_set_apn_info_req_msg_v01 qmiReq;
  dsd_set_apn_info_resp_msg_v01 qmiResp;

  memset(&qmiReq, 0, sizeof(qmiReq));
  memset(&qmiResp, 0, sizeof(qmiResp));
  strlcpy(&qmiReq.apn_info.apn_name[0], apn_name.c_str(), QMI_DSD_MAX_APN_LEN_V01);

  // indicate that the apn_invalid flag is set
  qmiReq.apn_invalid_valid = FALSE;
  qmiReq.apn_info.apn_type = apn_type;

  auto retVal = sendRawSync(QMI_DSD_SET_APN_INFO_REQ_V01,
                            (void *)&qmiReq, sizeof(qmiReq),
                            (void *)&qmiResp, sizeof(qmiResp),
                            QCRIL_DATA_QMI_TIMEOUT);
  if (retVal != QMI_NO_ERR) {
    Log::getInstance().d("[DSDModemEndPoint] Failed to send"
      "QMI_DSD_SET_APN_INFO_REQ_V01 for apn_name:"+apn_name+
      ", type:"+std::to_string(apn_type)+"with rc ="+std::to_string(retVal));
  } else {
    Log::getInstance().d("[DSDModemEndPoint] sendApnInfoQmi::Successfully sent"
      "QMI_DSD_SET_APN_INFO_REQ_V01 for type ="+ std::to_string(apn_type));
  }
}

Message::Callback::Status DSDModemEndPoint::setApnInfoSync
( const std::string apn_name,
  const ApnTypes_t apnTypesBitmap)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    std::stringstream ss;
    ss << "[DSDModemEndPoint::setApnInfoSync]: apn=" << apn_name << " types=" << apnTypesBitmap;
    Log::getInstance().d(ss.str());

    do
    {
      if ((int32_t)apnTypesBitmap <= 0)
      {
        Log::getInstance().d("Invalid inputs");
        return Message::Callback::Status::SUCCESS;
      }
      if (isApnTypeFound(ApnTypes_t::DEFAULT, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DEFAULT_V01);
      }
      if (isApnTypeFound(ApnTypes_t::IMS, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IMS_V01);
      }
      if (isApnTypeFound(ApnTypes_t::MMS, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_MMS_V01);
      }
      if (isApnTypeFound(ApnTypes_t::DUN, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_DUN_V01);
      }
      if (isApnTypeFound(ApnTypes_t::SUPL, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_SUPL_V01);
      }
      if (isApnTypeFound(ApnTypes_t::HIPRI, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_HIPRI_V01);
      }
      if (isApnTypeFound(ApnTypes_t::FOTA, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_FOTA_V01);
      }
      if (isApnTypeFound(ApnTypes_t::CBS, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_CBS_V01);
      }
      if (isApnTypeFound(ApnTypes_t::IA, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_IA_V01);
      }
      if (isApnTypeFound(ApnTypes_t::EMERGENCY, apnTypesBitmap))
      {
        sendApnInfoQmi(apn_name, DSD_APN_TYPE_EMERGENCY_V01);
      }
    } while(0);
    return Message::Callback::Status::SUCCESS;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::handleSetApnInfo]"
                         "Failed to send message SetApnInfoSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::sendAPAssistIWLANSupportedSync ( )
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::sendAPAssistIWLANSupportedSync]");
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<TurnOnAPAssistIWLANSyncMessage>(nullptr);
    Message::Callback::Status apiStatus =  Message::Callback::Status::FAILURE;
    auto r = std::make_shared<bool>();
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(r);
    }
    std::ostringstream ss;
    ss << "[DSDModemEndPoint::sendAPAssistIWLANSupportedSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::sendAPAssistIWLANSupportedSync]"
                         "Failed to send message TurnOnAPAssistIWLANSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

void DSDModemEndPoint::generateDsdSystemStatusInd()
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::generateDsdSystemStatusInd]"
                         "dispatching message GetDsdSystemStatusMessage");
    auto msg = std::make_shared<GetDsdSystemStatusMessage>();
    GenericCallback<DsdSystemStatusResult_t> cb([](std::shared_ptr<Message>, Message::Callback::Status status,
                            std::shared_ptr<DsdSystemStatusResult_t> r) -> void {
      if (status == Message::Callback::Status::SUCCESS && r != nullptr) {
        auto indMsg = std::make_shared<rildata::DsdSystemStatusMessage>(r->resp_ind);
        indMsg->broadcast();
        auto perApnIndMsg = std::make_shared<rildata::DsdSystemStatusPerApnMessage>(r->apn_sys);
        perApnIndMsg->broadcast();
      } else {
        Log::getInstance().d("[DSDModemEndPoint::generateDsdSystemStatusInd] Failed to get system status");
      }
    });
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    Log::getInstance().d("[DSDModemEndPoint::generateDsdSystemStatusInd]"
                         "Failed to send message GetDsdSystemStatusMessage");
  }
}

Message::Callback::Status DSDModemEndPoint::registerForSystemStatusSync ( )
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::registerForSystemStatusSync]"
                         "dispatching message RegisterForSystemStatusSyncMessage");
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<RegisterForSystemStatusSyncMessage>(nullptr);
    Message::Callback::Status apiStatus =  Message::Callback::Status::FAILURE;
    auto r = std::make_shared<int>();
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(r);
    }
    std::ostringstream ss;
    ss << "[DSDModemEndPoint::registerForSystemStatusSync] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForSystemStatusSync]"
                         "Failed to send message RegisterForSystemStatusSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::setV2Capabilities() {
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    auto msg = std::make_shared<SetCapabilitiesMessage>();
    msg->setDsdSystemStatusV2(true);
    msg->setDsdUiInfoV2(true);
    msg->broadcast();
    return Message::Callback::Status::SUCCESS;
  }

  Log::getInstance().d("[DSDModemEndPoint::setV2Capabilities]"
                       "Failed to send message SetCapabilitiesMessage");
  return Message::Callback::Status::FAILURE;
}

Message::Callback::Status DSDModemEndPoint::registerForDDSInd ( )
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_indication_register_req_msg_v01 ind_req;
    dsd_indication_register_resp_msg_v01 ind_resp;
    memset(&ind_req, 0, sizeof(ind_req));
    memset(&ind_resp, 0, sizeof(ind_resp));

    ind_req.report_current_dds_valid = true;
    ind_req.report_current_dds = 1;

    ind_req.report_temp_dds_reco_cap_valid = true;
    ind_req.report_temp_dds_reco_cap = 1;

    ind_req.report_dds_recommendation_valid = true;
    ind_req.report_dds_recommendation = 1;

    int rc = sendRawSync(QMI_DSD_INDICATION_REGISTER_REQ_V01,
                         (void *)&ind_req, sizeof(ind_req),
                         (void *)&ind_resp, sizeof(ind_resp),
                         QCRIL_DATA_QMI_TIMEOUT);
    if ((rc != QMI_NO_ERR) || (ind_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPoint::registerForDDSInd] Failed to send QMI_DSD_INDICATION_REGISTER_REQ_V01");
    } else {
      Log::getInstance().d("[DSDModemEndPoint::registerForDDSInd] QMI_DSD_INDICATION_REGISTER_REQ_V01::Successfully sent");
      return Message::Callback::Status::SUCCESS;
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForDDSInd]"
                         "Failed to send message registerForDDSInd");
  }
  return Message::Callback::Status::FAILURE;
}

Message::Callback::Status DSDModemEndPoint::registerForAPAsstIWlanInd (bool toRegister )
{
  /* To DO: Check whether this is needed for LE*/
  Message::Callback::Status ret = Message::Callback::Status::SUCCESS;
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_indication_register_req_msg_v01 ind_req;
    dsd_indication_register_resp_msg_v01 ind_resp;
    memset(&ind_req, 0, sizeof(ind_req));
    memset(&ind_resp, 0, sizeof(ind_resp));

    ind_req.report_intent_to_change_pref_sys_valid = true;
    ind_req.report_intent_to_change_pref_sys = toRegister;

    ind_req.report_ap_asst_apn_pref_sys_result_valid = true;
    ind_req.report_ap_asst_apn_pref_sys_result = toRegister;

    ind_req.report_pref_apn_update_valid = true;
    ind_req.report_pref_apn_update = toRegister;

    int rc = sendRawSync(QMI_DSD_INDICATION_REGISTER_REQ_V01,
                         (void *)&ind_req, sizeof(ind_req),
                         (void *)&ind_resp, sizeof(ind_resp),
                         QCRIL_DATA_QMI_TIMEOUT);

    if ((rc != QMI_NO_ERR) || (ind_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPoint::registerForAPAsstIWlanInd] Failed to send QMI_DSD_INDICATION_REGISTER_REQ_V01");
      ret = Message::Callback::Status::FAILURE;
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForAPAsstIWlanInd] : Failed to send message registerForDDSInd");
    ret = Message::Callback::Status::FAILURE;
  }
  return ret;
}

Message::Callback::Status DSDModemEndPoint::registerForCIWLAN ( )
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_indication_register_req_msg_v01 ind_req;
    dsd_indication_register_resp_msg_v01 ind_resp;
    memset(&ind_req, 0, sizeof(ind_req));
    memset(&ind_resp, 0, sizeof(ind_resp));

    ind_req.report_ciwlan_cap_valid = true;
    ind_req.report_ciwlan_cap = 1;

    int rc = sendRawSync(QMI_DSD_INDICATION_REGISTER_REQ_V01,
                         (void *)&ind_req, sizeof(ind_req),
                         (void *)&ind_resp, sizeof(ind_resp),
                         QCRIL_DATA_QMI_TIMEOUT);

    if ((rc != QMI_NO_ERR) || (ind_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPoint::registerForCIWLAN] Failed to send QMI_DSD_INDICATION_REGISTER_REQ_V01");
    } else {
      Log::getInstance().d("[DSDModemEndPoint::registerForCIWLAN] QMI_DSD_INDICATION_REGISTER_REQ_V01::Successfully sent");
      return Message::Callback::Status::SUCCESS;
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForCIWLAN] Failed to send message registerForDDSInd");
  }
  return Message::Callback::Status::FAILURE;
}

Message::Callback::Status DSDModemEndPoint::registerForRoamingStatusChanged ( )
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::registerForRoamingStatusChanged]"
                         "dispatching message RegisterForRoamingStatusChangedMessage");
    // Note that callback is not required for sync calls.
    auto msg = std::make_shared<RegisterForRoamingStatusChangedMessage>(nullptr);
    Message::Callback::Status apiStatus = Message::Callback::Status::FAILURE;
    auto r = std::make_shared<int>();
    if (msg != nullptr) {
        apiStatus = msg->dispatchSync(r);
    }
    std::ostringstream ss;
    ss << "[DSDModemEndPoint::registerForRoamingStatusChanged] status = " << (int) apiStatus;
    Log::getInstance().d(ss.str());
    return apiStatus;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::registerForRoamingStatusChanged]"
                         "Failed to send message RegisterForRoamingStatusChangedMessage");
    return Message::Callback::Status::FAILURE;
  }
}

RIL_Errno DSDModemEndPoint::registerForDataDuringVoiceCall(bool enable)
{
  RIL_Errno status = RIL_E_SUCCESS;
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::registerForDataDuringVoiceCall] enable="
                          +std::to_string((int)enable)+
                          " mUserDdsSwitchSelection="+std::to_string((int)mUserDdsSwitchSelection));
    if (enable == mUserDdsSwitchSelection) {
      Log::getInstance().d("[DSDModemEndPoint] registered user selection is not changed");
      return RIL_E_SUCCESS;
    }

    dsd_configure_dynamic_dds_switch_req_msg_v01 switch_req;
    dsd_configure_dynamic_dds_switch_resp_msg_v01 switch_resp;

    // enable Temporary DDS recommendation action, Permanent DDS Recommendation Action is enabled by default
    switch_req.dynamic_dds_switch_setting = DSD_DYNAMIC_DDS_SWITCH_RECOMMENDATION_V01;
    switch_req.perm_rec_action_valid = true;
    switch_req.perm_rec_action = DSD_DDS_RECOMMENDATION_STOP_V01;
    switch_req.temp_rec_action_valid = true;
    switch_req.temp_rec_action = enable?
      DSD_DDS_RECOMMENDATION_START_V01:DSD_DDS_RECOMMENDATION_STOP_V01;

    int rc = sendRawSync(QMI_DSD_CONFIGURE_DYNAMIC_DDS_SWITCH_REQ_V01,
                    (void *)&switch_req, sizeof(switch_req),
                    (void *)&switch_resp, sizeof(switch_resp),
                    QCRIL_DATA_QMI_TIMEOUT);

    if ((rc != QMI_NO_ERR) || (switch_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send QMI_DSD_CONFIGURE_DYNAMIC_DDS_SWITCH_REQ_V01");
      status = RIL_E_GENERIC_FAILURE;
    }
    else {
      Log::getInstance().d("[DSDModemEndPoint] QMI_DSD_CONFIGURE_DYNAMIC_DDS_SWITCH_REQ_V01::Successfully sent");
    }

    if (status != RIL_E_GENERIC_FAILURE) {
      mUserDdsSwitchSelection = enable;
      Log::getInstance().d("[DSDModemEndPoint::mUserDdsSwitchSelection="+std::to_string((int)mUserDdsSwitchSelection));
    }
  }

  return status;
}

Message::Callback::Status DSDModemEndPoint::setApnPreferredSystemChangeSync(
  const std::string apnName, const int32_t prefRat)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::setApnPreferredSystemChangeSync]"
                         "dispatching message SetApnPreferredSystemSyncMessage");

    dsd_set_apn_preferred_system_req_msg_v01 qmiReq;
    dsd_set_apn_preferred_system_resp_msg_v01 qmiResp;
    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));

    strlcpy(qmiReq.apn_pref_sys.apn_name, apnName.c_str(), QMI_DSD_MAX_APN_LEN_V01+1);
    qmiReq.apn_pref_sys.pref_sys = (dsd_apn_pref_sys_enum_v01)prefRat;
    qmiReq.is_ap_asst_valid = true;
    qmiReq.is_ap_asst = true;
    auto retVal = sendRawSync(QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send"
        "QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01");
      return Message::Callback::Status::FAILURE;
    } else {
      return Message::Callback::Status::SUCCESS;
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::setApnPreferredSystemChangeSync]"
                         "Failed to send message SetApnPreferredSystemSyncMessage");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::getCurrentDDSSync(DDSSubIdInfo &ddsInfo)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_get_current_dds_req_msg_v01 qmiReq;
    dsd_get_current_dds_resp_msg_v01 qmiResp;
    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));
    auto retVal = sendRawSync(QMI_DSD_GET_CURRENT_DDS_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send"
        "QMI_DSD_GET_CURRENT_DDS_REQ_V01");
      return Message::Callback::Status::FAILURE;
    } else if (!qmiResp.dds_valid) {
      // TODO: discuss how to handle case where temporary dds is returned

      Log::getInstance().d("[DSDModemEndPoint] QMI_DSD_GET_CURRENT_DDS_REQ_V01 "
        "did not return dds");
      return Message::Callback::Status::FAILURE;
    }
    switch (qmiResp.dds) {
      case DSD_PRIMARY_SUBS_V01:
        ddsInfo.dds_sub_id = 0;
        break;
      case DSD_SECONDARY_SUBS_V01:
        ddsInfo.dds_sub_id = 1;
        break;
      default:
        Log::getInstance().d("[DSDModemEndPoint] QMI_DSD_GET_CURRENT_DDS_REQ_V01 "
          "invalid subId=" + std::to_string(qmiResp.dds));
        return Message::Callback::Status::FAILURE;
    }
    if(qmiResp.dds_switch_type_valid) {

      switch(qmiResp.dds_switch_type) {
        case DSD_DDS_SWITCH_PERMANENT_V01:
          ddsInfo.switch_type = DSD_DDS_DURATION_PERMANANT_V01;
          break;
        case DSD_DDS_SWITCH_TEMPORARY_V01:
          ddsInfo.switch_type = DSD_DDS_DURATION_TEMPORARY_V01;
          break;
        default:
          ddsInfo.switch_type = DSD_DDS_DURATION_PERMANANT_V01;
      }
    } else {
      ddsInfo.switch_type = DSD_DDS_DURATION_PERMANANT_V01; //default value
    }
    return Message::Callback::Status::SUCCESS;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::getCurrentDDSSync]"
                         "Failed to send message QMI_DSD_GET_CURRENT_DDS_REQ_V01");
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::triggerDDSSwitchSync(int subId, int& error, dsd_dds_switch_type_enum_v01 switch_type)
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::triggerDDSSwitchSync]"
                         "dispatching message TriggerDDSSwitchSyncMessage");
    auto msg = std::make_shared<TriggerDDSSwitchSyncMessage>(subId, switch_type);
    auto retVal = std::make_shared<SetPreferredDataModemResult_t>();
    auto apiStatus = msg->dispatchSync(retVal);
    Log::getInstance().d("[DSDModemEndPoint::triggerDDSSwitchSync] status=" +
                         std::to_string(static_cast<int>(apiStatus)));
    if (retVal == nullptr) {
      Log::getInstance().d("[DSDModemEndPoint::triggerDDSSwitchSync] returned null");
      error = static_cast<int>(SetPreferredDataModemResult_t::QMI_ERROR);
      return Message::Callback::Status::FAILURE;
    }
    error = static_cast<int>(*retVal);
    Log::getInstance().d("[DSDModemEndPoint::triggerDDSSwitchSync] returned errorcode=" +
                         std::to_string(error));
    return apiStatus;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::triggerDDSSwitchSync]"
                         "Failed to send message TriggerDDSSwitchSyncMessage");
    error = static_cast<int>(SetPreferredDataModemResult_t::MODEM_ENDPOINT_NOT_OPERATIONAL);
    auto msg = std::make_shared<DDSSwitchResultIndMessage>();
    msg->setError(TriggerDDSSwitchError::MODEM_NOT_UP);
    msg->broadcast();
    return Message::Callback::Status::FAILURE;
  }
}

Message::Callback::Status DSDModemEndPoint::getDataSystemCapabilitySync()
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    Log::getInstance().d("[DSDModemEndPoint::getDataSystemCapabilitySync]");
    dsd_get_capability_req_msg_v01 req_msg;
    dsd_get_capability_resp_msg_v01 resp_msg;
    memset(&req_msg, 0, sizeof(req_msg));
    memset(&resp_msg, 0, sizeof(resp_msg));

    req_msg.temp_dds_rec_capability_valid = 1;
    req_msg.temp_dds_rec_capability = 1;
    req_msg.ciwlan_capability_valid = 1;
    req_msg.ciwlan_capability = 1;

    int rc = sendRawSync(QMI_DSD_GET_CAPABILITY_REQ_V01,
                        (void *)&req_msg, sizeof(req_msg),
                        (void *)&resp_msg, sizeof(resp_msg),
                        QCRIL_DATA_QMI_TIMEOUT);
    if ((rc != QMI_NO_ERR) || (resp_msg.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send QMI_DSD_GET_CAPABILITY_REQ_V01");
    } else{
      Log::getInstance().d("[DSDModemEndPoint] QMI_DSD_GET_CAPABILITY_REQ_V01::Successfully sent");
      if (resp_msg.temp_dds_rec_capability_valid) {
        bool capability = resp_msg.temp_dds_rec_capability ? true : false;
        if (!mDdsSwitchRecCapEnabled.has_value() ||
            (mDdsSwitchRecCapEnabled.has_value() &&
            mDdsSwitchRecCapEnabled.value() != capability)) {
          mDdsSwitchRecCapEnabled = capability;
          if (capability) {
            auto msg = std::make_shared<rildata::AutoDdsSwitchControlIndMessage>
                                        (AutoDdsSwitchControl_t::AUTO_DDS_SWITCH_CAPABILITY_CHANGED);
            msg->broadcast();
          }
        }
        Log::getInstance().d("[DSDModemEndPoint]: temp rec capability="+std::to_string((int)capability));
      }
      if (resp_msg.ciwlan_capability_valid) {
        bool capability = resp_msg.ciwlan_capability ? true : false;
        if (mCIWlanCapability != capability) {
          mCIWlanCapability = capability;
          auto msg = std::make_shared<DataCapabilityChangeIndMessage>();
          msg->setCIWlanSupported(capability);
          msg->broadcast();
        }
        Log::getInstance().d("[DSDModemEndPoint]: ciwlan capability="+std::to_string((int)capability));
      }
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::getDataSystemCapabilitySync]"
                         "Failed to query - not operational");
    return Message::Callback::Status::FAILURE;
  }
  return Message::Callback::Status::SUCCESS;
}

int DSDModemEndPoint::toggleLimitedSysIndicationChangeReport(bool report)
{
  Log::getInstance().d("[DSDModemEndPoint]: toggleLimitedSysIndicationChangeReport "+std::to_string((int)report));
  if (mReportLimitedSysIndicationChange != report) {
    mReportLimitedSysIndicationChange = report;

    dsd_system_status_change_req_msg_v01  qmiReq;
    dsd_system_status_change_resp_msg_v01 qmiResp;
    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));

    qmiReq.limit_so_mask_change_ind_valid = true;
    qmiReq.limit_so_mask_change_ind = report;

    auto retVal = sendRawSync(QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01,
                          (void *)&qmiReq, sizeof(qmiReq),
                          (void *)&qmiResp, sizeof(qmiResp),
                          QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint::toggleLimitedSysIndicationChangeReport] Failed to register");
    }

    /* when mReportLimitedSysIndicationChange = true, limit the DsdSystemStatusMessage*/
    if (!mReportLimitedSysIndicationChange) {
      dsd_get_system_status_resp_msg_v01  sysRespMsg;
      dsd_system_status_ind_msg_v01 *ind_data = nullptr;
      memset(&sysRespMsg, 0, sizeof(sysRespMsg));
      retVal = sendRawSync(QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                            nullptr, 0,
                            (void *)&sysRespMsg, sizeof(sysRespMsg),
                            QCRIL_DATA_QMI_TIMEOUT);
      if (retVal == QMI_NO_ERR) {
        Log::getInstance().d("[DSDModemEndPoint::toggleLimitedSysIndicationChangeReport] success to query");
        ind_data = (dsd_system_status_ind_msg_v01 *)((char *)&sysRespMsg +
                                                    offsetof(dsd_get_system_status_resp_msg_v01,
                                                    avail_sys_valid));

        auto indMsg = std::make_shared<rildata::DsdSystemStatusMessage>(*ind_data);
        indMsg->broadcast();
      }
      else {
        Log::getInstance().d("[DSDModemEndPoint::toggleLimitedSysIndicationChangeReport] Failed to query");
      }
    }
  }

  return 0;
}

Message::Callback::Status DSDModemEndPoint::handleScreenStateChangeInd(bool screenEnabled)
{
  dsd_register_ui_info_change_req_msg_v01 qmiReq;
  memset(&qmiReq, 0, sizeof(qmiReq));

  if( ((screenEnabled == true ) && ( mScreenState == ScreenState_t::SCREEN_ON )) ||
      ((screenEnabled == false ) && ( mScreenState == ScreenState_t::SCREEN_OFF )))
  {
    Log::getInstance().d("[DSDModemEndPoint::No change in Screen State!!");
    return Message::Callback::Status::SUCCESS;
  } else if ((screenEnabled == false ) && ( mScreenState == ScreenState_t::NONE ))
  {
    //If RIL restarts & when process is UP, Screen goes OFF.
    //We do not deregister as it is a NO OP at modem.
    Log::getInstance().d("[DSDModemEndPoint:: Already in Deregistered state."
                         " Not sending deregister request");
    return Message::Callback::Status::SUCCESS;
  }
  if (screenEnabled == mUiFilterRegistered) {
    Log::getInstance().d("[DSDModemEndPoint]::UI Indication Registration is already in the requested state");
    mScreenState = ( screenEnabled ) ? ScreenState_t::SCREEN_ON : ScreenState_t::SCREEN_OFF;
    return Message::Callback::Status::SUCCESS;
  }
  if( screenEnabled) {
    Log::getInstance().d("[DSDModemEndPoint::register for UI indication");
    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 1;
    qmiReq.suppress_so_change_valid = true;
    qmiReq.suppress_so_change = 1;
    qmiReq.suppress_null_bearer_reason_valid = true;
    qmiReq.suppress_null_bearer_reason = 1;
  } else {
    Log::getInstance().d("[DSDModemEndPoint::deregister for UI indication");
    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 0;
  }
  Message::Callback::Status status = sendRegisterForUiChangeInd(qmiReq);
  if (status == Message::Callback::Status::SUCCESS) {
    mScreenState = ( screenEnabled ) ? ScreenState_t::SCREEN_ON : ScreenState_t::SCREEN_OFF;
    mUiFilterRegistered = screenEnabled;
  }
  if (mScreenState == ScreenState_t::SCREEN_ON) {
    updateInitialUiInfo();
  }
  return status;
}

#ifdef RIL_FOR_MDM_LE
bool DSDModemEndPoint::uiChangeInfoRegistrationRequest(bool regValue)
{
  if (mUiFilterRegistered == regValue) {
    Log::getInstance().d("[DSDModemEndPoint]:: Registration State is Matching with Requested State.");
    return true;
  }
  dsd_register_ui_info_change_req_msg_v01 qmiReq;
  memset(&qmiReq, 0, sizeof(qmiReq));
  Log::getInstance().d("[DSDModemEndPoint]:: Register for UI indication Bit : " + std::to_string(regValue));
  if( regValue ) {
    Log::getInstance().d("[DSDModemEndPoint]::register for UI indication");
    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 1;
    qmiReq.suppress_so_change_valid = true;
    qmiReq.suppress_so_change = 1;
    qmiReq.suppress_null_bearer_reason_valid = true;
    qmiReq.suppress_null_bearer_reason = 1;
  } else {
    Log::getInstance().d("[DSDModemEndPoint]::deregister for UI indication");
    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 0;
  }
  Message::Callback::Status status = sendRegisterForUiChangeInd(qmiReq);
  if (status == Message::Callback::Status::SUCCESS) {
    if (regValue) {
      updateInitialUiInfo();
    }
    mUiFilterRegistered = regValue;
    return true;
  }
  return false;
}
#endif

void DSDModemEndPoint::registerForUiChangeInd()
{
  if( mScreenState == ScreenState_t::SCREEN_ON || mUiFilterRegistered)
  {
    dsd_register_ui_info_change_req_msg_v01 qmiReq;
    memset(&qmiReq, 0, sizeof(qmiReq));

    qmiReq.suppress_so_change_valid = true;
    qmiReq.suppress_so_change = 1;
    qmiReq.suppress_null_bearer_reason_valid = true;
    qmiReq.suppress_null_bearer_reason = 1;

    Log::getInstance().d("[DSDModemEndPoint:: Screen State is ON , register for UI indication");
    qmiReq.report_ui_changes_valid = true;
    qmiReq.report_ui_changes = 1;

    Message::Callback::Status status = sendRegisterForUiChangeInd(qmiReq);
    if (status == Message::Callback::Status::SUCCESS) {
      updateInitialUiInfo();
      mUiFilterRegistered = true;
    }
  }
}

Message::Callback::Status DSDModemEndPoint::sendRegisterForUiChangeInd(dsd_register_ui_info_change_req_msg_v01 qmiReq)
{
  Message::Callback::Status status = Message::Callback::Status::FAILURE;
  dsd_register_ui_info_change_resp_msg_v01 qmiResp;
  memset(&qmiResp, 0, sizeof(qmiResp));

  auto retVal = sendRawSync(QMI_DSD_REGISTER_UI_INFO_CHANGE_REQ_V01,
                            (void *)&qmiReq, sizeof(qmiReq),
                            (void *)&qmiResp, sizeof(qmiResp),
                            QCRIL_DATA_QMI_TIMEOUT);

  if (retVal == QMI_NO_ERR) {
    status = Message::Callback::Status::SUCCESS;
  }
  return status;
}

void DSDModemEndPoint::updateInitialUiInfo()
{
  auto msg = std::make_shared<GetDataNrIconTypeMessage>();
  GenericCallback<NrIconType_t> cb([](std::shared_ptr<Message>,
                                      Message::Callback::Status status,
                                      std::shared_ptr<NrIconType_t> rsp) -> void {
    if (rsp != nullptr) {
      Log::getInstance().d("updateInitialUiInfo cb invoked status " + std::to_string((int)status));
      auto indMsg = std::make_shared<DataNrIconTypeIndMessage>(*rsp);
      indMsg->broadcast();
    }
  });
  msg->setCallback(&cb);
  msg->dispatch();
}

#ifdef QMI_RIL_UTF
void DSDModemEndPoint::cleanup() {
  mScreenState = ScreenState_t::SCREEN_ON;
  mUiFilterRegistered = false;
  mCIWlanCapability = false;
  mCIWlanUISelection = false;
}
#endif
RIL_Errno DSDModemEndPoint::setIsDataEnabled(bool enabled)
{
  RIL_Errno errNum =  RIL_E_GENERIC_FAILURE;
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_notify_data_settings_req_msg_v01  qmiReq;
    dsd_notify_data_settings_resp_msg_v01 qmiResp;

    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));
    qmiReq.data_service_switch_valid = TRUE;
    qmiReq.data_service_switch = enabled;

    Log::getInstance().d("[DSDModemEndPoint::Setting data_enabled =" + std::to_string(enabled));

    auto retVal = sendRawSync(QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01");
    } else {
        errNum = RIL_E_SUCCESS;
    }
  } else {
    Log::getInstance().d("[DSDModemEndPoint::setIsDataEnabled] Failed to send QMI message");
  }
  return errNum;
}

RIL_Errno DSDModemEndPoint::setIsDataRoamingEnabled(bool enabled)
{
  RIL_Errno errNum =  RIL_E_GENERIC_FAILURE;
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_notify_data_settings_req_msg_v01  qmiReq;
    dsd_notify_data_settings_resp_msg_v01 qmiResp;

    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));
    qmiReq.data_service_roaming_switch_valid = TRUE;
    qmiReq.data_service_roaming_switch = enabled;

    Log::getInstance().d("Setting Data Roaming Enabled =" + std::to_string(enabled));

    auto retVal = sendRawSync(QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01");
    } else {
        errNum = RIL_E_SUCCESS;
    }
  } else {
    Log::getInstance().d("SetIsDataRoamingEnabled Failed to send QMI message");
  }
  return errNum;
}

RIL_Errno DSDModemEndPoint::setCIWlanUIOptionSelection(bool enabled)
{
  RIL_Errno errNum =  RIL_E_GENERIC_FAILURE;
  if (getState() == ModemEndPoint::State::OPERATIONAL && mCIWlanCapability)
  {
    dsd_notify_data_settings_req_msg_v01  qmiReq;
    dsd_notify_data_settings_resp_msg_v01 qmiResp;

    memset(&qmiReq, 0, sizeof(qmiReq));
    memset(&qmiResp, 0, sizeof(qmiResp));
    qmiReq.ciwlan_switch_valid = TRUE;
    qmiReq.ciwlan_switch = enabled;

    Log::getInstance().d("Setting CIWlan Enabled =" + std::to_string(enabled)+", mCIWlanUISelection="+std::to_string(mCIWlanUISelection));

    auto retVal = sendRawSync(QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                              (void *)&qmiReq, sizeof(qmiReq),
                              (void *)&qmiResp, sizeof(qmiResp),
                              QCRIL_DATA_QMI_TIMEOUT);
    if (retVal != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPoint] Failed to send QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01");
    } else {
      mCIWlanUISelection = enabled;
      Log::getInstance().d("mCIWlanUISelection updated ="+std::to_string(mCIWlanUISelection));
      errNum = RIL_E_SUCCESS;
    }
  } else {
    Log::getInstance().d("setCIWlanUIOptionSelection Failed to send QMI message "+std::to_string((int)mCIWlanCapability));
  }
  return errNum;
}

bool DSDModemEndPoint::deviceShutdownRequest()
{
  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_notify_data_settings_req_msg_v01 qmiReq;
    dsd_notify_data_settings_resp_msg_v01 qmiResp;

    memset(&qmiReq, 0, sizeof(dsd_notify_data_settings_req_msg_v01));
    memset(&qmiResp, 0, sizeof(dsd_notify_data_settings_resp_msg_v01));

    qmiReq.wifi_switch_valid = true;
    qmiReq.wifi_switch = false;

    Log::getInstance().d("QMI DSD Sending Device Shutdown Request ");

    auto retVal = sendRawSync(QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                                (void *)&qmiReq, sizeof(qmiReq),
                                (void *)&qmiResp, sizeof(qmiResp),
                                QCRIL_DATA_QMI_TIMEOUT);
    if (QMI_NO_ERR != retVal) {
      Log::getInstance().d("Sending of notify data setting failed retVal" + std::to_string(retVal));
      return false;
    }
    return true;
  } else {
    Log::getInstance().d("DSD EP not OPERATIONAL");
  }
  return false;
}

qmi_response_type_v01 DSDModemEndPoint::setQualityMeasurement
(
  dsd_set_quality_measurement_info_req_msg_v01 qmiReq
)
{
  qmi_response_type_v01 resp;
  memset(&resp, 0, sizeof(resp));
  resp.result = QMI_RESULT_FAILURE_V01;
  resp.error = QMI_ERR_INTERNAL_V01;

  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    dsd_set_quality_measurement_info_resp_msg_v01 qmiResp;
    memset(&qmiResp, 0, sizeof(qmiResp));

    auto rc = sendRawSync(QMI_DSD_SET_QUALITY_MEASUREMENT_INFO_REQ_V01,
                                  (void *)&qmiReq, sizeof(qmiReq),
                                  (void *)&qmiResp, sizeof(qmiResp),
                                  QCRIL_DATA_QMI_TIMEOUT);

    if (QMI_NO_ERR != rc) {
      Log::getInstance().d("failed to send QMI msg rc= "+std::to_string(rc));
    } else {
      resp = qmiResp.resp;
    }
  } else {
    Log::getInstance().d("DSD EP not OPERATIONAL");
  }
  Log::getInstance().d("qcril_data_set_quality_measurement():"
          " result"+std::to_string(resp.result)+" err=" +std::to_string(resp.error));
  return resp;
}

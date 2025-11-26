/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <utils/Log.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include "xm_xprofile_if.h"

#include <android-base/logging.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cutils/properties.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.xpan@1.0-xmxprofileif"

using bluetooth::xpanprovider::XpanProviderIf;
tXPAN_MANAGER_API XpanProviderCb;
namespace xpan {
namespace implementation {
XMXprofileIf::XMXprofileIf()
{
}

XMXprofileIf::~XMXprofileIf()
{
}

/******************************************************************
 *
 * Function         RemoteSupportsXpan
 *
 * Description      This API is notify profile when a new remote
 *                  is connected.
 * Parameters:      bdaddr_t - Bluetooth address of the remote.
 *                  enable   - add a place holder always set to true.
 *
 * Return:          bool - return the status wether the message is
 *                  queued to processed by AIDL.
 ******************************************************************/
bool XMXprofileIf::RemoteSupportsXpan(bdaddr_t addr, bool enable)
{
  ALOGD("%s: bd addr :%s and status:%d", __func__,
        ConvertRawBdaddress(addr), enable);
  /* status wether the device supports xpan or not are for reserved
   * purpose. This parameters might be used in future implementation.
   */
 if(XpanProviderIf::GetIf())
   XpanProviderIf::GetIf()->XpanDeviceFoundCb(addr);
 else
   ALOGE("Discarding %s AIDL service is not up and running", __func__);
  return true;
}

/******************************************************************
 *
 * Function         TransportEnabled
 *
 * Description      This API is invoked by profile to notify XM the status
 *                  of SAP connetion
 *
 * Parameters:      bdaddr_t - Bluetooth address of the remote.
 *                  status   - holds the status of SAP connection.
 *
 * Return:          bool - return the status wether the message is
 *                  queued to process by QHCI.
 ******************************************************************/
void XMXprofileIf::TransportEnabled(bdaddr_t addr, TransportType type,
		                    bool status, uint8_t reason)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGI("%s for bd address %s of transport %s with status: %d and reason:%d",
	__func__, TransportTypeToString(type), ConvertRawBdaddress(addr),
	status, reason);
  msg->TransportEnabled.eventId = XP_XM_TRANSPORT_ENABLED;
  memcpy(&msg->TransportEnabled.bdaddr, &addr, sizeof(bdaddr_t));
  msg->TransportEnabled.type = type;
  msg->TransportEnabled.status = status;
  msg->TransportEnabled.reason = reason;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::UpdateXpanBondedDevices(uint8_t numOfDevices, bdaddr_t *devices)
{
  ALOGI("%s with numOfDevices ", __func__);
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->BondedDevies.eventId = XP_XM_BONDED_DEVICES_LIST;
  msg->BondedDevies.numOfDevices = numOfDevices;
  msg->BondedDevies.bdaddr = (bdaddr_t *)malloc(numOfDevices * sizeof(bdaddr_t));
  for (int i =0; i < numOfDevices; i++) {
    memcpy(msg->BondedDevies.bdaddr, &devices[i], sizeof(bdaddr_t));
    ALOGI("%s: BondedDevies[%d]: %s", __func__, i, ConvertRawBdaddress(devices[i]));  
  }
  XpanManager::Get()->PostMessage(msg);
}

bool XMXprofileIf::XmXpPrepareAudioBearerReq(bdaddr_t addr, TransportType type)
{
  ALOGI("%s for bdaddr_t:%s with transport: %s", __func__,
        (ConvertRawBdaddress(addr)), (TransportTypeToString(type)));

  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->PrepareBearerCb(addr, (uint8_t) type);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);

  return true;
}

void XMXprofileIf::XpXmPrepareAudioBearerRsp(bdaddr_t addr, uint8_t bearer,
                                             uint8_t status)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  
  ALOGD("%s", __func__);
  msg->AudioBearerRsp.eventId = XP_XM_PREPARE_AUDIO_BEARER_RSP;
  memcpy(&msg->AudioBearerRsp.bdaddr, &addr, sizeof(bdaddr_t));
  msg->AudioBearerRsp.status = (RspStatus)status;
  XpanManager::Get()->PostMessage(msg);
}

bool XMXprofileIf::XmXpBearerSwitchInd(bdaddr_t addr, uint8_t status)
{
  return true;
}

void XMXprofileIf::XpXmBearerSwitchInd(bdaddr_t addr, uint8_t bearer,
		                       uint8_t status)
{
 
  ALOGD("%s", __func__);
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->XpBearerSwitchInd.eventId = XP_XM_AUDIO_BEARER_SWITCHED;
  memcpy(&msg->XpBearerSwitchInd.bdaddr, &addr, sizeof(bdaddr_t));
  msg->XpBearerSwitchInd.status = (RspStatus)status;
  XpanManager::Get()->PostMessage(msg);
}

/******************************************************************
 *
 * Function         UseCaseUpdate
 *
 * Description      This API is invoked by XM to notify on usecase.
 *
 * Parameters:      usecase - Usecase type
 *
 * Return:          bool - return the status whether the message is
 *                  queued to process by profile.
 ******************************************************************/

bool XMXprofileIf::UseCaseUpdate(UseCaseType usecase)
{
  ALOGI("%s usecase: %s", __func__, UseCaseToString(usecase));
  /* BD address is reserved for future use, for now sending
   * ACTIVE_BDADDR. Upper layers will apply usecase for either
   * active device or for next connected device.
   */
  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->UsecaseUpdateCb(ACTIVE_BDADDR, usecase);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);

  return true;;
}

void XMXprofileIf::EnableAcs(std::vector<uint32_t>freq_list)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  int i = 0,freq;
  int freq_list_size = freq_list.size();

  ALOGD("%s with freq_list_size: %d", __func__, freq_list_size);
  msg->Acslist.eventId = XP_XM_ENABLE_ACS;

  /* Create a separate chunk instead of relying on static memory locations */
  msg->Acslist.freq_list = (int *) malloc(sizeof(int) * freq_list_size);
  for (int freq : freq_list)
    msg->Acslist.freq_list[i++] = freq;

  msg->Acslist.freq_list_size = freq_list_size;

  XpanManager::Get()->PostMessage(msg);
}

bool XMXprofileIf::WiFiAcsResults(xm_ipc_msg_t *msg)
{
  ALOGD("%s: Rx Acs results freq: %d and status%d", __func__,
        msg->AcsResults.freq, msg->AcsResults.status);
  if(XpanProviderIf::GetIf())
     XpanProviderIf::GetIf()->AcsUpdateCb(msg->AcsResults.status,
		                       msg->AcsResults.freq);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);
  return true;
}

bool XMXprofileIf::WifiTwtEvent(xm_ipc_msg_t *msg)
{

  uint8_t twt_event = msg->TwtEvent.twt_event;
  int32_t wake_dur_us = msg->TwtEvent.wake_dur_us;
  int32_t wake_int_us = msg->TwtEvent.wake_int_us;

  ALOGD("%s", __func__);
  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->TwtSessionEstablishedCb(msg->TwtEvent.addr,
		                       wake_dur_us, wake_int_us, twt_event);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);
  return true;
}

bool XMXprofileIf::UpdateSapInterface(xm_ipc_msg_t *msg)
{
  uint8_t req_state = msg->CreateSapInfStatusParams.req_state;
  uint8_t status = msg->CreateSapInfStatusParams.status;
  ALOGD("%s: for state req :%s and status is %d", __func__,
	req_state ? "enable": "disable", status);
  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->SapInterfaceCreatedCb(req_state, status);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);
  return true;
}

bool XMXprofileIf::WifiSapPowerSaveEvent(xm_ipc_msg_t *msg)
{
  uint8_t dialog_id = msg->SapPowerStateRsp.dialog_id;
  uint16_t power_save_bi_multiplier = msg->SapPowerStateRsp.power_save_bi_multiplier;
  uint64_t next_tsf = msg->SapPowerStateRsp.next_tsf;
  ALOGD("%s", __func__);

  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->SapLowPowerModeUpdateCb(dialog_id,
		                       power_save_bi_multiplier, next_tsf);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);

  return true;
}

bool XMXprofileIf::TransportUpdate(TransportType transport)
{
  ALOGD("%s", __func__);
  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->TransportUpdatedCb(transport);
  else
    ALOGE("Discarding %s AIDL service is not up and running", __func__);

  return true;
}

void XMXprofileIf::UpdateTWTSessionParams(uint8_t num_devices,
		              std::vector<tXPAN_Twt_Session_Params> twt_params)
{
  int i = 0;
  XPANTwtSessionParams *Twtparams;

  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.twtconfig", value, "true");
  ALOGI("%s: persist.vendor.service.twtconfig:%s", __func__, value);
  if (strcmp(value, "false") == 0) {
    ALOGI("%s: disable to send TWT", __func__);
    return;
  }

  ALOGD("%s with num_devices %d", __func__, num_devices);
  CHECK(num_devices == twt_params.size());
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->TwtParams.eventId = XP_XM_TWT_SESSION_EST;
  msg->TwtParams.num_devices = num_devices;
  Twtparams = (XPANTwtSessionParams *)
	                  malloc(num_devices * sizeof(XPANTwtSessionParams));

  for (tXPAN_Twt_Session_Params params: twt_params) {
   for (int j =0; j < sizeof(macaddr_t); j++)
     Twtparams[i].mac_addr.b[j] = params.mac_addr.b[j];
//    memcpy(&msg->TwtParams.params[i].mac_addr, &params.mac_addr, sizeof(macaddr_t));
    Twtparams[i].interval = params.SI;
    Twtparams[i].peroid = params.SP;
    Twtparams[i].location = params.location;
    ALOGD("%s: macaddr %s interval: %d peroid: %d location :%d", __func__,
          ConvertRawMacaddress(Twtparams[i].mac_addr),
	  Twtparams->interval, Twtparams->peroid, Twtparams->location);
    i++;
  }
  msg->TwtParams.params = Twtparams;
  ALOGE("%s: XP_XM_TWT_SESSION_EST msg twt addr %p and locat twt %p", __func__, msg->TwtParams.params, Twtparams);
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::XpBearerPreferenceInd(uint8_t transport)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGD("%s", __func__);
  msg->BearerPreference.eventId = XP_XM_BEARER_PREFERENCE_IND;
  msg->BearerPreference.transport = transport;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::XpSapPowerSave(uint8_t dialogId, uint8_t mode)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
  ALOGD("%s: with dialogId %d and mode :%s",
	__func__, dialogId, mode == 1 ? "enable": "disable");
  msg->SapPowerStateParams.eventId = XP_XM_SAP_POWER_STATE;
  msg->SapPowerStateParams.dialog_id = dialogId;
  msg->SapPowerStateParams.state = mode;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::HostParameters(macaddr_t macaddr, uint16_t Ethertype)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGD("%s", __func__);
  msg->HostParams.eventId = XP_XM_HOST_PARAMETERS;
  memcpy(&msg->HostParams.macaddr, &macaddr, sizeof(macaddr_t));
  msg->HostParams.Ethertype = Ethertype;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::SapState(uint16_t state)
{
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  ALOGD("%s: with state:%d", __func__, state);
  msg->SapState.eventId = XP_XM_SAP_CURRENT_STATE;
  msg->SapState.state = state;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::CreateSapInterface(uint8_t state)
{
  ALOGD("%s: with state:%s", __func__, state? "enable": "disable");
  xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);

  msg->CreateSapInfParams.eventId = XP_XM_CREATE_SAP_INF;
  msg->CreateSapInfParams.state = state;
  XpanManager::Get()->PostMessage(msg);
}

void XMXprofileIf::ProcessMessage(XmIpcEventId eventId, xm_ipc_msg_t *msg)
{
  ALOGD("%s %s", __func__, ConvertMsgtoString(eventId));
  switch (eventId) {
    case QHCI_XM_REMOTE_SUPPORT_XPAN: {
      QhciXmRemoteSupportXpan SupportsXpan = msg->SupportsXpan;
      RemoteSupportsXpan(SupportsXpan.bdaddr, SupportsXpan.is_supported);
      break;
    } case QHCI_XM_USECASE_UPDATE: {
      QhciXmUseCase UseCase = msg->UseCase;
      UseCaseUpdate(UseCase.usecase);
      break;
    } case QHCI_XM_PREPARE_AUDIO_BEARER_REQ: {
      QhciXmPrepareAudioBearerReq AudioBearerReq =  msg->AudioBearerReq;
      TransportType type = AudioBearerReq.type;
      bdaddr_t bdaddr = AudioBearerReq.bdaddr;
      XmXpPrepareAudioBearerReq(bdaddr, type); 
      break;
    } case WIFI_XM_ACS_RESULTS: {
      WiFiAcsResults(msg);
      break;
    } case QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ: {
      QhciXmUnPrepareAudioBearerReq UnPrepareAudioBearerReq =  msg->UnPrepareAudioBearerReq;
      bdaddr_t bdaddr = UnPrepareAudioBearerReq.bdaddr;
      TransportType type = UnPrepareAudioBearerReq.type;
      XmXpPrepareAudioBearerReq(bdaddr, type); 
      break;
    } case CP_XM_TRANSPORT_UPDATE : {
      TransportType transport = msg->TransportUpdate.transport;
      TransportUpdate(transport);
      break;
    } case WIFI_XM_TWT_EVENT: {
      WifiTwtEvent(msg);
      break;
    } case WIFI_XM_SAP_POWER_SAVE_STATE_RSP: {
      WifiSapPowerSaveEvent(msg);
      break;
    } case XM_XP_CREATE_SAP_INF_STATUS: {
      UpdateSapInterface(msg);
      break;
    } default: {
      ALOGI("%s: this :%04x ipc message is not handled", __func__, eventId);
    }
  }
}

/*typedef struct {
  XM_UpdateXpanBondedDevices* update_bonded_dev = NULL;
  XM_UpdateTransport* update_transport = NULL;
  XM_EnableSapAcs* enable_acs = NULL;
  XM_UpdateBearerSwitched* update_bearer_switched = NULL;
  XM_UpdateTwtSessionParams* update_twt_params = NULL;
  XM_UpdateBearerPrepared* update_bearer_prepared = NULL;
  XM_UpdateLowPowerMode* update_low_power_mode = NULL;
  XM_UpdateHostParams* update_host_params = NULL;
  XM_UpdateSapState* update_sap_state = NULL;
  XM_UpdateVbcPeriodicity* update_vbc_periodcity = NULL;
  XM_UpdateUiSwitchState* update_ui_switch_state = NULL;
  XM_CreateSapInterface* create_sap_interface = NULL;
} tXPAN_MANAGER_API;
*/

void XMXprofileIf::Initialize(void)
{
  XpanProviderCb = {
    .update_bonded_dev = UpdateXpanBondedDevices,
    .update_transport = TransportEnabled,
    .enable_acs = EnableAcs,
    .update_bearer_switched = XpXmBearerSwitchInd,
    .update_twt_params = UpdateTWTSessionParams,
    .update_bearer_prepared = XpXmPrepareAudioBearerRsp,
    .update_low_power_mode = XpSapPowerSave,
    .update_host_params = HostParameters,
    .update_sap_state = SapState,
    .create_sap_interface = CreateSapInterface,
  };

  ALOGI("%s: registering with XpanProviderIf", __func__);
  if(XpanProviderIf::GetIf())
    XpanProviderIf::GetIf()->RegisterXpanManagerIf(&XpanProviderCb);
 else
   ALOGE("%s: Failed to register with AIDL service", __func__);
}

void XMXprofileIf::Deinitialize(void)
{
 ALOGI("%s: Deregistering with XpanProviderIf", __func__);
 if(XpanProviderIf::GetIf())
   XpanProviderIf::GetIf()->DeregisterXpanManagerIf();
 else
   ALOGE("%s: Failed to Deregister with AIDL service", __func__);
}

} //namespace implementation
} //namespace xpan

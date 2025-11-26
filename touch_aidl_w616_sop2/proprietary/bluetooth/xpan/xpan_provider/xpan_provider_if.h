/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef XPAN_PROVIDER_IF_H
#define XPAN_PROVIDER_IF_H

#include "XpanProviderService.h"
#include "xpan_utils.h"

/* XPAN Provider Commands Opcodes */
enum {
  XPAN_BONDED_DEVICES_CMD,
  XPAN_UPDATE_TRANSPORT_CMD,
  XPAN_ENABLE_SAP_ACS_CMD,
  XPAN_UPDATE_BEARER_PREPARED_CMD,
  XPAN_UPDATE_TWT_SESSION_PARAMS_CMD,
  XPAN_UPDATE_BEARER_SWITCHED_CMD,
  XPAN_UPDATE_HOST_PARAMS_CMD,
  XPAN_UPDATE_LOW_POWER_MODE_CMD,
  XPAN_UPDATE_SAP_STATE_CMD,
  XPAN_UPDATE_VBC_PERIODICITY_CMD,
  XPAN_CREATE_SAP_INTERFACE_CMD,
  XPAN_UPDATE_UI_SWITCH_STATE_CMD,
  XPAN_UPDATE_BEARER_PREFERENCE_CMD,
  XPAN_UPDATE_CTS_REQUEST_CMD,
};

/* XPAN Provider Callback Events Opcodes */
enum {
  XPAN_DEVICES_FOUND_EVT,
  XPAN_USECASE_UPDATE_EVT,
  XPAN_ACS_UPDATE_EVT,
  XPAN_PREPARE_BEARER_EVT,
  XPAN_TWT_SESSION_ESTABLISHED_EVT,
  XPAN_TRANSPORT_UPDATED_EVT,
  XPAN_SAP_LOW_POW_MODE_UPDATE,
  XPAN_BEARER_SWITCH_INDICATION,
  XPAN_SAP_INTERFACE_CREATED,
  XPAN_UPDATE_BEARER_PREFERENCE_RES,
};

typedef struct {
  macaddr_t mac_addr;
  uint32_t SI;
  uint32_t SP;
  uint32_t location;
  bool isEstablished;
} __attribute__((packed)) tXPAN_Twt_Session_Params;

/* Bluetooth Address length */
#define BD_ADDR_LENGTH 6

/* Opcode index */
#define OPCODE_INDEX 0

#define LENGTH_WITHOUT_DATA 1

/* To update bonded xpan devices */
typedef void (XM_UpdateXpanBondedDevices) (uint8_t numOfDevices, bdaddr_t devices[]);

/* Used to indicate when a given transport is enabled/disabled in upper layers */
typedef void (XM_UpdateTransport) (bdaddr_t addr, TransportType transport,
              bool isEnabled, uint8_t reason);

/* Used to start ACS algorith for SAP */
typedef void (XM_EnableSapAcs) (std::vector<uint32_t> freqList);

/* Received when remote device responds to Bearer Switch request */
typedef void (XM_UpdateBearerSwitched) (bdaddr_t addr, uint8_t bearer,
                                        uint8_t status);

/* API used to get Twt Session details with remote device details */
typedef void (XM_UpdateTwtSessionParams) (uint8_t num_devices,
                                          std::vector<tXPAN_Twt_Session_Params> twt_params);

/* Indication from Xpan Profile that Bearer has been prepared */
typedef void (XM_UpdateBearerPrepared) (bdaddr_t addr, uint8_t bearer,
                                        uint8_t status);

/* To update SAP Lower Mode status */
typedef void (XM_UpdateLowPowerMode) (uint8_t dialogId, uint8_t mode);

/* To update Host Params */
typedef void (XM_UpdateHostParams) (macaddr_t mac, uint16_t etherType);

/* To update SAP state */
typedef void (XM_UpdateSapState) (uint16_t sapState);

/* To update Voice back channel periodicity */
typedef void (XM_UpdateVbcPeriodicity) (uint16_t periodicity, bdaddr_t addr);

/* To update Xpan UI switch state */
typedef void (XM_UpdateUiSwitchState) (uint16_t state);

/* To create SAP interface */
typedef void (XM_CreateSapInterface) (uint8_t state);

/* Received Bearer preference indication from peer */
typedef void (XM_BearerPreferenceReq) (bdaddr_t addr, uint8_t bearer);

/* Received Clear to send indication from peer */
typedef void (XM_ClearToSendReq) (bdaddr_t addr, uint8_t req);


/* Xpan Manager interface API's */
typedef struct {
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
  XM_BearerPreferenceReq* update_bearer_preference_req = NULL;
  XM_ClearToSendReq* update_clear_to_send_req = NULL;
} tXPAN_MANAGER_API;

namespace bluetooth {
namespace xpanprovider {

class XpanProviderIf {
 public:
  virtual ~XpanProviderIf() = default;;
  static void Initialize(
      aidl::vendor::qti::hardware::bluetooth::xpanprovider::XpanProviderService* service);
  static XpanProviderIf* GetIf();
  virtual void RegisterXpanManagerIf (tXPAN_MANAGER_API *xmIf) = 0;
  virtual void DeregisterXpanManagerIf () = 0;
  virtual void XpanDeviceFoundCb (bdaddr_t bdAddr) = 0;
  virtual void UsecaseUpdateCb (bdaddr_t bdAddr, UseCaseType usecase) = 0;
  virtual void PrepareBearerCb (bdaddr_t bdAddr, uint8_t bearer) = 0;
  virtual void TwtSessionEstablishedCb (macaddr_t macAddress, uint32_t sp,
                                        uint32_t si, uint8_t eventType) = 0;
  virtual void TransportUpdatedCb (TransportType transport) = 0;
  virtual void AcsUpdateCb (uint8_t status, uint32_t frequency) = 0;
  virtual void SapLowPowerModeUpdateCb (uint8_t id, uint16_t power_save_bi_multiplier,
                                        uint64_t nextTsf) = 0;
  virtual void BearerSwitchIndicationCb (uint8_t bearerType, uint8_t status) = 0;
  virtual void SapInterfaceCreatedCb (uint8_t state, uint8_t status) = 0;
  virtual void BearerPreferenceResCb (bdaddr_t bdAddr, uint8_t status) = 0;
};

} // namespace bluetooth
} // namespace xpanprovider

#endif

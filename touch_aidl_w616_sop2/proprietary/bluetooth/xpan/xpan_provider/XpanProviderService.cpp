/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <log/log.h>
#include <android/binder_auto_utils.h>
#include <android-base/logging.h>
#include <vector>
#include "XpanProviderService.h"
#include "xpan_provider_if.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "vendor.qti.hardware.bluetooth.xpanprovider"

using aidl::vendor::qti::hardware::bluetooth::xpanprovider::XpanProviderService;
using bluetooth::xpanprovider::XpanProviderIf;

tXPAN_MANAGER_API *xm_intf = NULL;

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth {
namespace xpanprovider {

/* Used to register Xpan Profile Callback */
::ndk::ScopedAStatus XpanProviderService::registerXpanCallbacks(
        const std::shared_ptr<IXpanProviderCallback>& in_cb) {
  ALOGD("%s", __func__);
  xpanProviderCb = in_cb;

  AIBinder_DeathRecipient* deathRecipient = AIBinder_DeathRecipient_new(
      XpanProviderService::clientDeathRecipient);

  if (deathRecipient != NULL && in_cb != NULL) {
    auto status = AIBinder_linkToDeath(in_cb->asBinder().get(), deathRecipient,
                                       reinterpret_cast<void*>(this));
    if (status != STATUS_OK) {
      ALOGE("%s: Failed to register DeathRecipient with error(%d)", __func__, status);
      // no action needed
    }
  } else {
      ALOGE("%s: Failed ", __func__);
  }

  return ::ndk::ScopedAStatus::ok();
}

/* This API receives XPAN command from Xpan Profile and parses it */
::ndk::ScopedAStatus XpanProviderService::sendXpanCommand(
    const std::vector<uint8_t>& cmd) {
  if (cmd.size() < LENGTH_WITHOUT_DATA) {
    ALOGE("%s: Invalid data received.", __func__);
    return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
  }

  uint8_t opcode = (uint8_t)cmd[OPCODE_INDEX];

  std::vector<uint8_t> cmd_data{};
  if (cmd.size() != LENGTH_WITHOUT_DATA) {
    cmd_data.insert(cmd_data.begin(), cmd.begin() + 1, cmd.end());
  }

  switch(opcode) {
    case XPAN_BONDED_DEVICES_CMD:
        updateXpanBondedDevices(cmd_data);
        break;

    case XPAN_UPDATE_TRANSPORT_CMD:
        updateTransport(cmd_data);
        break;

    case XPAN_ENABLE_SAP_ACS_CMD:
        enableSapAcs(cmd_data);
        break;

    case XPAN_UPDATE_BEARER_PREPARED_CMD:
        updateBearerPrepared(cmd_data);
        break;

    case XPAN_UPDATE_TWT_SESSION_PARAMS_CMD:
        updateTwtSessionParams(cmd_data);
        break;

    case XPAN_UPDATE_BEARER_SWITCHED_CMD:
        updateBearerSwitched(cmd_data);
        break;

    case XPAN_UPDATE_HOST_PARAMS_CMD:
        updateHostParams(cmd_data);
        break;

    case XPAN_UPDATE_LOW_POWER_MODE_CMD:
        updateLowPowerMode(cmd_data);
        break;

    case XPAN_UPDATE_SAP_STATE_CMD:
        updateSapState(cmd_data);
        break;

    case XPAN_UPDATE_VBC_PERIODICITY_CMD:
        updateVbcPeriodicity(cmd_data);
        break;

    case XPAN_UPDATE_UI_SWITCH_STATE_CMD:
        updateUISwitchState(cmd_data);
        break;

    case XPAN_CREATE_SAP_INTERFACE_CMD:
        createSapInterface(cmd_data);
        break;

    case XPAN_UPDATE_BEARER_PREFERENCE_CMD:
        updateBearerPreferenceReq(cmd_data);
        break;

    case XPAN_UPDATE_CTS_REQUEST_CMD:
        updateClearToSendReq(cmd_data);
        break;

    default:
        ALOGW("%s: Unhandled Command(%02x) Received", __func__, opcode);
  }

  return ::ndk::ScopedAStatus::ok();
}


XpanProviderService::XpanProviderService() {
  XpanProviderIf::Initialize(this);
}

XpanProviderService::~XpanProviderService() {
  xpanProviderCb = NULL;
}

/* Xpan Profile death recipient */
void XpanProviderService::clientDeathRecipient(void* cookie) {
  ALOGD("%s: Xpan Provider Client (Profile) died..", __func__);

  auto* svc = static_cast<XpanProviderService*>(cookie);
  svc->xpanProviderCb = NULL;
}

/* API to receive Bonded XPan Devices from Profile */
void XpanProviderService::updateXpanBondedDevices(const std::vector<uint8_t>& data) {
  uint8_t bytesParsed = 0;

  if (data.empty()) {
    ALOGE("%s: Empty data received for the command", __func__);
    return;
  }

  // expectedLength = numOfDevices (1) + numOfDevices x 6
  uint8_t numOfDevices = data[bytesParsed++];
  uint16_t remLength = numOfDevices * BD_ADDR_LENGTH;

  if (remLength != data.size() - 1) {
    ALOGE("%s:Invalid length(%d) data received", __func__, remLength);
    return;
  }

  bdaddr_t* list = new bdaddr_t[numOfDevices];
  for (int i = 0; i < numOfDevices; i++) {
    std::copy(data.begin() + bytesParsed, data.begin()
              + bytesParsed + BD_ADDR_LENGTH, list[i].b);
    bytesParsed += BD_ADDR_LENGTH;
  }

  ALOGD("%s: numOfDevices = %d", __func__, numOfDevices);
  for (int i = 0; i < numOfDevices; i++) {
    ALOGD("%s: Device %d = %02X:%02X:%02X:%02X:%02X:%02X", __func__, (i + 1),
        list[i].b[5], list[i].b[4], list[i].b[3],
        list[i].b[2], list[i].b[1], list[i].b[0]);
  }

  if (xm_intf && xm_intf->update_bonded_dev) {
    ALOGD("%s: xm_intf = %p", __func__, xm_intf);
    xm_intf->update_bonded_dev(numOfDevices, list);
  }
  //TODO: release list
}

/* Used to indicate when a given transport is enabled/disabled in upper layers */
void XpanProviderService::updateTransport(const std::vector<uint8_t>& data) {
  /* expectedLength =  address(6) + transport(1)
                      + isEnabled(1) + reason (1) */
  uint8_t bytesParsed = 0, expectedLength = 9;
  bdaddr_t addr;
  TransportType transport;
  uint8_t reason;
  bool isEnabled;

  if (data.size() != expectedLength) {
    ALOGE("%s:Invalid length(%d) data received", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;
  transport = static_cast<TransportType>(data[bytesParsed++]);
  isEnabled = (bool)data[bytesParsed++];
  reason = data[bytesParsed];

  ALOGD("%s: transport = %d, isEnabled = %d, reason = %d"
        " address = %02X:%02X:%02X:%02X:%02X:%02X", __func__
        , transport, isEnabled, reason, addr.b[5], addr.b[4],
        addr.b[3], addr.b[2], addr.b[1], addr.b[0]);

  if (xm_intf && xm_intf->update_transport) {
    xm_intf->update_transport(addr, transport, isEnabled, reason);
  }
}

/* Used to start ACS algorith for SAP */
void XpanProviderService::enableSapAcs(const std::vector<uint8_t>& data) {
  /* expectedLength = (4*list_size) + freqListSize(1) */
  uint8_t bytesParsed = 0;

  if (data.empty()) {
    ALOGE("%s: Empty data received for the command", __func__);
    return;
  }

  uint32_t freqListSize = data[bytesParsed++];
  freqListSize |= (data[bytesParsed++] << 8);
  freqListSize |= (data[bytesParsed++] << 16);
  freqListSize |= (data[bytesParsed++] << 24);

  std::vector<uint32_t> freqList;

  if (data.size() - sizeof(uint32_t) != sizeof(uint32_t) * freqListSize) {
    ALOGE("%s: Incorrect frequency list received", __func__);
    return;
  }

  for (int i = 0; i < freqListSize; i++) {
    uint32_t freq = 0;
    freq = freq | (data[bytesParsed++]);
    freq = freq | (data[bytesParsed++] << 8);
    freq = freq | (data[bytesParsed++] << 16);
    freq = freq | (data[bytesParsed++] << 24);
    freqList.push_back(freq);
  }

  ALOGD("%s: freqListSize = %d", __func__, freqListSize);

  if (xm_intf && xm_intf->enable_acs) {
    xm_intf->enable_acs(freqList);
  }
}

/* Received when remote device responds to Bearer Switch request */
void XpanProviderService::updateBearerSwitched(const std::vector<uint8_t>& data) {
  /* expectedLength = address(6) + bearer(1) + status (1)*/
  uint8_t bytesParsed = 0, expectedLength = 8;
  bdaddr_t addr;
  uint8_t bearer, status;

  if (data.size() != expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;
  bearer = data[bytesParsed++];
  status = data[bytesParsed];

  ALOGD("%s: address(%02X:%02X:%02X:%02X:%02X:%02X), bearer = %d status = %d", __func__,
        addr.b[5], addr.b[4], addr.b[3], addr.b[2],
        addr.b[1], addr.b[0], bearer, status);

  if (xm_intf && xm_intf->update_bearer_switched) {
    xm_intf->update_bearer_switched(addr, bearer, status);
  }
}

/* API used to get Twt Session details with remote device details */
void XpanProviderService::updateTwtSessionParams(const std::vector<uint8_t>& data) {
  /* expectedLength = numOfDevices x sizeof(tXPAN_Twt_Session_Params) */
  uint8_t bytesParsed = 0, numOfDevices = 0;
  std::vector<tXPAN_Twt_Session_Params> remoteDevicesTwtParams;

  if (data.empty()) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  numOfDevices = data[bytesParsed++];
  ALOGD("%s: numOfDevices (%d)", __func__, numOfDevices);

  if (numOfDevices != 0
       && (data.size() - 1 != (numOfDevices * sizeof(tXPAN_Twt_Session_Params)))) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  for (int i = 0; i < numOfDevices; i++) {
    tXPAN_Twt_Session_Params twtParams;
    std::copy(data.begin() + bytesParsed,
              data.begin() + bytesParsed + sizeof(macaddr_t),
              twtParams.mac_addr.b);
    bytesParsed += sizeof(macaddr_t);
    twtParams.location = (data[bytesParsed]) | (data[bytesParsed + 1] << 8) |
                         (data[bytesParsed + 2] << 16) | (data[bytesParsed + 3] << 24);
    bytesParsed += sizeof(uint32_t);
    twtParams.isEstablished = data[bytesParsed++];
    twtParams.SI = (data[bytesParsed]) | (data[bytesParsed + 1] << 8) |
         (data[bytesParsed + 2] << 16) | (data[bytesParsed + 3] << 24);
    bytesParsed += sizeof(uint32_t);
    twtParams.SP = (data[bytesParsed]) | (data[bytesParsed + 1] << 8) |
         (data[bytesParsed + 2] << 16) | (data[bytesParsed + 3] << 24);
    bytesParsed += sizeof(uint32_t);

    ALOGD("%s: mac (%02X:%02X:%02X:%02X:%02X:%02X), location = %d, "
          "isEstablished = %d, SI = %d, SP = %d", __func__,
          twtParams.mac_addr.b[5], twtParams.mac_addr.b[4], twtParams.mac_addr.b[3],
          twtParams.mac_addr.b[2], twtParams.mac_addr.b[1], twtParams.mac_addr.b[0],
          twtParams.location, twtParams.isEstablished, twtParams.SI, twtParams.SP);
    remoteDevicesTwtParams.push_back(twtParams);
  }

  if (xm_intf && xm_intf->update_twt_params) {
    xm_intf->update_twt_params(numOfDevices, remoteDevicesTwtParams);
  }
}

/* Indication from Xpan Profile that Bearer has been prepared */
void XpanProviderService::updateBearerPrepared(const std::vector<uint8_t>& data) {
  /* expectedLength = address(6) + bearer(1) + status(1)*/
  uint8_t bytesParsed = 0, expectedLength = 8;
  bdaddr_t addr;
  uint8_t bearer, status;

  if (data.size() != expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;
  bearer = data[bytesParsed++];
  status = data[bytesParsed];

  ALOGD("%s: address(%02X:%02X:%02X:%02X:%02X:%02X), bearer = %d, status = %d", __func__,
        addr.b[5], addr.b[4], addr.b[3], addr.b[2],
        addr.b[1], addr.b[0], bearer, status);

  if (xm_intf && xm_intf->update_bearer_prepared) {
    xm_intf->update_bearer_prepared(addr, bearer, status);
  }
}

/* To update SAP Lower Mode status */
void XpanProviderService::updateLowPowerMode(const std::vector<uint8_t>& data) {
  /* expectedLength = dialog_id(1) + status(1) */
  uint8_t expectedLength = 2, bytesParsed = 0, dialogId, mode;

  if (data.size() != expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  dialogId = data[bytesParsed++];
  mode = data[bytesParsed];

  if (xm_intf && xm_intf->update_low_power_mode) {
    xm_intf->update_low_power_mode(dialogId, mode);
  }
}

/* To update device SoftAp Mac Address and Ether type */
void XpanProviderService::updateHostParams(const std::vector<uint8_t>& data) {
  /* expectedLength = status(1) */
  uint8_t bytesParsed = 0;
  uint16_t etherType = 0;
  macaddr_t macAddr;

  if (data.size() != sizeof(macaddr_t) + sizeof(uint16_t)) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + sizeof(macaddr_t), macAddr.b);
  bytesParsed += sizeof(macaddr_t);
  etherType = (data[bytesParsed]) | (data[bytesParsed + 1] << 8);

  if (xm_intf && xm_intf->update_host_params) {
    xm_intf->update_host_params(macAddr, etherType);
  }
}

/* To update device Device SAP state */
void XpanProviderService::updateSapState(const std::vector<uint8_t>& data) {
  /* expectedLength = state(1) */
  uint8_t bytesParsed = 0;

  if (data.size() !=  sizeof(uint8_t)) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  uint8_t state = (data[bytesParsed]) | (data[bytesParsed + 1] << 8);

  if (xm_intf && xm_intf->update_sap_state) {
    xm_intf->update_sap_state(state);
  }
}

/* To update device Voice back channel periodicity */
void XpanProviderService::updateVbcPeriodicity(const std::vector<uint8_t>& data) {
  /* expectedLength = Periodcity channel(1) + address(6)*/
  uint8_t bytesParsed = 0;
  uint8_t expectedLength = 7;
  bdaddr_t addr;
  if (data.size() !=  expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  uint8_t periodcity = (data[bytesParsed]) | (data[bytesParsed + 1] << 8);
  std::copy(data.begin()+1, data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;

  ALOGD("%s: address(%02X:%02X:%02X:%02X:%02X:%02X), periodcity = %d, ", __func__,
        addr.b[5], addr.b[4], addr.b[3], addr.b[2],
        addr.b[1], addr.b[0], periodcity);

  if (xm_intf && xm_intf->update_vbc_periodcity) {
    xm_intf->update_vbc_periodcity(periodcity, addr);
  }
}

/* To update Xpan UI Switch state */
void XpanProviderService::updateUISwitchState(const std::vector<uint8_t>& data) {
  /* expectedLength = state(1) */
  uint8_t bytesParsed = 0;

  if (data.size() !=  sizeof(uint8_t)) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  uint8_t state = data[bytesParsed];

  if (xm_intf && xm_intf->update_ui_switch_state) {
    xm_intf->update_ui_switch_state(state);
  }
}

/* To create SAP Interface for XPAN use case */
void XpanProviderService::createSapInterface(const std::vector<uint8_t>& data) {
  ALOGD("%s", __func__);

  /* expectedLength = state(1) */
  uint8_t bytesParsed = 0;

  if (data.size() !=  sizeof(uint8_t)) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  uint8_t isCreate = data[bytesParsed];

  if (xm_intf && xm_intf->create_sap_interface) {
    xm_intf->create_sap_interface(isCreate);
  }
}

/* Indication from Xpan Profile that Received Bearer preference from Peer */
void XpanProviderService::updateBearerPreferenceReq(const std::vector<uint8_t>& data) {
  /* expectedLength = address(6) + bearer(1)*/
  uint8_t bytesParsed = 0, expectedLength = 7;
  bdaddr_t addr;
  uint8_t bearer;

  if (data.size() != expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;
  bearer = data[bytesParsed];

  ALOGD("%s: address(%02X:%02X:%02X:%02X:%02X:%02X), bearer = %d ", __func__,
        addr.b[5], addr.b[4], addr.b[3], addr.b[2],
        addr.b[1], addr.b[0], bearer);

  if (xm_intf && xm_intf->update_bearer_preference_req) {
    xm_intf->update_bearer_preference_req(addr, bearer);
  }
}

/* Indication from Xpan Profile that Received Clear to send request from Peer */
void XpanProviderService::updateClearToSendReq(const std::vector<uint8_t>& data) {
  /* expectedLength = address(6) + bearer(1)*/
  uint8_t bytesParsed = 0, expectedLength = 7;
  bdaddr_t addr;
  uint8_t reqStatus;

  if (data.size() != expectedLength) {
    ALOGE("%s: Incorrect parameters length (%d)", __func__, data.size());
    return;
  }

  std::copy(data.begin(), data.begin() + BD_ADDR_LENGTH, addr.b);
  bytesParsed += BD_ADDR_LENGTH;
  reqStatus = data[bytesParsed];

  ALOGD("%s: address(%02X:%02X:%02X:%02X:%02X:%02X), reqStatus = %d ", __func__,
        addr.b[5], addr.b[4], addr.b[3], addr.b[2],
        addr.b[1], addr.b[0], reqStatus);

  if (xm_intf && xm_intf->update_clear_to_send_req) {
    xm_intf->update_clear_to_send_req(addr, reqStatus);
  }
}


/* This API converts Xpan Manager callback to appropriate Event stream */
void XpanProviderService::sendXpanEvent(const std::vector<uint8_t>& data) {
  ALOGD("%s", __func__);

  if (xpanProviderCb == NULL) {
    ALOGE("%s: XpanProvider Client not bound to the service", __func__);
    return;
  }

  xpanProviderCb->xpanEventReceivedCb(data);
}

} // xpanprovider
} // bluetooth
} // hardware
} // qti
} // vendor
} // aidl

namespace {

/* To translate byte stream to bdaddr_t */
static void addBdAddrToEventData(std::vector<uint8_t> &data, bdaddr_t bdAddr) {
  for (int i = 0; i < BD_ADDR_LENGTH; i++) {
    data.push_back(bdAddr.b[i]);
  }
}

/* To translate byte stream to macaddr_t */
static void addMacAddrToEventData(std::vector<uint8_t> &data, macaddr_t macAddr) {
  for (int i = 0; i < BD_ADDR_LENGTH; i++) {
    data.push_back(macAddr.b[i]);
  }
}

/* To translate uint32_t to byte stream */
static void addUint16ToEventData(std::vector<uint8_t> &data, uint16_t val) {
  data.push_back((uint8_t)(0xFF & (val)));
  data.push_back((uint8_t)(0xFF & (val >> 8)));
}

/* To translate uint32_t to byte stream */
static void addUint32ToEventData(std::vector<uint8_t> &data, uint32_t val) {
  data.push_back((uint8_t)(0xFF & (val)));
  data.push_back((uint8_t)(0xFF & (val >> 8)));
  data.push_back((uint8_t)(0xFF & (val >> 16)));
  data.push_back((uint8_t)(0xFF & (val >> 24)));
}

/* To translate uint64_t to byte stream */
static void addUint64ToEventData(std::vector<uint8_t> &data, uint64_t val) {
  data.push_back((uint8_t)(0xFF & (val)));
  data.push_back((uint8_t)(0xFF & (val >> 8)));
  data.push_back((uint8_t)(0xFF & (val >> 16)));
  data.push_back((uint8_t)(0xFF & (val >> 24)));
  data.push_back((uint8_t)(0xFF & (val >> 32)));
  data.push_back((uint8_t)(0xFF & (val >> 40)));
  data.push_back((uint8_t)(0xFF & (val >> 48)));
  data.push_back((uint8_t)(0xFF & (val >> 56)));
}
} // namespace anonymous

namespace bluetooth {
namespace xpanprovider {

class XpanProviderIfImpl;
XpanProviderIfImpl *instance = NULL;

class XpanProviderIfImpl: public XpanProviderIf {
 public:
  XpanProviderService* service_;

  XpanProviderIfImpl(XpanProviderService* service) {
    service_ = service;
  }

  ~XpanProviderIfImpl() {
    service_ = NULL;
    instance = NULL;
  }

/* Callback from XpanManager when xpan supporting device is connected */
  void RegisterXpanManagerIf(tXPAN_MANAGER_API *xmIf) {
    ALOGD("%s", __func__);
    xm_intf = xmIf;
    ALOGD("%s: xm_intf = %p", __func__, xm_intf);
  }

  void DeregisterXpanManagerIf() {
    ALOGD("%s", __func__);
    xm_intf = NULL;
  }

  /* Callback from XpanManager when xpan supporting device is connected */
  void XpanDeviceFoundCb (bdaddr_t bdAddr) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_DEVICES_FOUND_EVT);
    addBdAddrToEventData(data, bdAddr);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  /* Callback from XpanManager when xpan use case is updated */
  void UsecaseUpdateCb (bdaddr_t bdAddr, UseCaseType usecase) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_USECASE_UPDATE_EVT);
    addBdAddrToEventData(data, bdAddr);
    data.push_back(static_cast<uint8_t>(usecase));

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  /* Callback from XpanManager when bearer needs to be prepared in upper layer */
  void PrepareBearerCb (bdaddr_t bdAddr, uint8_t bearer) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_PREPARE_BEARER_EVT);
    addBdAddrToEventData(data, bdAddr);
    data.push_back(bearer);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  /* Callback from XpanManager when Twt Session is established with remote device */
  void TwtSessionEstablishedCb (macaddr_t macAddress, uint32_t sp,
                                uint32_t si, uint8_t eventType) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_TWT_SESSION_ESTABLISHED_EVT);
    addMacAddrToEventData(data, macAddress);
    addUint32ToEventData(data, sp);
    addUint32ToEventData(data, si);
    data.push_back(eventType);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  /* Callback from XpanManager when xpan transport is updated properly after bearer switch */
  void TransportUpdatedCb (TransportType transport) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_TRANSPORT_UPDATED_EVT);
    data.push_back(static_cast<uint8_t>(transport));

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  /* Callback from XpanManager when ACS algorithm is completed */
  void AcsUpdateCb (uint8_t status, uint32_t frequency) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_ACS_UPDATE_EVT);
    data.push_back(status);
    addUint32ToEventData(data, frequency);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  void SapLowPowerModeUpdateCb (uint8_t id, uint16_t power_save_bi_multiplier,
                                uint64_t nextTsf) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_SAP_LOW_POW_MODE_UPDATE);
    data.push_back(id);
    addUint16ToEventData(data, power_save_bi_multiplier);
    addUint64ToEventData(data, nextTsf);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  void BearerSwitchIndicationCb (uint8_t bearerType, uint8_t status) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_BEARER_SWITCH_INDICATION);
    data.push_back(bearerType);
    data.push_back(status);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  void SapInterfaceCreatedCb (uint8_t state, uint8_t status) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_SAP_INTERFACE_CREATED);
    data.push_back(state);
    data.push_back(status);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

  void BearerPreferenceResCb (bdaddr_t bdAddr, uint8_t bearer) {
    ALOGD("%s", __func__);

    std::vector<uint8_t> data;
    data.push_back(XPAN_UPDATE_BEARER_PREFERENCE_RES);
    addBdAddrToEventData(data, bdAddr);
    data.push_back(bearer);

    if (service_) {
      service_->sendXpanEvent(data);
    }
  }

};

} // xpanprovider
} // namespace

/* Provides XpanProviderIf to Xpan Manager for giving callback to Xpan Profile*/
XpanProviderIf* XpanProviderIf::GetIf() {
  return instance;
}

void XpanProviderIf::Initialize(XpanProviderService* service) {
  ALOGI("%s", __func__);
  instance = new XpanProviderIfImpl(service);
}

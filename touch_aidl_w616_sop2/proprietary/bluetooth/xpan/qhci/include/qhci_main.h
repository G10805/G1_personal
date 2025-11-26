/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#ifndef QHCI_MAIN_H
#define QHCI_MAIN_H

#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <string>
#include "qhci_packetizer.h"
#include <hidl/HidlSupport.h>
#include "hci_transport.h"
#include "qhci_xm_if.h"
#include "xpan_utils.h"


//#include "logger.h"

#define QHCI_CREATE_CIS_CMD_SIZE 4
#define QHCI_CIS_ESTABLISHED_EVT_SIZE 31

#define HCI_LE_ADD_DEVICE_TO_WHITE_LIST 0x2011
#define HCI_LE_SET_CIG_PARAMETERS 0x2062
#define HCI_LE_SETUP_ISO_DATA_PATH 0x206E
#define HCI_LE_CREATE_CIS 0x2064
#define HCI_DISCONNECT 0x0406
#define HCI_LE_REMOVE_CIG 0x2065
#define HCI_VENDOR_USECASE_UPDATE 0xFC0A
#define HCI_QLE_SET_HOST_FEATURE 0xFC51
#define HCI_SUB_OPCODE_QLE_SET_HOST_FEATURE 0x14
#define QHCI_SET_HOST_SUPPORTED_BIT_COMMAND_LEN 6

#define QHCI_DISCONNECT_COMMAND_LEN 6

#define VSC_QHCI_VENDOR_OPCODE 0x15
#define VSC_QHCI_VENDOR_USECASE_EVT_LEN 9
#define HCI_ENCRYPT_CHANGE_EVT_LEN 6

#define QHCI_DELAY_REPORT_EVT_LEN 6
#define QHCI_CMD_STATUS_CIS_LEN 6

#define HCI_DISCONNECT_CMPL_EVT 0x05
#define HCI_READ_REMOTE_VERSION_CMPL_EVT 0x0C
#define HCI_LE_EVT 0x3E
#define HCI_VENDOR_EVT 0xFF
#define QHCI_QBCE_SUBEVENT_CODE 0x51
#define READ_REMOTE_QLL_FEATURE_CMPL_EVT 0x06
#define HCI_ENCRYPT_CHANGE_EVT 0x08

#define QHCI_COMMAND_COMPLETE_EVENT 0x0E

#define HCI_LE_CONN_CMPL_EVT 0x0A
#define HCI_LE_EXT_ADV_EVT 0x0D
#define HCI_LE_READ_REMOTE_FEAT_CMPL_EVT 0x04
#define HCI_LE_READ_REMOTE_VERSION_EVT 0x0C
#define HCI_LE_CIS_ESTABLISHED_EVT 0x19

#define QHCI_AUDIO_BEARER_REQ_SIZE 100
#define QHCI_AUDIO_BEARER_RSP_SIZE 100
#define QHCI_USECASE_UPDATE_CONFIRM_SIZE 100

#define QHCI_TRANSPORT_NONE 0
#define QHCI_BT_TRANSPORT 1
#define QHCI_XPAN_TRANSPORT 2

#define QHCI_BT_SUCCESS 0

#define QHCI_BT_MIN_VERSION_SUPPORT 13
#define QHCI_BT_MIN_LMP_VERSION_SUPPORT 0x521D
#define QHCI_BT_MANUFACTURE_SUPPORT_ID 0x001D

#define QHCI_VS_QLL_CMD_REQ_LEN 6
#define QHCI_VS_QBCE_READ_REMOTE_QLL_SUPPORTED_FEATURES 0x3

#define QHCI_PKT_MESSAGE_LENGTH 100

using ::xpan::implementation::QHciPacketizer;

typedef enum {
  QHCI_IDLE_STATE = 0,
  QHCI_BT_CLOSE_XPAN_CLOSE,
  QHCI_BT_CLOSE_XPAN_CONNECTING,
  QHCI_BT_CLOSE_XPAN_OPEN,
  QHCI_BT_OPEN_XPAN_CLOSE,
  QHCI_BT_OPEN_XPAN_CONNECTING,
  QHCI_BT_OPEN_XPAN_OPEN,
} QHCI_CSM_STATE;

typedef enum {
  QHCI_CSM_LE_CONN_CMPL_EVT,
  QHCI_CSM_LE_CLOSE_EVT,
  QHCI_CSM_CIS_OPEN_XPAN_TRANS_DISABLE_EVT,
  QHCI_CSM_CIS_OPEN_XPAN_TRANS_ENABLE_EVT,
  QHCI_CSM_CIS_CLOSE_XPAN_TRANS_ENABLE_EVT,
  QHCI_CSM_XPAN_CONN_FAILED_EVT,
  QHCI_CSM_CIS_DISCONNECT_EVT,
  QHCI_CSM_USECASE_XPAN_TRANS_ENABLE_EVT,
  QHCI_CSM_USECASE_XPAN_TRANS_DISABLE_EVT,
  QHCI_CSM_UPDATE_TRANS_XPAN_EVT,
  QHCI_CSM_PREPARE_BEARER_BT,
  QHCI_CSM_PREPARE_BEARER_XPAN,
} QHCI_CSM_EVENT;

#define GAME_USECASE 0x08
#define HD_USECASE 0x04
#define LOSSLESS_USECASE 0x0A //Yet to check with Audio team

#define LE_UNICAST_HQ_PROFILE 0
#define LE_BROADCAST_HQ_PROFILE 1
#define LE_UNICAST_GAMING_PROFILE 2
#define XPAN_WIFI_HQ_PROFILE 3
#define XPAN_WIFI_GAMING_PROFILE 4


#define BT_LE_PUBLIC_ADDR_TYPE 0
#define BT_LE_RANDOM_ADDR_TYPE 1

typedef struct
{
  uint8_t cis_count;
  uint8_t cig_id;
  uint16_t cis_handles[2] = {0};
} qhci_cig_params_t;

typedef struct
{
  uint8_t cig_sync_delay[3]; //0x002fc2
  uint8_t cis_sync_delay[3]; //0x002fc2
  uint8_t trans_latency_m_s[3]; //0x014132
  uint8_t trans_latency_s_m[3]; //0x0056d2
  uint8_t phy_m_s;
  uint8_t phy_s_m;
  uint8_t num_sub_events;
  uint8_t burst_num_m_s;
  uint8_t burst_num_s_m;
  uint8_t flush_timeout_m_s;
  uint8_t flush_timeout_s_m;
  uint8_t max_pdu_size_m_s[2];
  uint8_t max_pdu_size_s_m[2];
  uint8_t iso_interval[2];
} qhci_cis_establish_evt_data_t;

typedef struct
{
  bool qll_send_from_host = false;
  bool is_xpan_device = false;
  QHCI_CSM_STATE state;
  uint16_t handle;
  uint16_t active_cis_handle;
  bdaddr_t rem_addr;
  uint8_t addr_type;
  qhci_cig_params_t cig_params;

  /* false - if create cis triggered from QHCI
   * true -  if create cis triggered from Stack */
  bool is_create_cis_from_stack;

  bool is_cis_established;

  /* false - XPAN Disabled means BT Transport
   * true - XPAN Enabled*/
  bool transport_enable;
  /* 0 - Default BT Transport
   * 1 -  XPAN Transport */
  uint8_t update_transport;

  TransportType current_transport;

  /* false - is prepare bearer not triggered from QHCI
   * true -  prepare bearer request triggered from QHCI */
  bool is_prepare_bearer_triggered;

  /* false - is create_cis not triggered from QHCI to SOC
   * true -  create_cis triggered from QHCI tto SOC*/
  bool is_create_cis_from_qhci_to_soc;

  bool is_xpan_usecase;

  qhci_cis_establish_evt_data_t qhci_cis_establish_evt_data;

  bool is_cis_disconnect_pending_from_soc;
} qhci_dev_cb_t;

namespace xpan {
namespace implementation {
using ::android::hardware::hidl_vec;


enum QHciTimerState {
  QHCI_TIMER_NOT_CREATED = 0x00,
  QHCI_TIMER_CREATED,
  QHCI_TIMER_ACTIVE,
  QHCI_TIMER_CLOSE,
};

typedef struct {
  QHciTimerState timer_state;
  timer_t timer_id;
} QHciCmdTimer;

class QHci {
  public:
    QHci();
    ~QHci();
    int Init(void);
    static std::shared_ptr<QHci> Get(void);
    int DeInit(void);
    void FromDataHandler(void);
    bool IsQhciRxPkt(const hidl_vec<uint8_t>*hidl_data);
    void ProcessRxPktEvent(HciPacketType type,
                                    const hidl_vec < uint8_t > * hidl_data);
    void ProcessTxPktCmd(const uint8_t *data, size_t length);
    bool IsQhciTxPkt(const uint8_t *data, size_t length);
    void PostMessage(qhci_msg_t *msg);

  private:
    void QHciMainThreadRoutine();
    static void curr_usr_handler(int);
    char * ConvertMsgtoString(uint8_t);
    void QHciMsgHandler(qhci_msg_t *msg);
    void SendRxPktToHost(qhci_msg_t *msg);
    bool ProcessRxLogger(qhci_msg_t *msg);
    void QHciHandleIdleState(qhci_dev_cb_t *rem_info, uint8_t event);
    void QHciHandleBtCloseXpanClose(qhci_dev_cb_t *rem_info, uint8_t event);
    void QHciHandleBtCloseXpanConnecting(qhci_dev_cb_t *rem_info,
                                                      uint8_t event);
    void QHciHandleBtCloseXpanOpen(qhci_dev_cb_t *rem_info,
                                              uint8_t event);
    void QHciHandleBtOpenXpanClose(qhci_dev_cb_t *rem_info,
                                             uint8_t event);
    void QHciHandleBtOpenXpanConnecting(qhci_dev_cb_t *rem_info,
                                                    uint8_t event);
    void QHciHandleBtOpenXpanOpen(qhci_dev_cb_t *rem_info, uint8_t event);

    void QHciSmExecute(qhci_dev_cb_t *rem_info, uint8_t event);
    qhci_dev_cb_t* GetQHciRemoteDeviceInfo(uint16_t handle);
    void SendCreateCisToSoc(qhci_dev_cb_t *rem_info);
    QHCI_CSM_STATE GetQHciState(qhci_dev_cb_t *rem_info);
    void QHciSendCisEstablishedEvt(qhci_dev_cb_t *rem_info, uint8_t status,
                                             uint8_t handle_num);
    void QHciLeConnCleanup(qhci_dev_cb_t *rem_info);
    bool IsQHciSupportLmpVersion (uint16_t subversion);
    bool IsQHciSupportVersion (uint8_t version);
    bool IsQHciSupportManuFacture (uint16_t man_facture_id);
    void QHciProcessRxPktEvt(qhci_msg_t *msg);
    void QHciParseUsecaseUpdateCmd(qhci_msg_t *msg);
    void QHciSetHostSupportedBit();
    void QHciStartCommandTimer();
    QHciTimerState QHciGetCmdTimerState();
    void QHciSetCmdTimerState(QHciTimerState timer_state);
    static void QHciCmdTimeOut(union sigval sig);
    void QHciStopCmdTimer();
    bool IsQHciXpanSupportedDevice(uint16_t handle);
    void QHciSendUsecaseUpdateCfm(uint8_t usecase);
    void QHciProcessQllReq(uint16_t handle);
    void QHciPrepareAndSendHciDisconnect(uint16_t handle);
    void QHciSendDisconnectCisToSoc(qhci_dev_cb_t *rem_info);
    uint16_t QHciGetMappingAclHandle(uint16_t cis_handle);
    void QHciDelayReportingEvt(qhci_msg_t *msg);
    void GetMainThreadState(void);
    bool QHciCmpBDAddrs(bdaddr_t bd_addr1, bdaddr_t bd_addr2);
    uint16_t QHciBDAddrToHandleMap(bdaddr_t bd_addr);
    void QHciSendCmdStatusForCis();
    void QHciProcessConnCmplEvt(qhci_msg_t *msg);
    void QHciSendDisconnectCmplt(uint16_t handle);
    void QHciSendDisconnectCmdStatus();
    void QHciUseCaseUpdateEvt(uint8_t usecase);
    void QHciSendVndrQllEvtMask();
    void QHciClearRemoteDeviceInfo(uint16_t handle);
    void QHciUnprepareAudioBearerRspfromXm(bdaddr_t bd_addr, bool status);
    char* ConvertEventToString(uint8_t eventId);

    std::atomic_bool main_thread_running;
    static std::shared_ptr<QHci> qhci_main_instance;
    //Logger *logger_;
    std::thread qhci_main_thread;
    QHciPacketizer qhci_packetizer;
    std::mutex qhci_wq_mtx;
    std::queue <qhci_msg_t *> qhci_workqueue;
    std::condition_variable qhci_wq_notifier;
    std::atomic_bool is_main_thread_busy{false};
    std::vector <qhci_dev_cb_t> qhci_xpan_dev_db;
    qhci_cig_params_t cig_params;
    bool qhci_wait_for_qll_event;
    bool qhci_set_host_bit;
    QHciCmdTimer qhci_cmd_timer_ = {QHCI_TIMER_NOT_CREATED, 0};
    std::map<uint16_t, bdaddr_t> xpan_active_devices_;
    std::map<uint16_t, bdaddr_t> handle_bdaddr_map_;
    bool qhci_wait_for_cmd_status_from_soc;
    bool is_xpan_supported;
    bool qhci_progress_cis_cmd;
    uint8_t qhci_remote_version_data[100];
    static std::mutex cmd_timer_mutex_;
    bool qhci_qll_req_sent;
    bool is_cis_handle_disc_pending;
    std::map<uint16_t, uint16_t> cis_acl_handle_map;
    std::vector <uint16_t> active_cis_handles;
    bool prep_bearer_active;
    bdaddr_t curr_bd_addr;
    bdaddr_t dbg_curr_bd_addr[2];
    bdaddr_t pending_bd_addr_cis;
    int dbg_cnt_bd_addr;
    qhci_dev_cb_t curr_active_xpan_dev;
    bool is_cis_conn_prog;
    bool encrypt_event_pending;
    bool dbg_mtp_mtp_prop;
    bool dbg_mtp_mora_prop;
    bool dbg_mora_nut_prop;
    UseCaseType usecase_type_to_xm;
    bool qhci_bearer_switch_pending;
    int pending_cis_est_evt;
};

} // namespace implementation
} // namespace xpan
#endif


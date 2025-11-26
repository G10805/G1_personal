/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#ifndef QHCI_PACKETIZER_H
#define QHCI_PACKETIZER_H

#include <stdint.h>
#include <hidl/HidlSupport.h>
#include "hci_transport.h"
#include "xpan_utils.h"
#include <thread>

using ::android::hardware::hidl_vec;

#define QHCI_COMMAND_PREAMBLE_SIZE 3 

//QBCE Vendor Specific Command Opcode
#define QHCI_VS_QBCE 0xFC51

//QLL SET HOST FEATURE SUBCOMMAND
#define QBCE_QHCI_QLE_SET_HOST_FEATURE 0x14

typedef struct {
  uint16_t qhci_event;
  uint16_t qhci_len;
  uint16_t qhci_offset;
  uint16_t qhci_layer_specific;
  uint8_t qhci_data[];
} QHCI_BT_HDR;


typedef enum {
  QHCI_MSG_BASE = 2000,
  QHCI_QLL_CMD_REQ,
  QHCI_QLL_EVT_RSP,
  QHCI_CIG_EVT_RCV,
  QHCI_CIS_CONN_CMD,
  QHCI_CIS_CONN_EVT,
  QHCI_LE_CONN_CMPL_EVT,
  QHCI_ACL_DISCONNECT,
  QHCI_XM_PREPARE_REQ,
  QHCI_XM_PREPARE_REQ_BT,
  QHCI_XM_PREPARE_RSP,
  QHCI_XM_UNPREPARE_REQ,
  QHCI_XM_UNPREPARE_RSP,
  QHCI_UPDATE_TRANSPORT,
  QHCI_TRANSPORT_ENABLE,
  //Move below event to other thread
  QHCI_PROCESS_RX_PKT_EVT,
  QHCI_USECASE_UPDATE_CFM,
  QHCI_USECASE_UPDATE_CMD,
  QHCI_PROCESS_TX_CMD_PKT,
  QHCI_DELAY_REPORT_EVT,
  QHCI_BEARER_SWITCH_IND,
}QHciEventId;

typedef struct {
  QHciEventId eventId;
  uint16_t handle;
} __attribute__((packed)) QHciQllCmd;

typedef struct {
  QHciEventId eventId;
  uint8_t len;
  uint8_t *data;
  bool from_stack;
} __attribute__((packed)) QHciQllEvt;

typedef struct {
  
}__attribute__((packed)) QHciCigEvt;

typedef struct {
  QHciEventId eventId;
  HciPacketType type;
  hidl_vec<uint8_t> *hidl_data;

  bool from_stack;
} __attribute__((packed)) QHciRxEvtPkt;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint16_t handle;
  uint8_t transport_type;
} __attribute__((packed)) QHciPrepareBearerReq;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint16_t handle;
  bool status;
} __attribute__((packed)) QHciPrepareBearerRsp;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint8_t transport_type;
} __attribute__((packed)) QHciUnPrepareBearerReq;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  bool status;
} __attribute__((packed)) QHciUnPrepareBearerRsp;

typedef struct {
  QHciEventId eventId;
  uint8_t usecase;
  bool status;
} __attribute__((packed)) QHciUsecaseUpdateCfm;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint16_t handle;
  bool status;
} __attribute__((packed)) QHciTransportEnabled;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint16_t handle;
  uint8_t transport_type;
} __attribute__((packed)) QHciUpdateTransport;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint8_t ind_status;
} __attribute__((packed)) QHciBearerSwitchInd;

typedef struct {
  QHciEventId eventId;
  uint8_t *data;
} __attribute__((packed)) QHciTxCmdPkt;

typedef struct {
  QHciEventId eventId;
  uint8_t opcode;
  uint8_t length;
  UseCaseType context_type;
} __attribute__((packed)) QHciTxUsecaseRcvd;

typedef struct {
  QHciEventId eventId;
  uint32_t delay_report;
} __attribute__((packed)) QHciDelayReport;

typedef struct {
  QHciEventId eventId;
  bdaddr_t bd_addr;
  uint16_t handle;
} __attribute__((packed)) QHciConnCmpl;


typedef union {
  QHciEventId eventId;
  QHciQllCmd QllCmd;
  QHciConnCmpl ConnCmpl;
  QHciQllEvt QllEvt;
  QHciRxEvtPkt RxEvtPkt;
  QHciTxCmdPkt TxCmdPkt;
  QHciTxUsecaseRcvd TxUsecaseRcvd;
  QHciPrepareBearerReq PreBearerReq;
  QHciPrepareBearerRsp PreBearerRsp;
  QHciUnPrepareBearerReq UnPreBearerReq;
  QHciUnPrepareBearerRsp UnPreBearerRsp;
  QHciUsecaseUpdateCfm UsecaseUpdateCfm;
  QHciTransportEnabled TransportEnabled;
  QHciUpdateTransport UpdateTransport;
  QHciDelayReport DelayReport;
  QHciBearerSwitchInd BearerSwitchInd;
} qhci_msg_t;

namespace xpan {
namespace implementation {

class QHciPacketizer
{
  public:
    QHciPacketizer();
    ~QHciPacketizer();
    void ProcessMessage(qhci_msg_t *msg);

  private:
    QHCI_BT_HDR* QHciMakeCommand(uint16_t opcode, size_t parameter_size,
                                      uint8_t** stream_out);
    QHCI_BT_HDR* QHciMakeQbceSetHostFeature(uint8_t bit_position,
                                            uint8_t bit_value);
    QHCI_BT_HDR* QHciMakePacket(size_t data_size);
};

} // namespace implementation
} // namespace xpan
#endif



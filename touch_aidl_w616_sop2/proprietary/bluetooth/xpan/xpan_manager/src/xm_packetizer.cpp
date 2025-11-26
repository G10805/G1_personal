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
#include "xm_packetizer.h"
#include <android-base/logging.h>

#include <iostream>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.xpan@1.0-xmpacketizer"

namespace xpan {
namespace implementation {

XMPacketizer::XMPacketizer()
{
}

XMPacketizer::~XMPacketizer()
{
}

void XMPacketizer::decode_and_dispatch_cp_pkt(uint32_t opcode,
		                              uint32_t len, uint8_t *buf)
{
  int i;
  uint8_t *cmd;
  switch (opcode) {
    case BTM_CP_CTRL_MASTER_CONFIG_RSP: {

      ALOGI("%s: Received master configuration response", __func__);
      CHECK(BTM_CTRL_MASTER_CONFIG_RSP_PKT_LEN == (len + BTM_PKT_HEADER_LEN));

      cmd = (uint8_t *)new uint8_t[len + BTM_PKT_HEADER_LEN];
      UINT32_TO_STREAM(cmd, BTM_KP_CTRL_MASTER_CONFIG_RSP);
      UINT32_TO_STREAM((cmd + BTM_OPCODE_LEN), len);

      for (i = BTM_PKT_HEADER_LEN; i <(len + BTM_PKT_HEADER_LEN); i++)
        cmd[i]= (uint8_t)buf[i- BTM_PKT_HEADER_LEN];

      log_pkt(BTM_CP_CTRL_MASTER_CONFIG_RSP, cmd);
      KernelProxyTransport::Get()->WritetoKpTransport(cmd,
		                   BTM_CTRL_MASTER_CONFIG_RSP_PKT_LEN);
      delete []cmd;
      break;      
    } case BTM_CP_CTRL_MASTER_SHUTDOWN_RSP: {
      ALOGI("%s: Received master shutdown response", __func__);
      CHECK(BTM_CTRL_MASTER_SHUTDOWN_RSP_PKT_LEN == (len + BTM_PKT_HEADER_LEN));
      cmd = (uint8_t *)new uint8_t[len + BTM_PKT_HEADER_LEN];
      UINT32_TO_STREAM(cmd, BTM_KP_CTRL_MASTER_SHUTDOWN_RSP);
      UINT32_TO_STREAM(cmd + BTM_OPCODE_LEN, len);

      for (i = BTM_PKT_HEADER_LEN; i <(len +BTM_PKT_HEADER_LEN); i++)
        cmd[i]= (uint8_t)buf[i - BTM_PKT_HEADER_LEN];
      log_pkt(BTM_CP_CTRL_MASTER_SHUTDOWN_RSP, cmd);
      KernelProxyTransport::Get()->WritetoKpTransport(cmd,
		                   BTM_CTRL_MASTER_SHUTDOWN_RSP_PKT_LEN);
      delete []cmd;
      break;      
  } case BTM_CP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_RSP: {
    ALOGI("%s: Received prepare audio bearer switch rsp from cp", __func__);
    CHECK(BTM_CTRL_PREPARE_AUDIO_BEARER_SWITCH_RSP_LEN == (len + BTM_PKT_HEADER_LEN));
    /* Post message to main thread */
    xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
    msg->CpAudioBearerRsp.eventId = CP_XM_PREPARE_AUDIO_BEARER_RSP;
    msg->CpAudioBearerRsp.current_transport = buf[0];
    msg->CpAudioBearerRsp.status = buf[1];
    XpanManager::Get()->PostMessage(msg);
    break;
  } case BTM_CP_CTRL_DELAY_REPORTING_IND : {
    uint32_t delay_reporting;
    CHECK(BTM_CP_CTRL_DELAY_REPORTING_LEN == (len + BTM_PKT_HEADER_LEN));
    STREAM_TO_UINT16(delay_reporting, buf);
    ALOGI("%s: Received delay reporting from cp", __func__);
    /* Post message to main thread */
    xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
    msg->DelayReporting.eventId = CP_XM_DELAY_REPORTING;
    msg->DelayReporting.delay_reporting = delay_reporting;
    XpanManager::Get()->PostMessage(msg);
    break;
  } case BTM_CP_CTRL_TRANSPORT_UPDATE_IND : {
    uint8_t transport;
    CHECK(BTM_CP_CTRL_TRANSPORT_UPDATE_LEN == (len + BTM_PKT_HEADER_LEN));
    STREAM_TO_UINT8(transport, buf);
    ALOGI("%s: Received Transport update from cp with %s", __func__,
          TransportTypeToString((TransportType)transport));
    /* Post message to main thread */
    xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
    msg->TransportUpdate.eventId = CP_XM_TRANSPORT_UPDATE;
    msg->TransportUpdate.transport  = (TransportType)transport;
    XpanManager::Get()->PostMessage(msg);
    break;
  } default:
    ALOGE("%s: Opcode %0008x not matching", __func__, opcode);
  }
}

void XMPacketizer::OnDataReady(int fd)
{
  std::stringstream ss;
  uint8_t buffer[MAX_BUF_SIZE] = {0};
  uint32_t len = 0;
  uint32_t opcode;
  int i;
  /* Both KP and Glink drivers are implemented as packet
   * read unlike a byte read in UART. At a time these drivers
   * will fetch the entire packet and copy it to user buffers
   * and then deletes it entires.
   */
  size_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer,
			                 BTM_MAX_PKT_SIZE));
  if (bytes_read > BTM_PKT_HEADER_LEN) {
    STREAM_TO_OPCODE(opcode, buffer);
    STREAM_TO_LENGTH(len, buffer + BTM_OPCODE_LEN);
    ALOGI("%s: received opcode :%04x with len:%d", __func__, opcode, len);
    for (i = BTM_PKT_HEADER_LEN; i < (len  + BTM_PKT_HEADER_LEN); ++i) {
       ss <<  std::uppercase << std::hex << (int)buffer[i] << " ";
    }
    if (ss.str().length() > 0) {
      std::string params = "packet received:\n";
      params += ss.str();
      ALOGE("%s: %s", __func__, params.c_str());
    }
    if (opcode >= BTM_KP_CTRL_OFFSET) 
      decode_and_dispatch_kp_pkt(opcode, len, buffer + BTM_PKT_HEADER_LEN);
    else if (opcode >= BTM_CP_CTRL_OFFSET)
      decode_and_dispatch_cp_pkt(opcode, len, buffer + BTM_PKT_HEADER_LEN);
    else
      ALOGE("%s: opcode :%04x is not handled", __func__, opcode);
  } else {
     if (bytes_read < 0) {
       ALOGE("%s: Error occur while reading packet", __func__);
       return;
     }
     ALOGE("%s: complete packet is not sent, discarding it..", __func__);
     for (i = 0; i < bytes_read ; ++i) {
       ss <<  std::uppercase << std::hex << (int)buffer[i] << " ";
     }
     if (ss.str().length() > 0) {
      std::string params = "discarded packet payload rx:\n";
      params += ss.str();
      ALOGE("%s: %s", __func__,params.c_str());
    }
  }
}

void XMPacketizer::decode_and_dispatch_kp_pkt(uint32_t opcode,
		                              uint32_t len, uint8_t *buf)
{
  int i;
  uint8_t *cmd;
  switch (opcode) {
    case BTM_KP_CTRL_MASTER_CONFIG_REQ: {

      ALOGI("%s: Received master configuration request", __func__);
      CHECK(BTM_CTRL_MASTER_CONFIG_REQ_PKT_LEN ==
	    (len + BTM_PKT_HEADER_LEN));
      cmd = (uint8_t *)new uint8_t[len + BTM_PKT_HEADER_LEN];
      UINT32_TO_STREAM(cmd, BTM_CP_CTRL_MASTER_CONFIG_REQ);
      UINT32_TO_STREAM(cmd + BTM_OPCODE_LEN, len);

      for (i = BTM_PKT_HEADER_LEN; i <(len +BTM_PKT_HEADER_LEN); i++)
        cmd[i]= (uint8_t)buf[i - BTM_PKT_HEADER_LEN];

      log_pkt(BTM_KP_CTRL_MASTER_CONFIG_REQ, cmd);
      GlinkTransport::Get()->WritetoGlinkCC(cmd,
		            BTM_CTRL_MASTER_CONFIG_REQ_PKT_LEN);
      delete []cmd;
      break;      
    } case BTM_KP_CTRL_MASTER_SHUTDOWN_REQ: {
      ALOGI("%s: Received master shutdown request", __func__);
      CHECK(BTM_CTRL_MASTER_SHUTDOWN_REQ_PKT_LEN == (len + BTM_PKT_HEADER_LEN));

      cmd = (uint8_t *)new uint8_t[len + BTM_PKT_HEADER_LEN];
      UINT32_TO_STREAM(cmd, BTM_CP_CTRL_MASTER_SHUTDOWN_REQ);
      UINT32_TO_STREAM(cmd + BTM_OPCODE_LEN, len);

      for (i = BTM_PKT_HEADER_LEN; i < (len +BTM_PKT_HEADER_LEN); i++)
        cmd[i]= (uint8_t)buf[i - BTM_PKT_HEADER_LEN];

      log_pkt(BTM_KP_CTRL_MASTER_SHUTDOWN_REQ, cmd);
      GlinkTransport::Get()->WritetoGlinkCC(cmd,
		             BTM_CTRL_MASTER_SHUTDOWN_REQ_PKT_LEN);
      delete []cmd;
      break;      
  } case BTM_KP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_RSP: {
    ALOGI("%s: Received prepare audio bearer switch rsp from kp", __func__);
    CHECK(BTM_CTRL_PREPARE_AUDIO_BEARER_SWITCH_RSP_LEN == (len + BTM_PKT_HEADER_LEN));
    /* Post message to main thread */
    xm_ipc_msg_t *msg = (xm_ipc_msg_t *) malloc(XM_IPC_MSG_SIZE);
    msg->KpAudioBearerRsp.eventId = KP_XM_PREPARE_AUDIO_BEARER_RSP;
    msg->KpAudioBearerRsp.current_transport = buf[BTM_PKT_HEADER_LEN];
    msg->KpAudioBearerRsp.status = buf[BTM_PKT_HEADER_LEN + 1];
    XpanManager::Get()->PostMessage(msg);
    break;
  } default:
    ALOGE("%s: Opcode %0008x not matching", __func__, opcode);
  }
}

void XMPacketizer::log_pkt(uint32_t rx_opcode, uint8_t *cmd)
{
  uint32_t opcode;
  uint32_t len;

  STREAM_TO_UINT32(opcode, cmd);
  STREAM_TO_UINT32(len, cmd);
  switch(rx_opcode) {
    case BTM_KP_CTRL_MASTER_CONFIG_REQ: {
      uint8_t stream_id, bit_width, num_channels, channel_num, codec_id;
      uint32_t device_id, sample_rate;
      STREAM_TO_UINT8(stream_id, cmd);
      STREAM_TO_UINT32(device_id, cmd);
      STREAM_TO_UINT32(sample_rate, cmd);
      STREAM_TO_UINT8(bit_width, cmd);
      STREAM_TO_UINT8(num_channels, cmd);
      STREAM_TO_UINT8(channel_num, cmd);
      STREAM_TO_UINT8(codec_id, cmd);
      ALOGD("KP-->CP opcode:%08x len:%08x stream_id:%02x device_id:%08x", opcode,
	     len, stream_id, device_id);

      ALOGD("KP-->CP sample_rate:%08x num_channels:%02x channel_num:%02x codec_id:%02x",
	     sample_rate, num_channels, channel_num, codec_id);
      break;
    }
    case BTM_KP_CTRL_MASTER_SHUTDOWN_REQ: {
      uint8_t stream_id;
      STREAM_TO_UINT8(stream_id, cmd);
      ALOGD("KP-->CP opcode:%04x len:%02x stream_id:%02x", opcode, len, stream_id);
      break;
    }
    case BTM_CP_CTRL_MASTER_CONFIG_RSP: {
      uint8_t stream_id, status;
      STREAM_TO_UINT8(stream_id, cmd);
      STREAM_TO_UINT8(status, cmd);
      ALOGD("CP-->KP opcode:%04x len:%02x stream_id:%02x status:%02x", opcode,
	     len, stream_id, status);
      break;
    }
    case BTM_CP_CTRL_MASTER_SHUTDOWN_RSP: {
      uint8_t stream_id, status;
      STREAM_TO_UINT8(stream_id, cmd);
      STREAM_TO_UINT8(status, cmd);
      ALOGD("CP-->KP opcode:%04x len:%02x stream_id:%02x status:%02x", opcode,
            len, stream_id, status);
      break;
    }
    default: {
      ALOGE("%s: this opcode is not handled:%04x", __func__, rx_opcode);
      break;
    }
  }
}

void XMPacketizer::decode_and_dispatch(uint32_t opcode, uint32_t len,
		                       uint8_t *buf) 
{
  int i;
  uint8_t *cmd;
  std::stringstream ss;
  uint8_t end_point;

  cmd = (uint8_t *)new uint8_t[len + BTM_PKT_HEADER_LEN];

  UINT32_TO_STREAM(cmd, opcode);
  UINT32_TO_STREAM(cmd + BTM_OPCODE_LEN, len);

  if (opcode >= BTM_CP_CTRL_OFFSET && opcode <= BTM_CP_CTRL_MAX_OFFSET) {
    end_point = 1;
    ALOGI("%s:%s", __func__, OpcodeToString(opcode));
  } else if (opcode >= BTM_KP_CTRL_OFFSET && opcode <= BTM_KP_CTRL_MAX_OFFSET) {
    end_point = 2;
    ALOGI("%s:%s", __func__, OpcodeToString(opcode));
  } else {
    ALOGE("%s: incorrect message :%s dropping here", __func__,
	  OpcodeToString(opcode));
    return;
  }
 
  for (i = BTM_PKT_HEADER_LEN; i < (len + BTM_PKT_HEADER_LEN); i++)
    cmd[i]= (uint8_t)buf[i - BTM_PKT_HEADER_LEN];

  for (i = 0; i < len + BTM_PKT_HEADER_LEN; i++)
    ss <<  std::uppercase << std::hex << (int)cmd[i] << " ";

  if (ss.str().length() > 0) {
    std::string params = "sending packet:\n";
    params += ss.str();
    ALOGE("%s: %s", __func__, params.c_str());
  }

  if(end_point == 1)
    GlinkTransport::Get()->WritetoGlinkCC(cmd, len+ BTM_PKT_HEADER_LEN);
  else
   KernelProxyTransport::Get()->WritetoKpTransport(cmd, len + BTM_PKT_HEADER_LEN);

  delete []cmd;
}

void XMPacketizer::KpBearerSwitchInd(RspStatus status)
{
  uint8_t cmd; 	
  cmd = (uint8_t)(status & 0x000000FF);
  decode_and_dispatch(BTM_KP_CTRL_BEARER_SWITCH_IND, 1, &cmd); 
}

void XMPacketizer::CpBearerSwitchInd(RspStatus status)
{
  uint8_t cmd; 	
  cmd = (uint8_t)(status & 0x000000FF);
  decode_and_dispatch(BTM_CP_CTRL_BEARER_SWITCH_IND, 1, &cmd); 
}

void XMPacketizer::ProcessMessage(XmIpcEventId eventId, xm_ipc_msg_t *msg)
{
  switch (eventId) {
    case DH_XM_COP_VER_IND: {
      DhXmCopVerInd CoPInd = msg->CoPInd;
      decode_and_dispatch(BTM_CP_CTRL_COP_VER_IND, CoPInd.len, CoPInd.data);
      break;
    } case QHCI_XM_USECASE_UPDATE: {
      QhciXmUseCase UseCase = msg->UseCase;
      uint8_t cmd;
      cmd = (uint8_t)(UseCase.usecase & 0x000000FF);
      decode_and_dispatch(BTM_CP_CTRL_UPDATE_AUDIO_MODE_IND, 1, &cmd);
      break;
    } case QHCI_XM_PREPARE_AUDIO_BEARER_REQ: {
      QhciXmPrepareAudioBearerReq AudioBearerReq = msg->AudioBearerReq;
      uint8_t cmd;
      cmd = (uint8_t)(AudioBearerReq.type & 0x000000FF);
      decode_and_dispatch(BTM_CP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_REQ, 1, &cmd); 
      decode_and_dispatch(BTM_KP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_REQ, 1, &cmd); 
      break;
    } case XP_XM_HOST_PARAMETERS: {
      uint8_t cmd[8];
      memcpy(cmd, &msg->HostParams.macaddr, sizeof(macaddr_t));
      cmd[6] = (uint8_t)(msg->HostParams.Ethertype & 0xFF);
      cmd[7] = (uint8_t)((msg->HostParams.Ethertype >> 8) & 0x00FF);
      decode_and_dispatch(BTM_CP_CTRL_HOST_PARAMETERS_IND,
                          (sizeof(macaddr_t)+ sizeof(uint16_t)), cmd); 
      break;
    } case XP_XM_TWT_SESSION_EST: {
      int i = 0;
      ALOGE("%s: XP_XM_TWT_SESSION_EST addr %p", __func__, msg->TwtParams.params);
      uint8_t num_devices = msg->TwtParams.num_devices;
      uint8_t len = BTM_XP_TWT_BASE_LEN + (num_devices * BTM_XP_TWT_PAYLOAD_LEN);
      uint8_t *cmd = (uint8_t*)new uint8_t[len];
      uint8_t count = 0;
      XPANTwtSessionParams *params =  msg->TwtParams.params;
      cmd[i++]= num_devices;
      for (; count < num_devices ; count++) {
        /* copy mac addr */
        int j = 0;
        for (; j < 6; j++)
          cmd[i+j] = params[count].mac_addr.b[j];
        i += j;

        UINT32_TO_STREAM(cmd + i, params[count].interval);
        i += sizeof(uint32_t);

        UINT32_TO_STREAM(cmd + i, params[count].peroid);
        i += sizeof(uint32_t);

        UINT32_TO_STREAM(cmd + i, params[count].location);
        i += sizeof(uint32_t);
      }
      ALOGE("%s: len %d and i %d", __func__, len, i);
      decode_and_dispatch(BTM_CP_CTRL_TWT_EST_IND, len , cmd); 
      delete []cmd;
      ALOGE("%s: freeing up the params", __func__);
      free(params);
      ALOGE("%s: freed ", __func__);
      break;
    } case XP_XM_BEARER_PREFERENCE_IND: {
      uint8_t cmd;
      cmd = (uint8_t)(msg->BearerPreference.transport & 0x000000FF);
      decode_and_dispatch(BTM_CP_CTRL_BEARER_PREFERENCE_IND, 1, &cmd);
    } case QHCI_XM_UNPREPARE_AUDIO_BEARER_REQ: {
      QhciXmUnPrepareAudioBearerReq UnPrepareAudioBearerReq = msg->UnPrepareAudioBearerReq;
      uint8_t cmd;
      cmd = (uint8_t)(UnPrepareAudioBearerReq.type & 0x000000FF);
      decode_and_dispatch(BTM_CP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_REQ, 1, &cmd); 
      decode_and_dispatch(BTM_KP_CTRL_PREPARE_AUDIO_BEARER_SWITCH_REQ, 1, &cmd); 
      break;
    } case XM_CP_LOG_LVL: {
      XmCpLogLvl Loglvl = msg->Loglvl;
      decode_and_dispatch(BTM_CP_CTRL_DATA_LOG_MASK_IND, Loglvl.len, &Loglvl.data);
    } default: {
      ALOGI("%s: this :%04x ipc message is not handled", __func__, eventId);
    }
  }
}

} // namespace implementation
} // namespace xpan

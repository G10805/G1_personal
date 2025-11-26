/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;


typedef struct {
  uint8_t b[6];
} __attribute__((packed)) macaddr_t;

typedef enum {
  BT = 1,
  XPAN,
  NONE,
} TransportType;

/* Use Same usecase as WiFi vendor lib */
typedef enum {
  USECASE_XPAN_NONE = 0,
  USECASE_XPAN_LOSSLESS,
  USECASE_XPAN_GAMING,
  USECASE_XPAN_VBC,
} UseCaseType;

/* Use Same usecase as WiFi vendor lib */
typedef enum {
  QHCI_To_XM = 1,
  XM_To_QHCI,
} ApiDirection;

/* assigned 20 error reason for every client */
#define XM_KP_ERROR_REASON_OFFSET      0x14

typedef enum {
  XM_SUCCESS = 0,
  XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT,
  XM_FAILED_WRONG_TRANSPORT_TYPE_REQUESTED,
  XM_FAILED_DUE_TO_AUDIO_BEARER_TIMEOUT,
  XM_FAILED_DUE_TO_UNPREPARE_AUDIO_BEARER_TIMEOUT,
  XM_FAILED_TO_ESTABLISH_ACS,
  XM_FAILED_REQUSTED_WRONG_TRANSPORT_TYPE,
  XM_NOTALLOWING_UNPREPARE_AS_STATE_IS_IDLE,
  XM_XP_PERFORMED_ON_WRONG_BD_ADDRESS,
  XM_FAILED,
  XM_KP_MSG_SUCCESS = XM_KP_ERROR_REASON_OFFSET,
  /* Error while processing the message */
  XM_KP_MSG_FAILED,
  /* Wrong transport type selected by XPAN manager */
  XM_KP_MSG_WRONG_TRANSPORT_TYPE,
  /* Timeout triggered to receive bearer switch indications*/
  XM_KP_MSG_INTERNAL_TIMEOUT,
  /* Failed to Configure HWEP */
  XM_KP_MSG_FAILED_TO_CONFIGURE_HWEP,
  /* Failed to shutdown HWEP */
  XM_KP_MSG_FAILED_TO_SHUTDOWN_HWEP,
  XM_KP_MSG_ERR_WHILE_SHUTING_DOWN_HWEP,
  XM_KP_MSG_INVALID,
} RspStatus;

static inline char * StatusToString(RspStatus status)
{
  if (status == XM_SUCCESS)
    return "XM_SUCCESS";
  else if (status == XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT)
    return "XM_FAILED_STATE_ALREADY_IN_REQUESTED_TRANSPORT";
  else if (status == XM_FAILED_DUE_TO_AUDIO_BEARER_TIMEOUT)
    return "XM_FAILED_DUE_TO_AUDIO_BEARER_TIMEOUT";
  else if (status == XM_FAILED_DUE_TO_UNPREPARE_AUDIO_BEARER_TIMEOUT)
    return "XM_FAILED_DUE_TO_UNPREPARE_AUDIO_BEARER_TIMEOUT";
  else if (status == XM_FAILED_TO_ESTABLISH_ACS)
    return "XM_FAILED_TO_ESTABLISH_ACS";
  else if (status == XM_FAILED_REQUSTED_WRONG_TRANSPORT_TYPE)
    return "XM_FAILED_REQUSTED_WRONG_TRANSPORT_TYPE";
  else if (status == XM_FAILED)
    return "XM_FAILED";
  else if (status == XM_KP_MSG_SUCCESS)
    return "XM_KP_MSG_SUCCESS";
  else if (status == XM_KP_MSG_FAILED)
    return "XM_KP_MSG_FAILED";
  else if (status == XM_KP_MSG_WRONG_TRANSPORT_TYPE)
    return "XM_KP_MSG_WRONG_TRANSPORT_TYPE";
  else if (status == XM_KP_MSG_INTERNAL_TIMEOUT)
    return "XM_KP_MSG_INTERNAL_TIMEOUT";
  else if (status == XM_KP_MSG_FAILED_TO_CONFIGURE_HWEP)
    return "XM_KP_MSG_FAILED_TO_CONFIGURE_HWEP";
  else if (status == XM_KP_MSG_FAILED_TO_SHUTDOWN_HWEP)
    return "XM_KP_MSG_FAILED_TO_SHUTDOWN_HWEP";
  else if (status == XM_KP_MSG_ERR_WHILE_SHUTING_DOWN_HWEP)
    return "XM_KP_MSG_ERR_WHILE_SHUTING_DOWN_HWEP";
  else
   return "INVALID Status";
}

#define ConvertRawBdaddress(bdaddr) \
({ \
  char buf[18]; \
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", bdaddr.b[5], \
	  bdaddr.b[4], bdaddr.b[3], bdaddr.b[2], bdaddr.b[1], \
	  bdaddr.b[0]);  \
  buf[17] = '\0'; \
  buf; \
})

static inline char * ConvertRawMacaddress(macaddr_t addr) \
{
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", addr.b[0], \
	  addr.b[1], addr.b[2], addr.b[3], addr.b[4], addr.b[5]);
  buf[17] = '\0';
  return buf;
}

static inline char * TransportTypeToString(TransportType type)
{
  if (type == BT)
    return "BT";
  else if(type == XPAN)
    return "XPAN";
  else if(type == NONE)
    return "NONE";
  else
    return "INVALID Transport";
}

static inline char* UseCaseToString(UseCaseType usecase)
{
  if (usecase == USECASE_XPAN_LOSSLESS)
    return "USECASE_XPAN_LOSSLESS";
  else if(usecase == USECASE_XPAN_VBC)
    return "USECASE_XPAN_VBC";
  else if(usecase == USECASE_XPAN_GAMING)
    return "USECASE_XPAN_GAMING";
  else
    return "INVALID USECASE";
}

#define ACTIVE_BDADDR {0x00, 0x00, 0x00, 0x00, 0x00,0x00}

#define SET_BIT(pp ,bit)	((pp) |=  (1<<(bit)))
#define CLEAR_BIT(pp, bit)	((pp) &= ~(1<<(bit)))
#define IS_BIT_SET(pp, bit)	((pp) &   (1<<(bit)))

/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t WifiMsg;

#define QTI_WIFI_REQ_BASE                                                           ((WifiMsg)(0x0000))

#define QTI_WIFI_LIST_AVAILABLE_INTERFACES_REQ                                      (WifiMsg)(QTI_WIFI_REQ_BASE + 0x0000)
#define QTI_WIFI_REGISTER_QTI_WIFI_CALLBACK_REQ                                     (WifiMsg)(QTI_WIFI_REQ_BASE + 0x0001)
#define QTI_WIFI_DO_QTI_WIFI_CMD_REQ                                                (WifiMsg)(QTI_WIFI_REQ_BASE + 0x0002)

#define QTI_WIFI_REQ_COUNT                                                          (QTI_WIFI_DO_QTI_WIFI_CMD_REQ - QTI_WIFI_LIST_AVAILABLE_INTERFACES_REQ + 1)
#define IS_QTI_WIFI_REQ(t)                                                          (((t) >= QTI_WIFI_LIST_AVAILABLE_INTERFACES_REQ) && ((t) <= QTI_WIFI_DO_QTI_WIFI_CMD_REQ))

#define QTI_WIFI_IND_BASE                                                           ((WifiMsg)(0x8000))

#define QTI_WIFI_ON_CTRL_EVENT_IND                                                  (WifiMsg)(QTI_WIFI_IND_BASE + 0x0000)

#define QTI_WIFI_IND_COUNT                                                          (QTI_WIFI_ON_CTRL_EVENT_IND - QTI_WIFI_ON_CTRL_EVENT_IND + 1)
#define IS_QTI_WIFI_IND(t)                                                          (((t) >= QTI_WIFI_ON_CTRL_EVENT_IND) && ((t) <= QTI_WIFI_ON_CTRL_EVENT_IND))
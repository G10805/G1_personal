#ifndef AMS_OSAL_SSR_H
#define AMS_OSAL_SSR_H
/**
*
* \file ams_osal_ssr.h
*
* \brief
*     This file contains mutex apis. Recursive mutexes are always used
*     for thread-safe programming.
* \copyright
*  Copyright (c) 2019-2021, 2023 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

enum ssr_ss_id {
    SS_ID_LPASS = 0x1,
    SS_ID_MODEM = 0x2,
    SS_ID_INVALID = 0xff
};

enum ssr_events {
    SSR_EVENT_RESTART_START = 0x2,
    SSR_EVENT_RESTART_COMPLETE = 0x4,
};

int32_t ams_osal_ssr_init(char* client_name, int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx);
int32_t ams_osal_ssr_deinit(void);

#endif
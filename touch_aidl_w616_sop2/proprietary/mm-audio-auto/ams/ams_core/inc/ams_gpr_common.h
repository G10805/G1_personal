/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ams_osal_mutex.h"
#include "ar_osal_signal.h"
#include "gpr_packet.h"

#define AMS_RM_SLEEP_MS 10
#define AMS_RM_RETRY_NUM 1
#define AMS_GPR_SRC_PORT 0x4001
#define AMS_MODULE_INSTANCE_ID 0x00000008

#define AMS_GPR_COMMON_SPF_TIMEOUT_MS 2000
#define AMS_GPR_COMMON_EVENT_MASK_SPF_RSP 0x01
#define AMS_GPR_COMMON_EVENT_MASK_SSR 0x10

#define AMS_ALIGN_8BYTE(x) (((x) + 7) & (~7))

#define GPR_IBASIC_RSP_RESULT (0x02001005)

struct spf_cmd_basic_rsp
{
    uint32_t opcode;
    int32_t status;
};

struct ams_gpr_common_ctx_s
{
    ams_osal_mutex_t mutex;
    struct ams_gpr_common_signal_obj *psig_obj;
    uint32_t dyn_src_port; // RFU:store port which register OK at GPR
    void *rsp_buff;        // not used
    uint32_t rsp_buff_sz;  // not used
};

extern struct ams_gpr_common_ctx_s ams_gpr_common_ctx;

/** signal object structure */
struct ams_gpr_common_signal_obj
{
    /** signal object */
    ar_osal_signal_t sig;
    /**
     * mutex used to synchronize access to below data
     */
    ams_osal_mutex_t lock;
    /**
     * flags that indicates which events caused the signal to get set
     * 0x01 - response was received from spf
     * 0x02-0x08 - RFU
     * 0x10 - SSR event
     */
    uint32_t flags;
    /** signal status */
    int32_t status;
    /** gpr packet pointer */
    void *gpr_packet;
};

struct ams_gpr_common_spf_cmd_basic_rsp
{
    uint32_t opcode;
    int32_t status;
};

// gpr common wrapper functions for MMOSAL signal handling API
int32_t ams_gpr_common_ctx_init(void);
int32_t ams_gpr_common_ctx_deinit(void);
// allocate gpr paket
int32_t ams_gpr_common_allocate_gpr_packet(uint32_t opcode, uint32_t src_domain, uint32_t dst_domain, uint32_t src_port,
                                           uint32_t dst_port, uint32_t payload_size, uint32_t token,
                                           struct gpr_packet_t **alloc_packet);
// send command and receive response synchronously
int32_t ams_gpr_common_send_spf_cmd_wait_for_rsp(struct gpr_packet_t *packet,
                                                 struct ams_gpr_common_signal_obj *sig_obj, void **rsp_pkt);
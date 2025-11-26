/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdint.h>
#include "ams_core_ioctl.h"
#include "ams_rsc_mgr.h"

typedef enum
{
    DSP_AMS_GRAPH_INIT = 0,
    DSP_AMS_GRAPH_OPENED = 1,
    DSP_AMS_GRAPH_STARTED
} ams_graph_state_t;

int32_t ams_rsc_mgr_gpr_ctx_init(void);

int32_t ams_rsc_mgr_gpr_ctx_deinit(void);

int32_t ams_rsc_mgr_resource_config(void);

int32_t ams_rsc_mgr_resource_free(void);

int32_t ams_rsc_mgr_send_tunnel_info(ams_core_tunnel_t *info_p, uint32_t ams_opcode);

int32_t ams_rsc_mgr_clk_enable(const ams_core_endpoint_t *endpoint);

int32_t ams_rsc_mgr_clk_disable(const ams_core_endpoint_t *endpoint);

int32_t ams_rsc_mgr_free_shmem(ams_core_shmem_free_t *shmem_free);

int32_t ams_rsc_mgr_alloc_shmem(ams_core_shmem_alloc_t *smem_alloc);

int32_t ams_rsc_mgr_shmem_init(void);

int32_t ams_rsc_mgr_shmem_deinit(void);

int32_t ams_rsc_mgr_get_lpass_resources(void);

int32_t ams_rsc_mgr_release_lpass_resources(void);

int32_t ams_rsc_mgr_share_resources(void);

int32_t ams_rsc_mgr_unshare_resources(void);

int32_t ams_rsc_mgr_mdsp_ssr_cleanup(void);

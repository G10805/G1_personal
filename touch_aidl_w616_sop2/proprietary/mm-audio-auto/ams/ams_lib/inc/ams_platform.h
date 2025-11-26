#ifndef AMS_PLATFORM_H
#define AMS_PLATFORM_H
/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <stdint.h>

#include "ams.h"
#include "ams_impl.h"
#include "ams_core_ioctl.h"
#include "dsp_audio_micro_service.h"

/* APR Basic Response Message */
#define APR_BASIC_RSP_RESULT 0x000110E8
#define APR_RSP_ACCEPTED 0x000100BE


#define GPR_IBASIC_RSP_RESULT (0x02001005)
struct spf_cmd_basic_rsp
{
    uint32_t opcode;
    int32_t status;
};

#define PMEM_ALIGNMENT_4K 12
#define PMEM_PAGESIZE (1U << PMEM_ALIGNMENT_4K)

#define AMS_FW_VERSION_MIN 0x01

#define AMS_LIB_APR_PKT_SIZE 4096

#define AMS_SMEM_BUF_SIZE (524288)

typedef ams_core_tunnel_t ams_tunnel_param_t;

typedef ams_core_endpoint_t ams_endpoint_cfg_t;

typedef ams_core_clk_attr_t ams_graph_clk_attr_cfg_t;

typedef ams_core_cached_graph_info_t ams_cached_graph_info_t;

typedef ams_core_cached_graph_descr_t ams_cached_graph_descr_t;

typedef ams_core_cached_graph_num_t ams_cached_graph_num_t;

typedef ams_core_shmem_alloc_t ams_shmem_alloc_t;

typedef ams_core_shmem_free_t ams_shmem_free_t;

#define DSP_AMS_CMD_MEM_MAP_NUM_REGIONS (1)

#define DSP_AMS_MAX_NUM_CH (64)

#define DSP_AMS_SAMPLE_BITWIDTH_MAX (32)

ams_status_t ams_platform_open(struct platform_drv *pdrv);

ams_status_t ams_platform_close(struct platform_drv *pdrv);

ams_status_t ams_platform_get_version(struct platform_drv *pdrv, uint32_t *version);

ams_status_t ams_platform_drv_alloc_smem(struct platform_drv *pdrv);

ams_status_t ams_platform_drv_free_smem(struct platform_drv *pdrv);

ams_status_t ams_platform_map_smem(struct platform_drv *pdrv);

ams_status_t ams_platform_unmap_smem(struct platform_drv *pdrv);

ams_status_t ams_platform_drv_set_smem_size(struct platform_drv *pdrv, uint32_t size);

ams_status_t ams_platform_open_graph(struct platform_drv *pdrv, struct ams_graph *gr);

ams_status_t ams_platform_close_graph(struct platform_drv *pdrv, struct ams_graph *gr);

ams_status_t ams_platform_start_graph(struct platform_drv *pdrv, struct ams_graph *gr);

ams_status_t ams_platform_stop_graph(struct platform_drv *pdrv, struct ams_graph *gr);

ams_status_t ams_platform_enable_endpoint(struct platform_drv *pdrv, ams_endpoint_t *e, uint32_t sr, uint32_t enable);

ams_status_t ams_platform_set_param(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t module_id,
    uint32_t param_id,
    uint32_t param_size,
    void *data);

ams_status_t ams_platform_get_param(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t module_id,
    uint32_t param_id,
    uint32_t *param_size,
    void *data);

ams_status_t ams_platform_drv_open(struct platform_drv *pdrv);

ams_status_t ams_platform_drv_close(struct platform_drv *pdrv);

ams_status_t ams_platform_drv_ioctl(struct platform_drv *pdrv, uint32_t cmd, void *params, uint32_t size);

ams_status_t ams_platform_drv_util_check_fw_version(struct platform_drv *pdrv);

ams_status_t ams_platform_drv_ssr_init(
    char *client_name,
    struct ssr_drv *ssr,
    int (*event_handler)(uint32_t ss_id_mask, uint32_t event_id, void *ctx),
    void *ctx);

ams_status_t ams_platform_drv_ssr_deinit(struct ssr_drv *ssr);

ams_status_t ams_platform_drv_prep_dsp_memaddr(void *pmem, uint32_t len, uint64_t *p_pa);

int32_t platform_drv_ready(struct platform_drv *pdrv);

ams_status_t ams_platform_apply_prop(struct platform_drv *pdrv, struct ams_graph *gr, uint32_t after_start);

ams_status_t ams_platform_set_endpoints_state(
    struct platform_drv *pdrv,
    struct ams_graph *gr,
    uint32_t enable);


ams_status_t ams_platform_get_num_graphs_cached(struct platform_drv *pdrv, uint32_t *num_graphs);

ams_status_t ams_platform_restore_graph_cached_info_by_idx(struct ams_session *pses, uint32_t idx, struct ams_graph **ppgr);

ams_status_t ams_platform_get_num_prop_cached(struct platform_drv *pdrv, uint32_t gr_idx, uint32_t *num_prop);

ams_status_t ams_platform_restore_prop_cached_by_idx(struct ams_session *pses, uint32_t prop_idx);

#endif

/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdint.h>
#include "ams_osal_mutex.h"
#include "ams_osal_shmem.h"
#include "ams_core_cfg.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define AMS_CORE_CLIENTS_NUM 1 //1 client is supported
#define AMS_CORE_CLIENTS_MEM_IDS 2
#define AMS_CORE_USER_GRAPH_MAX 1 //AMS customer has only 1 AMS graph
    struct ams_core_graph_info
    {
        uint32_t handle;
        uint16_t state;
        uint32_t graph_pld_sz;
        void *graph_pld;
    };

    struct ams_core_shmem_info
    {
        uint8_t mem_sys_ids[AMS_CORE_CLIENTS_MEM_IDS];
        ams_shmem_info shmem_info;
        uint32_t spf_mem_handle;
    };

    struct ams_core_data
    {
        ams_osal_mutex_t mutex;
        struct ams_core_shmem_info smem_cli_info[AMS_CORE_CLIENTS_NUM]; // idx is unique client id. currently client idx=0 is used(other is RFU)
        uint32_t g_ams_handle;
        ams_core_cfg_t ams_core_cfg;
        uint32_t gr_infoq_pos;
        uint32_t gr_infoq_el_num;
        struct ams_core_graph_info gr_infoq[AMS_CORE_USER_GRAPH_MAX];
        struct _ssr{
            uint32_t adsp_st;
            uint32_t mdsp_st;
            ams_osal_mutex_t mutex;
        }ssr;
    };

    int32_t ams_core_init(void);

    int32_t ams_core_deinit(void);

    uint32_t ams_core_open(void);

    int32_t ams_core_ioctl(uint32_t handle, uint32_t cmd, void *params, uint32_t size);

    int32_t ams_core_close(uint32_t hndl);

#ifdef __cplusplus
}
#endif
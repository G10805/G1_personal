/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdint.h>

#define AMS_CORE_CONF_CFG_MAX_NUM 6
#define AMS_CORE_CONF_CFG_DMA_TYPES_MAX_NUM 6
#ifdef __cplusplus
extern "C"
{
#endif
struct ams_sh_res_ctrl_s
{
    int32_t dma_types_num;
    int32_t dma_types[AMS_CORE_CONF_CFG_MAX_NUM];
    int32_t dma_rd_num[AMS_CORE_CONF_CFG_MAX_NUM];
    int32_t dma_wr_num[AMS_CORE_CONF_CFG_MAX_NUM];
};
typedef struct
{
    int32_t init;
    struct ams_sh_res_ctrl_s sh_res_ctrl;
} ams_core_cfg_t;

struct csd_ams_q6_shres_init
{
    uint16_t dma_types_num;
    uint16_t dma_types[AMS_CORE_CONF_CFG_DMA_TYPES_MAX_NUM];
    uint16_t num_rddma[AMS_CORE_CONF_CFG_DMA_TYPES_MAX_NUM];
    uint16_t num_wrdma[AMS_CORE_CONF_CFG_DMA_TYPES_MAX_NUM];
};
int32_t ams_core_cfg_init(ams_core_cfg_t* cfg);
#ifdef __cplusplus
}
#endif
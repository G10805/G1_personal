/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ams_core_ioctl.h"

#define AMS_MCLK_MAX           (5)
#define AMS_MLCK_INVALID       0
#define AMS_MCLK_DISABLE       false
#define AMS_MCLK_ENABLE        true

#define AMS_CLK_1_P536_MHZ     0x177000
#define AMS_CLK_12_P288_MHZ    0xBB8000

//tdm intf paramters
#define AMS_TDM_SYNC_SRC_EXTERNAL                              0
#define AMS_TDM_SYNC_SRC_INTERNAL                              1
#define AMS_TDM_CTRL_DATA_OE_DISABLE                           0
#define AMS_TDM_CTRL_DATA_OE_ENABLE                            1
#define AMS_TDM_SHORT_SYNC_BIT_MODE                             0
#define AMS_TDM_LONG_SYNC_MODE                                  1
#define AMS_TDM_SHORT_SYNC_SLOT_MODE                            2
#define AMS_TDM_SYNC_NORMAL                                     0
#define AMS_TDM_SYNC_INVERT                                     1
#define AMS_TDM_DATA_DELAY_0_BCLK_CYCLE                         0
#define AMS_TDM_DATA_DELAY_1_BCLK_CYCLE                         1
#define AMS_TDM_DATA_DELAY_2_BCLK_CYCLE                         2

/*device intf index*/
enum {
    INTF_IDX_INVALID = 0xFF,
    INTF_IDX_PRIMARY = 0x0,
    INTF_IDX_SECONDARY,
    INTF_IDX_TERTIARY,
    INTF_IDX_QUATERNARY,
    INTF_IDX_QUINARY,
    INTF_IDX_MAX,
};

enum {
    INTF_MODE_MI2S = 0x1,
    INTF_MODE_TDM,
    INTF_MODE_AUXPCM,
    INTF_MODE_INVALID,
};

static const bool g_ams_mclk_enable[AMS_MCLK_MAX] = {
    AMS_MCLK_DISABLE,//PRI
    AMS_MCLK_ENABLE,//SEC
    AMS_MCLK_ENABLE,//TERT
    AMS_MCLK_ENABLE,//QUAT
    AMS_MCLK_ENABLE,//QUIN
};

struct ams_prm_clk_obj {
    uint32_t id;
    uint32_t freq;
    uint32_t attri;
    uint32_t root;
};

struct ams_prm_clk_enable {
    uint32_t num_clks;
    struct ams_prm_clk_obj clk_objs[];
};

struct ams_prm_clk_disable {
    uint32_t num_clks;
    uint32_t clk_ids[];
};

struct ams_prm_header
{
    uint32_t miid;
    uint32_t param_id;
    uint32_t param_size;
    uint32_t error_code;
};

enum dma_dir_type
{
    RD_DMA=0,
    WR_DMA
};

struct aud_dev_obj
{
    uint32_t device_id;
    uint32_t ref_cnt; /**<session based ref cnt, multi graphs may share same dev*/
    uint32_t intf_index;
    uint32_t intf_mode;
    uint8_t dir; /**<0 playback, 1 capture*/
    bool is_group_device;
};

struct ams_prm_dma_rsc
{
    uint32_t dma_type;
    /**< DMA channel type
    @values #LPAIF_CORE_DMA */
    uint32_t num_dma_channels;
    /**< Number of DMA channels
    @values >= 0 */
    uint32_t dma_idx[];
};

struct ams_prm_lpm_rsc
{
    uint32_t lpm_type;
    /**< LPM memory type
    @values #LPAIF_CORE_LPM */
    uint32_t size_in_bytes;
    /**< Amount of LPM needed in bytes */
    uint32_t phy_addr_lsw;
    /**< LSW of allocated LPM physical address */
    uint32_t phy_addr_msw;
    /**< MSW of allocated LPM physical address */
};
// Called only once!
int32_t ams_prm_init(void);
 // Called only once!
int32_t ams_prm_deinit(void);

int32_t ams_prm_dev_enable(struct aud_dev_obj *dev_obj);
int32_t ams_prm_dev_disable(struct aud_dev_obj *dev_obj);
// new api
int32_t ams_prm_clk_enable(struct ams_prm_clk_enable *clk_obj);
int32_t ams_prm_clk_disable(struct ams_prm_clk_disable *clk_obj);
//request resources
int32_t ams_prm_dma_request(enum dma_dir_type type, struct ams_prm_dma_rsc *dma_rsc);
int32_t ams_prm_lpm_request(struct ams_prm_lpm_rsc *lpm_rsc);
//release resources
int32_t ams_prm_dma_release(enum dma_dir_type type, struct ams_prm_dma_rsc *dma_rsc);
int32_t ams_prm_lpm_release(struct ams_prm_lpm_rsc *lpm_rsc);
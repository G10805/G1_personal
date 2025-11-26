/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_endpoint_mgr"
#include <log/log.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ams_endpoint_mgr.h"
#include "ams_prm.h"
#include "audio_hw_clk_api.h"
#define AUD_DEV_CLK_12_288_MHZ 12288000
#define AUD_DEV_NUM_OF_INTERFACES 5

static int32_t ams_endpoint_clk_en_obj_prep(const ams_core_endpoint_t *endpoint, struct ams_prm_clk_enable **ppClks);
static int32_t ams_endpoint_clk_disable_obj_prep(const ams_core_endpoint_t *endpoint, struct ams_prm_clk_disable **ppClks);
int32_t ams_endpoint_clk_attr_get(int32_t port, uint16_t *attr);

typedef enum
{
    ams_endpoint_clock_disable = 0,
    ams_endpoint_clock_enable
} ams_endpoint_clock_setup_mode;

/* Clocks prop:clk_atr */
static struct
{
    uint16_t mask;
    uint16_t attr[AUD_DEV_NUM_OF_INTERFACES];
} ams_endpoint_clks_attr = {
    0, {0}};

int32_t ams_endpoint_clk_attr_set(int32_t port, uint16_t attr)
{
    if (port < AMS_CORE_HW_INTERFACE_TDM1 || port > AMS_CORE_HW_INTERFACE_TDM5)
        return EINVAL;

    ams_endpoint_clks_attr.attr[port - 1] = attr;
    ams_endpoint_clks_attr.mask |= 1 << (port - 1);
    ALOGD("AMS:CLK=> Set Clock for PortId[0x%x]!! attr=[%d] mask=%d", port, ams_endpoint_clks_attr.attr[port - 1], ams_endpoint_clks_attr.mask);
    return 0;
}

int32_t ams_endpoint_clk_attr_get(int32_t port, uint16_t *attr)
{
    if (port < AMS_CORE_HW_INTERFACE_TDM1 || port > AMS_CORE_HW_INTERFACE_TDM5)
        return EINVAL;

    if (ams_endpoint_clks_attr.mask & (1 << (port - 1)))
    {
        *attr = ams_endpoint_clks_attr.attr[port - 1] ? CLOCK_ATTRIBUTE_INVERT_COUPLE_NO : CLOCK_ATTRIBUTE_COUPLE_NO;
    }

    return 0;
}

int32_t ams_endpoint_clk_attr_clear(int32_t port)
{
    if (port < AMS_CORE_HW_INTERFACE_TDM1 || port > AMS_CORE_HW_INTERFACE_TDM5)
        return EINVAL;

    ams_endpoint_clks_attr.attr[port - 1] = 0;
    ams_endpoint_clks_attr.mask &= ~(1 << (port - 1));
    ALOGD("AMS:CLK=> Clear Clock for PortId[0x%x]!! attr=[%d], mask=%d", port, ams_endpoint_clks_attr.attr[port - 1], ams_endpoint_clks_attr.mask);
    return 0;
}

static int32_t ams_endpoint_clk_en_obj_prep(const ams_core_endpoint_t *endpoint, struct ams_prm_clk_enable **ppClks)
{
    uint32_t port = endpoint->tdm_params.hw_interface_id;
    int32_t rc = 0;
    struct ams_prm_clk_obj *pClks;
    const uint32_t max_clks = 2;
    uint32_t clks_sz = sizeof(struct ams_prm_clk_enable) + 1 * sizeof(struct ams_prm_clk_obj);
    if (NULL == (*ppClks = calloc(1, clks_sz)))
    {
        return ENOMEM;
    }

    memset(*ppClks, 0, clks_sz);
    (*ppClks)->num_clks = 1;
    pClks = &(*ppClks)->clk_objs[0];
    switch (port)
    {
    case AMS_CORE_HW_INTERFACE_TDM1:
        pClks->id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_PRI_TDM_IBIT : CLOCK_ID_PRI_TDM_EBIT;
        pClks->root = CLOCK_ROOT_DEFAULT;
        pClks->attri = CLOCK_ATTRIBUTE_INVERT_COUPLE_NO;
        break;
    case AMS_CORE_HW_INTERFACE_TDM2:
        pClks->id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_SEC_TDM_IBIT : CLOCK_ID_SEC_TDM_EBIT;
        pClks->root = CLOCK_ROOT_DEFAULT;
        pClks->attri = CLOCK_ATTRIBUTE_INVERT_COUPLE_NO;
        break;
    case AMS_CORE_HW_INTERFACE_TDM3:
        pClks->id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_TER_TDM_IBIT : CLOCK_ID_TER_TDM_EBIT;
        pClks->root = CLOCK_ROOT_DEFAULT;
        pClks->attri = CLOCK_ATTRIBUTE_INVERT_COUPLE_NO;
        break;
    case AMS_CORE_HW_INTERFACE_TDM4:
        pClks->id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_QUAD_PCM_IBIT : CLOCK_ID_QUAD_PCM_EBIT;
        pClks->root = CLOCK_ROOT_DEFAULT;
        pClks->attri = CLOCK_ATTRIBUTE_INVERT_COUPLE_NO;
        break;
    case AMS_CORE_HW_INTERFACE_TDM5:
        pClks->id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_QUI_PCM_IBIT : CLOCK_ID_QUI_PCM_EBIT;
        pClks->root = CLOCK_ROOT_DEFAULT;
        pClks->attri = CLOCK_ATTRIBUTE_INVERT_COUPLE_NO;
        break;
    default:
        rc = EINVAL;
        ALOGE("CSD2:AMS:CLK => ADSP clock setup NOT supported for port[0x%x]", port);
        break;
    }
    ams_endpoint_clk_attr_get(port, &pClks->attri);
    pClks->freq = endpoint->tdm_params.sample_rate * endpoint->tdm_params.nslots_per_frame * endpoint->tdm_params.slot_width;
    ALOGD("AMS:CLK => Enabling TDM Bit Clk Freq=%d, clk_inv=%d for PortId[0x%x]", pClks->freq, pClks->attri, endpoint->tdm_params.hw_interface_id);
    return rc;
}

static int32_t ams_endpoint_clk_disable_obj_prep(const ams_core_endpoint_t *endpoint, struct ams_prm_clk_disable **ppClks)
{
    uint32_t port = endpoint->tdm_params.hw_interface_id;
    int32_t rc = 0;
    uint32_t *pClks_id;
    uint32_t clks_sz = sizeof(struct ams_prm_clk_disable) + 1 * sizeof(uint32_t);
    if (NULL == (*ppClks = calloc(1, clks_sz)))
    {
        return ENOMEM;
    }

    memset(*ppClks, 0, clks_sz);
    (*ppClks)->num_clks = 1;
    pClks_id = &(*ppClks)->clk_ids[0];
    switch (port)
    {
    case AMS_CORE_HW_INTERFACE_TDM1:
        *pClks_id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_PRI_TDM_IBIT : CLOCK_ID_PRI_TDM_EBIT;
        break;
    case AMS_CORE_HW_INTERFACE_TDM2:
        *pClks_id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_SEC_TDM_IBIT : CLOCK_ID_SEC_TDM_EBIT;
        break;
    case AMS_CORE_HW_INTERFACE_TDM3:
        *pClks_id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_TER_TDM_IBIT : CLOCK_ID_TER_TDM_EBIT;
        break;
    case AMS_CORE_HW_INTERFACE_TDM4:
        *pClks_id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_QUAD_PCM_IBIT : CLOCK_ID_QUAD_PCM_EBIT;
        break;
    case AMS_CORE_HW_INTERFACE_TDM5:
        *pClks_id = (endpoint->tdm_params.sync_src == AMS_TDM_SYNC_SRC_INTERNAL) ? CLOCK_ID_QUI_PCM_IBIT : CLOCK_ID_QUI_PCM_EBIT;
        break;
    default:
        rc = EINVAL;
        ALOGE("AMS:CLK => ADSP clock setup NOT supported for port[0x%x]", port);
        break;
    }
    ALOGD("AMS:CLK => Disabling TDM Bit Clk for PortId[0x%x]", endpoint->tdm_params.hw_interface_id);
    return rc;
}

static int32_t ams_endpoint_setup_clks(const ams_core_endpoint_t *endpoint, ams_endpoint_clock_setup_mode eMode)
{
    int32_t rc = 0;
    void *pClks = NULL;
    uint32_t port_id = endpoint->tdm_params.hw_interface_id;

    if (0 == rc)
    {
        switch (eMode)
        {
        case ams_endpoint_clock_disable:
            rc = ams_endpoint_clk_disable_obj_prep(endpoint, &pClks);
            if (!rc)
                rc = ams_prm_clk_disable((struct ams_prm_clk_disable *)pClks);
            ALOGD("AMS:CLK=> Disable Clock for PortId[0x%x] completed with rc=[0x%x]", port_id, rc);
            break;
        case ams_endpoint_clock_enable:
            rc = ams_endpoint_clk_en_obj_prep(endpoint, &pClks);
            if (!rc)
                rc = ams_prm_clk_enable((struct ams_prm_clk_enable *)pClks);
            ALOGD("AMS:CLK=> Enable Clock for PortId[0x%x] completed with rc=[0x%x]", port_id, rc);
            break;
        }
    }

    if (pClks != NULL)
        free(pClks);

    return rc;
}

int32_t ams_endpoint_clk_enable(const ams_core_endpoint_t *endpoint)
{
    int32_t rc = 0;
    rc = ams_endpoint_setup_clks(endpoint, ams_endpoint_clock_enable);
    ALOGD("AMS:CLK=>Enable <Clock>stop</Clock> completed with rc=[0x%x]", rc);
    return rc;
}

int32_t ams_endpoint_clk_disable(const ams_core_endpoint_t *endpoint)
{
    int32_t rc = 0;
    rc = ams_endpoint_setup_clks(endpoint, ams_endpoint_clock_disable);
    ams_endpoint_clk_attr_clear(endpoint->tdm_params.hw_interface_id);
    ALOGD("AMS:CLK=>Enable <Clock>stop</Clock> completed with rc=[0x%x]", rc);
    return rc;
}

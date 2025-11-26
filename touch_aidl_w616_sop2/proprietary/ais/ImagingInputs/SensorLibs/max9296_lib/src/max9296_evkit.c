/**
 * @file max9296_evkit.c
 *
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "max9296_evkit.h"


typedef enum
{
    EVKIT_MODE_SINGLE_STREAM = 0,
    EVKIT_MODE_DUAL_STREAM,
    EVKIT_MODE_MAX,
}evkit_mode_id_t;

typedef struct
{
    maxim_pipeline_t pipeline;

    struct camera_i2c_reg_array* init_array;
    uint32_t         size_init_array;

    uint64_t         pclk;
    uint32_t         line_length_pck;
    uint32_t         frame_length_lines;

}evkit_mode_t;

typedef struct
{
    /* data */
    void* private_ctxt;
}evkit_contxt_t;

#define CID_VC0        0
#define CID_VC1        4

static int evkit_detect(max9296_context_t* ctxt, uint32 link);
static int evkit_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);
static int evkit_init_link(max9296_context_t* ctxt, uint32 link);
static int evkit_start_link(max9296_context_t* ctxt, uint32 link);
static int evkit_stop_link(max9296_context_t* ctxt, uint32 link);

static max9296_sensor_t evkit_info = {
    .id = MAXIM_SENSOR_ID_EVKIT,
    .detect = evkit_detect,
    .get_link_cfg = evkit_get_link_cfg,

    .init_link = evkit_init_link,
    .start_link = evkit_start_link,
    .stop_link = evkit_stop_link,
};

static maxim_pipeline_t evkit_modes[EVKIT_MODE_MAX][MAXIM_LINK_MAX][MAXIM_PIPELINE_MAX] =
{
    [EVKIT_MODE_SINGLE_STREAM] = {
        {
            {
                .id = MAXIM_PIPELINE_X,
                .mode =
                {
                    .fmt = QCARCAM_FMT_RGB_888,
                    .res = {.width = 1920, .height = 1728, .fps = 30.0f},
                    .channel_info = {.vc = 0, .dt = CSI_DT_RGB888, .cid = CID_VC0},
                }
            }
        }
    },
    [EVKIT_MODE_DUAL_STREAM] = {
        {
            {
                .id = MAXIM_PIPELINE_X,
                .mode =
                {
                    .fmt = QCARCAM_FMT_RGB_888,
                    .res = {.width = 960, .height = 1080, .fps = 30.0f},
                    .channel_info = {.vc = 0, .dt = CSI_DT_RGB888, .cid = CID_VC0},
                }
            },
            {
                .id = MAXIM_PIPELINE_Z,
                .mode =
                {
                    .fmt = QCARCAM_FMT_RGB_888,
                    .res = {.width = 960, .height = 1080, .fps = 30.0f},
                    .channel_info = {.vc = 1, .dt = CSI_DT_RGB888, .cid = CID_VC0},
                }
            },
        },
        {
            {
                .id = MAXIM_PIPELINE_Y,
                .mode =
                {
                    .fmt = QCARCAM_FMT_RGB_888,
                    .res = {.width = 960, .height = 1080, .fps = 30.0f},
                    .channel_info = {.vc = 0, .dt = CSI_DT_RGB888, .cid = CID_VC0},
                }
            },
            {
                .id = MAXIM_PIPELINE_U,
                .mode =
                {
                    .fmt = QCARCAM_FMT_RGB_888,
                    .res = {.width = 960, .height = 1080, .fps = 30.0f},
                    .channel_info = {.vc = 1, .dt = CSI_DT_RGB888, .cid = CID_VC0},
                }
            },
        }
    }
};

max9296_sensor_t* evkit_get_sensor_info(void)
{
    return &evkit_info;
}

static int evkit_detect(max9296_context_t* ctxt, uint32 link)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(link);

    SHIGH("evkit_detect");
    return 0;
}

static int evkit_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg)
{
    max9296_context_t* pCtxt = ctxt;
    int rc = 0, i = 0;

    unsigned int mode = pCtxt->max9296_sensors[link].mode;

    switch (mode)
    {
    case EVKIT_MODE_SINGLE_STREAM:
        p_cfg->num_pipelines = 1;
        break;
    case EVKIT_MODE_DUAL_STREAM:
        p_cfg->num_pipelines = 2;
        break;
    default:
        p_cfg->num_pipelines = 1;
        break;
    }

    for (i = 0; i < p_cfg->num_pipelines; i++)
    {
        p_cfg->pipelines[i] = evkit_modes[mode][link][i];
    }

    return rc;
}

static int evkit_init_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("evkit %d init in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

static int evkit_start_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        pSensor->state = SENSOR_STATE_STREAMING;
    }
    else
    {
        SERR("evkit %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

static int evkit_stop_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("evkit %d stop in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

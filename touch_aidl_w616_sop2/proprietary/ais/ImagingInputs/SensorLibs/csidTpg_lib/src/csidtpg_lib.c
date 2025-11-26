/**
 * @file csidtpg_lib.c
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "CameraSensorDeviceInterface.h"

#include "csidtpg_lib.h"


#define CSIDTPG_SETTLE_CNT 0xE
#define CSIDTPG_WIDTH     1920
#define CSIDTPG_STRIDE    CSIDTPG_WIDTH*2
#define CSIDTPG_HEIGHT    1080
#define CSIDTPG_FPS       60.0f

#define CSIDTPG_FMT       QCARCAM_FMT_UYVY_8
#define CSIDTPG_DT        CSI_DT_YUV422_8

/**
 * DEFINITIONS
 */
typedef struct
{
    /*must be first element*/
    sensor_lib_t sensor_lib;

    void* ctrl;
} csidtpg_context_t;

/**
 * FUNCTIONS
 */
static int csidtpg_close_lib(void* ctrl);
static int csidtpg_detect_device (void* ctxt);
static int csidtpg_detect_device_channels (void* ctxt);
static int csidtpg_init_setting (void* ctxt);
static int csidtpg_set_channel_mode (void* ctxt, unsigned int src_id_mask, unsigned int mode);
static int csidtpg_start_stream(void* ctxt, unsigned int src_id_mask);
static int csidtpg_stop_stream(void* ctxt, unsigned int src_id_mask);
static int csidtpg_config_resolution(void* ctxt, int32 width, int32 height);
static int csidtpg_field_info_query (void* ctxt, boolean *even_field, uint64 *field_ts);
static int csidtpg_set_platform_func_table (void* ctxt, sensor_platform_func_table_t* table);
static int csidtpg_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param);


/**
 * GLOBAL VARIABLES
 */
static sensor_lib_t csidtpg_lib_ptr =
{
  .sensor_slave_info =
  {
      .sensor_name = SENSOR_MODEL,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .i2c_freq_mode = SENSOR_I2C_MODE_CUSTOM,
  },
  .src_id_enable_mask = 1,
  .num_channels = 1,
  .channels =
  {
    {
      .output_mode =
      {
        .fmt = CSIDTPG_FMT,
        .res = {.width = CSIDTPG_WIDTH, .height = CSIDTPG_HEIGHT, .fps = CSIDTPG_FPS},
        .channel_info = {.vc = 0, .dt = CSIDTPG_DT, .cid = 0,},
      },
      .num_subchannels = 1,
      .subchan_layout =
      {
        {
          .src_id = 0,
          .x_offset = 0,
          .y_offset = 0,
          .stride = CSIDTPG_STRIDE,
        },
      },
    },
  },
  .num_subchannels = 1,
  .subchannels =
  {
    {
      .src_id = 0,
      .modes =
      {
        {
          .fmt = CSIDTPG_FMT,
          .res = {.width = CSIDTPG_WIDTH, .height = CSIDTPG_HEIGHT, .fps = CSIDTPG_FPS},
          .channel_info = {.vc = 0, .dt = CSIDTPG_DT, .cid = 0,},
        },
      },
      .num_modes = 1,
    },
  },
  .csi_params =
  {
    {
    .lane_cnt = 4,
    .settle_cnt = CSIDTPG_SETTLE_CNT,
    .lane_mask = 0x1F,
    .combo_mode = 0,
    .is_csi_3phase = 0,
    .mipi_rate = 0,
    }
  },
  .sensor_close_lib = &csidtpg_close_lib,
  .sensor_capability = 0,
  .sensor_custom_func =
  {
    .sensor_detect_device = &csidtpg_detect_device,
    .sensor_detect_device_channels = &csidtpg_detect_device_channels,
    .sensor_init_setting = &csidtpg_init_setting,
    .sensor_set_channel_mode = &csidtpg_set_channel_mode,
    .sensor_start_stream = &csidtpg_start_stream,
    .sensor_stop_stream = &csidtpg_stop_stream,
    .sensor_config_resolution = &csidtpg_config_resolution,
    .sensor_query_field = &csidtpg_field_info_query,
    .sensor_set_platform_func_table = &csidtpg_set_platform_func_table,
    .sensor_s_param = &csidtpg_s_param,
  },
  .use_sensor_custom_func = TRUE,
};

/**
 * FUNCTION: csidtpg_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
static void* csidtpg_open_lib(void* ctrl, void* arg)
{
    (void)ctrl;

    SLOW("open lib called");

    sensor_open_lib_t* device_info = (sensor_open_lib_t*)arg;
    csidtpg_context_t* pCtxt = calloc(1, sizeof(csidtpg_context_t));
    if (pCtxt)
    {
        memcpy(&pCtxt->sensor_lib, &csidtpg_lib_ptr, sizeof(pCtxt->sensor_lib));
        pCtxt->sensor_lib.sensor_slave_info.camera_id = device_info->cameraId;
    }

    return pCtxt;
}

static int csidtpg_close_lib(void* ctxt)
{
    SLOW("close lib called");

    if (ctxt)
    {
       free(ctxt);
    }

    return 0;
}

/**
 * FUNCTION: csidtpg_init_setting
 *
 * DESCRIPTION: init setting
 **/
static int csidtpg_init_setting(void* ctxt)
{
    CAM_UNUSED(ctxt);

    SLOW("csidtpg_init_setting called");

    return 0;
}


/**
 * FUNCTION: csidtpg_detect_device
 *
 * DESCRIPTION: detect and initialize
 **/
static int csidtpg_detect_device(void* ctxt)
{
    CAM_UNUSED(ctxt);

    SLOW("detect device called");

    return 0;
}

/**
 * FUNCTION: csidtpg_detect_device_channels
 *
 * DESCRIPTION: detect device channels
 **/
static int csidtpg_detect_device_channels(void* ctxt)
{
    CAM_UNUSED(ctxt);

    return 0;
}

/**
 * FUNCTION: csidtpg_set_channel_mode
 *
 * DESCRIPTION: set channel mode
 **/
static int csidtpg_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);
    CAM_UNUSED(mode);

    return 0;
}

/**
 * FUNCTION: csidtpg_start_stream
 *
 * DESCRIPTION: start stream
 **/
static int csidtpg_start_stream(void* ctxt, unsigned int src_id_mask)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);

    return 0;
}

/**
 * FUNCTION: csidtpg_stop_stream
 *
 * DESCRIPTION: stop stream
 **/
static int csidtpg_stop_stream(void* ctxt, unsigned int src_id_mask)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);

    return 0;
}

/**
 * FUNCTION: csidtpg_config_resolution
 *
 * DESCRIPTION: config dynamic resolution
 **/
static int csidtpg_config_resolution(void* ctxt, int32 width, int32 height)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(width);
    CAM_UNUSED(height);

    return -1;
}

/**
 * FUNCTION: csidtpg_field_info_query
 *
 * DESCRIPTION: get the current field info
 **/
static int csidtpg_field_info_query(void* ctxt, boolean *even_field, uint64 *field_ts)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(even_field);
    CAM_UNUSED(field_ts);

    return -1;
}

/**
 * FUNCTION: csidtpg_set_i2c_func_table
 *
 * DESCRIPTION: set i2c function table
 **/
static int csidtpg_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table)
{
    (void)ctxt;

    SLOW("csidtpg_set_i2c_func_table 0x%p", table);

    return 0;
}

/**
 * FUNCTION: csidtpg_s_param
 *
 * DESCRIPTION: set color parameters
 **/
static int csidtpg_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("csidtpg_s_param Invalid ctxt");
        return -1;
    }

    if (p_param == NULL)
    {
        SERR("csidtpg_s_param Invalid params");
        return -1;
    }

    switch(param)
    {
        default:
            SERR("csidtpg_s_param. Param not supported = %d, src_id = %d", param, src_id);
            rc = -1;
            break;
    }
    return rc;
}

/**
 * FUNCTION: CameraSensorDevice_Open_csidtpg
 *
 * DESCRIPTION: Entry function for csidtpg driver framework
 **/
CAM_API CameraResult CameraSensorDevice_Open_csidtpg(CameraDeviceHandleType** ppNewHandle,
                                                CameraDeviceIDType deviceId)
{
    sensor_lib_interface_t sensor_lib_interface = {
        .sensor_open_lib = csidtpg_open_lib,
    };
SERR("camera stub\n");
    SHIGH("CameraSensorDevice_Open_csidtpg called");

    return CameraSensorDevice_Open(ppNewHandle, deviceId, &sensor_lib_interface);
}

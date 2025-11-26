/**
 * @file sensorstub_lib.c
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

#include "sensorstub_lib.h"

#ifdef SENSORSTUB_ENABLE_XML_CONFIG
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#endif /* SENSORSTUB_ENABLE_XML_CONFIG */


#define SENSORSTUB_SETTLE_CNT 0xE
#define SENSORSTUB_WIDTH     1920
#define SENSORSTUB_STRIDE    SENSORSTUB_WIDTH*3
#define SENSORSTUB_HEIGHT    2160
#define SENSORSTUB_FPS       60.0f

#define SENSORSTUB_FMT       QCARCAM_FMT_RGB_888
#define SENSORSTUB_DT        CSI_DT_RGB888

#define NUM_CHANNELS         2


/**
 * DEFINITIONS
 */
typedef struct
{
    sensor_exposure_info_t exposure_info;
    qcarcam_hdr_exposure_config_t hdr_exposure_info;
}stubsensor_setting_t;

typedef struct
{
    /*must be first element*/
    sensor_lib_t sensor_lib;

    boolean platform_fcn_tbl_initialized;
    sensor_platform_func_table_t platform_fcn_tbl;

    stubsensor_setting_t sensor_setting[NUM_CHANNELS];

    void* ctrl;
} sensorstub_context_t;

/**
 * FUNCTIONS
 */
static int sensorstub_close_lib(void* ctrl);
static int sensorstub_detect_device (void* ctxt);
static int sensorstub_detect_device_channels (void* ctxt);
static int sensorstub_init_setting (void* ctxt);
static int sensorstub_set_channel_mode (void* ctxt, unsigned int src_id_mask, unsigned int mode);
static int sensorstub_start_stream(void* ctxt, unsigned int src_id_mask);
static int sensorstub_stop_stream(void* ctxt, unsigned int src_id_mask);
static int sensorstub_config_resolution(void* ctxt, int32 width, int32 height);
static int sensorstub_field_info_query (void* ctxt, boolean *even_field, uint64 *field_ts);
static int sensorstub_set_platform_func_table (void* ctxt, sensor_platform_func_table_t* table);
static int sensorstub_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param);
static int sensorstub_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* p_param);
static int sensorstub_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* p_param);
static int sensorstub_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* param);


/**
 * GLOBAL VARIABLES
 */
static sensor_lib_t sensorstub_lib_ptr =
{
  .sensor_slave_info =
  {
      .sensor_name = SENSOR_MODEL,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .i2c_freq_mode = SENSOR_I2C_MODE_CUSTOM,
  },
  .src_id_enable_mask = 3,
  .num_channels = 1,
  .channels =
  {
    {
      .output_mode =
      {
        .fmt = SENSORSTUB_FMT,
        .res = {.width = SENSORSTUB_WIDTH, .height = SENSORSTUB_HEIGHT, .fps = SENSORSTUB_FPS},
        .channel_info = {.vc = 0, .dt = SENSORSTUB_DT, .cid = 0,},
      },
      .num_subchannels = 2,
      .subchan_layout =
      {
        {
          .src_id = 0,
          .x_offset = 0,
          .y_offset = 0,
          .stride = SENSORSTUB_STRIDE,
        },
        {
          .src_id = 1,
          .x_offset = 0,
          .y_offset = 0,
          .stride = SENSORSTUB_STRIDE,
        }
      },
    },
  },
  .num_subchannels = 2,
  .subchannels =
  {
    {
      .src_id = 0,
      .modes =
      {
        {
          .fmt = SENSORSTUB_FMT,
          .res = {.width = SENSORSTUB_WIDTH, .height = SENSORSTUB_HEIGHT, .fps = SENSORSTUB_FPS},
          .channel_info = {.vc = 0, .dt = SENSORSTUB_DT, .cid = 0,},
        },
      },
      .num_modes = 1,
      .csi_idx = 0,
    },
    {
      .src_id = 1,
      .modes =
      {
        {
          .fmt = SENSORSTUB_FMT,
          .res = {.width = SENSORSTUB_WIDTH, .height = SENSORSTUB_HEIGHT, .fps = SENSORSTUB_FPS},
          .channel_info = {.vc = 0, .dt = SENSORSTUB_DT, .cid = 0,},
        },
      },
      .num_modes = 1,
      .csi_idx = 1,
    },
  },
  .sensor_output =
  {
    .output_format = SENSOR_YCBCR,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_8_BIT_DIRECT,
    .filter_arrangement = SENSOR_UYVY,
  },
  .csi_params =
  {
    {
      .lane_cnt = 4,
      .settle_cnt = SENSORSTUB_SETTLE_CNT,
      .lane_mask = 0x1F,
      .combo_mode = 0,
      .is_csi_3phase = 0,
      .mipi_rate = 0,
    },
    {
      .lane_cnt = 4,
      .settle_cnt = SENSORSTUB_SETTLE_CNT,
      .lane_mask = 0x1F,
      .combo_mode = 0,
      .is_csi_3phase = 0,
      .mipi_rate = 0,
    }
  },
  .sensor_close_lib = &sensorstub_close_lib,
  .sensor_capability = (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG),
  .sensor_custom_func =
  {
    .sensor_detect_device = &sensorstub_detect_device,
    .sensor_detect_device_channels = &sensorstub_detect_device_channels,
    .sensor_init_setting = &sensorstub_init_setting,
    .sensor_set_channel_mode = &sensorstub_set_channel_mode,
    .sensor_start_stream = &sensorstub_start_stream,
    .sensor_stop_stream = &sensorstub_stop_stream,
    .sensor_config_resolution = &sensorstub_config_resolution,
    .sensor_query_field = &sensorstub_field_info_query,
    .sensor_set_platform_func_table = &sensorstub_set_platform_func_table,
    .sensor_s_param = &sensorstub_s_param,
  },
  .use_sensor_custom_func = TRUE,
  .exposure_func_table =
  {
    .sensor_calculate_exposure = &sensorstub_calculate_exposure,
    .sensor_exposure_config = &sensorstub_exposure_config,
    .sensor_hdr_exposure_config = &sensorstub_hdr_exposure_config,
  },
};

/**
 * FUNCTION: sensorstub_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
static void* sensorstub_open_lib(void* ctrl, void* arg)
{
    (void)ctrl;

    SHIGH("open lib called");

    sensor_open_lib_t* device_info = (sensor_open_lib_t*)arg;
    sensorstub_context_t* pCtxt = calloc(1, sizeof(sensorstub_context_t));
    if (pCtxt)
    {
        memcpy(&pCtxt->sensor_lib, &sensorstub_lib_ptr, sizeof(pCtxt->sensor_lib));
        pCtxt->sensor_lib.sensor_slave_info.camera_id = device_info->cameraId;
        pCtxt->ctrl = ctrl;
    }

    return pCtxt;
}

static int sensorstub_close_lib(void* ctxt)
{
    SHIGH("close lib called");

    if (ctxt)
    {
       free(ctxt);
    }

    return 0;
}

/**
 * FUNCTION: sensorstub_init_setting
 *
 * DESCRIPTION: init setting
 **/
static int sensorstub_init_setting(void* ctxt)
{
    CAM_UNUSED(ctxt);

    SHIGH("sensorstub_init_setting called");

    return 0;
}


/**
 * FUNCTION: sensorstub_detect_device
 *
 * DESCRIPTION: detect and initialize
 **/
static int sensorstub_detect_device(void* ctxt)
{
    CAM_UNUSED(ctxt);

    SHIGH("detect device called");

    return 0;
}

/**
 * FUNCTION: sensorstub_detect_device_channels
 *
 * DESCRIPTION: detect device channels
 **/
static int sensorstub_detect_device_channels(void* ctxt)
{
    CAM_UNUSED(ctxt);

    return 0;
}

/**
 * FUNCTION: sensorstub_set_channel_mode
 *
 * DESCRIPTION: set channel mode
 **/
static int sensorstub_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);
    CAM_UNUSED(mode);

    return 0;
}

/**
 * FUNCTION: sensorstub_start_stream
 *
 * DESCRIPTION: start stream
 **/
static int sensorstub_start_stream(void* ctxt, unsigned int src_id_mask)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);

    return 0;
}

/**
 * FUNCTION: sensorstub_stop_stream
 *
 * DESCRIPTION: stop stream
 **/
static int sensorstub_stop_stream(void* ctxt, unsigned int src_id_mask)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(src_id_mask);

    return 0;
}

/**
 * FUNCTION: sensorstub_config_resolution
 *
 * DESCRIPTION: config dynamic resolution
 **/
static int sensorstub_config_resolution(void* ctxt, int32 width, int32 height)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(width);
    CAM_UNUSED(height);

    return -1;
}

/**
 * FUNCTION: sensorstub_field_info_query
 *
 * DESCRIPTION: get the current field info
 **/
static int sensorstub_field_info_query(void* ctxt, boolean *even_field, uint64 *field_ts)
{
    CAM_UNUSED(ctxt);
    CAM_UNUSED(even_field);
    CAM_UNUSED(field_ts);

    return -1;
}

/**
 * FUNCTION: sensorstub_set_i2c_func_table
 *
 * DESCRIPTION: set i2c function table
 **/
static int sensorstub_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table)
{
    sensorstub_context_t* pCtxt = (sensorstub_context_t*)ctxt;

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        if (!table || !table->event_cb)
        {
            SERR("Invalid func table param");
            return -1;
        }

        pCtxt->platform_fcn_tbl = *table;
        pCtxt->platform_fcn_tbl_initialized = 1;
        SLOW("Func table set");
    }

    return 0;
}

/**
 * FUNCTION: sensorstub_s_param
 *
 * DESCRIPTION: set color parameters
 **/
static int sensorstub_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("sensorstub_s_param Invalid ctxt");
        return -1;
    }

    if (p_param == NULL)
    {
        SERR("sensorstub_s_param Invalid params");
        return -1;
    }

    switch(param)
    {
        default:
            SERR("sensorstub_s_param. Param not supported = %d, src_id = %d", param, src_id);
            rc = -1;
            break;
    }
    return rc;
}

/**
 * FUNCTION: sensorstub_calculate_exposure
 *
 * DESCRIPTION: calculates exposure settings
 *              for non-SHDR sensors
 **/
static int sensorstub_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* param)
{
    (void)(ctxt);
    (void)(src_id);
    (void)(param);
    return 0;
}

/**
 * FUNCTION: sensorstub_calculate_exposure
 *
 * DESCRIPTION: set exposure settings for non-HDR sensors
 **/
static int sensorstub_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* param)
{
    sensorstub_context_t* pCtxt = (sensorstub_context_t*)ctxt;
    int rc = CAMERA_SUCCESS;

    if (src_id < NUM_CHANNELS)
    {
        pCtxt->sensor_setting[src_id].exposure_info = *param;
    }
    else
    {
        rc = CAMERA_EFAILED;
    }

    if (CAMERA_SUCCESS == rc)
    {
        // The exposure data is sent as a vendor event because sensor
        // library is a vendor specific and opaque for AIS framework.
        CameraInputEventPayloadType payload = {};
        payload.src_id = src_id;
        size_t vendorDataSize = 0, expSettingSize = 0;
        vendorDataSize = sizeof(payload.vendor_data.data);
        expSettingSize = sizeof(pCtxt->sensor_setting[src_id].exposure_info);
        if (vendorDataSize >= expSettingSize)
        {
            sensor_exposure_info_t* pExposure =
                (sensor_exposure_info_t*)(payload.vendor_data.data);
            *pExposure = pCtxt->sensor_setting[src_id].exposure_info;
            SLOW("Set exposure: exposure_mode_type %d exposure_time %f gain %f",
                pExposure->exposure_time, pExposure->real_gain);
            pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_VENDOR, &payload);
        }
        else
        {
            SERR("Size of exposure data size %u is bigger than vendor data size %u",
                expSettingSize, vendorDataSize);
            rc = CAMERA_EFAILED;
        }
    }

    return rc;
}

/**
 * FUNCTION: sensorstub_calculate_exposure
 *
 * DESCRIPTION: set exposure settings for HDR sensors
 **/
static int sensorstub_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* param)
{
    sensorstub_context_t* pCtxt = (sensorstub_context_t*)ctxt;
    int rc = CAMERA_SUCCESS;

    if (src_id < NUM_CHANNELS)
    {
        pCtxt->sensor_setting[src_id].hdr_exposure_info = *param;
    }
    else
    {
        rc = CAMERA_EFAILED;
    }

    if (CAMERA_SUCCESS == rc)
    {
        // The exposure data is sent as a vendor event because sensor
        // library is a vendor specific and opaque for AIS framework.
        CameraInputEventPayloadType payload = {};
        payload.src_id = src_id;
        size_t vendorDataSize = 0, expSettingSize = 0;
        vendorDataSize = sizeof(payload.vendor_data.data);
        expSettingSize = sizeof(pCtxt->sensor_setting[src_id].hdr_exposure_info);
        if (vendorDataSize >= expSettingSize)
        {
            qcarcam_hdr_exposure_config_t* pHdrExposure =
                (qcarcam_hdr_exposure_config_t*)(payload.vendor_data.data);
            *pHdrExposure = pCtxt->sensor_setting[src_id].hdr_exposure_info;
            SLOW("Set HDR exposure: mode %d exposure_time (%f,%f,%f) gain (%f,%f,%f)",
                pHdrExposure->exposure_mode_type, pHdrExposure->exposure_time[0],
                pHdrExposure->exposure_time[1], pHdrExposure->exposure_time[2],
                pHdrExposure->gain[0], pHdrExposure->gain[1], pHdrExposure->gain[2]);
            pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_VENDOR, &payload);
        }
        else
        {
            SERR("Size of exposure data size %u is bigger than vendor data size %u",
                expSettingSize, vendorDataSize);
            rc = CAMERA_EFAILED;
        }
    }

    return rc;
}

/**
 * FUNCTION: CameraSensorDevice_Open_sensorstub
 *
 * DESCRIPTION: Entry function for device driver framework
 **/
CAM_API CameraResult CameraSensorDevice_Open_sensorstub(CameraDeviceHandleType** ppNewHandle,
                                                CameraDeviceIDType deviceId)
{
    sensor_lib_interface_t sensor_lib_interface = {
        .sensor_open_lib = sensorstub_open_lib,
    };

    SHIGH("CameraSensorDevice_Open_sensorstub called");

    return CameraSensorDevice_Open(ppNewHandle, deviceId, &sensor_lib_interface);
}

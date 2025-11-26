/**
 * @file max9296_lib.c
 * Copyright (c) 2017-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>


#include "CameraSensorDeviceInterface.h"
#include "CameraEventLog.h"
#include "max9296_lib.h"
#include "vendor_ext_properties.h"

#include "ar0231.h"
#include "ar0231_ext_isp.h"
#include "ar0820.h"
#include "ar0234_ext_fpga.h"
#include "max9295_sensor.h"
#include "max9295_loopback_sensor.h"
#include "max9296_evkit.h"


#define MAX9296_INTR_SETTLE_SLEEP 100 //100ms for now
#define MAX9296_MAX_ERROR_CHECK_CNT 10

#define MAX9296_CLK_25MHZ                25000000
#define MAX9296_MIN_FPS_FSYNC            10
#define MAX9296_MAX_FPS_FSYNC            60

/*External APIs*/
static int max9296_sensor_close_lib(void* ctxt);
static int max9296_sensor_power_suspend(void* ctxt, CameraPowerEventType powerEventId);
static int max9296_sensor_power_resume(void* ctxt, CameraPowerEventType powerEventId);
static int max9296_sensor_detect_device(void* ctxt);
static int max9296_sensor_detect_device_channels(void* ctxt);
static int max9296_sensor_init_setting(void* ctxt);
static int max9296_sensor_set_channel_mode(void* ctxt, unsigned int src_id, unsigned int mode);
static int max9296_sensor_start_stream(void* ctxt, unsigned int src_id_mask);
static int max9296_sensor_stop_stream(void* ctxt, unsigned int src_id_mask);
static int max9296_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table);
static int max9296_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int max9296_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int max9296_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int max9296_sensor_gamma_config(void* ctxt, unsigned int src_id, qcarcam_gamma_config_t* gamma);
static int max9296_sensor_s_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param);
static int max9296_sensor_g_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param);
static int max9296_sensor_set_cci_sync_param(void* ctxt, void* param);

#ifdef MAX9296_ENABLE_FRAME_FREEZE
static int max9296_sensor_process_frame_data(void* ctxt, CameraInputProcessFrameDataType* p_frame);
#endif

/*Internal APIs*/
static int max9296_set_default(max9296_context_t * pCtxt, const sensor_open_lib_t* device_info);
static int max9296_sensor_remap_channels(max9296_context_t* pCtxt);
#ifndef DEBUG_SINGLE_SENSOR
static unsigned int max9296_get_bpp_from_dt(uint32 dt);
#endif
static int max9296_set_init_sequence(max9296_context_t* pCtxt);
static int max9296_fsync_init(max9296_context_t* pCtxt);
#ifdef MAX9296_ENABLE_INTR_HANDLER
static int max9296_intr_read_global_error(max9296_context_t* pCtxt, bool* err_flag);
static int max9296_intr_clear_cnt(max9296_context_t* pCtxt);
static int max9296_intr_read_status(max9296_context_t* pCtxt, unsigned short* intr_link_src);
static void max9296_intr_handler(void* data);
#endif
static int max9296_sensor_enter_sleep(max9296_context_t* pCtxt);
static int max9296_sensor_wake_up(max9296_context_t* pCtxt);
static void max9296_sensor_delay(max9296_context_t* pCtxt);
static void io_expander_detect(max9296_context_t* pCtxt);
static int io_expander_reset(max9296_context_t* pCtxt);
static int io_expander_control_pwr(max9296_context_t* pCtxt, uint8 enable);
static void max9296_gmsl2_check_lock_state(max9296_context_t* pCtxt, uint32 tryTimes);

static void max9296_test_i2c_bulk_write(void* ctxt, unsigned int *data);
static void max9296_test_i2c_bulk_read(void* ctxt, unsigned int *data);

#define I2C_BURST_ADDR_IDX 0
#define I2C_BURST_LENGTH_IDX 2

static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
      .sensor_name = SENSOR_MODEL,
      .slave_addr = MSM_DES_0_SLAVEADDR,
      .i2c_freq_mode = SENSOR_I2C_MODE_CUSTOM,
      .addr_type = CAMERA_I2C_WORD_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .sensor_id_info =
      {
        .sensor_id_reg_addr = 0x00,
        .sensor_id = MSM_DES_0_SLAVEADDR,
        .sensor_id_mask = 0xff00,
      },
      .power_setting_array =
      {
        .power_up_setting_a =
        {
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VDIG,
            .config_val = 1,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_LOW,
            .delay = 1,
          },
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_HIGH,
            .delay = 20,
          },
        },
        .size_up = 3,
        .power_down_setting_a =
        {
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_LOW,
            .delay = 1,
          },
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VDIG,
            .config_val = 0,
            .delay = 0,
          },
        },
        .size_down = 2,
      },
      .is_init_params_valid = 1,
  },
  .csi_params =
  {
    {
    .lane_cnt = 4,
    .settle_cnt = 0xE,
    .lane_mask = 0x1F,
    .combo_mode = 0,
    .is_csi_3phase = 0,
    .mipi_rate = 1200000000,
    .vcx_mode = 1 /*enable VCX extension*/
    }
  },
  .sensor_close_lib = max9296_sensor_close_lib,
  .exposure_func_table =
  {
    .sensor_calculate_exposure = &max9296_calculate_exposure,
    .sensor_exposure_config = &max9296_sensor_exposure_config,
    .sensor_hdr_exposure_config = &max9296_sensor_hdr_exposure_config,
  },
  .sensor_capability = (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG | 1 << SENSOR_CAPABILITY_GAMMA_CONFIG |
                          1 << SENSOR_CAPABILITY_VENDOR_PARAM),
  .sensor_custom_func =
  {
    .sensor_set_platform_func_table = &max9296_sensor_set_platform_func_table,
    .sensor_power_suspend = max9296_sensor_power_suspend,
    .sensor_power_resume = max9296_sensor_power_resume,
    .sensor_detect_device = &max9296_sensor_detect_device,
    .sensor_detect_device_channels = &max9296_sensor_detect_device_channels,
    .sensor_init_setting = &max9296_sensor_init_setting,
    .sensor_set_channel_mode = &max9296_sensor_set_channel_mode,
    .sensor_start_stream = &max9296_sensor_start_stream,
    .sensor_stop_stream = &max9296_sensor_stop_stream,
    .sensor_s_param = &max9296_sensor_s_param,
    .sensor_g_param = &max9296_sensor_g_param,
#ifdef MAX9296_ENABLE_FRAME_FREEZE
    .sensor_process_frame_data = &max9296_sensor_process_frame_data,
#endif
    .sensor_set_cci_sync_param = &max9296_sensor_set_cci_sync_param,
  },
  .use_sensor_custom_func = TRUE,
  //first frame valid image data bytes is less than expected, so skip first frame
  .sensor_num_frame_skip = 1,
};

#ifdef MAX9296_DEFAULT_BAYER
static max9296_topology_config_t default_config =
{
    .boardType = CAMERA_HW_BOARD_ADPAIR_V2_PL195,
    .opMode = MAXIM_OP_MODE_DEFAULT,
    .num_of_cameras = 1,
    .sensors = {
        { .id = MAXIM_SENSOR_ID_AR0231, .mode = 0 }
    },
    .powersave_mode = CAMERA_POWERSAVE_MODE_NONE,
};
#else
static max9296_topology_config_t default_config =
{
    .boardType = CAMERA_HW_BOARD_ADPAIR_V2_PL195,
    .opMode = MAXIM_OP_MODE_DEFAULT,
    .num_of_cameras = 2,
    .sensors = {
        { .id = MAXIM_SENSOR_ID_AR0231_EXT_ISP, .mode = 0, .fsync_mode = MAX9296_FSYNC_MODE_DISABLED },
        { .id = MAXIM_SENSOR_ID_AR0231_EXT_ISP, .mode = 0, .fsync_mode = MAX9296_FSYNC_MODE_DISABLED }
    },
    .powersave_mode = CAMERA_POWERSAVE_MODE_BRIDGECHIP_OFF,
};
#endif

static max9296_sensor_info_t max9296_sensors_init_table[] =
{
  {
      .state = SENSOR_STATE_INVALID,
      .serializer_alias = MSM_DES_0_ALIAS_ADDR_CAM_SER_0,
      .sensor_alias = MSM_DES_0_ALIAS_ADDR_CAM_SNSR_0,
  },
  {
      .state = SENSOR_STATE_INVALID,
      .serializer_alias = MSM_DES_0_ALIAS_ADDR_CAM_SER_1,
      .sensor_alias = MSM_DES_0_ALIAS_ADDR_CAM_SNSR_1,
  },
};

static struct camera_i2c_reg_array max9296_mode_split_receiver[] = CAM_DES_INIT_SPLITTER_MODE_RECEIVER;
static struct camera_i2c_reg_array max9296_mode_split_sender[] = CAM_DES_INIT_SPLITTER_MODE_SENDER;
#ifndef DEBUG_SINGLE_SENSOR
static struct camera_i2c_reg_array max9296_mode_evkit_sender[] = CAM_DES_INIT_EVKIT_MODE_SENDER;
static struct camera_i2c_reg_array max9296_mode_video_recorder[] = CAM_DES_INIT_RECORDER_MODE;
#endif
static struct camera_i2c_reg_array max9296_mode_link_receiver[] = CAM_DES_INIT_LINK_MODE_RECEIVER;
static struct camera_i2c_reg_array max9296_start_reg_array[] = CAM_DES_START;
static struct camera_i2c_reg_array max9296_stop_reg_array[] = CAM_DES_STOP;
static struct camera_i2c_reg_array max9296_reg_array_disable_reverse[] = CAM_DES_DISABLE_I2C_REVERSE;
#ifdef MAX9296_ENABLE_INTR_HANDLER
static struct camera_i2c_reg_array max9296_intr_enable_array[] = CAM_DES_INTR_INIT;
static struct camera_i2c_reg_array max9296_intr_disable_array[] = CAM_DES_INTR_DEINIT;
static struct camera_i2c_reg_array max9296_intr_read_clear_array[] = CAM_DES_INTR_READ_CLEAR;
#endif

/**
 * UTILITY MACROS
 */

/*Adds I2C command to i2c reg array*/
#define ADD_I2C_REG_ARRAY(_a_, _size_, _addr_, _val_, _delay_) \
do { \
    _a_[_size_].reg_addr = _addr_; \
    _a_[_size_].reg_data = _val_; \
    _a_[_size_].delay = _delay_; \
    _size_++; \
} while(0)


/**
 * FUNCTIONS
 */
static int max9296_set_default(max9296_context_t * pCtxt, const sensor_open_lib_t* device_info)
{
    pCtxt->max9296_config = default_config;

    if (MAXIM_DESER_ID_DEFAULT != device_info->config->type)
    {
        int i;

        pCtxt->max9296_config.boardType = device_info->boardType;
        pCtxt->max9296_config.opMode = device_info->config->opMode;
        pCtxt->max9296_config.powersave_mode = device_info->config->powerSaveMode;
        pCtxt->max9296_config.num_of_cameras = STD_MIN(device_info->config->numSensors, MAXIM_LINK_MAX);
        for(i = 0; i < pCtxt->max9296_config.num_of_cameras; i++)
        {
            pCtxt->max9296_config.sensors[i].id = device_info->config->sensors[i].type;
            pCtxt->max9296_config.sensors[i].mode = device_info->config->sensors[i].snsrModeId;
            pCtxt->max9296_config.sensors[i].color_space = device_info->config->sensors[i].colorSpace;
            pCtxt->max9296_config.sensors[i].fsync_mode = device_info->config->sensors[i].fsyncMode;
            pCtxt->max9296_config.sensors[i].fsync_freq = device_info->config->sensors[i].fsyncFreq;
            if (MAXIM_SENSOR_ID_MAX9295 == pCtxt->max9296_config.sensors[i].id)
            {
                SENSOR_WARN("Sensor id detected as MAXIM_SENSOR_ID_MAX9295 id : %d. All the other sensor ids should also be set to same mode.", pCtxt->max9296_config.sensors[i].mode);
            }
        }
    }

    memcpy(&pCtxt->max9296_sensors, max9296_sensors_init_table, sizeof(pCtxt->max9296_sensors));

    /*default to dev id 0*/
    switch (pCtxt->subdev_id)
    {
    case 1:
    case 3:
        pCtxt->slave_addr = MSM_DES_1_SLAVEADDR;
        pCtxt->max9296_sensors[0].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_0;
        pCtxt->max9296_sensors[1].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_1;
        pCtxt->max9296_sensors[0].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_0;
        pCtxt->max9296_sensors[1].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_1;
        break;
    default:
        pCtxt->slave_addr = MSM_DES_0_SLAVEADDR;
        break;
    }

    return 0;
}

/**
 * FUNCTION: sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
static void* max9296_sensor_open_lib(void* ctrl, void* arg)
{
    max9296_context_t* pCtxt;

    SHIGH("max9296_sensor_open_lib()");

    if (!ctrl || !arg)
    {
        SERR("invalid arguments");
        return NULL;
    }

    pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(max9296_context_t));

    if (pCtxt)
    {
        CameraResult rc;
        unsigned int i = 0;
        sensor_open_lib_t* device_info = (sensor_open_lib_t*)arg;

        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->subdev_id = device_info->config->subdevId;

        rc = CameraCreateMutex(&pCtxt->mutex);
        if (rc)
        {
            CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt);
            return NULL;
        }

        memcpy(&pCtxt->sensor_lib, &sensor_lib_ptr, sizeof(pCtxt->sensor_lib));

        pCtxt->ctrl = ctrl;
        pCtxt->sensor_lib.sensor_slave_info.camera_id = device_info->cameraId;
        pCtxt->state = MAX9296_STATE_INVALID;

        pCtxt->max9296_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->max9296_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pCtxt->io_expander_reg_setting.addr_type = CAMERA_I2C_BYTE_ADDR;
        pCtxt->io_expander_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        max9296_set_default(pCtxt, device_info);

        SHIGH("board_type = %d, powersave_mode = %d",
                pCtxt->max9296_config.boardType, pCtxt->max9296_config.powersave_mode);

        pCtxt->num_supported_sensors = STD_MIN(pCtxt->max9296_config.num_of_cameras, MAXIM_LINK_MAX);
        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            pCtxt->max9296_sensors[i].state = SENSOR_STATE_INVALID;
            pCtxt->max9296_sensors[i].signal_status = INTR_SIGNAL_STATUS_NONE;
            //snsrModeId from camera_config
            pCtxt->max9296_sensors[i].mode = device_info->config->sensors[i].snsrModeId;
            pCtxt->max9296_sensors[i].color_space = device_info->config->sensors[i].colorSpace;
        }

        pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id = pCtxt->slave_addr;
    }

    return pCtxt;
}

/**
 * FUNCTION: max9296_sensor_close_lib
 *
 * DESCRIPTION: Closes sensor library
 **/
static int max9296_sensor_close_lib(void* ctxt)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int i;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    for (i = 0; i < MAXIM_LINK_MAX; i++)
    {
        if (pCtxt->max9296_sensors[i].pPrivCtxt)
            CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt->max9296_sensors[i].pPrivCtxt);
    }

    CameraDestroyMutex(pCtxt->mutex);
    CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt);

    return 0;
}

#ifndef DEBUG_SINGLE_SENSOR
static unsigned int max9296_get_bpp_from_dt(uint32 dt)
{
    switch(dt)
    {
    case 0:
        return 8;
    case CSI_DT_RAW8:
        return 8;
    case CSI_DT_RAW10:
        return 10;
    case CSI_DT_RAW12:
        return 12;
    case CSI_DT_RAW14:
        return 14;
    case CSI_DT_RAW16:
        return 16;
    case CSI_DT_RAW20:
        return 20;
    case CSI_DT_RGB888:
        return 24;
    default:
        SERR("get bpp for dt 0x%x not implemented", dt);
        return 8;
    }
}
#endif

/**
 * FUNCTION: max9296_set_init_sequence
 *
 * DESCRIPTION: max9296_set_init_sequence
 **/
static int max9296_set_init_sequence(max9296_context_t* pCtxt)
{
    int rc = 0;

    SENSOR_HIGH("max9296_set_init_sequence()");

    CameraLockMutex(pCtxt->mutex);
    if (pCtxt->max9296_config.opMode != MAXIM_OP_MODE_RECEIVER)
    {
        uint32 i = 0;
        if (pCtxt->num_supported_sensors > 1)
        {
            struct camera_i2c_reg_array write_regs[] = { {0x0010, 0x20, _max9296_delay_} };

            //Dual mode to program both cameras
            pCtxt->max9296_reg_setting.reg_array = write_regs;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);
            pCtxt->max9296_reg_setting.reg_array[0].reg_data |= pCtxt->sensor_lib.src_id_enable_mask;

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
            if (rc)
            {
                SERR("Unable to set deserailzer 0x%x I2C Mode 0x%x. Fatal error!",
                        pCtxt->slave_addr, pCtxt->sensor_lib.src_id_enable_mask);
                CameraUnlockMutex(pCtxt->mutex);
                return rc;
            }
            CameraUnlockMutex(pCtxt->mutex);

            CameraMicroSleep(MAX9296_SELECT_LINK_DELAY);

            CameraLockMutex(pCtxt->mutex);
            SENSOR_WARN("Setting MAX9296 I2C to 0x%x", pCtxt->sensor_lib.src_id_enable_mask);
        }

        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[i];
            if (pCtxt->sensor_lib.src_id_enable_mask & (1 << i))
            {
                rc = pSensor->sensor->init_link(pCtxt, i);
            }
        }
    }
    else //MAXIM_OP_MODE_RECEIVER
    {
        //Set them to intialize as data is coming from another mizar and not actual cameras here
        if (2 == pCtxt->max9296_config.num_of_cameras)
        {
            pCtxt->max9296_reg_setting.reg_array = max9296_mode_split_receiver;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_mode_split_receiver);
            pCtxt->max9296_sensors[0].state = SENSOR_STATE_INITIALIZED;
            pCtxt->max9296_sensors[1].state = SENSOR_STATE_INITIALIZED;

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                           pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
            if (rc)
            {
                SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
            }
        }
        else
        {
            pCtxt->max9296_reg_setting.reg_array = max9296_mode_link_receiver;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_mode_link_receiver);
            pCtxt->max9296_sensors[0].state = SENSOR_STATE_INITIALIZED;

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                           pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
            if (rc)
            {
                SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
            }
        }
    }

#ifdef DEBUG_SINGLE_SENSOR
    pCtxt->max9296_reg_setting.reg_array = max9296_mode_split_sender;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_mode_split_sender);
    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                   pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
    if (rc)
    {
        SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
    }
#else
    {
        if (MAXIM_OP_MODE_VIDEO_RECORDER == pCtxt->max9296_config.opMode)
        {
            pCtxt->max9296_reg_setting.reg_array = max9296_mode_video_recorder;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_mode_video_recorder);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
               pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
            if (rc)
            {
                SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
            }
        }
        else
        {
            unsigned char val = 0;
            unsigned int seq_size = 0;
            unsigned int i = 0;
            boolean isVCX_Enabled = FALSE;

            if (MAXIM_SENSOR_ID_EVKIT == pCtxt->max9296_config.sensors->id)
            {
                memcpy(&pCtxt->init_array[seq_size],
                    max9296_mode_evkit_sender,
                    sizeof(max9296_mode_evkit_sender));

                seq_size += STD_ARRAY_SIZE(max9296_mode_evkit_sender);
            }
            else
            {
                /* default to mode split sender */
                memcpy(&pCtxt->init_array[seq_size],
                    max9296_mode_split_sender,
                    sizeof(max9296_mode_split_sender));

                seq_size += STD_ARRAY_SIZE(max9296_mode_split_sender);

                /***********************
                 * PIPE OVERRIDE CONFIGURATION
                 * ---------------------------
                 * The pipe override configuration is required for parallel input where we need to
                 *  assign VC/DT to the incoming data. We assign the pipe configuration based on what
                 *  we query from the link. Then we enable this configuration using pCtxt->pipelines[X].pipe_override
                 ***********************/
                if (pCtxt->pipe_override)
                {
                    val = 0x2 | (max9296_get_bpp_from_dt(pCtxt->pipelines[0].dst.dt) << 3);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0313, val, _max9296_delay_);

                    val = pCtxt->pipelines[0].dst.dt | ((pCtxt->pipelines[1].dst.dt & 0x30) << 2);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0316, val, _max9296_delay_);

                    val = (pCtxt->pipelines[1].dst.dt & 0x0F) | ((pCtxt->pipelines[2].dst.dt & 0x3C) << 2);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0317, val, _max9296_delay_);

                    val = (pCtxt->pipelines[2].dst.dt & 0x03) | (pCtxt->pipelines[3].dst.dt << 2);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0318, val, _max9296_delay_);

                    val = max9296_get_bpp_from_dt(pCtxt->pipelines[1].dst.dt) |
                            ((max9296_get_bpp_from_dt(pCtxt->pipelines[2].dst.dt) & 0x1C) << 3);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0319, val, _max9296_delay_);

                    val = (max9296_get_bpp_from_dt(pCtxt->pipelines[2].dst.dt) & 0x3) |
                            (max9296_get_bpp_from_dt(pCtxt->pipelines[3].dst.dt) << 2);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x031A, val, _max9296_delay_);
                }

                // 1200 Mbps/lane on port A and B
                // Set pipeline overrides as needed
                val = 0x2C |
                        (pCtxt->pipelines[0].pipe_override << 6) |
                        (pCtxt->pipelines[1].pipe_override << 7);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x031D, val, _max9296_delay_);
                // Override BPP, DT, VC on pipes 2 and 3 if required
                val = 0x2C |
                        (pCtxt->pipelines[2].pipe_override << 6) |
                        (pCtxt->pipelines[3].pipe_override << 7);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0320, val, _max9296_delay_);

                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0332, 0xF0, _max9296_delay_);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0100, 0x23, _max9296_delay_);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0112, 0x23, _max9296_delay_);

                /**********************
                * Do the VC/DT remapping here.
                * Each pipe requires 3 SRC->DST mappings for Video, FS and FE
                * All mappings destination CSI2 controller 0
                ***********************/
                for (i = 0; i < MAXIM_PIPELINE_MAX; i++)
                {
                    max9296_pipe_config_t* p_pipe = &pCtxt->pipelines[i];

                    if (!(pCtxt->sensor_lib.src_id_enable_mask & (1 << i)))
                    {
                        continue;
                    }

                    unsigned int mipi_tx_offset = 0x400 + 0x40*i;
                    unsigned int mipi_tx_ext_offset = 0x500 + 0x10*i;
                    unsigned char src_vc = p_pipe->src.vc << 6;
                    unsigned char src_vc_ext = (p_pipe->src.vc & 0xFC) >> 2;
                    unsigned char vc = (p_pipe->dst.vc & 0x3) << 6;
                    unsigned char vc_ext = (p_pipe->dst.vc & 0xFC) >> 2;
                    unsigned char dt = p_pipe->dst.dt;

                    //3 mappings to CSI0
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0B, 0x07, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x2D, 0x15, _max9296_delay_);
                    // MAP0|1|2 - VIDEO | FS | FE
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0D, src_vc | dt, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0E, vc | dt, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0F, src_vc | 0x00, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x10, vc, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x11, src_vc | 0x01, _max9296_delay_);
                    ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x12, vc | 0x1, _max9296_delay_);

                    //If VCX needed, enable VCX and apply for all 3 mappings
                    if (src_vc_ext || vc_ext)
                    {
                        val = (src_vc_ext << 5) | (vc_ext << 2);
                        if (!isVCX_Enabled)
                        {
                            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x44A, 0xC8, _max9296_delay_);
                            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x48A, 0xC8, _max9296_delay_);
                            isVCX_Enabled = TRUE;
                        }
                        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x00, val, _max9296_delay_);
                        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x01, val, _max9296_delay_);
                        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x02, val, _max9296_delay_);
                    }
                }
            }

            //print out init sequence for debugging
            for (i = 0; i < seq_size; i++)
            {
                SLOW("%d - 0x%x, 0x%x, %d", i,
                    pCtxt->init_array[i].reg_addr, pCtxt->init_array[i].reg_data, pCtxt->init_array[i].delay);
            }

            pCtxt->max9296_reg_setting.reg_array = pCtxt->init_array;
            pCtxt->max9296_reg_setting.size = seq_size;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                       pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
            if (rc)
            {
                SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
            }
        }
    }
#endif /*DEBUG_SINGLE_SENSOR*/

    CameraUnlockMutex(pCtxt->mutex);
    if (!rc)
    {
        pCtxt->state = MAX9296_STATE_INITIALIZED;
    }

    return rc;
}

static int max9296_sensor_enter_sleep(max9296_context_t* pCtxt)
{
    int32 rc = 0;

    SHIGH("enter");

    CameraLockMutex(pCtxt->mutex);
    struct camera_i2c_reg_array write_regs[] = {
                                                 {0x0010, 0x43, _max9296_delay_}, // reset_link = 1;
                                                 {0x000c, 0x0a, _max9296_delay_}, // wake up disable
                                                 {0x0010, 0x4b, _max9296_delay_}}; // sleep_mode = 1;
    pCtxt->max9296_reg_setting.reg_array = write_regs;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);

    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting);
    if (rc)
    {
        SERR("Sleep failed, Unable to disable 0x%x I2C.", pCtxt->slave_addr);
        goto EXIT;
    }

EXIT:
    CameraUnlockMutex(pCtxt->mutex);
    return rc;
}

static int max9296_sensor_wake_up(max9296_context_t* pCtxt)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    int32 i;
    int32 rc = 0;

    CameraLockMutex(pCtxt->mutex);
    //do dummy transaction
    for (i = 0; i < 3; i++)
    {
        pCtxt->max9296_reg_setting.reg_array = dummy_reg;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
        pCtxt->max9296_reg_setting.reg_array[0].reg_addr = 0x10;
        if ((pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                        &pCtxt->max9296_reg_setting)))
        {
            SENSOR_WARN("Dummy transfer to read from the MAX9296 De-serialzer 0x%x (0x%08x)",
                    pCtxt->slave_addr, pCtxt->sensor_lib.sensor_slave_info.camera_id);
        }
        else
        {
            break;
        }
        CameraSleep(5);
    }

    struct camera_i2c_reg_array write_regs[] = {{0x0010, 0x43, _max9296_delay_}, // reset_link = 0
                                                {0x0010, 3, _max9296_delay_},    // sleep_mode = 0
                                                {0x000c, 0x3a, _max9296_delay_}};// wake up enable
    pCtxt->max9296_reg_setting.reg_array = write_regs;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to i2c_slave_write_array 0x%x I2C.", pCtxt->slave_addr);
        goto EXIT;
    }

    // check local lock state
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_CTRL3_REG_ADDR;
    for (i = 0; i< 50; i++)
    {
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                        &pCtxt->max9296_reg_setting)))
        {
            SERR("Failed to read de-serializer(0x%x) revision", pCtxt->slave_addr);
            goto EXIT;
        }
        if (pCtxt->max9296_reg_setting.reg_array[0].reg_data & 0x08)
        {
            SHIGH("break, success read serializer(0x%x), count=0x%x, reg[0x%x]= 0x%x", pCtxt->slave_addr, i,
                    pCtxt->max9296_reg_setting.reg_array[0].reg_addr, pCtxt->max9296_reg_setting.reg_array[0].reg_data);
            break;
        }
        CameraSleep(5);
    }

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    return rc;
}

#if 0
static void io_expander_log(max9296_context_t* pCtxt)
{
    int rc;
    unsigned int slave_addr = IO_EXPANDER_ADDR;
    int array_size = STD_ARRAY_SIZE(io_expander_array);

    for (int i = 0; i < array_size; i++)
    {
        io_expander_reg_setting.reg_array = &io_expander_array[i];
        io_expander_reg_setting.size = 1;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, slave_addr,
                        &io_expander_reg_setting)))
        {
            SERR("Unable to read from the io_expander slave 0x%x reg 0x%x",
                    slave_addr, io_expander_reg_setting.reg_array[0].reg_addr);
            continue;
        }
        SHIGH("io_expander_reg_setting - 0x%x, 0x%x",
                io_expander_reg_setting.reg_array[0].reg_addr,
                io_expander_reg_setting.reg_array[0].reg_data)
    }
}
#endif

static void io_expander_detect(max9296_context_t* pCtxt)
{
    unsigned int slave_addr = IO_EXPANDER_ADDR;
    int rc;
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    uint8 regDirVal;

    pCtxt->is_gpio_expander_poweron = FALSE;
    pCtxt->is_gpio_expander_used = FALSE;

    CameraLockMutex(pCtxt->mutex);
    pCtxt->io_expander_reg_setting.reg_array = dummy_reg;
    pCtxt->io_expander_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->io_expander_reg_setting.reg_array[0].reg_addr = 0x0f;
    rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, slave_addr, &pCtxt->io_expander_reg_setting);
    CameraUnlockMutex(pCtxt->mutex);
    if (rc)
    {
        SWARN("Unable to read io_expander 0x%x.", slave_addr);
        return;
    }

    SHIGH("io_expander_reg_setting - 0x%x, 0x%x",
                    pCtxt->io_expander_reg_setting.reg_array[0].reg_addr,
                    pCtxt->io_expander_reg_setting.reg_array[0].reg_data)

    regDirVal = pCtxt->io_expander_reg_setting.reg_array[0].reg_data;

    /**
     * check the initial power status for every max9296 connected sensors rather than the total(0XFF)
     * when only subdev 0/2 is power on(0xf3), this can avoid the unecessary power on and delay
     */
    if (pCtxt->subdev_id == 0 || pCtxt->subdev_id == 2)
    {
        pCtxt->is_gpio_expander_poweron = (regDirVal & 0x03) ? TRUE : FALSE;
    }
    else
    {
        pCtxt->is_gpio_expander_poweron = (regDirVal & 0x0C) ? TRUE : FALSE;
    }
    pCtxt->is_gpio_expander_used = TRUE;
}

static int io_expander_reset(max9296_context_t* pCtxt)
{
    unsigned int slave_addr = IO_EXPANDER_ADDR;
    struct camera_i2c_reg_array io_expander_array[] = IO_EXPANDER_READ;
    int rc;

    /* Writing consecutively 0x12 and 0x34 to RegReset register will reset all
       registers to their default values */
    struct camera_i2c_reg_array write_regs[] = {
        {0x7d, 0x12, _max9296_delay_},
        {0x7d, 0x34, _max9296_delay_},
    };

    CameraLockMutex(pCtxt->mutex);
    pCtxt->io_expander_reg_setting.reg_array = write_regs;
    pCtxt->io_expander_reg_setting.size = STD_ARRAY_SIZE(write_regs);
    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, slave_addr, &pCtxt->io_expander_reg_setting);
    if (rc)
    {
        SERR("Unable to reset io_expander 0x%x on boardType %d", slave_addr, pCtxt->max9296_config.boardType);
        goto EXIT;
    }

    int array_size = STD_ARRAY_SIZE(io_expander_array);
    for (int i = 0; i < array_size; i++)
    {
        pCtxt->io_expander_reg_setting.reg_array = &io_expander_array[i];
        pCtxt->io_expander_reg_setting.size = 1;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, slave_addr,
                        &pCtxt->io_expander_reg_setting)))
        {
            SERR("Unable to read from the io_expander slave 0x%x reg 0x%x",
                    slave_addr, pCtxt->io_expander_reg_setting.reg_array[0].reg_addr);
            continue;
        }
        SHIGH("io_expander_reg_setting - 0x%x, 0x%x",
                pCtxt->io_expander_reg_setting.reg_array[0].reg_addr,
                pCtxt->io_expander_reg_setting.reg_array[0].reg_data)
    }

EXIT:
    CameraUnlockMutex(pCtxt->mutex);
    return rc;
}

static int io_expander_control_pwr(max9296_context_t* pCtxt, uint8 enable)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    uint8 regDirVal;
    int rc;

    unsigned int slave_addr = IO_EXPANDER_ADDR;

    CameraLockMutex(pCtxt->mutex);
    pCtxt->io_expander_reg_setting.reg_array = dummy_reg;
    pCtxt->io_expander_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->io_expander_reg_setting.reg_array[0].reg_addr = 0x0f;
    rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, slave_addr, &pCtxt->io_expander_reg_setting);
    CameraUnlockMutex(pCtxt->mutex);
    if (rc)
    {
        SERR("Unable to read io_expander 0x%x. Fatal error!", slave_addr);
        return rc;
    }

    SHIGH("io_expander_reg_setting - 0x%x, 0x%x",
            pCtxt->io_expander_reg_setting.reg_array[0].reg_addr,
            pCtxt->io_expander_reg_setting.reg_array[0].reg_data)

    regDirVal = pCtxt->io_expander_reg_setting.reg_array[0].reg_data;
    if (enable)
    {
        if (pCtxt->subdev_id == 0 || pCtxt->subdev_id == 2)
        {
            regDirVal |= 0xf3;
        }
        else
        {
            regDirVal |= 0xfc;
        }
    }
    else
    {
        if (pCtxt->subdev_id == 0 || pCtxt->subdev_id == 2)
        {
            regDirVal &= 0xfc;
        }
        else
        {
            regDirVal &= 0xf3;
        }
    }

    CameraLockMutex(pCtxt->mutex);
    pCtxt->io_expander_reg_setting.reg_array[0].reg_data = regDirVal;
    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, slave_addr, &pCtxt->io_expander_reg_setting);
    CameraUnlockMutex(pCtxt->mutex);
    if (rc)
    {
        SERR("Unable to set io_expander 0x%x. Fatal error!", slave_addr);
        return rc;
    }

    return rc;
}

static void max9296_sensor_delay(max9296_context_t* pCtxt)
{
#ifdef MAX9296_DETECT_SEQUENTIAL
    // Allow sensors to fully power up before trying to detect it

    // Only needed for first subdev_id since all sensors will be powered on by this time
    if (pCtxt->subdev_id == 0)
    {
        SHIGH("250ms delay for sensor power up");
        CameraSleep(250);
    }
#else
    (void)pCtxt;

    SHIGH("250ms delay for sensor power up");
    CameraSleep(250);
#endif
}

static int max9296_sensor_power_suspend(void* ctxt, CameraPowerEventType powerEventId)
{
    int rc = 0;
    uint32 i = 0;

    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (CAMERA_POWERSAVE_MODE_NONE == pCtxt->max9296_config.powersave_mode)
    {
        SHIGH("skipping as powersave_mode_none");
        return rc;
    }

    if (INTR_SIGNAL_STATUS_LOST == pCtxt->max9296_sensors[MAXIM_LINK_A].signal_status
            || INTR_SIGNAL_STATUS_LOST == pCtxt->max9296_sensors[MAXIM_LINK_B].signal_status)
    {
        SHIGH("detected intr signal lost, no need to power suspend");
        return rc;
    }

#ifdef MAX9296_ENABLE_INTR_HANDLER
    if (pCtxt->num_connected_sensors)
    {
        CameraLockMutex(pCtxt->mutex);
        pCtxt->max9296_reg_setting.reg_array = max9296_intr_disable_array;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_intr_disable_array);

        SLOW("disable max9296 intr");

        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                        pCtxt->ctrl,
                        pCtxt->slave_addr,
                        &pCtxt->max9296_reg_setting)))
        {
            SERR("Failed to disable max9296_intr_disable_array (0x%x)", pCtxt->slave_addr);
        }

        CameraUnlockMutex(pCtxt->mutex);
    }
#endif

    //Power down sensors through io expander only if it is used
    if (pCtxt->is_gpio_expander_used && CAMERA_POWERSAVE_MODE_BRIDGECHIP_OFF != pCtxt->max9296_config.powersave_mode)
    {
        rc |= io_expander_control_pwr(pCtxt, 0);
        if (rc)
        {
            SERR("Power down sensors through GPIO expander failed");
            return rc;
        }
    }

    if (CAMERA_POWERSAVE_MODE_SLEEP == pCtxt->max9296_config.powersave_mode)
    {
        rc = max9296_sensor_enter_sleep(pCtxt);
        if (rc)
        {
            SERR("Enter sleep mode failed");
        }
    }
    else if (CAMERA_POWERSAVE_MODE_POWEROFF == pCtxt->max9296_config.powersave_mode || CAMERA_POWERSAVE_MODE_BRIDGECHIP_OFF == pCtxt->max9296_config.powersave_mode)
    {
        rc = pCtxt->platform_fcn_tbl.execute_power_setting(pCtxt->ctrl,
                pCtxt->sensor_lib.sensor_slave_info.power_setting_array.power_down_setting_a,
                pCtxt->sensor_lib.sensor_slave_info.power_setting_array.size_down,
                powerEventId);
        if (rc)
        {
            SERR("Poweroff failed");
        }
    }

    if (!rc)
    {
        if (pCtxt->state == MAX9296_STATE_INITIALIZED)
        {
            pCtxt->state = MAX9296_STATE_SUSPEND;
            for (i = 0; i < pCtxt->num_supported_sensors; i++)
            {
                max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[i];
                SHIGH("sensor state %d", pSensor->state);
                if (pSensor->state == SENSOR_STATE_INITIALIZED)
                {
                    pSensor->state = SENSOR_STATE_DETECTED;
                }
                pSensor->signal_status = INTR_SIGNAL_STATUS_NONE;
            }
        }
    }

    return rc;
}

static int max9296_sensor_power_resume(void* ctxt, CameraPowerEventType powerEventId)
{
    int rc = 0;
    uint32 i = 0;

    SHIGH("enter 0x%p", ctxt);
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    //Power Resume only the bridge chips with sensors connected
    if ((CAMERA_POWERSAVE_MODE_NONE == pCtxt->max9296_config.powersave_mode)
            || !pCtxt->num_connected_sensors)
    {
        SHIGH("skipping as no sensor need to resume");
        return rc;
    }

    if (powerEventId == CAMERA_POWER_POST_HIBERNATION || powerEventId == CAMERA_POWER_POST_LPM)
    {
        SHIGH("Post power event %d", powerEventId);

        powerEventId = (powerEventId == CAMERA_POWER_POST_LPM) ? CAMERA_POWER_UP : CAMERA_POWER_RESUME;
        for (i = 0; i < pCtxt->num_supported_sensors && pCtxt->max9296_sensors[i].state == SENSOR_STATE_INITIALIZED; i++)
        {
            // Change the pSensor state to SENSOR_STATE_DETECTED to enable I2C forwarding
            pCtxt->max9296_sensors[i].state = SENSOR_STATE_DETECTED;
        }
    }

    if (INTR_SIGNAL_STATUS_VALID == pCtxt->max9296_sensors[MAXIM_LINK_A].signal_status
            || INTR_SIGNAL_STATUS_VALID == pCtxt->max9296_sensors[MAXIM_LINK_B].signal_status)
    {
        SHIGH("detected intr signal valid, already power on");
        return rc;
    }

    // wake up
    if (CAMERA_POWERSAVE_MODE_SLEEP == pCtxt->max9296_config.powersave_mode)
    {
        rc = max9296_sensor_wake_up(pCtxt);
    }
    else // POWERSAVE_MODE_POWEROFF mode
    {
        rc = pCtxt->platform_fcn_tbl.execute_power_setting(pCtxt->ctrl,
                pCtxt->sensor_lib.sensor_slave_info.power_setting_array.power_up_setting_a,
                pCtxt->sensor_lib.sensor_slave_info.power_setting_array.size_up,
                powerEventId);
    }

    // for V3 daughter card, need to power on the remote and delay when SLEEP & POWEROFF mode
    if (pCtxt->is_gpio_expander_used && CAMERA_POWERSAVE_MODE_BRIDGECHIP_OFF != pCtxt->max9296_config.powersave_mode)
    {
        rc |=io_expander_control_pwr(pCtxt, 1);
        if (rc)
        {
            SERR("Power up sensors through io expander failed");
            return rc;
        }
        max9296_sensor_delay(pCtxt);
    }
    else if (CAMERA_POWERSAVE_MODE_POWEROFF == pCtxt->max9296_config.powersave_mode || CAMERA_POWERSAVE_MODE_BRIDGECHIP_OFF == pCtxt->max9296_config.powersave_mode)
    { // for V2 daughter card, need to power on the total, need delay when POWEROFF mode
        max9296_sensor_delay(pCtxt);
    }

    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[i];

        if (pSensor->state != SENSOR_STATE_DETECTED)
        {
            SHIGH("No need to detect deserailzer 0x%x I2C Mode Link %d.", pCtxt->slave_addr, i);
            continue;
        }

        // Enable I2C forwarding and select link
        struct camera_i2c_reg_array write_regs[] = {
            {0x0001, 0x02, _max9296_delay_},
            {0x0010, 0x20 | (1 << i), _max9296_delay_}
        };
        CameraLockMutex(pCtxt->mutex);
        pCtxt->max9296_reg_setting.reg_array = write_regs;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);

        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting)))
        {
            CameraUnlockMutex(pCtxt->mutex);
            SERR("Unable to set deserailzer 0x%x I2C Mode Link %d. Fatal error!", pCtxt->slave_addr, i);
            return rc;
        }
        CameraUnlockMutex(pCtxt->mutex);
        CameraMicroSleep(MAX9296_SELECT_LINK_DELAY);

        CameraLockMutex(pCtxt->mutex);
        rc = pSensor->sensor->detect(pCtxt, i);
        CameraUnlockMutex(pCtxt->mutex);
    }

    rc = max9296_sensor_init_setting(pCtxt);

#ifdef MAX9296_ENABLE_INTR_HANDLER
    CameraLockMutex(pCtxt->mutex);
    pCtxt->max9296_reg_setting.reg_array = max9296_intr_enable_array;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_intr_enable_array);

    SLOW("max9296_init_enable_array");

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to start max9296_init_intr_array (0x%x)", pCtxt->slave_addr);
    }
    CameraUnlockMutex(pCtxt->mutex);
#endif

    pCtxt->state = MAX9296_STATE_INITIALIZED;

    return rc;
}

static void max9296_gmsl2_check_lock_state(max9296_context_t* pCtxt, uint32 tryTimes)
{
    int rc = 0;
    uint32 timeout = 0;
    uint32 max_check_times = tryTimes;
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};

    while (timeout++ < max_check_times)
    {
        pCtxt->max9296_reg_setting.reg_array = dummy_reg;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
        pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_CTRL3_REG_ADDR;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                &pCtxt->max9296_reg_setting)))
        {
            // in locking state, De-serializer maybe access failed
            SERR("Unable to read from the MAX9296 De-serializer 0x%x (0x%08x)",
                pCtxt->slave_addr, pCtxt->sensor_lib.sensor_slave_info.camera_id);
        }
        if (pCtxt->max9296_reg_setting.reg_array[0].reg_data & 0x08)
        {
            break;
        }
        CameraMicroSleep(5000);
    }
    SHIGH("subdev_id=%d, count=%d, reg[0x%x]= 0x%x", pCtxt->subdev_id, timeout-1,
        pCtxt->max9296_reg_setting.reg_array[0].reg_addr, pCtxt->max9296_reg_setting.reg_array[0].reg_data);
}

static int max9296_sensor_remap_channels(max9296_context_t* pCtxt)
{
    int rc = 0;
    uint32 i = 0;

    if (pCtxt->state != MAX9296_STATE_DETECTED)
    {
        SHIGH("max9296 0x%x not be detected - wrong state", pCtxt->slave_addr);
        return CAMERA_EBADSTATE;
    }

    max9296_gmsl2_check_lock_state(pCtxt, 2);

    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[i];

        pSensor->mode = pCtxt->max9296_config.sensors[i].mode;

        switch (pCtxt->max9296_config.sensors[i].id)
        {
        case MAXIM_SENSOR_ID_AR0231:
            pSensor->sensor = ar0231_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0231_EXT_ISP:
            pSensor->sensor = ar0231_ext_isp_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0820:
            pSensor->sensor = ar0820_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0234_EXT_FPGA:
            pSensor->sensor = &ar0234_ext_fpga_info;
            break;
        case MAXIM_SENSOR_ID_MAX9295:
            pSensor->sensor = ser9295_get_info();
            break;
        case MAXIM_SENSOR_ID_MAX9295_LOOPBACK:
            pSensor->sensor = max9295_loopback_get_info();
            break;
        case MAXIM_SENSOR_ID_EVKIT:
            pSensor->sensor = evkit_get_sensor_info();
            break;
        default:
            SERR("Slave ID %d NOT SUPPORTED", pCtxt->max9296_config.sensors[i].id);
            pSensor->state = SENSOR_STATE_UNSUPPORTED;
            break;
        }

        if (!pSensor->sensor)
        {
            continue;
        }

        if (pCtxt->num_supported_sensors > 1)
        {
            // Enable I2C forwarding and select link
            struct camera_i2c_reg_array write_regs[] = {
                {0x0001, 0x02, _max9296_delay_},
                {0x0010, 0x20 | (1 << i), _max9296_delay_}
            };
            pCtxt->max9296_reg_setting.reg_array = write_regs;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);

            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting)))
            {
                SERR("Unable to set deserailzer 0x%x I2C Mode Link %d. Fatal error!", pCtxt->slave_addr, i);
                return CAMERA_EBADSTATE;
            }

            max9296_gmsl2_check_lock_state(pCtxt, 20);
        }
        else
        {
            max9296_gmsl2_check_lock_state(pCtxt, 2);
        }

        rc = pSensor->sensor->detect(pCtxt, i);
        if (!rc)
        {
            pSensor->state = SENSOR_STATE_DETECTED;

            pCtxt->num_connected_sensors++;
        }
    }

    return 0;
}

static int max9296_sensor_detect_device(void* ctxt)
{
    int rc = 0;
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    SHIGH("detect max9296 0x%x", pCtxt->slave_addr);

    if (pCtxt->state >= MAX9296_STATE_DETECTED)
    {
        SHIGH("already detected");
        return 0;
    }

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        SERR("I2C function table not initialized");
        return CAMERA_EFAILED;
    }

    //check if io expander present and mis-configured boardType
    io_expander_detect(pCtxt);
    if (pCtxt->is_gpio_expander_used)
    {
        SHIGH("Detected GPIO Expander is_poweron %d", pCtxt->is_gpio_expander_poweron);
        if (!pCtxt->is_gpio_expander_poweron)
        { // if the gpio expander is not power on, reset it to power on all the four sensors
            rc = io_expander_reset(pCtxt);
            if (rc)
            {
                SERR("Failed to reset GPIO Expander. Fatal Error!");
                return rc;
            }
        }
    }
    else if (pCtxt->max9296_config.boardType == CAMERA_HW_BOARD_ADPAIR_V3_PM731
                || pCtxt->max9296_config.boardType == CAMERA_HW_BOARD_ADPSTAR_V2_PK901)
    {
          SERR("Failed to detect GPIO Expander on board type %d", pCtxt->max9296_config.boardType)
          return CAMERA_ENOTYPE;
    }

    //Detect MAX9296
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_CHIP_ID_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to read from the MAX9296 De-serialzer 0x%x (0x%08x)",
            pCtxt->slave_addr, pCtxt->sensor_lib.sensor_slave_info.camera_id);
        return rc;
    }

    if (MAXIM_OP_MODE_RECEIVER == pCtxt->max9296_config.opMode)
    {
        pCtxt->max9296_reg_setting.reg_array = max9296_reg_array_disable_reverse;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_reg_array_disable_reverse);

        // Disable any reverse channel for  I2C commands in case another board is connected to this board
        // Since the I2C addresses are common, this causes corruption in each other's programming
        // If this was a sender board then we don't want to disbale as we want to be able send I2C to cameras (so remove then)
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                  pCtxt->ctrl,
                  pCtxt->slave_addr,
                  &pCtxt->max9296_reg_setting)))
        {
            SERR("Failed to disable reverse channel on deserializer(0x%x)", pCtxt->slave_addr);
            return rc;
        }
    }

    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_REVISION_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read( pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to read deserializer(0x%x) revision", pCtxt->slave_addr);
    }
    else
    {
        //Check if its an ES4 or later chip
        if (pCtxt->max9296_reg_setting.reg_array[0].reg_data != MSM_DES_REVISION_ES2)
        {
            pCtxt->revision = 4;
        }
        else
        {
            pCtxt->revision = 2;
        }
        SENSOR_WARN("MAX9296 revision %d detected!", pCtxt->revision);

        /*Only if detecting them sequentially do we need to disable i2c to other devices to allow faster remapping*/
#ifdef MAX9296_DETECT_SEQUENTIAL
        // Disable I2C forwarding to allow other devices on the same I2C bus remap their sensors
        struct camera_i2c_reg_array write_regs[] = { {0x0001, 0x12, _max9296_delay_} };
        pCtxt->max9296_reg_setting.reg_array = write_regs;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);

        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max9296_reg_setting)))
        {
            SERR("Unable to disable 0x%x I2C.", pCtxt->slave_addr);
            return rc;
        }
#endif /*MAX9296_DETECT_SEQUENTIAL*/
        pCtxt->state = MAX9296_STATE_DETECTED;
    }

    return rc;
}

#ifdef MAX9296_ENABLE_INTR_HANDLER
static int max9296_intr_read_global_error(max9296_context_t* pCtxt, bool* err_flag)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    int rc = 0;

    *err_flag = 0;

    CameraLockMutex(pCtxt->mutex);
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_CTRL3_REG_ADDR;

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                    &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to read from the MAX9296 slave 0x%x reg 0x%x",
                pCtxt->slave_addr,pCtxt->max9296_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }

    *err_flag = (pCtxt->max9296_reg_setting.reg_array[0].reg_data & 0x04) == 0 ? false : true;

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    return rc;
}


static int max9296_intr_clear_cnt(max9296_context_t* pCtxt)
{
    int array_size = STD_ARRAY_SIZE(max9296_intr_read_clear_array);
    int rc = 0;

    CameraLockMutex(pCtxt->mutex);

    /* Read one register at a time for now
     * @Todo: use a burst read API to read all at once.
     */
    for (int i = 0; i < array_size; i++)
    {
        pCtxt->max9296_reg_setting.reg_array = &max9296_intr_read_clear_array[i];
        pCtxt->max9296_reg_setting.size = 1;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                        &pCtxt->max9296_reg_setting)))
        {
            SERR("Unable to read from the MAX9296 slave 0x%x reg 0x%x",
                    pCtxt->slave_addr,pCtxt->max9296_reg_setting.reg_array[0].reg_addr);
            goto EXIT;
        }
    }

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    return rc;
}

static int max9296_intr_read_status(max9296_context_t* pCtxt, unsigned short* intr_link_src)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    unsigned short intr3_val = 0, intr5_val = 0, intr7_val = 0;
    int rc = 0;

    *intr_link_src = 0;

    CameraLockMutex(pCtxt->mutex);
    /* DEC_ERR */
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR3_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to read from the MAX9296 slave 0x%x reg 0x%x",
            pCtxt->slave_addr, pCtxt->max9296_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr3_val = pCtxt->max9296_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_DEC_ERR_MASK;

    /* EOM_ERR */
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR5_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to read from the MAX9296 slave 0x%x reg 0x%x",
            pCtxt->slave_addr,pCtxt->max9296_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr5_val = pCtxt->max9296_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_EOM_ERR_MASK;

    /* CRC_ERR */
    pCtxt->max9296_reg_setting.reg_array = dummy_reg;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max9296_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR7_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max9296_reg_setting)))
    {
        SERR("Unable to read from the MAX9296 slave 0x%x reg 0x%x",
            pCtxt->slave_addr,pCtxt->max9296_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr7_val = pCtxt->max9296_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_CRC_ERR_MASK;

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    SERR("intr3_val=0x%02x, intr5_val=0x%02x, intr7_val=0x%02x",
            intr3_val, intr5_val, intr7_val);

    if ((intr3_val & 0x01) || (intr5_val & (1 << 6))) {
        //notify Link A signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_A;
    }
    if ((intr3_val & 0x02) || (intr5_val & (1 << 7))) {
        //notify Link B signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_B;
    }

    return rc;
}

static void max9296_intr_handler(void* data)
{
    max9296_context_t* pCtxt = (max9296_context_t*)data;
    unsigned short link_src = 0;
    bool err_flag = false;
    CameraInputEventPayloadType payload = {};
    int rc = 0;

    if(pCtxt == NULL)
    {
        SERR("Invalid max9296 context");
        return;
    }

    /* read ctrl3 status, reflect reverse value of ERRB pin */
    rc = max9296_intr_read_global_error(pCtxt, &err_flag);

    /* get which camera got signal error */
    rc |= max9296_intr_read_status(pCtxt, &link_src);
    SHIGH("INTR enter 0x%x, err_flag 0x%x, link_src 0x%x",
            pCtxt->slave_addr, err_flag, link_src);

    if (pCtxt->state != MAX9296_STATE_STREAMING)
    {
        SHIGH("max9296 context state %d not in streaming", pCtxt->state);
        return;
    }

    if (rc)
    {
        SERR("read reverse_pin or intr_status error rc = %d", rc);
        return;
    }

    while(link_src || err_flag)
    {
        /* send signal lost event */
        for (uint32 i = MAXIM_LINK_A; i < pCtxt->num_supported_sensors; i++)
        {
            if ((link_src & (1 << i))
                    && ((pCtxt->max9296_sensors[i].signal_status == INTR_SIGNAL_STATUS_NONE)
                        || (pCtxt->max9296_sensors[i].signal_status == INTR_SIGNAL_STATUS_VALID)))
            {
                payload.src_id = i;
                payload.lock_status = QCARCAM_INPUT_SIGNAL_LOST;
                pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);

                pCtxt->max9296_sensors[i].signal_status = INTR_SIGNAL_STATUS_LOST;
                SHIGH("notify signal lost");
            }
        }
        CameraSleep(MAX9296_INTR_SETTLE_SLEEP);

        rc = max9296_intr_clear_cnt(pCtxt);
        rc |= max9296_intr_read_status(pCtxt, &link_src);
        rc |= max9296_intr_read_global_error(pCtxt, &err_flag);

        if (rc)
        {
            SERR("loop read register faid, rc = %d", rc);
            break;
        }
        SHIGH("err_flag 0x%x, link_src 0x%x", err_flag, link_src);

        if (!err_flag)
        {
            max9296_intr_read_status(pCtxt, &link_src);
            SHIGH("second read link_src 0x%x", link_src);
            for (uint32 i = MAXIM_LINK_A; i < pCtxt->num_supported_sensors; i++)
            {
                if (!(link_src & (1 << i)) && (pCtxt->max9296_sensors[i].signal_status == INTR_SIGNAL_STATUS_LOST))
                {
                    payload.src_id = i;
                    payload.lock_status = QCARCAM_INPUT_SIGNAL_VALID;
                    pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);

                    pCtxt->max9296_sensors[i].signal_status = INTR_SIGNAL_STATUS_VALID;
                    SHIGH("notify signal valid");
                }
            }
        }
    }

    return;
}
#endif /*MAX9296_ENABLE_INTR_HANDLER*/

static int max9296_sensor_detect_device_channels(void* ctxt)
{
    int rc = 0;
    uint32 link = 0;

    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    SHIGH("initialize max9296 0x%x with %d sensors", pCtxt->slave_addr, pCtxt->num_supported_sensors);
    if(pCtxt->subdev_id == 0)
    {
       ais_log_kpi(AIS_EVENT_KPI_DETECT_SENSORS_START);
    }

    if (pCtxt->state >= MAX9296_STATE_INITIALIZED)
    {
        SERR("already detected %d out of %d", pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
        rc = 0;
        goto EXIT;
    }
    else if (pCtxt->state != MAX9296_STATE_DETECTED)
    {
        SERR("MAX9296 0x%x not detected - wrong state", pCtxt->slave_addr);
        rc = CAMERA_EBADSTATE;
        goto EXIT;
    }

    if (pCtxt->max9296_config.opMode == MAXIM_OP_MODE_RECEIVER)
    {
        //If receiver board no need to check for cameras just use what the use has indicated in the ini file
        pCtxt->num_connected_sensors = pCtxt->max9296_config.num_of_cameras;
        SENSOR_WARN("Receiver side forcing camera to %d", pCtxt->num_connected_sensors);
    }

    if (pCtxt->is_gpio_expander_used)
    { // for daughter card V3, if it is not power on, need to delay
        if (!pCtxt->is_gpio_expander_poweron)
        {
            max9296_sensor_delay(pCtxt);
        }
    }
    else
    { // for daughter card V2, must delay
        max9296_sensor_delay(pCtxt);
    }

    if (0 != (rc = max9296_sensor_remap_channels(pCtxt)))
    {
        SERR("Unable to remap deserailzer 0x%x", pCtxt->slave_addr);
        rc = CAMERA_EBADSTATE;
        goto EXIT;
    }

    for (link = 0; link < MAXIM_LINK_MAX; link++)
    {
        max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];

        if (pSensor->state != SENSOR_STATE_DETECTED)
        {
            continue;
        }

        rc = pSensor->sensor->get_link_cfg(pCtxt, link, &pSensor->link_cfg);

        if (!rc)
        {
            int pipe = 0;

            if (pSensor->link_cfg.num_pipelines > MAX_LINK_PIPELINES)
            {
                SERR("Only support max of %d pipelines per link", MAX_LINK_PIPELINES);
                pSensor->link_cfg.num_pipelines = MAX_LINK_PIPELINES;
            }

            /* LINK TO SRC_ID MAPPING TABLE
             * Keep Link 1 pipe 0 as src_id 1 for backward compatibility
             * where src_id 1 refers to LINK 1
             *
                LINK    pipeline  stream_id |   PIPE  src_id
                ---------------------------------------------
                 0         0          0     |    X       0
                 0         1          2     |    Z       2
                 1         0          1     |    Y       1
                 1         1          3     |    U       3
             */
            for (pipe = 0; pipe < pSensor->link_cfg.num_pipelines; pipe++)
            {
                max9296_pipe_config_t* p_pipe;
                int stream_id = link + 2*pipe;

                //Add new channel and subchannel for the stream
                img_src_channel_t *pChannel = &pCtxt->sensor_lib.channels[pCtxt->sensor_lib.num_channels];
                img_src_subchannel_t *pSubChannel = &pCtxt->sensor_lib.subchannels[pCtxt->sensor_lib.num_subchannels];
                img_src_subchannel_layout_t layout = {
                    .src_id = stream_id,
                };

                p_pipe = &pCtxt->pipelines[stream_id];
                p_pipe->link = link;
                p_pipe->input_id = pSensor->link_cfg.pipelines[pipe].id;
                p_pipe->link_type = pSensor->link_cfg.link_type;
                p_pipe->src = pSensor->link_cfg.pipelines[pipe].mode.channel_info;

                //Map each pipe to unique VC
                p_pipe->dst.vc = stream_id;
                p_pipe->dst.dt = p_pipe->src.dt;
                p_pipe->dst.cid = stream_id*4;

                if (p_pipe->link_type == MAXIM_LINK_TYPE_PARALLEL)
                {
                    //set pipe override for parallel inputs
                    p_pipe->pipe_override = 1;
                    pCtxt->pipe_override |= 1 << stream_id;
                }

                pChannel->num_subchannels = 1;
                pChannel->output_mode = pSensor->link_cfg.pipelines[pipe].mode;
                pChannel->output_mode.channel_info = p_pipe->dst;
                pChannel->subchan_layout[0] = layout;

                pSubChannel->src_id = stream_id;
                pSubChannel->modes[0] = pSensor->link_cfg.pipelines[pipe].mode;
                pSubChannel->modes[0].channel_info = p_pipe->dst;
                pSubChannel->num_modes = 1;

                pSensor->link_cfg.pipelines[pipe].stream_id = stream_id;

                pCtxt->sensor_lib.num_channels++;
                pCtxt->sensor_lib.num_subchannels++;

                //update enable_mask to indicate stream is available
                pCtxt->sensor_lib.src_id_enable_mask |= (1 << stream_id);
            }
        }
    }

#ifdef MAX9296_ENABLE_INTR_HANDLER
    /* INTR init */
    if (pCtxt->sensor_lib.num_channels)
    {
        rc = pCtxt->platform_fcn_tbl.setup_gpio_interrupt(pCtxt->ctrl,
                CAMERA_GPIO_INTR, max9296_intr_handler, pCtxt);
        if (!rc)
        {
            CameraLockMutex(pCtxt->mutex);
            pCtxt->max9296_reg_setting.reg_array = max9296_intr_enable_array;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_intr_enable_array);

            SLOW("setup_gpio_interrupt and max9296_intr_enable_array for (0x%x)", pCtxt->slave_addr);

            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
            {
                SERR("Failed to start max9296_init_intr_array (0x%x)", pCtxt->slave_addr);
            }
            CameraUnlockMutex(pCtxt->mutex);

            pCtxt->max9296_sensors[MAXIM_LINK_A].signal_status = INTR_SIGNAL_STATUS_NONE;
            pCtxt->max9296_sensors[MAXIM_LINK_B].signal_status = INTR_SIGNAL_STATUS_NONE;
        }
    }
#endif /*MAX9296_ENABLE_INTR_HANDLER*/

EXIT:
    if (!pCtxt->num_connected_sensors)
    {
        if (pCtxt->is_gpio_expander_used)
        {  /**
            * when no sensor is detected, set the is_gpio_expander_used = false
            * to avoid the power off again on powersuspend when !Granular mode
            */
            pCtxt->is_gpio_expander_used = FALSE;
            rc = io_expander_control_pwr(pCtxt, 0);
            if (rc)
            {
                 SERR("Failed to power off io expander");
            }
        }
    }

    if(pCtxt->subdev_id == 0)
    {
       ais_log_kpi(AIS_EVENT_KPI_DETECT_SENSORS_END);
    }

    CameraLogEvent(CAMERA_SENSOR_EVENT_PROBED, 0, 0);

    return rc;
}


static int max9296_fsync_init(max9296_context_t* pCtxt)
{
    int rc = 0;

    SLOW("Initialize frame sync for (0x%x)\n", pCtxt->slave_addr);

    CameraLockMutex(pCtxt->mutex);

    struct camera_i2c_reg_array write_regs[] = {{ 0x3EF, 0xC0, _max9296_delay_ }};
    pCtxt->max9296_reg_setting.reg_array = write_regs;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to set external clock mode (0x%x)", rc);
    }


    struct camera_i2c_reg_array write_regs_overlap[] = {{ 0x3E2, 0x00, _max9296_delay_ },
                                                        { 0x3EA, 0x00, _max9296_delay_ },
                                                        { 0x3EB, 0x00, _max9296_delay_ }};
    pCtxt->max9296_reg_setting.reg_array = write_regs_overlap;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs_overlap);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to set overlap window (0x%d)", rc);
    }

    /* clk_cycles = number of clock cycles to count before generating every fsync signal
	   for eg. 15 fps signal clk_cycles = 25000000/15 = 1666666 = 0x196E6A
       Min fps = 10, Max fps = 60 */
    unsigned int clk_cycles = STD_MIN((MAX9296_MIN_FPS_FSYNC + 5*pCtxt->max9296_config.sensors[0].fsync_freq), MAX9296_MAX_FPS_FSYNC);
    clk_cycles = MAX9296_CLK_25MHZ / clk_cycles;
    unsigned char lbyte = clk_cycles&0xff;
    unsigned char mbyte = (clk_cycles&0xff00) >> 8;
    unsigned char hbyte = (clk_cycles&0xff0000) >> 16;
    struct camera_i2c_reg_array write_regs_clk_cycles[] = {{ 0x3E5, lbyte, _max9296_delay_ },
                                                           { 0x3E6, mbyte, _max9296_delay_ },
                                                           { 0x3E7, hbyte, _max9296_delay_ }};
    pCtxt->max9296_reg_setting.reg_array = write_regs_clk_cycles;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs_clk_cycles);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to set clock counter for fsync generation  (0x%d)", rc);
    }

    struct camera_i2c_reg_array write_regs_enable_mfp[] = {{0x2D3, 0x84, _max9296_delay_ }, {0x2D5, 0x7, _max9296_delay_}};
    pCtxt->max9296_reg_setting.reg_array = write_regs_enable_mfp;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_regs_enable_mfp);
    for(int i = 0; i < pCtxt->max9296_config.num_of_cameras; i++)
    {
        if (MAX9296_FSYNC_MODE_DISABLED != pCtxt->max9296_config.sensors[i].fsync_mode)
        {
            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->max9296_sensors[i].serializer_alias,
                            &pCtxt->max9296_reg_setting)))
            {
                SERR("Failed to set MFP7 on serializer 0x%x (0x%d)", pCtxt->max9296_sensors[i].serializer_alias, rc);
            }
        }
    }

    struct camera_i2c_reg_array write_gpioid_tx[] = {{ 0x3F1, 0x38, _max9296_delay_ }};
    pCtxt->max9296_reg_setting.reg_array = write_gpioid_tx;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_gpioid_tx);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to set MFP on serializer(0x%d)", rc);
    }

    struct camera_i2c_reg_array write_fsync_on[] = {{ 0x3E0, 0x04, _max9296_delay_ }};
    pCtxt->max9296_reg_setting.reg_array = write_fsync_on;
    pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(write_fsync_on);

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max9296_reg_setting)))
    {
        SERR("Failed to enable fsync generation on de-serializer (0x%d)", rc);
    }

    CameraUnlockMutex(pCtxt->mutex);
    return rc;
}

static int max9296_sensor_init_setting(void* ctxt)
{
    int rc = 0;

    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (!pCtxt->num_connected_sensors)
    {
        SHIGH("skipping as no sensors detected");
        return 0;
    }
    else
    {
        CameraLogEvent(CAMERA_SENSOR_EVENT_INITIALIZE_START, 0, 0);
        if(pCtxt->subdev_id == 0)
        {
           ais_log_kpi(AIS_EVENT_KPI_SENSOR_PROG_START);
        }

        rc = max9296_set_init_sequence(pCtxt);

        if(pCtxt->subdev_id == 0)
        {
           ais_log_kpi(AIS_EVENT_KPI_SENSOR_PROG_END);
        }
        CameraLogEvent(CAMERA_SENSOR_EVENT_INITIALIZE_DONE, 0, 0);
    }

    return rc;
}

static int max9296_sensor_set_channel_mode(void* ctxt, unsigned int src_id, unsigned int mode)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    SHIGH("max9296_sensor_set_channel_mode()");

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if(pCtxt->max9296_sensors[src_id].state == SENSOR_STATE_INITIALIZED)
    {
        if (src_id < pCtxt->num_supported_sensors)
        {
            if (pCtxt->max9296_sensors[src_id].sensor &&
                pCtxt->max9296_sensors[src_id].sensor->apply_sensormode)
            {
                rc = pCtxt->max9296_sensors[src_id].sensor->apply_sensormode(pCtxt, src_id, mode);
            }
            else
            {
                SERR("Unsupported set sensormode for %d", src_id);
                rc = CAMERA_EUNSUPPORTED;
            }
        }
        else
        {
            SERR("invalid src_id %d", src_id);
            rc = CAMERA_EBADITEM;
        }
    }
    return rc;
}

static int max9296_sensor_set_cci_sync_param(void* ctxt, void* param)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    struct camera_i2c_sync_cfg* p_cci_sync_param = (struct camera_i2c_sync_cfg*)param;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    pCtxt->i2c_sync_settings.cid = p_cci_sync_param->cid;
    pCtxt->i2c_sync_settings.csid = p_cci_sync_param->csid;

    pCtxt->i2c_sync_settings.line = pCtxt->cci_sync_line_no;
    pCtxt->i2c_sync_settings.delay = pCtxt->cci_sync_delay;

    SHIGH("cid %d csid %d line %d delay %d",pCtxt->i2c_sync_settings.cid,
           pCtxt->i2c_sync_settings.csid,
           pCtxt->i2c_sync_settings.line,
           pCtxt->i2c_sync_settings.delay);

    return 0;
}


static int max9296_sensor_start_stream(void* ctxt, unsigned int src_id_mask)
{
    int rc = 0;
    unsigned int i = 0;
    unsigned int started_mask = 0;
    bool enablefsync = FALSE;

    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    SHIGH("max9296_sensor_start_stream(), src_id_mask 0x %x", src_id_mask);

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    //Now start the cameras
    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        if ((1 << i) & src_id_mask)
        {
            if (SENSOR_STATE_INITIALIZED == pCtxt->max9296_sensors[i].state)
            {
                SENSOR_WARN("starting slave %x", pCtxt->max9296_sensors[i].serializer_alias);
                {
                    CameraLockMutex(pCtxt->mutex);

                    //Only write to camera if sender or video recorder
                    if (MAXIM_OP_MODE_DEFAULT == pCtxt->max9296_config.opMode || MAXIM_OP_MODE_VIDEO_RECORDER == pCtxt->max9296_config.opMode)
                    {
                        pCtxt->max9296_sensors[i].sensor->start_link(pCtxt, i);
                    }

                   CameraUnlockMutex(pCtxt->mutex);
                }

                pCtxt->max9296_sensors[i].state = SENSOR_STATE_STREAMING;
                started_mask |= (1 << i);

                if (MAX9296_FSYNC_MODE_DISABLED != pCtxt->max9296_config.sensors[i].fsync_mode)
                {
                    enablefsync = TRUE;
                }
            }
            else
            {
                /*TODO: change this to SERR once we limit which slaves to start*/
                SHIGH("sensor 0x%x not ready to start (state=%d) - bad state",
                        pCtxt->max9296_sensors[i].serializer_alias, pCtxt->max9296_sensors[i].state);
            }
        }
    }


    if (!rc &&
        MAX9296_STATE_INITIALIZED == pCtxt->state &&
        started_mask)
    {
#ifdef MAX9296_ENABLE_INTR_HANDLER
        unsigned short link_src;
        bool err_flag = false;
        unsigned int retry_cnt = 0;
        max9296_intr_read_global_error(pCtxt, &err_flag);
        while (retry_cnt < MAX9296_MAX_ERROR_CHECK_CNT)
        {
            max9296_intr_clear_cnt(pCtxt);
            max9296_intr_read_status(pCtxt, &link_src);
            max9296_intr_read_global_error(pCtxt, &err_flag);
            if (!err_flag)
                break;

            retry_cnt++;

            CameraSleep(10);
        }

        if (retry_cnt == MAX9296_MAX_ERROR_CHECK_CNT)
        {
            for (i = 0; i < pCtxt->num_supported_sensors; i++)
            {
                if ((1 << i) & started_mask)
                {
                    pCtxt->max9296_sensors[i].state = SENSOR_STATE_INITIALIZED;
                }
            }
            SERR("Shouldn't start de-serializer(0x%x) with error", pCtxt->slave_addr);
            return CAMERA_EFAILED;
        }
#endif /*MAX9296_ENABLE_INTR_HANDLER*/

        CameraLockMutex(pCtxt->mutex);
        //@todo: dynamically only start needed streams
        pCtxt->max9296_reg_setting.reg_array = max9296_start_reg_array;
        pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_start_reg_array);

        SHIGH("starting deserializer");
        //Start the deserialzer
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->max9296_reg_setting)))
        {
            SERR("Failed to start de-serializer(0x%x)", pCtxt->slave_addr);
            SERR("stop link"); //start deserialize failed, then stop link
            for (i = 0; i < pCtxt->num_supported_sensors; i++)
            {
                if ((1 << i) & src_id_mask)
                {
                    SHIGH("stopping slave %x", pCtxt->max9296_sensors[i].serializer_alias);
                    pCtxt->max9296_sensors[i].sensor->stop_link(pCtxt, i);
                    pCtxt->max9296_sensors[i].state = SENSOR_STATE_INITIALIZED;
                }
            }
        }
        else
        {
            pCtxt->state = MAX9296_STATE_STREAMING;
        }
        CameraUnlockMutex(pCtxt->mutex);
    }

    if (!rc)
    {
        pCtxt->streaming_src_mask |= started_mask;
    }

    if(enablefsync && pCtxt)
    {
        if(max9296_fsync_init(pCtxt))
        {
            SWARN("max9296 fsync failed");
        }
    }

    CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_START, 0, 0);
    SHIGH("max9296(0x%x) streaming... src_mask(0x%x) started_mask(0x%x)", pCtxt->slave_addr, pCtxt->streaming_src_mask, started_mask);

    return rc;
}

static int max9296_sensor_stop_stream(void* ctxt, unsigned int src_id_mask)
{
    int rc = 0;
    unsigned int i;
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    SHIGH("max9296_sensor_stop_stream()");

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    /*stop transmitter first if no more clients*/
    if (!rc && MAX9296_STATE_STREAMING == pCtxt->state)
    {
        pCtxt->streaming_src_mask &= (~src_id_mask);
        SHIGH("stopping max9296 0x%x transmitter (%x)", pCtxt->slave_addr, pCtxt->streaming_src_mask);

        /*stop if no slaves streaming*/
        if (!pCtxt->streaming_src_mask)
        {
            CameraLockMutex(pCtxt->mutex);
            pCtxt->max9296_reg_setting.reg_array = max9296_stop_reg_array;
            pCtxt->max9296_reg_setting.size = STD_ARRAY_SIZE(max9296_stop_reg_array);
            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->max9296_reg_setting)))
            {
                SERR("Failed to stop de-serializer(0x%x)", pCtxt->slave_addr);
            }
            CameraUnlockMutex(pCtxt->mutex);
            pCtxt->state = MAX9296_STATE_INITIALIZED;
        }
    }

    /*then stop slaves*/
    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        if ((1 << i) & src_id_mask)
        {
            SHIGH("stopping slave %x", pCtxt->max9296_sensors[i].serializer_alias);

            CameraLockMutex(pCtxt->mutex);

            rc = pCtxt->max9296_sensors[i].sensor->stop_link(pCtxt, i);

            CameraUnlockMutex(pCtxt->mutex);
            if (rc)
            {
                /*TODO: change this to SERR once we limit which slaves to stop*/
                SHIGH("sensor 0x%x not in state to stop (state=%d) - bad state",
                        pCtxt->max9296_sensors[i].sensor_alias, pCtxt->max9296_sensors[i].state);
            }
        }
    }

    /* TODO: cleanup in case of failure */

    CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_STOP, 0, 0);
    SHIGH("max9296(0x%x) stopped", pCtxt->slave_addr);

    return rc;
}

static int max9296_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        if (!table || !table->i2c_slave_write_array || !table->i2c_slave_read)
        {
            SERR("Invalid i2c func table param");
            return CAMERA_EBADPARM;
        }

        pCtxt->platform_fcn_tbl = *table;
        pCtxt->platform_fcn_tbl_initialized = 1;
        SLOW("i2c func table set");
    }

    return 0;
}

static int max9296_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max9296_sensors[src_id].sensor &&
            pCtxt->max9296_sensors[src_id].sensor->calculate_exposure)
        {
            rc = pCtxt->max9296_sensors[src_id].sensor->calculate_exposure(pCtxt, src_id, exposure_info);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}


static int max9296_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max9296_sensors[src_id].sensor &&
            pCtxt->max9296_sensors[src_id].sensor->apply_exposure)
        {
            rc = pCtxt->max9296_sensors[src_id].sensor->apply_exposure(pCtxt, src_id, exposure_info);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}

static int max9296_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max9296_sensors[src_id].sensor &&
            pCtxt->max9296_sensors[src_id].sensor->apply_hdr_exposure)
        {
            rc = pCtxt->max9296_sensors[src_id].sensor->apply_hdr_exposure(pCtxt, src_id, hdr_exposure);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}

static int max9296_sensor_gamma_config(void* ctxt, unsigned int src_id, qcarcam_gamma_config_t* gamma)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max9296_sensors[src_id].sensor &&
            pCtxt->max9296_sensors[src_id].sensor->apply_gamma)
        {
            rc = pCtxt->max9296_sensors[src_id].sensor->apply_gamma(pCtxt, src_id, gamma);
        }
        else
        {
            SERR("Unsupported gamma config for %d", src_id);
            rc = CAMERA_EBADITEM;
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}

static void max9296_test_i2c_bulk_write(void* ctxt, unsigned int *data)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;

    unsigned int *write_regs = (data + (I2C_BURST_LENGTH_IDX+1));

    struct camera_i2c_bulk_reg_setting  max9296_bulk_reg = {};
    max9296_bulk_reg.addr_type = CAMERA_I2C_WORD_ADDR;
    max9296_bulk_reg.data_type = CAMERA_I2C_BYTE_DATA;

    vendor_ext_property_t *p_prop = (vendor_ext_property_t*)&(data[I2C_BURST_ADDR_IDX]);
    max9296_bulk_reg.reg_addr = p_prop->value.uint_val;
    max9296_bulk_reg.reg_data = write_regs;
    max9296_bulk_reg.size = data[I2C_BURST_LENGTH_IDX];
    for (int i=0; i< max9296_bulk_reg.size; i++)
    {
        SLOW(" Data to write [0x%x] = 0x%x", i, write_regs[i]);
    }

    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_bulk(pCtxt->ctrl, pCtxt->slave_addr, &max9296_bulk_reg, TRUE);
    if (rc != CAMERA_SUCCESS) {
        SERR("Write Bulk failed with ret = %d", rc);
    }
    else {
        SHIGH("Write bulk success to slave id 0x%0x", pCtxt->slave_addr);
    }

}

static int max9296_sensor_set_vendor_param(void* ctxt, unsigned int src_id, qcarcam_vendor_param_t* param)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    CameraInputEventPayloadType payload = {};
    int rc = 0;
    CAM_UNUSED(src_id);

    vendor_ext_property_t *p_prop = (vendor_ext_property_t*)&(param->data[0]);
    if (p_prop->type == VENDOR_EXT_PROP_TEST)
    {
        SHIGH("data[0]= %u data[2]= %u", p_prop->value.uint_val, param->data[2]);
    }
    else if (p_prop->type == VENDOR_EXT_PROP_TEST_I2C_BULK)
    {
        max9296_test_i2c_bulk_write(ctxt, param->data);
    }

    payload.src_id = src_id;
    payload.vendor_data = *param;
    pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_VENDOR, &payload);

    return rc;
}


static int max9296_sensor_s_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt");
        return CAMERA_EBADPARM;
    }

    if (param == NULL)
    {
        SERR("Invalid params");
        return CAMERA_EBADPARM;
    }

    switch(id)
    {
        case QCARCAM_PARAM_GAMMA:
            rc = max9296_sensor_gamma_config(ctxt, src_id, (qcarcam_gamma_config_t*)param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = max9296_sensor_set_vendor_param(ctxt, src_id, (qcarcam_vendor_param_t*)param);
            break;
        default:
            SERR("Param not supported = %d", id);
            rc = CAMERA_EBADPARM;
            break;
    }
    return rc;
}

static void max9296_test_i2c_bulk_read(void* ctxt, unsigned int *data)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    struct camera_i2c_bulk_reg_setting  max9296_bulk_reg = {};
    unsigned int *dummy_reg = (data + (I2C_BURST_LENGTH_IDX+1));
    int32 i;
    int rc = 0;

    max9296_bulk_reg.addr_type = CAMERA_I2C_WORD_ADDR;
    max9296_bulk_reg.data_type = CAMERA_I2C_BYTE_DATA;

    CameraLockMutex(pCtxt->mutex);
    vendor_ext_property_t *p_prop = (vendor_ext_property_t*)&(data[I2C_BURST_ADDR_IDX]);
    max9296_bulk_reg.reg_addr = p_prop->value.uint_val;
    max9296_bulk_reg.reg_data = dummy_reg;
    max9296_bulk_reg.size = data[I2C_BURST_LENGTH_IDX];
    rc = pCtxt->platform_fcn_tbl.i2c_slave_read_bulk(pCtxt->ctrl, pCtxt->slave_addr,
                     &max9296_bulk_reg, TRUE);
    CameraUnlockMutex(pCtxt->mutex);

    if (rc != CAMERA_SUCCESS)
    {
        SERR("Read Bulk failed with ret = %d", rc);
    }
    else
    {
        for (i=0; i< max9296_bulk_reg.size; i++)
        {
            SLOW(" Read data [0x%x] = 0x%x", (max9296_bulk_reg.reg_addr + i), max9296_bulk_reg.reg_data[i]);
        }
    }

    return;
}

static int max9296_sensor_get_vendor_param(void* ctxt, unsigned int src_id, qcarcam_vendor_param_t* param)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    CAM_UNUSED(src_id);

    vendor_ext_property_t *p_prop = (vendor_ext_property_t*)&(param->data[0]);
    if (p_prop->type == VENDOR_EXT_PROP_TEST)
    {
        SHIGH("data[0] = %u", p_prop->value.uint_val);
        param->data[2] = pCtxt->revision;
    }
    else if (p_prop->type == VENDOR_EXT_PROP_TEST_I2C_BULK)
    {
        max9296_test_i2c_bulk_read(ctxt, param->data);
    }
    else if (p_prop->type == VENDOR_EXT_PROP_FRAME_META_DATA)
    {
        //MetaData Size in no of bytes, max upto (QCARCAM_MAX_VENDOR_PAYLOAD_SIZE - 1) * sizeof(param->data[0])
        unsigned int metaDataSize = (QCARCAM_MAX_VENDOR_PAYLOAD_SIZE - 1) * sizeof(param->data[0]);

        //First element of vendor param should be metadatasize 
        param->data[0] = metaDataSize;

        //Filling Dummy Meta Data
        uint8_t* param_data = (uint8_t *)&param->data[1];
        for (uint8_t i=0; i<metaDataSize; i++) {
            param_data[i] = i;
        }
        SLOW("Meta Data Filled at Sensor Lib size %d", metaDataSize);

    }

    return rc;
}

static int max9296_sensor_get_channel_mode(void* ctxt, unsigned int src_id, uint32* param)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    SHIGH("max9296_sensor_get_channel_mode()");

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        *param = pCtxt->max9296_sensors[src_id].mode;
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}

static int max9296_sensor_get_color_space(void* ctxt, unsigned int src_id, uint32* param)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    SHIGH("max9296_sensor_get_color_space()");

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return CAMERA_EBADPARM;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        *param = pCtxt->max9296_sensors[src_id].color_space;
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = CAMERA_EBADITEM;
    }

    return rc;
}
static int max9296_sensor_g_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt");
        return CAMERA_EBADPARM;
    }

    if (param == NULL)
    {
        SERR("Invalid params");
        return CAMERA_EBADPARM;
    }

    switch(id)
    {
        case QCARCAM_PARAM_GAMMA:
            rc = CAMERA_EUNSUPPORTED;
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = max9296_sensor_get_vendor_param(ctxt, src_id, (qcarcam_vendor_param_t*)param);
            break;
        case QCARCAM_PARAM_INPUT_MODE:
            rc = max9296_sensor_get_channel_mode(ctxt, src_id, (uint32*)param);
            break;
        case QCARCAM_PARAM_INPUT_COLOR_SPACE:
            rc = max9296_sensor_get_color_space(ctxt, src_id, (uint32*)param);
            break;
        default:
            SERR("Param not supported = %d", id);
            rc = CAMERA_EBADPARM;
            break;
    }
    return rc;

}

#ifdef MAX9296_ENABLE_FRAME_FREEZE
static int max9296_sensor_process_frame_data(void* ctxt, CameraInputProcessFrameDataType* p_frame)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    CameraInputEventPayloadType payload = {};
    uint32* pbuf = (uint32*)p_frame->pBuffer->pVa;

    SERR("Frame seq no = %d", p_frame->pFrameInfo->seq_no[0]);
    pbuf[0] = p_frame->pFrameInfo->seq_no[0];

    /* Frame Freeze detection logic */
    if (!(p_frame->pFrameInfo->seq_no[0] % 50)) {
        ais_log_kpi(AIS_EVENT_KPI_DETECT_FRAME_FREEZE);
        payload.src_id = p_frame->srcId;
        payload.stream_hndl = p_frame->hStream;
        payload.frame_freeze.seq_no =  p_frame->pFrameInfo->seq_no[0];
        payload.frame_freeze.state = 1; //freeze detected
        SERR("==== Frame Freeze Detected ====");
        pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_FRAME_FREEZE, &payload);
    }
    return 0;
}
#endif

/**
 * FUNCTION: CameraSensorDevice_Open_max9296
 *
 * DESCRIPTION: Entry function for device driver framework
 **/
CAM_API CameraResult CameraSensorDevice_Open_max9296(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId)
{
    sensor_lib_interface_t sensor_lib_interface = {
            .sensor_open_lib = max9296_sensor_open_lib,
    };

    return CameraSensorDevice_Open(ppNewHandle, deviceId, &sensor_lib_interface);
}


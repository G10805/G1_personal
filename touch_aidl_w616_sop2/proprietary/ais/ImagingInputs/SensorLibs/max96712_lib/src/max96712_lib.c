/**
 * @file max96712_lib.c
 * Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
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
#include "max96712_lib.h"


#define MAX96712_INTR_SETTLE_SLEEP 100 //100ms for now
#define MAX96712_MAX_ERROR_CHECK_CNT 10

/*
 * EXTERNAL
 */
max96712_sensor_t* ar0820_ext_isp_gw5300_get_sensor_info(void);
max96712_sensor_t* max96712_pattern_gen_get_sensor_info(void);
max96712_sensor_t* ar0231_ext_isp_get_sensor_info(void);
max96712_sensor_t* s5k1h1sx_get_sensor_info(void);
max96712_sensor_t* ar0234_ext_fpga_get_sensor_info(void);
max96712_sensor_t* ar0231_get_sensor_info(void);

/*
 * INTERNAL
 */
static int max96712_sensor_close_lib(void* ctxt);
static int max96712_sensor_power_suspend(void* ctxt, CameraPowerEventType powerEventId);
static int max96712_sensor_power_resume(void* ctxt, CameraPowerEventType powerEventId);
static int max96712_sensor_detect_device(void* ctxt);
static int max96712_sensor_detect_device_channels(void* ctxt);
static int max96712_sensor_init_setting(void* ctxt);
static int max96712_sensor_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode);
static int max96712_sensor_start_stream(void* ctxt, unsigned int src_id_mask);
static int max96712_sensor_stop_stream(void* ctxt, unsigned int src_id_mask);
static int max96712_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table);
static void max96712_gmsl2_check_lock_state(max96712_context_t* pCtxt, uint32 link, uint32 tryTimes);
static int max96712_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int max96712_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int max96712_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int max96712_sensor_gamma_config(void* ctxt, unsigned int src_id, qcarcam_gamma_config_t* gamma);
static int max96712_sensor_s_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param);
static int max96712_sensor_g_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param);
#ifdef MAX96712_ENABLE_INTR_HANDLER
static int max96712_intr_read_global_error(max96712_context_t* pCtxt, bool* err_flag);
static int max96712_intr_clear_cnt(max96712_context_t* pCtxt);
static int max96712_intr_read_status(max96712_context_t* pCtxt, unsigned short* intr_link_src);
static void max96712_intr_handler(void* data);
#endif

/*
 * Describes the Sensor Library
 */
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
            .config_val = 0,
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
        .lane_cnt = 4, /*4 lane mode is selected */
        .settle_cnt = 0xE,
        .lane_mask = 0x1F,
        .combo_mode = 0,
        .is_csi_3phase = 0,
        .vcx_mode = 1 /*enable VCX extension*/
      }
    },
    .sensor_close_lib = max96712_sensor_close_lib,
    /*custom functions that were defined in the driver */
    .sensor_custom_func =
    {
      .sensor_set_platform_func_table = &max96712_sensor_set_platform_func_table,
      .sensor_power_suspend = &max96712_sensor_power_suspend,
      .sensor_power_resume = &max96712_sensor_power_resume,
      .sensor_detect_device = &max96712_sensor_detect_device,
      .sensor_detect_device_channels = &max96712_sensor_detect_device_channels,
      .sensor_init_setting = &max96712_sensor_init_setting,
      .sensor_set_channel_mode = &max96712_sensor_set_channel_mode,
      .sensor_start_stream = &max96712_sensor_start_stream,
      .sensor_stop_stream = &max96712_sensor_stop_stream,
      .sensor_s_param = &max96712_sensor_s_param,
      .sensor_g_param = &max96712_sensor_g_param,
    },
    .use_sensor_custom_func = TRUE,
    .exposure_func_table =
    {
        .sensor_calculate_exposure = &max96712_calculate_exposure,
        .sensor_exposure_config = &max96712_sensor_exposure_config,
        .sensor_hdr_exposure_config = &max96712_sensor_hdr_exposure_config,
    },
    .sensor_capability = (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG | 1 << SENSOR_CAPABILITY_GAMMA_CONFIG |
                          1 << SENSOR_CAPABILITY_VENDOR_PARAM),
};
/*
 * Describes the sensor Id for each Deserializer
 */
static max96712_topology_config_t default_config =
{
    .opMode = MAXIM_OP_MODE_DEFAULT,
    .num_of_cameras = 1,
    .sensors = {
        { .id = MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300, .mode = 0 },
        { .id = MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300, .mode = 0 },
        { .id = MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300, .mode = 0 },
        { .id = MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300, .mode = 0 }
    },
};

/*
 * Describes sensor-specific information for each connected sensor to De-serilizer
 */
static max96712_sensor_info_t max96712_sensors_init_table[] =
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
    {
      .state = SENSOR_STATE_INVALID,
      .serializer_alias = MSM_DES_0_ALIAS_ADDR_CAM_SER_2,
      .sensor_alias = MSM_DES_0_ALIAS_ADDR_CAM_SNSR_2,
    },
    {
      .state = SENSOR_STATE_INVALID,
      .serializer_alias = MSM_DES_0_ALIAS_ADDR_CAM_SER_3,
      .sensor_alias = MSM_DES_0_ALIAS_ADDR_CAM_SNSR_3,
  },
};
/*
 * Describes the register configuration macros for Deserializer.
 */
static struct camera_i2c_reg_array max96712_init_s5k1h1sx_reg_array[] = MAX96712_INIT_S5K1H1SX;
static struct camera_i2c_reg_array max96712_init_reg_array[] = MAX96712_INIT_REV3;
static struct camera_i2c_reg_array max96712_stop_reg_array[] = MAX96712_STOP;
#ifdef MAX96712_ENABLE_INTR_HANDLER
static struct camera_i2c_reg_array max96712_intr_enable_array[] = CAM_DES_INTR_INIT;
static struct camera_i2c_reg_array max96712_intr_read_clear_array[] = CAM_DES_INTR_READ_CLEAR;
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

static int max96712_set_default(max96712_context_t * pCtxt, const sensor_open_lib_t* device_info)
{
    pCtxt->max96712_config = default_config;
    if (MAXIM_DESER_ID_DEFAULT != device_info->config->type)
    {
        unsigned int i = 0;

        pCtxt->max96712_config.boardType = device_info->boardType;
        pCtxt->max96712_config.opMode = device_info->config->opMode;
        pCtxt->max96712_config.powersave_mode = device_info->config->powerSaveMode;
        pCtxt->max96712_config.num_of_cameras = STD_MIN(device_info->config->numSensors, MAXIM_LINK_MAX);
        for(i = 0; i < pCtxt->max96712_config.num_of_cameras; i++)
        {
            pCtxt->max96712_config.sensors[i].id = device_info->config->sensors[i].type;
            pCtxt->max96712_config.sensors[i].mode = device_info->config->sensors[i].snsrModeId;
        }
    }

    memcpy(&pCtxt->max96712_sensors, max96712_sensors_init_table, sizeof(pCtxt->max96712_sensors));

    /*default to dev id 0*/
    switch (pCtxt->subdev_id)
    {
    case 1:
    case 3:
        pCtxt->slave_addr = MSM_DES_1_SLAVEADDR;
        pCtxt->max96712_sensors[0].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_0;
        pCtxt->max96712_sensors[1].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_1;
        pCtxt->max96712_sensors[2].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_2;
        pCtxt->max96712_sensors[3].serializer_alias = MSM_DES_1_ALIAS_ADDR_CAM_SER_3;
        pCtxt->max96712_sensors[0].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_0;
        pCtxt->max96712_sensors[1].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_1;
        pCtxt->max96712_sensors[2].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_2;
        pCtxt->max96712_sensors[3].sensor_alias = MSM_DES_1_ALIAS_ADDR_CAM_SNSR_3;
        break;
    default:
        pCtxt->slave_addr = MSM_DES_0_SLAVEADDR;
        break;
    }


    if (pCtxt->max96712_config.opMode == MAXIM_OP_MODE_2_LANES)
    {
        pCtxt->sensor_lib.csi_params[0].lane_cnt = 2;
        pCtxt->sensor_lib.csi_params[0].settle_cnt = 0xE;
        pCtxt->sensor_lib.csi_params[0].lane_mask = 0x13;
    }

    return 0;
}

/**
*******************************************************************************
 * FUNCTION:  max96712_sensor_open_lib
 *
 * DESCRIPTION:  Open sensor library (libais_max96712.so),
 *               an instance of structure max96712_context_t is created for
 *               each De-serilizer,
 *               updating Sensor init table of each De-serilizer accordingly
*******************************************************************************
 **/
static void* max96712_sensor_open_lib(void* ctrl, void* arg)
{
    max96712_context_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(max96712_context_t));

    SHIGH("max96712_sensor_open_lib()");

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
        pCtxt->state = MAX96712_STATE_INVALID;
        pCtxt->max96712_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->max96712_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        max96712_set_default(pCtxt, device_info);

        pCtxt->num_supported_sensors = STD_MIN(pCtxt->max96712_config.num_of_cameras, MAXIM_LINK_MAX);
        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            pCtxt->max96712_sensors[i].state = SENSOR_STATE_INVALID;
        }

        pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id = pCtxt->slave_addr;
    }

    return pCtxt;
}

/**
*******************************************************************************
 * FUNCTION: max96712_sensor_close_lib
 *
 * DESCRIPTION: Closes the instance sensor library (libais_max96712.so).
 *              Free the all allocated memory.
*******************************************************************************
 **/
static int max96712_sensor_close_lib(void* ctxt)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int i;

    for (i = 0; i < MAXIM_LINK_MAX; i++)
    {
        if (pCtxt->max96712_sensors[i].pPrivCtxt)
            CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt->max96712_sensors[i].pPrivCtxt);
    }

    CameraDestroyMutex(pCtxt->mutex);
    CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt);

    return 0;
}

/**
 * return bpp for DT
 */
static unsigned int max96712_get_bpp_from_dt(uint32 dt)
{
    switch(dt)
    {
    case 0:
        return 8;
    case CSI_DT_RAW8:
    case CSI_DT_YUV422_8:
        return 8;
    case CSI_DT_RAW10:
    case CSI_DT_YUV422_10:
        return 10;
    case CSI_DT_RAW12:
        return 12;
    case CSI_DT_RAW14:
        return 14;
    case CSI_DT_RAW16:
        return 16;
    case CSI_DT_RAW20:
        return 20;
    default:
        SERR("get bpp for dt 0x%x not implemented", dt);
        return 8;
    }
}

/**
*******************************************************************************
 * FUNCTION: max96712_set_init_sequence
 *
 * DESCRIPTION: Write the Register configuration for Deserializer according to
 * selected mode.
*******************************************************************************
 **/
static int max96712_set_init_sequence(max96712_context_t* pCtxt)
{
    int rc = 0;
    SENSOR_HIGH("max96712_set_init_sequence()");

    if(pCtxt == NULL)
    {
        SERR("Invalid pCtxt pointer (0x%x)",pCtxt);
        return -1;
    }

    unsigned int i = 0;
    boolean bTpgEnable = FALSE;
    boolean bS5k1h1sxEnable = FALSE;

    //Reenable all control channels and disable unused links
    struct camera_i2c_reg_array write_regs1[] = {
        {0x0003, 0xAA, _max96712_delay_},
        {0x0006, pCtxt->connected_sensors | 0xF0, _max96712_delay_}
    };
    pCtxt->max96712_reg_setting.reg_array = write_regs1;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(write_regs1);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to re-enable all links");
        return rc;
    }

    SHIGH("Setting MAX96712 Links Enable to 0x%x", pCtxt->connected_sensors);

    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[i];

        if (pCtxt->connected_sensors & (1 << i))
        {
            rc = pSensor->sensor->init_link(pCtxt, i);
        }

        if (MAXIM_SENSOR_ID_PATTERN_GEN == pCtxt->max96712_config.sensors[i].id)
        {
            /* no init settings to be done for de-serializer if TPG */
            bTpgEnable = TRUE;
            SHIGH("Pattern gen detected. no need of deser init settings");
        }
        if (MAXIM_SENSOR_ID_S5K1H1SX == pCtxt->max96712_config.sensors[i].id)
        {
            /* diff init settings should be done for this sensor */
            bS5k1h1sxEnable = TRUE;
            SHIGH("s5k1h1sx detected. Apply specific deser init settings");
        }
    }

    if (bS5k1h1sxEnable)
    {
        pCtxt->max96712_reg_setting.reg_array = max96712_init_s5k1h1sx_reg_array;
        pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(max96712_init_s5k1h1sx_reg_array);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max96712_reg_setting);
        if (rc)
        {
            SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
        }
    }
    else if (!bTpgEnable)
    {
        unsigned char val = 0;
        unsigned int seq_size = 0;

        // configure assignment of all 8 pipes
        val = (pCtxt->pipelines[1].link << 6) | (pCtxt->pipelines[1].input_id << 4) |
                (pCtxt->pipelines[0].link << 2) | pCtxt->pipelines[0].input_id;
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x00F0, val, _max96712_delay_);
        val = (pCtxt->pipelines[3].link << 6) | (pCtxt->pipelines[3].input_id << 4) |
                (pCtxt->pipelines[2].link << 2) | pCtxt->pipelines[2].input_id;
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x00F1, val, _max96712_delay_);
        val = (pCtxt->pipelines[5].link << 6) | (pCtxt->pipelines[5].input_id << 4) |
                (pCtxt->pipelines[4].link << 2) | pCtxt->pipelines[4].input_id;
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x00F2, val, _max96712_delay_);
        val = (pCtxt->pipelines[7].link << 6) | (pCtxt->pipelines[7].input_id << 4) |
                (pCtxt->pipelines[6].link << 2) | pCtxt->pipelines[6].input_id;
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x00F3, val, _max96712_delay_);

        // Enable available pipes
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x00F4, pCtxt->sensor_lib.src_id_enable_mask, _max96712_delay_);

        // CSI output mapping is default
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x08A3, 0xE4, _max96712_delay_);
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x08A4, 0xE4, _max96712_delay_);

        /***********************
         * PIPE OVERRIDE CONFIGURATION
         * ---------------------------
         * The pipe override configuration is required for parallel input where we need to
         *  assign VC/DT to the incoming data. We assign the pipe configuration based on what
         *  we query from the link. Then we enable this configuration using pCtxt->pipelines[X].pipe_override
         ***********************/
        if (pCtxt->pipe_override)
        {
            // Pipe 0 BPP = 8
            val = 0x2 | (max96712_get_bpp_from_dt(pCtxt->pipelines[0].dst.dt) << 3);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x040B, val, _max96712_delay_);

            // Pipe 0 VC = 0, Pipe 1 VC = 0    USE MIPI_TX to remap
            val = pCtxt->pipelines[0].dst.vc | (pCtxt->pipelines[1].dst.vc << 4);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x040C, 0x00, _max96712_delay_);

            // Pipe 2 VC = 0, Pipe 3 VC = 0    USE MIPI_TX to remap
            val = pCtxt->pipelines[2].dst.vc | (pCtxt->pipelines[3].dst.vc << 4);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x040D, 0x00, _max96712_delay_);

            // Pipe 0 DT | Pipe 1 MSB DT
            val = (pCtxt->pipelines[0].dst.dt) | ((pCtxt->pipelines[1].dst.dt & 0x30) << 2 );
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x040E, val, _max96712_delay_);

            // Pipe 1 LSB DT | Pipe 2 MSB DT
            val = (pCtxt->pipelines[1].dst.dt & 0x0F) | ((pCtxt->pipelines[2].dst.dt & 0x3C) << 2);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x040F, val, _max96712_delay_);

            // Pipe 2 LSB DT | Pipe 3 DT
            val = (pCtxt->pipelines[2].dst.dt & 0x03) | ((pCtxt->pipelines[3].dst.dt) << 2);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0410, val, _max96712_delay_);

            // Pipe 1 BPP | Pipe 2 MSB BPP
            val = max96712_get_bpp_from_dt(pCtxt->pipelines[1].dst.dt) |
                    ((max96712_get_bpp_from_dt(pCtxt->pipelines[2].dst.dt) & 0x1C) << 3);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0411, val, _max96712_delay_);

            // Pipe 2 LSB BPP | Pipe 3 BPP
            val = (max96712_get_bpp_from_dt(pCtxt->pipelines[2].dst.dt) & 0x3) |
                    (max96712_get_bpp_from_dt(pCtxt->pipelines[3].dst.dt) << 2);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0412, val, _max96712_delay_);
        }

        // 2500 Mbps/lane on port A and B and C
        // Override BPP, DT, VC on pipes 0 and 1 if required
        val = 0x39 |
                (pCtxt->pipelines[0].pipe_override << 6) |
                (pCtxt->pipelines[1].pipe_override << 7);
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0415, val, _max96712_delay_);
        // Override BPP, DT, VC on pipes 2 and 3 if required
        val = 0x39 |
                (pCtxt->pipelines[2].pipe_override << 6) |
                (pCtxt->pipelines[3].pipe_override << 7);
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x0418, val, _max96712_delay_);
        // Override BPP, DT, VC on pipes 4 and 5 if required
        val = 0x39 |
                (pCtxt->pipelines[4].pipe_override << 6) |
                (pCtxt->pipelines[5].pipe_override << 7);
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x041B, val, _max96712_delay_);
        // Override BPP, DT, VC on pipes 6 and 7 if required
        val = (pCtxt->pipelines[6].pipe_override << 6) |
              (pCtxt->pipelines[7].pipe_override << 7);
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x041D, val, _max96712_delay_);


        /***********************
         * CSI CONFIGURATION
         * STAR HW CSI connect to PORT A of MAX96712.
         * AIR HW CSI connect to PORT B of MAX96712.
         ***********************/

        //now pipeline config map to Port A, so Copy Port A to Port B
        ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, 0x08A9, 0xC8, _max96712_delay_);

        /**********************
        * Do the VC/DT remapping here.
        * Each pipe requires 3 SRC->DST mappings for Video, FS and FE
        * All mappings destination CSI2 controller 0
        ***********************/
        for (i = 0; i < MAX96712_PIPE_MAX; i++)
        {
            max96712_pipe_config_t* p_pipe = &pCtxt->pipelines[i];

            unsigned int mipi_tx_offset = 0x900 + 0x40*i;
            unsigned int mipi_tx_ext_offset = 0x800 + 0x10*i;
            unsigned char src_vc = (p_pipe->src.vc & 0x3) << 6;
            unsigned char src_vc_ext = (p_pipe->src.vc & 0xFC) >> 2;
            unsigned char vc = (p_pipe->dst.vc & 0x3) << 6;
            unsigned char vc_ext = (p_pipe->dst.vc & 0xFC) >> 2;
            unsigned char dt = p_pipe->dst.dt;

            //3 mappings to CSI0
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0B, 0x07, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x2D, 0x15, _max96712_delay_);
            // MAP0|1|2 - VIDEO | FS | FE
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0D, src_vc | dt, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0E, vc | dt, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0F, src_vc | 0x00, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x10, vc, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x11, src_vc | 0x01, _max96712_delay_);
            ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x12, vc | 0x1, _max96712_delay_);

            //If VCX needed, enable VCX and apply for all 3 mappings
            if (src_vc_ext || vc_ext)
            {
                val = (src_vc_ext << 5) | (vc_ext << 2);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_offset+0x0A, 0xD0, _max96712_delay_);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x00, val, _max96712_delay_);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x01, val, _max96712_delay_);
                ADD_I2C_REG_ARRAY(pCtxt->init_array, seq_size, mipi_tx_ext_offset+0x02, val, _max96712_delay_);
            }
        }

        //print out settings for debugging
        for (i = 0; i < seq_size; i++)
        {
            SHIGH("%d - 0x%x, 0x%x, %d", i,
                pCtxt->init_array[i].reg_addr, pCtxt->init_array[i].reg_data, pCtxt->init_array[i].delay);
        }

        pCtxt->max96712_reg_setting.reg_array = pCtxt->init_array;
        pCtxt->max96712_reg_setting.size = seq_size;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max96712_reg_setting);
        if (rc)
        {
            SERR("Failed to init de-serializer(0x%x)", pCtxt->slave_addr);
        }
    }

    if(!rc)
    {
        pCtxt->state = MAX96712_STATE_INITIALIZED;
    }
    return rc;
}

/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_power_suspend
 *
 * DESCRIPTION: Change the state for MAX96712 as suspend
*******************************************************************************
**/
static int max96712_sensor_power_suspend(void* ctxt, CameraPowerEventType powerEventId)
{
    int rc = 0;
    (void)powerEventId;

    if(ctxt == NULL)
    {
        SERR("Invalid Ctxt pointer (0x%x)", ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    pCtxt->state = MAX96712_STATE_SUSPEND;

    return rc;
}

/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_power_resume
 *
 * DESCRIPTION: Change the state for MAX96712 as resume after it suspended.
*******************************************************************************
**/
static int max96712_sensor_power_resume(void* ctxt, CameraPowerEventType powerEventId)
{
    int rc = 0;
    (void)powerEventId;

    if(ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)", ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    pCtxt->state = MAX96712_STATE_INITIALIZED;

    return rc;
}

static void max96712_gmsl2_check_lock_state(max96712_context_t* pCtxt, uint32 link, uint32 tryTimes)
{
    int rc = 0;
    uint32 timeout = 0;
    uint32 max_check_times = tryTimes;
    uint32 reg_addr = 0;

    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};

    switch(link)
    {
        case 0:
            reg_addr = MSM_DES_CTRL3_REG_ADDR;
            break;
        case 1:
            reg_addr = MSM_DES_CTRL12_REG_ADDR;
            break;
        case 2:
            reg_addr = MSM_DES_CTRL13_REG_ADDR;
            break;
        case 3:
            reg_addr = MSM_DES_CTRL14_REG_ADDR;
            break;
        default:
            SERR("ERROR link %d", link);
            reg_addr = MSM_DES_CTRL3_REG_ADDR;
            break;
    }

    while (timeout++ < max_check_times)
    {
        pCtxt->max96712_reg_setting.reg_array = dummy_reg;
        pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
        pCtxt->max96712_reg_setting.reg_array[0].reg_addr = reg_addr;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                &pCtxt->max96712_reg_setting)))
        {
            // in locking state, De-serializer maybe access failed, so continue
            SERR("Unable to read from the max96712 De-serializer 0x%x (0x%08x)",
                pCtxt->slave_addr, pCtxt->sensor_lib.sensor_slave_info.camera_id);
        }
        if (pCtxt->max96712_reg_setting.reg_array[0].reg_data & 0x08)
        {
            break;
        }
        CameraMicroSleep(1000);
    }
    SHIGH("subdev_id=%d, count=%d, reg[0x%x]= 0x%x", pCtxt->subdev_id, timeout-1,
        pCtxt->max96712_reg_setting.reg_array[0].reg_addr, pCtxt->max96712_reg_setting.reg_array[0].reg_data);
}

/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_remap_channels
 *
 * DESCRIPTION:  Decides the Connected Sensor ID.
 *               Enables link one by one and check for camera detection.
 *               Re-enables all De-serilizer links.
*******************************************************************************
**/
static int max96712_sensor_remap_channels(max96712_context_t* pCtxt)
{
    int rc = 0;
    int link = 0;

    if(pCtxt == NULL)
    {
        SERR("Invalid pCtxt pointer (0x%x)",pCtxt);
        return -1;
    }

    if (pCtxt->state != MAX96712_STATE_DETECTED)
    {
        SHIGH("max96712 0x%x not be detected - wrong state", pCtxt->slave_addr);
        return -1;
    }

    for (link = 0; link < pCtxt->num_supported_sensors; link++)
    {
        max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];

        pSensor->mode = pCtxt->max96712_config.sensors[link].mode;
        switch (pCtxt->max96712_config.sensors[link].id)
        {
        case MAXIM_SENSOR_ID_AR0231:
            SHIGH("MAXIM_SENSOR_ID_AR0231 slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = ar0231_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0231_EXT_ISP:
            SHIGH("MAXIM_SENSOR_ID_AR0231_EXT_ISP slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = ar0231_ext_isp_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300:
            SHIGH("MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300 slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = ar0820_ext_isp_gw5300_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_S5K1H1SX:
            SHIGH("MAXIM_SENSOR_ID_S5K1H1SX slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = s5k1h1sx_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_AR0234_EXT_FPGA:
            SHIGH("MAXIM_SENSOR_ID_AR0234 slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = ar0234_ext_fpga_get_sensor_info();
            break;
        case MAXIM_SENSOR_ID_PATTERN_GEN:
        case MAXIM_SENSOR_ID_SER_PATTERN_GEN:
            SHIGH("MAXIM_SENSOR_ID_PATTERN_GEN/SER_PATTERN slave id %d", pCtxt->max96712_config.sensors[link].id);
            pSensor->sensor = max96712_pattern_gen_get_sensor_info();
            break;
        default:
            SERR("Slave ID %d NOT SUPPORTED", pCtxt->max96712_config.sensors[link].id);
            pSensor->state = SENSOR_STATE_UNSUPPORTED;
            break;
        }

        if (!pSensor->sensor)
        {
            continue;
        }

        if (pCtxt->num_supported_sensors > 1)
        {
            //disable back channel instead of disabling links so we do not require wait for re-lock
            // only enable back channel of selected link
            struct camera_i2c_reg_array write_regs[] = {
                {0x0003, 0xFF ^ (1 << (link*2)), _max96712_delay_}
            };
            pCtxt->max96712_reg_setting.reg_array = write_regs;
            pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(write_regs);
            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max96712_reg_setting)))
            {
                SERR("Unable to set deserailzer 0x%x I2C Mode Link %d. Fatal error!", pCtxt->slave_addr, link);
                return rc;
            }
        }

        max96712_gmsl2_check_lock_state(pCtxt, link, 100);

        rc = pSensor->sensor->detect(pCtxt, link);
        if (!rc)
        {
            pSensor->state = SENSOR_STATE_DETECTED;
            pCtxt->num_connected_sensors++;
            pCtxt->connected_sensors |= (1 << link);
        }
        else
        {
            SERR("Unable to detect serailzer on 0x%x Link %d.",  pCtxt->max96712_sensors[link].serializer_alias, link);
        }
    }

    return 0;
}


/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_detect_device
 *
 * DESCRIPTION: Detection of De-serilizer is done here,
                Reading chip id from MAX96712 register to decide chip revision.
*******************************************************************************
**/
static int max96712_sensor_detect_device(void* ctxt)
{
    int rc = 0;
    SHIGH("max96712_sensor_detect_device()");

    if(ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    if (pCtxt->state >= MAX96712_STATE_DETECTED)
    {
        SHIGH("already detected");
        return 0;
    }

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        SERR("I2C function table not initialized");
        return -1;
    }

    //Detect MAX96712
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_CHIP_ID_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 De-serialzer 0x%x (0x%08x)",
            pCtxt->slave_addr, pCtxt->sensor_lib.sensor_slave_info.camera_id);
        return rc;
    }

    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_REVISION_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read( pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Failed to read de-serializer(0x%x) revision", pCtxt->slave_addr);
    }
    else
    {
        pCtxt->revision = pCtxt->max96712_reg_setting.reg_array[0].reg_data;
        SHIGH("MAX96712 with revision %d detected!", pCtxt->revision);
        pCtxt->state = MAX96712_STATE_DETECTED;
    }

    return rc;
}

#ifdef MAX96712_ENABLE_INTR_HANDLER
/**
*******************************************************************************
* FUNCTION:    max96712_intr_read_global_error
*
* DESCRIPTION:  Read the state of MSM_DES_CTRL3_REG_ADDR,
*               Which indicates if bridge error happens.
*******************************************************************************
**/
static int max96712_intr_read_global_error(max96712_context_t* pCtxt, bool* err_flag)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    int rc = 0;

    *err_flag = 0;

    CameraLockMutex(pCtxt->mutex);
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_CTRL3_REG_ADDR;

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                    &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
             pCtxt->slave_addr, pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }

    *err_flag = (pCtxt->max96712_reg_setting.reg_array[0].reg_data & 0x04) == 0 ? false : true;

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    return rc;
}

/**
*******************************************************************************
* FUNCTION:    max96712_intr_clear_cnt
*
* DESCRIPTION:  Read & clear CNT registers.
*******************************************************************************
**/
static int max96712_intr_clear_cnt(max96712_context_t* pCtxt)
{
    int array_size = STD_ARRAY_SIZE(max96712_intr_read_clear_array);
    int rc = 0;

    CameraLockMutex(pCtxt->mutex);

    /* Read one register at a time for now
     * @Todo: use a burst read API to read all at once.
     */
    for (int i = 0; i < array_size; i++)
    {
        pCtxt->max96712_reg_setting.reg_array = &max96712_intr_read_clear_array[i];
        pCtxt->max96712_reg_setting.size = 1;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
                        &pCtxt->max96712_reg_setting)))
        {
            SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
                    pCtxt->slave_addr,pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
            goto EXIT;
        }
    }

EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    return rc;
}

/**
*******************************************************************************
* FUNCTION:    max96712_intr_read_status
*
* DESCRIPTION:  Read INTR registers.
*******************************************************************************
**/
static int max96712_intr_read_status(max96712_context_t* pCtxt, unsigned short* intr_link_src)
{
    struct camera_i2c_reg_array dummy_reg[] = {{0, 0, 0}};
    unsigned short intr3_val = 0, intr5_val = 0, intr7_val = 0, intr9_val = 0;
    int rc = 0;

    *intr_link_src = 0;

    CameraLockMutex(pCtxt->mutex);
    /* DEC_ERR */
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR3_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
            pCtxt->slave_addr, pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr3_val = pCtxt->max96712_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_DEC_ERR_MASK;

    /* EOM_ERR */
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR5_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
            pCtxt->slave_addr,pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr5_val = pCtxt->max96712_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_EOM_ERR_MASK;

    /* CRC_ERR */
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR7_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
            pCtxt->slave_addr,pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr7_val = pCtxt->max96712_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_CRC_ERR_MASK;

    /* IDLE_ERR */
    pCtxt->max96712_reg_setting.reg_array = dummy_reg;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
    pCtxt->max96712_reg_setting.reg_array[0].reg_addr = MSM_DES_INTR9_REG_ADDR;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, pCtxt->slave_addr,
              &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to read from the MAX96712 slave 0x%x reg 0x%x",
            pCtxt->slave_addr,pCtxt->max96712_reg_setting.reg_array[0].reg_addr);
        goto EXIT;
    }
    intr9_val = pCtxt->max96712_reg_setting.reg_array[0].reg_data & MSM_DES_INTR_IDLE_ERR_MASK;


EXIT:
    CameraUnlockMutex(pCtxt->mutex);

    SERR("intr3_val=0x%02x, intr5_val=0x%02x, intr7_val=0x%02x, intr9_val=0x%02x",
            intr3_val, intr5_val, intr7_val, intr9_val);

    if ((intr3_val & 0x01) || (intr5_val & (1 << 4)) || (intr9_val & 0x01)) {
        //notify Link A signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_A;
    }
    if ((intr3_val & 0x02) || (intr5_val & (1 << 5)) || (intr9_val & 0x02)) {
        //notify Link B signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_B;
    }
    if ((intr3_val & 0x04) || (intr5_val & (1 << 6)) || (intr9_val & 0x04)) {
        //notify Link C signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_C;
    }
    if ((intr3_val & 0x08) || (intr5_val & (1 << 7)) || (intr9_val & 0x08)) {
        //notify Link D signal lost;
        *intr_link_src |= 1 << MAXIM_LINK_D;
    }

    return rc;

}

/**
*******************************************************************************
* FUNCTION:    max96712_intr_handler
*
* DESCRIPTION:  When chip recieves bridge error interrupt, call this function 
*               to handle it.
*******************************************************************************
**/
static void max96712_intr_handler(void* data)
{
    max96712_context_t* pCtxt = (max96712_context_t*)data;
    unsigned short link_src = 0;
    bool err_flag = false;
    CameraInputEventPayloadType payload = {};
    int rc = 0;

    if(pCtxt == NULL)
    {
        SERR("Invalid max96712 context");
        return;
    }

    /* read ctrl3 status, reflect reverse value of ERRB pin */
    rc = max96712_intr_read_global_error(pCtxt, &err_flag);

    /* get which camera got signal error */
    rc |= max96712_intr_read_status(pCtxt, &link_src);
    SHIGH("INTR enter 0x%x, err_flag 0x%x, link_src 0x%x",
            pCtxt->slave_addr, err_flag, link_src);

    if (pCtxt->state != MAX96712_STATE_STREAMING)
    {
        SHIGH("max96712 context state %d not in streaming", pCtxt->state);
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
                    && ((pCtxt->max96712_sensors[i].signal_status == INTR_SIGNAL_STATUS_NONE)
                        || (pCtxt->max96712_sensors[i].signal_status == INTR_SIGNAL_STATUS_VALID)))
            {
                payload.src_id = i;
                payload.lock_status = QCARCAM_INPUT_SIGNAL_LOST;
                pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);

                pCtxt->max96712_sensors[i].signal_status = INTR_SIGNAL_STATUS_LOST;
                SHIGH("notify signal lost");
            }
        }
        CameraSleep(MAX96712_INTR_SETTLE_SLEEP);

        rc = max96712_intr_clear_cnt(pCtxt);
        rc |= max96712_intr_read_status(pCtxt, &link_src);
        rc |= max96712_intr_read_global_error(pCtxt, &err_flag);

        if (rc)
        {
            SERR("loop read register failed, rc = %d", rc);
            break;
        }
        SHIGH("err_flag 0x%x, link_src 0x%x", err_flag, link_src);

        if (!err_flag)
        {
            max96712_intr_read_status(pCtxt, &link_src);
            SHIGH("second read link_src 0x%x", link_src);
            for (uint32 i=MAXIM_LINK_A; i < pCtxt->num_supported_sensors; i++)
            {
                if (!(link_src & (1 << i)) && (pCtxt->max96712_sensors[i].signal_status == INTR_SIGNAL_STATUS_LOST))
                {
                    payload.src_id = i;
                    payload.lock_status = QCARCAM_INPUT_SIGNAL_VALID;
                    pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);

                    pCtxt->max96712_sensors[i].signal_status = INTR_SIGNAL_STATUS_VALID;
                    SHIGH("notify signal valid");
                }
            }
        }
    }

    return;
}
#endif /*MAX96712_ENABLE_INTR_HANDLER*/

/**                                                                                 \
*********************************************************************************** \
 * FUNCTION:    max96712_sensor_detect_device_channels                              \
 *                                                                                  \
 * DESCRIPTION: Fix the stream_id for each link and fills the channel information   \
 *              like format, resolution, virtual channel,fps.. etc for each channel \
*********************************************************************************** \
**/
static int max96712_sensor_detect_device_channels(void* ctxt)
{
    int rc = 0;
    int link = 0;
    int stream_id = 0;

    if(ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    SHIGH("initialize max96712 0x%x with %d sensors", pCtxt->slave_addr, pCtxt->num_supported_sensors);
    if(pCtxt->subdev_id == 0)
    {
       ais_log_kpi(AIS_EVENT_KPI_DETECT_SENSORS_START);
    }

    if (pCtxt->state >= MAX96712_STATE_INITIALIZED)
    {
        SERR("already detected %d out of %d", pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
        return 0;
    }
    else if (pCtxt->state != MAX96712_STATE_DETECTED)
    {
        SERR("MAX96712 0x%x not detected - wrong state", pCtxt->slave_addr);
        return -1;
    }

    pCtxt->max96712_reg_setting.reg_array = max96712_init_reg_array;
    pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(max96712_init_reg_array);
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pCtxt->slave_addr, &pCtxt->max96712_reg_setting)))
    {
        SERR("Unable to init deserailzer 0x%x", pCtxt->slave_addr);
        return -1;
    }

    //@TODO: do not need sleep entire 100ms
    // can try only 50ms or poll LOCK bits for the links
    CameraMicroSleep(MAX96712_SELECT_LINK_DELAY);

    if (0 != (rc = max96712_sensor_remap_channels(pCtxt)))
    {
        SERR("Unable to remap deserailzer 0x%x", pCtxt->slave_addr);
        return -1;
    }

    for (link = 0; link < pCtxt->num_supported_sensors; link++)
    {
        max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];

        rc = pSensor->sensor->get_link_cfg(pCtxt, link, &pSensor->link_cfg);
        if (!rc)
        {
            int pipe = 0;

            if (pSensor->link_cfg.num_pipelines > MAX_LINK_PIPELINES)
            {
                SERR("Only support max of %d pipelines per link", MAX_LINK_PIPELINES);
                pSensor->link_cfg.num_pipelines = MAX_LINK_PIPELINES;
            }

            if (pSensor->state != SENSOR_STATE_DETECTED)
            {
                stream_id += pSensor->link_cfg.num_pipelines;
                continue;
            }

            for (pipe = 0; pipe < pSensor->link_cfg.num_pipelines; pipe++)
            {
                img_src_channel_t *pChannel = &pCtxt->sensor_lib.channels[pCtxt->sensor_lib.num_channels];
                img_src_subchannel_t *pSubChannel = &pCtxt->sensor_lib.subchannels[pCtxt->sensor_lib.num_subchannels];
                img_src_subchannel_layout_t layout = {
                    .src_id = stream_id,
                };
                max96712_pipe_config_t* p_pipe;

                if (stream_id >= MAX96712_PIPE_MAX)
                {
                    SERR("Too many pipes. Cannot add link %d pipe %d", link, pipe);
                    continue;
                }

                p_pipe = &pCtxt->pipelines[stream_id];
                p_pipe->link = link;
                p_pipe->input_id = pSensor->link_cfg.pipelines[pipe].id;
                p_pipe->link_type = pSensor->link_cfg.link_type;
                p_pipe->src = pSensor->link_cfg.pipelines[pipe].mode.channel_info;

                //Enable VCX and map each pipe to unique VC
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

                //advertise stream as available
                pCtxt->sensor_lib.src_id_enable_mask |= (1 << stream_id);

                stream_id++;
            }
        }
    }

#ifdef MAX96712_ENABLE_INTR_HANDLER
    /* INTR init */
    if (pCtxt->sensor_lib.num_channels)
    {
        rc = pCtxt->platform_fcn_tbl.setup_gpio_interrupt(pCtxt->ctrl,
                CAMERA_GPIO_INTR, max96712_intr_handler, pCtxt);
        if (!rc)
        {
            CameraLockMutex(pCtxt->mutex);
            pCtxt->max96712_reg_setting.reg_array = max96712_intr_enable_array;
            pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(max96712_intr_enable_array);

            SHIGH("setup_gpio_interrupt and max96712_intr_enable_array for (0x%d)", pCtxt->slave_addr);

            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->max96712_reg_setting)))
            {
                SERR("Failed to start max96712_init_intr_array (0x%x)", pCtxt->slave_addr);
            }
            CameraUnlockMutex(pCtxt->mutex);

            pCtxt->max96712_sensors[MAXIM_LINK_A].signal_status = INTR_SIGNAL_STATUS_NONE;
            pCtxt->max96712_sensors[MAXIM_LINK_B].signal_status = INTR_SIGNAL_STATUS_NONE;
            pCtxt->max96712_sensors[MAXIM_LINK_C].signal_status = INTR_SIGNAL_STATUS_NONE;
            pCtxt->max96712_sensors[MAXIM_LINK_D].signal_status = INTR_SIGNAL_STATUS_NONE;
        }
    }
#endif /*MAX96712_ENABLE_INTR_HANDLER*/

    CameraLogEvent(CAMERA_SENSOR_EVENT_PROBED, 0, 0);

    return rc;
}

/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_init_setting
 *
 * DESCRIPTION: Serializer and De-serializer initialisation is performed.
*******************************************************************************
**/
static int max96712_sensor_init_setting(void* ctxt)
{
    int rc = 0;
    SHIGH("max96712_sensor_init_setting()");

    if(ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    if(!pCtxt->num_connected_sensors)
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

        rc = max96712_set_init_sequence(pCtxt);
        if(rc)
        {
            SERR("Failed to set init sequence for de-serializer(0x%x)", pCtxt->slave_addr);
            return rc;
        }

        if(pCtxt->subdev_id == 0)
        {
           ais_log_kpi(AIS_EVENT_KPI_SENSOR_PROG_END);
        }
        CameraLogEvent(CAMERA_SENSOR_EVENT_INITIALIZE_DONE, 0, 0);
    }
    return rc;
}

/**
*******************************************************************************
 * FUNCTION:    max96712_sensor_set_channel_mode
 *
 * DESCRIPTION:
*******************************************************************************
**/
static int max96712_sensor_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode)
{
    (void)ctxt;
    (void)mode;
    (void)src_id_mask;

    SHIGH("max96712_sensor_set_channel_mode()");

    return 0;
}

/**
*******************************************************************************
 * FUNCTION:   max96712_sensor_start_stream
 *
 * DESCRIPTION: writes the register configuration for starting stream on serializer
 *              as well as De-serializer
*******************************************************************************
**/
static int max96712_sensor_start_stream(void* ctxt, unsigned int src_id_mask)
{
    int rc = 0;
    unsigned int i = 0;
    unsigned int started_mask = 0;

    SHIGH("max96712_sensor_start_stream(), src_id_mask 0x %x", src_id_mask);

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    //Now start the cameras
    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        if ((1 << i) & src_id_mask)
        {
            if (SENSOR_STATE_INITIALIZED == pCtxt->max96712_sensors[i].state)
            {
                SENSOR_WARN("starting slave %x", pCtxt->max96712_sensors[i].serializer_alias);
                {
                    CameraLockMutex(pCtxt->mutex);
                    pCtxt->max96712_sensors[i].sensor->start_link(pCtxt, i);
                    CameraUnlockMutex(pCtxt->mutex);
                }
                pCtxt->max96712_sensors[i].state = SENSOR_STATE_STREAMING;
                started_mask |= (1 << i);
            }
            else
            {
                /*TODO: change this to SERR once we limit which slaves to start*/
                SHIGH("sensor 0x%x not ready to start (state=%d) - bad state",
                        pCtxt->max96712_sensors[i].serializer_alias, pCtxt->max96712_sensors[i].state);
            }
        }
    }
    if (!rc &&
        MAX96712_STATE_INITIALIZED == pCtxt->state &&
        started_mask)
    {
#ifdef MAX96712_ENABLE_INTR_HANDLER
        // start stream only if no bridge error
        unsigned short link_src;
        bool err_flag = false;
        unsigned int retry_cnt = 0;
        max96712_intr_read_global_error(pCtxt, &err_flag);
        while (retry_cnt < MAX96712_MAX_ERROR_CHECK_CNT)
        {
            max96712_intr_clear_cnt(pCtxt);
            max96712_intr_read_status(pCtxt, &link_src);
            max96712_intr_read_global_error(pCtxt, &err_flag);
            if (!err_flag)
                break;

            retry_cnt++;

            CameraSleep(10);
        }

        if (retry_cnt == MAX96712_MAX_ERROR_CHECK_CNT)
        {
            for (i = 0; i < pCtxt->num_supported_sensors; i++)
            {
                if ((1 << i) & started_mask)
                {
                    pCtxt->max96712_sensors[i].state = SENSOR_STATE_INITIALIZED;
                }
            }
            SERR("Shouldn't start de-serializer(0x%d) with error", pCtxt->slave_addr);
            return -1;
        }
#endif /*MAX96712_ENABLE_INTR_HANDLER*/

        if (MAXIM_SENSOR_ID_PATTERN_GEN != pCtxt->max96712_config.sensors[0].id)
        {
            struct camera_i2c_reg_array start_array[3];
            int start_size = 0;
            unsigned char val = 0;
            unsigned char dt0 = pCtxt->pipelines[0].dst.dt;

            /**
             * Disable CSI, take CSI out of standby then turn on CSI
             */
            //NOTE: 0x040B has bpp for pipe0 in highest bits so keep those as is.
            val = max96712_get_bpp_from_dt(dt0) << 3;
            ADD_I2C_REG_ARRAY(start_array, start_size, 0x040B, val, _max96712_delay_);
            ADD_I2C_REG_ARRAY(start_array, start_size, 0x08A2, 0xF4, _max96712_delay_);
            val = 0x2 | (max96712_get_bpp_from_dt(dt0) << 3);
            ADD_I2C_REG_ARRAY(start_array, start_size, 0x040B, val, _max96712_delay_);

            CameraLockMutex(pCtxt->mutex);
            //@todo: dynamically only start needed streams
            pCtxt->max96712_reg_setting.reg_array = start_array;
            pCtxt->max96712_reg_setting.size = start_size;

            SHIGH("starting deserializer");
            //Start the deserialzer
            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->max96712_reg_setting)))
            {
                SERR("Failed to start de-serializer(0x%x)", pCtxt->slave_addr);
            }
            else
            {
                pCtxt->state = MAX96712_STATE_STREAMING;
            }
            CameraUnlockMutex(pCtxt->mutex);
        }
    }

    if (!rc)
    {
        pCtxt->streaming_src_mask |= started_mask;
    }

    CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_START, 0, 0);
    SHIGH("max96712(0x%x) streaming...pCtxt->streaming_src_mask 0x%x",
        pCtxt->slave_addr, pCtxt->streaming_src_mask);

    return rc;
}

/**
*******************************************************************************
 * FUNCTION:   max96712_sensor_stop_stream
 *
 * DESCRIPTION: writes the register configuration for stop stream on serializer
 *               as well as De-serializer
*******************************************************************************
**/
static int max96712_sensor_stop_stream(void* ctxt, unsigned int src_id_mask)
{
    int rc = 0;
    unsigned int i;
    SHIGH("max96712_sensor_stop_stream(), src_id_mask 0x %x", src_id_mask);

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    /*stop transmitter first if no more clients*/
    if (!rc && MAX96712_STATE_STREAMING == pCtxt->state)
    {
        pCtxt->streaming_src_mask &= (~src_id_mask);
        SHIGH("stopping max96712 0x%x transmitter (%x)", pCtxt->slave_addr, pCtxt->streaming_src_mask);

        /*stop if no slaves streaming*/
        if (!pCtxt->streaming_src_mask)
        {
            pCtxt->max96712_reg_setting.reg_array = max96712_stop_reg_array;
            pCtxt->max96712_reg_setting.size = STD_ARRAY_SIZE(max96712_stop_reg_array);

            CameraLockMutex(pCtxt->mutex);
            if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->max96712_reg_setting)))
            {
                SERR("Failed to stop de-serializer(0x%x)", pCtxt->slave_addr);
            }
            CameraUnlockMutex(pCtxt->mutex);

            pCtxt->state = MAX96712_STATE_INITIALIZED;
        }
    }

    /*then stop slaves*/
    for (i = 0; i < pCtxt->num_supported_sensors; i++)
    {
        if ((1 << i) & src_id_mask)
        {
            SHIGH("stopping slave %x", pCtxt->max96712_sensors[i].serializer_alias);
            if (SENSOR_STATE_STREAMING == pCtxt->max96712_sensors[i].state)
            {
                CameraLockMutex(pCtxt->mutex);
                rc = pCtxt->max96712_sensors[i].sensor->stop_link(pCtxt, i);
                CameraUnlockMutex(pCtxt->mutex);
                if (rc)
                {
                    /*TODO: change this to SERR once we limit which slaves to stop*/
                    SERR("sensor 0x%x failed to stop", pCtxt->max96712_sensors[i].serializer_alias);
                }
                pCtxt->max96712_sensors[i].state = SENSOR_STATE_INITIALIZED;
            }
            else
            {
                /*TODO: change this to SERR once we limit which slaves to stop*/
                SERR("sensor 0x%x not in state to stop (state=%d) - bad state",
                    pCtxt->max96712_sensors[i].serializer_alias, pCtxt->max96712_sensors[i].state);
            }
        }
    }

    /* TODO: cleanup in case of failure */
    CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_STOP, 0, 0);
    SHIGH("max96712(0x%x) stopped", pCtxt->slave_addr);

    return rc;
}

/**
*******************************************************************************
 * FUNCTION: max96712_sensor_set_platform_func_table
 *
 * DESCRIPTION: Copies the initialization table in pctxt.
*******************************************************************************
 **/
static int max96712_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table)
{
    SHIGH("max96712_sensor_set_platform_func_table");

    if(ctxt == NULL)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        return -1;
    }

    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        if (!table || !table->i2c_slave_write_array || !table->i2c_slave_read)
        {
            SERR("Invalid i2c func table param");
            return -1;
        }

        pCtxt->platform_fcn_tbl = *table;
        pCtxt->platform_fcn_tbl_initialized = 1;
        SLOW("i2c func table set");
    }

    return 0;
}

static int max96712_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return -1;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max96712_sensors[src_id].sensor &&
            pCtxt->max96712_sensors[src_id].sensor->calculate_exposure)
        {
            rc = pCtxt->max96712_sensors[src_id].sensor->calculate_exposure(pCtxt, src_id, exposure_info);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = -1;
    }

    return rc;
}

static int max96712_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return -1;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max96712_sensors[src_id].sensor &&
            pCtxt->max96712_sensors[src_id].sensor->apply_exposure)
        {
            rc = pCtxt->max96712_sensors[src_id].sensor->apply_exposure(pCtxt, src_id, exposure_info);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = -1;
    }

    return rc;
}

static int max96712_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return -1;
    }

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max96712_sensors[src_id].sensor &&
            pCtxt->max96712_sensors[src_id].sensor->apply_hdr_exposure)
        {
            rc = pCtxt->max96712_sensors[src_id].sensor->apply_hdr_exposure(pCtxt, src_id, hdr_exposure);
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = -1;
    }

    return rc;
}

static int max96712_sensor_gamma_config(void* ctxt, unsigned int src_id, qcarcam_gamma_config_t* gamma)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;

    if (src_id < pCtxt->num_supported_sensors)
    {
        if (pCtxt->max96712_sensors[src_id].sensor &&
            pCtxt->max96712_sensors[src_id].sensor->apply_gamma)
        {
            rc = pCtxt->max96712_sensors[src_id].sensor->apply_gamma(pCtxt, src_id, gamma);
        }
        else
        {
            SERR("Unsupported gamma config for %d", src_id);
            rc = -1;
        }
    }
    else
    {
        SERR("invalid src_id %d", src_id);
        rc = -1;
    }

    return rc;
}

static int max96712_sensor_set_vendor_param(void* ctxt, unsigned int src_id, qcarcam_vendor_param_t* param)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    CameraInputEventPayloadType payload = {};
    int rc = 0;
    CAM_UNUSED(src_id);

    SHIGH("data[0]= %u data[1]= %u", param->data[0], param->data[1]);
    payload.src_id = src_id;
    payload.vendor_data = *param;
    pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_VENDOR, &payload);

    return rc;
}

static int max96712_sensor_get_vendor_param(void* ctxt, unsigned int src_id, qcarcam_vendor_param_t* param)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;
    CAM_UNUSED(src_id);

    SHIGH("data[0] = %u", param->data[0]);
    param->data[1] = pCtxt->revision;

    return rc;
}

static int max96712_sensor_s_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt");
        return -1;
    }

    if (param == NULL)
    {
        SERR("Invalid params");
        return -1;
    }

    switch(id)
    {
        case QCARCAM_PARAM_GAMMA:
            rc = max96712_sensor_gamma_config(ctxt, src_id, (qcarcam_gamma_config_t*)param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = max96712_sensor_set_vendor_param(ctxt, src_id, (qcarcam_vendor_param_t*)param);
            break;
        default:
            SERR("Param not supported = %d", id);
            rc = -1;
            break;
    }
    return rc;
}

static int max96712_sensor_g_param(void* ctxt, qcarcam_param_t id, unsigned int src_id, void* param)
{
    int rc = 0;

    if (ctxt == NULL)
    {
        SERR("Invalid ctxt");
        return -1;
    }

    if (param == NULL)
    {
        SERR("Invalid params");
        return -1;
    }

    switch(id)
    {
        case QCARCAM_PARAM_GAMMA:
            rc = CAMERA_EUNSUPPORTED;
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = max96712_sensor_get_vendor_param(ctxt, src_id, (qcarcam_vendor_param_t*)param);
            break;
        default:
            SERR("Param not supported = %d", id);
            rc = -1;
            break;
    }
    return rc;

}

/**
*******************************************************************************
 * FUNCTION: CameraSensorDevice_Open_max96712
 *
 * DESCRIPTION: Entry function for device driver framework
*******************************************************************************
 **/
CAM_API CameraResult CameraSensorDevice_Open_max96712(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId)
{
    sensor_lib_interface_t sensor_lib_interface = {
            .sensor_open_lib = max96712_sensor_open_lib,
    };

    return CameraSensorDevice_Open(ppNewHandle, deviceId, &sensor_lib_interface);
}

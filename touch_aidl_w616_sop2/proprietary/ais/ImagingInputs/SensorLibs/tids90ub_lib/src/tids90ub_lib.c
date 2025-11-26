/**
 * @file tids90ub_lib.c
 *
 * Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "CameraSensorDeviceInterface.h"

#include "tids90ub_lib.h"

/**
 * DEFINITIONS
 */

/* Turn this on to enable interrupt handler
 */
#define TIDS90UB_ENABLE_INTR_HANDLER 1

/* Turn this on to disable interrupts on sleep to prevent
 *   wake up of MSM
 */
#define DISABLE_INTR_ON_SLEEP 1

/* calculate the FPS from the frame sync frequency */
#define FRAME_SYNC_FREQ_TO_FPS(fsync_freq) ((fsync_freq+1)*5)

/* calculate the frame sync frequency from the FPS */
#define FSYNC_FPS_TO_FREQ(fps) ((fps/5)-1)

/* Turn this on to initialize remote serializer
 *   - sets higher throughput through BCC
 */
#define INITIALIZE_SERIALIZER 0

#if defined(__INTEGRITY) || defined(__QNXNTO__)
#define TIDS960_INIT_SLEEP 200
#endif

#define TIDS90UB_INTR_SETTLE_SLEEP 10 //10ms for now
#define TIDS90UB_SIGNAL_LOCK_NUM_TRIES 10 //10 tries with 20ms sleeps
#define TIDS90UB_SIGNAL_LOCK_WAIT 20
#define TIDS90UB_SIGNAL_LOCKED TRUE
#define TIDS90UB_SIGNAL_LOST   FALSE

/* Valid GPIO Check
 */
#define TIDS90UB_VALID_GPIO_CHECK(gpio)  ((TIDS90UB_GPIO_INVALID != gpio) && (TIDS90UB_GPIO_3 != gpio))

/*Adds I2C command to i2c reg array*/
#define ADD_I2C_REG_ARRAY(_a_, _size_, _addr_, _val_, _delay_) \
do { \
    _a_[_size_].reg_addr = _addr_; \
    _a_[_size_].reg_data = _val_; \
    _a_[_size_].delay = _delay_; \
    _size_++; \
} while(0)

#define TIDS90UB_PORT_SEL(_rxport_) ((1 << (_rxport_ & 0x3)) | ((_rxport_ & 0x3) << 4))
#define TIDS90UB_IAR_SEL(_rxport_)   ((1 + _rxport_) << 2)

/**
 * EXTERNAL
 */
/*Supported sensors get info functions*/
tids90ub_sensor_t* ox03a10_get_sensor_info(void);
tids90ub_sensor_t* imx_gw5200_get_sensor_info(void);
tids90ub_sensor_t* x3a_ov491_get_sensor_info(void);
tids90ub_sensor_t* tids90ub_pattern_gen_get_sensor_info(void);

/**
 * INTERNAL
 */
static void* tids90ub_sensor_open_lib(void* ctrl, void* arg);
static int tids90ub_sensor_close_lib(void* ctxt);
static int tids90ub_sensor_power_suspend(void* ctxt, CameraPowerEventType powerEventId);
static int tids90ub_sensor_power_resume(void* ctxt, CameraPowerEventType powerEventId);
static int tids90ub_sensor_detect_device(void* ctxt);
static int tids90ub_sensor_detect_device_channels(void* ctxt);
static int tids90ub_sensor_init_setting(void* ctxt);
static int tids90ub_sensor_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode);
static int tids90ub_sensor_start_stream(void* ctxt, unsigned int src_id_mask);
static int tids90ub_sensor_stop_stream(void* ctxt, unsigned int src_id_mask);
static int tids90ub_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table);
static int tids90ub_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int tids90ub_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info);
static int tids90ub_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure_info);
static int tids90ub_sensor_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param);
static int tids90ub_sensor_g_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param);

static void tids90ub_set_default(tids90ub_context_t* pCtxt, const sensor_open_lib_t* device_info);
static int tids90ub_set_init_sequence(tids90ub_context_t* pCtxt);
static int tids90ub_probe_sensor_serializer(tids90ub_context_t* pCtxt, uint32 port);
static int tids90ub_read_status(tids90ub_context_t* pCtxt, uint32 port);
static int tids90ub_read_lock_pass_status(tids90ub_context_t* pCtxt, boolean* lock_chg, boolean* lock_pass, uint32 rx_port);
static int tids90ub_set_fsync_rxport(tids90ub_context_t* pCtxt, uint32 port);
static int tids90ub_set_fsync_mode(tids90ub_context_t* pCtxt);
static int tids90ub_set_manual_eq(tids90ub_context_t* pCtxt, uint32 port);

#if TIDS90UB_ENABLE_INTR_HANDLER
static int tids90ub_enable_intr_pin(tids90ub_context_t* pCtxt);
static int tids90ub_disable_intr_pin(tids90ub_context_t* pCtxt);
static int tids90ub_read_int_sts(tids90ub_context_t* pCtxt, unsigned short * intr_src_mask);
static void tids90ub_intr_handler(void* data);
#endif

/**
 * LOCAL DEFINITIONS
 */
static const sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
      .sensor_name = SENSOR_MODEL,
      .slave_addr = TIDS90UB_0_SLAVEADDR,
      .i2c_freq_mode = SENSOR_I2C_MODE_CUSTOM,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .is_init_params_valid = 1,
      .sensor_id_info =
      {
        .sensor_id = TIDS90UB_0_SLAVEADDR,
        .sensor_id_reg_addr = 0x00,
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
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VANA,
            .config_val = 0,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VIO,
            .config_val = 0,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_LOW,
            .delay = 2,
          },
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_HIGH,
            .delay = 2,
          },
        },
        .size_up = 5,
        .power_down_setting_a =
        {
          {
            .seq_type = CAMERA_POW_SEQ_GPIO,
            .seq_val = CAMERA_GPIO_RESET,
            .config_val = GPIO_OUT_LOW,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VIO,
            .config_val = 0,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VANA,
            .config_val = 0,
            .delay = 0,
          },
          {
            .seq_type = CAMERA_POW_SEQ_VREG,
            .seq_val = CAMERA_VDIG,
            .config_val = 0,
            .delay = 0,
          },
        },
        .size_down = 4,
      },
  },
  .csi_params =
  {
    {
    .lane_cnt = 4,
    .settle_cnt = SETTLE_COUNT,
    .lane_mask = 0x1F,
    .combo_mode = 0,
    .is_csi_3phase = 0,
    .mipi_rate = 0,
    }
  },
  .sensor_close_lib = tids90ub_sensor_close_lib,
  .exposure_func_table =
  {
    .sensor_calculate_exposure = &tids90ub_calculate_exposure,
    .sensor_exposure_config = &tids90ub_sensor_exposure_config,
    .sensor_hdr_exposure_config = &tids90ub_sensor_hdr_exposure_config,
  },
  .sensor_capability = (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG | 1 << SENSOR_CAPABILITY_COLOR_CONFIG |
                        1 << SENSOR_CAPABILITY_GAMMA_CONFIG | 1 << SENSOR_CAPABILITY_VENDOR_PARAM |
                        1 << SENSOR_CAPABILITY_BRIGHTNESS | 1<< SENSOR_CAPABILITY_CONTRAST |
                        1 << SENSOR_CAPABILITY_MIRROR),
  .sensor_custom_func =
  {
    .sensor_set_platform_func_table = &tids90ub_sensor_set_platform_func_table,
    .sensor_power_suspend = tids90ub_sensor_power_suspend,
    .sensor_power_resume = tids90ub_sensor_power_resume,
    .sensor_detect_device = &tids90ub_sensor_detect_device,
    .sensor_detect_device_channels = &tids90ub_sensor_detect_device_channels,
    .sensor_init_setting = &tids90ub_sensor_init_setting,
    .sensor_set_channel_mode = &tids90ub_sensor_set_channel_mode,
    .sensor_start_stream = &tids90ub_sensor_start_stream,
    .sensor_stop_stream = &tids90ub_sensor_stop_stream,
    .sensor_s_param = &tids90ub_sensor_s_param,
    .sensor_g_param = &tids90ub_sensor_g_param,
  },
  .use_sensor_custom_func = TRUE,
};

static const tids90ub_config_t tids90ub_default_config =
{
    .deser_type = TIDS90UB_9702,
    .num_of_cameras = 1,
    .sensors =
    {
        {.sensor_id = TIDS90UB_SENSOR_ID_OX03A10, .sensor_mode = 0, .fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED},
        {.sensor_id = TIDS90UB_SENSOR_ID_OX03A10, .sensor_mode = 0, .fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED},
        {.sensor_id = TIDS90UB_SENSOR_ID_OX03A10, .sensor_mode = 0, .fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED},
        {.sensor_id = TIDS90UB_SENSOR_ID_OX03A10, .sensor_mode = 0, .fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED}
    },
    .gpio_num = {TIDS90UB_GPIO_INVALID, TIDS90UB_GPIO_INVALID, TIDS90UB_GPIO_INVALID},
    .fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED,
    .tx_port_map = TIDS90UB_TX_PORT_1_2,
    .access_mode = I2C_FULL_ACCESS
};

static const tids90ub_sensor_info_t tids90ub_sensors_init_table[] =
{
  {
      .rxport = 0,
      .rxport_en = RXPORT_EN_RX0,
      .sensor_alias = TIDS90UB_0_SENSOR_ALIAS0,
      .serializer_alias = TIDS90UB_0_SERIALIZER_ALIAS_RX0,
  },
  {
      .rxport = 1,
      .rxport_en = RXPORT_EN_RX1,
      .sensor_alias = TIDS90UB_0_SENSOR_ALIAS1,
      .serializer_alias = TIDS90UB_0_SERIALIZER_ALIAS_RX1,
  },
  {
      .rxport = 2,
      .rxport_en = RXPORT_EN_RX2,
      .sensor_alias = TIDS90UB_0_SENSOR_ALIAS2,
      .serializer_alias = TIDS90UB_0_SERIALIZER_ALIAS_RX2,
  },
  {
      .rxport = 3,
      .rxport_en = RXPORT_EN_RX3,
      .sensor_alias = TIDS90UB_0_SENSOR_ALIAS3,
      .serializer_alias = TIDS90UB_0_SERIALIZER_ALIAS_RX3,
  },
};



#if INITIALIZE_SERIALIZER
static struct camera_i2c_reg_array tids90ub_serializer_regs[] = INIT_SERIALIZER;
static struct camera_i2c_reg_setting tids90ub_serializer_setting =
{
    .reg_array = tids90ub_serializer_regs,
    .size = STD_ARRAY_SIZE(tids90ub_serializer_regs),
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};
#endif

static struct camera_i2c_reg_array tids90ub_9702_port1_start_reg_array[] = START_REG_ARRAY_9702_PORT_1;
static struct camera_i2c_reg_array tids90ub_9702_port2_start_reg_array[] = START_REG_ARRAY_9702_PORT_2;
static struct camera_i2c_reg_array tids90ub_9702_port12_start_reg_array[] = START_REG_ARRAY_9702_PORT_1_2;
static struct camera_i2c_reg_array tids90ub_960_port1_start_reg_array[]  = START_REG_ARRAY_960_PORT_1;
static struct camera_i2c_reg_array tids90ub_960_port2_start_reg_array[]  = START_REG_ARRAY_960_PORT_2;
static struct camera_i2c_reg_array tids90ub_960_port12_start_reg_array[]  = START_REG_ARRAY_960_PORT_1_2;
static struct camera_i2c_reg_array tids90ub_stop_reg_array[]       = STOP_REG_ARRAY;
#if TIDS90UB_ENABLE_INTR_HANDLER
static struct camera_i2c_reg_array tids90ub_init_intr_reg_array[]  = INIT_INTR_ARRAY;
#endif

/**
 * Set context to default configuration
 */
static void tids90ub_set_default(tids90ub_context_t* pCtxt, const sensor_open_lib_t* device_info)
{
    unsigned int i;
    int fsync_check = 0;

    //Set default config and override if valid TIDS90UB type
    pCtxt->config = tids90ub_default_config;
    if (TIDS90UB_DEFAULT != device_info->config->type)
    {
        pCtxt->config.deser_type = device_info->config->type;
        pCtxt->config.num_of_cameras = STD_MIN(device_info->config->numSensors, TIDS90UB_PORT_MAX);
        pCtxt->config.op_mode = device_info->config->opMode;

        pCtxt->config.tx_port_map = device_info->config->socMap;
        if (device_info->config->accessMode <= I2C_NO_ACCESS)
        {
            pCtxt->config.access_mode = device_info->config->accessMode;
        }
        else
        {
            pCtxt->config.access_mode = I2C_FULL_ACCESS;
        }
        for(i = 0; i < pCtxt->config.num_of_cameras; i++)
        {
            pCtxt->config.sensors[i].sensor_id = device_info->config->sensors[i].type;
            pCtxt->config.sensors[i].sensor_mode = device_info->config->sensors[i].snsrModeId;
            pCtxt->config.sensors[i].fsync_mode = device_info->config->sensors[i].fsyncMode;
            pCtxt->config.sensors[i].fsync_freq = device_info->config->sensors[i].fsyncFreq;
            if ((TIDS90UB_FSYNC_MODE_DISABLED != pCtxt->config.sensors[i].fsync_mode) &&
                (TIDS90UB_FSYNC_MODE_EXTERNAL_SENSOR != pCtxt->config.sensors[i].fsync_mode) &&
                (TIDS90UB_FSYNC_MODE_ONESHOT != pCtxt->config.sensors[i].fsync_mode))
            {
                if (0 == fsync_check)
                {
                    fsync_check = 1;
                    pCtxt->config.fsync_mode = pCtxt->config.sensors[i].fsync_mode;
                    pCtxt->config.fsync_freq = pCtxt->config.sensors[i].fsync_freq;
                    if(i >= 1 && pCtxt->config.fsync_freq !=  pCtxt->config.sensors[i-1].fsync_freq)
                    {
                        SERR("Sensor frame sync frequency mismatch, not enabled the fsync mode %d %d",pCtxt->config.fsync_freq, pCtxt->config.sensors[i-1].fsync_freq);
                        pCtxt->config.fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED;
                        pCtxt->config.fsync_freq = TIDS90UB_FSYNC_FREQ_DEFAULT;
                    }

                }
                else
                {
                    if (pCtxt->config.fsync_mode != pCtxt->config.sensors[i].fsync_mode)
                    {
                        SERR("This combination of frame sync mode is not supported with frame sync modes %d %d",pCtxt->config.fsync_mode,pCtxt->config.sensors[i].fsync_mode);
                        pCtxt->config.fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED;
                    }
                }
            }
        }
        for (i = 0; i < MAX_NUM_INPUT_DEV_INTERNAL_GPIO; i++)
        {
            pCtxt->config.gpio_num[i] = device_info->config->gpio[i];
        }
    }

    memcpy(&pCtxt->sensors, tids90ub_sensors_init_table, sizeof(pCtxt->sensors));

    // default to dev id 0
    pCtxt->slave_addr = TIDS90UB_0_SLAVEADDR;
    pCtxt->slave_alias_group = TIDS90UB_0_SENSOR_ALIAS_GROUP;

    // override values based on device ID
    if (1 == pCtxt->subdev_id)
    {
        pCtxt->slave_addr                  = TIDS90UB_1_SLAVEADDR;
        pCtxt->slave_alias_group           = TIDS90UB_1_SENSOR_ALIAS_GROUP;
        pCtxt->sensors[0].sensor_alias     = TIDS90UB_1_SENSOR_ALIAS0;
        pCtxt->sensors[0].serializer_alias = TIDS90UB_1_SERIALIZER_ALIAS_RX0;
        pCtxt->sensors[1].sensor_alias     = TIDS90UB_1_SENSOR_ALIAS1;
        pCtxt->sensors[1].serializer_alias = TIDS90UB_1_SERIALIZER_ALIAS_RX1;
        pCtxt->sensors[2].sensor_alias     = TIDS90UB_1_SENSOR_ALIAS2;
        pCtxt->sensors[2].serializer_alias = TIDS90UB_1_SERIALIZER_ALIAS_RX2;
        pCtxt->sensors[3].sensor_alias     = TIDS90UB_1_SENSOR_ALIAS3;
        pCtxt->sensors[3].serializer_alias = TIDS90UB_1_SERIALIZER_ALIAS_RX3;
    }

    // override values based on device ID
    if (2 == pCtxt->subdev_id)
    {
        pCtxt->slave_addr                  = TIDS90UB_2_SLAVEADDR;
        pCtxt->slave_alias_group           = TIDS90UB_2_SENSOR_ALIAS_GROUP;
        pCtxt->sensors[0].sensor_alias     = TIDS90UB_2_SENSOR_ALIAS0;
        pCtxt->sensors[0].serializer_alias = TIDS90UB_2_SERIALIZER_ALIAS_RX0;
        pCtxt->sensors[1].sensor_alias     = TIDS90UB_2_SENSOR_ALIAS1;
        pCtxt->sensors[1].serializer_alias = TIDS90UB_2_SERIALIZER_ALIAS_RX1;
        pCtxt->sensors[2].sensor_alias     = TIDS90UB_2_SENSOR_ALIAS2;
        pCtxt->sensors[2].serializer_alias = TIDS90UB_2_SERIALIZER_ALIAS_RX2;
        pCtxt->sensors[3].sensor_alias     = TIDS90UB_2_SENSOR_ALIAS3;
        pCtxt->sensors[3].serializer_alias = TIDS90UB_2_SERIALIZER_ALIAS_RX3;
    }

    pCtxt->sensor_lib.sensor_slave_info.slave_addr = pCtxt->slave_addr;
    pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id = pCtxt->slave_addr;
}

/**
 * FUNCTION: tids90ub_sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
static void* tids90ub_sensor_open_lib(void* ctrl, void* arg)
{
    tids90ub_context_t* pCtxt;

    if (!ctrl || !arg)
    {
        SERR("invalid arguments");
        return NULL;
    }

    pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(tids90ub_context_t));
    if (pCtxt)
    {
        sensor_open_lib_t* device_info = (sensor_open_lib_t*)arg;
        unsigned int i;

        memset(pCtxt, 0x0, sizeof(*pCtxt));
        pCtxt->subdev_id = device_info->config->subdevId;

        if (CAMERA_SUCCESS != CameraCreateMutex(&pCtxt->mutex))
        {
            CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt);
            return NULL;
        }

        memcpy(&pCtxt->sensor_lib, &sensor_lib_ptr, sizeof(pCtxt->sensor_lib));

        pCtxt->ctrl = ctrl;
        pCtxt->sensor_lib.sensor_slave_info.camera_id = device_info->cameraId;
        pCtxt->state = TIDS90UB_STATE_INVALID;
        pCtxt->revision = TIDS90UB_REV_ID_3;
        pCtxt->rxport_en = 0xff;

        pCtxt->tids90ub_reg_setting.addr_type = CAMERA_I2C_BYTE_ADDR;
        pCtxt->tids90ub_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;


        tids90ub_set_default(pCtxt, device_info);

        pCtxt->num_supported_sensors = STD_MIN(pCtxt->config.num_of_cameras, TIDS90UB_PORT_MAX);
        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            pCtxt->sensors[i].state = TIDS90UB_SENSOR_STATE_INVALID;
        }
    }

    return pCtxt;
}

static int tids90ub_sensor_close_lib(void* ctxt)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int i;

    for (i = 0; i < TIDS90UB_PORT_MAX; i++)
    {
        if (pCtxt->sensors[i].pPrivCtxt)
        {
            CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt->sensors[i].pPrivCtxt);
        }
    }

    CameraDestroyMutex(pCtxt->mutex);
    CameraFree(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, pCtxt);

    return 0;
}


/* tids90ub_probe_sensor_serializer
 *
 * Check if far end serializer is present
 */
static int tids90ub_probe_sensor_serializer(tids90ub_context_t* pCtxt, uint32 port)
{
    int rc = 0;
    struct camera_i2c_reg_array dummy_reg[1];
    tids90ub_sensor_info_t * pSensor = &pCtxt->sensors[port];

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        /*detect far end serializer*/
        pCtxt->tids90ub_reg_setting.delay = 0;
        pCtxt->tids90ub_reg_setting.reg_array = dummy_reg;
        pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
        pCtxt->tids90ub_reg_setting.reg_array[0].delay = 0;

        pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x4C;
        pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = TIDS90UB_PORT_SEL(pCtxt->sensors[port].rxport);
        pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("Failed to set port %d", pCtxt->sensors[port].rxport);
            goto ERROR;
        }

        if (TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951 == pCtxt->config.sensors[port].sensor_id)
        {
            /* NOTE: Workaround from TI for compatibility with 951 SER */
            /* Set back channel frequency to 10 Mbps */
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x58;
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0xBA;
            pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set back channel freq for port %d", pCtxt->sensors[port].rxport);
                goto ERROR;
            }

            /* Override MODE_SEL to synchronous mode */
            rc = pSensor->pInterface->set_port_mode(pCtxt, port, IMX_GW5200_MODE_SYNC);
            if (rc)
            {
                goto ERROR;
            }
        }

        if (TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971 == pCtxt->config.sensors[port].sensor_id)
        {
            /* NOTE: Workaround from TI for compatibility with 971 SER */
            /* Set back channel frequency to 10 Mbps */
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x58;
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0xFA;
            pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set back channel freq for port %d", pCtxt->sensors[port].rxport);
                goto ERROR;
            }

            /* Override MODE_SEL to synchronous mode */
            rc = pSensor->pInterface->set_port_mode(pCtxt, port, IMX_GW5200_MODE_SYNC);
            if (rc)
            {
                goto ERROR;
            }

            /* Override MODE_SEL to synchronous mode */
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x58;
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0xBE;
            pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set back channel freq for port %d", pCtxt->sensors[port].rxport);
                goto ERROR;
            }

            /* Override MODE_SEL to synchronous mode */
            rc = pSensor->pInterface->set_port_mode(pCtxt, port, IMX_GW5200_MODE_BACKW_COMPAT);
            if (rc)
            {
                goto ERROR;
            }
        }

        if (TIDS90UB_SENSOR_ID_0X01F10_TI933  == pCtxt->config.sensors[port].sensor_id)
        {
            /* Set back channel frequency to 2.5 Mbps */
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x58;
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0x58;
            pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set back channel freq for port %d", pCtxt->sensors[port].rxport);
                goto ERROR;
            }

            if (pCtxt->config.deser_type == TIDS90UB_9702)
            {
                pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0xE4;
                pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0x05;
                pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
                rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
                if (rc)
                {
                    SERR("Failed to set FPD3 DVP Channel mode for port %d", pCtxt->sensors[port].rxport);
                    goto ERROR;
                }
                goto END;
            }
        }
        else
        {
            /* Otherwise set back channel frequency to 50 Mbps */
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x58;
            pCtxt->tids90ub_reg_setting.reg_array[0].reg_data = 0x5E;
            pCtxt->tids90ub_reg_setting.delay = _tids90ub_port_delay_;
     
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set back channel freq for port %d", pCtxt->sensors[port].rxport);
                goto ERROR;
            }
        }

        pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = 0x5B;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("Failed to read SER ID %d", pCtxt->sensors[port].rxport);
            goto ERROR;
        }

        if(TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[port].sensor_id)
        {
            /*if SER ID not 0 then far end serializer is present*/
            if (pCtxt->tids90ub_reg_setting.reg_array[0].reg_data & 0xFE)
            {
                SWARN("Detect SER 0x%x on port %d",
                    pCtxt->tids90ub_reg_setting.reg_array[0].reg_data,
                    port);
            }
            else
            {
                SWARN("failed to detect port %d", port);
                /* Far end serializer not present*/
                rc = -1;
            }
        }
    }

END:
ERROR:
    return rc;
}

/* tids90ub_set_init_sequence
 *
 * Set init sequence to setup back channel communication, aliasing, etc...
 */
static int tids90ub_set_init_sequence(tids90ub_context_t* pCtxt)
{
    struct camera_i2c_reg_array *init_seq = pCtxt->init_seq;
    tids90ub_sensor_info_t* pInitSensor = NULL;

    unsigned short init_seq_size = 0;
    unsigned short enable_port_msk = 0;
    unsigned int port = 0;
    int rc = 0;

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        /*LOCK to protect RX_SELECT register and subsequent RX operations*/
        CameraLockMutex(pCtxt->mutex);

        /*System Power-Up Reset*/
        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x01, 0x01, _tids90ub_delay_);

        /*Updated initialization sequence from TI*/
        if (pCtxt->config.deser_type == TIDS90UB_9702)
        {
            /*HW Errata workaround - System Bring-up Initialization*/
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x4C, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x02, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x09, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x00, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0A, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0B, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0C, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0D, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0E, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x0F, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x4A, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x7F, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x1C, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x3C, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0xC7, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x5C, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x47, _tids90ub_delay_);

            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x18, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x22, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x60, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x23, _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x60, _tids90ub_delay_);

            /* Phase interpolator divider. Set compatibility with ti953 ser  */
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xC2, 0x55, _tids90ub_delay_);

            if (TIDS90UB_CSI_TX_800M == pCtxt->config.op_mode)
            {
                /* Set CSI Transmitter to 800Mbps */
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x1F, 0x02, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xC9, 0x20, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x1C, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x92, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x50, _tids90ub_delay_);   
            }
            else if (TIDS90UB_CSI_TX_1P5G == pCtxt->config.op_mode)
            {
                /* Set CSI Transmitter to 1.5Gbps */
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x1F, 0x00, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xC9, 0x1E, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x1C, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x92, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x40, _tids90ub_delay_);
            }
            else
            {
                /* Set CSI Transmitter to 2.5Gbps */
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x1F, 0x10, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xC9, 0x32, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x1C, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x92, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x40, _tids90ub_delay_);

                /* HW errata workaround. Adjust timing for 2.5Gps */
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB0, 0x00,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x40,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x91,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x41,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x4C,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x42,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x98,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x43,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x44,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x92,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x45,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0xA3,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x46,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x9C,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x47,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x19,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x48,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x91,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x60,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x91,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x61,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x4C,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x62,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x98,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x63,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x14,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x64,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x92,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x65,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0xA3,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x66,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x9C,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x67,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x19,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB1, 0x68,  _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xB2, 0x91,  _tids90ub_delay_);
            }
        }

        pCtxt->init_reg_settings.reg_array = init_seq;
        pCtxt->init_reg_settings.size = init_seq_size;
        pCtxt->init_reg_settings.addr_type = CAMERA_I2C_BYTE_ADDR;
        pCtxt->init_reg_settings.data_type = CAMERA_I2C_BYTE_DATA;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->init_reg_settings);

        init_seq_size = 0;

        for (port = 0; port < pCtxt->num_supported_sensors; port++)
        {
            tids90ub_sensor_info_t * pSensor = &pCtxt->sensors[port];

            switch (pCtxt->config.sensors[port].sensor_id)
            {
            case TIDS90UB_SENSOR_ID_OX03A10:
                pSensor->pInterface = ox03a10_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI953:
            case TIDS90UB_SENSOR_ID_IMX490_GW5200_TI953:
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951:
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = imx_gw5200_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_X3A_OV491_TI935:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = x3a_ov491_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_PATTERN_GEN:
            case TIDS90UB_SENSOR_ID_SER_PATTERN_GEN:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = tids90ub_pattern_gen_get_sensor_info();
                break;
            default:
                SERR("Slave ID %d NOT SUPPORTED", pCtxt->config.sensors[port].sensor_id);
                /* no break */
            case TIDS90UB_SENSOR_ID_INVALID:
                pSensor->state = TIDS90UB_SENSOR_STATE_INVALID;
                break;
            }

            if (!pSensor->pInterface)
            {
                continue;
            }

            rc = tids90ub_probe_sensor_serializer(pCtxt, port);
            if (rc)
            {
                continue;
            }

            rc = pSensor->pInterface->get_port_cfg(pCtxt, port, &pSensor->port_cfg);
            pSensor->state = TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED;

            if (!pInitSensor)
            {
                pInitSensor = pSensor;
            }

            enable_port_msk |= (1 << (pSensor->rxport));
        }

        //Clearing reg_setting as it was set to a local variable
        pCtxt->tids90ub_reg_setting.reg_array = NULL;
        pCtxt->tids90ub_reg_setting.delay = 0;

        CameraUnlockMutex(pCtxt->mutex);

        if (!pInitSensor)
        {
            SHIGH("No far end serializers detected");
            return 0;
        }

        /**
         * @TODO: support different combination of sensors
         * For now default init to sensor connected at 0
         */
        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x4c, 0x0f, _tids90ub_delay_);
        memcpy(&init_seq[init_seq_size],
            pInitSensor->port_cfg.deser_config,
            pInitSensor->port_cfg.deser_config_size * sizeof(pInitSensor->port_cfg.deser_config[0]));

        init_seq_size += pInitSensor->port_cfg.deser_config_size;
        port = 0;
        if(TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[port].sensor_id)
        {
            for (port = 0; port < pCtxt->num_supported_sensors; port++)
            {
                tids90ub_sensor_info_t * pSensor = &pCtxt->sensors[port];

                if (pSensor->state != TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED)
                {
                    continue;
                }

                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x4c, TIDS90UB_PORT_SEL(pSensor->rxport), _tids90ub_delay_);

                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x5c, pSensor->serializer_alias, _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x65, pSensor->sensor_alias, _tids90ub_delay_);

                if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    /*Select VC-ID for CSI_VC_MAP 0 */
                    ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0xA0,
                        pSensor->port_cfg.sources[0].channel_info.vc, _tids90ub_delay_);

                    /*Config VC-DT for 0X01F10_TI933 camera per port */
                    if (TIDS90UB_SENSOR_ID_0X01F10_TI933 == pCtxt->config.sensors[port].sensor_id)
                    {
                        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x70, (((port & 0x3) << 6) | (0x1e)),  _tids90ub_delay_);
                    }
                }

                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    if (pSensor->port_cfg.num_sources == 1)
                        /*Select VC-ID for CSI_VC_MAP 0 */
                        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x72,
                             pSensor->port_cfg.sources[0].channel_info.vc, _tids90ub_delay_);
                    else
                        /*keep VC-ID */
                        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x72,
                            0xE4, _tids90ub_delay_);


                    /*Config VC-DT for 0X01F10_TI933 camera per port */
                    if (TIDS90UB_SENSOR_ID_0X01F10_TI933 == pCtxt->config.sensors[port].sensor_id)
                    {
                        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x70, (((port & 0x3) << 6) | (0x1e)),  _tids90ub_delay_);
                    }
                }
            }

            /* Disable unused RX ports  */
            ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x0C, enable_port_msk, _tids90ub_delay_);

            /*Debug dump all init sequence*/
            for (port = 0; port < init_seq_size; port++)
            {
                SHIGH("%d - 0x%x, 0x%x, %d", port,
                    init_seq[port].reg_addr, init_seq[port].reg_data, init_seq[port].delay);
            }

            pCtxt->init_reg_settings.reg_array = init_seq;
            pCtxt->init_reg_settings.size = init_seq_size;
            pCtxt->init_reg_settings.addr_type = CAMERA_I2C_BYTE_ADDR;
            pCtxt->init_reg_settings.data_type = CAMERA_I2C_BYTE_DATA;
            /*LOCK to protect RX_SELECT register and subsequent RX operations*/
            CameraLockMutex(pCtxt->mutex);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->init_reg_settings);
            CameraUnlockMutex(pCtxt->mutex);

            if (rc)
            {
                SERR("Failed to set init sequence");
            }
        }
    }
    else if (I2C_READ_ACCESS == pCtxt->config.access_mode)
    {
        /*LOCK to protect RX_SELECT register and subsequent RX operations*/
        CameraLockMutex(pCtxt->mutex);

        init_seq_size = 0;

        for (port = 0; port < pCtxt->num_supported_sensors; port++)
        {
            tids90ub_sensor_info_t * pSensor = &pCtxt->sensors[port];

            switch (pCtxt->config.sensors[port].sensor_id)
            {
            case TIDS90UB_SENSOR_ID_OX03A10:
                pSensor->pInterface = ox03a10_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI953:
            case TIDS90UB_SENSOR_ID_IMX490_GW5200_TI953:
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951:
            case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = imx_gw5200_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_X3A_OV491_TI935:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = x3a_ov491_get_sensor_info();
                break;
            case TIDS90UB_SENSOR_ID_PATTERN_GEN:
            case TIDS90UB_SENSOR_ID_SER_PATTERN_GEN:
                pSensor->port_cfg.sensor_id = pCtxt->config.sensors[port].sensor_id;
                pSensor->pInterface = tids90ub_pattern_gen_get_sensor_info();
                break;
            default:
                SERR("Slave ID %d NOT SUPPORTED", pCtxt->config.sensors[port].sensor_id);
                /* no break */
            case TIDS90UB_SENSOR_ID_INVALID:
                pSensor->state = TIDS90UB_SENSOR_STATE_INVALID;
                break;
            }

            if (!pSensor->pInterface)
            {
                continue;
            }

            rc = pSensor->pInterface->get_port_cfg(pCtxt, port, &pSensor->port_cfg);
            pSensor->state = TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED;

            if (!pInitSensor)
            {
                pInitSensor = pSensor;
            }

            enable_port_msk |= (1 << (pSensor->rxport));
        }

        //Clearing reg_setting as it was set to a local variable
        pCtxt->tids90ub_reg_setting.reg_array = NULL;
        pCtxt->tids90ub_reg_setting.delay = 0;

        CameraUnlockMutex(pCtxt->mutex);

        if (!pInitSensor)
        {
            SHIGH("No far end serializers detected");
            return 0;
        }

        /**
         * @TODO: support different combination of sensors
         * For now default init to sensor connected at 0
         */
        ADD_I2C_REG_ARRAY(init_seq, init_seq_size, 0x4c, 0x0f, _tids90ub_delay_);
        memcpy(&init_seq[init_seq_size],
            pInitSensor->port_cfg.deser_config,
            pInitSensor->port_cfg.deser_config_size * sizeof(pInitSensor->port_cfg.deser_config[0]));

        init_seq_size += pInitSensor->port_cfg.deser_config_size;

        if(TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[port].sensor_id)
        {
            /*Debug dump all init sequence*/
            for (port = 0; port < init_seq_size; port++)
            {
                SHIGH("%d - 0x%x, 0x%x, %d", port,
                    init_seq[port].reg_addr, init_seq[port].reg_data, init_seq[port].delay);
            }
        }
    }

    return rc;
}

#if TIDS90UB_ENABLE_INTR_HANDLER
static int tids90ub_enable_intr_pin(tids90ub_context_t* pCtxt)
{
    int rc = 0;
    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
#if DISABLE_INTR_ON_SLEEP
        struct camera_i2c_reg_array tids90ub_enable_intr[] = {{ 0x23, 0x8f, 0 },};

        pCtxt->tids90ub_reg_setting.reg_array = tids90ub_enable_intr;
        pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_enable_intr);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("tids90ub 0x%x failed to stop", pCtxt->slave_addr);
        }
#endif
    }
    return rc;
}

static int tids90ub_disable_intr_pin(tids90ub_context_t* pCtxt)
{
    int rc = 0;
    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
#if DISABLE_INTR_ON_SLEEP
        struct camera_i2c_reg_array tids90ub_disable_intr[] = {{ 0x23, 0x0, 0 },};

        pCtxt->tids90ub_reg_setting.reg_array = tids90ub_disable_intr;
        pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_disable_intr);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("tids90ub 0x%x failed to stop", pCtxt->slave_addr);
        }
#endif
    }

    return rc;
}
#endif

static int tids90ub_sensor_power_suspend(void* ctxt,  CameraPowerEventType powerEventId)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_NO_ACCESS != pCtxt->config.access_mode)
    {
        (void)powerEventId;
#if TIDS90UB_ENABLE_INTR_HANDLER
        if (TIDS90UB_960 == pCtxt->config.deser_type && TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[0].sensor_id)
        {
            int num_tries = 0;

            tids90ub_disable_intr_pin(pCtxt);

            pCtxt->state = TIDS90UB_STATE_SUSPEND;

            while (pCtxt->intr_in_process && num_tries < 3)
            {
                CameraSleep(TIDS90UB_INTR_SETTLE_SLEEP);
                num_tries++;
            }
        }
#else
        pCtxt->state = TIDS90UB_STATE_SUSPEND;
#endif
    }

    return rc;
}

static int tids90ub_sensor_power_resume(void* ctxt,  CameraPowerEventType powerEventId)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_NO_ACCESS != pCtxt->config.access_mode)
    {
        (void)powerEventId;

        pCtxt->state = TIDS90UB_STATE_INITIALIZED;
#if TIDS90UB_ENABLE_INTR_HANDLER
        if (TIDS90UB_960 == pCtxt->config.deser_type && TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[0].sensor_id)
        {
            rc = tids90ub_enable_intr_pin(pCtxt);

            tids90ub_intr_handler(pCtxt);
        }
#endif
    }

    return rc;
}

static int tids90ub_sensor_detect_device(void* ctxt)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_NO_ACCESS != pCtxt->config.access_mode)
    {
        struct camera_i2c_reg_array dummy_reg[1];

        SLOW("detect tids90ub 0x%x", pCtxt->slave_addr);

        if (pCtxt->state >= TIDS90UB_STATE_DETECTED)
        {
            SLOW("already detected");
            return 0;
        }

        if (!pCtxt->platform_fcn_tbl_initialized)
        {
            SERR("I2C function table not initialized");
            return -1;
        }

        /*read bridge chip slave ID*/
        pCtxt->tids90ub_reg_setting.delay = 0;
        pCtxt->tids90ub_reg_setting.reg_array = dummy_reg;
        pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);

#if defined(__INTEGRITY) || defined(__QNXNTO__)
        /* TODO - time is randomaly selected to detect de-serializer, optimization is required */
        CameraSleep(TIDS960_INIT_SLEEP);
#endif

        pCtxt->tids90ub_reg_setting.reg_array[0].delay = 0;
        pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id_reg_addr;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting)))
        {
                SERR("Unable to read chip ID");
                goto error;
        }

        if (pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id != pCtxt->tids90ub_reg_setting.reg_array[0].reg_data)
        {
            SERR("Chip ID does not match. Expect 0x%x, Read back 0x%x",
                pCtxt->sensor_lib.sensor_slave_info.sensor_id_info.sensor_id,
                pCtxt->tids90ub_reg_setting.reg_array[0].reg_data);
            rc = -1;
            goto error;
        }

        /*read bridge chip revision ID*/
        pCtxt->tids90ub_reg_setting.reg_array[0].reg_addr = TIDS90UB_REV_ADDR;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting)))
        {
                SERR("Unable to read bridge chip revision ID");
                goto error;
        }

        pCtxt->revision = pCtxt->tids90ub_reg_setting.reg_array[0].reg_data;
        if (TIDS90UB_960 == pCtxt->config.deser_type)
        {
            switch(pCtxt->revision)
            {
            case TIDS90UB_REV_ID_1:
            case TIDS90UB_REV_ID_2:
            {
                rc = -1;
                SERR("tids90ub rev %d unsupported. PLEASE UPGRADE HW TO TI960 REV 3", pCtxt->revision);
                goto error;
            }
            case TIDS90UB_REV_ID_3:
                SHIGH("tids90ub rev 3 detected.");
                break;
            case TIDS90UB_REV_ID_4:
                SHIGH("tids90ub rev 4 detected. You have the latest known good TI960 HW.");
                break;
            default:
            {
                rc = -1;
                SERR("tids90ub unknown rev found 0x%x", pCtxt->revision);
                goto error;
            }
            }
        }

        pCtxt->state = TIDS90UB_STATE_DETECTED;
    }

error:
    return rc;
}

#if TIDS90UB_ENABLE_INTR_HANDLER
static int tids90ub_read_int_sts(tids90ub_context_t* pCtxt, unsigned short *intr_src_mask)
{
    int rc = 0;
    unsigned short inter_ports = 0;

    int i = 0;
    struct camera_i2c_reg_setting tids90ub_reg_setting;
    struct camera_i2c_reg_array dummy_reg[1];

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (intr_src_mask == NULL)
        {
            SERR("Invalid src mask");
            return -1;
        }

        tids90ub_reg_setting.delay = 0;
        tids90ub_reg_setting.addr_type = CAMERA_I2C_BYTE_ADDR;
        tids90ub_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
        tids90ub_reg_setting.reg_array = dummy_reg;
        tids90ub_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
        tids90ub_reg_setting.reg_array[0].delay = 0;
        tids90ub_reg_setting.reg_array[0].reg_addr = 0x24; //INTS register

        rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &tids90ub_reg_setting);
        if (rc)
        {
            SERR("Failed to read INTS register");
            return -rc;

        }
        else
        {
            SLOW("read INT status as = 0x%x", tids90ub_reg_setting.reg_array[0].reg_data);
        }

        for (i = 0; i < 4; i++)
        {
            if (tids90ub_reg_setting.reg_array[0].reg_data & (1 << i))
            {
                inter_ports |= 1 << i;
            }
        }

        *intr_src_mask = inter_ports;
    }

    return rc;
}
#endif

static int tids90ub_read_lock_pass_status(tids90ub_context_t* pCtxt, boolean* lock_chg, boolean* lock_pass, uint32 rx_port)
{
    int rc = 0;

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if(TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[rx_port].sensor_id)
        {
            unsigned short sts0_data, sts1_data, sts2_data;
            struct camera_i2c_reg_setting tids90ub_reg_setting;
            struct camera_i2c_reg_array dummy_reg[1];

            tids90ub_reg_setting.delay = 0;
            tids90ub_reg_setting.addr_type = CAMERA_I2C_BYTE_ADDR;
            tids90ub_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
            tids90ub_reg_setting.reg_array = dummy_reg;
            tids90ub_reg_setting.size = STD_ARRAY_SIZE(dummy_reg);
            tids90ub_reg_setting.reg_array[0].delay = 0;
            tids90ub_reg_setting.reg_array[0].reg_addr = 0x4C;
            tids90ub_reg_setting.reg_array[0].reg_data = TIDS90UB_PORT_SEL(rx_port);

            /*LOCK to protect RX_SELECT register and subsequent RX operations*/
            CameraLockMutex(pCtxt->mutex);

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                        pCtxt->ctrl,
                        pCtxt->slave_addr,
                        &tids90ub_reg_setting);
            if (rc)
            {
                SERR("Failed to set port %d ", rx_port);
            }
            else
            {
                //Read Port status register
                tids90ub_reg_setting.reg_array[0].reg_addr = 0xDB;
                rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &tids90ub_reg_setting);
                sts0_data = tids90ub_reg_setting.reg_array[0].reg_data;

                tids90ub_reg_setting.reg_array[0].reg_addr = 0x4D; //RX_PORT_STS1
                rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &tids90ub_reg_setting);
                sts1_data = tids90ub_reg_setting.reg_array[0].reg_data;

                tids90ub_reg_setting.reg_array[0].reg_addr = 0x4E; //RX_PORT_STS2
                rc |= pCtxt->platform_fcn_tbl.i2c_slave_read(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &tids90ub_reg_setting);
                sts2_data = tids90ub_reg_setting.reg_array[0].reg_data;
                if (rc)
                {
                    SERR("Failed to read RX_PORT_STATUS for port %d", rx_port);
                }

                *lock_chg = (sts0_data & 0x03) ? TRUE : FALSE;
                *lock_pass = ((sts1_data & 0x03) == 0x3) ? TIDS90UB_SIGNAL_LOCKED : TIDS90UB_SIGNAL_LOST;
                SLOW("read RX port %d status %d %d [0x%x, 0x%x, 0x%x]", rx_port,
                                            *lock_chg, *lock_pass,
                                            sts0_data, sts1_data, sts2_data);

                if (sts2_data & 0x80)
                {
                    SERR("RX port %d: LINE LEN UNSTABLE", rx_port);
                }
            }
        }
        else
        {
            *lock_chg = TRUE;
            *lock_pass = TIDS90UB_SIGNAL_LOCKED;
        }
        CameraUnlockMutex(pCtxt->mutex);
    }

    return rc;
}


static int tids90ub_read_status(tids90ub_context_t* pCtxt, uint32 port)
{
    boolean lock_sts = 0;
    boolean lock_sts_chg = 0;
    int rc = 0;
    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        rc = tids90ub_read_lock_pass_status(pCtxt, &lock_sts_chg, &lock_sts, port);
        if (rc < 0)
        {
            SERR("tids90ub_read_status failed for port %d.", port);
        }
        else
        {
            pCtxt->sensors[port].lock_status = lock_sts;
        }
    }

    return rc;
}

#if TIDS90UB_ENABLE_INTR_HANDLER
static void tids90ub_intr_handler(void* data)
{
    int i = 0;
    int rc = 0;
    CameraInputEventPayloadType payload = {};
    boolean lock_sts_chg[4] = {0,0,0,0};
    boolean lock_sts_chg2[4] = {0,0,0,0};
    boolean lock_sts[4] = {0,0,0,0};
    unsigned short intr_srcs = 0;

    tids90ub_context_t* pCtxt = (tids90ub_context_t*)data;

    if(pCtxt == NULL)
    {
        SERR("Invalid tids90ub context");
        return;
    }

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (pCtxt->state < TIDS90UB_STATE_INITIALIZED)
        {
            SERR("Invalid tids90ub context state");
            return;
        }

        if (pCtxt->state == TIDS90UB_STATE_SUSPEND)
        {
            SLOW("Received interrupt in suspend state");
            return;
        }

        pCtxt->intr_in_process = TRUE;
        SERR("tids90ub_intr_handler %x!", pCtxt->slave_addr);

        rc = tids90ub_read_int_sts(pCtxt, &intr_srcs);
        if (rc < 0)
        {
            pCtxt->intr_in_process = FALSE;
            SERR("Failed to read INTS status");
            return;
        }

        while (intr_srcs & 0x0F)
        {
            /* Read the PORT status for all ports */
            for (i = 0; i < 4; i++)
            {
                if (!(intr_srcs & (1 << i)))
                    continue;

                rc = tids90ub_read_lock_pass_status(pCtxt, &lock_sts_chg[i], &lock_sts[i], i);
                if (rc < 0)
                {
                    SERR("lock_pass_status read failed for port %d", i);
                    lock_sts_chg[i] = 1;
                    lock_sts[i] = TIDS90UB_SIGNAL_LOST;
                }
            }

            /* Send LOST if we lost lock */
            for (i = 0; i < 4; i++)
            {
                if (!(intr_srcs & (1 << i)))
                    continue;

                if (lock_sts_chg[i] && pCtxt->sensors[i].lock_status == TIDS90UB_SIGNAL_LOCKED)
                {
                    payload.src_id = i;
                    payload.lock_status = QCARCAM_INPUT_SIGNAL_LOST;
                    if (pCtxt->state == TIDS90UB_STATE_STREAMING)
                    {
                        SERR("QCARCAM_INPUT_SIGNAL_LOST port %d", i);
                        pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);
                    }
                }

                pCtxt->sensors[i].lock_status = lock_sts[i];
            }

            CameraSleep(TIDS90UB_INTR_SETTLE_SLEEP);

            rc = tids90ub_read_int_sts(pCtxt, &intr_srcs);
            if (rc < 0)
            {
                pCtxt->intr_in_process = FALSE;
                SERR("Failed to read INTS status");
                return;
            }

            /* Read the PORT status for all ports again*/
            for (i = 0; i < 4; i++)
            {
                if (!(intr_srcs & (1 << i)))
                    continue;

                rc = tids90ub_read_lock_pass_status(pCtxt, &lock_sts_chg2[i], &lock_sts[i], i);
                if (rc < 0)
                {
                    SERR("lock_pass_status read failed for port %d.", i);
                    lock_sts_chg2[i] = 1;
                    lock_sts[i] = TIDS90UB_SIGNAL_LOST;
                }
            }

            /* Send LOST if we lost lock */
            for (i = 0; i < 4; i++)
            {
                //if we lost signal again, send signal lost event
                if (lock_sts_chg2[i] && pCtxt->sensors[i].lock_status == TIDS90UB_SIGNAL_LOCKED)
                {
                    payload.src_id = i;
                    payload.lock_status = QCARCAM_INPUT_SIGNAL_LOST;
                    if (pCtxt->state == TIDS90UB_STATE_STREAMING)
                    {
                        SERR("QCARCAM_INPUT_SIGNAL_LOST port %d", i);
                        pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);
                    }
                }

                //assign new lock status
                if (intr_srcs & (1 << i))
                    pCtxt->sensors[i].lock_status = lock_sts[i];

                //if status ever changed and we are now locked, send signal valid
                if ((lock_sts_chg[i] || lock_sts_chg2[i]) && (pCtxt->sensors[i].lock_status == TIDS90UB_SIGNAL_LOCKED))
                {
                    payload.src_id = i;
                    payload.lock_status = QCARCAM_INPUT_SIGNAL_VALID;
                    pCtxt->platform_fcn_tbl.event_cb(pCtxt->ctrl, INPUT_EVENT_LOCK_STATUS, &payload);
                }
            }

            rc = tids90ub_read_int_sts(pCtxt, &intr_srcs);
            if (rc < 0)
            {
                SERR("Failed to read INTS status: %d", rc);
                return;
            }
        }

        pCtxt->intr_in_process = FALSE;
    }
}
#endif

static int tids90ub_set_fsync_mode(tids90ub_context_t* pCtxt)
{
    int rc = 0;
    unsigned short reg_seq_size = 0;
    unsigned int port = 0;
    unsigned int fsync_mode = pCtxt->config.fsync_mode;
    unsigned int fsync_freq = pCtxt->config.fsync_freq;

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        struct camera_i2c_reg_array reg_seq[TIDS90UB_FSYNC_REG_ARRAY_SIZE];

        /* Configure GPIO */
        if (TIDS90UB_VALID_GPIO_CHECK(pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_IN]))
        {
            if ((TIDS90UB_FSYNC_MODE_EXTERNAL == fsync_mode) ||
                (TIDS90UB_FSYNC_MODE_EXTERNAL_SOC == fsync_mode))
            {
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x10 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_IN], 0x90, _tids90ub_delay_);
            }
            else
            {
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x10 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_IN], 0x91, _tids90ub_delay_);
            }
        }
        else
        {
            SERR("tids90ub 0x%x fsync GPIO In is not provided", pCtxt->slave_addr);
        }

        if (TIDS90UB_VALID_GPIO_CHECK(pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_OUT_DESERIALIZER]))
        {
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x10 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_OUT_DESERIALIZER], 0x91, _tids90ub_delay_);
        }
        else
        {
            SLOW("tids90ub 0x%x fsync GPIO Out Deserializer is not provided", pCtxt->slave_addr);
        }

        if (TIDS90UB_VALID_GPIO_CHECK(pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_OUT_SOC]))
        {
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x10 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_OUT_SOC], 0x91, _tids90ub_delay_);
        }
        else
        {
            SLOW("tids90ub 0x%x fsync GPIO Out SOC is not provided", pCtxt->slave_addr);
        }

        for (port = 0; port < pCtxt->num_supported_sensors; port++)
        {
            if(TIDS90UB_FSYNC_FREQ_DEFAULT == pCtxt->config.sensors[port].fsync_freq && fsync_mode && pCtxt->sensors[port].port_cfg.sources[0].res.fps)
            {
                fsync_freq = FSYNC_FPS_TO_FREQ(pCtxt->sensors[port].port_cfg.sources[0].res.fps);
                if(pCtxt->config.fsync_freq &&  pCtxt->config.fsync_freq != fsync_freq)
                {
                    SERR("Sensor frame sync frequency mismatch, disable the fsync mode, freq to default %d %d ",pCtxt->config.fsync_freq, fsync_freq);
                    fsync_mode = TIDS90UB_FSYNC_MODE_DISABLED;
                    fsync_freq = TIDS90UB_FSYNC_FREQ_DEFAULT;
                }
                else
                {
                    pCtxt->config.fsync_freq = FSYNC_FPS_TO_FREQ(pCtxt->sensors[port].port_cfg.sources[0].res.fps);
                }
            }
        }
        switch(fsync_freq)
        {
            case TIDS90UB_FSYNC_FREQ_10HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1A, 0x01, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x45, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x85, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1A, 0x01, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x55, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xCC, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_15HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0xD9, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x03, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0xE3, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xDD, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_20HZ:
                 /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0xA2, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xC2, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0xAA, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xE6, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_25HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x82, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x35, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x88, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xB8, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_30HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x6C, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x82, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x71, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xEF, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_35HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x5D, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x01, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x61, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xA8, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_40HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x51, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x61, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x55, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x73, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_45HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x48, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x56, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x4B, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xF4, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_50HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x41, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x1B, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x44, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x5C, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_55HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x3B, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x2F, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x3E, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x25, _tids90ub_delay_);
                }
                break;
            case TIDS90UB_FSYNC_FREQ_60HZ:
                /* Configure Fsync pulse timing */
                if (TIDS90UB_960 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x36, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0x40, _tids90ub_delay_);
                }
                else if (TIDS90UB_9702 == pCtxt->config.deser_type)
                {
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1B, 0x38, _tids90ub_delay_);
                    ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x1C, 0xF7, _tids90ub_delay_);
                }
                break;

            default:
                SERR("tids90ub 0x%x fsync mode not supported", pCtxt->slave_addr);
                rc = 1;
                break;
        }

        switch(fsync_mode)
        {
            case TIDS90UB_FSYNC_MODE_INTERNAL:
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x18, 0x03, _tids90ub_delay_);
                break;
            case TIDS90UB_FSYNC_MODE_EXTERNAL:
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x18, (0x8 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_IN]) << 0x4, _tids90ub_delay_);
                break;
            case TIDS90UB_FSYNC_MODE_EXTERNAL_SOC:
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x18, (0x8 + pCtxt->config.gpio_num[TIDS90UB_GPIO_FSYNC_IN]) << 0x4, _tids90ub_delay_);
                /* This will be revised to match frequency */
                rc = pCtxt->platform_fcn_tbl.setup_cci_frame_sync(pCtxt->ctrl, FRAME_SYNC_FREQ_TO_FPS(fsync_freq));
                if (rc)
                {
                    SERR("Failed to setup cci frame sync 0x%x", pCtxt->slave_addr);
                }
                break;
            case TIDS90UB_FSYNC_MODE_EXTERNAL_SENSOR:
                /* TODO */
                break;

            default:
                SERR("tids90ub 0x%x fsync mode not supported", pCtxt->slave_addr);
                rc = 1;
                break;
        }

        if (!rc)
        {
            pCtxt->tids90ub_reg_setting.reg_array = reg_seq;
            pCtxt->tids90ub_reg_setting.size = reg_seq_size;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("tids90ub 0x%x failed to set fsync mode", pCtxt->slave_addr);
            }
        }
    }

    return rc;
}

static int tids90ub_set_fsync_rxport(tids90ub_context_t* pCtxt, uint32 port)
{
    int rc = 0;
    unsigned short reg_seq_size = 0;
    unsigned int sensor_fsync_mode = pCtxt->config.sensors[port].fsync_mode;

    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_SENSOR_ID_OV2310_TI935 == pCtxt->config.sensors[port].sensor_id)
        {
            SWARN("frame sync not suppoted for the sensor %d", pCtxt->config.sensors[port].sensor_id);
        }
        else if (TIDS90UB_FSYNC_MODE_DISABLED != sensor_fsync_mode)
        {
            struct camera_i2c_reg_array reg_seq[TIDS90UB_FSYNC_REG_ARRAY_SIZE];
            tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

            /* Configure port and back channel GPIO*/
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x4c, TIDS90UB_PORT_SEL(port), _tids90ub_delay_);
            if (TIDS90UB_FSYNC_MODE_ONESHOT == sensor_fsync_mode)
            {
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x6e, 0x09, _tids90ub_delay_);
            }
            else
            {
                ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x6e, 0x0A, _tids90ub_delay_);
            }


            pCtxt->tids90ub_reg_setting.reg_array = reg_seq;
            pCtxt->tids90ub_reg_setting.size = reg_seq_size;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("tids90ub 0x%x port %d failed to set rxport for fsync", pCtxt->slave_addr, port);
            }

            if (pSensor->pInterface->set_port_fsync)
            {
                /* Set Fsync settings for sensor */
                rc = pSensor->pInterface->set_port_fsync(pCtxt, port);
            }
        }
    }

    return rc;
}

static int tids90ub_set_manual_eq(tids90ub_context_t* pCtxt, uint32 port)
{
    int rc = 0;
    if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        CAM_UNUSED(pCtxt);
        CAM_UNUSED(port);
#if 0
        unsigned short reg_seq_size = 0;

        if (TIDS90UB_9702 == pCtxt->config.deser_type)
        {
            struct camera_i2c_reg_array reg_seq[TIDS90UB_MANUAL_EQ_ARRAY_SIZE];
            tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

            /* Configure manual EQ settings to max settings*/
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x4C, TIDS90UB_PORT_SEL(pSensor->rxport),  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB0, TIDS90UB_IAR_SEL(pSensor->rxport),  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x21,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0x2A,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0x87, 0x45,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x00,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0x87,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x01,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0x3F,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x02,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0xF7,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x06,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0x42,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB1, 0x14,  _tids90ub_delay_);
            ADD_I2C_REG_ARRAY(reg_seq, reg_seq_size, 0xB2, 0xA0,  _tids90ub_delay_);

            pCtxt->tids90ub_reg_setting.reg_array = reg_seq;
            pCtxt->tids90ub_reg_setting.size = reg_seq_size;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
        }
#endif
    }

    return rc;
}

static int tids90ub_sensor_detect_device_channels(void* ctxt)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;
    unsigned int port = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        SLOW("initialize tids90ub 0x%x with %d sensors",
            pCtxt->slave_addr,
            pCtxt->num_supported_sensors);

        if (pCtxt->state >= TIDS90UB_STATE_INITIALIZED)
        {
            SERR("already detected %d out of %d",
                pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
            return 0;
        }
        else if (pCtxt->state != TIDS90UB_STATE_DETECTED)
        {
            SERR("tids90ub 0x%x not detected - wrong state", pCtxt->slave_addr);
            return -1;
        }

        pCtxt->enable_intr = FALSE;
#if TIDS90UB_ENABLE_INTR_HANDLER
        if (TIDS90UB_960 == pCtxt->config.deser_type && TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[port].sensor_id)
        {
            rc = pCtxt->platform_fcn_tbl.setup_gpio_interrupt(pCtxt->ctrl,
                    CAMERA_GPIO_INTR, tids90ub_intr_handler, pCtxt);
        }
#endif

        /*Set init sequence to setup back channel communication, aliasing, etc... */
        rc = tids90ub_set_init_sequence(pCtxt);
        if (!rc)
        {
            pCtxt->num_connected_sensors = 0;
            for (port = 0; port < pCtxt->num_supported_sensors; port++)
            {
                tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
                if (pSensor->state != TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED)
                    continue;

                rc = pSensor->pInterface->detect(pCtxt, port);
                if (!rc)
                {
                    pCtxt->rxport_en &= pSensor->rxport_en;
                    pCtxt->num_connected_sensors++;
                    SHIGH("detected camera on port %d on bridge chip 0x%x", port, pCtxt->slave_addr);

                    /*read initial status of the sensor*/
                    rc = tids90ub_read_status(pCtxt, port);
                    if (rc)
                    {
                        SERR("sensor 0x%x could not read initial status", pSensor->sensor_alias);
                    }

                    /*Configure manual EQ settings*/
                    rc = tids90ub_set_manual_eq(pCtxt, port);
                    if (rc)
                    {
                        SERR("sensor 0x%x could set manual EQ settings", pSensor->sensor_alias);
                    }

                    /* Configure rxport for fsync */
                    if ((pCtxt->config.fsync_mode > TIDS90UB_FSYNC_MODE_DISABLED) && (pCtxt->config.fsync_mode < TIDS90UB_FSYNC_MODE_MAX))
                    {
                        rc = tids90ub_set_fsync_rxport(pCtxt, port);
                        if (rc)
                        {
                            SERR("sensor 0x%x rxport fsync configuration failed", pSensor->sensor_alias);
                        }
                    }

                    if (!rc)
                    {
                        for (unsigned int subChannel = 0; subChannel < pSensor->port_cfg.num_sources; subChannel++)
                        {
                            img_src_channel_t *pChannel = &pCtxt->sensor_lib.channels[pCtxt->sensor_lib.num_channels];
                            img_src_subchannel_t *pSubChannel = &pCtxt->sensor_lib.subchannels[pCtxt->sensor_lib.num_subchannels];
                            img_src_subchannel_layout_t layout = {
                                .src_id = pCtxt->sensor_lib.num_subchannels,
                            };

                            pChannel->num_subchannels = 1;
                            pChannel->output_mode = pSensor->port_cfg.sources[subChannel];
                            pChannel->subchan_layout[0] = layout;

                            pSubChannel->src_id = pCtxt->sensor_lib.num_subchannels;
                            pCtxt->sensor_lib.src_id_enable_mask |= (1 << pSubChannel->src_id);
                            pSubChannel->modes[0] = pSensor->port_cfg.sources[subChannel];
                            pSubChannel->num_modes = 1;

                            pCtxt->sensor_lib.num_channels++;
                            pCtxt->sensor_lib.num_subchannels++;
                        }
                    }
                }
            }

            /* Configure fsync mode*/
            if ((pCtxt->config.fsync_mode > TIDS90UB_FSYNC_MODE_DISABLED) && (pCtxt->config.fsync_mode < TIDS90UB_FSYNC_MODE_MAX))
            {
                rc = tids90ub_set_fsync_mode(pCtxt);
                if (rc)
                {
                    SERR("tids90ub 0x%x failed to configure frame sync mode", pCtxt->slave_addr);
                }
            }

            pCtxt->state = TIDS90UB_STATE_INITIALIZED;

#if TIDS90UB_ENABLE_INTR_HANDLER
            if (TIDS90UB_960 == pCtxt->config.deser_type && TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[0].sensor_id)
            {
                /**setup interrupts**/
                pCtxt->enable_intr = TRUE;

                pCtxt->tids90ub_reg_setting.reg_array = tids90ub_init_intr_reg_array;
                pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_init_intr_reg_array);
                rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                        pCtxt->ctrl,
                        pCtxt->slave_addr,
                        &pCtxt->tids90ub_reg_setting);
                if (rc)
                {
                    SERR("tids90ub 0x%x failed to init interrupt", pCtxt->slave_addr);
                }
            }
#endif
            /********************/

            SWARN("tids90ub detected %d out of %d",
                   pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
        }

        CameraLogEvent(CAMERA_SENSOR_EVENT_PROBED, rc, pCtxt->num_connected_sensors);
    }
    else if (I2C_READ_ACCESS == pCtxt->config.access_mode)
    {
        SLOW("initialize tids90ub 0x%x with %d sensors",
            pCtxt->slave_addr,
            pCtxt->num_supported_sensors);

        if (pCtxt->state >= TIDS90UB_STATE_INITIALIZED)
        {
            SERR("already detected %d out of %d",
                pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
            return 0;
        }
        else if (pCtxt->state != TIDS90UB_STATE_DETECTED)
        {
            SERR("tids90ub 0x%x not detected - wrong state", pCtxt->slave_addr);
            return -1;
        }

        pCtxt->enable_intr = FALSE;

        /*Set init sequence to setup back channel communication, aliasing, etc... */
        rc = tids90ub_set_init_sequence(pCtxt);
        if (!rc)
        {
            pCtxt->num_connected_sensors = 0;
            for (port = 0; port < pCtxt->num_supported_sensors; port++)
            {
                tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
                if (pSensor->state != TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED)
                    continue;

                rc = pSensor->pInterface->detect(pCtxt, port);
                if (!rc)
                {
                    pCtxt->rxport_en &= pSensor->rxport_en;
                    pCtxt->num_connected_sensors++;
                    SHIGH("detected camera on port %d on bridge chip 0x%x", port, pCtxt->slave_addr);

                    for (unsigned int subChannel = 0; subChannel < pSensor->port_cfg.num_sources; subChannel++)
                    {
                        img_src_channel_t *pChannel = &pCtxt->sensor_lib.channels[pCtxt->sensor_lib.num_channels];
                        img_src_subchannel_t *pSubChannel = &pCtxt->sensor_lib.subchannels[pCtxt->sensor_lib.num_subchannels];
                        img_src_subchannel_layout_t layout = {
                            .src_id = pCtxt->sensor_lib.num_subchannels,
                        };

                        pChannel->num_subchannels = 1;
                        pChannel->output_mode = pSensor->port_cfg.sources[subChannel];
                        pChannel->subchan_layout[0] = layout;

                        pSubChannel->src_id = pCtxt->sensor_lib.num_subchannels;
                        pCtxt->sensor_lib.src_id_enable_mask |= (1 << pSubChannel->src_id);
                        pSubChannel->modes[0] = pSensor->port_cfg.sources[subChannel];
                        pSubChannel->num_modes = 1;

                        pCtxt->sensor_lib.num_channels++;
                        pCtxt->sensor_lib.num_subchannels++;
                    }
                }
            }

            pCtxt->state = TIDS90UB_STATE_INITIALIZED;

            /********************/
            SWARN("tids90ub detected %d out of %d",
                   pCtxt->num_connected_sensors, pCtxt->num_supported_sensors);
        }

        CameraLogEvent(CAMERA_SENSOR_EVENT_PROBED, rc, pCtxt->num_connected_sensors);
    }

    return rc;
}

static int tids90ub_sensor_init_setting(void* ctxt)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;
    int err = 0;
    unsigned int i = 0;
    struct camera_i2c_reg_array set_group_alias[2];
    unsigned short set_group_size = 0;
    struct camera_i2c_reg_array set_indiv_alias[8];
    unsigned short set_indiv_size = 0;
    unsigned int src_mask = 0;
    tids90ub_sensor_info_t* pSensorInit = NULL;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        err = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (pCtxt->num_connected_sensors == 0)
        {
            SHIGH("No connected sensors");
            return 0;
        }

        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            if (TIDS90UB_SENSOR_STATE_DETECTED == pCtxt->sensors[i].state)
            {
                if (!pSensorInit)
                {
                    pSensorInit = &pCtxt->sensors[i];
                }

                src_mask |= (1 << i);
                ADD_I2C_REG_ARRAY(set_indiv_alias, set_indiv_size, 0x4c, TIDS90UB_PORT_SEL(pCtxt->sensors[i].rxport), _tids90ub_delay_);
                ADD_I2C_REG_ARRAY(set_indiv_alias, set_indiv_size, 0x65, pCtxt->sensors[i].sensor_alias, _tids90ub_delay_);
            }
        }

        CameraLogEvent(CAMERA_SENSOR_EVENT_INITIALIZE_START, 0, 0);

        if (!src_mask)
        {
            SLOW("skipping as no sensors detected");
            return 0;
        }

        ADD_I2C_REG_ARRAY(set_group_alias, set_group_size, 0x4c, src_mask & 0x0f, _tids90ub_delay_);
        ADD_I2C_REG_ARRAY(set_group_alias, set_group_size, 0x65, pCtxt->slave_alias_group, _tids90ub_delay_);

        CameraLockMutex(pCtxt->mutex);

        /*set group slave alias*/
        pCtxt->tids90ub_reg_setting.reg_array = set_group_alias;
        pCtxt->tids90ub_reg_setting.size = set_group_size;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            err = rc;
            SERR("tids90ub 0x%x failed to set group alias", pCtxt->slave_addr);
            goto RESET_ALIAS;
        }

        ais_log_kpi(AIS_EVENT_KPI_SENSOR_PROG_START);

        /* @TODO: Init all slaves; For now assume they are all same setting */
        pSensorInit->pInterface->init_port(pCtxt, TIDS90UB_PORT_MAX);

        ais_log_kpi(AIS_EVENT_KPI_SENSOR_PROG_END);

        for (i = 0; i < pCtxt->num_supported_sensors; i++)
        {
            if ((1 << i) & src_mask)
            {
                pCtxt->sensors[i].state = TIDS90UB_SENSOR_STATE_INITIALIZED;
            }
        }

RESET_ALIAS:
        pCtxt->tids90ub_reg_setting.reg_array = set_indiv_alias;
        pCtxt->tids90ub_reg_setting.size = set_indiv_size;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            err = rc;
            SERR("tids90ub 0x%x failed to set indiv alias", pCtxt->slave_addr);
        }

        pCtxt->tids90ub_reg_setting.reg_array = NULL;

        CameraUnlockMutex(pCtxt->mutex);

        /* TODO: cleanup initialized slaves in case of error */
        CameraLogEvent(CAMERA_SENSOR_EVENT_INITIALIZE_DONE, err, 0);
    }

    return err;
}

static int tids90ub_sensor_set_channel_mode(void* ctxt, unsigned int src_id_mask, unsigned int mode)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    (void)mode;
    (void)src_id_mask;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return -1;
    }

    return 0;
}

static int tids90ub_sensor_start_stream(void* ctxt, unsigned int src_id_mask)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;
    unsigned int port = 0;
    unsigned int started_mask = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if(TIDS90UB_SENSOR_ID_PATTERN_GEN != pCtxt->config.sensors[port].sensor_id)
        {
            if (TIDS90UB_960 == pCtxt->config.deser_type)
            {
                if (TIDS90UB_TX_PORT_1 == pCtxt->config.tx_port_map)
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_960_port1_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_960_port1_start_reg_array);
                }
                else if (TIDS90UB_TX_PORT_2 == pCtxt->config.tx_port_map)
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_960_port2_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_960_port2_start_reg_array);
                }
                else
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_960_port12_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_960_port12_start_reg_array);
                }
            }
            else
            {
                if (TIDS90UB_TX_PORT_1 == pCtxt->config.tx_port_map)
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_9702_port1_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_9702_port1_start_reg_array);
                }
                else if (TIDS90UB_TX_PORT_2 == pCtxt->config.tx_port_map)
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_9702_port2_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_9702_port2_start_reg_array);
                }
                else
                {
                    pCtxt->tids90ub_reg_setting.reg_array = tids90ub_9702_port12_start_reg_array;
                    pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_9702_port12_start_reg_array);
                }
            }
        }

        if (TIDS90UB_FSYNC_MODE_EXTERNAL_SOC == pCtxt->config.fsync_mode)
        {
            rc = pCtxt->platform_fcn_tbl.trigger_cci_frame_sync(pCtxt->ctrl);
        }

        /*start slaves first*/
        for (port = 0; port < pCtxt->num_supported_sensors; port++)
        {
            if ((1 << port) & src_id_mask)
            {
                tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

                if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
                {
                    int num_tries = 0;

                    SLOW("starting slave %x [%d]", pSensor->sensor_alias,
                            pSensor->lock_status);

                    rc = pSensor->pInterface->start_port(pCtxt, port);
                    if (rc)
                    {
                        SERR("sensor 0x%x failed to start", pSensor->sensor_alias);
                        break;
                    }

                    while (pSensor->lock_status != TIDS90UB_SIGNAL_LOCKED)
                    {
                        if (TIDS90UB_SIGNAL_LOCK_NUM_TRIES == num_tries)
                        {
                            /* TODO: in case of error, cleanup other sensors that were started */
                            SERR("tids90ub 0x%x not locked timeout", pSensor->sensor_alias);
                            rc = -1;
                            goto ERROR;
                        }
                        SERR("tids90ub 0x%x not locked try %d", pSensor->sensor_alias, num_tries);

                        CameraSleep(TIDS90UB_SIGNAL_LOCK_WAIT);
                        num_tries++;
                    }

                    pSensor->state = TIDS90UB_SENSOR_STATE_STREAMING;
                    started_mask |= (1 << port);
                }
                else
                {
                    /*TODO: change this to SERR once we limit which slaves to start*/
                    SLOW("sensor 0x%x not ready to start (state=%d) - bad state",
                        pSensor->sensor_alias, pSensor->state);
              }
            }
        }

        /* TODO: in case of error, cleanup other sensors that were started */

        /* then start tids90ub transmitter if not already started */
        if (!rc &&
            TIDS90UB_STATE_INITIALIZED == pCtxt->state &&
            started_mask)
        {
            SLOW("starting tids90ub 0x%x transmitter (%x)", pCtxt->slave_addr, started_mask);

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pCtxt->slave_addr,
                    &pCtxt->tids90ub_reg_setting);
            if (rc)
            {
                SERR("tids90ub 0x%x failed to start", pCtxt->slave_addr);
            }
            else
            {
                pCtxt->state = TIDS90UB_STATE_STREAMING;
            }
        }

        if (!rc)
        {
            pCtxt->streaming_src_mask |= started_mask;
        }

ERROR:
        CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_START, rc, 0);
    }

    return rc;
}

static int tids90ub_sensor_stop_stream(void* ctxt, unsigned int src_id_mask)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;
    unsigned int port = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        pCtxt->tids90ub_reg_setting.reg_array = tids90ub_stop_reg_array;
        pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_stop_reg_array);

        /*stop transmitter first if no more clients*/
        if (!rc && TIDS90UB_STATE_STREAMING == pCtxt->state)
        {
            pCtxt->streaming_src_mask &= (~src_id_mask);
            SLOW("stopping tids90ub 0x%x transmitter (%x)", pCtxt->slave_addr, pCtxt->streaming_src_mask);

            /*stop if no slaves streaming*/
            if (!pCtxt->streaming_src_mask)
            {
                rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                            pCtxt->ctrl,
                            pCtxt->slave_addr,
                            &pCtxt->tids90ub_reg_setting);
                if (rc)
                {
                    SERR("tids90ub 0x%x failed to stop", pCtxt->slave_addr);
                }

                pCtxt->state = TIDS90UB_STATE_INITIALIZED;
            }
        }

        /*then stop slaves*/
        for (port = 0; port < pCtxt->num_supported_sensors; port++)
        {
            if ((1 << port) & src_id_mask)
            {
                SLOW("stopping slave %x", pCtxt->sensors[port].sensor_alias);
                if (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[port].state)
                {
                    rc = pCtxt->sensors[port].pInterface->stop_port(pCtxt, port);
                    if (rc)
                    {
                        SERR("sensor 0x%x failed to stop", pCtxt->sensors[port].sensor_alias);
                        break;
                    }
                    pCtxt->sensors[port].state = TIDS90UB_SENSOR_STATE_INITIALIZED;
                }
                else
                {
                    /*TODO: change this to SERR once we limit which slaves to stop*/
                    SLOW("sensor 0x%x not in state to stop (state=%d) - bad state",
                            pCtxt->sensors[port].sensor_alias, pCtxt->sensors[port].state);
                }
            }
        }

        /* TODO: cleanup in case of failure */

        CameraLogEvent(CAMERA_SENSOR_EVENT_STREAM_STOP, rc, 0);
    }

    return rc;
}

static int tids90ub_sensor_set_platform_func_table(void* ctxt, sensor_platform_func_table_t* table)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        return -1;
    }

    if (!pCtxt->platform_fcn_tbl_initialized)
    {
        if (!table ||
            !table->i2c_slave_write_array ||
            !table->i2c_slave_read)
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

static int tids90ub_calculate_exposure(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_PORT_MAX <= src_id)
        {
           SERR("tids90ub_calculate_exposure Invalid src_id = %d", src_id);
           return -1;
        }

        if (!((TIDS90UB_SENSOR_STATE_INITIALIZED == pCtxt->sensors[src_id].state ) ||
              (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[src_id].state)))
        {
           SERR("Sensor src_id = %d invalid state = %d",
                    src_id,pCtxt->sensors[src_id].state);
           return -1;
        }

        if (pCtxt->sensors[src_id].pInterface &&
            pCtxt->sensors[src_id].pInterface->calculate_exposure)
        {
            rc = pCtxt->sensors[src_id].pInterface->calculate_exposure(pCtxt, src_id, exposure_info);
        }
        else
        {
            SWARN("tids90ub_calculate_exposure: Exposure calculation not supported for %u", src_id);
        }
    }

    return rc;
}

static int tids90ub_sensor_exposure_config(void* ctxt, unsigned int src_id, sensor_exposure_info_t* exposure_info)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_PORT_MAX <= src_id)
        {
           SERR("Invalid src_id = %d", src_id);
           return -1;
        }

        if (!((TIDS90UB_SENSOR_STATE_INITIALIZED == pCtxt->sensors[src_id].state ) ||
              (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[src_id].state)))
        {
           SERR("tids90ub_sensor_exposure_config Sensor src_id = %u invalid state = %d",
                    src_id,pCtxt->sensors[src_id].state);
           return -1;
        }

        if (pCtxt->sensors[src_id].pInterface &&
            pCtxt->sensors[src_id].pInterface->apply_exposure)
        {
            SLOW("tids90ub_sensor_exposure_config: setting exposure config for src_id (%u)", src_id);
            rc = pCtxt->sensors[src_id].pInterface->apply_exposure(pCtxt, src_id, exposure_info);
        }
        else
        {
            SERR("tids90ub_sensor_exposure_config: Unsupported exposure config for %u", src_id);
        }
    }

    return rc;
}

static int tids90ub_sensor_hdr_exposure_config(void* ctxt, unsigned int src_id, qcarcam_hdr_exposure_config_t* hdr_exposure_info)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (pCtxt == NULL)
    {
        SERR("pCtxt is null");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_PORT_MAX <= src_id)
        {
           SERR("Invalid src_id = %d", src_id);
           return -1;
        }

        if (!((TIDS90UB_SENSOR_STATE_INITIALIZED == pCtxt->sensors[src_id].state ) ||
              (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[src_id].state)))
        {
           SERR("tids90ub_sensor_exposure_config Sensor src_id = %u invalid state = %d",
                    src_id,pCtxt->sensors[src_id].state);
           return -1;
        }

        if (pCtxt->sensors[src_id].pInterface &&
            pCtxt->sensors[src_id].pInterface->apply_hdr_exposure)
        {
            SLOW("tids90ub_sensor_hdr_exposure_config: setting hdr exposure config for src_id (%u)", src_id);
            rc = pCtxt->sensors[src_id].pInterface->apply_hdr_exposure(pCtxt, src_id, hdr_exposure_info);
        }
        else
        {
            SERR("tids90ub_sensor_hdr_exposure_config: Unsupported hdr exposure config for %u", src_id);
        }
    }

    return rc;
}

static int tids90ub_sensor_s_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (NULL == pCtxt)
    {
        SERR("Invalid ctxt");
        rc =  -1;
    }
    else if (NULL == p_param)
    {
        SERR("Invalid params");
        rc =  -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_PORT_MAX <= src_id)
        {
           SERR("Invalid src_id = %d", src_id);
           rc = -1;
        }
        else if (!((TIDS90UB_SENSOR_STATE_INITIALIZED == pCtxt->sensors[src_id].state ) ||
                   (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[src_id].state)))
        {
           SERR("tids90ub_sensor_s_param Sensor src_id = %u invalid state = %d",
                    src_id,pCtxt->sensors[src_id].state);
           rc = -1;
        }
        else
        {
            if (pCtxt->sensors[src_id].pInterface &&
              pCtxt->sensors[src_id].pInterface->set_param)
            {
                SHIGH("tids90ub_sensor_s_param: setting sensor param (%d) for src_id (%u)", param, src_id);
                rc = pCtxt->sensors[src_id].pInterface->set_param(pCtxt, src_id, param, p_param);
            }
            else
            {
                SERR("tids90ub_sensor_s_param: Sensor doesn't support setting param for src %u", src_id);
                rc = -1;
            }
        }
    }
    return rc;
}

static int tids90ub_sensor_g_param(void* ctxt, qcarcam_param_t param, unsigned int src_id, void* p_param)
{
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    int rc = 0;

    if (NULL == pCtxt)
    {
        SERR("Invalid ctxt");
        rc =  -1;
    }
    else if (NULL == p_param)
    {
        SERR("Invalid params");
        rc = -1;
    }
    else if (I2C_FULL_ACCESS == pCtxt->config.access_mode)
    {
        if (TIDS90UB_PORT_MAX <= src_id)
        {
           SERR("Invalid src_id = %d", src_id);
           rc = -1;
        }
        else if (!((TIDS90UB_SENSOR_STATE_INITIALIZED == pCtxt->sensors[src_id].state ) ||
                   (TIDS90UB_SENSOR_STATE_STREAMING == pCtxt->sensors[src_id].state)))
        {
           SERR("tids90ub_sensor_s_param Sensor src_id = %u invalid state = %d",
                    src_id,pCtxt->sensors[src_id].state);
           rc =  -1;
        }
        else
        {
            if (pCtxt->sensors[src_id].pInterface &&
              pCtxt->sensors[src_id].pInterface->get_param)
            {
                SHIGH("tids90ub_sensor_g_param: getting sensor param (%d) for src_id (%u)", param, src_id);
                rc = pCtxt->sensors[src_id].pInterface->get_param(pCtxt, src_id, param, p_param);
            }
            else
            {
                SERR("tids90ub_sensor_s_param: Sensor doesn't support getting param for src %u", src_id);
                rc = -1;
            }
        }
    }

    return rc;
}

/**
 * FUNCTION: CameraSensorDevice_Open_tids90ub
 *
 * DESCRIPTION: Entry function for device driver framework
 **/
CAM_API CameraResult CameraSensorDevice_Open_tids90ub(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId)
{
    sensor_lib_interface_t sensor_lib_interface = {
            .sensor_open_lib = tids90ub_sensor_open_lib,
    };

    return CameraSensorDevice_Open(ppNewHandle, deviceId, &sensor_lib_interface);
}

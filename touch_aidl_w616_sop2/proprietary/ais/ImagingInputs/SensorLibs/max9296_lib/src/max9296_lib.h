/**
 * @file max9296_lib.h
 * Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MAX9296_LIB_H__
#define __MAX9296_LIB_H__

#include "sensor_lib.h"

//Use this define to simplify max serdes programming
//#define DEBUG_SINGLE_SENSOR

#define DEBUG_SENSOR_NAME SENSOR_MODEL
#include "SensorDebug.h"

#define SENSOR_MODEL "max9296"

#define MSM_DES_0_SLAVEADDR           0x90
#define MSM_DES_1_SLAVEADDR           0x94

#define CAM_SER_BROADCAST_ADDR        0x8E

#define IO_EXPANDER_ADDR              0x7C

//Addresses after reprogramming the serializer and cameras attached to MAX9296
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_0        0x82
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_1        0x84
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_0        0x8A
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_1        0x8C

#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_0       0xE4
#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_1       0xE8
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_0       0xEA
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_1       0xEC

#define MSM_DES_CHIP_ID_REG_ADDR              0xD
#define MSM_DES_REVISION_REG_ADDR             0xE
#define MSM_DES_REVISION_ES2                  0xFF

#define MSM_DES_CTRL3_REG_ADDR                0x13
#define MSM_DES_INTR3_REG_ADDR                0x1B
#define MSM_DES_INTR5_REG_ADDR                0x1D
#define MSM_DES_INTR7_REG_ADDR                0x1F
#define MSM_DES_INTR_DEC_ERR_MASK             0x07
#define MSM_DES_INTR_EOM_ERR_MASK             0xc0
#define MSM_DES_INTR_CRC_ERR_MASK             0x08

#define MSM_SER_CHIP_ID_REG_ADDR              0xD
#define MSM_SER_REVISION_REG_ADDR             0XE
#define MSM_SER_REVISION_ES2                  0xFF

#define MAX_LINK_PIPELINES 2

#define MAXIM9296_INIT_ARRAY_SIZE 64

struct max9296_context_t;
typedef struct max9296_context_t max9296_context_t;

/*
* Describes the GMSL link for connecting cameras
*/
typedef enum
{
    MAXIM_LINK_A,
    MAXIM_LINK_B,
    MAXIM_LINK_MAX
}maxim_link_id_t;

typedef enum
{
    MAXIM_DESER_ID_DEFAULT = 0,
    MAXIM_DESER_ID_MAX9296
}maxim_deser_id_t;

/*
* Describes the type of sensor connected
*/
typedef enum
{
    MAXIM_SENSOR_ID_INVALID,
    MAXIM_SENSOR_ID_AR0231,
    MAXIM_SENSOR_ID_AR0231_EXT_ISP,
    MAXIM_SENSOR_ID_AR0820,
    MAXIM_SENSOR_ID_AR0234_EXT_FPGA,
    MAXIM_SENSOR_ID_MAX9295,
    MAXIM_SENSOR_ID_MAX9295_LOOPBACK,
    MAXIM_SENSOR_ID_EVKIT,
    MAXIM_SENSOR_ID_MAX
}maxim_sensor_id_t;

/*
 * Describes the output mode to be configured

  MAXIM_OP_MODE_DEFAULT
  The following mapping table is used for generic programming

     LINK    pipeline  stream_id |   PIPE  src_id
    ---------------------------------------------
     0         0          0     |    X       0
     0         1          2     |    Z       2
     1         0          1     |    Y       1
     1         1          3     |    U       3

  MAXIM_OP_MODE_RECEIVER
      Limited support for deserializer as a receiver of data forwarded from another deserializer

  MAXIM_OP_MODE_BROADCAST
      Limited support for deserializer to forward data to another deserializer

  MAXIM_OP_MODE_VIDEO_RECORDER
      Limited support for a specific usecase where there is a remote recoder attached to the deserializer.
      This has very basic deser settings to operate with VC = 0 DT = YUV422 8 bits in CSI mode.
*/
typedef enum
{
    MAXIM_OP_MODE_DEFAULT,
    MAXIM_OP_MODE_RECEIVER,
    MAXIM_OP_MODE_BROADCAST,
    MAXIM_OP_MODE_VIDEO_RECORDER,
}maxim_op_mode_t;

typedef enum
{
    INTR_SIGNAL_STATUS_NONE = 0,
    INTR_SIGNAL_STATUS_VALID,
    INTR_SIGNAL_STATUS_LOST,
}maxim_intr_signal_status_t;

typedef struct
{
    maxim_sensor_id_t id;
    unsigned int      mode;
    unsigned int      color_space;
    unsigned int      fsync_mode;
    unsigned int      fsync_freq;
}max9296_topology_sensor_t;

/* Describes configuration topology */
typedef struct
{
    CameraHwBoardType boardType;

    // topology section params
    maxim_op_mode_t opMode;

    max9296_topology_sensor_t sensors[MAXIM_LINK_MAX];

    int num_of_cameras;

    CameraPowerSaveModeType powersave_mode;
}max9296_topology_config_t;

/*
* Describes the MAXIM PIPELINES for serailizer
*/
typedef enum
{
    MAXIM_PIPELINE_X,
    MAXIM_PIPELINE_Y,
    MAXIM_PIPELINE_Z,
    MAXIM_PIPELINE_U,
    MAXIM_PIPELINE_MAX
}maxim_pipeline_id_t;

/*
* Type of data input to the link (serializer)
* We may need set extra settings in case of PARALLEL mode
*/
typedef enum
{
    MAXIM_LINK_TYPE_MIPI,
    MAXIM_LINK_TYPE_PARALLEL
}maxim_link_t;

/*
* Mode for each pipeline
*/
typedef struct
{
    maxim_pipeline_id_t id;
    img_src_mode_t      mode;
    unsigned int        stream_id;
}maxim_pipeline_t;

typedef struct
{
    maxim_link_t     link_type;
    maxim_pipeline_t pipelines[MAX_LINK_PIPELINES];
    uint8 num_pipelines; /*number of ser pipelines used*/
}max9296_link_cfg_t;

/**
 * MAXIM Slave Description
 */
typedef struct
{
    maxim_sensor_id_t id;

    int (*detect)(max9296_context_t* ctxt, uint32 link);
    int (*get_link_cfg)(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);

    int (*init_link)(max9296_context_t* ctxt, uint32 link);
    int (*start_link)(max9296_context_t* ctxt, uint32 link);
    int (*stop_link)(max9296_context_t* ctxt, uint32 link);

    int (*calculate_exposure)(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
    int (*apply_exposure)(max9296_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);

    int (*apply_hdr_exposure)(max9296_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure);
    int (*apply_sensormode)(max9296_context_t* ctxt, uint32 link, uint32 sensormode);
    int (*apply_gamma)(max9296_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma_info);

}max9296_sensor_t;

/**
 * Describes sensor state
 */
typedef enum
{
    SENSOR_STATE_INVALID = 0,
    SENSOR_STATE_SERIALIZER_DETECTED,
    SENSOR_STATE_DETECTED,
    SENSOR_STATE_INITIALIZED,
    SENSOR_STATE_STREAMING,
    SENSOR_STATE_UNSUPPORTED
}max9296_sensor_state_t;

/**
 * Describes the sensor Information
 */
typedef struct
{
    max9296_sensor_state_t state;
    unsigned int serializer_addr;
    unsigned int serializer_alias;
    unsigned int sensor_addr;
    unsigned int sensor_alias;
    unsigned int mode;
    unsigned int color_space;

    max9296_link_cfg_t link_cfg;

    max9296_sensor_t*   sensor;

    unsigned int signal_status;

    void *pPrivCtxt;
}max9296_sensor_info_t;

/**
 * Describes deserializer state
 */
typedef enum
{
    MAX9296_STATE_INVALID = 0,
    MAX9296_STATE_DETECTED,
    MAX9296_STATE_INITIALIZED,
    MAX9296_STATE_SUSPEND,
    MAX9296_STATE_STREAMING
}max9296_state_t;

typedef enum
{
    MAX9296_MODE_LINK_A = 0,
    MAX9296_MODE_LINK_B,
    MAX9296_MODE_SPLITTER
}max9296_link_info_t;

typedef enum
{
    MAX9296_FSYNC_MODE_DISABLED = 0,      // Fsync is disabled
    MAX9296_FSYNC_MODE_EXT_CLK,           // use external 25MHz osc
    MAX9296_FSYNC_MODE_PCLK               // use pclk of incoming video
}max9296_fsync_mode_t;

/**
 * describe internal max96712 pipe configuration
 */
typedef struct
{
    maxim_link_id_t        link; //input link ID
    maxim_pipeline_id_t    input_id; //input pipe ID
    maxim_link_t           link_type;
    img_src_channel_info_t src;  //src channel info
    img_src_channel_info_t dst;  //dst channel mapping
    unsigned int           pipe_override; //override pipe setting
}max9296_pipe_config_t;

struct max9296_context_t
{
  /*must be first element*/
  sensor_lib_t sensor_lib;

  CameraMutex mutex;

  max9296_state_t state;
  unsigned int streaming_src_mask;

  unsigned int revision;
  unsigned int slave_addr;
  max9296_link_info_t link_mode;
  unsigned int num_supported_sensors;
  unsigned int num_connected_sensors;
  unsigned int platform_fcn_tbl_initialized;
  sensor_platform_func_table_t platform_fcn_tbl;
  struct camera_i2c_reg_setting init_reg_settings;
  unsigned int rxport_en;

  struct camera_i2c_reg_setting max9296_reg_setting;
  struct camera_i2c_reg_array init_array[MAXIM9296_INIT_ARRAY_SIZE];

  struct camera_i2c_sync_cfg    i2c_sync_settings;

  max9296_sensor_info_t max9296_sensors[MAXIM_LINK_MAX];
  max9296_topology_config_t max9296_config;
  boolean is_gpio_expander_used;
  boolean is_gpio_expander_poweron;

  uint32 pipe_override; //bit mask of pipe overrides
  max9296_pipe_config_t pipelines[MAXIM_PIPELINE_MAX];
  struct camera_i2c_reg_setting io_expander_reg_setting;
  unsigned int subdev_id;

  uint32 cci_sync_line_no;
  uint32 cci_sync_delay;

  void* ctrl;
};


/*CONFIGURATION OPTIONS*/
#define _max9296_delay_ 0
#if !defined(__INTEGRITY)
    #define MAX9296_SELECT_LINK_DELAY 100000
#else
    /*Fix: Random camera detection failure in multicamera */
    #define MAX9296_SELECT_LINK_DELAY 200000
#endif
#define MAX9296_LINK_DETECT_DELAY 10000


#define CAM_DES_START \
{ \
    { 0x0002, 0xF3, _max9296_delay_ }, \
}

#define CAM_DES_STOP \
{ \
    { 0x0002, 0x03, _max9296_delay_ }, \
}

#define CAM_DES_INTR_INIT \
{ \
    { 0x001a, MSM_DES_INTR_DEC_ERR_MASK, _max9296_delay_ }, \
    { 0x001c, MSM_DES_INTR_EOM_ERR_MASK, _max9296_delay_ }, \
    { 0x001e, MSM_DES_INTR_CRC_ERR_MASK, _max9296_delay_ }, \
}

#define CAM_DES_INTR_DEINIT \
{ \
    { 0x001a, 0x00, _max9296_delay_ }, \
    { 0x001c, 0x00, _max9296_delay_ }, \
    { 0x001e, 0x00, _max9296_delay_ }, \
}

#define CAM_DES_INTR_READ_CLEAR \
{ \
    { 0x0000, 0x00, _max9296_delay_ }, \
    { 0x0022, 0x00, _max9296_delay_ }, \
    { 0x0023, 0x00, _max9296_delay_ }, \
    { 0x0024, 0x00, _max9296_delay_ }, \
    { 0x0077, 0x00, _max9296_delay_ }, \
    { 0x007f, 0x00, _max9296_delay_ }, \
    { 0x0087, 0x00, _max9296_delay_ }, \
    { 0x008f, 0x00, _max9296_delay_ }, \
    { 0x0100, 0x00, _max9296_delay_ }, \
    { 0x0112, 0x00, _max9296_delay_ }, \
    { 0x0124, 0x00, _max9296_delay_ }, \
    { 0x0136, 0x00, _max9296_delay_ }, \
}

#define IO_EXPANDER_READ \
{ \
    { 0x00, 0x00, _max9296_delay_ }, \
    { 0x0f, 0x00, _max9296_delay_ }, \
    { 0x11, 0x00, _max9296_delay_ }, \
    { 0x7d, 0x00, _max9296_delay_ }, \
}

//NOTE: 1 -> 12 disables backward propogation of I2C commands.
// We only do this in the case of receriver board in case its only connected to another board.
// If connected to camera this will need to be removed

#define CAM_DES_INIT_LINK_MODE_RECEIVER \
{ \
    { 0x0001, 0x12, _max9296_delay_ }, \
    { 0x0002, 0x03, _max9296_delay_ }, \
    { 0x031C, 0x20, _max9296_delay_ }, \
    { 0x031F, 0x20, _max9296_delay_ }, \
    { 0x0051, 0x01, _max9296_delay_ }, \
    { 0x0473, 0x02, _max9296_delay_ }, \
    { 0x0100, 0x23, _max9296_delay_ }, \
    { 0x0112, 0x23, _max9296_delay_ }, \
    { 0x0124, 0x23, _max9296_delay_ }, \
    { 0x0136, 0x23, _max9296_delay_ }, \
    { 0x0320, 0x2C, _max9296_delay_ }, \
}


#define CAM_DES_INIT_SPLITTER_MODE_RECEIVER \
{\
    { 0x0001, 0x12, _max9296_delay_ }, \
    { 0x0002, 0x03, _max9296_delay_ }, \
    { 0x031C, 0x60, _max9296_delay_ }, \
    { 0x031F, 0x60, _max9296_delay_ }, \
    { 0x0473, 0x02, _max9296_delay_ }, \
    { 0x04B3, 0x02, _max9296_delay_ }, \
    { 0x0100, 0x23, _max9296_delay_ }, \
    { 0x0112, 0x23, _max9296_delay_ }, \
    { 0x0124, 0x23, _max9296_delay_ }, \
    { 0x0136, 0x23, _max9296_delay_ }, \
    { 0x0320, 0x30, _max9296_delay_ }, \
    { 0x048B, 0x07, _max9296_delay_ }, \
    { 0x04AD, 0x15, _max9296_delay_ }, \
    { 0x048D, 0x6A, _max9296_delay_ }, \
    { 0x048E, 0x6A, _max9296_delay_ }, \
    { 0x048F, 0x40, _max9296_delay_ }, \
    { 0x0490, 0x40, _max9296_delay_ }, \
    { 0x0491, 0x41, _max9296_delay_ }, \
    { 0x0492, 0x41, _max9296_delay_ }, \
}

#ifdef DEBUG_SINGLE_SENSOR

#define CAM_DES_INIT_SPLITTER_MODE_SENDER \
{ \
    { 0x0002, 0x03, _max9296_delay_ }, \
    { 0x0320, 0x2C, _max9296_delay_ }, \
    { 0x0051, 0x02, _max9296_delay_ }, \
}
#else //DEBUG_SINGLE_SENSOR

#define CAM_DES_INIT_SPLITTER_MODE_SENDER \
{ \
    { 0x0002, 0x03, _max9296_delay_ }, /*stop TX transmitter*/ \
    { 0x0050, 0x00, _max9296_delay_ }, /*assign each pipe to indiv stream id*/ \
    { 0x0051, 0x01, _max9296_delay_ }, \
    { 0x0052, 0x02, _max9296_delay_ }, \
    { 0x0053, 0x03, _max9296_delay_ }, \
}

#endif /*DEBUG_SINGLE_SENSOR*/

/*
 * EVKIT
 * remap PIPE X to CSI-2 controller 1
 * remap PIPE Y using default

    LINK    pipeline  stream_id |   PIPE  src_id
    ---------------------------------------------
     0         0          0     |    Y       0
     0         1          1     |    X       2
*/

#define CAM_DES_INIT_EVKIT_MODE_SENDER \
{ \
    { 0x0313, 0x00, _max9296_delay_ }, \
    { 0x0010, 0x31, _max9296_delay_ }, \
    { 0x0320, 0x2F, _max9296_delay_ }, \
    { 0x0325, 0x80, _max9296_delay_ }, \
    { 0x040b, 0x07, _max9296_delay_ }, \
    { 0x042d, 0x15, _max9296_delay_ }, \
    { 0x040d, 0x24, _max9296_delay_ }, \
    { 0x040e, 0x64, _max9296_delay_ }, \
    { 0x040f, 0x00, _max9296_delay_ }, \
    { 0x0410, 0x40, _max9296_delay_ }, \
    { 0x0411, 0x01, _max9296_delay_ }, \
    { 0x0412, 0x41, _max9296_delay_ }, \
    { 0x0316, 0x24, _max9296_delay_ }, \
    { 0x031d, 0x40, _max9296_delay_ }, \
    { 0x0313, 0xC2, _max9296_delay_ }, \
}

// NOTE: Below mode is used when we have a remote recoder attached to a deserializer
// end instead of an actual camera. This has very basic deser settings to operate
// with VC = 0 DT = YUV422 8 bits in CSI mode.
#define CAM_DES_INIT_RECORDER_MODE \
{ \
    { 0x0002, 0x03, _max9296_delay_ }, \
    { 0x0313, 0x42, _max9296_delay_ }, \
    { 0x0316, 0x2A, _max9296_delay_ }, \
    { 0x040B, 0x07, _max9296_delay_ }, \
    { 0x042D, 0x15, _max9296_delay_ }, \
    { 0x040D, 0x2A, _max9296_delay_ }, \
    { 0x040E, 0x2A, _max9296_delay_ }, \
    { 0x040F, 0x00, _max9296_delay_ }, \
    { 0x0410, 0x00, _max9296_delay_ }, \
    { 0x0411, 0x01, _max9296_delay_ }, \
    { 0x0412, 0x01, _max9296_delay_ }, \
    { 0x031D, 0x68, _max9296_delay_ }, \
    { 0x0320, 0x48, _max9296_delay_ }, \
    { 0x0100, 0x23, _max9296_delay_ }, \
    { 0x0112, 0x23, _max9296_delay_ }, \
    { 0x0124, 0x23, _max9296_delay_ }, \
    { 0x0136, 0x23, _max9296_delay_ }, \
}

// NOTE: 1 -> 12 disables backward propogation of I2C commands.
// We only do this in the case of receriver board in case its only connected to another board.
// If connected to camera this will need to be removed
#define CAM_DES_DISABLE_I2C_REVERSE \
{ \
    { 0x0001, 0x12, _max9296_delay_ }, \
}

#ifdef ENABLE_LOOPBACK_SEQUENCE
#undef CAM_DES_INIT_SPLITTER_MODE_SENDER
#define CAM_DES_INIT_SPLITTER_MODE_SENDER \
{ \
    { 0x0002, 0x03, _max9296_delay_ }, \
    { 0x0051, 0x00, _max9296_delay_ }, \
    { 0x0320, 0x26, _max9296_delay_ }, \
    { 0x0325, 0x80, _max9296_delay_ }, \
    { 0x01F9, 0x59, _max9296_delay_ }, \
    { 0x01D9, 0x59, _max9296_delay_ }, \
}
#endif

#endif /* __MAX9296_LIB_H__ */

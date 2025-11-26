/**
 * @file max96712_lib.h
 * Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MAX96712_LIB_H__
#define __MAX96712_LIB_H__

#include "sensor_lib.h"
#include <stdbool.h>

//Use this define to simplify max serdes programming
//#define DEBUG_SINGLE_SENSOR

#define DEBUG_SENSOR_NAME SENSOR_MODEL
#include "SensorDebug.h"

#define SENSOR_MODEL "max96712"

#define MSM_DES_0_SLAVEADDR          0x52
#define MSM_DES_1_SLAVEADDR          0xD6

#define CAM_SER_BROADCAST_ADDR         0x72



//Addresses after reprogramming the serializer and cameras attached to MAX96712
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_0        0x62
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_1        0x64
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_2        0x66
#define MSM_DES_0_ALIAS_ADDR_CAM_SER_3        0x68
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_0        0x6A
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_1        0x6C
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_2        0x6E
#define MSM_DES_1_ALIAS_ADDR_CAM_SER_3        0x70

#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_0       0xE0
#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_1       0xE2
#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_2       0xE4
#define MSM_DES_0_ALIAS_ADDR_CAM_SNSR_3       0xE6
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_0       0xE8
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_1       0xEA
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_2       0xEC
#define MSM_DES_1_ALIAS_ADDR_CAM_SNSR_3       0xEE

#define MSM_DES_CHIP_ID_REG_ADDR        0x0D
#define MSM_DES_REVISION_REG_ADDR       0x4C
#define MSM_DES_CTRL3_REG_ADDR          0x1A
#define MSM_DES_CTRL12_REG_ADDR         0x0A
#define MSM_DES_CTRL13_REG_ADDR         0x0B
#define MSM_DES_CTRL14_REG_ADDR         0x0C
#define MSM_DES_INTR3_REG_ADDR          0x26
#define MSM_DES_INTR5_REG_ADDR          0x28
#define MSM_DES_INTR7_REG_ADDR          0x2A
#define MSM_DES_INTR9_REG_ADDR          0x2C
#define MSM_DES_INTR_DEC_ERR_MASK       0x0F
#define MSM_DES_INTR_EOM_ERR_MASK       0xF0
#define MSM_DES_INTR_CRC_ERR_MASK       0x08
#define MSM_DES_INTR_IDLE_ERR_MASK      0x0F
#define MSM_DES_INTR_PXLCRC_ERR_MASK    0x00

#define MAX_LINK_PIPELINES                4
#define MAXIM96712_INIT_ARRAY_SIZE      128


struct max96712_context_t;
typedef struct max96712_context_t max96712_context_t;

/*
* Describes the GMSL link for connecting cameras
*/
typedef enum
{
    MAXIM_LINK_A = 0,
    MAXIM_LINK_B,
    MAXIM_LINK_C,
    MAXIM_LINK_D,
    MAXIM_LINK_MAX
}maxim_link_id_t;

typedef enum
{
    MAXIM_DESER_ID_DEFAULT = 0,
    MAXIM_DESER_ID_MAX96712
}maxim_deser_id_t;

/*
* Describes the type of sensor connected
*/
typedef enum
{
    MAXIM_SENSOR_ID_INVALID,

    MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300,
    MAXIM_SENSOR_ID_AR0231,
    MAXIM_SENSOR_ID_AR0231_EXT_ISP,
    MAXIM_SENSOR_ID_S5K1H1SX,
    MAXIM_SENSOR_ID_AR0234_EXT_FPGA,

    MAXIM_SENSOR_ID_SER_PATTERN_GEN = 253,
    MAXIM_SENSOR_ID_PATTERN_GEN,
    MAXIM_SENSOR_ID_MAX
}maxim_sensor_id_t;

/*
* Describes the output mode to be configured
*/
typedef enum
{
    MAXIM_OP_MODE_DEFAULT,
    MAXIM_OP_MODE_2_LANES
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
    unsigned int      fsync_mode;
    unsigned int      fsync_freq;
}max96712_topology_sensor_t;

/* Describes configuration topology */
typedef struct
{
    CameraHwBoardType boardType;

    // topology section params
    maxim_op_mode_t opMode;

    max96712_topology_sensor_t sensors[MAXIM_LINK_MAX];

    int num_of_cameras;

    CameraPowerSaveModeType powersave_mode;
}max96712_topology_config_t;

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

typedef enum
{
    MAX96712_PIPE_0,
    MAX96712_PIPE_MAX = 8
}max96712_pipe_t;

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
}max96712_link_cfg_t;

/**
 * MAXIM Slave Description
 */
typedef struct
{
    maxim_sensor_id_t id;

    int (*detect)(max96712_context_t* ctxt, uint32 link);
    int (*get_link_cfg)(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);

    int (*init_link)(max96712_context_t* ctxt, uint32 link);
    int (*start_link)(max96712_context_t* ctxt, uint32 link);
    int (*stop_link)(max96712_context_t* ctxt, uint32 link);

    int (*calculate_exposure)(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
    int (*apply_exposure)(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);

    int (*apply_hdr_exposure)(max96712_context_t* ctxt, uint32 link, qcarcam_hdr_exposure_config_t* hdr_exposure);

    int (*apply_gamma)(max96712_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma_info);

}max96712_sensor_t;

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
}max96712_sensor_state_t;

/**
 * Describes the sensor Information
 */
typedef struct
{
    max96712_sensor_state_t state;
    unsigned int serializer_addr;
    unsigned int serializer_alias;
    unsigned int sensor_addr;
    unsigned int sensor_alias;
    unsigned int mode;

    max96712_link_cfg_t link_cfg;

    max96712_sensor_t*   sensor;

    unsigned int signal_status;

    void *pPrivCtxt;
}max96712_sensor_info_t;

/**
 * Describes the different states for MAX96712
 */
typedef enum
{
    MAX96712_STATE_INVALID = 0,
    MAX96712_STATE_DETECTED,
    MAX96712_STATE_INITIALIZED,
    MAX96712_STATE_SUSPEND,
    MAX96712_STATE_STREAMING,
}max96712_state_t;

typedef enum
{
    MAX96712_MODE_LINK_A = 0,
    MAX96712_MODE_LINK_B,
    MAX96712_MODE_LINK_C,
    MAX96712_MODE_LINK_D,
    MAX96712_MODE_SPLITTER
}max96712_link_info_t;

typedef enum
{
    MAX96712_FSYNC_MODE_DISABLED = 0,      // Fsync is disabled
    MAX96712_FSYNC_MODE_EXT_CLK,           // use external 25MHz osc
    MAX96712_FSYNC_MODE_PCLK               // use pclk of incoming video
}max96712_fsync_mode_t;

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
}max96712_pipe_config_t;

/**
 * Gives all the information of MAX96712
 */
struct max96712_context_t
{
    /*must be first element*/
    sensor_lib_t sensor_lib;

    CameraMutex mutex;

    max96712_state_t state;
    unsigned int streaming_src_mask;

    unsigned int revision;
    unsigned int slave_addr;
    max96712_link_info_t link_mode;
    unsigned int num_supported_sensors;
    unsigned int num_connected_sensors;
    unsigned int connected_sensors;
    unsigned int platform_fcn_tbl_initialized;
    sensor_platform_func_table_t platform_fcn_tbl;
    struct camera_i2c_reg_setting init_reg_settings;
    unsigned int rxport_en;

    struct camera_i2c_reg_setting max96712_reg_setting;
    struct camera_i2c_reg_array init_array[MAXIM96712_INIT_ARRAY_SIZE];

    max96712_sensor_info_t max96712_sensors[MAXIM_LINK_MAX];
    max96712_topology_config_t max96712_config;

    max96712_pipe_config_t pipelines[MAX96712_PIPE_MAX];
    unsigned char pipe_override;

    unsigned int subdev_id;
    void* ctrl;
};

/*CONFIGURATION OPTIONS*/
#define _max96712_delay_ 0
#define MAX96712_SELECT_LINK_DELAY 100000
#define MAX96712_LINK_DETECT_DELAY 25000
#define _max96712_delay1_ 10000
#define _max96712_select_link_delay_ 100000
#define _max96712_reset_delay_ 100000

#define MAX96712_INIT_REV5 \
{ \
    /*All MIPI PHYs in standby*/ \
    { 0x08A2, 0x04, _max96712_delay_ }, \
    /*CRUSSCMode*/                      \
    { 0x1445, 0x00, _max96712_delay_ }, \
    { 0x1545, 0x00, _max96712_delay_ }, \
    { 0x1645, 0x00, _max96712_delay_ }, \
    { 0x1745, 0x00, _max96712_delay_ }, \
    /*Increase CMU regulator voltage*/  \
    { 0x06C2, 0x10, _max96712_delay_ }, \
    /*Enable VGA Hi-Gain for 6Gbps/3Gbps*/ \
    { 0x14D1, 0x03, _max96712_delay_ }, \
    { 0x15D1, 0x03, _max96712_delay_ }, \
    { 0x16D1, 0x03, _max96712_delay_ }, \
    { 0x17D1, 0x03, _max96712_delay_ }, \
    /*One-shot reset all links*/        \
    { 0x0018, 0x0F, _max96712_delay_} \
}

#define MAX96712_INIT_REV3 \
{ \
    /*@TODO: Add explicit reset */ \
    /*All MIPI PHYs in standby*/ \
    { 0x08A2, 0x04, _max96712_delay_ }, \
    /*CRUSSCMode*/                      \
    { 0x1445, 0x00, _max96712_delay_ }, \
    { 0x1545, 0x00, _max96712_delay_ }, \
    { 0x1645, 0x00, _max96712_delay_ }, \
    { 0x1745, 0x00, _max96712_delay_ }, \
    /*Increase CMU regulator voltage*/  \
    { 0x06C2, 0x10, _max96712_delay_ }, \
    /*Link Margin*/                     \
    { 0x1458, 0x28, _max96712_delay_ }, \
    { 0x1459, 0x68, _max96712_delay_ }, \
    { 0x1558, 0x28, _max96712_delay_ }, \
    { 0x1559, 0x68, _max96712_delay_ }, \
    { 0x1658, 0x28, _max96712_delay_ }, \
    { 0x1659, 0x68, _max96712_delay_ }, \
    { 0x1758, 0x28, _max96712_delay_ }, \
    { 0x1759, 0x68, _max96712_delay_ }, \
    /*One-shot reset all links*/        \
    { 0x0018, 0x0F, _max96712_delay_} \
}

#define MAX96712_INIT_S5K1H1SX \
{ \
    { 0x0006, 0xF1, _max96712_delay_ },\
    { 0x0010, 0x22, _max96712_delay_ },\
    { 0x0011, 0x22, _max96712_delay_ },\
    { 0x0018, 0x0F, _max96712_delay_ },\
    { 0x00F0, 0x10, _max96712_delay_ },\
    { 0x00F1, 0xEA, _max96712_delay_ },\
    { 0x00F2, 0x40, _max96712_delay_ },\
    { 0x00F3, 0xC8, _max96712_delay_ },\
    { 0x00F4, 0x03, _max96712_delay_ },\
    { 0x0100, 0x23, _max96712_delay_ },\
    { 0x0112, 0x23, _max96712_delay_ },\
    { 0x0124, 0x23, _max96712_delay_ },\
    { 0x0136, 0x23, _max96712_delay_ },\
    { 0x0148, 0x23, _max96712_delay_ },\
    { 0x0160, 0x23, _max96712_delay_ },\
    { 0x0172, 0x23, _max96712_delay_ },\
    { 0x0184, 0x23, _max96712_delay_ },\
    { 0x0401, 0x01, _max96712_delay_ },\
    { 0x0403, 0x01, _max96712_delay_ },\
    { 0x0405, 0x00, _max96712_delay_ },\
    { 0x0407, 0x00, _max96712_delay_ },\
    { 0x040B, 0x02, _max96712_delay_ },\
    { 0x040C, 0x00, _max96712_delay_ },\
    { 0x040D, 0x00, _max96712_delay_ },\
    { 0x040E, 0x00, _max96712_delay_ },\
    { 0x040F, 0x00, _max96712_delay_ },\
    { 0x0410, 0x00, _max96712_delay_ },\
    { 0x0411, 0x00, _max96712_delay_ },\
    { 0x0412, 0x00, _max96712_delay_ },\
    { 0x0414, 0x20, _max96712_delay_ },\
    { 0x0415, 0x2C, _max96712_delay_ },\
    { 0x0417, 0x20, _max96712_delay_ },\
    { 0x0418, 0x2C, _max96712_delay_ },\
    { 0x041A, 0x00, _max96712_delay_ },\
    { 0x041B, 0x2C, _max96712_delay_ },\
    { 0x041D, 0x00, _max96712_delay_ },\
    { 0x041E, 0x2C, _max96712_delay_ },\
    { 0x041F, 0x00, _max96712_delay_ },\
    { 0x0421, 0x00, _max96712_delay_ },\
    { 0x0423, 0x00, _max96712_delay_ },\
    { 0x0425, 0x00, _max96712_delay_ },\
    { 0x0427, 0x00, _max96712_delay_ },\
    { 0x042B, 0x00, _max96712_delay_ },\
    { 0x042C, 0x00, _max96712_delay_ },\
    { 0x042D, 0x00, _max96712_delay_ },\
    { 0x042E, 0x00, _max96712_delay_ },\
    { 0x042F, 0x00, _max96712_delay_ },\
    { 0x0430, 0x00, _max96712_delay_ },\
    { 0x0431, 0x00, _max96712_delay_ },\
    { 0x0432, 0x00, _max96712_delay_ },\
    { 0x0434, 0x00, _max96712_delay_ },\
    { 0x0435, 0x01, _max96712_delay_ },\
    { 0x0436, 0x01, _max96712_delay_ },\
    { 0x0437, 0x00, _max96712_delay_ },\
    { 0x043A, 0x00, _max96712_delay_ },\
    { 0x043D, 0x00, _max96712_delay_ },\
    { 0x043E, 0x00, _max96712_delay_ },\
    { 0x043F, 0x00, _max96712_delay_ },\
    { 0x08A0, 0x04, _max96712_delay_ },\
    { 0x08A2, 0xF0, _max96712_delay_ },\
    { 0x08A3, 0xE4, _max96712_delay_ },\
    { 0x08A4, 0xE4, _max96712_delay_ },\
    { 0x08A5, 0x00, _max96712_delay_ },\
    { 0x08A6, 0x00, _max96712_delay_ },\
    { 0x08A9, 0x00, _max96712_delay_ },\
    { 0x08AA, 0x02, _max96712_delay_ },\
    { 0x08AD, 0x00, _max96712_delay_ },\
    { 0x08AE, 0x00, _max96712_delay_ },\
    { 0x090A, 0x00, _max96712_delay_ },\
    { 0x090B, 0x07, _max96712_delay_ },\
    { 0x090C, 0x00, _max96712_delay_ },\
    { 0x090D, 0x2C, _max96712_delay_ },\
    { 0x090E, 0x2C, _max96712_delay_ },\
    { 0x090F, 0x00, _max96712_delay_ },\
    { 0x0910, 0x00, _max96712_delay_ },\
    { 0x0911, 0x01, _max96712_delay_ },\
    { 0x0912, 0x01, _max96712_delay_ },\
    { 0x092D, 0x15, _max96712_delay_ },\
    { 0x092E, 0x00, _max96712_delay_ },\
    { 0x092F, 0x00, _max96712_delay_ },\
    { 0x0930, 0x00, _max96712_delay_ },\
    { 0x0931, 0x00, _max96712_delay_ },\
    { 0x0933, 0x00, _max96712_delay_ },\
    { 0x094A, 0xC0, _max96712_delay_ },\
    { 0x094B, 0x01, _max96712_delay_ },\
    { 0x094C, 0x00, _max96712_delay_ },\
    { 0x094D, 0x12, _max96712_delay_ },\
    { 0x094E, 0x52, _max96712_delay_ },\
    { 0x094F, 0x00, _max96712_delay_ },\
    { 0x0950, 0x00, _max96712_delay_ },\
    { 0x0951, 0x01, _max96712_delay_ },\
    { 0x0952, 0x01, _max96712_delay_ },\
    { 0x096D, 0x15, _max96712_delay_ },\
    { 0x096E, 0x00, _max96712_delay_ },\
    { 0x096F, 0x00, _max96712_delay_ },\
    { 0x0970, 0x00, _max96712_delay_ },\
    { 0x0971, 0x00, _max96712_delay_ },\
    { 0x0973, 0x03, _max96712_delay_ },\
    { 0x098A, 0xC0, _max96712_delay_ },\
    { 0x098B, 0x00, _max96712_delay_ },\
    { 0x098C, 0x00, _max96712_delay_ },\
    { 0x09AD, 0x00, _max96712_delay_ },\
    { 0x09AE, 0x00, _max96712_delay_ },\
    { 0x09AF, 0x00, _max96712_delay_ },\
    { 0x09B0, 0x00, _max96712_delay_ },\
    { 0x09B1, 0x00, _max96712_delay_ },\
    { 0x09B3, 0x00, _max96712_delay_ },\
    { 0x09CA, 0x00, _max96712_delay_ },\
    { 0x09CB, 0x00, _max96712_delay_ },\
    { 0x09CC, 0x00, _max96712_delay_ },\
    { 0x09ED, 0x00, _max96712_delay_ },\
    { 0x09EE, 0x00, _max96712_delay_ },\
    { 0x09EF, 0x00, _max96712_delay_ },\
    { 0x09F0, 0x00, _max96712_delay_ },\
    { 0x09F1, 0x00, _max96712_delay_ },\
    { 0x09F3, 0x00, _max96712_delay_ },\
}

#define MAX96712_STOP \
{ \
    { 0x08A2, 0x04, _max96712_delay_ }, \
}

#define CAM_DES_INTR_INIT \
{ \
    { 0x0025, MSM_DES_INTR_DEC_ERR_MASK, _max96712_delay_ }, \
    { 0x0027, MSM_DES_INTR_EOM_ERR_MASK, _max96712_delay_ }, \
    { 0x0029, MSM_DES_INTR_CRC_ERR_MASK, _max96712_delay_ }, \
    { 0x002B, MSM_DES_INTR_IDLE_ERR_MASK, _max96712_delay_ }, \
    { 0x0044, MSM_DES_INTR_PXLCRC_ERR_MASK, _max96712_delay_ }, \
}

#define CAM_DES_INTR_DEINIT \
{ \
    { 0x0025, 0x00, _max96712_delay_ }, \
    { 0x0027, 0x00, _max96712_delay_ }, \
    { 0x0029, 0x00, _max96712_delay_ }, \
    { 0x002B, 0x00, _max96712_delay_ }, \
    { 0x0044, 0x00, _max96712_delay_ }, \
}

#define CAM_DES_INTR_READ_CLEAR \
{ \
    { 0x0000, 0x00, _max96712_delay_ }, \
    { 0x0035, 0x00, _max96712_delay_ }, \
    { 0x0036, 0x00, _max96712_delay_ }, \
    { 0x0037, 0x00, _max96712_delay_ }, \
    { 0x0038, 0x00, _max96712_delay_ }, \
    { 0x0039, 0x00, _max96712_delay_ }, \
    { 0x003A, 0x00, _max96712_delay_ }, \
    { 0x003B, 0x00, _max96712_delay_ }, \
    { 0x003C, 0x00, _max96712_delay_ }, \
    { 0x0100, 0x00, _max96712_delay_ }, \
    { 0x0112, 0x00, _max96712_delay_ }, \
    { 0x0124, 0x00, _max96712_delay_ }, \
    { 0x0136, 0x00, _max96712_delay_ }, \
    { 0x0148, 0x00, _max96712_delay_ }, \
    { 0x0160, 0x00, _max96712_delay_ }, \
    { 0x0172, 0x00, _max96712_delay_ }, \
    { 0x0184, 0x00, _max96712_delay_ }, \
}

#endif /* __MAX96712_LIB_H__ */

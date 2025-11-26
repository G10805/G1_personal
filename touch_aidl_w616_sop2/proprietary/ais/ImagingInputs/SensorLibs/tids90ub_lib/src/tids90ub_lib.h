/**
 * @file tids90ub_lib.h
 *
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __TIDS90UB_LIB_H__
#define __TIDS90UB_LIB_H__

#include "CameraEventLog.h"
#include "CameraOSServices.h"
#include "sensor_lib.h"

#define SENSOR_MODEL "tids90ub"
#define DEBUG_SENSOR_NAME SENSOR_MODEL
#include "SensorDebug.h"

// Rev 1 is TI960 rev 2 is TI964
#define TIDS90UB_REV_ADDR 0x03
#define TIDS90UB_REV_ID_1 0x10
#define TIDS90UB_REV_ID_2 0x20
#define TIDS90UB_REV_ID_3 0x30
#define TIDS90UB_REV_ID_4 0x40

/*CONFIGURATION OPTIONS*/
#define CSI_TRANSMISSION_2_5GBPS
//#define CSI_TRANSMISSION_800MBPS

#ifdef CSI_TRANSMISSION_2_5GBPS
#define SETTLE_COUNT_2_5GBPS 0xE
#define SETTLE_COUNT SETTLE_COUNT_2_5GBPS
#define CSI_2_5GBPS 0x4
#else
//#define SETTLE_COUNT_800MBPS 0x1A //TI960
//#define SETTLE_COUNT_800MBPS 0x18 //TI954 DEVKIT
#define SETTLE_COUNT_800MBPS 0xF    //TI9702
#define CSI_800MBPS_REV_1 0x5
#define CSI_800MBPS_REV_2 0x2
#define SETTLE_COUNT SETTLE_COUNT_800MBPS
#endif

/*CSI start setting*/
#define _tids90ub_delay_        0
#define _tids90ub_port_delay_   50
#define _start_delay_           5000

#define RXPORT_EN_RX0  0xe0
#define RXPORT_EN_RX1  0xd0
#define RXPORT_EN_RX2  0xb0
#define RXPORT_EN_RX3  0x70
#define RXPORT_EN_ALL  0x0

#define RXPORT_EN_NONE 0xf0

/*disable VIDEO_FREEZE and 1 frame threshold*/
#define PORT_PASS_CTL_VAL 0xb8

#define INIT_INTR_ARRAY \
{ \
  { 0x4c, 0x0f, _tids90ub_delay_ }, \
  { 0xd8, 0x00, _tids90ub_delay_ }, \
  { 0xd9, 0x53, _tids90ub_delay_ }, /*Disable video line count change interrupt*/ \
  { 0x7d, PORT_PASS_CTL_VAL, _tids90ub_delay_ }, \
  { 0x23, 0x8f, _tids90ub_delay_ }, \
}

#define START_REG_ARRAY_9702_PORT_1 \
{ \
  { 0x32, 0x01, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x3C, 0xAA /*0x80*/, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}
#define START_REG_ARRAY_9702_PORT_2 \
{ \
  { 0x32, 0x02, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x3C, 0xAA /*0x80*/, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}
#define START_REG_ARRAY_9702_PORT_1_2 \
{ \
  { 0x32, 0x03, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x3C, 0xAA /*0x80*/, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}
#define START_REG_ARRAY_960_PORT_1 \
{ \
  { 0x32, 0x01, _tids90ub_delay_ }, \
  { 0x1F, 0x00, _tids90ub_delay_ }, \
  { 0x21, 0x83, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}
#define START_REG_ARRAY_960_PORT_2 \
{ \
  { 0x32, 0x02, _tids90ub_delay_ }, \
  { 0x1F, 0x00, _tids90ub_delay_ }, \
  { 0x21, 0x83, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}
#define START_REG_ARRAY_960_PORT_1_2 \
{ \
  { 0x32, 0x03, _tids90ub_delay_ }, \
  { 0x1F, 0x00, _tids90ub_delay_ }, \
  { 0x21, 0x83, _tids90ub_delay_ }, \
  { 0x20, 0xF0, _tids90ub_delay_ }, \
  { 0x33, 0x03, _tids90ub_delay_ }, \
  { 0x20, RXPORT_EN_PORTS, _tids90ub_delay_ }, \
}

#define STOP_REG_ARRAY \
{ \
  { 0x20, RXPORT_EN_NONE, _tids90ub_delay_ }, \
  { 0x33, 0x2, _tids90ub_delay_ }, \
  { 0x3C, 0x00 /*0x2A*/, _tids90ub_delay_ }, \
}


#define RXPORT_EN_PORTS (RXPORT_EN_ALL)
#define TIDS90UB_0_SLAVEADDR 0x7a
#define TIDS90UB_0_SENSOR_ALIAS_GROUP 0xaa
#define TIDS90UB_0_SENSOR_ALIAS0 0xac
#define TIDS90UB_0_SENSOR_ALIAS1 0xae
#define TIDS90UB_0_SENSOR_ALIAS2 0xb0
#define TIDS90UB_0_SENSOR_ALIAS3 0xb2
#define TIDS90UB_0_SERIALIZER_ALIAS_RX0 0x88
#define TIDS90UB_0_SERIALIZER_ALIAS_RX1 0x8a
#define TIDS90UB_0_SERIALIZER_ALIAS_RX2 0x8c
#define TIDS90UB_0_SERIALIZER_ALIAS_RX3 0x8e

#define TIDS90UB_1_SLAVEADDR 0x60
#define TIDS90UB_1_SENSOR_ALIAS_GROUP 0xa0
#define TIDS90UB_1_SENSOR_ALIAS0 0xa2
#define TIDS90UB_1_SENSOR_ALIAS1 0xa4
#define TIDS90UB_1_SENSOR_ALIAS2 0xa6
#define TIDS90UB_1_SENSOR_ALIAS3 0xa8
#define TIDS90UB_1_SERIALIZER_ALIAS_RX0 0x80
#define TIDS90UB_1_SERIALIZER_ALIAS_RX1 0x82
#define TIDS90UB_1_SERIALIZER_ALIAS_RX2 0x84
#define TIDS90UB_1_SERIALIZER_ALIAS_RX3 0x86

#define TIDS90UB_2_SLAVEADDR 0x64
#define TIDS90UB_2_SENSOR_ALIAS_GROUP 0xb4
#define TIDS90UB_2_SENSOR_ALIAS0 0xb6
#define TIDS90UB_2_SENSOR_ALIAS1 0xb8
#define TIDS90UB_2_SENSOR_ALIAS2 0xba
#define TIDS90UB_2_SENSOR_ALIAS3 0xbc
#define TIDS90UB_2_SERIALIZER_ALIAS_RX0 0xe0
#define TIDS90UB_2_SERIALIZER_ALIAS_RX1 0xe2
#define TIDS90UB_2_SERIALIZER_ALIAS_RX2 0xe4
#define TIDS90UB_2_SERIALIZER_ALIAS_RX3 0xe6

#define TIDS90UB_FSYNC_REG_ARRAY_SIZE    8
#define TIDS90UB_MANUAL_EQ_ARRAY_SIZE    16


struct tids90ub_context_t;
typedef struct tids90ub_context_t tids90ub_context_t;

typedef enum
{
    TIDS90UB_DEFAULT,
    TIDS90UB_DEVKIT,
    TIDS90UB_954,
    TIDS90UB_960,
    TIDS90UB_9702
}tids90ub_deser_t;

typedef enum
{
    TIDS90UB_913,
    TIDS90UB_953,
    TIDS90UB_971,
    TIDS90UB_951
}tids90ub_ser_t;

/**
 * TI Port & Sensor Description
 */
typedef enum
{
    TIDS90UB_PORT_0,
    TIDS90UB_PORT_1,
    TIDS90UB_PORT_2,
    TIDS90UB_PORT_3,
    TIDS90UB_PORT_MAX
}tids90ub_port_t;

typedef enum
{
    TIDS90UB_SENSOR_ID_INVALID,
    TIDS90UB_SENSOR_ID_OX03A10,
    TIDS90UB_SENSOR_ID_OV10635,
    TIDS90UB_SENSOR_ID_OV10640,
    TIDS90UB_SENSOR_ID_AR0143,
    TIDS90UB_SENSOR_ID_IMX424_GW5200_TI953,
    TIDS90UB_SENSOR_ID_IMX490_GW5200_TI953,
    TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951,
    TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971,
    TIDS90UB_SENSOR_ID_X3A_OV491_TI935,
    TIDS90UB_SENSOR_ID_0X01F10_TI933,
    TIDS90UB_SENSOR_ID_OV2310_TI935,
    TIDS90UB_SENSOR_ID_SER_PATTERN_GEN = 253,
    TIDS90UB_SENSOR_ID_PATTERN_GEN,
    TIDS90UB_SENSOR_ID_MAX
}tids90ub_sensor_id_t;


typedef enum
{
    IMX_GW5200_MODE_SYNC = 0,
    IMX_GW5200_MODE_BACKW_COMPAT,
    IMX_GW5200_MODE_MAX
}tids90ub_sensor_port_mode_t;

typedef enum
{
    TIDS90UB_FSYNC_MODE_DISABLED = 0,      // Fsync is disabled
    TIDS90UB_FSYNC_MODE_ONESHOT,           // DES sends single Fsync via BC
    TIDS90UB_FSYNC_MODE_INTERNAL,          // DES send internal Fsync via BC
    TIDS90UB_FSYNC_MODE_EXTERNAL,          // DES receives Fsync in GPIO then forwards it via BC
    TIDS90UB_FSYNC_MODE_EXTERNAL_SOC,      // DES receives Fsync from SOC then forwards it via BC
    TIDS90UB_FSYNC_MODE_EXTERNAL_SENSOR,   // DES receives Fsync in GPIO from forward channel
    TIDS90UB_FSYNC_MODE_MAX
}tids90ub_fsync_mode_t;

/**
 *  Fps value with interval of 5, starting from 10Hz 
 */
typedef enum
{
    TIDS90UB_FSYNC_FREQ_DEFAULT = 0,       // Fsync is default
    TIDS90UB_FSYNC_FREQ_10HZ,              // DES sends Fsync via BC at 10 FPS
    TIDS90UB_FSYNC_FREQ_15HZ,              // DES sends Fsync via BC at 15 FPS
    TIDS90UB_FSYNC_FREQ_20HZ,              // DES sends Fsync via BC at 20 FPS
    TIDS90UB_FSYNC_FREQ_25HZ,              // DES sends Fsync via BC at 25 FPS
    TIDS90UB_FSYNC_FREQ_30HZ,              // DES sends Fsync via BC at 30 FPS
    TIDS90UB_FSYNC_FREQ_35HZ,              // DES sends Fsync via BC at 35 FPS
    TIDS90UB_FSYNC_FREQ_40HZ,              // DES sends Fsync via BC at 40 FPS
    TIDS90UB_FSYNC_FREQ_45HZ,              // DES sends Fsync via BC at 45 FPS
    TIDS90UB_FSYNC_FREQ_50HZ,              // DES sends Fsync via BC at 50 FPS
    TIDS90UB_FSYNC_FREQ_55HZ,              // DES sends Fsync via BC at 55 FPS
    TIDS90UB_FSYNC_FREQ_60HZ,              // DES sends Fsync via BC at 60 FPS
    TIDS90UB_FSYNC_FREQ_MAX
}tids90ub_fsync_freq_t;

typedef enum
{
    TIDS90UB_GPIO_FSYNC_IN = 0,
    TIDS90UB_GPIO_FSYNC_OUT_DESERIALIZER,
    TIDS90UB_GPIO_FSYNC_OUT_SOC,
    TIDS90UB_GPIO_TYPE_MAX
}tids90ub_gpio_type_t;

typedef enum
{
    TIDS90UB_GPIO_INVALID = -1,
    TIDS90UB_GPIO_0 = 0,
    TIDS90UB_GPIO_1,
    TIDS90UB_GPIO_2,
    TIDS90UB_GPIO_3,
    TIDS90UB_GPIO_4,
    TIDS90UB_GPIO_5,
    TIDS90UB_GPIO_6,
    TIDS90UB_GPIO_7,
    TIDS90UB_GPIO_MAX
}tids90ub_gpio_num_t;

typedef enum
{
    TIDS90UB_TX_PORT_1 = 1,
    TIDS90UB_TX_PORT_2,
    TIDS90UB_TX_PORT_1_2
}tids90ub_tx_port_map_t;

typedef enum
{
    TIDS90UB_OP_MODE_DEFAULT = 0,
    TIDS90UB_CSI_TX_800M,        // Configuring the CSI rate at 800Mbps per lane
    TIDS90UB_CSI_TX_1P5G,        // Configuring the CSI rate at 1.5Gbps per lane  
    TIDS90UB_OP_MODE_MAX
}tids90ub_op_mode_t;


typedef struct
{
    tids90ub_sensor_id_t sensor_id;
    unsigned int         sensor_mode;
    unsigned int         fsync_mode;
    unsigned int         fsync_freq;
}tids90ub_topology_sensor_t;

/* configuration structure which holds the config params, will be updated
 * upon parsing the user provided xml file
 */
typedef struct
{
    tids90ub_deser_t           deser_type;
    tids90ub_topology_sensor_t sensors[TIDS90UB_PORT_MAX];
    int num_of_cameras;
    int gpio_num[MAX_NUM_INPUT_DEV_INTERNAL_GPIO];
    unsigned int fsync_mode;
    unsigned int fsync_freq;
    tids90ub_tx_port_map_t tx_port_map;
    enum i2c_access_mode access_mode;
    tids90ub_op_mode_t op_mode;
}tids90ub_config_t;

typedef enum
{
    TIDS90UB_SENSOR_STATE_INVALID = 0,
    TIDS90UB_SENSOR_STATE_SERIALIZER_DETECTED,
    TIDS90UB_SENSOR_STATE_DETECTED,
    TIDS90UB_SENSOR_STATE_INITIALIZED,
    TIDS90UB_SENSOR_STATE_STREAMING,
} tids90ub_slave_state_t;

#define MAX_PORT_SOURCES 2

typedef struct
{
    img_src_mode_t sources[MAX_PORT_SOURCES];
    unsigned int num_sources;
    tids90ub_sensor_id_t sensor_id;

    unsigned char port_config;
    unsigned char port_config2;

    unsigned int gpio_power_up;
    unsigned int gpio_power_down;
    unsigned int rxport_en;
    volatile unsigned int lock_status;

    struct camera_i2c_reg_array* deser_config;
    unsigned int deser_config_size;

}tids90ub_port_cfg_t;

typedef struct
{
    int (*detect)(tids90ub_context_t* ctxt, uint32 port);
    int (*get_port_cfg)(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* pCfg);

    int (*init_port)(tids90ub_context_t* ctxt, uint32 port);
    int (*start_port)(tids90ub_context_t* ctxt, uint32 port);
    int (*stop_port)(tids90ub_context_t* ctxt, uint32 port);

    int (*set_port_fsync)(tids90ub_context_t* ctxt, uint32 port);
    int (*set_port_mode)(tids90ub_context_t* ctxt, uint32 port, tids90ub_sensor_port_mode_t mode);

    int (*calculate_exposure)(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
    int (*apply_exposure)(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
    int (*apply_hdr_exposure)(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);

    int (*set_param)(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);
    int (*get_param)(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);
}tids90ub_sensor_t;

/* Structure to keep a copy of any sensor/ISP parameters set by the user */
typedef struct
{
    sensor_exposure_info_t              exposure_info;
    qcarcam_hdr_exposure_config_t       hdr_exposure_info;
    qcarcam_gamma_config_t              gamma_info;
    float                               hue_value;
    float                               saturation_value;
    float                               brightness_value;
    float                               contrast_value;
    uint32_t                            horizontal_mirroring;
    uint32_t                            vertical_mirroring;

}tids90ub_sensor_param_info_t;

/*Structure that holds information regarding slave devices*/
typedef struct
{
    tids90ub_slave_state_t state;

    unsigned int rxport;
    unsigned int rxport_en;

    unsigned int sensor_addr;
    unsigned int sensor_alias;

    unsigned int serializer_id;
    unsigned int serializer_addr;
    unsigned int serializer_alias;

    volatile unsigned int lock_status;

    tids90ub_port_cfg_t port_cfg;

    const tids90ub_sensor_t* pInterface;

    tids90ub_sensor_param_info_t sensor_params;

    void *pPrivCtxt;
} tids90ub_sensor_info_t;


#define TIDS90UB_MAX_INIT_SEQUENCE_SIZE 256

typedef enum
{
    TIDS90UB_STATE_INVALID = 0,
    TIDS90UB_STATE_DETECTED,
    TIDS90UB_STATE_INITIALIZED,
    TIDS90UB_STATE_SUSPEND,
    TIDS90UB_STATE_STREAMING,
}tids90ub_state_t;

struct tids90ub_context_t
{
    /*must be first element*/
    sensor_lib_t sensor_lib;

    CameraMutex mutex;

    tids90ub_state_t state;
    unsigned int streaming_src_mask;
    unsigned int rxport_en;

    unsigned int revision;
    unsigned int slave_addr;
    unsigned int slave_alias_group;
    unsigned int num_supported_sensors;
    unsigned int num_connected_sensors;

    unsigned int platform_fcn_tbl_initialized;
    sensor_platform_func_table_t platform_fcn_tbl;

    struct camera_i2c_reg_setting init_reg_settings;

    struct camera_i2c_reg_setting tids90ub_reg_setting;

    struct camera_i2c_reg_array init_seq[TIDS90UB_MAX_INIT_SEQUENCE_SIZE];

    tids90ub_config_t      config;
    tids90ub_sensor_info_t sensors[TIDS90UB_PORT_MAX];

    boolean enable_intr;
    volatile boolean intr_in_process;

    unsigned int subdev_id;
    void* ctrl;
};

#endif /* __TIDS90UB_LIB_H__ */

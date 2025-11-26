/**
 * @file CameraConfigSA8540.h
 *
 * @brief SA8540 Camera Board Definition
 *
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/*============================================================================
                        INCLUDE FILES
============================================================================*/
#include "CameraConfig.h"

// PMIC/TLMM pins and TLMM interrupt pins
#if defined(__QNXNTO__) && !defined(CAMERA_UNITTEST)
#include "gpio_devctl.h"
#elif defined(__INTEGRITY)
#include "tlmm_gpio_driver.h"
#else
#define GPIO_PIN_CFG(output, pull, strength, func) 0
#endif

/* ===========================================================================
                SA8540 Definitions
=========================================================================== */
/*                    CSI     lanes   IFEs   I2C Port    Sensors
 * ti9702_0/ti960_0    0        4      0|6    /dev/cci0    4 x IMX424 YUV SENSORS
 * ti9702_1/ti960_1    1        4      1|7    /dev/cci0    4 x IMX424 YUV SENSORS
 * ti9702_2/ti960_2    2        4      2|4    /dev/cci1    4 x IMX424 YUV SENSORS
 * ti9702_3            3        4      3|5    /dev/cci1    4 x IMX424 YUV SENSORS
*/

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifdef AIS_BUILD_STATIC_DEVICES
#if defined(CAMERA_CONFIG_SENSORSTUB)
extern CameraResult CameraSensorDevice_Open_sensorstub(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#else
extern CameraResult CameraSensorDevice_Open_tids90ub(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#endif
#endif

/* --------------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
static const CameraBoardType cameraBoardDefinition =
{ // Configuration
#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
    .boardType = CAMERA_HW_BOARD_QDRIVE3P0_PM134,
#else
    .boardType = CAMERA_HW_BOARD_QDRIVE3P0_PM135,
#endif
    .boardName = "SA8540_ADP",
    .i2c =
    {
        {
            .i2cDevname = "cci0",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 0,
            .port_id = 0,
            .sda_pin = {
                .num = 113,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 114,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
        {
            .i2cDevname = "cci1",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 0,
            .port_id = 1,
            .sda_pin = {
                .num = 115,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 116,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
        {
            .i2cDevname = "cci2",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 1,
            .port_id = 0,
            .sda_pin = {
                .num = 10,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 11,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
        {
            .i2cDevname = "cci3",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 1,
            .port_id = 1,
            .sda_pin = {
                .num = 12,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 13,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        }
    },
    .camera =
    {
#if defined(CAMERA_CONFIG_SENSORSTUB)
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_sensorstub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_sensorstub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_sensorstub,
#endif
            },
            .devConfig = {
                    .subdevId = 0,
            },
            .csiInfo = {
                {
                .csiId = 0,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x6420,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
            },
            .i2cPort = {
                .i2ctype = CAMERA_I2C_TYPE_CCI,
                .device_id = 0,
                .port_id = 0,
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_sensorstub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_sensorstub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_sensorstub,
#endif
            },
            .devConfig = {
                    .subdevId = 0,
            },
            .csiInfo = {
                {
                .csiId = 1,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x7531,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
            },
            .i2cPort = {
                .i2ctype = CAMERA_I2C_TYPE_CCI,
                .device_id = 0,
                .port_id = 0,
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }
#else
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_tids90ub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_tids90ub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_tids90ub,
#endif
            },
            .devConfig = {
                    /*TI9702 - TI960 + 4x OX3A10*/
                    .subdevId = 1, /*0x60*/
#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
                    .type = 4,
#else
                    .type = 3,
#endif
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x3210,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 117,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype = CAMERA_I2C_TYPE_CCI,
                    .device_id = 0,
                    .port_id = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_tids90ub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_tids90ub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_tids90ub,
#endif
            },
            .devConfig = {
                    /*TI9702 - TI960 + 4x OX3A10*/
                    .subdevId = 0, /*0x7A*/
#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
                    .type = 4,
#else
                    .type = 3,
#endif
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 1,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x7654,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 118,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype = CAMERA_I2C_TYPE_CCI,
                    .device_id = 0,
                    .port_id = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_2,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_tids90ub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_tids90ub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_tids90ub,
#endif
            },
            .devConfig = {
                    /*TI9702 - TI960 + 4x OX3A10*/

#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
                    .subdevId = 1, /*0x60*/
#else
                    .subdevId = 2, /*0x64*/
#endif
                    .type = 4,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 2,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x7654,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 22,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype = CAMERA_I2C_TYPE_CCI,
                    .device_id = 0,
                    .port_id = 1,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
        {
            .devId = CAMERADEVICEID_INPUT_3,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_tids90ub",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_tids90ub",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_tids90ub,
#endif
            },
            .devConfig = {
                    /*TI9702 + 4x OX3A10*/
                    .subdevId = 0, /*0x7A*/
                    .type = 4,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 5, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 3,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x7654,
                .numIfeMap = 4,
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 23,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype = CAMERA_I2C_TYPE_CCI,
                    .device_id = 0,
                    .port_id = 1,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }

#endif /* CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD */
#endif
    },
    .engineSetting =
    {
        .numBufMax             = MAX_NUM_CAMERA_BUFFERS_DEFAULT,
        .powerManagementPolicy = CAMERA_PM_POLICY_LPM_EVENT_ALL,
        .latencyMeasurementMode = CAMERA_LM_MODE_DISABLE,
        .recoveryRestartDelay = RECOVERY_DEFAULT_RESTART_DELAY,
        .recoveryTimeoutAfterUsrCtxtRestart = RECOVERY_DEFAULT_TIMEOUT_AFTER_RESTART,
        .recoveryRetryDelay = RECOVERY_DEFAULT_RETRY_DELAY,
        .recoveryMaxNumAttempts = RECOVERY_DEFAULT_MAX_ATTEMPTS,
        .multiIfeInitFrameDrop = 1,
        .customMatchFunctions  =
        {
            {.type = CAMERA_CONFIG_MATCH_INPUT_DEVICE, .severity = 0},
            {.type = CAMERA_CONFIG_MATCH_IFE_DEVICE, .severity = 0},
            {.type = CAMERA_CONFIG_MATCH_IFE_DEVICE, .severity = 0}
        }
    }
};

static const CameraChannelInfoType InputDeviceChannelInfo[] =
{
#if defined(CAMERA_CONFIG_SENSORSTUB)
    { /*paired stream from stub0 and stub1*/
        .aisInputId = 50,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 },
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_PAIRED_INPUT
    },
    { /*0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*1*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    }
#else
    { /*TI9702_0 - TI960_0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0 - TI960_0*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0 - TI960_0*/
        .aisInputId = 2,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0 - TI960_0*/
        .aisInputId = 3,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 3 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1 - TI960_1*/
        .aisInputId = 4,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1 - TI960_1*/
        .aisInputId = 5,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1 - TI960_1*/
        .aisInputId = 8,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1 - TI960_1*/
        .aisInputId = 9,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 3 }
        },
    },
    { /*TI9702_2 - TI960_2*/
        .aisInputId = 12,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_2 - TI960_2*/
        .aisInputId = 13,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_2 - TI960_2*/
        .aisInputId = 14,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_2 - TI960_2*/
        .aisInputId = 15,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 3 }
        },
    },
#if defined(CAMERA_CONFIG_TI9702_ACP4_DAUGHTER_CARD)
    { /*TI9702_3*/
        .aisInputId = 16,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_3*/
        .aisInputId = 17,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_3*/
        .aisInputId = 18,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_3*/
        .aisInputId = 19,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 3 }
        },
    }
#endif
#endif
};

/* --------------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* ===========================================================================
**                          Macro Definitions
** ======================================================================== */

/* ===========================================================================
**                          Local Function Definitions
** ======================================================================== */
int SA8540_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        int *nChannels)
{
    int rtnVal = -1;
    if (ppBoardInfo && ppChannelInfo && nChannels)
    {
        *ppBoardInfo   = &cameraBoardDefinition;
        *ppChannelInfo = InputDeviceChannelInfo;
        *nChannels     = sizeof(InputDeviceChannelInfo)/sizeof(InputDeviceChannelInfo[0]);
        rtnVal         = 0;
    }

    return rtnVal;
}


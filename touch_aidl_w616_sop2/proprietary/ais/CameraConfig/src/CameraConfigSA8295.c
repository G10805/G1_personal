/**
 * @file CameraConfigSA8295.h
 *
 * @brief SA8295 Camera Board Definition
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
                SA8295 Definitions
=========================================================================== */
/*                    CSI     lanes   IFEs   I2C Port    Sensors
 * max96712_0          0        4      0|6    /dev/cci0    4 x SENSORS
 * max96712_1          1        4      1|7    /dev/cci0    4 x SENSORS
 * max96712_2          2        4      2|4    /dev/cci1    4 x SENSORS
 * max96712_3          3        4      3|5    /dev/cci1    4 x SENSORS
*/

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifdef AIS_BUILD_STATIC_DEVICES
extern CameraResult CameraSensorDevice_Open_max96712(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#endif

/* --------------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
static const CameraBoardType cameraBoardDefinition =
{ // Configuration
    .boardType = CAMERA_HW_BOARD_ADPAIR_GEN4_V1_PW227,
    .boardName = "SA8295_ADP",
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
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max96712",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max96712",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max96712,
#endif
            },
            .devConfig = {
                    .subdevId = 0,
                    .type = 1,
                    .numSensors = 4,
                    .opMode = 0,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            //AR0231 YUV8 CROP META
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                    }
            },
            .csiInfo = {
                {
                .csiId = 0,
                .numLanes = 2,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x40,
                .numIfeMap = 2,
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
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 162, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max96712",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max96712",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max96712,
#endif
            },
            .devConfig = {
                    .subdevId = 1,
                    .type = 1,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            //AR0231 YUV8 CROP META
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                    }
            },
            .csiInfo = {
                {
                .csiId = 1,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x51,
                .numIfeMap = 2,
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
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 163, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_2,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max96712",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max96712",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max96712,
#endif
            },
            .devConfig = {
                    .subdevId = 2,
                    .type = 1,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            //AR0231 YUV8 CROP META
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                    }
            },
            .csiInfo = {
                {
                .csiId = 2,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x62,
                .numIfeMap = 2,
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
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 16, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_3,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max96712",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max96712",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max96712,
#endif
            },
            .devConfig = {
                    .subdevId = 3,
                    .type = 1,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            //AR0231 YUV8 CROP META
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                            {.type = 3, .snsrModeId = 0, .fsyncMode = 0, .fsyncFreq= 0},
                    }
            },
            .csiInfo = {
                {
                .csiId = 3,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x73,
                .numIfeMap = 2,
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
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 17, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }
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
    { /*max96712_0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_0*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_0*/
        .aisInputId = 2,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_0*/
        .aisInputId = 3,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 3 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_1*/
        .aisInputId = 4,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_1*/
        .aisInputId = 5,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_1*/
        .aisInputId = 8,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_1*/
        .aisInputId = 9,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 3 }
        },
    },
    { /*max96712_2*/
        .aisInputId = 12,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_2*/
        .aisInputId = 13,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_2*/
        .aisInputId = 14,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_2*/
        .aisInputId = 15,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 3 }
        },
    },
    { /*max96712_3*/
        .aisInputId = 16,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_3*/
        .aisInputId = 17,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_3*/
        .aisInputId = 18,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max96712_3*/
        .aisInputId = 19,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 3 }
        },
    }
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
int SA8295_GetCameraConfig(CameraBoardType const **ppBoardInfo,
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


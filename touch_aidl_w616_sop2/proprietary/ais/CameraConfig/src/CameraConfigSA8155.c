/**
 * @file CameraConfigSA8155.h
 *
 * @brief SA8155 Camera Board Definition
 *
 * Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
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
                SA8155 Definitions
=========================================================================== */
/*              CSI     lanes   IFEs   I2C Port    Sensors
 * max9296_0     0        4      0    /dev/cci0    2 x AR0231 YUV SENSORS
 * max9296_1     1        4      1    /dev/cci0    2 x AR0231 YUV SENSORS
 * max9296_2     2        4      2    /dev/cci1    2 x AR0231 YUV SENSORS
 * max9296_3     3        4      3    /dev/cci1    2 x AR0231 YUV SENSORS
*/

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifdef AIS_BUILD_STATIC_DEVICES
#if defined(CAMERA_CONFIG_TI9702_DAUGHTER_CARD) || defined(CAMERA_CONFIG_TI960_DAUGHTER_CARD)
extern CameraResult CameraSensorDevice_Open_tids90ub(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#elif defined(CAMERA_CONFIG_SENSORSTUB)
extern CameraResult CameraSensorDevice_Open_sensorstub(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#elif defined(CAMERA_CONFIG_MX9671_DAUGHTER_CARD)
extern CameraResult CameraSensorDevice_Open_max96714(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
extern CameraResult CameraSensorDevice_Open_ba(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId);
#else
extern CameraResult CameraSensorDevice_Open_max9296(CameraDeviceHandleType** ppNewHandle,
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
    .boardType = CAMERA_HW_BOARD_ADPAIR_V2_PL195,
    .boardName = "SA8155_ADP",
    .i2c =
    {
        {
            .i2cDevname = "cci0",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 0,
            .port_id = 0,
            .sda_pin = {
                .num = 17,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 18,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
        {
            .i2cDevname = "cci1",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 0,
            .port_id = 1,
            .sda_pin = {
                .num = 19,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 20,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
#ifdef ENABLE_CCI1_DEVICE
        {
            .i2cDevname = "cci2",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 1,
            .port_id = 0,
            .sda_pin = {
                .num = 31,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 32,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        },
        {
            .i2cDevname = "cci3",
            .i2ctype = CAMERA_I2C_TYPE_CCI,
            .device_id = 1,
            .port_id = 1,
            .sda_pin = {
                .num = 33,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
            .scl_pin = {
                .num = 34,
                .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 1),
            },
        }
#endif
    },
    .camera =
    {
#if defined(CAMERA_CONFIG_TI9702_DAUGHTER_CARD)
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .detectThrdId = 0,
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
                    .subdevId = 1, /*0x60*/
                    .type = 4,
                    .numSensors = 4,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .sensors = {
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x20,
                .numIfeMap = 2
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 21,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
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
            .detectThrdId = 1,
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
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0},
                            {.type = 1, .snsrModeId = 0, .fsyncMode = 0}
                    }
            },
            .csiInfo = {
                {
                .csiId = 2,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x31,
                .numIfeMap = 2
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
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 1,
                    .port_id    = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }
#elif defined(CAMERA_CONFIG_SENSORSTUB)
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .detectThrdId = 0,
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
                .ifeMap = 0x0,
                .numIfeMap = 1,
                },
                {
                .csiId = 1,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x1,
                .numIfeMap = 1,
                }
            },
            .gpioConfig = {
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
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
            .detectThrdId = 1,
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
                .csiId = 2,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x2,
                .numIfeMap = 1,
                }
                {
                .csiId = 3,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x3,
                .numIfeMap = 1,
                }
            },
            .gpioConfig = {
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }
#elif defined(CAMERA_CONFIG_MX9671_DAUGHTER_CARD)
        /*Max96714/CSI1/CCI1 */
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .detectThrdId = 0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max96714",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max96714",
#ifdef AIS_BUILD_STATIC_DEVICES
        .pfnCameraDeviceOpen = CameraSensorDevice_Open_max96714,
#endif
            },
            .devConfig = {
                .subdevId = 0,
                .type = 0,
                .numSensors = 1,
                .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
            },
            .csiInfo = {
                {
                .csiId = 1,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x1,
                .numIfeMap = 1
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
                .port_id = 1
                }
            },
            .intr = {
                {.gpio_id = CAMERA_GPIO_INVALID,},
                {.gpio_id = CAMERA_GPIO_INVALID,},
                {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        /*CVBS/CSI0/CCI0 */
        {
            .devId = CAMERADEVICEID_INPUT_1,
            .detectThrdId = 1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_ba",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_ba",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_ba,
#endif
            },
            .devConfig = {
                .subdevId = 0,
                .type = 0,
                .numSensors = 1,
                .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
            },
            .csiInfo = {
                {
                .csiId = 0,
                .isSecure = 0,
                .numLanes = 1,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x2,
                .numIfeMap = 1
                }
            },
            .i2cPort = {
                {
                .i2ctype = CAMERA_I2C_TYPE_CCI,
                .device_id = 0,
                .port_id = 0
                }
            },
            .intr = {
                {.gpio_id = CAMERA_GPIO_INVALID,},
                {.gpio_id = CAMERA_GPIO_INVALID,},
                {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
#elif defined(CAMERA_CONFIG_TI960_DAUGHTER_CARD)
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .detectThrdId = 0,
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
                    .subdevId = 0, /*0x60*/
                    .type = 3,
                    .numSensors = 1,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .gpio = {-1, -1, -1},
                    .accessMode = I2C_FULL_ACCESS,
                    .sensors = {
                            {.type = 254, .snsrModeId = 6,  .fsyncMode = 0},
                    }
            },
            .csiInfo = {
                {
                .csiId = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x60,
                .numIfeMap = 2
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 21,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
                    .speed = I2C_SPEED_400,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 13, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        }
#else
        {
            .devId = CAMERADEVICEID_INPUT_0,
            .detectThrdId = 0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max9296",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max9296",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max9296,
#endif
            },
            .devConfig = {
                    .subdevId = 0,
                    /* type = 0 will use lib default.
                     * Change type to 1 to apply config below
                     */
                    .type = 0,
                    /*2x AR0231 YUV*/
                    .numSensors = 2,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .sensors = {
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601},
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601}
                    }
            },
            .csiInfo = {
                {
                .csiId = 0,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x0,
                .numIfeMap = 1
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 21,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 13, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_1,
            .detectThrdId = 0,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max9296",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max9296",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max9296,
#endif
            },
            .devConfig = {
                    .subdevId = 1,
                    /* type = 0 will use lib default.
                     * Change type to 1 to apply config below
                     */
                    .type = 0,
                    /*2x AR0231 YUV*/
                    .numSensors = 2,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .sensors = {
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601},
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601}
                    }
            },
            .csiInfo = {
                {
                .csiId = 1,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x1,
                .numIfeMap = 1
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
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 0,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 14, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_2,
            .detectThrdId = 1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max9296",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max9296",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max9296,
#endif
            },
            .devConfig = {
                    .subdevId = 2,
                    /* type = 0 will use lib default.
                     * Change type to 1 to apply config below
                     */
                    .type = 0,
                    .numSensors = 2,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .sensors = {
                            /*2x AR0231 YUV*/
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601},
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601}
                    }
            },
            .csiInfo = {
                {
                .csiId = 2,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x2,
                .numIfeMap = 1
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
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 1,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 15, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
        {
            .devId = CAMERADEVICEID_INPUT_3,
            .detectThrdId = 1,
            .driverInfo = {
                .deviceCategory = CAMERA_DEVICE_CATEGORY_SENSOR,
                .strDeviceLibraryName = "ais_max9296",
                .strCameraDeviceOpenFn = "CameraSensorDevice_Open_max9296",
#ifdef AIS_BUILD_STATIC_DEVICES
                .pfnCameraDeviceOpen = CameraSensorDevice_Open_max9296,
#endif
            },
            .devConfig = {
                    .subdevId = 3,
                    /* type = 0 will use lib default.
                     * Change type to 1 to apply config below
                     */
                    .type = 0,
                    .numSensors = 2,
                    .powerSaveMode = CAMERA_POWERSAVE_MODE_POWEROFF,
                    .sensors = {
                            /*2x AR0231 YUV*/
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601},
                            {.type = 2, .snsrModeId = 0, .colorSpace = QCARCAM_COLOR_SPACE_BT601}
                    }
            },
            .csiInfo = {
                {
                .csiId = 3,
                .isSecure = 0,
                .numLanes = 4,
                .laneAssign = CAMERA_DEFAULT_CSI_LANE_ASSIGN,
                .ifeMap = 0x3,
                .numIfeMap = 1
                }
            },
            .gpioConfig = {
                    [CAMERA_GPIO_RESET] = {
                        .num = 25,
                        .config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, 0),
                    }
            },
            .i2cPort = {
                {
                    .i2ctype    = CAMERA_I2C_TYPE_CCI,
                    .device_id  = 0,
                    .port_id    = 1,
                },
            },
            .intr = {
                    {.gpio_id = CAMERA_GPIO_INTR, .pin_id = 16, .intr_type = CAMERA_GPIO_INTR_TLMM,
                            .trigger = CAMERA_GPIO_TRIGGER_FALLING, .gpio_cfg = {0, 0x30, 0, 0}},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
                    {.gpio_id = CAMERA_GPIO_INVALID,},
            }
        },
#endif /* CAMERA_CONFIG_TI9702_DAUGHTER_CARD */
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
#if defined(CAMERA_CONFIG_TI9702_DAUGHTER_CARD) || defined(CAMERA_CONFIG_TI960_DAUGHTER_CARD)
    { /*TI9702_0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0*/
        .aisInputId = 2,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_0*/
        .aisInputId = 3,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 3 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1*/
        .aisInputId = 4,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1*/
        .aisInputId = 5,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1*/
        .aisInputId = 8,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 2 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*TI9702_1*/
        .aisInputId = 9,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 3 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    }
#elif defined(CAMERA_CONFIG_SENSORSTUB)
    { /*paired stream from stub0 CSI0/1*/
        .aisInputId = 50,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0, .csiIdx = 0},
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1, .csiIdx = 1}
        },
        .opMode = QCARCAM_OPMODE_PAIRED_INPUT
    },
    { /*paired stream from stub1 CSI2/3*/
        .aisInputId = 51,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0, .csiIdx = 0},
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1, .csiIdx = 1}
        },
        .opMode = QCARCAM_OPMODE_PAIRED_INPUT
    },
    { /*csi 0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0, .csiIdx = 0}
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*csi 1*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1, .csiIdx = 1}
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    }
    { /*csi 2*/
        .aisInputId = 2,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0, .csiIdx = 0}
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*csi 3*/
        .aisInputId = 3,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1, .csiIdx = 1}
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    }
#elif defined(CAMERA_CONFIG_MX9671_DAUGHTER_CARD)
    { /*Max96714*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /* adv7481 CVBS */
        .aisInputId = 7,
        inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    }
#else
    { /*max9296_0*/
        .aisInputId = 0,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_0*/
        .aisInputId = 1,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_1*/
        .aisInputId = 2,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_1*/
        .aisInputId = 3,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_1, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_2*/
        .aisInputId = 4,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_2*/
        .aisInputId = 5,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_2, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_3*/
        .aisInputId = 8,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*max9296_3*/
        .aisInputId = 9,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_3, .srcId = 1 }
        },
        .opMode = QCARCAM_OPMODE_RAW_DUMP
    },
    { /*paired stream from input 0*/
        .aisInputId = 50,
        .inputSrc = {
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 },
            { .devId = CAMERADEVICEID_INPUT_0, .srcId = 0 }
        },
        .opMode = QCARCAM_OPMODE_PAIRED_INPUT
    }
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
int SA8155_GetCameraConfig(CameraBoardType const **ppBoardInfo,
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


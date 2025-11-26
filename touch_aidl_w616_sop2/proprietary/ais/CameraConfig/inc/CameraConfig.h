#ifndef __CAMERACONFIG_H_
#define __CAMERACONFIG_H_

/**
 * @file CameraConfig.h
 *
 * @brief This header file defines the interface for the CameraConfig library
 *
 * Copyright (c) 2011-2012, 2014, 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
                        INCLUDE FILES FOR MODULE
=========================================================================== */
#include <stdint.h>
#include "CameraDeviceManager.h"
#include "sensor_sdk_common.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* ===========================================================================
                        DATA DECLARATIONS
=========================================================================== */

/* ---------------------------------------------------------------------------
** Constant / Define Declarations
** ------------------------------------------------------------------------ */
#define MAX_NUM_CAMERA_CHANNELS                    32
#define MAX_NUM_CAMERA_INPUT_DEVS                   8
#define MAX_NUM_CAMERA_I2C_DEVS                     8
#define MAX_CAMERA_DEV_INTR_PINS                    3
#define MAX_CAMERA_DEVICE_NAME                     64
#define MAX_NUM_CAMERA_BUFFERS_DEFAULT             12
#define MAX_DETECT_THREAD_IDX                       4
#define PRE_SUF_FIX_SIZE                            6 //reserve 6 bytes for prefix "lib" and suffix ".so"
#define MAX_I2C_DEVICE_NAME                        16
#define CAMERA_SENSOR_GPIO_INVALID                 0x7FFFFFFF
#define CAMERA_DEFAULT_CSI_LANE_ASSIGN             0x3210
#define RECOVERY_DEFAULT_RESTART_DELAY             33
#define RECOVERY_DEFAULT_TIMEOUT_AFTER_RESTART    500
#define RECOVERY_DEFAULT_RETRY_DELAY              300
#define RECOVERY_DEFAULT_MAX_ATTEMPTS               1

#define CAMERA_BOARD_LIBRARY                       "libais_config.so"
#define GET_CAMERA_CONFIG_INTERFACE                "GetCameraConfigInterface"

#define CAMERA_BOARD_LIBRARY_VERSION               0x403

#if defined(__QNXNTO__)
#define CAMERA_CONFIG_XML_FILE   "/var/camera_config.xml"
#elif defined(__AGL__)
#if defined(AIS_DATADIR_ENABLE)
#define CAMERA_CONFIG_XML_FILE "/usr/share/camera/camera_config.xml"
#elif !defined(AIS_EARLYSERVICE)
#define CAMERA_CONFIG_XML_FILE   "/data/misc/camera/camera_config.xml"
#else
#define CAMERA_CONFIG_XML_FILE   "/usr/bin/camera_config.xml"
#endif
#else
#define CAMERA_CONFIG_XML_FILE   "/vendor/etc/camera/camera_config.xml"
#endif


/* ---------------------------------------------------------------------------
** Type Declarations
** ------------------------------------------------------------------------ */
/**
 * This structure defines GPIO pin used by camera
 */
typedef struct {
    uint32 num;       /**< GPIO pin number */
    uint32 config;    /**< GPIO pin configuration */
} CameraGPIOPinType;

/**
 * This structure defines CSI info used by camera
 */
typedef struct {
    uint8  csiId;      /**< CSI port id */
    uint8  isSecure;   /**< set to (1) for secure or content protected CSI*/
    uint8  numLanes;   /**< Number of MIPI CSI physical lanes connected */
    uint16 laneAssign; /**< CSI Lane mapping configuration where each nibble (4bits), starting from the LSB
                            up to numLanes, assigns the physical lane.
                            The default is 0x3210 (no swaps).
                            For example 0x3120 can be set in case of lane 1 and 2 are swapped
                            */
    uint32 ifeMap;     /**< Mappings to IFE interfaces where each IFE index is expressed as a nibble (4bits).
                            The IFE priority is set in priority starting from the nibble at the LSB.
                            For instance, 0x31 would map starting with IFE 1 then IFE 3.
                            */
    uint8  numIfeMap;  /**< Number of IFE mappings in ifeMap (number of nibbles valid from LSB)
                            If 0, ifeMap is ignored and the IFE mapping will default to csiId */

    uint8  forceHSmode;  /**< used in multiCSI case */
} CameraCsiInfo;

/**
 * This enum describes available I2C port speeds
 */
typedef enum {
    I2C_SPEED_POW_CONSERVE = 0,
    I2C_SPEED_100,
    I2C_SPEED_400,
} I2CSpeedType;

/**
 * This enum describes available I2C port types
 */
typedef enum {
    CAMERA_I2C_TYPE_NONE = 0,
    CAMERA_I2C_TYPE_CCI,
    CAMERA_I2C_TYPE_I2C,
} CameraI2CType;

/**
 * This enum describes available SOC Ids
 */
typedef enum
{
    SOC_ID_0 = 0,
    SOC_ID_1,
    SOC_ID_MAX
} CameraSocId;

/**
 * Error Event IDs
 */
typedef enum
{
    CAMERA_CONFIG_EVENT_INPUT_FATAL_ERROR,
    CAMERA_CONFIG_EVENT_CSID_FATAL_ERROR,
    CAMERA_CONFIG_EVENT_IFE_OUTPUT_ERROR,
    CAMERA_CONFIG_EVENT_NUM
}CameraConfigErrorEventId;

/**
 * This enum should match AisResMgrMatchType in ais_res_mgr.h
 */
typedef enum
{
    CAMERA_CONFIG_MATCH_IFE_PATH = 0,
    CAMERA_CONFIG_MATCH_IFE_DEVICE,
    CAMERA_CONFIG_MATCH_CSI_DEVICE,
    CAMERA_CONFIG_MATCH_INPUT_SRC,
    CAMERA_CONFIG_MATCH_INPUT_DEVICE
}CameraConfigMatchType;

/**
 * This structure describes I2C Devices
 */
typedef struct {
    CameraI2CType i2ctype;      /**< CCI or I2C port */
    uint32 device_id;           /**< CCI device number */
    uint32 port_id;             /**< I2C port number */
    CameraGPIOPinType sda_pin;  /**< I2C SDA pin configuration */
    CameraGPIOPinType scl_pin;  /**< I2C SCL pin configuration */
    char i2cDevname[MAX_I2C_DEVICE_NAME];    /**< i2c port used by sensor */
} CameraI2CDeviceType;

/**
 * This structure describes I2C port used by the camera
 */
typedef struct {
    CameraI2CType i2ctype; /**< CCI or I2C port */
    uint32 device_id;      /**< CCI device number */
    uint32 port_id;        /**< I2C port number */
    I2CSpeedType speed;    /**< I2C port speed */
} CameraI2CPortType;

/**
 * This structure holds device driver information
 */
typedef struct
{
    /**
     * The type of the device driver (camera sensor or flash)
     */
    CameraDeviceCategoryType deviceCategory;

    /**
     * The name of the shared object containing the camera device driver.
     */
    char strDeviceLibraryName[MAX_CAMERA_DEVICE_NAME];

    /**
     * Name of the device driver open function
     */
    char strCameraDeviceOpenFn[MAX_CAMERA_DEVICE_NAME];

#ifdef AIS_BUILD_STATIC_DEVICES
    /**
     * Device Open function (for static linking)
     */
    CameraDeviceOpenType pfnCameraDeviceOpen;
#endif

} CameraDeviceDriverInfoType;

/**
 * This structure defines the board specific configurable items for a given
 * camera sensor.
 */
typedef struct
{
    /**< unique device id */
    CameraDeviceIDType devId;

    /**< Driver config */
    CameraDeviceConfigType      devConfig;

    /**< Driver loading info */
    CameraDeviceDriverInfoType  driverInfo;

    /**< Driver detection thread id */
    uint32 detectThrdId;

    /**< CSI info */
    CameraCsiInfo csiInfo[CSIPHY_CORE_MAX];

    /*GPIO config*/
    CameraGPIOPinType gpioConfig[CAMERA_GPIO_MAX];

    /*Interrupt config*/
    CameraSensorGPIO_IntrPinType intr[MAX_CAMERA_DEV_INTR_PINS];

    /**< I2C port */
    CameraI2CPortType i2cPort[SOC_ID_MAX];

} CameraSensorBoardType;

typedef struct
{
    /**< What resource to match against.*/
    CameraConfigMatchType type;

    /**< Custom vendor severity of the error.*/
    uint32 severity;
} CameraConfigCustomMatchFunc;


typedef struct
{
    /**< Maximum number of buffers per client */
    uint32 numBufMax;

    /**< power suspend/resume manager policy */
    CameraPowerManagerPolicyType powerManagementPolicy;

    /**< frame delivery latency measurement */
    CameraLatencyMeasurementModeType latencyMeasurementMode;

    /**< customize matching functions for events. This matching criteria is used to
          determine impacted clients for a particular event. */
    CameraConfigCustomMatchFunc customMatchFunctions[CAMERA_CONFIG_EVENT_NUM];

    /**< How many milliseconds to wait between calling start and stop of a user context
     * while in recovery.
    */
    uint32 recoveryRestartDelay;

    /**< For how many milliseconds to wait to receive a frame done event, after calling
     * Start() on a user context. Not receiving this event in this time frame
     * means that the recovery has failed.
     */
    uint32 recoveryTimeoutAfterUsrCtxtRestart;

    /**< For how many milliseconds to wait after a recovery failure before attempting
     * another recovery.
     */
    uint32 recoveryRetryDelay;

    /**< Maximum number of times the engine will try to recover from a single
     * error.
     */
    uint32  recoveryMaxNumAttempts;

    /**< init frame drop for mapping to multi-ife */
    uint32 multiIfeInitFrameDrop;
} CameraEngineSettings;

/**
 * This structure defines the board specific configurable items related to the
 * camera subsystem.
 */
typedef struct
{
    CameraHwBoardType boardType;

    /**< MultiSoc Enviroment */
    uint32 multiSocEnable;

    /**< Board type name */
    char boardName[MAX_CAMERA_DEVICE_NAME];

    /**< Camera configurations */
    CameraSensorBoardType camera[MAX_NUM_CAMERA_INPUT_DEVS];

    /**< I2C device configurations*/
    CameraI2CDeviceType i2c[MAX_NUM_CAMERA_I2C_DEVS];

    /**< Engine settings */
    CameraEngineSettings engineSetting;
} CameraBoardType;


/**
 * This structure defines an interface for camera board specific configurations
 * and methods.
 */
typedef struct
{
    /**
     * This method initializes the camera config
     * @return 0 SUCCESS and -1 otherwise
     */
    int (*CameraConfigInit)(void);

    /**
     * This method deinits the camera config
     * @return 0 SUCCESS and -1 otherwise
     */
    int (*CameraConfigDeInit)(void);

    /**
     * This method returns the CameraBoardType for the current hardware platform.
     * @return CameraBoardType Camera board configuration information.
     */
    CameraBoardType const* (*GetCameraBoardInfo)(void);

    /**
     * This method returns the Camera Board Config version
     * @return version
     */
    int (*GetCameraConfigVersion)(void);

    /*
     * This method returns the array of channel config info and its
     * size in no. of elements (channels).
     */
    int (*GetCameraChannelInfo)(CameraChannelInfoType const **ppChannelInfo, int *nChannels);
} ICameraConfig;

/*
 * Defined type for method GetCameraConfigInterface.
 */
typedef ICameraConfig const* (*GetCameraConfigInterfaceType)(void);

/*
 * This method is used to retrieve the camera board config interface pointer.
 * @return pointer to the interface.
 */
ICameraConfig const* GetCameraConfigInterface(void);


#ifdef __cplusplus
} // extern "C"
#endif  // __cplusplus

#endif // __CAMERADEVICEMANAGERQNX_H_

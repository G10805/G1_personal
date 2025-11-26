#ifndef __CAMERASENSORDEVICEINTERFACE_H_
#define __CAMERASENSORDEVICEINTERFACE_H_

/**
 * @file CameraSensorDeviceInterface.h
 *
 * @brief Camera Sensor Device interface
 *
 * Copyright (c) 2016-2017, 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ========================================================================== */
/*                        INCLUDE FILES                                       */
/* ========================================================================== */
#include "CameraDevice.h"
#include "CameraOSServices.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* ========================================================================== */
/*                        DATA DECLARATIONS                                   */
/* ========================================================================== */

/* ========================================================================== */
/*                        TYPE DECLARATIONS                                   */
/* ========================================================================== */
/**
 * SensorOpenLibType
 *
 * @brief  Sensor Driver entry point function
 *
 * @param ctrl : pointer to sensor driver framework context.
 * This is to be provided by the sensor driver library as an argument to any framework API.
 *
 * @param arg : pointer to sensor_open_lib_t.
 * This structure contains CameraConfig deviceConfig paramters that can be used
 * by the driver to initialize its instance.
 *
 * @return sensor_lib_t*   pointer to sensor library interface
 */
typedef void* (*SensorOpenLibType)(void* ctrl, void* arg);

/**
 * Sensor Library Interface
 */
typedef struct
{
    SensorOpenLibType sensor_open_lib;
} sensor_lib_interface_t;

/* ========================================================================== */
/*                        FUNCTION DECLARATIONS                               */
/* ========================================================================== */
CAM_API CameraResult CameraSensorDevice_Open(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId, sensor_lib_interface_t* pSensorLibInterface);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* __CAMERASENSORDEVICEINTERFACE_H_ */

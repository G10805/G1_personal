/**
 * @file CameraSensorDevice.cpp
 *
 * This file contains the implementation of the CameraDevice API for the
 * Common Camera Sensor Driver.
 *
 * Copyright (c) 2014-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* ========================================================================== */
/*                        INCLUDE FILES                                       */
/* ========================================================================== */
#include "CameraResult.h"
#include "CameraOSServices.h"

#include "AEEstd.h"
#include <string.h>

#include "CameraDeviceManager.h"
#include "CameraSensorDeviceInterface.h"
#include "SensorDriver.h"

/* ========================================================================== */
/*                        DATA DECLARATIONS                                   */
/* ========================================================================== */

/* -------------------------------------------------------------------------- */
/* Constant / Define Declarations                                             */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Type Declarations                                                          */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Global Object Definitions                                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Local Object Definitions                                                   */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* Forward Declarations                                                       */
/* -------------------------------------------------------------------------- */


/* **************************************************************************
 * Camera Device Interface
 * ************************************************************************** */

/* ========================================================================== */
/*                        MACRO DEFINITIONS                                   */
/* ========================================================================== */

/* ========================================================================== */
/*                        FUNCTION DEFINITIONS                                */
/* ========================================================================== */
extern "C" CameraResult CameraSensorDevice_Open(CameraDeviceHandleType** ppNewHandle,
        CameraDeviceIDType deviceId, sensor_lib_interface_t* pSensorLibInterface)
{
    CameraResult result = CAMERA_SUCCESS;
    CameraSensorDevice* pCameraSensorDevice = NULL;

    AIS_LOG(SENSOR_DEV, HIGH, "Opening device 0x%x", deviceId);

    // Sanity check input parameters
    if (ppNewHandle == NULL || pSensorLibInterface == NULL)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "null params");
        return CAMERA_EMEMPTR;
    }

    // Allocate instance memory
    pCameraSensorDevice = new CameraSensorDevice(deviceId, pSensorLibInterface);
    if (pCameraSensorDevice != NULL)
    {
        result = pCameraSensorDevice->Initialize();

        if (CAMERA_SUCCESS == result)
        {
            *ppNewHandle = pCameraSensorDevice;
        }
        else
        {
            delete pCameraSensorDevice;
            *ppNewHandle = NULL;
        }
    }
    else
    {
        result = CAMERA_ENOMEMORY;
    }

    AIS_LOG(SENSOR_DEV, HIGH, "Open device 0x%x return %d", deviceId, result);

    return result;
} // CameraSensorDevice_Open


/* ---------------------------------------------------------------------------
*    FUNCTION        CameraSensorDevice_LoadSensorLibs
*    DESCRIPTION     Open camera sensor libraries
*    DEPENDENCIES    None
*    PARAMETERS      CameraSensorDevice Pointer
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
 * ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::LoadSensorLibs()
{
    CameraResult result = CAMERA_SUCCESS;
    sensor_open_lib_t device_info = {};
    device_info.boardType = CameraPlatformGetCameraHwBoardType();
    device_info.config = CameraPlatformGetInputDeviceConfig(m_deviceId, &device_info.cameraId);

    m_SensorData.ps_SensorLibData =
            (sensor_lib_t*) m_sensorLibInterface.sensor_open_lib(
                    &m_SensorDriver,
                    &device_info);

    if (NULL == m_SensorData.ps_SensorLibData)
    {
        // Failed to open the Sensor Data File,
        SENSOR_ERROR("Sensor Data Not Found.");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
*    FUNCTION        CameraSensorDevice_UnloadSensorLibs
*    DESCRIPTION     Closes any open camera sensor libraries
*    DEPENDENCIES    None
*    PARAMETERS      CameraSensorDevice Pointer
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
 * ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::UnloadSensorLibs()
{
    CameraResult result = CAMERA_SUCCESS;
    sensor_lib_t* pSensorLib = m_SensorData.ps_SensorLibData;
    if (pSensorLib)
    {
        pSensorLib->sensor_close_lib(pSensorLib);
    }

    m_SensorData.ps_SensorLibData = NULL;

    return result;
}

CameraResult CameraSensorDevice::Initialize()
{
    CameraResult result = CAMERA_SUCCESS;

    m_pfnClientCallback = NULL;
    m_pClientData = NULL;

    result = LoadSensorLibs();
    if (CAMERA_SUCCESS == result)
    {
        result = DriverInitialize();

        if (CAMERA_SUCCESS != result)
        {
            (void)UnloadSensorLibs();
        }
    }

    return result;
}

void CameraSensorDevice::Uninitialize()
{
    (void)DriverUninitialize();

    (void)UnloadSensorLibs();
}

CameraResult CameraSensorDevice::Control(
    uint32 uidControl,
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    // Sanity check input parameters
    if (((NULL == pIn) && nInLen) ||
        ((NULL == pOut) && nOutLen))
    {
        return CAMERA_EMEMPTR;
    }

    switch (uidControl)
    {
    case Camera_Sensor_AEEUID_CTL_STATE_POWER_ON:
        result = PowerOn();
        break;

    case Camera_Sensor_AEEUID_CTL_STATE_POWER_OFF:
        result = PowerOff();
        break;

    case Camera_Sensor_AEEUID_CTL_STATE_RESET:
        result = Reset();
        break;

    case Camera_Sensor_AEEUID_CTL_STATE_POWER_SUSPEND:
    {
        CameraPowerEventType* nEvent = NULL;
        if (pIn && nInLen == sizeof(int))
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Enter power suspend");
            nEvent = (CameraPowerEventType*)pIn;
            result = PowerSuspend(*nEvent);
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
        break;
    }

    case Camera_Sensor_AEEUID_CTL_STATE_POWER_RESUME:
    {
        CameraPowerEventType* nEvent = NULL;
        if (pIn && nInLen == sizeof(int))
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Enter power resume");
            nEvent = (CameraPowerEventType*)pIn;
            result = PowerResume(*nEvent);
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
        break;
    }

    case Camera_Sensor_AEEUID_CTL_STATE_RECOVERY:
    {
        if (pIn && nInLen == sizeof(uint32_t))
        {
            uint32_t* severity = (uint32_t*)pIn;
            AIS_LOG(SENSOR_DEV, HIGH, "Enter sensor recovery");
            result = m_SensorDriver.ErrorRecovery(*severity);
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
        break;
    }

    case Camera_Sensor_AEEUID_CTL_DETECT_DEVICE:
        result = Detect();
        break;

    case Camera_Sensor_AEEUID_CTL_DETECT_DEVICE_CHANNELS:
        result = CCameraSensorDriver_DetectChannels(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_STATE_FRAME_OUTPUT_START:
        result = CCameraSensorDriver_OutputStart(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_STATE_FRAME_OUTPUT_STOP:
        result = CCameraSensorDriver_OutputStop(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_INFO_CHANNELS:
        result = CCameraSensorDriver_Info_Channels(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_INFO_SUBCHANNELS:
        result = CCameraSensorDriver_Info_Subchannels(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_CSI_INFO_PARAMS:
        result = CCameraSensorDriver_Info_Csi_Params(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_INFO_CHROMATIX:
        result = CCameraSensorDriver_GetInfoChromatix(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_CONFIG_MODE:
        result = CCameraSensorDriver_ConfigMode(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_CONFIG_RESOLUTION:
        result = CCameraSensorDriver_ConfigResolution(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_QUERY_FIELD:
        result = CCameraSensorDriver_QueryField(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_CONFIG_SENSOR_PARAMS:
        result = CCameraSensorDriver_ConfigParam(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_GET_VENDOR_PARAM:
        result = CCameraSensorDriver_GetVendorParam(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_GET_SENSOR_PARAM:
        result = CCameraSensorDriver_GetSensorParam(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_PROCESS_FRAME_DATA:
        result = CCameraSensorDriver_ProcessFrameData(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    case Camera_Sensor_AEEUID_CTL_CONFIG_CCI_SYNC_PARAM:
        result = CCameraSensorDriver_SetCCISyncParam(pIn, nInLen, pOut, nOutLen, pnOutLenReq);
        break;

    default:
        return CAMERA_EUNSUPPORTED;
    }

    return result;
}

CameraResult CameraSensorDevice::Close()
{
    CameraResult result = CAMERA_SUCCESS;

    Uninitialize();

    delete this;

    return result;
}

CameraResult CameraSensorDevice::RegisterCallback(
    CameraDeviceCallbackType pfnCallback,
    void* pClientData)
{
    CameraResult result = CAMERA_SUCCESS;

    if (NULL == pfnCallback)
    {
        result = CAMERA_EMEMPTR;
    }
    else
    {
        m_pfnClientCallback = pfnCallback;
        m_pClientData = pClientData;
    }

    return result;
}

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Initialize
*    DESCRIPTION     Initializes camera sensor driver and hardware
*    DEPENDENCIES    None
*    PARAMETERS      CameraSensorDevice Pointer
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::DriverInitialize()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    result = m_SensorDriver.Initialize(this,
            m_SensorData.ps_SensorLibData);

    if (CAMERA_SUCCESS == result)
    {
        /* make sure sensor is truly OFF */
        result = m_SensorDriver.PowerOff();
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to power off sensor");
        }
        m_info.sensorState = POWER_OFF;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Initialize */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Uninitialize
*    DESCRIPTION     Uninitializes camera sensor driver and hardware
*    DEPENDENCIES    None
*    PARAMETERS      CameraSensorDevice Pointer
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::DriverUninitialize()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    /* Power off the sensor */
    (void)PowerOff();

    result = m_SensorDriver.Uninitialize();

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Uninitialize */

/* ===========================================================================
PUBLIC SENSOR CONTROL FUNCTION DEFINITIONS
=========================================================================== */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerOn
*    DESCRIPTION     Configure PMIC/GPIO to apply power to sensor
*                    Enable clock to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerOn()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    /* Power on the sensor */
    result = PowerOn_Internal();
    if (CAMERA_SUCCESS != result)
    {
        // PowerOnInternal Failed, Failed to detect the sensor successfully.
        // Return this Camera Sensor unsupported
        SENSOR_ERROR("PowerOnInternal Failed, result %d", result);
        return CAMERA_EUNSUPPORTED;
    }

    if (m_SensorDriver.InitializeRegisters())
    {
        SENSOR_ERROR("InitializeRegisters Failed");
        return CAMERA_EUNSUPPORTED;
    }

    m_info.sensorState = INITIALIZED;

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerOn */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerOff
*    DESCRIPTION     Disable clock to sensor
*                    Configure PMIC/GPIO to stop power to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerOff()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    /* Power off the sensor */
    result = PowerOff_Internal();

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerOff */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerSuspend
*    DESCRIPTION     Configure PMIC/GPIO to apply power to sensor
*                    Enable clock to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerSuspend(CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = m_SensorDriver.PowerSuspend(powerEventId);
    if (CAMERA_SUCCESS != result)
    {
        SENSOR_ERROR("PowerSuspend Failed");
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerSuspend */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerResume
*    DESCRIPTION     Configure PMIC/GPIO to apply power to sensor
*                    Enable clock to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerResume(CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = m_SensorDriver.PowerResume(powerEventId);
    if (CAMERA_SUCCESS != result)
    {
        SENSOR_ERROR("PowerResume Failed");
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerResume */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Detect
*    DESCRIPTION     Determine if the the sensor controlled
*                    by this sensor driver is attached.
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::Detect()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    if (POWER_OFF != m_info.sensorState)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Sensor not powered off");
        return CAMERA_EBADSTATE;
    }

    result = PowerOn_Internal();
    if (CAMERA_SUCCESS == result)
    {
        /* Device detecting */
        if (CAMERA_SUCCESS == m_SensorDriver.DetectDevice())
        {
            AIS_LOG(SENSOR_DEV, LOW, "Sensor Detection Success");
            result = CAMERA_SUCCESS;
        }
        else
        {
            SENSOR_ERROR("Sensor Detection Failed, Return Unsupported");
            result = PowerOff_Internal();
            if (result != CAMERA_SUCCESS)
            {
                SENSOR_ERROR("PowerOff_Internal failed");
            }
            result = CAMERA_EUNSUPPORTED;
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Detect */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Detect_Channels
*    DESCRIPTION     Determine if the sensor attached is the sensor controlled
*                    by this sensor driver.
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_DetectChannels (
        const void* pIn, int nInLen,
        void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    (void)pIn;
    (void)nInLen;
    (void)pnOutLenReq;

    SENSOR_FUNCTIONENTRY("");

    if (POWER_OFF == m_info.sensorState)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Sensor not powered on");
        return CAMERA_EBADSTATE;
    }

    /* Device detecting */
    if (CAMERA_SUCCESS == m_SensorDriver.DetectDeviceChannels())
    {
        AIS_LOG(SENSOR_DEV, LOW, "Sensor Channels Detection Success");
        // Copy connected sensors information to out variable
        if (pOut &&  (sizeof(m_SensorDriver.m_pSensorLib->src_id_enable_mask) == nOutLen))
        {
            memcpy(pOut, &m_SensorDriver.m_pSensorLib->src_id_enable_mask, nOutLen);
        }

        (void)InitializeData();

        result = CAMERA_SUCCESS;
    }
    else
    {
        SENSOR_ERROR("Sensor Detection Failed, Return Unsupported");
        result = CAMERA_EUNSUPPORTED;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Detect_Channels */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_OutputStart
*    DESCRIPTION     Enable Straming
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_OutputStart (
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");
    if (pIn && nInLen == sizeof (uint32))
    {
        uint32 srcIdMask = *(uint32*)pIn;

        if (m_info.sensorState == INITIALIZED)
        {
            result = m_SensorDriver.StartStream(srcIdMask);
        }
        else
        {
            SENSOR_ERROR("sensor not initialized");
            result = CAMERA_EALREADY;
        }
    }
    else
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Invalid parameters");
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_OutputStart */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_OutputStop
*    DESCRIPTION     Stop Steaming
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_OutputStop (
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    if (pIn && nInLen == sizeof (uint32))
    {
        uint32 srcIdMask = *(uint32*)pIn;

        if (m_info.sensorState == INITIALIZED)
        {
            (void)m_SensorDriver.StopStream(srcIdMask);

            result = CAMERA_SUCCESS;
        }
        else
        {
            SENSOR_ERROR("sensor not initialized");
            result = CAMERA_EALREADY;
        }
    }
    else
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Invalid parameters");
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_OutputStop */

/* ---------------------------------------------------------------------------
*    FUNCTION        Reset
*    DESCRIPTION     Configure sensor to POR defaults
*                    Reset driver
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::Reset()
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    /* Power off the sensor */
    result = PowerOff();
    if (CAMERA_SUCCESS != result) return result;

    /* Power on the sensor */
    result = PowerOn();
    if (CAMERA_SUCCESS != result) return result;

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* Reset */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Info_Channels
*    DESCRIPTION     Get Channels information
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_Info_Channels(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    (void)pIn;
    (void)nInLen;

    if (pnOutLenReq)
    {
        *pnOutLenReq = sizeof(Camera_Sensor_ChannelsInfoType);
    }

    if (pOut)
    {
        if (nOutLen == sizeof (Camera_Sensor_ChannelsInfoType))
        {
            /* Copy the data */
            Camera_Sensor_ChannelsInfoType* pChannelsInfo = (Camera_Sensor_ChannelsInfoType*)pOut;
            *pChannelsInfo = m_info.channelsInfo;
            result = CAMERA_SUCCESS;
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Info_Channels */


/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Info_Subhannels
*    DESCRIPTION     Get Subchannels information
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_Info_Subchannels(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    (void)pIn;
    (void)nInLen;

    if (pnOutLenReq)
    {
        *pnOutLenReq = sizeof(Camera_Sensor_SubchannelsInfoType);
    }

    if (pOut)
    {
        if (nOutLen == sizeof (Camera_Sensor_SubchannelsInfoType))
        {
            /* Copy the data */
            Camera_Sensor_SubchannelsInfoType* pSubchannelsInfo = (Camera_Sensor_SubchannelsInfoType*)pOut;
            *pSubchannelsInfo = m_info.subchannelsInfo;
            result = CAMERA_SUCCESS;
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Info_Subhannels */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_Info_Csi_Params
*    DESCRIPTION     Get csi parameter information
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_Info_Csi_Params (
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    (void)pIn;
    (void)nInLen;

    if (pnOutLenReq)
    {
        *pnOutLenReq = sizeof(Camera_Sensor_MipiCsiInfoType);
    }

    if (pOut)
    {
        if (nOutLen == sizeof (Camera_Sensor_MipiCsiInfoType))
        {
            /* Populate the data */
            for(int i = 0; i < CSIPHY_CORE_MAX; i++)
            {
                Camera_Sensor_MipiCsiInfoType* pMipiCsiInfo = (Camera_Sensor_MipiCsiInfoType*)pOut;
                pMipiCsiInfo->MipiCsiInfo[i].num_lanes = m_SensorDriver.m_pSensorLib->csi_params[i].lane_cnt;
                pMipiCsiInfo->MipiCsiInfo[i].lane_mask = m_SensorDriver.m_pSensorLib->csi_params[i].lane_mask;
                pMipiCsiInfo->MipiCsiInfo[i].settle_count = m_SensorDriver.m_pSensorLib->csi_params[i].settle_cnt;
                pMipiCsiInfo->MipiCsiInfo[i].mipi_rate = m_SensorDriver.m_pSensorLib->csi_params[i].mipi_rate;
                pMipiCsiInfo->MipiCsiInfo[i].is_csi_3phase = m_SensorDriver.m_pSensorLib->csi_params[i].is_csi_3phase;
                pMipiCsiInfo->MipiCsiInfo[i].vcx_mode = m_SensorDriver.m_pSensorLib->csi_params[i].vcx_mode;
                pMipiCsiInfo->sensor_num_frame_skip = m_SensorDriver.m_pSensorLib->sensor_num_frame_skip;
            }
            result = CAMERA_SUCCESS;
        }
        else
        {
            result = CAMERA_EBADPARM;
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_Info_Csi_Params */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_GetInfoChromatix
*    DESCRIPTION     Retrieve the chromatix information
*    DEPENDENCIES    None
*    PARAMETERS      CCameraSensorDriver pointer, output pointer, output length,
*                    required output length pointer
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_GetInfoChromatix(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_GetInfoChromatix */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_ConfigMode
*    DESCRIPTION     Configure resolution to Sensor
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_ConfigMode(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pIn && nInLen == sizeof(Camera_Sensor_ParamConfigType))
    {
        result = m_SensorDriver.SetSensorMode((Camera_Sensor_ParamConfigType *)pIn);
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_ConfigMode */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_ConfigResolution
*    DESCRIPTION     Configure resolution to Sensor
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_ConfigResolution(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pIn && nInLen == sizeof (Camera_Size))
    {
        /* Config Resolution */
        Camera_Size* pResConfig = (Camera_Size*)pIn;
        result = m_SensorDriver.ConfigResolution(pResConfig);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to set Resolution");
        }
        else
        {
            m_info.subchannelsInfo.subchannels[0].modes[0].res.width = pResConfig->width;
            m_info.subchannelsInfo.subchannels[0].modes[0].res.height = pResConfig->height;
        }
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_ConfigResolution */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_QueryField
*    DESCRIPTION     Get Field type from Sensor
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_QueryField(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pOut && nOutLen == sizeof (Camera_Sensor_FieldType))
    {
        Camera_Sensor_FieldType* pFieldType = (Camera_Sensor_FieldType*)pOut;
        result = m_SensorDriver.QueryField(pFieldType);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to get Field type");
        }
        else
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Get Field type: %d %llu", pFieldType->fieldType, pFieldType->timestamp);
        }
    }
    else
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Invalid parameters");
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_ConfigResolution */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_ConfigExposure
*    DESCRIPTION     Set the manuual exposure time and gain
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_ConfigParam(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;
    (void)pnOutLenReq;
    (void)pOut;
    (void)nOutLen;

    SENSOR_FUNCTIONENTRY("");

    /* Sanity check input parameters */
    if (NULL == pIn || sizeof(Camera_Sensor_ParamConfigType) != nInLen)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Invalid input");
        result = CAMERA_EBADPARM;
    }
    else if (POWER_OFF == m_info.sensorState)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Sensor not powered on");
        result = CAMERA_EBADSTATE;
    }
    else
    {
        result = m_SensorDriver.ConfigParam((Camera_Sensor_ParamConfigType *)pIn);
        if (result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to set Exposure Config result = %d",result);
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_ConfigParam */


/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_GetSensorParam
*    DESCRIPTION     Get the sensor param (
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_GetSensorParam(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pOut && nOutLen == sizeof (Camera_Sensor_ParamConfigType))
    {
        Camera_Sensor_ParamConfigType* pConfig = (Camera_Sensor_ParamConfigType*)pOut;
        result = m_SensorDriver.GetParam(pConfig);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to get sensor param %u for src %d,result=%d",
                pConfig->paramId, pConfig->srcId, result);
        }
        else
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Get Sensor Param %u for src %u", pConfig->paramId, pConfig->srcId);
        }
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);


    return result;
} /* CCameraSensorDriver_GetSensorParam */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_ProcessFrameData
*    DESCRIPTION     Allows sensor driver to parse or process the frame data as needed
*
*    DEPENDENCIES    None
*    PARAMETERS      pIn of type Camera_Sensor_ProcessFrameDataType
*    RETURN VALUE    CameraResult
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_ProcessFrameData(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pIn && nInLen == sizeof (CameraInputProcessFrameDataType))
    {
        CameraInputProcessFrameDataType* pParam = (CameraInputProcessFrameDataType*)pIn;
        result = m_SensorDriver.ProcessFrameData(pParam);
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);


    return result;
} /* CCameraSensorDriver_ProcessFrameData */


/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_GetVendorParam
*    DESCRIPTION     Get the vendor param
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_GetVendorParam(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pOut && nOutLen == sizeof (Camera_Sensor_ParamConfigType))
    {
        Camera_Sensor_ParamConfigType* pConfig = (Camera_Sensor_ParamConfigType*)pOut;
        result = m_SensorDriver.GetVendorParam(pConfig);
        if (CAMERA_SUCCESS != result)
        {
            AIS_LOG(SENSOR_DEV, ERROR, "Failed to get Vendor Param %u result=%d",
                pConfig->param.pVendorParam->data[0], result);
        }
        else
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Get Vendor Param %u", pConfig->param.pVendorParam->data[0]);
        }
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);


    return result;
} /* CCameraSensorDriver_GetVendorParam */

/* ---------------------------------------------------------------------------
*    FUNCTION        CCameraSensorDriver_SetCCISyncParam
*    DESCRIPTION     Set CCI Sync parameters
*
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::CCameraSensorDriver_SetCCISyncParam(
    const void* pIn, int nInLen,
    void* pOut, int nOutLen, int* pnOutLenReq)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (pIn && nInLen == sizeof (Camera_Sensor_ParamConfigType))
    {
        result = m_SensorDriver.ConfigCCISyncParam((Camera_Sensor_ParamConfigType *)pIn);
    }
    else
    {
        result = CAMERA_EBADPARM;
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* CCameraSensorDriver_SetCCISyncParam */

/* ===========================================================================
PRIVATE FUNCTION DEFINITIONS
=========================================================================== */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerOn_Internal
*    DESCRIPTION     Configure PMIC/GPIO to apply power to sensor
*                    Enable clock to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerOn_Internal()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");


    if (POWER_OFF != m_info.sensorState)
    {
        AIS_LOG(SENSOR_DEV, HIGH, "Sensor already powered on");
        return CAMERA_SUCCESS; //@todo: should we return CAMERA_EALREADY?
    }

    result = m_SensorDriver.PowerOn();
    if (CAMERA_SUCCESS == result)
    {
        m_info.sensorState = POWER_ON;
    }
    else
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Failed to power on sensor");
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerOn_Internal */

/* ---------------------------------------------------------------------------
*    FUNCTION        PowerOff_Internal
*    DESCRIPTION     Disable clock to sensor
*                    Configure PMIC/GPIO to stop power to sensor
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the condition of return
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::PowerOff_Internal()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (POWER_OFF == m_info.sensorState)
    {
        AIS_LOG(SENSOR_DEV, HIGH, "Sensor already powered off");
        return CAMERA_SUCCESS; //@todo: should we return CAMERA_EALREADY?
    }

    result = m_SensorDriver.PowerOff();
    if (CAMERA_SUCCESS != result)
    {
        AIS_LOG(SENSOR_DEV, ERROR, "Failed to power off sensor");
    }

    m_info.sensorState = POWER_OFF;

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* PowerOff_Internal */

void CameraSensorDevice::CameraSensorDriver_DumpChannelData()
{
    unsigned int i=0, j=0;
    AIS_LOG(SENSOR_DEV, HIGH, "Subchannels %d", m_info.subchannelsInfo.num_subchannels);
    for (i=0; i<m_info.subchannelsInfo.num_subchannels; i++)
    {
        AIS_LOG(SENSOR_DEV, HIGH, "  [%d]: src_id=%d, num_modes=%d",
                i,
                m_info.subchannelsInfo.subchannels[i].src_id,
                m_info.subchannelsInfo.subchannels[i].num_modes);
        for (j=0; j<m_info.subchannelsInfo.subchannels[i].num_modes; j++)
        {
            AIS_LOG(SENSOR_DEV, HIGH, "    (%d): vc/dt/cid=%d/0x%x/%d, fmt=0x%08x, res=%d/%d, fps=%.3f",
                            j,
                            m_info.subchannelsInfo.subchannels[i].modes[j].channel_info.vc,
                            m_info.subchannelsInfo.subchannels[i].modes[j].channel_info.dt,
                            m_info.subchannelsInfo.subchannels[i].modes[j].channel_info.cid,
                            m_info.subchannelsInfo.subchannels[i].modes[j].fmt,
                            m_info.subchannelsInfo.subchannels[i].modes[j].res.width,
                            m_info.subchannelsInfo.subchannels[i].modes[j].res.height,
                            m_info.subchannelsInfo.subchannels[i].modes[j].res.fps);
        }
    }

    AIS_LOG(SENSOR_DEV, HIGH, "Channels %d", m_info.channelsInfo.num_channels);
    for (i=0; i<m_info.channelsInfo.num_channels; i++)
    {
        AIS_LOG(SENSOR_DEV, HIGH, "  [%d]: sub=%d vc/dt/cid=%d/0x%x/%d, fmt=0x%08x, res=%d/%d, fps=%.3f",
                i,
                m_info.channelsInfo.channels[i].num_subchannels,
                m_info.channelsInfo.channels[i].output_mode.channel_info.vc,
                m_info.channelsInfo.channels[i].output_mode.channel_info.dt,
                m_info.channelsInfo.channels[i].output_mode.channel_info.cid,
                m_info.channelsInfo.channels[i].output_mode.fmt,
                m_info.channelsInfo.channels[i].output_mode.res.width,
                m_info.channelsInfo.channels[i].output_mode.res.height,
                m_info.channelsInfo.channels[i].output_mode.res.fps);
        for (j=0; j<m_info.channelsInfo.channels[i].num_subchannels; j++)
        {
            AIS_LOG(SENSOR_DEV, HIGH, "    (%d): src_id=%d, offset=%d x %d",
                            j,
                            m_info.channelsInfo.channels[i].subchan_layout[j].src_id,
                            m_info.channelsInfo.channels[i].subchan_layout[j].x_offset,
                            m_info.channelsInfo.channels[i].subchan_layout[j].y_offset);
        }
    }
}

/* ---------------------------------------------------------------------------
*    FUNCTION        InitializeData
*    DESCRIPTION     Initialize sensor driver's internal variables.
*    DEPENDENCIES    None
*    PARAMETERS      None
*    RETURN VALUE    CameraResult type based on the conditon of return
*    SIDE EFFECTS    None
*    Notes           This function calls the sensor specified InitilaizeData
*                    after the completion of this function if registered,
*                    This provides option for customization for sensor driver
* ------------------------------------------------------------------------ */
CameraResult CameraSensorDevice::InitializeData()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    m_info.channelsInfo.num_channels = m_SensorDriver.m_pSensorLib->num_channels;
    memcpy(m_info.channelsInfo.channels,
            m_SensorDriver.m_pSensorLib->channels,
            m_info.channelsInfo.num_channels*sizeof(m_info.channelsInfo.channels[0]));

    m_info.subchannelsInfo.num_subchannels = m_SensorDriver.m_pSensorLib->num_subchannels;
    memcpy(m_info.subchannelsInfo.subchannels,
            m_SensorDriver.m_pSensorLib->subchannels,
            m_info.subchannelsInfo.num_subchannels*sizeof(m_info.subchannelsInfo.subchannels[0]));

    CameraSensorDriver_DumpChannelData();

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* InitializeData */

/* ---------------------------------------------------------------------------
*    FUNCTION        SendCallback
*    DESCRIPTION     Call the input configure callback.
*    DEPENDENCIES    None
*    PARAMETERS      event type and payload pointer
*    RETURN VALUE    None
*    SIDE EFFECTS    None
* ------------------------------------------------------------------------ */
void CameraSensorDevice::SendCallback(CameraInputEventType eType,
        CameraInputEventPayloadType *pPayload)
{
    if (m_pfnClientCallback)
    {
        Camera_Sensor_EventType event = {};
        event.device_id = m_deviceId;
        event.pPayload = pPayload;

        m_pfnClientCallback(m_pClientData,
                eType,
                sizeof(Camera_Sensor_EventType),
                &event);
    }
}

/**
 * @file SensorDriver.c
 *
 * @brief SensorDriver Implementation
 *
 * Copyright (c) 2014-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
           INCLUDE FILES FOR MODULE
=========================================================================== */
#include <string.h>

#include "SensorPlatform.h"
#include "SensorDriver.h"
#include "CameraOSServices.h"
#include "SensorDebug.h"

/*===========================================================================
 Macro Definitions
===========================================================================*/

/* ===========================================================================
**                          Internal Helper Functions
** =========================================================================*/

static int SensorDriver_SlaveReadRegister(void* ctrl, unsigned short slave_addr,
        struct camera_i2c_reg_setting *reg_setting)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    struct camera_reg_settings_t read_reg = {};
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    read_reg.i2c_operation = CAMERA_I2C_OP_READ;
    read_reg.addr_type = reg_setting->addr_type;
    read_reg.data_type = reg_setting->data_type;
    read_reg.reg_addr = reg_setting->reg_array[0].reg_addr;
    read_reg.delay = reg_setting->reg_array[0].delay;

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveI2cRead(slave_addr, &read_reg);
    if (CAMERA_SUCCESS == result)
    {
        reg_setting->reg_array[0].reg_data = read_reg.reg_data;
    }

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SlaveBulkRead(void* ctrl, unsigned short slave_addr,
        struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;
    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveI2cBulkRead(slave_addr, reg_setting, exec_pending_i2ccmds);

    SENSOR_FUNCTIONEXIT("");
    return result;
}

static int SensorDriver_SlaveWriteRegisters(void* ctrl, unsigned short slave_addr,
        struct camera_i2c_reg_setting *reg_setting)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveWriteI2cSetting(slave_addr, reg_setting);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SlaveBulkWrite(void* ctrl, unsigned short slave_addr,
        struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;
    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveI2cBulkWrite(slave_addr, reg_setting, exec_pending_i2ccmds);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SlaveWriteRegisters_sync(void* ctrl, struct camera_i2c_sync_cfg sync_cfg,
            struct camera_i2c_reg_setting_array_sync* reg_setting_array_sync,int size)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveWriteI2cSetting_sync(sync_cfg, reg_setting_array_sync, size);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SlaveBulkWriteRead(void* ctrl, unsigned short slave_addr,
        struct camera_i2c_bulk_reg_setting *write_reg_setting,
        struct camera_i2c_bulk_reg_setting *read_reg_setting, boolean exec_pending_i2ccmds)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;
    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSlaveI2cBulkWriteRead(slave_addr, write_reg_setting,
                                                read_reg_setting, exec_pending_i2ccmds);
    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_ExecutePowerSetting(void* ctrl,
        struct camera_power_setting *power_settings, unsigned short nSize, CameraPowerEventType mode)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorExecutePowerSetting(power_settings, nSize, mode);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SetupGpioInterrupt(void* ctrl,
        enum camera_gpio_type gpio_id, sensor_intr_callback_t cb, void *data)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSetupGpioInterrupt(gpio_id, cb, data);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_SetupCciFrameSync(void* ctrl, unsigned int fsync_freq)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorSetupCciFrameSync(fsync_freq);

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_TriggerCciFrameSync(void* ctrl)
{
    SensorDriver* pSensorDriver = (SensorDriver*)ctrl;
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    result = pSensorDriver->m_pSensorPlatform->SensorTriggerCciFrameSync();

    SENSOR_FUNCTIONEXIT("");

    return result;
}

static int SensorDriver_InputCallback(void *pCtrl,
    CameraInputEventType eType,
    CameraInputEventPayloadType *pPayload)
{
    CameraResult result = CAMERA_SUCCESS;

    if(pCtrl == NULL || pPayload == NULL)
    {
        SENSOR_ERROR("Invalid context or payload");
        return CAMERA_EMEMPTR;
    }

    SensorDriver* pSensorDriver = (SensorDriver*)pCtrl;
    CameraSensorDevice * pSensorDeviceContext = pSensorDriver->m_pDeviceContext;

    SENSOR_FUNCTIONENTRY("");

    if(pSensorDeviceContext == NULL)
    {
        SENSOR_ERROR("Invalid callback context");
        result = CAMERA_EFAILED;
    }
    else
    {
        pSensorDeviceContext->SendCallback(eType, pPayload);
    }

    SENSOR_FUNCTIONEXIT("");

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::PowerOn
* DESCRIPTION Power On
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::PowerOn()
{
    CameraResult result = CAMERA_SUCCESS;
    SENSOR_FUNCTIONENTRY("");

    result = m_pSensorPlatform->SensorPowerUp();

    if (result == CAMERA_SUCCESS)
    {
        if (m_pSensorLib->use_sensor_custom_func &&
                m_pSensorLib->sensor_custom_func.sensor_power_on)
        {
            result = m_pSensorLib->sensor_custom_func.sensor_power_on((void*)m_pSensorLib);
        }
        else
        {
            result = m_pSensorPlatform->SensorExecutePowerSetting(
                m_pSensorLib->sensor_slave_info.power_setting_array.power_up_setting_a,
                m_pSensorLib->sensor_slave_info.power_setting_array.size_up,
                CAMERA_POWER_UP);
        }
    }

    SENSOR_FUNCTIONEXIT("");
    return result;

} /* SensorDriver::PowerOn */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::PowerOff
* DESCRIPTION Power Off
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::PowerOff()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_power_off)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_power_off((void*)m_pSensorLib);
    }
    else
    {
        result = m_pSensorPlatform->SensorExecutePowerSetting(
            m_pSensorLib->sensor_slave_info.power_setting_array.power_down_setting_a,
            m_pSensorLib->sensor_slave_info.power_setting_array.size_down,
            CAMERA_POWER_DOWN);
    }

    if (result == CAMERA_SUCCESS)
    {
        result = m_pSensorPlatform->SensorPowerDown();
    }

    SENSOR_FUNCTIONEXIT("");
    return result;
} /* SensorDriver::PowerOff */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::PowerSuspend
* DESCRIPTION Power Suspend
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::PowerSuspend(CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_power_suspend)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_power_suspend((void*)m_pSensorLib, powerEventId);
    }
    else
    {
        result = m_pSensorPlatform->SensorExecutePowerSetting(
            m_pSensorLib->sensor_slave_info.power_setting_array.power_down_setting_a,
            m_pSensorLib->sensor_slave_info.power_setting_array.size_down,
            powerEventId);
    }

    /* cam I/O release can be done before/after calling sensor_power_suspend, since
       PowerSuspend operation in sensor libs is not dependant on I/O release */
    if (CAMERA_SUCCESS == result)
    {
        if (powerEventId == CAMERA_POWER_SUSPEND)
        {
            result = m_pSensorPlatform->SensorPowerSuspend();
        }
        else
        {
            result = m_pSensorPlatform->SensorPowerDown();
        }
    }
    else
    {
        SENSOR_ERROR("Failed to apply power suspend sequence on the resources used");
    }

    SENSOR_FUNCTIONEXIT("");
    return result;
}/* SensorDriver::PowerSuspend */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::PowerResume
* DESCRIPTION Power Off
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::PowerResume(CameraPowerEventType powerEventId)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    /* cam I/O init should be done before calling sensor_power_resume, else detection
       fails during PowerResume operation in respective sensor libs*/
    if ((powerEventId == CAMERA_POWER_RESUME) || (powerEventId == CAMERA_POWER_POST_HIBERNATION))
    {
        result = m_pSensorPlatform->SensorPowerResume();
    }
    else
    {
        result = m_pSensorPlatform->SensorPowerUp();
    }

    if (result == CAMERA_SUCCESS)
    {
        if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_power_resume)
        {
            result = m_pSensorLib->sensor_custom_func.sensor_power_resume((void*)m_pSensorLib, powerEventId);
        }
        else
        {
            result = m_pSensorPlatform->SensorExecutePowerSetting(
                m_pSensorLib->sensor_slave_info.power_setting_array.power_up_setting_a,
                m_pSensorLib->sensor_slave_info.power_setting_array.size_up,
                powerEventId);
        }
    }
    else
    {
        SENSOR_ERROR("Failed to power on the resouces used by sensor");
    }

    SENSOR_FUNCTIONEXIT("");
    return result;
} /* SensorDriver::PowerResume */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ErrorRecovery
* DESCRIPTION Error recovery sequence
* DEPENDENCIES None
* PARAMETERS Error severity
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ErrorRecovery(uint32_t severity)
{
    CameraResult result = CAMERA_SUCCESS;
    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
        m_pSensorLib->sensor_custom_func.sensor_error_recovery)
    {
        int rc = m_pSensorLib->sensor_custom_func.sensor_error_recovery((void*)m_pSensorLib, severity);
        if (0 == rc)
        {
            result = CAMERA_SUCCESS;
        }
        else
        {
            SENSOR_ERROR("Sensor lib recovery failed %d", rc);
            result = rc;
        }
    }

    SENSOR_FUNCTIONEXIT("");
    return result;

} /* SensorDriver::ErrorRecovery */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::DetectDevice
* DESCRIPTION Detect Device
* is not required.
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::DetectDevice()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func && m_pSensorLib->sensor_custom_func.sensor_detect_device)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_detect_device((void*)m_pSensorLib);
    }
    else
    {
        struct camera_reg_settings_t read_reg = {};
        byte SlaveAddr = m_pSensorLib->sensor_slave_info.slave_addr;
        uint16 RegAddr = m_pSensorLib->sensor_slave_info.sensor_id_info.sensor_id_reg_addr;
        uint32 ExpectedModelID = m_pSensorLib->sensor_slave_info.sensor_id_info.sensor_id;

        read_reg.i2c_operation = CAMERA_I2C_OP_READ;
        read_reg.addr_type = m_pSensorLib->sensor_slave_info.addr_type;
        read_reg.data_type = m_pSensorLib->sensor_slave_info.data_type;
        read_reg.reg_addr = RegAddr;
        read_reg.delay = 0;

        AIS_LOG(SENSOR_DEV, LOW, "SlaveAddr = 0x%x RegAddr = 0x%x  ModelId = 0x%x", SlaveAddr, RegAddr, ExpectedModelID);

        m_pSensorPlatform->SensorSlaveI2cRead(SlaveAddr, &read_reg);

        SENSOR_ERROR("ReadBack 0x%x, Expected 0x%x", read_reg.reg_data, ExpectedModelID);

        if (read_reg.reg_data == ExpectedModelID)
        {
            result = CAMERA_SUCCESS; // Sensor Detected Successfully
            SENSOR_LOG("ModelID = 0x%x Detected", read_reg.reg_data);
        }
        else
        {
            SENSOR_ERROR("Failed to Detect Sensor with Reference ID 0x%x, read back 0x%x", ExpectedModelID, read_reg.reg_data);
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;

} /* SensorDriver::DetectDevice */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::DetectDeviceChannels
* DESCRIPTION Detect Device Channels
* is not required.
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::DetectDeviceChannels()
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&m_pSensorLib->sensor_custom_func.sensor_detect_device_channels)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_detect_device_channels((void*)m_pSensorLib);
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::InitializeRegisters
* DESCRIPTION Registers initialization
* is not required.
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::InitializeRegisters()
{
    CameraResult result = CAMERA_SUCCESS;
    byte SlaveAddr;
    uint32 iInitSettingCount = 0;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_init_setting)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_init_setting((void*) m_pSensorLib);
    }
    else
    {
        SlaveAddr = (byte)m_pSensorLib->sensor_slave_info.slave_addr;
        result = CAMERA_SUCCESS;
        for (iInitSettingCount = 0; iInitSettingCount < m_pSensorLib->init_settings_array.size; iInitSettingCount++)
        {
            AIS_LOG(SENSOR_DEV, LOW, "Array[%d] ", iInitSettingCount);

            if (0 != m_pSensorPlatform->SensorSlaveWriteI2cSetting(SlaveAddr, &m_pSensorLib->init_settings_array.reg_settings[iInitSettingCount]))
            {
                result = CAMERA_EFAILED;
            }
        }
    }

    SENSOR_FUNCTIONEXIT("");

    return result;

} /* SensorDriver::InitializeRegisters */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::SetSensorMode
* DESCRIPTION Set sensor mode
* DEPENDENCIES None
* PARAMETERS this, mode to set
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::SetSensorMode(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    uint32 mode = pParamConfig->param.uVal;
    AIS_LOG(SENSOR_DEV, LOW, "mode = %d", mode);

    if (mode >= MAX_RESOLUTION_MODES)
    {
        SERR("invalid mode %d", mode);
        result = CAMERA_EBADPARM;
    }
    else
    {
        if (m_pSensorLib->use_sensor_custom_func &&
                m_pSensorLib->sensor_custom_func.sensor_set_channel_mode)
        {
            /*set mode*/
            result = m_pSensorLib->sensor_custom_func.sensor_set_channel_mode(
                    (void*) m_pSensorLib, pParamConfig->srcId, mode);
        }
        else
        {

            if (0 == m_pSensorPlatform->SensorSlaveWriteI2cSetting(m_pSensorLib->sensor_slave_info.slave_addr,
                    &m_pSensorLib->res_settings_array.reg_settings[mode]))
            {
                AIS_LOG(SENSOR_DEV, LOW, "Mode%d done", mode);
            }
            else
            {
                SENSOR_ERROR("Mode%d failure", mode);
                result = CAMERA_EFAILED;
            }
        }
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* SensorDriver::SetSensorMode */


/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::StartStream
* DESCRIPTION Start streaming.
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::StartStream(uint32 srcIdMask)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_start_stream)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_start_stream(
                (void*)m_pSensorLib, srcIdMask);
    }
    else
    {
        if (0 == m_pSensorPlatform->SensorSlaveWriteI2cSetting(
                        m_pSensorLib->sensor_slave_info.slave_addr,
                        &m_pSensorLib->start_settings))
        {
            result = CAMERA_SUCCESS;
        }
    }

    SENSOR_FUNCTIONEXIT("");

    return result;

} /* SensorDriver::StartStream */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::StopStream
* DESCRIPTION Stop stream.
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::StopStream(uint32 srcIdMask)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_stop_stream)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_stop_stream(
                (void*)m_pSensorLib, srcIdMask);
    }
    else
    {
        if (0 == m_pSensorPlatform->SensorWriteI2cSetting(&m_pSensorLib->stop_settings))
        {
            result = CAMERA_SUCCESS;
        }
    }

    SENSOR_FUNCTIONEXIT("");

    return result;
} /* SensorDriver::StopStream */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigResolution
* DESCRIPTION Config Sensor Resolution returned by BA
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigResolution(Camera_Size* pResConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");
    Camera_Size *resConfig = (Camera_Size*)pResConfig;

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_config_resolution)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_config_resolution(
                (void*)m_pSensorLib, resConfig->width,resConfig->height);
    }

    SENSOR_FUNCTIONEXIT("");

    return result;
} /* SensorDriver::ConfigResolution */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::QueryField
* DESCRIPTION Get Field type returned by low level driver
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::QueryField(Camera_Sensor_FieldType* pFieldType)
{
    CameraResult result = CAMERA_EFAILED;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_query_field)
    {
        boolean isEvenField;
        result = m_pSensorLib->sensor_custom_func.sensor_query_field(
                (void*)m_pSensorLib, &isEvenField, &pFieldType->timestamp);

        if (!result)
        {
            pFieldType->fieldType = (isEvenField) ? QCARCAM_FIELD_EVEN : QCARCAM_FIELD_ODD;

            result = CAMERA_SUCCESS;
        }
    }

    SENSOR_FUNCTIONEXIT("");

    return result;
} /* SensorDriver::ConfigResolution */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigExposure
* DESCRIPTION calculate exposures and updates exposures through settings
* DEPENDENCIES None
* PARAMETERS Register Gain and Exposure  time, and settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigExposure(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;
    sensor_exposure_info_t exposure_info = {};

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    qcarcam_exposure_config_t* pExposureConfig =
        pParamConfig->param.pExposureConfig;

    exposure_info.exposure_mode_type = pExposureConfig->exposure_mode_type;

    if (!(m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG)))
    {
        SENSOR_ERROR("Exposure control not supported");
        return CAMERA_EUNSUPPORTED;
    }

    if ((exposure_info.exposure_mode_type == QCARCAM_EXPOSURE_MANUAL) ||
        (exposure_info.exposure_mode_type == QCARCAM_EXPOSURE_AUTO))
    {
        exposure_info.real_gain = pExposureConfig->gain;
        exposure_info.exposure_time = pExposureConfig->exposure_time;
        memcpy(exposure_info.reserved, pExposureConfig->reserved, sizeof(exposure_info.reserved));

        SENSOR_HIGH("Manual exposure gain=%f, time=%f", exposure_info.real_gain, exposure_info.exposure_time);
        /** Calculate exposure params and set it.
         */
        if (m_pSensorLib->exposure_func_table.sensor_calculate_exposure)
        {
            m_pSensorLib->exposure_func_table.sensor_calculate_exposure(
                    (void*) m_pSensorLib, pParamConfig->srcId, &exposure_info);
        }
        else
        {
            SENSOR_ERROR("Manual exposure calculation not supported");
            return CAMERA_EUNSUPPORTED;
        }
    }

    if (m_pSensorLib->exposure_func_table.sensor_exposure_config)
    {
        if (m_pSensorLib->exposure_func_table.sensor_exposure_config(
                (void*) m_pSensorLib, pParamConfig->srcId, &exposure_info))
        {
            SENSOR_ERROR("Failed to set exposure configuration");
            result = CAMERA_EFAILED;
        }
    }
    else
    {
        SENSOR_ERROR("Exposure control not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigHdrExposure
* DESCRIPTION calculate exposures and updates exposures through settings
* DEPENDENCIES None
* PARAMETERS Register Gain and Exposure  time, and settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigHdrExposure(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->exposure_func_table.sensor_hdr_exposure_config)
    {
        if (m_pSensorLib->exposure_func_table.sensor_hdr_exposure_config(
                (void*)m_pSensorLib, pParamConfig->srcId, pParamConfig->param.pHdrExposureConfig))
        {
            SENSOR_ERROR("Failed to set hdr exposure configuration");
            result = CAMERA_EFAILED;
        }
    }
    else
    {
        SENSOR_ERROR("HDR Exposure control not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigGAMMA
* DESCRIPTION Update gamma correction parameters
* DEPENDENCIES None
* PARAMETERS gamma table to be set
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigGamma(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (!(m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_GAMMA_CONFIG)))
    {
        SENSOR_ERROR("Gamma control not supported");
        return CAMERA_EUNSUPPORTED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param)
    {
        if(m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                pParamConfig->param.pGammaConfig))
        {
            SENSOR_ERROR("Failed to set gamma param");
            result = CAMERA_EFAILED;
        }
    }
    else
    {
        SENSOR_ERROR("Gamma configure not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigVendor
* DESCRIPTION Update Vendor correction parameters
* DEPENDENCIES None
* PARAMETERS Vendor config to be set
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigVendor(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (!(m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_VENDOR_PARAM)))
    {
        SENSOR_ERROR("Vendor config not supported");
        return CAMERA_EUNSUPPORTED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                pParamConfig->param.pVendorParam);

        if(result)
        {
            SENSOR_ERROR("Failed to set vendor param with error %d", result);
        }
    }
    else
    {
        SENSOR_ERROR("Vendor configure not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;

}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigColorParam
* DESCRIPTION Update color settings
* DEPENDENCIES None
* PARAMETERS Saturation value to be set, settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigColorParam(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param
        && (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_COLOR_CONFIG)))
    {
           if(m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                &pParamConfig->param.fVal))
           {
               SENSOR_ERROR("Failed to set color param");
               result = CAMERA_EFAILED;
           }
    }
    else
    {
        SENSOR_ERROR("Color param not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigBrightness
* DESCRIPTION Update brightness settings
* DEPENDENCIES None
* PARAMETERS Brightness/Contrast value to be set, settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigBrightness(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param
        && (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_BRIGHTNESS)))
    {
           if(m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                &pParamConfig->param.fVal))
           {
               SENSOR_ERROR("Failed to set brightness.");
               result = CAMERA_EFAILED;
           }
    }
    else
    {
        SENSOR_ERROR("Brightness param not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigContrast
* DESCRIPTION Update contrast settings
* DEPENDENCIES None
* PARAMETERS Brightness/Contrast value to be set, settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigContrast(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param
        && (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_CONTRAST)))
    {
           if(m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                &pParamConfig->param.fVal))
           {
               SENSOR_ERROR("Failed to set contrast.");
               result = CAMERA_EFAILED;
           }
    }
    else
    {
        SENSOR_ERROR("Contrast param not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigMirror
* DESCRIPTION Update mirroring settings
* DEPENDENCIES None
* PARAMETERS Mirror value to be set, settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigMirror(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;

    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->use_sensor_custom_func
        && m_pSensorLib->sensor_custom_func.sensor_s_param
        && (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_MIRROR)))
    {
           if(m_pSensorLib->sensor_custom_func.sensor_s_param(
                (void*)m_pSensorLib,
                pParamConfig->paramId,
                pParamConfig->srcId,
                &pParamConfig->param.uVal))
           {
               SENSOR_ERROR("Failed to set mirror param");
               result = CAMERA_EFAILED;
           }
    }
    else
    {
        SENSOR_ERROR("Mirror param not supported");
        result = CAMERA_EUNSUPPORTED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigCciSyncParam
* DESCRIPTION Update cci sync settings
* DEPENDENCIES None
* PARAMETERS cci sync parameters to be set
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */

CameraResult SensorDriver::ConfigCCISyncParam(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_SUCCESS;


    if(NULL == pParamConfig)
    {
        return CAMERA_EFAILED;
    }

    if (m_pSensorLib->use_sensor_custom_func &&
            m_pSensorLib->sensor_custom_func.sensor_set_cci_sync_param)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_set_cci_sync_param(
                (void*)m_pSensorLib, &pParamConfig->param.CCISyncParam);
    }

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ConfigParam
* DESCRIPTION Config parameter
* DEPENDENCIES None
* PARAMETERS Saturation value to be set, settings
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ConfigParam(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    if(NULL == pParamConfig)
    {
        return CAMERA_EBADPARM;
    }

    switch(pParamConfig->paramId)
    {
    case QCARCAM_PARAM_EXPOSURE:
        result = ConfigExposure(pParamConfig);
        break;
    case QCARCAM_PARAM_HDR_EXPOSURE:
        result = ConfigHdrExposure(pParamConfig);
        break;
    case QCARCAM_PARAM_SATURATION:
    case QCARCAM_PARAM_HUE:
        result = ConfigColorParam(pParamConfig);
        break;
    case QCARCAM_PARAM_GAMMA:
        result = ConfigGamma(pParamConfig);
        break;
    case QCARCAM_PARAM_VENDOR:
        result = ConfigVendor(pParamConfig);
        break;
    case QCARCAM_PARAM_BRIGHTNESS:
        result = ConfigBrightness(pParamConfig);
        break;
    case QCARCAM_PARAM_CONTRAST:
        result = ConfigContrast(pParamConfig);
        break;
    case QCARCAM_PARAM_MIRROR_H:
    case QCARCAM_PARAM_MIRROR_V:
        result = ConfigMirror(pParamConfig);
        break;
    default:
        AIS_LOG(SENSOR_DEV, ERROR, "Unsupported sensor param %d", pParamConfig->paramId);
        break;
    }

    SENSOR_FUNCTIONEXIT("%d", result);

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::GetParam
* DESCRIPTION Config parameter
* DEPENDENCIES None
* PARAMETERS pParamConfig value that includes subcommand and data to be get
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::GetParam(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult  result = CAMERA_SUCCESS;
    void         *p_param = NULL;
    sensor_exposure_info_t exposure_info = {};

    SENSOR_FUNCTIONENTRY("");

    if(NULL == pParamConfig)
    {
        result = CAMERA_EBADPARM;
    }

    if (CAMERA_SUCCESS == result)
    {
        if (m_pSensorLib->use_sensor_custom_func &&
                m_pSensorLib->sensor_custom_func.sensor_g_param)
        {
            switch(pParamConfig->paramId)
            {
                case QCARCAM_PARAM_EXPOSURE:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG))
                    {
                        p_param = &exposure_info;
                    }
                    else
                    {
                        SENSOR_ERROR("Exposure param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_HDR_EXPOSURE:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_EXPOSURE_CONFIG))
                    {
                        p_param = pParamConfig->param.pHdrExposureConfig;
                    }
                    else
                    {
                        SENSOR_ERROR("HDR Exposure param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_INPUT_MODE:
                    p_param = &pParamConfig->param.uVal;
                    break;
                case QCARCAM_PARAM_INPUT_COLOR_SPACE:
                    p_param = &pParamConfig->param.uVal;
                    break;
                case QCARCAM_PARAM_SATURATION:
                case QCARCAM_PARAM_HUE:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_COLOR_CONFIG))
                    {
                        p_param = &pParamConfig->param.fVal;
                    }
                    else
                    {
                        SENSOR_ERROR("Color param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_GAMMA:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_GAMMA_CONFIG))
                    {
                        p_param = pParamConfig->param.pGammaConfig;
                    }
                    else
                    {
                        SENSOR_ERROR("Gamma param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_BRIGHTNESS:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_BRIGHTNESS))
                    {
                        p_param = &pParamConfig->param.fVal;
                    }
                    else
                    {
                        SENSOR_ERROR("Brightness param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_CONTRAST:
                    if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_CONTRAST))
                    {
                        p_param = &pParamConfig->param.fVal;
                    }
                    else
                    {
                        SENSOR_ERROR("Contrast param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                case QCARCAM_PARAM_MIRROR_H:
                case QCARCAM_PARAM_MIRROR_V:
                   if (m_pSensorLib->sensor_capability & (1 << SENSOR_CAPABILITY_MIRROR))
                    {
                        p_param = &pParamConfig->param.uVal;
                    }
                    else
                    {
                        SENSOR_ERROR("Mirror param not supported");
                        result = CAMERA_EUNSUPPORTED;
                    }
                    break;
                default:
                    AIS_LOG(SENSOR_DEV, ERROR, "Unsupported sensor param %d", pParamConfig->paramId);
                    result = CAMERA_EUNSUPPORTED;
                    break;
            }

            if ((NULL != p_param) && (CAMERA_SUCCESS == result))
            {
                result = m_pSensorLib->sensor_custom_func.sensor_g_param(
                       (void*)m_pSensorLib,
                       pParamConfig->paramId,
                       pParamConfig->srcId,
                       p_param);

                if (0 == result)
                {
                    // successfully get sensor param
                    AIS_LOG(SENSOR_DEV, HIGH, "GetParam succeeded src: %u paramId: %u", pParamConfig->srcId, pParamConfig->paramId);
                    result = CAMERA_SUCCESS;

                    /* The sensor library uses 'sensor_exposure_info_t' for setting the exposure settings,
                     * whereas the user expects the 'qcarcam_exposure_config_t' structure. We need to translate
                     * from the first struct to the other before returning. */
                    if (QCARCAM_PARAM_EXPOSURE == pParamConfig->paramId)
                    {
                        qcarcam_exposure_config_t* pExposureConfig =
                                pParamConfig->param.pExposureConfig;

                        pExposureConfig->exposure_mode_type = exposure_info.exposure_mode_type;

                        if (QCARCAM_EXPOSURE_MANUAL == exposure_info.exposure_mode_type)
                        {
                            pExposureConfig->gain = exposure_info.real_gain;
                            pExposureConfig->exposure_time = exposure_info.exposure_time;
                            memcpy(pExposureConfig->reserved,exposure_info.reserved, sizeof(exposure_info.reserved));
                        }
                    }
                }
                else
                {
                    // for some params, not support get from sensor, need engine return the reserved value to client
                    if (CAMERA_EUNSUPPORTED != result)
                    {
                        AIS_LOG(SENSOR_DEV, ERROR, "GetParam failed for src %u", pParamConfig->srcId);
                        result = CAMERA_EFAILED;
                    }
                }
            }
        }
        else
        {
            AIS_LOG(SENSOR_DEV, HIGH, "Sensor doesn't support get_param functionality - src: %u ", pParamConfig->srcId);
            result = CAMERA_EUNSUPPORTED;
        }
    }

    SENSOR_FUNCTIONEXIT("%d", result);
    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::GetVendorParam
* DESCRIPTION Get vendor param
* DEPENDENCIES None
* PARAMETERS pParamConfig value that includes subcommand and data to be get
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::GetVendorParam(Camera_Sensor_ParamConfigType* pParamConfig)
{
    CameraResult result = CAMERA_EFAILED;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
           m_pSensorLib->sensor_custom_func.sensor_g_param)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_g_param(
               (void*)m_pSensorLib,
               pParamConfig->paramId,
               pParamConfig->srcId,
               pParamConfig->param.pVendorParam);
        if (!result)
        {
            //successfully get vendor param
            AIS_LOG(SENSOR_DEV, HIGH, "GetVendorParam succeed src: %u paramId: %u", pParamConfig->srcId, pParamConfig->paramId);
            result = CAMERA_SUCCESS;
        }
    }

    SENSOR_FUNCTIONEXIT("%d", result);

    return result;

}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::ProcessFrameData
* DESCRIPTION Allows sensor driver to parse or process the frame data as needed
* DEPENDENCIES None
* PARAMETERS pParam  pointer to buffer and frame info
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::ProcessFrameData(CameraInputProcessFrameDataType* pParam)
{
    CameraResult result = CAMERA_EUNSUPPORTED;

    SENSOR_FUNCTIONENTRY("");

    if (m_pSensorLib->use_sensor_custom_func &&
           m_pSensorLib->sensor_custom_func.sensor_process_frame_data)
    {
        result = m_pSensorLib->sensor_custom_func.sensor_process_frame_data(
               (void*)m_pSensorLib,
               pParam);
    }

    SENSOR_FUNCTIONEXIT("%d", result);

    return result;
}

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::Initialize
* DESCRIPTION Initialize the driver interface
* DEPENDENCIES None
* PARAMETERS this
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */
CameraResult SensorDriver::Initialize(CameraSensorDevice* pCameraSensorDevice,
        sensor_lib_t *pSensorData)
{
    CameraResult result = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    m_pDeviceContext = pCameraSensorDevice;
    m_pSensorLib = pSensorData;

    // Initialize sensor parameters using sensor data
    std_strlcpy(m_szSensorName, pSensorData->sensor_slave_info.sensor_name, STD_ARRAY_SIZE(m_szSensorName));

    AIS_LOG(SENSOR_DEV, LOW, "m_szModelNameAndNumber = %s", m_szSensorName);

    m_pSensorPlatform = SensorPlatform::SensorPlatformInit(m_pSensorLib);
    if (!m_pSensorPlatform)
    {
        SENSOR_ERROR("Platform Initialization failed");
        result = CAMERA_EFAILED;
    }

    if (CAMERA_SUCCESS == result && pSensorData->use_sensor_custom_func &&
            pSensorData->sensor_custom_func.sensor_set_platform_func_table)
    {
        sensor_platform_func_table_t platform_func_table = {};
        platform_func_table.i2c_read = NULL;
        platform_func_table.i2c_write_array = NULL;
        platform_func_table.i2c_slave_read = &SensorDriver_SlaveReadRegister;
        platform_func_table.i2c_slave_read_bulk = &SensorDriver_SlaveBulkRead;
        platform_func_table.i2c_slave_write_array = &SensorDriver_SlaveWriteRegisters;
        platform_func_table.i2c_slave_write_bulk = &SensorDriver_SlaveBulkWrite,
        platform_func_table.i2c_slave_write_array_sync = &SensorDriver_SlaveWriteRegisters_sync;
        platform_func_table.i2c_slave_bulk_write_then_read = &SensorDriver_SlaveBulkWriteRead,
        platform_func_table.execute_power_setting = &SensorDriver_ExecutePowerSetting;
        platform_func_table.setup_gpio_interrupt = &SensorDriver_SetupGpioInterrupt;
        platform_func_table.setup_cci_frame_sync = &SensorDriver_SetupCciFrameSync;
        platform_func_table.trigger_cci_frame_sync = &SensorDriver_TriggerCciFrameSync;
        platform_func_table.event_cb = &SensorDriver_InputCallback;

        result = pSensorData->sensor_custom_func.sensor_set_platform_func_table(
                pSensorData, &platform_func_table);
    }

    if(CAMERA_SUCCESS == result)
    {
        SENSOR_LOG("Success");
    }
    else
    {
        SENSOR_ERROR("Initialization failed");
    }

    SENSOR_FUNCTIONEXIT("result %d", result);

    return result;
} /* SensorDriver::Initialize */

/* ---------------------------------------------------------------------------
* FUNCTION SensorDriver::Uninitialize
* DESCRIPTION UnInitialize the driver interface
* DEPENDENCIES None
* PARAMETERS None
* RETURN VALUE CameraResult
* SIDE EFFECTS None
* ------------------------------------------------------------------------ */

CameraResult SensorDriver::Uninitialize()
{
    SENSOR_FUNCTIONENTRY("");

    m_pSensorLib = NULL;
    SensorPlatform::SensorPlatformDeinit(m_pSensorPlatform);

    SENSOR_FUNCTIONEXIT("");

    return CAMERA_SUCCESS;
} /* SensorDriver::Interface_UnInitialize */


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "camxsensordriverapi.h"
// NOWHINE ENTIRE FILE

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Sensor specific defenitions
#define AR0820_REG_COARSE_INTEGRATION_TIME  0x3012
#define AR0820_REG_FINE_INTEGRATION_TIME    0x3014
#define AR0820_REG_ANALOG_GAIN              0x3366
#define AR0820_REG_ANALOG_FINE_GAIN         0x336A
#define AR0820_REG_DC_GAIN                  0x3362
#define AR0820_REG_DIG_GLOBAL_GAIN          0x305E

#define FINE_INTEGRATION_TIME               0x0
#define AR0820_ROPS                         4
#define AR0820_ALIAS_ADDR_CAM_SNSR          0x20

#define FLL                                 2336
#define LLP                                 4440
#define FPS                                 30
#define VT_PIX_CLK                          156000000
#define VT_PIX_CLK_CORRECTION_FACTOR        (FLL * LLP * FPS / VT_PIX_CLK)

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define AR0820_PACK_ANALOG_GAIN(_g_) \
    (((_g_) << 12) | ((_g_) << 8) | ((_g_) << 4) | (_g_))

struct gainTable
{
    FLOAT analogRealGain;           // Real value of the sensor analog gain
    CHAR analogRegisterGain;        // Register value of the sensor Analog gain
    CHAR analogFineRegisterGain;    // Register value of the sensor Analog Fine gain
    CHAR lcgHcg;                    // Register value of the sensor Conversion gain application
};

static struct gainTable gainTbl[] =
{
    { 1.0f,   0, 0, 0},
    { 1.062f, 0, 1, 0},
    { 1.124f, 0, 2, 0},
    { 1.186f, 0, 3, 0},
    { 1.252f, 0, 4, 0},
    { 1.314f, 0, 5, 0},
    { 1.376f, 0, 6, 0},
    { 1.438f, 0, 7, 0},
    { 1.5f,   0, 8, 0},
    { 1.562f, 0, 9, 0},
    { 1.624f, 0, 10, 0},
    { 1.686f, 0, 11, 0},
    { 1.752f, 0, 12, 0},
    { 1.814f, 0, 13, 0},
    { 1.876f, 0, 14, 0},
    { 1.938f, 0, 15, 0},
    { 2.0f,   1, 0, 0},
    { 2.124f, 1, 1, 0},
    { 2.252f, 1, 2, 0},
    { 2.376f, 1, 3, 0},
    { 2.5f,   1, 4, 0},
    { 2.624f, 1, 5, 0},
    { 2.752f, 1, 6, 0},
    { 2.876f, 1, 7, 0},
    { 3.0f,   1, 8, 0},
    { 3.124f, 1, 9, 0},
    { 3.252f, 1, 10, 0},
    { 3.376f, 1, 11, 0},
    { 3.5f,   1, 12, 0},
    { 3.624f, 1, 13, 0},
    { 3.752f, 1, 14, 0},
    { 3.876f, 1, 15, 0},
    { 4.0f,   2, 0, 0},
    { 4.252f, 2, 1, 0},
    { 4.5f,   2, 2, 0},
    { 4.752f, 2, 3, 0},
    { 5.0f,   2, 4, 0},
    { 5.252f, 2, 5, 0},
    { 5.5f,   2, 6, 0},
    { 5.752f, 2, 7, 0},
    { 6.0f,   2, 8, 0},
    { 6.252f, 2, 9, 0},
    { 6.5f,   2, 10, 0},
    { 6.752f, 2, 11, 0},
    { 7.0f,   2, 12, 0},
    { 7.252f, 2, 13, 0},
    { 7.5f,   2, 14, 0},
    { 7.752f, 2, 15, 0},
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getGainTableIdx
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static INT32 getGainTableIdx(float realGain)
{
    INT32 gainTableSize = sizeof(gainTbl)/sizeof(gainTbl[0]);
    INT32 lastIdx = gainTableSize - 1;
    INT32 idx = 0;

    if (realGain >= gainTbl[lastIdx].analogRealGain)
    {
        idx = lastIdx;
    }
    else
    {
        for (idx = 0; idx < lastIdx; idx++)
        {
            if (realGain < gainTbl[idx+1].analogRealGain)
            {
                break;
            }
        }
    }

    return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FillExposureSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CalculateExposure(
    SensorExposureInfo*          pExposureInfo,
    SensorCalculateExposureData* pCalculateExposureData)
{
    if (NULL == pExposureInfo || NULL == pCalculateExposureData)
    {
        return FALSE;
    }

    // Get index corresponding to the realGain and use it to
    // obtain all relevant data.
    INT32 idx = getGainTableIdx(pCalculateExposureData->realGain);

    // Store gain table index in analogRegisterGain for later use
    // in FillExposureSettings function. It has no meaning anywhere
    // else outside of this file.
    pExposureInfo->analogRegisterGain   = idx;
    pExposureInfo->analogRealGain       = gainTbl[idx].analogRealGain;
    pExposureInfo->digitalRealGain      = pCalculateExposureData->realGain / pExposureInfo->analogRealGain;
    // The digital gain needs to be at least 1.0f
    pExposureInfo->digitalRealGain      = (pExposureInfo->digitalRealGain < 1.0f) ?
                                            1.0f : pExposureInfo->digitalRealGain;
    pExposureInfo->digitalRegisterGain  = FLOAT_TO_Q(7, pExposureInfo->digitalRealGain);
    pExposureInfo->ISPDigitalGain       = 1.0;
    // We use adjusting factor to line count specific to this sensor.
    // AR0820_ROPS is sensor specific and a factor of about 2 is required to compensate for
    // incorrect calculation of VTPixelClock in ImageSensorData::GetLineReadoutTime
    pExposureInfo->lineCount            = pCalculateExposureData->lineCount / (AR0820_ROPS * VT_PIX_CLK_CORRECTION_FACTOR);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FillExposureSettings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FillExposureSettings(
    RegSettingsInfo*        pRegSettingsInfo,
    SensorFillExposureData* pExposureData)
{
    UINT32  index     = 0;
    INT32   offset    = 0;
    UINT16  regCount  = 0;

    if ((NULL == pRegSettingsInfo) || (NULL == pExposureData))
    {
        return FALSE;
    }

    for (index = 0; index < pExposureData->pRegInfo->groupHoldOnSettings.regSettingCount; index++)
    {
        pRegSettingsInfo->regSetting[regCount].slaveAddr    = AR0820_ALIAS_ADDR_CAM_SNSR;
        pRegSettingsInfo->regSetting[regCount].slaveAddrExists = TRUE;
        pRegSettingsInfo->regSetting[regCount].registerAddr =
            pExposureData->pRegInfo->groupHoldOnSettings.regSetting[index].registerAddr;
        pRegSettingsInfo->regSetting[regCount].registerData =
            pExposureData->pRegInfo->groupHoldOnSettings.regSetting[index].registerData;
        pRegSettingsInfo->regSetting[regCount].regAddrType  =
            pExposureData->pRegInfo->groupHoldOnSettings.regSetting[index].regAddrType;
        pRegSettingsInfo->regSetting[regCount].regDataType  =
            pExposureData->pRegInfo->groupHoldOnSettings.regSetting[index].regDataType;
        pRegSettingsInfo->regSetting[regCount].delayUs      =
            pExposureData->pRegInfo->groupHoldOnSettings.regSetting[index].delayUs;
        pRegSettingsInfo->regSetting[regCount].operation=IOOperationTypeWrite;
        regCount++;
    }
    offset = pExposureData->pRegInfo->groupHoldOnSettings.regSettingCount;

    // Get the index for gain table from analogRegisterGain
    INT32 gainTblIdx            = pExposureData->analogRegisterGain;
    CHAR analogRegisterGain     = gainTbl[gainTblIdx].analogRegisterGain;
    CHAR analogFineRegisterGain = gainTbl[gainTblIdx].analogFineRegisterGain;
    CHAR lcgHcg                 = gainTbl[gainTblIdx].lcgHcg;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_COARSE_INTEGRATION_TIME;
    pRegSettingsInfo->regSetting[regCount].registerData  = (pExposureData->lineCount & 0xffff);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_FINE_INTEGRATION_TIME;
    pRegSettingsInfo->regSetting[regCount].registerData  = FINE_INTEGRATION_TIME;
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_ANALOG_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = AR0820_PACK_ANALOG_GAIN(analogRegisterGain & 0xf);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_ANALOG_FINE_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = AR0820_PACK_ANALOG_GAIN(analogFineRegisterGain & 0xf);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_DC_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = (lcgHcg ? 0xFF : 0x00);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0820_REG_DIG_GLOBAL_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = pExposureData->digitalRegisterGain;
    regCount++;

    for (index = offset; index < regCount; index++)
    {
        pRegSettingsInfo->regSetting[index].slaveAddr       = AR0820_ALIAS_ADDR_CAM_SNSR;
        pRegSettingsInfo->regSetting[index].slaveAddrExists = TRUE;
        pRegSettingsInfo->regSetting[index].regAddrType     = I2CRegAddressDataTypeWord;
        pRegSettingsInfo->regSetting[index].regDataType     = I2CRegAddressDataTypeWord;
        pRegSettingsInfo->regSetting[index].delayUs         = 0;
        pRegSettingsInfo->regSetting[index].operation       = IOOperationTypeWrite;
    }
    for (index = 0; index < pExposureData->pRegInfo->groupHoldOffSettings.regSettingCount; index++)
    {
        pRegSettingsInfo->regSetting[regCount].slaveAddr     = AR0820_ALIAS_ADDR_CAM_SNSR;
        pRegSettingsInfo->regSetting[regCount].slaveAddrExists = TRUE;
        pRegSettingsInfo->regSetting[regCount].registerAddr  =
            pExposureData->pRegInfo->groupHoldOffSettings.regSetting[index].registerAddr;
        pRegSettingsInfo->regSetting[regCount].registerData  =
            pExposureData->pRegInfo->groupHoldOffSettings.regSetting[index].registerData;
        pRegSettingsInfo->regSetting[regCount].regAddrType  =
            pExposureData->pRegInfo->groupHoldOffSettings.regSetting[index].regAddrType;
        pRegSettingsInfo->regSetting[regCount].regDataType=
            pExposureData->pRegInfo->groupHoldOffSettings.regSetting[index].regDataType;
        pRegSettingsInfo->regSetting[regCount].delayUs      =
            pExposureData->pRegInfo->groupHoldOffSettings.regSetting[index].delayUs;
        pRegSettingsInfo->regSetting[regCount].operation=IOOperationTypeWrite;
        regCount++;
    }

    pRegSettingsInfo->regSettingCount = regCount;

    if (MAX_REG_SETTINGS <= regCount)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GetSensorLibraryAPIs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID GetSensorLibraryAPIs(
    SensorLibraryAPI* pSensorLibraryAPI)
{
    pSensorLibraryAPI->majorVersion          = 2;
    pSensorLibraryAPI->minorVersion          = 0;
    pSensorLibraryAPI->pCalculateExposure    = CalculateExposure;
    pSensorLibraryAPI->pFillExposureSettings = FillExposureSettings;
}

#ifdef __cplusplus
} // CamX Namespace
#endif // __cplusplus

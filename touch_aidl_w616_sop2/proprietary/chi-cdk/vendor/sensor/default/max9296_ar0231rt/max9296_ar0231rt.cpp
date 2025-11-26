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
#define AR0231_REG_COARSE_INTEGRATION_TIME  0x3012
#define AR0231_REG_FINE_INTEGRATION_TIME    0x3014
#define AR0231_REG_ANALOG_GAIN              0x3366
//#define AR0231_REG_ANALOG_FINE_GAIN         0x336A
#define AR0231_REG_DC_GAIN                  0x3362
#define AR0231_REG_DIG_GLOBAL_GAIN          0x305E

#define FINE_INTEGRATION_TIME               0x032A
#define AR0231_ROPS                         4
#define AR0231_ALIAS_ADDR_CAM_SNSR          0x20

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define AR0231_PACK_ANALOG_GAIN(_g_) \
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
    { 2.90f, 0, 0, 1 },
    { 3.08f, 0, 1, 1 },
    { 3.26f, 0, 2, 1 },
    { 3.44f, 0, 3, 1 },
    { 3.63f, 0, 4, 1 },
    { 3.81f, 0, 5, 1 },
    { 3.99f, 0, 6, 1 },
    { 4.17f, 0, 7, 1 },
    { 4.35f, 0, 8, 1 },
    { 4.53f, 0, 9, 1 },
    { 4.71f, 0, 10, 1 },
    { 4.89f, 0, 11, 1 },
    { 5.08f, 0, 12, 1 },
    { 5.26f, 0, 13, 1 },
    { 5.44f, 0, 14, 1 },
    { 5.62f, 0, 15, 1 },
    { 5.80f, 1, 0, 1 },
    { 6.16f, 1, 1, 1 },
    { 6.53f, 1, 2, 1 },
    { 6.89f, 1, 3, 1 },
    { 7.25f, 1, 4, 1 },
    { 7.61f, 1, 5, 1 },
    { 7.98f, 1, 6, 1 },
    { 8.34f, 1, 7, 1 },
    { 8.70f, 1, 8, 1 },
    { 9.06f, 1, 9, 1 },
    { 9.43f, 1, 10, 1 },
    { 9.79f, 1, 11, 1 },
    { 10.15f, 1, 12, 1 },
    { 10.51f, 1, 13, 1 },
    { 10.88f, 1, 14, 1 },
    { 11.24f, 1, 15, 1 },
    { 11.60f, 2, 0, 1 },
    { 12.33f, 2, 1, 1 },
    { 13.05f, 2, 2, 1 },
    { 13.78f, 2, 3, 1 },
    { 14.50f, 2, 4, 1 },
    { 15.23f, 2, 5, 1 },
    { 15.95f, 2, 6, 1 },
    { 16.68f, 2, 7, 1 },
    { 17.40f, 2, 8, 1 },
    { 18.13f, 2, 9, 1 },
    { 18.85f, 2, 10, 1 },
    { 19.58f, 2, 11, 1 },
    { 20.30f, 2, 12, 1 },
    { 21.03f, 2, 13, 1 },
    { 21.75f, 2, 14, 1 },
    { 22.48f, 2, 15, 1 },
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RegisterToRealGain
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
// RegisterToRealGain
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static DOUBLE RegisterToRealGain(
    UINT registerGain)
{
    DOUBLE real_gain;
    real_gain = (DOUBLE) (((DOUBLE)(registerGain))/16.0);
    return real_gain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RealToRegisterGain
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static UINT RealToRegisterGain(
    DOUBLE realGain)
{
    UINT reg_gain = 0;
    realGain = realGain*16.0;
    reg_gain = (UINT)realGain;
    return reg_gain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CalculateDigitalGain
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static UINT CalculateDigitalGain(
    FLOAT totalRealGain,
    FLOAT sensorRealGain)
{
    CDK_UNUSED_PARAM(totalRealGain);
    CDK_UNUSED_PARAM(sensorRealGain);
    return 1;
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
    // First entry of gain table starts from 2.90f gain. This will lead to digital
    // gain less than 1.0f if AEC real gain is less than this value. It needs to be
    // round up to 1.0f in this case.
    pExposureInfo->digitalRealGain      = (pExposureInfo->digitalRealGain < 1.0f) ?
                                            1.0f : pExposureInfo->digitalRealGain;
    pExposureInfo->digitalRegisterGain  = FLOAT_TO_Q(9, pExposureInfo->digitalRealGain);
    pExposureInfo->ISPDigitalGain       = 1.0;
    // We use adjusting factor to line count specific to this sensor.
    // AR0231_ROPS is sensor specific and 2 is factor to required for
    // incorrect calculation of VTPixelClock in ImageSensorData::GetLineReadoutTime
    pExposureInfo->lineCount            = pCalculateExposureData->lineCount / (AR0231_ROPS * 2);

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
        pRegSettingsInfo->regSetting[regCount].slaveAddr    = AR0231_ALIAS_ADDR_CAM_SNSR;
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

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_COARSE_INTEGRATION_TIME;
    pRegSettingsInfo->regSetting[regCount].registerData  = (pExposureData->lineCount & 0xffff);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_FINE_INTEGRATION_TIME;
    pRegSettingsInfo->regSetting[regCount].registerData  = FINE_INTEGRATION_TIME;
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_ANALOG_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = AR0231_PACK_ANALOG_GAIN(analogRegisterGain & 0xf);
    regCount++;

    //pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_ANALOG_FINE_GAIN;
    //pRegSettingsInfo->regSetting[regCount].registerData  = AR0231_PACK_ANALOG_GAIN(analogFineRegisterGain & 0xf);
    //regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_DC_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = (lcgHcg ? 0xFF : 0x00);
    regCount++;

    pRegSettingsInfo->regSetting[regCount].registerAddr  = AR0231_REG_DIG_GLOBAL_GAIN;
    pRegSettingsInfo->regSetting[regCount].registerData  = pExposureData->digitalRegisterGain;
    regCount++;

    for (index = offset; index < regCount; index++)
    {
        pRegSettingsInfo->regSetting[index].slaveAddr       = AR0231_ALIAS_ADDR_CAM_SNSR;
        pRegSettingsInfo->regSetting[index].slaveAddrExists = TRUE;
        pRegSettingsInfo->regSetting[index].regAddrType     = I2CRegAddressDataTypeWord;
        pRegSettingsInfo->regSetting[index].regDataType     = I2CRegAddressDataTypeWord;
        pRegSettingsInfo->regSetting[index].delayUs         = 0;
        pRegSettingsInfo->regSetting[index].operation       = IOOperationTypeWrite;
    }
    for (index = 0; index < pExposureData->pRegInfo->groupHoldOffSettings.regSettingCount; index++)
    {
        pRegSettingsInfo->regSetting[regCount].slaveAddr     = AR0231_ALIAS_ADDR_CAM_SNSR;
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

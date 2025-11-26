/**
 * @file CameraConfig.c
 *
 * @brief Implementation of CameraConfig
 *
 * Copyright (c) 2011-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/*============================================================================
                        INCLUDE FILES
============================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "CameraOSServices.h"
#include "CameraConfig.h"
#include "CameraPlatform.h"

#ifdef CAMERA_CONFIG_ENABLE_XML_PARSER
#ifdef AIS_BUILD_STATIC_DEVICES
#error "Cannot support static linking of devices with dynamic XML parser"
#endif

#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// PMIC/TLMM pins and TLMM interrupt pins
#if defined(__QNXNTO__) && !defined(CAMERA_UNITTEST)
#include "gpio_devctl.h"
#else
#define GPIO_PIN_CFG(output, pull, strength, func) 0
#endif

/*Parse int attribute*/
#define XML_GET_INT_ATTR(_var_, _node_, _attr_, _opt_, _type_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        _var_ = (_type_)strtoul((const char *)_p_, NULL, 0); \
        xmlFree(_p_); \
    } else if (!_opt_) { \
        CAM_MSG(ERROR, "could not get attribute " _attr_); \
    } else { \
        CAM_MSG(LOW, "optional attribute " _attr_ " not defined"); \
    }\
} while(0)

/*Parse float attribute*/
#define XML_GET_FLOAT_ATTR(_var_, _node_, _attr_, _opt_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        _var_ = strtof((const char *)_p_, NULL); \
        xmlFree(_p_); \
    } else if (!_opt_) { \
        CAM_MSG(ERROR, "could not get attribute " _attr_); \
    } else { \
        CAM_MSG(LOW, "optional attribute " _attr_ " not defined"); \
    }\
} while(0)

/*Get string attribute*/
#define XML_GET_STRING_ATTR(_var_, _node_, _attr_, _opt_) \
do { \
    xmlChar* _p_ = xmlGetProp(_node_, (const xmlChar *)_attr_); \
    if (_p_) { \
        snprintf(_var_, sizeof(_var_), "%s", _p_); \
        xmlFree(_p_); \
    } else if (!_opt_) { \
        CAM_MSG(ERROR, "could not get attribute " _attr_); \
    } else { \
        CAM_MSG(LOW, "optional attribute " _attr_ " not defined"); \
    }\
} while(0)

#define STRNCMP(_var_, _str_) \
    strncmp(_var_, _str_, strlen(_str_))

#endif /* CAMERA_CONFIG_ENABLE_XML_PARSER */

/* ===========================================================================
                DEFINITIONS AND DECLARATIONS FOR MODULE
=========================================================================== */
static int CameraConfigInit(void);
static int CameraConfigDeinit(void);
static CameraBoardType const * GetCameraBoardInfo(void);
static int GetCameraConfigVersion(void);
static int GetCameraChannelInfo(CameraChannelInfoType const **ppChannelInfo, int *nChannels);

#ifdef CAMERA_CONFIG_ENABLE_XML_PARSER
static CameraHwBoardType CameraConfigXMLParseBoardType(xmlNodePtr pCur);
static CameraI2CType CameraConfigXMLParseI2cType(xmlNodePtr pCur);
static CameraDeviceCategoryType CameraConfigXMLParseDeviceCategory(xmlNodePtr pCur);
static CameraSensorGPIO_IntrCfgType CameraConfigXMLParseIntrCfgType(xmlNodePtr pCur);
static CameraSensorGPIO_SignalType CameraConfigXMLParseGpioSignalType(xmlNodePtr pCur);
static CameraSensorGPIO_EventType CameraConfigXMLParseGpioEvtType(xmlNodePtr pCur);
static qcarcam_opmode_type CameraConfigXMLParseOpMode(xmlNodePtr pCur);
static qcarcam_isp_usecase_t CameraConfigXMLParseIspUseCase(xmlNodePtr pCur);

static CameraPowerManagerPolicyType CameraConfigXMLParsePowerManagementPolicyType(xmlNodePtr pCur);
static CameraLatencyMeasurementModeType CameraConfigXMLParseLatencyMeasurementModeType(xmlNodePtr pCur);

static void CameraConfigDumpBoardInfo(const CameraBoardType *pBoard);
static int CameraConfigXMLParseFile(CameraBoardType *pBoard, CameraChannelInfoType *pChannel, uint32 *pNumChannel);
static int CameraConfigXMLParseI2CDevs(xmlNodePtr pParent, CameraBoardType *boardInfo);
static int CameraConfigXMLParseInputDevs(xmlNodePtr pParent, CameraBoardType *boardInfo);
static int CameraConfigXMLParseInputMapping(xmlNodePtr pParent, CameraChannelInfoType *pChannelInfo, uint32 *pNumChannel);
static int CameraConfigXMLParseErrorMatchingFunctions(xmlNodePtr pParent, CameraBoardType *boardInfo);
#endif /*CAMERA_CONFIG_ENABLE_XML_PARSER*/


extern int SA8155_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        uint32 *nChannels);
extern int SA6155_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        uint32 *nChannels);
extern int SA8195_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        uint32 *nChannels);
extern int SA8295_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        uint32 *nChannels);
extern int SA8540_GetCameraConfig(CameraBoardType const **ppBoardInfo,
        CameraChannelInfoType const **ppChannelInfo,
        uint32 *nChannels);

static CameraBoardType g_cameraBoardDefinition = {};
static CameraChannelInfoType g_InputDeviceChannelInfo[MAX_NUM_CAMERA_CHANNELS] = {};

static const CameraBoardType* g_pCameraBoardDef = &g_cameraBoardDefinition;
static const CameraChannelInfoType* g_pInputDeviceChannelInfo = g_InputDeviceChannelInfo;
static uint32 g_numInputDeviceChannel = 0;

static ICameraConfig cameraConfigIf =
{
    CameraConfigInit,
    CameraConfigDeinit,
    GetCameraBoardInfo,
    GetCameraConfigVersion,
    GetCameraChannelInfo
};


/* ===========================================================================
                FUNCTIONS
=========================================================================== */
static int CameraConfigInit(void)
{
    int rc = 0;

#ifdef CAMERA_CONFIG_ENABLE_XML_PARSER
    rc = CameraConfigXMLParseFile(&g_cameraBoardDefinition, g_InputDeviceChannelInfo, &g_numInputDeviceChannel);

    //If failed to parse or non-existent file, use default config for target
    if (rc)
#endif
    {
        CameraPlatformChipIdType chipId = CameraPlatform_GetChipId();

        rc = 0;

        switch(chipId)
        {
        case CHIP_ID_SA8155:
        case CHIP_ID_SA8155P:
            SA8155_GetCameraConfig(&g_pCameraBoardDef, &g_pInputDeviceChannelInfo, &g_numInputDeviceChannel);
            break;
        case CHIP_ID_SA6155:
            SA6155_GetCameraConfig(&g_pCameraBoardDef, &g_pInputDeviceChannelInfo, &g_numInputDeviceChannel);
            break;
        case CHIP_ID_SA8195P:
            SA8195_GetCameraConfig(&g_pCameraBoardDef, &g_pInputDeviceChannelInfo, &g_numInputDeviceChannel);
            break;
        case CHIP_ID_SA8295P:
        case CHIP_ID_SA8540P:
            SA8295_GetCameraConfig(&g_pCameraBoardDef, &g_pInputDeviceChannelInfo, &g_numInputDeviceChannel);
            break;
        default:
            CAM_MSG(ERROR, "CameraConfig not implemented for chipId=%d", chipId);
            rc = -1;
            break;
        }
    }

    return rc;
}

static int CameraConfigDeinit(void)
{
    return 0;
}

// This method is used to retrieve the camera board interface pointer.
CAM_API ICameraConfig const * GetCameraConfigInterface(void)
{
    return &cameraConfigIf;
}

static CameraBoardType const * GetCameraBoardInfo(void)
{
    return g_pCameraBoardDef;
}

// This method is used to retrieve the camera board version.
static int GetCameraConfigVersion(void)
{
    return CAMERA_BOARD_LIBRARY_VERSION;
}

static int GetCameraChannelInfo(CameraChannelInfoType const **ppChannelInfo, int *nChannels)
{
    int rtnVal = -1;
    if (ppChannelInfo && nChannels)
    {
        *ppChannelInfo = g_pInputDeviceChannelInfo;
        *nChannels     = g_numInputDeviceChannel;
        rtnVal         = 0;
    }

    return rtnVal;
}

#ifdef CAMERA_CONFIG_ENABLE_XML_PARSER
static CameraHwBoardType CameraConfigXMLParseBoardType(xmlNodePtr pCur)
{
    CameraHwBoardType boardType = CAMERA_HW_BOARD_NONE;
    char* pBoardType = (char*)xmlGetProp(pCur, (const xmlChar *)"boardType");

    if (pBoardType)
    {
        if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPAIR_V1_PJ254"))
        {
            boardType = CAMERA_HW_BOARD_ADPAIR_V1_PJ254;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPAIR_V2_PL195"))
        {
            boardType = CAMERA_HW_BOARD_ADPAIR_V2_PL195;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPAIR_V3_PM731"))
        {
            boardType = CAMERA_HW_BOARD_ADPAIR_V3_PM731;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPSTAR_V1_W4944"))
        {
            boardType = CAMERA_HW_BOARD_ADPSTAR_V1_W4944;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPSTAR_V2_PK901"))
        {
            boardType = CAMERA_HW_BOARD_ADPSTAR_V2_PK901;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_TI9702_V1"))
        {
            boardType = CAMERA_HW_BOARD_TI9702_V1;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_QDRIVE2P5_PM134"))
        {
            boardType = CAMERA_HW_BOARD_QDRIVE2P5_PM134;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_QDRIVE2P5_PM135"))
        {
            boardType = CAMERA_HW_BOARD_QDRIVE2P5_PM135;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_QDRIVE3P0_PM134"))
        {
            boardType = CAMERA_HW_BOARD_QDRIVE3P0_PM134;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_QDRIVE3P0_PM135"))
        {
            boardType = CAMERA_HW_BOARD_QDRIVE3P0_PM135;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_QDRIVE2P5_PM136"))
        {
            boardType = CAMERA_HW_BOARD_QDRIVE2P5_PM136;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPSTAR_GEN4_PW171"))
        {
            boardType = CAMERA_HW_BOARD_ADPSTAR_GEN4_PW171;
        }
        else if(!STRNCMP(pBoardType, "CAMERA_HW_BOARD_ADPAIR_GEN4_V1_PW227"))
        {
            boardType = CAMERA_HW_BOARD_ADPAIR_GEN4_V1_PW227;
        }
        else
        {
            boardType = CAMERA_HW_BOARD_NONE;
        }

        xmlFree(pBoardType);
    }

    return boardType;
}

static CameraI2CType CameraConfigXMLParseI2cType(xmlNodePtr pCur)
{
    CameraI2CType i2cType = CAMERA_I2C_TYPE_NONE;
    char* pI2ctype = (char*)xmlGetProp(pCur, (const xmlChar *)"i2ctype");

    if (pI2ctype)
    {
        if(!STRNCMP(pI2ctype, "CAMERA_I2C_TYPE_CCI"))
        {
            i2cType = CAMERA_I2C_TYPE_CCI;
        }
        else if(!STRNCMP(pI2ctype, "CAMERA_I2C_TYPE_I2C"))
        {
            i2cType = CAMERA_I2C_TYPE_I2C;
        }
        else
        {
            i2cType = CAMERA_I2C_TYPE_NONE;
        }

        xmlFree(pI2ctype);
    }

    return i2cType;
}

static CameraDeviceCategoryType CameraConfigXMLParseDeviceCategory(xmlNodePtr pCur)
{
    CameraDeviceCategoryType devCategory = CAMERA_DEVICE_CATEGORY_ALL;
    char* pDevCategory = (char*)xmlGetProp(pCur, (const xmlChar *)"devCategory");

    if (pDevCategory)
    {
        if(!STRNCMP(pDevCategory, "CAMERA_DEVICE_CATEGORY_SENSOR"))
        {
            devCategory = CAMERA_DEVICE_CATEGORY_SENSOR;
        }
        else if(!STRNCMP(pDevCategory, "CAMERA_DEVICE_CATEGORY_CSIPHY"))
        {
            devCategory = CAMERA_DEVICE_CATEGORY_CSIPHY;
        }
        else if(!STRNCMP(pDevCategory, "CAMERA_DEVICE_CATEGORY_IFE"))
        {
            devCategory = CAMERA_DEVICE_CATEGORY_IFE;
        }
        else
        {
            devCategory = CAMERA_DEVICE_CATEGORY_ALL;
        }

        xmlFree(pDevCategory);
    }

    return devCategory;
}

static CameraSensorGPIO_SignalType CameraConfigXMLParseGpioSignalType(xmlNodePtr pCur)
{
    CameraSensorGPIO_SignalType gpioType = CAMERA_GPIO_INVALID;
    char* pGpioType = (char*)xmlGetProp(pCur, (const xmlChar *)"id");

    if (pGpioType)
    {
        if (!STRNCMP(pGpioType, "CAMERA_GPIO_INTR"))
        {
            gpioType = CAMERA_GPIO_INTR;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_RESET"))
        {
            gpioType = CAMERA_GPIO_RESET;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_VANA"))
        {
            gpioType = CAMERA_GPIO_VANA;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_VDIG"))
        {
            gpioType = CAMERA_GPIO_VDIG;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_VIO"))
        {
            gpioType = CAMERA_GPIO_VIO;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_VAF"))
        {
            gpioType = CAMERA_GPIO_VAF;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_VAF_PWDM"))
        {
            gpioType = CAMERA_GPIO_VAF_PWDM;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_CUSTOM_REG1"))
        {
            gpioType = CAMERA_GPIO_CUSTOM_REG1;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_CUSTOM_REG2"))
        {
            gpioType = CAMERA_GPIO_CUSTOM_REG2;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_STANDBY"))
        {
            gpioType = CAMERA_GPIO_STANDBY;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_CUSTOM1"))
        {
            gpioType = CAMERA_GPIO_CUSTOM1;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_CUSTOM2"))
        {
            gpioType = CAMERA_GPIO_CUSTOM2;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_INTR1"))
        {
            gpioType = CAMERA_GPIO_INTR1;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_INTR2"))
        {
            gpioType = CAMERA_GPIO_INTR2;
        }
        else if (!STRNCMP(pGpioType, "CAMERA_GPIO_FSYNC"))
        {
            gpioType = CAMERA_GPIO_FSYNC;
        }
        else
        {
            gpioType = CAMERA_GPIO_INVALID;
        }

        xmlFree(pGpioType);
    }

    return gpioType;
}

static CameraSensorGPIO_IntrCfgType CameraConfigXMLParseIntrCfgType(xmlNodePtr pCur)
{
    CameraSensorGPIO_IntrCfgType intrCfgType = CAMERA_GPIO_INTR_NONE;
    char* pIntrCfgType = (char*)xmlGetProp(pCur, (const xmlChar *)"intr_type");

    if (pIntrCfgType)
    {
        if(!STRNCMP(pIntrCfgType, "CAMERA_GPIO_INTR_POLL"))
        {
            intrCfgType = CAMERA_GPIO_INTR_POLL;
        }
        if(!STRNCMP(pIntrCfgType, "CAMERA_GPIO_INTR_TLMM"))
        {
            intrCfgType = CAMERA_GPIO_INTR_TLMM;
        }
        else if(!STRNCMP(pIntrCfgType, "CAMERA_GPIO_INTR_PMIC"))
        {
            intrCfgType = CAMERA_GPIO_INTR_PMIC;
        }
        else
        {
            intrCfgType = CAMERA_GPIO_INTR_NONE;
        }

        xmlFree(pIntrCfgType);
    }

    return intrCfgType;
}

static CameraSensorGPIO_EventType CameraConfigXMLParseGpioEvtType(xmlNodePtr pCur)
{
    CameraSensorGPIO_EventType gpioEventType = CAMERA_GPIO_TRIGGER_NONE;
    char* pGpioEventType = (char*)xmlGetProp(pCur, (const xmlChar *)"trigger");

    if (pGpioEventType)
    {
        if(!STRNCMP(pGpioEventType, "CAMERA_GPIO_TRIGGER_RISING"))
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_RISING;
        }
        else if(!STRNCMP(pGpioEventType, "CAMERA_GPIO_TRIGGER_FALLING"))
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_FALLING;
        }
        else if(!STRNCMP(pGpioEventType, "CAMERA_GPIO_TRIGGER_EDGE"))
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_EDGE;
        }
        else if(!STRNCMP(pGpioEventType, "CAMERA_GPIO_TRIGGER_LEVEL_LOW"))
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_LEVEL_LOW;
        }
        else if(!STRNCMP(pGpioEventType, "CAMERA_GPIO_TRIGGER_LEVEL_HIGH"))
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_LEVEL_HIGH;
        }
        else
        {
            gpioEventType = CAMERA_GPIO_TRIGGER_NONE;
        }

        xmlFree(pGpioEventType);
    }

    return gpioEventType;
}

static qcarcam_opmode_type CameraConfigXMLParseOpMode(xmlNodePtr pCur)
{
    qcarcam_opmode_type opMode = QCARCAM_OPMODE_RAW_DUMP;
    char* pOpMode = (char*)xmlGetProp(pCur, (const xmlChar *)"opMode");

    if (pOpMode)
    {
        if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_RAW_DUMP"))
        {
            opMode = QCARCAM_OPMODE_RAW_DUMP;
        }
        else if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_SHDR"))
        {
            opMode = QCARCAM_OPMODE_SHDR;
        }
        else if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_INJECT"))
        {
            opMode = QCARCAM_OPMODE_INJECT;
        }
        else if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_PAIRED_INPUT"))
        {
            opMode = QCARCAM_OPMODE_PAIRED_INPUT;
        }
        else if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_RGBIR"))
        {
            opMode = QCARCAM_OPMODE_RGBIR;
        }
        else if(!STRNCMP(pOpMode, "QCARCAM_OPMODE_ISP"))
        {
            opMode = QCARCAM_OPMODE_ISP;
        }
        else
        {
            opMode = QCARCAM_OPMODE_RAW_DUMP;
        }
        xmlFree(pOpMode);
    }

    return opMode;
}

static qcarcam_isp_usecase_t CameraConfigXMLParseIspUseCase(xmlNodePtr pCur)
{
    qcarcam_isp_usecase_t useCase = QCARCAM_ISP_USECASE_SHDR_BPS_IPE_AEC_AWB;
    char* pUseCase= (char*)xmlGetProp(pCur, (const xmlChar *)"useCase");

    if (pUseCase)
    {
        if(!STRNCMP(pUseCase, "QCARCAM_ISP_USECASE_SHDR_BPS_IPE_AEC_AWB"))
        {
            useCase = QCARCAM_ISP_USECASE_SHDR_BPS_IPE_AEC_AWB;
        }
        else if(!STRNCMP(pUseCase, "QCARCAM_ISP_USECASE_BPS_IPE_AEC_AWB"))
        {
            useCase = QCARCAM_ISP_USECASE_BPS_IPE_AEC_AWB;
        }
        xmlFree(pUseCase);
    }

    return useCase;
}

static CameraPowerManagerPolicyType CameraConfigXMLParsePowerManagementPolicyType(xmlNodePtr pCur)
{
    CameraPowerManagerPolicyType powerManagementPolicy = CAMERA_PM_POLICY_LPM_EVENT_ALL;
    char* pPowerManagementPolicy = (char*)xmlGetProp(pCur, (const xmlChar *)"powerManagementPolicy");

    if (pPowerManagementPolicy)
    {
        if(!STRNCMP(pPowerManagementPolicy, "CAMERA_PM_POLICY_NO_LPM"))
        {
            powerManagementPolicy = CAMERA_PM_POLICY_NO_LPM;
        }
        else if(!STRNCMP(pPowerManagementPolicy, "CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS"))
        {
            powerManagementPolicy = CAMERA_PM_POLICY_LPM_EVENT_FOR_INPUTS;
        }
        else if(!STRNCMP(pPowerManagementPolicy, "CAMERA_PM_POLICY_LPM_EVENT_ALL"))
        {
            powerManagementPolicy = CAMERA_PM_POLICY_LPM_EVENT_ALL;
        }

        xmlFree(pPowerManagementPolicy);
    }

    return powerManagementPolicy;
}

static CameraLatencyMeasurementModeType CameraConfigXMLParseLatencyMeasurementModeType(xmlNodePtr pCur)
{
    CameraLatencyMeasurementModeType latencyMeasurementMode = CAMERA_LM_MODE_DISABLE;
    char* pLatencyMeasurementMode = (char*)xmlGetProp(pCur, (const xmlChar *)"latencyMeasurementMode");

    if (pLatencyMeasurementMode)
    {
        if(!STRNCMP(pLatencyMeasurementMode, "CAMERA_LM_MODE_DISABLE"))
        {
            latencyMeasurementMode = CAMERA_LM_MODE_DISABLE;
        }
        else if(!STRNCMP(pLatencyMeasurementMode, "CAMERA_LM_MODE_END2END"))
        {
            latencyMeasurementMode = CAMERA_LM_MODE_END2END;
        }
        else if(!STRNCMP(pLatencyMeasurementMode, "CAMERA_LM_MODE_ALL_STEPS"))
        {
            latencyMeasurementMode = CAMERA_LM_MODE_ALL_STEPS;
        }

        xmlFree(pLatencyMeasurementMode);
    }

    return latencyMeasurementMode;
}

static int CameraConfigXMLParseI2CDevs(xmlNodePtr pParent, CameraBoardType *boardInfo)
{
    int rc = 0;
    xmlNodePtr pCur = pParent->xmlChildrenNode;
    uint32 i2cDevIdx = 0;

    while(pCur != NULL && !rc)
    {
        if(!(xmlStrcmp(pCur->name, (const xmlChar *) "i2cDev")))
        {
            xmlNodePtr pChild;

            if (i2cDevIdx >= MAX_NUM_CAMERA_I2C_DEVS)
            {
                CAM_MSG(ERROR, "Number of i2cDev exceeds max %d", MAX_NUM_CAMERA_I2C_DEVS);
                rc = -1;
                break;
            }

            XML_GET_STRING_ATTR(boardInfo->i2c[i2cDevIdx].i2cDevname, pCur, "name", 1);

            pChild = pCur->xmlChildrenNode;
            while(pChild != NULL)
            {
                if(!(xmlStrcmp(pChild->name, (const xmlChar *) "properties")))
                {
                    boardInfo->i2c[i2cDevIdx].i2ctype = CameraConfigXMLParseI2cType(pChild);

                    XML_GET_INT_ATTR(boardInfo->i2c[i2cDevIdx].device_id, pChild, "device_id", 1, uint32);
                    XML_GET_INT_ATTR(boardInfo->i2c[i2cDevIdx].port_id, pChild, "port_id", 1, uint32);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "sda_pin")))
                {
                    uint32 function = 0;

                    XML_GET_INT_ATTR(boardInfo->i2c[i2cDevIdx].sda_pin.num, pChild, "gpio", 1, uint32);
                    XML_GET_INT_ATTR(function, pChild, "function", 1, uint32);
                    boardInfo->i2c[i2cDevIdx].sda_pin.config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, function);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "scl_pin")))
                {
                    uint32 function = 0;

                    XML_GET_INT_ATTR(boardInfo->i2c[i2cDevIdx].scl_pin.num, pChild, "gpio", 1, uint32);
                    XML_GET_INT_ATTR(function, pChild, "function", 1, uint32);
                    boardInfo->i2c[i2cDevIdx].scl_pin.config = GPIO_PIN_CFG(GPIO_OUTPUT, GPIO_PULL_UP, GPIO_STRENGTH_2MA, function);
                }
                pChild = pChild->next;
            }

            i2cDevIdx++;
        }
        else if(!xmlNodeIsText(pCur))
        {
            CAM_MSG(ERROR, "CameraConfig %s inside %s", pCur->name, pParent->name);
            rc = -1;
        }

        pCur = pCur->next;
    }

    return rc;
}

static int CameraConfigParseInputDevProperties(xmlNodePtr pParent, CameraSensorBoardType* pInputDev, uint32 socId)
{
    int rc = 0;

    xmlNodePtr pChild = pParent->xmlChildrenNode;
    if (!pChild)
    {
        CAM_MSG(ERROR, "Missing input device properties config");
        rc = -1;
    }

    while (pChild != NULL && !rc)
    {
        if(!(xmlStrcmp(pChild->name, (const xmlChar *) "config")))
        {
            XML_GET_INT_ATTR(pInputDev->devConfig.subdevId, pChild, "subdevId", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.opMode, pChild, "opMode", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.type, pChild, "type", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.numSensors, pChild, "numSensors", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.masterSocId, pChild, "masterSocId", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.socMap, pChild, "socMap", 1, uint32);
            XML_GET_INT_ATTR(pInputDev->devConfig.powerSaveMode, pChild, "powersaveMode", 1, CameraPowerSaveModeType);

            // Override socMap value to 1 connected soc.
            if (0 == pInputDev->devConfig.socMap)
            {
                pInputDev->devConfig.socMap = 1;
            }

            // Providing I2C read  access.
            if (!(pInputDev->devConfig.socMap & (1 << socId)))
            {
                pInputDev->devConfig.accessMode = I2C_NO_ACCESS;
                CAM_MSG(HIGH, "No I2C Access");
            }
            else if ((pInputDev->devConfig.socMap & (1 << socId)) &&
                     (pInputDev->devConfig.masterSocId == socId))
            {
                pInputDev->devConfig.accessMode = I2C_FULL_ACCESS;
                CAM_MSG(HIGH, "Full I2C Access");
            }
            else if ((pInputDev->devConfig.socMap & (1 << socId)) &&
                     (pInputDev->devConfig.masterSocId != socId))
            {
                pInputDev->devConfig.accessMode = I2C_READ_ACCESS;
                CAM_MSG(HIGH, "Read I2C Access");
            }
            else
            {
                pInputDev->devConfig.accessMode = I2C_FULL_ACCESS;
                CAM_MSG(HIGH, "Full I2C Access");
            }
        }
        else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "dev_gpio")))
        {
            uint32 idx = 0;
            XML_GET_INT_ATTR(idx, pChild, "idx", 1, uint32);

            if (idx >= MAX_NUM_INPUT_DEV_INTERNAL_GPIO)
            {
                CAM_MSG(ERROR,"De-serializer GPIO idx exceeds max %d",MAX_NUM_INPUT_DEV_INTERNAL_GPIO);
                rc = -1;
            }
            else
            {
                XML_GET_INT_ATTR(pInputDev->devConfig.gpio[idx], pChild, "config", 1, int32);
            }
        }
        else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "sensor")))
        {
            uint32 idx = 0;
            XML_GET_INT_ATTR(idx, pChild, "link", 0, int);

            if (idx >= MAX_NUM_INPUT_DEV_SENSORS)
            {
                CAM_MSG(ERROR, "Link exceeds max %d", MAX_NUM_INPUT_DEV_SENSORS);
                rc = -1;
            }
            else
            {
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].type, pChild, "type", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].serAlias, pChild, "serAlias", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].snsrAlias, pChild, "snsrAlias", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].snsrModeId, pChild, "snsrModeId", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].colorSpace, pChild, "colorSpace", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].fsyncMode, pChild, "fsyncMode", 1, uint32);
                XML_GET_INT_ATTR(pInputDev->devConfig.sensors[idx].fsyncFreq, pChild, "fsyncFreq", 1, uint32);
            }
        }

        pChild = pChild->next;
    }

    return rc;
}

static int CameraConfigXMLParseInputDevs(xmlNodePtr pParent, CameraBoardType *boardInfo)
{
    int rc = 0;
    xmlNodePtr pCur = pParent->xmlChildrenNode;
    uint32 devIdx = 0;

    while(pCur != NULL && !rc)
    {
        if(!(xmlStrcmp(pCur->name, (const xmlChar *) "inputDev")))
        {
            xmlNodePtr pChild = pCur->xmlChildrenNode;
            CameraSensorBoardType* pInputDev = NULL;
            uint32 devId = devIdx;
            uint32 numIntr = 0;
            uint32 socId;
            uint32 csiIdx  = 0;

            socId = CameraPlatform_GetMultiSocId();
            if (devIdx >= MAX_NUM_CAMERA_INPUT_DEVS)
            {
                CAM_MSG(ERROR, "Number of inputDev exceeds max %d", MAX_NUM_CAMERA_INPUT_DEVS);
                rc = -1;
                break;
            }

            pInputDev = &boardInfo->camera[devIdx];

            XML_GET_INT_ATTR(devId, pCur, "devId", 1, uint32);
            pInputDev->devId = (devId | CAMERADEVICEID_INPUT_0);

            XML_GET_INT_ATTR(pInputDev->detectThrdId, pCur, "detectThrdId", 1, uint32);

            while (pChild != NULL && !rc)
            {
                if(!(xmlStrcmp(pChild->name, (const xmlChar *) "properties")))
                {
                    rc = CameraConfigParseInputDevProperties(pChild, pInputDev, socId);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "driverInfo")))
                {
                    pInputDev->driverInfo.deviceCategory = CameraConfigXMLParseDeviceCategory(pChild);
                    XML_GET_STRING_ATTR(pInputDev->driverInfo.strDeviceLibraryName, pChild, "libName", 1);
                    XML_GET_STRING_ATTR(pInputDev->driverInfo.strCameraDeviceOpenFn, pChild, "openFcn", 1);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "i2cPort")))
                {
                    uint32 idx = 0;
                    XML_GET_INT_ATTR(idx, pChild, "soc_id", 0, uint32);


                    if (idx >= SOC_ID_MAX)
                    {
                        CAM_MSG(ERROR, "soc_id exceeds max %d", SOC_ID_MAX);
                        rc = -1;
                    }
                    else
                    {
                        pInputDev->i2cPort[idx].i2ctype = CameraConfigXMLParseI2cType(pChild);

                        XML_GET_INT_ATTR(pInputDev->i2cPort[idx].device_id, pChild, "device_id", 1, int);
                        XML_GET_INT_ATTR(pInputDev->i2cPort[idx].port_id, pChild, "port_id", 1, int);
                    }
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "csiInfo")))
                {
                    if (csiIdx >= CSIPHY_CORE_MAX)
                    {
                        CAM_MSG(ERROR, "Number of CSI exceeds max %d", CSIPHY_CORE_MAX);
                        rc = -1;
                        break;
                    }

                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].csiId, pChild, "csiId", 0, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].isSecure, pChild, "isSecure", 1, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].numLanes, pChild, "numLanes", 0, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].laneAssign, pChild, "laneAssign", 0, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].numIfeMap, pChild, "numIfeMap", 1, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].ifeMap, pChild, "ifeMap", 1, int);
                    XML_GET_INT_ATTR(pInputDev->csiInfo[csiIdx].forceHSmode, pChild, "forceHSmode", 1, int);
                    csiIdx++;
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "gpio")))
                {
                    CameraSensorGPIO_SignalType gpioType =  CAMERA_GPIO_INVALID;

                    gpioType = CameraConfigXMLParseGpioSignalType(pChild);

                    if (gpioType >= CAMERA_GPIO_MAX)
                    {
                        CAM_MSG(ERROR, "Gpio id exceeds max %d", CAMERA_GPIO_MAX);
                        rc = -1;
                        break;
                    }

                    XML_GET_INT_ATTR(pInputDev->gpioConfig[gpioType].num, pChild, "num", 1, uint32);
                    XML_GET_INT_ATTR(pInputDev->gpioConfig[gpioType].config, pChild, "config", 1, uint32);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "intr")))
                {
                    uint32 gpio_cfg = 0;

                    if (numIntr >= MAX_CAMERA_DEV_INTR_PINS)
                    {
                        CAM_MSG(ERROR, "Number of interrupts exceeds max %d", MAX_CAMERA_DEV_INTR_PINS);
                        rc = -1;
                        break;
                    }

                    pInputDev->intr[numIntr].gpio_id = CameraConfigXMLParseGpioSignalType(pChild);

                    XML_GET_INT_ATTR(pInputDev->intr[numIntr].pin_id, pChild, "pin_id", 1, uint32);
                    pInputDev->intr[numIntr].intr_type = CameraConfigXMLParseIntrCfgType(pChild);
                    pInputDev->intr[numIntr].trigger = CameraConfigXMLParseGpioEvtType(pChild);

                    XML_GET_INT_ATTR(gpio_cfg, pChild, "gpio_cfg", 1, uint32);
                    pInputDev->intr[numIntr].gpio_cfg.dir = gpio_cfg & 0xF;
                    pInputDev->intr[numIntr].gpio_cfg.pull_dir = gpio_cfg & 0xF0;
                    pInputDev->intr[numIntr].gpio_cfg.strength = gpio_cfg & 0xF00;
                    pInputDev->intr[numIntr].gpio_cfg.fcn = gpio_cfg & 0x000F0000;

                    numIntr++;
                }

                pChild = pChild->next;
            }

            devIdx++;
        }
        else if(!xmlNodeIsText(pCur))
        {
            CAM_MSG(ERROR, "CameraConfig %s inside %s", pCur->name, pParent->name);
            rc = -1;
        }
        pCur = pCur->next;
    }
    return rc;
}

static int CameraConfigXMLParseInputMapping(xmlNodePtr pParent, CameraChannelInfoType *pChannelInfo, uint32 *pNumChannel)
{
    int rc = 0;
    xmlNodePtr pCur = pParent->xmlChildrenNode;
    uint32 idx = 0;

    while (pCur != NULL && !rc)
    {
        if (!(xmlStrcmp(pCur->name, (const xmlChar *) "inputMap")))
        {
            xmlNodePtr pChild = pCur->xmlChildrenNode;
            uint32 srcIdx = 0;
            uint32 ispInstanceIdx = 0;

            if (idx >= MAX_NUM_CAMERA_CHANNELS)
            {
                CAM_MSG(ERROR, "Number of mappings exceeds max %d", MAX_NUM_CAMERA_CHANNELS);
                rc = -1;
                break;
            }

            XML_GET_INT_ATTR(pChannelInfo[idx].aisInputId, pCur, "qcarcamId", 1, uint32);

            pChannelInfo[idx].opMode = CameraConfigXMLParseOpMode(pCur);

            while (pChild != NULL && !rc)
            {
                if(!(xmlStrcmp(pChild->name, (const xmlChar *) "inputSrc")))
                {
                    uint32 devId = 0;

                    if (srcIdx >= MAX_CHANNEL_INPUT_SRCS)
                    {
                        CAM_MSG(ERROR, "Number of inputSrc for inputMap[%d] exceeds max %d", idx, MAX_NUM_CAMERA_CHANNELS);
                        rc = -1;
                        break;
                    }
                    else
                    {
                        XML_GET_INT_ATTR(devId, pChild, "devId", 1, uint32);
                        pChannelInfo[idx].inputSrc[srcIdx].devId = (devId | CAMERADEVICEID_INPUT_0);

                        XML_GET_INT_ATTR(pChannelInfo[idx].inputSrc[srcIdx].srcId, pChild, "srcId", 1, uint32);

                        srcIdx++;
                    }
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "ISPInstance")))
                {
                    if (ispInstanceIdx >= MAX_CHANNEL_ISP_INSTANCES)
                    {
                        CAM_MSG(ERROR, "Number of ispInstance for inputMap[%d] exceeds max %d", idx, MAX_CHANNEL_ISP_INSTANCES);
                        rc = -1;
                        break;
                    }
                    else
                    {
                        XML_GET_INT_ATTR(pChannelInfo[idx].ispInstance[ispInstanceIdx].id, pChild, "id", 0, uint32);

                        XML_GET_INT_ATTR(pChannelInfo[idx].ispInstance[ispInstanceIdx].cameraId, pChild, "cameraId", 0, uint32);

                        pChannelInfo[idx].ispInstance[ispInstanceIdx].useCase = CameraConfigXMLParseIspUseCase(pChild);

                        ispInstanceIdx++;
                    }
                }

                pChild = pChild->next;
            }

            pChannelInfo[idx].numIspInstances = ispInstanceIdx;

            //increment input mapping idx
            idx++;
        }
        pCur = pCur->next;
    }

    *pNumChannel = idx;

    return rc;
}

static int CameraConfigXMLParseErrorMatchingFunctions(xmlNodePtr pParent, CameraBoardType *boardInfo)
{
    int rc = 0;

    while(pParent != NULL && !rc)
    {
        if(!(xmlStrcmp(pParent->name, (const xmlChar *) "inputFatalError")))
        {
            xmlNodePtr pChild = pParent->xmlChildrenNode;
            rc = -1;

            if (pChild != NULL)
            {
                rc = 0;
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_INPUT_FATAL_ERROR].type, pChild, "type", 1, CameraConfigMatchType);
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_INPUT_FATAL_ERROR].severity, pChild, "severity", 1, uint32);
            }
        }
        else if(!(xmlStrcmp(pParent->name, (const xmlChar *) "csidFatalError")))
        {
            xmlNodePtr pChild = pParent->xmlChildrenNode;
            rc = -1;

            if (pChild != NULL)
            {
                rc = 0;
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_CSID_FATAL_ERROR].type, pChild, "type", 1, CameraConfigMatchType);
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_CSID_FATAL_ERROR].severity, pChild, "severity", 1, uint32);
            }
        }
        else if(!(xmlStrcmp(pParent->name, (const xmlChar *) "ifeOutputError")))
        {
            xmlNodePtr pChild = pParent->xmlChildrenNode;
            rc = -1;

            if (pChild != NULL)
            {
                rc = 0;
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_IFE_OUTPUT_ERROR].type, pChild, "type", 1, CameraConfigMatchType);
                XML_GET_INT_ATTR(boardInfo->engineSetting.customMatchFunctions[CAMERA_CONFIG_EVENT_IFE_OUTPUT_ERROR].severity, pChild, "severity", 1, uint32);
            }
        }

        pParent = pParent->next;
    }

    return rc;
}

static void CameraConfigDumpBoardInfo(const CameraBoardType *pBoard)
{
    int i;

    CAM_MSG(MEDIUM, "CameraConfig Parsed XML board %s", pBoard->boardName);

    for (i = 0; i < MAX_NUM_CAMERA_INPUT_DEVS; i++)
    {
        if (pBoard->camera[i].devId == 0)
            break;

        CAM_MSG(MEDIUM, "%d - 0x%x %s",
                i,
                pBoard->camera[i].devId,
                pBoard->camera[i].driverInfo.strDeviceLibraryName);

        CAM_MSG(MEDIUM, "== csiInfo %d %d ifeMap[%d 0x%x]",
                pBoard->camera[i].csiInfo[0].csiId,
                pBoard->camera[i].csiInfo[0].laneAssign,
                pBoard->camera[i].csiInfo[0].numIfeMap,
                pBoard->camera[i].csiInfo[0].ifeMap);
    }
}

static int CameraConfigXMLParseFile(CameraBoardType *pBoard, CameraChannelInfoType *pChannel, uint32 *pNumChannels)
{
    int rc = 0;

    xmlDocPtr pXmlDoc = NULL;
    xmlNodePtr pCur = NULL;
    const char* filename = CAMERA_CONFIG_XML_FILE;
    uint32 boardVersion = 0x0;

    CAM_MSG(HIGH, "CameraConfig xml file name:%s", filename);
    if(access(filename, F_OK))
    {
        CAM_MSG(HIGH, "CameraConfig file not available. Will use defaults");
        rc = -1;
    }

    if (!rc)
    {
        pXmlDoc = xmlParseFile(filename);
        if (pXmlDoc == NULL)
        {
            CAM_MSG(ERROR, "CameraConfig file not parsed successfully");
            rc = -1;
        }
    }

    if (!rc)
    {
        pCur = xmlDocGetRootElement(pXmlDoc);
        if (pCur == NULL)
        {
            CAM_MSG(ERROR, "Empty CameraConfig file");
            xmlFreeDoc(pXmlDoc);
            rc = -1;
        }
    }

    if (!rc)
    {
        if (xmlStrcmp(pCur->name, (const xmlChar *) "CameraConfig"))
        {
            CAM_MSG(ERROR, "Wrong CameraConfig file format, root node != CameraConfig");
            xmlFreeDoc(pXmlDoc);
            rc = -1;
        }
    }

    if (!rc)
    {
        XML_GET_INT_ATTR(boardVersion, pCur, "version", 0, uint32);
        if (CAMERA_BOARD_LIBRARY_VERSION != boardVersion)
        {
            CAM_MSG(ERROR, "Unsupported CameraConfig version 0x%x (require 0x%x)", boardVersion, CAMERA_BOARD_LIBRARY_VERSION);
            xmlFreeDoc(pXmlDoc);
            rc = -1;
        }
        else
        {
            pCur = pCur->xmlChildrenNode;
        }
    }

    //Parse the file
    while(pCur != NULL && !rc)
    {
        if (!(xmlStrcmp(pCur->name, (const xmlChar *) "board")))
        {
            CameraConfigCustomMatchFunc customMatchFunctionsDefault[CAMERA_CONFIG_EVENT_NUM] =
            {
                {.type = CAMERA_CONFIG_MATCH_INPUT_DEVICE, .severity = 0},
                {.type = CAMERA_CONFIG_MATCH_IFE_DEVICE, .severity = 0},
                {.type = CAMERA_CONFIG_MATCH_IFE_DEVICE, .severity = 0}
            };

            XML_GET_STRING_ATTR(pBoard->boardName, pCur, "name", 1);
            pBoard->boardType = CameraConfigXMLParseBoardType(pCur);
            XML_GET_INT_ATTR(pBoard->multiSocEnable, pCur, "multiSocEnable", 1, uint32);

            pBoard->engineSetting.numBufMax = MAX_NUM_CAMERA_BUFFERS_DEFAULT;
            pBoard->engineSetting.powerManagementPolicy = CAMERA_PM_POLICY_LPM_EVENT_ALL;
            pBoard->engineSetting.latencyMeasurementMode = CAMERA_LM_MODE_DISABLE;
            pBoard->engineSetting.recoveryRestartDelay = RECOVERY_DEFAULT_RESTART_DELAY;
            pBoard->engineSetting.recoveryTimeoutAfterUsrCtxtRestart = RECOVERY_DEFAULT_TIMEOUT_AFTER_RESTART;
            pBoard->engineSetting.recoveryRetryDelay = RECOVERY_DEFAULT_RETRY_DELAY;
            pBoard->engineSetting.recoveryMaxNumAttempts = RECOVERY_DEFAULT_MAX_ATTEMPTS;
            pBoard->engineSetting.multiIfeInitFrameDrop = 1;
            memcpy(pBoard->engineSetting.customMatchFunctions,
                    customMatchFunctionsDefault,
                    sizeof(customMatchFunctionsDefault));

            xmlNodePtr pChild = pCur->xmlChildrenNode;
            while(pChild != NULL && !rc)
            {
                if(!(xmlStrcmp(pChild->name, (const xmlChar *) "i2cDevs")))
                {
                    rc = CameraConfigXMLParseI2CDevs(pChild, pBoard);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "inputDevs")))
                {
                    rc = CameraConfigXMLParseInputDevs(pChild, pBoard);
                }
                else if(!(xmlStrcmp(pChild->name, (const xmlChar *) "engineSettings")))
                {
                    XML_GET_INT_ATTR(pBoard->engineSetting.numBufMax, pChild, "numBufMax", 1, uint32);
                    pBoard->engineSetting.powerManagementPolicy = CameraConfigXMLParsePowerManagementPolicyType(pChild);
                    pBoard->engineSetting.latencyMeasurementMode = CameraConfigXMLParseLatencyMeasurementModeType(pChild);
                    XML_GET_INT_ATTR(pBoard->engineSetting.recoveryRestartDelay, pChild, "recoveryRestartDelay", 1, uint32);
                    XML_GET_INT_ATTR(pBoard->engineSetting.recoveryTimeoutAfterUsrCtxtRestart, pChild, "recoveryTimeoutAfterUsrCtxtRestart", 1, uint32);
                    XML_GET_INT_ATTR(pBoard->engineSetting.recoveryRetryDelay, pChild, "recoveryRetryDelay", 1, uint32);
                    XML_GET_INT_ATTR(pBoard->engineSetting.recoveryMaxNumAttempts, pChild, "recoveryMaxNumAttempts", 1, uint32);
                    XML_GET_INT_ATTR(pBoard->engineSetting.multiIfeInitFrameDrop, pChild, "multiIfeInitFrameDrop", 1, uint32);
                    rc = CameraConfigXMLParseErrorMatchingFunctions(pChild, pBoard);
                }
                pChild = pChild->next;
            }
        }
        else if(!(xmlStrcmp(pCur->name, (const xmlChar *) "inputMapping")))
        {
            rc = CameraConfigXMLParseInputMapping(pCur, pChannel, pNumChannels);
        }

        pCur = pCur->next;
    }

    xmlFreeDoc(pXmlDoc);

    if (!rc)
    {
        CameraConfigDumpBoardInfo(pBoard);
    }

    return rc;
}

#endif /*CAMERA_CONFIG_ENABLE_XML_PARSER*/


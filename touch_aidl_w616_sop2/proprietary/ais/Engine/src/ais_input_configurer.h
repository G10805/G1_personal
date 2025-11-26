#ifndef _AIS_INPUT_CONFIGURER_H_
#define _AIS_INPUT_CONFIGURER_H_

/*!
 * Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_configurer.h"
#include "CameraSensorDriver.h"
#include "CameraPlatform.h"

//////////////////////////////////////////////////////////////////////////////////
/// MACRO DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////
// max number of CSI input devices
#define MAX_CAMERA_INPUT_DEVICES 5

//////////////////////////////////////////////////////////////////////////////////
/// TYPE DEFINITIONS
//////////////////////////////////////////////////////////////////////////////////
 /**
  * input get and set parameters
  */
typedef enum
{
    AIS_INPUT_CTRL_INPUT_INTERF, /**< get the input interface */
    AIS_INPUT_CTRL_STATUS,     /**< get the input status */
    AIS_INPUT_CTRL_EVENT_NOTIFICATION, /**< set/get input event notification */
    AIS_INPUT_CTRL_CHROMATIX_DATA,
    AIS_INPUT_CTRL_MODE,
    AIS_INPUT_CTRL_RESOLUTION,  /**< set input resolution */
    AIS_INPUT_CTRL_EXPOSURE_CONFIG, /**< set exposure params */
    AIS_INPUT_CTRL_HDR_EXPOSURE_CONFIG,
    AIS_INPUT_CTRL_SATURATION_CONFIG, /**< set saturation params */
    AIS_INPUT_CTRL_HUE_CONFIG, /**< set hue params */
    AIS_INPUT_CTRL_FIELD_TYPE, /**< get field type */
    AIS_INPUT_CTRL_GAMMA_CONFIG, /**< set gamma params */
    AIS_INPUT_CTRL_VENDOR_PARAM, /**< set vendor param */
    AIS_INPUT_CTRL_BRIGHTNESS, /**< set brightness */
    AIS_INPUT_CTRL_CONTRAST, /**< set contrast */
    AIS_INPUT_CTRL_MIRROR_H, /**< set horizontal mirroring */
    AIS_INPUT_CTRL_MIRROR_V, /**< set vertical mirroring */
    AIS_INPUT_CTRL_CCI_SYNC,
    AIS_INPUT_CTRL_COLOR_SPACE,  /**< set color space */
    AIS_INPUT_CTRL_MAX
}ais_input_ctrl_t;

/* Maps channel to device index */
typedef struct
{
    CameraChannelInfoType sInfo;

    uint32 devIdx[MAX_CHANNEL_INPUT_SRCS];  /*index into input_devices[]*/
    uint32 numStreams;
    uint32 streamsAvailable;
    boolean isAvailable;
}InputMappingType;

typedef enum
{
    AIS_INPUT_STATE_UNAVAILABLE = 0,
    AIS_INPUT_STATE_DETECTED,
    AIS_INPUT_STATE_OFF,
    AIS_INPUT_STATE_ON,
    AIS_INPUT_STATE_STREAMING,
    AIS_INPUT_STATE_SUSPEND, /**< power suspend, not steam paused */
    AIS_INPUT_STATE_RESUME,  /**< power resume, not steam resumed */
}InputStateType;

typedef enum
{
    INPUT_EVENT_DEFER_DETECT = 0
}InputEventId;

typedef struct
{
    InputEventId eventId;
    int devIdx;
}InputEventMsgType;

typedef struct
{
    CameraSignal   signal;
    CameraThread   threadId;
    CameraQueue    detectQ;
}InputDetectHandlerType;

/**
 * Input interface description
 */
typedef struct
{
    qcarcam_input_desc_t inputId;
    uint32 streamIdx;

    struct {
        uint32 devId;   /**< input device idx */
        uint32 srcId;   /**< input device src */

        uint32 csiphy;  /**< CSIPHY core */
        uint32 cid;     /**< CSI channel id */

        CameraInterlacedType interlaced;
    }stream;
}AisInputInterfaceType;

typedef struct
{
    CameraDeviceIDType devId;

    CameraDeviceHandle hDevice;
    CameraDeviceInfoType sDeviceInfo;

    Camera_Sensor_ChannelsInfoType    channelsInfo;
    Camera_Sensor_SubchannelsInfoType subchannelsInfo;

    CameraCsiInfo csiInfo[CSIPHY_CORE_MAX];
    boolean isAvailable;

    uint8 refcnt; //Track users of input
    InputStateType state;
    uint32 streamRefCnt[MAX_IMAGE_SOURCES];
    uint32 src_id_enable_mask;
    CameraTimer pTimer;
    CameraMutex m_mutex;
    boolean delay_suspend_flag;
}InputDeviceType;

class AisInputConfigurer : public AisEngineConfigurer
{
public:
    static AisInputConfigurer* CreateInstance();
    static AisInputConfigurer* GetInstance();
    static void       DestroyInstance();

    virtual CameraResult PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId);
    virtual CameraResult PowerResume(AisUsrCtxt* pUsrCtxt, boolean  bGranular, CameraPowerEventType powerEventId);
    virtual CameraResult Config(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult GlobalConfig(AisGlobalCtxt* pGlobalCtxt);
    virtual CameraResult Start(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Stop(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Resume(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Pause(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param);
    virtual CameraResult GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *param);

    CameraResult DetectAll();
    CameraResult DetectInput(uint32 devIdx);

    CameraResult ProcessFrameData(AisUsrCtxt* pUsrCtxt, const AisBuffer* pBuffer, qcarcam_frame_info_v2_t* pFrameInfo);

    CameraResult QueryInputs(qcarcam_input_t* pQcarcamInputs, unsigned int nSize, unsigned int* pRetSize);
    CameraResult QueryInputsV2(qcarcam_input_v2_t* pQcarcamInputs, unsigned int nSize, unsigned int* pRetSize);
    CameraResult IsInputAvailable(uint32 input_id);

    uint32 GetInputNumStreams(qcarcam_input_desc_t inputId);
    CameraResult GetInterface(AisInputInterfaceType* interf);
    void FillInputDeviceDiagInfo(QCarCamDiagInputDevInfo* pInputDeviceInfo);
    CameraResult SensorRecovery(uint32 devId, uint32 severity);
    CameraResult GlobalStop(void);

private:
    static AisInputConfigurer* m_pInputConfigurerInstance;

    AisInputConfigurer()
    {
        mDeviceManagerContext = CameraDeviceManager::GetInstance();

        memset(m_InputDevices, 0x0, sizeof(m_InputDevices));
        m_nInputDevices = 0;

        memset(m_InputMappingTable, 0x0, sizeof(m_InputMappingTable));
        m_nInputMapping = 0;

        memset(m_detectHandler, 0x0, sizeof(m_detectHandler));

        m_detectHandlerIsExit = FALSE;
        m_detectInProgress = 0;
        m_detect_mutex = NULL;
        m_isMultiSoc = FALSE;
        m_detectDone = NULL;
    }

    ~AisInputConfigurer()
    {
    }

    CameraResult Init(void);
    CameraResult Deinit(void);

    CameraResult GetModeInfo(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream, uint32 mode, AisInputModeInfoType* pModeInfo);
    uint32 GetDeviceId(uint32 device_id);
    CameraResult ValidateInputId(AisUsrCtxt* hndl);

    static CameraResult InputDeviceCallback(void* pClientData,
            uint32 uidEvent, int nEventDataLen, void* pEventData);

    CameraResult FillStreamCSIParams(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult FillStreamInputModeInfo(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);

    void FillQCarCamInputResInfo(qcarcam_input_t* pQcarcamInput, InputMappingType* pInputMap);
    void DumpQCarCamQueryInputs(qcarcam_input_t* pQcarcamInputs, unsigned int nSize);

    void FillQCarCamInputResInfoV2(qcarcam_input_v2_t* pQcarcamInput, InputMappingType* pInputMap);
    void DumpQCarCamQueryInputsV2(qcarcam_input_v2_t* pQcarcamInputs, unsigned int nSize);

    CameraResult StartStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);
    CameraResult StopStream(AisUsrCtxt* pUsrCtxt, AisUsrCtxtStreamType* pStream);

    static void SuspendTimerHandle(void* pUsrData);

    static int DetectHandlerThread(void *pArg);
    int ProcessDetectEvent(InputDetectHandlerType* pDetectHndlr);
    CameraResult QueueDetectEvent(InputDetectHandlerType* pDetectHndlr, InputEventMsgType* pMsg);


    CameraDeviceManager* mDeviceManagerContext;
    InputDeviceType m_InputDevices[MAX_CAMERA_INPUT_DEVICES];
    uint32 m_nInputDevices;

    InputMappingType m_InputMappingTable[MAX_CAMERA_INPUT_CHANNELS];
    uint32 m_nInputMapping;

    InputDetectHandlerType m_detectHandler[MAX_DETECT_THREAD_IDX];
    volatile boolean m_detectHandlerIsExit;
    uint32 m_detectInProgress;
    CameraMutex m_detect_mutex;
    boolean m_isMultiSoc; /**< Multi SOC Envionment */
    CameraSignal m_detectDone;
};


#endif /* _AIS_INPUT_CONFIGURER_H_ */

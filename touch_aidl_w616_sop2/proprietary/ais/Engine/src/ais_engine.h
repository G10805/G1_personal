#ifndef _AIS_ENGINE_H_
#define _AIS_ENGINE_H_

/*!
 * Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <list>
#include <map>

#include "ais.h"
#include "ais_i.h"
#include "CameraDeviceManager.h"
#include "ais_proc_chain.h"
#include "ais_diag_mgr.h"

#define MAX_CAMERA_INPUT_CHANNELS MAX_NUM_CAMERA_CHANNELS

#define NUM_EVENT_HNDLR_POOL 4

#define EVENT_QUEUE_MAX_SIZE 256

#define AIS_CTXT_MAGIC 0xA150C0DE

/**
 * By default set notification for master change
 */
#define AIS_NOTIFICATION_EVENT_DEFAULT (1 << QCARCAM_PARAM_MASTER)


/**
 * Timeout for delayed suspend to apply suspend. If in this time
 * we get a new client, we will abort the suspend. This is useful on
 * first initialization for early clients that are ready as soon as
 * engine is ready.
 */
#define AIS_ENGINE_DELAYED_SUSPEND_TIMEOUT 1000

/**
 * Convert errno to CameraResult
 */
static inline int ERRNO_TO_RESULT(const int err)
{
    int result = CAMERA_EFAILED;
    switch (err)
    {
        case EOK:
            result = CAMERA_SUCCESS;
            break;
        case EPERM:
            result = CAMERA_ENOTOWNER;
            break;
        case ENOENT:
            result = CAMERA_EINVALIDITEM;
            break;
        case ESRCH:
            result = CAMERA_ERESOURCENOTFOUND;
            break;
        case EINTR:
            result = CAMERA_EINTERRUPTED;
            break;
        case EIO:
            result = CAMERA_EFAILED;
            break;
        case ENXIO:
            result = CAMERA_ENOSUCH;
            break;
        case E2BIG:
            result = CAMERA_EOUTOFBOUND;
            break;
        case ENOEXEC:
            result = CAMERA_EINVALIDFORMAT;
            break;
        case EBADF:
            result = CAMERA_EINVALIDITEM;
            break;
        case ECHILD:
            result = CAMERA_EBADTASK;
            break;
        case EAGAIN:
            result = CAMERA_EWOULDBLOCK;
            break;
        case ENOMEM:
            result = CAMERA_ENOMEMORY;
            break;
        case EACCES:
            result = CAMERA_EPRIVLEVEL;
            break;
        case EFAULT:
            result = CAMERA_EMEMPTR;
            break;
        case EBUSY:
            result = CAMERA_EITEMBUSY;
            break;
        case EEXIST:
            result = CAMERA_EALREADY;
            break;
        case ENODEV:
            result = CAMERA_ERESOURCENOTFOUND;
            break;
        case EINVAL:
            result = CAMERA_EBADPARM;
            break;
        case ENFILE:
            result = CAMERA_EOUTOFBOUND;
            break;
        case EMFILE:
            result = CAMERA_EOUTOFHANDLES;
            break;
        case ETXTBSY:
            result = CAMERA_EITEMBUSY;
            break;
        case EFBIG:
        case ENOSPC:
            result = CAMERA_EOUTOFBOUND;
            break;
        case EROFS:
            result = CAMERA_EREADONLY;
            break;
        case EMLINK:
            result = CAMERA_EOUTOFHANDLES;
            break;
        case ERANGE:
            result = CAMERA_EOUTOFBOUND;
            break;
        case ENOTSUP:
            result = CAMERA_EUNSUPPORTED;
            break;
        case ETIMEDOUT:
            result = CAMERA_EEXPIRED;
            break;
    }

    return result;
}

/**
 * AIS user context states
 */
typedef enum
{
    AIS_USR_STATE_UNINITIALIZED = 0,
    AIS_USR_STATE_OPENED,
    AIS_USR_STATE_RESERVED,
    AIS_USR_STATE_STREAMING,
    AIS_USR_STATE_PAUSED,
    AIS_USR_STATE_STOP_PENDING,
    AIS_USR_STATE_RECOVERY_START,
    AIS_USR_STATE_RECOVERY,
    AIS_USR_STATE_ERROR,
    AIS_USR_STATE_PAUSE_PENDING,
    AIS_USR_STATE_MAX
}AisUsrCtxtStateType;


/** HW pipeline resources */
typedef struct
{
    uint32 cid;     /**< cid on CSI */
    uint32 csiphy;  /**< CSI phy core */
    uint32 csid;    /**< CSID core */

    AisIfeStreamType ifeStream;
}AisUsrCtxtResourcesType;

typedef struct
{
    qcarcam_gamma_type_t config_type;  /// gamma configure mode
    float f_value;
    unsigned int length;
    unsigned int table[QCARCAM_MAX_GAMMA_TABLE];
}AisGammaCfgType;

/** User context private settings */
typedef struct
{
    uint32 bitmask;                                          /**< bitmask of user set parameters */

    uint32 width;                                            /**< width of output*/
    uint32 height;                                           /**< height of output*/
    uint32 sensormode;                                       /**< sensormode*/
    qcarcam_color_fmt_t fmt;                                 /**< output color format */
    qcarcam_exposure_config_t exposure_params;               /**< exposure */
    qcarcam_hdr_exposure_config_t hdr_exposure;              /**< HDR exposure */
    qcarcam_frame_rate_t frame_rate_config;                  /**< frame rate */
    qcarcam_batch_mode_config_t  batch_config;              /**< batch frams */
    float saturation_param;                                  /**< saturation param */
    float hue_param;                                         /**< hue param */
    AisGammaCfgType gamma_params;                            /**< gamma param */
    float brightness_param;                                  /**< brightness param */
    float contrast_param;                                    /**< contrast param */
    uint32 mirror_h_param;                                   /**< horizontal mirror param */
    uint32 mirror_v_param;                                   /**< vertical mirror param */
    boolean recovery;                                        /**< self-recovery param */

    void* pPrivData; /**< QCARCAM_PARAM_PRIVATE_DATA */

    uint32 n_latency_max; /**< max latency in number of buffers; this is the max number of buffers in buffer_done_q
                                after which we will proceed to remove stale ones */
    uint32 n_latency_reduce_rate; /**< the number of stale buffers we remove once n_latency_max is exceeded */

    qcarcam_param_isp_ctrls_t isp_ctrls;                 /**< ISP sensor settings */
    uint32 init_frame_drop;
}AisUsrCtxtSettingsType;

typedef struct
{
    uint32 devId;   /**< input device id */
    uint32 srcId;   /**< input source id */
    uint32 csiIdx;  /**< CSI index of input device */


    AisInputCsiParamsType csiInfo;
    AisInputModeInfoType  inputModeInfo;
}AisUsrCtxtInputCfgType;

typedef struct
{
    AisStreamType           type;
    AisUsrCtxtResourcesType resources; /**< resources that are reserved for user */
    AisUsrCtxtInputCfgType  inputCfg;  /**< input configuration */
}AisUsrCtxtStreamType;

/**
 * field related info
 */
typedef struct
{
    boolean  valid;         /**< specifies if this is obsolete */
    uint32   frameId;       /**< specifies which frame it belongs to */
    uint64   sofTimestamp;  /**< specifies the sof of current field timestamp: ns */
    uint64   timestamp;
    qcarcam_field_t  fieldType;
}AisFieldInfoType;

/**
 * ISP instance config
 */
typedef struct
{
    uint32 cameraId;         /**< specifies isp camera id */
    qcarcam_isp_usecase_t useCase; /**< specifies isp use case */
}AisISPInstanceType;

/**
 *  Global Context for all input ids
 */
typedef struct
{
    qcarcam_input_desc_t m_inputId[MAX_CAMERA_INPUT_CHANNELS];
    AisUsrCtxtStreamType m_streams[MAX_CAMERA_INPUT_CHANNELS][AIS_USER_CTXT_MAX_STREAMS];
    uint32 m_numStreams[MAX_CAMERA_INPUT_CHANNELS];
}AisGlobalCtxt;

typedef std::map<qcarcam_input_desc_t, AisUsrCtxtList*> AisEvtListener;
typedef std::map<qcarcam_input_desc_t, AisUsrCtxt*> AisMasterHdlMap;

class AisEngine;

/**
 * AIS User Context
 */
class AisUsrCtxt
{
private:
    AisUsrCtxt(qcarcam_input_desc_t input_id) {
        m_magic = AIS_CTXT_MAGIC;
        m_mapIdx = 0;
        m_qcarcamHndl = 0;
        m_inputId = input_id;

        m_mutex = NULL;
        m_state = AIS_USR_STATE_UNINITIALIZED;

        m_isPendingStart = FALSE;
        m_ppjobInProgress = 0;

        memset(&m_usrSettings, 0x0, sizeof(m_usrSettings));
        m_usrSettings.n_latency_max = 1;
        m_usrSettings.n_latency_reduce_rate = 1;
        m_usrSettings.recovery = FALSE;
        m_usrSettings.batch_config.num_batch_frames = 1;
        m_usrSettings.batch_config.detect_first_phase_timer = 10;

        m_secureMode = 0;
        m_opMode = QCARCAM_OPMODE_MAX;

        for (int idx = 0; idx < AIS_USER_CTXT_MAX_ISP_INSTANCES; idx++)
        {
            m_ispInstance[idx].cameraId = AIS_ISP_CAMERA_ID_DEFAULT;
            m_ispInstance[idx].useCase = QCARCAM_ISP_USECASE_SHDR_BPS_IPE_AEC_AWB;
        }

        memset(&m_streams, 0x0, sizeof(m_streams));
        m_numStreams = 0;
        m_pProcChainDef = NULL;
        memset(m_pPProc, 0x0, sizeof(m_pPProc));

        m_numBufMax = 0;
        memset(&m_bufferList, 0x0, sizeof(m_bufferList));

        memset(m_fieldInfo, 0x0, sizeof(m_fieldInfo));

        m_eventCbFcn = NULL;
        m_eventMask = 0;
        m_isMaster = FALSE;
        m_isUsrSetMaster = FALSE;
        m_notificationEvtMask = AIS_NOTIFICATION_EVENT_DEFAULT;

        m_startTime = 0;
        m_prevTime = 0;
        m_numFrameDone = 0;
        m_numSof = 0;
        m_frameRate = 0;

        m_RecoveryTimeoutTimer = NULL;
        m_RetryRecoveryTimer = NULL;
        m_numRecoveryAttempts = 0;
        m_totalRecoveryAttempts = 0;
        m_numSuccessfulRecoveries = 0;
    }

    CameraResult Initialize(void);

    CameraMutex m_mutex;
    uint32 m_magic; /**< canary initialized to AIS_CTXT_MAGIC to indicate valid ctxt */

public:
    static AisUsrCtxt* CreateUsrCtxt(qcarcam_input_desc_t input_id);
    static void DestroyUsrCtxt(AisUsrCtxt* pUsrCtxt);

    void Lock(void);
    void Unlock(void);

    CameraResult IncRefCnt(void);
    void DecRefCnt(void);


    CameraResult Reserve(void);
    CameraResult Release(void);
    CameraResult Start(void);
    CameraResult Stop(void);
    CameraResult Pause(void);
    CameraResult Resume(void);

    CameraResult InitMutex(void);
    CameraResult DeinitMutex(void);

    CameraResult InitUserBufferLists(void);
    CameraResult InitBufferLists(const AisBuflistDefType* pBuflistDef);
    CameraResult FreeUserBufferLists(void);
    CameraResult FreeInternalBufferLists(void);
    CameraResult InitOperationMode(void);
    CameraResult StopRecoveryTimeoutTimer(void);
    void RegisterEvents(uint64 pEvtMask);
    void UnRegisterEvents(uint64 pEvtMask);
    void UpdateFrameRate();
    void ProcessRecoverySuccessEvent(void);
    CameraResult ProcessRecoveryFailEvent(void);

    void SendErrorCb(qcarcam_event_payload_t* pPayload);
    CameraResult AddSofFieldInfo(AisFieldInfoType* pFieldInfo);
    CameraResult GetFrameFieldInfo(AisEventFrameDoneType* pFrameDone);

    unsigned int m_mapIdx; /**< index into mapping table */

    qcarcam_hndl_t m_qcarcamHndl;    /**< opened qcarcam handle */
    qcarcam_input_desc_t m_inputId;  /**< input desc associated with handle */

    /** states */
    AisUsrCtxtStateType m_state; /**< state of user context */

    boolean m_isPendingStart; /**< tracks start state until we receive first frame */
    int     m_ppjobInProgress;

    AisUsrCtxtSettingsType  m_usrSettings; /**< user settings */

    uint32               m_secureMode;
    qcarcam_opmode_type  m_opMode;
    AisUsrCtxtStreamType m_streams[AIS_USER_CTXT_MAX_STREAMS];
    uint32 m_numStreams;
    const AisProcChainDefType* m_pProcChainDef;
    AisPProc* m_pPProc[AIS_PPROC_MAX_NUM_PROCS];

    AisISPInstanceType m_ispInstance[AIS_USER_CTXT_MAX_ISP_INSTANCES];

    uint32               m_numBufMax;
    AisBufferList* m_bufferList[AIS_BUFLIST_MAX];

    AisFieldInfoType m_fieldInfo[AIS_FIELD_BUFFER_SIZE];

    /** event management */
    qcarcam_event_cb_t m_eventCbFcn; /**< event callback function */
    uint32 m_eventMask; /**< event mask */
    boolean m_isMaster; /**< whether this client is master or not >**/
    boolean m_isUsrSetMaster; /**< whether this client is user setting or default setting >**/
    uint64 m_notificationEvtMask; /**< Mask of events that are subscribed for notification >**/

    /* diag */
    uint64 m_startTime;   /**current start timestamp*/
    uint64 m_prevTime;    /**previous timestamp to calculate framerate interval*/
    uint64 m_numFrameDone;
    uint64 m_numSof;
    uint64 m_frameRate;

    /* recovery */
    CameraTimer m_RecoveryTimeoutTimer; /**< timer which on timeout indicates that recovery has failed */
    CameraTimer m_RetryRecoveryTimer; /**< timer which triggers recovery after a certain amount of time */
    uint32 m_numRecoveryAttempts;
    uint32 m_totalRecoveryAttempts;
    uint32 m_numSuccessfulRecoveries;
};

/**
 * AIS context map state
 */
typedef enum
{
    AIS_USR_CTXT_MAP_UNUSED = 0, /**< mapping is unused */
    AIS_USR_CTXT_MAP_USED,       /**< mapping in use */
    AIS_USR_CTXT_MAP_DESTROY,    /**< mapping slated for destroy */
}AisUsrCtxtMapState;

/**
 * AIS context table mapping
 */
typedef struct
{
    volatile uint32 in_use; /**< flag to indicate in use */
    uintptr_t phndl; /**< qcarcam handle */
    unsigned int refcount; /**< refcounter to handle */
    void* usr_ctxt; /**< user context pointer */
}AisUsrCtxtMapType;

/**
 * AIS Event Queue Type
 */
typedef enum
{
    AIS_ENGINE_QUEUE_EVENTS,
    AIS_ENGINE_QUEUE_ERRORS,
    AIS_ENGINE_QUEUE_MAX,
}AisEngineQueueType;

typedef enum
{
    AIS_CONFIGURER_INPUT = 0,
    AIS_CONFIGURER_CSI,
    AIS_CONFIGURER_IFE,
    AIS_CONFIGURER_MAX
}AisConfigurerType;


typedef CameraResult (*AisUsrCtxtOperateFunc)(AisUsrCtxt* pUsrCtxt, void* pData);

/**
 * Data type for HandleAISErrorEvent
 */
typedef struct
{
    qcarcam_event_payload_t eventPayload;
    boolean bRecoveryEnabled;
}AisEventErrorHandlerType;

class AisEngineConfigurer;
class AisResourceManager;
class AisDiagManager;

typedef enum
{
    AIS_ENGINE_STATE_UNINITIALIZED,
    AIS_ENGINE_STATE_READY,
    AIS_ENGINE_STATE_DETECTION_PENDING,
    AIS_ENGINE_STATE_SUSPEND_PENDING,
    AIS_ENGINE_STATE_SUSPEND
}AisEngineState;

/**
 * AIS Engine Singleton Context
 */
class AisEngine : public AisEngineBase
{
private:
    AisEngine();
    ~AisEngine();
    static AisEngine* m_pEngineInstance;        // Singleton instance of AisEngine
public:
    static AisEngine* CreateInstance();
    static AisEngine* GetInstance();
    static void       DestroyInstance();
    static CameraResult CameraPowerEventCallback(CameraPowerEventType powerEventId, void* pUsrData);

    virtual CameraResult ais_initialize(qcarcam_init_t* p_init_params);
    virtual CameraResult ais_uninitialize(void);
    virtual CameraResult ais_query_inputs(qcarcam_input_t* p_inputs, unsigned int size, unsigned int* ret_size);
    virtual CameraResult ais_query_inputs_v2(qcarcam_input_v2_t* p_inputs, unsigned int size, unsigned int* ret_size);
    virtual qcarcam_hndl_t ais_open(qcarcam_input_desc_t desc);
    virtual CameraResult ais_close(qcarcam_hndl_t hndl);
    virtual CameraResult ais_g_param(qcarcam_hndl_t hndl, qcarcam_param_t param, qcarcam_param_value_t* p_value);
    virtual CameraResult ais_s_param(qcarcam_hndl_t hndl, qcarcam_param_t param, const qcarcam_param_value_t* p_value);
    virtual CameraResult ais_s_buffers(qcarcam_hndl_t hndl, qcarcam_buffers_t* p_buffers);
    virtual CameraResult ais_s_buffers_v2(qcarcam_hndl_t hndl, const qcarcam_bufferlist_t* p_bufferlist);
    virtual CameraResult ais_start(qcarcam_hndl_t hndl);
    virtual CameraResult ais_stop(qcarcam_hndl_t hndl);
    virtual CameraResult ais_pause(qcarcam_hndl_t hndl);
    virtual CameraResult ais_resume(qcarcam_hndl_t hndl);
    virtual CameraResult ais_get_frame(qcarcam_hndl_t hndl, qcarcam_frame_info_t* p_frame_info,
                unsigned long long int timeout, unsigned int flags);
    virtual CameraResult ais_get_frame_v2(qcarcam_hndl_t hndl, qcarcam_frame_info_v2_t* p_frame_info,
                unsigned long long int timeout, unsigned int flags);
    virtual CameraResult ais_release_frame(qcarcam_hndl_t hndl, unsigned int idx);
    virtual CameraResult ais_release_frame_v2(qcarcam_hndl_t hndl, unsigned int id, unsigned int idx);

    virtual CameraResult ais_query_diagnostics(void *p_ais_diag_info, unsigned int diag_size);

    CameraResult AisUsrctxtNotifyEventListener(qcarcam_input_desc_t input_id, qcarcam_param_t evt_param);

    CameraResult FillUsrCtxtDiagInfo(QCarCamDiagClientInfo* pDiagClientInfo);

    CameraResult QueueEvent(AisEventMsgType* msg);

    void UpdateUsrCtxtInfo();
    void UpdateDiagErrInfo(void* pErr, AisEventId errType, AisUsrCtxt* pUsrCtxt);

    AisUsrCtxt* AcquireUsrCtxt(unsigned int idx);
    void RelinquishUsrCtxt(unsigned int idx);

    CameraLatencyMeasurementModeType GetLatencyMeasurementMode();

private:
    CameraResult Initialize(qcarcam_init_t* pInitParams);
    void Deinitialize(void);

    /**
     * InjectionStart
     *
     * @brief Submits new postprocessing job for input buffer
     *
     * @param pUsrCtxt
     * @param idx  -  input buffer index ready to inject
     *
     * @return CameraResult
     */
    CameraResult InjectionStart(AisUsrCtxt* pUsrCtxt, uint32 idx);

    int ProcessEvent(AisEventMsgType* msg);
    static int EventHandler(void *arg);

    void ProcessRawFrameDone(AisEventMsgType* pMsg);
    void ProcessPProcJob(AisEventMsgType* pMsg);
    void ProcessPProcJobDone(AisEventMsgType* pMsg);
    void ProcessPProcJobFail(AisEventMsgType* pMsg);
    void ProcessCsidFatalError(AisEventMsgType* pMsg);
    void ProcessInputStatusEvent(AisEventMsgType* pMsg);
    void ProcessInputFatalError(AisEventMsgType* pMsg);
    void ProcessInputFrameFreeze(AisEventMsgType* pMsg);
    void ProcessSOF(AisEventMsgType* pMsg);
    void ProcessIfeOutputError(AisEventMsgType* pMsg);
    void ProcessApplyParam(AisEventMsgType* pMsg);
    void ProcessDeferInputDetect(AisEventMsgType* pMsg);
    void ProcessDelayedSuspend(AisEventMsgType* pMsg);
    void ProcessVendorEvent(AisEventMsgType* pMsg);
    void ProcessEventUserNotification(AisEventMsgType* pMsg);

    CameraResult ProcessPowerEvent(CameraPowerEventType powerEventId);

    CameraResult PowerSuspend(CameraPowerEventType powerEventId, boolean isPowerGranular);
    CameraResult PowerResume(CameraPowerEventType powerEventId, boolean isPowerGranular);

    CameraResult PowerResumeForStart(AisUsrCtxt* pUsrCtxt);
    CameraResult PowerSuspendForStop(AisUsrCtxt* pUsrCtxt);

    void Lock()
    {
        CameraLockMutex(this->m_mutex);
    }

    void Unlock()
    {
        CameraUnlockMutex(this->m_mutex);
    }

    boolean IsInputAvailable(qcarcam_input_desc_t desc);

    uintptr_t AssignNewHndl(AisUsrCtxt* pUsrCtxt);
    void ReleaseHndl(void* user_hndl);
    void PutUsrCtxt(AisUsrCtxt* pUsrCtxt);
    AisUsrCtxt* GetUsrCtxt(void* user_hndl);

    void TraverseUsrCtxt(AisUsrCtxtList* pList,
            AisUsrCtxtOperateFunc opFunc, void* pData);

    void AisEvtListnerAddUsrhdl(AisUsrCtxt* pUsrCtxt);
    void AisEvtListnerRemoveUsrhdl(AisUsrCtxt* pUsrCtxt);
    CameraResult AisEvtSetMaster(AisUsrCtxt* pUsrCtxt, boolean isUsrSet);
    CameraResult AisEvtReleaseMaster(AisUsrCtxt* pUsrCtxt);
    CameraResult MatchUserList(uint32 device, uint32 path, AisUsrCtxtList* pList);

    uint32 m_magic; /**< canary initialized to AIS_CTXT_MAGIC to indicate valid ctxt */

    AisEngineState m_state;

    uint32  m_clientCount;

    CameraMutex m_mutex;

    CameraSignal m_delaySuspend;

    /*event handlers*/
    CameraSignal m_eventHandlerSignal;
    CameraThread m_eventHandlerThreadId[NUM_EVENT_HNDLR_POOL];
    volatile boolean m_eventHandlerIsExit;

    CameraQueue m_eventQ[AIS_ENGINE_QUEUE_MAX];

    CameraMutex m_usrCtxtMapMutex;
    AisUsrCtxtMapType m_usrCtxtMap[AIS_MAX_USR_CONTEXTS];
    AisEvtListener m_eventListener;
    AisMasterHdlMap m_masterHdl;
    CameraMutex m_evtListnrMutex;

    CameraDeviceManager*  m_DeviceManager;
    AisResourceManager*   m_ResourceManager;
    AisDiagManager*       m_DiagManager;
    AisProcChainManager*  m_ProcChainManager;
public:
    const CameraEngineSettings* m_engineSettings;
    AisEngineConfigurer* m_Configurers[AIS_CONFIGURER_MAX];
    AisGlobalCtxt m_GlobalCtxt;

    boolean m_isPowerGranular;                             /**< power suspend/resume by bridge */
    boolean m_isMultiSoc;                                  /**< Multi SOC Envionment */
    CameraPowerManagerPolicyType m_PowerManagerPolicyType; /**< power suspend/resume policy type */
    CameraLatencyMeasurementModeType m_LatencyMeasurementMode; /**< camera latency measurement mode */
private:

};

#endif /* _AIS_ENGINE_H_ */

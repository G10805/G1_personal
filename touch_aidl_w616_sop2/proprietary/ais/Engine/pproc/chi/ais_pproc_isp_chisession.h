/*!
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AIS_PPROC_ISP_CHISESSION_H_
#define _AIS_PPROC_ISP_CHISESSION_H_


#include "ais_i.h"
#include "ais_engine.h"

#include <queue>
#include <map>

#include "CameraResult.h"
#include "CameraOSServices.h"
#include "chi.h"
#include "chiiqmodulesettings.h"
#include "ais_proc_chain.h"

// Constants
//
static const int maxRequests = 10;

// Forward declarations
//
class ChiPipeline;
class ChiSession;

// This enum defines possible test cases
//
typedef enum _IspUsecaseId
{
    UsecaseBPSInputMipiRaw10OutFullP010 = 0,
    UsecaseBPSInputMipiRaw10OutNV12,
    UsecaseBPSInputRaw16OutNV12,
    UsecaseBPSInputMipiRaw10OutFullP010Stats,
    UsecaseBPSInputRaw16OutNV12Stats,
    UsecaseSHDRInputRaw,
    UsecaseShdrBpsInputRaw16OutNV12,
    UsecaseShdrBpsStatsProcInputRaw16OutNV12,
    UsecaseBpsAWBBGStats,
    UsecaseIPEInputNV12OutUBWCNV12,
    UsecaseIPEInputNV12OutNV12,
    UsecaseBPSBGBHISTAWB,
    UsecaseShdrBpsAECAWB,
    UsecaseBPSAECAWB,
    UsecaseBPSIPEInputMipiRaw16OutNV12,
    UsecaseBPSAECAWBIPEInputMipiRaw16OutNV12,
    UsecaseShdrBpsIpeAECAWB,
#ifdef __QNXNTO__
    UsecaseBPSInputRaw16OutP010,
#endif
    IspUsecaseMax
} IspUsecaseId;

typedef struct
{
    CHISTREAMBUFFER *pChiBuffer;
    void* pPrivateHndl;
} ChiBufferMap;

typedef std::map<UINT32, AisEventPProcJobType*> FramePPorcJobMap;

class AisPProcIspChiSession
{
public:
    AisPProcIspChiSession();
    ~AisPProcIspChiSession();

    /**
     * initialize
     *
     * @brief initialize this session/pipeline
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    CameraResult Initialize(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);
    CameraResult Flush();

    /**
     * Deinitialize
     *
     * @brief release this session/pipeline
     *
     *
     * @return CameraResult
     */
    CameraResult Deinitialize(const AisProcChainType* pProcChain);

    CameraResult ProcessFrame(AisEventPProcJobType* pIspJob);

    CameraResult ProcCaptureResult(CHICAPTURERESULT*        pCaptureResult);

    UINT32       GetCameraId() const { return m_cameraId; }

    CameraResult UpdateFrameParams(qcarcam_param_isp_ctrls_t *pBayer_ctrls);

    CameraResult GetFrameParams(qcarcam_param_isp_ctrls_t *pBayer_ctrls);

private:

    // This method create offline request and submits it through chi api
    //
    // param [in]: ISP job information.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult GenerateOfflineCaptureRequest(AisEventPProcJobType* pIspJob);


    CameraResult PrepareRDIBuffers(CHISTREAM* pChiStream,
        AisBufferList* pBufferList,
        ChiBufferMap *pBufferMap);

    // This method configures chi streams based on test case.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        SetupStreams(const AisBuflistIdType* inBuflistId, const AisBuflistIdType* outBuflistId);

    // This method creates pipelines based on test case.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreatePipelines(UINT32       cameraId);

    // This method creates sessions.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreateSessions();

    // Chi callback implementation
    //
    static void         ChiProcCaptureResult(
        CHICAPTURERESULT*           pCaptureResult,
        VOID*                       pPrivateCallbackData);

    static void         ChiProcNotify(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
        VOID*                       pPrivateCallbackData);

    static void         ChiProcPartialCaptureResult(
         CHIPARTIALCAPTURERESULT*   pCaptureResult,
         VOID*                      pPrivateCallbackData);

    // This method processes single capture result.
    //
    // param [in] : pCaptureResult Pointer to capture result.
    // param [out]: pFrameProcessed Pointer to boolean that tells if capture
    //                              result has output frame and it was processed.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    void                ProcFrameResult(
        CHICAPTURERESULT*           pCaptureResult,
        bool*                       pFrameProcessed);

    // This method creates buffer managers for each stream in the test.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreateBufManagers();

    // This method destroys buffer managers for each stream in the test.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    void                DestroyBufManagers();

    // This method creates input metadata pool.
    //
    // param [in]: poolSize Size of the input metadata pool to be created.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreateInputMetabufPool(int poolSize);

    // This method creates output metadata pool.
    //
    // param [in]: poolSize Size of the output metadata pool to be created.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreateOutputMetabufPool(int poolSize);

    // This mehtod destroys metadata pools.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        DestroyMetabufPools();

    // This method creates default metadata.
    //
    // param [out] pChiMetaHandle Pointer to metadata handle.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        CreateDefaultMetada(CHIMETAHANDLE* pChiMetaHandle);

    // This method deep copies capture results.
    //
    // param[in] pCaptureResult Pointer to chi capture result structure.
    //
    // returns copy of capture reslut if successfull or NULL otherwise.
    //
    CHICAPTURERESULT*   DeepCopyCaptureResult(CHICAPTURERESULT* pCaptureResult);

    // This method deep copies capture results.
    //
    // param[in] pCaptureResult Pointer to chi capture result structure.
    //
    // returns none.
    //
    void                DeepDestroyCaptureResult(CHICAPTURERESULT* pCaptureResult);

    // This methods wait for test results to be completed.
    //
    // param [in]: timeoutMs Timeout to wait for results. 0xffffffff means infinite wait.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        WaitForResults(uint32_t timeoutMs);

    // This method sets bps iq settings via metadata and vendor tags
    //
    // param [in]: pMetadata Pointer to metadata from chi request.
    // param [in]: pBpsSettings Pointer to bps settings structure.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        SetupBpsIqSettings(CHIMETADATAHANDLE            pMetadata,
                                                const OEMBPSIQSetting*  pBpsIqSettings);

    // This method sets ipe iq settings via metadata and vendor tags
    //
    // param [in]: pMetadata Pointer to metadata from chi request.
    // param [in]: pIpeIqSettings Pointer to ipe settings structure.
    //
    // returns: CameraResult indicating success or failure of this method.

    CameraResult        SetupIpeIqSettings(CHIMETADATAHANDLE            pMetadata,
                                                const OEMIPEIQSetting*  pIpeIqSettings);

    // This method sets bps stats configuration via metadata and vendor tags.
    //
    // param [in]: pMetadata Pointer to metadata from chi request.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        SetupBpsStatsControl(CHIMETADATAHANDLE  pMetadata);

    // This method converts qcarcam ISP usecase to AIS ISP usecase
    // param [in]: qcarcam ISP usecase.
    // param [out]: pointer to CHI ISP usecase.
    // returns: CameraResult indicating success or failure of this method.
    CameraResult QcarcamToChiIspUsecase(qcarcam_isp_usecase_t qcarcamIspUsecase,
                                                IspUsecaseId* pChiIspUsecase);


    void                CaptureRequestPrint(const CHICAPTUREREQUEST* pCaptureRequest);
    void                CaptureResultPrint(const CHICAPTURERESULT* pCaptureResult);
    void                SetAndroidTags(CHIMETADATAHANDLE  pMetadata,
                                            qcarcam_param_isp_ctrls_t *pBayerCtrls);
    void                GetAndroidTags(CHIMETADATAHANDLE  pMetadata,
                                            qcarcam_param_isp_ctrls_t *pBayerCtrls);
    void                SetVendorTags(CHIMETADATAHANDLE pMetadata,
                                            qcarcam_param_isp_ctrls_t *pBayerCtrls);
    void                GetVendorTags(CHIMETADATAHANDLE pMetadata,
                                            qcarcam_param_isp_ctrls_t *pBayerCtrls);


    // Constants
    //
    static const char*  BPS_TAG_NAME;
    static const char*  SHDR_TAG_NAME;
    static const char*  IQ_TAG_SECTION;
    static const char*  STATS_CONFIG_TAG_SECTION;
    static const char*  AWB_STATS_CTL_TAG_NAME;
    static const char*  IPE_TAG_NAME;
    static const char*  AEC_FRAME_CTL_TAG_NAME;

    // Class members
    //
    AisUsrCtxt*         m_pUsrCtxt;

    ChiStream*          m_pRequiredStreams;             // pointer to created stream objects
    unsigned int        m_numStreams;                   // total number of streams for given test
    IspUsecaseId        m_usecaseId;                    // test id to differentiate between tests
    ChiPipeline*        m_pChiPipeline;                 // pipeline instance for isp test
    ChiSession*         m_pChiSession;                  // session instance for isp test
    CHICALLBACKS        m_callbacks;                    // chi callbacks

    CameraSignal        m_pReqsCompleteSig;             // signal to wake main thread on completion of all request
    CHIMETADATAOPS      m_MetadataOps;                  // function pointers to all metadata related api
    CHITAGSOPS          m_TagOps;                       // function pointers to all tag related api
    CHIMETAHANDLE*      m_pInputMetabufPool;            // input meta data pool for capture request
    int                 m_InputMetabufPoolSize;         // input metadata pool size
    CHIMETAHANDLE*      m_pOutputMetabufPool;           // output meta data pool for capture request
    int                 m_OutputMetabufPoolSize;        // output metadata pool size
    CHICAPTUREREQUEST   m_OfflineRequests[maxRequests]; // array of offline capture requests
    CHIPRIVDATA         m_requestPvtData;               // request private data
    CHIPIPELINEREQUEST  m_submitRequest;                // pipeline request
    CHIMETADATAENTRY    m_metadataEntry;                // Metadata entry
    UINT32              m_aectagId;
    UINT32              m_cameraId;

    ChiBufferMap m_InputBufferMap[QCARCAM_MAX_NUM_BUFFERS];
    ChiBufferMap m_OutputBufferMap[QCARCAM_MAX_NUM_BUFFERS];
    ChiBufferMap m_JpegOutputBufferMap[QCARCAM_MAX_NUM_BUFFERS];

    CameraMutex           m_pFrame2NumProcBufsMapMutex;   // mutex that guards m_Frame2NumProcBufsMap
    std::map<UINT32, int> m_Frame2NumProcBufsMap;   // map for getting num buffers remaining to process for specific frame

    CameraMutex         m_pFrame2pAisEventPProcJobTypeMutex;    // mutex that guards m_pFrame2pAisEventPProcJobType
    FramePPorcJobMap     m_pFrame2pAisEventPProcJobType;    // map for getting pAisEventPProcJobType for specific frame


    // Perf counters. They hold information related to performance measuremnets.
    //
    uint64_t            m_SubmitReq[maxRequests];           // Timestamp of completion of submit request to CAMX.
    uint64_t            m_ProcRes[maxRequests];             // Timestamp of compeletion of proc result callback from CAMX.
    uint64_t            m_ProcFrameMin;                     // Min duration of proc frame.
    uint64_t            m_ProcFrameMax;                     // Max duration of proc frame.
    uint64_t            m_ProcFrameCum;                     // Cumulative value of proc frames.
    uint64_t            m_ProcFrameNSamples;                // Number of samples of proc frames.
    uint64_t            m_ProcFrameAvg;                     // Average duration of proc frame.
    CameraMutex         m_ApplySettingsMutex;               // Lock to protect settings
    uint32_t            m_ApplySettingsReqCnt;              // Indicate new setting has received.
    uint32_t            m_CurrentFrameNum;                  // Current frame number.
    qcarcam_param_isp_ctrls_t m_BayerCtrls;               // Bayer sensor settings.

};

#endif






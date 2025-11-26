/*!
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AIS_PPROC_ISP_H_
#define _AIS_PPROC_ISP_H_

#include "ais_i.h"
#include "ais_engine.h"

#include <queue>
#include <map>

#include "CameraResult.h"
#include "CameraOSServices.h"
#include "chi.h"
#include "chiiqmodulesettings.h"
#include "ais_pproc_isp_chisession.h"

// Forward declarations
//
class ChiModule;

typedef enum
{
    ISP_EVENT_PROCESS_FRAME
}AisIspEventId;

typedef struct
{
    AisIspEventId id;

    union {
        AisEventPProcJobType* pIspJob;
    }payload;
}AisIspEvent;

typedef struct
{
    AisPProcIspChiSession*  pSession;
    CHICAPTURERESULT*       pChiCaptureResult;
}AisIspCaptureResult;

typedef enum
{
    AIS_PPROC_ISP_STATE_INVALID,
    AIS_PPROC_ISP_STATE_INIT,
    AIS_PPROC_ISP_STATE_READY,
    AIS_PPROC_ISP_STATE_ERROR
}AisIspState;

typedef struct
{
    AisPProcIspChiSession* pSession;
    AisUsrCtxt* pUsrCtxt;
    uint32 instanceId;
}AisPProcIspChiSessionListType;

class AisPProcIsp : public AisPProc
{
public:
    static AisPProcIsp* GetInstance();
    static void DestroyInstance();

    /**
     * Create
     *
     * @brief Create ISP scheduler
     *
     * @return AisPProcIsp*
     */
    static AisPProcIsp* Create();

    /**
     * Destroy
     *
     * @brief Destroy ISP scheduler
     *
     * @return None
     */
    static void Destroy(void);

    /**
     * CreateSession
     *
     * @brief Create ISP session for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    virtual CameraResult CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);

    /**
     * DestroySession
     *
     * @brief Destroy ISP session for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    virtual CameraResult DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);

    /**
     * Flush
     *
     * @brief Flush job for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    virtual CameraResult Flush(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);


    /**
     * ProcessEvent
     *
     * @brief Process ISP event for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pEvent
     *
     * @return CameraResult
     */
    virtual CameraResult ProcessEvent(AisUsrCtxt* pUsrCtxt, AisEventMsgType* pEvent);

    /**
     * SetParams
     *
     * @brief Apply ISP Params for UsrCtxt
     *
     * @param pUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult SetParams(AisUsrCtxt* pUsrCtxt);

    /**
     * GetParams
     *
     * @brief Get ISP Params for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pParamOut
     *
     * @return CameraResult
     */
    virtual CameraResult GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut);

    void GetMetadataOp(CHIMETADATAOPS*);
    void GetTagOp(CHITAGSOPS*);

    CameraResult SubmitPipelineRequest(CHIPIPELINEREQUEST* );

    ChiModule* GetChiModule() { return m_pChiModule; }

    // This method enqueues capture result.
    //
    // param : pSession  Pointer to AisPProcIspChiSession
    // param : pCaptureResult Pointer to chi capture result structure.
    //
    // returns: CameraResult
    //
    CameraResult        EnqCaptureResult(AisPProcIspChiSession* pSession, CHICAPTURERESULT* pCaptureResult);

private:
    AisPProcIsp();

    static int IspEventHandler(void *pArg);

    int ProcessIspEvent(AisIspEvent* pEvent);

    CameraResult EnqueueEvent(AisIspEvent* pEvent);
    CameraResult PrepareRDIBuffers(CHISTREAM* pChiStream,
        AisBufferList* pBufferList,
        ChiBufferMap *pBufferMap);

    int ProcessIspEvent(void);

    //This method deinitializes members of this class.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    void        Deinitialize(void);

    //This method initializes members of this class.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        InitializeChi();

    // This is a main rootine for capture processing thread.
    //
    // param [in]: pArg Pointer to IspChiCdk instance
    //
    // return: int not used
    //
    static int        ProcThreadEntry(void *pArg);

    CameraResult      ProcCaptureResults();

    AisPProcIspChiSession*  GetSession(AisUsrCtxt*, uint32 instanceId);
    void                    RemoveSession(AisUsrCtxt*, uint32 instanceId);
    CameraResult            AddSession(AisUsrCtxt*, uint32 instanceId, AisPProcIspChiSession*);

    // Class members
    //
    static AisPProcIsp* m_pIspSchedulerInstance;

    AisIspState     m_state;

    CameraSignal    m_ispEventHandlerSignal; /* Signal to event handler that job available */

    /*ISP Event handler*/
    CameraThread    m_ispEventHandlerTid;
    volatile boolean    m_bIspEventHandlerExit;
    CameraQueue     m_ispEventQ;
    CameraMutex     m_ispMutex; // mutex that protect m_state


    ChiModule*          m_pChiModule;                   // pointer to the ChiModule

    CameraSignal        m_pProcThreadSignal;            // signal to wake processing thread
    CameraMutex         m_pCaptureResQMutex;            // mutex that guards capture results queue
    CameraThread        m_procThreadHandle;             // worker thread that processes capture results
    volatile boolean    m_procThreadExit;
    std::queue<AisIspCaptureResult>* m_pCaptureResultsQ; // queue that holds results to be processed

    std::list<AisPProcIspChiSessionListType> m_pChiSessionMap;
};

#endif /* _AIS_PPROC_ISP_H_ */

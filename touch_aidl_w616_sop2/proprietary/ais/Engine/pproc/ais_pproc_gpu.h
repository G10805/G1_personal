/*!
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AIS_PPROC_GPU_H_
#define _AIS_PPROC_GPU_H_

#include <map>
#include <list>

#include "ais_i.h"
#include "ais_engine.h"

typedef enum
{
    AIS_PPROC_GPU_FUNCTION_PAIRED_INPUT,
    AIS_PPROC_GPU_FUNCTION_DEINTERLACE,
    AIS_PPROC_GPU_FUNCTION_TRANSFORMER
} AisPProcGpuFunction;

typedef enum
{
    PPROC_GPU_BUFFER_OUT,
    PPROC_GPU_BUFFER_RIGHT,
    PPROC_GPU_BUFFER_LEFT,
    PPROC_GPU_BUFFER_IN,
    PPROC_GPU_BUFFER_MAX
} AisPProcGpuBufferType;

typedef struct
{
    qcarcam_frame_info_v2_t frameInfo; /**< Frame done info*/

    //c2d info
    void *c2dPhysAddr[3];
    uint32 c2dSurfaceId;
} AisPProcGpuBuffer;

class AisPProcGpuSession
{
public:
    virtual ~AisPProcGpuSession(){};
    virtual CameraResult ProcessEvent(AisEventPProcJobType*) = 0;

protected:
    CameraResult MapBuffers(AisPProcGpuBufferType, AisPProcGpuFunction);
    void ReturnBuffer(AisPProcGpuBufferType, uint32);
    CameraResult UnmapAllBuffers();

    /**< buffer info */
    AisPProcGpuBuffer* m_pBuffers[PPROC_GPU_BUFFER_MAX];
    std::list<AisBuffer*> m_bufsInFlight[PPROC_GPU_BUFFER_MAX];
    uint32     m_bufferlistIdx[PPROC_GPU_BUFFER_MAX];

    /**< user context */
    AisUsrCtxt* m_pUsrCtxt;
};

typedef std::map<AisUsrCtxt*, AisPProcGpuSession*> AisPProcGpuSessionMap;

class AisPProcGpuPairedInputSession : public AisPProcGpuSession
{
public:
    AisPProcGpuPairedInputSession(AisUsrCtxt*, const AisProcChainType*);
    ~AisPProcGpuPairedInputSession();

    virtual CameraResult ProcessEvent(AisEventPProcJobType*);

private:
    /**
     * CombineBuffers
     *
     * @brief Find matching timestamp for left/right buffers and blit using C2D onto output buffer
     *
     * @return CameraResult
     */
    CameraResult CombineBuffers(AisEventPProcJobType*);
};

class AisPProcGpuDeinterlaceSession : public AisPProcGpuSession
{
public:
    AisPProcGpuDeinterlaceSession(AisUsrCtxt*, const AisProcChainType*);
    ~AisPProcGpuDeinterlaceSession();

    virtual CameraResult ProcessEvent(AisEventPProcJobType*);

private:
    /**
     * WeaveBuffers
     *
     * @brief Use C2D to weave odd and even frames into output buffer
     *
     * @return CameraResult
     */
    CameraResult WeaveBuffers(AisEventPProcJobType*, qcarcam_field_t);
};

class AisPProcGpuTransformSession : public AisPProcGpuSession
{
public:
    AisPProcGpuTransformSession(AisUsrCtxt*, const AisProcChainType*);
    ~AisPProcGpuTransformSession();

    virtual CameraResult ProcessEvent(AisEventPProcJobType*);

private:
    CameraResult Transform(AisBuffer*, AisEventPProcJobType*);
};


class AisPProcGpu : public AisPProc
{
public:
    /**
     * Create
     *
     * @brief Create Node
     *
     * @return AisPProcGpu*
     */
    static AisPProcGpu* Create();

    /**
     * Destroy
     *
     * @brief Destroy Node
     *
     * @return None
     */
    static void Destroy(void);

    /**
     * CreateSession
     *
     * @brief Create GPU session for UsrCtxt
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
     * @brief Destroy GPU session for UsrCtxt
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
    virtual CameraResult Flush(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
    {
        CAM_UNUSED(pUsrCtxt);
        CAM_UNUSED(pProcChain);

        return CAMERA_SUCCESS;
    };

    /**
     * ProcessEvent
     *
     * @brief Process Event for UsrCtxt
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
     * @brief Apply Params for UsrCtxt
     *
     * @param pUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult SetParams(AisUsrCtxt* pUsrCtxt)
    {
        CAM_UNUSED(pUsrCtxt);

        return CAMERA_SUCCESS;
    }

    /**
     * GetParams
     *
     * @brief Get Params for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pParamOut
     *
     * @return CameraResult
     */
    virtual CameraResult GetParams(AisUsrCtxt* pUsrCtxt,
            void *pParamOut)
    {
        CAM_UNUSED(pUsrCtxt);
        CAM_UNUSED(pParamOut);

        return CAMERA_SUCCESS;
    }

private:
    AisPProcGpu();
    ~AisPProcGpu();

    /**
     * AddSession
     *
     * @brief Add session to map
     *
     * @param pUsrCtxt
     * @param pSession
     *
     * @return CameraResult
     */
    CameraResult AddSession(AisUsrCtxt* pUsrCtxt, AisPProcGpuSession* pSession);

    /**
     * AddSession
     *
     * @brief Remove session from map
     *
     * @param pUsrCtxt
     *
     * @return none
     */
    void RemoveSession(AisUsrCtxt* pUsrCtxt);

    /**
     * GetSession
     *
     * @brief Get session from map
     *
     * @param pUsrCtxt
     *
     * @return AisPProcCombineSession pointer
     */
    AisPProcGpuSession* GetSession(AisUsrCtxt* pUsrCtxt);

    /**
     * GPUEventHandler
     *
     * @brief Calls c2dDriverInit
     *
     * @param NULL
     *
     * @return 0
     */
    static int GPUEventHandler(void* pArg);

    static AisPProcGpu* m_pNodeInstance;
    boolean m_isC2dInitialized;
    CameraMutex m_gpuMutex;
    AisPProcGpuSessionMap m_sessionMap;
    CameraThread m_gpuEventHandlerTid;
};

#endif /* _AIS_PPROC_GPU_H_ */

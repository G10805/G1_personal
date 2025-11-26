#ifndef _AIS_PPROC_H_
#define _AIS_PPROC_H_

/*!
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_i.h"

class AisPProc;

#define AIS_PPROC_MAX_NUM_PROCS   16

typedef enum
{
    AIS_PPROC_USR_DONE,
    AIS_PPROC_ISP,
    AIS_PPROC_GPU,
    AIS_PPROC_FRAMESYNC,
    AIS_PPROC_MEMCPY,
    AIS_PPROC_RGBIR,
    AIS_PPROC_EXT,
    AIS_PPROC_MAX
}AisPProcIdType;

typedef enum
{
    AIS_PPROC_STEP_REQUIRED = 0,             /**< required step - abort job on any failure */
    AIS_PPROC_STEP_OPTIONAL_INPUT_BUFFERS    /**< optional step - keep going if fail with CAMERA_ENOREADYBUFFER */
}AisPProcStepType;

/**
 * Processing module description
 */
typedef struct AisProcChainType
{
    AisPProcIdType      id;
    uint32              pprocFunction; /**< Used in pproc nodes that may support multiple functions */
    AisBuflistIdType    inBuflistId[AIS_PPROC_MAX_BUFLISTS];
    AisBuflistIdType    outBuflistId[AIS_PPROC_MAX_BUFLISTS];
    const char*         pLibName;   /**< for dlopen */
    uint32              instanceId; /**< instance Id for multi instances of nodes */
    uint32              numIn;      /**< number of input bufferlists */
    uint32              numOut;     /**< number of output bufferlists */
    AisPProcStepType    stepType;   /**< required or not */
}AisProcChainType;

typedef enum
{
    AIS_BUFLIST_ALLOC_NONE = 0,
    AIS_BUFLIST_ALLOC_MATCH_INPUT,  /**< Match input device size and format */
    AIS_BUFLIST_ALLOC_MATCH_INPUT_SIZE, /**< Match input device size only. Specify format */
    AIS_BUFLIST_ALLOC_MATCH_BUFLIST, /**< Match another buffer list */
    AIS_BUFLIST_ALLOC_FIXED,    /**< Specify size and fmt */
    AIS_BUFLIST_ALLOC_QUERY,    /**< Query PProc for alloc information */
}AisBuflistAllocType;

typedef struct
{
    AisBuflistAllocType allocType; /**< Allocation type */

    uint32 maxBuffers; /**< Maximum number of buffers */

    /**< for MATCH_BUFLIST, buffer list id to match */
    uint32 matchBuflistId;

    /**< for MATCH_INPUT_SIZE and FIXED, define fmt */
    qcarcam_color_fmt_t fmt;

    /**< for FIXED, define size */
    uint32 width;
    uint32 height;
    uint32 stride;

    /**< for MATCH_INPUT and MATCH_BUFLIST, define alignment */
    uint32 align;
}AisBuflistAllocParams;

/**
 * Bufferlist usecase description
 */
typedef struct AisBuflistDefType
{
    AisBuflistIdType        id;
    GetFreeBufFcnType       GetFreeBuf;
    ReturnBufFcnType        ReturnBuf;
    AllocBufType            AllocBuf;
    AisBuflistAllocParams   allocParams;
}AisBuflistDefType;

typedef enum
{
    AIS_STREAM_TYPE_USER = 0,
    AIS_STREAM_TYPE_IFE,
}AisStreamType;

typedef struct AisStreamDefType
{
    AisStreamType      type;
    AisBuflistIdType   buflistId;
}AisStreamDefType;

typedef struct
{
    const AisProcChainType* pProcChain;  /**< proc chain steps */
    uint32 nProc;  /**< number of proc chain steps. Must be <= AIS_PPROC_MAX_NUM_PROCS */

    const AisBuflistDefType* pBufferlistDef; /**< bufferlist definitions */
    uint32 nBuflist;                         /**< number of bufferlist definitions */

    uint32 nStreams;                        /**< number of streams */
    AisStreamDefType streams[AIS_USER_CTXT_MAX_STREAMS];  /**< stream definitions */
}AisProcChainDefType;


class AisProcChainManager
{
protected:
    virtual ~AisProcChainManager(){};

    static AisProcChainManager* m_pProcChainManagerInstance;

public:
    static AisProcChainManager* CreateInstance();
    static AisProcChainManager* GetInstance();
    static void DestroyInstance();
    static CameraResult GetNumStreams(qcarcam_opmode_type opMode, uint32* pNumStreams);

    virtual CameraResult GetProcChain(AisUsrCtxt* pUsrCtxt) = 0;
    virtual CameraResult ReleaseProcChain(AisUsrCtxt* pUsrCtxt) = 0;

    virtual AisPProc* GetPProc(AisPProcIdType id) = 0;
};

/**
 * AIS Post Process Block Base Class
 */
class AisPProc
{
protected:
    virtual ~AisPProc(){};

public:
    /**
     * CreateSession
     *
     * @brief Create session for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    virtual CameraResult CreateSession(AisUsrCtxt* pUsrCtxt,
            const AisProcChainType* pProcChain) = 0;

    /**
     * DestroySession
     *
     * @brief Destroy session for UsrCtxt
     *
     * @param pUsrCtxt
     * @param pProcChain
     *
     * @return CameraResult
     */
    virtual CameraResult DestroySession(AisUsrCtxt* pUsrCtxt,
            const AisProcChainType* pProcChain) = 0;


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
    virtual CameraResult Flush(AisUsrCtxt* pUsrCtxt,
            const AisProcChainType* pProcChain) = 0;


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
    virtual CameraResult ProcessEvent(AisUsrCtxt* pUsrCtxt,
        AisEventMsgType* pEvent) = 0;

    /**
     * SetParams
     *
     * @brief Apply Params for UsrCtxt
     *
     * @param pUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult SetParams(AisUsrCtxt* pUsrCtxt) = 0;

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
            void *pParamOut) = 0;
};

#endif /* _AIS_PPROC_H_ */

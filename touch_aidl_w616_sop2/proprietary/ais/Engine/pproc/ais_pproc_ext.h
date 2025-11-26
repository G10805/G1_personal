/*!
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AIS_PPROC_NODE_H_
#define _AIS_PPROC_NODE_H_

#include "ais_proc_chain.h"

class AisPProcNode : public AisPProc
{
public:
    /**
     * Create
     *
     * @brief Create Node
     *
     * @return AisPProcNode*
     */
    static AisPProcNode* Create();

    /**
     * Destroy
     *
     * @brief Destroy Node
     *
     * @return None
     */
    void Destroy(void);

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
    virtual CameraResult CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain);

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
     * @brief Process Event
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
    virtual CameraResult SetParams(AisUsrCtxt* pUsrCtxt);

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
    virtual CameraResult GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut);

private:
    AisPProcNode();
    ~AisPProcNode();

    void* m_pCtxt;
};

#endif /* _AIS_PPROC_NODE_H_ */

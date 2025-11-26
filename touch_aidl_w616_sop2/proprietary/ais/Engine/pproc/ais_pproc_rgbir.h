/*!
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AIS_PPROC_RGBIR_H_
#define _AIS_PPROC_RGBIR_H_

#include "ais_proc_chain.h"

class AisPProcRgbIR : public AisPProc
{
public:
    /**
     * Create
     *
     * @brief Create Node
     *
     * @return AisNodeRgbIR*
     */
    static AisPProcRgbIR* Create()
    {
        m_pNodeInstance = new AisPProcRgbIR();
        return m_pNodeInstance;
    };

    /**
     * Destroy
     *
     * @brief Destroy Node
     *
     * @return None
     */
    static void Destroy(void)
    {
        if (m_pNodeInstance)
        {
            delete m_pNodeInstance;
            m_pNodeInstance = nullptr;
        }
    };

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
    virtual CameraResult CreateSession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
    {
        CAM_UNUSED(pUsrCtxt);
        CAM_UNUSED(pProcChain);

        return CAMERA_SUCCESS;
    };

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
    virtual CameraResult DestroySession(AisUsrCtxt* pUsrCtxt, const AisProcChainType* pProcChain)
    {
        CAM_UNUSED(pUsrCtxt);
        CAM_UNUSED(pProcChain);

        return CAMERA_SUCCESS;
    };

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
    virtual CameraResult SetParams(AisUsrCtxt* pUsrCtxt)
    {
        CAM_UNUSED(pUsrCtxt);

        return CAMERA_SUCCESS;
    };

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
    virtual CameraResult GetParams(AisUsrCtxt* pUsrCtxt, void *pParamOut)
    {
        CAM_UNUSED(pUsrCtxt);
        CAM_UNUSED(pParamOut);

        return CAMERA_SUCCESS;
    };

private:
    static AisPProcRgbIR* m_pNodeInstance;
};

#endif /* _AIS_PPROC_RGBIR_H_ */

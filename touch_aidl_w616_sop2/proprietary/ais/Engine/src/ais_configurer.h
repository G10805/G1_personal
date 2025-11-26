#ifndef _AIS_CONFIGURER_H_
#define _AIS_CONFIGURER_H_

/*!
 * Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ais_engine.h"

class AisUsrCtxt;

class AisEngineConfigurer
{
protected:
    virtual ~AisEngineConfigurer(){};
public:
    /**
     * Config
     *
     * @brief Configure according to user context streams.
     *
     * @param AisUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult Config(AisUsrCtxt* pUsrCtxt) = 0;

    /**
     * Global Config
     *
     * @brief Configure all devices.
     *
     * @param AisGlobalCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult GlobalConfig(AisGlobalCtxt* pGlobalCtxt) = 0;

    /**
     * Start
     *
     * @brief Start user context streams.
     *
     * @param AisUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult Start(AisUsrCtxt* pUsrCtxt) = 0;

    /**
     * Stop
     *
     * @brief Stop user context streams.
     *
     * @param AisUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult Stop(AisUsrCtxt* pUsrCtxt) = 0;

    /**
     * Resume
     *
     * @brief Resume user context streams.
     *
     * @param AisUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult Resume(AisUsrCtxt* pUsrCtxt) = 0;

    /**
     * Pause
     *
     * @brief Pause user context streams.
     *
     * @param AisUsrCtxt
     *
     * @return CameraResult
     */
    virtual CameraResult Pause(AisUsrCtxt* pUsrCtxt) = 0;

    /**
     * SetParam
     *
     * @brief Set user context param.
     *
     * @param AisUsrCtxt
     * @param nCtrl       Parameter ID
     * @param pParam      Pointer to parameter data
     *
     * @return CameraResult
     */
    virtual CameraResult SetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *pParam) = 0;

    /**
     * GetParam
     *
     * @brief Get user context param.
     *
     * @param AisUsrCtxt
     * @param nCtrl       Parameter ID
     * @param pParam      Pointer to parameter data
     *
     * @return CameraResult
     */
    virtual CameraResult GetParam(AisUsrCtxt* pUsrCtxt, uint32 nCtrl, void *pParam) = 0;

    /**
     * PowerSuspend
     *
     * @brief Shut power to devices.
     *
     * @param AisUsrCtxt
     * @param bGranular   If true, only power down resources held by AisUsrCtxt.
     *                    If false, power suspend all the configurer's resources.
     *
     * @return CameraResult
     */
    virtual CameraResult PowerSuspend(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId) = 0;

    /**
     * PowerResume
     *
     * @brief Restore power to devices.
     *
     * @param AisUsrCtxt
     * @param bGranular   If true, only power up resources held by AisUsrCtxt.
     *                    If false, power up all the configurer's resources.
     *
     * @return CameraResult
     */
    virtual CameraResult PowerResume(AisUsrCtxt* pUsrCtxt, boolean bGranular, CameraPowerEventType powerEventId) = 0;
};

#endif /* _AIS_CONFIGURER_H_ */

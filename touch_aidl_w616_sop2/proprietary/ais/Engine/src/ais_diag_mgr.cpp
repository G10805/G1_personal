/*!
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <dlfcn.h>

#include "ais_diag_mgr.h"
#include "ais_engine.h"
#include "ais_input_configurer.h"
#include "ais_csi_configurer.h"
#include "ais_ife_configurer.h"
#include "CameraPlatform.h"
#include "CameraMIPICSI2Types.h"
#include "ife_drv_api.h"

/* Interval in which diag info update happens unit ms */
#define AIS_DIAG_INFO_UPDATE_INTERVAL 10000

AisDiagManager* AisDiagManager::m_pDiagManagerInstance = nullptr;

/**
 * CreateInstance
 *
 * @brief creates the singleton instance of AisDiagManager
 *
 * @return AisDiagManager singleton instance
 *
 */
AisDiagManager* AisDiagManager::CreateInstance()
{
    if(m_pDiagManagerInstance == nullptr)
    {
        m_pDiagManagerInstance = new AisDiagManager();
        if (m_pDiagManagerInstance)
        {
            CameraResult rc = m_pDiagManagerInstance->Initialize();
            if (rc)
            {
                DestroyInstance();
            }
        }
    }
    return m_pDiagManagerInstance;
}

/**
 * GetInstance
 *
 * @brief Gets the singleton instance of AisDiagManager
 *
 * @return AisDiagManager singleton instance
 *
 */
AisDiagManager* AisDiagManager::GetInstance()
{
    return m_pDiagManagerInstance;
}

/**
 * DestroyInstance
 *
 * @brief Destroys the singleton instance of AisDiagManager
 *
 *@return none
 *
 */
void AisDiagManager::DestroyInstance()
{
    if(m_pDiagManagerInstance != nullptr)
    {
        m_pDiagManagerInstance->Uninitialize();

        delete m_pDiagManagerInstance;
        m_pDiagManagerInstance = nullptr;
    }
}

/**
 * Uninitialize
 *
 * @brief Uninitializes the AisDiagManager
 *
 */
void AisDiagManager::Uninitialize()
{
    m_diagUpdate = FALSE;

    if (m_diagInfoSignal)
    {
        CameraSetSignal(m_diagInfoSignal);

        if (m_diagMgrUpdateThread)
        {
            CameraJoinThread(m_diagMgrUpdateThread, NULL);
            CameraReleaseThread(m_diagMgrUpdateThread);
            m_diagMgrUpdateThread = NULL;
        }

        CameraDestroySignal(m_diagInfoSignal);
        m_diagInfoSignal = NULL;
    }

    if (m_errQueueMutex)
    {
        CameraDestroyMutex(m_errQueueMutex);
        m_errQueueMutex = NULL;
    }
}

/**
 * Initialize
 *
 * @brief Initialize the AisDiagManager
 *
 */
CameraResult AisDiagManager::Initialize()
{
    CameraResult rc;

    rc = CameraCreateMutex(&m_errQueueMutex);

    if (CAMERA_SUCCESS == rc)
    {
        rc = CameraCreateSignal(&m_diagInfoSignal);
    }

    if (CAMERA_SUCCESS == rc)
    {
        m_diagUpdate = TRUE;
        char name[64] = "ais_diag_mgr_update";

        if (0 !=  CameraCreateThread(CAMERA_THREAD_PRIO_DEFAULT,
                0,
                AisDiagManager::DiagInfoUpdate,
                this,
                0x8000,
                name,
                &m_diagMgrUpdateThread))
        {
            AIS_LOG(ENGINE, ERROR, "CameraCreateThread failed");
            rc = CAMERA_EFAILED;
        }
    }

    if (CAMERA_SUCCESS == rc)
    {
        AIS_LOG(ENGINE, LOW, "Diag Manager Initialize success");
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Diag Manager failed to initialize %d", rc);
    }

    return rc;
}

/**
 * QueryDiagnostics
 *
 * @brief Fill the diagnostic info in the memory sent by application
 *
 * @param pDiagInfo    memory where diagnostic info needs to be filled
 * @param diagInfoSize sizeof diagnostic memory
 *
 * @return CameraResult
 */
CameraResult AisDiagManager::QueryDiagnostics(void *pDiagInfo, uint32 diagInfoSize)
{
    CameraResult rc = CAMERA_SUCCESS;

    memset(&m_pDiagInfo.aisDiagClientInfo, 0x0, sizeof(m_pDiagInfo.aisDiagClientInfo));
    AisEngine::GetInstance()->FillUsrCtxtDiagInfo(m_pDiagInfo.aisDiagClientInfo);

    if (pDiagInfo != NULL && diagInfoSize == sizeof(m_pDiagInfo))
    {
        memcpy(pDiagInfo, &m_pDiagInfo, diagInfoSize);
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "memory is not allocated by user");
        rc = CAMERA_EBADPARM;
    }

    return rc;
}

/**
 * InitializeDiagstats
 *
 * @brief Initialize the device(input/csi/ife) statistics of diagnostic structure
 *
 * @return CameraResult
 *
 */
CameraResult AisDiagManager::InitializeDiagstats()
{
    CameraResult rc = CAMERA_SUCCESS;

    rc = InitSensorDeviceStats();
    if (CAMERA_SUCCESS == rc)
    {
        rc = InitIfeStats();
    }
    if (CAMERA_SUCCESS == rc)
    {
        rc = InitCsiphyStats();
    }

    return rc;
}

/**
 * InitSensorDeviceStats
 *
 * @brief Initialize input device statistics info
 *
 * @return CameraResult
 *
 */
CameraResult AisDiagManager::InitSensorDeviceStats()
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 socId = CameraPlatform_GetMultiSocId();

    for (int device = 0; device < MAX_NUM_CAMERA_INPUT_DEVS; device++)
    {
        const CameraSensorBoardType* inputDevInfo = NULL;
        inputDevInfo = CameraPlatformGetSensorBoardType((CameraSensorIndex)device);

        QCarCamDiagInputDevInfo* pInputDeviceInfo = &m_pDiagInfo.aisDiagInputDevInfo[device];

        if (NULL != inputDevInfo && socId < SOC_ID_MAX)
        {
            pInputDeviceInfo->inputDevId = inputDevInfo->devId;
            pInputDeviceInfo->numSensors = inputDevInfo->devConfig.numSensors;
            pInputDeviceInfo->cciMap.cciDevId = inputDevInfo->i2cPort[socId].device_id;
            pInputDeviceInfo->cciMap.cciPortId = inputDevInfo->i2cPort[socId].port_id;
        }
    }
    AisInputConfigurer::GetInstance()->FillInputDeviceDiagInfo(m_pDiagInfo.aisDiagInputDevInfo);
    return rc;
}

/**
 * InitIfeStats
 *
 * @brief Initialize IFE device statistics info
 *
 * @CameraResult
 *
 */
CameraResult AisDiagManager::InitIfeStats()
{
    CameraResult rc = CAMERA_SUCCESS;
    const AisResMgrIfeResourceType *ifeResourceInfo = NULL;

    ifeResourceInfo = AisResourceManager::GetInstance()->GetIfeResourcesInfo();

    if (NULL != ifeResourceInfo)
    {
        for (int device = 0; device < MAX_NUM_IFE_DEVICES; device++)
        {
            QCarCamDiagIfeDevInfo* pIfeDeviceInfo = &m_pDiagInfo.aisDiagIfeDevInfo[device];
            pIfeDeviceInfo->ifeDevId = device;
            pIfeDeviceInfo->csiDevId = ifeResourceInfo[device].csiPhy;
            pIfeDeviceInfo->csidPktsRcvd = 0;
            pIfeDeviceInfo->numRdi = ifeResourceInfo[device].numRdi;
            for (int i = 0; i < (int)pIfeDeviceInfo->numRdi; i++)
            {
                pIfeDeviceInfo->rdiInfo[i].rdiId = (uint32)ifeResourceInfo[device].interface[i].output;
                pIfeDeviceInfo->rdiInfo[i].rdiStatus = 0;
            }
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Failed to retrieve IFE information from resource manager!");
        rc = CAMERA_EFAILED;
    }
    return rc;
}

/**
 * InitCsiphyStats
 *
 * @brief Initialize CSIPHY device statistics info
 *
 * @return CameraResult
 *
 */
CameraResult AisDiagManager::InitCsiphyStats()
{
    CameraResult rc = CAMERA_SUCCESS;
    const CameraCsiInfo *csiResourceInfo = NULL;

    csiResourceInfo = AisResourceManager::GetInstance()->GetCsiResourcesInfo();

    if (NULL != csiResourceInfo)
    {
        for (int device = 0; device < CSIPHY_CORE_MAX; device++)
        {
            QCarCamDiagCsiDevInfo* pCsiDeviceInfo = &m_pDiagInfo.aisDiagCsiDevInfo[device];
            pCsiDeviceInfo->csiphyDevId = csiResourceInfo[device].csiId;
            pCsiDeviceInfo->csiLaneMapping = csiResourceInfo[device].laneAssign;
            pCsiDeviceInfo->numIfeMap = csiResourceInfo[device].numIfeMap;
            pCsiDeviceInfo->ifeMap = csiResourceInfo[device].ifeMap;
        }
    }
    else
    {
        AIS_LOG(ENGINE, ERROR, "Failed to retrieve CSI information from resource manager!");
        rc = CAMERA_EFAILED;
    }
    return rc;
}

/**
 * UpdateIfeRdiStatus
 *
 * @brief Updates the rdi status info when rdi is acquired/released by a usrctxt
 *
 * @param ifeCore ife device id
 *
 * @param ifeRdiId ife rdi output id
 *
 * @param status whether rdi is acquired or released
 *
 * @return none
 *
 */
void AisDiagManager::UpdateIfeRdiStatus(uint32 ifeCore, uint32 ifeRdiId, uint32 status)
{
    m_pDiagInfo.aisDiagIfeDevInfo[ifeCore].rdiInfo[ifeRdiId].rdiStatus = status;
}

/**
 * GetErrorQueueTop
 *
 * @brief return the top of diagnostic error info queue
 *
 * @return none
 *
 */
QCarCamDiagErrorInfo* AisDiagManager::GetErrorQueueTop()
{
    QCarCamDiagErrorInfo* pErrInfo = NULL;

    CameraLockMutex(m_errQueueMutex);
    pErrInfo = &m_pDiagInfo.aisDiagErrInfo[m_errQueueTop];

    m_errQueueTop++;
    m_errQueueTop %= MAX_ERR_QUEUE_SIZE;

    CameraUnlockMutex(m_errQueueMutex);

    memset(pErrInfo, 0x0, sizeof(*pErrInfo));

    return pErrInfo;
}

/**
 * GetIfeDeviceDiagInfo
 *
 * @brief Updates the csid pkt info with no.of pkts rcvd
 *
 * @return none
 *
 */
void AisDiagManager::GetIfeDeviceDiagInfo()
{
    CameraResult rc = CAMERA_SUCCESS;

    for (int i = 0; i < MAX_NUM_IFE_DEVICES; i++)
    {
        QCarCamDiagIfeDevInfo* ifeInfo = &m_pDiagInfo.aisDiagIfeDevInfo[i];
        IfeDiagInfo ifeDiagInfo = {};

        rc = AisIFEConfigurer::GetInstance()->GetIfeDiagInfo(ifeInfo->ifeDevId, (void*)&ifeDiagInfo, sizeof(ifeDiagInfo));

        if (CAMERA_SUCCESS == rc)
        {
            ifeInfo->csidPktsRcvd = ifeDiagInfo.pktsRcvd;
        }
    }
}

/**
 * GetCsiDeviceDiagInfo
 *
 * @brief gets the csiphy status
 *
 * @return none
 *
 */
void AisDiagManager::GetCsiDeviceDiagInfo()
{
    CameraResult rc = CAMERA_SUCCESS;

    for (int i = 0; i < MAX_NUM_CSIPHY_DEVICES; i++)
    {
        QCarCamDiagErrorInfo* errInfo = AisDiagManager::GetInstance()->GetErrorQueueTop();

        errInfo->csiphyDevId = i;
        errInfo->errorType = AIS_EVENT_CSIPHY_WARNING;

        CsiDiagInfo diagInfo = {};
        rc = AisCSIConfigurer::GetInstance()->GetCsiDiagInfo(i, (void*)&diagInfo, sizeof(diagInfo));
        if (CAMERA_SUCCESS != rc)
        {
            AIS_LOG(ENGINE, LOW, "Failed to retrieve CSI PHY[%d] status", i);
        }
        else
        {
            memcpy(&errInfo->payload, &(diagInfo.status), sizeof(diagInfo.status));
        }
    }
}

/**
 * DiagInfoUpdate
 *
 * @brief diagnostic info regular update
 *
 * @return 0
 *
 */
int AisDiagManager::DiagInfoUpdate(void* arg)
{
    AisDiagManager* aisDiagMgr = (AisDiagManager*)arg;

    if (aisDiagMgr)
    {
        while (aisDiagMgr->m_diagUpdate)
        {
            AisEngine::GetInstance()->UpdateUsrCtxtInfo();

            aisDiagMgr->GetIfeDeviceDiagInfo();

            aisDiagMgr->GetCsiDeviceDiagInfo();

            CameraWaitOnSignal(aisDiagMgr->m_diagInfoSignal, AIS_DIAG_INFO_UPDATE_INTERVAL);
        }
    }

    return 0;
}

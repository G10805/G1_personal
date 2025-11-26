#ifndef _AIS_DIAG_MGR_H_
#define _AIS_DIAG_MGR_H_

/*!
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "ais_i.h"
#include "qcarcam_diag_types.h"
#include "ais_res_mgr.h"

class AisDiagManager
{
private:
    static AisDiagManager *m_pDiagManagerInstance;

    QCarCamDiagInfo m_pDiagInfo;
    uint32 m_errQueueTop;
    CameraMutex m_errQueueMutex;
    CameraThread m_diagMgrUpdateThread;
    volatile boolean m_diagUpdate;
    CameraSignal m_diagInfoSignal;

    AisDiagManager()
    {
        memset(&m_pDiagInfo, 0, sizeof(m_pDiagInfo));
        m_errQueueTop = 0;
        m_errQueueMutex = NULL;
        m_diagMgrUpdateThread = NULL;
        m_diagUpdate = FALSE;
        m_diagInfoSignal = NULL;
    }
    ~AisDiagManager()
    {

    }

    CameraResult Initialize();
    void Uninitialize();

public:
    static AisDiagManager* CreateInstance();
    static AisDiagManager *GetInstance();
    static void DestroyInstance();

    CameraResult QueryDiagnostics(void *pDiagInfo, uint32 diagInfoSize);
    CameraResult InitializeDiagstats();
    CameraResult InitSensorDeviceStats();
    CameraResult InitIfeStats();
    CameraResult InitCsiphyStats();
    void UpdateIfeRdiStatus(uint32 ifeCore, uint32 ifeRdiId, uint32 status);
    QCarCamDiagErrorInfo* GetErrorQueueTop();
    static int DiagInfoUpdate(void* arg);
    void GetCsiDeviceDiagInfo();
    void GetIfeDeviceDiagInfo();
};

#endif /* _AIS_DIAG_MGR_H_ */

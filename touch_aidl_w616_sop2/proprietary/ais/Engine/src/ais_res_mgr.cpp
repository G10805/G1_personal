/*!
 * Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include <errno.h>

#include "ais_res_mgr.h"
#include "ais_engine.h"
#include "ais_input_configurer.h"
#include "ais_ife_configurer.h"
#include "ais_buffer_manager.h"
#include "ais_diag_mgr.h"

#define IFEMAP_MASK 0xF
#define IFEMAP_SHFT 0x4
#define IFEMAP_MAX  0x8

class AisResourceManagerPrivate : public AisResourceManager
{
public:
    AisResourceManagerPrivate();

    virtual CameraResult Init(void);
    virtual CameraResult Deinit(void);

    virtual CameraResult Reserve(AisUsrCtxt* pUsrCtxt);
    virtual CameraResult Release(AisUsrCtxt* pUsrCtxt);
    virtual const AisResMgrIfeResourceType* GetIfeResourcesInfo();
    virtual const CameraCsiInfo* GetCsiResourcesInfo();

    virtual CameraResult MatchUserList(AisResMgrMatch* pMatch, AisUsrCtxtList* pList);

private:
    void ResetResourceTable();
    IfeOutputPathType GetIfeOutputFromInterface(IfeInterfaceType ifeInterf);

    CameraResult ReserveStream(AisUsrCtxt* pUsrCtxt, uint32 streamIdx);
    CameraResult ReserveStreamUser(AisUsrCtxt* pUsrCtxt, uint32 streamIdx);
    IfeInterfaceType FindStreamInterface(uint32 ifeCore, IfeInterfaceType minIfeInterface, IfeInterfaceType maxIfeInterface);

    CameraResult MatchUserListIFE(AisResMgrMatch* pMatch, AisUsrCtxtList* pList);
    CameraResult MatchUserListCSI(AisResMgrMatch* pMatch, AisUsrCtxtList* pList);
    CameraResult MatchUserListInput(AisResMgrMatch* pMatch, AisUsrCtxtList* pList);

    static const uint32 maxNumUserResources = 16;

    AisResMgrIfeResourceType m_ifeResources[IFE_CORE_MAX];
    CameraCsiInfo m_csiResources[CSIPHY_CORE_MAX];
    AisResMgrUserResourceType m_userResources[maxNumUserResources];
    CameraMutex   m_mutex;
};


///@brief AisResourceManager singleton
AisResourceManager* AisResourceManager::m_pResourceManagerInstance = nullptr;

AisResourceManager* AisResourceManager::CreateInstance()
{
    if(m_pResourceManagerInstance == nullptr)
    {
        m_pResourceManagerInstance = new AisResourceManagerPrivate();
        if (m_pResourceManagerInstance)
        {
            CameraResult rc = m_pResourceManagerInstance->Init();
            if (rc != CAMERA_SUCCESS)
            {
                delete m_pResourceManagerInstance;
                m_pResourceManagerInstance = NULL;
            }
        }
    }

    return m_pResourceManagerInstance;
}

AisResourceManager* AisResourceManager::GetInstance()
{
    return m_pResourceManagerInstance;
}

void AisResourceManager::DestroyInstance()
{
    if(m_pResourceManagerInstance != nullptr)
    {
        m_pResourceManagerInstance->Deinit();

        delete m_pResourceManagerInstance;
        m_pResourceManagerInstance = nullptr;
    }
}

AisResourceManagerPrivate::AisResourceManagerPrivate()
{
    m_mutex = NULL;

    ResetResourceTable();
}

void AisResourceManagerPrivate::ResetResourceTable()
{
    uint32 csiphyCore = 0;
    uint32 ifeCore = 0;
    CameraResult rc = CAMERA_SUCCESS;

    memset(m_ifeResources, 0x0, sizeof(m_ifeResources));
    memset(m_userResources, 0x0, sizeof(m_userResources));

    for (csiphyCore = 0; csiphyCore < CSIPHY_CORE_MAX; csiphyCore++)
    {
        rc = CameraPlatformGetCsiIfeMap(csiphyCore, &m_csiResources[csiphyCore]);
        if (rc != CAMERA_SUCCESS)
        {
            AIS_LOG(ENGINE, ERROR, "Could not get IFE interface for csiphy: 0x%x", csiphyCore);
            continue;
        }
    }

    for (ifeCore = 0; ifeCore < IFE_CORE_MAX; ifeCore++)
    {
        uint32 j = 0;

        m_ifeResources[ifeCore].csiPhy = CSIPHY_CORE_MAX;

        const IFECapabilitiesType* pIfeCaps =
            AisIFEConfigurer::GetInstance()->GetIfeCapabilitiesInfo(ifeCore);

        if (pIfeCaps)
        {
            m_ifeResources[ifeCore].numRdi = pIfeCaps->numRdi;
        }
        else
        {
            m_ifeResources[ifeCore].numRdi = 0;
        }

        for (j = IFE_INTF_RDI0; j < (IFE_INTF_RDI0 + m_ifeResources[ifeCore].numRdi); j++)
        {
            m_ifeResources[ifeCore].interface[j].output =
                GetIfeOutputFromInterface((IfeInterfaceType)(j));
        }
    }
}

CameraResult AisResourceManagerPrivate::Init()
{
    CameraResult rc;

    rc = CameraCreateMutex(&m_mutex);

    return rc;
}

CameraResult AisResourceManagerPrivate::Deinit()
{
    if (m_mutex)
    {
        CameraDestroyMutex(m_mutex);
    }

    return CAMERA_SUCCESS;
}

CameraResult AisResourceManagerPrivate::Reserve(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;

    CameraLockMutex(m_mutex);

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams && CAMERA_SUCCESS == rc; streamIdx++)
    {
        if (pUsrCtxt->m_streams[streamIdx].type == AIS_STREAM_TYPE_IFE)
        {
            rc = ReserveStream(pUsrCtxt, streamIdx);
        }
        else if (pUsrCtxt->m_streams[streamIdx].type == AIS_STREAM_TYPE_USER)
        {
            rc = ReserveStreamUser(pUsrCtxt, streamIdx);
        }
    }

    CameraUnlockMutex(m_mutex);

    return rc;
}

CameraResult AisResourceManagerPrivate::ReserveStreamUser(AisUsrCtxt* pUsrCtxt, uint32 streamIdx)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisInputInterfaceType inputIntf;
    uint32 i;

    inputIntf.inputId = pUsrCtxt->m_inputId;
    inputIntf.streamIdx = streamIdx;
    rc = AisInputConfigurer::GetInstance()->GetInterface(&inputIntf);
    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "input %d not supported", pUsrCtxt->m_inputId);
        return CAMERA_ERESOURCENOTFOUND;
    }

    for (i = 0; i < maxNumUserResources; i++)
    {
        if (NULL == m_userResources[i].pUser)
        {
            m_userResources[i].inputDevId   = inputIntf.stream.devId;
            m_userResources[i].inputSrcId   = inputIntf.stream.srcId;
            m_userResources[i].pUser        = pUsrCtxt;
            break;
        }
    }
    if (i >= maxNumUserResources)
    {
        AIS_LOG(ENGINE, ERROR, "No more user resources available");
        return CAMERA_ENOMORE;
    }

    return rc;
}

CameraResult AisResourceManagerPrivate::ReserveStream(AisUsrCtxt* pUsrCtxt, uint32 streamIdx)
{
    CameraResult rc = CAMERA_SUCCESS;
    AisInputInterfaceType inputIntf;
    uint32 csiphyCore = CSIPHY_CORE_MAX;
    uint32 ifeCore = IFE_CORE_MAX;
    uint32 i = 0;
    boolean bFound = FALSE;

    inputIntf.inputId = pUsrCtxt->m_inputId;
    inputIntf.streamIdx = streamIdx;
    rc = AisInputConfigurer::GetInstance()->GetInterface(&inputIntf);
    if (CAMERA_SUCCESS != rc)
    {
        AIS_LOG(ENGINE, ERROR, "input %d not supported", pUsrCtxt->m_inputId);
        return CAMERA_ERESOURCENOTFOUND;
    }

    csiphyCore = inputIntf.stream.csiphy;

    uint32 numMapping = 1;
    uint32 ifeMapping = csiphyCore;

    if (m_csiResources[csiphyCore].numIfeMap &&
        m_csiResources[csiphyCore].numIfeMap <= IFEMAP_MAX)
    {
        numMapping =  m_csiResources[csiphyCore].numIfeMap;
        ifeMapping = m_csiResources[csiphyCore].ifeMap;
    }

    //Find available IFE interface
    for (i = 0; i < numMapping && !bFound; i++)
    {
        ifeCore = (ifeMapping >> (IFEMAP_SHFT * i)) & IFEMAP_MASK;
        if (ifeCore >= IFE_CORE_MAX)
        {
            AIS_LOG(ENGINE, ERROR, "Invalid ife core %d in csi %d mapping ", ifeCore, csiphyCore);
            return CAMERA_ERESOURCENOTFOUND;
        }

        if (m_ifeResources[ifeCore].csiPhy != csiphyCore &&
            m_ifeResources[ifeCore].csiPhy != CSIPHY_CORE_MAX)
        {
            continue;
        }
        IfeInterfaceType ifeInterface_1;
        if (QCARCAM_OPMODE_RDI_CONVERSION == pUsrCtxt->m_opMode)
        {
            if (0 == streamIdx)
            {
                ifeInterface_1 = FindStreamInterface(ifeCore, IFE_INTF_RDI0, IFE_INTF_RDI2);
            }
            else
            {
                ifeInterface_1 = FindStreamInterface(ifeCore, IFE_INTF_RDI2, (IfeInterfaceType)(IFE_INTF_RDI0 + m_ifeResources[ifeCore].numRdi));
            }
        }
        else
        {
            ifeInterface_1 = FindStreamInterface(ifeCore, IFE_INTF_RDI0, (IfeInterfaceType)(IFE_INTF_RDI0 + m_ifeResources[ifeCore].numRdi));
        }
        if (ifeInterface_1 < IFE_INTF_MAX)
        {
            m_ifeResources[ifeCore].intrfUseMask |= (1U << ifeInterface_1);
            m_ifeResources[ifeCore].interface[ifeInterface_1].pUser = pUsrCtxt;
            m_ifeResources[ifeCore].interface[ifeInterface_1].inputSrcId = inputIntf.stream.srcId;
            m_ifeResources[ifeCore].csiPhy = csiphyCore;
            m_ifeResources[ifeCore].inputDevId = inputIntf.stream.devId;
            AisDiagManager::GetInstance()->UpdateIfeRdiStatus(ifeCore, (uint32)ifeInterface_1, 1);

            AisIfeStreamType* pIfePathInfo = &pUsrCtxt->m_streams[streamIdx].resources.ifeStream;

            pIfePathInfo->ifeCore = (IfeCoreType)ifeCore;
            pIfePathInfo->ifeInterf = (IfeInterfaceType)ifeInterface_1;
            pIfePathInfo->ifeOutput = m_ifeResources[ifeCore].interface[ifeInterface_1].output;
            pIfePathInfo->bufferListIdx = pUsrCtxt->m_pProcChainDef->streams[streamIdx].buflistId;

            pUsrCtxt->m_streams[streamIdx].resources.csiphy = inputIntf.stream.csiphy;
            pUsrCtxt->m_streams[streamIdx].resources.csid = ifeCore;
            pUsrCtxt->m_streams[streamIdx].resources.cid = inputIntf.stream.cid;

            AIS_LOG(ENGINE, HIGH,  "ife core %d interf %d out %d - dev %d src %d - cid %d csi 0x%x",
                       pIfePathInfo->ifeCore, pIfePathInfo->ifeInterf, pIfePathInfo->ifeOutput,
                       pUsrCtxt->m_streams[streamIdx].inputCfg.devId, pUsrCtxt->m_streams[streamIdx].inputCfg.srcId,
                       pUsrCtxt->m_streams[streamIdx].resources.cid, pUsrCtxt->m_streams[streamIdx].resources.csiphy);
            bFound = TRUE;
        }
    }
    if (!bFound)
    {
        AIS_LOG(ENGINE, ERROR, "No IFE resource available");
        rc = CAMERA_ERESOURCENOTFOUND;
    }
    return rc;
}

IfeInterfaceType AisResourceManagerPrivate::FindStreamInterface(uint32 ifeCore, IfeInterfaceType minIfeInterface, IfeInterfaceType maxIfeInterface)
{
    IfeInterfaceType ifeInterface;
    for (ifeInterface = minIfeInterface; ifeInterface < maxIfeInterface; ifeInterface = (IfeInterfaceType)(ifeInterface + 1))
    {
        if (!(m_ifeResources[ifeCore].intrfUseMask & (1U << ifeInterface)) &&
            m_ifeResources[ifeCore].interface[ifeInterface].output != IFE_OUTPUT_PATH_MAX)
        {
           return  ifeInterface;
        }
    }
    return IFE_INTF_MAX;
}



CameraResult AisResourceManagerPrivate::Release(AisUsrCtxt* pUsrCtxt)
{
    CameraResult rc = CAMERA_SUCCESS;
    uint32 streamIdx;
    uint32 ifeInterface;
    uint32 ifeCore;
    boolean bFound = FALSE;

    CameraLockMutex(m_mutex);

    for (streamIdx = 0; streamIdx < pUsrCtxt->m_numStreams; streamIdx++)
    {
        AisUsrCtxtStreamType* pStream = &pUsrCtxt->m_streams[streamIdx];

        if (AIS_STREAM_TYPE_IFE == pStream->type)
        {
            bFound = FALSE;
            ifeCore = pStream->resources.ifeStream.ifeCore;
            ifeInterface = pStream->resources.ifeStream.ifeInterf;

            if (ifeCore < IFE_CORE_MAX && ifeInterface < IFE_INTF_MAX && ifeInterface < (IFE_INTF_RDI0 + m_ifeResources[ifeCore].numRdi))
            {
                if ((m_ifeResources[ifeCore].intrfUseMask & (1U << ifeInterface)) &&
                    (m_ifeResources[ifeCore].interface[ifeInterface].pUser == pUsrCtxt))
                {
                    bFound = TRUE;

                    m_ifeResources[ifeCore].intrfUseMask &= ~(1U << ifeInterface);
                    m_ifeResources[ifeCore].interface[ifeInterface].pUser = NULL;
                    AisDiagManager::GetInstance()->UpdateIfeRdiStatus(ifeCore, (uint32)ifeInterface, 0);
                    if (m_ifeResources[ifeCore].intrfUseMask == 0)
                    {
                        m_ifeResources[ifeCore].csiPhy = CSIPHY_CORE_MAX;
                    }
                }
            }
        }
        else if (AIS_STREAM_TYPE_USER == pStream->type)
        {
            for (uint32 i = 0; i < maxNumUserResources; i++)
            {
                if (pUsrCtxt == m_userResources[i].pUser)
                {
                    bFound = TRUE;
                    memset(&(m_userResources[i]), 0x0, sizeof(m_userResources[0]));
                }
            }
        }

        if (!bFound)
        {
            AIS_LOG(ENGINE, ERROR, "Did not find input resource to free");
            rc = CAMERA_ERESOURCENOTFOUND;
        }

        memset(&pStream->resources, 0x0, sizeof(pUsrCtxt->m_streams[streamIdx].resources));
    }

    CameraUnlockMutex(m_mutex);

    return rc;
}

IfeOutputPathType AisResourceManagerPrivate::GetIfeOutputFromInterface(IfeInterfaceType ifeInterf)
{
    IfeOutputPathType eOutput = IFE_OUTPUT_PATH_MAX;

    switch(ifeInterf)
    {
    case IFE_INTF_RDI0:
        eOutput = IFE_OUTPUT_PATH_RDI0;
        break;
    case IFE_INTF_RDI1:
        eOutput = IFE_OUTPUT_PATH_RDI1;
        break;
    case IFE_INTF_RDI2:
        eOutput = IFE_OUTPUT_PATH_RDI2;
        break;
    case IFE_INTF_RDI3:
        eOutput = IFE_OUTPUT_PATH_RDI3;
        break;
    default:
        eOutput = IFE_OUTPUT_PATH_MAX;
        break;
    }

    return eOutput;
}

const AisResMgrIfeResourceType* AisResourceManagerPrivate::GetIfeResourcesInfo()
{
    return m_ifeResources;
}

const CameraCsiInfo* AisResourceManagerPrivate::GetCsiResourcesInfo()
{
    return m_csiResources;
}

CameraResult AisResourceManagerPrivate::MatchUserListIFE(AisResMgrMatch* pMatch, AisUsrCtxtList* pList)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (pMatch->device >= IFE_CORE_MAX || pMatch->path >= IFE_OUTPUT_PATH_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid IFE dev%d path%d", pMatch->device, pMatch->path);
        return CAMERA_EBADPARM;
    }

    CameraLockMutex(m_mutex);
    switch(pMatch->matchType)
    {
    case AIS_RESMGR_MATCH_IFE_PATH:
    {
        AisUsrCtxt* pUsrCtxt = m_ifeResources[pMatch->device].interface[pMatch->path].pUser;
        if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
        {
            pList->push_back(pUsrCtxt);
        }
        else
        {
            AIS_LOG(ENGINE, ERROR, "Failed to match IFE dev%d path%d", pMatch->device, pMatch->path);
        }
        break;
    }
    case AIS_RESMGR_MATCH_IFE_DEVICE:
    {
        uint32 ifeInterface;
        uint32 ifeCore = pMatch->device;
        for (ifeInterface = IFE_INTF_RDI0; ifeInterface < IFE_INTF_MAX; ifeInterface++)
        {
            AisUsrCtxt* pUsrCtxt = m_ifeResources[ifeCore].interface[ifeInterface].pUser;
            if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
            {
                pList->push_back(pUsrCtxt);
            }
        }
        break;
    }
    case AIS_RESMGR_MATCH_CSI_DEVICE:
    {
        uint32 ifeInterface;
        uint32 ifeCore;
        uint32 csiCore = m_ifeResources[pMatch->device].csiPhy;

        for (ifeCore = 0; ifeCore < IFE_CORE_MAX; ifeCore++)
        {
            if (m_ifeResources[ifeCore].csiPhy != csiCore)
                continue;

            for (ifeInterface = IFE_INTF_RDI0; ifeInterface < m_ifeResources[ifeCore].numRdi; ifeInterface++)
            {
                AisUsrCtxt* pUsrCtxt = m_ifeResources[ifeCore].interface[ifeInterface].pUser;
                if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
                {
                    pList->push_back(pUsrCtxt);
                }
            }
        }
        break;
    }
    case AIS_RESMGR_MATCH_INPUT_DEVICE:
    {
        uint32 ifeInterface;
        uint32 ifeCore;
        uint32 inputDev = m_ifeResources[pMatch->device].inputDevId;

        for (ifeCore = 0; ifeCore < IFE_CORE_MAX; ifeCore++)
        {
            if (m_ifeResources[ifeCore].inputDevId != inputDev)
                continue;

            for (ifeInterface = IFE_INTF_RDI0; ifeInterface < m_ifeResources[ifeCore].numRdi; ifeInterface++)
            {
                AisUsrCtxt* pUsrCtxt = m_ifeResources[ifeCore].interface[ifeInterface].pUser;
                if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
                {
                    pList->push_back(pUsrCtxt);
                }
            }
        }
        break;
    }
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported matchType %d", pMatch->matchType);
        rc = CAMERA_EUNSUPPORTED;
        break;
    }
    CameraUnlockMutex(m_mutex);
    return rc;
}

CameraResult AisResourceManagerPrivate::MatchUserListCSI(AisResMgrMatch* pMatch, AisUsrCtxtList* pList)
{
    CameraResult rc = CAMERA_SUCCESS;

    if (pMatch->device >= IFE_CORE_MAX || pMatch->path >= IFE_OUTPUT_PATH_MAX)
    {
        AIS_LOG(ENGINE, ERROR, "Invalid IFE dev%d path%d", pMatch->device, pMatch->path);
        return CAMERA_EBADPARM;
    }

    CameraLockMutex(m_mutex);
    switch(pMatch->matchType)
    {
    case AIS_RESMGR_MATCH_CSI_DEVICE:
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported matchType %d", pMatch->matchType);
        rc = CAMERA_EUNSUPPORTED;
        break;
    }
    CameraUnlockMutex(m_mutex);
    return rc;
}

CameraResult AisResourceManagerPrivate::MatchUserListInput(AisResMgrMatch* pMatch, AisUsrCtxtList* pList)
{
    CameraResult rc = CAMERA_SUCCESS;

    CameraLockMutex(m_mutex);
    switch(pMatch->matchType)
    {
    case AIS_RESMGR_MATCH_INPUT_SRC:
    {
        uint32 ifeInterface;
        uint32 ifeCore;
        for (ifeCore = 0; ifeCore < IFE_CORE_MAX; ifeCore++)
        {
            if (m_ifeResources[ifeCore].inputDevId != pMatch->device)
                continue;

            for (ifeInterface = IFE_INTF_RDI0; ifeInterface < m_ifeResources[ifeCore].numRdi; ifeInterface++)
            {
                if (m_ifeResources[ifeCore].interface[ifeInterface].inputSrcId != pMatch->path)
                    continue;

                AisUsrCtxt* pUsrCtxt = m_ifeResources[ifeCore].interface[ifeInterface].pUser;

                if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
                {
                    pList->push_back(pUsrCtxt);
                }
            }
        }
        for (uint32 i = 0; i < maxNumUserResources; i++)
        {
            if ((pMatch->device == m_userResources[i].inputDevId) &&
                (pMatch->path == m_userResources[i].inputSrcId))
            {
                AisUsrCtxt* pUsrCtxt = m_userResources[i].pUser;
                if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
                {
                    pList->push_back(pUsrCtxt);
                }
            }
        }
        break;
    }
    case AIS_RESMGR_MATCH_INPUT_DEVICE:
    {
        uint32 ifeInterface;
        uint32 ifeCore;
        for (ifeCore = 0; ifeCore < IFE_CORE_MAX; ifeCore++)
        {
            if (m_ifeResources[ifeCore].inputDevId != pMatch->device)
                continue;

            for (ifeInterface = IFE_INTF_RDI0; ifeInterface < m_ifeResources[ifeCore].numRdi; ifeInterface++)
            {
                AisUsrCtxt* pUsrCtxt = m_ifeResources[ifeCore].interface[ifeInterface].pUser;
                if (pUsrCtxt && (CAMERA_SUCCESS == pUsrCtxt->IncRefCnt()))
                {
                    pList->push_back(pUsrCtxt);
                }
            }
        }
        break;
    }
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported matchType %d", pMatch->matchType);
        rc = CAMERA_EUNSUPPORTED;
        break;
    }
    CameraUnlockMutex(m_mutex);
    return rc;
}

/**
 * Adds all AisUsrCtxt that match criteria to a list
 *
 * @NOTE: the function will IncRefCnt of the AisUsrCtxt added to the list
 *        It is the responsibility of the caller to decRefCnt when done with the list
 *
 * @param [in]pMatch     Matching criteria
 * @param [in/out]pList  List to be filled with matching user contexts based on criteria
 *
 * @return CameraResult    ; //List to be filled based on matching criteria above
 */
CameraResult AisResourceManagerPrivate::MatchUserList(AisResMgrMatch* pMatch, AisUsrCtxtList* pList)
{
    CameraResult rc = CAMERA_SUCCESS;

    switch(pMatch->dataType)
    {
    case AIS_RESMGR_MATCH_DATA_IFE:
    {
        rc = MatchUserListIFE(pMatch, pList);
        break;
    }
    case AIS_RESMGR_MATCH_DATA_CSI:
    {
        rc = MatchUserListCSI(pMatch, pList);
        break;
    }
    case AIS_RESMGR_MATCH_DATA_INPUT:
    {
        rc = MatchUserListInput(pMatch, pList);
        break;
    }
    default:
        AIS_LOG(ENGINE, ERROR, "Unsupported match dataType %d", pMatch->dataType);
        rc = CAMERA_EUNSUPPORTED;
        break;
    }

    return rc;
}

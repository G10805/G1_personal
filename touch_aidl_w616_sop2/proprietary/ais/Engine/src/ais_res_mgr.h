#ifndef _AIS_RES_MGR_H_
#define _AIS_RES_MGR_H_

/*!
 * Copyright (c) 2016-2017, 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <list>
#include "ais_engine.h"

typedef struct
{
    IfeOutputPathType output;
    AisUsrCtxt* pUser;
    uint32      inputSrcId;
} AisResMgrIfeIntfType;

typedef struct
{
    CsiphyCoreType csiPhy;
    uint32 inputDevId;

    uint32 intrfUseMask;
    AisResMgrIfeIntfType interface[IFE_INTF_MAX];
    uint32 numRdi;
} AisResMgrIfeResourceType;

typedef struct
{
    uint32      inputDevId;
    uint32      inputSrcId;
    AisUsrCtxt* pUser;
} AisResMgrUserResourceType;

/**
 * AisResMgrMatchType
 *
 * Specified criteria as to which user contexts we are matching against
 * This enum should match CameraConfigMatchType in CameraConfig.h
 */
typedef enum
{
    AIS_RESMGR_MATCH_IFE_PATH = 0,  //match user of specific IFE output path
    AIS_RESMGR_MATCH_IFE_DEVICE,    //match all users of an IFE device
    AIS_RESMGR_MATCH_CSI_DEVICE,    //match all users of a CSIPHY device
    AIS_RESMGR_MATCH_INPUT_SRC,     //match all users of a specific input source
    AIS_RESMGR_MATCH_INPUT_DEVICE   //match all users of an input device
}AisResMgrMatchType;

/**
 * AisResMgrMatchDataType
 *
 * Specified criteria as to which user contexts we are matching against
 */
typedef enum
{
    AIS_RESMGR_MATCH_DATA_IFE,
    AIS_RESMGR_MATCH_DATA_CSI,
    AIS_RESMGR_MATCH_DATA_INPUT
}AisResMgrMatchDataType;

/**
 * AisResMgrMatch
 *
 * Specified matching criteria for Match function
 */
typedef struct
{
    AisResMgrMatchType matchType; //what user contexts will be matched

    AisResMgrMatchDataType dataType; //type of device information below
    uint32 device;                   //device ID to match against
    uint32 path;                     //path ID to match against
}AisResMgrMatch;

class AisUsrCtxt;

class AisResourceManager
{
protected:
    virtual ~AisResourceManager(){};

    static AisResourceManager* m_pResourceManagerInstance;

public:
    static AisResourceManager* CreateInstance();
    static AisResourceManager* GetInstance();
    static void DestroyInstance();

    virtual CameraResult Init(void) = 0;
    virtual CameraResult Deinit(void) = 0;

    virtual CameraResult Reserve(AisUsrCtxt* pUsrCtxt) = 0;
    virtual CameraResult Release(AisUsrCtxt* pUsrCtxt) = 0;
    virtual const AisResMgrIfeResourceType* GetIfeResourcesInfo() = 0;
    virtual const CameraCsiInfo* GetCsiResourcesInfo() = 0;

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
    virtual CameraResult MatchUserList(AisResMgrMatch* pMatch, AisUsrCtxtList* pList) = 0;
};

#endif /* _AIS_RES_MGR_H_ */

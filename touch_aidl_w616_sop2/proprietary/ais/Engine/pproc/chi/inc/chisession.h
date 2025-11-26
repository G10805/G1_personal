//*************************************************************************************************
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

#ifndef __CHISESSION_H__
#define __CHISESSION_H__

#include "CameraResult.h"
#include "chi.h"


// Constants
//
static const int MaxPipelinesPerSession = 3;

// Forward decalarations
//
class ChiPipeline;
class ChiModule;

typedef struct _SessionCreateData
{
    CHIPIPELINEINFO     pPipelinesInfo[MaxPipelinesPerSession];  // Chi pipeline info
    int                 numPipelines;                           // Number of pipelines in the session
    CHICALLBACKS*       pCallbacks;                             // Chi callbacks
    void*               pPrivCallbackData;                      // Private data for the callbacks
} SessionCreateData;


// This class implements session related functionality.
//
class ChiSession
{
public:
    // This methods creates chi pipeline object.
    //
    // param [in]: pPipelines Pointer to pipeline objects.
    // param [in]: numPipelines Number of pipelines in pPipelines.
    // param [in]: pCallbacks Callbacks to chi app.
    // param [in]: pPrivData pointer to private data from app.
    // param [in]: pChiModule Pointer to ChiModule object
    //
    // returns: ChiPipeline* Pointer to ChiPipeline instance if successful
    //                     or NULL otherwise.
    //
    static ChiSession*      Create(ChiPipeline**    ppPipelines,
                                     int            numPipelines,
                                     CHICALLBACKS*  pCallbacks,
                                     void*          pPrivData,
                                     ChiModule*     pChiModule);

    // This method sends request to camx to destroy session.
    //
    // returns: none.
    //
    void                    DestroySession();

    void                    Flush();

    // This mehtod obtains chi session handle
    //
    // returns: CHIHANDLE of previously created session.
    //
    CHIHANDLE               GetSessionHandle() const { return m_sessionHandle; }

private:
    // This method initializes internal data of chi session object.
    //
    // param [in]: pPipelines Pointer to pipeline objects.
    // param [in]: numPipelines Number of pipelines in pPipelines.
    // param [in]: pCallbacks Callbacks to chi app.
    // param [in]: pPrivData pointer to private data from app.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult            Initialize(ChiPipeline**     ppPipelines,
                                          int            numPipelines,
                                          CHICALLBACKS*  pCallbacks,
                                          void*          pPrivData);

    ChiModule*              m_pChiModule;           // pointer to the ChiModule
    SessionCreateData       m_sessionCreateData;    // data required for session creation
    CHIHANDLE               m_sessionHandle;        // chi session handle

    ChiSession(ChiModule*    pChiModule);
    ~ChiSession();

    // Do not allow the copy constructor or assignment operator
    ChiSession(const ChiSession& ) = delete;
    ChiSession& operator= (const ChiSession& ) = delete;

};


#endif // __CHISESSION_H__

//*************************************************************************************************
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//*************************************************************************************************

//
// This class implements all handling and communication to CHI interface.
//

#ifndef __CHIMODULE_H__
#define __CHIMODULE_H__

#include "chi.h"
#include "CameraResult.h"

#define EXPECT_CHI_API_MAJOR_VERSION   3
#define EXPECT_CHI_API_MINOR_VERSION   0

class ChiModule
{
public:
    // This method creates instance for ChiModule. It calls
    // internaly constructor and allocates required resources.
    //
    // returns: ChiModule* Pointer to ChiModule instance if successful
    //                     or NULL otherwise.
    //
    static ChiModule*   CreateInstance();


    // This method returns pointer to ChiOps that contains pointers to
    // all chi related APIs.
    //
    // returns: CHICONTEXTOPS* Pointer to previously obtained ChiOps object.
    //
    CHICONTEXTOPS*      GetChiOps() { return &m_chiOps; }

    // This method returns pointer to ChiOps that contains pointers to
    // all chi related APIs.
    //
    // returns: CHICONTEXTOPS* Pointer to previously obtained ChiOps object.
    //
    CHIHANDLE           GetContext() { return m_hContext; }

    ~ChiModule();

private:
    ChiModule();

    /// Do not allow the copy constructor or assignment operator
    ChiModule(const ChiModule& ) = delete;
    ChiModule& operator= (const ChiModule& ) = delete;

    CameraResult        Initialize();

    // This method loads camx shared library and obtains chi entry point.
    //
    // returns: CameraResult indicating success or failure of this method.
    //
    CameraResult        LoadLibraries();

    CameraResult        OpenContext();
    CameraResult        CloseContext();

    CHIHANDLE           m_hContext;         // Handle to the context
    CHICONTEXTOPS       m_chiOps;           // function pointers to all chi related APIs
    void*               m_hLibrary;         // pointer to the loaded driver .so library
};

#endif // __CHIMODULE_H__

//******************************************************************************************************************************
// Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

//
// Implementation of CHI module
//

#include <string.h>
#include <dlfcn.h>

#ifdef __QNXNTO__
#include <errno.h>
#include <sys/pathmgr.h>
#endif //__QNXNTO__

#include "chimodule.h"
#include "ais_log.h"

#define ISP_LOG(...) AIS_LOG(PPROC_ISP, ##__VA_ARGS__)

ChiModule* ChiModule::CreateInstance()
{
    ChiModule* pModuleInstance = new ChiModule();
    if (pModuleInstance != NULL)
    {
        if (pModuleInstance->Initialize() != CAMERA_SUCCESS)
        {
            ISP_LOG(ERROR, "Failed to initialize ChiModule.");
            delete pModuleInstance;
            return NULL;
        }
    }
    return pModuleInstance;
}

ChiModule::ChiModule() :
    m_hContext(NULL), m_chiOps{}, m_hLibrary(NULL)
{
}

ChiModule::~ChiModule()
{
    CloseContext();

    if (m_hLibrary != NULL)
    {
        dlclose(m_hLibrary);
        m_hLibrary = NULL;
    }
}

CameraResult ChiModule::Initialize()
{
    CameraResult rc = CAMERA_SUCCESS;

    // Load camx shared library and obtain function pointers
    // to chi related apis.
    //
    rc = LoadLibraries();
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "LoadLibraries() failed");
        return rc;
    }
    ISP_LOG(DBG, "LoadLibraries() successfully completed");

    // Initialize chi contex required for subsequent chi apis.
    //
    rc = OpenContext();
    if (rc != CAMERA_SUCCESS)
    {
        ISP_LOG(ERROR, "OpenContext() failed");
        return rc;
    }
    ISP_LOG(DBG, "OpenContext() successfully completed");
    return rc;
}

CameraResult ChiModule::LoadLibraries()
{
    CameraResult rc = CAMERA_SUCCESS;
#if defined(CAMERA_UNITTEST)
    PCHIENTRY pChiEntry = &ChiEntry;
#else
    char sharedLibName[80] = {0};
    uint32 openflag = RTLD_NOW | RTLD_GLOBAL;

#if defined(__AGL__)
#if defined(USELIB64)
    strlcpy(sharedLibName, "/usr/lib64/hw/camera.qcom.so", sizeof(sharedLibName));
#else
    strlcpy(sharedLibName, "/usr/lib/hw/camera.qcom.so", sizeof(sharedLibName));
#endif
#elif defined(__QNXNTO__)
    if (!access("/lib64/camera/libcamera.qcom.so", 0))
    {
        strlcpy(sharedLibName, "/lib64/camera/libcamera.qcom.so", sizeof(sharedLibName));
    }
    else if (!access("/ifs/lib64/camera/libcamera.qcom.so", 0))
    {
        strlcpy(sharedLibName, "/ifs/lib64/camera/libcamera.qcom.so", sizeof(sharedLibName));
        if (pathmgr_symlink("/ifs/lib64/camera", "/lib64/camera") == -1)
        {
            ISP_LOG(ERROR,
                "Failed to create link for /ifs/lib64/camera (%s).", strerror(errno));
        }

        if (pathmgr_symlink("/ifs/bin/camera", "/bin/camera") == -1)
        {
            ISP_LOG(ERROR,
                "Failed to create link for /ifs/bin/camera (%s).", strerror(errno));
        }
    }
    else if (!access("/mnt/lib64/camera/libcamera.qcom.so", 0))
    {
        strlcpy(sharedLibName, "/mnt/lib64/camera/libcamera.qcom.so", sizeof(sharedLibName));
        if (pathmgr_symlink("/mnt/lib64/camera", "/lib64/camera") == -1)
        {
            ISP_LOG(ERROR,
                "Failed to create link for /mnt/lib64/camera (%s).", strerror(errno));
        }

        if (pathmgr_symlink("/mnt/bin/camera", "/bin/camera") == -1)
        {
            ISP_LOG(ERROR,
                "Failed to create link for /mnt/bin/camera (%s).", strerror(errno));
        }
    }
    else
    {
        ISP_LOG(ERROR, "Failed to access libcamera.qcom.so");
        return CAMERA_EFAILED;
    }
#else
#if defined(_LP64)
    strlcpy(sharedLibName, "/vendor/lib64/hw/camera.qcom.so", sizeof(sharedLibName));
#else
    strlcpy(sharedLibName, "/vendor/lib/hw/camera.qcom.so", sizeof(sharedLibName));
#endif
#endif

    m_hLibrary = dlopen(sharedLibName, openflag);

    if (NULL == m_hLibrary)
    {
        ISP_LOG(ERROR, "Failed to load %s (%s)", sharedLibName, dlerror());
        return CAMERA_EUNABLETOLOAD;
    }
    ISP_LOG(DBG, "Successfully loaded %s.", sharedLibName);

    PCHIENTRY pChiEntry = reinterpret_cast<PCHIENTRY>(dlsym(m_hLibrary, "ChiEntry"));
    if (NULL == pChiEntry)
    {
        ISP_LOG(ERROR, "ChiEntry missing in library.");
        dlclose(m_hLibrary);
        m_hLibrary = NULL;
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG, "Successfully obtained ChiEntry.");
#endif

    pChiEntry(&m_chiOps);

    return rc;
}

CameraResult ChiModule::OpenContext()
{
    CameraResult rc = CAMERA_SUCCESS;

    UINT32 majorVersion = m_chiOps.majorVersion;
    UINT32 minorVersion = m_chiOps.minorVersion;

    // Check chi interface compatibility
    //
    if ((EXPECT_CHI_API_MAJOR_VERSION == majorVersion) &&
        (EXPECT_CHI_API_MINOR_VERSION == minorVersion))
    {
        ISP_LOG(DBG,
            "Chi api version: major=%d, minor=%d, matches expected: major=%d, minor=%d.",
            majorVersion, minorVersion, EXPECT_CHI_API_MAJOR_VERSION, EXPECT_CHI_API_MINOR_VERSION);
    }
    else
    {
        ISP_LOG(ERROR,
            "Chi api version: major=%d, minor=%d, doesn't match expected: major=%d, minor=%d.",
            majorVersion, minorVersion, EXPECT_CHI_API_MAJOR_VERSION, EXPECT_CHI_API_MINOR_VERSION);
        return CAMERA_EFAILED;
    }

    m_hContext = m_chiOps.pOpenContext();
    if (NULL == m_hContext)
    {
        ISP_LOG(ERROR, "Failed to open context.");
        return CAMERA_EFAILED;
    }
    ISP_LOG(DBG, "Successfully opened context.");

    return rc;
}

CameraResult ChiModule::CloseContext()
{
    CameraResult rc = CAMERA_SUCCESS;

    if (m_hContext != NULL)
    {
        m_chiOps.pCloseContext(m_hContext);
        m_hContext = NULL;
        ISP_LOG(DBG, "Successfully closed context.");
    }

    return rc;
}

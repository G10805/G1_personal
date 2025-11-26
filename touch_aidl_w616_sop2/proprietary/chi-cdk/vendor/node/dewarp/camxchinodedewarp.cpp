////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxchinodedewarp.cpp
/// @brief node for EIS dewarp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CL/cl_ext_qcom.h>
#include <system/camera_metadata.h>
#ifdef ANDROID
#include <cutils/properties.h>
#endif //ANDROID
#include "camxchinodedewarp.h"
#include "camxutils.h"
#if !defined(CAMX_USE_GRALLOC1)
#include "camxgrallocutils.h"
#endif //!CAMX_USE_GRALLOC1

// NOWHINE FILE CP040: Keyword new not allowed. Use the CAMX_NEW/CAMX_DELETE functions insteads
// NOWHINE FILE CP006: Avoid STL keywordL: map

#undef  LOG_TAG
#define LOG_TAG "DEWARP"

#define PRINT_METADATA FALSE

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080
#define EXTRA_FRAMEWORK_BUFFERS 4
#define NUMBEROFPLANES 2

#define GRID_MESH_SIZE_X 32
#define GRID_MESH_SIZE_Y 24

#if defined (_LINUX)
const CHAR* pDefaultGLESv2LibraryName               = "libGLESv2_adreno";
const CHAR* pDefaultEGLLibraryName                  = "libEGL_adreno";

#elif defined (_WIN32)
const CHAR* pDefaultEGLLibraryName                  = "EGL_adreno";
const CHAR* pDefaultGLESv2LibraryName               = "GLESv2_adreno";

#else
#error Unsupported target defined
#endif // defined(_LINUX) || defined(_WIN32)


///< Vendor tag section names
static const CHAR EISDewarpMatrixSectionName[]      = "org.quic.camera2.ipeicaconfigs";
static const CHAR StreamTypePresent[]               = "org.quic.camera.streamTypePresent";

///< Vendor tags of interest
static CHIVENDORTAGDATA g_VendorTagSectionICAGpu[]  =
{
    { "ICAInPerspectiveTransformLookAhead", 0, sizeof(IPEICAPerspectiveTransform) },
    { "ICAInPerspectiveTransform", 0, sizeof(IPEICAPerspectiveTransform) },
    { "ExtraFrameworkBuffers", 0, sizeof(UINT32) },
    { "ICAInGridTransformLookahead", 0, sizeof(IPEICAGridTransform) },
    { "ICAInGridTransform", 0, sizeof(IPEICAGridTransform) },
};

static CHIVENDORTAGDATA g_VendorTagSectionstreamTypePresent[] =
{
    { "preview", 0, 1 },
};

static ExtraVendorTags g_vendorTagId = { 0 };

static ChiNodeInterface    g_ChiNodeInterface;                                       ///< The instance save the CAMX Chi interface
static UINT32              g_vendorTagBase                 = 0;                      ///< Chi assigned runtime
                                                                              ///  vendor tag base for the node

static const UINT32 ChiNodeMajorVersion             = 0;                      ///< The major version of CHI interface
static const UINT32 ChiNodeMinorVersion             = 0;                      ///< The minor version of CHI interface

static const CHAR   DewarpNodeSectionName[]         = "com.qti.node.dewarp";  ///< The section name for node

static const UINT32 DewarpNodeTagBase               = 0;                      ///< Tag base

// =============================================================================================================================
// OpenGL Stuff
// =============================================================================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GetInstance
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GPUOpenGL* GPUOpenGL::GetInstance()
{
    GPUOpenGL* pGpuOpenGL = CAMX_NEW GPUOpenGL();

    if (pGpuOpenGL->m_initStatus == GLInitInvalid)
    {
        pGpuOpenGL->Initialize();
    }

    return pGpuOpenGL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GPUOpenGL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GPUOpenGL::GPUOpenGL()
    : m_initStatus(GLInitInvalid)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::LoadProgram
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::LoadProgram(
    GLuint    &program)
{
    GLuint vertexShader     = 0;
    GLuint fragmentShader   = 0;

    CDKResult result        = CDKResultEFailed;

    result                  = CreateShader(vertexShader, GL_VERTEX_SHADER, m_pVertexShaderSource);
    if (CDKResultSuccess != result)
    {
        CHICDK_LOG_ERROR("CreateShader vertexShader failed");
        return CDKResultEFailed;
    }

    result                  = CreateShader(fragmentShader, GL_FRAGMENT_SHADER, m_pFragmentShaderSource);
    if (CDKResultSuccess != result)
    {
        CHICDK_LOG_ERROR("CreateShader fragmentShader failed");
        return CDKResultEFailed;
    }

    result                  = CreateProgram(program, vertexShader, fragmentShader);
    if (CDKResultSuccess != result)
    {
        CHICDK_LOG_ERROR("CreateProgram program failed");
        return CDKResultEFailed;
    }

    // Clean up. Delete linked shaders
    m_pfngldeleteshader(vertexShader);
    m_pfngldeleteshader(fragmentShader);
    m_pfnglfinish();

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::CreateShader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::CreateShader(
    GLuint            &shader,
    GLenum            shaderType,
    const GLchar*     pString)
{
    shader              = m_pfnglcreateshader(shaderType);
    m_pfnglshadersource(shader, 1, &pString, NULL);
    m_pfnglcompileshader(shader);
    GLint nResult       = GL_FALSE;

    m_pfnglgetshaderiv(shader, GL_COMPILE_STATUS, &nResult);

    if (GL_TRUE != nResult)
    {
        CHICDK_LOG_ERROR("Shader compilation status: %d", nResult);
        GLint nLength   = 0;
        m_pfnglgetshaderiv(shader, GL_INFO_LOG_LENGTH, &nLength);
        if (nLength > 0)
        {
            CHAR* pInfoLogStr = (CHAR*)malloc(nLength);
            m_pfnglgetshaderinfolog(shader, nLength, NULL, pInfoLogStr);
            CHICDK_LOG_ERROR("Shader compilation failure [%s]", pInfoLogStr);
            free(pInfoLogStr);
        }
        m_pfngldeleteshader(shader);
        return CDKResultEFailed;
    }
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::CheckglError
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::CheckglError(
    const CHAR* pOperation)
{
    GLint       error     = m_pfnglgeterror();
    CDKResult   result    = CDKResultSuccess;
    if (error)
    {
        CHICDK_LOG_ERROR("after %s() glError (0x%x)", pOperation, error);
        result = CDKResultEFailed;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::CheckEglError
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::CheckEglError(
    const CHAR* pOperation)
{
    GLint       error     = m_pfneglgeterror();
    CDKResult   result    = CDKResultSuccess;
    if (error != 0x3000)
    {
        CHICDK_LOG_ERROR("after %s() eglError (0x%x)", pOperation, error);
        result = CDKResultEFailed;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::CreateProgram
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::CreateProgram(
    GLuint    &program,
    GLuint    vertexShader,
    GLuint    fragmentShader)
{
    program                     = m_pfnglcreateprogram();
    m_pfnglattachshader(program, vertexShader);
    m_pfnglattachshader(program, fragmentShader);

    CheckglError("Attach shaders");
    m_pfngllinkprogram(program);

    GLint nResult               = GL_FALSE;
    m_pfnglgetprogramiv(program, GL_LINK_STATUS, &nResult);
    CheckglError("Link program");

    if (GL_TRUE != nResult)
    {
        CHICDK_LOG_ERROR("Shader link status %d", nResult);
        GLint nLength           = 0;
        m_pfnglgetprogramiv(program, GL_INFO_LOG_LENGTH, &nLength);

        if (nLength > 0)
        {
            CHAR* pInfoLogStr   = (CHAR*)malloc(nLength);
            m_pfnglgetprograminfolog(program, nLength, NULL, pInfoLogStr);
            CHICDK_LOG_ERROR("Shader link status error [%s]", pInfoLogStr);
            free(pInfoLogStr);
        }

        m_pfngldeleteprogram(program);
        return CDKResultEFailed;
    }

    // Detach the shader after linking
    m_pfngldetachshader(program, vertexShader);
    m_pfngldetachshader(program, fragmentShader);
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::EnableTextureUnit
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID GPUOpenGL::EnableTextureUnit(
    GLuint          program,
    const CHAR*     pImageUnitString,
    INT32           unitNum)
{
    GLuint sampler = m_pfnglgetuniformlocation(program, pImageUnitString);
    m_pfnglactivetexture(unitNum);
    m_pfngluniform1i(sampler, unitNum - GL_TEXTURE0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::Initialize()
{
    m_initStatus                = GLInitRunning;

    m_flags                     = 0x8 | 0x00400000;
    m_usage                     = GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_RENDER;

    CDKResult   result          = CDKResultSuccess;
    GLint       error           = CL_SUCCESS;
    GLint       numPlatforms    = 0;

    m_shaderProgram             = 0;
    m_posId                     = 0;
    m_texId                     = 0;
    m_transId                   = 0;
    m_pVertices                 = NULL;
    m_pTexCoordinates           = NULL;
    m_pIndices                  = NULL;
    m_indexVBOID                = 0;
    m_configDefault             = { DEFAULT_WIDTH, DEFAULT_HEIGHT, 8, 8, 8, 8, 32, 24, 8, 0, 0 };

    m_nNumMeshX                 = GRID_MESH_SIZE_X;
    m_nNumMeshY                 = GRID_MESH_SIZE_Y;
    m_nNumVerticesX             = (m_nNumMeshX + 1);
    m_nNumVerticesY             = (m_nNumMeshY + 1);
    m_nNumVertices              = m_nNumVerticesX * m_nNumVerticesY;

    result                      = InitializeFuncPtrs();

    if (CDKResultSuccess == result)
    {
        m_defaultDisplay = m_pfneglgetdisplay(EGL_DEFAULT_DISPLAY);
        m_pfneglinitialize(m_defaultDisplay, &m_majorVersion, &m_minorVersion);
        result = CreateContext(m_defaultDisplay, NULL, &m_defaultSurface, &m_defaultContext);

        if (CDKResultSuccess == result)
        {
            if (!m_pfneglmakecurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface, m_defaultContext))
            {
                CHICDK_LOG_ERROR("eglMakeCurrent failed with error code: %x", m_pfneglgeterror());
                result = CDKResultEFailed;
            }
            else
            {
                CHICDK_LOG_INFO("LoadProgram");
                result = LoadProgram(m_shaderProgram);

                if (CDKResultSuccess == result)
                {
                    m_pfngluseprogram(m_shaderProgram);
                    m_posId     = m_pfnglgetattriblocation(m_shaderProgram, "inPosition");
                    m_texId     = m_pfnglgetattriblocation(m_shaderProgram, "inTexCoord");
                    CHICDK_LOG_INFO("posId %d texId %d", m_posId, m_texId);

                    m_pfnglbindattriblocation(m_shaderProgram, m_posId, "inPosition");
                    result      = CheckglError("glBindAttribLocation");

                    m_pfnglbindattriblocation(m_shaderProgram, m_texId, "inTexCoord");
                    result      = CheckglError("glBindAttribLocation");

                    EnableTextureUnit(m_shaderProgram, "tex", GL_TEXTURE0);
                    // Reset to null
                    m_pfneglmakecurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                    result      = CheckEglError("eglMakeCurrent");
                }
            }
        }
    }

    if (CDKResultSuccess == result)
    {
        m_initStatus = GLInitDone;
    }
    else
    {
        m_initStatus = GLInitInvalid;
        CHICDK_LOG_ERROR("OpenGL Initialization Failed");
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::InitializeFuncPtrs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::InitializeFuncPtrs()
{
    CDKResult       result = CDKResultSuccess;
    INT             numCharWritten = 0;
    CHAR            libFilePathGLESV2[FILENAME_MAX];
    CHAR            libFilePathEGL[FILENAME_MAX];

    numCharWritten = ChiNodeUtils::SNPrintF(libFilePathGLESV2, FILENAME_MAX, "%s%s%s.%s", VendorLibPath, PathSeparator, pDefaultGLESv2LibraryName, SharedLibraryExtension);
    numCharWritten = ChiNodeUtils::SNPrintF(libFilePathEGL,
                                            FILENAME_MAX,
                                            "%s%s%s.%s",
                                            VendorLibPath,
                                            PathSeparator,
                                            pDefaultEGLLibraryName,
                                            SharedLibraryExtension);

    m_hOpenGLESV2Lib = ChiNodeUtils::LibMapFullName(libFilePathGLESV2);
    m_hOpenEGLLib = ChiNodeUtils::LibMapFullName(libFilePathEGL);

    if (NULL == m_hOpenGLESV2Lib)
    {
        result = CDKResultEUnableToLoad;
        CHICDK_LOG_ERROR("OpenGL dll Path = %s Not loaded", libFilePathGLESV2);
    }
    else if (NULL == m_hOpenEGLLib)
    {
        result = CDKResultEUnableToLoad;
        CHICDK_LOG_ERROR("EGL dll Path = %s Not loaded", libFilePathEGL);
    }

    if (CDKResultSuccess == result)
    {
        m_pfngluseprogram                               = reinterpret_cast<PFNGLUSEPROGRAM>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glUseProgram"));

        m_pfnglgetattriblocation                        = reinterpret_cast<PFNGLGETATTRIBLOCATION>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetAttribLocation"));

        m_pfngldeleteshader                             = reinterpret_cast<PFNGLDELETESHADER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteShader"));

        m_pfnglfinish                                   = reinterpret_cast<PFNGLFINISH>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glFinish"));

        m_pfnglcreateshader                             = reinterpret_cast<PFNGLCREATESHADER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glCreateShader"));

        m_pfnglshadersource                             = reinterpret_cast<PFNGLSHADERSOURCE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glShaderSource"));

        m_pfnglcompileshader                            = reinterpret_cast<PFNGLCOMPILESHADER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glCompileShader"));

        m_pfnglgetshaderiv                              = reinterpret_cast<PFNGLGETSHADERIV>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetShaderiv"));

        m_pfnglgetprograminfolog                        = reinterpret_cast<PFNGLGETPROGRAMINFOLOG>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetProgramInfoLog"));

        m_pfngldeleteprogram                            = reinterpret_cast<PFNGLDELETEPROGRAM>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteProgram"));

        m_pfngldetachshader                             = reinterpret_cast<PFNGLDETACHSHADER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDetachShader"));

        m_pfnglgeterror                                 = reinterpret_cast<PFNGLGETERROR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetError"));

        m_pfnglgetuniformlocation                       = reinterpret_cast<PFNGLGETUNIFORMLOCATION>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetUniformLocation"));

        m_pfnglactivetexture                            = reinterpret_cast<PFNGLACTIVETEXTURE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glActiveTexture"));

        m_pfngluniform1i                                = reinterpret_cast<PFNGLUNIFORM1I>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glUniform1i"));

        m_pfngldisablevertexattribarray                 = reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAY>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDisableVertexAttribArray"));

        m_pfngldeletebuffers                            = reinterpret_cast<PFNGLDELETEBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteBuffers"));

        m_pfnglbindattriblocation                       = reinterpret_cast<PFNGLBINDATTRIBLOCATION>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBindAttribLocation"));

        m_pfnglcreateprogram                            = reinterpret_cast<PFNGLCREATEPROGRAM>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glCreateProgram"));

        m_pfnglgetprogramiv                             = reinterpret_cast<PFNGLGETPROGRAMIV>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetProgramiv"));

        m_pfnglgetshaderinfolog                         = reinterpret_cast<PFNGLGETSHADERINFOLOG>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGetShaderInfoLog"));

        m_pfnglattachshader                             = reinterpret_cast<PFNGLATTACHSHADER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glAttachShader"));

        m_pfngllinkprogram                              = reinterpret_cast<PFNGLLINKPROGRAM>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glLinkProgram"));

        m_pfnglbufferdata                               = reinterpret_cast<PFNGLBUFFERDATA>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBufferData"));

        m_pfnglgenbuffers                               = reinterpret_cast<PFNGLGENBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGenBuffers"));

        m_pfnglvertexattribpointer                      = reinterpret_cast<PFNGLVERTEXATTRIBPOINTER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glVertexAttribPointer"));

        m_pfnglenablevertexattribarray                  = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAY>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glEnableVertexAttribArray"));

        m_pfnglbindbuffer                               = reinterpret_cast<PFNGLBINDBUFFER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBindBuffer"));

        m_pfnglscissor                                  = reinterpret_cast<PFNGLSCISSOR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glScissor"));


        m_pfnglClearColor                               = reinterpret_cast<PFNGLCLEARCOLOR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glClearColor"));

        m_pfnglClear                                    = reinterpret_cast<PFNGLCLEAR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glClear"));

        m_pfnglbindframebuffer                          = reinterpret_cast<PFNGLBINDFRAMEBUFFER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBindFramebuffer"));

        m_pfnglbindtexture                              = reinterpret_cast<PFNGLBINDTEXTURE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBindTexture"));

        m_pfnglviewport                                 = reinterpret_cast<PFNGLVIEWPORT>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glViewport"));

        m_pfnglflush                                    = reinterpret_cast<PFNGLFLUSH>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glFlush"));

        m_pfngldrawelements                             = reinterpret_cast<PFNGLDRAWELEMENTS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDrawElements"));

        m_pfnglcheckframebufferstatus                   = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glCheckFramebufferStatus"));

        m_pfnglgentextures                              = reinterpret_cast<PFNGLGENTEXTURES>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGenTextures"));

        m_pfngltexparameteri                            = reinterpret_cast<PFNGLTEXPARAMETERI>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glTexParameteri"));

        m_pfngleglImageTargetTexture2DOES               = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glEGLImageTargetTexture2DOES"));

        m_pfngleglImageTargetRenderbufferStorageOES     = reinterpret_cast<PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glEGLImageTargetRenderbufferStorageOES"));

        m_pfnglgenframebuffers                          = reinterpret_cast<PFNGLGENFRAMEBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGenFramebuffers"));

        m_pfnglgenrenderbuffers                         = reinterpret_cast<PFNGLGENRENDERBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glGenRenderbuffers"));

        m_pfnglbindrenderbuffer                         = reinterpret_cast<PFNGLBINDRENDERBUFFER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glBindRenderbuffer"));

        m_pfnglframebufferrenderbuffer                  = reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFER>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glFramebufferRenderbuffer"));

        m_pfngldeleteframebuffers                       = reinterpret_cast<PFNGLDELETEFRAMEBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteFramebuffers"));

        m_pfngldeleterenderbuffers                      = reinterpret_cast<PFNGLDELETERENDERBUFFERS>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteRenderbuffers"));

        m_pfngldeletetextures                           = reinterpret_cast<PFNGLDELETETEXTURES>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenGLESV2Lib,
                                                                                "glDeleteTextures"));

    }

    if ((NULL == m_pfngluseprogram)                     ||
        (NULL == m_pfnglgetattriblocation)              ||
        (NULL == m_pfngldeleteshader)                   ||
        (NULL == m_pfnglfinish)                         ||
        (NULL == m_pfnglcreateshader)                   ||
        (NULL == m_pfnglshadersource)                   ||
        (NULL == m_pfnglcompileshader)                  ||
        (NULL == m_pfnglgetshaderiv)                    ||
        (NULL == m_pfnglgetprograminfolog)              ||
        (NULL == m_pfngldeleteprogram)                  ||
        (NULL == m_pfngldetachshader)                   ||
        (NULL == m_pfnglgeterror)                       ||
        (NULL == m_pfnglgetuniformlocation)             ||
        (NULL == m_pfnglactivetexture)                  ||
        (NULL == m_pfngluniform1i)                      ||
        (NULL == m_pfngldisablevertexattribarray)       ||
        (NULL == m_pfngldeletebuffers)                  ||
        (NULL == m_pfnglbindattriblocation)             ||
        (NULL == m_pfnglcreateprogram)                  ||
        (NULL == m_pfnglgetprogramiv)                   ||
        (NULL == m_pfnglgetshaderinfolog)               ||
        (NULL == m_pfnglattachshader)                   ||
        (NULL == m_pfngllinkprogram)                    ||
        (NULL == m_pfnglviewport)                       ||
        (NULL == m_pfnglbindtexture)                    ||
        (NULL == m_pfnglbindframebuffer)                ||
        (NULL == m_pfnglscissor)                        ||
        (NULL == m_pfnglClearColor)                     ||
        (NULL == m_pfnglClear)                          ||
        (NULL == m_pfnglbindbuffer)                     ||
        (NULL == m_pfnglenablevertexattribarray)        ||
        (NULL == m_pfnglvertexattribpointer)            ||
        (NULL == m_pfnglgenbuffers)                     ||
        (NULL == m_pfnglbufferdata)                     ||
        (NULL == m_pfngldrawelements)                   ||
        (NULL == m_pfnglgentextures)                    ||
        (NULL == m_pfngltexparameteri)                  ||
        (NULL == m_pfnglcheckframebufferstatus)         ||
        (NULL == m_pfngleglImageTargetTexture2DOES)     ||
        (NULL == m_pfnglgenframebuffers)                ||
        (NULL == m_pfnglgenrenderbuffers)               ||
        (NULL == m_pfnglbindrenderbuffer)               ||
        (NULL == m_pfnglframebufferrenderbuffer)        ||
        (NULL == m_pfngldeleteframebuffers)             ||
        (NULL == m_pfngldeleterenderbuffers)            ||
        (NULL == m_pfngldeletetextures))
    {
        CHICDK_LOG_ERROR("Error Initializing one or more function pointers in Library: %s%s.%s",
                    libFilePathGLESV2,
                    pDefaultGLESv2LibraryName,
                    SharedLibraryExtension);
        result                                          = CDKResultENoSuch;
    }

    if (CDKResultSuccess == result)
    {
        m_pfneglgetdisplay                              = reinterpret_cast<PFNEGLGETDISPLAY>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglGetDisplay"));

        m_pfneglinitialize                              = reinterpret_cast<PFNEGLINITIALIZE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglInitialize"));

        m_pfneglmakecurrent                             = reinterpret_cast<PFNEGLMAKECURRENT>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglMakeCurrent"));

        m_pfneglgeterror                                = reinterpret_cast<PFNEGLGETERROR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglGetError"));

        m_pfneglquerystring                             = reinterpret_cast<PFNEGLQUERYSTRING>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglQueryString"));

        m_pfneglcreatepbuffersurface                    = reinterpret_cast<PFNEGLCREATEPBUFFERSURFACE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglCreatePbufferSurface"));

        m_pfneglcreatecontext                           = reinterpret_cast<PFNEGLCREATECONTEXT>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglCreateContext"));

        m_pfnegldestroycontext                          = reinterpret_cast<PFNEGLDESTROYCONTEXT>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglDestroyContext"));

        m_pfnegldestroysurface                          = reinterpret_cast<PFNEGLDESTROYSURFACE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglDestroySurface"));

        m_pfneglterminate                               = reinterpret_cast<PFNEGLTERMINATE>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglTerminate"));

        m_pfneglchooseconfig                            = reinterpret_cast<PFNEGLCHOOSECONFIG>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglChooseConfig"));

        m_pfneglgetconfigattrib                         = reinterpret_cast<PFNEGLGETCONFIGATTRIB>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglGetConfigAttrib"));

        m_pfneglbindapi                                 = reinterpret_cast<PFNEGLBINDAPI>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglBindAPI"));

        m_pfneglcreateimagekhr                          = reinterpret_cast<PFNEGLCREATEIMAGEKHR>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglCreateImageKHR"));

        m_pfneglgetcurrentdisplay                       = reinterpret_cast<PFNEGLGETCURRENTDISPLAY>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglGetCurrentDisplay"));

        m_pfneglDestroyImageKHR                         = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
                                                        ChiNodeUtils::LibGetAddr(m_hOpenEGLLib,
                                                                                "eglDestroyImageKHR"));

    }

    if ((NULL == m_pfneglgetdisplay)            ||
        (NULL == m_pfneglinitialize)            ||
        (NULL == m_pfneglmakecurrent)           ||
        (NULL == m_pfneglgeterror)              ||
        (NULL == m_pfneglquerystring)           ||
        (NULL == m_pfneglcreatepbuffersurface)  ||
        (NULL == m_pfneglcreatecontext)         ||
        (NULL == m_pfnegldestroycontext)        ||
        (NULL == m_pfnegldestroysurface)        ||
        (NULL == m_pfneglterminate)             ||
        (NULL == m_pfneglchooseconfig)          ||
        (NULL == m_pfneglgetconfigattrib)       ||
        (NULL == m_pfneglbindapi)               ||
        (NULL == m_pfneglcreateimagekhr)        ||
        (NULL == m_pfneglgetcurrentdisplay)     ||
        (NULL == m_pfneglDestroyImageKHR))
    {
        CHICDK_LOG_ERROR("Error Initializing one or more function pointers in Library: %s%s.%s",
                libFilePathEGL,
                pDefaultEGLLibraryName,
                SharedLibraryExtension);
        result                                          = CDKResultENoSuch;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::~GPUOpenGL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GPUOpenGL::~GPUOpenGL()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::Uninitialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::Uninitialize()
{
    CHICDK_LOG_INFO("Deinitialization of OpenGL instance");

    if (m_pVertices)
    {
        free(m_pVertices);
        m_pVertices = NULL;
    }

    if (m_pTexCoordinates)
    {
        free(m_pTexCoordinates);
        m_pTexCoordinates = NULL;
    }

    if (m_pIndices)
    {
        free(m_pIndices);
        m_pIndices = NULL;
    }

    m_pfneglmakecurrent(m_defaultDisplay, m_defaultSurface,
        m_defaultSurface, m_defaultContext);
    m_pfngldisablevertexattribarray(m_posId);
    m_pfngldisablevertexattribarray(m_texId);

    CheckglError("glDisableVertexAttribArray");

    // Reset program
    m_pfngluseprogram(0);

    // Delete shader program
    m_pfngldeleteprogram(m_shaderProgram);
    CheckglError("m_pfngldeleteprogram");

    /// to Flush texture and frame buffer maps
    FlushBufferMaps();

    m_posId = 0;
    m_texId = 0;
    m_transId = 0;
    m_pfngldeletebuffers(1, &m_indexVBOID);
    m_indexVBOID = 0;
    if (m_pVertices)
    {
        CAMX_FREE(m_pVertices);
        m_pVertices = NULL;
    }
    if (m_pTexCoordinates)
    {
        CAMX_FREE(m_pTexCoordinates);
        m_pTexCoordinates = NULL;
    }
    if (m_pIndices)
    {
        CAMX_FREE(m_pIndices);
        m_pIndices = NULL;
    }

    // Reset default context to null
    if (m_defaultContext != EGL_NO_CONTEXT)
    {
        m_pfnegldestroycontext(m_defaultDisplay, m_defaultContext);
        m_defaultContext = EGL_NO_CONTEXT;
    }
    if (m_defaultSurface != EGL_NO_SURFACE)
    {
        m_pfnegldestroysurface(m_defaultDisplay, m_defaultSurface);
        m_defaultSurface = EGL_NO_SURFACE;
    }
    if (m_defaultDisplay != EGL_NO_DISPLAY)
    {
        m_pfneglmakecurrent(m_defaultDisplay, EGL_NO_SURFACE,
            EGL_NO_SURFACE, EGL_NO_CONTEXT);
        m_pfneglterminate(m_defaultDisplay);
        m_defaultDisplay = EGL_NO_DISPLAY;
    }

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::CreateContext
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::CreateContext(
    EGLDisplay      display,
    EGLContext      sharedContext,
    EGLSurface*     pSurface,
    EGLContext*     pContext)
{
    CDKResult   result = CDKResultEFailed;
    static EGLint PbufferConfigAttribs[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_STENCIL_SIZE,   8,
        EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_SAMPLES,        0,
        EGL_NONE
    };

    static const EGLint ContextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    static EGLint s_pbufferAttribs[] =
    {
        EGL_WIDTH,  0,
        EGL_HEIGHT, 0,
        EGL_NONE
    };

    s_pbufferAttribs[1]     = m_configDefault.winW;
    s_pbufferAttribs[3]     = m_configDefault.winH;
    m_pfneglbindapi(EGL_OPENGL_ES_API);

    const CHAR *str         = m_pfneglquerystring(m_defaultDisplay, EGL_VENDOR);
    CHICDK_LOG_INFO("EGL Vendor: %s ", str);
    CHICDK_LOG_INFO("EGL Version: %d:%d ", m_majorVersion, m_minorVersion);

    m_config                = GetEglConfig(m_defaultDisplay, PbufferConfigAttribs, &m_configDefault);
    *pContext = m_pfneglcreatecontext(display, m_config,
                                      sharedContext, ContextAttribs);
    if (*pContext == EGL_NO_CONTEXT)
    {
        CHICDK_LOG_ERROR("Unable to create EGL pContext");
        return result;
    }
    *pSurface = m_pfneglcreatepbuffersurface(m_defaultDisplay, m_config,
                                            s_pbufferAttribs);
    if (*pSurface == EGL_NO_SURFACE)
    {
        CHICDK_LOG_ERROR("Unable to create EGL pSurface");
        return result;
    }
    CHICDK_LOG_INFO("EglDisplay %p, EglSurface %p, EglContext %p",
                                display, *pSurface, *pContext);

    result = CDKResultSuccess;
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GetEglConfig
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EGLConfig GPUOpenGL::GetEglConfig(
    EGLDisplay        display,
    const EGLint*     pAttribList,
    EGLCONFIGATTRIB*  pConfig)
{
    EGLConfig   m_config = NULL;

    EGLConfig*  pConfigs;
    EGLint      numConfigs;

    m_pfneglchooseconfig(display, pAttribList, NULL, 0, &numConfigs);
    pConfigs = (EGLConfig*)malloc(sizeof(EGLConfig) * numConfigs);

    if (!pConfigs)
    {
        return 0;
    }

    m_pfneglchooseconfig(display, pAttribList, pConfigs, numConfigs, &numConfigs);
    // parse all returned configurations and select the one that we really want
    for (INT32 i = 0; i<numConfigs; ++i)
    {
        INT32 rBits;
        INT32 gBits;
        INT32 bBits;
        INT32 aBits;
        INT32 depth;
        INT32 stencil;
        INT32 sampleBuffers;
        INT32 samples;

        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_RED_SIZE, &rBits);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_GREEN_SIZE, &gBits);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_BLUE_SIZE, &bBits);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_ALPHA_SIZE, &aBits);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_DEPTH_SIZE, &depth);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_STENCIL_SIZE, &stencil);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_SAMPLE_BUFFERS, &sampleBuffers);
        m_pfneglgetconfigattrib(display, pConfigs[i], EGL_SAMPLES, &samples);

        if (rBits == pConfig->rBits && gBits == pConfig->gBits && bBits == pConfig->bBits &&
            (!pConfig->aBits || aBits == pConfig->aBits) && depth == pConfig->depthBits &&
            (!pConfig->stencilBits || stencil == pConfig->stencilBits) &&
            sampleBuffers == pConfig->sampleBuffers && samples == pConfig->samples)
        {
            m_config = pConfigs[i];
            break;
        }
    }

    free(pConfigs);

    return m_config;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ConvertChiFormat
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT32 GPUOpenGL::ConvertChiFormat(
    ChiFormat chiFormat)
{
    INT32 openGLFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;

    switch (chiFormat)
    {
        case ChiFormat::YUV420NV12:
            openGLFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
            break;
        case ChiFormat::UBWCNV12:
            openGLFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS_UBWC;
            break;
        default:
            CHICDK_LOG_ERROR("Unsupported Chi Format for OpenGL: %d, falling back to NV12", chiFormat);
            break;
    }

    CHICDK_LOG_INFO("Convert Chi format: %d -> %d", chiFormat, openGLFormat);
    return openGLFormat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::SetPrivateHandleFlags
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT32 GPUOpenGL::SetPrivateHandleFlags(
    ChiFormat chiFormat)
{
#if defined(ANDROID_Q_AOSP)
    INT32 priv_flags = private_handle_t::PRIV_FLAGS_USES_ION     |
                     private_handle_t::PRIV_FLAGS_NON_CPU_WRITER |
                     private_handle_t::PRIV_FLAGS_CAMERA_WRITE   |
                     private_handle_t::PRIV_FLAGS_HW_TEXTURE;

    switch (chiFormat)
    {
        case ChiFormat::UBWCNV12:
            priv_flags |= private_handle_t::PRIV_FLAGS_UBWC_ALIGNED;
            break;
        default:
            break;
    }

#else
    INT32 priv_flags = PRIV_FLAGS_USES_ION     |
                     PRIV_FLAGS_NON_CPU_WRITER |
                     PRIV_FLAGS_CAMERA_WRITE   |
                     PRIV_FLAGS_HW_TEXTURE;

    switch (chiFormat)
    {
        case ChiFormat::UBWCNV12:
            priv_flags |= PRIV_FLAGS_UBWC_ALIGNED;
            break;
        default:
            break;
    }

#endif

    return priv_flags;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::DewarpImage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::DewarpImage(
    CHINODEBUFFERHANDLE   hOutput,
    CHINODEBUFFERHANDLE   hInput,
    FLOAT*                pIPEICAGridTransform,
    BOOL                  eisEnable)
{
    INT32       numTriangles;
    INT32       texWidth;
    INT32       texHeight;
    INT32       outWidth;
    INT32       outHeight;

    m_formatInput             = ConvertChiFormat(hInput->format.format);
    m_formatOutput            = ConvertChiFormat(hOutput->format.format);
    m_PrivatHandleFlagsInput  = SetPrivateHandleFlags(hInput->format.format);
    m_PrivatHandleFlagsOutput = SetPrivateHandleFlags(hOutput->format.format);

    CHICDK_LOG_INFO("Input buffer format %d Output buffer format %d", m_formatInput, m_formatOutput);


    GLuint sourceTexture   = 0;
    GLuint frameBuffer     = 0;

    numTriangles            = m_nNumMeshX * m_nNumMeshY * 2;
    texWidth                = hInput->format.formatParams.yuvFormat[0].width;
    texHeight               = hInput->format.formatParams.yuvFormat[0].height;
    outWidth                = hOutput->format.formatParams.yuvFormat[0].width;
    outHeight               = hOutput->format.formatParams.yuvFormat[0].height;

    CHICDK_LOG_VERBOSE("EglDisplay %p, EglSurface %p, EglContext %p",
                     m_defaultDisplay, m_defaultSurface, m_defaultContext);

    m_pfneglmakecurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface, m_defaultContext);
    CheckEglError("eglMakeCurrent");

    m_pfngluseprogram(m_shaderProgram);
    CheckglError("glUseProgram");

    EnableTextureUnit(m_shaderProgram, "tex", GL_TEXTURE0);
    m_pfnglviewport(0, 0, hOutput->format.formatParams.yuvFormat[0].width, hOutput->format.formatParams.yuvFormat[0].height);

    // Setup vertices and texCoordinates. Once only.
    if (!m_pVertices || !m_pTexCoordinates)
    {
        m_pVertices       = (float*)malloc(m_nNumVertices * sizeof(float) * 2);
        m_pTexCoordinates = (float*)malloc(m_nNumVertices * sizeof(float) * 2);

        if (!m_pVertices || !m_pTexCoordinates)
        {
            CHICDK_LOG_ERROR("m_pVertices or m_pTexCoordinates is null!");
            return CDKResultEFailed;
        }
    }

    // Setup indices. Once only
    if (!m_pIndices)
    {
        m_pIndices = (unsigned short*)malloc(numTriangles * 3 * sizeof(UINT16));
        if (!m_pIndices)
        {
            CHICDK_LOG_ERROR("m_pIndices is null!");
            return CDKResultEFailed;
        }
        GenerateIndices(m_pIndices, m_nNumMeshX, m_nNumMeshY);
        CHICDK_LOG_VERBOSE("index %d %d %d %d %d %d", 
            m_pIndices[0], m_pIndices[1], m_pIndices[2],
            m_pIndices[3], m_pIndices[4], m_pIndices[5]);
        AllocIndices(&m_indexVBOID, numTriangles, m_pIndices);
    }
    CHICDK_LOG_VERBOSE("m_nNumVerticesX %d m_nNumVerticesY %d tex %dx%d out %dx%d", m_nNumVerticesX, m_nNumVerticesY, texWidth, texHeight, outWidth, outHeight);
    UpdateVertexAndTextureCoordinates(m_pVertices, m_pTexCoordinates, pIPEICAGridTransform, m_nNumVerticesX, m_nNumVerticesY, texWidth, texHeight, outWidth, outHeight, eisEnable);

    CHICDK_LOG_VERBOSE(" texWidth %d, texHeight %d, outWidth %d, outHeight %d", texWidth, texHeight, outWidth, outHeight);

    for (INT loopCount = 0; loopCount < m_nNumVerticesY*m_nNumVerticesX; loopCount++)
    {
        CHICDK_LOG_VERBOSE("verts index %d : %f %f", loopCount,m_pVertices[loopCount*2], m_pVertices[loopCount*2+1] );
    }

    for (INT loopCount = 0; loopCount < m_nNumVerticesY*m_nNumVerticesX; loopCount++)
    {
        CHICDK_LOG_VERBOSE("texCo index %d : %f %f", loopCount, m_pTexCoordinates[loopCount*2], m_pTexCoordinates[loopCount*2+1] );
    }

    for (INT loopCount = 0; loopCount < (m_nNumVerticesY+2)*(m_nNumVerticesX+2); loopCount++)
    {
        CHICDK_LOG_VERBOSE("Grid index %d : %f %f", loopCount, pIPEICAGridTransform[loopCount*2], pIPEICAGridTransform[loopCount*2+1] );
    }
    // Configure vertices, indices
    ConfigureVertexAndTextureCoordinates(m_pVertices, m_pTexCoordinates, m_posId, m_texId);
    ConfigureIndices(&m_indexVBOID);

    // Setup buffer information
    sourceTexture = GetSourceTexture(  hInput->pImageList[0].fd[0],
                                       hInput->pImageList[0].pAddr[0],
                                       (hInput->format.formatParams.yuvFormat[0].height*
                                       hInput->format.formatParams.yuvFormat[0].width) +
                                       (hInput->format.formatParams.yuvFormat[1].height*
                                       hInput->format.formatParams.yuvFormat[1].width),
                                       hInput->format.formatParams.yuvFormat[0].width,
                                       hInput->format.formatParams.yuvFormat[0].height,
                                       hInput->format.formatParams.yuvFormat[0].planeStride,
                                       hInput->format.formatParams.yuvFormat[0].sliceHeight);
    frameBuffer = GetOutputFrameBuffer(hOutput->pImageList[0].fd[0],
                                       hOutput->pImageList[0].pAddr[0],
                                       (hOutput->format.formatParams.yuvFormat[0].height*
                                       hOutput->format.formatParams.yuvFormat[0].width) +
                                       (hOutput->format.formatParams.yuvFormat[1].height*
                                       hOutput->format.formatParams.yuvFormat[1].width),
                                       hOutput->format.formatParams.yuvFormat[0].width,
                                       hOutput->format.formatParams.yuvFormat[0].height,
                                       hOutput->format.formatParams.yuvFormat[0].planeStride,
                                       hOutput->format.formatParams.yuvFormat[0].sliceHeight);

    CHICDK_LOG_VERBOSE("sourceTexture %d frameBuffer %d", sourceTexture, frameBuffer);

    // Bind Source Texture and Frame Buffer
    m_pfnglbindtexture(GL_TEXTURE_EXTERNAL_OES, sourceTexture);
    CheckglError("glBindTexture");

    m_pfnglbindframebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    CheckglError("glBindFramebuffer");


    m_pfnglscissor(0, 0, hOutput->format.formatParams.yuvFormat[0].width,
                    hOutput->format.formatParams.yuvFormat[0].height);
    CheckglError("glScissor");

    GLenum frameBufferCompletezOutput = m_pfnglcheckframebufferstatus(GL_DRAW_FRAMEBUFFER);

    if (GL_FRAMEBUFFER_COMPLETE == frameBufferCompletezOutput)
    {
        m_pfnglClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        CheckglError("glClearColor");

        m_pfnglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckglError("glClear");

        m_pfngldrawelements(GL_TRIANGLES, 3 * numTriangles, GL_UNSIGNED_SHORT, NULL);
        CheckglError("glDrawElements");
    }
    else
    {
        CHICDK_LOG_ERROR("GL output FrameBuffer Error : (0x%x)",
                                    frameBufferCompletezOutput);
    }

    m_pfnglflush();
    m_pfnglfinish();

    m_pfnglbindtexture(GL_TEXTURE_EXTERNAL_OES, 0);
    CheckglError("glBindTexture");

    m_pfnglbindframebuffer(GL_DRAW_FRAMEBUFFER, 0);
    CheckglError("glBindFramebuffer");

    m_pfneglmakecurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
        EGL_NO_CONTEXT);
    CheckEglError("eglMakeCurrent");

    CHICDK_LOG_VERBOSE(": X ret");

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ConfigureIndices
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GPUOpenGL::ConfigureIndices(
    GLuint *pIndexVBOID)
{
    m_pfnglbindbuffer(GL_ELEMENT_ARRAY_BUFFER, *pIndexVBOID);
    CheckglError("glBindBuffer");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::FlushBufferMaps
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::FlushBufferMaps()
{
    std::map<UINT32, BUFMAPINFO>::iterator itr = m_SourceTextureMap.begin();
    while (itr != m_SourceTextureMap.end())
    {
        BUFMAPINFO info = itr->second;
        CHICDK_LOG_VERBOSE("Flushing element for idx %d ", info.index);
        m_pfngldeletetextures(1, &info.hColorTexture);

        if (info.hEGLImage)
        {
            m_pfneglDestroyImageKHR(m_pfneglgetcurrentdisplay(), info.hEGLImage);
            CheckEglError("Destroy image");
        }
        // Free metadata buffer
        ImageBufferRelease(&info.hMetaBuffer);
        // Free handle
#if defined(ANDROID_Q_AOSP)
        info.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(info.phPrivateHandle));
#else
        info.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(info.NativeHandle));
#endif
        memset(&info, 0, sizeof(BUFMAPINFO));
        itr = m_SourceTextureMap.erase(itr);
    }

    itr = m_FrameBufferMap.begin();
    while (itr != m_FrameBufferMap.end())
    {
        BUFMAPINFO info = itr->second;

        CHICDK_LOG_VERBOSE("Flushing element for idx %d ", info.index);
        m_pfngldeleteframebuffers(1, &info.hFBO);
        m_pfngldeleterenderbuffers(1, &info.hRBO);

        if (info.hEGLImage)
        {
            m_pfneglDestroyImageKHR(m_pfneglgetcurrentdisplay(), info.hEGLImage);
            CheckEglError("Destroy image");
        }
        // Free metadata buffer
        ImageBufferRelease(&info.hMetaBuffer);
        // Free handle
#if defined(ANDROID_Q_AOSP)
        info.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(info.phPrivateHandle));
#else
        info.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(info.NativeHandle));
#endif
        memset(&info, 0, sizeof(BUFMAPINFO));
        itr = m_FrameBufferMap.erase(itr);
    }
    CHICDK_LOG_VERBOSE("Flushing of BufferMap Done");
    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ConfigureVertexAndTextureCoordinates
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GPUOpenGL::ConfigureVertexAndTextureCoordinates(
    FLOAT*       pVertices,
    FLOAT*       pTexCoordinates,
    GLint        posId,
    GLint        texId)
{
    m_pfnglenablevertexattribarray(posId);
    m_pfnglenablevertexattribarray(texId);
    m_pfnglvertexattribpointer(posId, 2, GL_FLOAT, GL_FALSE, 0, pVertices);
    CheckglError("glVertexAttribPointer pVertices");
    m_pfnglvertexattribpointer(texId, 2, GL_FLOAT, GL_FALSE, 0, pTexCoordinates);
    CheckglError("glVertexAttribPointer pTexCoordinates");

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::AllocIndices
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GPUOpenGL::AllocIndices(
    GLuint*            pIndexVBOID,
    INT32              numTriangles,
    UINT16*            pIndices)
{
    m_pfnglgenbuffers(1, pIndexVBOID);
    m_pfnglbindbuffer(GL_ELEMENT_ARRAY_BUFFER, *pIndexVBOID);
    CheckglError("glBindBuffer pIndices");
    m_pfnglbufferdata(GL_ELEMENT_ARRAY_BUFFER, numTriangles * 3 * sizeof(UINT16),
        pIndices, GL_STATIC_DRAW);
    CheckglError("glBufferData pIndices");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GenerateIndices
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GPUOpenGL::GenerateIndices(
    UINT16*              pIndices,
    INT32                numMeshX,
    INT32                numMeshY)
{
    INT32 numVerticesX    = (numMeshX + 1);
    for (INT32 y = 0; y<numMeshY; y++)
    {
        for (INT32 x = 0; x<numMeshX; x++)
        {
            pIndices[6 * y*numMeshX + 6 * x + 0]     = y*numVerticesX + x;
            pIndices[6 * y*numMeshX + 6 * x + 1]     = (y + 1)*numVerticesX + x;
            pIndices[6 * y*numMeshX + 6 * x + 2]     = y*numVerticesX + (x + 1);
            pIndices[6 * y*numMeshX + 6 * x + 3]     = (y + 1)*numVerticesX + x;
            pIndices[6 * y*numMeshX + 6 * x + 4]     = y*numVerticesX + (x + 1);
            pIndices[6 * y*numMeshX + 6 * x + 5]     = (y + 1)*numVerticesX + (x + 1);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::UpdateVertexAndTextureCoordinates
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID GPUOpenGL::UpdateVertexAndTextureCoordinates(
    FLOAT*         pVertices,
    FLOAT*         pTexCoordinates,
    const FLOAT*   pGridData,
    INT32          numVerticesX,
    INT32          numVerticesY,
    INT32          inImageWidth,
    INT32          inImageHeight,
    INT32          outImageWidth,
    INT32          outImageHeight,
    BOOL           eisEnable)
{
    FLOAT normOffsetX = -0.5 / inImageWidth;
    FLOAT normOffsetY = -0.5 / inImageHeight;

    FLOAT cropFactorX = inImageWidth / (FLOAT)outImageWidth;
    FLOAT cropFactorY = inImageHeight / (FLOAT)outImageHeight;

    FLOAT imageCenterX = (inImageWidth - 1.0f) / 2.0f;
    FLOAT imageCenterY = (inImageHeight - 1.0f) / 2.0f;

    CHICDK_LOG_VERBOSE("parameters: numVerticesX %d,numVerticesY %d,inImageWidth %d,inImageHeight %d,outImageWidth %d,outImageHeight %d", numVerticesX,numVerticesY, inImageWidth,inImageHeight,outImageWidth,outImageHeight);

    for (INT32 y = 0; y<numVerticesY; y++)
    {
        for (INT32 x = 0; x<numVerticesX; x++)
        {
            // y flip due to some unknown reasons (determined empirically)
            // normOffset accordingly to the grid mapping
            // cropFactor to crop 20%
            // imageCenter assumes that grid is defined s.t. (x + imageCenterX) starts from 0
            pVertices[2 * y*numVerticesX + 2 * x + 0] = cropFactorX * (-1.0f + 2.0f*(normOffsetX + x / (FLOAT)(numVerticesX - 1)));
            pVertices[2 * y*numVerticesX + 2 * x + 1] = cropFactorY * (-1.0f + 2.0f*(normOffsetY + y / (FLOAT)(numVerticesY - 1)));

            // +1 offset due to the extrapolation points in grid
            FLOAT coordX = pGridData[2 * ((y + 1)*(numVerticesX + 2) + (x + 1)) + 0];
            FLOAT coordY = pGridData[2 * ((y + 1)*(numVerticesX + 2) + (x + 1)) + 1];

            if (eisEnable == TRUE)
            {
                pTexCoordinates[2 * y*numVerticesX + 2 * x + 0] = +0.0f + 1.0f*((coordX + imageCenterX) / inImageWidth);
                pTexCoordinates[2 * y*numVerticesX + 2 * x + 1] = +0.0f + 1.0f*((coordY + imageCenterY) / inImageHeight);

                CHICDK_LOG_VERBOSE("calculation at (%d,%d) : data (%f %f), text (%f %f), vert (%f %f)",
                    x, y,
                    coordX, coordY,
                    pTexCoordinates[2 * y*numVerticesX + 2 * x + 0], pTexCoordinates[2 * y*numVerticesX + 2 * x + 1],
                    pVertices[2 * y*numVerticesX + 2 * x + 0], pVertices[2 * y*numVerticesX + 2 * x + 1]);
            }
            else
            {
                pTexCoordinates[2 * y*numVerticesX + 2 * x + 0] = +0.0f + 1.0f*(x / (FLOAT)(numVerticesX - 1));
                pTexCoordinates[2 * y*numVerticesX + 2 * x + 1] = +0.0f + 1.0f*(y / (FLOAT)(numVerticesY - 1));
                CHICDK_LOG_INFO("Applying unity transform");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ImageBufferOpen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT32 GPUOpenGL::ImageBufferOpen()
{
    if (!m_pImgBuffInfo)
    {
        m_pImgBuffInfo = (IMGBUFFERPRIVT *)calloc(1, sizeof(*m_pImgBuffInfo));
        if (!m_pImgBuffInfo)
        {
            CHICDK_LOG_ERROR("Cannot allocate memory");
            return -1;
        }

        m_pImgBuffInfo->refCount = 0;
        m_pImgBuffInfo->ionFd = -1;

        // Open ION device
        m_pImgBuffInfo->ionFd = open("/dev/ion", O_RDONLY);

        if (m_pImgBuffInfo->ionFd < 0)
        {
            CHICDK_LOG_ERROR("Ion open failed");
            free(m_pImgBuffInfo);
            m_pImgBuffInfo = NULL;
            return -1;
        }
    }
    m_pImgBuffInfo->refCount++;
    return m_pImgBuffInfo->ionFd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ImageBufferGet
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult GPUOpenGL::ImageBufferGet(
    INT32               length,
    IMGMEMHANDLET*      pHandle)
{
    IMGBUFFERT *pBuffer;

    if ((NULL == pHandle) || (length <= 0))
    {
        CHICDK_LOG_ERROR("Null check failed");
        return CDKResultEFailed;
    }

    pBuffer = (IMGBUFFERT*)malloc(sizeof(IMGBUFFERT));
    if (NULL == pBuffer)
    {
        CHICDK_LOG_ERROR("malloc failed");
        return CDKResultEFailed;
    }

    /* ION buffer */
    if (NULL == m_pImgBuffInfo)
    {
        CHICDK_LOG_VERBOSE("Opening ION device since fd is invalid");
        pBuffer->ionFd = ImageBufferOpen();

        if (pBuffer->ionFd < 0)
        {
            CHICDK_LOG_ERROR("Opening ION device failed");
            return CDKResultEFailed;
        }
    }
    else
    {
        pBuffer->ionFd  = m_pImgBuffInfo->ionFd;
    }

    pBuffer->allocData.len              = (UINT)length;
    pBuffer->allocData.flags            = 0;

    pBuffer->allocData.heap_id_mask     = (0x1 << ION_SYSTEM_HEAP_ID);

    /* Make it page size aligned */
    pBuffer->allocData.len              = (pBuffer->allocData.len + 4095) & (~4095U);
    INT32 result                        = ioctl(pBuffer->ionFd, ION_IOC_ALLOC, &pBuffer->allocData);
    if (result < 0)
    {
        CHICDK_LOG_ERROR("ioctl failure id : %d, err %s",
                                    errno, strerror(errno));
        return CDKResultEFailed;
    }

    pBuffer->pAddr                      = (uint8_t*)mmap(NULL, pBuffer->allocData.len,
                                                    PROT_READ | PROT_WRITE,
                                                    MAP_SHARED, pBuffer->allocData.fd, 0);
    pBuffer->buffFd                     = pBuffer->allocData.fd;

    if (pBuffer->pAddr == MAP_FAILED)
    {
        CHICDK_LOG_ERROR("Failure 05");
        return CDKResultEFailed;
    }

    pBuffer->cached                 = 0;

    pHandle->bufferFd               = pBuffer->buffFd;
    pHandle->pVirtualAddr           = pBuffer->pAddr;
    pHandle->length                 = (UINT)pBuffer->allocData.len;
    pHandle->phBufferHandle         = (VOID*)pBuffer;

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::ImageBufferRelease
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID GPUOpenGL::ImageBufferRelease(
    IMGMEMHANDLET*      pHandle)
{
    IMGBUFFERT *pBuffer;
    UINT lsize;

    if ((NULL == pHandle) ||
        (NULL == pHandle->phBufferHandle))
    {
        CHICDK_LOG_ERROR("invalid input");
        return;
    }

    pBuffer = (IMGBUFFERT *)pHandle->phBufferHandle;

    if (NULL == m_pImgBuffInfo)
    {
        CHICDK_LOG_VERBOSE("Opening ION device since fd is invalid");
        pBuffer->ionFd = ImageBufferOpen();
        if (pBuffer->ionFd < 0)
        {
            CHICDK_LOG_ERROR("Ion open failed");
            return;
        }
    }
    else
    {
        pBuffer->ionFd = m_pImgBuffInfo->ionFd;
    }

    lsize           = (pBuffer->allocData.len + 4095) & (~4095U);

    INT32 result    = munmap(pBuffer->pAddr, lsize);
    if (result < 0)
    {
        CHICDK_LOG_ERROR("unmap failed %s (%d)",
            strerror(errno), errno);
    }

    result          = close(pBuffer->allocData.fd);
    if (result < 0)
    {
        CHICDK_LOG_ERROR("ion fd close failed  %s (%d)",
            strerror(errno), errno);
    }

    if ( NULL != pBuffer)
    {
        free(pBuffer);
        pHandle->phBufferHandle = NULL;
        pHandle->pVirtualAddr  = NULL;
        pHandle->bufferFd      = -1;
        pHandle->length        = 0;
    }
    CHICDK_LOG_VERBOSE("Success fd %d addr %p",
        pHandle->bufferFd, pHandle->pVirtualAddr);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GetSourceTexture
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLuint GPUOpenGL::GetSourceTexture(
    UINT32   fd,
    UCHAR*   pVirtualAddr,
    UINT32   size,
    INT32    width,
    INT32    height,
    INT32    stride,
    INT32    scanline)
{
    BUFMAPINFO mapInfo;
    CDKResult result        = CDKResultSuccess;

    EGLint vEGLAttribs[]    = { EGL_WIDTH,  width, EGL_HEIGHT, height, EGL_NONE };

    std::map<UINT32, BUFMAPINFO> ::iterator it;
    it = m_SourceTextureMap.find(fd);
    if (it != m_SourceTextureMap.end())
    {
        CHICDK_LOG_VERBOSE("Map Hit! index : %d", fd);
        mapInfo = it->second;
        if (mapInfo.pAddr == pVirtualAddr)
        {
            CHICDK_LOG_VERBOSE("m_ReqNum %llu Inserted fd %d tex %d!", (long long int)m_ReqNum, fd, mapInfo.hColorTexture);
            return mapInfo.hColorTexture;
        }
        else
        {
            CHICDK_LOG_VERBOSE("Flushing element for idx %d ", mapInfo.index);
            m_pfngldeletetextures(1, &mapInfo.hColorTexture);

            if ( NULL != mapInfo.hEGLImage)
            {
                m_pfneglDestroyImageKHR(m_pfneglgetcurrentdisplay(), mapInfo.hEGLImage);
                CheckEglError("Destroy image");
            }
            // Free metadata buffer
            ImageBufferRelease(&mapInfo.hMetaBuffer);
            // Free handle
#if defined(ANDROID_Q_AOSP)
            mapInfo.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(mapInfo.phPrivateHandle));
#else
            mapInfo.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(mapInfo.NativeHandle));
#endif
            m_SourceTextureMap.erase(it);
            //just making sure to delete fd.
            m_SourceTextureMap.erase(fd);

        }
    }

    CHICDK_LOG_VERBOSE("Map miss! Need to add mapping index : %d", fd);

    memset(&mapInfo, 0, sizeof(BUFMAPINFO));

    mapInfo.pGrallocUtils = CamX::Gralloc::GetInstance();

    mapInfo.index = fd;
    result = ImageBufferGet(sizeof(MetaData_t), &mapInfo.hMetaBuffer);

    if (result != CDKResultSuccess)
    {
        CHICDK_LOG_ERROR("Cannot alloc meta buffer");
        //return (GLuint)0;
    }

    MetaData_t *pMetaData = (MetaData_t *)(mapInfo.hMetaBuffer.pVirtualAddr);

    INT32 dupfd = dup(fd);
    if (dupfd == -1)
    {
        CHICDK_LOG_ERROR("Unable to duplicate fd");
        return 0;
    }

    INT32 dupMetaBufferfd = dup(mapInfo.hMetaBuffer.bufferFd);
    if (dupMetaBufferfd == -1)
    {
        CHICDK_LOG_ERROR("Unable to duplicate fd");
        return 0;
    }

#if defined(ANDROID_Q_AOSP)
    mapInfo.phPrivateHandle = new private_handle_t(dupfd, dupMetaBufferfd,
                            m_PrivatHandleFlagsInput, stride, scanline, width, height,
                            m_formatInput, m_bufferType, size);

    if (mapInfo.phPrivateHandle == NULL)
    {
        CHICDK_LOG_ERROR("Create private_handle_t failed!!");
        return 0;
    }

    INT32 error = mapInfo.pGrallocUtils->Retain(reinterpret_cast<CamX::BufferHandle>(mapInfo.phPrivateHandle));
    CHICDK_LOG_VERBOSE("gralloc_Retain return = %d, buffer_handle = %p",
        error, mapInfo.phPrivateHandle);

    BufferDim_t dim;
    dim.sliceWidth = stride;
    dim.sliceHeight = scanline;
    INT32 ret = setMetaData(mapInfo.phPrivateHandle, UPDATE_BUFFER_GEOMETRY, &dim);
    if (ret < 0)
    {
        CHICDK_LOG_ERROR("SetMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", ret);
        //return 0;
    }

    ColorSpace_t colorSpace = ITU_R_601_FR;
    ret = setMetaData(mapInfo.phPrivateHandle, UPDATE_COLOR_SPACE, &colorSpace);
    if (ret < 0)
    {
        CHICDK_LOG_ERROR("SetMetaData(UPDATE_COLOR_SPACE) failed %d", ret);
        //return 0;
    }

    mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, m_formatInput,
                                            m_usage, mapInfo.phPrivateHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
        CHICDK_LOG_ERROR("GraphicBuffer Creation Failed \n");
        return 0;
    }

#else
    mapInfo.NativeHandle = native_handle_create(2, 8);
    (mapInfo.NativeHandle)->data[0] = dupfd;
    (mapInfo.NativeHandle)->data[1] = dupMetaBufferfd;
    (mapInfo.NativeHandle)->data[2] = m_PrivatHandleFlagsInput;
    (mapInfo.NativeHandle)->data[3] = stride;
    (mapInfo.NativeHandle)->data[4] = scanline;
    (mapInfo.NativeHandle)->data[5] = width;
    (mapInfo.NativeHandle)->data[6] = height;
    (mapInfo.NativeHandle)->data[7] = m_formatInput;
    (mapInfo.NativeHandle)->data[8] = m_bufferType;
    (mapInfo.NativeHandle)->data[8] = size;

    if (mapInfo.NativeHandle == NULL)
    {
        CHICDK_LOG_ERROR("Create native_handle_t failed!!");
        return 0;
    }

    INT32 error = mapInfo.pGrallocUtils->Retain(reinterpret_cast<CamX::BufferHandle>(mapInfo.NativeHandle));
    CHICDK_LOG_VERBOSE("gralloc_Retain return = %d, buffer_handle = %p",
        error, mapInfo.NativeHandle);

    BufferDim_t dim;
    dim.sliceWidth  = stride;
    dim.sliceHeight = scanline;

    result =  CamX::Gralloc::SetGrallocMetaData(static_cast<CamX::BufferHandle>(mapInfo.NativeHandle),
                                                        UPDATE_BUFFER_GEOMETRY, sizeof(BufferDim_t),&dim);

    if (CamxResultSuccess != result)
    {
        CHICDK_LOG_ERROR("SetGrallocMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", result);
    }

    ColorMetaData bufferMetadata  = {};
    bufferMetadata.colorPrimaries = ColorPrimaries_BT601_6_525;
    bufferMetadata.range          = Range_Full;
    result = CamX::Gralloc::SetGrallocMetaData(static_cast<CamX::BufferHandle>(mapInfo.NativeHandle),
                                            COLOR_METADATA, sizeof(ColorMetaData), &bufferMetadata);

    if (CamxResultSuccess != result)
    {
        CHICDK_LOG_ERROR("SetGrallocMetaData(COLOR_METADATA) failed %d",
                  result, pImage->pNativeHandle);
    }

    mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, m_formatInput,
                                            m_usage, mapInfo.NativeHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
        CHICDK_LOG_ERROR("GraphicBuffer Creation Failed \n");
        return 0;
    }
#endif

    EGLClientBuffer pNativeBuffer = (EGLClientBuffer)static_cast<ANativeWindowBuffer*>
                                                                (mapInfo.pNativeBuffer);

    mapInfo.hEGLImage = m_pfneglcreateimagekhr(m_pfneglgetcurrentdisplay(),
                                               EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                               pNativeBuffer, vEGLAttribs);
    if (!mapInfo.hEGLImage)
    {
        CheckEglError("EGLImage creation");
        return 0;
    }

    m_pfnglgentextures(1, &mapInfo.hColorTexture);
    if (CDKResultSuccess != CheckglError("glGenTextures"))
    {
        return 0;
    }

    m_pfnglbindtexture(GL_TEXTURE_EXTERNAL_OES, mapInfo.hColorTexture);
    CheckglError("glBindTexture");
    m_pfngleglImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, mapInfo.hEGLImage);
    CheckEglError("glEGLImageTargetTexture2DOES");
    CheckglError("glEGLImageTargetTexture2DOES");
    m_pfngltexparameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_pfngltexparameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CheckglError("glTexParameteri");

    mapInfo.isInputBuffer = TRUE;
    mapInfo.hRBO = 0;
    mapInfo.hFBO = 0;
    mapInfo.pAddr = pVirtualAddr;
    m_SourceTextureMap[fd] = mapInfo;

    CHICDK_LOG_VERBOSE("m_ReqNum %llu Inserted fd %d tex %d!", (long long int)m_ReqNum, fd, mapInfo.hColorTexture);

    return mapInfo.hColorTexture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GPUOpenGL::GetOutputFrameBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLuint GPUOpenGL::GetOutputFrameBuffer(
    UINT32   fd,
    UCHAR*   pVirtualAddr,
    UINT32   size,
    INT32    width,
    INT32    height,
    INT32    stride,
    INT32    scanline)
{
    BUFMAPINFO mapInfo;
    CDKResult result        = CDKResultSuccess;
    EGLint vEGLAttribs[]    = { EGL_WIDTH,  width, EGL_HEIGHT, height, EGL_NONE };

    std::map<UINT32, BUFMAPINFO> ::iterator it;
    it = m_FrameBufferMap.find(fd);
    if (it != m_FrameBufferMap.end())
    {
        CHICDK_LOG_VERBOSE("Map Hit! index : %d", fd);
        mapInfo = it->second;
        if (mapInfo.pAddr == pVirtualAddr)
        {
            CHICDK_LOG_VERBOSE("m_ReqNum %llu Inserted fd %d fbo %d (GLeglImageOES)mapInfo.hEGLImage %p!", (long long int)m_ReqNum, fd, mapInfo.hFBO, (GLeglImageOES)mapInfo.hEGLImage);
            return mapInfo.hFBO;
        }
        else
        {
            CHICDK_LOG_VERBOSE("Flushing element for idx %d ", mapInfo.index);
            m_pfngldeleteframebuffers(1, &mapInfo.hFBO);
            m_pfngldeleterenderbuffers(1, &mapInfo.hRBO);

            if (mapInfo.hEGLImage)
            {
                m_pfneglDestroyImageKHR(m_pfneglgetcurrentdisplay(), mapInfo.hEGLImage);
                CheckEglError("Destroy image");
            }
            // Free metadata buffer
            ImageBufferRelease(&mapInfo.hMetaBuffer);
            // Free handle
#if defined(ANDROID_Q_AOSP)
            mapInfo.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(mapInfo.phPrivateHandle));
#else
            mapInfo.pGrallocUtils->Destroy(reinterpret_cast<CamX::BufferHandle>(mapInfo.NativeHandle));
#endif
            m_FrameBufferMap.erase(it);
            //just making sure to delete fd.
            m_FrameBufferMap.erase(fd);
        }

    }
    memset(&mapInfo, 0, sizeof(BUFMAPINFO));
    CHICDK_LOG_VERBOSE("Map miss! Need to add mapping index : %d", fd);


    mapInfo.pGrallocUtils = CamX::Gralloc::GetInstance();

    mapInfo.index = fd;

    result = ImageBufferGet(sizeof(MetaData_t), &mapInfo.hMetaBuffer);

    if (result != CDKResultSuccess)
    {
        CHICDK_LOG_ERROR(" cannot alloc meta buffer");
        //return (GLuint)0;
    }

    MetaData_t *pMetaData = (MetaData_t *)(mapInfo.hMetaBuffer.pVirtualAddr);

    INT32 dupfd = dup(fd);
    if (dupfd == -1)
    {
        CHICDK_LOG_ERROR("Unable to duplicate fd");
        return 0;
    }
    INT32 dupMetaBufferfd = dup(mapInfo.hMetaBuffer.bufferFd);
    if (dupMetaBufferfd == -1)
    {
        CHICDK_LOG_ERROR("Unable to duplicate fd");
        return 0;
    }

#if defined(ANDROID_Q_AOSP)
    mapInfo.phPrivateHandle = new private_handle_t(dupfd, dupMetaBufferfd,
                              m_PrivatHandleFlagsOutput, stride, scanline, width, height,
                              m_formatOutput, m_bufferType, size);

    if (mapInfo.phPrivateHandle == NULL)
    {
        CHICDK_LOG_ERROR("create private_handle_t failed!!");
        return 0;
    }

    INT32 error = mapInfo.pGrallocUtils->Retain(reinterpret_cast<CamX::BufferHandle>(mapInfo.phPrivateHandle));
    CHICDK_LOG_VERBOSE("gralloc_Retain return = %d, buffer_handle = %p",
        error, mapInfo.phPrivateHandle);

    BufferDim_t dim;
    dim.sliceWidth = stride;
    dim.sliceHeight = scanline;
    INT32 ret = setMetaData(mapInfo.phPrivateHandle, UPDATE_BUFFER_GEOMETRY, &dim);
    if (ret < 0)
    {
        CHICDK_LOG_ERROR("setMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", ret);
        //return 0;
    }

    ColorSpace_t colorSpace = ITU_R_601_FR;
    ret = setMetaData(mapInfo.phPrivateHandle, UPDATE_COLOR_SPACE, &colorSpace);
    if (ret < 0)
    {
        CHICDK_LOG_ERROR("setMetaData(UPDATE_COLOR_SPACE) failed %d", ret);
        //return 0;
    }

    mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, m_formatOutput,
                                            m_usage, mapInfo.phPrivateHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
        CHICDK_LOG_ERROR("GraphicBuffer Creation Failed \n");
        return 0;
    }

#else
    mapInfo.NativeHandle = native_handle_create(2, 8);
    (mapInfo.NativeHandle)->data[0] = dupfd;
    (mapInfo.NativeHandle)->data[1] = dupMetaBufferfd;
    (mapInfo.NativeHandle)->data[2] = m_PrivatHandleFlagsOutput;
    (mapInfo.NativeHandle)->data[3] = stride;
    (mapInfo.NativeHandle)->data[4] = scanline;
    (mapInfo.NativeHandle)->data[5] = width;
    (mapInfo.NativeHandle)->data[6] = height;
    (mapInfo.NativeHandle)->data[7] = m_formatOutput;
    (mapInfo.NativeHandle)->data[8] = m_bufferType;
    (mapInfo.NativeHandle)->data[8] = size;


    if (mapInfo.NativeHandle == NULL)
    {
        CHICDK_LOG_ERROR("create native_handle_t failed!!");
        return 0;
    }

    INT32 error = mapInfo.pGrallocUtils->Retain(reinterpret_cast<CamX::BufferHandle>(mapInfo.NativeHandle));
    CHICDK_LOG_VERBOSE("gralloc_Retain return = %d, buffer_handle = %p",
        error, mapInfo.NativeHandle);

    BufferDim_t dim;
    dim.sliceWidth  = stride;
    dim.sliceHeight = scanline;

    result =  CamX::Gralloc::SetGrallocMetaData(static_cast<CamX::BufferHandle>(mapInfo.NativeHandle),
                                                        UPDATE_BUFFER_GEOMETRY, sizeof(BufferDim_t),&dim);

    if (CamxResultSuccess != result)
    {
        CHICDK_LOG_ERROR("SetGrallocMetaData(UPDATE_BUFFER_GEOMETRY) failed %d", result);
    }

    ColorMetaData bufferMetadata  = {};
    bufferMetadata.colorPrimaries = ColorPrimaries_BT601_6_525;
    bufferMetadata.range          = Range_Full;
    result = CamX::Gralloc::SetGrallocMetaData(static_cast<CamX::BufferHandle>(mapInfo.NativeHandle),
                                            COLOR_METADATA, sizeof(ColorMetaData), &bufferMetadata);

    if (CamxResultSuccess != result)
    {
        CHICDK_LOG_ERROR("SetGrallocMetaData(COLOR_METADATA) failed %d",
                  result, pImage->pNativeHandle);
    }

    mapInfo.pNativeBuffer = new NativeBuffer(width, height, stride, m_formatOutput,
                                            m_usage, mapInfo.NativeHandle);
    if (mapInfo.pNativeBuffer == NULL)
    {
        CHICDK_LOG_ERROR("GraphicBuffer Creation Failed \n");
        return 0;
    }
#endif

    EGLClientBuffer pNativeBuffer = (EGLClientBuffer)static_cast<ANativeWindowBuffer*>(mapInfo.pNativeBuffer);
    mapInfo.hEGLImage = m_pfneglcreateimagekhr(m_pfneglgetcurrentdisplay(),
        EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, pNativeBuffer, vEGLAttribs);
    if (!mapInfo.hEGLImage)
    {
        CheckEglError("EGLImage creation");
        return 0;
    }

    m_pfnglgenframebuffers(1, &mapInfo.hFBO);
    if (CDKResultSuccess != CheckglError("glGenFramebuffers"))
    {
        return 0;
    }
    m_pfnglbindframebuffer(GL_FRAMEBUFFER, mapInfo.hFBO);
    CheckglError("glBindFramebuffer");
    m_pfnglgenrenderbuffers(1, &mapInfo.hRBO);
    CheckglError("glGenRenderbuffers");
    m_pfnglbindrenderbuffer(GL_RENDERBUFFER, mapInfo.hRBO);
    CheckglError("glBindRenderbuffer");
    m_pfngleglImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, (GLeglImageOES)mapInfo.hEGLImage);
    CheckglError("glEGLImageTargetRenderbufferStorageOES");
    CheckEglError("glEGLImageTargetRenderbufferStorageOES");

    m_pfnglframebufferrenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mapInfo.hRBO);
    CheckglError("glFramebufferRenderbuffer");
    m_pfnglbindframebuffer(GL_FRAMEBUFFER, 0);
    CheckglError("glBindFramebuffer");
    m_pfnglbindrenderbuffer(GL_RENDERBUFFER, 0);
    CheckglError("glBindRenderbuffer");

    mapInfo.isInputBuffer = FALSE;
    mapInfo.hColorTexture = 0;
    mapInfo.pAddr         = pVirtualAddr;
    m_FrameBufferMap[fd] = mapInfo;
    CHICDK_LOG_VERBOSE("m_ReqNum %llu Inserted fd %d fbo %d (GLeglImageOES)mapInfo.hEGLImage %p!",
        (long long int)m_ReqNum, fd, mapInfo.hFBO, (GLeglImageOES)mapInfo.hEGLImage);

    return mapInfo.hFBO;
}

// =============================================================================================================================
// END OpenGL Stuff
// =============================================================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeGetCaps
///
/// @brief  Implementation of PFNNODEGETCAPS defined in chinode.h
///
/// @param  pCapsInfo   Pointer to a structure that defines the capabilities supported by the node.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeGetCaps(
    CHINODECAPSINFO* pCapsInfo)
{
    CDKResult result = CDKResultSuccess;

    if (pCapsInfo == NULL)
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument: pCapsInfo is NULL");
    }

    if ((NULL != pCapsInfo) && (CDKResultSuccess != result))
    {
        if (pCapsInfo->size >= sizeof(CHINODECAPSINFO))
        {
            pCapsInfo->nodeCapsMask = ChiNodeCapsDewarpEISV3 | ChiNodeCapsDewarpEISV2;
        }
        else
        {
            CHICDK_LOG_ERROR("CHINODECAPSINFO is smaller than expected");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeCreate
///
/// @brief  Implementation of PFNNODECREATE defined in chinode.h
///
/// @param  pCreateInfo Pointer to a structure that defines create session information for the node.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeCreate(
    CHINODECREATEINFO* pCreateInfo)
{
    CDKResult        result = CDKResultSuccess;
    ChiDewarpNode*   pNode = NULL;

    if (NULL == pCreateInfo)
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument: pTagTypeInfo is NULL");
    }

    if ((CDKResultSuccess == result) && (pCreateInfo->size < sizeof(CHINODECREATEINFO)))
    {
        CHICDK_LOG_ERROR("CHINODECREATEINFO is smaller than expected");
        result = CDKResultEFailed;
    }

    if (CDKResultSuccess == result)
    {
        pNode = new ChiDewarpNode;
        if (pNode == NULL)
        {
            result = CDKResultENoMemory;
        }
    }

    if (CDKResultSuccess == result)
    {
        result = pNode->Initialize(pCreateInfo);
    }

    if (CDKResultSuccess == result)
    {
        pCreateInfo->phNodeSession = reinterpret_cast<CHIHANDLE*>(pNode);
    }

    if (CDKResultSuccess != result)
    {
        delete pNode;
        pNode = NULL;
    }

    CHIVENDORTAGBASEINFO vendorTagBase = { 0 };
    CHIMETADATAINFO      metadataInfo = { 0 };
    const UINT32         tagSize = 1;
    UINT32               index = 0;
    CHITAGDATA           tagData[tagSize] = { { 0 } };
    UINT32               tagList[tagSize];
    UINT32               extraFrameworkBuffers = EXTRA_FRAMEWORK_BUFFERS;
    // Additional framework buffers are needed since first request processing delay is increased for EIS Talos usecase
    // due to addition of 4 chi nodes (2 for downscaling and 2 for dewarp) in the pipeline

    metadataInfo.size = sizeof(CHIMETADATAINFO);
    if (NULL == pCreateInfo)
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument: pTagTypeInfo is NULL");
    }
    else
    {
        metadataInfo.chiSession = pCreateInfo->hChiSession;
        metadataInfo.tagNum     = tagSize;
        metadataInfo.pTagList   = &tagList[0];
        metadataInfo.pTagData   = &tagData[0];
     }

    tagList[index] = (g_vendorTagId.ExtraFrameworkBuffersTagId | UsecaseMetadataSectionMask);
    tagData[index].size = sizeof(CHITAGDATA);
    tagData[index].requestId = 0;
    tagData[index].pData = &extraFrameworkBuffers;
    tagData[index].dataSize = g_VendorTagSectionICAGpu[2].numUnits;
    index++;
    CHICDK_LOG_VERBOSE("Publishing ExtraFrameworkBuffers %d", extraFrameworkBuffers);
    g_ChiNodeInterface.pSetMetadata(&metadataInfo);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeDestroy
///
/// @brief  Implementation of PFNNODEDESTROY defined in chinode.h
///
/// @param  pDestroyInfo    Pointer to a structure that defines the session destroy information for the node.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeDestroy(
    CHINODEDESTROYINFO* pDestroyInfo)
{
    CDKResult result = CDKResultSuccess;

    if ((NULL == pDestroyInfo) || (NULL == pDestroyInfo->hNodeSession))
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument: pDestroyInfo is NULL");
    }

    if (CDKResultSuccess == result)
    {
        if (pDestroyInfo->size >= sizeof(CHINODEDESTROYINFO))
        {
            ChiDewarpNode* pNode = static_cast<ChiDewarpNode*>(pDestroyInfo->hNodeSession);
            delete pNode;

            pNode = NULL;
            pDestroyInfo->hNodeSession = NULL;
        }
        else
        {
            CHICDK_LOG_ERROR("CHINODEDESTROYINFO is smaller than expected");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeQueryBufferInfo
///
/// @brief  Implementation of PFNNODEQUERYBUFFERINFO defined in chinode.h
///
/// @param  pQueryBufferInfo    Pointer to a structure to query the input buffer resolution and type.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeQueryBufferInfo(
    CHINODEQUERYBUFFERINFO* pQueryBufferInfo)
{
    CDKResult result = CDKResultSuccess;

    if ((NULL == pQueryBufferInfo) || (NULL == pQueryBufferInfo->hNodeSession))
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument");
    }

    if (CDKResultSuccess == result)
    {
        if (pQueryBufferInfo->size >= sizeof(CHINODEQUERYBUFFERINFO))
        {
            ChiDewarpNode* pNode = static_cast<ChiDewarpNode*>(pQueryBufferInfo->hNodeSession);
            result = pNode->QueryBufferInfo(pQueryBufferInfo);
        }
        else
        {
            CHICDK_LOG_ERROR("CHINODEQUERYBUFFERINFO is smaller than expected");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeSetBufferInfo
///
/// @brief  Implementation of PFNNODESETBUFFERINFO defined in chinode.h
///
/// @param  pSetBufferInfo  Pointer to a structure with information to set the output buffer resolution and type.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeSetBufferInfo(
    CHINODESETBUFFERPROPERTIESINFO* pSetBufferInfo)
{
    CDKResult result = CDKResultSuccess;

    if ((NULL == pSetBufferInfo) || (NULL == pSetBufferInfo->hNodeSession))
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument");
    }

    if (CDKResultSuccess == result)
    {
        if (pSetBufferInfo->size >= sizeof(CHINODESETBUFFERPROPERTIESINFO))
        {
            ChiDewarpNode* pNode = static_cast<ChiDewarpNode*>(pSetBufferInfo->hNodeSession);
            result = pNode->SetBufferInfo(pSetBufferInfo);
        }
        else
        {
            CHICDK_LOG_ERROR("CHINODESETBUFFERPROPERTIESINFO is smaller than expected");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeProcRequest
///
/// @brief  Implementation of PFNNODEPROCREQUEST defined in chinode.h
///
/// @param  pProcessRequestInfo Pointer to a structure that defines the information required for processing a request.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult DewarpNodeProcRequest(
    CHINODEPROCESSREQUESTINFO* pProcessRequestInfo)
{
    CDKResult result = CDKResultSuccess;

    if ((NULL == pProcessRequestInfo) || (NULL == pProcessRequestInfo->hNodeSession))
    {
        result = CDKResultEInvalidPointer;
        CHICDK_LOG_ERROR("Invalid argument");
    }

    if (CDKResultSuccess == result)
    {
        if (pProcessRequestInfo->size >= sizeof(CHINODEPROCESSREQUESTINFO))
        {
            ChiDewarpNode* pNode = static_cast<ChiDewarpNode*>(pProcessRequestInfo->hNodeSession);
            result = pNode->ProcessRequest(pProcessRequestInfo);
        }
        else
        {
            CHICDK_LOG_ERROR("CHINODEPROCESSREQUESTINFO is smaller than expected");
            result = CDKResultEFailed;
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DewarpNodeSetNodeInterface
///
/// @brief  Implementation of PFCHINODESETNODEINTERFACE defined in chinode.h
///
/// @param  pProcessRequestInfo Pointer to a structure that defines the information required for processing a request.
///
/// @return CDKResultSuccess if success or appropriate error code.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static VOID DewarpNodeSetNodeInterface(
    ChiNodeInterface* pNodeInterface)
{
    CDKResult result = CDKResultSuccess;
    result = ChiNodeUtils::SetNodeInterface(pNodeInterface, DewarpNodeSectionName,
        &g_ChiNodeInterface, &g_vendorTagBase);
}

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ChiNodeEntry
    ///
    /// @brief  Entry point called by the Chi driver to initialize the custom node.
    ///
    /// This function must be exported by every <library>.so in order for driver to initialize the Node. This function is
    /// called during the camera server initialization,which occurs during HAL process start.In addition to communicating
    /// the necessary function pointers between Chi and external nodes, this function allows a node to do any initialization
    /// work that it would typically do at process init. Anything done here should not be specific to a session, and any
    /// variables stored in the node must be protected against multiple sessions accessing it at the same time.
    ///
    /// @param pNodeCallbacks  Pointer to a structure that defines callbacks that the Chi driver sends to the node.
    ///                        The node must fill in these function pointers.
    ///
    /// @return VOID.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CDK_VISIBILITY_PUBLIC VOID ChiNodeEntry(
        CHINODECALLBACKS* pNodeCallbacks)
    {
        CHICDK_LOG_ERROR("Dewarp Node OpenGL CHINODEEntry funtion");

        if (NULL != pNodeCallbacks)
        {
            if (pNodeCallbacks->majorVersion == ChiNodeMajorVersion &&
                pNodeCallbacks->size >= sizeof(CHINODECALLBACKS))
            {
                pNodeCallbacks->majorVersion                = ChiNodeMajorVersion;
                pNodeCallbacks->minorVersion                = ChiNodeMinorVersion;
                pNodeCallbacks->pGetCapabilities            = DewarpNodeGetCaps;
                pNodeCallbacks->pCreate                     = DewarpNodeCreate;
                pNodeCallbacks->pDestroy                    = DewarpNodeDestroy;
                pNodeCallbacks->pQueryBufferInfo            = DewarpNodeQueryBufferInfo;
                pNodeCallbacks->pSetBufferInfo              = DewarpNodeSetBufferInfo;
                pNodeCallbacks->pProcessRequest             = DewarpNodeProcRequest;
                pNodeCallbacks->pChiNodeSetNodeInterface    = DewarpNodeSetNodeInterface;
            }
            else
            {
                CHICDK_LOG_ERROR("Chi API major version doesn't match (%d:%d) vs (%d:%d)",
                            pNodeCallbacks->majorVersion, pNodeCallbacks->minorVersion,
                            ChiNodeMajorVersion, ChiNodeMinorVersion);
            }
        }
        else
        {
            CHICDK_LOG_ERROR("Invalid Argument: %p", pNodeCallbacks);
        }
    }
#ifdef __cplusplus
}
#endif // __cplusplus


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::Initialize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::Initialize(
    CHINODECREATEINFO* pCreateInfo)
{
    CDKResult            result                      = CDKResultSuccess;
    CHIVENDORTAGBASEINFO vendorTagBase               = { 0 };
    CHAR                 setProp[PROPERTY_VALUE_MAX] = "";

    m_EISMarginRequest                    = { 0 };
    m_hChiSession                         = pCreateInfo->hChiSession;
    m_nodeId                              = pCreateInfo->nodeId;
    m_nodeCaps                            = pCreateInfo->nodeCaps.nodeCapsMask;
    m_nodeFlags                           = pCreateInfo->nodeFlags;
    m_processedFrame                      = 0;
    m_instanceId                          = pCreateInfo->nodeInstanceId;
    m_vendorTagBaseIsPreviewStreamPresent = 0;

    property_get("persist.vendor.camera.enabledewarpgriddump", setProp, "0");
    m_bEnableDewarpGridDump = atoi(setProp);
    CHICDK_LOG_INFO("Dewarp nodeCaps %d Grid dump enable %d", m_nodeCaps, m_bEnableDewarpGridDump);

    switch (m_nodeCaps)
    {
        case ChiNodeCapsDewarpEISV3:
            m_stabilizationType             = DewarpStabilizationTypeEISV3;
            break;

        case ChiNodeCapsDewarpEISV2:
            m_stabilizationType             = DewarpStabilizationTypeEISV2;
            break;

        default:
            m_stabilizationType             = DewarpStabilizationTypeNone;
            break;
    }

    m_pDewarpNodeMutex                      = CamX::Mutex::Create("CHIDEWARPNODE");

    if (NULL == m_pDewarpNodeMutex)
    {
        CHICDK_LOG_ERROR("DewarpNodeMutex failed");
        result = CDKResultENoMemory;
    }
    m_pOpenGL                           = GPUOpenGL::GetInstance();
    if (NULL == m_pOpenGL)
    {
        CHICDK_LOG_ERROR("OpenCL instance creation failed");
        result = CDKResultENoMemory;
    }

    // Lets set input buffer dependencies from dewarp itself
    // 1. To avoid unnecessarily going to DRQ 2 times, First from chinodewrapper for input fences, second from dewarp
    //    for property dependencies.
    // 2. To ensure we bind output buffers only after all dependencies (input fences + properties) are met. This will avoid
    //    unnecessary early binding of buffers and so reduces number of buffers required at dewarp node's output port.
    pCreateInfo->nodeFlags.canSetInputBufferDependency = TRUE;

    if (DewarpStabilizationTypeEISV3 == m_stabilizationType)
    {
        result = ChiNodeUtils::GetVendorTagBase(EISDewarpMatrixSectionName,
                                                g_VendorTagSectionICAGpu[0].pVendorTagName,
                                                &g_ChiNodeInterface,
                                                &vendorTagBase);
        if (CDKResultSuccess == result)
        {
            // Save ICA In Perspective Transform Vendor Tag Id
            m_vendorTagId.ICAInPerspectiveTransformTagId = vendorTagBase.vendorTagBase;
            CHICDK_LOG_INFO("ICA In Perspective Transform Vendor Tag Id EISV3");
        }
        else
        {
            CHICDK_LOG_ERROR("Unable to get ICA In Perspective Transform Vendor Tag Id EISV3");
        }
        result = ChiNodeUtils::GetVendorTagBase(EISDewarpMatrixSectionName,
                                                g_VendorTagSectionICAGpu[3].pVendorTagName,
                                                &g_ChiNodeInterface,
                                                &vendorTagBase);
        if (CDKResultSuccess == result)
        {
            m_vendorTagId.ICAInGridTransformTagId = vendorTagBase.vendorTagBase;
            CHICDK_LOG_INFO("ICA In Grid Transform Vendor Tag Id EISV2");
            m_metaTagsInitialized = TRUE;
        }
        else
        {
            CHICDK_LOG_ERROR("Unable to get ICA In Perspective Transform Vendor Tag Id EISV3");
        }
    }
    else if (DewarpStabilizationTypeEISV2 == m_stabilizationType)
    {
        result = ChiNodeUtils::GetVendorTagBase(StreamTypePresent,
            g_VendorTagSectionstreamTypePresent[0].pVendorTagName,
            &g_ChiNodeInterface,
            &vendorTagBase);
        if (CDKResultSuccess == result)
        {
            m_vendorTagBaseIsPreviewStreamPresent = vendorTagBase.vendorTagBase;
            CHICDK_LOG_INFO("StreamTypePresent");
        }
        else
        {
            CHICDK_LOG_ERROR("Unable to get StreamTypePresent Vendor Tag Id %d", result);
        }
        result = ChiNodeUtils::GetVendorTagBase(EISDewarpMatrixSectionName,
                                                g_VendorTagSectionICAGpu[1].pVendorTagName,
                                                &g_ChiNodeInterface,
                                                &vendorTagBase);
        if (CDKResultSuccess == result)
        {
            // Save ICA In Perspective Transform Vendor Tag Id
            m_vendorTagId.ICAInPerspectiveTransformTagId = vendorTagBase.vendorTagBase;
            CHICDK_LOG_INFO("ICA In Perspective Transform Vendor Tag Id EISV2");
        }
        else
        {
            CHICDK_LOG_ERROR("Unable to get ICA In Perspective Transform Vendor Tag Id EISV2");
        }
        result = ChiNodeUtils::GetVendorTagBase(EISDewarpMatrixSectionName,
                                                g_VendorTagSectionICAGpu[4].pVendorTagName,
                                                &g_ChiNodeInterface,
                                                &vendorTagBase);
        if (CDKResultSuccess == result)
        {
            m_vendorTagId.ICAInGridTransformTagId = vendorTagBase.vendorTagBase;
            CHICDK_LOG_INFO("ICA In Grid Transform Vendor Tag Id EISV2");
            m_metaTagsInitialized = TRUE;
        }
        else
        {
            CHICDK_LOG_ERROR("Unable to get ICA In Perspective Transform Vendor Tag Id EISV2");
        }
    }

    result = ChiNodeUtils::GetVendorTagBase(EISDewarpMatrixSectionName,
                                            g_VendorTagSectionICAGpu[2].pVendorTagName,
                                            &g_ChiNodeInterface,
                                            &vendorTagBase);
    if (CDKResultSuccess == result)
    {
        // Save ExtraFrameworkBuffers Vendor Tag Id
        g_vendorTagId.ExtraFrameworkBuffersTagId = vendorTagBase.vendorTagBase;
        CHICDK_LOG_INFO("ExtraFrameworkBuffersTagId Vendor Tag Id EISV3");
        m_metaTagsInitialized &= TRUE;
    }
    else
    {
        CHICDK_LOG_ERROR("Unable to get ExtraFrameworkBuffers Vendor Tag Id EISV3");
    }

    if ((CDKResultSuccess == result) && (TRUE == m_bEnableDewarpGridDump))
    {
        CHIDateTime dateTime = { 0 };
        CHAR dumpFilename[FILENAME_MAX];
        ChiNodeUtils::GetDateTime(&dateTime);

        ChiNodeUtils::SNPrintF(dumpFilename,
            sizeof(dumpFilename),
            "%s%swarp%d_input_%04d%02d%02d_%02d%02d%02d.txt",
            FileDumpPath,
            PathSeparator,
            (DewarpStabilizationTypeEISV2 == m_stabilizationType) ? 2 : 3,
            dateTime.year + 1900,
            dateTime.month + 1,
            dateTime.dayOfMonth,
            dateTime.hours,
            dateTime.minutes,
            dateTime.seconds);

        m_hOutputDumpFile = ChiNodeUtils::FOpen(dumpFilename, "a");
        if (NULL == m_hOutputDumpFile)
        {
            CHICDK_LOG_ERROR("Can't open %s for Warp output dump !", dumpFilename);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::QueryBufferInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::QueryBufferInfo(
    CHINODEQUERYBUFFERINFO* pQueryBufferInfo)
{
    CDKResult result = CDKResultSuccess;

    for (UINT32 i = 0; i < pQueryBufferInfo->numOutputPorts; i++)
    {
        INT                       strideAlignment   = 0;
        INT                       scanlineAlignment = 0;

        CHINODEBUFFERREQUIREMENT* pInputOptions             = &pQueryBufferInfo->pInputOptions[i].inputBufferOption;
        CHINODEBUFFERREQUIREMENT* pOutputOptions            = &pQueryBufferInfo->pOutputPortQueryInfo[i].outputBufferOption;
        CHINODEBUFFERREQUIREMENT* pOutputRequirement        = &pQueryBufferInfo->pOutputPortQueryInfo[i].pBufferRequirement[0];

        ChiFormat                 inputFormat               = pQueryBufferInfo->pInputOptions[i].inputBufferOption.format;
        ChiFormat                 outputFormat              = pQueryBufferInfo->pOutputPortQueryInfo[i].outputBufferOption.format;

        CHICDK_LOG_INFO("Dewarp input port id = %d, format = %d",
            pQueryBufferInfo->pInputOptions[i].inputPortId,
            pQueryBufferInfo->pInputOptions[i].inputBufferOption.format);

        CHICDK_LOG_INFO("Dewarp output port id = %d, format = %d",
            pQueryBufferInfo->pOutputPortQueryInfo[i].outputPortId,
            pQueryBufferInfo->pOutputPortQueryInfo[i].outputBufferOption.format);
        // set the nodeCaps later
        switch (m_nodeCaps)
        {
            case ChiNodeCapsDewarpEISV3:
                SetStabilizedOutputDimsLookAhead(pOutputRequirement->optimalH, pOutputRequirement->optimalW);
                GetEisV3Margin();
                break;

            case ChiNodeCapsDewarpEISV2:
                SetStabilizedOutputDimsRealTime(pOutputRequirement->optimalH, pOutputRequirement->optimalW);
                GetEisV2Margin();
                break;

            default:
                break;
        }

        pInputOptions->optimalW     = pOutputRequirement->optimalW;
        pInputOptions->optimalH     = pOutputRequirement->optimalH;
        pInputOptions->optimalW    += static_cast<UINT32>
                                       (pOutputRequirement->optimalW *
                                       m_EISMarginRequest.widthMargin);
        pInputOptions->optimalH    += static_cast<UINT32>
                                       (pOutputRequirement->optimalH *
                                       m_EISMarginRequest.heightMargin);

        pInputOptions->minW         = pOutputRequirement->minW;
        pInputOptions->minH         = pOutputRequirement->minH;
        pInputOptions->maxW         = ChiNodeUtils::MaxUINT32(pOutputRequirement->maxW, pInputOptions->optimalW);
        pInputOptions->maxH         = ChiNodeUtils::MaxUINT32(pOutputRequirement->maxH, pInputOptions->optimalH);

        for (UINT planeId = 0; planeId < NUMBEROFPLANES; planeId++)
        {
            ChiNodeUtils::GetAlignment(pInputOptions->optimalW,
                                       pInputOptions->optimalH,
                                       planeId,
                                       inputFormat,
                                       &strideAlignment,
                                       &scanlineAlignment);

            pInputOptions->planeAlignment[planeId].strideAlignment   = strideAlignment;
            pInputOptions->planeAlignment[planeId].scanlineAlignment = scanlineAlignment;
        }
        pOutputOptions->minW        = pOutputRequirement->minW;
        pOutputOptions->minH        = pOutputRequirement->minH;
        pOutputOptions->maxW        = pOutputRequirement->maxW;
        pOutputOptions->maxH        = pOutputRequirement->maxH;
        pOutputOptions->optimalW    = pOutputRequirement->optimalW;
        pOutputOptions->optimalH    = pOutputRequirement->optimalH;

        for (UINT planeId = 0; planeId < NUMBEROFPLANES; planeId++)
        {
            ChiNodeUtils::GetAlignment(pOutputOptions->optimalW,
                                       pOutputOptions->optimalH,
                                       planeId,
                                       outputFormat,
                                       &strideAlignment,
                                       &scanlineAlignment);

            pOutputOptions->planeAlignment[planeId].strideAlignment   = strideAlignment;
            pOutputOptions->planeAlignment[planeId].scanlineAlignment = scanlineAlignment;
        }

        CHICDK_LOG_INFO("Dewarp Input Alignments: %d, %d, %d, %d", pInputOptions->planeAlignment[0].strideAlignment,
            pInputOptions->planeAlignment[0].scanlineAlignment, pInputOptions->planeAlignment[1].strideAlignment,
            pInputOptions->planeAlignment[1].scanlineAlignment);

        CHICDK_LOG_INFO("Dewarp Output Alignments: %d, %d, %d, %d", pOutputOptions->planeAlignment[0].strideAlignment,
            pOutputOptions->planeAlignment[0].scanlineAlignment, pOutputOptions->planeAlignment[1].strideAlignment,
            pOutputOptions->planeAlignment[1].scanlineAlignment);

        CHICDK_LOG_INFO("Dewarp [%d, %d] - [%d, %d] - [%d, %d] - [%d, %d]",
                                    pInputOptions->optimalW,
                                    pInputOptions->optimalH,
                                    pInputOptions->minW,
                                    pInputOptions->minH,
                                    pInputOptions->maxW,
                                    pInputOptions->maxH,
                                    pOutputOptions->optimalW,
                                    pOutputOptions->optimalH);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::SetBufferInfo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::SetBufferInfo(
    CHINODESETBUFFERPROPERTIESINFO* pSetBufferInfo)
{
    m_format.width              = pSetBufferInfo->pFormat->width;
    m_format.height             = pSetBufferInfo->pFormat->height;
    m_fullOutputDimensions[0]   = pSetBufferInfo->pFormat->width;
    m_fullOutputDimensions[1]   = pSetBufferInfo->pFormat->height;

    return CDKResultSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// WriteGridToFile
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void WriteGridToFile(FILE* dumpFile, IPEICAGridTransform* pGrid, UINT64 frameNum)
{
    CHAR dumpData[64 * 1024] = { 0 };
    CHAR*  pos = &dumpData[0];
    UINT32 charWritten = 0;


    if (!pGrid->gridTransformEnable)
    {
        return; // nothing to do
    }

    memset(dumpData, 0x00, sizeof(dumpData));

    charWritten = ChiNodeUtils::SNPrintF(pos,
        sizeof(dumpData),
        "Frame %" PRIu64 ", %u, %u, ",
        frameNum,
        pGrid->transformDefinedOnWidth,
        pGrid->transformDefinedOnHeight);
    pos += charWritten;

    for (uint32_t i = 0; i < 35 * 27 - 1; i++)
    {
        charWritten = ChiNodeUtils::SNPrintF(pos,
            sizeof(dumpData),
            "%4.4lf, %4.4lf, ",
            pGrid->gridTransformArray[i].x,
            pGrid->gridTransformArray[i].y);

        pos += charWritten;
    }
    charWritten = ChiNodeUtils::SNPrintF(pos,
        sizeof(dumpData),
        "%4.4lf, %4.4lf \n",
        pGrid->gridTransformArray[35 * 27 - 1].x,
        pGrid->gridTransformArray[35 * 27 - 1].y);
    pos += charWritten;

    //    gridTransformArrayExtrapolatedCorners, gridTransformArrayCorners

    SIZE_T objCount = ChiNodeUtils::FWrite(dumpData, pos - dumpData, 1, dumpFile);

    if (1 != objCount)
    {
        CHICDK_LOG_ERROR("File write failed !");
    }
    else
    {
        CHICDK_LOG_VERBOSE("%zd byte successfully written to file", pos - dumpData);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::ProcessRequest
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::ProcessRequest(
    CHINODEPROCESSREQUESTINFO* pProcessRequestInfo)
{
    CHIDEPENDENCYINFO* pDependencyInfo  = pProcessRequestInfo->pDependency;
    UINT16             depCount         = 0;
    UINT16             sequenceNumber   = 0;
    CDKResult          result           = CDKResultSuccess;

    m_ReqNum                            = (long long int)pProcessRequestInfo->frameNum;

    sequenceNumber                      = pProcessRequestInfo->pDependency->processSequenceId;
    CHICDK_LOG_INFO("DewarpEIS%d Req[%llu] Seq number %d",
        m_stabilizationType, (long long int)pProcessRequestInfo->frameNum, sequenceNumber);

    if ((DewarpStabilizationTypeEISV2 == m_stabilizationType) && m_vendorTagBaseIsPreviewStreamPresent)
    {
        VOID* pData                  = NULL;
        BOOL  bPreviewStreamPresent  = FALSE;

        pData = ChiNodeUtils::GetMetaData(pProcessRequestInfo->frameNum,
            m_vendorTagBaseIsPreviewStreamPresent,
            ChiMetadataDynamic,
            &g_ChiNodeInterface, m_hChiSession);
        if (NULL != pData)
        {
             bPreviewStreamPresent = *static_cast<CHAR*>(pData);
             CHICDK_LOG_VERBOSE("bPreviewStreamPresent  %d, request %llu ", bPreviewStreamPresent,
                (long long int)pProcessRequestInfo->frameNum);
        }
        else
        {
            CHICDK_LOG_VERBOSE("Unable to read VendorTag bPreviewStreamPresent request %llu",
                (long long int)pProcessRequestInfo->frameNum);
        }

        if (FALSE == bPreviewStreamPresent)
        {
            CHICDK_LOG_VERBOSE("This is time to return request %llu", (long long int)pProcessRequestInfo->frameNum);
            return result;
        }
    }

    if (0 == sequenceNumber)
    {
        //< If the sequence number is zero then it means we are not called from the DRQ, in which case we need to set
        //< dependencies
        depCount = 0;

        // Query Vendor Tag Ids
        if (FALSE == m_metaTagsInitialized)
        {
            CHICDK_LOG_ERROR("Unable to query required vendor tag locations");
        }

        pDependencyInfo->inputBufferFenceCount = 0;

        for (UINT32 i = 0; i < pProcessRequestInfo->inputNum; i++)
        {
            if ((NULL != pProcessRequestInfo->phInputBuffer)                    &&
                (NULL != pProcessRequestInfo->phInputBuffer[i])                 &&
                (NULL != pProcessRequestInfo->phInputBuffer[i]->pfenceHandle)   &&
                (NULL != pProcessRequestInfo->phInputBuffer[i]->pIsFenceSignaled))
            {
                pDependencyInfo->pInputBufferFence[pDependencyInfo->inputBufferFenceCount]              =
                    pProcessRequestInfo->phInputBuffer[i]->pfenceHandle;
                pDependencyInfo->pInputBufferFenceIsSignaled[pDependencyInfo->inputBufferFenceCount]    =
                    pProcessRequestInfo->phInputBuffer[i]->pIsFenceSignaled;

                pDependencyInfo->inputBufferFenceCount++;

                CHICDK_LOG_VERBOSE("DewarpEIS%d Req[%llu] [%d] Fence=%p(%d) Signalled=%p(%d)",
                            m_stabilizationType, (long long int)pProcessRequestInfo->frameNum, i,
                            pProcessRequestInfo->phInputBuffer[i]->pfenceHandle,
                            *(reinterpret_cast<UINT*>(pProcessRequestInfo->phInputBuffer[i]->pfenceHandle)),
                            pProcessRequestInfo->phInputBuffer[i]->pIsFenceSignaled,
                            *(reinterpret_cast<UINT*>(pProcessRequestInfo->phInputBuffer[i]->pIsFenceSignaled)));
            }
        }

        pDependencyInfo->properties[depCount]                               = m_vendorTagId.ICAInGridTransformTagId;
        pDependencyInfo->offsets[depCount]                                  = 0;
        pDependencyInfo->count                                              = ++depCount;
        pDependencyInfo->hNodeSession                                       = m_hChiSession;
        pProcessRequestInfo->pDependency->processSequenceId                 = 1;
        pProcessRequestInfo->pDependency->hasIOBufferAvailabilityDependency = TRUE;

        if (DewarpStabilizationTypeEISV3 == m_stabilizationType)
        {
            // Send the Request Done for this node in order to not delay the preview buffer
            CHINODEPROCESSMETADATADONEINFO metadataDoneInfo;
            metadataDoneInfo.size                               = sizeof(metadataDoneInfo);
            metadataDoneInfo.hChiSession                        = m_hChiSession;
            metadataDoneInfo.frameNum                           = pProcessRequestInfo->frameNum;
            metadataDoneInfo.result                             = CDKResultSuccess;

            g_ChiNodeInterface.pProcessMetadataDone(&metadataDoneInfo);
        }
    }
    else if (1 == sequenceNumber)
    {
        VOID* pData = NULL;
        pData       = ChiNodeUtils::GetMetaData(pProcessRequestInfo->frameNum,
                                                m_vendorTagId.ICAInGridTransformTagId,
                                                ChiMetadataDynamic,
                                                &g_ChiNodeInterface,
                                                m_hChiSession);

        if (NULL == pData)
        {
            CHICDK_LOG_ERROR("Unable to query m_vendorTagId.ICAInGridTransformTagId vendor tag locations");
        }
        else
        {
            IPEICAGridTransform* pIPEICAGridTransformGrid = static_cast<IPEICAGridTransform*>(pData);
            if (NULL != m_hOutputDumpFile)
            {
                WriteGridToFile(m_hOutputDumpFile, pIPEICAGridTransformGrid, pProcessRequestInfo->frameNum);
            }

            m_pDewarpNodeMutex->Lock();

            DewarpImage(pProcessRequestInfo->phOutputBuffer[0],
                        pProcessRequestInfo->phInputBuffer[0],
                        pIPEICAGridTransformGrid);

            m_pDewarpNodeMutex->Unlock();

            m_processedFrame++;

            // Metadata Done is being handled already, the wrapper does not need to handle this for EIS3
            if (DewarpStabilizationTypeEISV3 == m_stabilizationType)
            {
                pProcessRequestInfo->isEarlyMetadataDone = TRUE;
            }
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::ChiDewarpNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiDewarpNode::ChiDewarpNode()
    : m_hChiSession(NULL)
    , m_nodeId(0)
    , m_nodeCaps(0)
    , m_processedFrame(0)
    , m_metaTagsInitialized(FALSE)
    , m_hOutputDumpFile(NULL)
{
    CHICDK_LOG_VERBOSE("ChiDewarpNode Constructor");
    memset(&m_format, 0, sizeof(CHINODEIMAGEFORMAT));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::~ChiDewarpNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChiDewarpNode::~ChiDewarpNode()
{
    if (NULL != m_pOpenGL)
    {
        m_pOpenGL->Uninitialize();
        CAMX_FREE(m_pOpenGL);
        m_pOpenGL = NULL;
    }

    if (NULL != m_pDewarpNodeMutex)
    {
        m_pDewarpNodeMutex->Destroy();
        m_pDewarpNodeMutex = NULL;
    }

    if (NULL != m_hOutputDumpFile)
    {
        CamxResult result = ChiNodeUtils::FClose(m_hOutputDumpFile);
        if (CDKResultSuccess != result)
        {
            CHICDK_LOG_ERROR("Warp dump file close failed");
        }
        m_hOutputDumpFile = NULL;
    }

    m_hChiSession = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::DewarpImage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID ChiDewarpNode::DewarpImage(
    CHINODEBUFFERHANDLE  hOutput,
    CHINODEBUFFERHANDLE  hInput,
    IPEICAGridTransform* pIPEICAGridTransformGrid)
{

    FLOAT stabilizationTransformGrid [TransformGridEISP0size] = { 0 };
    BOOL  eisEnable                                           = FALSE;

    eisEnable = pIPEICAGridTransformGrid->gridTransformEnable;
    if (DewarpStabilizationTypeEISV2 == m_stabilizationType || DewarpStabilizationTypeEISV3 == m_stabilizationType)
    {
        memcpy(stabilizationTransformGrid,
               pIPEICAGridTransformGrid->gridTransformArray,
               sizeof(float)*TransformGridEISP0size);
    }
    else
    {
        eisEnable = FALSE;
    }

    m_pOpenGL->m_ReqNum = m_ReqNum;
    m_pOpenGL->DewarpImage(hOutput, hInput, stabilizationTransformGrid, eisEnable);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::GetMetaData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID* ChiDewarpNode::GetMetaData(
    UINT64 requestId,
    UINT32 tagId)
{
    CHIMETADATAINFO metadataInfo        = { 0 };
    const UINT32    tagSize             = 1;
    CHITAGDATA      tagData             = { 0 };
    UINT32          tagList             = tagId;

    tagData.requestId                   = requestId;
    tagData.offset                      = 0;
    tagData.negate                      = FALSE;

    metadataInfo.size                   = sizeof(CHIMETADATAINFO);
    metadataInfo.chiSession             = m_hChiSession;
    metadataInfo.tagNum                 = tagSize;
    metadataInfo.pTagList               = &tagList;
    metadataInfo.pTagData               = &tagData;
    metadataInfo.metadataType           = ChiMetadataDynamic;

    g_ChiNodeInterface.pGetMetadata(&metadataInfo);
    return tagData.pData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::GetEisV2Margin
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::GetEisV2Margin()
{
    CHIVENDORTAGBASEINFO vendorTagBase  = { 0 };

    CDKResult result                    = CDKResultSuccess;
    result                              = ChiNodeUtils::GetVendorTagBase("org.quic.camera.eisrealtime",
                                                                         "RequestedMargin",
                                                                         &g_ChiNodeInterface,
                                                                         &vendorTagBase);

    if (CDKResultSuccess == result)
    {
        VOID* pData                     = NULL;

        pData                           = ChiNodeUtils::GetMetaData(
                                          0, (vendorTagBase.vendorTagBase | UsecaseMetadataSectionMask),
                                          ChiMetadataDynamic, &g_ChiNodeInterface, m_hChiSession);

        if (NULL != pData)
        {
            m_EISMarginRequest          = *static_cast<MarginRequest*>(pData);
            CHICDK_LOG_INFO("EisV2 Margin = %f", m_EISMarginRequest.heightMargin);
        }
        else
        {
            result                      = CDKResultEFailed;
            CHICDK_LOG_ERROR("Failure in Retreiving EISV2 margin");
        }
    }
    else
    {
        CHICDK_LOG_ERROR("Failure in Retreiving EISV2 margin");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::GetEisV3Margin
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::GetEisV3Margin()
{
    CHIVENDORTAGBASEINFO vendorTagBase  = { 0 };

    CDKResult result                    = CDKResultSuccess;
    result                              = ChiNodeUtils::GetVendorTagBase("org.quic.camera.eislookahead",
                                                                         "RequestedMargin",
                                                                         &g_ChiNodeInterface,
                                                                         &vendorTagBase);

    if (CDKResultSuccess == result)
    {
        VOID* pData                     = NULL;

        pData                           = ChiNodeUtils::GetMetaData(
                                          0, (vendorTagBase.vendorTagBase | UsecaseMetadataSectionMask),
                                          ChiMetadataDynamic, &g_ChiNodeInterface, m_hChiSession);

        if (NULL != pData)
        {
            m_EISMarginRequest          = *static_cast<MarginRequest*>(pData);
            CHICDK_LOG_INFO("EisV3 Margin = %f", m_EISMarginRequest.heightMargin);
        }
        else
        {
            result                      = CDKResultEFailed;
            CHICDK_LOG_ERROR("Failure in Retreiving EISV3 margin");
        }
    }
    else
    {
        CHICDK_LOG_ERROR("Failure in Retreiving EISV3 margin");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::SetStabilizedOutputDimsLookAhead
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::SetStabilizedOutputDimsLookAhead(
    UINT32 outputOptimalH,
    INT32 outputOptimalW)
{
    CHIVENDORTAGBASEINFO vendorTagBase  = { 0 };

    CDKResult result                    = CDKResultSuccess;
    result                              = ChiNodeUtils::GetVendorTagBase("org.quic.camera.eislookahead",
                                                                         "StabilizedOutputDims",
                                                                         &g_ChiNodeInterface,
                                                                         &vendorTagBase);

    if (CDKResultSuccess == result)
    {
        VOID* pData = NULL;
        CHIMETADATAINFO metadataInfo                = { 0 };
        const UINT32    tagSize                     = 1;
        CHITAGDATA      tagData                     = { 0 };
        UINT32          tagList;
        CHIDimension stabilizedOutputDimensions     = { 0 };
        stabilizedOutputDimensions.height           = outputOptimalH;
        stabilizedOutputDimensions.width            = outputOptimalW;

        metadataInfo.size                           = sizeof(CHIMETADATAINFO);
        metadataInfo.chiSession                     = m_hChiSession;
        metadataInfo.metadataType                   = ChiMetadataDynamic;
        metadataInfo.tagNum                         = tagSize;
        metadataInfo.pTagList                       = &tagList;
        metadataInfo.pTagData                       = &tagData;

        tagList                                     = { vendorTagBase.vendorTagBase | UsecaseMetadataSectionMask };
        tagData.size                                = sizeof(CHITAGDATA);
        tagData.requestId                           = 0;
        tagData.pData                               = &stabilizedOutputDimensions;
        tagData.dataSize                            = sizeof(CHIDimension);

        result                                      = g_ChiNodeInterface.pSetMetadata(&metadataInfo);
        if (CDKResultSuccess == result)
        {
            CHICDK_LOG_INFO("WriteEIsV3 Dims [%d, %d]",
                                        stabilizedOutputDimensions.width,
                                        stabilizedOutputDimensions.height);
        }
        else
        {
            CHICDK_LOG_ERROR("Failure WriteEIsV3 Dims");
        }
    }
    else
    {
        CHICDK_LOG_ERROR("Failure WriteEIsV3 Dims");
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ChiDewarpNode::SetStabilizedOutputDimsRealTime
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDKResult ChiDewarpNode::SetStabilizedOutputDimsRealTime(
    UINT32  outputOptimalH,
    INT32   outputOptimalW)
{
    CHIVENDORTAGBASEINFO vendorTagBase  = { 0 };

    CDKResult result                    = CDKResultSuccess;
    result                              = ChiNodeUtils::GetVendorTagBase("org.quic.camera.eisrealtime",
                                                         "StabilizedOutputDims",
                                                         &g_ChiNodeInterface,
                                                         &vendorTagBase);

    if (CDKResultSuccess == result)
    {
        VOID* pData                                 = NULL;
        CHIMETADATAINFO metadataInfo                = { 0 };
        const UINT32    tagSize                     = 1;
        CHITAGDATA      tagData                     = { 0 };
        UINT32          tagList;
        CHIDimension stabilizedOutputDimensions     = { 0 };
        stabilizedOutputDimensions.height           = outputOptimalH;
        stabilizedOutputDimensions.width            = outputOptimalW;

        metadataInfo.size                           = sizeof(CHIMETADATAINFO);
        metadataInfo.chiSession                     = m_hChiSession;
        metadataInfo.metadataType                   = ChiMetadataDynamic;
        metadataInfo.tagNum                         = tagSize;
        metadataInfo.pTagList                       = &tagList;
        metadataInfo.pTagData                       = &tagData;

        tagList                                     = { vendorTagBase.vendorTagBase | UsecaseMetadataSectionMask };
        tagData.size                                = sizeof(CHITAGDATA);
        tagData.requestId                           = 0;
        tagData.pData                               = &stabilizedOutputDimensions;
        tagData.dataSize                            = sizeof(CHIDimension);

        result                                      = g_ChiNodeInterface.pSetMetadata(&metadataInfo);
        if (CDKResultSuccess == result)
        {
            CHICDK_LOG_INFO("WriteEIsV2 Dims [%d, %d]",
                                        stabilizedOutputDimensions.width,
                                        stabilizedOutputDimensions.height);
        }
        else
        {
            CHICDK_LOG_ERROR("Failure WriteEIsV2 Dims");
        }
    }
    else
    {
        CHICDK_LOG_ERROR("Failure WriteEIsV2 Dims");
    }

    return result;
}


// CAMX_NAMESPACE_END

/**
 * @file qcarcam.c
 *
 * @brief these are wrappers of AIS API which is used for back-compatibility
 *
 * Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include "ais.h"

#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

static QCarCamRet_e camera_to_qcarcam_result(CameraResult result)
{
    switch (result)
    {
    case CAMERA_SUCCESS             : return QCARCAM_RET_OK         ;
    case CAMERA_EFAILED             : return QCARCAM_RET_FAILED     ;
    case CAMERA_ENOMEMORY           : return QCARCAM_RET_NOMEM      ;
    case CAMERA_ECLASSNOTSUPPORT    : return QCARCAM_RET_FAILED     ;
    case CAMERA_EVERSIONNOTSUPPORT  : return QCARCAM_RET_UNSUPPORTED;
    case CAMERA_EALREADYLOADED      : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EUNABLETOLOAD       : return QCARCAM_RET_FAILED     ;
    case CAMERA_EUNABLETOUNLOAD     : return QCARCAM_RET_FAILED     ;
    case CAMERA_EALARMPENDING       : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EINVALIDTIME        : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EBADCLASS           : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EBADMETRIC          : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EEXPIRED            : return QCARCAM_RET_TIMEOUT    ;
    case CAMERA_EBADSTATE           : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EBADPARM            : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_ESCHEMENOTSUPPORTED : return QCARCAM_RET_UNSUPPORTED;
    case CAMERA_EBADITEM            : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EINVALIDFORMAT      : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EINCOMPLETEITEM     : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_ENOPERSISTMEMORY    : return QCARCAM_RET_NOMEM      ;
    case CAMERA_EUNSUPPORTED        : return QCARCAM_RET_UNSUPPORTED;
    case CAMERA_EPRIVLEVEL          : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_ERESOURCENOTFOUND   : return QCARCAM_RET_FAILED     ;
    case CAMERA_EREENTERED          : return QCARCAM_RET_FAILED     ;
    case CAMERA_EBADTASK            : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EALLOCATED          : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EALREADY            : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EADSAUTHBAD         : return QCARCAM_RET_FAILED     ;
    case CAMERA_ENEEDSERVICEPROG    : return QCARCAM_RET_FAILED     ;
    case CAMERA_EMEMPTR             : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EHEAP               : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EIDLE               : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_EITEMBUSY           : return QCARCAM_RET_BUSY       ;
    case CAMERA_EBADSID             : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_ENOTYPE             : return QCARCAM_RET_FAILED     ;
    case CAMERA_ENEEDMORE           : return QCARCAM_RET_FAILED     ;
    case CAMERA_EADSCAPS            : return QCARCAM_RET_FAILED     ;
    case CAMERA_EBADSHUTDOWN        : return QCARCAM_RET_FAILED     ;
    case CAMERA_EBUFFERTOOSMALL     : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_ENOSUCH             : return QCARCAM_RET_FAILED     ;
    case CAMERA_EACKPENDING         : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_ENOTOWNER           : return QCARCAM_RET_FAILED     ;
    case CAMERA_EINVALIDITEM        : return QCARCAM_RET_BADSTATE   ;
    case CAMERA_ENOTALLOWED         : return QCARCAM_RET_FAILED     ;
    case CAMERA_EBADHANDLE          : return QCARCAM_RET_BADPARAM   ;
    case CAMERA_EOUTOFHANDLES       : return QCARCAM_RET_NOMEM      ;
    case CAMERA_EINTERRUPTED        : return QCARCAM_RET_FAILED     ;
    case CAMERA_ENOMORE             : return QCARCAM_RET_NOMEM      ;
    case CAMERA_ECPUEXCEPTION       : return QCARCAM_RET_FAILED     ;
    case CAMERA_EREADONLY           : return QCARCAM_RET_FAILED     ;
    case CAMERA_EWOULDBLOCK         : return QCARCAM_RET_FAILED     ;
    case CAMERA_EOUTOFBOUND         : return QCARCAM_RET_FAILED     ;
    default                         : return QCARCAM_RET_FAILED     ;
    }
}


PUBLIC_API QCarCamRet_e QCarCamInitialize(const QCarCamInit_t* pInitParams)
{
    return camera_to_qcarcam_result(ais_initialize(pInitParams));
}

PUBLIC_API QCarCamRet_e QCarCamUninitialize(void)
{
    return camera_to_qcarcam_result(ais_uninitialize());
}

PUBLIC_API QCarCamRet_e QCarCamQueryInputs(QCarCamInput_t* pInputs, uint32_t size, uint32_t* pRetSize)
{
    return camera_to_qcarcam_result(ais_query_inputs(pInputs, size, pRetSize));
}

PUBLIC_API QCarCamRet_e QCarCamQueryInputModes(const uint32_t inputId, QCarCamInputModes_t* pInputModes)
{
    return camera_to_qcarcam_result(AisQueryInputModes(inputId, pInputModes));
}

PUBLIC_API QCarCamRet_e QCarCamQueryDiagnostics(QCarCamDiagInfo *pDiagInfo)
{
    return camera_to_qcarcam_result(ais_query_diagnostics(pDiagInfo));
}

PUBLIC_API QCarCamRet_e QCarCamOpen(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl)
{
    return camera_to_qcarcam_result(ais_open(pOpenParams, pHndl));
}

PUBLIC_API QCarCamRet_e QCarCamClose(QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(ais_close(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamGetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, void* pValue, uint32_t size)
{
    return camera_to_qcarcam_result(ais_g_param(hndl, param, pValue, size));
}

PUBLIC_API QCarCamRet_e QCarCamSetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, const void* pValue, uint32_t size)
{
    return camera_to_qcarcam_result(ais_s_param(hndl, param, pValue, size));
}

PUBLIC_API QCarCamRet_e QCarCamSetBuffers(QCarCamHndl_t hndl, const QCarCamBufferList_t* pBuffers)
{
    return camera_to_qcarcam_result(ais_s_buffers(hndl, pBuffers));
}

PUBLIC_API QCarCamRet_e QCarCamStart(QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(ais_start(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamStop(QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(ais_stop(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamPause(QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(ais_pause(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamResume(QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(ais_resume(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamGetFrame(QCarCamHndl_t hndl, QCarCamFrameInfo_t* pFrameInfo,
        uint64_t timeout, uint32_t flags)
{
    return camera_to_qcarcam_result(ais_get_frame(hndl, pFrameInfo, timeout, flags));
}

PUBLIC_API QCarCamRet_e QCarCamReleaseFrame(QCarCamHndl_t hndl, uint32_t id, uint32_t bufferIdx)
{
    return camera_to_qcarcam_result(ais_release_frame_v2(hndl, id, bufferIdx));
}

PUBLIC_API QCarCamRet_e QCarCamRegisterEventCallback(
    const QCarCamHndl_t hndl,
    const QCarCamEventCallback_t callbackFunc,
    void  *pPrivateData)
{
    return camera_to_qcarcam_result(AisRegisterEventCallback(hndl, callbackFunc, pPrivateData));
}

PUBLIC_API QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl)
{
    return camera_to_qcarcam_result(AisUnRegisterEventCallback(hndl));
}

PUBLIC_API QCarCamRet_e QCarCamReserve(const QCarCamHndl_t hndl)
{
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamRelease(const QCarCamHndl_t hndl)
{
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest)
{
    return camera_to_qcarcam_result(AisSubmitRequest(hndl, pRequest));
}

/* ===========================================================================
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include "ais_aidl_client.h"
#include <map>

#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

// Application specific global variables
std::shared_ptr<aidl::vendor::qti::automotive::qcarcam2::IQcarCamera> pService;
QCarCamInput_t *pInputLists;
uint32_t queryNumInputs;
static std::map<unsigned long, std::shared_ptr<ais_aidl_client>> gMap;

using namespace android;

#define QCARCAM_MAX_STREAMS 8

PUBLIC_API QCarCamRet_e QCarCamInitialize(const QCarCamInit_t* p_init_params)
{
    (void)p_init_params;
    char *processName = (char *)"qcarcam_aidl_test_qcc6";
    ais_log_init(NULL, processName);
    AIS_AIDL_DBGMSG("Acquiring ais-aidl Enumerator");
    const std::string serviceName = std::string() + IQcarCamera::descriptor + "/default";
    pService = IQcarCamera::fromBinder(ndk::SpAIBinder(AServiceManager_checkService(serviceName.c_str())));
    if (!pService) {
        AIS_AIDL_ERRORMSG("getService returned NULL");
        return QCARCAM_RET_FAILED;
    }
    bool success = ABinderProcess_setThreadPoolMaxThreadCount(QCARCAM_MAX_STREAMS);
    if (success == false)
    {
        AIS_AIDL_ERRORMSG("ABinderProcess_setThreadPoolMaxThreadCount returned false");
    }
    ABinderProcess_startThreadPool();
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamUninitialize(void)
{
    pService = nullptr;
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamQueryInputs(QCarCamInput_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    uint32_t i = 0;
    (void)size;
    std::vector<::aidl::vendor::qti::automotive::qcarcam2::QCarCamInput> InputLists;
    if (NULL == pInputLists) {
        auto result = pService->getInputStreamList(&InputLists, &Error);
        if (!result.isOk()) {
            AIS_AIDL_ERRORMSG("Aidl transport error; getInputStreamList() failed");
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
        else {
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                AIS_AIDL_DBGMSG("Found %d input streams",InputLists.size());
                queryNumInputs = InputLists.size();
                pInputLists = (QCarCamInput_t *)calloc(queryNumInputs, sizeof(*pInputLists));
                if (pInputLists) {
                    for (auto&& cam: InputLists) {
                        AIS_AIDL_DBGMSG("Found camera %d", static_cast<uint32_t>(cam.inputId));
                        // Copy the result into qcarcam structure from aidl cb
                        pInputLists[i].inputId = cam.inputId;
                        pInputLists[i].devId = cam.devId;
                        pInputLists[i].subdevId = cam.subdevId;
                        pInputLists[i].numModes = cam.numModes;
                        pInputLists[i].flags = cam.flags;
                        strlcpy(pInputLists[i].inputName, InputLists[i].inputName.c_str(), sizeof(pInputLists[i].inputName));
                        i++;
                    }
                }
            } else {
                AIS_AIDL_ERRORMSG("getInputStreamList() Failed with Error %d",static_cast<int32_t>(Error));
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        }
    }
    *ret_size = queryNumInputs;

    if (NULL != p_inputs && NULL != pInputLists)
        memcpy(p_inputs, pInputLists, queryNumInputs*sizeof(QCarCamInput_t));

    return static_cast<QCarCamRet_e>(Error);
}

PUBLIC_API QCarCamRet_e QCarCamQueryInputModes(const uint32_t inputId, QCarCamInputModes_t* pInputModes)
{
    QCarCamError Error;
    QCarCamInputModes InputModes;
    auto result = pService->getInputStreamMode(inputId, &InputModes, &Error);
    if (QCarCamError::QCARCAM_RET_OK == Error) {
        // Copy the result into qcarcam structure from aidl cb
        pInputModes->currentMode = InputModes.currentMode;
        pInputModes->numModes = InputModes.numModes;
        for (uint32_t i=0; i<pInputModes->numModes; i++) {
            pInputModes->pModes[i].numSources = InputModes.Modes[i].numSources;
            for (uint32_t j=0; j<InputModes.Modes[i].numSources; j++) {
                pInputModes->pModes[i].sources[j].srcId = InputModes.Modes[i].sources[j].srcId;
                pInputModes->pModes[i].sources[j].width = InputModes.Modes[i].sources[j].width;
                pInputModes->pModes[i].sources[j].height = InputModes.Modes[i].sources[j].height;
                pInputModes->pModes[i].sources[j].colorFmt = static_cast<QCarCamColorFmt_e>(InputModes.Modes[i].sources[j].colorFmt);
                pInputModes->pModes[i].sources[j].fps = InputModes.Modes[i].sources[j].fps;
                pInputModes->pModes[i].sources[j].securityDomain = InputModes.Modes[i].sources[j].securityDomain;
            }
        }
    } else {
        AIS_AIDL_ERRORMSG("getInputStreamMode() Failed with Error %d",static_cast<int32_t>(Error));
    }
    if (!result.isOk()) {
        AIS_AIDL_ERRORMSG("Aidl transport error; getInputStreamMode() failed");
        Error = QCarCamError::QCARCAM_RET_FAILED;
    }
    return static_cast<QCarCamRet_e>(Error);
}

PUBLIC_API QCarCamRet_e QCarCamQueryDiagnostics(QCarCamDiagInfo* pDiag)
{
    (void)pDiag;
    return QCARCAM_RET_UNSUPPORTED;
}

PUBLIC_API QCarCamRet_e QCarCamOpen(const QCarCamOpen_t* pOpenParams, QCarCamHndl_t* pHndl)
{
    // Create and return stream obj
    QCarCamRet_e ret = QCARCAM_RET_OK;
    if(pHndl == NULL)
    {
        AIS_AIDL_ERRORMSG("pHndl is null");
    }
    std::shared_ptr<ais_aidl_client> clientObj = ndk::SharedRefBase::make<ais_aidl_client>(pService);
    ret = clientObj->ais_client_open_stream(pOpenParams);
    if (QCARCAM_RET_OK != ret) {
        AIS_AIDL_ERRORMSG("ais_client_open_stream failed with ret = %d", ret);
        clientObj.reset();
    } else {
        AIS_AIDL_ERRORMSG("ais_client_open_stream success with ret = %d", ret);
        clientObj->mInitialized = true;
    }
    *pHndl = (QCarCamHndl_t)clientObj.get();
    gMap[*pHndl] = clientObj;
    AIS_AIDL_DBGMSG("ais_client_open_stream after hndl = %p, clientObj.get=%p", *pHndl, clientObj.get());
    AIS_AIDL_DBGMSG("X");
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamClose(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        ret = gMap[hndl]->ais_client_close_stream();
        auto client = gMap[hndl];
        gMap.erase(hndl);
        client.reset();
    } else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamGetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, void* pValue, uint32_t size)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_g_param(param, pValue, size);
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamRegisterEventCallback(const QCarCamHndl_t hndl, const QCarCamEventCallback_t callbackFunc, void  *pPrivateData)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        ret = gMap[hndl]->ais_client_set_cb(callbackFunc, pPrivateData);
    }
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamSetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, const void* pValue, uint32_t size)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        ret = gMap[hndl]->ais_client_s_param(param, pValue, size);
    }
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamSetBuffers(QCarCamHndl_t hndl, const QCarCamBufferList_t* pBuffers)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_s_buffer(pBuffers);
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamStart(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_start_stream(hndl);
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamStop(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_stop_stream();
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamPause(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_pause_stream();
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamResume(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_resume_stream();
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamGetFrame(QCarCamHndl_t hndl, QCarCamFrameInfo_t* pFrameInfo,
        uint64_t timeout, uint32_t flags)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_get_frame(pFrameInfo, timeout, flags);
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamReleaseFrame(QCarCamHndl_t hndl, uint32_t id, uint32_t bufferIdx)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized)
        ret = gMap[hndl]->ais_client_release_frame(id, bufferIdx);
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;

    if (gMap[hndl] && gMap[hndl]->mInitialized) {
        ret = gMap[hndl]->ais_client_unset_cb();
    }
    else
        ret = QCARCAM_RET_BADPARAM;

    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamSubmitRequest(const QCarCamHndl_t hndl, const QCarCamRequest_t* pRequest)
{
    (void)hndl;
    (void)pRequest;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    return ret;
}

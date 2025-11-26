/* ===========================================================================
 * Copyright (c) 2019-2020, 2022, 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include "qcx_hidl_client.h"
#include <hidl/LegacySupport.h>

#if (defined (__GNUC__) && !defined(__INTEGRITY))
#define PUBLIC_API __attribute__ ((visibility ("default")))
#else
#define PUBLIC_API
#endif

// Application specific global variables
android::sp<IQcarCamera> pService;
QCarCamInput_t *pInputLists;
uint32_t queryNumInputs;

using namespace android;
using android::hardware::configureRpcThreadpool;

#define QCARCAM_MAX_STREAMS 8

class DeathRecipient : public android::hardware::hidl_death_recipient {
    qcx_hidl_client *clientObj = nullptr;
    virtual void serviceDied(uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who) {
        UNUSED_PARAM(cookie);
        UNUSED_PARAM(who);
        QCX_HIDL_ERRORMSG("Service died callback");
        if (clientObj != nullptr)
            clientObj->qcx_client_abort_due_to_death();
        else
            QCX_HIDL_ERRORMSG("Cannot do error Handling because no valid ClientObject");
    }
public:
    void clientObjInitialization(qcx_hidl_client *clientObject) {
        clientObj = clientObject;
    }
};

sp<DeathRecipient> death_recp = nullptr;

PUBLIC_API QCarCamRet_e QCarCamInitialize(const QCarCamInit_t* p_init_params)
{
    (void)p_init_params;
    QcxLogInit();
    QCX_HIDL_DBGMSG("Acquiring ais-hidl Enumerator");
    pService = IQcarCamera::getService();
    if (pService.get() == nullptr) {
        QCX_HIDL_ERRORMSG("getService returned NULL");
        return QCARCAM_RET_FAILED;
    }
    death_recp = new DeathRecipient();
    pService->linkToDeath(death_recp,0);
    configureRpcThreadpool(QCARCAM_MAX_STREAMS, false /* callerWillJoin */);
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamUninitialize(void)
{
    if (pService)
        pService->unlinkToDeath(death_recp);
    death_recp = nullptr;
    pService = nullptr;
    return QCARCAM_RET_OK;
}

PUBLIC_API QCarCamRet_e QCarCamQueryInputs(QCarCamInput_t* p_inputs, unsigned int size, unsigned int* ret_size)
{
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    uint32_t i = 0;
    (void)size;

    if (NULL == pInputLists) {
        auto result = pService->getInputStreamList([&](const auto& tmpError, hidl_vec<QCarCamInput> InputLists) {
                Error = tmpError;
                if (QCarCamError::QCARCAM_RET_OK == Error) {
                    QCX_HIDL_DBGMSG("Found %d input streams",InputLists.size());
                    queryNumInputs = InputLists.size();
                    pInputLists = (QCarCamInput_t *)calloc(queryNumInputs, sizeof(*pInputLists));
                    if (pInputLists) {
                        for (auto&& cam: InputLists) {
                            QCX_HIDL_DBGMSG("Found camera %d", static_cast<uint32_t>(cam.inputId));
                            // Copy the result into qcarcam structure from hidl cb
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
                    QCX_HIDL_ERRORMSG("getInputStreamList() Failed with Error %d",static_cast<int32_t>(Error));
                }
        } );
        if (!result.isOk()) {
            QCX_HIDL_ERRORMSG("Hidl transport error; getInputStreamList() failed");
            Error = QCarCamError::QCARCAM_RET_FAILED;
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
    auto result = pService->getInputStreamMode(inputId, [&](const auto& tmpError, const auto& InputModes) {
            Error = tmpError;
            if (QCarCamError::QCARCAM_RET_OK == Error) {
                // Copy the result into qcarcam structure from hidl cb
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
                    QCX_HIDL_ERRORMSG("getInputStreamMode() Failed with Error %d",static_cast<int32_t>(Error));
                }
            } );
    if (!result.isOk()) {
        QCX_HIDL_ERRORMSG("Hidl transport error; getInputStreamMode() failed");
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
    qcx_hidl_client *clientObj = new qcx_hidl_client(pService);
    if (clientObj == nullptr)
        return QCARCAM_RET_BADPARAM;
    ret = clientObj->qcx_client_open_stream(pOpenParams);
    if (QCARCAM_RET_OK != ret) {
        QCX_HIDL_ERRORMSG("qcx_client_open_stream failed with ret = %d", ret);
        //delete(clientObj);
        //clientObj = NULL;
    } else {
        clientObj->mInitialized = true;
    }
    *pHndl = reinterpret_cast<QCarCamHndl_t>(clientObj);
    death_recp->clientObjInitialization(clientObj);
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamClose(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        ret = clientObj->qcx_client_close_stream();
        // Delete stream obj
        clientObj->mInitialized = false;
        // TODO::Remove this and solve crash
        //delete(clientObj);
        //clientObj = nullptr;
    } else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamGetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, void* pValue, uint32_t size)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_g_param(param, pValue, size);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamRegisterEventCallback(const QCarCamHndl_t hndl, const QCarCamEventCallback_t callbackFunc, void  *pPrivateData)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        ret = clientObj->qcx_client_set_cb(callbackFunc, pPrivateData);
    }
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamSetParam(QCarCamHndl_t hndl, QCarCamParamType_e param, const void* pValue, uint32_t size)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        ret = clientObj->qcx_client_s_param(param, pValue, size);
    }
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamSetBuffers(QCarCamHndl_t hndl, const QCarCamBufferList_t* pBuffers)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_s_buffer(pBuffers);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamStart(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_start_stream(hndl);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamStop(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_stop_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamPause(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_pause_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamResume(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_resume_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamReserve(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_reserve_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamRelease(QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_release_stream();
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamGetFrame(QCarCamHndl_t hndl, QCarCamFrameInfo_t* pFrameInfo,
        uint64_t timeout, uint32_t flags)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_get_frame(pFrameInfo, timeout, flags);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamReleaseFrame(QCarCamHndl_t hndl, uint32_t id, uint32_t bufferIdx)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized)
        ret = clientObj->qcx_client_release_frame(id, bufferIdx);
    else
        ret = QCARCAM_RET_BADPARAM;
    return ret;
}

PUBLIC_API QCarCamRet_e QCarCamUnregisterEventCallback(const QCarCamHndl_t hndl)
{
    QCarCamRet_e ret = QCARCAM_RET_OK;
    qcx_hidl_client *clientObj;
    clientObj = reinterpret_cast<qcx_hidl_client*>(hndl);
    if (clientObj && clientObj->mInitialized) {
        ret = clientObj->qcx_client_unset_cb();
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

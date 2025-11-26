/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <utils/Log.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef HAL_CAMERA_CPMS_SUPPORT
#include <cutils/properties.h>
#endif
#include "ais_aidl_camera.h"
#include "ais_aidl_stream.h"

#ifdef HAL_CAMERA_CPMS_SUPPORT
#define HAL_SLEEP_USEC 1000000
#define HAL_DEFAULT_SLEEP_USEC 2
#endif

namespace aidl {
namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam2 {

// Global stream array for storing stream data
gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
std::mutex gMultiStreamQLock;
std::condition_variable gClientNotifier;

std::vector<std::shared_ptr<IQcarCameraStream>> sCameraStreams;

#ifdef HAL_CAMERA_CPMS_SUPPORT
pm_event_t gPMStatus;
std::shared_ptr<PowerPolicyService> gPowerService;

void PowerEventThread (void *args) {
    gPowerService = PowerEventInit(ais_aidl_camera::PowerEventCb, args);
}

int ais_aidl_camera::PowerEventCb(pm_event_t eventId, void* pUsrCtxt) {
    AIS_AIDL_INFOMSG("Entry PowerEventCb");

    std::vector<std::shared_ptr<IQcarCameraStream>>::iterator itr;
    ais_aidl_camera *pCamera = (ais_aidl_camera *)pUsrCtxt;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.qcom.ais.power.sleep", value, "0");

    int sleepTime = 0;
    sleepTime = atoi(value);

    int sleepTimeUs = ((sleepTime > 0 && sleepTime <= 3) ?
        sleepTime : HAL_DEFAULT_SLEEP_USEC) * HAL_SLEEP_USEC;
    AIS_AIDL_INFOMSG("sleep Value : %d", sleepTimeUs);

    switch (eventId)
    {
        case AIS_PM_SUSPEND:
        {
            AIS_AIDL_ERRORMSG("PowerEventCb AIS_PM_SUSPEND ");
            if (gPMStatus == AIS_PM_SUSPEND) {
                AIS_AIDL_ERRORMSG("Status is already in the Suspend");
                break;
            }

            AIS_AIDL_ERRORMSG("PowerEventCb Starting sleep for 2 Sec to allow App to close sessions");
            usleep(sleepTimeUs);
            AIS_AIDL_ERRORMSG("PowerEventCb Checking for any resource still alive and closing them");

            if (sCameraStreams.empty()) {
                AIS_AIDL_ERRORMSG("Apps Got closed");
            } else {
                itr = sCameraStreams.begin();
                while(itr != sCameraStreams.end()) {
                    if (*itr != nullptr) {
                        const std::shared_ptr<IQcarCameraStream> camStream;
                        QCarCamError _aidl_return;
                        pCamera->closeStream(camStream, &_aidl_return);
                    }
                    itr++;
                }
                sCameraStreams.clear();
           }

            gPMStatus = AIS_PM_SUSPEND;
            break;
        }
        case AIS_PM_RESUME:
        {
            AIS_AIDL_ERRORMSG("PowerEventCb AIS_PM_RESUME");
            if (gPMStatus == AIS_PM_RESUME) {
                AIS_AIDL_ERRORMSG("Status is already in the Resume");
                break;
            }

            gPMStatus = AIS_PM_RESUME;
            break;
        }
    }

    AIS_AIDL_INFOMSG("Exit PowerEventCb");
    return 0;
}
#endif

//Default constructors
ais_aidl_camera::ais_aidl_camera()  {
    AIS_AIDL_INFOMSG("Entry ais_aidl_camera constructor");
    mQcarcamClientCnt = 0;
    mMonitorThreadExit = false;

    // Create a thread for monitoring client count and call
    // qcarcam_uninitialize()
    mMonitorThread = std::thread([this](){ clientMonitorThread(); });
    mNumInputs = 0;
    mpQcarcamInput = NULL;
    mpQarcamModes = NULL;

#ifdef HAL_CAMERA_CPMS_SUPPORT
    gPMStatus = AIS_PM_RESUME;
    mPowerEventThread = std::thread(PowerEventThread, this);
#endif
}

//Default destructor
ais_aidl_camera::~ais_aidl_camera()  {
    AIS_AIDL_INFOMSG("Entry ais_aidl_camera destructor");

#ifdef HAL_CAMERA_CPMS_SUPPORT
    gPowerService.reset();
    if (mPowerEventThread.joinable())
        mPowerEventThread.join();
#endif

    mMonitorThreadExit=true;
    gClientNotifier.notify_all();
    // Block until the background thread exit
    if (mMonitorThread.joinable()) {
        mMonitorThread.join();
    }
    if (mpQarcamModes) {
        for (int i=0; i<mNumInputs; i++) {
            if (mpQarcamModes[i].pModes)
                free(mpQarcamModes[i].pModes);
        }
        free(mpQarcamModes);
    }
    if (mpQcarcamInput)
        free(mpQcarcamInput);
}

//Thread to monitor client count and call QCarCamUninitialize
void ais_aidl_camera::clientMonitorThread() {
    AIS_AIDL_INFOMSG("ais_aidl_camera::clientMonitorThread starting\n");

    while(!mMonitorThreadExit) {
        std::unique_lock<std::mutex> lk(gMultiStreamQLock);

        AIS_AIDL_INFOMSG("going to wait for event from stream obj");
        // Wait until stream obj notified
        gClientNotifier.wait(lk);

        std::unique_lock<std::mutex> client_lk(mClientCntLock);
        if (mQcarcamClientCnt == 0) {
            // All clients are closed. Perform QCarCamUninitialize
            QCarCamRet_e ret = QCARCAM_RET_OK;
            AIS_AIDL_INFOMSG("Calling QCarCamUninitialize()");
            ret = QCarCamUninitialize();
            if (ret != QCARCAM_RET_OK) {
                AIS_AIDL_ERRORMSG("QCarCamUninitialize() failed %d", ret);
            }
        }
        client_lk.unlock();
    }
    AIS_AIDL_INFOMSG("clientMonitorThread thread exiting");
}

::ndk::ScopedAStatus ais_aidl_camera::getInputStreamList(std::vector<QCarCamInput>* out_inputs, QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("Entry getInputStreamList");
    QCarCamRet_e ret = QCARCAM_RET_OK;
    //aidl_vec<QCarCamInput> out_inputs;
    uint32_t queryFilled = 0;
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    unsigned i = 0;
    bool qcarcamInitialized = false;

    if (NULL == mpQcarcamInput) {
        std::unique_lock<std::mutex> client_lk(mClientCntLock);
        if (0 == mQcarcamClientCnt) {
            // try initialize once
            QCarCamInit_t QCarCamInit = {};
            QCarCamInit.apiVersion = QCARCAM_VERSION;
            ret = QCarCamInitialize(&QCarCamInit);
            if (ret != QCARCAM_RET_OK) {
                AIS_AIDL_ERRORMSG("QCarCamInitialize failed %d", ret);
                // Return empty list
                Error = static_cast<QCarCamError>(ret);
                //_aidl_cb(Error, out_inputs);
                client_lk.unlock();
                goto exit;
            } else
                qcarcamInitialized = true;
        }
        client_lk.unlock();

        // Call qcarcam API
        ret = QCarCamQueryInputs(NULL, 0, &mNumInputs);
        if (QCARCAM_RET_OK == ret && 0 != mNumInputs) {
            mpQcarcamInput = (QCarCamInput_t *)calloc(mNumInputs, sizeof(QCarCamInput_t));
            if (NULL != mpQcarcamInput)
                ret = QCarCamQueryInputs(mpQcarcamInput, mNumInputs, &queryFilled);
            else {
                // Return empty list
                Error = static_cast<QCarCamError>(ret);
                //_aidl_cb(Error, out_inputs);
                goto exit;
            }
        }
        if (QCARCAM_RET_OK != ret) {
            AIS_AIDL_ERRORMSG("Error qcarcam_query_inputs failed");
            // free mpQcarcamInput
            free(mpQcarcamInput);
            mpQcarcamInput = NULL;
            Error = static_cast<QCarCamError>(ret);
            //_aidl_cb(Error, out_inputs);
            goto exit;
        }
    }

    if (NULL == mpQarcamModes) {
        // store input mode details
        mpQarcamModes = (QCarCamInputModes_t *)calloc(mNumInputs, sizeof(QCarCamInputModes_t));
    }

    // Build up a packed array of QcarcamInputInfo for return
    out_inputs->resize(mNumInputs);
    sCameraStreams.resize(mNumInputs);

    for (i=0; i<mNumInputs; i++) {
        (*out_inputs)[i].inputId = mpQcarcamInput[i].inputId;
        (*out_inputs)[i].devId = mpQcarcamInput[i].devId;
        (*out_inputs)[i].subdevId = mpQcarcamInput[i].subdevId;
        (*out_inputs)[i].numModes = mpQcarcamInput[i].numModes;
        (*out_inputs)[i].flags = mpQcarcamInput[i].flags;
        (*out_inputs)[i].inputName = mpQcarcamInput[i].inputName;

        if (NULL == mpQarcamModes[i].pModes) {
            QCarCamMode_t *pModes = (QCarCamMode_t *)calloc(mpQcarcamInput[i].numModes, sizeof(QCarCamMode_t));
            mpQarcamModes[i].pModes = pModes;
            mpQarcamModes[i].numModes = mpQcarcamInput[i].numModes;

            // Call qcarcam API
            ret = QCarCamQueryInputModes(mpQcarcamInput[i].inputId, &mpQarcamModes[i]);
            if (QCARCAM_RET_OK != ret) {
                AIS_AIDL_ERRORMSG("Error QCarCamQueryInputModes() failed ret %d id = ", ret, i);
                // free pModes
                free(pModes);
                free(mpQarcamModes[i].pModes);
                mpQarcamModes[i].pModes = NULL;
            }
        }
    }

    // Send back the results
    AIS_AIDL_INFOMSG("reporting %zu cameras available", out_inputs->size());
    Error = static_cast<QCarCamError>(ret);
    //_aidl_cb(Error, out_inputs);
    //_aidl_return = &Error;
    //return ndk::ScopedAStatus::ok();

exit:
    if (qcarcamInitialized)
    {
        // Call qcarcam API
        AIS_AIDL_INFOMSG("QCarCamUninitialize getting called from getInputStreamList %d", ret);
        QCarCamRet_e ret = QCARCAM_RET_OK;
        ret = QCarCamUninitialize();
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("QCarCamUninitialize() failed %d", ret);
        }
        qcarcamInitialized = false;
    }
    // AIDL convention says we return Void if we sent our result back via callback
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_camera::getInputStreamMode(int32_t in_inputId, QCarCamInputModes* out_modes, QCarCamError* _aidl_return)  {
    AIS_AIDL_INFOMSG("Entry getInputStreamMode");
    //QCarCamInputModes out_modes;
    QCarCamInputModes_t *pQcarcamModes = NULL;
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    unsigned i = 0, j = 0;

    if (NULL == mpQcarcamInput || NULL == mpQarcamModes) {
        Error = QCarCamError::QCARCAM_RET_FAILED;
        //_aidl_cb(Error, out_modes);
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }

    for (i=0; i<mNumInputs; i++)
        if (mpQcarcamInput[i].inputId == in_inputId)
            pQcarcamModes = &mpQarcamModes[i];

    if (NULL == pQcarcamModes || NULL == pQcarcamModes->pModes) {
        Error = QCarCamError::QCARCAM_RET_FAILED;
        //_aidl_cb(Error, out_modes);
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    }

    // Build up a packed array of QcarcamInputInfo for return
    out_modes->currentMode = pQcarcamModes->currentMode;
    out_modes->numModes = pQcarcamModes->numModes;

    for (i=0; i<pQcarcamModes->numModes; i++) {
        out_modes->Modes[i].numSources = pQcarcamModes->pModes[i].numSources;
        for (j=0; j<pQcarcamModes->pModes[i].numSources; j++) {
            out_modes->Modes[i].sources[j].srcId = pQcarcamModes->pModes[i].sources[j].srcId;
            out_modes->Modes[i].sources[j].width = pQcarcamModes->pModes[i].sources[j].width;
            out_modes->Modes[i].sources[j].height = pQcarcamModes->pModes[i].sources[j].height;
            out_modes->Modes[i].sources[j].colorFmt = static_cast<QCarCamColorFmt>(pQcarcamModes->pModes[i].sources[j].colorFmt);
            out_modes->Modes[i].sources[j].fps = pQcarcamModes->pModes[i].sources[j].fps;
            out_modes->Modes[i].sources[j].securityDomain = pQcarcamModes->pModes[i].sources[j].securityDomain;
        }
    }

    // Send back the results
    //_aidl_cb(Error, out_modes);

    // AIDL convention says we return Void if we sent our result back via callback
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_camera::openStream(const QCarCamOpenParam& in_OpenParam, std::shared_ptr<IQcarCameraStream>* pActiveStream) {
    AIS_AIDL_INFOMSG("Entry openStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamHndl_t qcarcam_hndl;
    int32_t i = 0;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // initialize qcarcam for first client
        QCarCamRet_e ret = QCARCAM_RET_OK;
        QCarCamInit_t qCarCamInit = {};
        qCarCamInit.apiVersion = QCARCAM_VERSION;
        ret = QCarCamInitialize(&qCarCamInit);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("QCarCamInitialize failed %d", ret);
            // Return empty list
            Error = static_cast<QCarCamError>(ret);
            client_lk.unlock();
            return ndk::ScopedAStatus::ok();
        }
    }

    // Call qcarcam API
    QCarCamOpen_t q_openParams = {};
    q_openParams.numInputs = in_OpenParam.numInputs;
    for (int i=0; i<in_OpenParam.numInputs; i++) {
        q_openParams.inputs[i].inputId = in_OpenParam.inputs[i].inputId;
        q_openParams.inputs[i].srcId = in_OpenParam.inputs[i].srcId;
        q_openParams.inputs[i].inputMode = in_OpenParam.inputs[i].inputMode;
    }
    q_openParams.opMode = static_cast<QCarCamOpmode_e>(in_OpenParam.opMode);
    q_openParams.priority = in_OpenParam.priority;
    q_openParams.flags = in_OpenParam.flags;

    ret = QCarCamOpen(&q_openParams, &qcarcam_hndl);

    if (ret != QCARCAM_RET_OK || QCARCAM_HNDL_INVALID == qcarcam_hndl) {
        AIS_AIDL_ERRORMSG("QCarCamOpen failed");
        Error = static_cast<QCarCamError>(ret);
    } else {
        mQcarcamClientCnt++;
        // Store qcarcam hndl in Event Q for callback mapping
        gMultiStreamQLock.lock();
        // Find an empty slot and insert handle
        for (i=0; i<MAX_AIS_CLIENTS; i++) {
            if (NOT_IN_USE == gQcarCamClientArray[i].qcarcam_context)
                break;
        }
        if (i < MAX_AIS_CLIENTS) {
            gQcarCamClientArray[i].qcarcam_context = qcarcam_hndl;
            gMultiStreamQLock.unlock();

            // Construct a camera instance for the caller
            *pActiveStream = ndk::SharedRefBase::make<ais_aidl_stream>(in_OpenParam.inputs[i].inputId, &gQcarCamClientArray[i], qcarcam_hndl);
            if (pActiveStream == nullptr) {
                AIS_AIDL_ERRORMSG("Failed to allocate new ais_aidl_stream object for %d\n", static_cast<int>(in_OpenParam.inputs[i].inputId));
                Error = QCarCamError::QCARCAM_RET_FAILED;
            } else {
                if (std::find (sCameraStreams.begin(), sCameraStreams.end(), *pActiveStream) == sCameraStreams.end()) {
                    sCameraStreams.push_back(*pActiveStream);
                }
            }
        } else {
            gMultiStreamQLock.unlock();
            AIS_AIDL_ERRORMSG("Max number od streams reached!!");
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
    }
    client_lk.unlock();
    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d hndl %p", static_cast<int>(Error), qcarcam_hndl);
    //_aidl_cb(pActiveStream, Error);

    // AIDL convention says we return Void if we sent our result back via callback
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ais_aidl_camera::closeStream(const std::shared_ptr<IQcarCameraStream>& in_camStream, QCarCamError* _aidl_return) {
    AIS_AIDL_INFOMSG("Entry closeStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (mQcarcamClientCnt <= 0) {
        AIS_AIDL_ERRORMSG("Error qcarcam not initialized");
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _aidl_return = &Error;
        return ndk::ScopedAStatus::ok();
    } else {
        mQcarcamClientCnt--;
    }
    client_lk.unlock();
    //if (in_camStream == nullptr) {
    //    AIS_AIDL_ERRORMSG("Ignoring call to closeCamera with null camera ptr");
    //}

    // No need to explicitly detele pCamera since its a smart pointer, destructor
    // gets called at the end of this function and qcarcam_close() will be
    // called in destructor

    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    _aidl_return = &Error;
    return ndk::ScopedAStatus::ok();
}

} // qcarcam2
} // automotive
} // qti
} // vendor
} // aidl

/* ===========================================================================
 * Copyright (c) 2019, 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <utils/Log.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "ais_hidl_camera.h"
#include "ais_hidl_stream.h"

namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V2_0 {
namespace implementation {

// Global stream array for storing stream data
gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
std::mutex gMultiStreamQLock;
std::condition_variable gClientNotifier;

//Default constructors
ais_hidl_camera::ais_hidl_camera()  {
    AIS_HIDL_INFOMSG("Entry ais_hidl_camera constructor");
    mQcarcamClientCnt = 0;
    mMonitorThreadExit = false;

    // Create a thread for monitoring client count and call
    // QCarCamUninitialize()
    mMonitorThread = std::thread([this](){ clientMonitorThread(); });
    mNumInputs = 0;
    mpQcarcamInput = NULL;
    mpQarcamModes = NULL;
    char *processName = (char *)"qcarcam_hal";
    ais_log_init(NULL, processName);
}

//Default destructor
ais_hidl_camera::~ais_hidl_camera()  {
    AIS_HIDL_INFOMSG("Entry ais_hidl_camera destructor");

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
void ais_hidl_camera::clientMonitorThread()
{
    AIS_HIDL_INFOMSG("ais_hidl_camera::clientMonitorThread starting\n");

    while(!mMonitorThreadExit) {
        std::unique_lock<std::mutex> lk(gMultiStreamQLock);

        AIS_HIDL_INFOMSG("going to wait for event from stream obj");
        // Wait until stream obj notified
        gClientNotifier.wait(lk);

        std::unique_lock<std::mutex> client_lk(mClientCntLock);
        if (mQcarcamClientCnt == 0) {
            // All clients are closed. Perform QCarCamUninitialize
            QCarCamRet_e ret = QCARCAM_RET_OK;
            AIS_HIDL_INFOMSG("Calling QCarCamUninitialize()");
            ret = QCarCamUninitialize();
            if (ret != QCARCAM_RET_OK) {
                AIS_HIDL_ERRORMSG("QCarCamUninitialize() failed %d", ret);
            }
        }
        client_lk.unlock();
    }
    AIS_HIDL_INFOMSG("clientMonitorThread thread exiting");
}

// API definations
Return<void> ais_hidl_camera::getInputStreamList(getInputStreamList_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getInputStreamList");
    QCarCamRet_e ret = QCARCAM_RET_OK;
    hidl_vec<QCarCamInput> InputLists;
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
                AIS_HIDL_ERRORMSG("QCarCamInit_tialize failed %d", ret);
                // Return empty list
                Error = static_cast<QCarCamError>(ret);
                _hidl_cb(Error, InputLists);
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
                _hidl_cb(Error, InputLists);
                goto exit;
            }
        }
        if (QCARCAM_RET_OK != ret) {
            AIS_HIDL_ERRORMSG("Error qcarcam_query_inputs failed");
            // free mpQcarcamInput
            free(mpQcarcamInput);
            mpQcarcamInput = NULL;
            Error = static_cast<QCarCamError>(ret);
            _hidl_cb(Error, InputLists);
            goto exit;
        }
    }

    if (NULL == mpQarcamModes) {
        // store input mode details
        mpQarcamModes = (QCarCamInputModes_t *)calloc(mNumInputs, sizeof(QCarCamInputModes_t));
        if (mpQarcamModes == NULL)
        {
            AIS_HIDL_ERRORMSG("Failed to allocate memory : mpQarcamModes");
            // free mpQcarcamInput
            free(mpQcarcamInput);
            mpQcarcamInput = NULL;
            Error = static_cast<QCarCamError>(ret);
            _hidl_cb(Error, InputLists);
            goto exit;
        }
    }

    // Build up a packed array of QcarcamInputInfo for return
    InputLists.resize(mNumInputs);

    for (i=0; i<mNumInputs; i++) {
        InputLists[i].inputId = mpQcarcamInput[i].inputId;
        InputLists[i].devId = mpQcarcamInput[i].devId;
        InputLists[i].subdevId = mpQcarcamInput[i].subdevId;
        InputLists[i].numModes = mpQcarcamInput[i].numModes;
        InputLists[i].flags = mpQcarcamInput[i].flags;
        InputLists[i].inputName = mpQcarcamInput[i].inputName;

        if (NULL == mpQarcamModes[i].pModes) {
            QCarCamMode_t *pModes = (QCarCamMode_t *)calloc(mpQcarcamInput[i].numModes, sizeof(QCarCamMode_t));
            mpQarcamModes[i].pModes = pModes;
            mpQarcamModes[i].numModes = mpQcarcamInput[i].numModes;

            // Call qcarcam API
            ret = QCarCamQueryInputModes(mpQcarcamInput[i].inputId, &mpQarcamModes[i]);
            if (QCARCAM_RET_OK != ret) {
                AIS_HIDL_ERRORMSG("Error QCarCamQueryInputModes() failed ret %d id = ", ret, i);
                // free pModes
                free(pModes);
                free(mpQarcamModes[i].pModes);
                mpQarcamModes[i].pModes = NULL;
            }
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("reporting %zu cameras available", InputLists.size());
    Error = static_cast<QCarCamError>(ret);
    _hidl_cb(Error, InputLists);

exit:
    if (qcarcamInitialized)
    {
        // Call qcarcam API
        AIS_HIDL_INFOMSG("QCarCamUninitialize getting called from getInputStreamList %d", ret);
        QCarCamRet_e ret = QCARCAM_RET_OK;
        ret = QCarCamUninitialize();
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("QCarCamUninitialize() failed %d", ret);
        }
        qcarcamInitialized = false;
    }
    // HIDL convention says we return Void if we sent our result back via callback
    return Void();

}

Return<void> ais_hidl_camera::getInputStreamMode(uint32_t inputId, getInputStreamMode_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getInputStreamList");
    QCarCamInputModes InputModes;
    QCarCamInputModes_t *pQcarcamModes = NULL;
    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    unsigned i = 0, j = 0;

    if (NULL == mpQcarcamInput || NULL == mpQarcamModes) {
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _hidl_cb(Error, InputModes);
        return Void();
    }

    for (i=0; i<mNumInputs; i++)
        if (mpQcarcamInput[i].inputId == inputId)
            pQcarcamModes = &mpQarcamModes[i];

    if (NULL == pQcarcamModes || NULL == pQcarcamModes->pModes) {
        Error = QCarCamError::QCARCAM_RET_FAILED;
        _hidl_cb(Error, InputModes);
        return Void();
    }

    // Build up a packed array of QcarcamInputInfo for return
    InputModes.currentMode = pQcarcamModes->currentMode;
    InputModes.numModes = pQcarcamModes->numModes;

    for (i=0; i<pQcarcamModes->numModes; i++) {
        InputModes.Modes[i].numSources = pQcarcamModes->pModes[i].numSources;
        for (j=0; j<pQcarcamModes->pModes[i].numSources; j++) {
            InputModes.Modes[i].sources[j].srcId = pQcarcamModes->pModes[i].sources[j].srcId;
            InputModes.Modes[i].sources[j].width = pQcarcamModes->pModes[i].sources[j].width;
            InputModes.Modes[i].sources[j].height = pQcarcamModes->pModes[i].sources[j].height;
            InputModes.Modes[i].sources[j].colorFmt = static_cast<QCarCamColorFmt>(pQcarcamModes->pModes[i].sources[j].colorFmt);
            InputModes.Modes[i].sources[j].fps = pQcarcamModes->pModes[i].sources[j].fps;
            InputModes.Modes[i].sources[j].securityDomain = pQcarcamModes->pModes[i].sources[j].securityDomain;
        }
    }

    // Send back the results
    _hidl_cb(Error, InputModes);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();

}

Return<void> ais_hidl_camera::openStream(const QCarCamOpenParam& openParam, openStream_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry openStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    QCarCamRet_e ret = QCARCAM_RET_OK;
    QCarCamHndl_t qcarcam_hndl;
    sp<IQcarCameraStream> pActiveStream = nullptr;
    int32_t i = 0;
    int32_t j = 0;
    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // initialize qcarcam for first client
        QCarCamRet_e ret = QCARCAM_RET_OK;
        QCarCamInit_t qCarCamInit = {};
        qCarCamInit.apiVersion = QCARCAM_VERSION;
        ret = QCarCamInitialize(&qCarCamInit);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("QCarCamInitialize failed %d", ret);
            // Return empty list
            Error = static_cast<QCarCamError>(ret);
            client_lk.unlock();
            _hidl_cb(pActiveStream, Error);
            return Void();
        }
    }

    // Call qcarcam API
    QCarCamOpen_t q_openParams = {};
    q_openParams.numInputs = openParam.numInputs;
    if (q_openParams.numInputs > 4)
    {
        AIS_HIDL_ERRORMSG("Maximum number of inputs supported is 4");
        q_openParams.numInputs = 4;
    }
    for (i=0; i<openParam.numInputs; i++) {
        q_openParams.inputs[i].inputId = openParam.inputs[i].inputId;
        q_openParams.inputs[i].srcId = openParam.inputs[i].srcId;
        q_openParams.inputs[i].inputMode = openParam.inputs[i].inputMode;
    }
    q_openParams.opMode = static_cast<QCarCamOpmode_e>(openParam.opMode);
    q_openParams.priority = openParam.priority;
    q_openParams.flags = openParam.flags;

    ret = QCarCamOpen(&q_openParams, &qcarcam_hndl);

    if (ret != QCARCAM_RET_OK || QCARCAM_HNDL_INVALID == qcarcam_hndl) {
        AIS_HIDL_ERRORMSG("QCarCamOpen failed");
        Error = static_cast<QCarCamError>(ret);
    } else {
        mQcarcamClientCnt++;
        // Store qcarcam hndl in Event Q for callback mapping
        gMultiStreamQLock.lock();
        // Find an empty slot and insert handle
        for (j=0; j<MAX_AIS_CLIENTS; j++) {
            if (NOT_IN_USE == gQcarCamClientArray[j].qcarcam_context)
                break;
        }
        if (j < MAX_AIS_CLIENTS) {
            gQcarCamClientArray[j].qcarcam_context = qcarcam_hndl;
            gMultiStreamQLock.unlock();

            // Construct a camera instance for the caller
            pActiveStream = new ais_hidl_stream(openParam.inputs[i-1].inputId, &gQcarCamClientArray[j], qcarcam_hndl);
            if (pActiveStream == nullptr) {
                AIS_HIDL_ERRORMSG("Failed to allocate new ais_hidl_stream object for %d\n", static_cast<int>(openParam.inputs[i-1].inputId));
                Error = QCarCamError::QCARCAM_RET_FAILED;
            }
        } else {
            gMultiStreamQLock.unlock();
            AIS_HIDL_ERRORMSG("Max number od streams reached!!");
            Error = QCarCamError::QCARCAM_RET_FAILED;
        }
    }

    client_lk.unlock();
    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d hndl %p", static_cast<int>(Error), qcarcam_hndl);
    _hidl_cb(pActiveStream, Error);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<QCarCamError> ais_hidl_camera::closeStream(const ::android::sp<IQcarCameraStream>& pCamera)  {
    AIS_HIDL_INFOMSG("Entry closeStream");

    QCarCamError Error = QCarCamError::QCARCAM_RET_OK;
    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (mQcarcamClientCnt <= 0) {
        AIS_HIDL_ERRORMSG("Error qcarcam not initialized");
        return QCarCamError::QCARCAM_RET_FAILED;
    } else {
        mQcarcamClientCnt--;
    }
    client_lk.unlock();
    if (pCamera == nullptr) {
        AIS_HIDL_ERRORMSG("Ignoring call to closeCamera with null camera ptr");
    }

    // No need to explicitly detele pCamera since its a smart pointer, destructor
    // gets called at the end of this function and qcarcam_close() will be
    // called in destructor

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

} //implementation
} //V2_0
} //qcarcam
} //automotive
} //qti
} //vendor

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
namespace qcarcam {

// Global stream array for storing stream data
gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
std::mutex gMultiStreamQLock;
std::condition_variable gClientNotifier;

std::vector<std::shared_ptr<IQcarCameraStream>> sCameraStreams;

#ifdef HAL_CAMERA_CPMS_SUPPORT
pm_event_t gPMStatus;
std::shared_ptr<PowerPolicyService> gPowerService;

void PowerEventThread (void *args) {
    AIS_AIDL_ERRORMSG("Initialize PowerEventInit");
    gPowerService = PowerEventInit(ais_aidl_camera::PowerEventCb, args);
}

int ais_aidl_camera::PowerEventCb(pm_event_t eventId, void* pUsrCtxt) {
    AIS_AIDL_ERRORMSG("Entry PowerEventCb");

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

            AIS_AIDL_ERRORMSG("PowerEventCb Starting sleep for 3 Sec to allow App to close sessions");
            usleep(sleepTimeUs);
            AIS_AIDL_ERRORMSG("PowerEventCb Checking for any resource still alive and closing them");

            if (sCameraStreams.empty()) {
                AIS_AIDL_ERRORMSG("Apps Got closed Abruptly");
            } else {
                itr = sCameraStreams.begin();
                while(itr != sCameraStreams.end()) {
                    if (*itr != nullptr) {
                        //const std::shared_ptr<IQcarCameraStream> camStream = ((*itr).get());
                        const std::shared_ptr<IQcarCameraStream> camStream;
                        QcarcamError _aidl_return;
                        pCamera->closeStream(camStream, &_aidl_return);
                        //pCamera->closeStream(*itr, &_aidl_return);
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
}

//Thread to monitor client count and call qcarcam_uninitialize
void ais_aidl_camera::clientMonitorThread()
{
    AIS_AIDL_INFOMSG("ais_aidl_camera::clientMonitorThread starting\n");

    while(!mMonitorThreadExit) {
        std::unique_lock<std::mutex> lk(gMultiStreamQLock);

        AIS_AIDL_INFOMSG("going to wait for event from stream obj");
        // Wait until stream obj notified
        gClientNotifier.wait(lk);

        std::unique_lock<std::mutex> client_lk(mClientCntLock);
        if (mQcarcamClientCnt == 0) {
            // All clients are closed. Perform Qcarcam_deinit
            qcarcam_ret_t ret = QCARCAM_RET_OK;
            AIS_AIDL_INFOMSG("Calling qcarcam_uninitialize");
            ret = qcarcam_uninitialize();
            if (ret != QCARCAM_RET_OK) {
                AIS_AIDL_ERRORMSG("qcarcam_uninitialize failed %d", ret);
            }
        }
        client_lk.unlock();
    }
    AIS_AIDL_INFOMSG("clientMonitorThread thread exiting");
}

::ndk::ScopedAStatus ais_aidl_camera::getInputStreamList_1_1(std::vector<QcarcamInputInfov2>* inputs, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}
::ndk::ScopedAStatus ais_aidl_camera::getInputStreamList(std::vector<QcarcamInputInfov2>* inputs, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry getInputStreamList");
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    std::vector<QcarcamInputInfov2> &InputLists = *inputs;
    uint32_t numInputs = 0, queryFilled = 0;

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    qcarcam_input_v2_t *sQueryInput = NULL;
    unsigned i = 0, j = 0;
    bool qcarcamInitialized = false;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // try initialize once
        qcarcam_init_t qcarcam_init = {};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            client_lk.unlock();
            goto exit;
        } else
            qcarcamInitialized = true;
    }
    else {
        client_lk.unlock();
    }

    // Call qcarcam API
    ret = qcarcam_query_inputs_v2(NULL, 0, &numInputs);
    if (QCARCAM_RET_OK == ret) {
        sQueryInput = (qcarcam_input_v2_t *)calloc(numInputs, sizeof(*sQueryInput));
        if (NULL != sQueryInput)
            ret = qcarcam_query_inputs_v2(sQueryInput, numInputs, &queryFilled);
        else
            goto exit;
    }
    if (QCARCAM_RET_OK != ret) {
        AIS_AIDL_ERRORMSG("Error qcarcam_query_inputs failed");
        // Return empty list
        Error = QcarcamError::QCARCAM_FAILED;
        // free sQueryInput
        free(sQueryInput);
        goto exit;
    }
    // Build up a packed array of QcarcamInputInfo for return
    InputLists.resize(numInputs);
    sCameraStreams.resize(numInputs);

    for (i = 0; i<numInputs; i++) {
        InputLists[i].desc = static_cast<QcarcamInputDesc>(sQueryInput[i].desc);
        InputLists[i].name = sQueryInput[i].name;
        InputLists[i].num_modes = sQueryInput[i].num_modes;
        for (j = 0; j<sQueryInput[i].num_modes; j++) {
            InputLists[i].modes[j].res.width = sQueryInput[i].modes[j].res.width;
            InputLists[i].modes[j].res.height = sQueryInput[i].modes[j].res.height;
            InputLists[i].modes[j].res.fps = sQueryInput[i].modes[j].res.fps;
            InputLists[i].modes[j].color_fmt = static_cast<uint32_t>(sQueryInput[i].modes[j].fmt);
        }
        InputLists[i].flags = sQueryInput[i].flags;
    }

    // Send back the results
    AIS_AIDL_ERRORMSG("reporting %zu cameras available", InputLists.size());

    // free sQueryInput
    free(sQueryInput);
 exit:
    if (qcarcamInitialized)
    {
        // Call qcarcam API
        AIS_AIDL_ERRORMSG("qcarcam_uninitialize getting called from getInputStreamList %d", ret);
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_uninitialize();
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("qcarcam_uninitialize failed %d", ret);
        }
        qcarcamInitialized = false;
        client_lk.unlock();
    }
    // AIDL convention says we return Void if we sent our result back via callback
    return ndk::ScopedAStatus::ok();

}

::ndk::ScopedAStatus ais_aidl_camera::openStream_1_1(QcarcamInputDesc Desc, std::shared_ptr<IQcarCameraStream>* pActiveStream) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

}
::ndk::ScopedAStatus ais_aidl_camera::openStream(QcarcamInputDesc Desc, std::shared_ptr<IQcarCameraStream>* pActiveStream) {
    AIS_AIDL_INFOMSG("Entry openStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_hndl_t qcarcam_hndl;
    qcarcam_input_desc_t desc;
    int32_t i = 0;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // initialize qcarcam for first client
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_init_t qcarcam_init = {};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK) {
            AIS_AIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            client_lk.unlock();
            return ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(Error));
        }
    }

    // Call qcarcam API
    desc = static_cast<qcarcam_input_desc_t>(Desc);
    qcarcam_hndl = qcarcam_open(desc);

    if (NULL == qcarcam_hndl) {
        AIS_AIDL_ERRORMSG("qcarcam_open failed");
        Error = QcarcamError::QCARCAM_FAILED;
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
            *pActiveStream = ndk::SharedRefBase::make<ais_aidl_stream>(Desc, &gQcarCamClientArray[i], qcarcam_hndl);
            if (*pActiveStream == nullptr) {
                AIS_AIDL_ERRORMSG("Failed to allocate new ais_aidl_stream object for %d\n", static_cast<int>(Desc));
                Error = QcarcamError::QCARCAM_FAILED;
            } else {
                if (std::find (sCameraStreams.begin(), sCameraStreams.end(), *pActiveStream) == sCameraStreams.end()) {
                    sCameraStreams.push_back(*pActiveStream);
                }
              }
        } else {
            gMultiStreamQLock.unlock();
            AIS_AIDL_ERRORMSG("Max number od streams reached!!");
            Error = QcarcamError::QCARCAM_FAILED;
        }
    }

    client_lk.unlock();
    // Send back the results
    AIS_AIDL_INFOMSG("sending Error status %d hndl %p", static_cast<int>(Error), qcarcam_hndl);

    // AIDL convention says we return Void if we sent our result back via callback
    return (Error==QcarcamError::QCARCAM_OK) ? ndk::ScopedAStatus::ok() : ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(Error));
}

::ndk::ScopedAStatus ais_aidl_camera::closeStream_1_1(const std::shared_ptr<IQcarCameraStream>& camStream, QcarcamError* _aidl_return) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

::ndk::ScopedAStatus ais_aidl_camera::closeStream(const std::shared_ptr<IQcarCameraStream>& camStream, QcarcamError* _aidl_return) {
    AIS_AIDL_ERRORMSG("Entry closeStream");

    QcarcamError &Error = *_aidl_return;
    Error = QcarcamError::QCARCAM_OK;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (mQcarcamClientCnt <= 0) {
        AIS_AIDL_ERRORMSG("Error qcarcam not initialized");
        Error = QcarcamError::QCARCAM_FAILED;
        return ndk::ScopedAStatus::ok();
    } else {
        mQcarcamClientCnt--;
    }
    client_lk.unlock();
    if (camStream == nullptr) {
        AIS_AIDL_ERRORMSG("Ignoring call to closeCamera with null camera ptr");
    } else {
        if (std::find (sCameraStreams.begin(), sCameraStreams.end(), camStream) != sCameraStreams.end()) {
            AIS_AIDL_ERRORMSG("Found an entry removing it");
            std::vector<std::shared_ptr<IQcarCameraStream>>::iterator itr;
            itr = std::find (sCameraStreams.begin(), sCameraStreams.end(), camStream);
            sCameraStreams.erase(itr);
        }
        else
            AIS_AIDL_ERRORMSG("Entry not found");
    }

    // No need to explicitly detele camStream since its a smart pointer, destructor
    // gets called at the end of this function and qcarcam_close() will be
    // called in destructor

    // Send back the results
    AIS_AIDL_ERRORMSG("sending Error status %d", static_cast<int>(Error));
    return ndk::ScopedAStatus::ok();
}

} // qcarcam
} // automotive
} // qti
} // vendor
} // aidl

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
#include "ais_hidl_camera.h"
#include "ais_hidl_stream.h"

#ifdef HAL_CAMERA_CPMS_SUPPORT
#define HAL_SLEEP_USEC 1000000
#define HAL_DEFAULT_SLEEP_USEC 2
#endif

namespace vendor {
namespace qti {
namespace automotive {
namespace qcarcam {
namespace V1_0 {
namespace implementation {

// Global stream array for storing stream data
gQcarCamClientData gQcarCamClientArray[MAX_AIS_CLIENTS];
std::mutex gMultiStreamQLock;
std::condition_variable gClientNotifier;

std::vector<sp<IQcarCameraStream_1_1>> sCameraStreams;

#ifdef HAL_CAMERA_CPMS_SUPPORT
pm_event_t gPMStatus;
std::shared_ptr<PowerPolicyService> gPowerService;

void PowerEventThread (void *args) {
    AIS_HIDL_ERRORMSG("Initialize PowerEventInit");
    gPowerService = PowerEventInit(ais_hidl_camera::PowerEventCb, args);
}

int ais_hidl_camera::PowerEventCb(pm_event_t eventId, void* pUsrCtxt) {
    AIS_HIDL_ERRORMSG("Entry PowerEventCb");

    std::vector<sp<IQcarCameraStream_1_1>>::iterator itr;
    ais_hidl_camera *pCamera = (ais_hidl_camera *)pUsrCtxt;

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.qcom.ais.power.sleep", value, "0");

    int sleepTime = 0;
    sleepTime = atoi(value);

    int sleepTimeUs = ((sleepTime > 0 && sleepTime <= 3) ?
        sleepTime : HAL_DEFAULT_SLEEP_USEC) * HAL_SLEEP_USEC;
    AIS_HIDL_INFOMSG("sleep Value : %d", sleepTimeUs);

    switch (eventId)
    {
        case AIS_PM_SUSPEND:
        {
            AIS_HIDL_ERRORMSG("PowerEventCb AIS_PM_SUSPEND ");
            if (gPMStatus == AIS_PM_SUSPEND) {
                AIS_HIDL_ERRORMSG("Status is already in the Suspend");
                break;
            }

            AIS_HIDL_ERRORMSG("PowerEventCb Starting sleep for 3 Sec to allow App to close sessions");
            usleep(sleepTimeUs);
            AIS_HIDL_ERRORMSG("PowerEventCb Checking for any resource still alive and closing them");

            if (sCameraStreams.empty()) {
                AIS_HIDL_ERRORMSG("Apps Got closed Abruptly");
            } else {
                itr = sCameraStreams.begin();
                while(itr != sCameraStreams.end()) {
                    if (*itr != nullptr) {
                        const sp<IQcarCameraStream_1_1> camStream;
                        pCamera->closeStream(camStream);
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
            AIS_HIDL_ERRORMSG("PowerEventCb AIS_PM_RESUME");
            if (gPMStatus == AIS_PM_RESUME) {
                AIS_HIDL_ERRORMSG("Status is already in the Resume");
                break;
            }

            gPMStatus = AIS_PM_RESUME;
            break;
        }
    }

    AIS_HIDL_INFOMSG("Exit PowerEventCb");
    return 0;
}
#endif

//Default constructors
ais_hidl_camera::ais_hidl_camera()  {
    AIS_HIDL_INFOMSG("Entry ais_hidl_camera constructor");
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
ais_hidl_camera::~ais_hidl_camera()  {
    AIS_HIDL_INFOMSG("Entry ais_hidl_camera destructor");

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
            // All clients are closed. Perform Qcarcam_deinit
            qcarcam_ret_t ret = QCARCAM_RET_OK;
            AIS_HIDL_INFOMSG("Calling qcarcam_uninitialize");
            ret = qcarcam_uninitialize();
            if (ret != QCARCAM_RET_OK) {
                AIS_HIDL_ERRORMSG("qcarcam_uninitialize failed %d", ret);
            }
        }
        client_lk.unlock();
    }
    AIS_HIDL_INFOMSG("clientMonitorThread thread exiting");
}

// API definations
Return<void> ais_hidl_camera::getInputStreamList(getInputStreamList_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getInputStreamList");
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    hidl_vec<QcarcamInputInfo> InputLists;
    uint32_t numInputs = 0, queryFilled = 0;
    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_input_t *sQueryInput = NULL;
    unsigned i = 0, j = 0;
    bool qcarcamInitialized = false;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // try initialize once
        qcarcam_init_t qcarcam_init = {};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            _hidl_cb(Error, InputLists);
            client_lk.unlock();
            goto exit;
        } else
            qcarcamInitialized = true;
    }
    else {
        client_lk.unlock();
    }

    // Call qcarcam API
    ret = qcarcam_query_inputs(NULL, 0, &numInputs);
    if (QCARCAM_RET_OK == ret) {
        sQueryInput = (qcarcam_input_t *)calloc(numInputs, sizeof(*sQueryInput));
        if (NULL != sQueryInput)
            ret = qcarcam_query_inputs(sQueryInput, numInputs, &queryFilled);
        else
            goto exit;
    }
    if (QCARCAM_RET_OK != ret) {
        AIS_HIDL_ERRORMSG("Error qcarcam_query_inputs failed");
        // Return empty list
        Error = QcarcamError::QCARCAM_FAILED;
        _hidl_cb(Error, InputLists);
        // free sQueryInput
        free(sQueryInput);
        goto exit;
    }
    // Build up a packed array of QcarcamInputInfo for return
    InputLists.resize(numInputs);

    for (i = 0; i<numInputs; i++) {
        InputLists[i].desc = static_cast<QcarcamInputDesc>(sQueryInput[i].desc);
        InputLists[i].name = sQueryInput[i].name;
        InputLists[i].parent_name = sQueryInput[i].parent_name;
        InputLists[i].num_res = sQueryInput[i].num_res;
        for (j = 0; j<sQueryInput[i].num_res; j++) {
            InputLists[i].res[j].width = sQueryInput[i].res[j].width;
            InputLists[i].res[j].height = sQueryInput[i].res[j].height;
            InputLists[i].res[j].fps = sQueryInput[i].res[j].fps;
        }
        InputLists[i].num_color_fmt = sQueryInput[i].num_color_fmt;
        for (j = 0; j<sQueryInput[i].num_color_fmt; j++)
            InputLists[i].color_fmt[j] = static_cast<uint32_t>(sQueryInput[i].color_fmt[j]);
        InputLists[i].flags = sQueryInput[i].flags;
    }

    // Send back the results
    AIS_HIDL_INFOMSG("reporting %zu cameras available", InputLists.size());
    _hidl_cb(Error, InputLists);

    // free sQueryInput
    free(sQueryInput);
 exit:
    if (qcarcamInitialized)
    {
        // Call qcarcam API
        AIS_HIDL_INFOMSG("qcarcam_uninitialize getting called from getInputStreamList %d", ret);
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_uninitialize();
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_uninitialize failed %d", ret);
        }
        qcarcamInitialized = false;
        client_lk.unlock();
    }
    // HIDL convention says we return Void if we sent our result back via callback
    return Void();

}

Return<void> ais_hidl_camera::getInputStreamList_1_1(getInputStreamList_1_1_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry getInputStreamList_1_1");
    qcarcam_ret_t ret = QCARCAM_RET_OK;
    hidl_vec<vendor::qti::automotive::qcarcam::V1_1::QcarcamInputInfov2> InputLists;
    uint32_t numInputs = 0, queryFilled = 0;
    QcarcamError Error = QcarcamError::QCARCAM_OK;
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
            AIS_HIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            _hidl_cb(Error, InputLists);
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
        AIS_HIDL_ERRORMSG("Error qcarcam_query_inputs failed");
        // Return empty list
        Error = QcarcamError::QCARCAM_FAILED;
        _hidl_cb(Error, InputLists);
        // free sQueryInput
        free(sQueryInput);
        goto exit;
    }
    // Build up a packed array of QcarcamInputInfo for return
    InputLists.resize(numInputs);

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
    AIS_HIDL_INFOMSG("reporting %zu cameras available", InputLists.size());
    _hidl_cb(Error, InputLists);

    // free sQueryInput
    free(sQueryInput);
 exit:
    if (qcarcamInitialized)
    {
        // Call qcarcam API
        AIS_HIDL_INFOMSG("qcarcam_uninitialize getting called from getInputStreamList %d", ret);
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        ret = qcarcam_uninitialize();
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_uninitialize failed %d", ret);
        }
        qcarcamInitialized = false;
        client_lk.unlock();
    }
    // HIDL convention says we return Void if we sent our result back via callback
    return Void();

}

Return<void> ais_hidl_camera::openStream(QcarcamInputDesc Desc, openStream_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry openStream");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_hndl_t qcarcam_hndl;
    qcarcam_input_desc_t desc;
    sp<IQcarCameraStream_1_0> pActiveStream = nullptr;
    int32_t i = 0;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // initialize qcarcam for first client
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_init_t qcarcam_init = {};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            client_lk.unlock();
            _hidl_cb(pActiveStream, Error);
            return Void();
        }
    }

    // Call qcarcam API
    desc = static_cast<qcarcam_input_desc_t>(Desc);
    qcarcam_hndl = qcarcam_open(desc);

    if (NULL == qcarcam_hndl) {
        AIS_HIDL_ERRORMSG("qcarcam_open failed");
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
            pActiveStream = new ais_hidl_stream(Desc, &gQcarCamClientArray[i], qcarcam_hndl);
            if (pActiveStream == nullptr) {
                AIS_HIDL_ERRORMSG("Failed to allocate new ais_hidl_stream object for %d\n", static_cast<int>(Desc));
                Error = QcarcamError::QCARCAM_FAILED;
            }
        } else {
            gMultiStreamQLock.unlock();
            AIS_HIDL_ERRORMSG("Max number od streams reached!!");
            Error = QcarcamError::QCARCAM_FAILED;
        }
    }

    client_lk.unlock();
    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d hndl %p", static_cast<int>(Error), qcarcam_hndl);
    _hidl_cb(pActiveStream, Error);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<void> ais_hidl_camera::openStream_1_1(QcarcamInputDesc Desc, openStream_1_1_cb _hidl_cb)  {
    AIS_HIDL_INFOMSG("Entry openStream_1_1");

    QcarcamError Error = QcarcamError::QCARCAM_OK;
    qcarcam_hndl_t qcarcam_hndl;
    qcarcam_input_desc_t desc;
    sp<IQcarCameraStream_1_1> pActiveStream = nullptr;
    int32_t i = 0;

    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (0 == mQcarcamClientCnt) {
        // initialize qcarcam for first client
        qcarcam_ret_t ret = QCARCAM_RET_OK;
        qcarcam_init_t qcarcam_init = {};
        qcarcam_init.version = QCARCAM_VERSION;
        ret = qcarcam_initialize(&qcarcam_init);
        if (ret != QCARCAM_RET_OK) {
            AIS_HIDL_ERRORMSG("qcarcam_initialize failed %d", ret);
            // Return empty list
            Error = QcarcamError::QCARCAM_FAILED;
            client_lk.unlock();
            _hidl_cb(pActiveStream, Error);
            return Void();
        }
    }
    mQcarcamClientCnt++;
    client_lk.unlock();

    // Call qcarcam API
    desc = static_cast<qcarcam_input_desc_t>(Desc);
    qcarcam_hndl = qcarcam_open(desc);

    if (NULL == qcarcam_hndl) {
        AIS_HIDL_ERRORMSG("qcarcam_open failed");
        Error = QcarcamError::QCARCAM_FAILED;
    } else {
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
            pActiveStream = new ais_hidl_stream(Desc, &gQcarCamClientArray[i], qcarcam_hndl);
            if (pActiveStream == nullptr) {
                AIS_HIDL_ERRORMSG("Failed to allocate new ais_hidl_stream object for %d\n", static_cast<int>(Desc));
                Error = QcarcamError::QCARCAM_FAILED;
            } else {
                if (std::find (sCameraStreams.begin(), sCameraStreams.end(), pActiveStream) == sCameraStreams.end()) {
                    sCameraStreams.push_back(pActiveStream);
                }
              }
        } else {
            gMultiStreamQLock.unlock();
            AIS_HIDL_ERRORMSG("Max number od streams reached!!");
            Error = QcarcamError::QCARCAM_FAILED;
        }
    }

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d hndl %p", static_cast<int>(Error), qcarcam_hndl);
    _hidl_cb(pActiveStream, Error);

    // HIDL convention says we return Void if we sent our result back via callback
    return Void();
}

Return<QcarcamError> ais_hidl_camera::closeStream(const ::android::sp<IQcarCameraStream_1_0>& pCamera)  {
    AIS_HIDL_INFOMSG("Entry closeStream");

   QcarcamError Error = QcarcamError::QCARCAM_OK;
    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (mQcarcamClientCnt <= 0) {
        AIS_HIDL_ERRORMSG("Error qcarcam not initialized");
        return QcarcamError::QCARCAM_FAILED;
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

Return<QcarcamError> ais_hidl_camera::closeStream_1_1(const ::android::sp<IQcarCameraStream_1_1>& pCamera)  {
    AIS_HIDL_INFOMSG("Entry closeStream_1_1");

   QcarcamError Error = QcarcamError::QCARCAM_OK;
    std::unique_lock<std::mutex> client_lk(mClientCntLock);
    if (mQcarcamClientCnt <= 0) {
        AIS_HIDL_ERRORMSG("Error qcarcam not initialized");
        return QcarcamError::QCARCAM_FAILED;
    } else {
        mQcarcamClientCnt--;
    }
    client_lk.unlock();
    if (pCamera == nullptr) {
        AIS_HIDL_ERRORMSG("Ignoring call to closeCamera with null camera ptr");
    } else {
        if (std::find (sCameraStreams.begin(), sCameraStreams.end(), pCamera) != sCameraStreams.end()) {
            AIS_HIDL_ERRORMSG("Found an entry removing it");
            std::vector< sp<IQcarCameraStream_1_1 > >::iterator itr;
            itr = std::find (sCameraStreams.begin(), sCameraStreams.end(), pCamera);
            sCameraStreams.erase(itr);
        }
        else
            AIS_HIDL_ERRORMSG("Entry not found");
    }

    // No need to explicitly detele pCamera since its a smart pointer, destructor
    // gets called at the end of this function and qcarcam_close() will be
    // called in destructor

    // Send back the results
    AIS_HIDL_INFOMSG("sending Error status %d", static_cast<int>(Error));
    return Error;
}

} //implementation
} //V1_0
} //qcarcam
} //automotive
} //qti
} //vendor

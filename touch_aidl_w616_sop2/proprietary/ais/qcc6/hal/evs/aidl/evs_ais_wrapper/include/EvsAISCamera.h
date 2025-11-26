/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_AUTOMOTIVE_EVS_AIDL_EVSAISCAMERA_H
#define ANDROID_HARDWARE_AUTOMOTIVE_EVS_AIDL_EVSAISCAMERA_H

#include <aidl/android/hardware/automotive/evs/BnEvsCamera.h>
#include <aidl/android/hardware/automotive/evs/BufferDesc.h>
#include <aidl/android/hardware/automotive/evs/CameraDesc.h>
#include <aidl/android/hardware/automotive/evs/CameraParam.h>
#include <aidl/android/hardware/automotive/evs/EvsResult.h>
#include <aidl/android/hardware/automotive/evs/IEvsCameraStream.h>
#include <aidl/android/hardware/automotive/evs/IEvsDisplay.h>
#include <aidl/android/hardware/automotive/evs/ParameterRange.h>
#include <aidl/android/hardware/automotive/evs/Stream.h>
#include <aidl/android/hardware/graphics/common/PixelFormat.h>
#include <android-base/result.h>
#include <c2d2.h>
#include <ui/GraphicBuffer.h>
#include <android/hardware_buffer.h>
#include <QtiGralloc.h>

#include <functional>
#include <thread>
#include <set>
#include <list>

#include "ConfigManager.h"
#include "qcarcam.h"

#ifdef FPS_PRINT
#define FPS_TIME_INTERVAL_IN_MS 10
#define FPS_TIME_COUNTER 100
#endif

using namespace std;

//qcarcam data structures
#define MAX_AIS_CAMERAS 12
typedef struct
{
    void* ptr;
    int size;
    uint32_t srcC2dSurfaceId;
}qcarcam_mapped_buffer_t;

struct QEventDesc {
    uint32_t event_id;
    QCarCamEventPayload_t *p_payload;

    QEventDesc(uint32_t event, QCarCamEventPayload_t *payload) {event_id = event; p_payload = payload;}
};

typedef struct QcarcamCamera {
    QCarCamHndl_t gQcarcamHndl;
    std::mutex gEventQLock;
    std::condition_variable gCV;
    std::list<QEventDesc> sQcarcamList;
}gQcarCamClientData;

static gQcarCamClientData sQcarcamCameraArray[MAX_AIS_CAMERAS];
static std::mutex mMultiCamQLock;

namespace aidl::android::hardware::automotive::evs::implementation {

namespace aidlevs = ::aidl::android::hardware::automotive::evs;

// From EvsEnumerator.h
class EvsEnumerator;

class EvsAISCamera : public ::aidl::android::hardware::automotive::evs::BnEvsCamera {
public:
    // Methods from ::android::hardware::automotive::aidlevs::IEvsCamera follow.
    ::ndk::ScopedAStatus getCameraInfo(aidlevs::CameraDesc* _aidl_return) override;
    ::ndk::ScopedAStatus setMaxFramesInFlight(int32_t bufferCount) override;
    ::ndk::ScopedAStatus startVideoStream(
            const std::shared_ptr<aidlevs::IEvsCameraStream>& receiver) override;
    ::ndk::ScopedAStatus doneWithFrame(const std::vector<aidlevs::BufferDesc>& buffers) override;
    ::ndk::ScopedAStatus stopVideoStream() override;

    ::ndk::ScopedAStatus getPhysicalCameraInfo(const std::string& deviceId,
                                               aidlevs::CameraDesc* _aidl_return) override;
    ::ndk::ScopedAStatus pauseVideoStream() override;
    ::ndk::ScopedAStatus resumeVideoStream() override;
    ::ndk::ScopedAStatus setPrimaryClient() override;
    ::ndk::ScopedAStatus unsetPrimaryClient() override;
    ::ndk::ScopedAStatus forcePrimaryClient(
            const std::shared_ptr<aidlevs::IEvsDisplay>& display) override;
    ::ndk::ScopedAStatus getParameterList(std::vector<aidlevs::CameraParam>* _aidl_return) override;
    ::ndk::ScopedAStatus getIntParameterRange(aidlevs::CameraParam id,
                                              aidlevs::ParameterRange* _aidl_return) override;
    ::ndk::ScopedAStatus setIntParameter(aidlevs::CameraParam id, int32_t value,
                                         std::vector<int32_t>* effectiveValue) override;
    ::ndk::ScopedAStatus getIntParameter(aidlevs::CameraParam id,
                                         std::vector<int32_t>* value) override;
    ::ndk::ScopedAStatus setExtendedInfo(int32_t opaqueIdentifier,
                                         const std::vector<uint8_t>& opaqueValue) override;
    ::ndk::ScopedAStatus getExtendedInfo(int32_t opaqueIdentifier,
                                         std::vector<uint8_t>* value) override;
    ::ndk::ScopedAStatus importExternalBuffers(const std::vector<aidlevs::BufferDesc>& buffers,
                                               int32_t* _aidl_return) override;
//check if below function is needed to support test app; if not needed remove it
    ::ndk::ScopedAStatus doneWithFrame(const aidlevs::BufferDesc& buffers);
    static std::shared_ptr<EvsAISCamera> Create(const char *deviceName,
                                   void *m_pAisInputList,
                                   int numAisInputs,
                                   bool c2dInitStatus,
                                   EvsEnumerator *service = nullptr);
    static std::shared_ptr<EvsAISCamera> Create(const char *deviceName,
                                   void *m_pAisInputList,
                                   int numAisInputs,
                                   bool c2dInitStatus,
                                   unique_ptr<ConfigManager::CameraInfo> &camInfo,
                                   EvsEnumerator *service = nullptr,
                                   const Stream *streamCfg = nullptr);
    EvsAISCamera(const EvsAISCamera&) = delete;
    EvsAISCamera& operator=(const EvsAISCamera&) = delete;

    virtual ~EvsAISCamera() override;
    void shutdown();

    const aidlevs::CameraDesc& getDesc() { return mDescription; }

    // Constructors
    EvsAISCamera(const char *deviceName,
                 unique_ptr<ConfigManager::CameraInfo> &camInfo, EvsEnumerator *service = nullptr);

#ifdef FPS_PRINT
    bool startCalculateFps();
    void terminateCalculateFps();
#endif

private:

#ifdef FPS_PRINT
    void calculateFps();
    std::thread mCalculateFps;
    bool get_frame_cnt;
    unsigned int frame_cnt;
#endif

    // These three functions are expected to be called while mAccessLock is held
    bool setAvailableFrames_Locked(unsigned bufferCount);
    unsigned increaseAvailableFrames_Locked(unsigned numToAdd);
    unsigned decreaseAvailableFrames_Locked(unsigned numToRemove);

    void forwardFrame(QCarCamFrameInfo_t *frame_info);
    bool convertToqcarcamParamID(CameraParam id, int32_t value);
    bool convertFormqcarcamParamID(CameraParam id, vector<int32_t>& values);

    // The callback used to deliver each frame
    std::shared_ptr<aidlevs::IEvsCameraStream> mStream;
    EvsEnumerator *mEvs = nullptr;

    // The properties of this camera
    aidlevs::CameraDesc mDescription = {};

    uint32_t mOutBufFormat = 0;           // Values from android_pixel_format_t
    uint32_t mUsage  = 0;           // Values from from Gralloc.h
    uint32_t mStride = 0;           // Pixels per row (may be greater than image width)
    uint32_t mOutHeight = 0;           // Output Target Height
    uint32_t mOutWidth = 0;           // Output Target Width

    struct BufferRecord {
        buffer_handle_t handle;
        bool inUse;

        explicit BufferRecord(buffer_handle_t h) : handle(h), inUse(false) {};
    };

    std::vector <BufferRecord> mBuffers;    // Graphics buffers to transfer images
    unsigned mFramesAllowed;                // How many buffers are we currently using
    unsigned mFramesInUse;                  // How many buffers are currently outstanding

    // Which format specific function we need to use to move camera imagery into our output buffers
    void(*mFillBufferFromVideo)(const BufferDesc& tgtBuff, uint8_t* tgt,
                                void* imgData, unsigned imgStride);


    aidlevs::EvsResult doneWithFrame_impl(uint32_t id, buffer_handle_t handle);

    // Synchronization necessary to deconflict the capture thread from the main service thread
    // Note that the service interface remains single threaded (ie: not reentrant)
    mutable std::mutex mAccessLock;

    // Static camera module information
    std::unique_ptr<ConfigManager::CameraInfo> &mCameraInfo;

    // Extended information
    std::unordered_map<uint32_t, std::vector<uint8_t>> mExtInfo;

    //qcarcam datatypes
    QCarCamHndl_t mQcarcamHndl;
    std::thread mCaptureThread;             // The thread we'll use to dispatch frames
#ifdef MOCK_CAMERA_HOTPLUG
    std::thread mHotPlugThread;             // The thread is use to mock hot plug on android property
#endif
    std::atomic<int> mRunMode;              // Used to signal the frame loop (see RunModes below)
    // Careful changing these -- we're using bit-wise ops to manipulate these
    enum RunModes {
        STOPPED     = 0,
        RUN         = 1,
        STOPPING    = 2,
    };
    uint32_t mAisFrameHeight;
    uint32_t mAisFrameWidth;
    float mAisFrameFPS;
    QCarCamColorFmt_e mAisFrameFormat;
    uint32_t mHalFrameFormat;
    void collectEvents();
#ifdef MOCK_CAMERA_HOTPLUG
    void mockCameraHotPlug();
#endif
    void handleEvents(QEventDesc &events);
    void dumpqcarcamFrame(QCarCamFrameInfo_t *frame_info);
    void sendFramesToApp(QCarCamFrameInfo_t *frame_info);
    void fillTargetBuffer(uint8_t* tgt, void* imgData, unsigned imgStride);
    float clamp(float v, float min, float max);
    uint32_t yuvToRgbx(const unsigned char Y, const unsigned char Uin, const unsigned char Vin);
    static QCarCamColorFmt_e getHalToQcarcamFormat(uint32_t halFormat);
    static uint32_t getQcarcamStride(uint32_t width, uint32_t mFormat);
    static uint32_t getQcarcamToHalFormat(QCarCamColorFmt_e qcarcamFormat);
    bool allocQcarcamInternalBuffers(uint32_t count);
    bool setExternalBuffersAsInteranl(vector<int>& extFds);

    static gQcarCamClientData* findEventQByHndl(const QCarCamHndl_t qcarcam_hndl);

    ::android::sp<::android::GraphicBuffer> *mpGfxBufs;
    QCarCamBufferList_t mQcarcamOutBufs;
    qcarcam_mapped_buffer_t *mpQcarcamMmapBufs;
    QCarCamFrameInfo_t mQcarcamFrameInfo;

    static QCarCamRet_e evs_ais_event_cb(const QCarCamHndl_t hndl,
        const uint32_t event_id,
        const QCarCamEventPayload_t *p_payload,
        void  *pPrivateData);
    gQcarCamClientData *pCurrentRecord;
    uint32_t getHalToC2dFormat(uint32_t halFormat);
    C2D_FORMAT_MODE getC2dColorSpaceFormat(uint32_t colorSpace);
    std::map<std::uint64_t, uint32_t> mTgtSurfaceIdList;
    bool mDirectRendering;
    bool mC2dInitStatus;
    std::map<std::uint64_t, void *> mSrcGpuMapList; ///> List to store mapped Source GPU addr
    std::map<std::uint64_t, void *> mTgtGpuMapList; ///> List to store mapped Target GPU addr
    std::map<std::uint32_t, std::uint64_t> mFdToMemHndlMapList; ///> List to store mapped external handles
};

} // namespace implementation

#endif  // ANDROID_HARDWARE_AUTOMOTIVE_EVS_V1_1_EVSV4LCAMERA_H

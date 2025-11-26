/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *Not a contribution.
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

#ifndef VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE3SESSION_H
#define VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE3SESSION_H

#include <aidl/android/hardware/camera/device/BnCameraDeviceSession.h>
#include <aidl/android/hardware/camera/device/HalStream.h>
#include <aidl/android/hardware/camera/device/ICameraDevice.h>
#include <aidl/android/hardware/camera/device/ICameraDeviceSession.h>
#include <aidl/android/hardware/common/fmq/MQDescriptor.h>
#include <fmq/AidlMessageQueue.h>
#include <fmq/MessageQueue.h>
#include <deque>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "CameraMetadata.h"
#include "convert.h"
#include "HandleImporter.h"
#include "camera3.h"
#include "camera_common.h"

namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace implementation {

using ::aidl::android::hardware::camera::common::Status;
using ::aidl::android::hardware::camera::device::BnCameraDeviceSession;
using ::aidl::android::hardware::camera::device::BufferCache;
using ::aidl::android::hardware::camera::device::BufferRequestStatus;
using ::aidl::android::hardware::camera::device::BufferRequest;
using ::aidl::android::hardware::camera::device::BufferStatus;
using ::aidl::android::hardware::camera::device::CameraMetadata;
using ::aidl::android::hardware::camera::device::CameraOfflineSessionInfo;
using ::aidl::android::hardware::camera::device::CaptureRequest;
using ::aidl::android::hardware::camera::device::CaptureResult;
using ::aidl::android::hardware::camera::device::HalStream;
using ::aidl::android::hardware::camera::device::ICameraDeviceCallback;
using ::aidl::android::hardware::camera::device::ICameraOfflineSession;
using ::aidl::android::hardware::camera::device::NotifyMsg;
using ::aidl::android::hardware::camera::device::RequestTemplate;
using ::aidl::android::hardware::camera::device::Stream;
using ::aidl::android::hardware::camera::device::StreamBuffer;
using ::aidl::android::hardware::camera::device::StreamBufferRet;
using ::aidl::android::hardware::camera::device::StreamBufferRequestError;
using ::aidl::android::hardware::camera::device::StreamBuffersVal;
using ::aidl::android::hardware::camera::device::StreamConfiguration;
using ::aidl::android::hardware::camera::device::StreamType;
using ::aidl::android::hardware::common::fmq::MQDescriptor;
using ::aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using ::aidl::android::hardware::graphics::common::BufferUsage;
using ::android::hardware::camera::common::V1_0::helper::HandleImporter;

/**
 * Function pointer types with C calling convention to
 * use for HAL callback functions.
 */
extern "C" {
    typedef void (callbacks_process_capture_result_t)(
        const struct camera3_callback_ops *,
        const camera3_capture_result_t *);

    typedef void (callbacks_notify_t)(
        const struct camera3_callback_ops *,
        const camera3_notify_msg_t *);

    typedef camera3_buffer_request_status_t (callbacks_request_stream_buffer_t)(
            const struct camera3_callback_ops *,
            uint32_t num_buffer_reqs,
            const camera3_buffer_request_t *buffer_reqs,
            /*out*/uint32_t *num_returned_buf_reqs,
            /*out*/camera3_stream_buffer_ret_t *returned_buf_reqs);

    typedef void (callbacks_return_stream_buffer_t)(
            const struct camera3_callback_ops *,
            uint32_t num_buffers,
            const camera3_stream_buffer_t* const* buffers);
}

struct CameraDeviceSession : public BnCameraDeviceSession, protected camera3_callback_ops  {

    CameraDeviceSession(camera3_device_t*,
                        const camera_metadata_t* deviceInfo,
                        const std::shared_ptr<ICameraDeviceCallback>&);
    virtual ~CameraDeviceSession();
    // Call by CameraDevice to dump active device states
    void dumpState(int fd);
    // Caller must use this method to check if CameraDeviceSession ctor failed
    bool isInitFailed() { return mInitFail; }
    // Used by CameraDevice to signal external camera disconnected
    void disconnect();
    bool isClosed();

protected:

    ::ndk::ScopedAStatus constructDefaultRequestSettings(
        RequestTemplate type,
        CameraMetadata* defaultRequestSettings) override;

    ::ndk::ScopedAStatus configureStreams(
        const StreamConfiguration& requestedConfiguration,
        std::vector<HalStream>*    halStreamList) override;

    ::ndk::ScopedAStatus configureStreams_Impl(
        const StreamConfiguration& requestedConfiguration,
        std::vector<HalStream>*    halStreamList,
        uint32_t                   streamConfigCounter       = 0,
        bool                       useOverriddenFields       = true,
        std::vector<int32_t>*      groupIds                  = nullptr,
        bool                       multiResolutionInputImage = false);

    ::ndk::ScopedAStatus signalStreamFlush(
        const std::vector<int32_t>& in_streamIds,
        int32_t                     in_streamConfigCounter) override;

    ::ndk::ScopedAStatus switchToOffline(
        const std::vector<int32_t>&             in_streamsToKeep,
        CameraOfflineSessionInfo*               out_offlineSessionInfo,
        std::shared_ptr<ICameraOfflineSession>* cameraOfflineSession) override;

    ::ndk::ScopedAStatus getCaptureRequestMetadataQueue(
        ::aidl::android::hardware::common::fmq::MQDescriptor<
          int8_t, SynchronizedReadWrite>* requestMetadataQuesu) override;

    ::ndk::ScopedAStatus getCaptureResultMetadataQueue(
        ::aidl::android::hardware::common::fmq::MQDescriptor<
          int8_t, SynchronizedReadWrite>* resultMetadataQuesu) override;

    ::ndk::ScopedAStatus processCaptureRequest(
            const std::vector<CaptureRequest>& requests,
            const std::vector<BufferCache>&    cachesToRemove,
            int32_t*                           numRequestProcessed) override;

    ::ndk::ScopedAStatus flush() override;

    ::ndk::ScopedAStatus close() override;

    // Helper methods
    ::ndk::ScopedAStatus constructDefaultRequestSettingsRaw(
        int             type,
        CameraMetadata* outMetadata);

    bool preProcessConfigurationLocked(
        const StreamConfiguration&      requestedConfiguration,
        bool                            useOverriddenFields,
        camera3_stream_configuration_t* stream_list /*out*/,
        std::vector<camera3_stream_t*>* streams /*out*/);

    void postProcessConfigurationLocked(const StreamConfiguration& requestedConfiguration);

    void postProcessConfigurationFailureLocked(const StreamConfiguration& requestedConfiguration);

    int getStreamWithInflightBufferInGroup(
        const Camera3Stream* stream,
        uint32_t&            frameNumber);

    ::ndk::ScopedAStatus processCaptureRequest_Impl(
        const std::vector<CaptureRequest>& requests,
        const std::vector<BufferCache>&    cachesToRemove,
        int32_t*                           numRequestProcessed,
        std::vector<uint32_t>*             inputWidth  = nullptr,
        std::vector<uint32_t>*             inputHeight = nullptr);

    Status processOneCaptureRequest(
        const CaptureRequest& request,
        uint32_t              inputWidth  = 0,
        uint32_t              inputHeight = 0);

    ::ndk::ScopedAStatus repeatingRequestEnd(
        int32_t                     in_frameNumber,
        const std::vector<int32_t>& in_streamIds) override {
        // Unimplemented
        return ndk::ScopedAStatus::ok();
    };

    std::map<int, std::string> mPhysicalCameraIdMap;
protected:
    using RequestMetadataQueue = AidlMessageQueue<int8_t, SynchronizedReadWrite>;
    std::unique_ptr<RequestMetadataQueue>       mRequestMetadataQueue;
    using ResultMetadataQueue = AidlMessageQueue<int8_t, SynchronizedReadWrite>;
    std::shared_ptr<ResultMetadataQueue>        mResultMetadataQueue;

    struct BufferHasher {
        size_t operator()(const buffer_handle_t& buf) const {
            if (buf == nullptr)
                return 0;

            size_t result = 1;
            result = 31 * result + buf->numFds;
            for (int i = 0; i < buf->numFds; i++) {
                result = 31 * result + buf->data[i];
            }
            return result;
        }
    };

    struct BufferComparator {
        bool operator()(const buffer_handle_t& buf1, const buffer_handle_t& buf2) const {
            if (buf1->numFds == buf2->numFds) {
                for (int i = 0; i < buf1->numFds; i++) {
                    if (buf1->data[i] != buf2->data[i]) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
    };

    typedef std::unordered_map<const buffer_handle_t, uint64_t,
            BufferHasher, BufferComparator>                BufferIdMap;
    // buffers currently ciculating between HAL and camera service
    // key: bufferId sent via HIDL interface
    // value: imported buffer_handle_t
    // Buffer will be imported during process_capture_request and will be freed
    // when the its stream is deleted or camera device session is closed
    typedef std::unordered_map<uint64_t, buffer_handle_t> CirculatingBuffers;

    struct AETriggerCancelOverride {
        bool    applyAeLock;
        uint8_t aeLock;
        bool    applyAePrecaptureTrigger;
        uint8_t aePrecaptureTrigger;
    };

    class ResultBatcher {
    public:
        ResultBatcher(const std::shared_ptr<ICameraDeviceCallback>& callback);
        void setNumPartialResults(uint32_t n);
        void setBatchedStreams(const std::vector<int>& streamsToBatch);
        void setResultMetadataQueue(std::shared_ptr<ResultMetadataQueue> q);
        void registerBatch(uint32_t frameNumber, uint32_t batchSize);
        void notify(NotifyMsg& msg);
        void processCaptureResult(CaptureResult& result);

    protected:
        struct InflightBatch {
            // Protect access to entire struct. Acquire this lock before read/write any data or
            // calling any methods. processCaptureResult and notify will compete for this lock
            // HIDL IPCs might be issued while the lock is held
            Mutex                  mLock;
            uint32_t               mFirstFrame;
            uint32_t               mLastFrame;
            uint32_t               mBatchSize;
            bool                   mShutterDelivered = false;
            std::vector<NotifyMsg> mShutterMsgs;

            bool allDelivered() const;

            struct BufferBatch {
                BufferBatch(uint32_t batchSize) {
                    mBuffers.reserve(batchSize);
                }
                // This currently assumes every batched request will output to the batched stream
                // and since HAL must always send buffers in order, no frameNumber tracking is
                // needed
                std::vector<StreamBuffer> mBuffers;
                bool                      mDelivered = false;
            };
            // Stream ID -> VideoBatch
            std::unordered_map<int, BufferBatch> mBatchBufs;

            struct MetadataBatch {
                //                   (frameNumber, metadata)
                std::vector<std::pair<uint32_t, CameraMetadata>> mMds;
            };
            // Partial result IDs that has been delivered to framework
            uint32_t                          mNumPartialResults;
            uint32_t                          mPartialResultProgress = 0;
            // partialResult -> MetadataBatch
            std::map<uint32_t, MetadataBatch> mResultMds;

            // Set to true when batch is removed from mInflightBatches
            // processCaptureResult and notify must check this flag after acquiring mLock to make
            // sure this batch isn't removed while waiting for mLock
            bool                              mRemoved = false;
        };

        // Get the batch index and pointer to InflightBatch (nullptrt if the frame is not batched)
        // Caller must acquire the InflightBatch::mLock before accessing the InflightBatch
        // It's possible that the InflightBatch is removed from mInflightBatches before the
        // InflightBatch::mLock is acquired (most likely caused by an error notification), so
        // caller must check InflightBatch::mRemoved flag after the lock is acquried.
        // This method will hold ResultBatcher::mLock briefly
        std::pair<int, std::shared_ptr<InflightBatch>> getBatch(uint32_t frameNumber);

        // move/push function avoids "hidl_handle& operator=(hidl_handle&)", which clones native
        // handle
        void moveStreamBuffer(StreamBuffer&& src, StreamBuffer& dst);
        void pushStreamBuffer(StreamBuffer&& src, std::vector<StreamBuffer>& dst);

        void sendBatchMetadataLocked(
            std::shared_ptr<InflightBatch> batch, uint32_t lastPartialResultIdx);

        // Check if the first batch in mInflightBatches is ready to be removed, and remove it if so
        // This method will hold ResultBatcher::mLock briefly
        void checkAndRemoveFirstBatch();

        // The following sendXXXX methods must be called while the InflightBatch::mLock is locked
        // HIDL IPC methods will be called during these methods.
        void sendBatchShutterCbsLocked(std::shared_ptr<InflightBatch> batch);
        // send buffers for all batched streams
        void sendBatchBuffersLocked(std::shared_ptr<InflightBatch> batch);
        // send buffers for specified streams
        void sendBatchBuffersLocked(
            std::shared_ptr<InflightBatch> batch, const std::vector<int>& streams);
       // End of sendXXXX methods

        // helper methods
        void freeReleaseFences(std::vector<CaptureResult>&);

        void notifySingleMsg(NotifyMsg& msg);
        void processOneCaptureResult(CaptureResult& result);
        void invokeProcessCaptureResultCallback(
            std::vector<CaptureResult> &results, bool tryWriteFmq);

        static const int                             NOT_BATCHED = -1;

        // Protect access to mInflightBatches, mNumPartialResults and mStreamsToBatch
        // processCaptureRequest, processCaptureResult, notify will compete for this lock
        // Do NOT issue HIDL IPCs while holding this lock (except when HAL reports error)
        mutable Mutex                                mLock;
        std::deque<std::shared_ptr<InflightBatch>>   mInflightBatches;
        uint32_t                                     mNumPartialResults;
        std::vector<int>                             mStreamsToBatch;
        const std::shared_ptr<ICameraDeviceCallback> mCallback;
        std::shared_ptr<ResultMetadataQueue>         mResultMetadataQueue;
        // Protect against invokeProcessCaptureResultCallback()
        Mutex                                        mProcessCaptureResultLock;

    } mResultBatcher;

    bool initialize();

    static bool shouldFreeBufEarly();

    Status initStatus() const;

    // Validate and import request's input buffer and acquire fence
    virtual Status importRequest(
        const CaptureRequest&          request,
        std::vector<buffer_handle_t*>& allBufPtrs,
        std::vector<int>&              allFences);

    ::ndk::ScopedAStatus isReconfigurationRequired(
        const CameraMetadata& oldSessionParams,
        const CameraMetadata& newSessionParams,
        bool*                 reconfiguration_required) override;

    Status importRequestImpl(
        const CaptureRequest&          request,
        std::vector<buffer_handle_t*>& allBufPtrs,
        std::vector<int>&              allFences,
        // Optional argument for ICameraDeviceSession@3.5 impl
        bool                           allowEmptyBuf = false);

    Status importBuffer(
        int32_t           streamId,
        uint64_t          bufId,
        buffer_handle_t   buf,
        buffer_handle_t** outBufPtr,
        bool              allowEmptyBuf);

    static void cleanupInflightFences(
        std::vector<int>& allFences,
        size_t            numFences);

    void cleanupBuffersLocked(int id);

    void updateBufferCaches(const std::vector<BufferCache>& cachesToRemove);

    android_dataspace mapToLegacyDataspace(
        android_dataspace dataSpace) const;

    bool handleAePrecaptureCancelRequestLocked(
        const camera3_capture_request_t&                                   halRequest,
        ::android::hardware::camera::common::V1_0::helper::CameraMetadata* settings /*out*/,
        AETriggerCancelOverride*                                           override /*out*/);

    void overrideResultForPrecaptureCancelLocked(
        const AETriggerCancelOverride&                                     aeTriggerCancelOverride,
        ::android::hardware::camera::common::V1_0::helper::CameraMetadata* settings /*out*/);

    camera3_buffer_request_status_t requestStreamBuffers(
        uint32_t num_buffer_reqs,
        const camera3_buffer_request_t *buffer_reqs,
        /*out*/uint32_t *num_returned_buf_reqs,
        /*out*/camera3_stream_buffer_ret_t *returned_buf_reqs);

    void returnStreamBuffers(
        uint32_t num_buffers,
        const camera3_stream_buffer_t* const* buffers);

    Camera3Stream* getStreamPointer(int32_t streamId);

    // Register buffer to mBufferIdMaps so we can find corresponding bufferId
    // when the buffer is returned to camera service
    void pushBufferId(
        const buffer_handle_t& buf,
        uint64_t               bufferId,
        int                    streamId);

    // Method to pop buffer's bufferId from mBufferIdMaps
    // BUFFER_ID_NO_BUFFER is returned if no matching buffer is found
    uint64_t popBufferId(
        const buffer_handle_t& buf,
        int                    streamId);

    // Method to cleanup imported buffer/fences if requestStreamBuffers fails half way
    void cleanupInflightBufferFences(
        std::vector<int>&                             fences,
        std::vector<std::pair<buffer_handle_t, int>>& bufs);

    status_t constructCaptureResult(
        CaptureResult&                result,
        const camera3_capture_result* hal_result);

    // By default camera service uses frameNumber/streamId pair to retrieve the buffer that
    // was sent to HAL. Override this implementation if HAL is using buffers from buffer management
    // APIs to send output buffer.
    virtual uint64_t getCapResultBufferId(
        const buffer_handle_t& buf,
        int                    streamId);

    // protecting mClosed/mDisconnected/mInitFail
    mutable Mutex                               mStateLock;
    // device is closed either
    //    - closed by user
    //    - init failed
    //    - camera disconnected
    bool                                        mClosed = false;

    // Set by CameraDevice (when external camera is disconnected)
    bool                                        mDisconnected = false;
    camera3_device_t*                           mDevice;
    const uint32_t                              mDeviceVersion;
    const bool                                  mFreeBufEarly;
    bool                                        mIsAELockAvailable;
    bool                                        mDerivePostRawSensKey;
    uint32_t                                    mNumPartialResults;
    // Stream ID -> Camera3Stream cache
    std::map<int, Camera3Stream>                mStreamMap;
    mutable Mutex                               mInflightLock; // protecting mInflightBuffers and mCirculatingBuffers
    // (streamID, frameNumber) -> inflight buffer cache
    std::map<std::pair<int, uint32_t>, camera3_stream_buffer_t>  mInflightBuffers;
    // (frameNumber, AETriggerOverride) -> inflight request AETriggerOverrides
    std::map<uint32_t, AETriggerCancelOverride> mInflightAETriggerOverrides;
    std::map<uint32_t, bool>                    mInflightRawBoostPresent;
    ::android::hardware::camera::common::V1_0::helper::CameraMetadata mOverridenResult;
    ::android::hardware::camera::common::V1_0::helper::CameraMetadata mOverridenRequest;
    ::android::hardware::camera::common::V1_0::helper::CameraMetadata mDeviceInfo;

    static const uint64_t                       BUFFER_ID_NO_BUFFER = 0;
    // Stream ID -> circulating buffers map
    std::map<int, CirculatingBuffers>           mCirculatingBuffers;
    static HandleImporter                       sHandleImporter;
    static buffer_handle_t                      sEmptyBuffer;
    bool                                        mInitFail;
    bool                                        mFirstRequest = false;
    std::mutex                                  mBufferIdMapLock; // protecting mBufferIdMaps and mNextBufferId
    // stream ID -> per stream buffer ID map for buffers coming from requestStreamBuffers API
    // Entries are created during requestStreamBuffers when a stream first request a buffer, and
    // deleted in returnStreamBuffers/processCaptureResult* when all buffers are returned
    std::unordered_map<int, BufferIdMap>        mBufferIdMaps;
    std::shared_ptr<ICameraDeviceCallback>      aidl_device_callback;
    bool                                        mSupportBufMgr;
    /**
     * Static callback forwarding methods from HAL to instance
     */
    static callbacks_process_capture_result_t   sProcessCaptureResult;
    static callbacks_notify_t                   sNotify;
    static callbacks_request_stream_buffer_t    sRequestStreamBuffers;
    static callbacks_return_stream_buffer_t     sReturnStreamBuffers;
    // Physical camera ids for the logical multi-camera. Empty if this
    // is not a logical multi-camera.
    std::unordered_set<std::string>             mPhysicalCameraIds;
    Mutex                                       mStreamConfigCounterLock;
    uint32_t                                    mStreamConfigCounter = 1;
    std::vector<int>                            mVideoStreamIds;

    // Static helper method to copy/shrink capture result metadata sent by HAL
    // Temporarily allocated metadata copy will be hold in mds
    static void sShrinkCaptureResult(
            camera3_capture_result* dst, const camera3_capture_result* src,
            std::vector<::android::hardware::camera::common::V1_0::helper::CameraMetadata>* mds,
            std::vector<const camera_metadata_t*>* physCamMdArray,
            bool handlePhysCam);

    static bool sShouldShrink(const camera_metadata_t* md);

    static camera_metadata_t* sCreateCompactCopy(const camera_metadata_t* src);
};

}  // namespace implementation
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  // VENDOR_QTI_CAMERA_DEVICE_CAMERADEVICE3SESSION_H

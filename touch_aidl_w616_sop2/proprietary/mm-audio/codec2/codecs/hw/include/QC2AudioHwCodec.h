/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_CODEC_H_
#define _QC2AUDIO_AUDIO_HW_CODEC_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>
#include "QC2.h"
#include "QC2Codec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2AudioHw.h"
#include "QC2Monitor.h"

namespace qc2audio {

static constexpr const int DEFAULT_OUTPUT_BUFFER_SIZE = 8192;
static const char *kHwCodecSuspendEnabledProp = "vendor.qc2audio.suspend.enabled";
static const char *kHwCodecSuspendTimeoutProp = "vendor.qc2audio.suspend.timeout";

class BufferList;
class QC2AudioHwCodecWorkerThread;
class QC2AudioHwCodec;

class QC2AudioHwDecoderFactory: public QC2CodecFactory {

public:
    virtual ~QC2AudioHwDecoderFactory() = default;
    QC2AudioHwDecoderFactory(const std::string& name, const std::string& mime,
            const std::string& nickName, uint32_t variant);

    QC2Status createCodec(
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) override;

    virtual std::string name();

 private:
    const std::string mName;
    const std::string mNickName;
    const std::string mCompressedMime;
    const uint32_t mVariant;
};

class QC2AudioHwEncoderFactory: public QC2CodecFactory {

public:
    virtual ~QC2AudioHwEncoderFactory() = default;
    QC2AudioHwEncoderFactory(const std::string& name, const std::string& mime,
            const std::string& nickName, uint32_t variant);

    QC2Status createCodec(
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) override;

    virtual std::string name();

 private:
    const std::string mName;
    const std::string mNickName;
    const std::string mCompressedMime;
    const uint32_t mVariant;
};

class QC2AudioHwCodecWorkerThread : public QC2Thread {
public:
    QC2AudioHwCodecWorkerThread(const std::string& name,
                                    QC2AudioHwCodec *codec);
    ~QC2AudioHwCodecWorkerThread();
    QC2Status stopThread();
    void signalError(QC2Status errorCode,
                            const std::string& errorMsg);
    void handleSignalledError(QC2Status errorCode,
                            std::string& errorMsg);
private:
    static const constexpr int timeout = 100;
    QC2AudioHwCodec& mCodec;
    const std::string mName;
    std::mutex mLock;
    std::condition_variable mCondition;
    std::condition_variable mAckExitCondition;
    QC2Status mErrorCode;
    std::string mErrorMsg;
    void threadLoop();
};

class QC2AudioHwCodec : public QC2Codec, public QC2AudioHw::Notifier {
 public:

    QC2AudioHwCodec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& inputMime,
            const std::string& outputMime,
            ComponentKind kind,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwCodec() = default;

    QC2Status getCapsHelpers(
            std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers
            ) const override;


    virtual QC2Status configure(const C2Param& param);
    virtual QC2Status configure(std::vector<std::unique_ptr<C2Param>> params);
    QC2Status queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const override;

    virtual QC2Status query(C2Param *const param);

    QC2Status start() override;

    virtual QC2Status stop();

    virtual QC2Status queueInputs(
            const std::list<std::shared_ptr<InputBufferPack>>& inputs,
            ReportConfigFailure_cb configFailureReporter,
            uint32_t *numInputsQueued);

    QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) override;

    virtual QC2Status flush();

    virtual QC2Status drain();

    virtual QC2Status release();

    QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) override;
    QC2Status deinit() override;

    QC2Status suspend() override;

    QC2Status resume() override;

    // Driver::Notifier
    void onInputsDone(std::shared_ptr<QC2AudioHwBuffer> inBuf);
    virtual void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf);
    void onError(QC2Status errorCode, const std::string& errorMsg);
    void onOutputReferenceReleased(uint64_t outputIndex) {
        (void)outputIndex;
    }
    void onReconfig(std::list<std::unique_ptr<C2Param>> targetConfig) {
        (void)targetConfig;
    }
    size_t getPendingInputsSize();
    size_t getPendingOutputsSize();
    bool isPendingInputsEmpty();
    bool isPendingOutputsEmpty();
    uint64_t getNumQueuedInputFrames();

 protected:
    virtual QC2Status onInit() = 0;
    virtual QC2Status onStart() = 0;
    virtual QC2Status onDeinit();
    virtual QC2Status onCodecConfig(std::shared_ptr<QC2Buffer> input);
    virtual QC2Status onFlush();
    void onConfigUpdate(std::list<std::shared_ptr<C2Param>> updatedConfig);
    void setHwSessionId(uint64_t* thizHwSession);
    uint64_t* getHwSessionId() { return mHwSessionId; }
    QC2Status returnPendingInputs();
    void clearUnAckedOutputs();
    void logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix);
    bool isNonDataBuffer(const std::shared_ptr<QC2Buffer> input);
    virtual QC2Status prepareAndQueueInputBuffer(const std::shared_ptr<InputBufferPack>& input);
    virtual QC2Status prepareAndQueueOutputBuffer(const std::shared_ptr<QC2Buffer>& output);
    QC2Status startWorkerThread();
    QC2Status stopWorkerThread();
    void createWorkerThread();

    friend QC2AudioHwCodecWorkerThread;
    struct InputPackMsg : Bundle::Storable {
        std::list<std::shared_ptr<InputBufferPack>> inputs;
    };
    // Updated C2 params sent back to the client
    std::list<std::shared_ptr<C2Param>> mUpdatedInfos;
    /*const*/ std::string deviceName;
    std::mutex mLock;
    std::mutex mAckLock;
    uint32_t mOutputTag = 0u;
    uint32_t mOutBufSize = DEFAULT_OUTPUT_BUFFER_SIZE;
    size_t mNumInputsQueued = 0;   //< Number of inputs queued to Codec in a given sequence
    size_t mNumOutputsQueued = 0;   //< Number of outputs queued to Codec in a given sequence
    int mNumExtraOutputBufs = 0;
    bool mUseAVTimerStamps = false;     // only for encoder
    uint64_t mOutputBufIndex = 0;
    std::unique_ptr<QC2AudioHw> mAudioHw;
    QC2AudioHwCommonCodecConfig mInputMediaFmt, mOutputMediaFmt;
    std::unordered_map<uint64_t, std::shared_ptr<QC2Buffer>> mPendingOutputs; // unordered_map<input index, output buffer>
    std::unique_ptr<QC2AudioHwCodecWorkerThread> mErrorHandlerThread;
    // Number of inputs queued to hardware that have not been ACKed
    std::atomic<uint64_t> mNumQueuedInputFrames = 0;

 protected:
    uint64_t* mHwSessionId;
    void signalInputsDone(std::shared_ptr<QC2AudioHwBuffer> inBuf);
    void signalOutputsDone(std::shared_ptr<QC2Buffer> outputBuffer);
    const std::string mInstName;
    std::shared_ptr<QC2LinearBufferPool> mLinearPool = nullptr;
    static ParamCapsHelperFactories mCapsFactories;
    bool mHwQueueFull = false;
    std::unique_ptr<QC2Monitor> mHwMonitor = nullptr;
    uint32_t mHwSuspendTimeoutValMs = QC2Monitor::kDefaultTimeOutMs;
    QC2Status checkHwMonitorSuspend();

    struct MsgInfoKey {
        static constexpr const char * kInputs = "inputs";
        static constexpr const char * kReconfig = "reconfig";
    };

    enum MsgId : uint32_t {
        MSG_ID_QUEUE = 0x1,
        MSG_ID_FLUSH = 0x2,
        MSG_ID_RECONFIG = 0x3,
        MSG_ID_DRAIN = 0x4,
        MSG_ID_ALLOC = 0x5,
        MSG_ID_SUSPEND = 0x6,
        MSG_ID_RESUME = 0x7
    };

    enum HwState : uint8_t {
        HW_DOWN = 0,
        HW_UP = 1
    };

    HwState mHwState = HW_DOWN;
    void setHwState(HwState state) {mHwState = state;}
    HwState getHwState() {return mHwState;}
    // Handler to handle messages posted to the EventQueue
    class Handler : public EventHandler {
     public:
        Handler(QC2AudioHwCodec& _parent) : mCodec(_parent) {}
        virtual ~Handler() = default;
     protected:
        QC2AudioHwCodec& mCodec;
        QC2Status onEvent(std::shared_ptr<Event> event) override {
            return mCodec.handleMessage(event);
        }
    };
    std::mutex mHandlerLock;
    std::shared_ptr<Handler> mHandler = nullptr;
    QC2Status handleMessage(std::shared_ptr<Event> event);

    std::shared_ptr<EventQueue> mEventQueue = nullptr;
    QC2Status startEventQueue();
    QC2Status stopEventQueue();

    volatile bool mFlushing = false;
    volatile bool mStopping = false;
    volatile bool mSuspended = false;
    std::shared_ptr<InputPackMsg> mPendingInputs = nullptr;
    // to safeguard mPendingInputs.input list changes
    std::mutex mPendingInputsMutex;
    // to safeguard mPendingOutputs map changes
    std::mutex mPendingOutputsMutex;
    volatile int32_t mOutputDeficit = 0;
    bool mForceFlush = false;

    // handler's called in eventQueue's context
    QC2Status handleQueueInputs();
    QC2Status handleFlush();
    QC2Status handleDrain();
    QC2Status handleSuspend();
    QC2Status handleResume();
    QC2Status handleStart();
    QC2Status handleStop();
    virtual QC2Status handleReconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
        (void)targetConfig;
        return QC2_OK;
    }
    QC2Status handleNonDataBuffer(std::shared_ptr<QC2Buffer> input);
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_CODEC_H_

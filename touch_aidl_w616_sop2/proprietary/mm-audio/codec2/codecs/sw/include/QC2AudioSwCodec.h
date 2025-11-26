/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_CODEC_H_
#define _QC2_AUDIO_SW_CODEC_H_

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

namespace qc2audio {

class QC2AudioSwDecoderFactory: public QC2CodecFactory {

public:
    virtual ~QC2AudioSwDecoderFactory() = default;
    QC2AudioSwDecoderFactory(const std::string& name, const std::string& mime,
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

class QC2AudioSwEncoderFactory: public QC2CodecFactory {

public:
    virtual ~QC2AudioSwEncoderFactory() = default;
    QC2AudioSwEncoderFactory(const std::string& name, const std::string& mime,
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

class QC2AudioSwCodec : public QC2Codec {

public:
    QC2AudioSwCodec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& inputMime,
            const std::string& outputMime,
            ComponentKind kind,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioSwCodec() = default;

    QC2Status getCapsHelpers(
            std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers
            ) const override;

    virtual QC2Status configure(const C2Param& param);

    virtual QC2Status configure(std::vector<std::unique_ptr<C2Param>> config);

    QC2Status queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const override;

    virtual QC2Status query(C2Param *const param);

    QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) override;

    QC2Status deinit() override;

    QC2Status start() override;

    QC2Status stop() override;

    QC2Status queueInputs(
            const std::list<std::shared_ptr<InputBufferPack>>& inputs,
            ReportConfigFailure_cb configFailureReporter,
            uint32_t *numInputsQueued) override;

    QC2Status flush() override;

    QC2Status drain() override;

    QC2Status suspend() override;

    QC2Status resume() override;

    QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) override;

    QC2Status release() override;

    void onError(QC2Status errorCode, const std::string& errorMsg);

protected:
    /* Start: Functions to be implemented by each codec */

    // Actions needed to start the codec
    virtual QC2Status onStart() = 0;

    // Actions needed to stop the codec
    virtual QC2Status onStop() = 0;

    // Codec initializations
    virtual QC2Status onInit() = 0;

    // Process queued input to generate an output (decode/encode)
    virtual QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                              std::shared_ptr<QC2Buffer>& output) = 0;

    // Optional, codec can implement if needed
    virtual QC2Status onFlush();

    /* End: Functions to be implemented by each codec */

    const std::string mInstName;
    std::shared_ptr<QC2LinearBufferPool> mLinearPool = nullptr;
    static ParamCapsHelperFactories mCapsFactories;
    uint32_t mMinOutBufSize;
    volatile bool mFlushing = false;
    volatile bool mStopping = false;
    // Subscribed C2 param updates sent back to the client
    std::list<std::shared_ptr<C2Param>> mUpdatedInfos;

    struct InputPackMsg : Bundle::Storable {
        std::list<std::shared_ptr<InputBufferPack>> inputs;
    };

    struct MsgInfoKey {
        static constexpr const char * kInputs = "inputs";
    };

    enum MsgId : uint32_t {
        MSG_ID_PROCESS = 0x1,
        MSG_ID_FLUSH = 0x2,
        MSG_ID_DRAIN = 0x3,
        MSG_ID_STOP = 0x4,
    };

     // Handler to handle messages posted to the EventQueue
    class Handler : public EventHandler {
     public:
        Handler(QC2AudioSwCodec& _parent) : mCodec(_parent) {}
        virtual ~Handler() = default;
     protected:
        QC2AudioSwCodec& mCodec;
        QC2Status onEvent(std::shared_ptr<Event> event) override {
            return mCodec.handleMessage(event);
        }
    };
    std::mutex mHandlerLock;
    std::shared_ptr<Handler> mHandler = nullptr;

    void onInputDone(std::shared_ptr<QC2Buffer> inBuf);
    void onOutputDone(std::shared_ptr<QC2Buffer> outBuf);
    // Updates configuration on the C2 interface
    void onConfigUpdate(std::list<std::shared_ptr<C2Param>> params);
    QC2Status produceOutput(std::shared_ptr<InputBufferPack>& inPack,
                            std::shared_ptr<QC2Buffer> outBuf);
    void logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix);
    QC2Status handleMessage(std::shared_ptr<Event> event);

    std::shared_ptr<EventQueue> mEventQueue = nullptr;
    QC2Status startEventQueue();
    QC2Status stopEventQueue();

private:
    DECLARE_NON_COPYASSIGNABLE(QC2AudioSwCodec);

};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_CODEC_H_

/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_MOCK_CODEC_H_
#define _QC2AUDIO_MOCK_CODEC_H_

#ifdef HAVE_MOCK_CODEC

#include "QC2.h"
#include "codecs/include/QC2Codec.h"
#include "utils/include/QC2SyncQueue.h"
#include <unordered_map>

namespace qc2audio {

struct ModelData;
struct ModelOutput;
class QC2MockCodec : public QC2Codec {
public:

    QC2MockCodec(
            const std::string& name,
            const std::string& mime,
            ComponentKind kind,
            bool canBeSecure,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2MockCodec();

    virtual QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) override;

    virtual QC2Status getCapsHelpers(
            std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers
            ) const override;

    virtual QC2Status configure(const C2Param& param) override;

    virtual QC2Status query(C2Param *const param) override;

    virtual QC2Status start() override;

    virtual QC2Status stop() override;

    virtual QC2Status queueInputs(
            const std::list<std::shared_ptr<InputBufferPack>>& inputs,
            ReportConfigFailure_cb configFailureReporter,
            uint32_t *numInputsQueued) override;

    virtual QC2Status flush() override;

    virtual QC2Status drain() override;

    virtual QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) override;

    virtual QC2Status release() override;

    static QC2Status threadWrapper(QC2MockCodec&);

    class Tester;

private:
    DECLARE_NON_COPYASSIGNABLE(QC2MockCodec);

    std::string mName;  // name of the model
    std::unique_ptr<std::thread> mThread;
    std::mutex mLock;
    std::atomic_bool mStopRequest;
    ComponentKind mKind;        // deliberate copy !! since registry does not list mock entries

    std::atomic_bool mStalled;
    std::condition_variable mStalledCondition;

    QC2SyncQueue<std::shared_ptr<InputBufferPack>> mInQ;
    QC2SyncQueue<std::shared_ptr<QC2Buffer>> mOutQ;

    std::unique_ptr<QC2GraphicBufferPool> mGraphicPool;
    std::unique_ptr<QC2LinearBufferPool> mLinearPool;

    // processing state (config)
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mPixFmt;
    uint32_t mLatency;
    std::list<std::shared_ptr<QC2Buffer>> mPipeline;
    uint32_t mOutputCounter;

    bool mCSDSent;

    // implementation
    std::unordered_map<std::string, std::shared_ptr<ModelData>> mModelStore;
    std::shared_ptr<ModelData> mModel;
    void loadModelData();

    void ProcessorLoop();

    QC2Status allocateAndQueueOutputs();
    QC2Status produceOutput(const InputBufferPack& inPack, std::shared_ptr<QC2Buffer> outBuf);
    void dispatchOutput(bool purge = false);
    void dispatchInput(bool purge = false);
    void dispatchReconfig(uint32_t w, uint32_t h, uint32_t f);
    void reportError(QC2Status errorCode, const std::string& errorMsg);
};

}; // namespace qc2audio

#endif // HAVE_MOCK_CODEC

#endif // _QC2AUDIO_MOCK_CODEC_H_

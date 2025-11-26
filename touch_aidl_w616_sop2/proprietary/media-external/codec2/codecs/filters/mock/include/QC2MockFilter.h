/*
 **************************************************************************************************
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_MOCK_CODEC_H_
#define _QC2_MOCK_CODEC_H_

#include "QC2.h"
#include "QC2Codec.h"
#include "QC2SyncQueue.h"
#include <unordered_map>


namespace qc2 {
enum QC2MockFilterVendorExtensionParamIndexKind : C2Param::type_index_t;
}

C2ENUM(qc2::QC2MockFilterVendorExtensionParamIndexKind, C2Param::type_index_t,
    kParamIndexVideoMockSizeMultiplier = C2Param::TYPE_INDEX_VENDOR_START + 2000,
    kParamIndexVideoMockFrameMultiplier,
    kParamIndexVideoMockColorEnhancer,
    kParamIndexVideoMockFilterEnabled
);



namespace qc2 {
// key: "qti-ext-mock-resize"
//      field -> multiplier : uint32_t
/* struct C2VideoMockSizeMultiplierStruct {
    uint32_t multiplier;

    DEFINE_AND_DESCRIBE_BASE_C2STRUCT(VideoMockSizeMultiplier)
    C2FIELD(multiplier, "multiplier")
};*/

typedef C2StreamParam<C2Tuning, C2Uint32Value, kParamIndexVideoMockSizeMultiplier>
 C2VideoMockSizeMultiplier;
constexpr char QC2_PARAMKEY_MOCK_RESIZE[] = "qti-ext-mock-resize";


// key: "qti-ext-mock-frc"
//      field -> multiplier : uint32_t
/*struct C2VideoMockFrameMultiplierStruct {
    uint32_t multiplier;

    DEFINE_AND_DESCRIBE_BASE_C2STRUCT(VideoMockFrameMultiplier)
    C2FIELD(multiplier, "multiplier")
};*/

typedef C2StreamParam<C2Tuning, C2Uint32Value, kParamIndexVideoMockFrameMultiplier>
   C2VideoMockFrameMultiplier;
constexpr char QC2_PARAMKEY_MOCK_FRC[] = "qti-ext-mock-frc";

typedef C2StreamParam<C2Tuning, C2BoolValue, kParamIndexVideoMockColorEnhancer>
   C2VideoMockColorEnhance;
constexpr char QC2_PARAMKEY_MOCK_COLORENH[] = "qti-ext-mock-colorenhance";

typedef C2GlobalParam<C2Tuning, C2BoolValue, kParamIndexVideoMockFilterEnabled>
   C2VideoMockFilterEnabled;
constexpr char QC2_PARAMKEY_MOCK_ENABLED[] = "qti-ext-mock-enabled";

// ordered maps of <sequenceId, Tunings>
using TuningsCollection =
    std::map<uint64_t /* sequence index */,
             std::list<std::shared_ptr<C2Param>>>;

struct ModelData;
struct ModelOutput;
class QC2MockFilter : public QC2Codec {
 public:
    QC2MockFilter(
            const std::string& name,
            const std::string& mime,
            ComponentKind kind,
            bool canBeSecure,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2MockFilter();

//    virtual std::shared_ptr<QC2MetadataTranslator> getMetadataTranslator() override;

    QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) override;

    QC2Status getCapsHelpers(
            std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers)
            const override;

    QC2Status configure(const C2Param& param) override;

    QC2Status queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const override;

    QC2Status query(C2Param *const param) override;

    QC2Status start() override;

    QC2Status stop() override;

    QC2Status queueInputs(
            const std::list<std::shared_ptr<InputBufferPack>>& inputs,
            ReportConfigFailure_cb configFailureReporter,
            uint32_t *numInputsQueued) override;

    QC2Status flush() override;

    QC2Status drain() override;

    QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) override;

    QC2Status release() override;

    static QC2Status threadWrapper(QC2MockFilter&);

    class Tester;

    uint32_t getFRCMultiplier() const {
        return mFRCMultiplier;
    }

    uint32_t getResizeMultiplier() const {
        return mUpscaleMultiplier;
    }

    void setStartFail() {
        mFailStart = true;
    }

    void setInitFail() {
        mFailInit = true;
    }

    uint32_t getNumInputsQueued() {
        return mNumInputsQueued;
    }

    TuningsCollection& getAllTunings() {
        return mAllTunings;
    }

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2MockFilter);

    std::string mName;  // name of the model
    std::unique_ptr<std::thread> mThread;
    std::mutex mLock;
    std::atomic_bool mStopRequest;
    std::atomic_bool mInitDone;
    ComponentKind mKind;        // deliberate copy !! since registry does not list mock entries

    std::atomic_bool mStalled;
    std::condition_variable mStalledCondition;

    QC2SyncQueue<std::shared_ptr<InputBufferPack>> mInQ;
    QC2SyncQueue<std::shared_ptr<QC2Buffer>> mOutQ;

    std::unique_ptr<QC2GraphicBufferPool> mGraphicPool;

    // processing state (config)
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mPixFmt;
    uint32_t mLatency;
    std::list<std::shared_ptr<QC2Buffer>> mPipeline;
    uint32_t mOutputCounter;

    bool mColorEnhance = false;
    uint32_t mFRCMultiplier = 1;
    uint32_t mUpscaleMultiplier = 1;

    TuningsCollection mAllTunings;

    // implementation
    std::unordered_map<std::string, std::shared_ptr<ModelData>> mModelStore;
    std::shared_ptr<ModelData> mModel;

// test controls
    bool mFailInit = false;
    bool mFailStart =  false;
    uint32_t mNumInputsQueued = 0;

// test controls


    void loadModelData();

    void ProcessorLoop();

    QC2Status allocateAndQueueOutputs();
    QC2Status produceOutput(const InputBufferPack& inPack, std::shared_ptr<QC2Buffer> outBuf);
    void dispatchOutput(bool purge = false);
    void dispatchInput(bool purge = false);
    void dispatchReconfig(uint32_t w, uint32_t h, uint32_t f);
    void reportError(QC2Status errorCode, const std::string& errorMsg);
};

};  // namespace qc2

#endif  // _QC2_MOCK_CODEC_H_

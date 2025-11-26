/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#define LOG_TAG "MockCodec"
#include "codecs/mock/include/QC2MockCodec.h"

#ifdef HAVE_MOCK_CODEC

namespace qc2audio {

//-------------------------------------------------------------------------------------------------
// Models for output behavior for the mock
//-------------------------------------------------------------------------------------------------
struct ModelOutput {

    struct Graphic {
        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mFormat;
    };

    struct Compressed {
        uint32_t mAllocLen;
        uint32_t mFilledLen;
        uint32_t mOffset;
        uint64_t flags;
    };

    ModelOutput(const Graphic& g, bool inputPending, int32_t repeat)
    : mIsGraphic(true), mGraphic(g), mCompressed({0,0,0,0}), mInputPending(inputPending),
      mTimeStamp(0), mFrameIndex(0), mRepeat(repeat) {
    }

    ModelOutput(const Compressed& c, bool inputPending)
    : mIsGraphic(false), mGraphic({0,0,0}), mCompressed(c), mInputPending(inputPending),
      mTimeStamp(0), mFrameIndex(0), mRepeat(0) {
    }

    const bool mIsGraphic;
    Graphic mGraphic;
    Compressed mCompressed;

    bool mInputPending;     // indicates that input corresponding to this output will generate more
                            // outputs
    uint64_t mTimeStamp;    // propagated from input
    uint64_t mFrameIndex;   // propagated from input
    int32_t mRepeat;        // number of times to repeat this output. -1 => forever
};

struct CompressedFrame {
    uint32_t mLen;
    uint64_t mFlags;
    const uint8_t *mData;
};

struct BitStream {
    uint32_t mWidth;
    uint32_t mHeight;
    std::vector<CompressedFrame> mFrames;
    uint32_t mFrameIdx;
};

struct ModelData {
    ModelData(std::initializer_list<ModelOutput> l) {
        for (auto& i : l) {
            mOutputs.push_back(i);
        }
    }
    ModelData(const std::list<BitStream>& b) {
        mStreams = b;
    }
    // list of output behavior
    std::list<ModelOutput> mOutputs;
    std::list<BitStream> mStreams;
    BitStream mStream;

    int32_t mFailsOutputAllocationsAfter;
    int32_t mConfigErrorAfter;
    bool mFailsStart;
    bool mFailsInit;

    void selectStream(uint32_t w, uint32_t h) {
        for (auto& s : mStreams) {
            if (s.mWidth == w && s.mHeight == h) {
                mStream = s;
                mStream.mFrameIdx = 0;
                QLOGD("Found stream matching size %u x %u", w, h);
                return;
            }
        }
        QLOGD("No stream found matching size %u x %u", w, h);
        mStream.mFrames = {};
    }
};

//-------------------------------------------------------------------------------------------------
// Mock implementation
//-------------------------------------------------------------------------------------------------
QC2MockCodec::QC2MockCodec(
        const std::string& name,
        const std::string& mime,
        ComponentKind kind,
        bool canBeSecure,
        std::shared_ptr<EventHandler> listener)
    : QC2Codec(name, mime, kind, canBeSecure, listener),
      mName(name),
      mThread(nullptr) {
    mWidth = 1280;
    mHeight = 720;
    mPixFmt = PixFormat::VENUS_NV12;
    mLatency = 2;
    mOutputCounter = 0;
    mCSDSent = false;

    mStopRequest.store(false);

    loadModelData();

    mModel = nullptr;
    for (auto& model : mModelStore) {
        if (!strncmp(mName.c_str(), model.first.c_str(), model.first.size())) {
            mModel = model.second;
        }
    }
    if (mModel == nullptr) {
        QLOGE("Could not find model %s", mName.c_str());
        if (mKind == KIND_DECODER) {
            mModel = mModelStore["default"];
            QLOGI("Using default model..");
        }
    }

    mKind = KIND_DECODER;
    if (mName.find("encoder") != std::string::npos) {
        mWidth = 1280;
        mHeight = 720;
        mKind = KIND_ENCODER;
    }
    QLOGI("QC2MockCodec() : %s : kind=%s", mName.c_str(), mKind == KIND_DECODER ? "DEC" : "ENC");
}

QC2MockCodec::~QC2MockCodec() {
    if (mThread) {
        stop();
    }
}

QC2Status QC2MockCodec::init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) {
    mBufPoolProvider = std::move(bufPoolProvider);
    QLOGI("init()");
    return QC2_OK;
}

QC2Status QC2MockCodec::getCapsHelpers(
        std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers) const {
    if (helpers) {
        // return dummy caps helpers
    } else {
        return QC2_BAD_ARG;
    }
    return QC2_OK;
}

QC2Status QC2MockCodec::configure(const C2Param& param) {
    QLOGI("configure: core-index=%x baseIndex=%x", param.coreIndex().coreIndex(), param.index());
    switch(param.coreIndex().typeIndex()) {
    case kParamIndexPictureSize:
    {
        C2StreamPictureSizeInfo::output o;
        if (param.index() == o.index()) {
            C2StreamPictureSizeInfo::output *outSize = (C2StreamPictureSizeInfo::output*)&param;
            QLOGI("Configure o/p resolution = %u x %u", outSize->width, outSize->height);
            if (mKind == KIND_DECODER && mGraphicPool) {
                mGraphicPool->setResolution(outSize->width, outSize->height);
            }
            mWidth = outSize->width;
            mHeight = outSize->height;
        }
        break;
    }
    case kParamIndexPixelFormat:
    {
        C2StreamPixelFormatInfo::input inFmt;
        if (param.index() == inFmt.index()) {
            mPixFmt = ((C2StreamPixelFormatInfo::input*)(&param))->value;
            QLOGI("Configuring input pixel format = %s", PixFormat::Str(mPixFmt));
        }
        break;
    }
    default:
        break;
    }

    return QC2_OK;
}

QC2Status QC2MockCodec::query(C2Param *const param) {
    (void)param;
    return QC2_OK;
}

QC2Status QC2MockCodec::start() {
    QLOGV("starting...");

    // Get handles for different buffer pools
    if (mBufPoolProvider == nullptr) {
        QLOGE("No Provider to request buffer-pools !");
        return QC2_NO_INIT;
    }
    mLinearPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (mLinearPool == nullptr) {
        QLOGE("Failed to create Linear buffer pool !");
        return QC2_ERROR;
    }

    if (mKind == KIND_DECODER) {
        mGraphicPool->setUsage(MemoryUsage::HW_CODEC_WRITE | MemoryUsage::CPU_WRITE);
        mGraphicPool->setResolution(mWidth, mHeight);
        mGraphicPool->setFormat(mPixFmt);
        QLOGI("Graphic pool initialized for %u x %u", mWidth, mHeight);
    } else if (mKind == KIND_ENCODER) {
        mLinearPool->setUsage(MemoryUsage::HW_CODEC_WRITE | MemoryUsage::CPU_WRITE);
        mLinearPool->setBufferSize(mWidth * mHeight);
        QLOGI("Linear pool initialized for %u size", mWidth * mHeight);

        mModel->selectStream(mWidth, mHeight);
        if (mModel->mStream.mFrames.empty()) {
            QLOGE("No stream matching %u x %u found !!", mWidth, mHeight);
            return QC2_ERROR;
        }
        QLOGD("Loaded stream for %u x %u", mWidth, mHeight);
    }

#if 0
    if (mFailStart) {
        QLOGI("Simulation: Forcing an error on start");
        std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
        mListener->postAsync(e);
        return QC2_ERROR;
    }
#endif

    mThread = std::make_unique<std::thread>(QC2MockCodec::threadWrapper, std::ref(*this));
    QLOGV("started...");

    return QC2_OK;
}

QC2Status QC2MockCodec::stop() {
    QLOGV("stopping..");
    {
        std::lock_guard<std::mutex> lock(mLock);
        mStopRequest.store(true);
        mInQ.interrupt();
        mOutQ.interrupt();
    }

    if (mStalled) {
        mStalledCondition.notify_one();
    }

    if (mThread && mThread->joinable()) {
        mThread->join();
    }

    std::lock_guard<std::mutex> lock(mLock);
    mThread = nullptr;
    QLOGV("stopped..");
    return QC2_OK;
}

QC2Status QC2MockCodec::queueInputs(
        const std::list<std::shared_ptr<InputBufferPack>>& inputs,
        ReportConfigFailure_cb configFailureReporter,
        uint32_t *numInputsQueued) {
    (void)configFailureReporter;

    if (!mModel) {
        QLOGE("queueInputs: Output model is not configured !");
        return QC2_NO_INIT;
    }

    if (numInputsQueued == nullptr) {
        QLOGE("queueInputs: Invalid arg");
        return QC2_ERROR;
    }
    *numInputsQueued = 0;
    for (auto& input : inputs) {
        if (!(input->mInput)) {
            QLOGE("queueInputs: Invalid input buffer");
            return QC2_ERROR;
        }
        if ((mKind == KIND_DECODER && input->mInput->isLinear()) ||
                (mKind == KIND_ENCODER && input->mInput->isGraphic())) {
            if (mKind == KIND_ENCODER) {
                auto& g = input->mInput->graphic();
                if (g.width() != mWidth || g.height() != mHeight) {
                    QLOGE("Stream dimensions(%ux%u) not same as configured(%ux%u)",
                            g.width(), g.height(), mWidth, mHeight);
                    return QC2_CANNOT_DO;
                }
                if (g.format() != mPixFmt) {
                    QLOGE("Stream color (%s) not same as configured (%s)",
                            PixFormat::Str(g.format()), PixFormat::Str(mPixFmt));
                    return QC2_CANNOT_DO;
                }
            }
            mInQ.enqueue(input);
        } else {
            QLOGE("queueInputs: incompatible input type!");
            return QC2_ERROR;
        }

        // TODO(PC) apply tunings/infos (assumed to be pre-sanitized by component/intf)
        //   return failure via configFailureReporter to simulate a failure to apply some tuning

        ++(*numInputsQueued);
    }

    // allocate and queue outputs
    return allocateAndQueueOutputs();
}

QC2Status QC2MockCodec::flush() {
    return QC2_OK;
}

QC2Status QC2MockCodec::drain() {
    QLOGD("drain()");
    std::shared_ptr<QC2Buffer> drainBuf;
    if (mKind == KIND_DECODER) {
        mGraphicPool->allocate(&drainBuf);
    } else  {
        mLinearPool->allocate(&drainBuf);
    }
    if (!drainBuf) {
        QLOGD("drain: Failed to allocate drain buffer");
        return QC2_ERROR;
    }
    drainBuf->bufferInfo().put("drain", (uint32_t)1);
    auto drainInput = std::make_shared<InputBufferPack>(drainBuf);

    mInQ.enqueue(drainInput);
    return QC2_OK;
}

QC2Status QC2MockCodec::reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    QC2Status ret = QC2_OK;
    // 1. apply configuration. This should set new resolution and also configure buffer-pool
    for (auto& param : targetConfig) {
        ret = configure(*param);
        if (ret != QC2_OK) {
            return ret;
        }
    }
    // 2. free previous generation outputs
    mOutQ.flush(nullptr);

    // 3. allocate and queue new-sized buffers
    ret = allocateAndQueueOutputs();

    // 4. signal processor to resume
    {
        std::unique_lock<std::mutex> lock(mLock);
        if (mStalled) {
            mStalledCondition.notify_one();
        }
    }
    return ret;
}

QC2Status QC2MockCodec::release() {
    return QC2_OK;
}

QC2Status QC2MockCodec::threadWrapper(QC2MockCodec& mock) {
    mock.ProcessorLoop();
    return QC2_OK;
}

void QC2MockCodec::ProcessorLoop() {
    QLOGV("ProcessorLoop: begin");
    while (!mStopRequest) {
        // inspect the first input
        std::shared_ptr<InputBufferPack> inPack = nullptr;
        if (mInQ.peek(&inPack) != QC2_OK || !inPack || !(inPack->mInput)) {
            usleep(10000);
            QLOGI("processorLoop...waiting for an input..");
            continue;
        }
        if (inPack->mInput->hasBufferInfo("drain")) {
            QLOGD("Draining %u outputs", mOutQ.size());
            dispatchOutput(true /*purge*/);

            std::shared_ptr<InputBufferPack> inPack = nullptr;
            mInQ.dequeue(&inPack);

            continue;
        }

        uint64_t bufFlags = inPack->mInput->flags();
        // consume the codec-config buffer without generating an output
        if (bufFlags & BufFlag::CODEC_CONFIG) {
            dispatchInput();
            continue;
        }

        // dequeue output
        std::shared_ptr<QC2Buffer> outBuf = nullptr;
        QLOGI("processorLoop...waiting for an output..");
        if (mOutQ.dequeue(&outBuf) != QC2_OK || outBuf == nullptr) {
            continue;
        }

        bool inputPending = false;
        if (mKind == KIND_DECODER) {
            // output compatible ?
            // if not, throw a reconfig-event and stall
            QLOGI("getting model");
            if (mModel->mOutputs.empty()) {
                QLOGE("No more model data !");
                return;
            }
            auto& outModel = mModel->mOutputs.front();

            if (!outModel.mIsGraphic) {
                QLOGE("FATAL: invalid buffer-type in output-model !!");
                return;
            }
            auto& g = outModel.mGraphic;
            if (g.mWidth != mWidth || g.mHeight != mHeight ||
                    (g.mFormat && (g.mFormat != mPixFmt))) {
                QLOGI("Port-settings changed %ux%u/%x -> %ux%u/%x",
                        mWidth, mHeight, mPixFmt, g.mWidth, g.mHeight, g.mFormat);
                std::unique_lock<std::mutex> lock(mLock);
                mStalled.store(true);
                dispatchReconfig(g.mWidth, g.mHeight, g.mFormat);
                mStalledCondition.wait(lock);   // will be woken up after handling reconfig
                // hopefully, reconfig has been handled (or was woken up due to a flush/stop)
                // What happens to the output-buffer dequeued ? it will be dropped here and it is OK
                mStalled.store(false);
                continue;
            }

            // generate output
            if (produceOutput(*inPack, outBuf) != QC2_OK) {
                return;
            }

            if (outModel.mRepeat == 0) {
                mModel->mOutputs.pop_front();
            } else if (outModel.mRepeat > 0) {
                --(outModel.mRepeat);
            }

            inputPending = outModel.mInputPending;
        } else if (mKind == KIND_ENCODER) {
            if (mModel->mStream.mFrames.empty()) {
                QLOGE("No stream to generate output !!");
                return;
            }
            // generate output
            if (produceOutput(*inPack, outBuf) != QC2_OK) {
                return;
            }
        }

        // if mInputPending (i.e input generates multiple outputs), do not dequeue & dispatch input
        if (!inputPending) {
            dispatchInput();
        }

        // signal output(s) done
        dispatchOutput(!!(bufFlags & BufFlag::EOS));
    }
    QLOGV("ProcessorLoop: end");
}

QC2Status QC2MockCodec::allocateAndQueueOutputs() {
    int minCount = (int)mInQ.size() + (int)mLatency + 1 - (int)mOutQ.size();
    for (int i = 0; i < minCount; ++i) {
        if (mKind == KIND_DECODER) {

            std::shared_ptr<QC2Buffer> buf;
            QC2Status ret = mGraphicPool->allocate(&buf);
            if (ret == QC2_OK && buf != nullptr && buf->isGraphic()) {
                QLOGV("Graphic output-buf allocated");
                mOutQ.enqueue(buf);
            } else {
                QLOGE("Graphic output-buf allocation failed!");
                return QC2_NO_MEMORY;
            }
        } else if (mKind == KIND_ENCODER) {

            std::shared_ptr<QC2Buffer> buf;
            QC2Status ret = mLinearPool->allocate(&buf);
            if (ret == QC2_OK && buf != nullptr && buf->isLinear() && buf->linear().valid()) {
                QLOGV("Linear output-buf allocated");
                mOutQ.enqueue(buf);
            } else {
                QLOGE("Linear output-buf allocation failed!");
                return QC2_NO_MEMORY;
            }
        }
    }
    return QC2_OK;
}

QC2Status QC2MockCodec::produceOutput(const InputBufferPack& inPack,
        std::shared_ptr<QC2Buffer> outBuf) {
    if (inPack.mInput == nullptr) {
        QLOGE("produceOutput: Invalid input !");
        return QC2_BAD_ARG;
    }
    if (!outBuf) {
        QLOGE("produceOutput: Invalid output !");
        return QC2_BAD_ARG;
    }

    // TODO(PC) apply tunings/infos (assumed to be pre-sanitized by component/intf)
    //     simulate failing tunings here (that will be equivalent to FW throwing an error)

    QC2Buffer& inBuf = *(inPack.mInput);
    uint64_t flags = inBuf.flags() & BufFlag::EOS;

    if (mKind == KIND_ENCODER) {
        do {
            uint32_t& idx = mModel->mStream.mFrameIdx;
            auto& frame = mModel->mStream.mFrames[idx];
            ++(idx);
            if (idx >= mModel->mStream.mFrames.size()) {
                idx = 0;
            }
            // check if dimensions match
            {
                auto& g = inBuf.graphic();
                if (g.width() != mWidth || g.height() != mHeight) {
                    QLOGE("Stream dimensions(%ux%u) not same as configured(%ux%u)",
                            g.width(), g.height(), mModel->mStream.mWidth, mModel->mStream.mHeight);
                    return QC2_CANNOT_DO;
                }
            }

            if (frame.mFlags & BufFlag::CODEC_CONFIG) {
                if (mCSDSent) {
                    continue;
                }
                std::shared_ptr<C2StreamInitDataInfo::output> csdInfo =
                        C2StreamInitDataInfo::output::AllocShared(frame.mLen, 0u);
                memcpy_s(csdInfo->m.value, frame.mLen, frame.mData, frame.mLen);
                outBuf->addInfo(csdInfo);
                QLOGD("Added CSD of %u bytes", frame.mLen);
                mCSDSent = true;
                continue;
            }
            auto m = outBuf->linear().map(/*RW*/);
            if (!m || !m->baseRW()) {
                QLOGE("produceOutput: failed to map buffer");
                return QC2_ERROR;
            }
            QLOGD("produceOutput: frame id=%u size=%u", idx, frame.mLen);
            memcpy_s(m->baseRW(), frame.mLen, frame.mData, frame.mLen);
            outBuf->linear().setRange(0, frame.mLen);

            if (frame.mFlags & BufFlag::I_FRAME) {
                flags |= BufFlag::I_FRAME;
            }
            break;
        } while (true);

    } else if (mKind == KIND_DECODER) {
        auto& outModel = mModel->mOutputs.front();
        flags |= (outModel.mInputPending) ? BufFlag::INPUT_PENDING : 0;
    }

    outBuf->setInputIndex(inBuf.inputIndex());
    outBuf->setTimestamp(inBuf.timestamp());
    outBuf->setFlags(flags);
    outBuf->setOutputIndex(mOutputCounter++);
    QLOGI("generated output for input=%u", (uint32_t)inBuf.inputIndex());
    mPipeline.push_back(outBuf);
    return QC2_OK;
}

void QC2MockCodec::dispatchOutput(bool purge) {
    size_t sz = mPipeline.size();
    QLOGI("dispatchOutput : %d . sz=%zu", purge, sz);
    int numToGo = purge ? (int)sz : (int)sz - (int)mLatency;
    QLOGI("dispatching %d outputs", numToGo);
    if (numToGo < 1) {
        return;
    }
    for (; numToGo > 0; --numToGo) {
        auto buf = mPipeline.front();
        mPipeline.pop_front();

        auto oDone = std::make_shared<OutputBuffersDoneInfo>();
        oDone->mOutputs = {buf};
        oDone->mOutputMetadatas.clear();

        auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE);
        e->info().put(QC2Codec::NotifyKey::kBufsDone, oDone);

        QLOGI("Output done @ %u", (int)(buf->timestamp()));
        mListener->postAsync(e);
    }
}

void QC2MockCodec::dispatchInput(bool purge) {
    uint32_t n = mInQ.size();
    QLOGI("dispatching %u inputs", n);
    if (n < 1) {
        return;
    }
    n = purge ? n : 1;
    for (uint32_t i = 0; i < n; ++i) {
        std::shared_ptr<InputBufferPack> inPack = nullptr;
        mInQ.dequeue(&inPack);
        if (!inPack) {
            QLOGE("bug! : failed to dequeue input buffer");
            return;
        }

        auto iDone = std::make_shared<InputBuffersDoneInfo>();
        iDone->mInputFrameIds = {inPack->mInput->inputIndex()};

        auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
        e->info().put(QC2Codec::NotifyKey::kBufsDone, iDone);

        QLOGI("Returning Input @ %u", (int)inPack->mInput->timestamp());
        mListener->postAsync(e);
    }
}

void QC2MockCodec::dispatchReconfig(uint32_t w, uint32_t h, uint32_t f __unused) {
    // ReconfigParamsInfo kReconfig CODEC_NOTIFY_RECONFIG
    auto sizeParam = std::make_unique<C2StreamPictureSizeInfo::output>();
    sizeParam->width = w;
    sizeParam->height = h;

    auto reconfInfo = std::make_shared<ReconfigParamsInfo>();
    reconfInfo->mTargetConfig.push_back(std::move(sizeParam));

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_RECONFIG);
    e->info().put(QC2Codec::NotifyKey::kReconfig, reconfInfo);

    QLOGI("Posting reconfig event");
    mListener->postAsync(e);
}

void QC2MockCodec::reportError(QC2Status errorCode, const std::string& errorMsg) {
    std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
    e->info().put(QC2Codec::NotifyKey::kErrorCode, errorCode);
    e->info().put(QC2Codec::NotifyKey::kErrorMsg, errorMsg);
    mListener->postAsync(e);
}

//-------------------------------------------------------------------------------------------------
// 96x96 bitstream
//-------------------------------------------------------------------------------------------------
//static uint64_t stream96x96__ts_001 = 0;
static uint64_t stream96x96__flags_001 = 1;
static uint8_t stream96x96__blob_001[] = {
    "\x00\x00\x00\x01\x67\x64\x00\x0a\xac\xd9\x46\x36\xc0\x44\x00\x00\x03\x00\x04\x00\x00"
    "\x03\x00\xc8\x3c\x48\x96\x58"
    "\x00\x00\x00\x01\x68\xeb\xe1\xb2\xc8\xb0"
};

static uint64_t stream96x96__flags_002 = 4;
static uint8_t stream96x96__blob_002[] = {
    "\x00\x00\x00\x01\x65\x88\x84\x00\x23\xff\xfe"
    "\xdb\x5b\xf3\x2c\x5b\xe5\xab\x27\xb7\x1b\xf0\x87\xaf\xd7\x0e\xac\xbd\xdc\x2a\x4d"
    "\x26\xfb\x80\x00\x81\x50\xf3\x2a\x14\x74\xe6\x7f\x4d\x85\x53\x60\x07\x78\xd0\x81"
    ""
};

//static uint64_t stream96x96__ts_003 = 66666;
static uint64_t stream96x96__flags_003 = 0;
static uint8_t stream96x96__blob_003[] = {
    "\x00\x00\x00\x01\x41\x9a\x21\x63\x88\x81\xf8\x05\x85\x00\xfe\x40\x3f\x90\xaf\xfe\xa7"
    "\x84\x0d\x78"
};

//static uint64_t stream96x96__ts_004 = 99999;
static uint64_t stream96x96__flags_004 = 0;
static uint8_t stream96x96__blob_004[] = {
    "\x00\x00\x00\x01\x41\x9a\x42\x49\xe1\x08\x43\x21\xcf\x03\xf0\x0d\xd8\x1f\x81\x58\x1f"
    "\x80\xdb\x03\xf0\x0d\xf0\x21\x5f\xfe\xa7\x84\x0d\x79"
};

//static uint64_t stream96x96__ts_005 = 13333;
static uint64_t stream96x96__flags_005 = 0;
static uint8_t stream96x96__blob_005[] = {
    "\x00\x00\x00\x01\x41\x88\x98\xc0\xaf\xfe\xe1\x03\xf8\x14\xca\x4b\xe6\x2e\x89\xf5\x08"
    "\x23\x25\x72\xe4\x6d\xc4\xa5\xd3\x36\x68\x77\xe0\x0c\xd7\x62\xf4\x52\xe5\x68\x2d"
    "\x2a\xd6\x5d\xfb\xcf\x50\x04\x24\x76\x40"
};

//static uint64_t stream96x96__ts_006 = 166666;
static uint64_t stream96x96__flags_006 = 0;
static uint8_t stream96x96__blob_006[] = {
    "\x00\x00\x00\x01\x41\x9a\x84\x4b\xe1\x08\x43\xc8\x73\xc0\xfc\x0d\x20\x48\x01\xfc\x06"
    "\xa0\x08\x0c\x0f\xc0\xc8\x02\x15\xff\x8a\xa7\x7a\xe9\x67\x8f\x36\x86\xdc\x98\x45"
    "\xff\x01\x4b\x72\x22\xf6\x0f\xaf\x68\x8b\xd9\xda\x86\xe0\x16\x20\x2a\x11\x91"
};

//-------------------------------------------------------------------------------------------------
// 1280x720 bitstream
//-------------------------------------------------------------------------------------------------
//static uint64_t stream1280x720__ts_001 = 33333;
static uint64_t stream1280x720__flags_001 = 1;
static uint8_t stream1280x720__blob_001[] = {
    "\x00\x00\x00\x01\x67\x64\x00\x1f\xac\xd9\x40\x50\x05\xbb\x01\x10\x00\x00\x03\x00\x10"
    "\x00\x00\x03\x03\xc0\xf1\x83\x19\x60"
    "\x00\x00\x00\x01\x68\xeb\xe3\xcb\x22\xc0"
};

//static uint64_t stream1280x720__ts_002 = 99999;
static uint64_t stream1280x720__flags_002 = 4;
static uint8_t stream1280x720__blob_002[] = {
    "\x00\x00\x00\x01\x65\x88\x84\x00\x7f\xfe\xc2"
    "\x7f\xe6\x58\x2b\xe3\x57\x50\x9b\x7d\x94\xa5\x90\xec\x29\x31\xff\x3c\x90\x00\xdb"
    "\x61\xa5\x38\xc8\xaa\xda\xc0\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x06\x2a"
    "\x93\xef\xc0\xb5\x82\x8c\x6b\xeb\x3f\x20\x00\x00\x03\x00\x00\x0e\xc8\x00\x0e\x80"
    "\x00\x1b\x40\x00\x4e\x40\x01\x1f\x00\x03\xb8\x00\x10\x80\x00\x59\x00\x02\x38\x00"
    "\x0b\xe0\x00\x5e\x00\x02\x20\x00\x11\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x40\x41"
};

//static uint64_t stream1280x720__ts_003 = 133332;
static uint64_t stream1280x720__flags_003 = 0;
static uint8_t stream1280x720__blob_003[] = {
    "\x00\x00\x00\x01\x41\x9a\x21\x63\x88\x81\xf8\x05\x85\x00\xfe\x40\x3f\x90\x9f\xfd\xf1"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x06\xf4"
};

//static uint64_t stream1280x720__ts_004 = 166665;
static uint64_t stream1280x720__flags_004 = 0;
static uint8_t stream1280x720__blob_004[] = {
    "\x00\x00\x00\x01\x41\x9a\x42\x49\xe1\x08\x43\x21\xcf\x03\xf0\x0d\xd8\x1f\x81\x58\x1f"
    "\x80\xdb\x03\xf0\x0d\xf0\x21\x3f\xfd\xf1\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x06\xf5"
};

//static uint64_t stream1280x720__ts_005 = 199998;
static uint64_t stream1280x720__flags_005 = 0;
static uint8_t stream1280x720__blob_005[] = {
    "\x00\x00\x00\x01\x41\x88\x98\xc0\xaf\xfe\xd8\xe7\xf3\x2c\x78\x4c\x6a\xd3\x49\xd3\x58"
    "\x1e\x3a\xa5\xc3\xab\x2f\x77\x0a\x94\x0d\x19\x43\x3b\x4f\x25\xea\x66\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x0c\x0d\x27\xde\x01\x2c\x70\x86\xfe\x33"
    "\xe2\xa0\x00\x00\x03\x00\x00\x17\x10\x00\x19\x10\x00\x28\x60\x00\x5a\xc0\x01\x1f"
    "\x00\x03\xb8\x00\x10\x80\x00\x59\x00\x02\x38\x00\x0b\xe0\x00\x5e\x00\x02\x20\x00"
    "\x11\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x40\x40"
};

//static uint64_t stream1280x720__ts_006 = 233331;
static uint64_t stream1280x720__flags_006 = 0;
static uint8_t stream1280x720__blob_006[] = {
    "\x00\x00\x00\x01\x41\x9a\x85\x4b\xe1\x08\x43\xc8\x73\xc0\xfc\x0d\x20\x48\x01\xfc\x06"
    "\xa0\x08\x0c\x0f\xc0\xc8\x05\x11\x3c\x27\xff\x7a\xc2\x95\x64\x05\x70\xcf\x46\x6c"
    "\xb9\x1c\x67\x65\x5b\x74\x35\x8e\xaa\xc5\x7b\x2c\x4f\xcc\xc5\xba\x3a\x86\x46\x33"
    "\xa3\xa4\xb4\x13\x59\x7b\x8c\x12\x27\x84\x98\x49\x85\x68\x2b\x01\x2d\x63\xc6\x18"
    "\x0d\x26\x00\x00\x03\x00\x08\x48\x00\x01\x4a\x00\x00\x43\x80\x00\x0d\x00\x00\x03"
    "\x02\x98\x00\x00\xbb\x00\x00\x2f\x80\x00\x0e\xe0\x00\x04\xb0\x00\x01\x6a\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03"
    "\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00"
    "\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x1a\x11"
};

//static uint64_t stream1280x720__ts_007 = 266664;
static uint64_t stream1280x720__flags_007 = 2;
static uint8_t stream1280x720__blob_007[] = {
    "\x00\x00\x00\x01\x01\x9e\xa4\x6a\x46\xff\xf8\x6a\x13\xac\x00\x00\x03\x00\x00\x03\x00"
    "\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x00\x03\x00\x04\x5d"
};

#define _COMPR(stream, id) CompressedFrame{   \
        .mLen = sizeof(stream##__blob_##id),  \
        .mFlags = stream##__flags_##id,        \
        .mData = stream##__blob_##id}        \

void QC2MockCodec::loadModelData() {

    std::shared_ptr<ModelData> model;

    model = std::shared_ptr<ModelData>(new ModelData({
        ModelOutput(ModelOutput::Graphic({640, 480, 0}), false, -1),
    }));
    mModelStore.emplace("mockavcD", model);
    mModelStore.emplace("default", model);

    {
        BitStream stream96x96;
        stream96x96.mWidth = 96;
        stream96x96.mHeight = 96;
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 001));
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 002));
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 003));
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 004));
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 005));
        stream96x96.mFrames.emplace_back(_COMPR(stream96x96, 006));

        BitStream stream1280x720;
        stream1280x720.mWidth = 1280;
        stream1280x720.mHeight = 720;
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 001));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 002));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 003));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 004));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 005));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 006));
        stream1280x720.mFrames.emplace_back(_COMPR(stream1280x720, 007));

        model = std::shared_ptr<ModelData>(new ModelData(
                {stream96x96, stream1280x720}));
        mModelStore.emplace("mockavcE", model);
    }
}

}  // namespace qc2audio

#endif // HAVE_MOCK_CODEC

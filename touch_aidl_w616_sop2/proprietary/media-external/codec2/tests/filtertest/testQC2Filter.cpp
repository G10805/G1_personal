/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "QC2FilterTest"

#include <gtest/gtest.h>

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <dlfcn.h>

#include <C2PlatformSupport.h>

#include "QC2Codec.h"
#include "QC2CodecRegistry.h"
#include "QC2PipelineCodec.h"
#include "QC2OS.h"
#include "QC2Buffer.h"
#include "QC2Component.h"
#include "QC2ComponentStore.h"
#include "QC2Platform.h"
#include "QC2TestInputStream.h"
#include "QC2CodecPlugin.h"
#include "QC2V4L2Config.h"
#include "QC2MockFilter.h"

using namespace std::chrono_literals;
using namespace android;
using namespace qc2;

constexpr unsigned int kNumInstances = 1;

#define _RETURN_IF(condition, ret)       \
    if (condition) {                     \
        QLOGE("FAILED: %s", #condition); \
        return ret;                      \
    }                                    \

class ArgsParser {
public:
    static std::string qc2filter_lib;
    static std::string qc2filter_name;

    static void parse(int argc, char **argv);

private:
    ArgsParser(int argc, char **argv);
    void extractArgValue(std::string option, std::string &value);

    int mArgc;
    char **mArgv;
};

std::string ArgsParser::qc2filter_lib{"libqc2colorconvertfilter.so"};
std::string ArgsParser::qc2filter_name{"c2.qti.colorconvert.filter"};

void ArgsParser::parse(int argc, char **argv) {
    ArgsParser parser(argc, argv);
    parser.extractArgValue("qc2filter_lib", qc2filter_lib);
    parser.extractArgValue("qc2filter_name", qc2filter_name);
    QLOGV("cmd arg [%s] = [%s]", "qc2filter_lib", qc2filter_lib.c_str());
    QLOGV("cmd arg [%s] = [%s]", "qc2filter_name", qc2filter_name.c_str());
}

ArgsParser::ArgsParser(int argc, char **argv)
    :mArgc(argc),
    mArgv(argv) {
}

void ArgsParser::extractArgValue(std::string option, std::string &value) {
    std::string optionPrefix{ "--" + option + "=" };
    for (int idx = 0; idx < mArgc; ++idx) {
        std::string arg{ mArgv[idx] };
        if (arg.size() >= optionPrefix.size() && arg.substr(0, optionPrefix.size()) == optionPrefix) {
            value = arg.substr(optionPrefix.size(), arg.size() - optionPrefix.size());
            if (value.size() == 0) {
                QLOGE("value of option %s is empty", option.c_str());
            }
            // ASSERT_NE(value.size(), 0);
            break;
        }
    }
}

struct QC2FilterTestRegistryTester: public QC2CodecRegistry {
    static QC2Status CreateQC2Codec(const std::string& name,
                                    const std::string& upstreamMime,
                                    const std::string& downstreamMime,
                                    std::shared_ptr<EventHandler> listener,
                                    std::unique_ptr<QC2Codec> * const codec) {
        return QC2CodecRegistry::CreateCodec(name, 1, upstreamMime, downstreamMime, listener, codec);
    }
};

class QC2FilterTest: public ::testing::Test {
public:
    enum : uint32_t {
        QC2FilterTest_EVENT_FLUSH_DONE = 0x1000,
    };

    QC2Status onEvent(std::shared_ptr<Event> event);

protected:
    std::shared_ptr<EventQueue> mQueue;
    std::shared_ptr<QC2Codec> mCodec;
    std::vector<std::shared_ptr<QC2MockFilter>> mFilters;
    std::shared_ptr<QC2Codec> mDecoder;
    std::shared_ptr<EventHandler> mHandler;
    std::unordered_map<std::string, std::unique_ptr<QC2CodecFactory>> mFactories;
    void* mLib = nullptr;

    std::shared_ptr<C2ComponentStore> mStore;
    std::shared_ptr<QC2Component> mComp;     // dummy component required to request block-pools

    std::shared_ptr<QC2LinearBufferPool> mLinearPool;   // to allocate inputs to decoder
    std::shared_ptr<QC2GraphicBufferPool> mGraphicPool; // to allocate inputs to encoder
    std::unordered_map<uint32_t, std::list<std::unique_ptr<C2Param>>> mFrameTunings;

    uint64_t mCurrentInputIndex;
    uint64_t mLastQueuedInputIndex;

    uint32_t mNumQueuedInputs = 0;

    std::mutex mLock;

    // info of produced-output
    struct OutputDescr {
        uint64_t mInputId;
        uint64_t mTs;
        uint64_t mFlags;
        std::weak_ptr<QC2Buffer> mBufRef;
    };

    //TODO(PC) : check graphic-buffer sizes (correlated with timestamps?)
    std::list<std::weak_ptr<QC2Buffer>> mInputRefs;

    struct WorkItem {
        std::shared_ptr<QC2Buffer> mInput;
        bool mMaybeFlushed; // was downstream when flush was initiated. may be flushed
        bool mDone;         // Codec has signaled input-done
        std::list<std::shared_ptr<QC2Buffer>> mOutputs;
    };

    // Tracks inputs queued to the codec and incomplete work.
    // NOTE: strict criteria for a test to pass is: 'No work must be pending'.
    // Inputs (+outputs) are evicted from pending-queue based on following criteria
    //    [1] [i] Input must be returned
    //        AND
    //        [ii] Output(s) must be produced, with atleast 1 output not marked with INPUT_PENDING
    // OR [2] [i] Input is returned and was marked as may-be-flushed
    //        AND
    //        [ii] flush is completed
    class PendingWork {
    public:
        void addInput(std::shared_ptr<QC2Buffer> input);

        void onOutputDone(std::shared_ptr<QC2Buffer> output);
        void onInputDone(uint64_t inputIndex);
        void onFlushStart();
        void onFlushEnd();

        bool hasPendingWork() {
            std::lock_guard<std::mutex> lock(mLock);
            if (!mItems.empty()) {
                QLOGW("PendingQueue has %zu works left !", mItems.size());
            }
            return !(mItems.empty());
        }

        // wait till a work (with specified input-index) has been completed
        QC2Status waitForWorkCompletion(uint64_t inputIndex, uint32_t timeoutMs);

        uint32_t lastOutputFlags() const {
            return mLastOutputFlags;
        }

        bool areAllOutputsReleased();

    private:
        std::unordered_map<uint64_t /*input-index*/, WorkItem /*item*/> mItems;

        std::mutex mLock;
        std::condition_variable mNotify;

        uint32_t mLastOutputFlags;
        std::list<OutputDescr> mReceivedOutputs;
    };
    PendingWork mPendingWork;

    struct QueueMode {
        bool mCountCodecConfigBuffers;
        bool mForceEmptyEOSBuffer;
        bool mForceInvalidEOSBuffer;

        QueueMode() {
            mCountCodecConfigBuffers = false;
            mForceEmptyEOSBuffer = false;
            mForceInvalidEOSBuffer = false;
        }
    };

    // Test modes
    QueueMode mQueueMode;
    bool mSimulateBlockingGraphicPool = false;

    // from ::testing::Test
    virtual void SetUp();
    virtual void TearDown();

    // interface to Tests
    void createCodec(std::string name);
    void loadplugin();
    QC2Status initCodec();
    QC2Status startCodec();
    QC2Status stopCodec();
    QC2Status queueInputs(TestInputSource *source, int numInputs, bool forceEOS);

    // tunables

    // event handlers
    void handleOutputBufDone(std::shared_ptr<Event> e);
    void handleInputBufDone(std::shared_ptr<Event> e);
    void handleReconfig(std::shared_ptr<Event> e);
    void handleInternalFlushDone();

    // helpers
    void logBuffer(std::shared_ptr<QC2Buffer> buf, const char* prefix);

    // validators
    // mPendingWork.hasPendingWork()
    bool areAllOutputsReleased();   // checks if all output references have been released (by Codec)
    bool areAllInputsReleased();    // checks if all input references have been released (by Codec)
    bool areOutputsInOrder();   // for now, assumes display-order. TODO(PC): use ordinal info

    // generic tests
    void decode(
            const std::string& mime,
            const std::string& codecName,
            const std::string& stream,
            uint32_t numFrames,
            bool bValidateBufferFlow = false);

    void decodeMultipleDrain(
        const std::string& mime,
        const std::string& codecName,
        const std::string& streamName,
        std::vector<uint32_t> numFramesBeforeDrain);

    void decodeBasicParam(
        const std::string& mime,
        const std::string& codecName,
        const std::string& streamName,
        uint32_t numFrames,
        std::list<std::unique_ptr<C2Param>> settings);

    void decodeMultipleFlush(
        const std::string& mime,
        const std::string& codecName,
        const std::string& streamName,
        std::vector<uint32_t> numFramesBeforeFlush);

    void validateCapsHelpers();

    void validateBufferFlow();

};

// event-handler supplied to the codec
class QC2FilterTestHandler : public EventHandler {
public:
    QC2FilterTestHandler(QC2FilterTest& test)
    : mTest(test) {}
    virtual ~QC2FilterTestHandler() = default;
protected:
    virtual QC2Status onEvent(std::shared_ptr<Event> event) override {
        return mTest.onEvent(event);
    }
    QC2FilterTest& mTest;
};

/*
 * bufferpool to simulate blockpool blocking behavior
 */
class QC2FilterTestGraphicBufferPool : public QC2GraphicBufferPoolImpl {
 public:
    QC2FilterTestGraphicBufferPool(std::shared_ptr<C2BlockPool> pool, bool isSecure)
        : QC2GraphicBufferPoolImpl(pool, isSecure), mCount(0), mFirstAllocatedTimeUs(0llu),
          mMinDelayUs(33000llu) {
    }
    virtual ~QC2FilterTestGraphicBufferPool() = default;
    QC2Status allocate(std::shared_ptr<QC2Buffer> *buf) override {
        uint64_t now = OS::CurrentTimeUs();
        QC2Status ret = QC2_TIMED_OUT;
        if (now - mFirstAllocatedTimeUs > (uint64_t)(mCount + 1) * mMinDelayUs) {
            QLOGD("QC2FilterTestGraphicBufferPool: trying to allocate..");
            ret = QC2GraphicBufferPoolImpl::allocate(buf);
            if (ret == QC2_OK) {
                if (mCount == 0) {
                    mFirstAllocatedTimeUs = OS::CurrentTimeUs();
                }
                ++mCount;
                return ret;
            } else {
                QLOGE("QC2FilterTestGraphicBufferPool: failed to allocate..");
            }
        } else {
            uint64_t earlyUs = (uint64_t)(mCount + 1) * mMinDelayUs - (now - mFirstAllocatedTimeUs);
            QLOGD("QC2FilterTestGraphicBufferPool: Cannot allocate for next %llu ms (min delay=%llu)",
                    (unsigned long long)(earlyUs) / 1000,
                    (unsigned long long)mMinDelayUs / 1000);
        }
        return ret;
    }
 private:
    size_t mCount = 0;
    uint64_t mFirstAllocatedTimeUs = 0llu;
    uint64_t mMinDelayUs = 33000llu; //33 ms
};

// buf-pool provider for Codec
struct FilterTestCodecBufPoolProvider : public QC2Codec::BufferPoolProvider {
    FilterTestCodecBufPoolProvider(std::shared_ptr<C2Component> comp, bool blockingGraphicAllocator)
        : mComp (comp), mBlockingGraphicAllocator(blockingGraphicAllocator) {
    }
    virtual ~FilterTestCodecBufPoolProvider() = default;

    std::unique_ptr<QC2LinearBufferPool> requestLinearPool(int64_t poolId) override {
        if (poolId == C2BlockPool::BASIC_GRAPHIC) {
            QLOGE("FilterTestCodecBufPoolProvider: incompatible(GRAPHIC) block-pool id for linear");
            return nullptr;
        }
        return std::make_unique<QC2LinearBufferPool>(getBlockPool(poolId), 0);
    }

    std::unique_ptr<QC2GraphicBufferPool> requestGraphicPool(int64_t poolId) override {
        if (poolId == C2BlockPool::BASIC_LINEAR) {
            QLOGE("FilterTestCodecBufPoolProvider: incompatible(LINEAR) block-pool id for graphic");
            return nullptr;
        }
        if (mBlockingGraphicAllocator) {
            return std::make_unique<QC2FilterTestGraphicBufferPool>(getBlockPool(poolId), 0);
        } else {
            return std::make_unique<QC2GraphicBufferPoolImpl>(getBlockPool(poolId), 0);
        }
    }

 private:
    std::weak_ptr<C2Component> mComp;
    bool mBlockingGraphicAllocator;

    std::shared_ptr<C2BlockPool> getBlockPool(int64_t poolId) {
        auto comp = mComp.lock();
        if (comp == nullptr) {
            QLOGE("FilterTestCodecBufPoolProvider: No component to create buffer-pool");
            return nullptr;
        }
        std::shared_ptr<C2BlockPool> pool = nullptr;
        auto ret = GetCodec2BlockPool(poolId, comp, &pool);
        if (ret != C2_OK || pool == nullptr) {
            QLOGE("FilterTestCodecBufPoolProvider: Failed to create block-pool (%lld)!",
                    (long long)poolId);
        } else {
            QLOGD("FilterTestCodecBufPoolProvider: Created block-pool (%lld)",
                    (long long)poolId);
        }
        return pool;
    }
};


static void QC2FilterTest_deleter(C2Component *comp) {
    QLOGI("Deleting component");
    delete comp;
}

void QC2FilterTest::SetUp() {
    mQueue = std::make_shared<EventQueue>("codecTestEventQ");
    mHandler = std::shared_ptr<EventHandler>(new QC2FilterTestHandler(*this));
    mStore = QC2ComponentStore::Get();
    mHandler->attach(mQueue);
    mQueue->start();

    // request block-pool and init buffer-pools
    auto status = QC2Component::Create(
            MOCK_NAME("c2.qti.avc.decoder"), 0, nullptr, QC2FilterTest_deleter, &mComp);
    ASSERT_TRUE(status == QC2_OK);
    ASSERT_TRUE(mComp != nullptr);

    std::shared_ptr<C2BlockPool> pool;
    c2_status_t ret = GetCodec2BlockPool(C2BlockPool::BASIC_LINEAR, mComp, &pool);
    ASSERT_TRUE(ret == C2_OK);
    ASSERT_TRUE(pool != nullptr);
    mLinearPool = std::make_unique<QC2LinearBufferPool>(pool, MemoryUsage::HW_CODEC_READ);

    C2StoreIonUsageInfo ionInfo;
    std::vector<std::unique_ptr<C2Param>> params;
    if (mStore->query_sm({&ionInfo}, {}, &params) == C2_OK) {
        auto alignment = ionInfo.minAlignment;
        QLOGI("set linear buffer alignment to %d", alignment);
        mLinearPool->setAlignment(alignment);
    } else {
        QLOGE("query platform ion usage info failed");
    }

    pool = nullptr;
    ret = GetCodec2BlockPool(C2BlockPool::BASIC_GRAPHIC, mComp, &pool);
    ASSERT_TRUE(ret == C2_OK);
    ASSERT_TRUE(pool != nullptr);
    mGraphicPool = std::make_shared<QC2GraphicBufferPoolImpl>(pool, MemoryUsage::HW_CODEC_READ);

    mCurrentInputIndex = 0;
    mLastQueuedInputIndex = 0;
}

void QC2FilterTest::TearDown() {
    mQueue->stop();
    mQueue = nullptr;
    mHandler = nullptr;
    mCodec = nullptr;
}

void QC2FilterTest::loadplugin() {
    void *mLib = dlopen(ArgsParser::qc2filter_lib.c_str(), RTLD_LAZY);
    if (!mLib) {
        QLOGE("Failed to dlopen %s : %s", ArgsParser::qc2filter_lib.c_str(), dlerror());
    }
    ASSERT_TRUE(mLib != nullptr);

    QC2CodecPlugin::GetPluginFunc getPluginFunc =
            (QC2CodecPlugin::GetPluginFunc) dlsym(mLib, QC2CodecPlugin::kGetPluginFuncName);
    ASSERT_TRUE(getPluginFunc != nullptr);

    QC2CodecPlugin *pluginInst = (*getPluginFunc)();

    ASSERT_TRUE(pluginInst != nullptr);

    auto codecs = pluginInst->getSupportedCodecs();

    ASSERT_TRUE(codecs.size() > 0);

    for (size_t i = 0; i < codecs.size(); ++i) {
         std::string name;
        QC2CodecEntry entry;
        std::unique_ptr<QC2CodecFactory> factory;
        std::tie(name, entry, factory) = std::move(codecs[i]);
        ASSERT_TRUE(factory != nullptr);
        mFactories.emplace(name, std::move(factory));
    }

}

void QC2FilterTest::createCodec(std::string name) {
    loadplugin();
    QLOGI("creating codec %s", name.c_str());
    ASSERT_TRUE(QC2CodecRegistry::HasCodec(name));
    std::unique_ptr<QC2Codec> codec;
    std::vector<std::shared_ptr<QC2Codec>> codecs;
    ASSERT_EQ(QC2FilterTestRegistryTester::CreateQC2Codec(name, "", "", nullptr, &codec), QC2_OK);
    mDecoder = std::move(codec);
    codecs.push_back(mDecoder);

    ASSERT_GT(mFactories.count(ArgsParser::qc2filter_name.c_str()), 0);
    for (uint32_t i = 0; i < kNumInstances; ++i) {
        mFactories[ArgsParser::qc2filter_name.c_str()]->createCodec(0, ArgsParser::qc2filter_name.c_str(), "", nullptr, &codec);
        ASSERT_NE(codec, nullptr);
        codecs.emplace_back(std::move(codec));
    }

  //  std::copy(codecs.begin(), codecs.end(), mFilters.begin());

    QLOGE("Number of codecs = %zu", codecs.size());

    for (uint32_t i = 1; i < codecs.size(); ++i) {
        mFilters.emplace_back(std::static_pointer_cast<QC2MockFilter>(codecs[i]));
    }

    struct QC2CodecRegistry::Details details;
    ASSERT_EQ(QC2CodecRegistry::DescribeCodec(name, &details), QC2_OK);

    QC2Status status = QC2PipelineCodec::CreatePipelinedCodec(
                                name, 1, details.inputMime(), details.outputMime(), details.kind(),
                                details.variant(), codecs, mHandler, &mCodec, nullptr);
    ASSERT_EQ(status, QC2_OK);
    return;
}

QC2Status QC2FilterTest::initCodec() {
    if (!mComp) {
        QLOGE("initCodec: Component not created yet !");
        return QC2_ERROR;
    }

    auto bufPoolProvider = std::make_unique<FilterTestCodecBufPoolProvider>(mComp,
            mSimulateBlockingGraphicPool);
    return mCodec->init(std::move(bufPoolProvider));
}

QC2Status QC2FilterTest::startCodec() {
    QLOGI("startCodec");
    if (!mComp) {
        QLOGE("Component not created yet !");
        return QC2_ERROR;
    }
    return mCodec->start();
}

QC2Status QC2FilterTest::queueInputs(
        TestInputSource *source, int numInputs, bool forceEOS) {
    while (numInputs > 0) {
        std::shared_ptr<QC2Buffer> frame;
        QC2Status ret = source->nextFrame(&frame);
        _RETURN_IF((ret != QC2_OK) || (frame == nullptr), QC2_ERROR);

        logBuffer(frame, "stream Input");
        if (!(frame->flags() & BufFlag::CODEC_CONFIG) || mQueueMode.mCountCodecConfigBuffers) {
            --numInputs;
        }

        if (forceEOS && numInputs == 0) {
            if (mQueueMode.mForceEmptyEOSBuffer) {
                if (frame->isLinear()) {
                    frame->linear().setRange(0, 0);
                }
                frame->setFlags(0u);
            }
            frame->setFlags(frame->flags() | BufFlag::EOS);
        }
        frame->setInputIndex(mCurrentInputIndex);
        mLastQueuedInputIndex = mCurrentInputIndex;

        logBuffer(frame, "Queuing Input");
        auto iPack = std::make_shared<QC2Codec::InputBufferPack>(frame);
        uint32_t numQueued = 0;

        // config error handler
        auto cfgerr_cb = [&](uint64_t inputIndex, const C2Param& param, QC2Status err) {
            (void)param;
            (void)err;
            QLOGE("Config Error for input %u", (uint32_t)inputIndex);
        };

        // queueing to codec and registering work with trackers must be atomic
        std::lock_guard<std::mutex> lock(mLock);
        mPendingWork.addInput(frame);
        mNumQueuedInputs++;
        ret = mCodec->queueInputs({iPack}, cfgerr_cb, &numQueued);
        _RETURN_IF(ret != QC2_OK, ret);
        _RETURN_IF(numQueued != 1, QC2_ERROR);

        mInputRefs.push_back(frame);

        ++mCurrentInputIndex;

        static const uint32_t kMaxOutstandingBufs = 18;
        if (mLastQueuedInputIndex > kMaxOutstandingBufs) {
            mPendingWork.waitForWorkCompletion(
                    mLastQueuedInputIndex - kMaxOutstandingBufs, kMaxOutstandingBufs * 100);
        }
    }

    return QC2_OK;
}

QC2Status QC2FilterTest::onEvent(std::shared_ptr<Event> e) {
    switch(e->id()) {
        case QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE:
            handleOutputBufDone(e);
            break;
        case QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE:
            handleInputBufDone(e);
            break;
        case QC2Codec::CODEC_NOTIFY_RECONFIG:
            handleReconfig(e);
            break;
        case QC2Codec::CODEC_NOTIFY_ERROR:
            QLOGE("Codec reported error!!");
            break;
        case QC2FilterTest_EVENT_FLUSH_DONE:
            QLOGD("handling flush-done");
            mPendingWork.onFlushEnd();
            break;
        default:
            break;
    }
    return QC2_OK;
}

void QC2FilterTest::handleOutputBufDone(std::shared_ptr<Event> e) {
    QLOGV("Handling output done..");
    std::shared_ptr<QC2Codec::OutputBuffersDoneInfo> oDone;
    e->info().get(QC2Codec::NotifyKey::kBufsDone, &oDone);

    for (auto& outBuf : oDone->mOutputs) {
        logBuffer(outBuf.mBuf, "Output");
        mPendingWork.onOutputDone(outBuf.mBuf);
    }

}

void QC2FilterTest::handleInputBufDone(std::shared_ptr<Event> e) {
    QLOGV("Handling input done..");
    std::shared_ptr<QC2Codec::InputBuffersDoneInfo> iDone;
    e->info().get(QC2Codec::NotifyKey::kBufsDone, &iDone);

    for (uint64_t id : iDone->mInputFrameIds) {
        QLOGI("Input returned idx=%u", (uint32_t)id);
        mPendingWork.onInputDone(id);
    }
}

void QC2FilterTest::handleReconfig(std::shared_ptr<Event> e) {
    QLOGI("Handling reconfig event");
    std::shared_ptr<QC2Codec::ReconfigParamsInfo> reconf;
    e->info().get(QC2Codec::NotifyKey::kReconfig, &reconf);
    if (!reconf) {
        QLOGE("Reconfig event is broken!");
        return;
    }

    std::lock_guard<std::mutex> lock(mLock);
    if (mCodec->reconfigure(std::move(reconf->mTargetConfig)) != QC2_OK) {
        QLOGE("Codec refused to reconfigure !!");
    }
}

void QC2FilterTest::logBuffer(std::shared_ptr<QC2Buffer> buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGI("%s: %s %s", prefix, BufFlag::Str(buf->flags()).c_str(),
            QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

// validators
bool QC2FilterTest::areAllOutputsReleased() {
    // give a chance to the test handler to schedule and release the buffer reference
    std::this_thread::yield();
    usleep(100e3);
    return mPendingWork.areAllOutputsReleased();
}

bool QC2FilterTest::areAllInputsReleased() {
    for (auto& i : mInputRefs) {
        auto I = i.lock();
        if (I) {
            QLOGE("Input=%u (ts=%u) still referenced!!",
                    (uint32_t)I->inputIndex(), (uint32_t)I->timestamp());
            return false;
        }
    }
    return true;
}

bool QC2FilterTest::areOutputsInOrder() {
    //TODO(PC)
    return true;
}

//----------------------------
// Pending work queue
//----------------------------
void QC2FilterTest::PendingWork::addInput(std::shared_ptr<QC2Buffer> input) {
    if (!input) {
        return;
    }
    std::lock_guard<std::mutex> lock(mLock);
    uint64_t id = input->inputIndex();
    {
        auto duplicate = mItems.find(id);
        if (duplicate != mItems.end()) {
            QLOGE("PendingWork::addInput: index(%u) already added !", (uint32_t)id);
            return;
        }
    }
    mItems.emplace(id, WorkItem{
            .mInput = input,
            .mMaybeFlushed = false,
            .mDone = false,
            .mOutputs = {}});
    QLOGD("Pending work : %u", (uint32_t)id);
}

// Every produced output results in a WorkDone.
// Eviction from pending-queue depends on the state of input or more outputs pending for the input
void QC2FilterTest::PendingWork::onOutputDone(std::shared_ptr<QC2Buffer> output) {
    if (!output) {
        return;
    }
    std::lock_guard<std::mutex> lock(mLock);
    uint64_t inputId = output->inputIndex();
    auto pair = mItems.find(inputId);
    if (pair != mItems.end()) {
    } else {
        QLOGE("Codec returned output for a non-existant input-id(%u)", (uint32_t)inputId);
        return;
    }

    auto& item = pair->second;
    item.mOutputs.push_back(output);

    // work-done can be signaled here
    mLastOutputFlags = output->flags();

    // check for possible eviction
    if (item.mDone) {
        if (!(output->flags() & BufFlag::INPUT_PENDING)) {
            mItems.erase(inputId);
            QLOGD("Evicted work : %u", (uint32_t)inputId);
            mNotify.notify_one();
        }
    }

    mReceivedOutputs.emplace_back(QC2FilterTest::OutputDescr{
        .mInputId = output->inputIndex(),
        .mTs = output->timestamp(),
        .mFlags = output->flags(),
        .mBufRef = output,
    });
}

void QC2FilterTest::PendingWork::onInputDone(uint64_t inputIndex) {
    std::lock_guard<std::mutex> lock(mLock);
    auto pair = mItems.find(inputIndex);
    if (pair != mItems.end()) {
    } else {
        QLOGE("Codec returned non-existant input-id(%u)", (uint32_t)inputIndex);
        return;
    }
    auto& item = pair->second;
    item.mDone = true;

    // check if there is atleast one output that does not contain INPUT_PENDING => input
    // has been fully-consumed and also signaled 'done'; hence can be evicted from the pending-list
    bool completed = false;
    for (auto& output : item.mOutputs) {
        completed |= !(output->flags() & BufFlag::INPUT_PENDING);
    }
    // TODO(PC): review, is this assumption correct ? (CSD will not generate output)
    //           How will this work for encoder frame-drop-RC mode ?
    completed |= !!(item.mInput->flags() & BufFlag::CODEC_CONFIG);
    if (completed) {
        mItems.erase(inputIndex);
        QLOGD("Evicted work : %u", (uint32_t)inputIndex);
        mNotify.notify_one();
    }
}

void QC2FilterTest::PendingWork::onFlushStart() {
    std::lock_guard<std::mutex> lock(mLock);
    // mark all items in pending-list as 'flush-likely' since these are downstream (with Codec)
    // at the time flush is called
    for (auto& pair : mItems) {
        auto& item = pair.second;
        item.mMaybeFlushed = true;
    }
}

void QC2FilterTest::PendingWork::onFlushEnd() {
    std::lock_guard<std::mutex> lock(mLock);
    // This method indicates the end of flush sequence.
    // All input-buffers returned and that were marked as 'maybe-flushed' shall be evicted here.
    // Additionally, ones without output will be returned back as flushed-work to client
    std::list<uint64_t> flushedItems;
    for (auto& pair : mItems) {
        auto& item = pair.second;
        if (item.mDone && item.mMaybeFlushed) {
            if (item.mOutputs.empty()) {
                // WorkDone (flushed)
            }
            flushedItems.push_back(pair.first);
        }
    }
    for (auto id : flushedItems) {
        mItems.erase(id);
        QLOGD("Flushed work : %u", (uint32_t)id);
        mNotify.notify_one();
    }
    QLOGV("Number of works pending: %zu", mItems.size());
}

QC2Status QC2FilterTest::PendingWork::waitForWorkCompletion(
        uint64_t inputIndex, uint32_t timeoutMs) {
    static const int kNumRetries = 10;
    size_t retry = 0;
    while (mItems.count(inputIndex) > 0 && retry++ < kNumRetries) {
        QLOGD("waiting for work input-id = %u (timeout=%u)", (uint32_t)inputIndex,
                (uint32_t)(timeoutMs/kNumRetries));
        std::unique_lock<std::mutex> lock(mLock);
        if (mNotify.wait_for(lock, timeoutMs*1ms / kNumRetries,
                [&]() -> bool {
                    return !(mItems.count(inputIndex) > 0);
                }) == false) {
            QLOGV("Still waiting for work with input-id = %u", (uint32_t)inputIndex);
            std::this_thread::yield();
        }
    }
    if (mItems.count(inputIndex) > 0) {
        QLOGW("Timed-out waiting for work with input-id = %u", (uint32_t)inputIndex);
        return QC2_TIMED_OUT;
    } else {
        std::this_thread::yield();  // give a chance to the other thread to drop buffer references
        QLOGD("wait done for work input-id = %u", (uint32_t)inputIndex);
        return QC2_OK;
    }
}

bool QC2FilterTest::PendingWork::areAllOutputsReleased() {
    for (auto& o : mReceivedOutputs) {
        auto O = o.mBufRef.lock();
        if (O) {
            QLOGE("Output for input=%u (ts=%u) still referenced!!",
                    (uint32_t)O->inputIndex(), (uint32_t)O->timestamp());
            return false;
        }
    }
    return true;
}

void QC2FilterTest::decode(
        const std::string& mime,
        const std::string& codecName,
        const std::string& streamName,
        uint32_t numFrames,
        bool bValidateBufferFlow) {
    // create a stream
    std::unique_ptr<TestInputStream> stream;
    auto ret = TestInputStream::Create(mime, streamName, mLinearPool, &stream);

    ASSERT_TRUE(stream != nullptr);

    if (!stream->valid() || ret != QC2_OK) {
        std::cout << "[ SKIPPED  ] ---- Failed to open stream : " << streamName << "\n";
        return;
    }

    if (!codecName.empty()) {
        createCodec(codecName.c_str());
        ASSERT_TRUE(mCodec != nullptr);
    }
    ASSERT_TRUE(initCodec() == QC2_OK);

    ASSERT_TRUE(startCodec() == QC2_OK);

    ASSERT_EQ(queueInputs(stream.get(), numFrames, true /*EOS*/), QC2_OK);

    ASSERT_EQ(mPendingWork.waitForWorkCompletion(mLastQueuedInputIndex, 1000 + (numFrames * 100)),
            QC2_OK);

    // since EOS was sent with last frame, expect all inputs and outputs to be released by Codec
    ASSERT_TRUE(!mPendingWork.hasPendingWork());
    ASSERT_TRUE(!!(mPendingWork.lastOutputFlags() & BufFlag::EOS));
    ASSERT_TRUE(areAllOutputsReleased());
    ASSERT_TRUE(areAllInputsReleased());
    ASSERT_TRUE(areOutputsInOrder());
    if (bValidateBufferFlow) {
        validateBufferFlow();
    }

    ASSERT_TRUE(mCodec->stop() == QC2_OK);
    ASSERT_TRUE(mCodec->release() == QC2_OK);
}

void QC2FilterTest::decodeBasicParam(
    const std::string& mime,
    const std::string& codecName,
    const std::string& streamName,
    uint32_t numFrames,
    std::list<std::unique_ptr<C2Param>> settings
) {
    // create a stream
    std::unique_ptr<TestInputStream> stream;
    auto ret = TestInputStream::Create(mime, streamName, mLinearPool, &stream);
    if (!stream->valid() || ret != QC2_OK) {
        std::cout << "[ SKIPPED  ] ---- Failed to open stream : " << streamName << "\n";
        return;
    }
    ASSERT_TRUE(stream != nullptr);

    createCodec(codecName.c_str());
    ASSERT_TRUE(mCodec != nullptr);
    ASSERT_TRUE(initCodec() == QC2_OK);

    for (auto& setting : settings) {
        ASSERT_TRUE(mCodec->configure(*setting) == QC2_OK);
    }

    ASSERT_TRUE(startCodec() == QC2_OK);

    ASSERT_EQ(queueInputs(stream.get(), numFrames, true /*EOS*/), QC2_OK);

    ASSERT_EQ(mPendingWork.waitForWorkCompletion(mLastQueuedInputIndex, 1000 + (numFrames * 33)),
            QC2_OK);

    // since EOS was sent with last frame, expect all inputs and outputs to be released by Codec
    ASSERT_TRUE(!mPendingWork.hasPendingWork());
    ASSERT_TRUE(!!(mPendingWork.lastOutputFlags() & BufFlag::EOS));
    ASSERT_TRUE(areAllOutputsReleased());
    ASSERT_TRUE(areAllInputsReleased());
    ASSERT_TRUE(areOutputsInOrder());

    ASSERT_TRUE(mCodec->stop() == QC2_OK);
    ASSERT_TRUE(mCodec->release() == QC2_OK);
}

void QC2FilterTest::decodeMultipleDrain(
    const std::string& mime,
    const std::string& codecName,
    const std::string& streamName,
    std::vector<uint32_t> numFramesBeforeDrain
) {
        // create a stream
    std::unique_ptr<TestInputStream> stream;
    auto ret = TestInputStream::Create(mime, streamName, mLinearPool, &stream);
    if (!stream->valid() || ret != QC2_OK) {
        std::cout << "[ SKIPPED  ] ---- Failed to open stream : " << streamName << "\n";
        return;
    }
    ASSERT_TRUE(stream != nullptr);

    createCodec(codecName.c_str());
    ASSERT_TRUE(mCodec != nullptr);
    ASSERT_TRUE(initCodec() == QC2_OK);

    ASSERT_TRUE(startCodec() == QC2_OK);

    for (auto& numFrames : numFramesBeforeDrain) {
        QLOGD("Queueing %u number of frames with no EOS", numFrames);
        ASSERT_EQ(queueInputs(stream.get(), numFrames, false), QC2_OK);
        ASSERT_EQ(mCodec->drain(), QC2_OK);
        QLOGD("Drain sent");
        ASSERT_EQ(mPendingWork.waitForWorkCompletion(mLastQueuedInputIndex, 1000 + (numFrames * 100)), QC2_OK);
        ASSERT_TRUE(!mPendingWork.hasPendingWork());
        ASSERT_TRUE(areAllOutputsReleased());
        ASSERT_TRUE(areAllInputsReleased());
        ASSERT_TRUE(areOutputsInOrder());
    }

    ASSERT_TRUE(mCodec->stop() == QC2_OK);
    ASSERT_TRUE(mCodec->release() == QC2_OK);
}

void QC2FilterTest::decodeMultipleFlush(
    const std::string& mime,
    const std::string& codecName,
    const std::string& streamName,
    std::vector<uint32_t> numFramesBeforeFlush
) {
        // create a stream
    std::unique_ptr<TestInputStream> stream;
    auto ret = TestInputStream::Create(mime, streamName, mLinearPool, &stream);
    if (!stream->valid() || ret != QC2_OK) {
        std::cout << "[ SKIPPED  ] ---- Failed to open stream : " << streamName << "\n";
        return;
    }
    ASSERT_TRUE(stream != nullptr);

    createCodec(codecName.c_str());
    ASSERT_TRUE(mCodec != nullptr);
    ASSERT_TRUE(initCodec() == QC2_OK);

    ASSERT_TRUE(startCodec() == QC2_OK);

    for (auto& numFrames : numFramesBeforeFlush) {
        QLOGD("Queueing %u number of frames with no EOS", numFrames);
        ASSERT_EQ(queueInputs(stream.get(), numFrames, false), QC2_OK);
        QLOGD("Sending flush");
        mPendingWork.onFlushStart();
        ASSERT_EQ(mCodec->flush(), QC2_OK);
        // wait till in-flight flushed buffers have landed in pending work-queue
        {
            auto e = std::make_shared<Event>(QC2FilterTest_EVENT_FLUSH_DONE);
            mHandler->postSyncAndWait(e);
        }
        ASSERT_EQ(mPendingWork.waitForWorkCompletion(mLastQueuedInputIndex,
                1000 + (numFrames * 100)), QC2_OK);
        ASSERT_EQ(mPendingWork.hasPendingWork(), false);
    }

    ASSERT_EQ(mCodec->drain(), QC2_OK);
    ASSERT_EQ(mPendingWork.waitForWorkCompletion(mLastQueuedInputIndex, 1000 + (*prev(numFramesBeforeFlush.end()) * 100)), QC2_OK);
    ASSERT_TRUE(!mPendingWork.hasPendingWork());
    ASSERT_TRUE(areAllOutputsReleased());
    ASSERT_TRUE(areAllInputsReleased());
    ASSERT_TRUE(areOutputsInOrder());

    ASSERT_TRUE(mCodec->stop() == QC2_OK);
    ASSERT_TRUE(mCodec->release() == QC2_OK);
}

void QC2FilterTest::validateBufferFlow() {
    uint32_t nFRCMultiplier = 1;
    for (auto filter: mFilters) {
        ASSERT_EQ(filter->getNumInputsQueued(), mNumQueuedInputs * nFRCMultiplier);
        nFRCMultiplier = filter->getFRCMultiplier();
        if (!nFRCMultiplier) {
            nFRCMultiplier = 1;
        }
    }
}

void QC2FilterTest::validateCapsHelpers() {
    std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>> capshelpersCombined;
    std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>> capshelpersDecoder;
    std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>> capshelpersFilter;
    ASSERT_TRUE(mCodec != nullptr);
    ASSERT_TRUE(mCodec->getCapsHelpers(&capshelpersCombined) == QC2_OK);
    ASSERT_TRUE(mDecoder->getCapsHelpers(&capshelpersDecoder) == QC2_OK);

    for (const auto &helper: capshelpersDecoder) {
        auto r = capshelpersCombined.find(helper.first);
        ASSERT_TRUE(r != capshelpersCombined.end());
    }

    for (size_t i = 0; i < mFilters.size(); ++i) {
        ASSERT_TRUE(mFilters[i]->getCapsHelpers(&capshelpersFilter) == QC2_OK);
        for (const auto &helper: capshelpersFilter) {
            auto r = capshelpersCombined.find(helper.first);
            ASSERT_TRUE(r != capshelpersCombined.end());
        }
        capshelpersFilter.clear();
    }
    return;
}

class QC2FilterTestParamBuilder {
public:
    void setVideoOutputSize(uint32_t w, uint32_t h) {
        auto size = std::make_unique<C2StreamPictureSizeInfo::output>();
        size->width = w;
        size->height = h;
        mSettings.push_back(std::move(size));
    }
    void setVideoInputSize(uint32_t w, uint32_t h) {
        auto size = std::make_unique<C2StreamPictureSizeInfo::input>();
        size->width = w;
        size->height = h;
        mSettings.push_back(std::move(size));
    }
    void setVideoProfileLevel(VideoProfileType profile, VideoLevelType level) {
        auto pl = std::make_unique<C2StreamProfileLevelInfo::input>();
        pl->profile = (C2Config::profile_t)profile;
        pl->level = (C2Config::level_t)level;
        mSettings.push_back(std::move(pl));
    }

    void setVideoBitrate(uint32_t bitrate) {
        auto outputBitrate = std::make_unique<C2StreamBitrateInfo::output>();
        outputBitrate->value = bitrate;
        mSettings.push_back(std::move(outputBitrate));
    }

    void setVideoFramerate(float framerate) {
        auto inputFramerate = std::make_unique<C2StreamFrameRateInfo::input>();
        inputFramerate->value = framerate;
        mSettings.push_back(std::move(inputFramerate));
    }
    void setVideoColorAspects(C2Color::primaries_t c, C2Color::range_t r,
            C2Color::matrix_t m, C2Color::transfer_t t) {
        auto colorAspects = std::make_unique<C2StreamColorAspectsInfo::output>();
        colorAspects->primaries = c;
        colorAspects->range = r;
        colorAspects->matrix = m;
        colorAspects->transfer = t;
        mSettings.push_back(std::move(colorAspects));
    }

    void setVideoPictureOrder(uint32_t enable /* 0 or 1 */) {
        auto pictureOrder = std::make_unique<C2VideoPictureOrder::output>();
        pictureOrder->enable = enable;
        mSettings.push_back(std::move(pictureOrder));
    }

    void setFRCParams(uint32_t val) {
        auto frc = std::make_unique<C2VideoMockFrameMultiplier::output>();
        frc->value = val;
        mSettings.push_back(std::move(frc));
        }

    void setResizeParams(uint32_t val) {
        auto resize = std::make_unique<C2VideoMockSizeMultiplier::output>();
        resize->value = val;
        mSettings.push_back(std::move(resize));
    }

    std::list<std::unique_ptr<C2Param>> get() {
        return std::move(mSettings);
    }
private:
    std::list<std::unique_ptr<C2Param>> mSettings;
};

//-------------------------------------------------------------------------------------------------
// Tests
//-------------------------------------------------------------------------------------------------
TEST_F(QC2FilterTest, Test_LoadPlugin) {
    loadplugin();
}

TEST_F(QC2FilterTest, TestAll_BasicInitReleaseForAllCodecs) {
    std::list<uint32_t> filter;
    QC2CodecRegistry::Details entry;
    filter.push_back(ComponentKind::KIND_DECODER);
    filter.push_back(ComponentKind::KIND_ENCODER);
    std::vector<std::string> codecs;
    QC2CodecRegistry::ListCodecs(filter, &codecs);
    for (auto& codec : codecs) {
        QC2CodecRegistry::DescribeCodec(codec, &entry);
        if (!(entry.variant() & QC2CodecVariant::CAN_PIPELINE)) {
            continue;
        }
        mCodec = nullptr;
        createCodec(codec);
        ASSERT_TRUE(mCodec != nullptr);
        ASSERT_TRUE(initCodec() == QC2_OK);
        ASSERT_TRUE(mCodec->release() == QC2_OK);
    }
}

TEST_F(QC2FilterTest, TestAll_CapsHelpersAllCodecs) {
    std::list<uint32_t> filter;
    QC2CodecRegistry::Details entry;
    filter.push_back(ComponentKind::KIND_DECODER);
    filter.push_back(ComponentKind::KIND_ENCODER);
    std::vector<std::string> codecs;
    QC2CodecRegistry::ListCodecs(filter, &codecs);
    for (auto& codec : codecs) {
        QC2CodecRegistry::DescribeCodec(codec, &entry);
        if (!(entry.variant() & QC2CodecVariant::CAN_PIPELINE)) {
            continue;
        }
        mCodec = nullptr;
        createCodec(codec);
        ASSERT_TRUE(mCodec != nullptr);
        ASSERT_TRUE(initCodec() == QC2_OK);
        validateCapsHelpers();
        ASSERT_TRUE(mCodec->release() == QC2_OK);
    }
}


TEST_F(QC2FilterTest, TestAVCDec_BasicStartStop) {
    std::string codecName = MOCK_NAME("c2.qti.avc.decoder");

    QC2CodecRegistry::Details entry;
    QC2CodecRegistry::DescribeCodec(codecName,&entry);
    ASSERT_TRUE((entry.variant() & QC2CodecVariant::CAN_PIPELINE) != 0);

    createCodec(codecName.c_str());
    ASSERT_TRUE(mCodec != nullptr);
    ASSERT_TRUE(initCodec() == QC2_OK);

    ASSERT_TRUE(startCodec() == QC2_OK);
    ASSERT_TRUE(mCodec->stop() == QC2_OK);
    ASSERT_TRUE(mCodec->release() == QC2_OK);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueOneInputWithEOS) {
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 1);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueOneEmptyEOSInput) {
    mQueueMode.mCountCodecConfigBuffers = true;  // we want the fist buffer to be EOS
    mQueueMode.mForceEmptyEOSBuffer = true;
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 1);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsWithEOS) {
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 100);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsWithEOSAndBlockingAllocator) {
    mSimulateBlockingGraphicPool = true;
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 100);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsAndEmptyEOSBuffer) {
    mQueueMode.mForceEmptyEOSBuffer = true;
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 32);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsAndEmptyEOSBufferWithBlockingAllocator) {
    mQueueMode.mForceEmptyEOSBuffer = true;
    mSimulateBlockingGraphicPool = true;
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 32);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputWithEOSExtractor) {
    decode(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "/sdcard/qc2_media/avc/avc_1080p.mp4", 218);
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsDrainAfterAllFrames) {
    decodeMultipleDrain(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", {300});
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsFollowedByDrain) {
    decodeMultipleDrain(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", {150,150});
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsFollowedByDrainWithBlockingAllocator) {
    mSimulateBlockingGraphicPool = true;
    decodeMultipleDrain(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", {32,32});
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsFollowedByFlush) {
    decodeMultipleFlush(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", {150});
}

TEST_F(QC2FilterTest, TestAVCDec_BasicQueueMultipleInputsFollowedByFlushWithBlockingAllocator) {
    mSimulateBlockingGraphicPool = true;
    decodeMultipleFlush(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", {32});
}

TEST_F(QC2FilterTest, TestAVCDec_BasicParams) {
    float framerate = 60.0;

    QC2FilterTestParamBuilder params;
    params.setVideoFramerate(framerate);
    params.setVideoPictureOrder(1);

    decodeBasicParam(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 100, params.get());
}

TEST_F(QC2FilterTest, TestFrameAccurateParamsAndConfigUpdates) {
    float framerate = 60.0;

    QC2FilterTestParamBuilder params;
    params.setVideoFramerate(framerate);
    params.setVideoPictureOrder(1);

    decodeBasicParam(MimeType::AVC, MOCK_NAME("c2.qti.avc.decoder"), "avc_basic_vga", 100, params.get());
}

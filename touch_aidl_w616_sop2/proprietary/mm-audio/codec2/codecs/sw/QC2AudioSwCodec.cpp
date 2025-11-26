/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <unordered_map>
#include <memory>
#include <string>
#include <list>
#include <algorithm>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioSwCodec.h"
#include "QC2AudioSwAlacDec.h"
#include "QC2AudioSwApeDec.h"
#include "QC2AudioSwEvrcDec.h"
#include "QC2AudioSwQcelpDec.h"
#include "QC2AudioSwFlacDec.h"
#include "QC2AudioSwDsdDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwCodec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwCodec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwCodec::QC2AudioSwCodec(
        const std::string& name,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& inputMime,
        const std::string& outputMime,
        ComponentKind kind,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2Codec(name, sessionId, inputMime, outputMime, kind, variant, listener),
              mInstName(instanceName) {
}

QC2Status QC2AudioSwCodec::getCapsHelpers(
        std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers) const {
    if (helpers) {
        for (auto& f : mCapsFactories) {
            auto b = f(mCodecName, mInputMime, mOutputMime, mVariant, mKind, nullptr);
            if (b->isSupported()) {
                helpers->emplace(b->getIndex(), b);
                QLOGD("%s registered", DebugString::C2Param(b->getIndex()).c_str());
            }
        }
    } else {
        return QC2_BAD_ARG;
    }
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::configure(const C2Param& param) {
    QLOGV_INST("configure: %s core-index=%x baseIndex=%x",
        DebugString::C2Param(param.index()).c_str(),
        param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexBlockPools: {
            auto blockPools = (C2PortBlockPoolsTuning::output *)&param;
            QLOGV_INST("Output pool-id = %llu", (unsigned long long)blockPools->m.values[0]);
            if (mBufPoolProvider == nullptr) {
                QLOGE_INST("Not buffer-pool provider available!");
                break;
            }
            auto updatedPool = mBufPoolProvider->requestLinearPool(blockPools->m.values[0]);
            if (updatedPool == nullptr) {
                QLOGE("Failed to update linear pool !");
                return QC2_ERROR;
            }
            mLinearPool = std::move(updatedPool);
            mLinearPool->setBufferSize(mMinOutBufSize);
            mLinearPool->setUsage(MemoryUsage::HW_CODEC_WRITE|MemoryUsage::HW_CODEC_WRITE_CACHED);
            QLOGD_INST("Linear pool initialized for size=%u", mMinOutBufSize);
            break;
        }
        default: {
            QLOGW_INST("unknown/unsupported param %s index = 0x%X",
                DebugString::C2Param(param.index()).c_str(), param.index());
            break;
        }
    }
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::configure(std::vector<std::unique_ptr<C2Param>> config) {
    QC2Status retVal = QC2_OK;
    if (config.empty()) {
        QLOGE("%s: target configuration is empty", __func__);
        return QC2_ERROR;
    }

    for (auto& param : config) {
        if (configure(*param) != QC2_OK) {
            retVal = QC2_ERROR;
        }
    }

    return retVal;
}

QC2Status QC2AudioSwCodec::queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const {
    (void)params;
    return QC2_OMITTED;
}

QC2Status QC2AudioSwCodec::query(C2Param *const param) {
    (void)param;
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::stop() {
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }

    QLOGV("%s: Enter", __func__);
    auto e = std::make_shared<Event>(MSG_ID_STOP);
    mStopping = true;
    mHandler->postSyncAndWait(e);

    stopEventQueue();

    return QC2_OK;
}

QC2Status QC2AudioSwCodec::reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    (void)targetConfig;
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::queueInputs(
        const std::list<std::shared_ptr<InputBufferPack>>& inputs,
        ReportConfigFailure_cb configFailureReporter,
        uint32_t *numInputsQueued) {
    (void)configFailureReporter;
    if (!numInputsQueued) {
        return QC2_BAD_ARG;
    }
    *numInputsQueued = 0u;
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }

    QLOGV("%s: Enter", __func__);
    QC2Status ret = QC2_OK;
    if (inputs.size()) {
        auto inputPack = std::make_shared<InputPackMsg>();
        std::for_each (inputs.begin(), inputs.end(),
            [&](std::shared_ptr<InputBufferPack> input) {
                ++(*numInputsQueued);
                inputPack->inputs.push_back(input);
            }
        );
        if (inputPack->inputs.size() > 0) {
            auto e = std::make_shared<Event>(MSG_ID_PROCESS);
            e->info().put(MsgInfoKey::kInputs, inputPack);
            mHandler->postAsync(e);
        }
    }

    return ret;
}

QC2Status QC2AudioSwCodec::flush() {
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }

    QLOGV("%s: Enter", __func__);
    auto e = std::make_shared<Event>(MSG_ID_FLUSH);
    mFlushing = true;
    mHandler->postSyncAndWait(e);

    return QC2_OK;
}

QC2Status QC2AudioSwCodec::drain() {
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }

    QLOGV("%s: Enter", __func__);
    auto e = std::make_shared<Event>(MSG_ID_DRAIN);
    mHandler->postAsync(e);
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::suspend() {
    QLOGV("%s: Enter", __func__);
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::resume() {
    QLOGV("%s: Enter", __func__);
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::release() {
    QLOGV("%s: Enter", __func__);
    return QC2_OK;
}

void QC2AudioSwCodec::logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGD_INST("%s %s", prefix, QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

QC2Status QC2AudioSwCodec::init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) {
    QLOGV("%s", __func__);

    QC2Status status = onInit();
    if (status != QC2_OK) {
        return status;
    }
    mBufPoolProvider = std::move(bufPoolProvider);

    return QC2_OK;
}

QC2Status QC2AudioSwCodec::deinit() {
    // deinit API is not required for SW components
    // Can be implemented in the future if necessary
    QLOGV("%s", __func__);
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::start() {
    QLOGV("%s", __func__);

    QC2Status status = onStart();
    if (status != QC2_OK) {
        return status;
    }

    if (mBufPoolProvider == nullptr) {
        QLOGE_INST("No Provider to request buffer-pools !");
        return QC2_NO_INIT;
    }
    mLinearPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (mLinearPool == nullptr) {
        QLOGE("Failed to create default linear pool !");
        return QC2_ERROR;
    }
    mLinearPool->setBufferSize(mMinOutBufSize);
    mLinearPool->setUsage(MemoryUsage::CPU_WRITE | MemoryUsage::CPU_READ);

    startEventQueue();
    return QC2_OK;
}

void QC2AudioSwCodec::onError(QC2Status errorCode, const std::string& errorMsg) {
     std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
     e->info().put(QC2Codec::NotifyKey::kErrorCode, errorCode);
     e->info().put(QC2Codec::NotifyKey::kErrorMsg, errorMsg);
     mListener->postAsync(e);
}

QC2Status QC2AudioSwCodec::onFlush(){
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::handleMessage(std::shared_ptr<Event> event) {
    QC2Status status = QC2_OK;
    if (!event) {
        return QC2_BAD_ARG;
    }

    switch (event->id()) {
    case MSG_ID_PROCESS: {
        std::shared_ptr<InputPackMsg> inputPack = nullptr;
        event->info().get(MsgInfoKey::kInputs, &inputPack);
        if (inputPack == nullptr) {
            QLOGE_INST("No inputs retrieved");
            return QC2_ERROR;
        }

        for (auto input = inputPack->inputs.begin(); input != inputPack->inputs.end(); ) {
            if (*input == nullptr) {
                QLOGE("%s: input is null, exiting.", __func__);
                return QC2_ERROR;
            }
            std::shared_ptr<QC2Buffer> outBuf;
            status = mLinearPool->allocate(&outBuf);
            if (status == QC2_OK && outBuf != nullptr) {
                logBuffer(outBuf, "Allocated");
            } else {
                QLOGE_INST("Failed to queue output buffer");
                onError(status,"Buffer Allocation Error");
                return QC2_ERROR;
            }

            // Generate output
            status = produceOutput(*input, outBuf);
            input = inputPack->inputs.erase(input);
            if (status != QC2_OK) {
                QLOGE_INST("Cannot generate output");
                onError(status,"Output generation error");
                return QC2_ERROR;
            }
        }
        break;
    }
    case MSG_ID_FLUSH: {
        onFlush();
        mFlushing = false;
        QLOGV("%s: Flushed all inputs and outputs", __func__);
        break;
    }
    case MSG_ID_DRAIN: {
        /* Drain (with or without EOS) is a NO-OP as we handle
           all events in event queue sequentially */
        QLOGV("%s: Drained all data", __func__);
        break;
    }
    case MSG_ID_STOP: {
        if (mStopping) {
            onStop();
            mStopping = false;
        }
        QLOGV("%s: Stopped", __func__);
        break;
    }
    default:
        break;
    }

    return status;
}

QC2Status QC2AudioSwCodec::startEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (!mHandler) {
        mHandler = std::make_shared<QC2AudioSwCodec::Handler>(*this);
    }
    if (!mEventQueue) {
        mEventQueue = std::make_shared<EventQueue>("EvtQ_codecPipe");
    }
    if (mHandler && mEventQueue) {
        mHandler->attach(mEventQueue);
        mEventQueue->start();
    } else {
        return QC2_NO_MEMORY;
    }
    return QC2_OK;
}

QC2Status QC2AudioSwCodec::stopEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (mEventQueue) {
        mEventQueue->stop();
        mEventQueue = nullptr;
    }
    mHandler = nullptr;
    return QC2_OK;
}

void QC2AudioSwCodec::onInputDone(std::shared_ptr<QC2Buffer> inBuf) {
    if (inBuf == nullptr) {
        QLOGE("%s: Invalid input buffer", __func__);
        return;
    }
    auto inBufDone = std::make_shared<InputBuffersDoneInfo>();
    inBufDone->mInputFrameIds.push_back(inBuf->inputIndex());

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, inBufDone);
    QLOGV("Returning Input @ %u", (int)inBuf->timestamp());
    mListener->postAsync(e);
}

void QC2AudioSwCodec::onOutputDone(std::shared_ptr<QC2Buffer> outBuf) {
    if (outBuf == nullptr) {
        QLOGE("%s: Invalid output buffer", __func__);
        return;
    }

    QC2Codec::OutputBufferPack outBufPack(outBuf);
    if (mUpdatedInfos.size()) {
        QLOGD_INST("ConfigUpdate: sending %zu updates", mUpdatedInfos.size());
        // Update component interface with infos
        onConfigUpdate(mUpdatedInfos);
        // Add config updates to work
        for (auto& info: mUpdatedInfos) {
            if (!info) {
                continue;
            }
            outBufPack.mConfigUpdates.push_back(C2Param::Copy(*info));
        }
        mUpdatedInfos.clear();
    }
    auto outBufs = std::make_shared<QC2Codec::OutputBuffersDoneInfo>();
    outBufs->mOutputs.emplace_back(std::move(outBufPack));
    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, outBufs);
    QLOGV("Output done @ %u", (int)(outBuf->timestamp()));
    mListener->postAsync(e);
}

void QC2AudioSwCodec::onConfigUpdate(std::list<std::shared_ptr<C2Param>> params) {
    if (params.empty()) {
        QLOGE("%s: Invalid params", __func__);
        return;
    }

    auto configUpdate = std::make_shared<QC2Codec::ConfigUpdateInfo>();
    configUpdate->mTargetConfig = params;

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_CONFIG_UPDATE);
    e->info().put(QC2Codec::NotifyKey::kConfigUpdate, configUpdate);
    mListener->postAsync(e);
}

QC2Status QC2AudioSwCodec::produceOutput(std::shared_ptr<InputBufferPack>& inPack,
                                         std::shared_ptr<QC2Buffer> outBuf) {
    QC2Status status = QC2_OK;

    if (inPack->mInput == nullptr) {
        QLOGE("%s: Invalid input !", __func__);
        return QC2_BAD_ARG;
    }
    if (outBuf == nullptr) {
        QLOGE("%s: Invalid output !", __func__);
        return QC2_BAD_ARG;
    }

    QTRACEV("produceOutput");
    std::shared_ptr<QC2Buffer> inBuf = inPack->mInput;

    outBuf->setInputIndex(inBuf->inputIndex());
    outBuf->setOutputIndex(inBuf->inputIndex());
    outBuf->setTimestamp(inBuf->timestamp());

    if ((mFlushing || mStopping) && !(inBuf->flags() & BufFlag::CODEC_CONFIG)) {
        QLOGV("%s: Flushing inputs and outputs", __func__);
        outBuf->setFlags(inBuf->flags() | BufFlag::DISCARD);
        outBuf->linear().setRange(0, 0);
        onOutputDone(outBuf);
        onInputDone(inBuf);
        return status;
    }

    status = process(inBuf, outBuf);
    if (status != QC2_OK) {
        QLOGE_INST("Failed to process input buffer");
        onError(status, "Decoding Error");
        return QC2_ERROR;
    }

    QLOGV("Generated output for input=%u", (uint32_t)inBuf->inputIndex());
    return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//                              Codec Factories                                              //
///////////////////////////////////////////////////////////////////////////////////////////////


QC2AudioSwDecoderFactory::QC2AudioSwDecoderFactory(const std::string& name, const std::string& mime,
            const std::string& nickName, uint32_t variant)
        : mName(name), mNickName(nickName), mCompressedMime(mime), mVariant(variant) {
    }

QC2Status QC2AudioSwDecoderFactory::createCodec(uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) {
    (void)upstreamMime;
    (void)downstreamMime;
    if (codec == nullptr) {
        return QC2_BAD_ARG;
    }
    std::string instName = mNickName + "_" + std::to_string(sessionId);

    if (!(strcasecmp(mCompressedMime.c_str(), MimeType::ALAC))) {
        *codec = std::make_unique<QC2AudioSwAlacDec>(
                     mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created ALAC decoder");
    } else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::APE))) {
        *codec = std::make_unique<QC2AudioSwApeDec>(
                     mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created APE decoder");
    } else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::EVRC))) {
        *codec = std::make_unique<QC2AudioSwEvrcDec>(
                    mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created EVRC decoder");
    } else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::QCELP))) {
        *codec = std::make_unique<QC2AudioSwQcelpDec>(
                    mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created QCELP decoder");
    } else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::FLAC))) {
        *codec = std::make_unique<QC2AudioSwFlacDec>(
                    mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created FLAC decoder");
    } else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::DSD))) {
        *codec = std::make_unique<QC2AudioSwDsdDec>(
                    mName, sessionId, instName, mCompressedMime, mVariant, listener);
        QLOGV("Created DSD decoder");
    } else {
        *codec = nullptr;
        QLOGE("Invalid mime type %s", mCompressedMime.c_str());
    }

    if (*codec == nullptr) {
        return QC2_NOT_FOUND;
    }
    return QC2_OK;
}

std::string QC2AudioSwDecoderFactory::name() {
    return "QC2AudioSwDecoderFactory" + mName;
}

QC2AudioSwEncoderFactory::QC2AudioSwEncoderFactory(const std::string& name, const std::string& mime,
            const std::string& nickName, uint32_t variant)
        : mName(name), mNickName(nickName), mCompressedMime(mime), mVariant(variant) {
    }

QC2Status QC2AudioSwEncoderFactory::createCodec(
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) {

    (void)sessionId;
    (void)upstreamMime;
    (void)downstreamMime;
    // Temporarily casting listener and mSecure to void
    // Please remove the cast when an encoder is implemented
    (void)listener;
    (void)mVariant;
    (void)codec;
    // Add code here to create appropriate encoder class once
    // audio software encoder is supported
    return QC2_NOT_FOUND;
}

std::string QC2AudioSwEncoderFactory::name() {
    return "QC2AudioSwEncoderFactory" + mName;
}

}   // namespace qc2audio

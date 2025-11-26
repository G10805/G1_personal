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
#include <chrono>

#include "QC2EventQueue.h"
#include "QC2AudioHwCodec.h"
#include "QC2AudioHwAacDec.h"
#include "QC2AudioHwAlacDec.h"
#include "QC2AudioHwApeDec.h"
#include "QC2AudioHwAmrWbPlusDec.h"
#include "QC2AudioHwWmaDec.h"
#include "QC2AudioHwAmrNbWbEnc.h"
#include "QC2AudioHwAacEnc.h"
#include "QC2AudioHwEvrcEnc.h"
#include "QC2AudioHwQcelpEnc.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwCodec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

#define RETURN_IF_ERROR(retVal, str) \
do {\
    if (retVal) { \
        QLOGE_INST(str);\
        return retVal;\
     } \
} while (0); \

using namespace std::chrono_literals;

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                             QC2Monitor callback                                           //
///////////////////////////////////////////////////////////////////////////////////////////////

std::mutex sMonitorCallbackLock;

static QC2Status MonitorCallbackFunction(uint32_t event, void *client, bool *wait) {
    QC2Status retVal = QC2_OK;
    std::lock_guard<std::mutex> lock(sMonitorCallbackLock);

    QC2AudioHwCodec *codec = (QC2AudioHwCodec *)client;
    switch (event) {
        case QC2Monitor::MonitorEvent::EVT_TIMEOUT: {
            if (codec->getPendingInputsSize() > 0 ||
                    codec->getNumQueuedInputFrames() > 0 ||
                    codec->getPendingOutputsSize() > 0) {
                *wait = true;
            }
            break;
        }
        case QC2Monitor::MonitorEvent::EVT_ACTION: {
            retVal = codec->suspend();
            break;
        }
    }
    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwCodec                                              //
///////////////////////////////////////////////////////////////////////////////////////////////
/* static */
QC2AudioHwCodec::QC2AudioHwCodec(
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

    mPendingInputs = std::make_shared<InputPackMsg>();
    mHwSessionId = nullptr;
}

void QC2AudioHwCodec::setHwSessionId(uint64_t* thizHwSession) {
    mHwSessionId = thizHwSession;
}

QC2Status QC2AudioHwCodec::getCapsHelpers(
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

QC2Status QC2AudioHwCodec::configure(std::vector<std::unique_ptr<C2Param>> params) {
    QC2Status retVal = QC2_OK;

    if (params.empty()) {
        QLOGE("%s: target configuration is empty", __func__);
        return QC2_ERROR;
    }

    for (auto& param : params) {
        if (configure(*param) != QC2_OK) {
            retVal = QC2_ERROR;
        }
    }
    return retVal;
}


QC2Status QC2AudioHwCodec::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;

    QLOGV_INST("configure: core-index=%x baseIndex=%x",
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
            mLinearPool->setBufferSize(mOutBufSize);
            mLinearPool->setUsage(MemoryUsage::HW_CODEC_WRITE|MemoryUsage::HW_CODEC_WRITE_CACHED);
            QLOGD_INST("Linear pool initialized for size=%u", mOutBufSize);
            break;
        }
        default: {
            QLOGW_INST("unknown/unsupported param %s index = 0x%X",
                DebugString::C2Param(param.index()).c_str(), param.index());
            break;
        }
    }
    return retVal;
}

QC2Status QC2AudioHwCodec::queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const {
         (void)params;
          return QC2_OMITTED;
}

QC2Status QC2AudioHwCodec::query(C2Param *const param) {

    QLOGV_INST("query: core-index=%x baseIndex=%x",
        param->coreIndex().coreIndex(), param->index());

    switch (param->coreIndex().typeIndex()) {

        case kParamIndexAudioHwState: {
            auto state = getHwState();
            auto hwState = reinterpret_cast<C2AudioHwStateInfo::input*>(param);
            hwState->value = state;
            break;
        }

        default:
            QLOGW_INST("unknown/unsupported param %s index = 0x%X",
                DebugString::C2Param(param->index()).c_str(), param->index());
            break;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    (void)targetConfig;
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::start() {
    QC2Status retVal = QC2_OK;

    if (mBufPoolProvider == nullptr) {
        QLOGE_INST("No Provider to request buffer-pools !");
        return QC2_NO_INIT;
    }

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    //if blockpool for current session not set, use default linear pool
    if (mLinearPool == nullptr) {
        mLinearPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
        if (mLinearPool == nullptr) {
            QLOGE("Failed to create default linear pool !");
            return QC2_ERROR;
        }
        mLinearPool->setBufferSize(mOutBufSize);
        //TODO: Update below flags with MemoryUsage flag for UNCACHED memory
        mLinearPool->setUsage(MemoryUsage::CPU_WRITE | MemoryUsage::CPU_READ);
    }
    mOutputBufIndex = 0;

    if (!mHwMonitor && property_get_bool(kHwCodecSuspendEnabledProp, false)) {
        int32_t timeout = property_get_int32(
                kHwCodecSuspendTimeoutProp, mHwSuspendTimeoutValMs);
        mHwMonitor = std::make_unique<QC2Monitor>(
                        mCodecName, MonitorCallbackFunction, this, timeout);
        if (mHwMonitor && mHwMonitor->create() != QC2_OK) {
            QLOGI("%s: Monitor creation failed", __func__);
            mHwMonitor = nullptr;
        }
    }

    retVal = onStart();
    if (retVal == QC2_OK) {
        startEventQueue();
    }
    return retVal;
}

QC2Status QC2AudioHwCodec::stop() {
    QC2Status retVal = QC2_OK;

    QLOGV("%s: Enter", __func__);
    mStopping = true;

    if (mHwMonitor && mSuspended) {
       if (resume() != QC2_OK) {
            QLOGW("%s: HW resume from suspend failed...stopping", __func__);
       }
    }
    if (mHwMonitor && mHwMonitor->stop() != QC2_OK) {
        QLOGV("%s: HW monitor stop failed", __func__);
    }
    stopEventQueue();
    retVal = mAudioHw->stop_session(mHwSessionId);
    return retVal;
}

QC2Status QC2AudioHwCodec::checkHwMonitorSuspend() {
    if (mSuspended) {
        if (resume() != QC2_OK) {
            QLOGE("%s: HW resume from suspend failed", __func__);
            return QC2_CORRUPTED;
        }
    } else {
        mHwMonitor->reset();
    }
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::queueInputs(
        const std::list<std::shared_ptr<InputBufferPack>>& inputs,
        ReportConfigFailure_cb configFailureReporter,
        uint32_t *numInputsQueued) {
    (void)configFailureReporter;
    QC2Status retVal = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (mHwMonitor) {
        if ((retVal = checkHwMonitorSuspend()) != QC2_OK) {
            return retVal;
        }
    }

    if (inputs.size()) {
        std::lock_guard<std::mutex> nowLock(mPendingInputsMutex);
        //auto inputPack = std::make_shared<InputPackMsg>();
        for (auto input : inputs) {
            ++(*numInputsQueued);
            //Add input to Pending Queue
            mPendingInputs->inputs.emplace_back(input);
        }

        if (mPendingInputs->inputs.size() > 0) {
            auto e = std::make_shared<Event>(MSG_ID_QUEUE);
            e->info().put(MsgInfoKey::kInputs, mPendingInputs);
            mHandler->postAsync(e);
        }
    }
    return retVal;
}

QC2Status QC2AudioHwCodec::handleQueueInputs() {
    QC2Status status = QC2_OK;
    std::shared_ptr<InputPackMsg> inputPackTemp =
        std::make_shared<InputPackMsg>();
    {
        // lock only unsafe operation
        std::lock_guard<std::mutex> nowLock(mPendingInputsMutex);
        // move all the queued elements
        inputPackTemp->inputs.splice(inputPackTemp->inputs.cend(),
                                     mPendingInputs->inputs);
        mPendingInputs->inputs.clear();
    }

    if (inputPackTemp == nullptr) {
        QLOGE_INST("No inputs retrieved");
        return QC2_ERROR;
    }
    for (auto iter = inputPackTemp->inputs.begin();
         iter != inputPackTemp->inputs.end();) {
        auto input = *iter;
        if (input == nullptr) {
            QLOGE("%s: input is null, exiting.", __func__);
            return QC2_ERROR;
        }
        if (mHwQueueFull) {
            QLOGE("%s: DSP Queue full, exiting queuing buffers", __func__);
            continue;
        }
        if (isNonDataBuffer(input->mInput)) {
            status = handleNonDataBuffer(input->mInput);
            if (status != QC2_OK) {
                QLOGE("%s: Error handling non-data buffer", __func__);
                onError(status, "Error handling non-data buffer");
                return status;
            }
        } else {
            std::shared_ptr<QC2Buffer> outBuf;
            status = mLinearPool->allocate(&outBuf);
            if (status) {
                QLOGE_INST("Failed to queue output buffer");
                onError(status, "Buffer Allocation Error");
            }
            auto sendEmptyOutput = [&]() {
                outBuf->setInputIndex(input->mInput->inputIndex());
                outBuf->setTimestamp(input->mInput->timestamp());
                outBuf->setFlags(input->mInput->flags() | BufFlag::DISCARD);
                outBuf->linear().setRange(0, 0);
                signalOutputsDone(outBuf);
            };
            auto sendUnprocessedInput = [&]() {
                auto inBuf = std::make_shared<QC2AudioHwBuffer>();
                inBuf->input_frame_id = input->mInput->inputIndex();
                signalInputsDone(inBuf);
            };
            status = prepareAndQueueInputBuffer(input);
            if (status == QC2_OK && outBuf != nullptr) {
                logBuffer(outBuf, "Allocated");
                outBuf->setInputIndex(input->mInput->inputIndex());
                outBuf->setOutputIndex(mOutputBufIndex++);
                outBuf->setTimestamp(input->mInput->timestamp());
                outBuf->setFlags(input->mInput->flags());
                status = prepareAndQueueOutputBuffer(outBuf);
                if (status != QC2_OK) {
                    QLOGE(
                        "%s: Failed to queue output buffer to HW, returning "
                        "empty output",
                        __func__);
                    sendEmptyOutput();
                }
                if (input->mInput->flags() & BufFlag::EOS) {
                    QLOGV("%s: EOS buffer detected", __func__);
                    drain();
                }
            } else {
                // Sending Input buffer returned error from DSP
                // return empty output for input
                QLOGE(
                    "%s: Failed to queue input buffer, error returned from DSP",
                    __func__);
                sendEmptyOutput();
                sendUnprocessedInput();
            }
            QLOGV(
                "%s: codec session id:%u, Done processing 1 input buffer, "
                "remaining inputs %lu",
                __func__, mSessionId,
                (unsigned long)inputPackTemp->inputs.size());
        }
        iter = inputPackTemp->inputs.erase(iter);
    }
    return status;
}

QC2Status QC2AudioHwCodec::flush() {
    QC2Status ret = QC2_OK;
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (mHwMonitor) {
        if ((ret = checkHwMonitorSuspend()) != QC2_OK) {
            return ret;
        }
    }

    auto e = std::make_shared<Event>(MSG_ID_FLUSH);
    mFlushing = true;
    mHandler->postSyncAndWait(e);
    e->reply().get("status", reinterpret_cast<int32_t*>(&ret));
    return ret;
}

QC2Status QC2AudioHwCodec::handleFlush() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("Flush both write and read buffers");
    std::shared_ptr<InputPackMsg> inputPackTemp =
        std::make_shared<InputPackMsg>();
    {
        // lock only unsafe operation
        std::lock_guard<std::mutex> nowLock(mPendingInputsMutex);
        // move all the queued elements
        inputPackTemp->inputs.splice(inputPackTemp->inputs.cend(),
                                    mPendingInputs->inputs);
        mPendingInputs->inputs.clear();
    }

    // mForceFlush is set for codecs that need to flush unconditionally
    if (!mForceFlush && inputPackTemp->inputs.empty() && isPendingOutputsEmpty())
        return QC2_OK;

    //Flush all intermediate buffers in pending inputs list
    for (auto iter = inputPackTemp->inputs.begin(); iter != inputPackTemp->inputs.end(); ) {
        QLOGV("%s: Flushing inputs and outputs", __func__);
        auto input = *iter;
        std::shared_ptr<QC2Buffer> outBuf;
        retVal = mLinearPool->allocate(&outBuf);
        outBuf->setInputIndex(input->mInput->inputIndex());
        outBuf->setTimestamp(input->mInput->timestamp());
        outBuf->setFlags(input->mInput->flags() | BufFlag::DISCARD);
        outBuf->linear().setRange(0, 0);
        signalOutputsDone(outBuf);

        auto inBuf = std::make_shared<QC2AudioHwBuffer>();
        inBuf->input_frame_id = input->mInput->inputIndex();
        signalInputsDone(inBuf);
        //Clear input from pending list once its ACKed
        iter = inputPackTemp->inputs.erase(iter);
    }
    // flush both input and output buffers
    retVal = mAudioHw->flush(mHwSessionId);
    if (retVal != QC2_OK)
        return retVal;
    retVal = onFlush();
    return retVal;
}

QC2Status QC2AudioHwCodec::drain() {
    QC2Status retVal = QC2_OK;
    if (!mHandler || !mEventQueue) {
        return QC2_BAD_STATE;
    }
    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (mHwMonitor) {
        if ((retVal = checkHwMonitorSuspend()) != QC2_OK) {
            return retVal;
        }
    }

    auto e = std::make_shared<Event>(MSG_ID_DRAIN);
    mHandler->postAsync(e);
    e->reply().get("status", reinterpret_cast<int32_t*>(&retVal));
    return retVal;
}

QC2Status QC2AudioHwCodec::handleDrain() {
    QLOGD_INST("Drain");
    QC2Status retVal = QC2_OK;
    //Send an extra Output buffer to enable DSP to return EOS ACK
    std::shared_ptr<QC2Buffer> outBuf;
    retVal = mLinearPool->allocate(&outBuf);
    if (!retVal) {
        outBuf->setInputIndex(0);
        outBuf->setOutputIndex(mOutputBufIndex++);
        outBuf->setFlags(BufFlag::EOS);
        prepareAndQueueOutputBuffer(outBuf);
        retVal = mAudioHw->drain(mHwSessionId);
    } else
        QLOGE("%s:Error allocating EOS ACK read buffer %u", __func__, retVal);

    //RETURN_IF_ERROR(retVal, "Sending stop command to driver while drain failed");
    return retVal;
}

QC2Status QC2AudioHwCodec::suspend() {
    QC2Status retVal = QC2_OK;

    if (getHwState() == HW_DOWN) {
        return QC2_CORRUPTED;
    }
    if (mSuspended) {
        QLOGI("%s: HW already suspended", __func__);
        return QC2_OK;
    }
    auto e = std::make_shared<Event>(MSG_ID_SUSPEND);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", reinterpret_cast<int32_t*>(&retVal));

    return retVal;
}

QC2Status QC2AudioHwCodec::handleSuspend() {
    QLOGD_INST("Suspend");
    QC2Status retVal = QC2_OK;

    retVal = mAudioHw->suspend_session(mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("%s: HW suspend failed", __func__);
        return retVal;
    }
    mSuspended = true;
    return retVal;
}

QC2Status QC2AudioHwCodec::resume() {
    QC2Status retVal = QC2_OK;

    if (getHwState() == HW_DOWN) {
        return QC2_CORRUPTED;
    }
    if (!mSuspended) {
        QLOGI("%s: Resume not required when HW is not suspended", __func__);
        return QC2_OK;
    }
    auto e = std::make_shared<Event>(MSG_ID_RESUME);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", reinterpret_cast<int32_t*>(&retVal));

    if (mHwMonitor) {
        mHwMonitor->reset();
    }
    return retVal;
}

QC2Status QC2AudioHwCodec::handleResume() {
    QLOGD_INST("Resume");
    QC2Status retVal = QC2_OK;

    retVal = mAudioHw->resume_session(mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("%s: HW resume failed", __func__);
        return retVal;
    }
    mSuspended = false;
    return retVal;
}

QC2Status QC2AudioHwCodec::release() {
    if (getHwState() == HW_UP)
        deinit();
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::prepareAndQueueInputBuffer(const std::shared_ptr<InputBufferPack>& input) {
    QC2Status retVal = QC2_OK;
    QLOGV("%s: Enter", __func__);
    auto inBuf = std::make_shared<QC2AudioHwBuffer>();
    auto inBufSize = input->mInput->linear().capacity();
    auto inBufFilledLength = input->mInput->linear().size();
    auto inBufFd = input->mInput->linear().fd();
    auto inBufOffset = input->mInput->linear().offset();
    auto inBufTs = static_cast<int64_t>(input->mInput->timestamp());

    //Audio HW cannot process inputs until it receives atleast
    //one valid input buffer
    if (input->mInput->inputIndex() == 0 && inBufFilledLength == 0) {
        QLOGE("%s: Zero-sized first buffer detected. Returning empty output for empty input", __func__);
        return QC2_ERROR;
    }

    inBuf->offset = inBufOffset;
    inBuf->size = inBufSize;
    inBuf->filled_length = inBufFilledLength;
    inBuf->fd = inBufFd;
    inBuf->ts = inBufTs;
    inBuf->input_frame_id = input->mInput->inputIndex();
    inBuf->buf_type = BufferType_INPUT;
    inBuf->last_frame = ((input->mInput->flags() & BufFlag::EOS)? true: false);
    QLOGV("%s: offset=%u, size=%u, filled_length=%u, fd=%u, ts=%lld, id=%lu", __func__,
                            inBufOffset, inBufSize, inBufFilledLength, inBufFd, (long long)inBufTs, (unsigned long)input->mInput->inputIndex());
    if ((retVal = mAudioHw->write_data(mHwSessionId, inBuf)) == QC2_OK) {
        ++mNumQueuedInputFrames;
    }
    return retVal;
}

void QC2AudioHwCodec::logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGD_INST("%s %s", prefix, QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

QC2Status QC2AudioHwCodec::prepareAndQueueOutputBuffer(const std::shared_ptr<QC2Buffer>& output) {
    QC2Status retVal = QC2_OK;

    QLOGV("%s: Enter", __func__);
    auto outBufSize = output->linear().capacity();
    auto outBufFilledLength = output->linear().size();
    auto outBufFd = output->linear().fd();
    auto outBufOffset = output->linear().offset();

    std::unique_ptr<QC2Buffer::Mapping> bufMappingInfo = output->linear().map();
    if (bufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto basePtr = bufMappingInfo->baseRW();
    if (basePtr == nullptr) {
        QLOGE("%s: Base ptr is null for output buffer", __func__);
        return QC2_NO_MEMORY;
    }

    // Prepare a QC2AudioHWBuffer buffer and queue it to the QC2AudioHw
    auto outBuf = std::make_shared<QC2AudioHwBuffer>();
    outBuf->output_frame_id = output->outputIndex();
    outBuf->buffer = basePtr;
    outBuf->offset = outBufOffset;
    outBuf->size = outBufSize;
    outBuf->filled_length = outBufFilledLength;
    outBuf->fd = outBufFd;
    outBuf->buf_type = BufferType_OUTPUT;
    {
        std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
        mPendingOutputs.emplace(std::make_pair(output->inputIndex(), output));
    }
    retVal = mAudioHw->queue_read_buffer(mHwSessionId, outBuf);
    return retVal;
}

QC2Status QC2AudioHwCodec::init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);
    retVal = QC2AudioHwFactory::CreateHwInstance(mAudioHw);
    RETURN_IF_ERROR(retVal, "Error initializing audio hardware");
    mBufPoolProvider = std::move(bufPoolProvider);
    onInit();
    retVal = mAudioHw->init(this);
    if (retVal == QC2_OK){
        setHwState(HW_UP);
        createWorkerThread();
        retVal = startWorkerThread();
    }
    return retVal;
}

QC2Status QC2AudioHwCodec::deinit() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);
    setHwState(HW_DOWN);
    if (mHwSessionId) {
        retVal = mAudioHw->close_session(mHwSessionId);
        mHwSessionId = nullptr;
    }
    retVal = mAudioHw->deinit();
    retVal = onDeinit();    //clean internal common resources
    return retVal;
}

QC2Status QC2AudioHwCodec::onDeinit() {
    QLOGD_INST("%s", __func__);
    stopWorkerThread();
    //mErrorHandlerThread->signalError(QC2_OK, "Deinit called");
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::onCodecConfig(std::shared_ptr<QC2Buffer> input) {
    (void)input;
    QLOGD_INST("%s", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::onFlush() {
    QLOGD_INST("%s", __func__);
    return QC2_OK;
}

// QC2Status QC2AudioHwCodec::onInit() {
//     QLOGD_INST("%s", __func__);
//     return QC2_OK;
// }

// QC2Status QC2AudioHwCodec::onStart() {
//     QLOGD_INST("%s", __func__);
//     return mAudioHw->start_session(mHwSessionId.get());
// }

QC2Status QC2AudioHwCodec::handleStart() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    return retVal;
}

QC2Status QC2AudioHwCodec::handleStop() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    retVal = mAudioHw->stop_session(mHwSessionId);
    retVal = mAudioHw->close_session(mHwSessionId);
    mHwSessionId = nullptr;
    return retVal;
}

QC2Status QC2AudioHwCodec::handleNonDataBuffer(std::shared_ptr<QC2Buffer> input) {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    if (input->flags() & BufFlag::CODEC_CONFIG) {    // CSD in input frame
        QLOGV("%s: CSD buffer detected", __func__);
        retVal = onCodecConfig(input);
    } else if ((input->flags() & BufFlag::EOS) &&
                (input->linear().size() == 0)) {  // EMPTY EOS input
        QLOGV("%s: Empty EOS buffer detected", __func__);
        retVal = drain();
    }
    if (retVal != QC2_OK)
        return retVal;

    // Return input buffer as is
    auto inHwBuf = std::make_shared<QC2AudioHwBuffer>();
    inHwBuf->size = input->linear().capacity();
    inHwBuf->filled_length = input->linear().size();
    inHwBuf->offset = input->linear().offset();
    inHwBuf->fd = input->linear().fd();
    inHwBuf->input_frame_id = input->inputIndex();
    inHwBuf->buf_type = BufferType_INPUT;

    std::shared_ptr<QC2Buffer> outBuf;
    retVal = mLinearPool->allocate(&outBuf);
    if (retVal == QC2_OK && outBuf != nullptr) {
        logBuffer(outBuf, "Allocated");
    } else {
        QLOGE_INST("Failed to queue output buffer");
        onError(retVal,"Buffer Allocation Error");
        return QC2_ERROR;
    }
    outBuf->setInputIndex(input->inputIndex());
    outBuf->setOutputIndex(mOutputBufIndex++);
    outBuf->setFlags(input->flags());

    // Return empty output buffer
    auto outHwBuf = std::make_shared<QC2AudioHwBuffer>();
    outHwBuf->size = 0;
    outHwBuf->filled_length = 0;
    outHwBuf->offset = 0;
    outHwBuf->input_frame_id = outBuf->inputIndex();
    outHwBuf->output_frame_id = outBuf->outputIndex();
    outHwBuf->buf_type = BufferType_OUTPUT;

    {
        std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
        mPendingOutputs.emplace(std::make_pair(outBuf->inputIndex(), outBuf));
    }

    onInputsDone(inHwBuf);
    onOutputsDone(outHwBuf);
    return QC2_OK;
}

void QC2AudioHwCodec::signalInputsDone(
                        std::shared_ptr<QC2AudioHwBuffer> inBuf) {
    std::shared_ptr<QC2Codec::InputBuffersDoneInfo> inBufsDone =
                                            std::make_shared<QC2Codec::InputBuffersDoneInfo>();

    QLOGV("%s: Enter", __func__);
    inBufsDone->mInputFrameIds.emplace_back(inBuf->input_frame_id);
    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, inBufsDone);
    mListener->postAsync(e);
}

void QC2AudioHwCodec::signalOutputsDone(
                                    std::shared_ptr<QC2Buffer> outputBuffer) {
    QLOGV("%s: Enter", __func__);
    QC2Codec::OutputBufferPack outBufPack(outputBuffer);
    if (mUpdatedInfos.size()) {
        for (auto& info: mUpdatedInfos) {
             if (!info) {
                 continue;
             }
             outBufPack.mConfigUpdates.push_back(C2Param::Copy(*info));
        }
    }
    auto outBufs = std::make_shared<QC2Codec::OutputBuffersDoneInfo>();
    outBufs->mOutputs.emplace_back(std::move(outBufPack));
    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, outBufs);
    mListener->postAsync(e);
}

void QC2AudioHwCodec::onInputsDone(
        std::shared_ptr<QC2AudioHwBuffer> inBuf) {
    QLOGV("%s: Enter", __func__);
    if (mNumQueuedInputFrames)
        --mNumQueuedInputFrames;
    signalInputsDone(inBuf);
    mHwQueueFull = false;
}

void QC2AudioHwCodec::onOutputsDone(
        std::shared_ptr<QC2AudioHwBuffer> outBuf) {
    QLOGV("%s: Enter", __func__);

    {
        std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
        auto outputPair = mPendingOutputs.find(outBuf->input_frame_id);
        if (outputPair == mPendingOutputs.end()) {
            QLOGE("%s: Invalid output returned from HW, output frame id = %llu. Dropping output",
                                         __func__, (unsigned long long)outBuf->output_frame_id);
            return;
        }

        auto output = outputPair->second;
        if (mKind == KIND_DECODER) { // Set timestamp returned from DSP only for decoders
            output->setTimestamp(outBuf->ts);
        }
        output->linear().setRange(outBuf->offset, outBuf->filled_length);
        QLOGV("%s: Output received for input %lu, size %u", __func__,
                                                    (unsigned long)outBuf->input_frame_id,
                                                    (unsigned int)outBuf->filled_length);
        if (mUpdatedInfos.size()) {
            QLOGD_INST("ConfigUpdate: sending %zu updates", mUpdatedInfos.size());
            //Ensure Component Interface is also updated with latest param values
            onConfigUpdate(mUpdatedInfos);
        }

        //Signal to component that output is ready
        signalOutputsDone(output);
        mUpdatedInfos.clear();
        mPendingOutputs.erase(outputPair);
    }

    if (mHwMonitor && !getPendingOutputsSize() && !getNumQueuedInputFrames()) {
        QLOGV("%s: All inputs and outputs have been processed, notify monitor",
                  __func__);
        mHwMonitor->notify();
    }
}

 void QC2AudioHwCodec::onConfigUpdate(std::list<std::shared_ptr<C2Param>> targetConfig) {
    if (targetConfig.empty()) {
        QLOGE("%s: Empty target config received", __func__);
    }
    auto updatedParams = std::make_shared<QC2Codec::ConfigUpdateInfo>();
    updatedParams->mTargetConfig = targetConfig;
    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_CONFIG_UPDATE);
    e->info().put(QC2Codec::NotifyKey::kConfigUpdate, updatedParams);
    mListener->postAsync(e);
 }

void QC2AudioHwCodec::clearUnAckedOutputs() {
    QLOGV("%s: Enter", __func__);
    //Return blank outputs
    if (getPendingOutputsSize() > 0) {
        std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
        for (auto const& itOutput : mPendingOutputs) {
            auto output = itOutput.second;
            output->setFlags(output->flags() | BufFlag::DISCARD);
            output->linear().setRange(0,0);
            signalOutputsDone(output);
        }
    }
    mPendingOutputs.clear();
}

void QC2AudioHwCodec::onError(QC2Status errorCode, const std::string& errorMsg) {
    mErrorHandlerThread->signalError(errorCode, errorMsg);
}

bool QC2AudioHwCodec::isNonDataBuffer(const std::shared_ptr<QC2Buffer> input) {
        return input->flags() & BufFlag::CODEC_CONFIG;
}

QC2Status QC2AudioHwCodec::handleMessage(std::shared_ptr<Event> event) {
    QC2Status status = QC2_OK;
    if (!event) {
        return QC2_BAD_ARG;
    }

    switch (event->id()) {
    case MSG_ID_QUEUE: {
        std::shared_ptr<InputPackMsg> dummyPendingInputs = nullptr;
        event->info().get(MsgInfoKey::kInputs, &dummyPendingInputs);
        dummyPendingInputs=nullptr;
        status = handleQueueInputs();
        break;
    }

    case MSG_ID_FLUSH: {
        status = handleFlush();
        mFlushing = false;
        QLOGV("%s: Flushed all inputs and outputs", __func__);
        break;
    }

    case MSG_ID_DRAIN: {
        /* Drain (with or without EOS) is a NO-OP as we handle
           all events in event queue sequentially */
        status = handleDrain();
        if (status != QC2_OK)
            QLOGE("%s: Error processing drain", __func__);
        QLOGV("%s: Issued Drain command", __func__);
        break;
    }

    case MSG_ID_SUSPEND: {
        status = handleSuspend();
        QLOGV("%s: Issued suspend command", __func__);
        break;
    }

    case MSG_ID_RESUME: {
        status = handleResume();
        QLOGV("%s: Issued resume command", __func__);
        break;
    }

    default:
        break;
    }
    event->reply().put("status", status);
    return status;
}

size_t QC2AudioHwCodec::getPendingInputsSize() {
    // Returning input size since at present there is 1:1 correlation
    // between inputs and outputs for hardware codecs
    std::lock_guard<std::mutex> inputLock(mPendingInputsMutex);
    return mPendingInputs->inputs.size();
}

size_t QC2AudioHwCodec::getPendingOutputsSize() {
    // Returning input size since at present there is 1:1 correlation
    // between inputs and outputs for hardware codecs
    std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
    return mPendingOutputs.size();
}

uint64_t QC2AudioHwCodec::getNumQueuedInputFrames() {
    // Returning number of queued input frames to hardware
    return mNumQueuedInputFrames.load(std::memory_order_relaxed);
}

bool QC2AudioHwCodec::isPendingInputsEmpty() {
    std::lock_guard<std::mutex> inputLock(mPendingInputsMutex);
    return mPendingInputs->inputs.empty();
}

bool QC2AudioHwCodec::isPendingOutputsEmpty() {
    std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
    return mPendingOutputs.empty();
}


QC2Status QC2AudioHwCodec::startEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (!mHandler) {
        mHandler = std::make_shared<QC2AudioHwCodec::Handler>(*this);
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

QC2Status QC2AudioHwCodec::stopEventQueue() {
    std::lock_guard<std::mutex> lock(mHandlerLock);
    if (mEventQueue) {
        mEventQueue->stop();
        mEventQueue = nullptr;
    }
    mHandler = nullptr;
    return QC2_OK;
}

QC2Status QC2AudioHwCodec::returnPendingInputs() {
    return QC2_OK;
}

void QC2AudioHwCodec::createWorkerThread() {
    mErrorHandlerThread = std::make_unique<QC2AudioHwCodecWorkerThread>("HwCodecThread", this);
}

QC2Status QC2AudioHwCodec::startWorkerThread() {
    if (!mErrorHandlerThread)
        return QC2_NO_INIT;
    return mErrorHandlerThread->startThread();
}

QC2Status QC2AudioHwCodec::stopWorkerThread() {
    if (!mErrorHandlerThread)
        return QC2_NO_INIT;
    return mErrorHandlerThread->stopThread();
}

///////////////////////////////////////////////////////////////////////////////////////////////
//                             QC2AudioHwCodecWorkerThread                                   //
///////////////////////////////////////////////////////////////////////////////////////////////
#undef LOG_TAG
#define LOG_TAG "QC2AudioHwCodecWorkerThread"
QC2AudioHwCodecWorkerThread::QC2AudioHwCodecWorkerThread(const std::string& name,
    QC2AudioHwCodec *codec)
    :QC2Thread("Worker_"+ name),
     mCodec(*codec),
     mName(name),
     mErrorCode(QC2_OK) {
}

QC2AudioHwCodecWorkerThread::~QC2AudioHwCodecWorkerThread() {
    if (isThreadRunning()) {
        QLOGW("HwCodecWorkerThread thread is still running");
        stopThread();
    }
}

QC2Status QC2AudioHwCodecWorkerThread::stopThread(){
    requestStopThread();
    QLOGI("%s: Stopping HwCodec Worker Thread ", __func__);
    signalError(QC2_OK, "Deinit called");
    return joinThread();
}

void QC2AudioHwCodecWorkerThread::handleSignalledError(QC2Status errorCode,
                             std::string& errorMsg) {
    std::shared_ptr<QC2Buffer> outBuf;
    QC2Status status = QC2_OK;
    QLOGE("%s: Received error code=0x%x", __func__, errorCode);
    if (errorCode == QC2_CORRUPTED) {
        mCodec.setHwState(QC2AudioHwCodec::HW_DOWN);
        mCodec.stopEventQueue();
        std::shared_ptr<QC2AudioHwCodec::InputPackMsg> inputPackTemp =
            std::make_shared<QC2AudioHwCodec::InputPackMsg>();
        {
            // lock only unsafe operation
            std::lock_guard<std::mutex> nowLock(mCodec.mPendingInputsMutex);
            // move all the queued elements
            inputPackTemp->inputs.splice(inputPackTemp->inputs.cend(),
                                        mCodec.mPendingInputs->inputs);
            mCodec.mPendingInputs->inputs.clear();
        }
        if (inputPackTemp->inputs.size() > 0) {
            for (auto it = inputPackTemp->inputs.begin();
                            it != inputPackTemp->inputs.end();) {
                auto input = *it;
                auto inBuf = std::make_shared<QC2AudioHwBuffer>();
                inBuf->input_frame_id = input->mInput->inputIndex();
                mCodec.signalInputsDone(inBuf);

                status = mCodec.mLinearPool->allocate(&outBuf);
                if (status == QC2_OK) {
                    outBuf->setInputIndex(input->mInput->inputIndex());
                    outBuf->setTimestamp(input->mInput->timestamp());
                    outBuf->setFlags(input->mInput->flags() | BufFlag::DISCARD);
                    outBuf->linear().setRange(0,0);
                    mCodec.signalOutputsDone(outBuf);
                }
                it = inputPackTemp->inputs.erase(it);
            }
        }
        //return un-acked outputs
        mCodec.clearUnAckedOutputs();
        QLOGE("%s: Done processing error code=0x%x", __func__, errorCode);
    }
    std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
    e->info().put(QC2Codec::NotifyKey::kErrorCode, errorCode);
    e->info().put(QC2Codec::NotifyKey::kErrorMsg, errorMsg);
    mCodec.mListener->postAsync(e);
}

void QC2AudioHwCodecWorkerThread::signalError(QC2Status errorCode,
                                        const std::string& errorMsg) {
    mErrorCode = errorCode;
    mErrorMsg = errorMsg;
    std::unique_lock<std::mutex> ul(mLock);
    mCondition.notify_one();
}

void QC2AudioHwCodecWorkerThread::threadLoop()  {

    while (!shouldExitThread()) {
        std::unique_lock<std::mutex> ul(mLock);
        if (mCondition.wait_for(ul, 5s) == std::cv_status::no_timeout) {
            // QC2_OK indicates graceful exit, do nothing and exit
            QLOGE("%s: Waking up. Received error code=0x%x", __func__, mErrorCode);
            if (mErrorCode != QC2_OK) {
                //condition variable notification received, report Error
                handleSignalledError(mErrorCode, mErrorMsg);
            }
        }
    }
    QLOGE("%s: Exiting threadloop", __func__);
}


///////////////////////////////////////////////////////////////////////////////////////////////
//                              Codec Factories                                              //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwDecoderFactory::QC2AudioHwDecoderFactory(const std::string& name, const std::string& mime,
             const std::string& nickName, uint32_t variant)
        : mName(name), mNickName(nickName), mCompressedMime(mime), mVariant(variant) {
        }

QC2Status QC2AudioHwDecoderFactory::createCodec(uint32_t sessionId,
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
    if (!(strcasecmp(mCompressedMime.c_str(), MimeType::AAC)))
        *codec = std::make_unique<QC2AudioHwAacDec>(mName, sessionId, instName, mCompressedMime,
                                                    mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::AMR_WB_PLUS)) )
        //TODO: Replace with codec specific class
        *codec = std::make_unique<QC2AudioHwAmrWbPlusDec>(mName, sessionId, instName, mCompressedMime,
                                                          mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::ALAC)))
        *codec = std::make_unique<QC2AudioHwAlacDec>(mName, sessionId, instName, mCompressedMime,
                                                     mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::APE)))
        *codec = std::make_unique<QC2AudioHwApeDec>(mName, sessionId, instName, mCompressedMime,
                                                    mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::WMA)) ||
             !(strcasecmp(mCompressedMime.c_str(), MimeType::WMA_PRO)) ||
             !(strcasecmp(mCompressedMime.c_str(), MimeType::WMA_LOSSLESS)))
        *codec = std::make_unique<QC2AudioHwWmaDec>(mName, sessionId, instName, mCompressedMime,
                                                    mVariant, listener);

    if (codec == nullptr) {
        return QC2_BAD_ARG;
    }
    return QC2_OK;
}

std::string QC2AudioHwDecoderFactory::name() {
    return "QC2AudioHwDecoderFactory" + mName;
}


QC2AudioHwEncoderFactory::QC2AudioHwEncoderFactory(const std::string& name, const std::string& mime,
            const std::string& nickName, uint32_t variant)
    : mName(name), mNickName(nickName), mCompressedMime(mime), mVariant(variant) {
}

QC2Status QC2AudioHwEncoderFactory::createCodec(
        uint32_t sessionId,
        const std::string& upstreamMime,
        const std::string& downstreamMime,
        std::shared_ptr<EventHandler> listener,
        std::unique_ptr<QC2Codec> * const codec) {
    (void)upstreamMime;
    (void)downstreamMime;
    std::string instName = mNickName + "_" + std::to_string(sessionId);

    if (!(strcasecmp(mCompressedMime.c_str(), MimeType::AMR_NB)))
        *codec = std::make_unique<QC2AudioHwAmrNbWbEnc>(mName, sessionId, instName, mCompressedMime,
                                                        mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::AMR_WB)))
        *codec = std::make_unique<QC2AudioHwAmrNbWbEnc>(mName, sessionId, instName, mCompressedMime,
                                                        mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::AAC)) ||
             !(strcasecmp(mCompressedMime.c_str(), MimeType::AAC_ADTS)))
        *codec = std::make_unique<QC2AudioHwAacEnc>(mName, sessionId, instName, mCompressedMime,
                                                    mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::EVRC)))
        *codec = std::make_unique<QC2AudioHwEvrcEnc>(mName, sessionId, instName, mCompressedMime,
                                                     mVariant, listener);
    else if (!(strcasecmp(mCompressedMime.c_str(), MimeType::QCELP)))
        *codec = std::make_unique<QC2AudioHwQcelpEnc>(mName, sessionId, instName, mCompressedMime,
                                                     mVariant, listener);
    if (codec == nullptr) {
        return QC2_BAD_ARG;
    }

    return QC2_OK;
}

std::string QC2AudioHwEncoderFactory::name() {
    return "QC2AudioHwEncoderFactory" + mName;
}

};   // namespace qc2audio

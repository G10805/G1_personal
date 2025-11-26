/*
 *******************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#undef LOG_TAG
#define LOG_TAG "QC2Filter"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

#include "QC2Config.h"
#include "QC2Filter.h"
#include "QC2FilterCapsHelper.h"
#include "QC2FilterConstants.h"

namespace qc2 {

class FuncTracker {
 public:
    explicit FuncTracker(const char *debugStr):
    mDebugStr(debugStr) {
        QLOGI("FUNCTION_BEGIN:%s", mDebugStr.c_str());
        mStartTimeUs = OS::CurrentTimeUs();
    }

    ~FuncTracker() {
        uint64_t endTimeUs = OS::CurrentTimeUs();
        int diff = static_cast<int>((endTimeUs - mStartTimeUs)/1000);
        QLOGI("FUNCTION_END:%s TIME=%d ms", mDebugStr.c_str(), diff);
    }

 private:
    const std::string mDebugStr;
    uint64_t mStartTimeUs;
};

// BUFFER QUEUE
constexpr uint32_t FILTER_INP_BUF_Q_SIZE = 128;
constexpr uint32_t FILTER_OUT_BUF_Q_SIZE = 128;
constexpr uint32_t FILTER_EXTRADATA_BUFFER_SIZE = 4096;

#define ALIGNED_WIDTH  QC2Buffer::Graphic::GetAlignedWidth
#define ALIGNED_HEIGHT QC2Buffer::Graphic::GetAlignedHeight

//------------------------------------------------------------------------------
// QC2Filter helper APIs
//------------------------------------------------------------------------------
void QC2Filter::logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGD_INST("%s %s", prefix, QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

int QC2Filter::extractFD(const std::shared_ptr<QC2Buffer>& qc2Buf) {
    if(qc2Buf && qc2Buf->isGraphic()) {
        const auto& g = qc2Buf->graphic();
        return g.fd();
    }
    return -1;
}

QC2Status QC2Filter::updateFilterOperatingMode(FilterMode mode) {
    QLOGI_INST("Runtime update filter operating mode [%d]", mode);

    switch(mode) {
        case FILTER_MODE_INPLACE:
        case FILTER_MODE_NORMAL:
        case FILTER_MODE_BYPASS:
            mFilterMode = mode;
            break;

        default:
            QLOGE_INST("Incorrect mode requested. Push filter in bypass mode");
            mFilterMode = FILTER_MODE_BYPASS;
            break;
    }
    mbUpdateDelays = true;
    return QC2_OK;
}

QC2Status QC2Filter::addOutputConfigUpdates(std::unique_ptr<C2Param>& param) {
    QLOGI_INST("addOutputConfigUpdates: Add custom config update");
    mUpdatedInfos.push_back(std::move(param));
    return QC2_OK;
}

//------------------------------------------------------------------------------
// QC2Filter CTOR/DTOR
//------------------------------------------------------------------------------
QC2Filter::QC2Filter(const std::string& name,
                     const uint32_t sessionId,
                     const std::string& instName,
                     const std::string& inputMime,
                     const std::string & outputMime,
                     ComponentKind kind,
                     uint32_t variant,
                     std::shared_ptr<EventHandler> listener)
                     : QC2Codec(name, sessionId, inputMime, outputMime, kind, variant, listener),
                       mInstName(instName) {
    FuncTracker track(__func__);

    QLOGD_INST("Create buffer lists to hold references of buffers");

    // Create buffer list to hold references of queued buffers
    mIpBufList = std::make_shared<QC2FilterBufferList>("InputList", FILTER_INP_BUF_Q_SIZE);
    mOpBufList = std::make_shared<QC2FilterBufferList>("OutputList", FILTER_OUT_BUF_Q_SIZE);

    // Create buffer list to hold references of metadata infos
    mIpMetadataBufList = std::make_shared<QC2FilterBufferList>("IpMetadataBufList",
                                                               FILTER_INP_BUF_Q_SIZE);
    mOpMetadataBufList = std::make_shared<QC2FilterBufferList>("OpMetadataBufList",
                                                               FILTER_OUT_BUF_Q_SIZE);

    if (!mIpBufList || !mOpBufList) {
        QLOGE_INST("Cannot allocate memory for InputList or OutputList");
        throw QC2_NO_MEMORY;
    }

    if (!mIpMetadataBufList || !mOpMetadataBufList) {
        QLOGE_INST("Cannot allocate memory for IpMetadaBufList or OpMetadataBuflist");
        throw QC2_NO_MEMORY;
    }

    mFilterMode = FILTER_MODE_NORMAL;
    mState = STATE_FILTER_CREATED;
    mbUpdateDelays = true;
}

QC2Filter::~QC2Filter() {
    FuncTracker track(__func__);

    QC2Status retval;

    QLOGD_INST("Call release");
    retval = release();
    if (retval != QC2_OK) {
        QLOGE_INST("release returned error=%d", retval);
    }

    QLOGD_INST("Stop handlers and queues");
    if (mHandler && mEventQueue) {
        stopEventQueue();
    }
}

//------------------------------------------------------------------------------
// QC2Filter QC2Codec APIs implementation
//------------------------------------------------------------------------------
// This is a blocking call. Hence handled in the same context
QC2Status QC2Filter::init( std::unique_ptr<BufferPoolProvider> bufPoolProvider) {
    FuncTracker track(__func__);

    QC2Status retval;
    mBufPoolProvider = std::move(bufPoolProvider);

    // Handle init
    QLOGD_INST("calling handleInit()");

    retval = handleInit();

    mState = STATE_FILTER_INITIALIZED;
    return retval;
}

QC2Status QC2Filter::release() {
    FuncTracker track(__func__);

    QC2Status retval;

    if (mState == STATE_FILTER_CREATED ||
       mState == STATE_FILTER_INITIALIZED) {
        return QC2_OK;
    }


    // First stop
    QLOGD_INST("calling stop()");
    retval = stop();

    if (retval != QC2_OK) {
        QLOGE_INST("stop returned error=%d", retval);
        return retval;
    }

    // Handle release
    QLOGD_INST("calling handleRelease()");
    retval = handleRelease();
    if (retval != QC2_OK) {
        QLOGE_INST("stop returned error=%d", retval);
    }

    return retval;
}

QC2Status QC2Filter::getCapsHelpers(std::unordered_map<uint32_t,
                                    std::shared_ptr<QC2ParamCapsHelper >>* helpers) const {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper >> baseHelpers;

    // 1. Generate the cap helpers from base class
    if (true) {
        for (auto&f : QC2FilterCapsHelper::getFilterCapsHelpers()) {
            auto b = f(mCodecName, mInputMime, mOutputMime, mVariant, mKind, nullptr);
            if (b->isSupported()) {
                baseHelpers.emplace(b->getIndex(), b);
                QLOGD_INST("%s from base registered", DebugString::C2Param(b->getIndex()).c_str());
            }
        }
    }

    if (helpers) {
        // 2. Now query the cap helpers from custom filter
        retval = onGetCapsHelpers(helpers);

        if (retval != QC2_OK) {
            QLOGE_INST("onGetCapsHelpers failed. Populating only base caps helpers");
            // Not a potential error since we would still populate and advertise
            // base caps helpers
            retval = QC2_OK;
        }

        // 3 Now that "helpers" has all the caps populated from custom filter, we'll
        // now merge them with baseHelpers. Rule is to let caps from custom
        // filters override base caps helpers. Hence we can simply use insert() to
        // achieve this as it would insert only unique keys from baseHelpers and
        // discard if the key is already present in helpers
        helpers->insert(baseHelpers.begin(), baseHelpers.end());
    }
    return retval;
}

QC2Status QC2Filter::configure(const C2Param & param) {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    QLOGE_INST("configure: %s core-index=%x, baseIndex=%x",
        DebugString::C2Param(param.index()).c_str(), param.coreIndex().coreIndex(),
        param.index());

    switch (param.coreIndex().typeIndex()) {
        // Image resolution for Input and Output
        case kParamIndexPictureSize: {
            QLOGD_INST("configure: INDEX: PICTURE SIZE");
            C2StreamPictureSizeInfo::output o;
            C2StreamPictureSizeInfo::input i;

            if (param.index() == o.index()) {
                QLOGD_INST("configure: PICTURE_SIZE for output");
                retval = configureParam((C2StreamPictureSizeInfo::output*)&param);
                if (retval != QC2_OK) {
                    QLOGE_INST("configure[C2StreamPictureSizeInfo::output] failed %d", retval);
                }
            } else if (param.index() == i.index()) {
            QLOGD_INST("configure: PICTURE_SIZE for input");
                retval = configureParam((C2StreamPictureSizeInfo::input*)&param);
                if (retval != QC2_OK) {
                    QLOGE_INST("configure[C2StreamPictureSizeInfo::input] failed %d", retval);
                }
            }
            break;
        }

        // Pixel Format for Input and Output
        case kParamIndexPixelFormat: {
            C2StreamPixelFormatInfo::input iFmt;
            C2StreamPixelFormatInfo::output oFmt;
            QLOGD_INST("configure: INDEX: PIX FORMAT");

            if (param.index() == oFmt.index()) {
                QLOGD_INST("configure: PICTURE_FMT for output");
                retval = configureParam((C2StreamPixelFormatInfo::output*)&param);
                if (retval != QC2_OK) {
                    QLOGE_INST("configure[C2StreamPixelFormatInfo::output] failed %d", retval);
                }
            } else if (param.index() == iFmt.index()) {
                QLOGD_INST("configure: PICTURE_FMT for input");
                retval = configureParam((C2StreamPixelFormatInfo::input*)&param);
                if (retval != QC2_OK) {
                    QLOGE_INST("configure[C2StreamPixelFormatInfo::input] failed %d", retval);
                }
            }
            break;
        }

        // Output Graphic Buffer Pool
        case kParamIndexBlockPools: {
            QLOGD_INST("configure: INDEX: BLOCK POOLS");
            retval = configureParam((C2PortBlockPoolsTuning::output*)&param);
            if (retval != QC2_OK) {
                QLOGE_INST("configure[C2PortBlockPoolsTuning] failed %d", retval);
            }
            break;
        }

        // Secure Mode for component
        case kParamIndexSecureMode: {
            retval = configureParam((C2SecureModeTuning *)&param);
            if (retval != QC2_OK) {
                QLOGE_INST("configure[C2SecureModeTuning] failed:%d", retval);
            }
            break;
        }

        // Incase there is no handling of certain C2Param, check if the conf
        // can be handled by the custom filter implmentation
        default: {
            QLOGE_INST("configure: param not handled. call onConfigure");
            retval = onConfigure(param);
            if (retval != QC2_OK) {
                QLOGE_INST("onConfigure failed %d", retval);
            }
            break;
        }
    }

    return retval;
}

QC2Status QC2Filter::queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const {
    (void)params;
    return QC2_OMITTED;
}

QC2Status QC2Filter::query(C2Param * const param) {
    FuncTracker track(__func__);

  (void)param;
    QC2Status retval = QC2_OK;
    // DRUNWAL: TODO. Not implmented
    return retval;
}

QC2Status QC2Filter::start() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    // Start the Event Q
    QLOGD_INST("calling starEventQueue()");
    retval = startEventQueue();

    if (!mHandler || !mEventQueue || retval != QC2_OK) {
        QLOGE_INST("Incorrect/Bad state to proccess start");
        return QC2_BAD_STATE;
    }

    // sync call
    int32_t status;
    QLOGD_INST("Post FILTER_START event");
    auto e = std::make_shared<Event>(MSG_ID_FILTER_START);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", &status);

    retval = (QC2Status)status;

    return retval;
}

QC2Status QC2Filter::stop() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    if (!mHandler || !mEventQueue) {
        QLOGE_INST("Incorrect/Bad state to process stop");
        return QC2_BAD_STATE;
    }

    // sync call
    int32_t status;
    QLOGD_INST("Post FILTER_STOP event");
    auto e = std::make_shared<Event>(MSG_ID_FILTER_STOP);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", &status);
    retval = (QC2Status)status;

    if (retval != QC2_OK) {
        QLOGE_INST("Bad status on post FILTER_STOP event");
        return QC2_BAD_STATE;
    }

    // stop the event Q so that no further messages are process
    QLOGD_INST("calling stopEventQueue()");
    retval = stopEventQueue();

    return retval;
}

QC2Status QC2Filter::flush() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    if (!mHandler || !mEventQueue) {
        QLOGE_INST("Incorrect/Bad state to process flush");
        return QC2_BAD_STATE;
    }

    mFlushing = true;

    // sync call
    int32_t status;
    QLOGD_INST("Post FILTER_FLUSH event");
    auto e = std::make_shared<Event>(MSG_ID_FILTER_FLUSH);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", &status);
    retval = (QC2Status)status;

    if (retval != QC2_OK) {
        QLOGE_INST("Bad status on post FILTER_FLUSH event");
        return QC2_BAD_STATE;
    }

    mFlushing = false;

    return retval;
}

QC2Status QC2Filter::drain() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    if (!mHandler || !mEventQueue) {
        QLOGE_INST("Incorrect/Bad state to process drain");
        return QC2_BAD_STATE;
    }

    // async call
    int32_t status;
    QLOGD_INST("Post FILTER_DRAIN Event");
    auto e = std::make_shared<Event>(MSG_ID_FILTER_DRAIN);
    mHandler->postAsync(e);
    e->reply().get("status", &status);
    retval = (QC2Status)status;

    if (retval != QC2_OK) {
        QLOGE_INST("Bad status on post FILTER_DRAIN event");
        return QC2_BAD_STATE;
    }

    return retval;
}

QC2Status QC2Filter::reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    if (!mHandler || !mEventQueue) {
        QLOGE_INST("Incorrect/Bad state to process reconfigure");
        return QC2_BAD_STATE;
    }

    auto reconfInfo = std::make_shared<QC2Codec::ReconfigParamsInfo>();
    reconfInfo->mTargetConfig = std::move(targetConfig);

    // sync call
    int32_t status;
    QLOGD_INST("Post FILTER_RECONFIGURE event");
    auto e = std::make_shared<Event>(MSG_ID_FILTER_RECONFIGURE);
    e->info().put(MsgInfoKey::kReconfig, reconfInfo);
    mHandler->postSyncAndWait(e);
    e->reply().get("status", &status);
    retval = (QC2Status)status;

    return retval;
}

QC2Status QC2Filter::queueInputs(const std::list<std::shared_ptr<InputBufferPack>>& inputs,
                                 ReportConfigFailure_cb configFailureReporter,
                                 uint32_t *numInputsQueued) {
    FuncTracker track(__func__);

  (void)configFailureReporter;
    QC2Status retval = QC2_OK;

    if (numInputsQueued == nullptr) {
        QLOGE_INST("Invalid arg: numInputQueued is NULL");
        return QC2_BAD_ARG;
    }

    *numInputsQueued = 0;

    if (!mHandler || !mEventQueue) {
        QLOGE_INST("mHandler/mEventQueue not initialized");
        return QC2_BAD_STATE;
    }

    QLOGD_INST("Size of input list received: %zu", inputs.size());

    // TODO(DRUNWAL): Tunings are not handled yet!

    if (inputs.size()) {
        auto inputPack = std::make_shared<InputPackMsg>();
        for (auto input : inputs) {
            if (!input->mInput->isGraphic()) {
                QLOGE_INST("Received non-graphic buffer as input, cannot process");
                continue;
            }

            // Check if the buffer parameters (h/w/f) are proper and/or changed
            auto ret = checkBufferParams(input->mInput, FILTER_PORT_INPUT);
            if (ret != QC2_OK) {
                // Resolution or format has changed
                QLOGD_INST("Buffer Params changed");
                // 1. Queue all the buffers till now that were compatible
                if (inputPack->inputs.size() > 0) {
                    auto e = std::make_shared<Event>(MSG_ID_FILTER_QUEUE_INPUT);
                    e->info().put(MsgInfoKey::kInputs, inputPack);
                    mHandler->postAsync(e);
                }

                // 2. Extract the reconfigure parameters info from buffer
                QLOGD_INST("Constructing reconfig param info");
                auto reconfigInfo = std::make_shared<QC2Codec::ReconfigParamsInfo>();
                getTargetReconfigInfo(input->mInput, reconfigInfo);

                // 3. Reconfigure
                QLOGD_INST("Call reconfigure with the new params");
                retval = reconfigure(std::move(reconfigInfo->mTargetConfig));
                if (retval == QC2_OK) {
                    inputPack->inputs.clear();
                } else {
                    QLOGE_INST("Failed to reconfigure %u", retval);
                    // TODO(DRUNWAL): What to do in this case / bypass?
                    continue;
                }
            }
            ++(*numInputsQueued);
            inputPack->inputs.push_back(input);
        }

        QLOGD_INST("Post FILTER_QUEUE_INPUT event");
        if (inputPack->inputs.size() > 0) {
            auto e = std::make_shared<Event>(MSG_ID_FILTER_QUEUE_INPUT);
            e->info().put(MsgInfoKey::kInputs, inputPack);
            mHandler->postAsync(e);
        }
    }
    return retval;
}

//------------------------------------------------------------------------------
// QC2Filter APIs from EventQ Handler
//------------------------------------------------------------------------------
QC2Status QC2Filter::handleMessage(std::shared_ptr <Event> event) {
    QC2Status retval = QC2_OK;

    if (!event) {
        QLOGE_INST("No event passed to handleMessage");
        return QC2_BAD_ARG;
    }

    switch (event->id()) {
        case MSG_ID_FILTER_START: {
            QLOGD_INST("MSG_ID_FILTER_START");
            retval = handleStart();
            if (retval != QC2_OK) {
                QLOGE_INST("Error during handleStart() %d", retval);
            }
            break;
        }

        case MSG_ID_FILTER_STOP: {
            QLOGD_INST("MSG_ID_FILTER_STOP");
            retval = handleStop();
            if (retval != QC2_OK) {
                QLOGE_INST("Error during handleStop() %d", retval);
            }
            break;
        }

        case MSG_ID_FILTER_QUEUE_INPUT: {
            QLOGD_INST("MSG_ID_FILTER_QUEUE_INPUT");
            std::shared_ptr<InputPackMsg> inputPack = nullptr;
            event->info().get(MsgInfoKey::kInputs, &inputPack);
            if (inputPack != nullptr) {
                QLOGD_INST("Populate input pack");
                for (auto itr = inputPack->inputs.begin(); itr != inputPack->inputs.end();) {
                    mPendingInputs.push_back(*itr);
                    itr = inputPack->inputs.erase(itr);
                }
            }

            retval = handleQueueInputs(&mPendingInputs);
            if (retval != QC2_OK) {
                QLOGE_INST("Failed to handle queue input buffers");
                retval = returnPendingInputs();
            }
            break;
        }

        case MSG_ID_FILTER_FLUSH: {
            QLOGD_INST("MSG_ID_FILTER_FLUSH");
            // Flush all the buffers submitted to custom filter and then return
            // all the pending inputs
            retval = handleFlush();
            if (retval != QC2_OK) {
                QLOGE_INST("Failed to  handle flush");
            }

            retval = returnPendingInputs();
            break;
        }

        case MSG_ID_FILTER_DRAIN: {
            QLOGD_INST("MSG_ID_FILTER_DRAIN");
            // Add EOS in last pending input buffer, signal drain otherwise
            if (mPendingInputs.size() && mPendingInputs.back()->mInput != nullptr) {
                auto& lastInput = mPendingInputs.back()->mInput;
                lastInput->setFlags(lastInput->flags() | BufFlag::EOS);
            } else {
                retval = handleDrain();
            }
            break;
        }

        case MSG_ID_FILTER_RECONFIGURE: {
            QLOGD_INST("MSG_ID_FILTER_RECONFIGURE");
            std::shared_ptr<QC2Codec::ReconfigParamsInfo> reconfigInfo = nullptr;
            event->info().get(MsgInfoKey::kReconfig, &reconfigInfo);
            if (reconfigInfo == nullptr) {
                QLOGE_INST("No reconfig info present in event");
                return QC2_ERROR;
            }
            retval = handleReconfigure(std::move(reconfigInfo->mTargetConfig));
            if (retval != QC2_OK) {
                QLOGE_INST("Failure in handle reconfigure %d", retval);
            }
            break;
        }

        default: {
            QLOGE_INST("Unsupported event ID received %d", event->id());
            retval = QC2_NOT_FOUND;
            break;
        }
    }

    event->reply().put("status", retval);
    return retval;
}

QC2Status QC2Filter::handleInit() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    //Query & set the filter mode
    retval = setFilterOperatingMode(mFilterMode);
    if(retval != QC2_OK) {
        QLOGE_INST("Filter operating mode setting failed");
        return retval;
    }

    retval = onInit();
    if (retval != QC2_OK) {
        QLOGE_INST("Filter onInit() failed");
    }

    return retval;
}

QC2Status QC2Filter::handleRelease() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    retval = onRelease();
    if (retval != QC2_OK) {
        QLOGE_INST("Filter onRelease failed");
    }

    return retval;
}

QC2Status QC2Filter::handleStart() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    // Get all the buffer pool handles
    if (mBufPoolProvider == nullptr) {
        QLOGE_INST("No buffer pool provider present");
        return QC2_NO_INIT;
    }

    // Request for a graphic pool needed for output buffers
    auto newGraphicPool = mBufPoolProvider->requestGraphicPool(C2BlockPool::BASIC_GRAPHIC);
    if (newGraphicPool == nullptr) {
        QLOGE_INST("Failed to allocate graphic pool");
        return QC2_NO_MEMORY;
    }

    // Update & set usage flags
    auto graphicPool = updateGraphicBufferPool(std::move(newGraphicPool));
    graphicPool->setUsage(MemoryUsage::HW_CODEC_WRITE | MemoryUsage::CPU_WRITE);

    // Get the custom filter properties (h/w/f) and configure the graphic pool
    FilterPortProp props;
    getPortProperties(FILTER_PORT_OUTPUT, props);

    QLOGD_INST("Create graphicPool h=%d w=%d fmt=%d", props.height, props.width,
                                                      props.format);
    graphicPool->setResolution(props.width, props.height);
    graphicPool->setFormat(props.format);

    // Create linear pool for extradata buffers
    mLinearPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (mLinearPool == nullptr) {
        QLOGE_INST("Failed to create extradata linear pool !");
        return QC2_ERROR;
    }
    mLinearPool->setBufferSize(FILTER_EXTRADATA_BUFFER_SIZE);
    mLinearPool->setUsage(MemoryUsage::HW_CODEC_WRITE);

    mFilterOutputCounter = 0;
    mNumInputsQueued = 0;

    // call start on underlying filter
    retval = onStart();
    if (retval != QC2_OK) {
        QLOGE_INST("Filter onStart failed");
        return retval;
    }

    // Update state handling
    mStopping = false;

    return retval;
}

QC2Status QC2Filter::handleStop() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    mStopping = true;

    // Flush all the buffers before stopping
    retval = handleFlush();
    if (retval != QC2_OK) {
        QLOGE_INST("Failed to flush during handleStop");
    }

    // call stop on underlying filter
    retval = onStop();
    if (retval != QC2_OK) {
        QLOGE_INST("Filter onStop failed");
    }

    return retval;
}

QC2Status QC2Filter::handleFlush() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    // Flush both input and output ports if buffers are present on port
    if (mIpBufList->listDepth()) {
        QLOGD_INST("Call flush on input port");
        retval = onFlush(FILTER_PORT_INPUT);
    }

    if (mOpBufList->listDepth()) {
        QLOGD_INST("Call flush on output port");
        retval = onFlush(FILTER_PORT_OUTPUT);
    }

    // wait for the buffers to be returned back

    // Clear all references
    auto graphicPool = getGraphicBufferPool();
    if (graphicPool) {
        graphicPool->dropAllReferences();
    }

    return retval;
}

QC2Status QC2Filter::handleDrain() {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    // TODO(DRUNWAL): What should underlying filter onDrain do ??
    return retval;
}

QC2Status QC2Filter::handleReconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    FilterPortProp ipProps, opProps;
    getPortProperties(FILTER_PORT_INPUT, ipProps);
    getPortProperties(FILTER_PORT_OUTPUT, opProps);

    for (auto& param : targetConfig) {
        QLOGD_INST("handleReconfigure: type-index=%x, base-index=%x", param->coreIndex().typeIndex(),
                                    param->index());
        switch (param->coreIndex().typeIndex()) {
            case kParamIndexPictureSize: {
                C2StreamPictureSizeInfo::output o;
                C2StreamPictureSizeInfo::input i;

                if (param->index() == o.index()) {
                    C2StreamPictureSizeInfo::output *outSize;
                    outSize = (C2StreamPictureSizeInfo::output*)param.get();

                    opProps.height = outSize->height;
                    opProps.width = outSize->width;
                    opProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(opProps.width),
                                                   opProps.format);
                    opProps.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(opProps.height),
                                                       opProps.format);
                    QLOGD_INST("handleReconfigure: Output w=%u, h=%u",
                                                opProps.width, opProps.height);
                } else if (param->index() == i.index()) {
                    C2StreamPictureSizeInfo::input *ipSize;
                    ipSize = (C2StreamPictureSizeInfo::input*)param.get();

                    ipProps.height = ipSize->height;
                    ipProps.width = ipSize->width;
                    ipProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(ipProps.width),
                                                   ipProps.format);
                    ipProps.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(ipProps.height),
                                                       ipProps.format);
                    QLOGD_INST("handleReconfigure: Input w=%u, h=%u",
                                                ipProps.width, ipProps.height);
                }
                break;
            }

            case kParamIndexPixelFormat: {
                C2StreamPixelFormatInfo::input iFmt;
                C2StreamPixelFormatInfo::output oFmt;

                if (param->index() == oFmt.index()) {
                    C2StreamPixelFormatInfo::output *outputFmt;
                    outputFmt = (C2StreamPixelFormatInfo::output *)param.get();

                    uint32_t fmt = outputFmt->value;
                    opProps.format = fmt;
                    opProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(opProps.width), fmt);
                    opProps.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(opProps.height), fmt);
                    QLOGD_INST("handleReconfigure: Output fmt=%u, stride=%u, scanlines=%u",
                                        opProps.format, opProps.stride, opProps.scanLines);
                } else if (param->index() == iFmt.index()) {
                    C2StreamPixelFormatInfo::input *inputFmt;
                    inputFmt = (C2StreamPixelFormatInfo::input *)param.get();

                    uint32_t fmt = inputFmt->value;
                    ipProps.format = fmt;
                    ipProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(ipProps.width), fmt);
                    ipProps.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(ipProps.height), fmt);

                    QLOGD_INST("handleReconfigure: Input fmt=%u, stride=%u, scanlines=%u",
                                        ipProps.format, ipProps.stride, ipProps.scanLines);
                }
                break;
            }

            default: {
                QLOGE_INST("handleReconfigure: No default handling");
                // TODO:(DRUNWAL).onReconfigure() ?
                break;
            }
        }
    }

#if 0
    //Check if post-reconfiguration can filter operate with the new resolution
    if(ipProps.height > FILTER_MAX_INPUT_HEIGHT ||
       ipProps.width > FILTER_MAX_INPUT_WIDTH) {
        QLOGI_INST("handleReconfigure: Force bypass: unsupported h=%u, w=%u",
            ipProps.height, ipProps.width);
        mFilterMode = FILTER_MODE_BYPASS;
    } else if (mFilterMode == FILTER_MODE_BYPASS &&
               (ipProps.height <= FILTER_MAX_INPUT_HEIGHT ||
                ipProps.width <= FILTER_MAX_INPUT_WIDTH)) {
        QLOGI_INST("handleReconfigure: Reset bypass: new resolution h=%u, w=%u",
            ipProps.height, ipProps.width);
        setFilterOperatingMode(mFilterMode);
    }
#endif

    // Now reconfigure filter based on these properties
    retval = onSessionReconfigure(ipProps, opProps);
    if (retval == QC2_OK) {
        // Flush all the remaining output buffers
        if (mOpBufList->listDepth()) {
            QLOGD_INST("handleReconfigure: Flush output port");
            retval = onFlush(FILTER_PORT_OUTPUT);
        }

        QLOGD_INST("handleReconfigure: Update graphic pool h=%u, w=%u, fmt=%u",
            opProps.height, opProps.width, opProps.format);
        auto graphicPool = getGraphicBufferPool();
        if (graphicPool) {
            graphicPool->dropAllReferences();
            graphicPool->setResolution(opProps.width, opProps.height);
            graphicPool->setFormat(opProps.format);
        }
    }
    return retval;
}

QC2Status QC2Filter::handleQueueInputs(std::list<std::shared_ptr<InputBufferPack>>* inputs) {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;

    for (auto itr = inputs->begin(); itr != inputs->end(); ) {
        auto input = *itr;
        if (!(input->mInput)) {
            QLOGE_INST("handleQueueInputs: Invalid input buffer");
            return QC2_ERROR;
        }

        // Allocate a linear extraData buffer, to which the flattened C2Infos will be copied
        std::shared_ptr<QC2Buffer> extraData = nullptr;
        if (mLinearPool && input->mInput->infos().size()) {
            QC2Status ret = mLinearPool->allocate(&extraData);
            if (ret != QC2_OK || extraData == nullptr || !extraData->isLinear()) {
                QLOGE_INST("Linear extraData-buf allocation failed!");
                return QC2_NO_MEMORY;
            }
        }

        // Currently store the infos right away so they can be attached back to
        // outputs generated from this buffer [moved here since IO is in same context]
        if (input->mInput->infos().size()) {
            QLOGD_INST("handleQInputs:: Store C2Infos for cookieInToOut[%" PRIu64 "]",
                QC2FilterInputTag::fromId(input->mInput->inputIndex()));
            mInputExtraData.emplace(QC2FilterInputTag::fromId(input->mInput->inputIndex()),
                                    input->mInput->infos());
        }

        // Prepare the input buffer and send it to filter
        retval = prepAndQInputBuffer(input->mInput, extraData);
        if (retval != QC2_OK) {
            QLOGE_INST("handleQueueInputs: failed to prepareAndQueueInputBuffer");
            return retval;
        } else {
            itr = inputs->erase(itr);
        }
    }
    return QC2_OK;
}

//------------------------------------------------------------------------------
// QC2Filter Private APIs / Helper APIs
//------------------------------------------------------------------------------
QC2Status QC2Filter::getTargetReconfigInfo(std::shared_ptr<QC2Buffer> inpBuf,
                     std::shared_ptr<QC2Codec::ReconfigParamsInfo> reconfigInfo) {
    if (!inpBuf || !reconfigInfo) {
        QLOGE_INST("Incorrect/Bad parameters for extracting reconfig info");
        return QC2_BAD_ARG;
    }

    QLOGD_INST("getTargetReconfigInfo: Extracting info from input buffer");

    // C2StreamPictureSizeInfo::input
    auto iRes = std::make_unique<C2StreamPictureSizeInfo::input>();
    if (!iRes) {
        QLOGE_INST("Failed to create C2StreamPictureSizeInfo::input");
        return QC2_NO_MEMORY;
    }
    iRes->height = inpBuf->graphic().height();
    iRes->width = inpBuf->graphic().width();

    reconfigInfo->mTargetConfig.push_back(std::move(iRes));

    // C2StreamPictureSizeInfo::output
    auto oRes = std::make_unique<C2StreamPictureSizeInfo::output>();
    if (!oRes) {
        QLOGE_INST("Failed to create C2StreamPictureSizeInfo::output");
        return QC2_NO_MEMORY;
    }

    oRes->height = inpBuf->graphic().height();
    oRes->width = inpBuf->graphic().width();
    reconfigInfo->mTargetConfig.push_back(std::move(oRes));

    // C2StreamPixelFormatInfo::input
    auto iFmt = std::make_unique<C2StreamPixelFormatInfo::input>();
    if (!iFmt) {
        QLOGE_INST("Failed to create C2StreamPixelFormatInfo::iunput");
        return QC2_NO_MEMORY;
    }

    iFmt->value = inpBuf->graphic().format();
    reconfigInfo->mTargetConfig.push_back(std::move(iFmt));

    // C2StreamPixelFormatInfo::output
    auto oFmt = std::make_unique<C2StreamPixelFormatInfo::output>();
    if (!oFmt) {
        QLOGE_INST("Failed to create C2StreamPixelFormatInfo::output");
        return QC2_NO_MEMORY;
    }

    oFmt->value = inpBuf->graphic().format();
    reconfigInfo->mTargetConfig.push_back(std::move(oFmt));

    return QC2_OK;
}

QC2Status QC2Filter::checkBufferParams(std::shared_ptr<QC2Buffer> buf,
                                       FilterPort port) {
    FilterPortProp props;
    QC2Buffer::Graphic& graphicBuf = buf->graphic();

    // Get port properties from underlying filter
    QLOGD_INST("checkBufferParams: Get current filter properties");
    getPortProperties(port, props);

    // No content to check if buffer is empty or codec_config
    if ((buf->flags() & BufFlag::CODEC_CONFIG) ||
        (buf->flags() & BufFlag::EMPTY)) {
        return QC2_OK;
    }

    // Check if there is any change in buffer properties & filter props
    if ( (props.height != graphicBuf.height()) ||
        (props.width != graphicBuf.width()) ||
        (props.stride != graphicBuf.alignedWidth()) ||
        (props.scanLines != graphicBuf.sliceHeight()) ||
        (props.format != graphicBuf.format())) {
        QLOGI_INST("checkBufferParams: Buffer parameters have changed");
        QLOGI_INST("Filter: h=%u, w=%u, st=%u, sl=%u, fmt=%u",
            props.height, props.width, props.stride, props.scanLines, props.format);
        QLOGI_INST("Buffer: h=%u, w=%u, st=%u, sl=%u, fmt=%u",
            graphicBuf.height(), graphicBuf.width(), graphicBuf.alignedWidth(),
            graphicBuf.sliceHeight(), graphicBuf.format());
        return QC2_CANNOT_DO;
    }
    return QC2_OK;
}

QC2Status QC2Filter::returnPendingInputs() {
    QC2Status retval = QC2_OK;
    auto iDone = std::make_shared<QC2Codec::InputBuffersDoneInfo>();

    // traverse the pending Input Q and add it to QC2Codec::InputBuffersDoneInfo
    for (auto itr = mPendingInputs.begin(); itr != mPendingInputs.end();) {
        if (*itr && (*itr)->mInput) {
            iDone->mInputFrameIds.push_back((*itr)->mInput->inputIndex());
        }
        itr = mPendingInputs.erase(itr);
    }

    QLOGD_INST("returnPendingInputs: Notify input buffers done");
    // Notify upper layer about input buffers done
    if (iDone->mInputFrameIds.size()) {
        auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
        e->info().put(QC2Codec::NotifyKey::kBufsDone, iDone);
        mListener->postAsync(e);
    }
    return retval;
}

void QC2Filter::dispatchInputBuffer(std::shared_ptr<QC2Buffer> inBuf) {
    auto iDone = std::make_shared<QC2Codec::InputBuffersDoneInfo>();

    iDone->mInputFrameIds = {inBuf->inputIndex()};

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, iDone);

    QLOGD_INST("Dispatch Input Buffer: inputIndex=%u", (uint32_t)inBuf->inputIndex());
    mListener->postAsync(e);
}

void QC2Filter::dispatchOutputBuffer(std::shared_ptr<QC2Buffer> outBuf) {
    auto oDone = std::make_shared<QC2Codec::OutputBuffersDoneInfo>();

    // construct oDone
    oDone->mOutputs.emplace_back(QC2Codec::OutputBufferPack(outBuf));

    // Check if filter is operating in bypass or inplace mode. In such case update output delays
    if(mFilterMode == FILTER_MODE_BYPASS || mFilterMode == FILTER_MODE_INPLACE) {
        if(mbUpdateDelays) {
            uint32_t outputDelay = 1;
            auto outDelayInfo = std::unique_ptr<C2Param>(
                                new C2PortActualDelayTuning::output(outputDelay));
            mUpdatedInfos.push_back(std::move(outDelayInfo));
            mbUpdateDelays = false;
        }
    }

    // Add config updates (some may be added by custom implementor as well)
    if(mUpdatedInfos.size()) {
        for(auto itr = oDone->mOutputs.begin(); itr != oDone->mOutputs.end(); itr++) {
            auto& outBufPack = *itr;

            if(outBufPack.mBuf == nullptr) {
                continue;
            }

            std::move(mUpdatedInfos.begin(), mUpdatedInfos.end(),
                      std::back_inserter(outBufPack.mConfigUpdates));
        }
        mUpdatedInfos.clear();
    }

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE);
    e->info().put(QC2Codec::NotifyKey::kBufsDone, oDone);

    QLOGD_INST("Dispatch Output Buffer: inputIndex=%u", (uint32_t)outBuf->inputIndex());
    mListener->postAsync(e);
}

QC2Status QC2Filter::dispatchBypassedBuffer(const FilterBuffer& buf) {
    uint32_t bufId = (uint32_t)buf.cookie;

    //No output would be generated for this. Hence remove the extradata reference
    mInputExtraData.erase(buf.cookieInToOut);

    // This is a input buffer bypassed to output port, return it back to client
    std::shared_ptr<QC2Buffer> inBuf = nullptr;
    mIpBufList->pop(bufId, inBuf);
    if (inBuf == nullptr) {
        QLOGE_INST("No mapping in the input buf list for id=%u", bufId);
        return QC2_ERROR;
    }

    if(buf.isCodec_Config) {
        // This is codec_config buffer. Indicate no-outputs will be generated
        auto flags = inBuf->flags() | BufFlag::CODEC_CONFIG;
        inBuf->setFlags(flags);
    }

    //dispatch this buffer
    QLOGD_INST("FILTER_CB_BYPASS:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isCC[%u] isValidOP[%u] cookieInToOut[%" PRIu64 "]",
        buf.cookie, buf.timestamp, buf.filledLen, buf.isCodec_Config, buf.isValidOutput, buf.cookieInToOut);
    dispatchOutputBuffer(inBuf);

    return QC2_OK;
}

QC2Status QC2Filter::prepAndQInputBuffer(std::shared_ptr<QC2Buffer> qc2Buf,
                                         std::shared_ptr<QC2Buffer> extraDataBuf) {
    FuncTracker track(__func__);

    QC2Status retval = QC2_OK;
    int bufIndex = -1;
    FilterBuffer ipBuffer;

    logBuffer(qc2Buf, "filter-queue-ip");

    mNumInputsQueued++;

    // Hold reference to the input buffer
    if (mIpBufList->store(qc2Buf, bufIndex)) {
        QLOGE_INST("The buffer index for input is out of range");
        return QC2_ERROR;
    }

    auto frameId = qc2Buf->inputIndex();
    if (QC2FilterInputTag::fromId(frameId) == QC2FilterInputTag::INVALID) {
        QLOGE_INST("%s: Invalid frame-index 0x%" PRIx64, __func__, frameId);
        return QC2_CORRUPTED;
    }

    QLOGD_INST("Populate filled length");
    QC2Buffer::Graphic& pxBuf = qc2Buf->graphic();
    ipBuffer.filledLen = pxBuf.compressedSize();

    // Check EOS in QC2Buffer and update the required flag
    ipBuffer.isEOS = (qc2Buf->flags() & BufFlag::EOS) ? true : false;

    //If codec_config is set, then ensure buffer is dispatched directly
    ipBuffer.isCodec_Config = (qc2Buf->flags() & BufFlag::CODEC_CONFIG);
    if(ipBuffer.isCodec_Config) {
        ipBuffer.isValidOutput = false;
        ipBuffer.isPendingOutput = false;
    }

    ipBuffer.isEmpty = (qc2Buf->flags() & BufFlag::EMPTY);

    //Timestamp
    ipBuffer.timestamp = qc2Buf->timestamp();

    // Hold Cookie associated with this buffer
    ipBuffer.cookie = (uint64_t)bufIndex;

    // Move Buffers
    ipBuffer.buffer = std::move(qc2Buf);
    ipBuffer.extradata = std::move(extraDataBuf);

    // Reference buffer index
    ipBuffer.cookieInToOut = QC2FilterInputTag::fromId(frameId);

    QLOGD_INST("FILTER_Q_INPUT:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isCC[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        ipBuffer.cookie, ipBuffer.timestamp, ipBuffer.filledLen, ipBuffer.isEOS, ipBuffer.isCodec_Config, ipBuffer.isEmpty, ipBuffer.cookieInToOut);

    //Bypass this buffer if codec-config
    if(ipBuffer.isCodec_Config) {
        //Send buffer to output directly
        onFilterHandleOutputDone(ipBuffer);
        return QC2_OK;
    }

    // Queue buffer to the custom filter
    retval = onProcessInput(ipBuffer);

    QLOGD_INST("Input buffer process and consumed by underlying filter");
    if (retval != QC2_OK) {
        // Failed to process the input buffer
        std::shared_ptr<QC2Buffer> inBuf = nullptr;
        QLOGE_INST("Filter onProcessInput error: %u", retval);
        mIpBufList->pop(bufIndex, inBuf);
        // TODO(DRUNWAL): Should we drain/bypass this buffer ?
        dispatchInputBuffer(inBuf);
    }

    return retval;
}

QC2Status QC2Filter::allocateAndRequestOutputBuffer(const FilterPortProp& props,
                                                    FilterBuffer& opBuffer) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    std::shared_ptr<QC2Buffer> qc2Buf = nullptr;

    // Get access to graphic pool
    auto graphicPool = getGraphicBufferPool();

    if (!graphicPool) {
        QLOGE_INST("Graphic pool is not available, error!");
        return QC2_NO_MEMORY;
    }

    if (graphicPool == nullptr) {
        QLOGE_INST("Graphic pool is null, error!");
        return QC2_ERROR;
    }

    // Update required resolution & format
    QLOGD_INST("allocateOP: setting w%u x h%u fmt=%u on graphic pool",
        props.width, props.height, props.format);

    graphicPool->setResolution(props.width, props.height);
    graphicPool->setFormat(props.format);

    // Allocate output buffer
    uint32_t uRetry = 30;
    while(uRetry--) {
        QC2Status ret = graphicPool->allocate(&qc2Buf);
        if (ret == QC2_OK && qc2Buf != nullptr && qc2Buf->isGraphic()) {
            QLOGD_INST("Successful allocation");
            // validate the buffer
            retval = validateOutputBuffer(qc2Buf);
            if (retval != QC2_OK) {
                QLOGI_INST("Output buffer validation failed");
                // TODO(DRUNWAL): Handle this ?
            }
            break;
        } else {
            QLOGE_INST("Failed to allocate op buffer. Retry [%u]", uRetry);
            usleep(1000);
            if(mFlushing) return QC2_ERROR;
        }
    }

    // Let underlying filter implementation handle the oom usecase
    if(qc2Buf == nullptr || !qc2Buf->isGraphic()) {
        QLOGE_INST("Failed to allocate output buffer after timeout");
        return QC2_NO_MEMORY;
    }

    // Allocate a linear extradata buffer
    std::shared_ptr<QC2Buffer> extradata = nullptr;
    if (mLinearPool) {
        QC2Status ret = mLinearPool->allocate(&extradata);
        if (ret != QC2_OK || extradata == nullptr || !extradata->isLinear()) {
            QLOGE_INST("Linear extradata-buf allocation failed!");
            return QC2_NO_MEMORY;
        }
    }

    int bufIndex = -1;
    // Hold reference to the output buffer created
    if (mOpBufList->store(qc2Buf, bufIndex)) {
        QLOGE_INST("The buffer index for output is out of range");
        return QC2_ERROR;
    }

    // Update buffers
    opBuffer.buffer = std::move(qc2Buf);
    opBuffer.extradata = std::move(extradata);

    opBuffer.cookie = uint64_t(bufIndex);
    opBuffer.cookieInToOut = QC2FilterInputTag::INVALID;

    //Mark this as valid output buffer to differentiate with bypassed buffers
    opBuffer.isValidOutput = true;

    // store the extradata along with bufindex
    QLOGD_INST("allocateOp: store op extradata for cookie= %" PRIu64, opBuffer.cookie);
    mOutputExtraData.emplace(bufIndex, extradata);

    return QC2_OK;
}

QC2Status QC2Filter::validateOutputBuffer(const std::shared_ptr<QC2Buffer>& output) {
    QC2Status retval = QC2_OK;

    retval = checkBufferParams(output, FILTER_PORT_OUTPUT);
    if (retval != QC2_OK) {
        QLOGE_INST("validateOutputBuffer: Buffer parameters are incorrect");
    }

    return retval;
}

QC2Status QC2Filter::configureParam(C2StreamPictureSizeInfo::output* outSize) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("ConfiureParam: o/p resolution=%ux%u", outSize->width, outSize->height);

    if (mGraphicPool) {
        mGraphicPool->setResolution(outSize->width, outSize->height);
    } else {
        QLOGE_INST("No graphics pool is available for configuration");
    }

    // Get the current output port props
    FilterPortProp props;
    getPortProperties(FILTER_PORT_OUTPUT, props);

    // Create the new port props
    FilterPortProp res;

    res.width = outSize->width;
    res.height = outSize->height;
    res.stride = ALIGNED_WIDTH(static_cast<uint32_t>(res.width), props.format);
    res.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(res.height), props.format);

    // Set the new port props
    retval = setPortProperties(FILTER_PORT_OUTPUT, res);
    if (retval != QC2_OK) {
        QLOGE_INST("setPortResolution[output] returned error=%d", retval);
    }
    return retval;
}


QC2Status QC2Filter::configureParam(C2StreamPictureSizeInfo::input* inpSize) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("ConfigureParam: Configure i/p resolution=%ux%u", inpSize->width, inpSize->height);

    // Get current port props
    FilterPortProp props;
    getPortProperties(FILTER_PORT_INPUT, props);

    // Create the new port props
    FilterPortProp res;

    res.width = inpSize->width;
    res.height = inpSize->height;
    res.stride = ALIGNED_WIDTH(static_cast<uint32_t>(res.width), props.format);
    res.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(res.height), props.format);

    // Set the new port props
    retval = setPortProperties(FILTER_PORT_INPUT, res);
    if (retval != QC2_OK) {
        QLOGE_INST("setPortResolution[input] returned error=%d", retval);
    }
    return retval;
}

QC2Status QC2Filter::configureParam(C2StreamPixelFormatInfo::output* outFmt) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    uint32_t newFmt =  outFmt->value;

    // Get the current props
    FilterPortProp res;
    getPortProperties(FILTER_PORT_OUTPUT, res);

    // Update the props based on Pixel format info
    res.stride = ALIGNED_WIDTH(static_cast<uint32_t>(res.width), newFmt);
    res.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(res.height), newFmt);
    res.format = newFmt;

    // Set the props
    retval = setPortProperties(FILTER_PORT_OUTPUT, res);
    if (retval != QC2_OK) {
        QLOGE_INST("setPixelFormat[output] returned error=%d", retval);
    }

    return retval;
}

QC2Status QC2Filter::configureParam(C2StreamPixelFormatInfo::input* inpFmt) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("configureParam: inpFmt=%s", PixFormat::Str(inpFmt->value));

    // Get the current props
    FilterPortProp res;
    getPortProperties(FILTER_PORT_INPUT, res);

    // Update the props based on Pixel format info
    res.stride = ALIGNED_WIDTH(static_cast<uint32_t>(res.width), inpFmt->value);
    res.scanLines = ALIGNED_HEIGHT(static_cast<uint32_t>(res.height), inpFmt->value);
    res.format = inpFmt->value;

    // Set the props
    retval = setPortProperties(FILTER_PORT_INPUT, res);
    if (retval != QC2_OK) {
        QLOGE_INST("setPixelFormat[input] returned error=%d", retval);
    }

    return retval;
}

QC2Status QC2Filter::configureParam(C2PortBlockPoolsTuning::output* outBlockPools) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("Output pool-id = 0x%" PRIx64, outBlockPools->m.values[0]);
    if (mBufPoolProvider == nullptr) {
        QLOGE_INST("Not buffer-pool provider available!");
        return QC2_CANNOT_DO;
    }

    auto updatedPool = mBufPoolProvider->requestGraphicPool(outBlockPools->m.values[0]);
    if (updatedPool == nullptr) {
        QLOGE_INST("Failed to update graphic pool!");
        return QC2_ERROR;
    }

    auto graphicPool = updateGraphicBufferPool(std::move(updatedPool));

    // Get the Port propoerties
    FilterPortProp props;
    getPortProperties(FILTER_PORT_OUTPUT, props);

    QLOGD_INST("ConfigureParam: Graphic pool configured for w%u x h%u and format:%u", props.width,
                props.height, props.format);

    auto outputUsage = MemoryUsage::HW_CODEC_WRITE;
    outputUsage |= 0u;  // TODO(DRUNWAL): update secure MemoryUsage::HW_PROTECTED
    graphicPool->setUsage(outputUsage);

    graphicPool->setResolution(props.width, props.height);
    graphicPool->setFormat(props.format);

    return retval;
}

QC2Status QC2Filter::configureParam(C2SecureModeTuning *secureMode) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    bool secure = secureMode->value == C2Config::SM_READ_PROTECTED;

    QLOGD_INST("Configuring secure mode : %s", secure ? "SECURE" : "NON-SECURE");

    if (secure && !(mVariant & QC2CodecVariant::SECURE)) {
        QLOGE_INST("Codec does not accept secure mode setting !");
        return QC2_CANNOT_DO;
    }

    // TODO(DRUNWAL): Set secure mode to underlying custom filter
    return retval;
}

//------------------------------------------------------------------------------
// QC2Filter Callbacks from underlying filter
//------------------------------------------------------------------------------
QC2Status QC2Filter::onFilterInPlaceOutputDone(const FilterBuffer& buf) {
    FuncTracker track(__func__);

    if(mFilterMode == FILTER_MODE_NORMAL) {
        QLOGE_INST("In place output done cannot be called in normal mode");
        return QC2_BAD_STATE;
    }

    uint32_t bufId = (uint32_t)buf.cookie;

    //Extradata handling is not required. Same buffer
    mInputExtraData.erase(buf.cookieInToOut);

    // This is a input buffer processed in place
    std::shared_ptr<QC2Buffer> inBuf = nullptr;
    mIpBufList->pop(bufId, inBuf);
    if (inBuf == nullptr) {
        QLOGE_INST("No mapping in the input buf list for id=%u", bufId);
        return QC2_ERROR;
    }

    // Set the indices
    inBuf->setInputIndex(QC2FilterInputTag::toId(buf.cookieInToOut));
    inBuf->setOutputIndex(mFilterOutputCounter++);

    QLOGD_INST("FILTER_IN_PLACE_OUTPUT_DONE:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isPendingOp[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        buf.cookie, buf.timestamp, buf.filledLen, buf.isEOS, buf.isPendingOutput, buf.isEmpty, buf.cookieInToOut);

    // Now notify output buffer done
    dispatchOutputBuffer(inBuf);
    return QC2_OK;
}

QC2Status QC2Filter::onFilterHandleInputDone(const FilterBuffer& buf) {
    FuncTracker track(__func__);

    std::shared_ptr<QC2Buffer> inBuf = nullptr;
    mIpBufList->pop(buf.cookie, inBuf);

    if (inBuf == nullptr) {
        QLOGE_INST("No mapping in the input buf list for id= %u", (uint32_t)buf.cookie);
        return QC2_ERROR;
    }
    dispatchInputBuffer(inBuf);
    return QC2_OK;
}

QC2Status QC2Filter::onFilterHandleOutputDone(const FilterBuffer& buf) {
    FuncTracker track(__func__);

    // Get Cookie
    uint32_t bufId = (uint32_t)buf.cookie;
    QLOGD_INST("onHandleOutputDone: Cookie for reference in OpBufferList is [%u] filledLen[%u] isEmpty[%u] isEOS[%u]",
                                                        bufId, buf.filledLen, buf.isEmpty, buf.isEOS);

    // Handle both input buffers (bypassed to output)
    //      - valid input buffers
    //      - EOS buffers
    // and interim output buffers (if allocated by few custom implementors) in bypass mode
    if(mFilterMode == FILTER_MODE_BYPASS) {
        if((buf.filledLen && !buf.isEmpty) || buf.isEOS) {
            // This is input buffer bypassed to output port. dispatch it
            return dispatchBypassedBuffer(buf);
        } else {
            // Some underlying filter may even allocate output buffer before bypassing so discard them
            std::shared_ptr<QC2Buffer> outBuf = nullptr;
            mOpBufList->pop(bufId, outBuf);
            outBuf = nullptr;
            QLOGD_INST("onHandleOutputDone: Bypass mode: optional discard output buffer bufId[%u]",
                bufId);
            return QC2_OK;
        }
    }

    // General codec-config or invalid output in normal filter operating mode
    if((!(buf.isValidOutput))) {
        return dispatchBypassedBuffer(buf);
    }

    // Valid output buffer generated in normal filter operating mode->
    std::shared_ptr<QC2Buffer> outBuf = nullptr;
    mOpBufList->pop(bufId, outBuf);
    if (outBuf == nullptr) {
        QLOGE_INST("No mapping in the output buf list for id=%u", bufId);
        return QC2_ERROR;
    }

    QLOGD_INST("onHandleOutputDone: CookieInToOut = %" PRIu64, buf.cookieInToOut);

    // Don't send unprocessed buffers to Component (identified by invalid tag)
    // Exception: [1] EOS output due to STOP command
    if (buf.cookieInToOut == QC2FilterInputTag::INVALID) {
        QLOGE_INST("Dropping unprocessed buffer with buf-index:%" PRIu64, buf.cookieInToOut);
        return QC2_ERROR;
    }

    // Set the indices
    outBuf->setInputIndex(QC2FilterInputTag::toId(buf.cookieInToOut));
    outBuf->setOutputIndex(mFilterOutputCounter++);

    //set fields
    outBuf->setTimestamp(buf.timestamp);
    outBuf->graphic().setCompressedSize(buf.filledLen);

    QLOGD_INST("onHandleOutputDone: isEOS[%u] isEmpty[%u] isPendingOutput[%u]",
                                buf.isEOS, buf.isEmpty, buf.isPendingOutput);

    // Construct flags
    uint64_t flags = 0;
    flags |= (buf.isEOS == true) ? BufFlag::EOS : 0;
    flags |= (buf.isPendingOutput == true) ? BufFlag::INPUT_PENDING : 0;
    flags |= (buf.isEmpty == true) ? BufFlag::EMPTY : 0;

    outBuf->setFlags(flags);

    // Copy C2Infos from Input buffer to Output buffer
    QLOGD_INST("Copying back the C2Infos from Input to Output");
    std::vector<std::shared_ptr<C2Info>> inputBufC2Infos = mInputExtraData[buf.cookieInToOut];
    for (auto info : inputBufC2Infos) {
        outBuf->addInfo(info);
    }

    // Remove C2Infos iff this is the last output buffer corresponding to input
    if (!(buf.isPendingOutput)) {
        mInputExtraData.erase(buf.cookieInToOut);
        QLOGD_INST("Removed input C2Infos : inRefBufIndex:%" PRIu64, buf.cookieInToOut);
    }

    // TODO(DRUNWAL): Extradata handling

    QLOGD_INST("FILTER_CB_OP_DONE:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isPendingOp[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        buf.cookie, buf.timestamp, buf.filledLen, buf.isEOS, buf.isPendingOutput, buf.isEmpty, buf.cookieInToOut);
    // dispatch the output buffer
    dispatchOutputBuffer(outBuf);

    return QC2_OK;
}
}   // namespace qc2

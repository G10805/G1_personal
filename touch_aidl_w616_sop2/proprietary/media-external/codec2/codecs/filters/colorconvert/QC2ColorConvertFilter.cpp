/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#undef LOG_TAG
#define LOG_TAG "QC2ColorConvertFilter"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

#include "QC2ColorConvertFilter.h"
#include "QC2ColorConvertFilterCapsHelper.h"

namespace qc2 {
#define DEBUG_ONLY

#define ALIGNED_WIDTH  QC2Buffer::Graphic::GetAlignedWidth
#define ALIGNED_HEIGHT QC2Buffer::Graphic::GetAlignedHeight

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

QC2Status QC2ColorConvertFilter::getPortProperties(FilterPort port,FilterPortProp & props) {
    if(port == FILTER_PORT_INPUT) {
        props.height = mInputProps.height;
        props.width = mInputProps.width;
        props.format = mInputProps.format;
        props.scanLines = mInputProps.scanLines;
        props.stride = mInputProps.stride;
    } else if(port == FILTER_PORT_OUTPUT) {
        props.height = mOutputProps.height;
        props.width = mOutputProps.width;
        props.format = mOutputProps.format;
        props.scanLines = mOutputProps.scanLines;
        props.stride = mOutputProps.stride;
    } else {
        QLOGE_INST("getPortProperties: incorrect port submitted");
        return QC2_BAD_ARG;
    }
    return QC2_OK;
}

QC2Status QC2ColorConvertFilter::setPortProperties(FilterPort port,const FilterPortProp& props) {
    if(port == FILTER_PORT_INPUT) {
        mInputProps.height = props.height;
        mInputProps.width = props.width;
        mInputProps.format = props.format;
        mInputProps.scanLines = props.scanLines;
        mInputProps.stride = props.stride;
    } else if (port == FILTER_PORT_OUTPUT) {
        mOutputProps.height = props.height;
        mOutputProps.width = props.width;
#ifdef DEBUG_ONLY
        /* Update different color format on output port for reference colorconvert */
        mOutputProps.format = overrideOutputColorFormat();
        mOutputProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(mOutputProps.width), mOutputProps.format);
        mOutputProps.scanLines= ALIGNED_HEIGHT(static_cast<uint32_t>(mOutputProps.height), mOutputProps.format);
#else
        mOutputProps.format = props.format;
        mOutputProps.stride = props.stride;
        mOutputProps.scanLines = props.scanLines;
#endif
    } else {
        QLOGE_INST("setPortProperties: incorrect port submitted");
        return QC2_BAD_ARG;
    }

    return QC2_OK;
}

QC2ColorConvertFilter::QC2ColorConvertFilter(const std::string & name,
                                             const uint32_t sessionId,

                                             const std::string & instName,
                                             const std::string & upStreamCodec,
                                             const std::string & downStreamCodec,
                                             uint32_t variant,
                                             std::shared_ptr<EventHandler>listener)
            : QC2Filter(name, sessionId, instName, upStreamCodec,
                        downStreamCodec, KIND_FILTER, variant, listener) {
    FuncTracker track(__func__);
    mEnableFilter = false;
    mConverter = nullptr;
    mRotateAngle = QC2ImageConverter::Capabilities::ROTATION_0;
    mFlipDirection = QC2ImageConverter::Capabilities::FLIP_NONE;
}

QC2ColorConvertFilter::~QC2ColorConvertFilter() {
    FuncTracker track(__func__);
}

QC2Status QC2ColorConvertFilter::setFilterOperatingMode(FilterMode& mode) {
    FuncTracker track(__func__);

    // Set filter mode (normal or in-place transform)
    if (OS::SystemProperty::GetUInt32("vendor.qc2.filter.colorconvert.inplace", 0u) > 0) {
        QLOGD_INST("Filter will operate in in-place mode");
        mode = FILTER_MODE_INPLACE;
    } else {
        QLOGE("Filter will operate in normal mode");
        mode = FILTER_MODE_NORMAL;
    }

    return QC2_OK;
}

bool QC2ColorConvertFilter::forceBypassMode() {
    // Temporary property to configure filter in bypass mode on the fly
    if (OS::SystemProperty::GetUInt32("vendor.qc2.filter.colorconvert.bypass", 0u) > 0) {
        QLOGD_INST("Force filter in bypass mode");
        return true;
    } else {
        QLOGE("Filter will operate in non-bypass mode");
        return false;
    }
}

QC2Status QC2ColorConvertFilter::onInit() {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("Init ColorConverter");

    return retval;
}

QC2Status QC2ColorConvertFilter::onStart() {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    if(!mEnableFilter) {
        QLOGE_INST("ColorConvert filter not enabled. Will work in bypass mode");
    }
    return retval;
}

QC2Status QC2ColorConvertFilter::onStop() {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("onStop done");
    //Nothing specific to do here
    return retval;

}

QC2Status QC2ColorConvertFilter::onFlush(FilterPort port) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;
    (void)port;
    //DRUNWAL: TODO
    return retval;
}

QC2Status QC2ColorConvertFilter::onRelease() {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    //Nothing specific to do here
    QLOGD_INST("onRelease done");
    return retval;
}

QC2Status QC2ColorConvertFilter::onProcessInput(FilterBuffer& buffer) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGD_INST("onProcessInput[ipBuffer]:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isCC[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        buffer.cookie, buffer.timestamp, buffer.filledLen, buffer.isEOS, buffer.isCodec_Config, buffer.isEmpty, buffer.cookieInToOut);

    for (auto& info : buffer.buffer->infos()) {
        if (info->coreIndex().coreIndex() == C2StreamHdrStaticInfo::output::CORE_INDEX) {
            // extract HDR static info recieved from the codec
            C2StreamHdrStaticInfo::output streamHsi = *((C2StreamHdrStaticInfo::output*)info.get());
            QLOGD_INST("HDR Static Info available");
            QLOGD_INST("HDR Info. Max light level = %f, Average Max Frame light level = %f",
                       streamHsi.maxCll, streamHsi.maxFall);
        }
    }

    //Example for in place buffer processing
    if(mFilterMode == FILTER_MODE_INPLACE) {
        return processInPlace(buffer);
    }

    if(!mConverter) {
        QLOGD_INST("Create Converter as input is now avaiable");
        retval = initConverter();
        if(retval != QC2_OK) {
            QLOGE_INST("Failed to create engine");
            return retval;
        }
    }

    //1. Allocate output buffer
    FilterBuffer opBuffer;
    retval = allocateAndRequestOutputBuffer(mOutputProps, opBuffer);
    QLOGD_INST("onProcessInput: Allocated output buffer");

    if(retval != QC2_OK) {
        QLOGE_INST("Failed to get output buffer");
        return QC2_ERROR;
        //TODO(DRUNWAL): Handle output buffer failure ?
    }

#ifdef DEBUG_ONLY
        //Reference logic for runtime bypass mode
        if(forceBypassMode()) {
            // Update the filter operating mode once
            if(mFilterMode != FILTER_MODE_BYPASS) {
                updateFilterOperatingMode(FILTER_MODE_BYPASS);
            }

            // return the buffer back w/o processing
            onFilterHandleOutputDone(buffer);

            // optionally return unused output buffer as well if allocation was made
            opBuffer.filledLen = 0;
            opBuffer.isValidOutput = false;
            opBuffer.cookieInToOut = buffer.cookieInToOut;
            opBuffer.isEmpty = true;

            onFilterHandleOutputDone(opBuffer);
            return retval;
        } else {
            // Regular mode. Check if filter was previously in bypass mode or not
            if(mFilterMode == FILTER_MODE_BYPASS) {
                FilterMode defaultMode;
                //set the filter back to its previous working mode (inplace/normal)
                setFilterOperatingMode(defaultMode);
                updateFilterOperatingMode(defaultMode);
            }
        }
#endif

    //convert the buffer
    if(!(buffer.isEmpty)) {
        QLOGD_INST("Perform C2D");
        retval = mConverter->process(&(buffer.buffer->graphic()),
                                     &(opBuffer.buffer->graphic()));

        if(retval != QC2_OK) {
            QLOGE_INST("Failed to color convert");
        }
        QLOGD_INST("onProcessInput: ColorConvert Performed!");
    }

    //Fill length and empty falg
    //TODO(DRUNWAL): Fix this once we have correct format
    QLOGD_INST("onProcessInput: Populate data in buffer filledLen=%u:", buffer.filledLen);
    opBuffer.filledLen = buffer.filledLen;
    if(opBuffer.filledLen == 0) {
        opBuffer.isEmpty = true;
    } else {
        opBuffer.isEmpty = false;
    }

    //Fill crop info
    //TODO(DRUNWAL): Fix this by extracting required crop info
    opBuffer.buffer->graphic().setCrop(buffer.buffer->graphic().crop());

    const auto& og = opBuffer.buffer->graphic();
    auto oc = og.crop();
    const auto& ig = buffer.buffer->graphic();
    auto ic = ig.crop();

    QLOGD_INST("onProcessInput: Input crop (%d,%d)[%dx%d] Output crop (%d,%d)[%dx%d]",
        ic.left, ic.top, ic.width, ic.height, oc.left, oc.top, oc.width, oc.height);

    //Fill fields
    opBuffer.timestamp = buffer.timestamp;

    //copy cookie
    opBuffer.cookieInToOut = buffer.cookieInToOut;

    //Populate flags
    opBuffer.isPendingOutput = false;
    opBuffer.isEOS = buffer.isEOS;

    QLOGD_INST("onProcessInput: NOTIFY_INPUT_DONE");
    //notify input done
    onFilterHandleInputDone(buffer);

    QLOGD_INST("onProcessInput[opBuffer]:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isCC[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        opBuffer.cookie, opBuffer.timestamp, opBuffer.filledLen, opBuffer.isEOS, opBuffer.isCodec_Config, opBuffer.isEmpty, opBuffer.cookieInToOut);

#ifdef DEBUG_ONLY
    if(mFilterMode == FILTER_MODE_NORMAL) {
        // Business logic to update output-delay at runtime
        uint32_t outputDelay = 5;
        auto outDelayInfo = std::unique_ptr<C2Param>(
                            new C2PortActualDelayTuning::output(outputDelay));

        QLOGD_INST("onPrcessInput: Add custom outputDelay [%u] to output generated", outputDelay);
        addOutputConfigUpdates(outDelayInfo);
    }
#endif

    QLOGD_INST("onProcessInput: NOTIFY_OUTPUT_DONE");
    //notify output done
    onFilterHandleOutputDone(opBuffer);

    return retval;
}

QC2Status QC2ColorConvertFilter::initConverter() {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    mConverter = QC2ImageConverter::Builder({mInputProps.format, mInputProps.width, mInputProps.height})
                    .withDestinationColor(mOutputProps.format)
                    .withFlip(mFlipDirection)
                    .withRotation(mRotateAngle)
                    .build();

    if (!mConverter) {
        QLOGE_INST("Failed to create color converter !");
        return QC2_ERROR;
    }

    return retval;
}

QC2Status QC2ColorConvertFilter::processInPlace(FilterBuffer& buffer) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    //Perform required operation on input buffer
    //operate(...)

    //Populate required otuput flags
    buffer.isEmpty = buffer.filledLen == 0 ? true : false;
    buffer.isPendingOutput = false;
    buffer.isValidOutput = true;

    QLOGD_INST("processInPlace:: cookie[%" PRIu64 "] timestamp [%" PRIu64 "] filledLen[%u] isEOS[%u] isCC[%u] isEmpty[%u] cookieInToOut[%" PRIu64 "]",
        buffer.cookie, buffer.timestamp, buffer.filledLen, buffer.isEOS, buffer.isCodec_Config, buffer.isEmpty, buffer.cookieInToOut);

    //Send the buffer
    onFilterInPlaceOutputDone(buffer);

    return retval;
}

QC2Status QC2ColorConvertFilter::onGetCapsHelpers(std::unordered_map<uint32_t,
                                std::shared_ptr<QC2ParamCapsHelper>>* helpers) const {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    if (helpers) {
        for (auto& f : QC2ColorConvertFilterCapsHelper::getColorConvertFilterCapsHelpers()) {
            auto b = f(mCodecName, mInputMime, mOutputMime, mVariant, mKind, nullptr);
            if (b->isSupported()) {
                helpers->emplace(b->getIndex(), b);
                QLOGD_INST("%s registered", ColorConvFilterDebugString::C2Param(b->getIndex()).c_str());
            }
        }
    } else {
        retval = QC2_BAD_ARG;
    }
    return retval;
}

QC2Status QC2ColorConvertFilter::onConfigure(const C2Param& param) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;

    QLOGE_INST("onConfigure: %s core-index=%x, baseIndex=%x",
        DebugString::C2Param(param.index()).c_str(), param.coreIndex().coreIndex(),
        param.index());

    //handling required for filter enabler ? TODO: DRUNWAL

    return retval;
}

QC2Status QC2ColorConvertFilter::onQuery(C2Param* const param) {
    FuncTracker track(__func__);
    QC2Status retval = QC2_OK;
    (void)param;
    //Not implemented
    return retval;
}

QC2Status QC2ColorConvertFilter::onSessionReconfigure(FilterPortProp& ipPort,
                                                      FilterPortProp& opPort) {

    QLOGD_INST("onSessionReconfigure: Input h=%u, w=%u, fmt=%u, stride=%u, sl=%u",
        ipPort.height, ipPort.width, ipPort.format, ipPort.stride, ipPort.scanLines);

    mInputProps.height = ipPort.height;
    mInputProps.width = ipPort.width;
    mInputProps.format = ipPort.format;
    mInputProps.stride = ipPort.stride;
    mInputProps.scanLines= ipPort.scanLines;

#ifdef DEBUG_ONLY
    QLOGD_INST("onSessionReconfigure: Output h=%u, w=%u, [IGNORE]fmt=%u, [IGNORE]stride=%u, [IGNORE]sl=%u",
        opPort.height, opPort.width, opPort.format, opPort.stride, opPort.scanLines);
#else
    QLOGD_INST("onSessionReconfigure: Output h=%u, w=%u, fmt=%u, stride=%u, sl=%u",
        opPort.height, opPort.width, opPort.format, opPort.stride, opPort.scanLines);
#endif

    mOutputProps.height = opPort.height;
    mOutputProps.width = opPort.width;
#ifdef DEBUG_ONLY
    /* Update different color format on output port for reference colorconvert */
    mOutputProps.format = overrideOutputColorFormat();
    mOutputProps.stride = ALIGNED_WIDTH(static_cast<uint32_t>(mOutputProps.width), mOutputProps.format);
    mOutputProps.scanLines= ALIGNED_HEIGHT(static_cast<uint32_t>(mOutputProps.height), mOutputProps.format);
#else
    mOutputProps.format = opPort.format;
    mOutputProps.stride = opPort.stride;
    mOutputProps.scanLines = opPort.scanLines;
#endif
    return QC2_OK;
}

uint32_t QC2ColorConvertFilter::overrideOutputColorFormat() {
    switch(mInputProps.format) {
        case PixFormat::VENUS_NV12:
            return PixFormat::VENUS_NV12_UBWC;
        case PixFormat::VENUS_NV12_UBWC:
            return PixFormat::VENUS_NV12;

        case PixFormat::RGBA8888:
            return PixFormat::RGBA8888_UBWC;
        case PixFormat::RGBA8888_UBWC:
            return PixFormat::RGBA8888;

        case PixFormat::VENUS_TP10:
            return PixFormat::VENUS_P010;
        case PixFormat::VENUS_P010:
            return PixFormat::VENUS_TP10;

        default:
            return PixFormat::YUV420SP;
    }
}
};

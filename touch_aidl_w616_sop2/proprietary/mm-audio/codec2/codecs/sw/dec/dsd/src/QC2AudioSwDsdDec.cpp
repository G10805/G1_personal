/*
 **************************************************************************************************
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
 */
#include "QC2AudioSwDsdDec.h"

#include <cutils/properties.h>
#include <inttypes.h>

#include <algorithm>
#include <memory>
#include <string>

#include "QC2EventQueue.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwDsdDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static char const *DSD_DECODER_LIB = "libdsd2pcm.so";

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwDsdDec //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwDsdDec::QC2AudioSwDsdDec(const std::string &codecName,
                                   uint32_t sessionId,
                                   const std::string &instanceName,
                                   const std::string &mime, uint32_t variant,
                                   std::shared_ptr<EventHandler> listener)
    : QC2AudioSwCodec(codecName, sessionId, instanceName, mime, MimeType::RAW,
                      KIND_DECODER, variant, listener) {
    mDSDDecHandle = nullptr;
    mDSDLibHandle = nullptr;
    mMinOutBufSize = 0;
    mDecoderConfig = std::make_shared<QC2DsdDecConfig>();
}

QC2Status QC2AudioSwDsdDec::configure(const C2Param &param) {
    QC2Status retVal = QC2_OK;
    QLOGV("Configure %s, core-index=%x baseIndex=%x",
          DebugString::C2Param(param.index()).c_str(),
          param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexSampleRate: {
            C2StreamSampleRateInfo::input in;
            if (param.index() == in.index()) {
                auto inputSampleRate =
                    ((C2StreamSampleRateInfo::input *)&param)->value;
                if (!mDecoderConfig->isInputSampleRateValid(inputSampleRate)) {
                    QLOGE("Invalid sample rate %d", inputSampleRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: updated input sample rate %d", __func__, inputSampleRate);
                mDecoderConfig->setInputSampleRate(inputSampleRate);
            }
            break;
        }
        case kParamIndexChannelCount: {
            C2StreamChannelCountInfo::output out;
            if (param.index() == out.index()) {
                auto channels =
                    ((C2StreamChannelCountInfo::output *)&param)->value;
                if (!mDecoderConfig->isChannelCountValid(channels)) {
                    QLOGE("Invalid channel count %d", channels);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: channels %d", __func__, channels);
                mDecoderConfig->setInputChannelCount(channels);
            }
            break;
        }
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::output out;
            if (param.index() == out.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::output *)&param)->value;
                auto bitsPerSample =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mDecoderConfig->isOutputBitsPerSampleValid(bitsPerSample)) {
                    QLOGE("Invalid pcm encoding format %d", pcmEncoding);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bitsPerSample);
                //This sets outputs QFormat param
                mDecoderConfig->setOutputBitsPerSample(bitsPerSample);
            }
            break;
        }

        case kParamIndexAudioNumOutSamplesPerChannel: {
            C2AudioNumOutSamplesPerChannelInfo::output out;
            if (param.index() == out.index()) {
                auto NumOutSamplesPerChannel =
                    ((C2StreamPcmEncodingInfo::output *)&param)->value;
                if (!mDecoderConfig->isNumOutSamplesPerChannelValid(
                        NumOutSamplesPerChannel)) {
                    QLOGE("Invalid numOutSamplesPerChannel %d",
                          NumOutSamplesPerChannel);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: NumOutSamplesPerChannel %d", __func__,
                      NumOutSamplesPerChannel);
                mDecoderConfig->setInputNumOutSamplesPerChannel(
                    NumOutSamplesPerChannel);
            }
            break;
        }
        default: {
            retVal = QC2AudioSwCodec::configure(param);
            break;
        }
    }

    return retVal;
}

QC2Status QC2AudioSwDsdDec::onStop() {
    QLOGV("%s", __func__);

    DecErr_t result = DEC_FAILURE;
    InterfaceTypes_t initType = DEC_DEINIT;
    auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
    if (mDSDDecHandle != nullptr) {
        (*mDecoderInit)((void *)mDSDDecHandle, &result, nullptr,
                        reqMediaType.get(), initType);
    }
    if (mDSDLibHandle != nullptr) {
        dlclose(mDSDLibHandle);
    }

    mParamIntfMap.clear();
    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not allocated, return error", __func__);
        return QC2_CORRUPTED;
    }

    // Initialize DSD specific input CSIM configs with defaults
    mDecoderConfig->setInputSampleRate(DsdDefaults::DSDDefaultInputSampleRate);
    mDecoderConfig->setInputChannelCount(DsdDefaults::DSDDefaultNumChannels);
    mDecoderConfig->setInputNumOutSamplesPerChannel(
        DsdDefaults::DSDDefaultNumOutSamplesPerChannel);
    mDecoderConfig->setInputGain(DsdDefaults::DSDDefaultGain);
    mDecoderConfig->setInputQFormat(DsdDefaults::DSDDefaultQFormat);

    mDSDLibHandle = dlopen(DSD_DECODER_LIB, RTLD_LAZY);
    if (!mDSDLibHandle) {
        QLOGE("%s: CSIM decoder library open failed!", __func__);
        return QC2_CORRUPTED;
    }
    mDecoderInit = (DecoderInit)dlsym(mDSDLibHandle, "dsd_dec_interface");
    mProcessData = (DecoderLib_Process)dlsym(mDSDLibHandle, "dsd_dec_process");

    if (!mDecoderInit || !mProcessData) {
        QLOGE("%s: Symbols not found in the CSIM decoder library!", __func__);
        return QC2_CORRUPTED;
    }

    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::onFlush() {
    QLOGV("%s: reconfiguring component to update about samplerate", __func__);
    if (reconfigureComponent() != QC2_OK) {
        QLOGE("%s: Failed to reconfigure", __func__);
        return QC2_CORRUPTED;
    }
    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::onStart() {
    QLOGD_INST("%s", __func__);

    if (createDsdDecoder() != QC2_OK) {
        QLOGE("%s: unable to create DSD CSIM decoder", __func__);
        return QC2_CORRUPTED;
    }

    if (reconfigureComponent() != QC2_OK) {
        QLOGE("%s: Failed to reconfigure", __func__);
        return QC2_CORRUPTED;
    }
    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::process(const std::shared_ptr<QC2Buffer> &input,
                                    std::shared_ptr<QC2Buffer> &output) {

    DecErr_t result = DEC_FAILURE;
    uint64_t flags = input->flags();

    QLOGV(
        "%s: process input size:%d, output size:%d; input offset: "
        "%d; input flags %" PRIu64 "\n",
        __func__, input->linear().size(), output->linear().size(),
        input->linear().offset(), flags);

    auto inBuffer = std::make_shared<AudioBuf_t>();
    auto outBuffer = std::make_shared<AudioBuf_t>();

    if (flags & BufFlag::EOS) {
        QLOGD("%s: Parser reported EOS", __func__);
        inBuffer->mMarker.mEosFlag = 1;
    }

    uint32_t usedInBufLen = 0;
    uint32_t filledOutBufLen = 0;

    auto outMediaType = std::make_shared<GenericDecMediaFmt_t>();
    outMediaType->mDecSpecificFmt =
        (void *)(mDecoderConfig->getDsdDecOutputFmt().get());
    outMediaType->mBitsPerSample = mDecoderConfig->getOutputBitsPerSample();

    std::unique_ptr<QC2Buffer::Mapping> inBufMappingInfo =
        input->linear().map();
    if (inBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto inBufBasePtr = inBufMappingInfo->baseRW();
    if (inBufBasePtr == nullptr) {
        QLOGE("%s: Base ptr is null for input buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *inBufDataPtr = inBufBasePtr + input->linear().offset();

    std::unique_ptr<QC2Buffer::Mapping> outBufMappingInfo =
        output->linear().map();
    if (outBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto outBufBasePtr = outBufMappingInfo->baseRW();
    if (outBufBasePtr == nullptr) {
        QLOGE("%s: Base ptr is null for write output buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *outWriteBufDataPtr = outBufBasePtr + output->linear().offset();
    uint8_t *initialOutWriteBufDataPtr = outWriteBufDataPtr;

    uint32_t availInputLength = input->linear().size();
    uint32_t inBufConsumed = 0;
    uint32_t outBufConsumed = 0;
    while (availInputLength > 0) {
        inBufDataPtr = inBufDataPtr + inBufConsumed;
        inBuffer->mDataPtr = inBufDataPtr;
        inBuffer->mMaxDataLen = availInputLength;

        if (availInputLength < (mDecoderConfig->getDsdDecOutputFmt()->nInputBytesPerFrame)) {
            QLOGV("%s: forcing EOS on input buffer", __func__);
            inBuffer->mActualDataLen = availInputLength;
            inBuffer->mMarker.mEosFlag = 1;
        } else {
            inBuffer->mActualDataLen = mDecoderConfig->getDsdDecOutputFmt()->nInputBytesPerFrame;
            inBuffer->mMarker.mEosFlag = 0;
        }

        outWriteBufDataPtr = outWriteBufDataPtr + outBufConsumed;
        outBuffer->mDataPtr = outWriteBufDataPtr;
        outBuffer->mMaxDataLen = mMinOutBufSize;
        outBuffer->mActualDataLen = mMinOutBufSize;

        inBufConsumed = 0;
        outBufConsumed = 0;

        QLOGV(
            "%s: Calling Decode for input buffer size: %d and output buffer "
            "size %d",
            __func__, inBuffer->mActualDataLen, outBuffer->mMaxDataLen);
        if (!mProcessData) {
            QLOGE("%s: CSIM functions not initialized", __func__);
            return QC2_CORRUPTED;
        }
        (*mProcessData)(reinterpret_cast<void *>(mDSDDecHandle), &result,
                        inBuffer.get(), outBuffer.get(), outMediaType.get());
        inBufConsumed = inBuffer->mActualDataLen;
        outBufConsumed = outBuffer->mActualDataLen;
        usedInBufLen += inBufConsumed;
        filledOutBufLen += outBufConsumed;
        availInputLength -= inBufConsumed;

        if ((result == DEC_SUCCESS) || (result == DEC_EOF)) {
            QLOGV(
                "%s: Decode successful consumed bytes are %d and output frame "
                "size is %d",
                __func__, inBufConsumed, outBufConsumed);
        } else {
            break;
        }
    }

    if (usedInBufLen != input->linear().size()) {
        QLOGV("%s: Input Buffer not fully decoded", __func__);
    } else {
        QLOGV("%s: Input Buffer fully decoded", __func__);
    }
    if ((result == DEC_SUCCESS) || (result == DEC_EOF)) {
        output->linear().setRange(0, filledOutBufLen);
    } else {
        // incase of csim decoder error, we sending zeros as data instead QC2_CORRUPTED
        QLOGW("%s: decoder returned error %d, substituting silence", __func__,
              result);
        outWriteBufDataPtr = initialOutWriteBufDataPtr;
        memset(outWriteBufDataPtr, 0, mMinOutBufSize);
        output->linear().setRange(0, mMinOutBufSize);
    }
    output->setFlags(flags);
    onOutputDone(output);
    onInputDone(input);
    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::reconfigureComponent() {
    // Update sample rate always
    auto sampleRate = mDecoderConfig->getDsdDecOutputFmt()->nOutSampleRate;
    auto sampleRateParam =
        std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
    mUpdatedInfos.push_back(sampleRateParam);
    mParamIntfMap[kParamIndexSampleRate] = sampleRate;

    // Update bits per sample
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(
            mDecoderConfig->getOutputBitsPerSample());
    if (pcmEncoding != mParamIntfMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam =
            std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mParamIntfMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update Num Out Samples Per Channel
    auto numOutSamplesPerChannel = mDecoderConfig->getInputNumOutSamplesPerChannel();
    if (numOutSamplesPerChannel !=
        mParamIntfMap[kParamIndexAudioNumOutSamplesPerChannel]) {
        auto numOutSamplesPerChannelParam =
            std::make_shared<C2AudioNumOutSamplesPerChannelInfo::output>(
                0, numOutSamplesPerChannel);
        mUpdatedInfos.push_back(numOutSamplesPerChannelParam);
        mParamIntfMap[kParamIndexAudioNumOutSamplesPerChannel] =
            numOutSamplesPerChannel;
    }

    // Update channels
    auto channels = mDecoderConfig->getDsdDecOutputFmt()->nOutChannels;
    if (channels != mParamIntfMap[kParamIndexChannelCount]) {
        auto channelsParam =
            std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mParamIntfMap[kParamIndexChannelCount] = channels;
    }

    // update Max Input Buffer Size
    auto maxInputBufferSize =
        mDecoderConfig->getDsdDecOutputFmt()->nInputBytesPerFrame;
    if (maxInputBufferSize != mParamIntfMap[kParamIndexMaxBufferSize]) {
        auto maxInputBufferSizeParam =
            std::make_shared<C2StreamMaxBufferSizeInfo::input>(
                0, maxInputBufferSize);
        mUpdatedInfos.push_back(maxInputBufferSizeParam);
        mParamIntfMap[kParamIndexMaxBufferSize] = maxInputBufferSize;
    }

    return QC2_OK;
}

QC2Status QC2AudioSwDsdDec::createDsdDecoder() {
    // create DSD Decoder
    if (mDSDDecHandle != nullptr) {
        QLOGI(
            "%s: dsd decoder already initialized, dsd decoder handle not "
            "nullptr",
            __func__);
        return QC2_OK;
    }

    // Configure decoder to output 20 msec data for DSD256 and DSD512
    if (mDecoderConfig->getInputSampleRate() == DsdInputSampleRate::SR_DSD256 ||
        mDecoderConfig->getInputSampleRate() == DsdInputSampleRate::SR_DSD512) {
        // increase samples per channel
        mDecoderConfig->setInputNumOutSamplesPerChannel(
            DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L4);
    }
    QLOGV("%s: InputConfigs: inputSampleRate: %d, inputChannelCount:%d",
          __func__, mDecoderConfig->getInputSampleRate(),
          mDecoderConfig->getInputChannelCount());

    DecErr_t result = DEC_FAILURE;
    InterfaceTypes_t initType = DEC_INIT;

    auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
    reqMediaType->mDecSpecificFmt =
        (void *)(mDecoderConfig->getDsdDecOutputFmt().get());

    // create DSD handle
    mDSDDecHandle =
        (*mDecoderInit)((void *)mDSDDecHandle, &result,
                        (void *)(mDecoderConfig->getDsdDecInputConfig().get()),
                        reqMediaType.get(), initType);

    if (result != DEC_SUCCESS || !mDSDDecHandle) {
        mDSDDecHandle = nullptr;
        QLOGE("%s: Unable to setup DSD decoder", __func__);
        return QC2_CORRUPTED;
    }

    // set output bits per sample
    mDecoderConfig->setOutputBitsPerSample(
        (mDecoderConfig->getDsdDecOutputFmt()->nOutBytesSample) * 8);
    auto csimInputFrameSize =
        mDecoderConfig->getDsdDecOutputFmt()->nInputBytesPerFrame;

    // DSD CSIM requires below atleastoutput buffer size per each encoded
    // inputframe
    auto csimOutputBufferSize =
        (((mDecoderConfig->getDsdDecOutputFmt()->nOutSampleFrame) *
          mDecoderConfig->getOutputBitsPerSample()) /
         8);

    // set large enough output buffer to accomodate more encoded input frame
    // size
    mMinOutBufSize =
        std::get<DSD_MAX_OUTPUT_BUF_SIZE>(DsdDecMinMaxList);  // 768KB

    QLOGV(
        "%s: OutSampleRate:%d, outChannelCount:%d, OutBitsPerSample:%d; each "
        "DSD CSIM process decodes inputframe of size:%d, output number of PCM "
        "samples:%d, atleast output buffer size required:%d, allocating output "
        "buffer size:%d",
        __func__, mDecoderConfig->getDsdDecOutputFmt()->nOutSampleRate,
        mDecoderConfig->getDsdDecOutputFmt()->nOutChannels,
        mDecoderConfig->getOutputBitsPerSample(), csimInputFrameSize,
        mDecoderConfig->getDsdDecOutputFmt()->nOutSampleFrame,
        csimOutputBufferSize, mMinOutBufSize);
    return QC2_OK;
}

}  // namespace qc2audio

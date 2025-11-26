/*
 **************************************************************************************************
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <algorithm>
#include <inttypes.h>
#include <memory>
#include <string>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioSwAlacDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwAlacDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static char const * ALAC_DECODER_LIB = "libAlacSwDec.so";

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwAlacDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwAlacDec::QC2AudioSwAlacDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
            : QC2AudioSwCodec(codecName, sessionId, instanceName, mime,
                    MimeType::RAW, KIND_DECODER, variant, listener) {
        mALACDecHandle = nullptr;
        mALACLibHandle = nullptr;
        mMinOutBufSize = 0;
        mDecoderConfig = std::make_shared<QC2AlacDecConfig>();
        mCHReorder = std::make_shared<CHReorder>();
}

QC2Status QC2AudioSwAlacDec::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;
    QLOGV("Configure %s, core-index=%x baseIndex=%x",
               DebugString::C2Param(param.index()).c_str(),
               param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexSampleRate: {
            C2StreamSampleRateInfo::output out;
            if (param.index() == out.index()) {
                auto sampleRate =
                    ((C2StreamSampleRateInfo::output*)&param)->value;
                if (!mDecoderConfig->isSampleRateValid(sampleRate)) {
                    QLOGE("Invalid sample rate %d", sampleRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: sample rate %d", __func__, sampleRate);
                mDecoderConfig->setSampleRate(sampleRate);
            }
            break;
        }
        case kParamIndexChannelCount: {
            C2StreamChannelCountInfo::output out;
            if (param.index() == out.index()) {
                auto channels =
                    ((C2StreamChannelCountInfo::output*)&param)->value;
                if (!mDecoderConfig->isChannelCountValid(channels)) {
                    QLOGE("Invalid channel count %d", channels);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: channels %d", __func__, channels);
                mDecoderConfig->setChannels(channels);
            }
            break;
        }
        case kParamIndexBitrate: {
            C2StreamBitrateInfo::input out;
            if (param.index() == out.index()) {
                auto bitrate = ((C2StreamBitrateInfo::input*)&param)->value;
                QLOGV("%s: avg bitrate %d", __func__, bitrate);
                mDecoderConfig->setAvgBitRate(bitrate);
            }
            break;
        }
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::output out;
            if (param.index() == out.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::output*)&param)->value;
                auto bitsPerSample =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mDecoderConfig->isBitsPerSampleValid(bitsPerSample)) {
                    QLOGE("Invalid pcm encoding format %d", pcmEncoding);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bitsPerSample);
                mDecoderConfig->setBitDepth(bitsPerSample);
            }
            break;
        }
        case kParamIndexAudioFrameLength: {
            C2AudioFrameLengthInfo::input in;
            if (param.index() == in.index()) {
                auto frameLength =
                    ((C2AudioFrameLengthInfo::input*)&param)->value;
                QLOGV("%s: frame length %d", __func__, frameLength);
                mDecoderConfig->setFrameLength(frameLength);
            }
            break;
        }
        case kParamIndexAudioPbTuning: {
            C2AudioPbTuningParam::input in;
            if (param.index() == in.index()) {
                auto pB =
                    ((C2AudioPbTuningParam::input*)&param)->value;
                QLOGV("%s: pB %d", __func__, pB);
                mDecoderConfig->setPbTuningParam(pB);
            }
            break;
        }
        case kParamIndexAudioMbTuning: {
            C2AudioMbTuningParam::input in;
            if (param.index() == in.index()) {
                auto mB =
                    ((C2AudioMbTuningParam::input*)&param)->value;
                QLOGV("%s: mB %d", __func__, mB);
                mDecoderConfig->setMbTuningParam(mB);
            }
            break;
        }
        case kParamIndexAudioKbTuning: {
            C2AudioKbTuningParam::input in;
            if (param.index() == in.index()) {
                auto kB =
                    ((C2AudioKbTuningParam::input*)&param)->value;
                QLOGV("%s: kB %d", __func__, kB);
                mDecoderConfig->setKbTuningParam(kB);
            }
            break;
        }
        case kParamIndexAudioMaxFrameBytes: {
            C2AudioMaxFrameBytesInfo::input in;
            if (param.index() == in.index()) {
                auto maxFrameBytes =
                    ((C2AudioMaxFrameBytesInfo::input*)&param)->value;
                QLOGV("%s: max frame bytes %d", __func__, maxFrameBytes);
                mDecoderConfig->setMaxFrameBytes(maxFrameBytes);
            }
            break;
        }
        case kParamIndexAudioMaxRun: {
            C2AudioMaxRunInfo::input in;
            if (param.index() == in.index()) {
                auto maxRun =
                    ((C2AudioMaxRunInfo::input*)&param)->value;
                QLOGV("%s: max run %d", __func__, maxRun);
                mDecoderConfig->setMaxRun(maxRun);
            }
            break;
        }
        default: {
            QC2AudioSwCodec::configure(param);
            break;
        }
    }

    return retVal;
}

QC2Status QC2AudioSwAlacDec::onStop() {
    QLOGV("%s", __func__);

    DecErr_t result = DEC_FAILURE;
    InterfaceTypes_t initType = DEC_DEINIT;
    auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
    if (mALACDecHandle != nullptr) {
        (*mDecoderInit)(reinterpret_cast<void*>(mALACDecHandle), &result,
                reinterpret_cast<void*>(mDecoderConfig->getAlacDecConfig().get()),
            reqMediaType.get(), initType);
        dlclose(mALACLibHandle);
    }
    mParamIntfMap.clear();
    return QC2_OK;
}

QC2Status QC2AudioSwAlacDec::onInit() {
    QLOGD_INST("%s", __func__);

    // Initialize ALAC specific CSIM structure with defaults
    mDecoderConfig->setChannels(AlacDefaults::ALACDefaultChannels);
    mDecoderConfig->setBitDepth(AlacDefaults::ALACDefaultBitDepth);
    mDecoderConfig->setSampleRate(AlacDefaults::ALACDefaultSampleRate);
    mDecoderConfig->setCompatibleVersion(AlacDefaults::ALACCompatibleVersion);
    mDecoderConfig->setFrameLength(AlacDefaults::ALACDefaultFrameSize);
    mDecoderConfig->setMaxFrameBytes(AlacDefaults::ALACDefaultMaxFrameBytes);
    mDecoderConfig->setAvgBitRate(AlacDefaults::ALACDefaultAvgBitRate);
    mDecoderConfig->setPbTuningParam(AlacDefaults::ALACDefaultPbTuningParam);
    mDecoderConfig->setMbTuningParam(AlacDefaults::ALACDefaultMbTuningParam);
    mDecoderConfig->setKbTuningParam(AlacDefaults::ALACDefaultKbTuningParam);
    mDecoderConfig->setMaxRun(AlacDefaults::ALACDefaultMaxRun);

    mALACLibHandle = dlopen(ALAC_DECODER_LIB, RTLD_LAZY);
    if (!mALACLibHandle) {
        QLOGE("%s: CSIM decoder library open failed!", __func__);
        return QC2_ERROR;
    }
    mDecoderInit = (DecoderInit) dlsym (mALACLibHandle, "alac_dec_interface");
    mProcessData = (DecoderLib_Process) dlsym (mALACLibHandle, "alac_dec_process");

    if (!mDecoderInit || !mProcessData) {
        QLOGE("%s: Symbols not found in the CSIM decoder library!", __func__);
        return QC2_NOT_FOUND;
    }

    return QC2_OK;
}

QC2Status QC2AudioSwAlacDec::onStart() {
    QLOGD_INST("%s", __func__);

    mMinOutBufSize = mDecoderConfig->getFrameLength() *
                         (mDecoderConfig->getBitDepth() >> 3) * mDecoderConfig->getChannels();

    return QC2_OK;
}

QC2Status QC2AudioSwAlacDec::process(const std::shared_ptr<QC2Buffer>& input,
                                     std::shared_ptr<QC2Buffer>& output) {

    uint32_t availLength = input->linear().size();
    DecErr_t result = DEC_FAILURE;
    uint64_t flags = input->flags();

    if (flags & BufFlag::EOS) {
        QLOGD("%s: Parser reported EOS", __func__);
    }

    auto inBuffer = std::make_shared<AudioBuf_t>();
    auto outBuffer = std::make_shared<AudioBuf_t>();
    auto outMediaType = std::make_unique<GenericDecMediaFmt_t>();

    std::unique_ptr<QC2Buffer::Mapping> bufMappingInfo = input->linear().map();
    if (bufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto basePtr = bufMappingInfo->baseRW();
    if (basePtr == nullptr) {
        QLOGE("%s: Base ptr is null for input buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *inputOffset = basePtr + input->linear().offset();
    inBuffer->mDataPtr = inputOffset;
    inBuffer->mMaxDataLen = availLength;
    inBuffer->mActualDataLen = availLength;

    QLOGV("%s: size %d offset %d flags %" PRIu64 "\n",
          __func__, availLength, input->linear().offset(), flags);

    if (flags & BufFlag::CODEC_CONFIG) {
        mDecoderConfig->setAlacDecConfig(inputOffset);

        QLOGD("%s: ALAC CSD values: frameLength %d bitDepth %d numChannels %d"
                                  " maxFrameBytes %d avgBitRate %d samplingRate %d\n"
                                  " compatibleVersion %d maxRun %d pb %d mb %d kb %d",
              __func__, mDecoderConfig->getFrameLength(), mDecoderConfig->getBitDepth(),
              mDecoderConfig->getChannels(), mDecoderConfig->getMaxFrameBytes(),
              mDecoderConfig->getAvgBitRate(), mDecoderConfig->getSampleRate(),
              mDecoderConfig->getCompatibleVersion(), mDecoderConfig->getMaxRun(),
              mDecoderConfig->getPbTuningParam(), mDecoderConfig->getMbTuningParam(),
              mDecoderConfig->getKbTuningParam());

        if (mDecoderConfig->checkAlacCSDParameters() != QC2_OK) {
            return QC2_CORRUPTED;
        }

        DecErr_t result = DEC_FAILURE;
        InterfaceTypes_t initType = DEC_INIT;
        auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
        if (!mDecoderInit) {
            QLOGE("%s: CSIM functions not initialized", __func__);
            return QC2_NOT_FOUND;
        }
        mALACDecHandle = (*mDecoderInit)(reinterpret_cast<void*>(mALACDecHandle), &result,
                                reinterpret_cast<void*>(mDecoderConfig->getAlacDecConfig().get()),
            reqMediaType.get(), initType);
        if (result != DEC_SUCCESS || !mALACDecHandle) {
            QLOGD("%s: Unable to setup decoder", __func__);
            return QC2_CANNOT_DO;
        }

        mMinOutBufSize = mDecoderConfig->getFrameLength() *
                             (mDecoderConfig->getBitDepth() >> 3) * mDecoderConfig->getChannels();
        mLinearPool->setBufferSize(mMinOutBufSize);

        reconfigureComponent();
        output->setFlags(flags);
        output->linear().setRange(0, 0);
        onOutputDone(output);
        onInputDone(input);
        return QC2_OK;
    }

    // writeBuffer: scratch buffer for ALAC CSIM
    std::shared_ptr<QC2Buffer> writeBuffer;
    QC2Status status = mLinearPool->allocate(&writeBuffer);
    if (status != QC2_OK || !writeBuffer) {
        QLOGE("%s: Could not allocate memory", __func__);
        return QC2_NO_MEMORY;
    }
    std::unique_ptr<QC2Buffer::Mapping> writeBufMappingInfo = writeBuffer->linear().map();
    if (writeBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto writeBufBasePtr = writeBufMappingInfo->baseRW();
    if (writeBufBasePtr == nullptr) {
        QLOGE("%s: Base ptr is null for write buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *writeBufOffset = writeBufBasePtr + writeBuffer->linear().offset();

    outBuffer->mDataPtr = writeBufOffset;
    outBuffer->mMaxDataLen = mMinOutBufSize;
    outBuffer->mActualDataLen = mMinOutBufSize;

    if (availLength != 0) {
        QLOGV("%s: Calling Decode in frame mode input buffer size is %d and output buff size %d",
               __func__, inBuffer->mMaxDataLen, outBuffer->mMaxDataLen);
        if (!mProcessData) {
            QLOGE("%s: CSIM functions not initialized", __func__);
            return QC2_NOT_FOUND;
        }
        (*mProcessData)(reinterpret_cast<void*>(mALACDecHandle), &result,
                 inBuffer.get(), outBuffer.get(), outMediaType.get());
    } else {
        QLOGI("Received zero length buffer, returning as is.");
        output->setFlags(flags);
        output->linear().setRange(0, availLength);
        onOutputDone(output);
        onInputDone(input);
        return QC2_OK;
    }

    std::unique_ptr<QC2Buffer::Mapping> outbufMappingInfo = output->linear().map();
    if (outbufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    uint8_t *outputOffset = outbufMappingInfo->baseRW();
    if (outputOffset == nullptr) {
        QLOGE("%s: Base ptr is null for output buffer", __func__);
        return QC2_NO_MEMORY;
    }

    if ((result == DEC_SUCCESS) || (result == DEC_EOF)) {
        QLOGV("%s: Decode successful consumed bytes are %d and output frame size is %d",
               __func__, inBuffer->mActualDataLen, outBuffer->mActualDataLen);
        if (output->linear().capacity() < outBuffer->mActualDataLen) {
            QLOGV("%s: insufficient output buffer size", __func__);
            return QC2_ERROR;
        }
        uint32_t outSize = output->linear().capacity();
        mCHReorder->setChannelCount((uint8_t)outMediaType->mNumChannels);
        mCHReorder->setBitDepth(outMediaType->mBitsPerSample);
        mCHReorder->setSampleCount(mDecoderConfig->getFrameLength());
        mCHReorder->setSrcChMap(outMediaType->mChannelMap, false/* default */);
        mCHReorder->setTgtChMap(NULL, true/* default */);
        mCHReorder->remap(outputOffset, (uint8_t *)outBuffer->mDataPtr, outSize);

        output->setFlags(flags);
        output->linear().setRange(0, outBuffer->mActualDataLen);
        onOutputDone(output);
        onInputDone(input);
    } else {
        QLOGW("%s: decoder returned error %d, substituting silence", __func__, result);
        memset(outputOffset, 0, mMinOutBufSize);
        output->setFlags(flags);
        output->linear().setRange(0, mMinOutBufSize);
        onOutputDone(output);
        onInputDone(input);
    }
    return QC2_OK;
}

QC2Status QC2AudioSwAlacDec::reconfigureComponent() {
    // Update sample rate
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mParamIntfMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mParamIntfMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitDepth());
    if (pcmEncoding != mParamIntfMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mParamIntfMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update channels
    auto channels = mDecoderConfig->getChannels();
    if (channels != mParamIntfMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mParamIntfMap[kParamIndexChannelCount] = channels;
    }

    // Update bit rate
    auto bitRate = mDecoderConfig->getAvgBitRate();
    if (bitRate != mParamIntfMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::input>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mParamIntfMap[kParamIndexBitrate] = bitRate;
    }

    // Update frame length
    auto frameLength = mDecoderConfig->getFrameLength();
    if (frameLength != mParamIntfMap[kParamIndexAudioFrameLength]) {
        auto frameLengthParam = std::make_shared<C2AudioFrameLengthInfo::input>(0, frameLength);
        mUpdatedInfos.push_back(frameLengthParam);
        mParamIntfMap[kParamIndexAudioFrameLength] = frameLength;
    }

    // Update pB tuning
    auto pB = mDecoderConfig->getPbTuningParam();
    if (pB != mParamIntfMap[kParamIndexAudioPbTuning]) {
        auto pBParam = std::make_shared<C2AudioPbTuningParam::input>(0, pB);
        mUpdatedInfos.push_back(pBParam);
        mParamIntfMap[kParamIndexAudioPbTuning] = pB;
    }

    // Update mB tuning
    auto mB = mDecoderConfig->getMbTuningParam();
    if (mB != mParamIntfMap[kParamIndexAudioMbTuning]) {
        auto mBParam = std::make_shared<C2AudioMbTuningParam::input>(0, mB);
        mUpdatedInfos.push_back(mBParam);
        mParamIntfMap[kParamIndexAudioMbTuning] = mB;
    }

    // Update kB tuning
    auto kB = mDecoderConfig->getKbTuningParam();
    if (kB != mParamIntfMap[kParamIndexAudioKbTuning]) {
        auto kBParam = std::make_shared<C2AudioKbTuningParam::input>(0, kB);
        mUpdatedInfos.push_back(kBParam);
        mParamIntfMap[kParamIndexAudioKbTuning] = kB;
    }

    // Update max frame bytes
    auto maxFrameBytes = mDecoderConfig->getMaxFrameBytes();
    if (maxFrameBytes != mParamIntfMap[kParamIndexAudioMaxFrameBytes]) {
        auto maxFrameBytesParam = std::make_shared<C2AudioMaxFrameBytesInfo::input>(0, maxFrameBytes);
        mUpdatedInfos.push_back(maxFrameBytesParam);
        mParamIntfMap[kParamIndexAudioMaxFrameBytes] = maxFrameBytes;
    }

    // Update max run
    auto maxRun = mDecoderConfig->getMaxRun();
    if (maxRun != mParamIntfMap[kParamIndexAudioMaxRun]) {
        auto maxRunParam = std::make_shared<C2AudioMaxRunInfo::input>(0, maxRun);
        mUpdatedInfos.push_back(maxRunParam);
        mParamIntfMap[kParamIndexAudioMaxRun] = maxRun;
    }

    return QC2_OK;
}

}   // namespace qc2audio

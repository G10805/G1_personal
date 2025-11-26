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
#include "QC2AudioHwAlacDec.h"
#include "QC2AudioHwUtils.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAlacDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwAlacDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAlacDec::QC2AudioHwAlacDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, mime,
                              MimeType::RAW, KIND_DECODER, variant, listener) {
        mOutBufSize = 0;
        mDecoderConfig = std::make_shared<QC2AlacDecConfig>();
}

QC2Status QC2AudioHwAlacDec::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;

    QLOGV_INST("configure: core-index=%x baseIndex=%x",
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
            QC2AudioHwCodec::configure(param);
            break;
        }
    }
    return retVal;
}

QC2Status QC2AudioHwAlacDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not created, return error", __func__);
        return QC2_ERROR;
    }

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
    mDecoderConfig->setChannelLayoutTag(AlacDefaults::ALACDefaultChannelLayoutTag);

    //Setting default output buffer size
    mOutBufSize = mDecoderConfig->getMaxOutputBufSize();

    return QC2_OK;
}

QC2Status QC2AudioHwAlacDec::onStart() {
    QLOGD_INST("%s", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwAlacDec::onCodecConfig(std::shared_ptr<QC2Buffer> input) {
    QC2Status retVal = QC2_OK;
    uint32_t availLength = input->linear().size();
    uint64_t flags = input->flags();
    bool hasCSDChanged = false;

    QLOGV_INST("%s", __func__);

    // Get input buffer's data offset
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

    QLOGV("%s: size %d offset %d flags %" PRIu64 "\n",
          __func__, availLength, input->linear().offset(), flags);

    // Set ALAC decoder config from input CSD
    mDecoderConfig->setAlacDecConfig(inputOffset);
    updateConfigParams(&hasCSDChanged);

    if (getHwSessionId()) {
        if (hasCSDChanged) {
            // onCodecConfig in a running session can come with seek operation
            // Flush is assumed to be handled by client as a part of seek
            QLOGW("%s: CSD changed in a running session", __func__);
        } else {
            QLOGW("%s: CSD unchanged, no action needed", __func__);
        }
        return QC2_OK;
    }

    QLOGD("%s: ALAC CSD values: frameLength %d bitDepth %d numChannels %d"
                        " maxFrameBytes %d avgBitRate %d samplingRate %d\n"
                        " compatibleVersion %d maxRun %d pb %d mb %d kb %d",
            __func__, mDecoderConfig->getFrameLength(), mDecoderConfig->getBitDepth(),
            mDecoderConfig->getChannels(), mDecoderConfig->getMaxFrameBytes(),
            mDecoderConfig->getAvgBitRate(), mDecoderConfig->getSampleRate(),
            mDecoderConfig->getCompatibleVersion(), mDecoderConfig->getMaxRun(),
            mDecoderConfig->getPbTuningParam(), mDecoderConfig->getMbTuningParam(),
            mDecoderConfig->getKbTuningParam());

    if (mDecoderConfig->getChannels() > 2 &&
            mDecoderConfig->getSampleRate() > sAlacMaxMultiChSampleRate) {
        QLOGE("%s: ALAC HW component cannot support %u sample rate for multi-channel clips",
              __func__, mDecoderConfig->getSampleRate());
        return QC2_CORRUPTED;
    }

    if (mDecoderConfig->checkAlacCSDParameters() != QC2_OK) {
        return QC2_CORRUPTED;
    }

    // Setting output buffer size for ALAC
    mOutBufSize = mDecoderConfig->getFrameLength() *
                      (mDecoderConfig->getBitDepth() >> 3) * mDecoderConfig->getChannels();
    mLinearPool->setBufferSize(mOutBufSize);
    QLOGV("%s: Output Buffer size=%u", __func__, mOutBufSize);

    mInputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitDepth(),
                        .channel_count = mDecoderConfig->getChannels(),
                        .codec_type = Type_ALAC,
                        .codec_info_size = sizeof(mDecoderConfig),
                        .codec_info = (void *)(mDecoderConfig->getAlacDecConfig().get())};
    mOutputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitDepth(),
                        .channel_count = mDecoderConfig->getChannels(),
                        .codec_type = Type_PCM};
    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }

    return mAudioHw->start_session(getHwSessionId());
}

void QC2AudioHwAlacDec::updateConfigParams(bool *hasCSDChanged) {

    // Update sample rate on interface and output buffer
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
        *hasCSDChanged = true;
    }

    // Update bits per sample on interface and output buffer
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitDepth());
    if (pcmEncoding != mIntfParamValuesMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mIntfParamValuesMap[kParamIndexPcmEncoding] = pcmEncoding;
        *hasCSDChanged = true;
    }

    // Update channels
    auto channels = mDecoderConfig->getChannels();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
        *hasCSDChanged = true;
    }

    // Update bit rate
    auto bitRate = mDecoderConfig->getAvgBitRate();
    if (bitRate != mIntfParamValuesMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::input>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mIntfParamValuesMap[kParamIndexBitrate] = bitRate;
        *hasCSDChanged = true;
    }

    // Update frame length
    auto frameLength = mDecoderConfig->getFrameLength();
    if (frameLength != mIntfParamValuesMap[kParamIndexAudioFrameLength]) {
        auto frameLengthParam = std::make_shared<C2AudioFrameLengthInfo::input>(0, frameLength);
        mUpdatedInfos.push_back(frameLengthParam);
        mIntfParamValuesMap[kParamIndexAudioFrameLength] = frameLength;
        *hasCSDChanged = true;
    }

    // Update pB tuning
    auto pB = mDecoderConfig->getPbTuningParam();
    if (pB != mIntfParamValuesMap[kParamIndexAudioPbTuning]) {
        auto pBParam = std::make_shared<C2AudioPbTuningParam::input>(0, pB);
        mUpdatedInfos.push_back(pBParam);
        mIntfParamValuesMap[kParamIndexAudioPbTuning] = pB;
        *hasCSDChanged = true;
    }

    // Update mB tuning
    auto mB = mDecoderConfig->getMbTuningParam();
    if (mB != mIntfParamValuesMap[kParamIndexAudioMbTuning]) {
        auto mBParam = std::make_shared<C2AudioMbTuningParam::input>(0, mB);
        mUpdatedInfos.push_back(mBParam);
        mIntfParamValuesMap[kParamIndexAudioMbTuning] = mB;
        *hasCSDChanged = true;
    }

    // Update kB tuning
    auto kB = mDecoderConfig->getKbTuningParam();
    if (kB != mIntfParamValuesMap[kParamIndexAudioKbTuning]) {
        auto kBParam = std::make_shared<C2AudioKbTuningParam::input>(0, kB);
        mUpdatedInfos.push_back(kBParam);
        mIntfParamValuesMap[kParamIndexAudioKbTuning] = kB;
        *hasCSDChanged = true;
    }

    // Update max frame bytes
    auto maxFrameBytes = mDecoderConfig->getMaxFrameBytes();
    if (maxFrameBytes != mIntfParamValuesMap[kParamIndexAudioMaxFrameBytes]) {
        auto maxFrameBytesParam = std::make_shared<C2AudioMaxFrameBytesInfo::input>(0, maxFrameBytes);
        mUpdatedInfos.push_back(maxFrameBytesParam);
        mIntfParamValuesMap[kParamIndexAudioMaxFrameBytes] = maxFrameBytes;
        *hasCSDChanged = true;
    }

    // Update max run
    auto maxRun = mDecoderConfig->getMaxRun();
    if (maxRun != mIntfParamValuesMap[kParamIndexAudioMaxRun]) {
        auto maxRunParam = std::make_shared<C2AudioMaxRunInfo::input>(0, maxRun);
        mUpdatedInfos.push_back(maxRunParam);
        mIntfParamValuesMap[kParamIndexAudioMaxRun] = maxRun;
        *hasCSDChanged = true;
    }
}

}   // namespace qc2audio

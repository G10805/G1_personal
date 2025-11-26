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
#include "QC2AudioHwApeDec.h"
#include "QC2AudioHwUtils.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwApeDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwApeDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwApeDec::QC2AudioHwApeDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
        :QC2AudioHwCodec(codecName, sessionId, instanceName, mime,
                         MimeType::RAW, KIND_DECODER, variant, listener) {
        mOutBufSize = 0;
        mDecoderConfig = std::make_shared<QC2ApeDecConfig>();
}

QC2Status QC2AudioHwApeDec::configure(const C2Param& param) {
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
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::output out;
            if (param.index() == out.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::output*)&param)->value;
                auto bitsPerSample =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mDecoderConfig->isBitsPerSampleValid(bitsPerSample)) {
                    QLOGE("Invalid bitsPerSample %d", bitsPerSample);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bitsPerSample);
                mDecoderConfig->setBitsPerSample(bitsPerSample);
            }
            break;
        }
        case kParamIndexAudioCompressionLevel: {
            C2AudioCompressionLevelInfo::input in;
            if (param.index() == in.index()) {
                auto compressionLevel =
                    ((C2AudioCompressionLevelInfo::input*)&param)->value;
                QLOGV("%s: compression level %d", __func__, compressionLevel);
                mDecoderConfig->setCompressionLevel(compressionLevel);
            }
            break;
        }
        case kParamIndexAudioFormatFlags: {
            C2AudioFormatFlagsInfo::input in;
            if (param.index() == in.index()) {
                auto formatFlags =
                    ((C2AudioFormatFlagsInfo::input*)&param)->value;
                QLOGV("%s: format flags %d", __func__, formatFlags);
                mDecoderConfig->setFormatFlags(formatFlags);
            }
            break;
        }
        case kParamIndexAudioBlocksPerFrame: {
            C2AudioBlocksPerFrameInfo::input in;
            if (param.index() == in.index()) {
                auto blocksPerFrame =
                    ((C2AudioBlocksPerFrameInfo::input*)&param)->value;
                QLOGV("%s: blocks per frame %d", __func__, blocksPerFrame);
                mDecoderConfig->setBlocksPerFrame(blocksPerFrame);
            }
            break;
        }
        case kParamIndexAudioFinalFrameBlocks: {
            C2AudioFinalFrameBlocksInfo::input in;
            if (param.index() == in.index()) {
                auto finalFrameBlocks =
                    ((C2AudioFinalFrameBlocksInfo::input*)&param)->value;
                QLOGV("%s: final frame blocks %d", __func__, finalFrameBlocks);
                mDecoderConfig->setFinalFrameBlocks(finalFrameBlocks);
            }
            break;
        }
        case kParamIndexAudioTotalFrames: {
            C2AudioTotalFramesInfo::input in;
            if (param.index() == in.index()) {
                auto totalFrames =
                    ((C2AudioTotalFramesInfo::input*)&param)->value;
                QLOGV("%s: total frames %d", __func__, totalFrames);
                mDecoderConfig->setTotalFrames(totalFrames);
            }
            break;
        }
        case kParamIndexAudioSeekTablePresent: {
            C2AudioSeekTablePresentInfo::input in;
            if (param.index() == in.index()) {
                auto seekTablePresent =
                    ((C2AudioSeekTablePresentInfo::input*)&param)->value;
                QLOGV("%s: seek table %d", __func__, seekTablePresent);
                mDecoderConfig->setSeekTablePresent(seekTablePresent);
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

QC2Status QC2AudioHwApeDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not created, return error", __func__);
        return QC2_ERROR;
    }

    // Initialize APE specific CSIM structure with defaults
    mDecoderConfig->setChannels(ApeDefaults::APEDefaultNumChannels);
    mDecoderConfig->setBitsPerSample(ApeDefaults::APEDefaultBitsPerSample);
    mDecoderConfig->setSampleRate(ApeDefaults::APEDefaultSampleRate);
    mDecoderConfig->setCompatibleVersion(ApeDefaults::APECompatibleVersion);
    mDecoderConfig->setCompressionLevel(ApeDefaults::APEDefaultCompressionLevel);
    mDecoderConfig->setFormatFlags(ApeDefaults::APEDefaultFormatFlags);
    mDecoderConfig->setBlocksPerFrame(ApeDefaults::APEDefaultBlocksPerFrame);
    mDecoderConfig->setFinalFrameBlocks(ApeDefaults::APEDefaultFinalFrameBlocks);
    mDecoderConfig->setTotalFrames(ApeDefaults::APEDefaultTotalFrames);
    mDecoderConfig->setSeekTablePresent(ApeDefaults::APEDefaultSeekTablePresent);

    mOutBufSize = mDecoderConfig->getMaxOutputBufSize();

    return QC2_OK;
}

QC2Status QC2AudioHwApeDec::onStart() {
    QLOGD_INST("%s", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwApeDec::onCodecConfig(std::shared_ptr<QC2Buffer> input) {
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

    // Set APE decoder config from input CSD
    mDecoderConfig->setApeDecConfig(inputOffset);
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

    QLOGD("%s: APE CSD values: BlocksPerFrame %d Total Frames %d Bits Per Sample %d"
                    " Channels %d samplingRate %d Version %d\n"
                    " Compression Level %d Final Frame Blocks %d seek table present %d",
              __func__, mDecoderConfig->getBlocksPerFrame(), mDecoderConfig->getTotalFrames(),
              mDecoderConfig->getBitsPerSample(), mDecoderConfig->getChannels(),
              mDecoderConfig->getSampleRate(), mDecoderConfig->getCompatibleVersion(),
              mDecoderConfig->getCompressionLevel(), mDecoderConfig->getFinalFrameBlocks(),
              mDecoderConfig->isSeekTablePresent());

    if (mDecoderConfig->getCompressionLevel() == APE_COMPRESSION_LEVEL_EXTRA_HIGH ||
            mDecoderConfig->getCompressionLevel() == APE_COMPRESSION_LEVEL_INSANE) {
        QLOGE("%s: Extra high/insane APE clips not supported by DSP", __func__);
        return QC2_CORRUPTED;
    }

    mInputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getChannels(),
                        .codec_type = Type_APE,
                        .codec_info_size = sizeof(mDecoderConfig),
                        .codec_info = (void *)(mDecoderConfig->getApeDecConfig().get())};
    mOutputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getChannels(),
                        .codec_type = Type_PCM};
    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }

    return mAudioHw->start_session(getHwSessionId());
}

void QC2AudioHwApeDec::updateConfigParams(bool *hasCSDChanged) {

    // Update sample rate
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
        *hasCSDChanged = true;
    }

    // Update bits per sample
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitsPerSample());
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

    // Update compression level
    auto compressionLevel = mDecoderConfig->getCompressionLevel();
    if (compressionLevel != mIntfParamValuesMap[kParamIndexAudioCompressionLevel]) {
        auto compressionLevelParam =
            std::make_shared<C2AudioCompressionLevelInfo::input>(0, compressionLevel);
        mUpdatedInfos.push_back(compressionLevelParam);
        mIntfParamValuesMap[kParamIndexAudioCompressionLevel] = compressionLevel;
        *hasCSDChanged = true;
    }

    // Update format flags
    auto formatFlags = mDecoderConfig->getFormatFlags();
    if (formatFlags != mIntfParamValuesMap[kParamIndexAudioFormatFlags]) {
        auto formatFlagsParam = std::make_shared<C2AudioFormatFlagsInfo::input>(0, formatFlags);
        mUpdatedInfos.push_back(formatFlagsParam);
        mIntfParamValuesMap[kParamIndexAudioFormatFlags] = formatFlags;
        *hasCSDChanged = true;
    }

    // Update blocks per frame
    auto blocksPerFrame = mDecoderConfig->getBlocksPerFrame();
    if (blocksPerFrame != mIntfParamValuesMap[kParamIndexAudioBlocksPerFrame]) {
        auto blocksPerFrameParam =
            std::make_shared<C2AudioBlocksPerFrameInfo::input>(0, blocksPerFrame);
        mUpdatedInfos.push_back(blocksPerFrameParam);
        mIntfParamValuesMap[kParamIndexAudioBlocksPerFrame] = blocksPerFrame;
        *hasCSDChanged = true;
    }

    // Update final frame blocks
    auto finalFrameBlocks = mDecoderConfig->getFinalFrameBlocks();
    if (finalFrameBlocks != mIntfParamValuesMap[kParamIndexAudioFinalFrameBlocks]) {
        auto finalFrameBlocksParam =
            std::make_shared<C2AudioFinalFrameBlocksInfo::input>(0, finalFrameBlocks);
        mUpdatedInfos.push_back(finalFrameBlocksParam);
        mIntfParamValuesMap[kParamIndexAudioFinalFrameBlocks] = finalFrameBlocks;
        *hasCSDChanged = true;
    }

    // Update total frames
    auto totalFrames = mDecoderConfig->getTotalFrames();
    if (totalFrames != mIntfParamValuesMap[kParamIndexAudioTotalFrames]) {
        auto totalFramesParam = std::make_shared<C2AudioTotalFramesInfo::input>(0, totalFrames);
        mUpdatedInfos.push_back(totalFramesParam);
        mIntfParamValuesMap[kParamIndexAudioTotalFrames] = totalFrames;
        *hasCSDChanged = true;
    }

    // Update seek table info
    auto seekTablePresent = mDecoderConfig->isSeekTablePresent();
    if (seekTablePresent != mIntfParamValuesMap[kParamIndexAudioSeekTablePresent]) {
        auto seekTablePresentParam =
            std::make_shared<C2AudioSeekTablePresentInfo::input>(0, seekTablePresent);
        mUpdatedInfos.push_back(seekTablePresentParam);
        mIntfParamValuesMap[kParamIndexAudioSeekTablePresent] = seekTablePresent;
        *hasCSDChanged = true;
    }
}

}   // namespace qc2audio

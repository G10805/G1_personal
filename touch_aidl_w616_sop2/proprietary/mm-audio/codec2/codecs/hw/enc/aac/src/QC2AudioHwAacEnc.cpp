/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2AudioHwAacEnc.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAacEnc"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC


namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwAacEnc                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAacEnc::QC2AudioHwAacEnc(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, MimeType::RAW,
              mime, KIND_ENCODER, variant, listener) {
        mEncoderConfig = std::make_shared<QC2AacEncConfig>(AAC_LC);
}

QC2Status QC2AudioHwAacEnc::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;

    QLOGV_INST("configure: core-index=%x baseIndex=%x",
    param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexSampleRate: {
            C2StreamSampleRateInfo::input in;
            if (param.index() == in.index()) {
                auto sampleRate =
                    ((C2StreamSampleRateInfo::input*)&param)->value;
                if (!mEncoderConfig->isSampleRateValid(sampleRate)) {
                    QLOGE("Invalid sample rate %d", sampleRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mEncoderConfig->setSampleRate(sampleRate);
                QLOGV("%s: sample rate %d", __func__, sampleRate);
            }
            break;
        }
        case kParamIndexChannelCount: {
            C2StreamChannelCountInfo::input chCountInfo;
            if (param.index() == chCountInfo.index()) {
                auto chCount =
                    ((C2StreamChannelCountInfo::input*)&param)->value;
                if (!mEncoderConfig->isChannelCountValid(chCount)) {
                    QLOGE("Invalid channel count %d", chCount);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mEncoderConfig->setChannelCount(chCount);
                QLOGV("%s: channel config %d", __func__, chCount);
            }
            break;
        }
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::input pcmEncodingInfo;
            if (param.index() == pcmEncodingInfo.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::input*)&param)->value;
                auto bps =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mEncoderConfig->isBitsPerSampleValid(bps)) {
                    QLOGE("Invalid bits per sample %d", bps);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bps);
                mEncoderConfig->setBitsPerSample(bps);
            }
            break;
        }
        case kParamIndexBitrate: {
            C2StreamBitrateInfo::output bitRateInfo;
            if (param.index() == bitRateInfo.index()) {
                auto bitRate =
                    ((C2StreamBitrateInfo::output*)&param)->value;
                QLOGV("%s: bit rate %d", __func__, bitRate);
                if (mEncoderConfig->setBitRate(bitRate) != QC2_OK) {
                    QLOGE("%s: Cannot set bitrate %d", __func__, bitRate);
                    retVal = QC2_BAD_VALUE;
                }
            }
            break;
        }
        case kParamIndexProfileLevel: {
            C2StreamProfileLevelInfo::output profileInfo;
            if (param.index() == profileInfo.index()) {
                auto profile =
                    ((C2StreamProfileLevelInfo::output*)&param)->profile;
                if (!mEncoderConfig->isProfileValid(profile)) {
                    QLOGE("Invalid profile %d", profile);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: aac profile %d", __func__, profile);
                mEncoderConfig->setEncodingMode(profile);
            }
            break;
        }
        case kParamIndexAacPackaging: {
            C2StreamAacFormatInfo::output formatInfo;
            if (param.index() == formatInfo.index()) {
                auto format =
                    ((C2StreamAacFormatInfo::output*)&param)->value;
                if (!mEncoderConfig->isStreamFormatValid(format)) {
                    QLOGE("Invalid format %d", (int)format);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: aac format %d", __func__, (int)format);
                mEncoderConfig->setFormatFlag(format);
            }
            break;
        }
        default: {
            QC2AudioHwCodec::configure(param);
            break;
        }
    }

    updateConfigParams();
    return retVal;
}

void QC2AudioHwAacEnc::updateConfigParams() {

    // Update sample rate on interface and output buffer
    auto sampleRate = mEncoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::input>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample on interface and output buffer
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mEncoderConfig->getBitsPerSample());
    if (pcmEncoding != mIntfParamValuesMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::input>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mIntfParamValuesMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update channels on interface and output buffer
    auto channels = mEncoderConfig->getChannelCount();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::input>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
    }

    // Update bit rate on interface and output buffer
    auto bitRate = mEncoderConfig->getBitRate();
    if (bitRate != mIntfParamValuesMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::output>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mIntfParamValuesMap[kParamIndexBitrate] = bitRate;
    }

    uint16_t encMode = mEncoderConfig->getEncodingMode();
    auto profile = AacEncodingModeToC2Profile.at(encMode);
    if (profile != mIntfParamValuesMap[kParamIndexProfileLevel]) {
        auto profileParam = std::make_shared<C2StreamProfileLevelInfo::output>(
                0u, static_cast<C2Config::profile_t>(profile),
                static_cast<C2Config::level_t>(C2Config::level_t::LEVEL_UNUSED));
        mUpdatedInfos.push_back(profileParam);
        mIntfParamValuesMap[kParamIndexProfileLevel] = profile;
    }

    uint16_t formatFlag = mEncoderConfig->getFormatFlag();
    auto streamFormat = AacFormatFlagToC2PackagingType.at(formatFlag);
    if (streamFormat != mIntfParamValuesMap[kParamIndexAacPackaging]) {
        auto streamFormatParam = std::make_shared<C2StreamAacFormatInfo::output>(0, streamFormat);
        mUpdatedInfos.push_back(streamFormatParam);
        mIntfParamValuesMap[kParamIndexAacPackaging] = streamFormat;
    }
}

QC2Status QC2AudioHwAacEnc::onInit() {
    QLOGD_INST("%s", __func__);

    mEncoderConfig->setEncodingMode(AacDefaults::AacDefaultProfile);
    mEncoderConfig->setSampleRate(AacDefaults::AacDefaultSampleRate);
    mEncoderConfig->setChannelCount(AacDefaults::AacDefaultNumChannels);
    mEncoderConfig->setBitsPerSample(AacDefaults::AacDefaultBitsPerSample);
    mEncoderConfig->setBitRate(AacDefaults::AacDefaultBitRate);
    mEncoderConfig->setFormatFlag(AacDefaults::AacDefaultPackagingType);
    //set output buffer size
    mOutBufSize = mEncoderConfig->getMaxOutputBufSize();
    return QC2_OK;
}

QC2Status QC2AudioHwAacEnc::onStart() {

    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    if (!mEncoderConfig) {
        QLOGE("%s: encoder config not initialized", __func__);
        return QC2_ERROR;
    }
    if (!isHwAacEncodeSupported()) {
        return QC2_CORRUPTED;
    }
    mMp4ffHeaderSent = false;

    mInputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitsPerSample(),
                        .channel_count = mEncoderConfig->getChannelCount(),
                        .codec_type = Type_PCM};
    mOutputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitsPerSample(),
                        .channel_count = mEncoderConfig->getChannelCount(),
                        .codec_type = Type_AAC,
                        .codec_info_size = sizeof(QC2AacEncConfig::AacEncConfig),
                        .codec_info = mEncoderConfig->getAacEncConfig().get()};

    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }
    return mAudioHw->start_session(getHwSessionId());
}

QC2Status QC2AudioHwAacEnc::onFlush() {
    mMp4ffHeaderSent = false;
    return QC2_OK;
}

void QC2AudioHwAacEnc::onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) {
    if (!mMp4ffHeaderSent) {
        uint8_t aacHeaderMp4ff[AUDAAC_MAX_MP4FF_HEADER_LENGTH];
        if (sampleRateToSampleIdx.find(mEncoderConfig->getSampleRate()) ==
                sampleRateToSampleIdx.end()) {
            QLOGW("%s: Cannot add raw header for sample rate %u",
                    __func__, mEncoderConfig->getSampleRate());
            mMp4ffHeaderSent = true;
            QC2AudioHwCodec::onOutputsDone(outBuf);
            return;
        }
        auto sampleIdx = sampleRateToSampleIdx.at(mEncoderConfig->getSampleRate());
        auto channelCount = mEncoderConfig->getChannelCount();
        addMp4ffHeaderVar(aacHeaderMp4ff, sampleIdx, (uint8_t)channelCount);
        size_t csdSize = AUDAAC_MAX_MP4FF_HEADER_LENGTH;
        std::shared_ptr<C2StreamInitDataInfo::output> csd =
            C2StreamInitDataInfo::output::AllocUnique(csdSize, 0u);
        if (!csd) {
            QLOGW("%s: CSD allocation failed", __func__);
            QC2AudioHwCodec::onOutputsDone(outBuf);
            return;
        }
        memcpy_s(csd->m.value, AUDAAC_MAX_MP4FF_HEADER_LENGTH,
                 &aacHeaderMp4ff[0], AUDAAC_MAX_MP4FF_HEADER_LENGTH);
        mUpdatedInfos.push_back(csd);
        mMp4ffHeaderSent = true;
    }
    QC2AudioHwCodec::onOutputsDone(outBuf);
}

void QC2AudioHwAacEnc::aacRecInstallBits(uint8_t *input,
                              uint8_t numBitsReqd,
                              uint32_t  value,
                              uint16_t *hdrBitIndex) {
    uint8_t numRemaining = numBitsReqd;
    uint8_t bitMask = 0xFF;
    while (numRemaining) {
        uint32_t byteIndex = (*hdrBitIndex) >> 3;
        uint8_t bitIndex  = (*hdrBitIndex) &  0x07;
        uint8_t bitsAvailInByte = (uint8_t)(8 - bitIndex);
        uint8_t numToCopy = std::min(bitsAvailInByte, numRemaining);

        uint8_t byteToCopy =
            (uint8_t)((uint8_t)((value >> (numRemaining - numToCopy)) & 0xFF) <<
                    (bitsAvailInByte - numToCopy));

        input[byteIndex] &= ((uint8_t)(bitMask << bitsAvailInByte));
        input[byteIndex] |= byteToCopy;

        *hdrBitIndex = (uint16_t)(*hdrBitIndex + numToCopy);

        numRemaining = (uint8_t)(numRemaining - numToCopy);
    }
}

void QC2AudioHwAacEnc::addMp4ffHeaderVar(uint8_t *aacHeaderMp4ff,
                                      uint32_t sampleIndex,
                                      uint8_t channelConfig) {
    aacHeaderMp4ff[0] = 0;
    aacHeaderMp4ff[1] = 0;
    uint16_t aacHdrBitIndex = 0;

    // Audio object type, 5 bit
    aacRecInstallBits(aacHeaderMp4ff,
                      AUDAAC_MP4FF_OBJ_TYPE,
                      2, &aacHdrBitIndex);

    // Frequency index, 4 bit
    aacRecInstallBits(aacHeaderMp4ff,
                      AUDAAC_MP4FF_FREQ_IDX,
                      sampleIndex, &aacHdrBitIndex);

    // Channel config filed, 4 bit
    aacRecInstallBits(aacHeaderMp4ff,
                      AUDAAC_MP4FF_CH_CONFIG,
                      channelConfig, &aacHdrBitIndex);
}

bool QC2AudioHwAacEnc::isHwAacEncodeSupported() {
    // Check whether profile is supported for HW encode
    uint32_t profile = AacEncodingModeToC2Profile.at(mEncoderConfig->getEncodingMode());
    if (std::find(sSupportedHwAacEncoderProfiles.begin(), sSupportedHwAacEncoderProfiles.end(),
            profile) == sSupportedHwAacEncoderProfiles.end()) {
        QLOGE("%s: Profile %d is not supported by encoder", __func__, profile);
        return false;
    }
    // Return error for non-standard sample rates
    if (std::find(std::begin(AacEncSampleRateSet),
            std::end(AacEncSampleRateSet), mEncoderConfig->getSampleRate()) ==
                std::end(AacEncSampleRateSet)) {
        QLOGE("%s: Non-standard sample rate %u not supported", __func__,
               mEncoderConfig->getSampleRate());
        return false;
    }
    return true;
}

}   // namespace qc2audio

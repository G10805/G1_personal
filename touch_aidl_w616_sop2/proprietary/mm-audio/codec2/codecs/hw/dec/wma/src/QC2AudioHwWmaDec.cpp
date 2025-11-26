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
#include "QC2AudioHwWmaDec.h"
#include "QC2AudioHwUtils.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwWmaDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwWmaDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwWmaDec::QC2AudioHwWmaDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, mime,
                              MimeType::RAW, KIND_DECODER, variant, listener) {
        mOutBufSize = 0;
        mDecoderConfig = std::make_shared<QC2WmaDecConfig>(WMA_DEFAULT);
}

QC2Status QC2AudioHwWmaDec::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;
    QLOGV("Configure %s, core-index=%x baseIndex=%x",
               DebugString::C2Param(param.index()).c_str(),
               param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexSampleRate: {
            C2StreamSampleRateInfo::output sampleRateInfo;
            if (param.index() == sampleRateInfo.index()) {
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
            C2StreamChannelCountInfo::output channelCountInfo;
            if (param.index() == channelCountInfo.index()) {
                auto channels =
                    ((C2StreamChannelCountInfo::output*)&param)->value;
                if (!mDecoderConfig->isChannelCountValid(channels)) {
                    QLOGE("Invalid channel count %d", channels);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: channels %d", __func__, channels);
                mDecoderConfig->setChannelCount(channels);
            }
            break;
        }
        case kParamIndexBitrate: {
            C2StreamBitrateInfo::input bitRateInfo;
            if (param.index() == bitRateInfo.index()) {
                auto bitrate = ((C2StreamBitrateInfo::input*)&param)->value;
                QLOGV("%s: avg bitrate %d", __func__, bitrate);
                mDecoderConfig->setBitRate(bitrate);
            }
            break;
        }
        case kParamIndexAudioWmaConfig: {
            C2AudioWmaConfig::input wmaCfg;
            if (param.index() == wmaCfg.index()) {
                auto cfg = reinterpret_cast<const C2AudioWmaConfig::input *>(&param);
                setWmaDecConfig(cfg);
            }
            break;
        }
        case kParamIndexAudioWmaVersion: {
            C2AudioWmaVersionInfo::input wmaVersion;
            if (param.index() == wmaVersion.index()) {
                auto version = ((C2AudioWmaVersionInfo::input*)&param)->value;
                if (!mDecoderConfig->isWmaVersionValid(version)) {
                    QLOGE("Invalid wma version %d", version);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: wma version %d", __func__, version);
                mDecoderConfig->setWmaVersion(version);
                // Set output buffer size based on version
                mOutBufSize = mDecoderConfig->getMaxOutputBufSize();
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

QC2Status QC2AudioHwWmaDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not created, return error", __func__);
        return QC2_ERROR;
    }

    // Initialize decoder config with defaults
    mDecoderConfig->setChannelCount(WmaStdDefaults::kWmaStdDefaultChannelCount);
    mDecoderConfig->setBitsPerSample(WmaStdDefaults::kWmaStdDefaultBitsPerSample);
    mDecoderConfig->setBitRate(WmaStdDefaults::kWmaStdDefaultAvgBytesPerSecond * 8);
    mDecoderConfig->setSampleRate(WmaStdDefaults::kWmaStdDefaultSampleRate);
    mDecoderConfig->setFmtTag(WmaStdDefaults::kWmaStdDefaultFmtTag);
    mDecoderConfig->setBlkAlign(WmaStdDefaults::kWmaStdDefaultBlkAlign);
    mDecoderConfig->setChannelMask(WmaStdDefaults::kWmaStdDefaultChannelMask);
    mDecoderConfig->setEncOptions(WmaStdDefaults::kWmaStdDefaultEncOptions);
    mDecoderConfig->setReserved(WmaStdDefaults::kWmaStdDefaultReserved);
    mDecoderConfig->setWmaVersion(WMA_DEFAULT);
    mDecoderConfig->setAdvancedEncOption(WmaProDefaults::kWmaProDefaultAdvancedEncOption);
    mDecoderConfig->setAdvancedEncOptions2(WmaProDefaults::kWmaProDefaultAdvancedEncOptions2);

    mOutBufSize = mDecoderConfig->getMaxOutputBufSize();
    return QC2_OK;
}

QC2Status QC2AudioHwWmaDec::onStart() {
    QC2Status retVal = QC2_OK;

    QLOGD_INST("%s", __func__);
    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not created, return error", __func__);
        return QC2_ERROR;
    }

    uint32_t version = mDecoderConfig->getWmaVersion();
    auto getHwCodecType = [&]() {
        if (version ==  WMA_STD) {
            return Type_WMA_STD;
        } else if (version == WMA_PRO) {
            return Type_WMA_PRO;
        } else if (version == WMA_LOSSLESS) {
            return Type_WMA_LOSSLESS;
        }
        QLOGW("%s: Unrecognized WMA version %u, using default",
               __func__, version);
        return Type_WMA_STD;
    };
    auto getHwCodecInfoSize = [&]() { 
        if (version == WMA_PRO || version == WMA_LOSSLESS) {
            return sizeof(QC2AudioWmaProDecConfig);
        }
        return sizeof(QC2AudioWmaStdDecConfig);
    };

    mInputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                      .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                      .channel_count = mDecoderConfig->getChannelCount(),
                      .codec_type = getHwCodecType(),
                      .codec_info_size = static_cast<uint32_t>(getHwCodecInfoSize()),
                      .codec_info = nullptr};

    QC2AudioWmaStdDecConfig wmaStdConfig;
    QC2AudioWmaProDecConfig wmaProConfig;
    if (version ==  WMA_STD) {
        QLOGD("%s: WMA STD CSD values: Channel count %d Bits Per Sample %d"
                " Bit rate %d samplingRate %d Format tag %d Block align %d\n"
                " Channel Mask %d Encoding options %d Reserved %d",
            __func__, mDecoderConfig->getChannelCount(), mDecoderConfig->getBitsPerSample(),
            mDecoderConfig->getBitRate(), mDecoderConfig->getSampleRate(),
            mDecoderConfig->getFmtTag(), mDecoderConfig->getBlkAlign(),
            mDecoderConfig->getChannelMask(), mDecoderConfig->getEncOptions(),
            mDecoderConfig->getReserved());

        mDecoderConfig->getWmaStdDecConfig(&wmaStdConfig);
        mInputMediaFmt.codec_info = (void *)(&wmaStdConfig);
    } else if (version == WMA_PRO || version == WMA_LOSSLESS) {
        QLOGD("%s: WMA PRO CSD values: Channel count %d, Bits Per Sample %d,"
                " Bit rate %d, samplingRate %d, Format tag %d\n"
                " Block align %d, Channel Mask %d, Encoding options %d,"
                " Advanced encoding opt %d, Advanced encoding opt2 %d",
            __func__, mDecoderConfig->getChannelCount(), mDecoderConfig->getBitsPerSample(),
            mDecoderConfig->getBitRate(), mDecoderConfig->getSampleRate(),
            mDecoderConfig->getFmtTag(), mDecoderConfig->getBlkAlign(),
            mDecoderConfig->getChannelMask(), mDecoderConfig->getEncOptions(),
            mDecoderConfig->getAdvancedEncOption(), mDecoderConfig->getAdvancedEncOptions2());

        mDecoderConfig->getWmaProDecConfig(&wmaProConfig);
        mInputMediaFmt.codec_info = (void *)(&wmaProConfig);

        // Set output buffer size for WMA PRO/LOSSLESS
        mOutBufSize = mDecoderConfig->getSampleRate() * mDecoderConfig->getBitsPerSample()/8 *
                        mDecoderConfig->getChannelCount() * WMAPRO_MAX_DURATION_PER_PACKET;
        mLinearPool->setBufferSize(mOutBufSize);
        QLOGV("%s: Output Buffer size=%u", __func__, mOutBufSize);
    }

    // WMA decoder calculates timestamp based on the number of bytes decoded.
    // Force flush on configured WMA decoder, even if no data is queued, so that
    // it can snap to the input timestamp after seek
    mForceFlush = true;

    mOutputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                       .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                       .channel_count = mDecoderConfig->getChannelCount(),
                       .codec_type = Type_PCM};

    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("%s: Open session failed for codec type %d  with error code %d",
               __func__, mInputMediaFmt.codec_type, retVal);
        return retVal;
    }
    return mAudioHw->start_session(getHwSessionId());
}

void QC2AudioHwWmaDec::updateConfigParams() {

    // Update sample rate
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitsPerSample());
    if (pcmEncoding != mIntfParamValuesMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mIntfParamValuesMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update channels
    auto channels = mDecoderConfig->getChannelCount();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
    }

    // Update bit rate
    auto bitRate = mDecoderConfig->getBitRate();
    if (bitRate != mIntfParamValuesMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::input>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mIntfParamValuesMap[kParamIndexBitrate] = bitRate;
    }
}

void QC2AudioHwWmaDec::setWmaDecConfig(const C2AudioWmaConfig::input *cfg) {
    auto bitsPerSample = cfg->bitsPerSample;
    QLOGV("%s: bitsPerSample %d", __func__, bitsPerSample);
    mDecoderConfig->setBitsPerSample(bitsPerSample);

    auto blockAlign = cfg->blkAlign;
    QLOGV("%s: block align %d", __func__, blockAlign);
    mDecoderConfig->setBlkAlign(blockAlign);

    auto channelMask = cfg->channelMask;
    QLOGV("%s: channel mask 0x%x", __func__, channelMask);
    mDecoderConfig->setChannelMask(channelMask);

    auto formatTag = cfg->formatTag;
    QLOGV("%s: format tag %d", __func__, formatTag);
    mDecoderConfig->setFmtTag(formatTag);

    auto encOptions = cfg->encOptions;
    QLOGV("%s: encoding options %d", __func__, encOptions);
    mDecoderConfig->setEncOptions(encOptions);

    auto advEncodingOption = cfg->advEncOption;
    QLOGV("%s: advanced encoding option %d", __func__, advEncodingOption);
    mDecoderConfig->setAdvancedEncOption(advEncodingOption);

    auto advEncodingOptions2 = cfg->advEncOptions2;
    QLOGV("%s: advanced encoding options2 %d",__func__, advEncodingOptions2);
    mDecoderConfig->setAdvancedEncOptions2(advEncodingOptions2);
}

}   // namespace qc2audio

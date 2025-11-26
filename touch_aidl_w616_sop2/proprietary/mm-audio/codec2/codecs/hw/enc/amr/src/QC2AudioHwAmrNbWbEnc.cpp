/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2AudioHwAmrNbWbEnc.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAmrNbWbEnc"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

#define RETURN_IF_ERROR(retVal, str) \
do {\
    if (retVal) { \
        QLOGE_INST(str);\
        return retVal;\
     } \
} while (0); \

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwAmrNbWbEnc                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAmrNbWbEnc::QC2AudioHwAmrNbWbEnc(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, MimeType::RAW,
                              mime, KIND_ENCODER, variant, listener) {
        if (!(strcasecmp(mime.c_str(), MimeType::AMR_WB))) {
            setEncoderConfig(std::make_shared<QC2AmrWbCodecConfig>());
            setAmrCodecType(AmrVersion::AMRWB);
        } else {
            // DEFAULT to AMR-NB encoder
            setEncoderConfig(std::make_shared<QC2AmrNbCodecConfig>());
            setAmrCodecType(AmrVersion::AMRNB);
        }
}

QC2Status QC2AudioHwAmrNbWbEnc::configure(const C2Param& param) {
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
            C2StreamChannelCountInfo::input in;
            if (param.index() == in.index()) {
                auto chCount =
                    ((C2StreamChannelCountInfo::input*)&param)->value;
                if (!mEncoderConfig->isChannelConfigValid(chCount)) {
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
            C2StreamPcmEncodingInfo::input in;
            if (param.index() == in.index()) {
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
            C2StreamBitrateInfo::output out;
            if (param.index() == out.index()) {
                auto bitRate =
                    ((C2StreamBitrateInfo::output*)&param)->value;
                if (!mEncoderConfig->isBitRateValid(bitRate)) {
                    QLOGE("Invalid bit rate %d", bitRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bit rate %d", __func__, bitRate);
                mEncoderConfig->setBitRate(bitRate);
            }
            break;
        }
        default: {
           // QLOGW_INST("unknown/unsupported param %s index = 0x%X",
           //     DebugString::C2Param(param.index()).c_str(), param.index());
            QC2AudioHwCodec::configure(param);
            break;
        }
    }
    updateConfigParams();
    return retVal;
}

QC2Status QC2AudioHwAmrNbWbEnc::updateConfigParams() {

    // Update sample rate on interface and output buffer
    auto sampleRate = mEncoderConfig->getSampleRate();
    if(sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
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
    if(channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::input>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
    }

    // Update bit rate on interface and output buffer
    auto bitRate = mEncoderConfig->getBitRate();
    if(channels != mIntfParamValuesMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::output>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mIntfParamValuesMap[kParamIndexBitrate] = bitRate;
    }

    return QC2_OK;
}

QC2Status QC2AudioHwAmrNbWbEnc::onStart() {

    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    QC2AudioAmrConfig codecConfig;
    auto outputCodecType = [&]()
    {
        return (getAmrCodecType() == AmrVersion::AMRNB)? Type_AMR_NB: Type_AMR_WB;
    };

    updateConfigParams();
    codecConfig = {.bitrate = static_cast<uint16_t>(mEncoderConfig->getBitRate()),
                    .dtx_mode = 0};

    mInputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitsPerSample(),
                        .channel_count = mEncoderConfig->getChannelCount(),
                        .codec_type = Type_PCM};
    mOutputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitsPerSample(),
                        .channel_count = mEncoderConfig->getChannelCount(),
                        .codec_type = outputCodecType(),
                        .codec_info_size = sizeof(QC2AudioAmrConfig),
                        .codec_info = &codecConfig };

    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }
    return mAudioHw->start_session(getHwSessionId());
}

QC2Status QC2AudioHwAmrNbWbEnc::onInit() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    //set output buffer size
    mOutBufSize = std::get<AMR_MAX_ENC_OUTPUT_BUF_SIZE>(AmrMinMaxList[mAmrVersion]);

    if (getAmrCodecType() == AmrVersion::AMRNB) {
        mEncoderConfig->setSampleRate(AmrDefaults::AmrNbDefaultSampleRate);
        mEncoderConfig->setChannelCount(AmrDefaults::AmrNbDefaultNumChannels);
        mEncoderConfig->setBitsPerSample(AmrDefaults::AmrNbDefaultBitsPerSample);
        mEncoderConfig->setBitRate(AmrDefaults::AmrNbDefaultBitRate);
    } else if (getAmrCodecType() == AmrVersion::AMRWB) {
        mEncoderConfig->setSampleRate(AmrDefaults::AmrWbDefaultSampleRate);
        mEncoderConfig->setChannelCount(AmrDefaults::AmrWbDefaultNumChannels);
        mEncoderConfig->setBitsPerSample(AmrDefaults::AmrWbDefaultBitsPerSample);
        mEncoderConfig->setBitRate(AmrDefaults::AmrWbDefaultBitRate);
    }

    return retVal;
}

////////////////////////////// Helper Functions ///////////////////////////////
void QC2AudioHwAmrNbWbEnc::logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGD_INST("%s %s", prefix, QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

}   // namespace qc2audio

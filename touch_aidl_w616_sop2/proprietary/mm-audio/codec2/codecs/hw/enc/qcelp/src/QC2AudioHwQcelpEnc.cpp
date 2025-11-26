/*
 **************************************************************************************************
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2AudioHwQcelpEnc.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwQcelpEnc"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwQcelpEnc                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwQcelpEnc::QC2AudioHwQcelpEnc(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, MimeType::RAW,
                              mime, KIND_ENCODER, variant, listener) {
    mEncoderConfig = std::make_shared<QC2QcelpEncConfig>();
}

QC2Status QC2AudioHwQcelpEnc::configure(const C2Param& param) {
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
                if (!mEncoderConfig->isChannelCountValid(chCount)) {
                    QLOGE("Invalid channel count %d", chCount);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mEncoderConfig->setChannels(chCount);
                QLOGV("%s: channel config %d", __func__, chCount);
            }
            break;
        }
        case kParamIndexAudioMinVocoderRate: {
            C2AudioMinVocoderRateInfo::input in;
            if (param.index() == in.index()) {
                auto minRate =
                    ((C2AudioMinVocoderRateInfo::input*)&param)->value;
                if (!mEncoderConfig->isVocoderRateValid(minRate)) {
                    QLOGE("Invalid min rate %d", minRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mEncoderConfig->setMinRate(minRate);
                QLOGV("%s: min rate %d", __func__, minRate);
            }
            break;
        }
        case kParamIndexAudioMaxVocoderRate: {
            C2AudioMaxVocoderRateInfo::input in;
            if (param.index() == in.index()) {
                auto maxRate =
                    ((C2AudioMaxVocoderRateInfo::input*)&param)->value;
                if (!mEncoderConfig->isVocoderRateValid(maxRate)) {
                    QLOGE("Invalid max rate %d", maxRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mEncoderConfig->setMaxRate(maxRate);
                QLOGV("%s: max rate %d", __func__, maxRate);
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

QC2Status QC2AudioHwQcelpEnc::updateConfigParams() {

    // Update sample rate on interface and output buffer
    auto sampleRate = mEncoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::input>(0, sampleRate);
            mUpdatedInfos.push_back(sampleRateParam);
            mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update channels on interface and output buffer
    auto channels = mEncoderConfig->getChannels();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::input>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
    }

    // Update min rate on interface and output buffer
    auto minRate = mEncoderConfig->getMinRate();
    if (minRate != mIntfParamValuesMap[kParamIndexAudioMinVocoderRate]) {
        auto minRateParam = std::make_shared<C2AudioMinVocoderRateInfo::input>(0, minRate);
        mUpdatedInfos.push_back(minRateParam);
        mIntfParamValuesMap[kParamIndexAudioMinVocoderRate] = minRate;
    }

    // Update max rate on interface and output buffer
    auto maxRate = mEncoderConfig->getMaxRate();
    if (maxRate != mIntfParamValuesMap[kParamIndexAudioMaxVocoderRate]) {
        auto maxRateParam = std::make_shared<C2AudioMaxVocoderRateInfo::input>(0, maxRate);
        mUpdatedInfos.push_back(maxRateParam);
        mIntfParamValuesMap[kParamIndexAudioMaxVocoderRate] = maxRate;
    }

    return QC2_OK;
}

QC2Status QC2AudioHwQcelpEnc::onStart() {

    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    if (!mEncoderConfig) {
        QLOGE("%s: encoder config not initialized", __func__);
        return QC2_ERROR;
    }

    QC2QcelpEncConfig::QcelpEncConfig encConfig = {
                        .minRate = mEncoderConfig->getMinRate(),
                        .maxRate = mEncoderConfig->getMaxRate(),
                        .rateModCmd = mEncoderConfig->getRateModulation(),
                        .reducedRateCmd = mEncoderConfig->getReducedRateCmd()};

    mInputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitDepth(),
                        .channel_count = mEncoderConfig->getChannels(),
                        .codec_type = Type_PCM};
    mOutputMediaFmt = {.sample_rate = mEncoderConfig->getSampleRate(),
                        .bits_per_sample = mEncoderConfig->getBitDepth(),
                        .channel_count = mEncoderConfig->getChannels(),
                        .codec_type = Type_QCELP,
                        .codec_info_size = sizeof(QC2QcelpEncConfig::QcelpEncConfig),
                        .codec_info = &encConfig };

    QLOGV("%s: Qcelp enc config: sample_rate %u, channel_count %u, minRate %u, "
          "maxRate %u, rateModCmd %u", __func__, mEncoderConfig->getSampleRate(),
          mEncoderConfig->getChannels(), mEncoderConfig->getMinRate(),
          mEncoderConfig->getMaxRate(), mEncoderConfig->getRateModulation());

    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("%s: Open session failed for codec type %d  with error code %d",
               __func__, mInputMediaFmt.codec_type, retVal);
        return retVal;
    }
    return mAudioHw->start_session(getHwSessionId());
}

QC2Status QC2AudioHwQcelpEnc::onInit() {
    QLOGD_INST("%s", __func__);

    //set output buffer size
    mOutBufSize = QC2QcelpEncConfig::getMaxOutputBufferSize();

    if (!mEncoderConfig) {
        QLOGE("%s: encoder config not initialized", __func__);
        return QC2_ERROR;
    }

    // Set defaults
    mEncoderConfig->setChannels(SpeechDefaults::SpeechDefaultChannels);
    mEncoderConfig->setBitDepth(SpeechDefaults::SpeechDefaultBitDepth);
    mEncoderConfig->setSampleRate(SpeechDefaults::SpeechDefaultSampleRate);
    mEncoderConfig->setMinRate(QcelpDefaults::QcelpDefaultMinRate);
    mEncoderConfig->setMaxRate(QcelpDefaults::QcelpDefaultMaxRate);
    mEncoderConfig->setRateModulation(QcelpDefaults::QcelpDefaultRateModCmd);
    mEncoderConfig->setReducedRateCmd(QcelpDefaults::QcelpDefaultReducedRateCmd);

    return QC2_OK;
}

}   // namespace qc2audio

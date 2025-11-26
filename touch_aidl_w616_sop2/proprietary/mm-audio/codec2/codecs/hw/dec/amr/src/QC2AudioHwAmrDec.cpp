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
#include "QC2AudioHwAmrDec.h"
#include "QC2AudioHwUtils.h"
#include "QC2Constants.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAmrDec"
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
//                              QC2AudioHwAmrDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAmrDec::QC2AudioHwAmrDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, mime,
                              MimeType::RAW, KIND_DECODER, variant, listener) {
}

QC2Status QC2AudioHwAmrDec::configure(const C2Param& param) {
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
                mDecoderConfig->setSampleRate(sampleRate);
                QLOGV("%s: sample rate %d", __func__, sampleRate);
            }
            break;
        }
        case kParamIndexChannelCount: {
            C2StreamChannelCountInfo::output out;
            if (param.index() == out.index()) {
                auto chCount =
                    ((C2StreamChannelCountInfo::output*)&param)->value;
                if (!mDecoderConfig->isChannelConfigValid(chCount)) {
                    QLOGE("Invalid channel count %d", chCount);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                mDecoderConfig->setChannelCount(chCount);
                QLOGV("%s: channel config %d", __func__, chCount);
            }
            break;
        }
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::output out;
            if (param.index() == out.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::output*)&param)->value;
                auto bps =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mDecoderConfig->isBitsPerSampleValid(bps)) {
                    QLOGE("Invalid bits per sample %d", bps);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bps);
                mDecoderConfig->setBitsPerSample(bps);
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

QC2Status QC2AudioHwAmrDec::updateConfigParams() {

    //std::list<std::shared_ptr<C2Param>> paramUpdatesOnInterface;

    // Update sample rate on interface and output buffer
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample on interface and output buffer
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitsPerSample());
    if (pcmEncoding != mIntfParamValuesMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mIntfParamValuesMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update channels on interface and output buffer
    auto channels = mDecoderConfig->getChannelCount();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        //paramUpdatesOnInterface.push_back(channelsParam);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
    }
    return QC2_OK;
}


QC2Status QC2AudioHwAmrDec::onInit() {
    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);

    //set output buffer size
    mOutBufSize = std::get<AMR_MAX_DEC_OUTPUT_BUF_SIZE>(AmrMinMaxList[getAmrCodecType()]);
    if ( AmrVersion::AMRWBPlus == getAmrCodecType() ) {
        QLOGI("%s: Setting defaults for AMR WB Plus format", __func__);
        mDecoderConfig->setSampleRate(AmrDefaults::AmrWbPlusDefaultSampleRate);
        mDecoderConfig->setChannelCount(AmrDefaults::AmrWbPlusDefaultNumChannels);
        mDecoderConfig->setBitsPerSample(AmrDefaults::AmrWbPlusDefaultBitsPerSample);
    }
    return retVal;
}

////////////////////////////// Helper Functions ///////////////////////////////
void QC2AudioHwAmrDec::logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix) {
    if (!buf || !prefix) {
        return;
    }
    char str[128];
    QLOGD_INST("%s %s", prefix, QC2Buffer::AsString(buf, str, sizeof(str) - 1));
}

}   // namespace qc2audio

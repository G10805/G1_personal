/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <algorithm>
#include <inttypes.h>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioSwSpeechCommonDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwSpeechCommonDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwSpeechCommonDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwSpeechCommonDec::QC2AudioSwSpeechCommonDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
            : QC2AudioSwCodec(codecName, sessionId, instanceName, mime,
                    MimeType::RAW, KIND_DECODER, variant, listener) {
        mDecStructInst = nullptr;
        mLibHandle = nullptr;
        mTransBuf = nullptr;
        mResidualBuf = nullptr;
        mMinOutBufSize = 0;
        mDecoderConfig = std::make_shared<QC2SpeechCodecConfig>();
}

QC2Status QC2AudioSwSpeechCommonDec::transcodeData(uint8_t *srcOffset, uint32_t srcDataLen,
                                           uint32_t *srcDataConsumed, uint8_t *rate,
                                           const struct frameInformation *g_frmInfo) {

    if (!mTransBufDataOffset || !mResidualBufDataOffset) {
        QLOGE("%s: Invalid data offset for internal buffers", __func__);
        return QC2_ERROR;
    }
    if (srcOffset == nullptr) {
        QLOGV("%s: srcOffset is NULL", __func__);
        return QC2_ERROR;
    }

    *rate = !mResidualDataLen ? *srcOffset : *mResidualBufDataOffset;
    QLOGD("%s: rate is %d", __func__, *rate);

    if (*rate > 4) {
        QLOGE("%s: Error: Wrong rate", __func__);
        return QC2_BAD_VALUE;
    }
    uint8_t index = *rate;

    if (!mResidualDataLen) { // No residual data
        if ((srcDataLen - *srcDataConsumed) >= g_frmInfo[index].frameLen) {
            *(mTransBufDataOffset++) = 0;
            memcpy(mTransBufDataOffset, srcOffset, g_frmInfo[index].frameLen);
            srcOffset += g_frmInfo[index].frameLen;
            *srcDataConsumed += g_frmInfo[index].frameLen;
            mTransBufDataOffset = mTransBufDataOffset +
                                      QC2SpeechCodecConfig::getMaxFrameLength(getCodecType());
        } else {
            memcpy(mResidualBufDataOffset, srcOffset, (srcDataLen - *srcDataConsumed));
            mResidualDataLen += srcDataLen - *srcDataConsumed;
            *srcDataConsumed = srcDataLen;
            mIsFullFrame = false;
        }
    } else {
        *(mTransBufDataOffset++) = 0;
        memcpy(mTransBufDataOffset, mResidualBufDataOffset, mResidualDataLen);
        mTransBufDataOffset += mResidualDataLen;
        if (srcDataLen >= (g_frmInfo[index].frameLen - mResidualDataLen)) {
            memcpy(mTransBufDataOffset, srcOffset,
                (g_frmInfo[index].frameLen - mResidualDataLen));
            srcOffset += g_frmInfo[index].frameLen - mResidualDataLen;
            mTransBufDataOffset +=
                (QC2SpeechCodecConfig::getMaxFrameLength(getCodecType()) - mResidualDataLen) ;
            *srcDataConsumed +=
                (g_frmInfo[index].frameLen - mResidualDataLen);
            mResidualDataLen = 0;
            mResidualBufDataOffset = mResidualBufStartOffset;
            memset(mResidualBufDataOffset, 0,
                    sizeof(uint8_t) * QC2SpeechCodecConfig::getResidualBufSize(getCodecType()));
        }
        else {
            memcpy(mResidualBufDataOffset, srcOffset, srcDataLen);
            srcOffset += srcDataLen;
            *srcDataConsumed = srcDataLen;
            mResidualDataLen += srcDataLen;
            mIsFullFrame = false;
            QLOGE("%s: Insufficient data", __func__);
            return QC2_ERROR;
        }
    }
    return QC2_OK;
}

uint8_t * QC2AudioSwSpeechCommonDec::allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
                                            std::unique_ptr<QC2Buffer::Mapping>& bufMapping,
                                            uint32_t bufSize) {
    mScratchPool->setBufferSize(bufSize);
    QC2Status status = mScratchPool->allocate(buf);
    if (status != QC2_OK || !buf) {
        QLOGE("%s: Could not allocate memory", __func__);
        return nullptr;
    }

    bufMapping = (*buf)->linear().map();
    if (bufMapping == nullptr) {
        return nullptr;
    }

    auto basePtr = bufMapping->baseRW();
    if (basePtr == nullptr) {
        QLOGE("%s: Base ptr is null", __func__);
        return nullptr;
    }
    return basePtr + (*buf)->linear().offset();
}

void QC2AudioSwSpeechCommonDec::byteSwap(uint8_t *startAddr, uint8_t num) {
    uint8_t tmp;
    for (uint8_t index = 0; index < num;) {
        tmp = *(startAddr + index);
        *(startAddr + index) = *(startAddr + index + 1);
        *(startAddr + index + 1) = tmp;
        index = index + 2;
    }
}

QC2Status QC2AudioSwSpeechCommonDec::configure(const C2Param& param) {
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
                    mDecoderConfig->mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!bitsPerSample) {
                    QLOGE("Invalid pcm encoding format %d", pcmEncoding);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bitsPerSample);
                mDecoderConfig->setBitDepth(bitsPerSample);
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

QC2Status QC2AudioSwSpeechCommonDec::configure(std::vector<std::unique_ptr<C2Param>> config) {
    QC2Status retVal = QC2_OK;
    if (config.empty()) {
        QLOGE("%s: target configuration is empty", __func__);
        return QC2_ERROR;
    }

    for (auto& param : config) {
        if (configure(*param) != QC2_OK) {
            retVal = QC2_ERROR;
        }
    }

    return retVal;
}

QC2Status QC2AudioSwSpeechCommonDec::onStop() {
    QLOGV("%s", __func__);

    dlclose(mLibHandle);
    mParamIntfMap.clear();
    mTransBufMapping.reset();
    mResidualBufMapping.reset();
    return QC2_OK;
}

QC2Status QC2AudioSwSpeechCommonDec::onInit() {
    QLOGD_INST("%s", __func__);

    mTransBufMapping = nullptr;
    mTransBufStartOffset = nullptr;
    mTransBufDataOffset = nullptr;
    mResidualBufMapping = nullptr;
    mResidualBufStartOffset = nullptr;
    mResidualBufDataOffset = nullptr;
    mResidualDataLen = 0;
    mIsFullFrame = false;
    mFrameCount = 0;

    return QC2_OK;
}

QC2Status QC2AudioSwSpeechCommonDec::onStart() {
    QLOGD_INST("%s", __func__);

    mMinOutBufSize = QC2SpeechCodecConfig::getMaxOutputBufferSize();

    mScratchPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (mScratchPool == nullptr) {
        QLOGE("Failed to create scratch buffer pool !");
        return QC2_ERROR;
    }
    mScratchPool->setUsage(MemoryUsage::CPU_WRITE | MemoryUsage::CPU_READ);

    // Allocate transcode buffer
    mTransBufStartOffset = allocateBuffer(&mTransBuf, mTransBufMapping,
                                QC2SpeechCodecConfig::getTranscodeBufSize(getCodecType()));
    if (!mTransBufStartOffset) {
        QLOGE("%s: Failed to allocate transcode buffer", __func__);
        return QC2_ERROR;
    }
    mTransBufDataOffset = mTransBufStartOffset;

    // Allocate residual buffer
    mResidualBufStartOffset =
        allocateBuffer(&mResidualBuf, mResidualBufMapping,
                       QC2SpeechCodecConfig::getResidualBufSize(getCodecType()));
    if (!mResidualBufStartOffset) {
        QLOGE("%s: Failed to allocate residual buffer", __func__);
        return QC2_ERROR;
    }
    mResidualBufDataOffset = mResidualBufStartOffset;

    reconfigureComponent();
    return QC2_OK;
}

QC2Status QC2AudioSwSpeechCommonDec::process(const std::shared_ptr<QC2Buffer>& input,
                                     std::shared_ptr<QC2Buffer>& output) {
    (void)input;
    (void)output;
    return QC2_OK;
}

QC2Status QC2AudioSwSpeechCommonDec::reconfigureComponent() {

    // C2 Params to be updated on the interface
    std::list<std::shared_ptr<C2Param>> paramUpdatesOnInterface;

    // Update sample rate
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mParamIntfMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        paramUpdatesOnInterface.push_back(sampleRateParam);
        mUpdatedInfos.push_back(sampleRateParam);
        mParamIntfMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample
    auto bitDepth = mDecoderConfig->getBitDepth();
    if (bitDepth != mParamIntfMap[kParamIndexPcmEncoding]) {
        auto it = sBitsPerSampletoPcmEncoding.find(bitDepth);
        if (it != sBitsPerSampletoPcmEncoding.end()) {
            auto bitDepthParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, it->second);
            paramUpdatesOnInterface.push_back(bitDepthParam);
            mUpdatedInfos.push_back(bitDepthParam);
            mParamIntfMap[kParamIndexPcmEncoding] = bitDepth;
        }
    }

    // Update channels
    auto channels = mDecoderConfig->getChannels();
    if (channels != mParamIntfMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        paramUpdatesOnInterface.push_back(channelsParam);
        mUpdatedInfos.push_back(channelsParam);
        mParamIntfMap[kParamIndexChannelCount] = channels;
    }

    onConfigUpdate(paramUpdatesOnInterface);

    return QC2_OK;
}

}   // namespace qc2audio

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
#include "QC2AudioHwAmrWbPlusDec.h"
#include "QC2AudioHwUtils.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAmrWbPlusDec"
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
//                              QC2AudioHwAmrWbPlusDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAmrWbPlusDec::QC2AudioHwAmrWbPlusDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwAmrDec(codecName, sessionId, instanceName, mime,
                               variant, listener) {
       setDecoderConfig(std::make_shared<QC2AmrWbPlusCodecConfig>());
       setAmrCodecType(AmrVersion::AMRWBPlus);
}

QC2Status QC2AudioHwAmrWbPlusDec::onStart() {

    QC2Status retVal = QC2_OK;
    QLOGD_INST("%s", __func__);
    uint64_t* sessionHandle;

    mInputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getChannelCount(),
                        .codec_type = Type_AMR_WB_PLUS};
    mOutputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getChannelCount(),
                        .codec_type = Type_PCM};
    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &sessionHandle);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }
    setHwSessionId(sessionHandle);
    return mAudioHw->start_session(sessionHandle);
}

void QC2AudioHwAmrWbPlusDec::onOutputsDone(
        std::shared_ptr<QC2AudioHwBuffer> outBuf) {


    std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
    auto outputPair = mPendingOutputs.find(outBuf->input_frame_id);
    if (outputPair == mPendingOutputs.end()) {
        QLOGE("%s: Invalid output returned from HW, output frame id = %llu. Dropping output",
                                     __func__, (unsigned long long)outBuf->output_frame_id);
        return;
    }

    auto output = outputPair->second;
    output->setTimestamp(outBuf->ts);
    output->linear().setRange(outBuf->offset, outBuf->filled_length);
    QLOGV("%s: Output flags = 0x%x", __func__, (unsigned int)output->flags());
    if (output->flags() & BufFlag::EOS)
        QLOGD("%s: EOS ACK Buffer detected in read buffer %llu", __func__,
                                                    (unsigned long long)outBuf->output_frame_id);
    if (outBuf->payload != nullptr) {
        QC2AudioCommonCodecConfig* updated_amrwbplus_config =
                                    reinterpret_cast<QC2AudioCommonCodecConfig*>(outBuf->payload);
        auto sr = updated_amrwbplus_config->sample_rate;
        auto ch = updated_amrwbplus_config->channel_count;
        auto bps = updated_amrwbplus_config->bits_per_sample;
        if (sr && sr != mDecoderConfig->getSampleRate()) {
            QLOGV("%s: New Sample Rate = %u", __func__, (unsigned int)sr);
            auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sr);
            mUpdatedInfos.push_back(sampleRateParam);
            mDecoderConfig->setSampleRate(sr);
        }
        if (ch && ch != mDecoderConfig->getChannelCount()) {
            QLOGV("%s: New Channel Count = %u", __func__, (unsigned int)ch);
            auto channelCountParam = std::make_shared<C2StreamChannelCountInfo::output>(0, ch);
            mUpdatedInfos.push_back(channelCountParam);
            mDecoderConfig->setChannelCount(ch);
        }
        if (bps && bps != mDecoderConfig->getBitsPerSample()) {
            QLOGV("%s: New bits per sample %u", __func__, bps);
            mDecoderConfig->setBitsPerSample(bps);
            auto pcmEncoding = FormatMapper::mapBitsPerSampletoPcmEncoding(
                mDecoderConfig->getBitsPerSample());
            auto pcmEncodingParam =
                std::make_shared<C2StreamPcmEncodingInfo::output>(0,
                                                                  pcmEncoding);
            mUpdatedInfos.push_back(pcmEncodingParam);
        }
    }
    if (mUpdatedInfos.size()) {
        QLOGD_INST("ConfigUpdate: sending %zu updates", mUpdatedInfos.size());
        //Ensure Component Interface is also updated with latest param values
        onConfigUpdate(mUpdatedInfos);
    }
    //Signal to component that output is ready
    signalOutputsDone(output);
    mUpdatedInfos.clear();
    mPendingOutputs.erase(outputPair);
}

////////////////////////////// Helper Functions ///////////////////////////////


}   // namespace qc2audio

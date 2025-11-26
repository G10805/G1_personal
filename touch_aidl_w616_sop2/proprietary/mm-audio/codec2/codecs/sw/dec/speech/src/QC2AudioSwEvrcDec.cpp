/*
 **************************************************************************************************
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
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
#include "QC2AudioSwEvrcDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwEvrcDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static char const * EVRC_DECODER_LIB = "libEvrcSwCodec.so";

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwEvrcDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwEvrcDec::QC2AudioSwEvrcDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
            : QC2AudioSwSpeechCommonDec(codecName, sessionId, instanceName, mime, variant, listener) {
}

QC2Status QC2AudioSwEvrcDec::onStop() {
    return QC2AudioSwSpeechCommonDec::onStop();
}

QC2Status QC2AudioSwEvrcDec::onInit() {
    QLOGD_INST("%s", __func__);

    // Initialize EVRC specific CSIM structure with defaults
    mDecoderConfig->setChannels(SpeechDefaults::SpeechDefaultChannels);
    mDecoderConfig->setBitDepth(SpeechDefaults::SpeechDefaultBitDepth);
    mDecoderConfig->setSampleRate(SpeechDefaults::SpeechDefaultSampleRate);

    mLibHandle = dlopen(EVRC_DECODER_LIB, RTLD_LAZY);
    if (!mLibHandle) {
        QLOGE("%s: CSIM decoder library open failed!", __func__);
        return QC2_ERROR;
    }
    mDecoderInit = (DecoderInit) dlsym (mLibHandle, "evrc_init_dec");
    mGetDecoderConfigSize =
        (DecoderConfigSize) dlsym (mLibHandle, "get_sizeof_evrc_dec_struct");
    mProcessData = (DecoderLib_Process) dlsym (mLibHandle, "evrc_decode");

    if (!mDecoderInit || !mGetDecoderConfigSize || !mProcessData) {
        QLOGE("%s: Symbols not found in the CSIM decoder library!", __func__);
        return QC2_NOT_FOUND;
    }

    return QC2AudioSwSpeechCommonDec::onInit();
}

QC2Status QC2AudioSwEvrcDec::onStart() {
    int16_t decStructSize = (*mGetDecoderConfigSize)();
    mDecStructInst = std::make_shared<std::vector<uint8_t>>(decStructSize);
    (*mDecoderInit)(static_cast<void *>(mDecStructInst->data()));
    return QC2AudioSwSpeechCommonDec::onStart();
}

QC2Status QC2AudioSwEvrcDec::process(const std::shared_ptr<QC2Buffer>& input,
                                     std::shared_ptr<QC2Buffer>& output) {

    // Obtain input buffer offset
    std::unique_ptr<QC2Buffer::Mapping> bufMappingInfo = input->linear().map();
    if (bufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    uint8_t *inputBasePtr = bufMappingInfo->baseRW();

    // Obtain output buffer offset
    std::unique_ptr<QC2Buffer::Mapping> outBufMappingInfo = output->linear().map();
    if (outBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    uint8_t *outputBasePtr = outBufMappingInfo->baseRW();

    if (!inputBasePtr || !outputBasePtr) {
        QLOGE("%s: Could not obtain input or output data offset", __func__);
        return QC2_ERROR;
    }

    uint32_t srcDataConsumed = 0;
    uint8_t rate = 0;
    mIsFullFrame = true;
    uint32_t bytesTranscoded = 0;
    uint8_t transcodeCount = 0;
    uint8_t *swapIndex = nullptr;
    uint32_t availLength = input->linear().size();
    uint64_t flags = input->flags();
    mFrameCount = 0;

   QLOGV("%s: size %d offset %d flags %" PRIu64 "\n",
          __func__, availLength, input->linear().offset(), flags);

    if (!availLength) {
        QLOGI("Received zero length buffer, returning as is.");
        output->setFlags(flags);
        output->linear().setRange(0, availLength);
        onOutputDone(output);
        onInputDone(input);
        return QC2_OK;
    }

    uint32_t transcodeFrameLength =
        QC2SpeechCodecConfig::getTranscodeFrameLength(getCodecType());
    uint32_t transcodeFrameCount =
        QC2SpeechCodecConfig::getTranscodedFrames(getCodecType());

    while (srcDataConsumed < availLength) {
        QC2Status status = transcodeData(inputBasePtr, availLength,
                                         &srcDataConsumed, &rate, &sFrmInfo[0]);
        if (status != QC2_OK) {
            QLOGE("%s: Failed to transcode data", __func__);
            output->setFlags(flags);
            output->linear().setRange(0, 0);
            onOutputDone(output);
            onInputDone(input);
            return status;
        }

        if (mIsFullFrame) {
            bytesTranscoded += transcodeFrameLength;
            transcodeCount++;
            if (!bytesTranscoded) {
                QLOGE("%s: Zero bytes transcoded", __func__);
                output->setFlags(flags);
                output->linear().setRange(0, 0);
                onOutputDone(output);
                onInputDone(input);
                return QC2_ERROR;
            }
            QLOGV("%s: bytesTranscoded =%u, transcodeCount=%d, srcDataConsumed=%u",
                   __func__, bytesTranscoded,transcodeCount,srcDataConsumed);
            if (bytesTranscoded == QC2SpeechCodecConfig::getMaxOutputBufferSize()) {
                for (int transIndex = 0; transIndex < transcodeFrameCount; transIndex++) {
                    if (transIndex * 160 > output->linear().size()) {
                        QLOGE("%s: Exceeding the capacity range %d.", __func__, output->linear().size());
                        break;
                    }
                    swapIndex = mTransBufStartOffset + transIndex * transcodeFrameLength;
                    byteSwap(swapIndex, transcodeFrameLength);
                    (*mProcessData)(static_cast<void *>(mDecStructInst->data()),
                        (int16_t*)(mTransBufStartOffset + transIndex * transcodeFrameLength),
                        (int16_t*)(outputBasePtr + transIndex * 160), 0);
                }
                mFrameCount = transcodeCount;
                bytesTranscoded = 0;
                transcodeCount = 0;
                mTransBufDataOffset = mTransBufStartOffset;
            }
        }
    }

    if (bytesTranscoded > 0) {
        for (int transIndex = 0; transIndex < transcodeCount; transIndex++) {
            if (transIndex * 160 > output->linear().size()) {
                QLOGE("%s: Exceeding the capacity range %d.", __func__, output->linear().size());
                break;
            }
            swapIndex = mTransBufStartOffset + transIndex * transcodeFrameLength;
            byteSwap(swapIndex, transcodeFrameLength);
            (*mProcessData)(static_cast<void *>(mDecStructInst->data()),
                (int16_t*)(mTransBufStartOffset + transIndex * transcodeFrameLength),
                (int16_t*)(outputBasePtr + transIndex * 160), 0);
        }
        mFrameCount = transcodeCount;
        bytesTranscoded = 0;
        transcodeCount = 0;
        mTransBufDataOffset = mTransBufStartOffset;
    }

    output->setFlags(flags);
    output->linear().setRange(0, mFrameCount*320);
    onOutputDone(output);
    onInputDone(input);

    return QC2_OK;
}

}   // namespace qc2audio

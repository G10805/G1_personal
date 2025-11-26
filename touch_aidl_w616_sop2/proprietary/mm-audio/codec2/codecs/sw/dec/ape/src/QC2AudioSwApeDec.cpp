/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <algorithm>
#include <inttypes.h>
#include <memory>
#include <string>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioSwApeDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwApeDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static char const * APE_DECODER_LIB = "libApeSwDec.so";

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwApeDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwApeDec::QC2AudioSwApeDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
            : QC2AudioSwCodec(codecName, sessionId, instanceName, mime,
                    MimeType::RAW, KIND_DECODER, variant, listener) {
        mAPEDecHandle = nullptr;
        mAPELibHandle = nullptr;
        mMinOutBufSize = 0;
        mByteStreamBuf = nullptr;
        mDecoderConfig = std::make_shared<QC2ApeDecConfig>();
}

QC2Status QC2AudioSwApeDec::configure(const C2Param& param) {
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
                    QLOGE("Invalid pcm encoding format %d", pcmEncoding);
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
            QC2AudioSwCodec::configure(param);
            break;
        }
    }

    return retVal;
}

QC2Status QC2AudioSwApeDec::onStop() {
    QLOGV("%s", __func__);

    DecErr_t result = DEC_FAILURE;
    InterfaceTypes_t initType = DEC_DEINIT;
    auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
    if (mAPEDecHandle != nullptr) {
        (*mDecoderInit)((void*)mAPEDecHandle, &result,
                (void *)(mDecoderConfig->getApeDecConfig().get()),
            reqMediaType.get(), initType);
        dlclose(mAPELibHandle);
    }
    mParamIntfMap.clear();
    return QC2_OK;
}

QC2Status QC2AudioSwApeDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not allocated, return error", __func__);
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

    mAPELibHandle = dlopen(APE_DECODER_LIB, RTLD_LAZY);
    if (!mAPELibHandle) {
        QLOGE("%s: CSIM decoder library open failed!", __func__);
        return QC2_ERROR;
    }
    mDecoderInit = (DecoderInit) dlsym (mAPELibHandle, "ape_dec_interface");
    mProcessData = (DecoderLib_Process) dlsym (mAPELibHandle, "ape_dec_process");

    if (!mDecoderInit || !mProcessData) {
        QLOGE("%s: Symbols not found in the CSIM decoder library!", __func__);
        return QC2_NOT_FOUND;
    }

    return QC2_OK;
}

QC2Status QC2AudioSwApeDec::onStart() {
    QLOGD_INST("%s", __func__);

    mScratchPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (!mScratchPool) {
        QLOGE("%s: Failed to create scratch buffer pool !", __func__);
        return QC2_ERROR;
    }
    mScratchPool->setUsage(MemoryUsage::CPU_WRITE | MemoryUsage::CPU_READ);
    if (!mDecoderConfig) {
        QLOGE("%s: decoder config not allocated, return error", __func__);
        return QC2_ERROR;
    }
    allocByteStreamBuffer(mDecoderConfig->getMaxInputBufSize() * 2);
    mMinOutBufSize = mDecoderConfig->getMaxOutputBufSize();

    return QC2_OK;
}

QC2Status QC2AudioSwApeDec::process(const std::shared_ptr<QC2Buffer>& input,
                                     std::shared_ptr<QC2Buffer>& output) {

    uint32_t inputSize = input->linear().size();
    DecErr_t result = DEC_FAILURE;
    uint64_t flags = input->flags();
    bool eos = false;
    size_t outputSize = 0;

    if (flags & BufFlag::EOS) {
        eos = true;
        QLOGV("%s: Parser reported EOS", __func__);
    }

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

    // Get output buffer's data offset
    std::unique_ptr<QC2Buffer::Mapping> outbufMappingInfo = output->linear().map();
    if (outbufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    uint8_t *outputOffset = outbufMappingInfo->baseRW();
    if (outputOffset == nullptr) {
        QLOGE("%s: Base ptr is null for output buffer", __func__);
        return QC2_NO_MEMORY;
    }

    QLOGV("%s: input: size %d offset %d flags %" PRIu64 "\n",
          __func__, inputSize, input->linear().offset(), flags);

    if (flags & BufFlag::CODEC_CONFIG) {
        mDecoderConfig->setApeDecConfig(inputOffset);

        QLOGD("%s: APE CSD values: BlocksPerFrame %d Total Frames %d Bits Per Sample %d"
                        " Channels %d samplingRate %d Version %d\n"
                        " Compression Level %d Final Frame Blocks %d seek table present %d",
              __func__, mDecoderConfig->getBlocksPerFrame(), mDecoderConfig->getTotalFrames(),
              mDecoderConfig->getBitsPerSample(), mDecoderConfig->getChannels(),
              mDecoderConfig->getSampleRate(), mDecoderConfig->getCompatibleVersion(),
              mDecoderConfig->getCompressionLevel(), mDecoderConfig->getFinalFrameBlocks(),
              mDecoderConfig->isSeekTablePresent());

        DecErr_t result = DEC_FAILURE;
        InterfaceTypes_t initType = DEC_INIT;
        auto reqMediaType = std::make_shared<GenericDecMediaFmt_t>();
        if (!mDecoderInit) {
            QLOGE("%s: CSIM functions not initialized", __func__);
            return QC2_NOT_FOUND;
        }
        mAPEDecHandle = (*mDecoderInit)((void*)mAPEDecHandle, &result,
                                 (void *)(mDecoderConfig->getApeDecConfig().get()),
            reqMediaType.get(), initType);
        if (result != DEC_SUCCESS || !mAPEDecHandle) {
            QLOGD("%s: Unable to setup decoder", __func__);
            return QC2_CANNOT_DO;
        }

        uint32_t compressionLevel = mDecoderConfig->getCompressionLevel();
        if (compressionLevel == APE_COMPRESSION_LEVEL_INSANE ||
                compressionLevel == APE_COMPRESSION_LEVEL_EXTRA_HIGH) {
            QLOGI("%s: Reallocate buffers for APE extra high or insane mode", __func__);
            uint32_t inputMultiplier = (compressionLevel == APE_COMPRESSION_LEVEL_INSANE) ?
                        sApeInsaneModeInputSizeMultiplier : sApeExtraHighModeInputSizeMultiplier;
            uint32_t outputMultiplier = (compressionLevel == APE_COMPRESSION_LEVEL_INSANE) ?
                        sApeInsaneModeOutputSizeMultiplier : sApeExtraHighModeOutputSizeMultiplier;
            allocByteStreamBuffer(mDecoderConfig->getMaxInputBufSize() * inputMultiplier);
            mMinOutBufSize = mDecoderConfig->getMaxOutputBufSize() * outputMultiplier;
            mLinearPool->setBufferSize(mMinOutBufSize);
        }

        reconfigureComponent();
        output->setFlags(flags);
        output->linear().setRange(0, 0);
        onOutputDone(output);
        onInputDone(input);
        return QC2_OK;
    } else if (inputSize == 0) {
        output->setFlags(flags);
        output->linear().setRange(0, 0);
        onOutputDone(output);
        onInputDone(input);
    }

    if (updateInputBitstream(&mBuffer, inputOffset, inputSize) == -1) {
        QLOGE("%s: bitstream error!", __func__);
        return QC2_ERROR;
    }

    auto inBuffer = std::make_shared<AudioBuf_t>();
    auto outBuffer = std::make_shared<AudioBuf_t>();
    auto outMediaType = std::make_shared<GenericDecMediaFmt_t>();

    while (enoughDataAvailable(&mBuffer) || eos) {
        // EOS reported; see if there is any data left in the input buffer
        if ((mBuffer.i32WritePtr >= mBuffer.i32ReadPtr) && eos) {
            QLOGV("%s: Parser reported EOS", __func__);
            mBuffer.eos = 1;
        }
        int64_t bytesRemaining = mBuffer.i32WritePtr - mBuffer.i32ReadPtr;
        if (bytesRemaining <= 0 && eos) {
            // EOS reported and also all the data in input buffer is consumed
            QLOGV("%s: Report EOS as no more bitstream is left with the decoder", __func__);
            output->setFlags(flags | BufFlag::EOS);
            output->linear().setRange(0, outputSize);
            onOutputDone(output);
            onInputDone(input);
            return QC2_OK;
        }
        QLOGV("%s: Bytes left in internal buffer: %" PRIi64 "\n", __func__, bytesRemaining);

        if (bytesRemaining > 0) {
            QLOGV("%s: Calling Decode Frame; writePtr=%d, readPtr=%d",
                   __func__, mBuffer.i32WritePtr, mBuffer.i32ReadPtr);
            inBuffer->mDataPtr = &mBuffer.ui8TempBuf[mBuffer.i32ReadPtr];
            inBuffer->mMaxDataLen = bytesRemaining;
            inBuffer->mActualDataLen = bytesRemaining;

            outBuffer->mDataPtr = outputOffset + outputSize;
            outBuffer->mMaxDataLen = mMinOutBufSize - outputSize;
            outBuffer->mActualDataLen = mMinOutBufSize - outputSize;

            // Hack - To make APE CSIM decode any input size
            // This needs to stay until CSIM is updated
            inBuffer->mMarker.mEosFlag = 1;

            QLOGV("%s: Calling decode in frame mode input buf size is %d and output buf size %d",
                   __func__, inBuffer->mMaxDataLen, outBuffer->mMaxDataLen);
            if (!mProcessData) {
                QLOGE("%s: CSIM functions not initialized", __func__);
                return QC2_NOT_FOUND;
            }
            (*mProcessData)((void*)mAPEDecHandle, &result,
                     inBuffer.get(), outBuffer.get(), outMediaType.get());
            QLOGV("%s: Decode Frame done; status=%d, OutputBufSize=%d, usedBitStream=%d",
                   __func__, result, outBuffer->mActualDataLen, inBuffer->mActualDataLen);
        } else {
            break;
        }
        // Decoding is done; Update read and write pointers of the input buffer
        if (!updatePointers(&mBuffer, inBuffer->mActualDataLen, result)) {
            if (mBuffer.eos) {
                break;
            } else {
                // Some error or insufficient data - read again from parser
                continue;
            }
        } else {
            //Successful decode!
            QLOGV("%s: Decode successful consumed bytes are %d and output frame size is %d",
                   __func__, inBuffer->mActualDataLen, outBuffer->mActualDataLen);
            outputSize += outBuffer->mActualDataLen;
        }
    }

    if (result != DEC_SUCCESS && result != DEC_EOF) {
        QLOGW("%s: decoder returned error %d, substituting silence", __func__, result);
        memset(outputOffset, 0, mMinOutBufSize);
    }
    output->setFlags(flags);
    output->linear().setRange(0, outputSize);
    onOutputDone(output);
    onInputDone(input);

    return QC2_OK;
}

QC2Status QC2AudioSwApeDec::reconfigureComponent() {

    // Update sample rate
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mParamIntfMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mParamIntfMap[kParamIndexSampleRate] = sampleRate;
    }

    // Update bits per sample
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitsPerSample());
    if (pcmEncoding != mParamIntfMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mParamIntfMap[kParamIndexPcmEncoding] = pcmEncoding;
    }

    // Update channels
    auto channels = mDecoderConfig->getChannels();
    if (channels != mParamIntfMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mParamIntfMap[kParamIndexChannelCount] = channels;
    }

    // Update compression level
    auto compressionLevel = mDecoderConfig->getCompressionLevel();
    if (compressionLevel != mParamIntfMap[kParamIndexAudioCompressionLevel]) {
        auto compressionLevelParam =
            std::make_shared<C2AudioCompressionLevelInfo::input>(0, compressionLevel);
        mUpdatedInfos.push_back(compressionLevelParam);
        mParamIntfMap[kParamIndexAudioCompressionLevel] = compressionLevel;
    }

    // Update format flags
    auto formatFlags = mDecoderConfig->getFormatFlags();
    if (formatFlags != mParamIntfMap[kParamIndexAudioFormatFlags]) {
        auto formatFlagsParam = std::make_shared<C2AudioFormatFlagsInfo::input>(0, formatFlags);
        mUpdatedInfos.push_back(formatFlagsParam);
        mParamIntfMap[kParamIndexAudioFormatFlags] = formatFlags;
    }

    // Update blocks per frame
    auto blocksPerFrame = mDecoderConfig->getBlocksPerFrame();
    if (blocksPerFrame != mParamIntfMap[kParamIndexAudioBlocksPerFrame]) {
        auto blocksPerFrameParam =
            std::make_shared<C2AudioBlocksPerFrameInfo::input>(0, blocksPerFrame);
        mUpdatedInfos.push_back(blocksPerFrameParam);
        mParamIntfMap[kParamIndexAudioBlocksPerFrame] = blocksPerFrame;
    }

    // Update final frame blocks
    auto finalFrameBlocks = mDecoderConfig->getFinalFrameBlocks();
    if (finalFrameBlocks != mParamIntfMap[kParamIndexAudioFinalFrameBlocks]) {
        auto finalFrameBlocksParam =
            std::make_shared<C2AudioFinalFrameBlocksInfo::input>(0, finalFrameBlocks);
        mUpdatedInfos.push_back(finalFrameBlocksParam);
        mParamIntfMap[kParamIndexAudioFinalFrameBlocks] = finalFrameBlocks;
    }

    // Update total frames
    auto totalFrames = mDecoderConfig->getTotalFrames();
    if (totalFrames != mParamIntfMap[kParamIndexAudioTotalFrames]) {
        auto totalFramesParam = std::make_shared<C2AudioTotalFramesInfo::input>(0, totalFrames);
        mUpdatedInfos.push_back(totalFramesParam);
        mParamIntfMap[kParamIndexAudioTotalFrames] = totalFrames;
    }

    // Update seek table info
    auto seekTablePresent = mDecoderConfig->isSeekTablePresent();
    if (seekTablePresent != mParamIntfMap[kParamIndexAudioSeekTablePresent]) {
        auto seekTablePresentParam =
            std::make_shared<C2AudioSeekTablePresentInfo::input>(0, seekTablePresent);
        mUpdatedInfos.push_back(seekTablePresentParam);
        mParamIntfMap[kParamIndexAudioSeekTablePresent] = seekTablePresent;
    }

    return QC2_OK;
}

uint8_t * QC2AudioSwApeDec::allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
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

int32_t QC2AudioSwApeDec::enoughDataAvailable(byteStreamBuffer *pByteBuffer) {

    // Check if data from parser is needed or not
    int64_t bytesRemain = pByteBuffer->i32WritePtr - pByteBuffer->i32ReadPtr;
    if (((bytesRemain > 0 && bytesRemain <= INT32_MAX) || pByteBuffer->eos) &&
             !pByteBuffer->error) {
        return 1;
    }

    return 0;
}

int32_t QC2AudioSwApeDec::updateInputBitstream(byteStreamBuffer *pByteBuffer,
                                                void * bitstream, int32_t inSize) {
    uint8_t *bitsbuffer = (uint8_t*) bitstream;

    int64_t bytesRemain = pByteBuffer->i32WritePtr - pByteBuffer->i32ReadPtr;
    QLOGV("%s: bytesRemain : %" PRIi64 ", InData : %d, \
             pByteBuffer->error : %d, pByteBuffer->eos: %d", __func__,
           bytesRemain, inSize, pByteBuffer->error, pByteBuffer->eos);
    if ((pByteBuffer->eos || inSize == 0) && !pByteBuffer->error) {
        pByteBuffer->error = 0;
        return 0;
    }
    pByteBuffer->error = 0;

    if (pByteBuffer->i32ReadPtr < 0 || pByteBuffer->i32WritePtr < 0 ||
            pByteBuffer->i32ReadPtr > pByteBuffer->i32WritePtr) {
        QLOGW("%s: Resetting corrupted read/write pointers", __func__);
        pByteBuffer->i32WritePtr = 0;
        pByteBuffer->i32ReadPtr = 0;
        bytesRemain = 0;
    }

    /* First copy the un-decoded bitstream to start of buffer */
    for (uint32_t i = 0; i < bytesRemain; i++) {
        pByteBuffer->ui8TempBuf[i] = pByteBuffer->ui8TempBuf[pByteBuffer->i32ReadPtr+i];
    }
    pByteBuffer->i32ReadPtr = 0;
    pByteBuffer->i32WritePtr = bytesRemain;

    int64_t sizeRemaining = pByteBuffer->i32BufferSize - pByteBuffer->i32WritePtr;
    if (inSize > sizeRemaining) {
        // report error for now?
        QLOGE("inSize (%d) > avail size (%" PRIi64 ")", inSize, sizeRemaining);
        return -1;
    } else {
        memcpy(&pByteBuffer->ui8TempBuf[pByteBuffer->i32WritePtr], bitsbuffer, inSize);
        pByteBuffer->i32WritePtr += inSize;
    }

    return 1;
}

int32_t QC2AudioSwApeDec::updatePointers(byteStreamBuffer *pByteBuffer,
                                          uint32_t readBytes, int32_t result) {
    pByteBuffer->i32ReadPtr += readBytes;
    if (pByteBuffer->i32ReadPtr > pByteBuffer->i32WritePtr) {
        QLOGW("%s: Processed data length exceeds input length!", __func__);
    }
    if ((result == DEC_SUCCESS) || (result == DEC_EOF)) {
        QLOGV("%s: Successful decode!", __func__);
        return 1;
    } else if ((result == DEC_FAILURE) || (result == DEC_BADPARAM_FAILURE)) {
        QLOGE("%s: Erroneous decode!", __func__);
    } else if (result == DEC_NEED_MORE) {
        QLOGV("%s: Not enough data to decode!", __func__);
    }

    /* Reset internal buffering if the entire data contains erroneous data
     * and no sync word has been found
     */

    int64_t bytesRemaining = pByteBuffer->i32WritePtr - pByteBuffer->i32ReadPtr;
    if (bytesRemaining >= pByteBuffer->i32BufferSize || bytesRemaining < 0) {
        QLOGE("%s: erroneous write and read ptrs", __func__);
        pByteBuffer->i32WritePtr = 0;
        pByteBuffer->i32ReadPtr = 0;
    }
    pByteBuffer->error = 1;

    return 0;
}

int32_t QC2AudioSwApeDec::allocByteStreamBuffer(uint32_t bufferingSize) {
    mBuffer.i32BufferSize = bufferingSize;
    mBuffer.ui8TempBuf = allocateBuffer(&mByteStreamBuf,
                                        mByteStreamBufMapping, mBuffer.i32BufferSize);
    if (!mBuffer.ui8TempBuf) {
        QLOGW("%s: Failed to allocate temp buffer", __func__);
        return -1;
    }
    mBuffer.i32ReadPtr = 0;
    mBuffer.i32WritePtr = 0;
    mBuffer.eos = 0;
    mBuffer.error = 0;
    return 0;
}

}   // namespace qc2audio

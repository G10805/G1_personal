/*
 **************************************************************************************************
 * Copyright (c) 2015-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * Not a Contribution.

 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************
*/

#include <algorithm>
#include <inttypes.h>
#include <list>
#include <memory>
#include <string>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioSwFlacDec.h"

#undef LOG_TAG
#define LOG_TAG "QC2AudioSwFlacDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static char const * FLAC_DECODER_LIB = "libFlacSwDec.so";


///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioSwFlacDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioSwFlacDec::QC2AudioSwFlacDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
            : QC2AudioSwCodec(codecName, sessionId, instanceName, mime,
                    MimeType::RAW, KIND_DECODER, variant, listener) {
        mFLACLibHandle = nullptr;
        mMinOutBufSize = 0;
        mInitStatus = false;
        mDecodedBlocks = 0;
        mDecoderInit = nullptr;
        mProcessData = nullptr;
        mDecoderEnd = nullptr;
        mSignalledError = false;
        mBuffer.i32BufferSize = 0;
        mBuffer.i32ReadPtr = 0;
        mBuffer.i32WritePtr = 0;
        mBuffer.ui8TempBuf = nullptr;
        mBuffer.eos = 0;
        mBuffer.error = 0;
        mOutBuf = nullptr;
        mTmpBuf = nullptr;
        mByteStreamBuf = nullptr;
        mOutBufOffset = nullptr;
        mTmpBufOffset = nullptr;
        mParserInfoToPass.i32NumChannels = FlacDefaults::FLACDefaultChannels;
        mParserInfoToPass.i32SampleRate = FlacDefaults::FLACDefaultSampleRate;
        mParserInfoToPass.i32BitsPerSample = FlacDefaults::FLACDefaultBitDepth;
        mParserInfoToPass.i32MinBlkSize = FlacDefaults::FLACDefaultMinBlkSize;
        mParserInfoToPass.i32MaxBlkSize = FlacDefaults::FLACDefaultMaxBlkSize;
        mParserInfoToPass.i32MinFrmSize = FlacDefaults::FLACDefaultMinFrameSize;
        mParserInfoToPass.i32MaxFrmSize = FlacDefaults::FLACDefaultMaxFrameSize;
        memcpy(&mCsdInfo, &mParserInfoToPass, sizeof(FLACDec_ParserInfo));
        mDecoderConfig = std::make_shared<QC2FlacCodecConfig>();
}

uint8_t * QC2AudioSwFlacDec::allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
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

void QC2AudioSwFlacDec::updateDecoderConfig() {
    mDecoderConfig->setChannels(mParserInfoToPass.i32NumChannels);
    mDecoderConfig->setBitDepth(mParserInfoToPass.i32BitsPerSample);
    mDecoderConfig->setSampleRate(mParserInfoToPass.i32SampleRate);
    mDecoderConfig->setMinFrameSize(mParserInfoToPass.i32MinFrmSize);
    mDecoderConfig->setMaxFrameSize(mParserInfoToPass.i32MaxFrmSize);
    mDecoderConfig->setMinBlkSize(mParserInfoToPass.i32MinBlkSize);
    mDecoderConfig->setMaxBlkSize(mParserInfoToPass.i32MaxBlkSize);
}

QC2Status QC2AudioSwFlacDec::configure(const C2Param& param) {
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
        case kParamIndexAudioMinFrameBytes: {
            C2AudioMinFrameBytesInfo::input in;
            if (param.index() == in.index()) {
                auto minFrameSize =
                    ((C2AudioMinFrameBytesInfo::input*)&param)->value;
                QLOGV("%s: min frame size %d", __func__, minFrameSize);
                mDecoderConfig->setMinFrameSize(minFrameSize);
            }
            break;
        }
        case kParamIndexAudioMaxFrameBytes: {
            C2AudioMaxFrameBytesInfo::input in;
            if (param.index() == in.index()) {
                auto maxFrameSize =
                    ((C2AudioMaxFrameBytesInfo::input*)&param)->value;
                QLOGV("%s: max frame size %d", __func__, maxFrameSize);
                mDecoderConfig->setMaxFrameSize(maxFrameSize);
            }
            break;
        }
        case kParamIndexAudioMinBlockSize: {
            C2AudioMinBlockSizeInfo::input in;
            if (param.index() == in.index()) {
                auto minBlkSize =
                    ((C2AudioMinBlockSizeInfo::input*)&param)->value;
                QLOGV("%s: min block size %d", __func__, minBlkSize);
                mDecoderConfig->setMinBlkSize(minBlkSize);
            }
            break;
        }
        case kParamIndexAudioMaxBlockSize: {
            C2AudioMaxBlockSizeInfo::input in;
            if (param.index() == in.index()) {
                auto maxBlkSize =
                    ((C2AudioMaxBlockSizeInfo::input*)&param)->value;
                QLOGV("%s: max block size %d", __func__, maxBlkSize);
                mDecoderConfig->setMaxBlkSize(maxBlkSize);
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

QC2Status QC2AudioSwFlacDec::configure(std::vector<std::unique_ptr<C2Param>> config) {
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

QC2Status QC2AudioSwFlacDec::onStop() {
    QLOGV("%s", __func__);

    if (mTmpBufOffset) {
        mTmpBuf.reset();
        mTmpBufOffset = nullptr;
    }

    if (mOutBufOffset) {
        mOutBuf.reset();
        mOutBufOffset = nullptr;
    }

    if (mDecoderEnd && mInitStatus) {
        (*mDecoderEnd)(&mFlacDecState);
        mDecoderEnd = nullptr;
    }

    if (mFLACLibHandle) {
        dlclose(mFLACLibHandle);
        mFLACLibHandle = nullptr;
    }

    if (mBuffer.ui8TempBuf) {
        mByteStreamBuf.reset();
        mBuffer.ui8TempBuf = nullptr;
    }
    mBuffer.i32WritePtr = 0;
    mBuffer.i32ReadPtr = 0;
    mBuffer.eos = 0;
    mEos = false;
    mDecodedBlocks = 0;
    mIsDecodeSuccess = true;
    mCodecConfigChanged = false;
    mInitStatus = false;
    mFrameStartTimestamp = 0;
    mNonEmptyOutputGenerated = true;
    mPendingInputs.clear();
    mPendingOutputs.clear();
    mCompletedWorks.clear();
    return QC2_OK;
}

QC2Status QC2AudioSwFlacDec::onInit() {
    QLOGD_INST("%s", __func__);

    if (property_get_bool("vendor.qc2audio.per_frame.flac.dec.enabled", true)) {
        mIsFrameByFrameDecode = true;
    } else {
        mIsFrameByFrameDecode = false;
    }

    mFLACLibHandle = dlopen(FLAC_DECODER_LIB, RTLD_LAZY);
    if (!mFLACLibHandle) {
        QLOGE("%s: CSIM decoder library open failed!", __func__);
        return QC2_ERROR;
    }
    mDecoderInit = (DecoderInit) dlsym (mFLACLibHandle, "CFlacDecoderLib_Meminit");
    mProcessData = (DecoderLib_Process) dlsym (mFLACLibHandle, "CFlacDecoderLib_Process");
    mDecoderEnd = (DecoderEnd) dlsym (mFLACLibHandle, "CFlacDecoderLib_End");

    if (!mDecoderInit || !mProcessData || !mDecoderEnd) {
        QLOGE("%s: Symbols not found in the CSIM decoder library!", __func__);
        return QC2_NOT_FOUND;
    }

    return QC2_OK;
}

QC2Status QC2AudioSwFlacDec::initFlacDecoder() {
    QLOGV("%s", __func__);
    int result = 0;

    if (mDecoderEnd && mInitStatus){
        (*mDecoderEnd)(&mFlacDecState);
        QLOGD("%s: will reinit", __func__);
    }
    memset(&mFlacDecState, 0, sizeof(CFlacDecState));
    QLOGV("%s: bitspersample=%d", __func__, mParserInfoToPass.i32BitsPerSample);
    (*mDecoderInit)(&mFlacDecState, &result, (int)mParserInfoToPass.i32BitsPerSample);

    if (result != DEC_SUCCESS) {
        QLOGE("%s: CSIM decoder init failed! Result %d", __func__, result);
        if (result == DEC_NOMEMORY_FAILURE) {
            return QC2_NO_MEMORY;
        }
        return QC2_ERROR;
    } else {
        mInitStatus = true;
    }

    stFLACDec* pstFLACDec = (stFLACDec*)(mFlacDecState.m_pFlacDecoder);
    memcpy(&pstFLACDec->MetaDataBlocks.MetaDataStrmInfo, &mParserInfoToPass, sizeof(FLACDec_ParserInfo));
    mFlacDecState.m_bIsStreamInfoPresent = 1;
    mFlacDecState.ui32MaxBlockSize = pstFLACDec->MetaDataBlocks.MetaDataStrmInfo.i32MaxBlkSize;
    memcpy(mFlacDecState.pFlacDecMetaDataStrmInfo, &mParserInfoToPass, sizeof(FLACDec_ParserInfo));

    if (mFlacDecState.ui32MaxBlockSize > sFlacSupersetThreshold) {
        QLOGV("%s: Allocating bigger buffer for superset clips", __func__);
        mBuffer.i32BufferSize = 2 * sFlacBufferingSize;
    } else {
        mBuffer.i32BufferSize = sFlacBufferingSize;
    }

    if (mBuffer.ui8TempBuf) {
        mByteStreamBuf.reset();
        mBuffer.ui8TempBuf = nullptr;
    }
    mBuffer.ui8TempBuf = allocateBuffer(&mByteStreamBuf,
                                        mByteStreamBufMapping, mBuffer.i32BufferSize);
    mBuffer.i32ReadPtr = 0;
    mBuffer.i32WritePtr = 0;
    mBuffer.eos = 0;
    mBuffer.error = 0;
    if (!mBuffer.ui8TempBuf) {
        QLOGW("%s: Failed to allocate temp buffer", __func__);
        mInitStatus = false;
        return QC2_NO_MEMORY;
    }

    if (mParserInfoToPass.i32BitsPerSample == 24) {
        //decoder output will be in 8_24 PCM format which is 4 bytes
        mDecoderConfig->setBitDepth(sizeof(uint32_t) * 8);
    } else {
        mDecoderConfig->setBitDepth(sizeof(uint16_t) * 8);
    }

    // Update FLAC decoder config with parser info
    updateDecoderConfig();
    // notify component/interface with param updates
    reconfigureComponent();

    // Set input delay
    if (!mIsFrameByFrameDecode) {
        auto inDelayInfo = std::unique_ptr<C2Param>(new C2PortActualDelayTuning::input(10));
        mUpdatedInfos.push_back(std::move(inDelayInfo));
    }
    return QC2_OK;
}

QC2Status QC2AudioSwFlacDec::onStart() {
    QLOGV("%s", __func__);

    mScratchPool = mBufPoolProvider->requestLinearPool(C2BlockPool::BASIC_LINEAR);
    if (!mScratchPool) {
        QLOGE("%s: Failed to create scratch buffer pool !", __func__);
        return QC2_ERROR;
    }
    mScratchPool->setUsage(MemoryUsage::CPU_WRITE | MemoryUsage::CPU_READ);

    mOutBufOffset = allocateBuffer(&mOutBuf, mOutBufMapping, sFlacInstSize);
    mTmpBufOffset = allocateBuffer(&mTmpBuf, mTmpBufMapping, sFlacInstSize);
    if (!mOutBufOffset || !mTmpBufOffset) {
        QLOGW("Failed to allocate scratch buffer\n");
        return QC2_NO_MEMORY;
    }
    mIsDecodeSuccess = true;
    mEos = false;
    mPendingInputs.clear();
    mPendingOutputs.clear();
    mCompletedWorks.clear();
    mNonEmptyOutputGenerated = true;
    mFrameStartTimestamp = 0;
    mCodecConfigChanged = true;
    mMinOutBufSize = sFlacInstSize;

    return QC2_OK;
}

QC2Status QC2AudioSwFlacDec::process(const std::shared_ptr<QC2Buffer> &input,
                                     std::shared_ptr<QC2Buffer> &output) {
    if (mIsFrameByFrameDecode) {
        return processFrameByFrame(input, output);
    }
    return processByThreshold(input, output);
}

QC2Status QC2AudioSwFlacDec::processByThreshold(const std::shared_ptr<QC2Buffer>& input,
                                     std::shared_ptr<QC2Buffer>& output) {
    uint32_t blockSize = 0, usedBitstream = 0, availLength = 0;
    int status = 0;
    QC2Status retVal = QC2_OK;
    bool flagEOS = false;
    bool skipBuffering = false;
    uint64_t flags = 0;
    uint64_t inputIndex = 0;
    size_t outputSize = 0;
    uint32_t outputDeficit = 0;

    stFLACDec* pstFLACDec = (stFLACDec*)(mFlacDecState.m_pFlacDecoder);
    stMetaDataStrmInfo* pstMetaDataStrmInfo =
    (stMetaDataStrmInfo*)(&pstFLACDec->MetaDataBlocks.MetaDataStrmInfo);

    if (mSignalledError) {
        QLOGE("Flac decoder in error state");
        return QC2_ERROR;
    }

    mPendingInputs.push_back(input);
    mPendingOutputs.push_back(output);
    QLOGV("%s: Input: size %d offset %d flags %" PRIu64 "\n",
           __func__, input->linear().size(), input->linear().offset(), input->flags());

    while (!mPendingInputs.empty() && !mPendingOutputs.empty() ) {

        std::shared_ptr<QC2Buffer> inBuf = mPendingInputs.front();
        availLength = inBuf->linear().size();
        flags = inBuf->flags();
        if (mNonEmptyOutputGenerated) {
            QLOGD("%s: Set timestamp %u for input index %u",
                   __func__, (int)inBuf->timestamp(), (uint32_t)inBuf->inputIndex());
            mFrameStartTimestamp = inBuf->timestamp();
        }
        inputIndex = inBuf->inputIndex();

        // Get input buffer's data offset
        std::unique_ptr<QC2Buffer::Mapping> bufMappingInfo = inBuf->linear().map();
        if (bufMappingInfo == nullptr) {
            return QC2_NO_MEMORY;
        }
        auto basePtr = bufMappingInfo->baseRW();
        if (basePtr == nullptr) {
            QLOGE("%s: Base ptr is null for input buffer", __func__);
            return QC2_NO_MEMORY;
        }
        uint8_t *inputOffset = basePtr + inBuf->linear().offset();

        // Get output buffer's data offset
        std::shared_ptr<QC2Buffer> outBuf = mPendingOutputs.front();
        std::unique_ptr<QC2Buffer::Mapping> outbufMappingInfo = outBuf->linear().map();
        if (outbufMappingInfo == nullptr) {
            return QC2_NO_MEMORY;
        }
        uint8_t *outputOffset = outbufMappingInfo->baseRW() + outBuf->linear().offset();
        if (outputOffset == nullptr) {
            QLOGE("%s: Base ptr is null for output buffer", __func__);
            return QC2_NO_MEMORY;
        }

        QLOGV("%s: Process inBuf: size %d offset %d flags %" PRIu64 "\n",
               __func__, availLength, inBuf->linear().offset(), flags);

        // check if new input buffer from parser needs to be read
        if (flags & BufFlag::EOS) {
            mEos = true;
            QLOGD("%s: Parser reported EOS", __func__);
        }

        if (flags & BufFlag::CODEC_CONFIG) {
            if (!parseMetaData(inputOffset, availLength)) {
                QLOGE("%s parseMetaData error",__func__);
                mSignalledError = true;
                return QC2_ERROR;
            }
            if (mCodecConfigChanged) {
                retVal = initFlacDecoder();
                if (retVal != QC2_OK) {
                    QLOGE("Failed to initialize Flac decoder");
                    return retVal;
                }
                mCodecConfigChanged = false;
            }
            outBuf->setFlags(flags);
            outBuf->linear().setRange(0, 0);
            mPendingOutputs.erase(mPendingOutputs.begin());
            onOutputDone(outBuf);
            mPendingInputs.erase(mPendingInputs.begin());
            onInputDone(inBuf);
            return QC2_OK;
        }
        status = updateInputBitstream(&mBuffer, inputOffset, availLength);
        if (status < 0) {
            mSignalledError = true;
            outBuf->setFlags(flags);
            outBuf->linear().setRange(0, 0);
            QLOGE("%s: bitstream error", __func__);
            mPendingOutputs.erase(mPendingOutputs.begin());
            onOutputDone(outBuf);
            mPendingInputs.erase(mPendingInputs.begin());
            onInputDone(inBuf);
            return QC2_ERROR;
        } else if (status > 0) {
            outputDeficit++;
        }

        // check if new output buffer is needed
        if (mIsDecodeSuccess) {
            mIsDecodeSuccess = false;
        }

        while (1) {
            if (enoughDataAvailable(&mBuffer) || mEos) {
                QLOGV("%s: Decoder has enough data. Need not read from parser", __func__);
                availLength = 0;
                // EOS reported; see if there is any data left in the input buffer
                if ((mBuffer.i32WritePtr >= mBuffer.i32ReadPtr) && mEos) {
                    QLOGV("%s: Parser reported EOS", __func__);
                    mBuffer.eos = 1;
                    availLength = 0;
                }
                if ((((mBuffer.i32WritePtr - mBuffer.i32ReadPtr) <= 0) && mBuffer.eos) || flagEOS) {
                    // EOS reported and also all the data in input buffer is consumed
                    availLength = 0;
                    QLOGV("%s: Report EOS as no more bitstream is left with the decoder", __func__);
                    outBuf->setFlags(flags | BufFlag::EOS);
                    outBuf->linear().setRange(0, availLength);
                    mPendingOutputs.erase(mPendingOutputs.begin());
                    onOutputDone(outBuf);
                    mPendingInputs.erase(mPendingInputs.begin());
                    onInputDone(inBuf);
                    mCompletedWorks.clear();
                    return QC2_OK;
                }
            } else if ((int32_t)(mBuffer.i32BufferSize - mBuffer.i32WritePtr) >= sFlacParserBufSize) {
                // Not enough data available in the input buffer, set flag to read more data
                QLOGV("%s: set flag to read bitsream from parser", __func__);
                break;
            } else {
                availLength = 0;
            }

            // availLength being passed is the size read from parser
            status = updateInputBitstream(&mBuffer, inputOffset, availLength);
            if (status < 0) {
                QLOGE("%s: bitstream read error", __func__);
                mSignalledError = true;
                return QC2_ERROR;
            } else if (status > 0) {
                outputDeficit++;
            }

            availLength = mBuffer.i32WritePtr - mBuffer.i32ReadPtr;
            QLOGV("%s: Bytes left in internal buffer: %d", __func__, availLength);

            if (availLength) {
                usedBitstream = 0;
                uint32 flacOutputBufSize = sFlacMaxOutBufSize;
                eFLACDec_Result retVal = (*mProcessData)((void *)&mFlacDecState,
                                         &mBuffer.ui8TempBuf[mBuffer.i32ReadPtr],
                                         availLength,
                                         (void *)mOutBufOffset,
                                         &flacOutputBufSize,
                                         &usedBitstream,
                                         &blockSize);
                mDecoderConfig->setChannels(pstMetaDataStrmInfo->i32NumChannels);
                mDecoderConfig->setSampleRate(pstMetaDataStrmInfo->i32SampleRate);
                status = (int)retVal;

                QLOGV("%s: Decode Frame done; status=%d, flacOutputBufSize=%d, \
                           usedBitstream=%d blockSize %d",
                      __func__, status, flacOutputBufSize, usedBitstream, blockSize);
            } else {
                break;
            }

            // Decoding is done; Update read and write pointers of the input buffer
            if (!updatePointers(&mBuffer, usedBitstream, status)) {
                if (mBuffer.eos && status != FLACDEC_SUCCESS) {
                    QLOGV("%s: Error during decode after EOS is set. Report EOS", __func__);
                    flagEOS = true;
                } else {
                    // Some error or insufficient data - read again from parser
                    QLOGW("%s: insufficient data for decode", __func__);
                    continue;
                }
            } else {
                   //Successful decode!
                   //check if we have enough space to accumulate if skipBuffering
                   // Two cases are possible here
                   // a) We are at the boundary hence need to write the earlier buffered data and then copy
                   //    back the new data
                   // b) Error case where in the blockSize from core decoder is corrupt
                   if (isBufferFull(blockSize)) {
                       QLOGI("%s: Not enough space error scenario, mDecodedBlocks %" PRId64 " blockSize %d",
                                __func__, mDecodedBlocks, blockSize);
                       skipBuffering = true;
                       break;
                   }
                   //Interleave the output from decoder for multichannel clips.
                   if (mFlacDecState.m_bits_per_sample == 24) {
                       interleaveData<uint32_t>(blockSize);
                   } else {
                       interleaveData<uint16_t>(blockSize);
                   }
                   break;
            }
        }  // End of while for decoding

        if (checkIfEnoughDecodedPCMSamples(mDecodedBlocks) || mBuffer.eos || skipBuffering) {
            size_t filledlength = 0;
            if (mFlacDecState.m_bits_per_sample == 24) {
                //decoder gives output in 8_24 pcm format, convert it to 24 bit packed and copy
                filledlength = mDecodedBlocks * mDecoderConfig->getChannels() * sizeof(uint8_t) * 3;
                convertFrom8_24ToPacked((uint8_t *)(outputOffset + outputSize),
                                    (int32_t *) mTmpBufOffset, mDecodedBlocks * mDecoderConfig->getChannels());
            } else {
                filledlength = mDecodedBlocks * mDecoderConfig->getChannels() * (mDecoderConfig->getBitDepth() / 8);
                memcpy(outputOffset + outputSize, mTmpBufOffset, filledlength);
            }

            QLOGV("%s: decoder output size = %zu bytes", __func__, filledlength);
            mDecodedBlocks = 0;
            mIsDecodeSuccess = true;
            outputSize += filledlength;
        }
        if (skipBuffering) {
            if (isBufferFull(blockSize)) {
                QLOGW("This is error case, blocksize %d is not supported", blockSize);
            } else {
                // Copy the remaining data as it was skipped because buffer was full
                // Interleave the output from decoder for multichannel clips.
                if (mFlacDecState.m_bits_per_sample == 24) {
                    interleaveData<uint32_t>(blockSize);
                } else {
                    interleaveData<uint16_t>(blockSize);
                }
            }
            skipBuffering = false;
        }

        uint32_t bytesRemain = mBuffer.i32WritePtr - mBuffer.i32ReadPtr;
        // Not enough data to decode post EOS
        if (mEos && bytesRemain && !enoughDataAvailable(&mBuffer)) {
            outputSize += decodeEOSFrame(bytesRemain, availLength, outputOffset, outputSize);
        }

        if (!outputSize) {
            flags |= BufFlag::EMPTY;
            mNonEmptyOutputGenerated = false;
        } else {
            outBuf->setTimestamp(mFrameStartTimestamp);
        }
        outBuf->setInputIndex(inputIndex);
        outBuf->setFlags(flags);
        outBuf->linear().setRange(0, outputSize);
        std::shared_ptr<Work> completedWork = std::make_shared<Work>(inBuf, outBuf);
        mCompletedWorks[inputIndex] = completedWork;

        if (!enoughDataAvailable(&mBuffer) && outputDeficit > 0) {
            mPendingOutputs.erase(mPendingOutputs.begin());
            mPendingInputs.erase(mPendingInputs.begin());
            outputDeficit--;
        }
        if (!outputDeficit && outputSize) {
            mNonEmptyOutputGenerated = true;
        }
    }

    // Return completed works
    for (auto &work: mCompletedWorks) {
        onOutputDone(work.second->getOutput());
        onInputDone(work.second->getInput());
    }
    mCompletedWorks.clear();

    return QC2_OK;
}

QC2Status QC2AudioSwFlacDec::processFrameByFrame(
    const std::shared_ptr<QC2Buffer> &input,
    std::shared_ptr<QC2Buffer> &output) {
    uint32_t blockSize = 0, usedBitstream = 0, availLength = 0;
    QC2Status retVal = QC2_OK;
    size_t outputSize = 0;
    bool isEosInput = false;
    bool isEmptyEosInput = false;

    stFLACDec *pstFLACDec = (stFLACDec *)(mFlacDecState.m_pFlacDecoder);
    stMetaDataStrmInfo *pstMetaDataStrmInfo =
        (stMetaDataStrmInfo *)(&pstFLACDec->MetaDataBlocks.MetaDataStrmInfo);

    if (mSignalledError) {
        QLOGE("Flac decoder in error state");
        return QC2_ERROR;
    }
    availLength = input->linear().size();
    QLOGV("%s: Received input: size %d offset %d flags %" PRIu64
          " index %" PRIu64,
          __func__, availLength, input->linear().offset(), input->flags(),
          input->inputIndex());

    // Get input buffer's data offset
    std::unique_ptr<QC2Buffer::Mapping> inBufMappingInfo =
        input->linear().map();
    if (inBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto inBufBasePtr = inBufMappingInfo->baseRW();
    if (inBufBasePtr == nullptr) {
        QLOGE("%s: input base ptr is null for input buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *inBufDataPtr = inBufBasePtr + input->linear().offset();

    if (input->flags() & BufFlag::EOS) {
        isEosInput = true;
        QLOGD("%s: Parser reported EOS", __func__);
        if (!availLength) {
            isEmptyEosInput = true;
        }
    }

    if (input->flags() & BufFlag::CODEC_CONFIG) {
        if (!parseMetaData(inBufDataPtr, availLength)) {
            QLOGE("%s parseMetaData error", __func__);
            mSignalledError = true;
            return QC2_ERROR;
        }
        if (mCodecConfigChanged) {
            retVal = initFlacDecoder();
            if (retVal != QC2_OK) {
                QLOGE("Failed to initialize Flac decoder");
                return retVal;
            }
            mCodecConfigChanged = false;
        }
        output->setFlags(input->flags());
        output->linear().setRange(0, 0);
        onOutputDone(output);
        onInputDone(input);
        return QC2_OK;
    }

    // Get output buffer's data offset to fill decode data
    std::unique_ptr<QC2Buffer::Mapping> outBufMappingInfo =
        output->linear().map();
    if (outBufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto outBufBasePtr = outBufMappingInfo->baseRW();
    if (outBufBasePtr == nullptr) {
        QLOGE("%s: output base ptr is null for input buffer", __func__);
        return QC2_NO_MEMORY;
    }
    uint8_t *outWriteBufDataPtr = outBufBasePtr + output->linear().offset();

    QLOGV("%s: Encoded data bytes in input buffer: %d", __func__, availLength);
    eFLACDec_Result decStatus = FLACDEC_SUCCESS;
    uint32_t flacOutputBufSize = sFlacMaxOutBufSize;
    if (!isEmptyEosInput) {
        usedBitstream = 0;
        decStatus =
            (*mProcessData)((void *)&mFlacDecState, inBufDataPtr, availLength,
                            (void *)mOutBufOffset, &flacOutputBufSize,
                            &usedBitstream, &blockSize);
        mDecoderConfig->setChannels(pstMetaDataStrmInfo->i32NumChannels);
        mDecoderConfig->setSampleRate(pstMetaDataStrmInfo->i32SampleRate);

        QLOGV(
            "%s: Decode Frame : Decoder status=%d, flacOutputBufSize=%d, "
            "usedBitstream=%d blockSize %d",
            __func__, decStatus, flacOutputBufSize, usedBitstream, blockSize);
    }

    if ((decStatus == FLACDEC_SUCCESS || decStatus == FLACDEC_EOF) &&
        !isEmptyEosInput) {
        // Decoding is done
        QLOGV("%s: Successful decode!", __func__);
        // process decoded data from decoder
        if (mFlacDecState.m_bits_per_sample == 24) {
            outputSize = processDataFromDecoder<uint32_t>(
                outWriteBufDataPtr, mOutBufOffset, flacOutputBufSize,
                blockSize);
        } else {
            outputSize = processDataFromDecoder<uint16_t>(
                outWriteBufDataPtr, mOutBufOffset, flacOutputBufSize,
                blockSize);
        }
        QLOGV("%s: decoder output size = %zu bytes", __func__, outputSize);
    } else if ((decStatus == FLACDEC_FAIL) ||
               (decStatus == FLACDEC_METADATA_NOT_FOUND)) {
        QLOGE("%s: Erroneous decode!", __func__);
    } else if (decStatus == FLACDEC_ERROR_CODE_NEEDS_MORE_DATA) {
        QLOGW("%s: Not enough data to decode!", __func__);
    }

    output->setFlags(input->flags());
    output->linear().setRange(0, outputSize);
    onOutputDone(output);
    onInputDone(input);
    return QC2_OK;
}

template <typename T>
size_t QC2AudioSwFlacDec::processDataFromDecoder(uint8_t *dest, uint8_t *src,
                                                 size_t srcLength,
                                                 uint32_t blockSize) {
    // Interleave the output from decoder for multichannel clips.
    T *tmpSrc = (T *)src;
    T *tmpDest = (T *)mTmpBufOffset;

    uint32_t channels = mDecoderConfig->getChannels();
    QLOGV("%s: non-interleaved decoded data got from decoder: %zu", __func__, srcLength);
    // Don't interleave for mono channel
    if (channels > 1) {
        for (uint32_t k = 0; k < blockSize; k++) {
            for (uint32_t i = k, j = channels * k; i < blockSize * channels;
                 i += blockSize, j++) {
                tmpDest[j] = tmpSrc[i];
            }
        }
        // change src since we interleaved data
        src = mTmpBufOffset;
    }
    size_t decodeLength =
        blockSize * channels * (mDecoderConfig->getBitDepth() / 8);

    if (mFlacDecState.m_bits_per_sample == 24) {
        // decoder gives output in 8_24 pcm format, convert it to 24 bit packed
        // and copy
        uint32_t numSamples = blockSize * mDecoderConfig->getChannels();
        convertFrom8_24ToPacked(dest, (int32_t *)src, numSamples);
        decodeLength =
            blockSize * mDecoderConfig->getChannels() * sizeof(uint8_t) * 3;
    } else {
        memcpy(dest, src, decodeLength);
    }

    return decodeLength;
}

QC2Status QC2AudioSwFlacDec::reconfigureComponent() {

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

    // Update min frame size
    auto minFrameSize = mDecoderConfig->getMinFrameSize();
    if (minFrameSize != mParamIntfMap[kParamIndexAudioMinFrameBytes]) {
        auto minFrameSizeParam = std::make_shared<C2AudioMinFrameBytesInfo::input>(0, minFrameSize);
        paramUpdatesOnInterface.push_back(minFrameSizeParam);
        mParamIntfMap[kParamIndexAudioMinFrameBytes] = minFrameSize;
    }

    // Update max frame size
    auto maxFrameSize = mDecoderConfig->getMaxFrameSize();
    if (maxFrameSize != mParamIntfMap[kParamIndexAudioMaxFrameBytes]) {
        auto maxFrameSizeParam = std::make_shared<C2AudioMaxFrameBytesInfo::input>(0, maxFrameSize);
        paramUpdatesOnInterface.push_back(maxFrameSizeParam);
        mParamIntfMap[kParamIndexAudioMaxFrameBytes] = maxFrameSize;
    }

    // Update min block size
    auto minBlockSize = mDecoderConfig->getMinBlkSize();
    if (minBlockSize != mParamIntfMap[kParamIndexAudioMinBlockSize]) {
        auto minBlockSizeParam = std::make_shared<C2AudioMinBlockSizeInfo::input>(0, minBlockSize);
        paramUpdatesOnInterface.push_back(minBlockSizeParam);
        mParamIntfMap[kParamIndexAudioMinBlockSize] = minBlockSize;
    }

    // Update max block size
    auto maxBlockSize = mDecoderConfig->getMaxBlkSize();
    if (maxBlockSize != mParamIntfMap[kParamIndexAudioMaxBlockSize]) {
        auto maxBlockSizeParam = std::make_shared<C2AudioMaxBlockSizeInfo::input>(0, maxBlockSize);
        paramUpdatesOnInterface.push_back(maxBlockSizeParam);
        mParamIntfMap[kParamIndexAudioMaxBlockSize] = maxBlockSize;
    }

    onConfigUpdate(paramUpdatesOnInterface);

    return QC2_OK;
}

bool operator!=(const FLACDec_ParserInfo& pInfo, const FLACDec_ParserInfo& csdInfo) {
    return std::tie(pInfo.i32SampleRate, pInfo.i32NumChannels, pInfo.i32BitsPerSample) !=
               std::tie(csdInfo.i32SampleRate, csdInfo.i32NumChannels, csdInfo.i32BitsPerSample);
}

void QC2AudioSwFlacDec::setStreamInfo(uint8_t* buffer, int offset) {
    mCsdInfo.i32MinBlkSize = (uint16)((buffer[offset]<<8)|(buffer[offset+1]));
    offset += 2;

    mCsdInfo.i32MaxBlkSize = (uint16)((buffer[offset]<<8)|(buffer[offset+1]));
    offset += 2;

    mCsdInfo.i32MinFrmSize = (uint32)((buffer[offset]<<16)|(buffer[offset+1] <<8)|
                            (buffer[offset+2]));
    offset += 3;

    mCsdInfo.i32MaxFrmSize = (uint32)((buffer[offset]<<16)|(buffer[offset+1]<<8)|
                            (buffer[offset+2]));
    offset += 3;

    mCsdInfo.i32SampleRate = (uint32)((buffer[offset]<<16)|(buffer[offset+1]<<8)|
                            (buffer[offset+2]));
    mCsdInfo.i32SampleRate = (uint32)((mCsdInfo.i32SampleRate & 0xFFFFF0)>>4);
    mCsdInfo.i32NumChannels = (uint8)(((buffer[offset+2] & 0x0E) >> 1) + 1);
    mCsdInfo.i32BitsPerSample = (uint8)((((buffer[offset + 2] & 0x1) << 4) |
           ((buffer[offset + 3] & 0xf0) >> 4)) + 1 );
    offset += 3;
}

bool QC2AudioSwFlacDec::parseMetaData(uint8_t* buffer, size_t len) {
    bool status = false;
    int offset = 0;
    uint32_t blockSize = 0;
    if (!memcmp(buffer, sFlacSignBytes, sFlacSignSize)) {
        status = true;
        offset += sFlacSignSize;
    } else {
        return status;
    }

    while (offset <= len - sFlacSignSize) {
        uint8_t blockType = buffer[offset] & 0x7F;
        blockSize = (uint32)((((uint32)buffer[offset+1]) << 16) |
                                 (((uint32)buffer[offset+2]) << 8) |
                                 (((uint32)buffer[offset+3])));
        offset += sFlacSignSize;
        if (sFlacMDBlkStreamInfo == blockType && (offset + blockSize) <= len)
            setStreamInfo(buffer, offset);
        offset += blockSize;
    }

    if (mParserInfoToPass != mCsdInfo) {
        mCodecConfigChanged = true;
        mParserInfoToPass.i32SampleRate = mCsdInfo.i32SampleRate;
        mParserInfoToPass.i32NumChannels = mCsdInfo.i32NumChannels;
        mParserInfoToPass.i32BitsPerSample = mCsdInfo.i32BitsPerSample;
        mParserInfoToPass.i32MinBlkSize = mCsdInfo.i32MinBlkSize;
        mParserInfoToPass.i32MaxBlkSize = mCsdInfo.i32MaxBlkSize;
        mParserInfoToPass.i32MinFrmSize = mCsdInfo.i32MinFrmSize;
        mParserInfoToPass.i32MaxFrmSize = mCsdInfo.i32MaxFrmSize;
    }
    return status;
}

int32_t QC2AudioSwFlacDec::enoughDataAvailable(flacByteStreamBuffer *pByteBuffer) {
    uint32_t bytesRemain;

    /* Check if data from parser is needed or not */
    bytesRemain = pByteBuffer->i32WritePtr - pByteBuffer->i32ReadPtr;
    if (((bytesRemain >= sFlacThreshold) || (pByteBuffer->eos)) && (!pByteBuffer->error)) {
        return 1;
    }

    return 0;
}

int32_t QC2AudioSwFlacDec::updateInputBitstream(flacByteStreamBuffer *pByteBuffer,
                                                void * bitstream, int32_t inSize) {
    uint32_t i, bytesRemain;
    uint8_t *bitsbuffer = (uint8_t*) bitstream;

    bytesRemain = pByteBuffer->i32WritePtr - pByteBuffer->i32ReadPtr;

    QLOGV("updateInputBitstream: bytesRemain : %d , InData : %d, \
             pByteBuffer->error : %d, pByteBuffer->eos: %d",
           bytesRemain, inSize, pByteBuffer->error, pByteBuffer->eos);
    if ((bytesRemain >= sFlacThreshold || pByteBuffer->eos || inSize == 0) &&
           !pByteBuffer->error) {
        pByteBuffer->error = 0;
        return 0;
    }
    pByteBuffer->error = 0;

    /* First copy the un-decoded bitstream to start of buffer */
    for (i = 0; i < bytesRemain; i++) {
        pByteBuffer->ui8TempBuf[i] = pByteBuffer->ui8TempBuf[pByteBuffer->i32ReadPtr+i];
    }
    pByteBuffer->i32ReadPtr = 0;
    pByteBuffer->i32WritePtr = bytesRemain;

    uint32_t sizeRemaining = pByteBuffer->i32BufferSize - pByteBuffer->i32WritePtr;
    if (inSize > (int32_t)sizeRemaining) {
        // report error for now?
        QLOGE("inSize (%d) > avail size (%d)", inSize, sizeRemaining);
        return -1;
    } else {
        memcpy(&pByteBuffer->ui8TempBuf[pByteBuffer->i32WritePtr], bitsbuffer, inSize);
        pByteBuffer->i32WritePtr += inSize;
    }

    return 1;
}

int32_t QC2AudioSwFlacDec::updatePointers(flacByteStreamBuffer *pByteBuffer,
                                          uint32_t readBytes, int32_t result) {
    pByteBuffer->i32ReadPtr += readBytes;
    if ((result == FLACDEC_SUCCESS) || (result == FLACDEC_EOF)) {
        QLOGV("QC2AudioSwFlacDec: Successful decode!");
        return 1;
    } else if ((result == FLACDEC_FAIL) || (result == FLACDEC_METADATA_NOT_FOUND)) {
        QLOGE("QC2AudioSwFlacDec: Erroneous decode!");
    } else if (result == FLACDEC_ERROR_CODE_NEEDS_MORE_DATA) {
        QLOGV("QC2AudioSwFlacDec: Not enough data to decode!");
    }

    /* Reset internal buffering when running into scenarios:
     * 1) erroneous decoding without consuming any input frame,
     *    and available internal buffer cannot accomodate any incoming buffer from parser
     * 2) the entire data contains erroneous data and no sync word has been found
     */
    if (((int32_t)(mBuffer.i32BufferSize - mBuffer.i32WritePtr) < sFlacParserBufSize)
                && (readBytes == 0)) {
        QLOGE("%s: erroneous decode, no bytes read by decoder", __func__);
        pByteBuffer->i32WritePtr = 0;
        pByteBuffer->i32ReadPtr = 0;
    }

    if ((pByteBuffer->i32WritePtr - (pByteBuffer->i32ReadPtr)) >= pByteBuffer->i32BufferSize) {
        QLOGE("%s: erroneous write and read ptrs", __func__);
        pByteBuffer->i32WritePtr = 0;
        pByteBuffer->i32ReadPtr = 0;
    }
    pByteBuffer->error = 1;

    return 0;
}

bool QC2AudioSwFlacDec::checkIfEnoughDecodedPCMSamples(int64_t decodedBlocks) {
     if ((sFlacOutBufThresholdMsec <=
             (uint64_t)((decodedBlocks * 1000LL) / mDecoderConfig->getSampleRate())) ||
         (decodedBlocks * (mDecoderConfig->getBitDepth() / 8) *
              mDecoderConfig->getChannels() >= sFlacMaxOutBufSize))
          return true;
     else {
          QLOGV("Not enough decoded data %" PRId64, decodedBlocks);
          return false;
     }
}

inline bool QC2AudioSwFlacDec::isBufferFull(uint32_t blockSize) {
      return (((mDecodedBlocks + blockSize) * mDecoderConfig->getChannels() *
                (mDecoderConfig->getBitDepth() / 8)) > sFlacMaxOutBufSize ? true : false);
}

void QC2AudioSwFlacDec::convertFrom8_24ToPacked(uint8_t *dest, int32_t *src, uint32_t numSamples) {
    const int32_t maxPos = 0x7fffff;
    const int32_t minNeg = -0x800000;
    while (numSamples) {
        int32_t val = *src++;
        if (val < minNeg) {
            val = minNeg;
        } else if (val > maxPos) {
            val = maxPos;
        }
        *dest++ = val;
        *dest++ = val >> 8;
        *dest++ = val >> 16;
        numSamples--;
    }
}

template<typename T>
void QC2AudioSwFlacDec::interleaveData(uint32_t blockSize) {
    T *ptr = (T *) mOutBufOffset;
    uint32_t channels = mDecoderConfig->getChannels();
    T *ptr1 = (T *) mTmpBufOffset + mDecodedBlocks * channels;

    mDecodedBlocks += blockSize;
    if (channels > 1) {
        for (uint32_t k = 0; k < blockSize; k++) {
            for (uint32_t i = k, j = channels * k;
                    i < blockSize * channels;
                    i += blockSize, j++) {
                ptr1[j] = ptr[i];
            }
        }
    } else {
        memcpy(ptr1, mOutBufOffset, blockSize * channels * (mDecoderConfig->getBitDepth() / 8));
    }
}

size_t QC2AudioSwFlacDec::decodeEOSFrame(uint32_t bytesRemain, uint32_t availLength,
                                        uint8_t *outputOffset, size_t outputSize) {
    size_t bufSize = sFlacThreshold - bytesRemain;
    uint8_t zerosBuffer[bufSize];
    uint32_t usedBitstream = 0;
    uint32_t blockSize = 0;
    int status = 0;
    uint32 flacOutputBufSize = sFlacMaxOutBufSize;

    memset(zerosBuffer, 0, bufSize);
    updateInputBitstream(&mBuffer, zerosBuffer, bufSize);

    status = (*mProcessData)(&mFlacDecState,
                             &mBuffer.ui8TempBuf[mBuffer.i32ReadPtr],
                             availLength,
                             mOutBufOffset,
                             &flacOutputBufSize,
                             &usedBitstream,
                             &blockSize);

    if (updatePointers(&mBuffer, usedBitstream, status)) {
        if (mFlacDecState.m_bits_per_sample == 24) {
            interleaveData<uint32_t>(blockSize);
        } else {
            interleaveData<uint16_t>(blockSize);
        }
    }
    size_t filledlength = 0;
    if (checkIfEnoughDecodedPCMSamples(mDecodedBlocks)) {
        if (mFlacDecState.m_bits_per_sample == 24) {
            //decoder gives output in 8_24 pcm format, convert it to 24 bit packed and copy
            filledlength = mDecodedBlocks * mDecoderConfig->getChannels() * sizeof(uint8_t) * 3;
            convertFrom8_24ToPacked((uint8_t *)(outputOffset + outputSize),
                        (int32_t *) mTmpBufOffset, mDecodedBlocks * mDecoderConfig->getChannels());
        } else {
            filledlength = mDecodedBlocks * mDecoderConfig->getChannels() * (mDecoderConfig->getBitDepth() / 8);
            memcpy(outputOffset + outputSize, mTmpBufOffset, filledlength);
        }
    }

    return filledlength;
}

}   // namespace qc2

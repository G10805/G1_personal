/*
 **************************************************************************************************
 * Copyright (c) 2015-2016, 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
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

#ifndef _QC2_AUDIO_SW_FLAC_DEC_H_
#define _QC2_AUDIO_SW_FLAC_DEC_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>

#include "FLACDec_Wrapper.h"
#include "FLACDec_BitStream.h"
#include "FLACDec_MetaData.h"
#include "FLACDec_Struct.h"

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2FlacCodecConfig.h"

#include <dlfcn.h>


namespace qc2audio {

class QC2AudioSwFlacDec : public QC2AudioSwCodec {

public:
    QC2AudioSwFlacDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwFlacDec() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status configure(std::vector<std::unique_ptr<C2Param>> config) override;

protected:
    QC2Status onStart() override;
    QC2Status onStop() override;
    QC2Status onInit() override;
    QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                      std::shared_ptr<QC2Buffer>& output) override;
    QC2Status reconfigureComponent();

    // Parameter values set on ComponentInterface
    std::unordered_map<C2Param::type_index_t, uint32_t> mParamIntfMap;

    std::shared_ptr<QC2FlacCodecConfig> mDecoderConfig;
    void *mFLACLibHandle;

    typedef struct {
        uint32_t i32BufferSize;
        uint32_t i32ReadPtr;
        uint32_t i32WritePtr;
        uint8_t *ui8TempBuf;
        int32_t eos;
        int32_t error;
    } flacByteStreamBuffer;
    flacByteStreamBuffer mBuffer;

    class Work {
    public:
        Work(std::shared_ptr<QC2Buffer> inBuf,
            std::shared_ptr<QC2Buffer>outBuf) : mInput(inBuf), mOutput(outBuf) { };
        void setInput(std::shared_ptr<QC2Buffer> inBuf) {
            mInput = inBuf;
        }
        void setOutput(std::shared_ptr<QC2Buffer> outBuf) {
            mOutput = outBuf;
        }
        std::shared_ptr<QC2Buffer> getInput() {
            return mInput;
        }
        std::shared_ptr<QC2Buffer> getOutput() {
            return mOutput;
        }
    private:
        std::shared_ptr<QC2Buffer> mInput;
        std::shared_ptr<QC2Buffer> mOutput;
    };

    bool mIsDecodeSuccess;
    bool mEos;
    bool mInitStatus;
    int64_t mDecodedBlocks;
    bool mSignalledError;
    uint64_t mFrameStartTimestamp;
    bool mNonEmptyOutputGenerated;
    bool mCodecConfigChanged;
    bool mIsFrameByFrameDecode;
    std::vector<std::shared_ptr<QC2Buffer>> mPendingInputs;
    std::vector<std::shared_ptr<QC2Buffer>> mPendingOutputs;
    std::unordered_map<uint64_t, std::shared_ptr<Work>> mCompletedWorks;

    CFlacDecState mFlacDecState;
    FLACDec_ParserInfo mParserInfoToPass;
    FLACDec_ParserInfo mCsdInfo;

    typedef void (*DecoderInit) (CFlacDecState* pFlacDecState, int* nRes, int bitWidth);
    typedef eFLACDec_Result (*DecoderLib_Process) (void *pFlacDecState, uint8* pInBitStream,
                                       uint32 nActualDataLen, void *pOutSamples,
                                       uint32* uFlacOutputBufSize, uint32* usedBitstream,
                                       uint32* blockSize);
    typedef void (*DecoderEnd) (void* pFlacDec);

    DecoderInit mDecoderInit;
    DecoderLib_Process mProcessData;
    DecoderEnd mDecoderEnd;

    std::shared_ptr<QC2LinearBufferPool> mScratchPool = nullptr;
    std::shared_ptr<QC2Buffer> mOutBuf;
    std::unique_ptr<QC2Buffer::Mapping> mOutBufMapping;
    uint8_t *mOutBufOffset;

    std::shared_ptr<QC2Buffer> mTmpBuf;
    std::unique_ptr<QC2Buffer::Mapping> mTmpBufMapping;
    uint8_t *mTmpBufOffset;

    std::shared_ptr<QC2Buffer> mByteStreamBuf;
    std::unique_ptr<QC2Buffer::Mapping> mByteStreamBufMapping;

    uint8_t * allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
                             std::unique_ptr<QC2Buffer::Mapping>& bufMapping,
                             uint32_t bufSize);
    QC2Status initFlacDecoder();
    void updateDecoderConfig();
    int32_t enoughDataAvailable(flacByteStreamBuffer *pByteBuffer);
    int32_t updateInputBitstream(flacByteStreamBuffer *pByteBuffer, void * bitstream, int32_t inSize);
    int32_t updatePointers(flacByteStreamBuffer *pByteBuffer, uint32_t readBytes, int32_t result);
    bool checkIfEnoughDecodedPCMSamples(int64_t decodedPcmBlocks);
    inline bool isBufferFull(uint32_t blockSize);
    void convertFrom8_24ToPacked(uint8_t *dest, int32_t *src, uint32_t numSamples);
    template<typename T> void interleaveData(uint32_t blockSize);
    QC2Status processByThreshold(const std::shared_ptr<QC2Buffer> &input,
                                 std::shared_ptr<QC2Buffer> &output);
    QC2Status processFrameByFrame(const std::shared_ptr<QC2Buffer> &input,
                                  std::shared_ptr<QC2Buffer> &output);
    template <typename T>
    size_t processDataFromDecoder(uint8_t *dest, uint8_t *src, size_t srcLength,
                                  uint32_t blockSize);
    void setStreamInfo(uint8_t* buffer, int offset);
    bool parseMetaData(uint8_t *buffer, size_t inBufferLen);
    size_t decodeEOSFrame(uint32_t bytesRemain, uint32_t availLength,
                          uint8_t *outputOffset, size_t outputSize);
};

};  // namespace qc2

#endif  // _QC2_AUDIO_SW_FLAC_DEC_H_

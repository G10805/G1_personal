/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_APE_CODEC_H_
#define _QC2_AUDIO_SW_APE_CODEC_H_

#include <unordered_map>

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2ApeCodecConfig.h"
#include "QC2AudioSwCommonDefs.h"

#include <dlfcn.h>

namespace qc2audio {

class QC2AudioSwApeDec : public QC2AudioSwCodec {

public:
    QC2AudioSwApeDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwApeDec() = default;

    QC2Status configure(const C2Param& param) override;

protected:
    std::shared_ptr<QC2ApeDecConfig> mDecoderConfig;
    void* mAPELibHandle;
    void* mAPEDecHandle;
    // Parameter values set on ComponentInterface
    std::unordered_map<C2Param::type_index_t, uint32_t> mParamIntfMap;

    QC2Status onStart() override;
    QC2Status onStop() override;
    QC2Status onInit() override;
    QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                      std::shared_ptr<QC2Buffer>& output) override;
    QC2Status reconfigureComponent();

    typedef void* (*DecoderInit) (void* mApeDecHandle, DecErr_t* result, void* media_fmt_blk,
        GenericDecMediaFmt_t* reqMediaType, const InterfaceTypes_t initType);
    typedef void (*DecoderLib_Process) (void* mApeDecHandle, DecErr_t* result,
        AudioBuf_t* inBuffer, AudioBuf_t* outBuffer, GenericDecMediaFmt_t* outMediaType);
    DecoderInit mDecoderInit;
    DecoderLib_Process mProcessData;

    typedef struct {
        int32_t i32BufferSize;
        int32_t i32ReadPtr;
        int32_t i32WritePtr;
        uint8_t *ui8TempBuf;
        int32_t eos;
        int32_t error;
    } byteStreamBuffer;
    byteStreamBuffer mBuffer;

    std::shared_ptr<QC2LinearBufferPool> mScratchPool = nullptr;
    std::shared_ptr<QC2Buffer> mByteStreamBuf;
    std::unique_ptr<QC2Buffer::Mapping> mByteStreamBufMapping;

    uint8_t * allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
                             std::unique_ptr<QC2Buffer::Mapping>& bufMapping,
                             uint32_t bufSize);
    int32_t enoughDataAvailable(byteStreamBuffer *pByteBuffer);
    int32_t updateInputBitstream(byteStreamBuffer *pByteBuffer, void * bitstream, int32_t inSize);
    int32_t updatePointers(byteStreamBuffer *pByteBuffer, uint32_t readBytes, int32_t result);
    int32_t allocByteStreamBuffer(uint32_t bufferingSize);
};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_APE_CODEC_H_

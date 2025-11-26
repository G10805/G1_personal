/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
 */
#ifndef _QC2_AUDIO_SW_DSD_CODEC_H_
#define _QC2_AUDIO_SW_DSD_CODEC_H_

#include <dlfcn.h>

#include <unordered_map>

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2AudioSwCommonDefs.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2DsdCodecConfig.h"

namespace qc2audio {

class QC2AudioSwDsdDec : public QC2AudioSwCodec {
   public:
    QC2AudioSwDsdDec(const std::string &codecName, uint32_t sessionId,
                     const std::string &instanceName, const std::string &mime,
                     uint32_t variant, std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwDsdDec() = default;

    QC2Status configure(const C2Param &param) override;

   protected:
    std::shared_ptr<QC2DsdDecConfig> mDecoderConfig;
    void *mDSDLibHandle;
    void *mDSDDecHandle;
    // Parameter values set on ComponentInterface
    std::unordered_map<C2Param::type_index_t, uint32_t> mParamIntfMap;

    QC2Status onStart() override;
    QC2Status onStop() override;
    QC2Status onInit() override;
    QC2Status onFlush() override;
    QC2Status process(const std::shared_ptr<QC2Buffer> &input,
                      std::shared_ptr<QC2Buffer> &output) override;

    QC2Status createDsdDecoder();
    QC2Status reconfigureComponent();

    typedef void *(*DecoderInit)(void *mDSDDecHandle, DecErr_t *result,
                                 void *media_fmt_blk,
                                 GenericDecMediaFmt_t *reqMediaType,
                                 const InterfaceTypes_t initType);
    typedef void (*DecoderLib_Process)(void *mDSDDecHandle, DecErr_t *result,
                                       AudioBuf_t *inBuffer,
                                       AudioBuf_t *outBuffer,
                                       GenericDecMediaFmt_t *outMediaType);

    DecoderInit mDecoderInit;
    DecoderLib_Process mProcessData;
};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_DSD_CODEC_H_

/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_ALAC_CODEC_H_
#define _QC2_AUDIO_SW_ALAC_CODEC_H_

#include <unordered_map>

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2AlacCodecConfig.h"
#include "QC2ChannelReorder.h"
#include "QC2AudioSwCommonDefs.h"

#include <dlfcn.h>

namespace qc2audio {

class QC2AudioSwAlacDec : public QC2AudioSwCodec {

public:
    QC2AudioSwAlacDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwAlacDec() = default;

    QC2Status configure(const C2Param& param) override;

protected:
    std::shared_ptr<QC2AlacDecConfig> mDecoderConfig;
    void* mALACLibHandle;
    void* mALACDecHandle;
    // Parameter values set on ComponentInterface
    std::unordered_map<C2Param::type_index_t, uint32_t> mParamIntfMap;

    QC2Status onStart() override;
    QC2Status onStop() override;
    QC2Status onInit() override;
    QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                      std::shared_ptr<QC2Buffer>& output) override;
    QC2Status reconfigureComponent();

    typedef void* (*DecoderInit) (void* mAlacDecHandle, DecErr_t* result, void* media_fmt_blk,
        GenericDecMediaFmt_t* reqMediaType, const InterfaceTypes_t initType);
    typedef void (*DecoderLib_Process) (void* mAlacDecHandle, DecErr_t* result,
        AudioBuf_t* inBuffer, AudioBuf_t* outBuffer, GenericDecMediaFmt_t* outMediaType);
    DecoderInit mDecoderInit;
    DecoderLib_Process mProcessData;
    std::shared_ptr<CHReorder> mCHReorder;
};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_ALAC_CODEC_H_

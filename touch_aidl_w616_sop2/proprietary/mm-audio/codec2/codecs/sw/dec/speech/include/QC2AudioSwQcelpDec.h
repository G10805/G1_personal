/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_QCELP_CODEC_H_
#define _QC2_AUDIO_SW_QCELP_CODEC_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2AudioSwCommonDefs.h"
#include "QC2AudioSwSpeechCommonDec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2SpeechCodecConfig.h"

#include <dlfcn.h>

namespace qc2audio {

class QC2AudioSwQcelpDec : public QC2AudioSwSpeechCommonDec {

public:
    QC2AudioSwQcelpDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwQcelpDec() = default;

    const struct frameInformation sFrmInfo[5] =
                {{0, 1}, {1, 4}, {2, 8}, {3, 17}, {4, 35}};

protected:
    QC2Status onStart() override;
    QC2Status onStop() override;
    QC2Status onInit() override;
    QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                      std::shared_ptr<QC2Buffer>& output) override;

    TTYDecStruct ttyStruct;
    SpeechCodecType getCodecType() override {
        return SpeechCodecType::QCELP;
    }
    typedef void (*DecoderInit) (void *ptrLocalStruct);
    typedef int16_t (*DecoderConfigSize) (void);
    typedef int16_t (*DecoderLib_Process) (void *ptr, int16_t *rxPacket,
            int16_t *decOutBuf, TTYDecStruct *pTTYDecStruct);
    DecoderInit mDecoderInit;
    DecoderConfigSize mGetDecoderConfigSize;
    DecoderLib_Process mProcessData;
};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_QCELP_CODEC_H_

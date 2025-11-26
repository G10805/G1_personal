/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_SPEECH_CODEC_H_
#define _QC2_AUDIO_SW_SPEECH_CODEC_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>

#include "QC2.h"
#include "QC2AudioSwCodec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2CodecCapsHelper.h"
#include "QC2SpeechCodecConfig.h"

#include <dlfcn.h>

namespace qc2audio {

class QC2AudioSwSpeechCommonDec : public QC2AudioSwCodec {

public:
    QC2AudioSwSpeechCommonDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    ~QC2AudioSwSpeechCommonDec() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status configure(std::vector<std::unique_ptr<C2Param>> config) override;

protected:
    std::shared_ptr<QC2SpeechCodecConfig> mDecoderConfig;
    void* mLibHandle;
    std::shared_ptr<std::vector<uint8_t>> mDecStructInst;
    // Parameter values set on ComponentInterface
    std::unordered_map<C2Param::type_index_t, uint32_t> mParamIntfMap;

    virtual QC2Status onStart();
    virtual QC2Status onStop();
    virtual QC2Status onInit();
    virtual QC2Status process(const std::shared_ptr<QC2Buffer>& input,
                              std::shared_ptr<QC2Buffer>& output);
    QC2Status reconfigureComponent();
    // To be implemented by derived speech codecs
    virtual SpeechCodecType getCodecType() = 0;

    struct frameInformation {
        uint8_t rate;
        uint8_t frameLen;
    };
    std::shared_ptr<QC2LinearBufferPool> mScratchPool = nullptr;
    std::shared_ptr<QC2Buffer> mTransBuf;
    std::shared_ptr<QC2Buffer> mResidualBuf;
    std::unique_ptr<QC2Buffer::Mapping> mTransBufMapping;
    std::unique_ptr<QC2Buffer::Mapping> mResidualBufMapping;
    uint8_t *mTransBufStartOffset;
    uint8_t *mResidualBufStartOffset;
    uint8_t *mTransBufDataOffset;
    uint8_t *mResidualBufDataOffset;
    uint32_t mResidualDataLen;
    bool mIsFullFrame;
    int mFrameCount;
    QC2Status transcodeData(uint8_t *srcOffset, uint32_t srcDataLen,
                            uint32_t *srcDataConsumed, uint8_t *rate,
                            const struct frameInformation *g_frmInfo);
    uint8_t* allocateBuffer(std::shared_ptr<QC2Buffer> *buf,
                            std::unique_ptr<QC2Buffer::Mapping>& bufMapping,
                            uint32_t bufSize);
    void byteSwap(uint8_t *startAddr, uint8_t num);
};

};  // namespace qc2audio

#endif  // _QC2_AUDIO_SW_SPEECH_CODEC_H_

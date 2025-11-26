/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_AAC_ENC_H_
#define _QC2AUDIO_AUDIO_HW_AAC_ENC_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>

#include "QC2.h"
#include "QC2AudioHwCodec.h"
#include "QC2CodecPlugin.h"
#include "QC2Constants.h"
#include "QC2CodecCaps.h"
#include "QC2AacCodecConfig.h"

namespace qc2audio {

class QC2AudioHwAacEnc : public QC2AudioHwCodec {

public:
    QC2AudioHwAacEnc(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwAacEnc() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status onStart() override;
    QC2Status onInit() override;
    QC2Status onFlush() override;

    void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) override;

private:
    std::shared_ptr<QC2AacEncConfig> mEncoderConfig;
    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;
    bool mMp4ffHeaderSent;

    void updateConfigParams();
    void aacRecInstallBits(uint8_t *input, uint8_t numBitsReqd,
                           uint32_t value, uint16_t *hdrBitIndex);
    void addMp4ffHeaderVar(uint8_t *aacHeaderMp4ff, uint32_t sampleIndex,
                           uint8_t channelConfig);
    bool isHwAacEncodeSupported();
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_AAC_ENC_H_

/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_APE_DEC_H_
#define _QC2AUDIO_AUDIO_HW_APE_DEC_H_

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
#include "QC2ApeCodecConfig.h"

namespace qc2audio {

class QC2AudioHwApeDec : public QC2AudioHwCodec {

public:
    QC2AudioHwApeDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwApeDec() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status onInit() override;
    QC2Status onStart() override;
    QC2Status onCodecConfig(std::shared_ptr<QC2Buffer> input) override;

private:
    std::shared_ptr<QC2ApeDecConfig> mDecoderConfig;
    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;

    void updateConfigParams(bool *hasCSDChanged);

};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_APE_DEC_H_

/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_HW_WMA_DEC_H_
#define _QC2AUDIO_HW_WMA_DEC_H_

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
#include "QC2WmaCodecConfig.h"

namespace qc2audio {

class QC2AudioHwWmaDec : public QC2AudioHwCodec {

public:
    QC2AudioHwWmaDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwWmaDec() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status onStart() override;
    QC2Status onInit() override;

private:
    std::shared_ptr<QC2WmaDecConfig> mDecoderConfig;
    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;

    void updateConfigParams();
    void setWmaDecConfig(const C2AudioWmaConfig::input *cfg);
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_HW_WMA_DEC_H_

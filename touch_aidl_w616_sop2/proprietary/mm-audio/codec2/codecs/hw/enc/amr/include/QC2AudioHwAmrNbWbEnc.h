/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_AMR_NB_ENC_H_
#define _QC2AUDIO_AUDIO_HW_AMR_NB_ENC_H_

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
#include "QC2AmrCodecConfig.h"

namespace qc2audio {

class QC2AudioHwAmrNbWbEnc : public QC2AudioHwCodec {
 public:

    QC2AudioHwAmrNbWbEnc(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwAmrNbWbEnc() = default;

    QC2Status configure(const C2Param& param) override;

    QC2Status onInit() override;
    QC2Status onStart() override;


 protected:

    std::shared_ptr<QC2AmrCodecConfig> mEncoderConfig;
    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;
    AmrVersion mAmrVersion;

    QC2Status updateConfigParams();
    void logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix);
    void setAmrCodecType(AmrVersion codecType) {mAmrVersion = codecType;}
    AmrVersion getAmrCodecType() {return mAmrVersion;}
    void setEncoderConfig(std::shared_ptr<QC2AmrCodecConfig> config) {
        mEncoderConfig = config;
    }
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_AMR_NB_ENC_H_

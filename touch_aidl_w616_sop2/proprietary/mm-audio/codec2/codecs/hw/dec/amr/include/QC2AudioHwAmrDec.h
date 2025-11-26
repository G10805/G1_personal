/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_AMR_DEC_H_
#define _QC2AUDIO_AUDIO_HW_AMR_DEC_H_

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

class BufferList;

class QC2AudioHwAmrDec : public QC2AudioHwCodec{
 public:

    QC2AudioHwAmrDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwAmrDec() = default;

    QC2Status onInit() override;
    QC2Status configure(const C2Param& param) override;
    virtual QC2Status onStart() = 0;

 protected:

    std::shared_ptr<QC2AmrCodecConfig> mDecoderConfig;

    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;
    AmrVersion mAmrVersion;

    virtual void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) = 0;
    QC2Status updateConfigParams();
    void logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix);
    void setAmrCodecType(AmrVersion codecType) {mAmrVersion = codecType;}
    AmrVersion getAmrCodecType() {return mAmrVersion;}
    void setDecoderConfig(std::shared_ptr<QC2AmrCodecConfig> config) {
        mDecoderConfig = config;
    }
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_AMR_DEC_H_

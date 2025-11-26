/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_AAC_DEC_H_
#define _QC2AUDIO_AUDIO_HW_AAC_DEC_H_

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

class BufferList;

#define DUMPS_ENABLE false

class QC2AudioHwAacDec : public QC2AudioHwCodec{
 public:

    QC2AudioHwAacDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwAacDec() = default;

    QC2Status configure(const C2Param& param) override;

  protected:
    std::shared_ptr<QC2AacDecConfig> mDecoderConfig;
    QC2AudioAacConfig mAacDecoderExtnConfig;
    std::unordered_map<C2Param::type_index_t, uint32_t> mIntfParamValuesMap;

    QC2Status onStart() override;
    QC2Status onCodecConfig(std::shared_ptr<QC2Buffer> input) override;
    void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) override;
    QC2Status onInit() override;

    void updateConfigParams(bool *hasCSDChanged);

    void audaac_extract_bits(uint8_t  *input,
                                uint8_t  num_bits_reqd,
                                uint32_t *out_buf);
    void audaac_extract_adif_header(uint8_t *data,
                                    std::shared_ptr<QC2AacDecConfig>& aac_config);
    int audaac_extract_loas_header(uint8_t *data, uint32_t len);
    QC2Status parseAndExtractAacInfoFromHeader(uint8_t* buf, size_t buf_length);



private:
        uint32_t m_aac_hdr_bit_index = 0;
        uint32_t m_bytes_to_skip = 0;
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_AAC_DEC_H_

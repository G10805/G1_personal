/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#undef LOG_TAG
#define LOG_TAG "QC2AudioSwCodecPlugin"

#include <unordered_map>
#include <memory>
#include <string>
#include <list>
#include <algorithm>

#include "QC2AudioHwCodec.h"
#include "QC2CodecPlugin.h"

namespace qc2audio {

const std::vector<QC2CodecEntry>& GetQC2AudioHwCodecEntries();

struct QC2AudioHwCodecPlugin : public QC2CodecPlugin {
    virtual ~QC2AudioHwCodecPlugin() = default;

    std::string name() override {
        return "qc2_audioHw_codecs";
    }

    uint32_t version() override {
        return 0;
    }

    std::vector<QC2CodecPlugin::Entry> getSupportedCodecs() override;
};

std::vector<QC2CodecPlugin::Entry> QC2AudioHwCodecPlugin::getSupportedCodecs() {
    QLOGI("getSupportedCodecs");
    std::vector<QC2CodecPlugin::Entry> entries;
    auto AudioHwEntries = GetQC2AudioHwCodecEntries();
    for (const auto& v : AudioHwEntries) {
        std::unique_ptr<QC2CodecFactory> factory;
        if (v.mKind == KIND_DECODER) {
            factory = std::unique_ptr<QC2AudioHwDecoderFactory>(
                    new QC2AudioHwDecoderFactory(v.mName, v.mInputMime, v.mNickName, v.mVariant));
        } else {
            factory = std::unique_ptr<QC2AudioHwEncoderFactory>(
                    new QC2AudioHwEncoderFactory(v.mName, v.mOutputMime, v.mNickName, v.mVariant));
        }
        entries.emplace_back(std::make_tuple(v.mName, v, std::move(factory)));
    }
    return entries;
}

extern "C" QC2CodecPlugin * QC2Codec_GetPlugin() {
    return new QC2AudioHwCodecPlugin;
}

};  // namespace qc2audio

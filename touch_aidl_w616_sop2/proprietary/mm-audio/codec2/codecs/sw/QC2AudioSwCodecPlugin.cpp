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

#include "QC2AudioSwCodec.h"
#include "QC2CodecPlugin.h"

namespace qc2audio {

const std::vector<QC2CodecEntry>& GetQC2AudioSwCodecEntries();

struct QC2AudioSwCodecPlugin : public QC2CodecPlugin {
    virtual ~QC2AudioSwCodecPlugin() = default;

    std::string name() override {
        return "qc2_audioSw_codecs";
    }

    uint32_t version() override {
        return 0;
    }

    std::vector<QC2CodecPlugin::Entry> getSupportedCodecs() override;
};

std::vector<QC2CodecPlugin::Entry> QC2AudioSwCodecPlugin::getSupportedCodecs() {
    QLOGI("getSupportedCodecs");
    std::vector<QC2CodecPlugin::Entry> entries;
    auto AudioSwEntries = GetQC2AudioSwCodecEntries();
    for (const auto& v : AudioSwEntries) {
        std::unique_ptr<QC2CodecFactory> factory;
        if (v.mKind == KIND_DECODER) {
            factory = std::unique_ptr<QC2AudioSwDecoderFactory>(
                    new QC2AudioSwDecoderFactory(v.mName, v.mInputMime, v.mNickName, v.mVariant));
        } else {
            factory = std::unique_ptr<QC2AudioSwEncoderFactory>(
                    new QC2AudioSwEncoderFactory(v.mName, v.mOutputMime, v.mNickName, v.mVariant));
        }
        entries.emplace_back(std::make_tuple(v.mName, v, std::move(factory)));
    }
    return entries;
}

extern "C" QC2CodecPlugin * QC2Codec_GetPlugin() {
    return new QC2AudioSwCodecPlugin;
}

};  // namespace qc2audio

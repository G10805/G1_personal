
/*
 **************************************************************************************************
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#undef LOG_TAG
#define LOG_TAG "QC2ColorConvertFilterPlugin"

#include <unordered_map>
#include <memory>
#include <string>
#include <list>
#include <algorithm>
#define HAVE_IMPL
#ifdef HAVE_IMPL
#include "QC2ColorConvertFilter.h"
#else
#include "QC2MockFilter.h"
#endif
#include "QC2CodecPlugin.h"
#include "QC2ColorConvertFilterConfig.h"

namespace qc2 {

using ENTRY = QC2CodecEntry;
using V = QC2CodecVariant;

static QC2ColorConvertEnableInfo enableInfo;
static QC2CodecEntry::PipelinedParamIndices sPipelineIds = {
                                                           .inputPortPipelinedIndex = 0,
                                                           .outputPortPipelinedIndex = 0,
                                                           .enabledIndex = enableInfo.index(),
                                                           };

static std::vector<QC2CodecEntry> sColorConvertFilterEntries = {
    ENTRY({"c2.qti.colorconvert.filter", KIND_FILTER, MimeType::RAW, MimeType::RAW, "ColorConvertFltr",
           V::CAN_PIPELINE, sPipelineIds}),
    ENTRY({"c2.qti.colorconvert.filter.secure", KIND_FILTER, MimeType::RAW, MimeType::RAW, "ColorConvertFltr",
           V::SECURE | V::CAN_PIPELINE, sPipelineIds}),
};

const std::vector<QC2CodecEntry>& GetQC2ColorConvertFilterEntries() {
    return sColorConvertFilterEntries;
}

class QC2ColorConvertFilterFactory : public QC2CodecFactory {
 public:
    virtual ~QC2ColorConvertFilterFactory() = default;

    QC2ColorConvertFilterFactory(const std::string& name, const std::string& mime,
                        const std::string& nickName, uint32_t variant)
        : mName(name), mNickName(nickName), mCompressedMime(mime), mVariant(variant) {
    }

    QC2Status createCodec(
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) override {


        if (codec == nullptr) {
            return QC2_BAD_ARG;
        }
        try {
#ifdef HAVE_IMPL
            std::string instName = mNickName + "_" + std::to_string(sessionId);
            *codec = std::make_unique<QC2ColorConvertFilter>(mName, sessionId, instName, upstreamMime,
                                                    downstreamMime, mVariant, listener);
#else
            *codec = std::make_unique<QC2MockFilter>(mName, MimeType::RAW, KIND_FILTER, false, listener);
            (void)sessionId;
            (void)upstreamMime;
            (void)downstreamMime;
            (void)mVariant;
#endif

        }
        catch(...) {  // Catch all other exceptions
            QLOGE("Caught unknown exception from QC2ColorConvertFilter constructor");
            *codec = nullptr;
            return QC2_ERROR;
        }
        return QC2_OK;
    }

    std::string name() override {
        return "ColorConvertFilter Factory";
    }

 private:
    const std::string mName;
    const std::string mNickName;
    const std::string mCompressedMime;
    const uint32_t mVariant;
};

struct QC2ColorConvertFilterPlugin : public QC2CodecPlugin {
    virtual ~QC2ColorConvertFilterPlugin() = default;

    std::string name() override {
        return "qc2_ColorConvertFilter";
    }

    uint32_t version() override {
        return 0;
    }

    std::vector<QC2CodecPlugin::Entry> getSupportedCodecs() override;
};

std::vector<QC2CodecPlugin::Entry> QC2ColorConvertFilterPlugin::getSupportedCodecs() {
    QLOGI("getSupportedCodecs");
    std::vector<QC2CodecPlugin::Entry> entries;
    auto ColorConvertEntries = GetQC2ColorConvertFilterEntries();
    for (const auto& v : ColorConvertEntries) {
        std::unique_ptr<QC2CodecFactory> factory;
        if (v.mKind == KIND_FILTER) {
            factory = std::unique_ptr<QC2ColorConvertFilterFactory>(
                    new QC2ColorConvertFilterFactory(v.mName, v.mInputMime, v.mNickName, v.mVariant));
            entries.emplace_back(std::make_tuple(v.mName, v, std::move(factory)));
        } else {
            QLOGE("Only KIND_FILTER is supported");
        }
    }
    return entries;
}

extern "C" QC2CodecPlugin * QC2Codec_GetPlugin() {
    return new QC2ColorConvertFilterPlugin;
}

};  // namespace qc2

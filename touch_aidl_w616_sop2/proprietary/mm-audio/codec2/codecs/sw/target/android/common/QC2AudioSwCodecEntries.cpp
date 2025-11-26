/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QC2CodecPlugin.h"

namespace qc2audio {

using ENTRY = QC2CodecEntry;

static QC2CodecEntry::PipelinedParamIndices sPipelineIds;

static std::vector<QC2CodecEntry> sQC2AudioSwCodecEntries = {
    // decoders
    ENTRY({"c2.qti.alac.sw.decoder", KIND_DECODER, MimeType::ALAC, MimeType::RAW,
           "alacSwD", IMPL_SW, sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    ENTRY({"c2.qti.ape.sw.decoder",  KIND_DECODER, MimeType::APE, MimeType::RAW,
           "apeSwD", IMPL_SW,  sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    ENTRY({"c2.qti.evrc.sw.decoder", KIND_DECODER, MimeType::EVRC, MimeType::RAW,
           "evrcSwD", IMPL_SW, sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    ENTRY({"c2.qti.qcelp.sw.decoder", KIND_DECODER, MimeType::QCELP, MimeType::RAW,
           "qcelpSwD", IMPL_SW, sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    ENTRY({"c2.qti.flac.sw.decoder", KIND_DECODER, MimeType::FLAC, MimeType::RAW,
           "flacSwD", IMPL_SW, sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    ENTRY({"c2.qti.dsd.sw.decoder", KIND_DECODER, MimeType::DSD, MimeType::RAW,
           "dsdSwD", IMPL_SW, sPipelineIds, QC2MaxCodecInstances::SW_MAX}),
    // encoders
};

const std::vector<QC2CodecEntry>& GetQC2AudioSwCodecEntries() {
    return sQC2AudioSwCodecEntries;
}

};  // namsespace qc2audio

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

static std::vector<QC2CodecEntry> sQC2AudioHwCodecEntries = {
    // decoders
    ENTRY({"c2.qti.aac.hw.decoder",          KIND_DECODER, MimeType::AAC,          MimeType::RAW,  "aacHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.aac.hw.decoder",          KIND_DECODER, MimeType::AAC_ADTS,     MimeType::RAW,  "aacHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.alac.hw.decoder",         KIND_DECODER, MimeType::ALAC,         MimeType::RAW,  "alacHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.amrwbplus.hw.decoder",    KIND_DECODER, MimeType::AMR_WB_PLUS,  MimeType::RAW,  "amrWbPlusHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.ape.hw.decoder",          KIND_DECODER, MimeType::APE,          MimeType::RAW,  "apeHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.wma.hw.decoder",          KIND_DECODER, MimeType::WMA,          MimeType::RAW,  "wmaHwD", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),

    // encoders
    ENTRY({"c2.qti.aac.hw.encoder",          KIND_ENCODER, MimeType::RAW,          MimeType::AAC,         "aacHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.aac.hw.encoder",          KIND_ENCODER, MimeType::RAW,          MimeType::AAC_ADTS,    "aacAdtsHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.amrnb.hw.encoder",        KIND_ENCODER, MimeType::RAW,          MimeType::AMR_NB,      "amrNbHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.amrwb.hw.encoder",        KIND_ENCODER, MimeType::RAW,          MimeType::AMR_WB,      "amrWbHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.evrc.hw.encoder",         KIND_ENCODER, MimeType::RAW,          MimeType::EVRC,        "evrcHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
    ENTRY({"c2.qti.qcelp.hw.encoder",        KIND_ENCODER, MimeType::RAW,          MimeType::QCELP,       "qcelpHwE", IMPL_DSP, sPipelineIds, QC2MaxCodecInstances::HW_MAX}),
};

const std::vector<QC2CodecEntry>& GetQC2AudioHwCodecEntries() {
    return sQC2AudioHwCodecEntries;
}

};  // namsespace qc2

/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QC2AudioHwElite.h"
#include <cmath>

#undef LOG_TAG
#define LOG_TAG "QC2AudioHwElite"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

QC2AudioHwElite::QC2AudioHwElite() {
}

QC2Status QC2AudioHwElite::flush(uint64_t* hw_session_handle) {
    QLOGD_INST("sending FLUSH command");
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::init(Notifier* notifier) {
    (void)notifier;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::open_session(QC2AudioCommonCodecConfig& inputMediaFmt,
                                QC2AudioCommonCodecConfig& outputMediaFmt,
                                uint64_t** hw_session_handle) {
    (void)inputMediaFmt;
    (void)outputMediaFmt;
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::start_session(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}
QC2Status QC2AudioHwElite::stop_session(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::write_data(uint64_t* hw_session_handle, std::shared_ptr<QC2AudioHwBuffer>) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::queue_read_buffer(uint64_t* hw_session_handle, std::shared_ptr<QC2AudioHwBuffer>) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::suspend_session(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::resume_session(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::set_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) {
    (void)hw_session_handle;
    (void)codecConfig;
    (void)codec;
    (void)param_index;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::get_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) {
    (void)hw_session_handle;
    (void)codec;
    (void)codecConfig;
    (void)param_index;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::drain(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::close_session(uint64_t* hw_session_handle) {
    (void)hw_session_handle;
    return QC2_OK;
}

QC2Status QC2AudioHwElite::deinit() {
    return QC2_OK;
}
};  //  namespace qc2audio

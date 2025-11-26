/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_ELITE_H_
#define _QC2AUDIO_AUDIO_HW_ELITE_H_

#include <fcntl.h>
#include <sys/eventfd.h>
#include <stack>
#include <array>
#include <poll.h>
#include <numeric>
#include <condition_variable>
#include <chrono>
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>
#include <list>
using namespace std::chrono_literals;

#include "QC2.h"
#include "QC2Buffer.h"
#include "QC2AudioHw.h"
#include "QC2Thread.h"
#include "QC2EventQueue.h"

namespace qc2audio {
class BufferList;

class QC2AudioHwElite: public QC2AudioHw {
 public:
    QC2Status init(Notifier* notifier) override;
    QC2Status open_session(QC2AudioCommonCodecConfig& inputMediaFmt,
                                QC2AudioCommonCodecConfig& outputMediaFmt,
                                uint64_t** hw_session_handle) override;
    QC2Status start_session(uint64_t* hw_session_handle) override;
    QC2Status stop_session(uint64_t* hw_session_handle) override;
    QC2Status write_data(uint64_t* hw_session_handle,
                         std::shared_ptr<QC2AudioHwBuffer>) override;
    QC2Status queue_read_buffer(uint64_t* hw_session_handle,
                                std::shared_ptr<QC2AudioHwBuffer>) override;
    QC2Status flush(uint64_t* hw_session_handle) override;
    QC2Status drain(uint64_t* hw_session_handle) override;
    QC2Status suspend_session(uint64_t* hw_session_handle);
    QC2Status resume_session(uint64_t* hw_session_handle);
    QC2Status set_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) override;
    QC2Status get_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) override;
    QC2Status close_session(uint64_t* hw_session_handle) override;
    QC2Status deinit() override;
    Notifier* getNotifier() {return mNotifier;}
    QC2AudioHwElite();
   ~QC2AudioHwElite() = default;
 private:
    const std::string mInstName;
    Notifier* mNotifier = nullptr;
};

};  // namespace qc2audio


#endif //_QC2AUDIO_AUDIO_HW_ELITE_H_

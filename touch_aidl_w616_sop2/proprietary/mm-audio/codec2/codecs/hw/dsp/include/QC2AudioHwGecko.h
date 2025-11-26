/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_GECKO_H_
#define _QC2AUDIO_AUDIO_HW_GECKO_H_

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
#include "PalApi.h"
#include "media_fmt_api_ext.h"

namespace qc2audio {

typedef struct channel_info {
    uint16_t channel_count;
    std::vector<uint8_t> channel_position;
} channel_info;


class BufferList;

class QC2AudioHwGecko: public QC2AudioHw {
 public:
    QC2Status init(Notifier* notifier) override;
    QC2Status open_session(QC2AudioCommonCodecConfig& writeMediaFmt,
                                QC2AudioCommonCodecConfig& readMediaFmt,
                                uint64_t** hw_session_handle) override;
    QC2Status start_session(uint64_t* hw_session_handle) override;
    QC2Status stop_session(uint64_t* hw_session_handle) override;
    QC2Status close_session(uint64_t* hw_session_handle) override;
    QC2Status write_data(uint64_t* hw_session_handle,
                         std::shared_ptr<QC2AudioHwBuffer>) override;
    QC2Status queue_read_buffer(uint64_t* hw_session_handle,
                                std::shared_ptr<QC2AudioHwBuffer>) override;
    QC2Status flush(uint64_t* hw_session_handle) override;
    QC2Status drain(uint64_t* hw_session_handle) override;
    QC2Status suspend_session(uint64_t* hw_session_handle) override;
    QC2Status resume_session(uint64_t* hw_session_handle) override;
    QC2Status set_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) override;
    QC2Status get_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) override;
    QC2Status deinit() override;
    static HardwareState getHwState() {return gHwState;}
    static void setHwState(HardwareState state){gHwState = state;}

    //Methods needed by callback function
    Notifier* getNotifier() {return mNotifier;}
    QC2Status getInputBufferIndex(uint32_t fd, uint32_t offset, uint64_t &buf_index);
    QC2Status parseOutputMetadata(uint8_t* output_metadata, size_t metadata_size,
                                        uint64_t& input_buf_index, void*& payload);
    void clearPendingInputs();
    QC2AudioHwGecko();
    ~QC2AudioHwGecko() = default;

protected:
    std::shared_ptr<QC2LinearBufferPool> mLinearPool = nullptr;
    QC2AudioCommonCodecConfig mInputMediaFmt, mOutputMediaFmt;
    std::shared_ptr<amrnb_enc_cfg_t> amrnb_enc_cfg_payload = nullptr;
    std::shared_ptr<amrwb_enc_cfg_t> amrwb_enc_cfg_payload = nullptr;
    std::shared_ptr<payload_media_fmt_eamr_t> amr_media_fmt_payload = nullptr;
    std::shared_ptr<payload_media_fmt_amrwbplus_t> amr_wb_plus_media_fmt_payload = nullptr;

private:
    const std::string mInstName;
    Notifier* mNotifier = nullptr;
    int mAudioHwFd;   /*!< file descriptor of the device */
    uint64_t mPrevTS;
    CodecRawFormatPath mPcmPath;
    uint32_t mSampleSizePerChannel;
    std::map<uint32_t, std::map<uint32_t, uint64_t>> mInputsPendingAck; // map<FD, map<offset, input_frame_id>>
    uint32_t mModuleInstanceId;
    static HardwareState gHwState;
    std::mutex mInputsPendingAckMutex;
    static int32_t hw_callback_fn(pal_stream_handle_t *stream_handle,
                                  uint32_t event_id, uint32_t *event_data,
                                  uint32_t event_data_size, uint64_t cookie);
    QC2Status setStreamBufferConfig(pal_stream_handle_t *stream_handle,
                                  CodecType codec, CodecRawFormatPath pcm_path);
    QC2Status setOpenSessionParams(pal_stream_handle_t *stream_handle,
                             CodecType type, CodecRawFormatPath pcm_path,
                             const void *codec_info);
    QC2Status setDynamicParam(pal_stream_handle_t *stream_handle,
                              CodecType codec, const void *codec_info);
    QC2Status setEncoderExtnParams(CodecType codec, const void* codec_info,
                                   std::shared_ptr<std::vector<uint8_t>>& module_payload,
                                   size_t *size);
    QC2Status setDecoderExtnParams(CodecType codec, const void* codec_info,
                                   std::shared_ptr<std::vector<uint8_t>>& module_payload,
                                   size_t *size);
    void getPayloadForMediaFormatParam(CodecType codec, const void* codec_info,
                                       uint8_t** payload, size_t* payload_size);
    void getPayloadForEncoderConfigParam(CodecType codec, const void* codec_info,
                                         uint8_t** payload, size_t* payload_size);
    QC2Status buildModulePayload(std::shared_ptr<std::vector<uint8_t>>& module_payload,
                                 size_t *size, uint8_t *custom_payload,
                                 uint32_t custom_payload_size, uint32_t module_instance_id,
                                 uint32_t param_id);
    QC2Status getModuleInstanceId(pal_stream_handle_t *stream_handle, uint32_t tag_id,
                                  uint32_t *miid);
    QC2Status getBufferConfig(CodecType codec, CodecRawFormatPath pcm_path,
                             std::shared_ptr<pal_buffer_config_t>& in_buf_cfg,
                             std::shared_ptr<pal_buffer_config_t>& out_buf_cfg);
    QC2Status fillChStruct(uint32_t channel_count, channel_info *ch_info);
    uint32_t convertCodecTypeToHwFormat(CodecType codec);
    uint32_t convertCodecTypeToPALCodecFormat(CodecType codec, uint32_t bits_per_sample);
    uint32_t convertCodecTypeToGeckoCodecFormat(CodecType codec);
    int16_t convertBitRateToGeckoFormat(CodecType codec, uint16_t bitrate);
    void addToPendingInputs(std::shared_ptr<QC2AudioHwBuffer> hw_buffer);
    void fillMetaData(std::shared_ptr<std::vector<uint8_t>>& write_metadata_block,
                      size_t& metadata_size, std::shared_ptr<QC2AudioHwBuffer> hw_buffer);
};

typedef struct QC2AudioHwGeckoCbkCookie {
    QC2AudioHwGecko *this_hw;
    QC2AudioHw::Notifier *this_codec;
}QC2AudioHwGeckoCbkCookie;

};  // namespace qc2audio


#endif //_QC2AUDIO_AUDIO_HW_GECKO_H_

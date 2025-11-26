/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2AudioHwGecko.h"
#include "QC2Constants.h"
#include "PalApi.h"
#include "dsp_includes.h"
#include "apm_api.h"
#include "media_fmt_api_basic.h"
#include "kvh2xml.h"
#include <cmath>
#include <tuple>
#include <cstddef>
#include <errno.h>
//#ifdef C2AUDIO_DEBUG_MODE
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/AString.h>
//#endif
#undef LOG_TAG
#define LOG_TAG "QC2AudioHwGecko"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

static constexpr uint32_t BYTES_PER_SAMPLE( uint32_t x) {return (x / 8);}
static constexpr uint32_t MAKE_POSITIVE( int32_t x) { return (x < 0? -x: x);}
static constexpr uint32_t GET_LSW(uint64_t num) {return static_cast<uint32_t>(num & UINT32_MAX); }
static constexpr uint32_t GET_MSW(uint64_t num) {return static_cast<uint32_t>((num & ~UINT32_MAX) >> 32); }
static constexpr long long CONVERT_TIMESPEC_TO_US(struct timespec* ts) {
        if (ts->tv_sec < 0 || ts->tv_nsec < 0) { return 0; }
        return ((ts->tv_sec * 1000000ull) + (ts->tv_nsec / 1000ull));
    }

//add new metadata item size compute function here
size_t START_METADATA_SIZE() {return sizeof(metadata_header_t) + sizeof(module_cmn_md_buffer_start_t); }
size_t END_METADATA_SIZE() {return sizeof(metadata_header_t) + sizeof(module_cmn_md_buffer_end_t); }
size_t MEDIA_FORMAT_METADATA_SIZE() {return sizeof(metadata_header_t)
                                          + sizeof(struct media_format_t)
                                          + sizeof(payload_media_fmt_pcm_t)
                                          + (ChannelHelper::MAX_NUM_CHANNELS*sizeof(int8_t)); }

//update applicable lists for new metadata item added above
static std::vector<size_t> WRITE_METADATA_ITEM_SIZES = {START_METADATA_SIZE(), END_METADATA_SIZE()};
static std::vector<size_t> READ_METADATA_ITEM_SIZES = {START_METADATA_SIZE(), END_METADATA_SIZE(),
                                                                          MEDIA_FORMAT_METADATA_SIZE()};

size_t WRITE_METADATA_MAX_SIZE() { return std::accumulate(WRITE_METADATA_ITEM_SIZES.begin(),
                                                                                WRITE_METADATA_ITEM_SIZES.end(), 0);
                                                    }

size_t READ_METADATA_MAX_SIZE() { return std::accumulate(READ_METADATA_ITEM_SIZES.begin(),
                                                                                READ_METADATA_ITEM_SIZES.end(), 0);
}

int32_t GET_C2_ERROR(int32_t hw_error) {
    hw_error = MAKE_POSITIVE(hw_error);
    return (HwToC2ErrorMap.find(hw_error) == HwToC2ErrorMap.end() ?
            HwToC2ErrorMap.at(EFAULT): HwToC2ErrorMap.at(hw_error));
}

enum
MetadataType: uint8_t {
    START_METADATA = 0,
    END_METADATA,
    MEDIA_FORMAT_EVENT
};
std::mutex gLock;
static const uint32_t TAG_MODULE_LIST_SIZE = 1024;
static const uint8_t MAX_RETRIES_FOR_HW_INIT = 10;
static const uint16_t RETRY_INTERVAL_FOR_HW_INIT_MS = 200;
HardwareState QC2AudioHwGecko::gHwState = HW_DOWN;

channel_info defaultChannelmappings[] =
                                 { {1,
                                      {PAL_CHMAP_CHANNEL_FL}},
                                   {2,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR}},
                                   {3,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_C}},
                                   {4,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_LB,
                                       PAL_CHMAP_CHANNEL_RB}},
                                   {5,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_LB,
                                       PAL_CHMAP_CHANNEL_RB,
                                       PAL_CHMAP_CHANNEL_C}},
                                   {6,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_C,
                                       PAL_CHMAP_CHANNEL_LFE,
                                       PAL_CHMAP_CHANNEL_LB,
                                       PAL_CHMAP_CHANNEL_RB}},
                                   {7,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_C,
                                       PAL_CHMAP_CHANNEL_LFE,
                                       PAL_CHMAP_CHANNEL_LB,
                                       PAL_CHMAP_CHANNEL_RB,
                                       PAL_CHMAP_CHANNEL_RC}},
                                   {8,
                                      {PAL_CHMAP_CHANNEL_FL,
                                       PAL_CHMAP_CHANNEL_C,
                                       PAL_CHMAP_CHANNEL_FR,
                                       PAL_CHMAP_CHANNEL_SL,
                                       PAL_CHMAP_CHANNEL_SR,
                                       PAL_CHMAP_CHANNEL_LB,
                                       PAL_CHMAP_CHANNEL_RB,
                                       PAL_CHMAP_CHANNEL_LFE}},
                                   };


QC2AudioHwGecko::QC2AudioHwGecko()
:
  mNotifier(nullptr),
  mSampleSizePerChannel(1),
  mModuleInstanceId(0) {
}

QC2Status QC2AudioHwGecko::open_session(
    QC2AudioCommonCodecConfig& writeMediaFmt,
    QC2AudioCommonCodecConfig& readMediaFmt,
    uint64_t** hw_session_handle) {
    QLOGV("%s: Enter", __func__);

    QC2Status ret = QC2_OK;
    int32_t hw_error_val = -EINVAL;
    pal_stream_handle_t *stream_handle = NULL;
    channel_info chInfo;
    mPcmPath = ((writeMediaFmt.codec_type == Type_PCM)? Path_INPUT: Path_OUTPUT);

    auto srcm_flag_if_required = [](CodecType type)
    {
        return (type == Type_AAC || type == Type_AMR_WB_PLUS) ?
                                    PAL_STREAM_FLAG_SRCM_INBAND: 0;
    };

    auto session_attrib = std::make_unique<struct pal_stream_attributes>();

    if (!session_attrib)
        return QC2_ERROR;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    session_attrib->type = PAL_STREAM_NON_TUNNEL;
    //Register for SRCM events only if required
    session_attrib->flags = static_cast<pal_stream_flags_t>
                            (PAL_STREAM_FLAG_EXTERN_MEM |
                             PAL_STREAM_FLAG_TIMESTAMP |
                             PAL_STREAM_FLAG_NON_BLOCKING |
                             srcm_flag_if_required(writeMediaFmt.codec_type));

    session_attrib->direction = PAL_AUDIO_INPUT_OUTPUT;

    // Write media config (out)
    session_attrib->out_media_config.sample_rate = writeMediaFmt.sample_rate;
    session_attrib->out_media_config.bit_width = writeMediaFmt.bits_per_sample;
    if (fillChStruct(writeMediaFmt.channel_count, &chInfo) != QC2_OK) {
        return QC2_ERROR;
    }
    session_attrib->out_media_config.ch_info.channels = chInfo.channel_count;
    memcpy_s(session_attrib->out_media_config.ch_info.ch_map,
            chInfo.channel_count,
            &chInfo.channel_position,
            chInfo.channel_count);
    session_attrib->out_media_config.aud_fmt_id =
            static_cast<pal_audio_fmt_t>(convertCodecTypeToPALCodecFormat(writeMediaFmt.codec_type,
                                                                   writeMediaFmt.bits_per_sample));

    // Read media config (in)
    session_attrib->in_media_config.sample_rate = readMediaFmt.sample_rate;
    session_attrib->in_media_config.bit_width = readMediaFmt.bits_per_sample;
    if (fillChStruct(readMediaFmt.channel_count, &chInfo) != QC2_OK) {
        return QC2_ERROR;
    }
    session_attrib->in_media_config.ch_info.channels = chInfo.channel_count;
    memcpy_s(session_attrib->in_media_config.ch_info.ch_map,
             chInfo.channel_count,
             &chInfo.channel_position,
             chInfo.channel_count);
    session_attrib->in_media_config.aud_fmt_id =
            static_cast<pal_audio_fmt_t>(convertCodecTypeToPALCodecFormat(readMediaFmt.codec_type,
                                                                   readMediaFmt.bits_per_sample));

    QC2AudioHwGeckoCbkCookie pCbkObject = QC2AudioHwGeckoCbkCookie();
    pCbkObject.this_hw = this;
    pCbkObject.this_codec = mNotifier;
    auto this_ptr = reinterpret_cast<uint64_t>(this);
    QLOGV("%s: this ptr 0x%llx",__func__, (unsigned long long)(this_ptr));
    hw_error_val = pal_stream_open(
            session_attrib.get(), 0, nullptr, 0, nullptr,
            &hw_callback_fn, reinterpret_cast<uint64_t>(this), &stream_handle);
    if (hw_error_val && !stream_handle) {
        QLOGE("%s: Error in pal_stream_open, error code =0x%x, stream_handle=0x%lx",
                    __func__, hw_error_val, reinterpret_cast<unsigned long>(stream_handle));
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_error_val));
        if (ret == QC2_CORRUPTED) {
          setHwState(HW_DOWN);
          QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    QLOGV("%s: Pal stream open completed successfully, handle=0x%lx",
                    __func__, reinterpret_cast<unsigned long>(stream_handle));
    *hw_session_handle = static_cast<uint64_t *>(stream_handle);

    auto getCompressedType = [&]() {
            return (mPcmPath == Path_OUTPUT) ? writeMediaFmt.codec_type:
                 readMediaFmt.codec_type;
    };
    auto getCodecInfo = [&]() {
        return (mPcmPath == Path_OUTPUT) ? writeMediaFmt.codec_info:
                readMediaFmt.codec_info;
    };
    // Set buffer config
    if ((ret = setStreamBufferConfig(stream_handle, getCompressedType(), mPcmPath)) != QC2_OK) {
        QLOGE("%s: Error in setting PAL stream buffer config", __func__);
        close_session(*hw_session_handle);
        *hw_session_handle = nullptr;
        return static_cast<QC2Status>(GET_C2_ERROR(ret));
    }

    // Set codec specific parameters on stream open
    if ((ret = setOpenSessionParams(
                stream_handle, getCompressedType(), mPcmPath, getCodecInfo())) != QC2_OK) {
        QLOGE("%s: Error in setting codec parameters", __func__);
        close_session(*hw_session_handle);
        *hw_session_handle = nullptr;
        return static_cast<QC2Status>(GET_C2_ERROR(ret));
    }

    if (mPcmPath == Path_INPUT) {
        mSampleSizePerChannel =
            BYTES_PER_SAMPLE(writeMediaFmt.bits_per_sample) * writeMediaFmt.channel_count;
    }

    return ret;
}


QC2Status QC2AudioHwGecko::flush(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    QLOGD_INST("sending FLUSH command");
    int32_t hw_ret_val = pal_stream_flush(static_cast<pal_stream_handle_t*>(hw_session_handle));
    ret = (hw_ret_val)? static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val)): QC2_OK;
    if (ret != QC2_OK) {
        if (ret == QC2_CORRUPTED) {
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::init(Notifier* notifier) {
    QLOGV("%s: Enter", __func__);
    mNotifier = notifier;
    int32_t hw_ret_val = pal_init();
    if (hw_ret_val) {
        QLOGE("%s: Error in pal_init, error code =0x%x", __func__, hw_ret_val);
        return static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
    }
    uint8_t *hw_state_ptr = nullptr;
    uint8_t retry_attempt = 0;
    size_t  size = 0;

    if (getHwState() != HW_UP) {
      do {
          hw_ret_val = pal_get_param(PAL_PARAM_ID_SNDCARD_STATE, (void**)&hw_state_ptr,
                                                                             &size, nullptr);

          if ( *hw_state_ptr != CARD_STATUS_ONLINE || hw_ret_val) {
              QLOGW("%s:Error in retrieving snd card state, error code =0x%x", __func__,
                                                                                 hw_ret_val);
              QLOGW("%s:Retry after %u ms, Attempts left=%u", __func__,
                                                     RETRY_INTERVAL_FOR_HW_INIT_MS,
                                                     MAX_RETRIES_FOR_HW_INIT - retry_attempt);
              std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL_FOR_HW_INIT_MS));
              retry_attempt++;
          } else
            break;
      } while (retry_attempt < MAX_RETRIES_FOR_HW_INIT);

      if (*hw_state_ptr != CARD_STATUS_ONLINE) {
          QLOGE("%s:HW Offline with status= 0x%x, failing Init", __func__, *hw_state_ptr);
          setHwState(HW_DOWN);
          return QC2_CORRUPTED;
      }
      setHwState(HW_UP);
    }

    QLOGV("%s: HW init completed successfully", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::start_session(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    pal_stream_handle_t* stream_handle = static_cast<pal_stream_handle_t*>(hw_session_handle);
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = pal_stream_start(stream_handle);
    if (hw_ret_val) {
        QLOGE("Error in pal_stream_start, error code =0x%x, \
                              return code =0x%x", hw_ret_val, GET_C2_ERROR(hw_ret_val));
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (ret == QC2_CORRUPTED) {
          setHwState(HW_DOWN);
          QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    QLOGV("%s: Pal start_session completed successfully", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::stop_session(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return static_cast<QC2Status>(GET_C2_ERROR(EINVAL));
    }
    pal_stream_handle_t* stream_handle = static_cast<pal_stream_handle_t*>(hw_session_handle);
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = pal_stream_stop(stream_handle);
    if (hw_ret_val) {
        QLOGE("Error in pal_stream_stop, error code =0x%x", hw_ret_val);
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (ret == QC2_CORRUPTED) {
          setHwState(HW_DOWN);
          QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::close_session(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;
    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return static_cast<QC2Status>(GET_C2_ERROR(EINVAL));
    }
    pal_stream_handle_t* stream_handle = static_cast<pal_stream_handle_t*>(hw_session_handle);
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = pal_stream_close(stream_handle);
    if (hw_ret_val) {
        //non-zero return value indicates error
        QLOGE("Error in pal_stream_close, error code =0x%x", hw_ret_val);
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (ret == QC2_CORRUPTED) {
          setHwState(HW_DOWN);
          QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::write_data(uint64_t* hw_session_handle,
                                        std::shared_ptr<QC2AudioHwBuffer> hw_buffer) {

    QC2Status status = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = 0;
    auto hw_write_block = std::make_shared<std::vector<uint8_t>>(sizeof(struct pal_buffer));
    if (!hw_write_block) {
        QLOGE("%s: Could not allocate mem for hw write buffer", __func__);
        return (QC2Status)GET_C2_ERROR(ENOMEM);
    }
    struct pal_buffer* write_buf = reinterpret_cast<struct pal_buffer*>(hw_write_block->data());
    write_buf->buffer = reinterpret_cast<uint8_t*>(hw_buffer->buffer);
    write_buf->size = hw_buffer->filled_length;
    write_buf->offset = 0;
    auto ts = std::make_shared<std::vector<uint8_t>>(sizeof(struct timespec));
    write_buf->ts = reinterpret_cast<struct timespec*>(ts->data());
    if (hw_buffer->ts > 0) {
      write_buf->ts->tv_sec = static_cast<int64_t>(hw_buffer->ts / 1000000ull);
      write_buf->ts->tv_nsec = static_cast<int64_t>((hw_buffer->ts % 1000000ull) * 1000ull);
      if (write_buf->ts->tv_nsec < 0) {
          write_buf->ts->tv_sec--;
          write_buf->ts->tv_nsec += 1000000000ull;
      }
    } else {
          QLOGW("%s: Negative or zero timestamp received, reseting to 0", __func__);
    }
    QLOGV("%s: input_ts sec:%lld, nsec:%lld", __func__,
               (unsigned long long)write_buf->ts->tv_sec,
               (unsigned long long)write_buf->ts->tv_nsec);
    write_buf->flags = 0 | PAL_STREAM_FLAG_TIMESTAMP | PAL_STREAM_FLAG_EOF;
    if (hw_buffer->last_frame)
        write_buf->flags |= WR_SH_MEM_EP_BIT_MASK_LAST_BUFFER_FLAG;
    write_buf->alloc_info.alloc_handle = hw_buffer->fd;
    write_buf->alloc_info.alloc_size   = hw_buffer->size;
    write_buf->alloc_info.offset       = hw_buffer->offset;

    auto write_metadata_block = std::make_shared<std::vector<uint8_t>>(WRITE_METADATA_MAX_SIZE());
    if (!write_metadata_block) {
        QLOGE("%s: Could not allocate mem for write metadata", __func__);
        return QC2_NO_MEMORY;
    }
    fillMetaData(write_metadata_block, write_buf->metadata_size, hw_buffer);
    write_buf->metadata = write_metadata_block->data();

    /*android::AString hex;
    hexdump(write_buf->buffer, write_buf->size, 0, &hex);
    QLOGV("%s: DEBUGC2: writeBuf: %s\n", __func__, hex.c_str());*/

    hw_ret_val = pal_stream_write(reinterpret_cast<pal_stream_handle_t*>(hw_session_handle), write_buf);
    if (hw_ret_val < 0) {
        //non-zero return value indicates error
        status = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        QLOGE("Error in pal_stream_write, error code =0x%x, qc2status=0x%x",
                                                hw_ret_val, status);
        if (status == QC2_CORRUPTED){
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return status;
    }
    QLOGV("%s: Write Buffer %lu written successfully ", __func__, static_cast<unsigned long>(hw_buffer->input_frame_id));
    addToPendingInputs(hw_buffer);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::queue_read_buffer(uint64_t* hw_session_handle,
                                        std::shared_ptr<QC2AudioHwBuffer> hw_buffer) {
    QC2Status status = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = 0;
    auto hw_read_block = std::make_shared<std::vector<uint8_t>>(sizeof(struct pal_buffer));
    if (!hw_read_block) {
            QLOGE("%s: Could not allocate mem for hw read buffer", __func__);
            return (QC2Status)GET_C2_ERROR(ENOMEM);
    }
    struct pal_buffer* read_buf = reinterpret_cast<struct pal_buffer*>(hw_read_block->data());
    pal_stream_handle_t* stream_handle = static_cast<pal_stream_handle_t*>(hw_session_handle);
    read_buf->alloc_info.alloc_handle = hw_buffer->fd;
    read_buf->alloc_info.alloc_size   = hw_buffer->size;
    read_buf->alloc_info.offset       = hw_buffer->offset;
    read_buf->size                    = hw_buffer->filled_length;
    read_buf->metadata_size           = READ_METADATA_MAX_SIZE();
    QLOGV("%s: READ METADATA MAX SIZE=%u", __func__, (unsigned int)read_buf->metadata_size);
    hw_ret_val = pal_stream_read(stream_handle, read_buf);
    if (hw_ret_val < 0) {
        //non-zero return value indicates error
        QLOGE("Error in pal_stream_read, error code =0x%x", hw_ret_val);
        status = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (status == QC2_CORRUPTED){
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return status;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::suspend_session(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (getHwState() == HW_DOWN)
      return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val =
        pal_stream_suspend(static_cast<pal_stream_handle_t*>(hw_session_handle));

    ret = ((hw_ret_val) ? static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val)) : QC2_OK);
    if (ret == QC2_CORRUPTED) {
        setHwState(HW_DOWN);
        QLOGE("Fatal error returned from HW");
    }
    return ret;
}

QC2Status QC2AudioHwGecko::resume_session(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (getHwState() == HW_DOWN)
      return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val =
        pal_stream_start(static_cast<pal_stream_handle_t*>(hw_session_handle));

    ret = ((hw_ret_val) ? static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val)) : QC2_OK);
    if (ret == QC2_CORRUPTED) {
        setHwState(HW_DOWN);
        QLOGE("Fatal error returned from HW");
    }
    return ret;
}

QC2Status QC2AudioHwGecko::set_codec_config(uint64_t* hw_session_handle,
                            CodecType codec,
                            std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                            uint32_t param_index) {
    (void)hw_session_handle;
    (void)codecConfig;
    (void)codec;
    (void)param_index;
    //mapC2ParamToPalParam();
    //setDynamicParam();
    QLOGV("%s: Enter", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::get_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) {
    (void)hw_session_handle;
    (void)codec;
    (void)codecConfig;
    (void)param_index;
    QLOGV("%s: Enter", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::drain(uint64_t* hw_session_handle) {
    QC2Status ret = QC2_OK;

    if (getHwState() == HW_DOWN)
        return QC2_CORRUPTED;

    if (!hw_session_handle) {
        QLOGE("%s: Hw session handle is invalid", __func__);
        return QC2_ERROR;
    }
    QLOGV("%s: Enter", __func__);
    int32_t hw_ret_val = pal_stream_drain(static_cast<pal_stream_handle_t*>(hw_session_handle), PAL_DRAIN);
    if (hw_ret_val < 0) {
        //non-zero return value indicates error
        QLOGE("Error in pal_stream_drain, error code =0x%x", hw_ret_val);
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (ret == QC2_CORRUPTED) {
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::deinit() {
  QLOGV("%s: Enter", __func__);
    pal_deinit();
    return QC2_OK;
}

//--------------------- helper functions ---------------------------------

void QC2AudioHwGecko::fillMetaData(std::shared_ptr<std::vector<uint8_t>>& write_metadata_block,
                                   size_t& metadata_size, std::shared_ptr<QC2AudioHwBuffer> hw_buffer) {

    auto getOffsetForEndMetadata = [pcmPath = mPcmPath, sampleSizePerCh = mSampleSizePerChannel](uint32_t buf_size)
    {
        return (pcmPath == Path_INPUT)? (buf_size/sampleSizePerCh): buf_size;
    };

    uint8_t* current_write_ptr = write_metadata_block->data();
    //Fill start metadata
    metadata_header_t* start_metadata_item = reinterpret_cast<metadata_header_t*>(current_write_ptr);
    start_metadata_item->metadata_id = MODULE_CMN_MD_ID_BUFFER_START;
    start_metadata_item->flags = static_cast<uint32_t>(MD_HEADER_FLAGS_BUFFER_ASSOCIATED << 4);
    start_metadata_item->offset = 0;
    start_metadata_item->payload_size = sizeof(module_cmn_md_buffer_start_t);
    current_write_ptr = static_cast<uint8_t*>(current_write_ptr + sizeof(metadata_header_t));
    module_cmn_md_buffer_start_t start_metadata_payload = {GET_LSW(hw_buffer->input_frame_id),
                                                            GET_MSW(hw_buffer->input_frame_id)};
    memcpy_s(reinterpret_cast<void*>(current_write_ptr), sizeof(module_cmn_md_buffer_start_t),
                      reinterpret_cast<const void*>(&start_metadata_payload), sizeof(module_cmn_md_buffer_start_t));
    current_write_ptr += start_metadata_item->payload_size;

    //Fill end metadata
    metadata_header_t* end_metadata_item = reinterpret_cast<metadata_header_t*>(current_write_ptr);
    end_metadata_item->metadata_id = MODULE_CMN_MD_ID_BUFFER_END;
    end_metadata_item->flags = static_cast<uint32_t>(MD_HEADER_FLAGS_BUFFER_ASSOCIATED << 4);
    end_metadata_item->offset = getOffsetForEndMetadata(hw_buffer->filled_length);
    end_metadata_item->payload_size = sizeof(module_cmn_md_buffer_end_t);
    current_write_ptr = current_write_ptr + sizeof(metadata_header_t);
    module_cmn_md_buffer_end_t end_metadata_payload = {GET_LSW(hw_buffer->input_frame_id),
                                                       GET_MSW(hw_buffer->input_frame_id),
                                                       0};
    memcpy_s(reinterpret_cast<void*>(current_write_ptr), sizeof(module_cmn_md_buffer_end_t),
                reinterpret_cast<const void*>(&end_metadata_payload), sizeof(module_cmn_md_buffer_end_t));
    current_write_ptr += end_metadata_item->payload_size;
    metadata_size = static_cast<uint32_t>(WRITE_METADATA_MAX_SIZE());
}

QC2Status QC2AudioHwGecko::setStreamBufferConfig(pal_stream_handle_t *stream_handle,
                                        CodecType codec, CodecRawFormatPath pcm_path) {
    auto in_buf_cfg = std::make_shared<pal_buffer_config_t>();
    auto out_buf_cfg = std::make_shared<pal_buffer_config_t>();
    QC2Status ret = QC2_OK;
    int32_t hw_ret = 0;

    if ((ret = getBufferConfig(codec, pcm_path, in_buf_cfg, out_buf_cfg)) != QC2_OK) {
        QLOGE("%s: Could not fetch stream buffer configs", __func__);
        return ret;
    }

    if ((hw_ret = pal_stream_set_buffer_size(
                        stream_handle, in_buf_cfg.get(), out_buf_cfg.get())) < 0) {
        QLOGE("%s: PAL set buffer size returned error %d", __func__, ret);
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret));
        if (ret == QC2_CORRUPTED) {
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return ret;
    }

    return QC2_OK;
}

QC2Status QC2AudioHwGecko::setOpenSessionParams(pal_stream_handle_t *stream_handle,
                                          CodecType codec, CodecRawFormatPath pcm_path,
                                          const void *codec_info) {
    size_t module_payload_size = 0; // final payload size
    size_t wrapped_size = 0; // media format header + param payload
    size_t param_payload_size = 0;
    std::shared_ptr<std::vector<uint8_t>> module_payload = nullptr;
    uint8_t *param_payload = nullptr;
    int32_t hw_ret_val = 0;
    QC2Status ret = QC2_OK;

    // Construct media format/encoder config param
    if (pcm_path == Path_INPUT) {
        if (!mModuleInstanceId) {
            if ((ret = getModuleInstanceId(stream_handle,
                        TAG_STREAM_PLACEHOLDER_ENCODER, &mModuleInstanceId)) != QC2_OK) {
                QLOGE("%s: Could not get module instance id for stream %p",
                       __func__, stream_handle);
                if (ret == QC2_CORRUPTED) {
                    setHwState(HW_DOWN);
                    QLOGE("Fatal error returned from HW");
                }
                return ret;
            }
        }

        // Encoder output config parameter
        getPayloadForEncoderConfigParam(codec, codec_info,
                                        &param_payload, &param_payload_size);

        if (!param_payload || !param_payload_size) {
            QLOGE("%s: Invalid %s", __func__,
                        (param_payload_size? "param_payload" : "param_payload_size"));
            return QC2_ERROR;
        }

        wrapped_size =
            sizeof(struct param_id_encoder_output_config_t) + param_payload_size;
        std::vector<uint8_t> enc_out_block(wrapped_size);
        if (!enc_out_block.data()) {
            QLOGE("%s: Could not allocate media format header", __func__);
            return QC2_NO_MEMORY;
        }
        struct param_id_encoder_output_config_t *enc_out_cfg =
                reinterpret_cast<struct param_id_encoder_output_config_t *>(enc_out_block.data());
        enc_out_cfg->data_format = DATA_FORMAT_FIXED_POINT;
        enc_out_cfg->fmt_id = convertCodecTypeToGeckoCodecFormat(codec);
        enc_out_cfg->payload_size = param_payload_size;
        if (!enc_out_cfg->payload_size) {
            QLOGE("%s: enc out cfg payload size is 0 for codec %d", __func__, codec);
            return QC2_ERROR;
        }
        void *enc_payload = reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(enc_out_cfg) +
                                sizeof(struct param_id_encoder_output_config_t));
        memcpy_s(enc_payload, enc_out_cfg->payload_size, param_payload, param_payload_size);

        // Wrap parameter in apm module header
        if (buildModulePayload(module_payload, &module_payload_size,
                        reinterpret_cast<uint8_t *>(enc_out_block.data()),
                        wrapped_size, mModuleInstanceId,
                        PARAM_ID_ENCODER_OUTPUT_CONFIG) != QC2_OK) {
            QLOGE("%s: custom payload formation failed", __func__);
            return QC2_ERROR;
        }

        // Extension parameters other than media format
        if (setEncoderExtnParams(codec, codec_info,
                        module_payload, &module_payload_size) != QC2_OK) {
            QLOGE("%s: decoder extension param formation failed", __func__);
            return QC2_ERROR;
        }
    } else if (pcm_path == Path_OUTPUT) {
        if (!mModuleInstanceId) {
            // Get module instance id for WR endpoint
            if ((ret = getModuleInstanceId(stream_handle,
                        STREAM_INPUT_MEDIA_FORMAT, &mModuleInstanceId)) != QC2_OK) {
                QLOGE("%s: Could not get module instance id for stream %p",
                       __func__, stream_handle);
                if (ret == QC2_CORRUPTED) {
                    setHwState(HW_DOWN);
                    QLOGE("Fatal error returned from HW");
                }
                return ret;
            }
        }

        // Media format parameter
        getPayloadForMediaFormatParam(codec, codec_info, &param_payload, &param_payload_size);
        if (!param_payload || !param_payload_size) {
            QLOGE("%s: Invalid %s", __func__,
                        (param_payload_size? "param_payload" : "param_payload_size"));
            return QC2_ERROR;
        }
        wrapped_size = sizeof(struct media_format_t) + param_payload_size;
        std::vector<uint8_t> media_fmt_block(wrapped_size);
        if (!media_fmt_block.data()) {
            QLOGE("%s: Could not allocate media format header", __func__);
            return QC2_NO_MEMORY;
        }
        struct media_format_t *media_fmt_hdr =
                reinterpret_cast<struct media_format_t *>(media_fmt_block.data());
        media_fmt_hdr->data_format = DATA_FORMAT_RAW_COMPRESSED;
        media_fmt_hdr->fmt_id = convertCodecTypeToGeckoCodecFormat(codec);
        media_fmt_hdr->payload_size = param_payload_size;
        if (!media_fmt_hdr->payload_size) {
            QLOGE("%s: media format payload size is 0 for codec %d", __func__, codec);
            return QC2_ERROR;
        }
        void *dec_payload = reinterpret_cast<void *>(
                    reinterpret_cast<uint8_t *>(media_fmt_hdr) + sizeof(struct media_format_t));
        memcpy_s(dec_payload, param_payload_size, param_payload, param_payload_size);

        // Wrap parameter in apm header
        if (buildModulePayload(module_payload, &module_payload_size,
                        reinterpret_cast<uint8_t *>(media_fmt_block.data()),
                        wrapped_size, mModuleInstanceId,
                        PARAM_ID_MEDIA_FORMAT) != QC2_OK) {
            QLOGE("%s: custom payload formation failed", __func__);
            return QC2_ERROR;
        }

        // Extension parameters other than media format
        if (setDecoderExtnParams(codec, codec_info,
                        module_payload, &module_payload_size) != QC2_OK) {
            QLOGE("%s: decoder extension param formation failed", __func__);
            return QC2_ERROR;
        }
    }

    auto pal_param_block = std::make_shared<std::vector<uint8_t>>(
                        (sizeof(pal_param_payload) + module_payload_size));
    if (!pal_param_block) {
        QLOGE("%s: Could not allocate pal_param_payload", __func__);
        return QC2_NO_MEMORY;
    }
    pal_param_payload *pal_param_payload_ptr =
            (pal_param_payload *)pal_param_block->data();
    pal_param_payload_ptr->payload_size = module_payload_size;
    memcpy_s(pal_param_payload_ptr->payload, module_payload_size,
                 (uint8_t *)module_payload->data(), module_payload_size);

    if ((hw_ret_val = pal_stream_set_param(stream_handle,
                PAL_PARAM_ID_MODULE_CONFIG, pal_param_payload_ptr)) < 0) {
        QLOGE("%s: PAL set param returned error %d", __func__, hw_ret_val);
        ret = static_cast<QC2Status>(GET_C2_ERROR(hw_ret_val));
        if (ret == QC2_CORRUPTED) {
            setHwState(HW_DOWN);
            QLOGE("Fatal error returned from HW");
        }
        return ret;
    }
    return ret;
}

QC2Status QC2AudioHwGecko::setDynamicParam(pal_stream_handle_t *stream_handle,
                                          CodecType codec, const void *codec_info) {
    (void)stream_handle;
    (void)codec;
    (void)codec_info;

    return QC2_OK;
}

QC2Status QC2AudioHwGecko::setDecoderExtnParams(
                            CodecType codec, const void* codec_info,
                            std::shared_ptr<std::vector<uint8_t>>& module_payload,
                            size_t *size) {
    (void)codec;
    (void)codec_info;
    (void)module_payload;
    (void)size;

    QLOGV("%s: enter", __func__);
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::setEncoderExtnParams(
                            CodecType codec, const void* codec_info,
                            std::shared_ptr<std::vector<uint8_t>>& module_payload,
                            size_t *size) {
    QLOGV("%s: enter", __func__);
    switch (codec) {
        case Type_AAC: {
            // Set AAC encoder bitrate
            QC2AudioAacEncConfig *aacEncConfig = const_cast<QC2AudioAacEncConfig *>(
                                reinterpret_cast<const QC2AudioAacEncConfig*>(codec_info));
            auto bit_rate_param = std::make_shared<param_id_enc_bitrate_param_t>();
            if (!bit_rate_param) {
                QLOGE("%s: Could not allocate memory for bit rate param", __func__);
                return QC2_NO_MEMORY;
            }
            bit_rate_param->bitrate = aacEncConfig->bitRate;
            // Wrap parameter in apm module header
            if (buildModulePayload(module_payload, size,
                        reinterpret_cast<uint8_t *>(bit_rate_param.get()),
                        sizeof(struct param_id_enc_bitrate_param_t),
                        mModuleInstanceId, PARAM_ID_ENC_BITRATE) != QC2_OK) {
                QLOGE("%s: custom payload formation failed", __func__);
                return QC2_ERROR;
            }
            break;
        }
        default:
            break;
    }
    return QC2_OK;
}

void QC2AudioHwGecko::getPayloadForEncoderConfigParam(
                        CodecType codec, const void* codec_info,
                        uint8_t** payload, size_t* payload_size) {
    int16_t enc_mode = 0;

    switch (codec) {
        case Type_AMR_NB: {
            QC2AudioAmrConfig* amr_nb_config = (QC2AudioAmrConfig *)(codec_info);
            amrnb_enc_cfg_payload = std::make_shared<amrnb_enc_cfg_t>();
            enc_mode = convertBitRateToGeckoFormat(codec, amr_nb_config->bitrate);
            if (enc_mode < 0 || !amrnb_enc_cfg_payload ) {
                *payload_size = 0;
                *payload = nullptr;
                break;
            }
            amrnb_enc_cfg_payload->enc_mode = static_cast<uint16_t>(enc_mode);
            amrnb_enc_cfg_payload->dtx_mode = amr_nb_config->dtx_mode;
            *payload = reinterpret_cast<uint8_t *>(amrnb_enc_cfg_payload.get());
            *payload_size = sizeof(amrnb_enc_cfg_t);
            break;
        }
        case Type_AMR_WB: {
            QC2AudioAmrConfig* amr_wb_config = const_cast<QC2AudioAmrConfig*>(
                                reinterpret_cast<const QC2AudioAmrConfig*>(codec_info));
            amrwb_enc_cfg_payload = std::make_shared<amrwb_enc_cfg_t>();
            enc_mode = convertBitRateToGeckoFormat(codec, amr_wb_config->bitrate);
            if (enc_mode < 0 || !amrwb_enc_cfg_payload) {
                *payload_size = 0;
                *payload = nullptr;
                break;
            }
            amrwb_enc_cfg_payload->enc_mode = static_cast<uint16_t>(enc_mode);
            amrwb_enc_cfg_payload->dtx_mode = amr_wb_config->dtx_mode;
            *payload = reinterpret_cast<uint8_t *>(amrwb_enc_cfg_payload.get());
            *payload_size = sizeof(amrwb_enc_cfg_t);
            break;
        }
        case Type_AAC: {
            QC2AudioAacEncConfig *aacEncConfig = const_cast<QC2AudioAacEncConfig *>(
                                reinterpret_cast<const QC2AudioAacEncConfig *>(codec_info));
            if(!aacEncConfig) {
                *payload_size = 0;
                *payload = nullptr;
                break;
            }
            *payload = reinterpret_cast<uint8_t *>(&aacEncConfig->encOpConfig);
            *payload_size = sizeof(aac_enc_cfg_t);
            break;
        }
        case Type_EVRC: {
            // EVRC codec_info is in line with SPF encoder output config payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            *payload_size = sizeof(evrc_enc_cfg_t);
            break;
        }
        case Type_QCELP: {
            // QCELP codec_info is in line with SPF encoder output config payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            *payload_size = sizeof(v13k_enc_cfg_t);
            break;
        }
        default: {
            *payload = nullptr;
            *payload_size = 0;
            break;
        }
    }
}

void QC2AudioHwGecko::getPayloadForMediaFormatParam(
                        CodecType codec, const void* codec_info,
                        uint8_t** payload, size_t* payload_size) {
    switch (codec) {

        case Type_AAC: {
            // AAC codec_info is in line with SPF media format payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload_size = sizeof(payload_media_fmt_aac_t);
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            break;
        }
        case Type_AMR_NB: {
            *payload_size = sizeof(payload_media_fmt_eamr_t);
            amr_media_fmt_payload = std::make_shared<payload_media_fmt_eamr_t>();
            if (!amr_media_fmt_payload) {
                *payload_size = 0;
                *payload = nullptr;
                break;
            }
            amr_media_fmt_payload->media_format_id = MEDIA_FMT_ID_AMRNB_FS;
            *payload = reinterpret_cast<uint8_t *>(amr_media_fmt_payload.get());
            break;
        }
        case Type_AMR_WB_PLUS: {
            *payload_size = sizeof(payload_media_fmt_amrwbplus_t);
            amr_wb_plus_media_fmt_payload = std::make_shared<payload_media_fmt_amrwbplus_t>();
            if (!amr_wb_plus_media_fmt_payload) {
                *payload_size = 0;
                *payload = nullptr;
                break;
            }
            amr_wb_plus_media_fmt_payload->amr_frame_fmt = ONE_HEADER_IN_SUPERFRAME;
            *payload = reinterpret_cast<uint8_t *>(amr_wb_plus_media_fmt_payload.get());
            break;
        }
        case Type_ALAC: {
            // ALAC codec_info is in line with SPF media format payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload_size = sizeof(payload_media_fmt_alac_t);
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            break;
        }
        case Type_APE: {
            // APE codec_info is in line with SPF media format payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload_size = sizeof(payload_media_fmt_ape_t);
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            break;
        }
        case Type_WMA_STD: {
            // WMA STD codec_info is in line with SPF media format payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload_size = sizeof(payload_media_fmt_wmastd_t);
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            break;
        }
        case Type_WMA_PRO:
        case Type_WMA_LOSSLESS: {
            // WMA PRO codec_info is in line with SPF media format payload
            // Any changes to payload struct in SPF header need to be tracked
            *payload_size = sizeof(payload_media_fmt_wmapro_t);
            *payload = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(codec_info));
            break;
        }
        default: {
            *payload = nullptr;
            *payload_size = 0;
            break;
        }
    }
}

QC2Status QC2AudioHwGecko::buildModulePayload(
            std::shared_ptr<std::vector<uint8_t>>& module_payload, size_t *size,
            uint8_t *custom_payload,  uint32_t custom_payload_size,
            uint32_t module_instance_id, uint32_t param_id) {
    // Allocate memory for apm module parameter
    size_t paramSize = sizeof(struct apm_module_param_data_t) + custom_payload_size;
    if (!module_payload) {
        module_payload = std::make_shared<std::vector<uint8_t>>(paramSize);
    } else {
        module_payload->reserve(paramSize + *size);
    }

    struct apm_module_param_data_t *header =
            (struct apm_module_param_data_t *)(module_payload->data() + *size);
    header->module_instance_id = module_instance_id;
    header->param_id = param_id;
    header->error_code = 0x0;
    header->param_size = custom_payload_size;

    if (custom_payload_size) {
        memcpy_s(module_payload->data() + *size + sizeof(struct apm_module_param_data_t),
                         custom_payload_size,
                         custom_payload,
                         custom_payload_size);
    }
    *size += paramSize;
    return QC2_OK;
}

QC2Status QC2AudioHwGecko::getModuleInstanceId(pal_stream_handle_t *stream_handle,
                                               uint32_t tag_id, uint32_t *miid) {
    // Retrieve tagged module info for stream
    size_t tag_module_size = TAG_MODULE_LIST_SIZE;
    std::vector<uint8_t> tag_module_info(tag_module_size);
    if (!tag_module_info.data()) {
        QLOGE("%s: Could not allocate memory for module info", __func__);
        return QC2_NO_MEMORY;
    }
    int32_t ret = pal_stream_get_tags_with_module_info(stream_handle,
                        &tag_module_size, reinterpret_cast<uint8_t *>(tag_module_info.data()));
    if (ret == ENODATA) {
        tag_module_info.resize(tag_module_size);
        ret = pal_stream_get_tags_with_module_info(stream_handle,
                        &tag_module_size, reinterpret_cast<uint8_t *>(tag_module_info.data()));
    }
    if (ret < 0) {
        QLOGE("%s: Could not retrieve tag module info from PAL", __func__);
        return static_cast<QC2Status>(GET_C2_ERROR(ret));
    }

    struct pal_tag_module_info *tag_info =
            reinterpret_cast<struct pal_tag_module_info *>(tag_module_info.data());
    if (tag_info == nullptr) {
        QLOGE("%s: Could not retrieve tag module info from PAL", __func__);
        return QC2_ERROR;
    }
    struct pal_tag_module_mapping *tag_entry =
            reinterpret_cast<struct pal_tag_module_mapping *>(&tag_info->pal_tag_module_list[0]);
        if (tag_entry == nullptr) {
        QLOGE("%s: Could not find tag entry", __func__);
                return QC2_ERROR;
        }

    for (uint32_t idx = 0, offset = 0; idx < tag_info->num_tags; idx++) {
        tag_entry += offset / sizeof(struct pal_tag_module_mapping);
        QLOGV("%s: tag id[%d] = %u, num_modules = %u\n",
                  __func__, idx, tag_entry->tag_id, tag_entry->num_modules);
        offset = sizeof(struct pal_tag_module_mapping) +
                    (tag_entry->num_modules * sizeof(struct module_info));
        if (tag_entry->tag_id == tag_id) {
            if (tag_entry->num_modules) {
                 struct module_info *mod_info_entry =
                        reinterpret_cast<struct module_info *>(&tag_entry->mod_list[0]);
                 *miid = mod_info_entry->module_iid;
                 QLOGV("%s: MIID is %x\n", __func__, *miid);
                 return QC2_OK;
            }
        }
    }
    return QC2_NOT_FOUND;
}

QC2Status QC2AudioHwGecko::getBufferConfig(CodecType codec, CodecRawFormatPath pcm_path,
                                          std::shared_ptr<pal_buffer_config_t>& in_buf_cfg,
                                          std::shared_ptr<pal_buffer_config_t>& out_buf_cfg) {
    (void)codec;
    (void)pcm_path;

    in_buf_cfg->buf_count = 0,
    in_buf_cfg->buf_size = 0,
    in_buf_cfg->max_metadata_size = WRITE_METADATA_MAX_SIZE();

    out_buf_cfg->buf_count = 0;
    out_buf_cfg->buf_size = 0,
    out_buf_cfg->max_metadata_size = READ_METADATA_MAX_SIZE();

    return QC2_OK;
}

QC2Status QC2AudioHwGecko::fillChStruct(uint32_t channel_count, channel_info *ch_info) {
    if (!channel_count || channel_count > 8) {
        return QC2_ERROR;
    }
    *ch_info = defaultChannelmappings[channel_count-1];
    return QC2_OK;
}

int16_t QC2AudioHwGecko::convertBitRateToGeckoFormat(CodecType codec, uint16_t bitrate) {

    if (codec == Type_AMR_NB) {
        switch (bitrate) {
            //AMR-NB encoding rates
            case 4750:
                return AMRNB_FS_ENCODE_MODE_MR475;
            case 5150:
                return AMRNB_FS_ENCODE_MODE_MR515;
            case 5900:
                return AMRNB_FS_ENCODE_MODE_MMR59;
            case 6700:
                return AMRNB_FS_ENCODE_MODE_MMR67;
            case 7400:
                return AMRNB_FS_ENCODE_MODE_MMR74;
            case 7950:
                return AMRNB_FS_ENCODE_MODE_MMR795;
            case 10200:
                return AMRNB_FS_ENCODE_MODE_MMR102;
            case 12200:
                return AMRNB_FS_ENCODE_MODE_MMR122;
            default:
                return AMRNB_FS_ENCODE_MODE_MR475;
        }
    } else if (codec == Type_AMR_WB){
        switch (bitrate) {
            //AMR-WB encoding rates
            case 6600:
                return AMRWB_FS_ENCODE_MODE_MR66;
            case 8850:
                return AMRWB_FS_ENCODE_MODE_MR885;
            case 12650:
                return AMRWB_FS_ENCODE_MODE_MR1265;
            case 14250:
                return AMRWB_FS_ENCODE_MODE_MR1425;
            case 15850:
                return AMRWB_FS_ENCODE_MODE_MR1585;
            case 18250:
                return AMRWB_FS_ENCODE_MODE_MR1825;
            case 19850:
                return AMRWB_FS_ENCODE_MODE_MR1985;
            case 23050:
                return AMRWB_FS_ENCODE_MODE_MR2305;
            case 23850:
                return AMRWB_FS_ENCODE_MODE_MR2385;
            default:
                return AMRWB_FS_ENCODE_MODE_MR66;
        }
    }

    return NOT_FOUND;
}

uint32_t QC2AudioHwGecko::convertCodecTypeToGeckoCodecFormat(CodecType codec) {
    switch (codec) {
        case Type_AAC:
            return MEDIA_FMT_ID_AAC;
        case Type_AMR_NB:
            return MEDIA_FMT_ID_AMRNB_FS;
        case Type_AMR_WB:
            return MEDIA_FMT_ID_AMRWB_FS;
        case Type_AMR_WB_PLUS:
            return MEDIA_FMT_ID_AMRWBPLUS;
        case Type_ALAC:
            return MEDIA_FMT_ID_ALAC;
        case Type_APE:
            return MEDIA_FMT_ID_APE;
        case Type_WMA_STD:
            return MEDIA_FMT_ID_WMASTD;
        case Type_WMA_PRO:
        case Type_WMA_LOSSLESS:
            return MEDIA_FMT_ID_WMAPRO;
        case Type_EVRC:
            return MEDIA_FMT_ID_EVRC;
        case Type_QCELP:
            return MEDIA_FMT_ID_V13K_FS;
        default:
            return 0;
    }
}

uint32_t QC2AudioHwGecko::convertCodecTypeToPALCodecFormat(CodecType codec,
                                                           uint32_t bits_per_sample) {
    switch (codec) {
        case Type_AAC:
             return PAL_AUDIO_FMT_AAC;
        case Type_ALAC:
             return PAL_AUDIO_FMT_ALAC;
        case Type_AMR_NB:
             return PAL_AUDIO_FMT_AMR_NB;
        case Type_AMR_WB:
            return PAL_AUDIO_FMT_AMR_WB;
        case Type_AMR_WB_PLUS:
            return PAL_AUDIO_FMT_AMR_WB_PLUS;
        case Type_APE:
            return PAL_AUDIO_FMT_APE;
        case Type_WMA_STD:
             return PAL_AUDIO_FMT_WMA_STD;
        case Type_WMA_PRO:
        case Type_WMA_LOSSLESS:
             return PAL_AUDIO_FMT_WMA_PRO;
        case Type_EVRC:
             return PAL_AUDIO_FMT_EVRC;
        case Type_QCELP:
             return PAL_AUDIO_FMT_QCELP;
        case Type_PCM:
            switch (bits_per_sample) {
                case 8:
                     return PAL_AUDIO_FMT_PCM_S8;
                case 16:
                default:
                     return PAL_AUDIO_FMT_PCM_S16_LE;
                case 24:
                     return PAL_AUDIO_FMT_PCM_S24_3LE;
                case 32:
                     return PAL_AUDIO_FMT_PCM_S32_LE;
             }
        case Type_Unknown:
        default:
             return PAL_AUDIO_FMT_PCM_S16_LE;
    }
    return PAL_AUDIO_FMT_PCM_S16_LE;
}

void QC2AudioHwGecko::clearPendingInputs() {
  std::shared_ptr<QC2AudioHwBuffer> emptyInBuf(nullptr);
  std::lock_guard<std::mutex> pendingAcksLock(mInputsPendingAckMutex);
  for (auto const& itFd : mInputsPendingAck) {
      auto fd = itFd.first;
      std::map<uint32_t, uint64_t> offsetToFrameIdxMap = itFd.second;
      for (auto const& itOffset : offsetToFrameIdxMap) {
          emptyInBuf = std::make_shared<QC2AudioHwBuffer>();
          if (emptyInBuf) {
              emptyInBuf->buf_type = BufferType_INPUT_ACK;
              emptyInBuf->offset = itOffset.first;
              emptyInBuf->filled_length = 0;
              emptyInBuf->size = 0;
              emptyInBuf->fd = fd;
              emptyInBuf->input_frame_id = itOffset.second;
              mNotifier->onInputsDone(emptyInBuf);
         }
      }
  }
}

void QC2AudioHwGecko::addToPendingInputs(std::shared_ptr<QC2AudioHwBuffer> hw_buffer) {
    auto fd = hw_buffer->fd;
    auto offset = hw_buffer->offset;
    auto ip_frame_id = hw_buffer->input_frame_id;

    std::lock_guard<std::mutex> pendingAcksLock(mInputsPendingAckMutex);
    auto itFd = mInputsPendingAck.find(fd);
    if (itFd != mInputsPendingAck.end()) {
        //found entry
        mInputsPendingAck[fd][offset] = ip_frame_id;
        QLOGV("%s: added offset %u and frame id %u", __func__, (uint32_t)offset,
               (uint32_t)mInputsPendingAck[fd][offset]);
    } else {
            //create new map<offset,input_buffer_index> and add to FD map
            QLOGV("%s: added map for fd %lu", __func__, (unsigned long)fd);
            mInputsPendingAck.insert(std::make_pair(fd, std::map<uint32_t, uint64_t>()));
            QLOGV("%s: added frame id %lu for fd %lu offset %u", __func__,
                        (unsigned long)ip_frame_id,  (unsigned long)fd, (unsigned int)offset);
            mInputsPendingAck[fd].insert(std::make_pair(offset, ip_frame_id));
    }
}

QC2Status QC2AudioHwGecko::getInputBufferIndex(uint32_t fd, uint32_t offset, uint64_t &buf_index) {

    QC2Status status = QC2_OK;
    std::lock_guard<std::mutex> pendingAcksLock(mInputsPendingAckMutex);
    std::map<uint32_t, std::map<uint32_t, uint64_t>>::iterator itFd = mInputsPendingAck.find(fd);
    if (itFd != mInputsPendingAck.end()) {
        //found entry
        std::map<uint32_t, uint64_t> offsetToFrameIdxMap = itFd->second;
        auto itOffsetFrameIdxPair = offsetToFrameIdxMap.find(offset);
        if (itOffsetFrameIdxPair != offsetToFrameIdxMap.end()){
            buf_index = itOffsetFrameIdxPair->second;
            QLOGV("%s ip_frame_id=%lu", __func__, (unsigned long)buf_index);
        } else {
            status = QC2_ERROR;
            QLOGE("%s: Entry doesn't exist for FD 0x%x and offset 0x%x", __func__,
                                    fd, offset);
        }
    }
    return status;
}

QC2Status QC2AudioHwGecko::parseOutputMetadata(uint8_t* output_metadata, size_t metadata_size, uint64_t& input_buf_index, void*& payload) {

    QC2Status status = QC2_OK;
    metadata_header_t* metadata_item = nullptr;
    uint8_t md_bytes_read = 0;

    if (output_metadata == nullptr || metadata_size < MIN(START_METADATA_SIZE(), END_METADATA_SIZE()) ){
            //TODO: may not work for multiple frames/buffer
            QLOGE("%s: Metadata payload smaller than expected, bytes 0x%x, expected 0x%x", __func__, md_bytes_read, MIN(START_METADATA_SIZE(), END_METADATA_SIZE()));
            return QC2_ERROR;
    }

    auto read_metadata_ptr = output_metadata;

    while (md_bytes_read < metadata_size) {
        metadata_item = reinterpret_cast<metadata_header_t*>(read_metadata_ptr + md_bytes_read);
        if (!metadata_item)
            break;
        md_bytes_read += sizeof(metadata_header_t);

        switch (metadata_item->metadata_id) {
            //TODO: check if 2 start metadata/2 end metadata is present
            case MODULE_CMN_MD_ID_BUFFER_START:
            {
                uint8_t start_metadata_payload_size = metadata_item->payload_size;
                if (md_bytes_read + start_metadata_payload_size > metadata_size){
                    QLOGE("%s: Metadata item payload size larger than advertized metadata size, metadata id 0x%x,\
                                                            md_bytes_read = 0x%x, item payload size 0x%x,\
                                                            metadata size = 0x%x ", __func__, metadata_item->metadata_id,
                                                            md_bytes_read, start_metadata_payload_size, static_cast<uint32_t>(metadata_size));
                    status = QC2_ERROR;
                    break;
                }
                module_cmn_md_buffer_start_t* start_metadata = reinterpret_cast<module_cmn_md_buffer_start_t*>(read_metadata_ptr+md_bytes_read);
                if (!start_metadata) {
                    QLOGE("%s: Metadata start payload not found at offset 0x%x", __func__, md_bytes_read);
                    status = QC2_ERROR;
                    break;
                }
                input_buf_index = static_cast<uint64_t>((static_cast<uint64_t>(start_metadata->buffer_index_msw) << 32) | start_metadata->buffer_index_lsw);
                md_bytes_read += sizeof(module_cmn_md_buffer_start_t);
                break;
            }
            case MODULE_CMN_MD_ID_BUFFER_END:
            {
                uint8_t end_metadata_payload_size = metadata_item->payload_size;
                if (md_bytes_read + end_metadata_payload_size > metadata_size){
                    QLOGE("%s: Metadata item payload size larger than advertized metadata size, metadata id 0x%x,\
                                                            md_bytes_read = 0x%x, item payload size 0x%x,\
                                                            metadata size = 0x%x ", __func__, metadata_item->metadata_id,
                                                            md_bytes_read, end_metadata_payload_size, static_cast<uint32_t>(metadata_size));
                    status = QC2_ERROR;
                    break;
                }
                module_cmn_md_buffer_end_t* end_metadata = reinterpret_cast<module_cmn_md_buffer_end_t*>(read_metadata_ptr+md_bytes_read);
                if (!end_metadata) {
                    QLOGE("%s: Metadata end payload not found at offset 0x%x", __func__, md_bytes_read);
                    status = QC2_ERROR;
                    break;
                }
                //TODO: compare previous input buffer index from start metadata, treat different value as error
                input_buf_index = static_cast<uint64_t>((static_cast<uint64_t>(end_metadata->buffer_index_msw) << 32) | end_metadata->buffer_index_lsw);

                if (end_metadata->flags) {
                  QLOGI("%s: End Metdata Flags=0x%x", __func__, end_metadata->flags);
                  if ( ((end_metadata->flags & MD_END_PAYLOAD_FLAGS_BIT_MASK_ERROR_RECOVERY_DONE)
                      >> MD_END_PAYLOAD_FLAGS_SHIFT_ERROR_RECOVERY_DONE)
                      == MD_END_RESULT_ERROR_RECOVERY_DONE ) {
                      QLOGI("%s: Error detected in input buffer and recovery attempted", __func__);
                  } else if ( ((end_metadata->flags & MD_END_PAYLOAD_FLAGS_BIT_MASK_ERROR_RESULT)
                      >> MD_END_PAYLOAD_FLAGS_SHIFT_ERROR_RESULT) == MD_END_RESULT_FAILED ) {
                      QLOGI("%s: Non-recoverable error detected in input buffer", __func__);
                  }
                }
                md_bytes_read += sizeof(module_cmn_md_buffer_end_t);
                break;
            }
            case MODULE_CMN_MD_ID_MEDIA_FORMAT:
            {
              uint8_t mf_metadata_payload_size = metadata_item->payload_size;
              if (md_bytes_read + mf_metadata_payload_size > metadata_size){
                  QLOGE("%s: Metadata item payload size larger than advertized metadata size, metadata id 0x%x,\
                                                          md_bytes_read = 0x%x, item payload size 0x%x,\
                                                          metadata size = 0x%x ", __func__, metadata_item->metadata_id,
                                                          md_bytes_read, mf_metadata_payload_size, static_cast<uint32_t>(metadata_size));
                  status = QC2_ERROR;
                  break;
              }
              media_format_t* mf_payload = reinterpret_cast<media_format_t*>(read_metadata_ptr+md_bytes_read);
              md_bytes_read += sizeof(media_format_t);

              if (!mf_payload) {
                  QLOGE("%s: Media metadata payload not found at offset 0x%x", __func__, md_bytes_read);
                  status = QC2_ERROR;
                  break;
              }
              if (mf_payload->fmt_id != MEDIA_FMT_ID_PCM){
                QLOGE("%s: Format ID within Media metadata payload not PCM, fmt_id=x%x, offset=0x%x",
                                                      __func__, mf_payload->fmt_id, (uint32_t)offsetof(media_format_t, fmt_id));
                status = QC2_ERROR;
                break;
              }

              payload_media_fmt_pcm_t* pcm_payload = reinterpret_cast<payload_media_fmt_pcm_t*>(read_metadata_ptr+md_bytes_read);

              auto codec_config = reinterpret_cast<QC2AudioCommonCodecConfig*>(payload);
              codec_config->sample_rate = pcm_payload->sample_rate;
              codec_config->channel_count = pcm_payload->num_channels;
              codec_config->bits_per_sample = pcm_payload->bits_per_sample;
              QLOGI("%s: sample_rate=%u, channel_count=%u, bits per sample=%u",
                    __func__, (unsigned int)codec_config->sample_rate,
                    (unsigned int)codec_config->channel_count,
                    codec_config->bits_per_sample);
              md_bytes_read +=
                      ALIGN(sizeof(payload_media_fmt_pcm_t)+pcm_payload->num_channels*sizeof(int8_t), 4);
              break;
            }
            default:
                QLOGE("%s: Unknown Metadata marker found at offset 0x%x, Metadata ID=0x%x", __func__, md_bytes_read, metadata_item->metadata_id);
                // increment bytes read
                md_bytes_read += metadata_item->payload_size;
                break;
        }
        QLOGV("%s: md_bytes_read=%u, metadata_size=%u", __func__,
                                                        (unsigned int)md_bytes_read,
                                                        (unsigned int)metadata_size);

        if (status != QC2_OK)
            break;
    } // end while

    return status;
}

int32_t QC2AudioHwGecko::hw_callback_fn(pal_stream_handle_t *stream_handle,
                                  uint32_t event_id, uint32_t *event_data,
                                  uint32_t event_data_size, uint64_t cookie) {
    std::lock_guard<std::mutex> hw_callback_lock(gLock);
    (void)stream_handle;
    (void)event_data_size;
    uint32_t status = 0;
    uint32_t md_status = 0;

    QC2AudioHwGecko* thiz = reinterpret_cast<QC2AudioHwGecko*>(cookie);
    pal_event_read_write_done_payload* hwAckBuffer = nullptr;
    std::shared_ptr<QC2AudioHwBuffer> inBuf;
    std::shared_ptr<QC2AudioHwBuffer> outBuf;
    auto output_media_fmt = std::make_shared<std::vector<uint8_t>>(sizeof(QC2AudioCommonCodecConfig));
    uint64_t ip_frame_id = 0;

    QLOGV("%s Enter this=%p", __func__, thiz);

    switch (event_id) {
        case PAL_STREAM_CBK_EVENT_WRITE_READY:
            QLOGV("%s PAL_STREAM_CBK_EVENT_WRITE_READY received", __func__);
            hwAckBuffer = reinterpret_cast<pal_event_read_write_done_payload*>(event_data);
            if (status){
                QLOGE("%s: Error received during write done, data status = 0x%x", __func__, status);
                thiz->getNotifier()->onError(static_cast<QC2Status>(GET_C2_ERROR(status)), "Error returned in data status for read done");
                break;
            } else if (md_status != 0) {
                if (md_status == ENOTRECOVERABLE)
                    QLOGE("%s:Error, md cannot be parsed in write buffer", __func__);
                else if (md_status == EOPNOTSUPP)
                    QLOGE("%s:Error, md id not recognized in write buffer", __func__);
                else
                    QLOGE("%s: Error received during read done, md status = 0x%x", __func__, status);
                thiz->getNotifier()->onError(static_cast<QC2Status>(GET_C2_ERROR(md_status)), "Error returned in md status for read done");
            }
            //handle write done event
            inBuf = std::make_shared<QC2AudioHwBuffer>();
            if (!inBuf) {
                QLOGE("%s: Allocation failed for WRITE_READY return buffer", __func__);
                return -1;
            }
            inBuf->buf_type = BufferType_INPUT_ACK;
            inBuf->offset = hwAckBuffer->buff.alloc_info.offset;
            inBuf->filled_length = hwAckBuffer->buff.size;
            inBuf->size = hwAckBuffer->buff.alloc_info.alloc_size;
            inBuf->fd = hwAckBuffer->buff.alloc_info.alloc_handle;

            if (thiz->getInputBufferIndex(inBuf->fd, inBuf->offset, ip_frame_id) == QC2_OK){
                inBuf->input_frame_id = ip_frame_id;
                QLOGV("%s: Obtained fd=%lu, offset=%u, frame id=%u", __func__,
                      (unsigned long)inBuf->fd, (unsigned int)inBuf->offset,
                      (uint32_t)inBuf->input_frame_id);
                thiz->getNotifier()->onInputsDone(inBuf);
            } else {
                QLOGE("%s: Obtained fd=%lu, offset=%u, frame id=%u Failed",
                      __func__, (unsigned long)inBuf->fd,
                      (unsigned int)inBuf->offset,
                      (uint32_t)inBuf->input_frame_id);
            }

            break;
        case PAL_STREAM_CBK_EVENT_READ_DONE:
          QLOGV("%s PAL_STREAM_CBK_EVENT_READ_DONE received", __func__);
            if (status){
                QLOGE("%s: Error received during read done, data status = 0x%x", __func__, status);
                thiz->getNotifier()->onError(static_cast<QC2Status>(GET_C2_ERROR(status)), "Error returned in data status for read done");
                break;
            } else if (md_status) {
                if (md_status == ENOMEM)
                    QLOGE("%s:Error, md buffer size small received during read done", __func__);
                else
                    QLOGE("%s: Error received during read done, md status = 0x%x", __func__, status);
                thiz->getNotifier()->onError(static_cast<QC2Status>(GET_C2_ERROR(md_status)), "Error returned in md status for read done");
                break;
            }

            // handle read done event
            hwAckBuffer = reinterpret_cast<pal_event_read_write_done_payload*>(event_data);
            outBuf = std::make_shared<QC2AudioHwBuffer>();
            if (!outBuf) {
                QLOGE("%s: Allocation failed for READ_DONE return buffer", __func__);
                return -1;
            }
            outBuf->buf_type = BufferType_OUTPUT_ACK;
            outBuf->offset = hwAckBuffer->buff.alloc_info.offset;
            outBuf->filled_length = hwAckBuffer->buff.size;
            outBuf->size = hwAckBuffer->buff.alloc_info.alloc_size;
            outBuf->fd = hwAckBuffer->buff.alloc_info.alloc_handle;
                        QLOGV("%s: Output buffer ts tv_sec=%llu, tv_nsec=%llu", __func__,
                        (unsigned long long)hwAckBuffer->buff.ts->tv_sec,
                        (unsigned long long)hwAckBuffer->buff.ts->tv_nsec);
            outBuf->ts = CONVERT_TIMESPEC_TO_US(hwAckBuffer->buff.ts);
            outBuf->payload = reinterpret_cast<void*>(output_media_fmt->data());
            if (thiz->parseOutputMetadata(hwAckBuffer->buff.metadata,
                                         hwAckBuffer->buff.metadata_size,
                                         ip_frame_id, outBuf->payload) == QC2_OK) {
                outBuf->input_frame_id = ip_frame_id;
                thiz->getNotifier()->onOutputsDone(outBuf);
            }
            break;
        case PAL_STREAM_CBK_EVENT_DRAIN_READY:
            //handle drain event
            QLOGV("%s PAL_STREAM_CBK_EVENT_DRAIN_READY received", __func__);
            break;
       //case PAL_STREAM_CBK_EVENT_MEDIA_FORMAT_CHANGE:
            //handle inband format change for HEAACv1/HEAACv2
        case PAL_STREAM_CBK_EVENT_ERROR:
            //handle error event
            QLOGV("%s PAL_STREAM_CBK_EVENT_ERROR received", __func__);
            thiz->setHwState(HW_DOWN);
            thiz->getNotifier()->onError(static_cast<QC2Status>(EFAULT), "Error returned from HW");
            thiz->clearPendingInputs();
            break;
        default:
            //Unknown event received
            QLOGV("%s Unknown event id received, event id = 0x%x", __func__, event_id);
            break;
    }
    return 0;
 }
};  //  namespace qc2audio

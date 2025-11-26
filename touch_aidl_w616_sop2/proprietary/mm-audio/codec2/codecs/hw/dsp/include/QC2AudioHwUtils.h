/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_UTILS_H_
#define _QC2AUDIO_AUDIO_HW_UTILS_H_

#include <unordered_map>
#include <errno.h>
#include "QC2CodecConfig.h"

namespace qc2audio {
class BufferList;

class InputTag {
 public:
    static constexpr uint32_t INVALID = 0;
    static inline constexpr uint32_t fromId(uint64_t id) {
        return id >= (UINT32_MAX - kOffset) ? INVALID : id + kOffset;
    }
    static inline constexpr uint64_t toId(uint32_t tag) {
        return tag - kOffset;
    }
 private:
    static constexpr uint32_t kOffset = 1;
};

static inline constexpr uint32_t MIN(uint32_t X, uint32_t Y) { return (X < Y)? X: Y; }
static constexpr int8_t NOT_FOUND = -1;

enum CodecType: uint32_t {
    Type_PCM          = 0x0,
    Type_AAC          = 0x1,
    Type_ALAC         = 0x2,
    Type_AMR_NB       = 0x3,
    Type_AMR_WB       = 0x4,
    Type_AMR_WB_PLUS  = 0x5,
    Type_APE          = 0x6,
    Type_EVRC         = 0x7,
    Type_G711_ALAW    = 0x8,
    Type_G711_ULAW    = 0x9,
    Type_WMA_STD      = 0xA,
    Type_WMA_PRO      = 0xB,
    Type_WMA_LOSSLESS = 0xC,
    Type_QCELP        = 0xD,
    Type_Unknown      = 0xFFFF,
};

enum CodecRawFormatPath: uint8_t {
    Path_INPUT = 0x0,    //encode
    Path_OUTPUT = 0x1    //decode
};

enum AACPackaging: uint16_t {
    AACPackaging_ADTS = 0x0,
    AACPackaging_LOAS = 0x1,
    AACPackaging_ADIF = 0x2,
    AACPackaging_RAW  = 0x3,
    AACPackaging_LATM = 0x4,
};

enum AACProfile: uint16_t {
    AACProfile_AAC_LC     = 0x2,
    AACProfile_AAC_SBR    = 0x5,
    AACProfile_AAC_SBR_PS = 0x1d,
};

enum AmrFrameFmt: uint32_t {
    ONE_HEADER_IN_SUPERFRAME       = 0x0,
    HEADER_PER_FRAME_IN_SUPERFRAME = 0x6,
};

enum AudioHWBufferType: uint8_t {
    BufferType_INPUT       = 0x1,
    BufferType_INPUT_ACK   = 0x2,
    BufferType_OUTPUT      = 0x3,
    BufferType_OUTPUT_ACK  = 0x4
};

enum HardwareState: uint8_t {
    HW_DOWN = 0x0,    // Entered when ADSP SSR/PD restart occurs
    HW_UP = 0x1       //Default State - in Normal operation
};

typedef struct QC2AudioHwAmrConfig {
    uint16_t bitrate;
    uint16_t dtx_mode;
} QC2AudioAmrConfig;

typedef struct QC2AudioHwAacConfig {
    AACPackaging   aac_format;
    AACProfile     audio_object_type;
    uint16_t       num_channels;
    uint16_t       total_size_of_pce_bits;
    uint32_t       sample_rate;
}QC2AudioAacConfig;

typedef QC2AlacDecConfig::AlacDecConfig QC2AudioAlacDecConfig;
typedef QC2ApeDecConfig::ApeDecConfig QC2AudioApeDecConfig;
typedef QC2WmaDecConfig::WmaStdDecConfig QC2AudioWmaStdDecConfig;
typedef QC2WmaDecConfig::WmaProDecConfig QC2AudioWmaProDecConfig;
typedef QC2AacEncConfig::AacEncConfig QC2AudioAacEncConfig;

typedef struct QC2AudioHwCommonCodecConfig {
    uint32_t sample_rate;
    uint32_t bits_per_sample;
    uint32_t channel_count;
    CodecType codec_type;
    uint32_t codec_info_size;
    void*     codec_info; // variable sized pointer to specific codec config struct
}QC2AudioCommonCodecConfig;


typedef struct QC2AudioHwBuffer {
    void* buffer;
    AudioHWBufferType buf_type;
    uint32_t offset;             //offset of buffer in FD
    uint32_t size;               //FD size
    uint32_t filled_length;      //buffer length
    uint32_t fd;
    int64_t ts;                 //timestamp in us
    uint64_t flags;
    uint64_t input_frame_id;     //input frame id
    uint64_t output_frame_id;    //output frame id, valid only if buf_type=TYPEOUTPUT
    bool     last_frame;
    void*    payload;
}QC2AudioHwBuffer;


typedef union {
    QC2AudioAacConfig aacConfig;
    QC2AudioAacEncConfig aacEncConfig;
    QC2AudioAlacDecConfig alacDecConfig;
    QC2AudioApeDecConfig apeDecConfig;
    QC2AudioWmaStdDecConfig wmaStdDecConfig;
    QC2AudioWmaProDecConfig wmaProDecConfig;
} QC2AudioCodecConfig;

static inline constexpr size_t align8Byte(size_t val) {
    return (((val) + 7) & (~7));
}

static const std::unordered_map<uint32_t, int32_t> HwToC2ErrorMap = {
                                                                    {EFAULT, EFAULT},     //EFAULT should be first entry always (default)
                                                                    {EAGAIN, ETIMEDOUT},
                                                                    {EALREADY, EEXIST},
                                                                    {EBUSY, EPERM},
                                                                    {ECANCELED, EFAULT},
                                                                    {EBADR, EINVAL},
                                                                    {EIO, EFAULT},
                                                                    {EINVAL,EINVAL},
                                                                    {ENETRESET, EFAULT},
                                                                    {ENODATA, ENOMEM},
                                                                    {ENOMEM, ENOMEM},
                                                                    {ENOSPC, ENOMEM},
                                                                    {ENODEV, ENODEV},
                                                                    {EPERM, EPERM},
                                                                    {ENOTRECOVERABLE, EFAULT},
                                                                    {ENETRESET, EFAULT},
                                                                    {EINPROGRESS, EPERM},
                                                                    {ETIMEDOUT, ETIMEDOUT},
                                                                    {EOPNOTSUPP, EPERM},
                                                                    {ENOPROTOOPT, ENOSYS}
                                                                };

};  // namespace qc2audio
#endif

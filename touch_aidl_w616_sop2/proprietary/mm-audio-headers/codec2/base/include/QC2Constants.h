/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_CONSTANTS_H_
#define _QC2AUDIO_CONSTANTS_H_

#include <string>
#include <unordered_map>
#include <C2BufferBase.h>
#include <C2Component.h>
#include <C2Config.h>

namespace qc2audio {

/// @addtogroup constants Global Constants
/// @{

/**
 * mime types
 */
struct MimeType {
    static constexpr const char * AMR_NB             = "audio/3gpp";
    static constexpr const char * AMR_WB             = "audio/amr-wb";
    static constexpr const char * AMR_WB_PLUS        = "audio/amr-wb-plus";
    static constexpr const char * MPEG_LAYER_I       = "audio/mpeg-L1";
    static constexpr const char * MPEG_LAYER_II      = "audio/mpeg-L2";
    static constexpr const char * AAC                = "audio/mp4a-latm";
    static constexpr const char * AAC_ADTS           = "audio/aac-adts";
    static constexpr const char * QCELP              = "audio/qcelp";
    static constexpr const char * G711_ALAW          = "audio/g711-alaw";
    static constexpr const char * G711_MLAW          = "audio/g711-mlaw";
    static constexpr const char * FLAC               = "audio/flac";
    static constexpr const char * ALAC               = "audio/alac";
    static constexpr const char * WMA                = "audio/x-ms-wma";
    static constexpr const char * WMA_PRO            = "audio/x-ms-wma-pro";
    static constexpr const char * WMA_LOSSLESS       = "audio/x-ms-wma-lossless";
    static constexpr const char * APE                = "audio/x-ape";
    static constexpr const char * EVRC               = "audio/evrc";
    static constexpr const char * DSD                = "audio/dsd";
    static constexpr const char * MPEGH              = "audio/mhas";
    static constexpr const char * RAW                = "audio/raw";
};

constexpr uint32_t MimeLen = 100;
/**
 *  Codec implementation type
 */
enum CodecImplType : uint32_t {
    IMPL_SW = 1,      ///< SW based implementation
    IMPL_DSP = 2,    ///< DSP based implementation
    IMPL_MOCK = 4,   ///< Mockup
};

/// Codec kind
enum ComponentKind : uint32_t {
    KIND_DECODER = C2Component::kind_t::KIND_DECODER,   ///< Codec is a decoder
    KIND_ENCODER = C2Component::kind_t::KIND_ENCODER,   ///< Codec is an encoder
    KIND_OTHER = C2Component::kind_t::KIND_OTHER,
    KIND_FILTER = 7,    ///< Codec is a filter
};
const char *Str(ComponentKind);

/// Memory usage
struct MemoryUsage {
    static constexpr const uint64_t CPU_READ                = C2MemoryUsage::CPU_READ;
    static constexpr const uint64_t CPU_WRITE               = C2MemoryUsage::CPU_WRITE;
    static constexpr const uint64_t CPU_WRITE_UNCACHED      = (uint32_t(1u) << 29);
    static constexpr const uint64_t HW_CODEC_READ           = 0x00010000U;
    static constexpr const uint64_t HW_CODEC_WRITE_CACHED = HW_CODEC_READ | C2MemoryUsage::CPU_READ;
    static constexpr const uint64_t HW_CODEC_WRITE        = HW_CODEC_READ | CPU_WRITE_UNCACHED;
    static constexpr const uint64_t HW_PROTECTED            = C2MemoryUsage::READ_PROTECTED;
    // static const char* Str(uint64_t);
};

// Consumer usage
struct ConsumerUsage {
    static constexpr const uint64_t OTHERS          = 0;
};

/**
 * Buffer flags. Will be added-to/retrieved-from QC2Buffer::info()
 */
struct BufFlag {
    static constexpr const uint64_t CODEC_CONFIG    = 0x0001;   ///< Buffer contains header (only)
    static constexpr const uint64_t EOS             = 0x0002;   ///< End Of Stream
    static constexpr const uint64_t I_FRAME         = 0x0004;   ///< Key-frame
    static constexpr const uint64_t CORRUPT         = 0x0020;   ///< Input data is corrupt
    // INPUT_PENDING: Signaled with output buffer to indicate that corresponding input will
    // result in further outputs (eg: packed frame)
    static constexpr const uint64_t INPUT_PENDING   = 0x0040;   ///< Input produces more outputs
    static constexpr const uint64_t EMPTY           = 0x0080;   ///< Buffer not produced
    static constexpr const uint64_t REFERENCED      = 0x0100;   ///< Buffer still being referenced
    static constexpr const uint64_t UPDATE_INPUT_TS = 0x0200;   ///< Ovewrite input TS (for AVTimer)
    static constexpr const uint64_t DISCARD         = 0x0400;   ///< Discard frame
    static std::string Str(uint64_t);
};

struct ChannelHelper {
    static constexpr const int MAX_NUM_CHANNELS = 16;
};

struct BitRateDefaultRange{
    static constexpr const uint MAX_BITRATE = UINT_MAX;
    static constexpr const uint MIN_BIRATE = 0;
};

static const std::pair<uint32_t, uint32_t> defaultSampleRateRange = { 8000, 192000 };

static const std::vector<uint32_t> defaultBitsPerSampleSet = {
        C2Config::PCM_8, C2Config::PCM_16
};

static const std::unordered_map<C2Config::pcm_encoding_t, uint32_t> sPcmEncodingtoBitsPerSample = {
        { C2Config::PCM_8, 8 },
        { C2Config::PCM_16, 16 },
        { C2Config::PCM_24, 24},
        { C2Config::PCM_32, 32 },
};

static const std::unordered_map<uint32_t, C2Config::pcm_encoding_t> sBitsPerSampletoPcmEncoding = {
        { 8, C2Config::PCM_8 },
        { 16, C2Config::PCM_16 },
        { 24, C2Config::PCM_24 },
        { 32, C2Config::PCM_32 },
};


struct FormatMapper {
    static uint32_t mapPcmEncodingToBitsPerSample(const C2Config::pcm_encoding_t pcmEncoding);
    static C2Config::pcm_encoding_t mapBitsPerSampletoPcmEncoding(const uint32_t bitsPerSample);
};

enum SampleRateIndex : uint32_t {
    MIN_SAMPLE_RATE_IDX = 0,
    MAX_SAMPLE_RATE_IDX
};

/// @}

};  // namespace qc2audio

#endif  // _QC2AUDIO_CONSTANTS_H_

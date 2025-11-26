/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_CONFIG_H_
#define _QC2AUDIO_CONFIG_H_

#include <sstream>
#include <unordered_map>
#include <C2Config.h>

namespace qc2audio {
// param indices for vendor extensions
enum QC2VendorExtensionParamIndexKind : C2Param::type_index_t;

}   // namespace qc2audio

// C2ENUMs work in global namespace
// param indices for vendor extensions
C2ENUM(qc2audio::QC2VendorExtensionParamIndexKind, C2Param::type_index_t,
    kParamIndexAudioFrameLength = C2Param::TYPE_INDEX_VENDOR_START,
    kParamIndexAudioPbTuning,
    kParamIndexAudioMbTuning,
    kParamIndexAudioKbTuning,
    kParamIndexAudioMinFrameBytes,
    kParamIndexAudioMaxFrameBytes,
    kParamIndexAudioMaxRun,
    kParamIndexAudioCompressionLevel,
    kParamIndexAudioFormatFlags,
    kParamIndexAudioBlocksPerFrame,
    kParamIndexAudioFinalFrameBlocks,
    kParamIndexAudioTotalFrames,
    kParamIndexAudioSeekTablePresent,
    kParamIndexAudioBeAmr,
    kParamIndexAudioWmaVersion,
    kParamIndexAudioWmaConfig,
    kParamIndexAudioMinBlockSize,
    kParamIndexAudioMaxBlockSize,
    kParamIndexAudioMinVocoderRate,
    kParamIndexAudioMaxVocoderRate,
    kParamIndexAudioNumOutSamplesPerChannel,
    kParamIndexAudioHwState,
    kParamIndexAudioEnd,
);

namespace qc2audio {

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioFrameLength> C2AudioFrameLengthInfo;
constexpr char QC2_PARAMKEY_AUDIO_FRAME_LENGTH[] = "qti-frame-length";

typedef C2StreamParam<C2Tuning, C2Uint32Value, kParamIndexAudioPbTuning> C2AudioPbTuningParam;
constexpr char QC2_PARAMKEY_AUDIO_PB_TUNING[] = "qti-pb-tuning";

typedef C2StreamParam<C2Tuning, C2Uint32Value, kParamIndexAudioMbTuning> C2AudioMbTuningParam;
constexpr char QC2_PARAMKEY_AUDIO_MB_TUNING[] = "qti-mb-tuning";

typedef C2StreamParam<C2Tuning, C2Uint32Value, kParamIndexAudioKbTuning> C2AudioKbTuningParam;
constexpr char QC2_PARAMKEY_AUDIO_KB_TUNING[] = "qti-kb-tuning";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMinFrameBytes> C2AudioMinFrameBytesInfo;
constexpr char QC2_PARAMKEY_AUDIO_MIN_FRAME_BYTES[] = "qti-min-frame-bytes";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMaxFrameBytes> C2AudioMaxFrameBytesInfo;
constexpr char QC2_PARAMKEY_AUDIO_MAX_FRAME_BYTES[] = "qti-max-frame-bytes";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMaxRun> C2AudioMaxRunInfo;
constexpr char QC2_PARAMKEY_AUDIO_MAX_RUN[] = "qti-max-run";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioCompressionLevel> C2AudioCompressionLevelInfo;
constexpr char QC2_PARAMKEY_AUDIO_COMPRESSION_LEVEL[] = "qti-compression-level";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMinBlockSize> C2AudioMinBlockSizeInfo;
constexpr char QC2_PARAMKEY_AUDIO_MIN_BLOCK_SIZE[] = "qti-min-block-size";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMaxBlockSize> C2AudioMaxBlockSizeInfo;
constexpr char QC2_PARAMKEY_AUDIO_MAX_BLOCK_SIZE[] = "qti-max-block-size";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioFormatFlags> C2AudioFormatFlagsInfo;
constexpr char QC2_PARAMKEY_AUDIO_FORMAT_FLAGS[] = "qti-format-flags";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioBlocksPerFrame> C2AudioBlocksPerFrameInfo;
constexpr char QC2_PARAMKEY_AUDIO_BLOCKS_PER_FRAME[] = "qti-blocks-per-frame";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioFinalFrameBlocks> C2AudioFinalFrameBlocksInfo;
constexpr char QC2_PARAMKEY_AUDIO_FINAL_FRAME_BLOCKS[] = "qti-final-frame-blocks";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioTotalFrames> C2AudioTotalFramesInfo;
constexpr char QC2_PARAMKEY_AUDIO_TOTAL_FRAMES[] = "qti-total-frames";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioSeekTablePresent> C2AudioSeekTablePresentInfo;
constexpr char QC2_PARAMKEY_AUDIO_SEEK_TABLE[] = "qti-seek-table-present";

typedef C2StreamParam<C2Info, C2BoolValue, kParamIndexAudioBeAmr> C2AudioBeAmrEnable;
constexpr char QC2_PARAMKEY_AUDIO_BEAMR_ENABLE[] = "qti-be-amr";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMinVocoderRate> C2AudioMinVocoderRateInfo;
constexpr char QC2_PARAMKEY_AUDIO_MIN_VOCODER_RATE[] = "qti-min-vocoder-rate";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioMaxVocoderRate> C2AudioMaxVocoderRateInfo;
constexpr char QC2_PARAMKEY_AUDIO_MAX_VOCODER_RATE[] = "qti-max-vocoder-rate";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioWmaVersion> C2AudioWmaVersionInfo;
constexpr char QC2_PARAMKEY_AUDIO_WMA_VERSION[] = "qti-wma-version";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioNumOutSamplesPerChannel> C2AudioNumOutSamplesPerChannelInfo;
constexpr char QC2_PARAMKEY_AUDIO_NUM_OUT_SAMPLES_PER_CHANNEL[] = "qti-num-out-samples-per-channel";

typedef C2StreamParam<C2Info, C2Uint32Value, kParamIndexAudioHwState> C2AudioHwStateInfo;
constexpr char QC2_PARAMKEY_AUDIO_HW_STATE[] = "qti-audio-hw-state";

struct C2AudioWmaConfigStruct {
    uint32_t bitsPerSample;
    uint32_t blkAlign;
    uint32_t channelMask;
    uint32_t formatTag;
    uint32_t encOptions;
    uint32_t advEncOption;
    uint32_t advEncOptions2;

    DEFINE_AND_DESCRIBE_BASE_C2STRUCT(AudioWmaConfig)
    C2FIELD(bitsPerSample, "bitsPerSample")
    C2FIELD(blkAlign, "blkAlign")
    C2FIELD(channelMask, "channelMask")
    C2FIELD(formatTag, "formatTag")
    C2FIELD(encOptions, "encOptions")
    C2FIELD(advEncOption, "advEncOption")
    C2FIELD(advEncOptions2, "advEncOptions2")
};

typedef C2StreamParam<C2Info, C2AudioWmaConfigStruct, kParamIndexAudioWmaConfig> C2AudioWmaConfig;
constexpr char QC2_PARAMKEY_AUDIO_WMA_CONFIG[] = "qti-wma-config";

/**
 * @brief Helper to map two enums and translate from one to another
 *
 * [1] declare an instance in .h
 *      Eg: DECLARE_ENUM_MAPPER(OurToTheirEnumMapper, OurEnum, TheirEnum)
 * [2] Assign the mapping in .cpp
 *      Eg: INIT_ENUM_MAPPER(OurToTheirEnumMapper, OurEnum, TheirEnum) = {{OurEnum1, TheirEnum2},..}
 *      Eg: INIT_ENUM_MAPPER(OurToTheirEnumMapper, OurEnum, TheirEnum) = {{OurEnum1, TheirEnum2},..}
 *   [a] Some enums have duplicates and hence break reverse mapping. Such mappers can explicitly
 *       provide a reverse mapper using
 *          INIT_ENUM_FORWARD_MAPPER(XYZ, OurEnum, TheirEnum) = {{O1, T1}, {O2, T1}, {O3, T2}};
 *          INIT_ENUM_REVERSE_MAPPER(XYZ, TheirEnum, OurEnum) = {{T1, O1}, {T2, O3}}; //  skip O2
 * [3] Translate from one to another
 *      Eg: OurToTheirEnumMapper::Translate(OurEnum, &toTheirEnum)   [ Our   -> Their ]
 *          OurToTheirEnumMapper::Translate(TheirEnum, &toOurEnum)   [ Their -> Our   ]
 */
#define DECLARE_ENUM_MAPPER(name, E1, E2)                                   \
    struct name {                                                           \
        static bool Translate(E1 e1, E2 *e2) {                              \
            if (e2) {                                                       \
                auto it = kMap.find(e1);                                    \
                if (it != kMap.end()) {                                     \
                    *e2 = it->second;                                       \
                    return true;                                            \
                }                                                           \
            }                                                               \
            return false;                                                   \
        }                                                                   \
        static bool Translate(E2 e2, E1 *e1) {                              \
            if (e1) {                                                       \
                if (kReverseMap.size() > 0) {                               \
                    auto it = kReverseMap.find(e2);                         \
                    if (it != kReverseMap.end()) {                          \
                        *e1 = it->second;                                   \
                        return true;                                        \
                    } else {                                                \
                        return false;                                       \
                    }                                                       \
                }                                                           \
                /* fallback for no reverse mapper */                        \
                for (auto it = kMap.begin(); it != kMap.end(); it++) {      \
                    if (it->second == e2) {                                 \
                        *e1 = it->first;                                    \
                        return true;                                        \
                    }                                                       \
                }                                                           \
            }                                                               \
            return false;                                                   \
        }                                                                   \
                                                                            \
    private:                                                               \
        static const std::unordered_map<E1, E2> kMap;                       \
        static const std::unordered_map<E2, E1> kReverseMap;                \
    };

// Define mapper with no reverse mapping
#define INIT_ENUM_MAPPER(name, E1, E2)                                      \
    const std::unordered_map<E2, E1> name::kReverseMap;                     \
    const std::unordered_map<E1, E2> name::kMap

// Define separate forward and reverse mappers
#define INIT_ENUM_FORWARD_MAPPER(name, E1, E2)                              \
    const std::unordered_map<E1, E2> name::kMap
#define INIT_ENUM_REVERSE_MAPPER(name, E2, E1)                              \
    const std::unordered_map<E2, E1> name::kReverseMap

struct DebugString {
    static inline const std::string C2Param(uint32_t id) {
        std::stringstream paramStr;
        uint32_t dir = id & 0x30000000;
        switch (dir) {
            case 0x00000000: paramStr << "Input"; break;
            case 0x10000000: paramStr << "Output"; break;
            case 0x20000000: paramStr << "Global"; break;
            default: paramStr << "Undefined"; break;
        }
        paramStr << "::";
        uint32_t coreId = id & 0xffff;
        switch (coreId) {
            case kParamIndexApiLevel : paramStr << "ApiLevel "; break;
            case kParamIndexApiFeatures: paramStr << "ApiFeatures"; break;
            case kParamIndexName: paramStr << "Name"; break;
            case kParamIndexAliases: paramStr << "Aliases"; break;
            case kParamIndexKind: paramStr << "Kind"; break;
            case kParamIndexDomain: paramStr << "Domain"; break;
            case kParamIndexAttributes: paramStr << "Attributes"; break;
            case kParamIndexTimeStretch: paramStr << "TimeStretch"; break;
            case kParamIndexProfileLevel: paramStr << "ProfileLevel"; break;
            case kParamIndexInitData: paramStr << "InitData"; break;
            case kParamIndexSupplementalData: paramStr << "SupplementalData"; break;
            case kParamIndexSubscribedSupplementalData:
                        paramStr << "SubscribedSupplementalData"; break;
            case kParamIndexMediaType: paramStr << "MediaType"; break;
            case kParamIndexDelayRequest: paramStr << "DelayRequest"; break;
            case kParamIndexDelay: paramStr << "Delay"; break;
            case kParamIndexMaxReferenceAge: paramStr << "MaxReferenceAge"; break;
            case kParamIndexMaxReferenceCount: paramStr << "MaxReferenceCount"; break;
            case kParamIndexReorderBufferDepth: paramStr << "ReorderBufferDepth"; break;
            case kParamIndexReorderKey: paramStr << "ReorderKey"; break;
            case kParamIndexStreamCount: paramStr << "StreamCount"; break;
            case kParamIndexSubscribedParamIndices: paramStr << "SubscribedParamIndices"; break;
            case kParamIndexSuggestedBufferCount: paramStr << "SuggestedBufferCount"; break;
            case kParamIndexBatchSize: paramStr << "BatchSize"; break;
            case kParamIndexCurrentWork: paramStr << "CurrentWork"; break;
            case kParamIndexLastWorkQueued: paramStr << "LastWorkQueued"; break;
            case kParamIndexAllocators: paramStr << "Allocators"; break;
            case kParamIndexBlockPools: paramStr << "BlockPools"; break;
            case kParamIndexBufferType: paramStr << "BufferType"; break;
            case kParamIndexUsage: paramStr << "Usage"; break;
            case kParamIndexOutOfMemory: paramStr << "OutOfMemory"; break;
            case kParamIndexMaxBufferSize: paramStr << "MaxBufferSize"; break;
            case kParamIndexTripped: paramStr << "Tripped"; break;
            case kParamIndexConfigCounter: paramStr << "ConfigCounter"; break;
            case kParamIndexResourcesNeeded: paramStr << "ResourcesNeeded"; break;
            case kParamIndexResourcesReserved: paramStr << "ResourcesReserved"; break;
            case kParamIndexOperatingRate: paramStr << "OperatingRate"; break;
            case kParamIndexRealTimePriority: paramStr << "RealTimePriority"; break;
            case kParamIndexSecureMode: paramStr << "SecureMode"; break;
            case kParamIndexBitrate : paramStr << "Bitrate "; break;
            case kParamIndexBitrateMode: paramStr << "BitrateMode"; break;
            case kParamIndexQuality: paramStr << "Quality"; break;
            case kParamIndexComplexity: paramStr << "Complexity"; break;
            case kParamIndexPrependHeaderMode: paramStr << "PrependHeaderMode"; break;
            case kParamIndexBlockSize: paramStr << "BlockSize"; break;
            case kParamIndexBlockCount: paramStr << "BlockCount"; break;
            case kParamIndexBlockRate: paramStr << "BlockRate"; break;
            case kParamIndexFrameRate : paramStr << "FrameRate "; break;
            case kParamIndexMaxBitrate: paramStr << "MaxBitrate"; break;
            case kParamIndexMaxFrameRate: paramStr << "MaxFrameRate"; break;
            case kParamIndexSampleRate : paramStr << "SampleRate "; break;
            case kParamIndexChannelCount: paramStr << "ChannelCount"; break;
            case kParamIndexMaxChannelCount: paramStr << "Max Output ChannelCount"; break;
            case kParamIndexPcmEncoding: paramStr << "PcmEncoding"; break;
            case kParamIndexAacPackaging: paramStr << "AacPackaging"; break;
            case kParamIndexPlatformLevel : paramStr << "PlatformLevel "; break;
            case kParamIndexPlatformFeatures: paramStr << "PlatformFeatures"; break;
            case kParamIndexStoreIonUsage: paramStr << "StoreIonUsage"; break;
            case kParamIndexStartAt: paramStr << "StartAt"; break;
            case kParamIndexSuspendAt: paramStr << "SuspendAt"; break;
            case kParamIndexResumeAt: paramStr << "ResumeAt"; break;
            case kParamIndexStopAt: paramStr << "StopAt"; break;
            case kParamIndexTimeOffset: paramStr << "TimeOffset"; break;
            case kParamIndexMinFrameRate: paramStr << "MinFrameRate"; break;
            case kParamIndexAudioFrameSize: paramStr << "AudioFrameSize"; break;
            case kParamIndexAudioFrameLength: paramStr << "AudioFrameLength"; break;
            case kParamIndexAudioPbTuning: paramStr << "AudioPbTuning"; break;
            case kParamIndexAudioMbTuning: paramStr << "AudioMbTuning"; break;
            case kParamIndexAudioKbTuning: paramStr << "AudioKbTuning"; break;
            case kParamIndexAudioMinFrameBytes: paramStr << "AudioMinFrameBytes"; break;
            case kParamIndexAudioMaxFrameBytes: paramStr << "AudioMaxFrameBytes"; break;
            case kParamIndexAudioMaxRun: paramStr << "AudioMaxRun"; break;
            case kParamIndexAudioCompressionLevel: paramStr << "AudioCompressionLevel"; break;
            case kParamIndexAudioFormatFlags: paramStr << "AudioFormatFlags"; break;
            case kParamIndexAudioBlocksPerFrame: paramStr << "AudioBlocksPerFrame"; break;
            case kParamIndexAudioFinalFrameBlocks: paramStr << "AudioFinalFrameBlocks"; break;
            case kParamIndexAudioTotalFrames: paramStr << "AudioTotalFrames"; break;
            case kParamIndexAudioSeekTablePresent: paramStr << "AudioSeekTablePresent"; break;
            case kParamIndexAudioBeAmr: paramStr << "AudioBeAmrEnable"; break;
            case kParamIndexAudioWmaVersion: paramStr << "AudioWmaVersion"; break;
            case kParamIndexAudioWmaConfig: paramStr << "AudioWmaConfig"; break;
            case kParamIndexAudioHwState: paramStr << "AudioHwState" ; break;
            case kParamIndexAudioMinBlockSize: paramStr << "AudioMinBlockSize"; break;
            case kParamIndexAudioMaxBlockSize: paramStr << "AudioMaxBlockSize"; break;
            case kParamIndexAudioMinVocoderRate: paramStr << "AudioMinVocoderRate"; break;
            case kParamIndexAudioMaxVocoderRate: paramStr << "AudioMaxVocoderRate"; break;
            case kParamIndexAudioNumOutSamplesPerChannel:
                    paramStr << "AudioNumOutSamplesPerChannel";
                    break;
            case kParamIndexAudioEnd: paramStr << "AudioEnd"; break; // DO NOT ADD any case below this
            default: paramStr << "UNKNOWN"; break;
        }
        return paramStr.str();
    }
};

}  // namespace qc2audio

#endif  // _QC2AUDIO_CONFIG_H_


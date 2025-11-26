/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_CONSTANTS_H_
#define _QC2_CONSTANTS_H_

#include <string>
#include <C2BufferBase.h>
#include <C2Component.h>

namespace qc2 {

/// @addtogroup constants Global Constants
/// @{

/**
 * Pixel formats
 */
struct PixFormat {
    // declarations
    static constexpr const uint32_t INVALID = 0;
    ///< canonical YUV 4:2:0 Planar (I420) = HAL_PIXEL_FORMAT_YCBCR_420_888
    static constexpr const uint32_t YUV420P = 35;
    ///< canonical YVU 4:2:0 Planar (YV12) = HAL_PIXEL_FORMAT_YV12
    static constexpr const uint32_t YV12 = 842094169;
    ///< < canonical YUV 4:2:0 Semi-planar (P010) = HAL_PIXEL_FORMAT_YCBCR_P010
    static constexpr const uint32_t YCBCR_420_P010 = 54;
    ///< canonical YUV 4:2:0 Semi-planar (NV12) = HAL_PIXEL_FORMAT_YCbCr_420_SP
    static constexpr const uint32_t YUV420SP = 0x109;
    ///< RGB-Alpha 8 bit per channel = HAL_PIXEL_FORMAT_RGBA_8888
    static constexpr const uint32_t RGBA8888 = 1;
    ///< RGBA 8 bit compressed (NOTE: this is internal and not recognized by gralloc)
    static constexpr const uint32_t RGBA8888_UBWC = 0xC2000000;
    ///< NV12 EXT with 128 width and height alignment = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS
    static constexpr const uint32_t VENUS_NV12 = 0x7FA30C04;
    ///< NV12 EXT with UBWC compression = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS_UBWC
    static constexpr const uint32_t VENUS_NV12_UBWC = 0x7FA30C06;
    ///< NV12 EXT with 512 width and height alignment (used by HEIC tile encoder)
    static constexpr const uint32_t VENUS_NV12_512 = 0x116 /*HAL_PIXEL_FORMAT_NV12_HEIF*/;

    ///< 10-bit Tightly-packed and compressed YUV = HAL_PIXEL_FORMAT_YCbCr_420_TP10_UBWC
    static constexpr const uint32_t VENUS_TP10 = 0x7FA30C09;
    ///< Venus 10-bit YUV 4:2:0 Planar format = HAL_PIXEL_FORMAT_YCbCr_420_P010_VENUS
    static constexpr const uint32_t VENUS_P010 = 0x7FA30C0A;

    ///< MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface (aka Surface) format
    static constexpr const uint32_t SURFACE = 0x7f000789;
    ///< Opaque (aka Native Surface) format = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED
    static constexpr const uint32_t OPAQUE = 34;
    ///< Opaque (aka Native Surface) format : Legacy variant = HAL_PIXEL_FORMAT_NV12_ENCODEABLE
    static constexpr const uint32_t NV12_ENCODEABLE = 0x102;
    ///< SuperBuffer Format. Maps to VENUS_NV12 = HAL_PIXEL_FORMAT_NV12_LINEAR_FLEX
    static constexpr const uint32_t NV12_LINEAR_FLEX = 0x125;
    ///< SuperBuffer Format. Maps to VENUS_NV12_UBWC = HAL_PIXEL_FORMAT_NV12_UBWC_FLEX
    static constexpr const uint32_t NV12_UBWC_FLEX = 0x126;
    ///< SuperBuffer Format. Maps to VENUS_NV12_UBWC = HAL_PIXEL_FORMAT_NV12_UBWC_FLEX_2_BATCH
    static constexpr const uint32_t NV12_UBWC_FLEX_2_BATCH = 0x128;
    ///< SuperBuffer Format. Maps to VENUS_NV12_UBWC = HAL_PIXEL_FORMAT_NV12_UBWC_FLEX_4_BATCH
    static constexpr const uint32_t NV12_UBWC_FLEX_4_BATCH = 0x129;
    ///< SuperBuffer Format. Maps to VENUS_NV12_UBWC = HAL_PIXEL_FORMAT_NV12_UBWC_FLEX_8_BATCH
    static constexpr const uint32_t NV12_UBWC_FLEX_8_BATCH = 0x130;
    ///< VENUS_NV21 Format = HAL_PIXEL_FORMAT_YCrCb_420_SP_VENUS
    static constexpr const uint32_t VENUS_NV21 = 0x114;
    ///< CbYCrY Format = HAL_PIXEL_FORMAT_CbYCrY_422_I
    static constexpr const uint32_t CbYCrY = 0x120;

    static const char * Str(uint32_t);

    static bool IsCompressed(uint32_t fmt);

    static uint32_t GetUncompressedFor(uint32_t fmt);
};

/**
 * mime types
 */
struct MimeType {
    static constexpr const char * AVC = "video/avc";
    static constexpr const char * H263 = "video/3gpp";
    static constexpr const char * HEVC = "video/hevc";
    static constexpr const char * VP8 = "video/x-vnd.on2.vp8";
    static constexpr const char * VP9 = "video/x-vnd.on2.vp9";
    static constexpr const char * MPEG2 = "video/mpeg2";
    static constexpr const char * MPEG4 = "video/mp4v-es";
    static constexpr const char * VC1 = "video/x-ms-wmv";
    static constexpr const char * RAW = "video/raw";
    static constexpr const char * HEIC = "image/vnd.android.heic";
};

constexpr uint32_t MimeLen = 100;
/**
 *  Codec implementation type
 */
enum CodecImplType : uint32_t {
    IMPL_V4L2 = 1,      ///< V4L2 based implementation
    IMPL_SW_DSP = 2,    ///< DSP based SW implementation
    IMPL_VPP = 3,       ///< VPP filter
    IMPL_MOCK = 4,      ///< Mockup
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
    // TODO(PC): gralloc seems to allocate undefined pixel-format if YUV420SP is requested.
    //           Force USAGE_HW_VIDEO_ENCODER till this is fixed.
    static constexpr const uint64_t HW_CODEC_WRITE_CACHED = HW_CODEC_READ | C2MemoryUsage::CPU_READ;
    static constexpr const uint64_t HW_CODEC_WRITE        = HW_CODEC_READ | CPU_WRITE_UNCACHED;
    static constexpr const uint64_t HW_PROTECTED          = C2MemoryUsage::READ_PROTECTED;
    /*
    * HW_CODEC is QC internal flag to indicate GPU will be required.
    * If this flag is set then gralloc will give a gpu alligned buffer
    */
    static constexpr const uint64_t HW_CODEC           = (uint64_t(1u) << 52);

    static constexpr const uint64_t HW_TEXTURE          = 0x00000100U;
    static constexpr const uint64_t HW_RENDER           = 0x00000200U;

    static constexpr const uint64_t VULKAN_USAGE        = (HW_TEXTURE | HW_RENDER | HW_CODEC);

    // static const char* Str(uint64_t);
};

/**
 *  Consumer usage
 * ----------------------------------------------------------
 * REQUEST_UBWC |   REQUEST_10BIT   |   COLOR FORMAT        |
 * ---------------------------------------------------------|
 *      0       |        0          |     VENUS_NV12        |
 *      0       |        1          |     VENUS_P010        |
 *      1       |        0          |     VENUS_NV12_UBWC   |
 *      1       |        1          |     VENUS_TP10        |
 * ----------------------------------------------------------
 */
struct ConsumerUsage {
    static constexpr const uint64_t REQUEST_UBWC    = (uint32_t(1u) << 28);
    static constexpr const uint64_t IMAGE_ENCODER   = 0x08000000U;
    static constexpr const uint64_t REQUEST_10BIT   = (uint32_t(1u) << 30);
};

/**
 * Buffer flags. Will be added-to/retrieved-from QC2Buffer::info()
 */
struct BufFlag {
    static constexpr const uint64_t CODEC_CONFIG    = 0x0001;   ///< Buffer contains header (only)
    static constexpr const uint64_t EOS             = 0x0002;   ///< End Of Stream
    static constexpr const uint64_t I_FRAME         = 0x0004;   ///< Key-frame
    static constexpr const uint64_t P_FRAME         = 0x0008;   ///< P-frame
    static constexpr const uint64_t B_FRAME         = 0x0010;   ///< B-frame
    static constexpr const uint64_t CORRUPT         = 0x0020;   ///< Input data is corrupt
    // INPUT_PENDING: Signaled with output buffer to indicate that corresponding input will
    // result in further outputs (eg: packed frame)
    static constexpr const uint64_t INPUT_PENDING   = 0x0040;   ///< Input produces more outputs
    static constexpr const uint64_t EMPTY           = 0x0080;   ///< Buffer not produced
    static constexpr const uint64_t REFERENCED      = 0x0100;   ///< Buffer still being referenced
    static constexpr const uint64_t UPDATE_INPUT_TS = 0x0200;   ///< Ovewrite input TS (for AVTimer)
    static std::string Str(uint64_t);
};

/// @}

};  // namespace qc2

#endif  // _QC2_CONSTANTS_H_

/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_CONFIG_H_
#define _QC2_CONFIG_H_

#include <sstream>
#include <unordered_map>

//Below conditional compilation code is added to avoid
//global symbols for v4l2codec plugin.
#if defined(_LINUX_VENV_) || defined(_AGL_LINUX_)
#ifdef __C2_GENERATE_GLOBAL_VARS__
#undef __C2_GENERATE_GLOBAL_VARS__
#define __REDEF_C2_GEN_GLOBAL_VARS__
#endif
#include <C2Config.h>
#ifdef __REDEF_C2_GEN_GLOBAL_VARS__
#define __C2_GENERATE_GLOBAL_VARS__
#endif
#else
#include <C2Config.h>
#endif // end _AGL_LINUX_

// C2ENUMs work in global namespace - define them here

namespace qc2 {

// Start offsets for the extension parameters for different "Codec domains".
// C2Param::Index has 15 bits available for accomodating vendor parameter indices
//
// TYPE_INDEX_VENDOR_START    +------> Range of indices for each codec domain (ParamIndexRangeMask)
//             |              |
//             v         |<----->|
//             1000 0000 0000 0000
//              ^^^
//              |||
//              ||+--> custom codec parameter
//              ||
//              |+---> VPP parameter
//              |
//              +----> V4L2 parameter
//
constexpr uint32_t kQC2CodecParamIndexRangeMask = 0x00FF;
constexpr uint32_t kQC2CodecParamIndexDomainMask = 0x7000;
constexpr uint32_t kQC2CommonVendorParamIndexStart = C2Param::TYPE_INDEX_VENDOR_START; // 0x8000
constexpr uint32_t kQC2V4L2VendorParamIndexStart   = C2Param::TYPE_INDEX_VENDOR_START | 0x4000;
constexpr uint32_t kQC2VPPVendorParamIndexStart    = C2Param::TYPE_INDEX_VENDOR_START | 0x2000;
constexpr uint32_t kQC2CustomVendorParamIndexStart = C2Param::TYPE_INDEX_VENDOR_START | 0x1000;

// param indices for common vendor parameters
enum QC2CommonVendorParamIndexKind : C2Param::type_index_t {
    kParamIndexVideoFilterEnabledSampleParam = kQC2CommonVendorParamIndexStart,
    kParamIndexVideoForceNonUBWCFormat,
    kParamIndexVideoInterlaceInfo,
};

enum InterlaceType : uint32_t;
const char *Str(InterlaceType s);

}   // namespace qc2

C2ENUM(qc2::InterlaceType, uint32_t,
    INTERLACE_NONE = 0,                         ///< progressive
    INTERLACE_INTERLEAVED_TOP_FIRST = 1,        ///< line-interleaved. top-field-first
    INTERLACE_INTERLEAVED_BOTTOM_FIRST = 2,     ///< line-interleaved. bottom-field-first
    INTERLACE_FIELD_TOP_FIRST = 3,              ///< field-sequential. top-field-first
    INTERLACE_FIELD_BOTTOM_FIRST = 4,           ///< field-sequential. bottom-field-first
);

namespace qc2 {

typedef C2GlobalParam<C2Setting, C2BoolValue, kParamIndexVideoFilterEnabledSampleParam>
                    C2FilterEnabledSampleParam;
constexpr char QC2_PARAMKEY_VIDEO_FILTER_ENABLED_SAMPLE_PARAM[] = "filter.enabled.sample";


typedef C2StreamParam<C2Setting, C2BoolValue, kParamIndexVideoForceNonUBWCFormat>
                    C2VideoForceNonUBWCFormatParam;
constexpr char QC2_PARAMKEY_VIDEO_FORCENONUBWCFORMAT[] = "qti-ext-dec-forceNonUBWC";

/********** Interlace Info************/
struct C2VideoInterlaceInfoStruct {
    uint32_t format;        ///< interlace type
    uint32_t color_format;  ///< pixel format
    uint32_t mbaff;         ///< 1 => MBAFF
    uint32_t deinterlaced;  ///< whether the video format is deinterlaced

    C2VideoInterlaceInfoStruct()
        : format(InterlaceType::INTERLACE_NONE), color_format(0u), mbaff(0u), deinterlaced(0u) {
    }
    DEFINE_AND_DESCRIBE_BASE_C2STRUCT(VideoInterlaceInfo)
    C2FIELD(format, "format")
    C2FIELD(color_format, "color_format")
    C2FIELD(mbaff, "mbaff")
    C2FIELD(deinterlaced, "deinterlaced")
};
typedef C2StreamParam<C2Info, C2VideoInterlaceInfoStruct, kParamIndexVideoInterlaceInfo>
       C2VideoInterlaceInfo;
constexpr char QC2_PARAMKEY_INTERLACE_INFO[] = "qti-ext-dec-info-interlace";

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
    static const std::string C2Param(uint32_t id);
};

}   // namespace qc2

#endif // _QC2_CONFIG_H_


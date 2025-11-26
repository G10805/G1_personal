#ifndef CODEC2_CODECS_FILTER_INCLUDE_QC2COLORCONVERTFILTERCONFIG_H_
#define CODEC2_CODECS_FILTER_INCLUDE_QC2COLORCONVERTFILTERCONFIG_H_
/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#include "QC2Config.h"

namespace qc2 {
enum QC2ColorConvertFilterVendorExtensionParamIndexKind : C2Param::type_index_t;
}

// param indices for vendor extensions
C2ENUM(qc2::QC2ColorConvertFilterVendorExtensionParamIndexKind, C2Param::type_index_t,
    kParamIndexColorConvEnable = C2Param::TYPE_INDEX_VENDOR_START + 0x600
);

namespace qc2 {
typedef C2GlobalParam<C2Setting, C2BoolValue, kParamIndexColorConvEnable> QC2ColorConvertEnableInfo;

constexpr char QC2_PARAMKEY_COLORCONV_FILTER_ENABLED[] = "qti-ext-colorconv-filter-enabled";

struct ColorConvFilterDebugString {
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
            case kParamIndexColorConvEnable: paramStr << "ColConvEnable"; break;
            case kParamIndexVideoForceNonUBWCFormat: paramStr << "ForceLinearPixelFmt"; break;
            default: paramStr << "UNKNOWN"; break;
        }
        return paramStr.str();
    }
};

}
#endif  // CODEC2_CODECS_FILTER_INCLUDE_QC2COLORCONVERTFILTERCONFIG_H_

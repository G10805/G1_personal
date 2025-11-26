/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_PLATFORM_H_
#define _QC2AUDIO_PLATFORM_H_

#include "QC2.h"

namespace qc2audio {

/**
 * Device specific magic
 */
struct Platform {
    /**
     * Group of methods for deriving platform-specific ion flags and heaps for different usages
     */
    struct IonInfo {
        /**
         * secure domains
         */
        enum SecureDomain {
            SECURE_NONE,                ///< non-secure
            SECURE_DOMAIN_BITSTREAM,    ///< secure bitstream
            SECURE_DOMAIN_PIXEL,        ///< secure pixel-data
        };

        static uint32_t HeapMaskFor(SecureDomain domain);

        static uint32_t AllocFlagsFor(SecureDomain domain);

        static uint32_t AlignmentFor(SecureDomain domain);
    };

    struct VenusBufferLayout {
        enum PlaneID {
            PLANE_Y = 0,
            PLANE_UV = 1,
            PLANE_Y_META = 2,
            PLANE_UV_META = 3,

            PLANE_RGB = 4,
            PLANE_RGB_META = 5,

            PLANE_MAX = PLANE_UV_META,
        };

        static size_t AllocSize(uint32_t pixFmt, size_t width, size_t height);  /// bytes
        static size_t Stride(uint32_t pixFmt, PlaneID plane, size_t width);
        static size_t Scanlines(uint32_t pixFmt, PlaneID plane, size_t height);
        static size_t Offset(uint32_t pixFmt, PlaneID plane, size_t width, size_t height);
    };

};

}   // namespace qc2audio

#endif  // _QC2AUDIO_PLATFORM_H_

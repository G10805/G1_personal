/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2Platform.h"
#include <linux/ion.h>
#include <linux/msm_ion.h>

namespace qc2audio {

// static
uint32_t Platform::IonInfo::HeapMaskFor(SecureDomain domain) {
    switch (domain) {
        case SECURE_NONE:
            return ION_HEAP(ION_SYSTEM_HEAP_ID);

        case SECURE_DOMAIN_BITSTREAM:
            return ION_HEAP(ION_SECURE_HEAP_ID) | ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID);

        case SECURE_DOMAIN_PIXEL:
            return ION_HEAP(ION_SECURE_HEAP_ID);

        default:
            return 0;
    }
}

// static
uint32_t Platform::IonInfo::AllocFlagsFor(SecureDomain domain) {
    switch (domain) {
        case SECURE_NONE:
            // return ION_FLAG_CACHED;
               return 0; //uncached

        case SECURE_DOMAIN_BITSTREAM:
            return (ION_FLAG_SECURE | ION_FLAG_CP_BITSTREAM);

        case SECURE_DOMAIN_PIXEL:
            return (ION_FLAG_SECURE | ION_FLAG_CP_PIXEL);

        default:
            return 0;
    }
}

// static
uint32_t Platform::IonInfo::AlignmentFor(SecureDomain domain) {
    (void)domain;
    return 4096;
}

}   // namespace qc2audio


/*!
 * @file IPasrManager.hal
 *
 * @cr
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services Defines the external interface for PASR Manager.
 */


// FIXME Remove this file if you don't need to translate types in this backend.

#include "vendor/qti/memory/pasrmanager/translate-ndk.h"

namespace android::h2a {

static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::ERROR == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::ERROR));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::INCOMPLETE_ONLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::INCOMPLETE_ONLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::INCOMPLETE_OFFLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::INCOMPLETE_OFFLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::OFFLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::OFFLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::ONLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::ONLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrStatus::SUCCESS == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrStatus>(::vendor::qti::memory::pasrmanager::V1_0::PasrStatus::SUCCESS));

static_assert(aidl::vendor::qti::memory::pasrmanager::PasrState::MEMORY_ONLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrState>(::vendor::qti::memory::pasrmanager::V1_0::PasrState::MEMORY_ONLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrState::MEMORY_OFFLINE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrState>(::vendor::qti::memory::pasrmanager::V1_0::PasrState::MEMORY_OFFLINE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrState::MEMORY_UNKNOWN == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrState>(::vendor::qti::memory::pasrmanager::V1_0::PasrState::MEMORY_UNKNOWN));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrState::MAX_STATE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrState>(::vendor::qti::memory::pasrmanager::V1_0::PasrState::MAX_STATE));

static_assert(aidl::vendor::qti::memory::pasrmanager::PasrSrc::PASR_SRC_PSI == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrSrc>(::vendor::qti::memory::pasrmanager::V1_1::PasrSrc_1_1::PASR_SRC_PSI));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrSrc::PASR_SRC_POWER == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrSrc>(::vendor::qti::memory::pasrmanager::V1_1::PasrSrc_1_1::PASR_SRC_POWER));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrSrc::PASR_SRC_PERF == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrSrc>(::vendor::qti::memory::pasrmanager::V1_1::PasrSrc_1_1::PASR_SRC_PERF));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrSrc::PASR_SRC_HAL == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrSrc>(::vendor::qti::memory::pasrmanager::V1_1::PasrSrc_1_1::PASR_SRC_HAL));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrSrc::PASR_SRC_MAX == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrSrc>(::vendor::qti::memory::pasrmanager::V1_1::PasrSrc_1_1::PASR_SRC_MAX));

static_assert(aidl::vendor::qti::memory::pasrmanager::PasrPriority::PASR_PRI_NONE == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrPriority>(::vendor::qti::memory::pasrmanager::V1_0::PasrPriority::PASR_PRI_NONE));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrPriority::PASR_PRI_LOW == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrPriority>(::vendor::qti::memory::pasrmanager::V1_0::PasrPriority::PASR_PRI_LOW));
static_assert(aidl::vendor::qti::memory::pasrmanager::PasrPriority::PASR_PRI_CRITICAL == static_cast<aidl::vendor::qti::memory::pasrmanager::PasrPriority>(::vendor::qti::memory::pasrmanager::V1_0::PasrPriority::PASR_PRI_CRITICAL));


__attribute__((warn_unused_result)) bool translate(const ::vendor::qti::memory::pasrmanager::V1_0::PasrInfo& in, aidl::vendor::qti::memory::pasrmanager::PasrInfo* out) {
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.ddr_size > std::numeric_limits<int32_t>::max() || in.ddr_size < 0) {
        return false;
    }
    out->ddr_size = static_cast<int32_t>(in.ddr_size);
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.granule > std::numeric_limits<int32_t>::max() || in.granule < 0) {
        return false;
    }
    out->granule = static_cast<int32_t>(in.granule);
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.num_blocks > std::numeric_limits<int32_t>::max() || in.num_blocks < 0) {
        return false;
    }
    out->num_blocks = static_cast<int32_t>(in.num_blocks);
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.min_free_mem > std::numeric_limits<int64_t>::max() || in.min_free_mem < 0) {
        return false;
    }
    out->min_free_mem = static_cast<int64_t>(in.min_free_mem);
    return true;
}

}  // namespace android::h2a

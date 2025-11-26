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

#pragma once

#include "vendor/qti/memory/pasrmanager/1.0/types.h"
#include "vendor/qti/memory/pasrmanager/1.1/types.h"
#include "vendor/qti/memory/pasrmanager/PasrInfo.h"
#include "vendor/qti/memory/pasrmanager/PasrPriority.h"
#include "vendor/qti/memory/pasrmanager/PasrSrc.h"
#include "vendor/qti/memory/pasrmanager/PasrState.h"
#include "vendor/qti/memory/pasrmanager/PasrStatus.h"
#include <limits>

namespace android::h2a {

__attribute__((warn_unused_result)) bool translate(const ::vendor::qti::memory::pasrmanager::V1_0::PasrInfo& in, vendor::qti::memory::pasrmanager::PasrInfo* out);

}  // namespace android::h2a

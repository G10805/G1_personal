/*
**************************************************************************************************
* Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**************************************************************************************************
*/

/*
**************************************************************************************************
* Copyright (c) 2010-2020, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************
*/

#include <string>
#include "QC2PerfController.h"

namespace qc2audio {

QC2PerfController::QC2PerfController(const std::string& instName,
                                     ComponentKind kind)
    :mPerfLibHandle(nullptr),
     mPerfLockHandle(0),
     mPerfLockAcquire(nullptr),
     mPerfLockRelease(nullptr),
     mInstName(instName),
     mComponentkind(kind) {
      QLOGV_INST("Perf Hint Init: %s(%d)", Str(kind), kind);
      init();
}

void QC2PerfController::init() {
    char perfLibPath[PROPERTY_VALUE_MAX] = {0};

    if (perfLibPath[0] == 0 &&
        property_get("ro.vendor.extension_library", perfLibPath, NULL) <= 0) {
        QLOGE_INST("Vendor extension for Perf library path does not exist.");
        return;
    }

    if (!mPerfLibHandle &&
        (mPerfLibHandle = dlopen(perfLibPath, RTLD_NOW)) == NULL) {
        QLOGE_INST("Failed to open %s, Error: \"%s\"", perfLibPath, dlerror());
        return;
    }

    if (!mPerfLockAcquire &&
        (mPerfLockAcquire = (tPerfLockAcquire)dlsym(mPerfLibHandle,
                                                    "perf_lock_acq")) == NULL) {
        QLOGE_INST("Could notfind symbol \"perf_lock_acq\" in %s", perfLibPath);
        return;
    }

    if (!mPerfLockRelease &&
        (mPerfLockRelease = (tPerfLockRelease)dlsym(mPerfLibHandle,
                                                    "perf_lock_rel")) == NULL) {
        QLOGE_INST("Could notfind symbol \"perf_lock_rel\" in %s", perfLibPath);
        return;
    }

    QLOGV_INST("Perf Control is enabled");
}

QC2PerfController::~QC2PerfController() {
    if (mPerfLibHandle) {
        dlclose(mPerfLibHandle);
    }
}

bool QC2PerfController::acquire() {
    std::vector<int> hints = {0, MPCTLV3_VIDEO_DECODE_PB_HINT, MPCTLV3_VIDEO_ENCODE_PB_HINT};
    std::vector<int> vArgs = {hints[mComponentkind], 1};

    if (mPerfLockHandle > 0) {
        QLOGE_INST("Perf Lock has already been acquired.");
        return false;
    }

    if (mPerfLockHandle <= 0 &&  // Check if mPerfLockHandle is valid
                                 // =0 - First time called
                                 //      or after a successful release
                                 // <0 - Last acquire failed.
        mPerfLockAcquire &&      // Check if the perf library has acquire API
        (mPerfLockHandle = mPerfLockAcquire(0, 0, vArgs.data(),
                                            vArgs.size())) < 0) {
        QLOGE_INST("Perf Lock Acquire failed.");
        return false;
    }

    // If perf is not enabled or library loading failed, return true.
    QLOGV_INST("Perf Lock %s(%#X) %s.", Str(mComponentkind), hints[mComponentkind],
               mPerfLockAcquire? "acquired":"control is not enabled");
    return true;
}

void QC2PerfController::release() {
    if (mPerfLockHandle < 0) {
        QLOGE_INST("Last Perf Lock Acquire failed. Resetting the Lock handle.");
        mPerfLockHandle = 0;
        return;
    }

    if (mPerfLockHandle > 0 && mPerfLockRelease) {
        mPerfLockRelease(mPerfLockHandle);
        QLOGV_INST("Perf Lock is released.");
    }
}

}  // namespace qc2audio

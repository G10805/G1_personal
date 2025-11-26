/*
**************************************************************************************************
* Copyright (c) 2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**************************************************************************************************
*/

#ifndef CODEC2_HOOKS_INCLUDE_QC2PERFCONTROLLER_H_
#define CODEC2_HOOKS_INCLUDE_QC2PERFCONTROLLER_H_

#include <string>
#include "QC2.h"
#include "QC2Constants.h"

namespace qc2 {

#define MPCTLV3_VIDEO_DECODE_PB_HINT 0x41C04000
#define MPCTLV3_VIDEO_ENCODE_PB_HINT 0x41C00000

/**
 * @brief Function pointer to resource locking operations
 */
typedef int (*tPerfLockAcquire)(int, int, int*, int);
typedef int (*tPerfLockRelease)(int);


class QC2PerfController {
 private:
        void *mPerfLibHandle;
        int mPerfLockHandle;
        tPerfLockAcquire mPerfLockAcquire;
        tPerfLockRelease mPerfLockRelease;

        std::string mInstName;
        ComponentKind mComponentkind;

        void init();

 public:
        QC2PerfController(const std::string& instName, ComponentKind kind);
        ~QC2PerfController();

        bool acquire();
        void release();
};

}  // namespace qc2

#endif  // CODEC2_HOOKS_INCLUDE_QC2PERFCONTROLLER_H_

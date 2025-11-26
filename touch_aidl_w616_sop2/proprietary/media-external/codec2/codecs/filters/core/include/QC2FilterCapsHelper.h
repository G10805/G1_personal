/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#ifndef _QC2_FILTER_CAPS_HELPER_H_
#define _QC2_FILTER_CAPS_HELPER_H_

#include "QC2CodecCapsHelper.h"

namespace qc2 {

class QC2FilterCapsHelper {
 public:
    static ParamCapsHelperFactories getFilterCapsHelpers() {
        return mSFCapsFactories;
    }
    static ParamCapsHelperFactories mSFCapsFactories;
};
};  // namespace qc2

#endif// _QC2_FILTER_CAPS_HELPER_H_

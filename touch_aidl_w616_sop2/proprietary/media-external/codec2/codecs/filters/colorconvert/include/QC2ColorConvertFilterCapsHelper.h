/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#ifndef _QC2_COLORCONVERT_FILTER_CAPS_HELPER_H_
#define _QC2_COLORCONVERT_FILTER_CAPS_HELPER_H_

#include "QC2CodecCapsHelper.h"

namespace qc2 {

class QC2ColorConvertFilterCapsHelper {
public:
    static ParamCapsHelperFactories getColorConvertFilterCapsHelpers() {
        return mSFCapsFactories;
    }
    static ParamCapsHelperFactories mSFCapsFactories;
};
};  // namespace qc2

#endif// _QC2_COLORCONVERT_FILTER_CAPS_HELPER_H_

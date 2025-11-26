/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamExposureMode;

@VintfStability
parcelable QCarCamExposureConfig {
    QCarCamExposureMode mode;
    int hdrMode;
    int numExposures;
    float[4] exposureTime;
    float[4] exposureRatio;
    float[4] gain;
}

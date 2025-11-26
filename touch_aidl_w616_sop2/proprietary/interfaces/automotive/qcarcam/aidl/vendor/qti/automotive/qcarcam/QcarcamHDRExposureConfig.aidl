/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamExposureMode;

@VintfStability
parcelable QcarcamHDRExposureConfig {
    QcarcamExposureMode exposure_mode_type;
    int hdr_mode;
    int num_exposures;
    float[4] exposure_time;
    float[4] exposure_ratio;
    float[4] gain;
    float target;
    float lux_index;
}

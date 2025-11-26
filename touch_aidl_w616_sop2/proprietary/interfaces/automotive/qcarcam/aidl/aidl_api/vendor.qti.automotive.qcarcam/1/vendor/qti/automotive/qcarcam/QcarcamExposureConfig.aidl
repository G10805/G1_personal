/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamExposureMode;

@VintfStability
parcelable QcarcamExposureConfig {
    QcarcamExposureMode exposure_mode_type;
    float exposure_time;
    float gain;
    float target;
    float lux_index;
}

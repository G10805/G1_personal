/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamFrameDropMode;

@VintfStability
parcelable QcarcamFrameRate {
    QcarcamFrameDropMode frame_drop_mode;
    char frame_drop_period;
    int frame_drop_pattern;
}

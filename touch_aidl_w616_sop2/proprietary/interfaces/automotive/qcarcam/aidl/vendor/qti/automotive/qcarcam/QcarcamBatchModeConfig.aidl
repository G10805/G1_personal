/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamBatchModeType;

@VintfStability
parcelable QcarcamBatchModeConfig {
    QcarcamBatchModeType batch_mode;
    int num_batch_frames;
    int frame_increment;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamBatchMode;

@VintfStability
parcelable QCarCamBatchConfig {
    QCarCamBatchMode mode;
    int numBatchFrames;
    int frameIncrement;
    int detectFirstPhaseTimer;
}

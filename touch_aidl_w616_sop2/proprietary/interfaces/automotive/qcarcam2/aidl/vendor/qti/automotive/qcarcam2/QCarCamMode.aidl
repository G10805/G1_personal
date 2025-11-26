/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamInputSrc;

@VintfStability
parcelable QCarCamMode {
    QCarCamInputSrc[160] sources;
    int numSources;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamGammaType;

@VintfStability
parcelable QCarCamGammaConfig {
    QCarCamGammaType type;
    float fpValue;
    int[128] table;
    int tableSize;
}

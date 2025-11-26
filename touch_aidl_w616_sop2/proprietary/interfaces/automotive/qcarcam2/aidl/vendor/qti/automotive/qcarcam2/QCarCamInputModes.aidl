/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamMode;

@VintfStability
parcelable QCarCamInputModes {
    int currentMode;
    int numModes;
    QCarCamMode[10] Modes;
}

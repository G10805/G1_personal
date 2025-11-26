/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamInputStream;
import vendor.qti.automotive.qcarcam2.QCarCamOpmode;

@VintfStability
parcelable QCarCamOpenParam {
    QCarCamOpmode opMode;
    int priority;
    int flags;
    QCarCamInputStream[4] inputs;
    int numInputs;
    int clientId;
}

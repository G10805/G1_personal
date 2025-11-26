/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
parcelable QCarCamInput {
    int inputId;
    int devId;
    int subdevId;
    String inputName;
    int numModes;
    int flags;
}

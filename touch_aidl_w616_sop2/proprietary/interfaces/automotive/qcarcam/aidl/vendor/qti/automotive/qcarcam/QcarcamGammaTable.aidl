/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
parcelable QcarcamGammaTable {
    int length;
    int[128 /* QcarcamMaxGammaTable:QCARCAM_MAX_GAMMA_TABLE */] arr_gamma;
}

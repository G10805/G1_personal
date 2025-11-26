/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
parcelable QcarcamPlaneInfov2 {
    int width;
    int height;
    int stride;
    int size;
    long hndl;
    int offset;
}

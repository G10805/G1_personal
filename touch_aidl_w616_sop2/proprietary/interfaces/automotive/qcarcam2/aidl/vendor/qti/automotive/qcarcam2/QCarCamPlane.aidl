/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
parcelable QCarCamPlane {
    int width;
    int height;
    int stride;
    int size;
    long memHndl;
    int offset;
}

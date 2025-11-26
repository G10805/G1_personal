/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
parcelable AiScalerRoi {
    boolean enable;
    int xStart;
    int yStart;
    int width;
    int height;
}

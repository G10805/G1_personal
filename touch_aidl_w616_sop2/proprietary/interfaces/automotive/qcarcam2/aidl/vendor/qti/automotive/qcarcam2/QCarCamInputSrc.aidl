/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamColorFmt;

@VintfStability
parcelable QCarCamInputSrc {
    int srcId;
    int width;
    int height;
    QCarCamColorFmt colorFmt;
    float fps;
    int securityDomain;
}

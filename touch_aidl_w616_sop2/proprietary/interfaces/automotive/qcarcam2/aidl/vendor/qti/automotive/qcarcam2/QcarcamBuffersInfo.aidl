/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamColorFmt;
import vendor.qti.automotive.qcarcam2.QCarCamPlane;

@VintfStability
parcelable QcarcamBuffersInfo {
    int id;
    QCarCamColorFmt colorFmt;
    int nBuffers;
    int flags;
    int numPlanes;
    QCarCamPlane[3] planes;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamColorFmt;
import vendor.qti.automotive.qcarcam.QcarcamPlaneInfo;

@VintfStability
parcelable QcarcamBuffersInfo {
    QcarcamColorFmt color_fmt;
    QcarcamPlaneInfo[3] planes;
    int n_planes;
    int flags;
}

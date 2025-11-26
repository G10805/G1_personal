/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamColorFmt;
import vendor.qti.automotive.qcarcam.QcarcamPlaneInfov2;

@VintfStability
parcelable QcarcamBuffersInfov2 {
    QcarcamColorFmt color_fmt;
    QcarcamPlaneInfov2[3] planes;
    int n_planes;
    int flags;
}

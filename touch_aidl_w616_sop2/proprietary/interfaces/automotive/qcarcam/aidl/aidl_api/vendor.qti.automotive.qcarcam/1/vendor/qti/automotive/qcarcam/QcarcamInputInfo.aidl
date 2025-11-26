/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamInputDesc;
import vendor.qti.automotive.qcarcam.QcarcamResolution;

@VintfStability
parcelable QcarcamInputInfo {
    QcarcamInputDesc desc;
    String name;
    String parent_name;
    QcarcamResolution[10] res;
    int num_res;
    int[5] color_fmt;
    int num_color_fmt;
    int flags;
}

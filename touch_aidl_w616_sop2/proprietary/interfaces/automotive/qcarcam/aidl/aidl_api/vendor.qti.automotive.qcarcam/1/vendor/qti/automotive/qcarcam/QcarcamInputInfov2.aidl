/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamInputDesc;
import vendor.qti.automotive.qcarcam.QcarcamMode;

@VintfStability
parcelable QcarcamInputInfov2 {
    QcarcamInputDesc desc;
    String name;
    String parent_name;
    QcarcamMode[10] modes;
    int num_modes;
    int flags;
}

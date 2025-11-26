/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamFrameField;

@VintfStability
parcelable QcarcamFrameInfov2 {
    int id;
    int idx;
    int flags;
    int[4] seq_no;
    long timestamp;
    long timestamp_system;
    long[4] sof_qtimestamp;
    QcarcamFrameField field_type;
}

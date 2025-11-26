/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
parcelable QcarcamFrameFreeze {
    int state;
    int seq_no;
    int[16] data;
}

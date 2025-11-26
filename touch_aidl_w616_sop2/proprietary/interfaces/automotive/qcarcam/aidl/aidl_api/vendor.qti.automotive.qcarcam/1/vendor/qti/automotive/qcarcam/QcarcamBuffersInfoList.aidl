/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamBuffersInfov2;

@VintfStability
parcelable QcarcamBuffersInfoList {
    int id;
    QcarcamBuffersInfov2[20] BuffersInfo;
    int n_buffers;
}

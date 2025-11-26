/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamFrameFreeze;
import vendor.qti.automotive.qcarcam.QcarcamFrameInfov2;
import vendor.qti.automotive.qcarcam.QcarcamTimestamp;
import vendor.qti.automotive.qcarcam.QcarcamVendorParam;

@VintfStability
// FIXME Any discriminators should be removed since they are automatically added.
union QcarcamEventPayload {
    int uint_payload;
    QcarcamTimestamp sof_timestamp;
    QcarcamFrameFreeze frame_freeze;
    QcarcamVendorParam vendor_data;
    QcarcamFrameInfov2 frame_info;
    int[64] array;
}

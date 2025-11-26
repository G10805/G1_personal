/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamErrorInfo;
import vendor.qti.automotive.qcarcam2.QCarCamFrameInfo;
import vendor.qti.automotive.qcarcam2.QCarCamHWTimestamp;
import vendor.qti.automotive.qcarcam2.QCarCamRecovery;
import vendor.qti.automotive.qcarcam2.QCarCamVendorParam;

@VintfStability
// FIXME Any discriminators should be removed since they are automatically added.
union QCarCamEventPayload {
    int u32Data;
    QCarCamErrorInfo errInfo;
    QCarCamHWTimestamp hwTimestamp;
    QCarCamVendorParam vendorData;
    QCarCamFrameInfo frameInfo;
    QCarCamRecovery recovery;
    byte[1024] array;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamBatchFramesInfo;
import vendor.qti.automotive.qcarcam2.QCarCamHWTimestamp;
import vendor.qti.automotive.qcarcam2.QCarCamInterlaceField;

@VintfStability
parcelable QCarCamFrameInfo {
    int id;
    int bufferIndex;
    int seqNo;
    long timestamp;
    QCarCamHWTimestamp sofTimestamp;
    int flags;
    QCarCamInterlaceField fieldType;
    int requestId;
    int inputMetaIdx;
    QCarCamBatchFramesInfo[4] batchFramesInfo;
}

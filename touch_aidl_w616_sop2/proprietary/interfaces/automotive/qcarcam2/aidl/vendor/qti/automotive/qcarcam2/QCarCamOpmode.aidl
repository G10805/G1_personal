/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
@Backing(type="int")
enum QCarCamOpmode {
    QCARCAM_OPMODE_RAW_DUMP,
    QCARCAM_OPMODE_ISP,
    QCARCAM_OPMODE_OFFLINE_ISP,
    QCARCAM_OPMODE_PAIRED_INPUT,
    QCARCAM_OPMODE_DEINTERLACE,
    QCARCAM_OPMODE_TRANSFORMER,
    QCARCAM_OPMODE_PREPROCESS_ISP,
    QCARCAM_OPMODE_RDI_CONVERSION,
    QCARCAM_OPMODE_MAX = 0x7FFFFFFF,
}

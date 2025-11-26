/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
@Backing(type="int")
enum QCarCamFrameDropMode {
    QCARCAM_KEEP_ALL_FRAMES,
    QCARCAM_KEEP_EVERY_2FRAMES,
    QCARCAM_KEEP_EVERY_3FRAMES,
    QCARCAM_KEEP_EVERY_4FRAMES,
    QCARCAM_DROP_ALL_FRAMES,
    QCARCAM_FRAMEDROP_MANUAL,
}

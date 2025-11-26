/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
@Backing(type="int")
enum QcarcamEvent {
    QCARCAM_EVENT_FRAME_READY = 1 << 0,
    QCARCAM_EVENT_INPUT_SIGNAL = 1 << 1,
    QCARCAM_EVENT_ERROR = 1 << 2,
    QCARCAM_EVENT_VENDOR = 1 << 3,
    QCARCAM_EVENT_PROPERTY_NOTIFY = 1 << 4,
    QCARCAM_EVENT_FRAME_SOF = 1 << 5,
    QCARCAM_EVENT_RECOVERY = 1 << 6,
    QCARCAM_EVENT_RECOVERY_SUCCESS = 1 << 7,
    QCARCAM_EVENT_ERROR_ABORTED = 1 << 8,
    QCARCAM_EVENT_FRAME_FREEZE = 1 << 9,
}

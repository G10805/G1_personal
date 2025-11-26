/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
@Backing(type="int")
enum QCarCamError {
    QCARCAM_RET_OK = 0,
    QCARCAM_RET_FAILED,
    QCARCAM_RET_BADPARAM,
    QCARCAM_RET_INVALID_OP,
    QCARCAM_RET_BADSTATE,
    QCARCAM_RET_NOT_PERMITTED,
    QCARCAM_RET_OUT_OF_BOUND,
    QCARCAM_RET_TIMEOUT,
    QCARCAM_RET_NOMEM,
    QCARCAM_RET_UNSUPPORTED,
    QCARCAM_RET_BUSY,
    QCARCAM_RET_NOT_FOUND,
    QCARCAM_RET_LAST = 255,
}

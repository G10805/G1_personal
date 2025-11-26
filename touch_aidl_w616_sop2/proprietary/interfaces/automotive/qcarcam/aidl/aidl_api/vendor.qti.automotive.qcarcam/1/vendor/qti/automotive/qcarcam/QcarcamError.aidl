/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
@Backing(type="int")
enum QcarcamError {
    QCARCAM_OK = 0,
    QCARCAM_FAILED,
    QCARCAM_BADPARAM,
    QCARCAM_BADSTATE,
    QCARCAM_NOMEM,
    QCARCAM_UNSUPPORTED,
    QCARCAM_TIMEOUT,
}

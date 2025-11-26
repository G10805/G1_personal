/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
@Backing(type="int")
enum QcarcamFrameField {
    QCARCAM_FIELD_UNKNOWN = 0,
    QCARCAM_FIELD_ODD,
    QCARCAM_FIELD_EVEN,
    QCARCAM_FIELD_ODD_EVEN,
    QCARCAM_FIELD_EVEN_ODD,
}

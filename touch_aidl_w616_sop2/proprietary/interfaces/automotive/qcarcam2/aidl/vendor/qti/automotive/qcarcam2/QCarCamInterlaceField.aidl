/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
@Backing(type="int")
enum QCarCamInterlaceField {
    QCARCAM_INTERLACE_FIELD_NONE = 0,
    QCARCAM_INTERLACE_FIELD_UNKNOWN,
    QCARCAM_INTERLACE_FIELD_ODD,
    QCARCAM_INTERLACE_FIELD_EVEN,
    QCARCAM_INTERLACE_FIELD_ODD_EVEN,
    QCARCAM_INTERLACE_FIELD_EVEN_ODD,
    QCARCAM_INTERLACE_FIELD_MAX = 0x7FFFFFFF,
}

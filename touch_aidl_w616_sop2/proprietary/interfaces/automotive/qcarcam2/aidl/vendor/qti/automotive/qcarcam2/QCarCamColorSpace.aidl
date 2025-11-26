/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

/**
 * @brief Supported color space formats
 */
@VintfStability
@Backing(type="int")
enum QCarCamColorSpace {
    QCARCAM_COLOR_SPACE_UNCORRECTED = 0,
    QCARCAM_COLOR_SPACE_SRGB,
    QCARCAM_COLOR_SPACE_LRGB,
    QCARCAM_COLOR_SPACE_BT601,
    QCARCAM_COLOR_SPACE_BT601_FULL,
    QCARCAM_COLOR_SPACE_BT709,
    QCARCAM_COLOR_SPACE_BT709_FULL,
}

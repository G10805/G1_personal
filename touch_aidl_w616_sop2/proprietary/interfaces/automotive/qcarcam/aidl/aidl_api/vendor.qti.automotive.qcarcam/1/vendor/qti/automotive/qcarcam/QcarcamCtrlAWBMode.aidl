/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

@VintfStability
@Backing(type="int")
enum QcarcamCtrlAWBMode {
    QCARCAM_HAL_AWB_MODE_OFF,
    QCARCAM_HAL_AWB_MODE_AUTO,
    QCARCAM_HAL_AWB_MODE_INCANDESCENT,
    QCARCAM_HAL_AWB_MODE_FLUORESCENT,
    QCARCAM_HAL_AWB_MODE_WARM_FLUORESCENT,
    QCARCAM_HAL_AWB_MODE_DAYLIGHT,
    QCARCAM_HAL_AWB_MODE_CLOUDY_DAYLIGHT,
    QCARCAM_HAL_AWB_MODE_TWILIGHT,
    QCARCAM_HAL_AWB_MODE_SHADE,
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamCtrlAEAntibandingMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlAELock;
import vendor.qti.automotive.qcarcam.QcarcamCtrlAEMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlAWBLock;
import vendor.qti.automotive.qcarcam.QcarcamCtrlAWBMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlControlEffectMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlControlMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlControlSceneMode;
import vendor.qti.automotive.qcarcam.QcarcamCtrlAeRegions;

@VintfStability
parcelable QcarcamIspCtrls {
    long param_mask;
    QcarcamCtrlAELock ae_lock;
    QcarcamCtrlAEMode ae_mode;
    QcarcamCtrlAWBLock awb_lock;
    QcarcamCtrlAWBMode awb_mode;
    QcarcamCtrlControlEffectMode effect_mode;
    QcarcamCtrlControlMode ctrl_mode;
    QcarcamCtrlControlSceneMode scene_mode;
    QcarcamCtrlAEAntibandingMode ae_antibanding_mode;
    float contrast_level;
    float saturation;
    float ae_compensation;
    QcarcamCtrlAeRegions ae_regions;
}

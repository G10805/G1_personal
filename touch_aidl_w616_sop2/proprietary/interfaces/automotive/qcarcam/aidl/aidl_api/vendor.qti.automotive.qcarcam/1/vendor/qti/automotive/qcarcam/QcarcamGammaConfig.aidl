/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamGamma;
import vendor.qti.automotive.qcarcam.QcarcamGammaType;

@VintfStability
parcelable QcarcamGammaConfig {
    QcarcamGammaType config_type;
    QcarcamGamma gamma;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamGammaTable;

@VintfStability
// FIXME Any discriminators should be removed since they are automatically added.
union QcarcamGamma {
    float f_value;
    QcarcamGammaTable table;
}

/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamIspUsecase;

@VintfStability
parcelable QcarcamIspUsecaseConfig {
    int id;
    int camera_id;
    QcarcamIspUsecase use_case;
}

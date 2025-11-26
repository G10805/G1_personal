/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamIspUsecase;

@VintfStability
parcelable QCarCamIspUsecaseConfig {
    int id;
    int cameraId;
    QCarCamIspUsecase usecaseId;
    int tuningMode;
}

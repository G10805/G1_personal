/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

@VintfStability
@Backing(type="int")
enum QCarCamIspUsecase {
    QCARCAM_ISP_USECASE_PREVIEW,
    QCARCAM_ISP_USECASE_SHDR_PREPROCESS,
    QCARCAM_ISP_USECASE_CUSTOM_START = 0x100,
    QCARCAM_ISP_USECASE_MAX = 0x7FFFFFFF,
}

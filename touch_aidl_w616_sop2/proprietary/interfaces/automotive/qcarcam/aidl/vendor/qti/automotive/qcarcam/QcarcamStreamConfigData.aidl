/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package vendor.qti.automotive.qcarcam;

import vendor.qti.automotive.qcarcam.QcarcamExposureConfig;
import vendor.qti.automotive.qcarcam.QcarcamFrameRate;
import vendor.qti.automotive.qcarcam.QcarcamGammaConfig;
import vendor.qti.automotive.qcarcam.QcarcamHDRExposureConfig;
import vendor.qti.automotive.qcarcam.QcarcamResolution;
import vendor.qti.automotive.qcarcam.QcarcamBatchModeConfig;
import vendor.qti.automotive.qcarcam.QcarcamColorFmt;
import vendor.qti.automotive.qcarcam.QcarcamIspCtrls;
import vendor.qti.automotive.qcarcam.QcarcamIspUsecaseConfig;
import vendor.qti.automotive.qcarcam.QcarcamVendorParam;

@VintfStability
// FIXME Any discriminators should be removed since they are automatically added.
union QcarcamStreamConfigData {
    long ptr_value;
    float float_value;
    int uint_value;
    QcarcamResolution res_value;
    QcarcamColorFmt color_value;
    QcarcamExposureConfig exposure_config;
    QcarcamHDRExposureConfig hdr_exposure_config;
    QcarcamFrameRate frame_rate_config;
    QcarcamGammaConfig gamma_config;
    int[32] arr_padding;
    QcarcamIspCtrls isp_ctrls;
    QcarcamVendorParam vendor_param;
    QcarcamBatchModeConfig batch_config;
    long uint64_value;
    QcarcamIspUsecaseConfig isp_config;
}

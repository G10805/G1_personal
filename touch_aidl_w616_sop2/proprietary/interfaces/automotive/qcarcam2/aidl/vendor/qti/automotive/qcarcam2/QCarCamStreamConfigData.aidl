/*
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.automotive.qcarcam2;

import vendor.qti.automotive.qcarcam2.QCarCamBatchConfig;
import vendor.qti.automotive.qcarcam2.QCarCamColorSpace;
import vendor.qti.automotive.qcarcam2.QCarCamExposureConfig;
import vendor.qti.automotive.qcarcam2.QCarCamFrameDropConfig;
import vendor.qti.automotive.qcarcam2.QCarCamGammaConfig;
import vendor.qti.automotive.qcarcam2.QCarCamInputSignal;
import vendor.qti.automotive.qcarcam2.QCarCamIspUsecaseConfig;
import vendor.qti.automotive.qcarcam2.QCarCamLatencyControl;
import vendor.qti.automotive.qcarcam2.QCarCamVendorParam;
import vendor.qti.automotive.qcarcam2.QCarCamCropRegion;

@VintfStability
union QCarCamStreamConfigData {
    long uint64Value;
    float floatValue;
    QCarCamExposureConfig exposureConfig;
    QCarCamFrameDropConfig frameDropConfig;
    QCarCamGammaConfig gammaConfig;
    QCarCamVendorParam vendorParam;
    QCarCamLatencyControl latencyParam;
    QCarCamBatchConfig batchConfig;
    QCarCamIspUsecaseConfig ispConfig;
    QCarCamInputSignal inputSignal;
    QCarCamColorSpace colorSpace;
    QCarCamCropRegion cropRegion;
}

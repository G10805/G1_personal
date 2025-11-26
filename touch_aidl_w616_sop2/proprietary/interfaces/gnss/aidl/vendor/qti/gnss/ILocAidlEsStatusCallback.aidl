/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.gnss;

@VintfStability
interface ILocAidlEsStatusCallback {
    void onEsStatusChanged(boolean isEmergencyMode);
}

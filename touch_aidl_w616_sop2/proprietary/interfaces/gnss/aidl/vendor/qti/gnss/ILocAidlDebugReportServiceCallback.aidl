/*
* Copyright (c) 2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;
import vendor.qti.gnss.LocAidlXtraStatus;

@VintfStability
interface ILocAidlDebugReportServiceCallback {
    void onXtraStatusChanged(in LocAidlXtraStatus xtraStatus);
}

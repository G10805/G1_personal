/*
* Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlSystemStatusReports;
import vendor.qti.gnss.LocAidlSystemStatusRfAndParams;
import vendor.qti.gnss.ILocAidlDebugReportServiceCallback;

@VintfStability
interface ILocAidlDebugReportService {


    boolean deinit();

    LocAidlSystemStatusReports getReport(in int maxReports);

    /**
     * Interface
     */
    boolean init();
    void registerXtraStatusListener(in ILocAidlDebugReportServiceCallback callback);
    void unregisterXtraStatusListener();
    void getXtraStatus();
}

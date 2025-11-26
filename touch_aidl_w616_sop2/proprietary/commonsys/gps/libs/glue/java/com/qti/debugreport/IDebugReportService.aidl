/* ======================================================================
*  Copyright (c) 2017, 2022 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.debugreport;

import com.qti.debugreport.IDebugReportCallback;
import com.qti.debugreport.IXtraStatusCallback;
import android.os.Bundle;

interface IDebugReportService {

    void registerForDebugReporting (in IDebugReportCallback callback);
    void unregisterForDebugReporting (in IDebugReportCallback callback);

    void startReporting();
    void stopReporting();

    Bundle getDebugReport();
    void registerXtraStatusListener(in IXtraStatusCallback listener);
    void unregisterXtraStatusListener();
    void getXtraStatus();
}

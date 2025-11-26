/* ======================================================================
*  Copyright (c) 2015, 2017, 2022 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.izat;

import android.os.IBinder;
import com.qti.flp.IFlpService;
import com.qti.flp.ITestService;
import com.qti.geofence.IGeofenceService;
import com.qti.debugreport.IDebugReportService;
import com.qti.wifidbreceiver.IWiFiDBReceiver;
import com.qti.wwandbreceiver.IWWANDBReceiver;
import com.qti.gnssconfig.IGnssConfigService;
import com.qti.wifidbprovider.IWiFiDBProvider;
import com.qti.wwandbprovider.IWWANDBProvider;
import com.qti.altitudereceiver.IAltitudeReceiver;
import com.qti.gtp.IGTPService;

interface IIzatService {
    IFlpService getFlpService();
    ITestService getTestService();
    IGeofenceService getGeofenceService();
    String getVersion();
    IDebugReportService getDebugReportService();
    IWiFiDBReceiver getWiFiDBReceiver();
    IWWANDBReceiver getWWANDBReceiver();
    IGnssConfigService getGnssConfigService();
    IWiFiDBProvider getWiFiDBProvider();
    IWWANDBProvider getWWANDBProvider();
    void registerProcessDeath(in IBinder clientDeathListener);
    IAltitudeReceiver getAltitudeReceiver();
    IGTPService getGTPService();
}

/*
* Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlAGnss;
import vendor.qti.gnss.ILocAidlDebugReportService;
import vendor.qti.gnss.ILocAidlFlpService;
import vendor.qti.gnss.ILocAidlGeofenceService;
import vendor.qti.gnss.ILocAidlGnssNi;
import vendor.qti.gnss.ILocAidlIzatProvider;
import vendor.qti.gnss.ILocAidlIzatSubscription;
import vendor.qti.gnss.ILocAidlRilInfoMonitor;
import vendor.qti.gnss.ILocAidlWWANDBReceiver;
import vendor.qti.gnss.ILocAidlWiFiDBReceiver;
import vendor.qti.gnss.ILocAidlAGnss;
import vendor.qti.gnss.ILocAidlFlpService;
import vendor.qti.gnss.ILocAidlGnssConfigService;
import vendor.qti.gnss.ILocAidlWWANDBProvider;
import vendor.qti.gnss.ILocAidlWWANDBReceiver;
import vendor.qti.gnss.ILocAidlWiFiDBProvider;
import vendor.qti.gnss.ILocAidlWiFiDBReceiver;
import vendor.qti.gnss.ILocAidlIzatConfig;
import vendor.qti.gnss.ILocAidlFlpService;
import vendor.qti.gnss.ILocAidlGnssConfigService;
import vendor.qti.gnss.ILocAidlGeocoder;
import vendor.qti.gnss.ILocAidlEsStatusReceiver;
import vendor.qti.gnss.ILocAidlQesdkTracking;

@VintfStability
interface ILocAidlGnss {

    vendor.qti.gnss.ILocAidlAGnss getExtensionLocAidlAGnss();

    vendor.qti.gnss.ILocAidlDebugReportService getExtensionLocAidlDebugReportService();

    ILocAidlFlpService getExtensionLocAidlFlpService();

    vendor.qti.gnss.ILocAidlGeofenceService getExtensionLocAidlGeofenceService();

    ILocAidlGnssConfigService getExtensionLocAidlGnssConfigService();

    vendor.qti.gnss.ILocAidlGnssNi getExtensionLocAidlGnssNi();

    vendor.qti.gnss.ILocAidlIzatConfig getExtensionLocAidlIzatConfig();

    vendor.qti.gnss.ILocAidlIzatProvider getExtensionLocAidlIzatFusedProvider();

    vendor.qti.gnss.ILocAidlIzatProvider getExtensionLocAidlIzatNetworkProvider();

    vendor.qti.gnss.ILocAidlIzatSubscription getExtensionLocAidlIzatSubscription();

    vendor.qti.gnss.ILocAidlRilInfoMonitor getExtensionLocAidlRilInfoMonitor();

    vendor.qti.gnss.ILocAidlWWANDBProvider getExtensionLocAidlWWANDBProvider();

    vendor.qti.gnss.ILocAidlWWANDBReceiver getExtensionLocAidlWWANDBReceiver();

    vendor.qti.gnss.ILocAidlWiFiDBProvider getExtensionLocAidlWiFiDBProvider();

    vendor.qti.gnss.ILocAidlWiFiDBReceiver getExtensionLocAidlWiFiDBReceiver();

    vendor.qti.gnss.ILocAidlGeocoder getExtensionLocAidlGeocoder();

    vendor.qti.gnss.ILocAidlEsStatusReceiver getExtensionLocAidlEsStatusReceiver();

    vendor.qti.gnss.ILocAidlQesdkTracking getExtensionLocAidlQesdkTracking();
}

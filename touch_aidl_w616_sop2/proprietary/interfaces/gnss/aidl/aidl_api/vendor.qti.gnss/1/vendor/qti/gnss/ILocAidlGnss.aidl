/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.qti.gnss;
@VintfStability
interface ILocAidlGnss {
  vendor.qti.gnss.ILocAidlAGnss getExtensionLocAidlAGnss();
  vendor.qti.gnss.ILocAidlDebugReportService getExtensionLocAidlDebugReportService();
  vendor.qti.gnss.ILocAidlFlpService getExtensionLocAidlFlpService();
  vendor.qti.gnss.ILocAidlGeofenceService getExtensionLocAidlGeofenceService();
  vendor.qti.gnss.ILocAidlGnssConfigService getExtensionLocAidlGnssConfigService();
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
}

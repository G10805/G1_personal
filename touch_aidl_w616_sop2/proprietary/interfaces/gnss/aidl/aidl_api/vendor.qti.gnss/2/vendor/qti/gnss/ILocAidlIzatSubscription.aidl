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
interface ILocAidlIzatSubscription {
  void batteryLevelUpdate(in vendor.qti.gnss.LocAidlBatteryLevelDataItem dataItem);
  void boolDataItemUpdate(in vendor.qti.gnss.LocAidlBoolDataItem[] dataItemArray);
  void btClassicScanDataInject(in vendor.qti.gnss.LocAidlBtDeviceScanDetailsDataItem dataItem);
  void btleScanDataInject(in vendor.qti.gnss.LocAidlBtLeDeviceScanDetailsDataItem dataItem);
  void cellCdmaUpdate(in vendor.qti.gnss.LocAidlCellCdmaDataItem dataItem);
  void cellGwUpdate(in vendor.qti.gnss.LocAidlCellGwDataItem dataItem);
  void cellLteUpdate(in vendor.qti.gnss.LocAidlCellLteDataItem dataItem);
  void cellOooUpdate(in vendor.qti.gnss.LocAidlCellOooDataItem dataItem);
  void deinit();
  boolean init(in vendor.qti.gnss.ILocAidlIzatSubscriptionCallback callback);
  void networkinfoUpdate(in vendor.qti.gnss.LocAidlNetworkInfoDataItem dataItem);
  void powerConnectStatusUpdate(in vendor.qti.gnss.LocAidlPowerConnectStatusDataItem dataItem);
  void screenStatusUpdate(in vendor.qti.gnss.LocAidlScreenStatusDataItem dataItem);
  void serviceStateUpdate(in vendor.qti.gnss.LocAidlServiceStateDataItem dataItem);
  void serviceinfoUpdate(in vendor.qti.gnss.LocAidlRilServiceInfoDataItem dataItem);
  void shutdownUpdate();
  void stringDataItemUpdate(in vendor.qti.gnss.LocAidlStringDataItem dataItem);
  void timeChangeUpdate(in vendor.qti.gnss.LocAidlTimeChangeDataItem dataItem);
  void timezoneChangeUpdate(in vendor.qti.gnss.LocAidlTimeZoneChangeDataItem dataItem);
  void wifiSupplicantStatusUpdate(in vendor.qti.gnss.LocAidlWifiSupplicantStatusDataItem dataItem);
}

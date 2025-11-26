/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlBatteryLevelDataItem;
import vendor.qti.gnss.LocAidlBoolDataItem;
import vendor.qti.gnss.LocAidlBtDeviceScanDetailsDataItem;
import vendor.qti.gnss.LocAidlBtLeDeviceScanDetailsDataItem;
import vendor.qti.gnss.ILocAidlIzatSubscriptionCallback;
import vendor.qti.gnss.LocAidlCellCdmaDataItem;
import vendor.qti.gnss.LocAidlCellGwDataItem;
import vendor.qti.gnss.LocAidlCellLteDataItem;
import vendor.qti.gnss.LocAidlCellOooDataItem;
import vendor.qti.gnss.LocAidlNetworkInfoDataItem;
import vendor.qti.gnss.LocAidlPowerConnectStatusDataItem;
import vendor.qti.gnss.LocAidlRilServiceInfoDataItem;
import vendor.qti.gnss.LocAidlScreenStatusDataItem;
import vendor.qti.gnss.LocAidlServiceStateDataItem;
import vendor.qti.gnss.LocAidlStringDataItem;
import vendor.qti.gnss.LocAidlTimeChangeDataItem;
import vendor.qti.gnss.LocAidlTimeZoneChangeDataItem;
import vendor.qti.gnss.LocAidlWifiSupplicantStatusDataItem;

@VintfStability
interface ILocAidlIzatSubscription {
    void batteryLevelUpdate(in LocAidlBatteryLevelDataItem dataItem);

    void boolDataItemUpdate(in LocAidlBoolDataItem[] dataItemArray);

    void btClassicScanDataInject(in LocAidlBtDeviceScanDetailsDataItem dataItem);

    void btleScanDataInject(in LocAidlBtLeDeviceScanDetailsDataItem dataItem);

    void cellCdmaUpdate(in LocAidlCellCdmaDataItem dataItem);

    void cellGwUpdate(in LocAidlCellGwDataItem dataItem);

    void cellLteUpdate(in LocAidlCellLteDataItem dataItem);

    void cellOooUpdate(in LocAidlCellOooDataItem dataItem);

    void deinit();

    boolean init(in ILocAidlIzatSubscriptionCallback callback);

    void networkinfoUpdate(in LocAidlNetworkInfoDataItem dataItem);

    void powerConnectStatusUpdate(in LocAidlPowerConnectStatusDataItem dataItem);

    void screenStatusUpdate(in LocAidlScreenStatusDataItem dataItem);

    void serviceStateUpdate(in LocAidlServiceStateDataItem dataItem);

    void serviceinfoUpdate(in LocAidlRilServiceInfoDataItem dataItem);

    void shutdownUpdate();

    void stringDataItemUpdate(in LocAidlStringDataItem dataItem);

    void timeChangeUpdate(in LocAidlTimeChangeDataItem dataItem);

    void timezoneChangeUpdate(in LocAidlTimeZoneChangeDataItem dataItem);

    void wifiSupplicantStatusUpdate(
        in LocAidlWifiSupplicantStatusDataItem dataItem);
}

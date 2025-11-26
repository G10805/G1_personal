/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlWiFiDBProviderCallback;

@VintfStability
interface ILocAidlWiFiDBProvider {
    boolean init(in ILocAidlWiFiDBProviderCallback callback);

    void registerWiFiDBProvider(in ILocAidlWiFiDBProviderCallback callback);

    void sendAPObsLocDataRequest();

    void unregisterWiFiDBProvider();
}

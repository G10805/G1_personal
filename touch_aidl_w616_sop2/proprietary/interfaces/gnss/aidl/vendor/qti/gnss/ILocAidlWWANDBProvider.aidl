/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlWWANDBProviderCallback;

@VintfStability
interface ILocAidlWWANDBProvider {
    boolean init(in ILocAidlWWANDBProviderCallback callback);

    void registerWWANDBProvider(in ILocAidlWWANDBProviderCallback callback);

    void sendBSObsLocDataRequest();

    void unregisterWWANDBProvider();
}

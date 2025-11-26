/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.ILocAidlIzatProviderCallback;
import vendor.qti.gnss.LocAidlIzatRequest;

@VintfStability
interface ILocAidlIzatProvider {
    void deinit();

    boolean init(in ILocAidlIzatProviderCallback callback);

    boolean onAddRequest(in LocAidlIzatRequest request);

    boolean onDisable();

    boolean onEnable();

    boolean onRemoveRequest(in LocAidlIzatRequest request);
}

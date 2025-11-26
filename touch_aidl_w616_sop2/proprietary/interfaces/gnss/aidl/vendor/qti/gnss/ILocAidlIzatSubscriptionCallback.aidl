/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlSubscriptionDataItemId;
import vendor.qti.gnss.LocAidlBoolDataItem;

@VintfStability
interface ILocAidlIzatSubscriptionCallback {
    void requestData(in LocAidlSubscriptionDataItemId[] l);

    void turnOffModule(in LocAidlSubscriptionDataItemId di);

    void turnOnModule(in LocAidlSubscriptionDataItemId di, in int timeout);

    void unsubscribeAll();

    void updateSubscribe(in LocAidlSubscriptionDataItemId[] l, in boolean subscribe);

    void boolDataItemUpdate(in LocAidlBoolDataItem[] dataItemArray);
}

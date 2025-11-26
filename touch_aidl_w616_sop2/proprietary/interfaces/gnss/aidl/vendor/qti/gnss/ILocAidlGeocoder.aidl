/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocation;
import vendor.qti.gnss.LocAidlAddress;
import vendor.qti.gnss.ILocAidlGeocoderCallback;

@VintfStability
interface ILocAidlGeocoder {
    void setCallback(in ILocAidlGeocoderCallback callback);
    void injectLocationAndAddr(in LocAidlLocation loc, in LocAidlAddress addr);
}

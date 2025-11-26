/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocation;

@VintfStability
interface ILocAidlGeocoderCallback {
    void getAddrFromLocationCb(in LocAidlLocation loc);
}

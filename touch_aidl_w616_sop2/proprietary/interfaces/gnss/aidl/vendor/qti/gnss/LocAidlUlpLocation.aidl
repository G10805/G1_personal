/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocation;

@VintfStability
parcelable LocAidlUlpLocation {
    LocAidlLocation gpsLocation;
    int position_source;
    int tech_mask;
    byte[] raw_data;
    boolean is_indoor;
    float floor_number;
    String map_url;
    String map_index;
}


/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlBsCellInfo;
import vendor.qti.gnss.LocAidlUlpLocation;

@VintfStability
parcelable LocAidlBSObsData {
    long scanTimestamp_ms;
    LocAidlUlpLocation gpsLoc;
    boolean bUlpLocValid;
    LocAidlBsCellInfo cellInfo;
}


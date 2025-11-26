/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

import vendor.qti.gnss.LocAidlLocationExtended;
import vendor.qti.gnss.LocAidlTime;
import vendor.qti.gnss.LocAidlUlpLocation;

/*
 * Various Status Reports
 */
@VintfStability
parcelable LocAidlSystemStatusLocation {
    LocAidlTime mUtcTime;
    LocAidlTime mUtcReported;
    LocAidlUlpLocation mLocation;
    LocAidlLocationExtended mLocationEx;
}


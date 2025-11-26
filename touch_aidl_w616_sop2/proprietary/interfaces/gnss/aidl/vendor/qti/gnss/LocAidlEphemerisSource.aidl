/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlEphemerisSource {
    UNKNOWN = 0,
    DEMODULATED,
    SUPL_PROVIDED,
    OTHER_SERVER_PROVIDED,
    LOCAL_SRC,
}

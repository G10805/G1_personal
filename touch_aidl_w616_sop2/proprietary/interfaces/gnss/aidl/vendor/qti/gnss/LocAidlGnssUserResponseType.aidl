/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="byte")
enum LocAidlGnssUserResponseType {
    RESPONSE_ACCEPT  = 1,
    RESPONSE_DENY    = 2,
    RESPONSE_NORESP  = 3,
}

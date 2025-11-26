/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="byte")
enum LocAidlGnssNiEncodingType {
    ENC_NONE              = 0,
    ENC_SUPL_GSM_DEFAULT  = 1,
    ENC_SUPL_UTF8         = 2,
    ENC_SUPL_UCS2         = 3,
    ENC_UNKNOWN           = -1,
}

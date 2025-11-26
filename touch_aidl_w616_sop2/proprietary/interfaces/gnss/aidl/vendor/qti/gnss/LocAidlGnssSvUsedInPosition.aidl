/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlGnssSvUsedInPosition {
    long gps_sv_used_ids_mask;
    long glo_sv_used_ids_mask;
    long gal_sv_used_ids_mask;
    long bds_sv_used_ids_mask;
    long qzss_sv_used_ids_mask;
}


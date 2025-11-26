/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlApnTypeMask {
    APN_TYPE_MASK_DEFAULT = 1 << 0L,
    APN_TYPE_MASK_IMS = 1 << 1L,
    APN_TYPE_MASK_MMS = 1 << 2L,
    APN_TYPE_MASK_DUN = 1 << 3L,
    APN_TYPE_MASK_SUPL = 1 << 4L,
    APN_TYPE_MASK_HIPRI = 1 << 5L,
    APN_TYPE_MASK_FOTA = 1 << 6L,
    APN_TYPE_MASK_CBS = 1 << 7L,
    APN_TYPE_MASK_IA = 1 << 8L,
    APN_TYPE_MASK_EMERGENCY = 1 << 9L,
}

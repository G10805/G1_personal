/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
@Backing(type="int")
enum LocAidlRobustLocationConfigValidBit {
    ENABLE_BIT = (1 << 0),
    ENABLE_FOR_E911_BIT = (1 << 1),
    VERSION_INFO_BIT = (1 << 2),
}

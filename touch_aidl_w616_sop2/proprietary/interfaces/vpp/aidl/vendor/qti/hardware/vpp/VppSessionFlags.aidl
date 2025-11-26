/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppSessionFlags {
    VPP_SESSION_SECURE = (1 << 0),
    VPP_SESSION_NON_REALTIME = (1 << 1),
}

/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppEventType {
    VPP_EVENT_FLUSH_DONE = (1 << 0),
    VPP_EVENT_RECONFIG_DONE = (1 << 1),
    VPP_EVENT_DRAIN_DONE = (1 << 2),
    VPP_EVENT_ERROR = (1 << 3),
}

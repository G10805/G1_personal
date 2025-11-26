/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppBufferFlag {
    VPP_BUFFER_FLAG_EOS = (1 << 0),
    VPP_BUFFER_FLAG_DATACORRUPT = (1 << 1),
    VPP_BUFFER_FLAG_SYNCFRAME = (1 << 2),
    VPP_BUFFER_FLAG_READONLY = (1 << 3),
    VPP_BUFFER_FLAG_EXTRADATA = (1 << 4),
    VPP_BUFFER_FLAG_B_FRAME = (1 << 5),
    VPP_BUFFER_FLAG_P_FRAME = (1 << 6),
    VPP_BUFFER_FLAG_PENDING_OUTPUT = (1 << 7),
    VPP_BUFFER_FLAG_BYPASS = (1 << 8),
}

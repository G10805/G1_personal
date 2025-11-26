/*
* Copyright (c) 2023 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.vpp;

@VintfStability
@Backing(type="int")
enum VppCodecType {
    VPP_CODEC_TYPE_UNKNOWN,
    VPP_CODEC_TYPE_AVC,
    VPP_CODEC_TYPE_DIVX4,
    VPP_CODEC_TYPE_DIVX,
    VPP_CODEC_TYPE_DIVX311,
    VPP_CODEC_TYPE_MPEG4,
    VPP_CODEC_TYPE_MPEG2,
    VPP_CODEC_TYPE_VC1,
    VPP_CODEC_TYPE_WMV,
    VPP_CODEC_TYPE_H263,
    VPP_CODEC_TYPE_HEVC,
    VPP_CODEC_TYPE_VP8,
    VPP_CODEC_TYPE_VP9,
    VPP_CODEC_TYPE_AV1,
}

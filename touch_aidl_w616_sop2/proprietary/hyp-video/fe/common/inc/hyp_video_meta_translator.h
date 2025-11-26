/*========================================================================

*//** @file hyp_video_meta_translator.h

@par FILE SERVICES:
      The header file of metadata translater for hypervisor video FE


@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/04/22    rq     support 8255 metadata backward compatible with Codec2.1.0
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef __HYP_VIDEO_FE_META_TRANSLATION_H__
#define __HYP_VIDEO_FE_META_TRANSLATION_H__

#include <hyp_video.h>

enum vidc_qmetadata_type
{
    VIDC_QMETADATA_BUFFER_TAG = 0x0300016b,
    VIDC_QMETADATA_END = 0x03001000
};

/* 8155 VPU firmware metadata struct and definations*/
struct msm_vidc_8155_input_crop_payload
{
    uint32 size;
    uint32 version;
    uint32 port_index;
    uint32 left;
    uint32 top;
    uint32 width;
    uint32 height;
};

struct msm_vidc_8155_misr_info
{
    uint32 misr_set;
    uint32 misr_dpb_luma[8];
    uint32 misr_dpb_chroma[8];
    uint32 misr_opb_luma[8];
    uint32 misr_opb_chroma[8];
};

struct msm_vidc_8155_output_crop_payload
{
    uint32 size;
    uint32 version;
    uint32 port_index;
    uint32 left;
    uint32 top;
    uint32 display_width;
    uint32 display_height;
    uint32 width;
    uint32 height;
    uint32 frame_num;
    uint32 bit_depth_y;
    uint32 bit_depth_c;
    struct msm_vidc_8155_misr_info misr_info[2];
};

struct hyp_msm_vidc_8155_extradata_output_index
{
    uint32 type;
    struct msm_vidc_8155_output_crop_payload output_crop;
};

/* Firmware 3.1 struct and definations*/
/*
 * metadata_buffer layout details
 *
 * This is SINGLE metadata_header per metadata buffer:
 * metadata_header details:
 * @count           : total number of metadata header present
 * @size            : total size = headers + payload + gaps
 * @version         : global Metadata version = (major << 16) | minor
 *                    should be 0x1 << 16
 * @reserved        : reserved for future use
 *
 * Each type of metadata types, there should be corresponding metapayload_header.
 * metapayload_header details:
 * @type            : metadata property type
 * @size            : metadata payload size in bytes (not including any header)
 *                    Host will read this size from offset to read metadata
 *                    payload details
 * @version         : metadata version  = (major << 16) | minor
 *                    should be 0x1 << 16
 * @offset          : offset in bytes of metadata payload from the start of metadata_header
 * @reserved        : reserved for future use
 *
 * Example of metadata layouts
 * metadata_header.count           : 3 (assume there are 3 types of metadata)
 *  => This is the ZERO offset of the buffer. All the other offsets are calcuated based on this.
 * metadata_header.size            : (8*4) + 3*(8*4) +
 * metadata_header.version         : (1 << 16)
 * metadata_header.reserved[0-4]   : Ignored
 * metapayload_header.type         : property ID-1
 * metapayload_header.size         : ID-1-payload-size
 * metapayload_header.version      : (1<< 16)
 * metapayload_header.offset       : ID-1-offset
 * metapayload_header.flags        : one value of hfi_metapayload_header_flags
 * metapayload_header.reserved[0-2]: ignored
 * metapayload_header.type         : property ID-2
 * metapayload_header.size         : ID-2-payload-size
 * metapayload_header.version      : (1<< 16)
 * metapayload_header.offset       : ID-2-offset
 * metapayload_header.flags        : one value of hfi_metapayload_header_flags
 * metapayload_header.reserved[0-2]: ignored
 * metapayload_header.type         : property ID-3
 * metapayload_header.size         : ID-3-payload-size
 * metapayload_header.version      : (1<< 16)
 * metapayload_header.offset       : ID-3-offset
 * metapayload_header.flags        : one value of hfi_metapayload_header_flags
 * metapayload_header.reserved[0-2]: ignored
 * can be some hole - unused memory
 * ID-1-offset                     : ID-1 payload of size ID-1-payload-size
 * can be some hole - unused memory
 * ID-1-offset                     : ID-1 payload of size ID-1-payload-size
 * can be some hole - unused memory
 * ID-1-offset                     : ID-1 payload of size ID-1-payload-size
 * can be some hole - unused memory
*/

#define SET_INPUT_PENDING(X)  (X | (1ULL << 63))
#define GET_INPUT_PENDING(X) ((X & (1ULL <<63)) >> 63)
#define GET_TAG_VALUE(X) (X & ~(1ULL << 63))

struct hyp_8155_metadata_header
{
    uint32 size;
    uint32 version;
    uint32 portIndex;
    uint32 metadataType;
    uint32 dataSize;
};

struct hyp_metadata_header
{
    uint32 size;
    uint32 version;
    uint32 portIndex;
    uint32 metadataType;
    uint32 dataSize;
};

struct hyp_metadata
{
    struct hyp_metadata_header hdr;
    void* data;
};

struct hyp_buffer_tag
{
    uint64 tag;
    uint64 tag2;
};

 /*========================================================================
 Enumerations
 ========================================================================*/

 /*========================================================================
 Functions
 ========================================================================*/
hypv_status_type hyp_video_8255_metadata_extract_tags(
    const uint8* src,
    uint32 capacity,
    hyp_buffer_tag* tagInfo,
    bool forInput);

hypv_status_type hyp_video_8255_metadata_flatten_to_8155(
    const uint8* dst,
    uint32 capacity,
    hyp_metadata** infos,
    uint32 infos_count);

hypv_status_type hyp_video_8155_metadata_flatten_to_8255(
    const uint8* dst,
    uint32 capacity,
    hyp_metadata** infos,
    uint32 infos_count);
#endif /* __HYP_VIDEO_META_TRANSLATOR_H__ */

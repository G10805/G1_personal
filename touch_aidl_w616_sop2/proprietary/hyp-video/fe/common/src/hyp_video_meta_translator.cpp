/*========================================================================

*//** @file hyp_video_meta_translator.cpp
    This file provides backward compatibility for 8255 VPU with functions
    that translating metadata from 8155 VPU to the metadata types supported
    by 8155 codec2.

 Copyright (c) 2022 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
*//*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/02/22    sk     Use flag and change header path for kernel 5.15
06/04/22    rq     support 8255 metadata backward compatible with Codec2.1.0
========================================================================== */

#include "hyp_video_meta_translator.h"
#include "hyp_debug.h"
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_vidc_utils.h>
#else
#include <media/msm_vidc_utils.h>
#endif

#define METAHEADER_VERSION (1 << 16)

enum v4l2_mpeg_vidc_metapayload_header_flags
{
    METADATA_FLAGS_NONE = 0,
    METADATA_FLAGS_TOP_FIELD = (1 << 0),
    METADATA_FLAGS_BOTTOM_FIELD = (1 << 1),
};

struct msm_vidc_metabuf_header
{
    uint32 count;
    uint32 size;
    uint32 version;
    uint32 reserved[5];
};

struct msm_vidc_metapayload_header
{
    uint32 type;
    uint32 size;
    uint32 version;
    uint32 offset;
    uint32 flags;
    uint32 reserved[3];
};

enum v4l2_mpeg_vidc_metadata
{
    METADATA_BITSTREAM_RESOLUTION         = 0x03000103,
    METADATA_CROP_OFFSETS                 = 0x03000105,
    METADATA_LTR_MARK_USE_DETAILS         = 0x03000137,
    METADATA_SEQ_HEADER_NAL               = 0x0300014a,
    METADATA_DPB_LUMA_CHROMA_MISR         = 0x03000153,
    METADATA_OPB_LUMA_CHROMA_MISR         = 0x03000154,
    METADATA_INTERLACE                    = 0x03000156,
    METADATA_TIMESTAMP                    = 0x0300015c,
    METADATA_CONCEALED_MB_COUNT           = 0x0300015f,
    METADATA_HISTOGRAM_INFO               = 0x03000161,
    METADATA_PICTURE_TYPE                 = 0x03000162,
    METADATA_SEI_MASTERING_DISPLAY_COLOUR = 0x03000163,
    METADATA_SEI_CONTENT_LIGHT_LEVEL      = 0x03000164,
    METADATA_HDR10PLUS                    = 0x03000165,
    METADATA_EVA_STATS                    = 0x03000167,
    METADATA_BUFFER_TAG                   = 0x0300016b,
    METADATA_SUBFRAME_OUTPUT              = 0x0300016d,
    METADATA_ENC_QP_METADATA              = 0x0300016e,
    METADATA_DEC_QP_METADATA              = 0x0300016f,
    METADATA_ROI_INFO                     = 0x03000173,
    METADATA_DPB_TAG_LIST                 = 0x03000179,
    METADATA_MAX_NUM_REORDER_FRAMES       = 0x03000127,
    METADATA_SALIENCY_INFO                = 0x0300018A,
    METADATA_FENCE                        = 0x0300018B,
};

hypv_status_type hyp_video_8255_metadata_extract_tags(const uint8 *src, uint32 capacity,
        hyp_buffer_tag *tagInfo, bool forInput)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if (nullptr == src || nullptr == tagInfo ||
        capacity <= sizeof(msm_vidc_metabuf_header) + sizeof(msm_vidc_metapayload_header))
    {
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        struct msm_vidc_metabuf_header *mhdr = (struct msm_vidc_metabuf_header *)(src);
        struct msm_vidc_metapayload_header *mphdr = (struct msm_vidc_metapayload_header *)(mhdr + 1);
        uint32 hdrOffset = sizeof(struct msm_vidc_metabuf_header)
                         + sizeof(struct msm_vidc_metapayload_header);
        uint32 count = 0;

        while (hdrOffset < capacity && count < mhdr->count)
        {
            if (0 == mphdr->size)
            {
                count++;
                mphdr++;
                hdrOffset += sizeof(struct msm_vidc_metapayload_header);
                continue;
            }

            if (METADATA_BUFFER_TAG == mphdr->type)
            {
                    if (mphdr->offset + mphdr->size > capacity)
                    {
                        HYP_VIDEO_MSG_ERROR("Cannot parse meta buffer "
                            "offset %u, size %u in buf of capacity %u",
                            mphdr->offset, mphdr->size, capacity);
                        rc = HYPV_STATUS_FAIL;
                        break;
                    }

                    uint64 *payload = (uint64 *)(src + mphdr->offset);
                    if (forInput)
                    {
                        tagInfo->tag = *payload;
                        tagInfo->tag2 = 0;
                        HYP_VIDEO_MSG_INFO("Extract buffer tag %llu",
                            (unsigned long long)GET_TAG_VALUE(tagInfo->tag));
                    }
                    else
                    {
                        if (METADATA_FLAGS_NONE == mphdr->flags ||
                            METADATA_FLAGS_TOP_FIELD == mphdr->flags)
                        {
                            tagInfo->tag = *payload;
                            HYP_VIDEO_MSG_INFO("Extract buffer tag %llu input pending: %llu",
                                (unsigned long long)GET_TAG_VALUE(tagInfo->tag),
                                GET_INPUT_PENDING(tagInfo->tag));
                        }
                        else
                        {
                            tagInfo->tag2 = *payload;
                            HYP_VIDEO_MSG_INFO("Extract buffer tag2 %llu",
                                (unsigned long long)tagInfo->tag2);
                        }
                    }

                    break;
            }
            count++;
            mphdr++;
            hdrOffset += sizeof(struct msm_vidc_metapayload_header);
        }
    }

    if (HYPV_STATUS_FAIL == rc)
    {
        HYP_VIDEO_MSG_ERROR("Fail to extract tags from metadata");
    }

	return rc;
}

/* convert to 8255 metadata format */
hypv_status_type hyp_video_8155_metadata_flatten_to_8255(const uint8* dst, uint32 capacity,
        hyp_metadata** infos, uint32 infos_count)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    struct msm_vidc_metabuf_header *mhdr = (struct msm_vidc_metabuf_header*)(dst);
    struct msm_vidc_metapayload_header *mphdr = NULL;
    uint32 payloadSize = 0;
    uint32 payloadOffset = 0;

    if (NULL == mhdr || 0 == capacity || 0 == infos_count)
    {
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        memset(mhdr, 0, sizeof(struct msm_vidc_metabuf_header));
        mhdr->version = METAHEADER_VERSION;
        mphdr = (struct msm_vidc_metapayload_header*)(mhdr + 1);
        //always +1 metapayload header offset in case one HDRStaticInfo includes two payloads
        payloadOffset = sizeof(struct msm_vidc_metabuf_header) + sizeof(struct msm_vidc_metapayload_header) * (infos_count + 1);

        for (uint32 n = 0; n < infos_count && HYPV_STATUS_SUCCESS == rc; n++)
        {
            hyp_metadata* info = infos[n];

            switch (info->hdr.metadataType)
            {
                case VIDC_QMETADATA_BUFFER_TAG:
                {
                    payloadSize = sizeof(uint64);
                    if (payloadOffset + payloadSize > capacity)
                    {
                        HYP_VIDEO_MSG_ERROR("no enough memory for metadata tag");
                        rc = HYPV_STATUS_FAIL;
                    }
                    else
                    {
                        memset(mphdr, 0, sizeof(struct msm_vidc_metapayload_header));
                        mphdr->type = METADATA_BUFFER_TAG;
                        mphdr->size = payloadSize;
                        mphdr->version = 1 << 16;
                        mphdr->offset = payloadOffset;
                        mphdr->flags = METADATA_FLAGS_NONE;

                        uint64* payload = (uint64*)(dst + payloadOffset);
                        struct hyp_buffer_tag* tagInfo = (struct hyp_buffer_tag *)(info->data);
                        *payload = tagInfo->tag;
                        HYP_VIDEO_MSG_INFO("Flatten buffer tag 0x%llx", (uint64)tagInfo->tag);
                    }
                    break;
                }
                default:
                    break;
            }
            mhdr->count++;
            mphdr++;
            payloadOffset += payloadSize;
        }

        mhdr->size = payloadOffset;
        HYP_VIDEO_MSG_INFO("Total metadata count %u, size %u", mhdr->count, mhdr->size);
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        HYP_VIDEO_MSG_INFO("Flatten total %u metadata payloads", infos_count);
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("Failed to flatten total %u metadata payloads", infos_count);
    }

    return rc;
}

/* convert to 8155 metadata format */
hypv_status_type hyp_video_8255_metadata_flatten_to_8155(const uint8* dst, uint32 capacity,
    hyp_metadata** infos, uint32 infos_count)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint8* tmp = (uint8*)dst;
    size_t metadataSize = 0;
    size_t offset = 0;
    hyp_8155_metadata_header* hdr = NULL;
    hyp_metadata* info = NULL;

    if (NULL == tmp || 0 == capacity)
    {
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        for (uint32 n = 0; n < infos_count && HYPV_STATUS_SUCCESS == rc; n++)
        {
            info = infos[n];

            switch (info->hdr.metadataType)
            {
                case MSM_VIDC_EXTRADATA_INDEX:
                {
                    metadataSize = sizeof(hyp_8155_metadata_header)
                        + sizeof(hyp_msm_vidc_8155_extradata_output_index);

                    if ((uint8*)tmp + metadataSize > (uint8*)dst + capacity)
                    {
                        rc = HYPV_STATUS_FAIL;
                    }

                    hdr = (hyp_8155_metadata_header*)tmp;
                    hdr->metadataType = MSM_VIDC_EXTRADATA_INDEX;
                    hdr->size = metadataSize;
                    hdr->dataSize = sizeof(struct hyp_msm_vidc_8155_extradata_output_index);

                    struct hyp_msm_vidc_8155_extradata_output_index* payload =
                        (struct hyp_msm_vidc_8155_extradata_output_index*)
                        (tmp + sizeof(hyp_8155_metadata_header));

                    payload->type = MSM_VIDC_EXTRADATA_OUTPUT_CROP;
                    memcpy(&payload->output_crop, info->data, info->hdr.dataSize);

                    break;
                }
                default:
                    break;
            }
            offset = offset + metadataSize;
            tmp = tmp + metadataSize;
            metadataSize = 0;
        }

        metadataSize = (size_t)sizeof(hyp_8155_metadata_header);

        if (offset + metadataSize < capacity)
        {
            hdr = (hyp_8155_metadata_header*)tmp;
            hdr->metadataType = MSM_VIDC_EXTRADATA_NONE;
            hdr->size = sizeof(hyp_8155_metadata_header);
            hdr->dataSize = 0;
        }
        else
        {
            rc = HYPV_STATUS_FAIL;
        }
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        HYP_VIDEO_MSG_INFO("Flatten total %u metadata payloads", infos_count);
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("Failed to flatten total %u metadata payloads", infos_count);
    }

    return rc;
}

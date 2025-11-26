/*===========================================================================

*//** @file hyp_video_fe_translation.c
This file implements message translation between video FE and habmm

Copyright (c) 2016-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*===========================================================================
                             Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           nb          Fix compilation errors due to additon of new compiler flags
11/02/22           sk          Use flag and change header path for kernel 5.15
06/04/22           rq          support 8255 metadata backward compatible with Codec2.1.0
06/16/21           sh          Include input_tag2 as its needed for dropping inputs in codec2
04/01/21           sh          Bringup video on RedBend Hypervisor
08/27/20           sh          Bringup video decode using codec2
07/18/19           sm          Update logging for more info on errors
06/17/19           sm          Check for output2 for multi-stream usecases and log cleanup
05/30/19           sm          Add support for hab export using ion fd
03/06/19           rz          Handle decoder output dynamic map/unmap/export/unexport buffers
01/30/19           rz          Add decoder dynamic input buffer mode
10/30/18           sm          Clear cached buffers on stream off to handle multistream playback
01/18/18           sm          Add support for passthrough mode feature
11/29/17           sm          Add support for video input interrupts handling
07/25/17           sm          Fix issues with dynamic allocation mode
06/28/17           aw          Unify and update all logs in hyp-video
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
04/03/17           sm          Add support for Video input FE-BE
02/02/17           hl          Support video encode
01/30/17           sm          Update BE-FE message passing
08/16/16           rz          Dynamically loading hab library
07/27/16           rz          Initialize buffer address in HYPVIDEO_MSGRESP_EVENT
07/26/16           rz          Handle contiguous memory mapping for QNX OMX
07/15/16           hl          Add code to support LA target
07/08/16           hl          Isolate video data from ioctl
06/22/16           henryl      Add dynamic buffer mode support
06/20/16           henryl      Clean up and add error messages
06/09/16           henryl      Add support linux hypervisor for non-contiguous metadata
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/
#include "hyp_vidc_inf.h"
#include "hyp_vidc_types.h"
#include "hyp_buffer_manager.h"
#include "hyp_videopriv.h"
#include "hyp_video.h"
#include "hyp_video_fe_translation.h"
#include "hyp_debug.h"
#include "hyp_video_meta_translator.h"
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_vidc_utils.h>
#else
#include <media/msm_vidc_utils.h>
#endif
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/dma-buf.h>


static uint8* map(int fd, int length, int offset)
{
    uint8* va = (uint8 *)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if (va == MAP_FAILED)
    {
        HYP_VIDEO_MSG_ERROR("Failed to map fd(%d) size(%d) offset(%d)",
                fd, length, offset);
        va = nullptr;
    }

    if (va != nullptr)
    {
        struct dma_buf_sync sync;
        HABMM_MEMSET(&sync, 0, sizeof(struct dma_buf_sync));

        sync.flags = DMA_BUF_SYNC_START|DMA_BUF_SYNC_RW;

        if (ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync))
        {
            HYP_VIDEO_MSG_ERROR("Failed to sync cache: start");
        }
    }

    return va;
}

static void unmap(int fd, uint8* base, int length)
{
    struct dma_buf_sync sync;
    HABMM_MEMSET(&sync, 0, sizeof(struct dma_buf_sync));

    sync.flags = DMA_BUF_SYNC_END|DMA_BUF_SYNC_RW;

    if (ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync))
    {
        HYP_VIDEO_MSG_ERROR("Failed to sync cache: end");
    }

    if (-1 == munmap(base, length))
    {
        HYP_VIDEO_MSG_ERROR("munmap %p failed len %d", base, length);
    }
}

/**===========================================================================

FUNCTION get_meta_data_type

@brief  Get meta data type from buffer type

@param [in] buffer type

@dependencies
  None

@return
  Returns meta data buffer type

===========================================================================*/
static vidc_buffer_type get_meta_data_type(vidc_buffer_type buf_type)
{
    vidc_buffer_type meta_buf_type = VIDC_BUFFER_UNUSED;

    switch(buf_type)
    {
    case VIDC_BUFFER_INPUT:
        meta_buf_type = VIDC_BUFFER_METADATA_INPUT;
        break;
    case VIDC_BUFFER_OUTPUT:
        meta_buf_type = VIDC_BUFFER_METADATA_OUTPUT;
        break;
    case VIDC_BUFFER_OUTPUT2:
        meta_buf_type = VIDC_BUFFER_METADATA_OUTPUT2;
        break;
    default:
        meta_buf_type = VIDC_BUFFER_UNUSED;
    }

    return meta_buf_type;
}

/**===========================================================================

FUNCTION translate_hvfe_to_habmm

@brief  Translate message from hypervisor FE to HAB

@param [in] hypv session pointer
@param [in] msg id
@param [in] input buffer pointer
@param [in] input buffer size
@param [out] output buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type translate_hvfe_to_habmm(hypv_session_t* hypv_session, int msg_id,
                                         void* aInBuf, int nInSize, void* aOutBuf)
{
    vidc_buffer_info_64b_type* pCmdBuffer64b;
    vidc_buffer_info_type* pCmdBuffer;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if ((VIDEO_SESSION_DECODE == hypv_session->video_session) ||
        (VIDEO_SESSION_ENCODE == hypv_session->video_session))
    {
        switch (msg_id)
        {
        case VIDC_PING_GET_CALLBACK:
            {
                vidc_drv_msg_info_type *pMsgInfo;
                vidc_drv_msg_info_64b_type *pMsgInfo64;
                hypv_map_entry_t *find_entry;
                pMsgInfo = (vidc_drv_msg_info_type *)aInBuf;
                pMsgInfo64 = (vidc_drv_msg_info_64b_type *)aOutBuf;
                HABMM_MEMSET(pMsgInfo64, 0, sizeof(vidc_drv_msg_info_64b_type));
                find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pMsgInfo->payload.frame_data.frame_addr);
                if (NULL == find_entry)
                {
                    rc = HYPV_STATUS_FAIL;
                    HYP_VIDEO_MSG_ERROR("failed to find an entry node. msg id 0x%x frame addr %p",
                                         (unsigned int)msg_id, pMsgInfo->payload.frame_data.frame_addr);
                }
                else
                {
                    pMsgInfo64->payload.frame_data.frame_addr = (uint64)find_entry->bufferid;
                }
                if (rc == HYPV_STATUS_SUCCESS)
                {
                    if (pMsgInfo->payload.frame_data.non_contiguous_metadata && 0 != pMsgInfo->payload.frame_data.metadata_addr)
                    {
                        find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pMsgInfo->payload.frame_data.metadata_addr);
                        if (NULL == find_entry)
                        {
                            rc = HYPV_STATUS_FAIL;
                            HYP_VIDEO_MSG_ERROR("failed to find an entry node. msg id 0x%x metadata addr %p",
                                                 (unsigned int)msg_id, pMsgInfo->payload.frame_data.metadata_addr);
                        }
                        else
                        {
                            pMsgInfo64->payload.frame_data.metadata_addr = (uint64)find_entry->bufferid;
                        }
                    }
                }
                if (rc == HYPV_STATUS_SUCCESS)
                {
                    pMsgInfo64->status = ( pMsgInfo->status == VIDC_ERR_NONE ) ? HYPV_STATUS_SUCCESS : HYPV_STATUS_FAIL ;
                    pMsgInfo64->event_type = pMsgInfo->event_type;
                    pMsgInfo64->payload.frame_data.data_len = pMsgInfo->payload.frame_data.data_len;
                    pMsgInfo64->payload.frame_data.frm_clnt_data = pMsgInfo->payload.frame_data.frm_clnt_data;
                    pMsgInfo64->payload.frame_data.data_len = pMsgInfo->payload.frame_data.data_len;
                    pMsgInfo64->payload.frame_data.flags = pMsgInfo->payload.frame_data.flags;
                }
                break;
            }
        case VIDC_IOCTL_ALLOCATE_BUFFER:
            {
                pCmdBuffer = (vidc_buffer_info_type*)aInBuf;
                pCmdBuffer64b = (vidc_buffer_info_64b_type*)aOutBuf;
                HABMM_MEMSET(pCmdBuffer64b, 0, sizeof(vidc_buffer_info_64b_type));
                pCmdBuffer64b->buf_size = pCmdBuffer->buf_size;
                pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                pCmdBuffer64b->contiguous = pCmdBuffer->contiguous;
                pCmdBuffer64b->extradata_buf_size = pCmdBuffer->extradata_buf_size;
                break;
            }
        case VIDC_IOCTL_SET_BUFFER:
            {
                uint32 export_id = 0;

                pCmdBuffer = (vidc_buffer_info_type*)aInBuf;
                pCmdBuffer64b = (vidc_buffer_info_64b_type*)aOutBuf;
                HABMM_MEMSET(pCmdBuffer64b, 0, sizeof(vidc_buffer_info_64b_type));
                if (pCmdBuffer->contiguous)
                {
                   /** Obtain export id for the data buffer */
                   export_id = hypv_map_add_to_remote(&hypv_session->habmm_if,
                                        &hypv_session->lookup_queue,
                                        hypv_session->habmm_handle/*habmm handle*/,
                                        pCmdBuffer->buf_addr,
                                        pCmdBuffer->buf_size + pCmdBuffer->extradata_buf_size,
                                        pCmdBuffer->buf_type,
                                        hypv_session->export_as_fd);

                   if (0 == export_id)
                   {
                       rc = HYPV_STATUS_FAIL;
                       HYP_VIDEO_MSG_ERROR("failed to obtain export id. msg id 0x%x buf addr %p",
                                            (unsigned int)msg_id, pCmdBuffer->buf_addr);
                   }
                   else
                   {
                       pCmdBuffer64b->buf_addr = (uint64)export_id;
                   }

                }
                else
                {
                   /** Obtain export id for the data buffer */
                   export_id = hypv_map_add_to_remote(&hypv_session->habmm_if,
                                             &hypv_session->lookup_queue,
                                             hypv_session->habmm_handle/*habmm handle*/,
                                             pCmdBuffer->buf_addr,
                                             pCmdBuffer->buf_size,
                                             pCmdBuffer->buf_type,
                                             hypv_session->export_as_fd);

                   if (0 == export_id)
                   {
                       rc = HYPV_STATUS_FAIL;
                       HYP_VIDEO_MSG_ERROR("failed to export.msg id 0x%x addr %p size %u buf type 0x%x",
                                            (unsigned int)msg_id, pCmdBuffer->buf_addr,
                                            pCmdBuffer->buf_size, (unsigned int)pCmdBuffer->buf_type);
                   }
                   else
                   {
                       pCmdBuffer64b->buf_addr = (uint64)export_id;
                   }

                   /** Obtain export id for the extradata */
                   vidc_buffer_type meta_buf_type = get_meta_data_type(pCmdBuffer->buf_type);

                   if (0 != pCmdBuffer->extradata_buf_addr)
                   {
                       export_id = hypv_map_add_to_remote(&hypv_session->habmm_if,
                                               &hypv_session->lookup_queue,
                                               hypv_session->habmm_handle/*habmm handle*/,
                                               pCmdBuffer->extradata_buf_addr/*user extradata buffer va*/,
                                               pCmdBuffer->extradata_buf_size,
                                               meta_buf_type,
                                               hypv_session->export_as_fd);
                       if (0 == export_id)
                       {
                           rc = HYPV_STATUS_FAIL;
                           HYP_VIDEO_MSG_ERROR("failed to export meta.msg id 0x%x addr %p size %u buf type 0x%x",
                                                (unsigned int)msg_id, pCmdBuffer->extradata_buf_addr,
                                                pCmdBuffer->extradata_buf_size, (unsigned int)meta_buf_type);
                       }
                       else
                       {
                           pCmdBuffer64b->extradata_buf_addr = (uint64)export_id;
                       }
                   }
                }

                pCmdBuffer64b->buf_size = pCmdBuffer->buf_size;
                pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                pCmdBuffer64b->contiguous = pCmdBuffer->contiguous;
                pCmdBuffer64b->extradata_buf_size = pCmdBuffer->extradata_buf_size;

                break;
            }

        case VIDC_IOCTL_FREE_BUFFER:
            {
                hypv_map_entry_t *find_entry = NULL;

                pCmdBuffer = (vidc_buffer_info_type*)aInBuf;
                pCmdBuffer64b = (vidc_buffer_info_64b_type*)aOutBuf;
                HABMM_MEMSET(pCmdBuffer64b, 0, sizeof(vidc_buffer_info_64b_type));

                /** Obtain virtual address for the data buffer */
                find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pCmdBuffer->buf_addr);
                if (NULL == find_entry)
                {
                    if ( VIDC_BUFFER_OUTPUT == pCmdBuffer->buf_type ||
                         VIDC_BUFFER_OUTPUT2 == pCmdBuffer->buf_type ||
                         VIDC_BUFFER_INPUT == pCmdBuffer->buf_type )
                    {
                        pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                    }
                    else
                    {
                        rc = HYPV_STATUS_FAIL;
                        HYP_VIDEO_MSG_ERROR("failed to find mapped entry node. msg id 0x%x buf addr %p",
                                             (unsigned int)msg_id, pCmdBuffer->buf_addr);
                    }
                }
                else
                {
                    pCmdBuffer64b->buf_addr = (uint64)find_entry->bufferid;
                    if (!pCmdBuffer64b->contiguous && 0 != pCmdBuffer->extradata_buf_addr)
                    {
                        /** Obtain virtual address for the extradata buffer */
                        find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pCmdBuffer->extradata_buf_addr);
                        if (NULL == find_entry)
                        {
                            rc = HYPV_STATUS_FAIL;
                            HYP_VIDEO_MSG_ERROR("failed to find mapped entry node. msg id 0x%x extradata %p",
                                                 (unsigned int)msg_id, pCmdBuffer->extradata_buf_addr);
                        }
                        else
                        {
                            pCmdBuffer64b->extradata_buf_addr = (uint64)find_entry->bufferid;
                        }
                    }
                    pCmdBuffer64b->buf_size = pCmdBuffer->buf_size;
                    pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                    pCmdBuffer64b->contiguous = pCmdBuffer->contiguous;
                    pCmdBuffer64b->extradata_buf_size = pCmdBuffer->extradata_buf_size;
                }
                break;
            }
        case VIDC_IOCTL_EMPTY_INPUT_BUFFER:
        case VIDC_IOCTL_FILL_OUTPUT_BUFFER:
            {
                vidc_frame_data_64b_type* pCmdBuffer64b;
                vidc_frame_data_type* pCmdBuffer;
                uint32 export_id = 0;

                pCmdBuffer = (vidc_frame_data_type*)aInBuf;
                pCmdBuffer64b = (vidc_frame_data_64b_type*)aOutBuf;
                HABMM_MEMSET(pCmdBuffer64b, 0, sizeof(vidc_frame_data_64b_type));

                /* Frame buffer */
                // handle the dynamic buffer mode such that the buffer has not been mapped before
                export_id = hypv_map_add_to_remote(&hypv_session->habmm_if,
                                                &hypv_session->lookup_queue,
                                                hypv_session->habmm_handle,
                                                pCmdBuffer->frame_addr,
                                                pCmdBuffer->alloc_len,
                                                pCmdBuffer->buf_type,
                                                hypv_session->export_as_fd);
                if (0 == export_id)
                {
                   rc = HYPV_STATUS_FAIL;
                   HYP_VIDEO_MSG_ERROR("failed to export.msg id 0x%x addr %p size %u buf type 0x%x",
                                        (unsigned int)msg_id, pCmdBuffer->frame_addr,
                                        pCmdBuffer->alloc_len, (unsigned int)pCmdBuffer->buf_type);
                }
                else
                {
                   pCmdBuffer64b->frame_addr = (uint64)export_id;
                }

                if (rc == HYPV_STATUS_SUCCESS)
                {
                    /* Metadata buffer */
                    if (pCmdBuffer->non_contiguous_metadata && 0 != pCmdBuffer->metadata_addr)
                    {
                       vidc_buffer_type meta_buf_type = get_meta_data_type(pCmdBuffer->buf_type);

                       // handle the dynamic buffer mode such that the buffer has not been mapped before
                       export_id = hypv_map_add_to_remote(&hypv_session->habmm_if,
                                                       &hypv_session->lookup_queue,
                                                       hypv_session->habmm_handle,
                                                       pCmdBuffer->metadata_addr,
                                                       pCmdBuffer->alloc_metadata_len,
                                                       meta_buf_type,
                                                       hypv_session->export_as_fd);
                       if (0 == export_id)
                       {
                            rc = HYPV_STATUS_FAIL;
                            HYP_VIDEO_MSG_ERROR("failed to export meta.msg id 0x%x addr %p size %u buf type 0x%x",
                                                (unsigned int)msg_id, pCmdBuffer->metadata_addr,
                                                pCmdBuffer->alloc_metadata_len, (unsigned int)meta_buf_type);
                       }
                       else
                       {
                            pCmdBuffer64b->metadata_addr = (uint64)export_id;
                       }
                    }
                    pCmdBuffer64b->alloc_len = pCmdBuffer->alloc_len;
                    pCmdBuffer64b->data_len = pCmdBuffer->data_len;
                    pCmdBuffer64b->offset = pCmdBuffer->offset;
                    pCmdBuffer64b->timestamp = pCmdBuffer->timestamp;
                    pCmdBuffer64b->flags = pCmdBuffer->flags;
                    pCmdBuffer64b->frm_clnt_data = pCmdBuffer->frm_clnt_data;
                    pCmdBuffer64b->frame_decsp = pCmdBuffer->frame_decsp;
                    pCmdBuffer64b->frame_type = pCmdBuffer->frame_type;
                    pCmdBuffer64b->buf_type = pCmdBuffer->buf_type;
                    pCmdBuffer64b->mark_target = pCmdBuffer->mark_target;
                    pCmdBuffer64b->mark_data = pCmdBuffer->mark_data;
                    pCmdBuffer64b->input_tag = pCmdBuffer->input_tag;
                    pCmdBuffer64b->output_tag = pCmdBuffer->output_tag;
                    pCmdBuffer64b->alloc_metadata_len = pCmdBuffer->alloc_metadata_len;
                    pCmdBuffer64b->non_contiguous_metadata = pCmdBuffer->non_contiguous_metadata;

                    /* convert meta data from 8155 format to 8255 format */
                    if (HYP_TARGET_LEMANS == hypv_session->target_variant)
                    {
                        if (VIDC_IOCTL_EMPTY_INPUT_BUFFER == msg_id)
                        {
                            /* need to map the fd to the real virtual address */
                            uint8* meta_addr = map((int)(uintptr_t)pCmdBuffer->metadata_addr,
                                pCmdBuffer->alloc_metadata_len, 0);

                            if (nullptr != meta_addr)
                            {

                                int infos_count = 1;
                                hyp_buffer_tag tagInfo = {};
                                tagInfo.tag = pCmdBuffer->input_tag;
                                tagInfo.tag2 = pCmdBuffer->input_tag;

                                hyp_metadata meta_tag = {};
                                meta_tag.hdr.metadataType = VIDC_QMETADATA_BUFFER_TAG;
                                meta_tag.data = &tagInfo;
                                hyp_metadata* infos[1] = { &meta_tag };

                                HYP_VIDEO_MSG_INFO("metadata_addr %p, meta_addr %p, len %u, type 0x%x, input_tag %lu, alloc_metadata_len %u",
                                    pCmdBuffer->metadata_addr, meta_addr, pCmdBuffer->data_len, (unsigned int)pCmdBuffer->buf_type,
                                    pCmdBuffer->input_tag, pCmdBuffer->alloc_metadata_len);

                                hyp_video_8155_metadata_flatten_to_8255(
                                    meta_addr,
                                    pCmdBuffer->alloc_metadata_len,
                                    infos,
                                    infos_count);

                                unmap((int)(uintptr_t)pCmdBuffer->metadata_addr,
                                    meta_addr, pCmdBuffer->alloc_metadata_len);
                            }
                            else
                            {
                                HYP_VIDEO_MSG_ERROR("failed to map metadata %p", pCmdBuffer->metadata_addr);
                                rc = HYPV_STATUS_FAIL;
                            }
                        }
                    }
                    else
                    {
                        HYP_VIDEO_MSG_INFO("No meta translation for non Lemans");
                    }
                }
                break;
            }
        default:
            {
                /** Get & Set property - type conversion is not required */
                HABMM_MEMCPY(aOutBuf, aInBuf, nInSize);
                break;
            }
        }
    }
    else
    {
        HABMM_MEMCPY(aOutBuf, aInBuf, nInSize);
    }

    return rc;
}

/**===========================================================================

FUNCTION translate_habmm_to_hvfe

@brief  Translate message from HAB to hypervisor FE

@param [in] hypv session pointer
@param [in] msg id
@param [in] input buffer pointer
@param [in] input buffer size
@param [out] output buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type translate_habmm_to_hvfe(hypv_session_t* hypv_session, int msg_id,
                                         void* aInBuf, int nInSize, void* aOutBuf)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hypvideo_msg_data_type* hypvideo_msg = (hypvideo_msg_data_type*)aInBuf;

    if (HYPVIDEO_MSGRESP_OPEN_RET == (hypvideo_msg_id_type)msg_id)
    {
        hypvideo_open_data_type* value = (hypvideo_open_data_type*)aOutBuf;
        /** device handle */
        value->return_io_handle = hypvideo_msg->open_data.return_io_handle;
        value->return_status= hypvideo_msg->open_data.return_status;
        value->passthrough_mode= hypvideo_msg->open_data.passthrough_mode;
        value->hyp_plt_id = hypvideo_msg->open_data.hyp_plt_id;
    }
    else if (HYPVIDEO_MSGRESP_CLOSE_RET == (hypvideo_msg_id_type)msg_id)
    {
        hypvideo_close_data_type* value = (hypvideo_close_data_type*)aOutBuf;
        value->return_status= hypvideo_msg->open_data.return_status;
    }
    else if(HYPVIDEO_MSGRESP_IOCTL_RET == (hypvideo_msg_id_type)msg_id)
    {
       if (NULL == aOutBuf)
       {
          HYP_VIDEO_MSG_ERROR("aOutBuf is NULL");
          rc = HYPV_STATUS_FAIL;
       }
       else
       {
          uint32 vidc_ioctl = hypvideo_msg->ioctl_data.vidc_ioctl;

          switch (vidc_ioctl)
          {
             case VIDC_IOCTL_ALLOCATE_BUFFER:
             case VIDC_IOCTL_SET_BUFFER:
             case VIDC_IOCTL_FREE_BUFFER:
             {
                vidc_buffer_info_64b_type* pCmdBuffer64b;
                vidc_buffer_info_type* pCmdBuffer;
                void* va = NULL;

                pCmdBuffer64b = (vidc_buffer_info_64b_type*)hypvideo_msg->ioctl_data.payload;
                pCmdBuffer = (vidc_buffer_info_type*)aOutBuf;

                if(VIDC_IOCTL_ALLOCATE_BUFFER == hypvideo_msg->ioctl_data.vidc_ioctl)
                {
                   if (pCmdBuffer64b->contiguous)
                   {
                      va = hypv_map_add_from_remote(&hypv_session->habmm_if,
                                                     &hypv_session->lookup_queue,
                                                     hypv_session->habmm_handle,
                                                     pCmdBuffer64b->buf_size + pCmdBuffer64b->extradata_buf_size,
                                                     (uint32)pCmdBuffer64b->buf_addr/* import id for frame buffer */,
                                                     pCmdBuffer64b->buf_type);

                      if (0 == va)
                      {
                          rc = HYPV_STATUS_FAIL;
                          HYP_VIDEO_MSG_ERROR("failed to create a map entry. vidc ioctl %u for buf addr 0x%lx",
                                               vidc_ioctl, (unsigned long)pCmdBuffer64b->buf_addr);
                      }
                      else
                      {
                         pCmdBuffer64b->buf_addr = (uint64)(unsigned long)va;

                         if (pCmdBuffer64b->extradata_buf_size)
                         {
                            pCmdBuffer64b->extradata_buf_addr = pCmdBuffer64b->buf_addr + pCmdBuffer64b->buf_size;
                         }
                      }

                   }
                   else
                   {
                      va = hypv_map_add_from_remote(&hypv_session->habmm_if,
                                                     &hypv_session->lookup_queue,
                                                     hypv_session->habmm_handle,
                                                     pCmdBuffer64b->buf_size,
                                                     (uint32)pCmdBuffer64b->buf_addr/* import id for frame buffer */,
                                                     pCmdBuffer64b->buf_type);

                      if (0 == va)
                      {
                          rc = HYPV_STATUS_FAIL;
                          HYP_VIDEO_MSG_ERROR("failed to create a map entry. vidc ioctl %u buf addr 0x%lx",
                                               vidc_ioctl, (unsigned long)pCmdBuffer64b->buf_addr);
                      }
                      else
                      {
                         pCmdBuffer64b->buf_addr = (uint64)(unsigned long)va;

                         if (0 != pCmdBuffer64b->extradata_buf_addr)/** for extra data */
                         {
                             va = hypv_map_add_from_remote(&hypv_session->habmm_if,
                                                            &hypv_session->lookup_queue,
                                                            hypv_session->habmm_handle,
                                                            pCmdBuffer64b->extradata_buf_size,
                                                            (uint32)pCmdBuffer64b->extradata_buf_addr/* import id for extra data */,
                                                            pCmdBuffer64b->buf_type);

                             pCmdBuffer64b->extradata_buf_addr = (uint64)(unsigned long)va;
                             if (0 == va)
                             {
                                 rc = HYPV_STATUS_FAIL;
                                 HYP_VIDEO_MSG_ERROR("failed to create a map entry. vidc ioctl %u extradata buf addr 0x%lx",
                                                      vidc_ioctl, (unsigned long)pCmdBuffer64b->extradata_buf_addr);
                             }
                         }
                      }
                   }

                }

                pCmdBuffer->buf_addr = (uint8*)(unsigned long)pCmdBuffer64b->buf_addr;
                pCmdBuffer->buf_size = pCmdBuffer64b->buf_size;
                pCmdBuffer->buf_type = pCmdBuffer64b->buf_type;
                pCmdBuffer->contiguous = pCmdBuffer64b->contiguous;
                pCmdBuffer->extradata_buf_size = pCmdBuffer64b->extradata_buf_size;
                pCmdBuffer->extradata_buf_addr = (uint8*)(unsigned long)pCmdBuffer64b->extradata_buf_addr;
                break;
             }

             default:
             {
                /** Get property*/
                HABMM_MEMCPY(aOutBuf, hypvideo_msg->ioctl_data.payload, nInSize);
                break;
             }
          }
       }
    }
    else if(HYPVIDEO_MSGRESP_EVENT == (hypvideo_msg_id_type)msg_id)
    {
        vidc_drv_msg_info_type* msgBuff32b = (vidc_drv_msg_info_type*)aOutBuf;
        hypv_map_entry_t *find_entry;
        uint32 bufferid = 0;

        msgBuff32b->status = ( hypvideo_msg->event_data.vidc.status == HYPV_STATUS_SUCCESS ) ? VIDC_ERR_NONE : VIDC_ERR_FAIL ;
        msgBuff32b->event_type = (vidc_event_type) hypvideo_msg->event_data.vidc.event_type;
        if (VIDEO_SESSION_VINPUT == hypv_session->video_session)
        {
            msgBuff32b->payload.event_data_1 = hypvideo_msg->event_data.vidc.payload.event_data_1;
        }
        else
        {
            msgBuff32b->payload.frame_data.alloc_len = hypvideo_msg->event_data.vidc.payload.frame_data.alloc_len;
            msgBuff32b->payload.frame_data.buf_type = hypvideo_msg->event_data.vidc.payload.frame_data.buf_type;
            msgBuff32b->payload.frame_data.data_len = hypvideo_msg->event_data.vidc.payload.frame_data.data_len;
            msgBuff32b->payload.frame_data.flags = hypvideo_msg->event_data.vidc.payload.frame_data.flags;
            msgBuff32b->payload.frame_data.frame_addr = (uint8*)NULL;

            if (hypvideo_msg->event_data.vidc.payload.frame_data.frame_addr)
            {
                bufferid = (uint32)(long)hypvideo_msg->event_data.vidc.payload.frame_data.frame_addr;
                find_entry = hypv_map_from_lookup(&hypv_session->lookup_queue, bufferid);
                if (NULL == find_entry)
                {
                    HYP_VIDEO_MSG_ERROR("failed to find an entry node for buf addr 0x%p",
                                    (void *)(unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.frame_addr);
                }
                else
                {
                    msgBuff32b->payload.frame_data.frame_addr = (uint8*)find_entry->va;
                    if ( ( VIDC_EVT_RESP_OUTPUT_DONE == msgBuff32b->event_type &&
                          !(msgBuff32b->payload.frame_data.flags & VIDC_FRAME_FLAG_READONLY) )  ||
                         ( VIDC_EVT_RELEASE_BUFFER_REFERENCE == msgBuff32b->event_type ) ||
                         ( VIDC_EVT_RESP_INPUT_DONE == msgBuff32b->event_type ) )
                    {
                        hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, find_entry->va, bufferid);
                    }
                }
            }

            msgBuff32b->payload.frame_data.frame_decsp = hypvideo_msg->event_data.vidc.payload.frame_data.frame_decsp;
            msgBuff32b->payload.frame_data.frame_type = hypvideo_msg->event_data.vidc.payload.frame_data.frame_type;
            msgBuff32b->payload.frame_data.frm_clnt_data = hypvideo_msg->event_data.vidc.payload.frame_data.frm_clnt_data;
            msgBuff32b->payload.frame_data.mark_data = (unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.mark_data;
            msgBuff32b->payload.frame_data.mark_target = (unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.mark_target;
            msgBuff32b->payload.frame_data.input_tag = (unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.input_tag;
            msgBuff32b->payload.frame_data.input_tag2 = (unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.input_tag2;
            msgBuff32b->payload.frame_data.output_tag = (unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.output_tag;
            msgBuff32b->payload.frame_data.alloc_metadata_len = hypvideo_msg->event_data.vidc.payload.frame_data.alloc_metadata_len;
            msgBuff32b->payload.frame_data.non_contiguous_metadata = hypvideo_msg->event_data.vidc.payload.frame_data.non_contiguous_metadata;
            msgBuff32b->payload.frame_data.metadata_addr = NULL;

            if (hypvideo_msg->event_data.vidc.payload.frame_data.non_contiguous_metadata &&
                hypvideo_msg->event_data.vidc.payload.frame_data.metadata_addr)
            {
                bufferid = (uint32)(long)hypvideo_msg->event_data.vidc.payload.frame_data.metadata_addr;
                find_entry = hypv_map_from_lookup(&hypv_session->lookup_queue, bufferid);
                if (NULL == find_entry)
                {
                    HYP_VIDEO_MSG_ERROR("failed to find an entry node for metadata addr 0x%p",
                              (void *)(unsigned long)hypvideo_msg->event_data.vidc.payload.frame_data.metadata_addr);
                }
                else
                {
                    msgBuff32b->payload.frame_data.metadata_addr = (uint8*)find_entry->va;

                    /* convert meta data from 8255 format to 8155 format */
                    if (HYP_TARGET_LEMANS == hypv_session->target_variant)
                    {
                        if (VIDC_EVT_RESP_OUTPUT_DONE == msgBuff32b->event_type)
                        {
                            /* need to map the fd to the real virtual address */
                            uint8* meta_addr = map((int)(uintptr_t)msgBuff32b->payload.frame_data.metadata_addr,
                                msgBuff32b->payload.frame_data.alloc_metadata_len, 0);

                            if (nullptr != meta_addr)
                            {

                                hyp_buffer_tag tagInfo = {};
                                hyp_video_8255_metadata_extract_tags(
                                    meta_addr,
                                    msgBuff32b->payload.frame_data.alloc_metadata_len,
                                    &tagInfo,
                                    false);
                                msgBuff32b->payload.frame_data.input_tag = (unsigned long)tagInfo.tag;
                                msgBuff32b->payload.frame_data.input_tag2 = (unsigned long)tagInfo.tag2;
                                msgBuff32b->payload.frame_data.output_tag = (unsigned long)tagInfo.tag;

                                HYP_VIDEO_MSG_INFO("event 0x%x, metadata_addr %p, bufferid %u, va %p, input_tag %lu, input_tag2 %lu, output_tag %lu",
                                    (unsigned int)msgBuff32b->event_type, msgBuff32b->payload.frame_data.metadata_addr, bufferid, find_entry->va,
                                    msgBuff32b->payload.frame_data.input_tag, msgBuff32b->payload.frame_data.input_tag2,
                                    msgBuff32b->payload.frame_data.output_tag);

                                int infos_count = 1;
                                msm_vidc_8155_output_crop_payload meta_crop = {};
                                meta_crop.size = 0;
                                meta_crop.version = 0;
                                meta_crop.port_index = VIDC_BUFFER_OUTPUT;
                                meta_crop.left = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.left;
                                meta_crop.top = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.top;
                                meta_crop.display_width = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.stride;
                                meta_crop.display_height = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.scan_lines;
                                meta_crop.width = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.width;
                                meta_crop.height = msgBuff32b->payload.frame_data.frame_decsp.luma_plane.height;
                                meta_crop.frame_num = 0;
                                meta_crop.bit_depth_y = 0;
                                meta_crop.bit_depth_c = 0;

                                hyp_metadata cropInfo = {};
                                cropInfo.hdr.size = sizeof(meta_crop) + sizeof(hyp_8155_metadata_header);
                                cropInfo.hdr.version = 0;
                                cropInfo.hdr.portIndex = 0;
                                cropInfo.hdr.metadataType = MSM_VIDC_EXTRADATA_INDEX;
                                cropInfo.hdr.dataSize = sizeof(meta_crop);
                                cropInfo.data = &meta_crop;

                                hyp_metadata* infos[1] = { &cropInfo };
                                hyp_video_8255_metadata_flatten_to_8155(
                                    meta_addr,
                                    msgBuff32b->payload.frame_data.alloc_metadata_len,
                                    infos,
                                    infos_count);

                                unmap((int)(uintptr_t)msgBuff32b->payload.frame_data.metadata_addr,
                                    meta_addr, msgBuff32b->payload.frame_data.alloc_metadata_len);
                            }
                            else
                            {
                                HYP_VIDEO_MSG_ERROR("failed to map metadata %p", msgBuff32b->payload.frame_data.metadata_addr);
                                rc = HYPV_STATUS_FAIL;
                            }
                        }
                    }
                    else
                    {
                        HYP_VIDEO_MSG_INFO("No meta translation for non Lemans");
                    }

                    if ((VIDC_EVT_RESP_OUTPUT_DONE == msgBuff32b->event_type) ||
                        (VIDC_EVT_RELEASE_BUFFER_REFERENCE == msgBuff32b->event_type) ||
                        (VIDC_EVT_RESP_INPUT_DONE == msgBuff32b->event_type))
                    {
                        hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, find_entry->va, bufferid);
                        HYP_VIDEO_MSG_INFO("Removing metabuf entry for %s framedata",
                            (msgBuff32b->payload.frame_data.flags & VIDC_FRAME_FLAG_READONLY) ? "READONLY" : "READWRITE");
                    }
                }
            }

            msgBuff32b->payload.frame_data.offset = hypvideo_msg->event_data.vidc.payload.frame_data.offset;
            msgBuff32b->payload.frame_data.timestamp = hypvideo_msg->event_data.vidc.payload.frame_data.timestamp;
        }
    }

    return rc;
}



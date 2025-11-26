/*========================================================================

*//** @file linux_vdec_venc_common.cpp

@par DESCRIPTION:
Linux video encode and decode shared hypervisor front-end implementation

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/1.0/fe/linux/src/linux_vdec_venc_common.c#7 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/19/25    su     Report ENOMEM error code when setting alloc_mode is failed
07/25/24    su     Update AVC levels 5.2 onwards
07/08/24    sk     Improve lock handing during queueing of output buffer in FE
11/24/23    ll     Solve KW issue
09/26/23    mm     Correct the wrong HEVC tier/level
08/11/23    cg     Add a check for null pointer dereference
08/30/23    nb     Fix compilation errors due to additon of new compiler flags
06/22/23    mm     Add flush lock to avoid race condition issue
05/24/23    pc     Add HDR10 static metadata support for HEVC encode
11/07/22    su     Add support to SYNCFRAMEDECODE in thumbnail mode
11/02/22    sk     Ion to DMABuffer changes for Kernel 5.15
09/26/22    mm     Remove dependency OMX
09/03/22    mz     support 8255 metadata backward compatible with Codec2.1.0
08/03/22    nb     Set H264 entropy coding property during streamon
07/30/22    nb     Update P-frames setting during streamon & remove B-frames setting
07/30/22    nb     Set frame QP during streamon
07/30/22    nb     Set rate_control property during streamon
05/26/22    ll     Enable bitrate saving mode
05/31/22    sj     Handle VP8 level conversion from V4l2 to/from VIDC
03/28/22    sh     Enable Layer Encode for GVM
03/30/22    sd     Add support for blur filter
03/25/22    sh     Enable LTR support with Codec2
02/14/22    sj     Synchronize lock handling
01/28/22    sj     Handle unsupported ctrl & reduce loglevel
01/14/22    nb     Remove secure property call during streamon
10/05/21    rq     Postpone setting multi slice until streamon and fix MISRA error
09/02/21    sj     Free the secondary out buf in dynamic buffer mode in case of multistream
07/30/21    nb     Assign Non-zero value to mark_data & mark_target to get FBDs for discrd pics
07/29/21    mm     Postpone setting of intra refresh and clean up
07/13/21    sj     Set IDR period to 1 for HEIC encode based on grid mode
07/07/21    sj     Initialize variables to avoid K/W errors
06/07/21    hs     Enable support for dmabuf
05/06/21    sj     Add support to query supported profiles/levels
04/01/21    sh     Bringup video on RedBend Hypervisor
03/20/21    sj     Handle buffer freeing in case of dynamic buffer mode
03/18/21    rq     Call back only once for the flush command with FLASH_ALL type
02/08/21    mm     Enalbe grid mode for HEIC
02/01/21    rq     Delay setting secure mode until right before streamon
11/10/20    sh     Set filled length to 0 for empty EOS buffer
11/03/20    hl     Add Long Term Reference Support
10/12/20    sh     Enable CSC for H264/HEVC encoder
08/27/20    sh     Bringup video decode using codec2
08/12/20    sh     Update Linux FE with kernel 5.4 macros
07/23/20    sh     Add STOP & RELEASE_RESOURCE IOCTLs to maintain proper driver & FW states
04/24/20    mm     Fix video can't play issue for LV
03/03/20    sh     Avoid flush call on the driver when buffers are not registered
01/09/20    sm     Support for secure playback
09/09/19    sm     Fix timestamp overflow
09/04/19    sm     Calculate the actual extradata buffer size
08/23/19    sm     Enable extradata flag only when an extradata is set
07/18/19    sm     Add input extradata info into EOS buffer
07/10/19    sm     Populate input extradata only for encode usecase
07/09/19    sm     Add additional supported color format
06/27/19    sm     Add support for input crop
06/17/19    sm     A few log cleanup
05/30/19    sm     Add support for hab export using ion fd
05/13/19    sh     Avoid calling FTB when flush in progress
04/24/19    sm     Multi stream buffer handling during reconfig and split buffer API support
04/11/19    sm     Add V4l2 dequeue event and buffer API handling
04/09/19    sm     Handle encoder buffer reference in dynamic mode
03/18/19    sm     Synchronize FW reference frames between FE and BE
03/14/19    sm     Generalize enc and dec command handling
03/06/19    rz     Handle decoder output dynamic map/unmap/export/unexport buffers
03/04/19    sm     Add dynamic buffer mode for encode usecase
03/04/19    sm     Make sure logging is properly enabled
02/15/19    rz     Use internal buffer to handle EOS
02/05/19    rz     Add decoder dynamic input buffer mode
11/01/18    sm     Add a common function to map color formats
10/30/18    sm     Clear cached buffers on stream off to handle multistream playback
10/15/18    la     fix null pointer dereferencing
09/14/18    sh     Extract buffer address when index is passed during EOS
08/21/18    aw     Fix Klockwork P1, compilation and MISRA warning
06/12/18    sm     Add sequence header retrieval in FE
05/08/18    sm     Add support for 10 bit playback
04/02/18    sm     Set session codec before querying frame size capability
01/18/18    sm     Add support for passthrough mode feature
10/12/17    sm     Simplify buffer management logics for dynamic alloc mode
07/25/17    sm     Fix issues with dynamic allocation mode
06/28/17    aw     Unify and update all logs in hyp-video
05/08/17    sm     Update for new hyp-video architecture
04/03/17    sm     Add support for Video input FE-BE
02/02/17    hl     Support video encode
09/29/16    hl     Fix to work with hypervisor LA target test
07/15/16    hl     Add code to support LA target
06/22/16    hl     Add dynamic buffer mode support
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include <sys/ioctl.h>
#include "hyp_videopriv_fe.h"
#include "hyp_videopriv.h"
#include "hyp_vidc_types.h"
#include "hyp_vidc_inf.h"
#include "linux_video_fe.h"
#include "linux_vdec_fe.h"
#include "linux_venc_fe.h"
#include "linux_vdec_venc_common.h"
#include "hyp_buffer_manager.h"
#include "MMThread.h"
#include "MMCriticalSection.h"
#ifdef WIN32
#include "types.h"
#include "msm_vidc_dec.h"
#include "VideoPlatform.h"
#include "VideoComDef.h"
#include "msm_vidc_enc.h"
#include "msm_media_info.h"
#else
#include <string>
#include <linux/videodev2.h>
#include <linux/msm_ion.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#include <BufferAllocator/BufferAllocatorWrapper.h>
#else
#include <media/msm_media_info.h>
#endif
#endif
#include "hyp_debug.h"

#define VIDIOC_S_FMT_64_BIT 0xc0d05605
#define VIDIOC_G_FMT_64_BIT 0xc0d05604
#define VIDIOC_PREPARE_BUF_64_BIT 0xc058565d
#define VIDIOC_QBUF_64_BIT 0xc058560f

static int v4l2fe_log2(int mask)
{
   int res = 0;

   while (mask >>= 1)
   {
      res++;
   }

   return res;
}

static hypv_status_type dec_enc_create_dyn_buf(fe_io_session_t *fe_ioss);
static void dec_enc_destroy_dyn_buf(fe_io_session_t *fe_ioss);
static hypv_status_type dec_enc_add_dyn_buf_ref(fe_io_session_t *fe_ioss, struct v4l2_buffer *buf,
                                                vidc_frame_data_type *frame_data, uint32 *index);


/* Following defines are used by 8255 compatibility only */
#define VIDC_QMETADATA_BUFFER_TAG                  0x0300016b
#define VIDC_QMETADATA_PORT_INPUT_FROM_CLIENT      0x1
#define VIDC_QMETADATA_PORT_OUTPUT_TO_CLIENT       0x20
#define VIDC_QMETADATA_VERSION_MINOR   0x00
#define VIDC_QMETADATA_VERSION_MAJOR   0x01
#define VIDC_QMETADATA_VERSION   ( ( VIDC_QMETADATA_VERSION_MAJOR << 16 ) | \
                                   ( VIDC_QMETADATA_VERSION_MINOR ) )

#define ALLOC_METADATA_LEN            16384    // lemans metadata

   /**===========================================================================

   FUNCTION dec_enc_enable_8255_metadata_tag

   @brief  Eanble tag metadata by default for 8255

   @param [in] fe_ioss pointer

   @dependencies
     None

   @return
     Returns hypv_status_type

   ===========================================================================*/
static hypv_status_type dec_enc_enable_8255_metadata_tag(fe_io_session_t* fe_ioss)
{
   hypv_status_type rc = HYPV_STATUS_SUCCESS;
   vidc_metadata_header_type meta_header;
   HABMM_MEMSET(&meta_header, 0, sizeof(vidc_metadata_header_type));

   meta_header.enable = TRUE;
   meta_header.client_type = VIDC_QMETADATA_BUFFER_TAG;
   meta_header.metadata_type = VIDC_QMETADATA_BUFFER_TAG;
   meta_header.port_index = VIDC_QMETADATA_PORT_OUTPUT_TO_CLIENT | VIDC_QMETADATA_PORT_INPUT_FROM_CLIENT;
   meta_header.version = VIDC_QMETADATA_VERSION;

   HYP_VIDEO_MSG_INFO("set property VIDC_I_METADATA_HEADER type: VIDC_QMETADATA_BUFFER_TAG");

   rc = dec_enc_set_drv_property(fe_ioss,
      VIDC_I_METADATA_HEADER,
      sizeof(vidc_metadata_header_type),
      (uint8*)&meta_header);

   if (HYPV_STATUS_SUCCESS != rc)
   {
      HYP_VIDEO_MSG_ERROR("failed to set drv property metadata type VIDC_I_METADATA_HEADER");
   }

   return rc;
}

/**===========================================================================

FUNCTION dec_enc_allocate_meta_buffer

@brief  Allocate metadata buffer for decoder and encode

@param [in] alloc_data ion data pointer for metadata buffer allocation

@param [in] meta_fd metadata fd

@dependencies
   None

@return
Returns hypv_status_type

===========================================================================*/
#ifdef SUPPORT_DMABUF
static hypv_status_type dec_enc_allocate_meta_buffer(
   uint32 buffer_size, std::string dmaBufHeapName, int* meta_fd)
{
   hypv_status_type rc = HYPV_STATUS_SUCCESS;
   BufferAllocator *bufferAllocator = NULL;

   if (!meta_fd)
   {
      HYP_VIDEO_MSG_ERROR("invalid meta_fd");
      rc = HYPV_STATUS_FAIL;
   }

   if (HYPV_STATUS_SUCCESS == rc)
   {
      bufferAllocator = CreateDmabufHeapBufferAllocator();
      if (!bufferAllocator)
      {
         HYP_VIDEO_MSG_ERROR("Failed to create DMABufHeap Buffer Allocator object");
         rc = HYPV_STATUS_FAIL;
      }
   }

   if (HYPV_STATUS_SUCCESS == rc)
   {
      int metafd = -1;
      char* heapName = const_cast<char*>(dmaBufHeapName.c_str());

      metafd = DmabufHeapAlloc(bufferAllocator, heapName, buffer_size, 0, 0);
      FreeDmabufHeapBufferAllocator(bufferAllocator);

      if (0 > metafd)
      {
         HYP_VIDEO_MSG_ERROR("Memory allocation failed");
         rc = HYPV_STATUS_FAIL;
      }
      else
      {
         *meta_fd = metafd;
         HYP_VIDEO_MSG_INFO("DmabufHeap Buffer Allocation done successfully, buffer_size: %u, heapName: %s, meta_fd: %d", buffer_size, heapName, metafd);
      }
   }

   return rc;
}
#else
static hypv_status_type dec_enc_allocate_meta_buffer(
   ion_allocation_data* alloc_data, int* meta_fd)
{
   hypv_status_type rc = HYPV_STATUS_SUCCESS;

   if (!alloc_data || !meta_fd)
   {
      HYP_VIDEO_MSG_ERROR("invalid alloc_data");
      rc = HYPV_STATUS_FAIL;
   }

   if (HYPV_STATUS_SUCCESS == rc)
   {
      int dev_fd = ion_open();
      if (0 > dev_fd)
      {
         HYP_VIDEO_MSG_ERROR("opening ion device failed with ion_fd = %d", dev_fd);
         rc = HYPV_STATUS_FAIL;
      }
      else
      {
         int ret = 0;
         int metafd = -1;
         ret = ion_alloc_fd(dev_fd, alloc_data->len, 0,
            alloc_data->heap_id_mask, alloc_data->flags, &metafd);

         if (ret || 0 > metafd)
         {
            HYP_VIDEO_MSG_ERROR("ION ALLOC memory failed");
            rc = HYPV_STATUS_FAIL;
         }
         else
         {
            *meta_fd = metafd;

            HYP_VIDEO_MSG_INFO("Alloc ion memory: fd (dev:%d data:%d) len %d flags %#x mask %#x",
               dev_fd, metafd, (unsigned int)alloc_data->len,
               (unsigned int)alloc_data->flags, (unsigned int)alloc_data->heap_id_mask);
         }

         ion_close(dev_fd);
      }
   }

   return rc;
}
#endif

/**===========================================================================

FUNCTION dec_enc_free_meta_buffer

@brief  Free decoder and encode metadata buffer

@param [in] metabuffer fd

@dependencies
   None

@return
Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_free_meta_buffer(int fd)
{
   HYP_VIDEO_MSG_INFO("freeing metadata buffer fd %d", fd);
   if (0 <= fd)
   {
      close(fd);
   }

   return HYPV_STATUS_SUCCESS;
}


/**===========================================================================

FUNCTION dec_enc_v4l2_to_vidc

@brief  Convert V4L2 profiles/levels into VIDC profiles/levels

@param [in] control id
@param [in] V4L2 profile/level

@dependencies
  None

@return
  Returns VIDC profile/level or -EINVAL

===========================================================================*/
int dec_enc_convert_v4l2_to_vidc(int id, int value)
{
    int vidc_id = -1;

    switch(id)
    {
       case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
          switch(value)
          {
             case V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE:
                vidc_id = VIDC_PROFILE_H264_BASELINE;
                break;
             case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE:
                vidc_id = VIDC_PROFILE_H264_CONSTRAINED_BASE;
                break;
             case V4L2_MPEG_VIDEO_H264_PROFILE_MAIN:
                vidc_id = VIDC_PROFILE_H264_MAIN;
                break;
             case V4L2_MPEG_VIDEO_H264_PROFILE_HIGH:
                vidc_id = VIDC_PROFILE_H264_HIGH;
                break;
             case V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_HIGH:
                vidc_id = VIDC_PROFILE_H264_CONSTRAINED_HIGH;
                break;
             default:
                vidc_id = VIDC_PROFILE_H264_BASELINE;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN:
                vidc_id = VIDC_PROFILE_HEVC_MAIN;
                break;
             case V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10:
                 vidc_id = VIDC_PROFILE_HEVC_MAIN10;
                 break;
             case V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_STILL_PICTURE:
                vidc_id = VIDC_PROFILE_HEVC_MAIN_STILL_PICTURE;
                break;
             default:
                vidc_id = VIDC_PROFILE_HEVC_MAIN;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_VP9_PROFILE_0:
                vidc_id = VIDC_PROFILE_VP9_0;
                break;
             case V4L2_MPEG_VIDEO_VP9_PROFILE_2:
                vidc_id = VIDC_PROFILE_VP9_2_10B;
                break;
             default:
                vidc_id = VIDC_PROFILE_VP9_0;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE:
          switch (value)
          {
             case V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_SIMPLE:
                vidc_id = VIDC_PROFILE_MPEG2_SIMPLE;
                break;
             case V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_MAIN:
                vidc_id = VIDC_PROFILE_MPEG2_MAIN;
                break;
             default:
                vidc_id = VIDC_PROFILE_MPEG2_SIMPLE;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_VP8_PROFILE_0:
                vidc_id = VIDC_PROFILE_VPX_VERSION_0;
                break;
             case V4L2_MPEG_VIDEO_VP8_PROFILE_1:
                vidc_id = VIDC_PROFILE_VPX_VERSION_1;
                break;
             case V4L2_MPEG_VIDEO_VP8_PROFILE_2:
                vidc_id = VIDC_PROFILE_VPX_VERSION_2;
                break;
             case V4L2_MPEG_VIDEO_VP8_PROFILE_3:
                vidc_id = VIDC_PROFILE_VPX_VERSION_3;
                break;
             default:
                vidc_id = VIDC_PROFILE_VPX_VERSION_0;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_HEVC_TIER_MAIN:
                vidc_id = VIDC_PROFILE_HEVC_TIER_MAIN;
                break;
             case V4L2_MPEG_VIDEO_HEVC_TIER_HIGH:
                vidc_id = VIDC_PROFILE_HEVC_TIER_HIGH;
                break;
             default:
                vidc_id = VIDC_PROFILE_HEVC_TIER_MAIN;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_H264_LEVEL_1_0:
                vidc_id = VIDC_LEVEL_H264_1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_1B:
                vidc_id = VIDC_LEVEL_H264_1b;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_1_1:
                vidc_id = VIDC_LEVEL_H264_1p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_1_2:
                vidc_id = VIDC_LEVEL_H264_1p2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_1_3:
                vidc_id = VIDC_LEVEL_H264_1p3;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_2_0:
                vidc_id = VIDC_LEVEL_H264_2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_2_1:
                vidc_id = VIDC_LEVEL_H264_2p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_2_2:
                vidc_id = VIDC_LEVEL_H264_2p2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_3_0:
                vidc_id = VIDC_LEVEL_H264_3;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_3_1:
                vidc_id = VIDC_LEVEL_H264_3p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_3_2:
                vidc_id = VIDC_LEVEL_H264_3p2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_4_0:
                vidc_id = VIDC_LEVEL_H264_4;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_4_1:
                vidc_id = VIDC_LEVEL_H264_4p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_4_2:
                vidc_id = VIDC_LEVEL_H264_4p2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_5_0:
                vidc_id = VIDC_LEVEL_H264_5;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_5_1:
                vidc_id = VIDC_LEVEL_H264_5p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_5_2:
                vidc_id = VIDC_LEVEL_H264_5p2;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_6_0:
                vidc_id = VIDC_LEVEL_H264_6;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_6_1:
                vidc_id = VIDC_LEVEL_H264_6p1;
                break;
             case V4L2_MPEG_VIDEO_H264_LEVEL_6_2:
                vidc_id = VIDC_LEVEL_H264_6p2;
                break;
             default:
                vidc_id = VIDC_LEVEL_H264_1;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
          switch (value)
          {
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_1:
                vidc_id = VIDC_LEVEL_HEVC_1;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_2:
                vidc_id = VIDC_LEVEL_HEVC_2;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_2_1:
                vidc_id = VIDC_LEVEL_HEVC_21;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_3:
                vidc_id = VIDC_LEVEL_HEVC_3;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_3_1:
                vidc_id = VIDC_LEVEL_HEVC_31;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_4:
                vidc_id = VIDC_LEVEL_HEVC_4;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_4_1:
                vidc_id = VIDC_LEVEL_HEVC_41;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_5:
                vidc_id = VIDC_LEVEL_HEVC_5;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_5_1:
                vidc_id = VIDC_LEVEL_HEVC_51;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_5_2:
                vidc_id = VIDC_LEVEL_HEVC_52;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_6:
                vidc_id = VIDC_LEVEL_HEVC_6;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_6_1:
                vidc_id = VIDC_LEVEL_HEVC_61;
                break;
             case V4L2_MPEG_VIDEO_HEVC_LEVEL_6_2:
                vidc_id = VIDC_LEVEL_HEVC_62;
                break;
             default:
                vidc_id = VIDC_LEVEL_HEVC_1;
                break;
          }
          break;

       case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
          vidc_id = value;
          break;

       default:
          HYP_VIDEO_MSG_ERROR("Unknown control id %d", id);
          vidc_id = -1;
    }

    return vidc_id;
}

/**===========================================================================

FUNCTION dec_enc_vidc_to_v4l2

@brief  Convert VIDC profiles/levels into V4L2 profiles/levels

@param [in] control id
@param [in] VIDC profile/level

@dependencies
  None

@return
  Returns V4L2 profile/level or -EINVAL

===========================================================================*/
int dec_enc_convert_vidc_to_v4l2(int id, int value)
{
    int v4l2_id = -1;

    switch(id)
    {
       case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
          switch(value)
          {
             case VIDC_PROFILE_H264_BASELINE:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
                break;
             case VIDC_PROFILE_H264_CONSTRAINED_BASE:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE;
                break;
             case VIDC_PROFILE_H264_MAIN:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
                break;
             case VIDC_PROFILE_H264_HIGH:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
                break;
             case VIDC_PROFILE_H264_CONSTRAINED_HIGH:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_HIGH;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
          switch (value)
          {
             case VIDC_PROFILE_HEVC_MAIN:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN;
                break;
             case VIDC_PROFILE_HEVC_MAIN10:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10;
                break;
             case VIDC_PROFILE_HEVC_MAIN_STILL_PICTURE:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_STILL_PICTURE;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
          switch (value)
          {
             case VIDC_PROFILE_VP9_0:
                v4l2_id = V4L2_MPEG_VIDEO_VP9_PROFILE_0;
                break;
             case VIDC_PROFILE_VP9_2_10B:
                v4l2_id = V4L2_MPEG_VIDEO_VP9_PROFILE_2;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_VP9_PROFILE_0;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDC_VIDEO_MPEG2_PROFILE:
          switch (value)
          {
             case VIDC_PROFILE_MPEG2_SIMPLE:
                v4l2_id = V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_SIMPLE;
                break;
             case VIDC_PROFILE_MPEG2_MAIN:
                v4l2_id = V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_MAIN;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDC_VIDEO_MPEG2_PROFILE_SIMPLE;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
          switch (value)
          {
             case VIDC_PROFILE_VPX_VERSION_0:
                v4l2_id = V4L2_MPEG_VIDEO_VP8_PROFILE_0;
                break;
             case VIDC_PROFILE_VPX_VERSION_1:
                v4l2_id = V4L2_MPEG_VIDEO_VP8_PROFILE_1;
                break;
             case VIDC_PROFILE_VPX_VERSION_2:
                v4l2_id = V4L2_MPEG_VIDEO_VP8_PROFILE_2;
                break;
             case VIDC_PROFILE_VPX_VERSION_3:
                v4l2_id = V4L2_MPEG_VIDEO_VP8_PROFILE_3;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_VP8_PROFILE_0;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
          switch (value)
          {
             case VIDC_PROFILE_HEVC_TIER_MAIN:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_TIER_MAIN;
                break;
             case VIDC_PROFILE_HEVC_TIER_HIGH:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_TIER_HIGH;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_TIER_MAIN;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
          switch (value)
          {
             case VIDC_LEVEL_H264_1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
                break;
             case VIDC_LEVEL_H264_1b:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1B;
                break;
             case VIDC_LEVEL_H264_1p1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
                break;
             case VIDC_LEVEL_H264_1p2:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
                break;
             case VIDC_LEVEL_H264_1p3:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
                break;
             case VIDC_LEVEL_H264_2:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
                break;
             case VIDC_LEVEL_H264_2p1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
                break;
             case VIDC_LEVEL_H264_2p2:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
                break;
             case VIDC_LEVEL_H264_3:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
                break;
             case VIDC_LEVEL_H264_3p1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
                break;
             case VIDC_LEVEL_H264_3p2:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
                break;
             case VIDC_LEVEL_H264_4:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
                break;
             case VIDC_LEVEL_H264_4p1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
                break;
             case VIDC_LEVEL_H264_4p2:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
                break;
             case VIDC_LEVEL_H264_5:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_5_0;
                break;
             case VIDC_LEVEL_H264_5p1:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_5_1;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
                break;
          }
          break;
       case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
          switch (value)
          {
             case VIDC_LEVEL_HEVC_1:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_1;
                break;
             case VIDC_LEVEL_HEVC_2:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_2;
                break;
             case VIDC_LEVEL_HEVC_21:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_2_1;
                break;
             case VIDC_LEVEL_HEVC_3:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_3;
                break;
             case VIDC_LEVEL_HEVC_31:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_3_1;
                break;
             case VIDC_LEVEL_HEVC_4:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_4;
                break;
             case VIDC_LEVEL_HEVC_41:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_4_1;
                break;
             case VIDC_LEVEL_HEVC_5:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_5;
                break;
             case VIDC_LEVEL_HEVC_51:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_5_1;
                break;
             case VIDC_LEVEL_HEVC_52:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_5_2;
                break;
             case VIDC_LEVEL_HEVC_6:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_6;
                break;
             case VIDC_LEVEL_HEVC_61:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_6_1;
                break;
             case VIDC_LEVEL_HEVC_62:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_6_2;
                break;
             default:
                v4l2_id = V4L2_MPEG_VIDEO_HEVC_LEVEL_1;
                break;
          }
          break;

       case V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL:
          v4l2_id = value;
          break;

       default:
          HYP_VIDEO_MSG_ERROR("Unknown control id %d", id);
          v4l2_id = -1;
    }

    return v4l2_id;
}

/**===========================================================================

FUNCTION dec_enc_submit_command

@brief  Submit an encoder/decoder command from video FE

@param [in] fe_ioss pointer
@param [in] cmd

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_submit_command(fe_io_session_t* fe_ioss, uint32 vidc_cmd)
{
    hypv_session_t* hypv_session = fe_ioss->io_handle;

    return hyp_device_ioctl(hypv_session, vidc_cmd, NULL, 0, NULL, 0);
}

/**===========================================================================

FUNCTION dec_enc_query_menu

@brief  FE query from the menu of supported profiles

@param [in] v4l2fe_vidc_ctrl pointer
@param [in] size of ctrl struct
@param [in] v4l2_querymenu pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_query_menu(v4l2fe_vidc_ctrl *ctrls, int size, struct v4l2_querymenu* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    int index = 0;

    HYP_VIDEO_MSG_INFO("query for id %u", data->id);
    for (index = 0; index < size; index++)
    {
        if (ctrls[index].id == data->id)
        {
           if ((uint)ctrls[index].minimum > data->index ||
               (uint)ctrls[index].maximum < data->index ||
               ctrls[index].menu_skip_mask & (1 << data->index))
           {
              rc = HYPV_STATUS_FAIL;
           }
           break;
        }
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
       HYP_VIDEO_MSG_INFO("name %s min %d max %d menu_skip_mask %#x index %u rc %d",
                          ctrls[index].name, ctrls[index].minimum, ctrls[index].maximum,
                          ctrls[index].menu_skip_mask, data->index, rc);
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_query_profile

@brief  FE enumerate supported profiles for codec

@param [in] fe_ioss pointer
@param [in] v4l2fe_vidc_ctrl pointer
@param [in] size of ctrl struct
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_query_profile(fe_io_session_t *fe_ioss, v4l2fe_vidc_ctrl *ctrl,
                                    int size, struct v4l2_queryctrl* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_capability_profile_level_type payload;
    int profile_mask = 0;
    int v4l2_profile_mask = 0;
    int index = 0;
    int profile = 0;
    int hevc_tier = 0;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_capability_profile_level_type));

    HYP_VIDEO_MSG_INFO("query profile for id %u", data->id);
    for (index = 0; HYPV_STATUS_SUCCESS == rc; index++)
    {
        payload.index = index;
        rc = dec_enc_get_drv_property(fe_ioss,
                VIDC_I_CAPABILITY_PROFILE_LEVEL,
                sizeof(vidc_capability_profile_level_type),
                (uint8*)&payload);

        profile_mask |= payload.profile;

        if (V4L2_CID_MPEG_VIDEO_HEVC_TIER == data->id)
        {
          /* The meaning of level's value is as follows:
           * Bits: 0-27 shall be used to specify the level information.
           * Bits: 28-31 is reserved for non-level qualifications For
           * HEVC these 4 bits shall be used to communicate
           * tier infromation. */
           hevc_tier = dec_enc_convert_vidc_to_v4l2(data->id, payload.level >> 28);
           if (hevc_tier >= 0)
           {
              v4l2_profile_mask |= (1 << hevc_tier);
              HYP_VIDEO_MSG_INFO("supported hevc tier %d", hevc_tier);
           }
           else
           {
              HYP_VIDEO_MSG_ERROR("Unsupported cmd 0x%x", data->id);
              rc = HYPV_STATUS_FAIL;
           }
        }
        else
        {
           profile = dec_enc_convert_vidc_to_v4l2(data->id, payload.profile);
           if (profile >= 0)
           {
              v4l2_profile_mask |= (1 << profile);
           }
           else
           {
              HYP_VIDEO_MSG_ERROR("Unsupported cmd 0x%x", data->id);
              rc = HYPV_STATUS_FAIL;
           }
        }
    }

    data->flags = profile_mask;

    if (HYPV_STATUS_SUCCESS != rc && 1 == index)
    {
        HYP_VIDEO_MSG_ERROR("failed to drv property - VIDC_I_CAPABILITY_PROFILE_LEVEL");
    }
    else
    {
        for (index = 0; index < size; index++)
        {
            if (ctrl[index].id == data->id)
            {
               ctrl[index].menu_skip_mask = ~(v4l2_profile_mask);

               data->minimum = ctrl[index].minimum;
               data->maximum = v4l2fe_log2(v4l2_profile_mask);
               data->default_value = ctrl[index].default_value;

               ctrl[index].maximum = data->maximum;

               HYP_VIDEO_MSG_INFO("Supported profiles for name %s min %d max %d mask 0x%x",
                                   ctrl[index].name, ctrl[index].minimum, ctrl[index].maximum,
                                   ctrl[index].menu_skip_mask);
               break;
            }
        }
        rc = HYPV_STATUS_SUCCESS;
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_query_level

@brief  FE enumerate supported levels for codec

@param [in] fe_ioss pointer
@param [in] v4l2fe_vidc_ctrl pointer
@param [in] size of ctrl struct
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_query_level(fe_io_session_t *fe_ioss, v4l2fe_vidc_ctrl *ctrl,
                                     int size, struct v4l2_queryctrl* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_capability_profile_level_type payload;
    int level_mask = 0, level = 0, v4l2_level_mask = 0;
    int index = 0;

    HABMM_MEMSET(&payload, 0, sizeof(vidc_capability_profile_level_type));

    HYP_VIDEO_MSG_INFO("query level for id %u", data->id);
    for (index = 0; HYPV_STATUS_SUCCESS == rc; index++)
    {
        payload.index = index;
        rc = dec_enc_get_drv_property(fe_ioss,
                VIDC_I_CAPABILITY_PROFILE_LEVEL,
                sizeof(vidc_capability_profile_level_type),
                (uint8*)&payload);
        if (V4L2_CID_MPEG_VIDEO_HEVC_LEVEL == data->id)
        {
           /* Bits: 0-27 shall be used to specify the level information. */
           level_mask |= (payload.level & 0xFFFFFFF);
        }
        else
        {
           level_mask |= payload.level;
        }

        /* VIDC & V4l2 Level enums don't have one-one mapping for VP8
        ** hence, convert the level received from driver to V4l2
        */
        if (V4L2_CID_MPEG_VIDC_VIDEO_VP8_PROFILE_LEVEL == data->id)
        {
           level = dec_enc_convert_vidc_to_v4l2(data->id, payload.level);
           if (0 <= level)
           {
              v4l2_level_mask |= (1 << level);
           }
           else
           {
              HYP_VIDEO_MSG_ERROR("Unsupported cmd 0x%x", data->id);
              rc = HYPV_STATUS_FAIL;
           }
        }
        else
        {
           v4l2_level_mask = level_mask;
        }
        HYP_VIDEO_MSG_INFO("index %d payload.level %u level_mask 0x%x",index, payload.level, (unsigned int)v4l2_level_mask);
    }

    data->flags = level_mask;

    if (HYPV_STATUS_SUCCESS != rc && 1 == index)
    {
        HYP_VIDEO_MSG_ERROR("failed to drv property - VIDC_I_CAPABILITY_PROFILE_LEVEL");
    }
    else
    {
       for (index = 0; index < size; index++)
       {
           if (ctrl[index].id == data->id)
           {
              data->minimum = ctrl[index].minimum;
              data->maximum = v4l2fe_log2(v4l2_level_mask);
              data->default_value = ctrl[index].default_value;

              ctrl[index].maximum = data->maximum;
              ctrl[index].menu_skip_mask = ~(v4l2_level_mask);
              HYP_VIDEO_MSG_INFO("Supported levels for name %s min %d max %d mask 0x%x",
                                  ctrl[index].name, ctrl[index].minimum, ctrl[index].maximum,
                                  ctrl[index].menu_skip_mask);
              break;
           }
       }
       rc = HYPV_STATUS_SUCCESS;
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_set_drv_property

@brief  Set encoder/decoder driver property

@param [in] fe_ioss pointer
@param [in] property Id
@param [in] packet size
@param [in] packet pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_set_drv_property (fe_io_session_t* fe_ioss, vidc_property_id_type   propId,
                                uint32 nPktSize, uint8* pPkt)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_drv_property_type* pProp = (vidc_drv_property_type* )fe_ioss->dev_cmd_buffer;
    int32 nMsgSize = sizeof(vidc_property_hdr_type) + nPktSize;
    hypv_session_t* hypv_session = fe_ioss->io_handle;

    HABMM_MEMCPY(pProp->payload, pPkt, nPktSize);
    pProp->prop_hdr.size    = nPktSize;
    pProp->prop_hdr.prop_id = propId;

    rc = hyp_device_ioctl(hypv_session,
                          VIDC_IOCTL_SET_PROPERTY,
                          fe_ioss->dev_cmd_buffer,
                          nMsgSize,
                          NULL,
                          0);

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set property, prop_id %d rc %d", propId, rc);
        if ((HYPV_STATUS_ERR_MAX_CLIENT != rc) && (HYPV_STATUS_ALLOC_FAIL != rc))
        {
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_get_drv_property

@brief  Get encoder/decoder driver property

@param [in] fe_ioss pointer
@param [in] property Id
@param [in] packet size
@param [in] packet pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_get_drv_property(fe_io_session_t* fe_ioss, vidc_property_id_type propId,
                               uint32 nPktSize, uint8* pPkt)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_drv_property_type* pProp = (vidc_drv_property_type* )fe_ioss->dev_cmd_buffer;
    int32 nMsgSize = sizeof( vidc_property_hdr_type ) + nPktSize;
    hypv_session_t* hypv_session = fe_ioss->io_handle;

    HABMM_MEMCPY(pProp->payload, pPkt, nPktSize);
    pProp->prop_hdr.size    = nPktSize;
    pProp->prop_hdr.prop_id = propId;

    if (HYPV_STATUS_SUCCESS != (rc = hyp_device_ioctl(hypv_session,
        VIDC_IOCTL_GET_PROPERTY,
        fe_ioss->dev_cmd_buffer,
        nMsgSize,
        pPkt,
        nPktSize)))
    {
        HYP_VIDEO_MSG_ERROR("failed to get property, prop_id %d rc %d", propId, rc);
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION get_available_store_entry

@brief  Get pointer to free entry in a store

@param [in] store
@param [in] store_size

@dependencies
  None

@return
  Returns vidc_buffer_info_type pointer

===========================================================================*/
static vidc_buffer_info_type *get_available_store_entry(vidc_buffer_info_type *store, uint32 store_size)
{
    uint32   i;

    // find the next available store_buffer_info
    for (i=0; i<store_size; i++)
    {
        if (store[i].buf_type == 0)
        {
            break;
        }
    }
    if (i >= store_size)
    {
        return NULL;
    }
    else
    {
        return &store[i];
    }
}

/**===========================================================================

FUNCTION dec_enc_prepare_buf

@brief  Encode/Decode prepare buffer

@param [in] fe_ioss pointer
@param [in] v4l2_buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_prepare_buf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_buffer_info_type       buffer_info;
    vidc_buffer_reqmnts_type    buffer_req_data;
    vidc_buffer_info_type*      store_buffer_info = NULL;
    uint32                      size_store_info = 0;
    uint32                      total_store_size = 0;
    hypv_session_t*             hypv_session = fe_ioss->io_handle;

    HABMM_MEMSET(&buffer_req_data, 0, sizeof(vidc_buffer_reqmnts_type));
    HABMM_MEMSET(&buffer_info, 0, sizeof(vidc_buffer_info_type));

    HYP_VIDEO_MSG_INFO("buf type %u", data->type);
    if (V4L2_MEMORY_DMABUF == data->memory)
    {
        HYP_VIDEO_MSG_INFO("address 0x%p fd %d",
                          (uint8 *)data->m.planes[0].m.userptr + data->m.planes[0].data_offset,
                           data->m.planes[0].m.fd);
    }
    else
    {
        HYP_VIDEO_MSG_INFO("address 0x%p fd %u",
                           (uint8 *)data->m.planes[0].m.userptr + data->m.planes[0].reserved[MSM_VIDC_DATA_OFFSET],
                            data->m.planes[0].reserved[MSM_VIDC_BUFFER_FD]);
    }
    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        buffer_info.buf_type = VIDC_BUFFER_INPUT;
        buffer_req_data.buf_type = VIDC_BUFFER_INPUT;
        size_store_info = fe_ioss->input_buffer_count;
        if (NULL == fe_ioss->vidc_input_buffer_info)
        {
            total_store_size = size_store_info*sizeof(vidc_buffer_info_type);
            fe_ioss->vidc_input_buffer_info =
                (vidc_buffer_info_type *)malloc(total_store_size);
            if (fe_ioss->vidc_input_buffer_info)
            {
                HABMM_MEMSET(fe_ioss->vidc_input_buffer_info, 0, total_store_size);
            }
        }

        if (fe_ioss->vidc_input_buffer_info)
        {
            store_buffer_info = get_available_store_entry(fe_ioss->vidc_input_buffer_info,
                                                          size_store_info);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to allocate vidc_input_buffer_info");
            rc = HYPV_STATUS_FAIL;
        }

        if (NULL == store_buffer_info)
        {
            HYP_VIDEO_MSG_ERROR("failed to get available store entry for input buffer");
            rc = HYPV_STATUS_FAIL;
        }
    }
    else
    {
        if (true == hypv_session->multi_stream_enable)
        {
            buffer_info.buf_type = VIDC_BUFFER_OUTPUT2;
            buffer_req_data.buf_type = VIDC_BUFFER_OUTPUT2;
        }
        else
        {
            buffer_info.buf_type = VIDC_BUFFER_OUTPUT;
            buffer_req_data.buf_type = VIDC_BUFFER_OUTPUT;
        }
        size_store_info = fe_ioss->output_buffer_count;
        if (NULL == fe_ioss->vidc_output_buffer_info)
        {
            total_store_size = size_store_info*sizeof(vidc_buffer_info_type);
            fe_ioss->vidc_output_buffer_info =
                (vidc_buffer_info_type *)malloc(total_store_size);
            if (fe_ioss->vidc_output_buffer_info)
            {
                HABMM_MEMSET(fe_ioss->vidc_output_buffer_info, 0, total_store_size);
            }
        }

        if (fe_ioss->vidc_output_buffer_info)
        {
            store_buffer_info = get_available_store_entry(fe_ioss->vidc_output_buffer_info,
                                                          size_store_info);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to allocate vidc_output_buffer_info");
            rc = HYPV_STATUS_FAIL;
        }

        if (NULL == store_buffer_info)
        {
            HYP_VIDEO_MSG_ERROR("failed to get available store entry for output buffer");
            rc = HYPV_STATUS_FAIL;
        }
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        buffer_info.buf_size = data->m.planes[0].length;
        if (V4L2_MEMORY_DMABUF == data->memory)
        {
            buffer_info.buf_addr = (uint8 *)(uintptr_t)data->m.planes[0].m.fd;
        }
        else
        {
            buffer_info.buf_addr = (uint8 *)(uintptr_t)data->m.planes[0].reserved[MSM_VIDC_BUFFER_FD];
        }
        if (data->length > 1)
        {
            buffer_info.extradata_buf_size = data->m.planes[1].length;
            buffer_info.extradata_buf_addr = (uint8 *)data->m.planes[1].m.userptr;
            buffer_info.contiguous = false;
        }
        else
        {
            // no extra data
            buffer_info.contiguous = true;
        }
        if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(hypv_session,
            VIDC_IOCTL_SET_BUFFER,
            (uint8 *)&buffer_info,
            sizeof(vidc_buffer_info_type),
            NULL,
            0))
        {
            rc = HYPV_STATUS_FAIL;
            HYP_VIDEO_MSG_ERROR("set buffer failed");
        }
        else
        {
            *store_buffer_info = buffer_info;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_apply_properties

@brief  Apply dec/enc properties

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_apply_properties(fe_io_session_t* fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    if (HYP_TARGET_LEMANS == fe_ioss->io_handle->target_variant)
    {
        dec_enc_enable_8255_metadata_tag(fe_ioss);
    }

    if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
    {
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_multi_slice(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_video_signal_info(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_vpe_csc(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_rate_control(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_hb_max_layer(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_hp_max_layer(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_hp_layer(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_grid_mode(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_idr_period(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc && TRUE == fe_ioss->enable_intra_refresh)
        {
            rc = enc_s_intra_refresh(fe_ioss, &fe_ioss->intra_refresh_info);
        }
        if (HYPV_STATUS_SUCCESS == rc && TRUE == fe_ioss->enable_blur_filter)
        {
            rc = enc_s_blur_dimensions(fe_ioss, &fe_ioss->blur_filter_info);
        }
        if (HYPV_STATUS_SUCCESS == rc && TRUE == fe_ioss->enable_bitrate_saving_mode)
        {
            rc = enc_s_bitrate_saving_mode(fe_ioss, &fe_ioss->bitrate_saving_mode);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_frame_qp(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_num_p_frames(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = enc_s_h264_entropy_mode(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = v4l2fe_enc_set_hdr_info(fe_ioss);
        }
    }

    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed with %d", rc);
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_streamon

@brief  Encode/Decode stream ON

@param [in] fe_ioss pointer
@param [in] v4l2_buf_type pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_streamon(fe_io_session_t* fe_ioss, enum v4l2_buf_type data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    int wait_executing_state = 0;
    fe_linux_plt_data_t *linux_plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);

    HYP_VIDEO_MSG_INFO("stream on buf type %d state %u", data, linux_plt_data->state);
    // has to ensure the state has changed to V4L2FE_STATE_EXECUTING
    // only consider the state
    switch (linux_plt_data->state)
    {
    case V4L2FE_STATE_DEINIT:
    case V4L2FE_STATE_UNUSED:
        HYP_VIDEO_MSG_ERROR("failed in deinit/unused state");
        rc = HYPV_STATUS_FAIL;
        break;
    case V4L2FE_STATE_LOADED:
        rc = dec_enc_apply_properties(fe_ioss);
        HYP_VIDEO_MSG_INFO("VIDC_IOCTL_LOAD_RESOURCES@V4L2FE_STATE_LOADED");
        rc = dec_enc_submit_command(fe_ioss, VIDC_IOCTL_LOAD_RESOURCES);
        [[clang::fallthrough]];
        // let it flow through to the IDLE state flow no need to wait for the state change here
    case V4L2FE_STATE_IDLE:
        HYP_VIDEO_MSG_INFO("VIDC_IOCTL_START@V4L2FE_STATE_IDLE");
        rc = dec_enc_submit_command(fe_ioss, VIDC_IOCTL_START);
        wait_executing_state = 1;
        break;
    case V4L2FE_STATE_EXECUTING:
        // already there, no further processing
        HYP_VIDEO_MSG_INFO("V4L2FE_STATE_EXECUTING");
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data)
        {
            fe_ioss->io_handle->in_output_reconfig = FALSE;
        }
        break;
    case V4L2FE_STATE_PAUSE:
        HYP_VIDEO_MSG_INFO("VIDC_IOCTL_RESUME@V4L2FE_STATE_PAUSE");
        rc = dec_enc_submit_command(fe_ioss, VIDC_IOCTL_RESUME);
        wait_executing_state = 1;
        break;
    default:
        HYP_VIDEO_MSG_INFO("Wrong state=%u", linux_plt_data->state);
        break;
    }
    if ((HYPV_STATUS_SUCCESS == rc) && (0 != wait_executing_state))
    {
        int32 time_out = 0;
        MM_SignalQ_TimedWait(fe_ioss->state_synch_obj_q, WAIT_STATE_TIMEOUT, NULL, &time_out);
        if (time_out)
        {
            HYP_VIDEO_MSG_ERROR("wait time out on MM SignalQ");
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            if (V4L2FE_STATE_EXECUTING != linux_plt_data->state)
            {
                HYP_VIDEO_MSG_ERROR("end@state=%u",linux_plt_data->state);
                rc = HYPV_STATUS_FAIL;
            }
        }
    }
    return rc;
}

/**===========================================================================

FUNCTION dec_enc_qbuf

@brief  Encode/Decode queue buffer

@param [in] fe_ioss pointer
@param [in] v4l2_buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_qbuf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    vidc_frame_data_type frame_data;
    fe_linux_plt_data_t *linux_plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
    hypv_session_t* hypv_session = fe_ioss->io_handle;
    uintptr_t frame_address = 0, frame_address_plane1 = 0;
    unsigned int data_offset = 0;

    HABMM_MEMSET(&frame_data, 0, sizeof(vidc_frame_data_type));

    HYP_VIDEO_MSG_INFO("buf type %u", data->type);
    if (V4L2_MEMORY_DMABUF == data->memory)
    {
        frame_address = (uintptr_t)data->m.planes[0].m.fd;
        data_offset = data->m.planes[0].data_offset;
        frame_address_plane1 = (uintptr_t)data->m.planes[1].m.fd;
    }
    else
    {
        frame_address = (uintptr_t)data->m.planes[0].reserved[MSM_VIDC_BUFFER_FD];
        data_offset = data->m.planes[0].reserved[MSM_VIDC_DATA_OFFSET];
        frame_address_plane1 = (uintptr_t)data->m.planes[1].reserved[MSM_VIDC_BUFFER_FD];
    }
    frame_data.frame_addr = (uint8 *)frame_address;
    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        uint32 i;

        if (!fe_ioss->vidc_input_buffer_size)
        {
           fe_ioss->vidc_input_buffer_size = data->m.planes[0].length;
        }
        if (VIDEO_SESSION_ENCODE == fe_ioss->video_session)
        {
            if (frame_address_plane1)
            {
               frame_data.metadata_addr = (uint8 *)frame_address_plane1;
               hypv_session->export_as_fd = true;
               frame_data.alloc_metadata_len = data->m.planes[1].length;
            }
            else
            {
                frame_data.metadata_addr = (uint8 *)data->m.planes[1].m.userptr;
                frame_data.alloc_metadata_len = data->m.planes[1].length / MAX_EXTRADATA_BUFS;
            }
         }

         /* allocate metadata buffer internally rather than in C2
          * to support lemans with backward compatibility
          */
         if (HYP_TARGET_LEMANS == hypv_session->target_variant)
         {
            if (VIDEO_SESSION_DECODE == fe_ioss->video_session)
            {
#ifdef SUPPORT_DMABUF
                uint32 buffer_size = ALIGN(ALLOC_METADATA_LEN, 4096);
                std::string dmaBufHeapName = "qcom,system";
#else
                struct ion_allocation_data alloc_data;

                HABMM_MEMSET(&alloc_data, 0, sizeof(ion_allocation_data));

                alloc_data.len = ALIGN(ALLOC_METADATA_LEN, 4096);
                alloc_data.flags = 0;
                alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
#endif

               int meta_fd = -1;

#ifdef SUPPORT_DMABUF
               rc = dec_enc_allocate_meta_buffer(buffer_size, dmaBufHeapName, &meta_fd);
#else
               rc = dec_enc_allocate_meta_buffer(&alloc_data, &meta_fd);
#endif

               if (HYPV_STATUS_SUCCESS == rc)
               {
                  frame_address_plane1 = (uintptr_t)meta_fd;

                  frame_data.metadata_addr = (uint8 *)frame_address_plane1;
                  hypv_session->export_as_fd = true;

#ifdef SUPPORT_DMABUF
                  frame_data.alloc_metadata_len = buffer_size;
#else
                  frame_data.alloc_metadata_len = alloc_data.len;
#endif
               }
               else
               {
                  HYP_VIDEO_MSG_ERROR("malloc metadata failed");
               }
            }
         }

        if (NULL != frame_data.metadata_addr)
        {
            frame_data.non_contiguous_metadata = true;
        }
        HYP_VIDEO_MSG_HIGH("ETB: session %d frame addr %p fd %d length %u flag 0x%x byteused %u index %u",
                            fe_ioss->video_session,
                            (uint8 *)data->m.planes[0].m.userptr + data_offset,
                            (int)frame_address, data->m.planes[0].length,
                            data->flags, data->m.planes[0].bytesused, data->index);
        HYP_VIDEO_MSG_HIGH("ETB: metadata addr %p metadata len %u meta contiguous %d",
                            (void *)frame_data.metadata_addr, frame_data.alloc_metadata_len,
                            !frame_data.non_contiguous_metadata);

        /* Shared data strcutures are updated in EBD callback
        ** hence, take lock to prevent contention
        */
        MM_CriticalSection_Enter(fe_ioss->lock_buffer);
        if (HYPV_STATUS_SUCCESS == rc &&
            NULL == linux_plt_data->v4l2fe_etb_flag_info)
        {
            uint32 total_size = fe_ioss->input_buffer_count*sizeof(v4l2fe_etb_flag_info_type);
            linux_plt_data->v4l2fe_etb_flag_info =
                (v4l2fe_etb_flag_info_type *)malloc(total_size);
            if (linux_plt_data->v4l2fe_etb_flag_info)
            {
                HABMM_MEMSET(linux_plt_data->v4l2fe_etb_flag_info, 0, total_size);
            }
            else
            {
               HYP_VIDEO_MSG_ERROR("malloc failed, v4l2fe_etb_flag_info is NULL");
               rc = HYPV_STATUS_FAIL;
            }
        }

        if ( HYPV_STATUS_SUCCESS == rc )
        {
           for ( i = 0; i < fe_ioss->input_buffer_count; i++ )
           {
               if (linux_plt_data->v4l2fe_etb_flag_info &&
                   (0 == linux_plt_data->v4l2fe_etb_flag_info[i].is_etb))
               {
                   // found an empty spot
                   linux_plt_data->v4l2fe_etb_flag_info[i].is_etb = 1;
                   linux_plt_data->v4l2fe_etb_flag_info[i].address = frame_data.frame_addr;
                   linux_plt_data->v4l2fe_etb_flag_info[i].flags = data->flags;
                   break;
               }
           }
           frame_data.buf_type = VIDC_BUFFER_INPUT;

           frame_data.alloc_len = data->m.planes[0].length;
           frame_data.data_len = data->m.planes[0].bytesused;
           frame_data.offset = 0;
           if (data->flags & V4L2_BUF_FLAG_EOS)
           {
               frame_data.flags |= VIDC_FRAME_FLAG_EOS;
               //sa6155 has an issue with EMPTY EOS buffer.
               /*if (0 == frame_data.data_len)
               {
                   //Need to update with alloc_len since FW does not update the input tag
                   //if data_len is set to 0.
                   frame_data.data_len = frame_data.alloc_len;
               }*/
           }
           if (data->flags & V4L2_BUF_FLAG_CODECCONFIG)
           {
               frame_data.flags |= VIDC_FRAME_FLAG_CODECCONFIG;
           }
           if (frame_data.alloc_metadata_len && hypv_session->input_extradata_enable)
           {
               frame_data.flags |= VIDC_FRAME_FLAG_METADATA;
           }
        }
        MM_CriticalSection_Leave(fe_ioss->lock_buffer);

        hypv_session->etb_count++;

        //Need to update mark_data & mark_target to get FBD for buffer
        // which Video FW discards
        frame_data.mark_target = 0xFF;
        frame_data.mark_data = 0xFF;

        if (HYPV_STATUS_SUCCESS == rc)
        {
            frame_data.timestamp = (uint64) V4L2FE_CONVERT_SEC_TO_USEC(data->timestamp.tv_sec) +
                                   (uint64) data->timestamp.tv_usec;
            if ((0 != data->m.planes[0].reserved[MSM_VIDC_INPUT_TAG_1]))
            {
               frame_data.frm_clnt_data = data->m.planes[0].reserved[MSM_VIDC_INPUT_TAG_1];
               frame_data.input_tag = data->m.planes[0].reserved[MSM_VIDC_INPUT_TAG_1];
            }
            else
            {
               frame_data.frm_clnt_data = data->index;
               frame_data.input_tag = data->index;
            }
            //Map is needed to retrieve the correct buffer index in EBD.
            MM_CriticalSection_Enter(fe_ioss->lock_buffer);
            fe_ioss->input_tag_entry->insert(std::pair<uint64, uint64>(frame_data.frm_clnt_data, data->index));
            MM_CriticalSection_Leave(fe_ioss->lock_buffer);
            HYP_VIDEO_MSG_HIGH("MAP Entry: frm_clnt_data %lu index %lu", (unsigned long)frame_data.frm_clnt_data, (unsigned long)data->index);
            if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(hypv_session,
                VIDC_IOCTL_EMPTY_INPUT_BUFFER,
                (uint8 *)&frame_data,
                sizeof(vidc_frame_data_type),
                NULL,
                0))
            {
                rc = HYPV_STATUS_FAIL;
                HYP_VIDEO_MSG_ERROR("failed to empty input buffer");
            }
        }
    }
    else
    {
        uint32 dyn_index = 0;

        if (true == hypv_session->multi_stream_enable)
        {
            frame_data.buf_type = VIDC_BUFFER_OUTPUT2;
        }
        else
        {
            frame_data.buf_type = VIDC_BUFFER_OUTPUT;
        }

        /* FTB and release buffer reference threads can
        ** happen in parallel and corrupt the dynamic buf
        ** pool index eventually causing refcount to be
        ** incorrect. Hence, take the lock
        */
        MM_CriticalSection_Enter(fe_ioss->lock_buffer);
        if (NULL == fe_ioss->out_dynamic_info)
        {
            rc = dec_enc_create_dyn_buf(fe_ioss);
        }
        if (HYPV_STATUS_SUCCESS == rc)
        {
            rc = dec_enc_add_dyn_buf_ref(fe_ioss, data,
                                         &frame_data, &dyn_index);
        }

        if (HYPV_STATUS_SUCCESS == rc)
        {
           frame_data.metadata_addr = (uint8 *)data->m.planes[1].m.userptr;
           if (frame_address_plane1)
           {
               frame_data.metadata_addr = (uint8 *)frame_address_plane1;
               hypv_session->export_as_fd = true;
           }
           frame_data.alloc_len = data->m.planes[0].length;
           frame_data.data_len = data->m.planes[0].bytesused;
           frame_data.offset = 0;
           frame_data.alloc_metadata_len = data->m.planes[1].length;
           if (NULL != frame_data.metadata_addr)
           {
               frame_data.non_contiguous_metadata = true;
           }
           frame_data.frm_clnt_data = (uint64) data->index;
           hypv_session->ftb_count++;

           HYP_VIDEO_MSG_HIGH("FTB: index %u frame fd %d session %d buf type %d length %u"
                              " metadata addr %p fd %d metadata len %u"
                              " meta contiguous %d",
                               data->index,
                               (int)frame_address,
                               fe_ioss->video_session, frame_data.buf_type,
                               data->m.planes[0].length,
                               (void *)data->m.planes[1].m.userptr,
                               (int)frame_address_plane1,
                               data->m.planes[1].length,
                               !frame_data.non_contiguous_metadata);

           //ToDo : Remove this condition if Encode works on 6155
           //With ftb_count = 1, two copies of CSD buffer is sent to Codec2.
           //Due to this QuTest is skipping first IDR frame while writing to file
           if ((VIDEO_SESSION_ENCODE == fe_ioss->video_session) &&
               (0 == hypv_session->ftb_count) &&
               (VIDC_CODEC_VP8 != hypv_session->codec) &&
               (VIDC_CODEC_H263 != hypv_session->codec))
           {
               /* reserve the first encoder output frame for header info */
               HABMM_MEMCPY(&hypv_session->first_frame_info, &frame_data,
                             sizeof(vidc_frame_data_type));
               HYP_VIDEO_MSG_INFO("FTB: First encode output buffer %p not pushed to BE",
                                   frame_data.frame_addr);
           }
           else
           {
               if (1 == fe_ioss->out_dynamic_info[dyn_index].ref_count)
               {
                   /* Queue buffer to BE only if it has been released by FW */
                   if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(hypv_session,
                       VIDC_IOCTL_FILL_OUTPUT_BUFFER,
                       (uint8 *)&frame_data,
                       sizeof(vidc_frame_data_type),
                       NULL, 0))
                   {
                       rc = HYPV_STATUS_FAIL;
                       HYP_VIDEO_MSG_ERROR("failed to fill output buffer");
                   }
               }
           }
        }
        MM_CriticalSection_Leave(fe_ioss->lock_buffer);
    }

    return rc;
}

hypv_status_type dec_enc_streamoff_internal(fe_io_session_t* fe_ioss, uint32 vidc_cmd)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_linux_plt_data_t *linux_plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
    v4l2fe_state_t state = V4L2FE_STATE_IDLE;
    std::string cmd = "VIDC_IOCTL_STOP";

    switch (vidc_cmd)
    {
        case VIDC_IOCTL_STOP:
             state = V4L2FE_STATE_IDLE;
             cmd = "VIDC_IOCTL_STOP";
             break;

        case VIDC_IOCTL_RELEASE_RESOURCES:
             state = V4L2FE_STATE_LOADED;
             cmd = "VIDC_IOCTL_RELEASE_RESOURCES";
             break;
        default:
            HYP_VIDEO_MSG_INFO("Unexpected vidc_cmd=%u", vidc_cmd);
            break;
    }

    rc = dec_enc_submit_command(fe_ioss, vidc_cmd);
    if (HYPV_STATUS_SUCCESS == rc)
    {
        int32 time_out = 0;
        MM_SignalQ_TimedWait(fe_ioss->state_synch_obj_q, WAIT_STATE_TIMEOUT, NULL, &time_out);
        if (time_out)
        {
            HYP_VIDEO_MSG_ERROR("wait time out on MM SignalQ");
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            if (state != linux_plt_data->state)
            {
                HYP_VIDEO_MSG_ERROR("end@state=%u",linux_plt_data->state);
                rc = HYPV_STATUS_FAIL;
            }
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("%s failed", cmd.c_str());
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_streamoff

@brief  Encode/Decode stream OFF function

@param [in] fe_ioss pointer
@param [in] v4l2_buf_type

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_streamoff(fe_io_session_t* fe_ioss, enum v4l2_buf_type data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_buffer_info_type **store_buffer_ptr, *store_buffer;
    uint32* store_size_ptr;
    uint32   i;
    fe_linux_plt_data_t *linux_plt_data = (fe_linux_plt_data_t *)(fe_ioss->fe_plt_data);
    hypv_session_t* hypv_session = fe_ioss->io_handle;

    HYP_VIDEO_MSG_INFO("v4l2 buf type %d and state %u", data, linux_plt_data->state);
    // Need to ensure the VIDC in idle state
    if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data)
    {
        store_buffer_ptr = &fe_ioss->vidc_input_buffer_info;
        store_size_ptr = &fe_ioss->input_buffer_count;
        free(linux_plt_data->v4l2fe_etb_flag_info);
        linux_plt_data->v4l2fe_etb_flag_info = 0;
    }
    else if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data)
    {
        store_buffer_ptr = &fe_ioss->vidc_output_buffer_info;
        store_size_ptr = &fe_ioss->output_buffer_count;

        MM_CriticalSection_Enter( fe_ioss->lock_buffer );
        dec_enc_destroy_dyn_buf(fe_ioss);
        MM_CriticalSection_Leave( fe_ioss->lock_buffer );
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("unsupported v4l2 buf type %d", data);
        rc = HYPV_STATUS_FAIL;
    }
    if (HYPV_STATUS_SUCCESS == rc)
    {
        store_buffer = *store_buffer_ptr;
        if (NULL != store_buffer) /* static buffer mode */
        {
            HYP_VIDEO_MSG_INFO("state = %u", linux_plt_data->state);
            for (i = 0; i < *store_size_ptr; i++)
            {
               if (store_buffer[i].buf_type != 0)
               {
                  if (true == hypv_session->multi_stream_enable &&
                      V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data)
                  {
                     store_buffer[i].buf_type = VIDC_BUFFER_OUTPUT2;
                  }
                  if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(hypv_session,
                                                 VIDC_IOCTL_FREE_BUFFER,
                                                 (uint8 *)&store_buffer[i],
                                                 sizeof(vidc_buffer_info_type),
                                                 NULL, 0))
                  {
                     rc = HYPV_STATUS_FAIL;
                     HYP_VIDEO_MSG_ERROR("failed to free buffer - buf type %d, frame addr 0x%p",
                                          store_buffer[i].buf_type,store_buffer[i].buf_addr);
                  }
                  else
                  {
                     HYP_VIDEO_MSG_INFO("free buf type %d, frame addr 0x%p",
                                         store_buffer[i].buf_type,store_buffer[i].buf_addr);
                  }
               }
            }
            free(store_buffer);
            *store_buffer_ptr = NULL;
            *store_size_ptr = 0;
        }
        else /* dynamic buffer mode */
        {
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data)
            {
                vidc_buffer_info_64b_type output_buf;
                vidc_buffer_type buf_type;

                HABMM_MEMSET(&output_buf, 0, sizeof(vidc_buffer_info_64b_type));
                if (true == hypv_session->multi_stream_enable)
                {
                    buf_type = VIDC_BUFFER_OUTPUT2;
                }
                else
                {
                    buf_type = VIDC_BUFFER_OUTPUT;
                }
                output_buf.buf_type = buf_type;

                /* If ftb_count or etb_count is zero then buffer freeing is
                 * not required. In such case it would try to free static buffer if dynamic
                 * buffer mode is not set properly. Add check for secondary buffer in case
                 * of multistream.
                */
                if (((VIDC_BUFFER_INPUT == buf_type) && (0 < hypv_session->etb_count)) ||
                    ((VIDC_BUFFER_OUTPUT2 == buf_type || VIDC_BUFFER_OUTPUT == buf_type) &&
                    (0 < hypv_session->ftb_count)))
                {
                     HYP_VIDEO_MSG_HIGH("freeing the buffer of buf type %d", buf_type);
                     if (HYPV_STATUS_SUCCESS != hyp_device_ioctl(hypv_session,
                                               VIDC_IOCTL_FREE_BUFFER,
                                               (uint8 *)&output_buf,
                                               sizeof(vidc_buffer_info_64b_type),
                                               NULL, 0))
                     {
                         rc = HYPV_STATUS_FAIL;
                         HYP_VIDEO_MSG_ERROR("failed to free output buffer");
                     }
                }

                /* free up any resource from BE */
                if (FALSE == fe_ioss->io_handle->in_output_reconfig)
                {
                    rc = dec_enc_streamoff_internal(fe_ioss, VIDC_IOCTL_STOP);
                    if (HYPV_STATUS_SUCCESS == rc)
                    {
                        rc = dec_enc_streamoff_internal(fe_ioss, VIDC_IOCTL_RELEASE_RESOURCES);
                    }
                }
                hypv_map_cleanup_buf_type(&hypv_session->habmm_if,
                                          &hypv_session->lookup_queue,
                                          buf_type);
                fe_ioss->io_handle->multi_stream_enable = FALSE;
            }
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_enum_framesizes

@brief  Encode/Decode enum framesize

@param [in] fe_ioss pointer
@param [in] v4l2_frmsizeenum pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_enum_framesizes(fe_io_session_t* fe_ioss, struct v4l2_frmsizeenum* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_capability_frame_size_type  cap_frame_size;

    HABMM_MEMSET(&cap_frame_size, 0, sizeof(vidc_capability_frame_size_type));

    /* Make sure to set session codec before querying
       frame size capability */
    if (0 == fe_ioss->session_codec.codec)
    {
        vidc_session_codec_type session_codec;

        session_codec.session = fe_ioss->session_codec.session;
        session_codec.codec = codecV4l2ToVidc(data->pixel_format);
        if (VIDC_CODEC_UNUSED == session_codec.codec)
        {
            HYP_VIDEO_MSG_ERROR("bad codec %u", data->pixel_format);
            rc = HYPV_STATUS_FAIL;
        }
        else
        {
            rc = dec_enc_set_drv_property(fe_ioss, VIDC_I_SESSION_CODEC,
                                          sizeof(vidc_session_codec_type),
                                          ( uint8* )&session_codec );
            if (HYPV_STATUS_SUCCESS == rc)
            {
                if (VIDC_SESSION_DECODE == fe_ioss->session_codec.session)
                {
                    fe_ioss->input_fourCC = data->pixel_format;
                }
                else if (VIDC_SESSION_ENCODE == fe_ioss->session_codec.session)
                {
                    fe_ioss->output_fourCC = data->pixel_format;
                }
                fe_ioss->session_codec = session_codec;
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("failed to set session codec");
            }
        }
    }

    if (rc == HYPV_STATUS_SUCCESS)
    {
        rc = dec_enc_get_drv_property( fe_ioss, VIDC_I_CAPABILITY_FRAME_SIZE,
                                       sizeof( vidc_capability_frame_size_type ),
                                       ( uint8* )&cap_frame_size );
        if (rc == HYPV_STATUS_SUCCESS)
        {
            data->type = V4L2_FRMSIZE_TYPE_STEPWISE;
            data->stepwise.min_width = cap_frame_size.width.min;
            data->stepwise.max_width = cap_frame_size.width.max;
            data->stepwise.step_width = cap_frame_size.width.step_size;
            data->stepwise.min_height = cap_frame_size.height.min;
            data->stepwise.max_height = cap_frame_size.height.max;
            data->stepwise.step_height = cap_frame_size.height.step_size;
            HYP_VIDEO_MSG_INFO("get drv property capability frame size. width=%u-%u step=%u height=%u-%u step=%u",
                                data->stepwise.min_width, data->stepwise.max_width,
                                data->stepwise.step_width, data->stepwise.min_height,
                                data->stepwise.max_height,data->stepwise.step_height);
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("failed to get drv capability frame size property for data type %u",
                                 data->type);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_subscribe_event

@brief  Encode/Decode subscribe for an event

@param [in] fe_ioss pointer
@param [in] v4l2_event_subscription pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_subscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("type=%u id=%u",data->type, data->id);
    return rc;
}

/**===========================================================================

FUNCTION dec_enc_unsubscribe_event

@brief  Encode/Decode unsubscribe an event

@param [in] fe_ioss pointer
@param [in] v4l2_event_subscription pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_unsubscribe_event(fe_io_session_t* fe_ioss, struct v4l2_event_subscription* data)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("type=%u id=%u",data->type, data->id);
    return rc;
}

/**===========================================================================

FUNCTION dec_enc_querycap

@brief  Encode/Decode query cababilities

@param [in] fe_ioss pointer
@param [in] v4l2_capability pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_querycap(fe_io_session_t* fe_ioss, struct v4l2_capability* data)
{

    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    UNUSED(fe_ioss);

    HYP_VIDEO_MSG_INFO("enter");
    HABMM_MEMSET(data, 0, sizeof(struct v4l2_capability));
    return rc;
}

/**===========================================================================

FUNCTION dec_enc_s_ext_ctrl

@brief  Encode/Decode set ext controls

@param [in] fe_ioss pointer
@param [in] ext v4l2_fmtdesc pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_s_ext_ctrl(fe_io_session_t* fe_ioss, struct v4l2_ext_controls* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_LOW("set ext control: id 0x%x value 0x%x",
                       data->controls[0].id, (unsigned int)data->controls[0].value);
    if (V4L2_CID_MPEG_VIDC_VIDEO_STREAM_OUTPUT_MODE == data->controls[0].id)
    {
        if (V4L2_CID_MPEG_VIDC_VIDEO_STREAM_OUTPUT_SECONDARY == data->controls[0].value)
        {
            rc = v4l2fe_decoder_multistream_config(fe_ioss);
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION v4l2fe_vdec_venc_ioctl_passthrough

@brief  Video decode and encode ioctl passthrough

@param [in] fe_ioss
@param [in] cmd
@param [in] data point

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type v4l2fe_vdec_venc_ioctl_passthrough(fe_io_session_t *fe_ioss, uint32 cmd, void* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 size = 0;
    unsigned long userptr = 0, userptr_extradata = 0;
    unsigned long extradata_size = 0;
    hypv_session_t* hypv_session = fe_ioss->io_handle;
    uint32 temp_cmd = cmd;
    void* temp_data = data;
    struct v4l2_format_64b fmt;
    struct v4l2_buffer_64b exp_buf;

    HABMM_MEMSET(&fmt, 0, sizeof(struct v4l2_format_64b));
    HABMM_MEMSET(&exp_buf, 0, sizeof(struct v4l2_buffer_64b));

    switch (cmd)
    {
    case VIDIOC_S_FMT:
    case VIDIOC_G_FMT:
        {
            struct v4l2_format* temp = (struct v4l2_format*)data;
            if (VIDIOC_S_FMT == cmd)
            {
                //Need to overwrite with 64 bit value since LV BE is 64 bit
                cmd = VIDIOC_S_FMT_64_BIT;
            }
            else
            {
                //Need to overwrite with 64 bit value since LV BE is 64 bit
                cmd = VIDIOC_G_FMT_64_BIT;
            }

            fmt.type = temp->type;
            HABMM_MEMCPY(&fmt.fmt.raw_data[0], &temp->fmt.raw_data[0], 200);
            data = &fmt;

            size = sizeof(struct v4l2_format_64b);
            break;
        }
    case VIDIOC_S_EXT_CTRLS:
        {
            size = sizeof(struct v4l2_ext_controls);
            break;
        }
    case VIDIOC_REQBUFS:
        {
            size = sizeof(struct v4l2_requestbuffers);
            struct v4l2_requestbuffers* buf_req = (struct v4l2_requestbuffers*)data;
            if(0 == buf_req->count)
            {
                HYP_VIDEO_MSG_INFO("Removing %s buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == buf_req->type)?"input":"output");
                hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                             &hypv_session->lookup_queue_ex,
                             buf_req->type);
                if (hypv_session->num_output_planes > 1)
                {
                    HYP_VIDEO_MSG_INFO("Removing %s extradata buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == buf_req->type)?"input":"output");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 buf_req->type);
                }
                if (hypv_session->num_input_planes > 1)
                {
                    HYP_VIDEO_MSG_INFO("Removing %s extradata buffer map entries", (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == buf_req->type)?"input":"output");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 buf_req->type);
                }
            }
            break;
        }
    case VIDIOC_G_CTRL:
    case VIDIOC_S_CTRL:
        {
            size = sizeof(struct v4l2_control);
            break;
        }
    case VIDIOC_DQBUF:
        {
            rc = dec_enc_dqbuf(fe_ioss, (struct v4l2_buffer *)data);
            break;
        }
    case VIDIOC_PREPARE_BUF:
    case VIDIOC_QBUF:
        {
            struct v4l2_buffer *buf = (struct v4l2_buffer *)data;
            size = sizeof(struct v4l2_buffer_64b);
            if (VIDIOC_PREPARE_BUF == cmd)
            {
                //Need to overwrite with 64 bit value since LV BE is 64 bit
                cmd = VIDIOC_PREPARE_BUF_64_BIT;
            }
            else
            {
                //Need to overwrite with 64 bit value since LV BE is 64 bit
                cmd = VIDIOC_QBUF_64_BIT;
            }

            exp_buf.index = buf->index;
            exp_buf.type = buf->type;
            exp_buf.bytesused = buf->bytesused;
            exp_buf.flags = buf->flags;
            exp_buf.field = buf->field;
            exp_buf.tv_sec = buf->timestamp.tv_sec;
            exp_buf.tv_usec = buf->timestamp.tv_usec;
            HABMM_MEMCPY(&exp_buf.timecode, &buf->timecode, sizeof(struct v4l2_timecode));
            exp_buf.sequence = buf->sequence;
            exp_buf.memory = buf->memory;
            for (uint i = 0; i < buf->length; i++)
            {
                exp_buf.m.planes[i].bytesused = buf->m.planes[i].bytesused;
                exp_buf.m.planes[i].length = buf->m.planes[i].length;
                exp_buf.m.planes[i].m.userptr = buf->m.planes[i].m.userptr;
                exp_buf.m.planes[i].data_offset = buf->m.planes[i].data_offset;
                HABMM_MEMCPY(&exp_buf.m.planes[i].reserved[0], &buf->m.planes[i].reserved[0], 44);
                exp_buf.m.planes[i].m.fd = buf->m.planes[i].m.fd;
            }
            exp_buf.length = buf->length;
            exp_buf.reserved2 = buf->reserved2;
            exp_buf.reserved = buf->reserved;

            userptr = V4L2_MEMORY_DMABUF == buf->memory ? buf->m.planes[0].m.fd : buf->m.planes[0].reserved[MSM_VIDC_BUFFER_FD];
            if (1 < buf->length)
            {
                userptr_extradata = V4L2_MEMORY_DMABUF == buf->memory ? buf->m.planes[1].m.fd : buf->m.planes[1].reserved[MSM_VIDC_BUFFER_FD];
                extradata_size = buf->m.planes[1].length;
            }
            hypv_map_entry_ex_t* entry = hypv_map_add_to_remote_ext(&hypv_session->habmm_if,
                                                                   &hypv_session->lookup_queue_ex,
                                                                   hypv_session->habmm_handle,
                                                                   buf->index,
                                                                   (void *)(uintptr_t)userptr,
                                                                   buf->m.planes[0].length,
                                                                   (void *)(uintptr_t)userptr_extradata,
                                                                   extradata_size,
                                                                   buf->type);

            if (NULL != entry)
            {
                if (V4L2_MEMORY_DMABUF != buf->memory)
                {
                    exp_buf.m.planes[0].reserved[MSM_VIDC_BUFFER_FD] = entry->frame_bufferid;
                    if (1 < buf->length)
                    {
                        exp_buf.m.planes[1].reserved[MSM_VIDC_BUFFER_FD] = entry->extradata_bufferid;
                    }
                }
                else
                {
                    exp_buf.m.planes[0].m.fd = entry->frame_bufferid;
                    if (1 < buf->length)
                    {
                        exp_buf.m.planes[1].m.fd = entry->extradata_bufferid;
                    }
                }

                if (TRUE == entry->is_readonly)
                {
                    // Need to handle this scenario.
                    size = 0;
                    HYP_VIDEO_MSG_INFO("Skipping QBUF since buffer is READONLY");
                }
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("Failed to export buffer for index %u", buf->index);
                size = 0;
            }
            data = &exp_buf;
            break;
        }
    case VIDIOC_STREAMON:
        {
            size = sizeof(enum v4l2_buf_type);
            break;
        }
    case VIDIOC_SUBSCRIBE_EVENT:
    case VIDIOC_UNSUBSCRIBE_EVENT:
        {
            size = sizeof(struct v4l2_event_subscription);
            break;
        }
    case VIDIOC_QUERYCAP:
        {
            size = sizeof(struct v4l2_capability);
            break;
        }
    case VIDIOC_QUERYCTRL:
        {
            size = sizeof(struct v4l2_queryctrl);
            break;
        }
    case VIDIOC_QUERYMENU:
        {
            size = sizeof(struct v4l2_querymenu);
            break;
        }
    case VIDIOC_ENUM_FMT:
        {
            size = sizeof(struct v4l2_fmtdesc);
            break;
        }
    case VIDIOC_DECODER_CMD:
        {
            size = sizeof(struct v4l2_decoder_cmd);
            break;
        }
    case VIDIOC_ENCODER_CMD:
        {
            size = sizeof(struct v4l2_encoder_cmd);
            break;
        }

    case VIDIOC_STREAMOFF:
        {
            size = sizeof(enum v4l2_buf_type);
            enum v4l2_buf_type *buf_type = (enum v4l2_buf_type*)data;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == *buf_type)
            {
                HYP_VIDEO_MSG_INFO("VIDIOC_STREAMOFF:Removing output buffer map entries");
                hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                             &hypv_session->lookup_queue_ex,
                             V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
                if (1 < hypv_session->num_output_planes)
                {
                    HYP_VIDEO_MSG_INFO("VIDIOC_STREAMOFF:Removing output extradata buffer map entries");
                    hypv_map_cleanup_buf_type_ext(&hypv_session->habmm_if,
                                 &hypv_session->lookup_queue_ex,
                                 V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
                }
            }
            break;
        }
    case VIDIOC_S_PARM:
        {
            size = sizeof(struct v4l2_streamparm);
            break;
        }
    case VIDIOC_ENUM_FRAMESIZES:
        {
            size = sizeof(struct v4l2_frmsizeenum);
            break;
        }
    case VIDIOC_DQEVENT:
        {
            rc = dec_enc_dqevent(fe_ioss, (struct v4l2_event*)data);
            break;
        }
     default:
            HYP_VIDEO_MSG_ERROR("vdec_venc cmd not found %x !!", cmd);
            break;
    }

    if (HYPV_STATUS_SUCCESS == rc && size)
    {
        rc = hyp_device_ioctl(hypv_session, cmd, (uint8*)data, size, (uint8*)data, size);
        cmd = temp_cmd;
        if (VIDIOC_PREPARE_BUF == cmd || VIDIOC_QBUF == cmd)
        {
            struct v4l2_buffer *buf = (struct v4l2_buffer *)temp_data;

            /* restore the va from export id */
            if (V4L2_MEMORY_DMABUF != buf->memory)
            {
                buf->m.planes[0].reserved[MSM_VIDC_BUFFER_FD] = userptr;

                if (1 < buf->length)
                {
                    buf->m.planes[1].reserved[MSM_VIDC_BUFFER_FD] = userptr_extradata;
                }
            }
            else
            {
                buf->m.planes[0].m.fd = userptr;

                if (1 < buf->length)
                {
                    buf->m.planes[1].m.fd = userptr_extradata;
                }
            }
        }
        if (VIDIOC_G_FMT == cmd)
        {
            struct v4l2_format* fmt_copy = (struct v4l2_format*)temp_data;
            struct v4l2_format_64b* temp = (struct v4l2_format_64b*)data;
            fmt_copy->type = temp->type;
            HABMM_MEMCPY(&fmt_copy->fmt.raw_data[0], &temp->fmt.raw_data[0], 200);
            if ((HYPV_STATUS_SUCCESS == rc) &&
                (VIDEO_SESSION_DECODE == hypv_session->video_session ||
                 VIDEO_SESSION_ENCODE == hypv_session->video_session))
            {
                if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == fmt_copy->type)
                {
                    hypv_session->num_output_planes = fmt_copy->fmt.pix_mp.num_planes;
                }
                else
                {
                    hypv_session->num_input_planes = fmt_copy->fmt.pix_mp.num_planes;
                }

                HYP_VIDEO_MSG_ERROR("num_output_planes %d num_input_planes %d", hypv_session->num_output_planes, hypv_session->num_input_planes);
            }

        }
     }

    return rc;
}

/**===========================================================================

FUNCTION codecV4l2ToVidc

@brief  Convert codec format type from v4l2 to hypervisor format type

@param [in] v4l2Codec

@dependencies
  None

@return
  Returns vidc_codec_type

===========================================================================*/
vidc_codec_type codecV4l2ToVidc(uint32 v4l2Codec)
{
    vidc_codec_type codec = VIDC_CODEC_UNUSED;

    switch (v4l2Codec)
    {
    case V4L2_PIX_FMT_H264:
        codec = VIDC_CODEC_H264;
        break;
    case V4L2_PIX_FMT_MPEG4:
        codec = VIDC_CODEC_MPEG4;
        break;
    case V4L2_PIX_FMT_MPEG2:
        codec = VIDC_CODEC_MPEG2;
        break;
    case V4L2_PIX_FMT_H263:
        codec = VIDC_CODEC_H263;
        break;
#ifdef V4L2_PIX_FMT_DIVX_311
    case V4L2_PIX_FMT_DIVX_311:
        codec = VIDC_CODEC_DIVX_311;
        break;
#endif
#ifdef V4L2_PIX_FMT_DIVX
    case V4L2_PIX_FMT_DIVX:
        codec = VIDC_CODEC_DIVX;
        break;
#endif
    case V4L2_PIX_FMT_HEVC:
        codec = VIDC_CODEC_HEVC;
        break;
    case V4L2_PIX_FMT_VC1_ANNEX_G:
    case V4L2_PIX_FMT_VC1_ANNEX_L:
        // to verify if VIDC_CODEC_VC1 can handle both ANNEX G AND L
        codec = VIDC_CODEC_VC1;
        break;
    case V4L2_PIX_FMT_VP8:
        codec = VIDC_CODEC_VP8;
        break;
    case V4L2_PIX_FMT_VP9:
        codec = VIDC_CODEC_VP9;
        break;
    default:
        codec = VIDC_CODEC_UNUSED;
    }

    return codec;
}

/**===========================================================================

FUNCTION mapV4l2colorfmt

@brief  Map v4l2 color format to internal color format

@param [in] v4l2colorfmt

@dependencies
  None

@return
  Returns enum color_fmts

===========================================================================*/
enum color_fmts mapV4l2colorfmt(uint32 v4l2colorfmt)
{
    enum color_fmts color_format = COLOR_FMT_NV12;

    switch(v4l2colorfmt)
    {
    case V4L2_PIX_FMT_NV12:
        color_format = COLOR_FMT_NV12;
        break;
    case V4L2_PIX_FMT_NV12_128:
        color_format = COLOR_FMT_NV12_128;
        break;
    case V4L2_PIX_FMT_NV12_512:
        color_format = COLOR_FMT_NV12_512;
        break;
    case V4L2_PIX_FMT_NV12_UBWC:
        color_format = COLOR_FMT_NV12_UBWC;
        break;
    case V4L2_PIX_FMT_NV21:
        color_format = COLOR_FMT_NV21;
        break;
    case V4L2_PIX_FMT_RGB32:
        color_format = COLOR_FMT_RGBA8888;
        break;
    case V4L2_PIX_FMT_RGBA8888_UBWC:
        color_format = COLOR_FMT_RGBA8888_UBWC;
        break;
    case V4L2_PIX_FMT_NV12_TP10_UBWC:
        color_format = COLOR_FMT_NV12_BPP10_UBWC;
        break;
    case V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS:
        color_format = COLOR_FMT_P010;
        break;
    default:
        color_format = COLOR_FMT_NV12;
    }

    return color_format;
}

/**===========================================================================

FUNCTION dec_enc_set_buf_alloc_mode

@brief  Set buffer allocation mode for enc/dec buffers

@param [in] fe_ioss
@param [in] buf_mode
@param [in] buf_type

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_set_buf_alloc_mode(fe_io_session_t* fe_ioss, vidc_buffer_mode_type buf_mode,
                                            vidc_buffer_type buf_type)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_buffer_alloc_mode_type vidc_buffer_mode;

    vidc_buffer_mode.buf_type = buf_type;

    // The BE driver and firmware only support dynamic mode in all cases
    // do not call the static mode to the firmware and driver
    // but return ok so as not to disrupt the client logic to handle static mode buffer
    vidc_buffer_mode.buf_mode = buf_mode;
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_BUFFER_ALLOC_MODE,
                                  sizeof(vidc_buffer_alloc_mode_type),
                                  ( uint8* )&vidc_buffer_mode);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set buf alloc mode property. buf mode %d buf_type %d",
                             buf_mode, buf_type);
    }

    return rc;
}


/**===========================================================================

FUNCTION dec_enc_allocate_eos_buffer

@brief  Allocate EOS buffers for decoder and encode

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
#ifdef SUPPORT_DMABUF
static hypv_status_type dec_enc_allocate_eos_buffer( fe_io_session_t *fe_ioss )
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hyp_internal_buffer* eos_ion_buf = &fe_ioss->eos_buffer;

    uint32 buffer_size = 0;
    std::string dmaBufHeapName;
    BufferAllocator *bufferAllocator = NULL;

    if (!fe_ioss->vidc_input_buffer_size)
    {
        HYP_VIDEO_MSG_ERROR("input buffer size of 0, unexpected.");
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
       buffer_size = fe_ioss->vidc_input_buffer_size;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
       bufferAllocator = CreateDmabufHeapBufferAllocator();
       if (!bufferAllocator)
       {
           HYP_VIDEO_MSG_ERROR("Failed to create DMABufHeap Buffer Allocator object");
           rc = HYPV_STATUS_FAIL;
       }
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
       if (fe_ioss->io_handle->secure)
       {
           if (VIDC_SESSION_DECODE == fe_ioss->session_codec.session)
           {
               dmaBufHeapName = "system-secure";
           }
           else if (VIDC_SESSION_ENCODE == fe_ioss->session_codec.session)
           {
               dmaBufHeapName = "qcom,secure-pixel";
           }
       }
       else
       {
           dmaBufHeapName = "qcom,system";
       }

       char heapName[32];
       snprintf(heapName, sizeof(heapName), "%s", dmaBufHeapName.c_str());

       eos_ion_buf->data_fd = DmabufHeapAlloc(bufferAllocator, heapName, buffer_size, 0, 0);
       FreeDmabufHeapBufferAllocator(bufferAllocator);

       if (0 > eos_ion_buf->data_fd)
       {
           HYP_VIDEO_MSG_ERROR("Memory Alloc failed");
           eos_ion_buf->data_fd = -1;
           eos_ion_buf->dev_fd = -1;
           rc = HYPV_STATUS_FAIL;
       }
       else
       {
           HYP_VIDEO_MSG_INFO("DmabufHeap Buffer Allocation done successfully, buffer_size: %u, heapName: %s, data_fd: %d", buffer_size, heapName, eos_ion_buf->data_fd);
       }
    }

    return rc;
}
#else
static hypv_status_type dec_enc_allocate_eos_buffer( fe_io_session_t *fe_ioss )
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    hyp_internal_buffer* eos_ion_buf = &fe_ioss->eos_buffer;

    if ( !fe_ioss->vidc_input_buffer_size )
    {
        HYP_VIDEO_MSG_ERROR("input buffer size of 0, unexpected.");
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
       eos_ion_buf->dev_fd = ion_open();

       if ( 0 > eos_ion_buf->dev_fd )
       {
           HYP_VIDEO_MSG_ERROR("opening ion device failed with ion_fd = %d",
                                eos_ion_buf->dev_fd );
           rc = HYPV_STATUS_FAIL;
       }
    }

    if ( HYPV_STATUS_SUCCESS == rc )
    {
       int ret = 0;
       struct ion_allocation_data alloc_data;

       HABMM_MEMSET( &alloc_data, 0, sizeof(ion_allocation_data) );

       alloc_data.len = fe_ioss->vidc_input_buffer_size;
       if (fe_ioss->io_handle->secure)
       {
           if (VIDC_SESSION_DECODE == fe_ioss->session_codec.session)
           {
               alloc_data.flags = ION_FLAG_SECURE | ION_FLAG_CP_BITSTREAM;
           }
           else if (VIDC_SESSION_ENCODE == fe_ioss->session_codec.session)
           {
               alloc_data.flags = ION_FLAG_SECURE | ION_FLAG_CP_PIXEL;
           }
           alloc_data.heap_id_mask = ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID);
       }
       else
       {
           alloc_data.flags = 0;
           alloc_data.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
       }

       ret = ion_alloc_fd(eos_ion_buf->dev_fd, alloc_data.len, 0,
                         alloc_data.heap_id_mask, alloc_data.flags,
                         &eos_ion_buf->data_fd);

       if ( ret || 0 > eos_ion_buf->data_fd )
       {
           HYP_VIDEO_MSG_ERROR("ION ALLOC memory failed");
           ion_close(eos_ion_buf->dev_fd);
           eos_ion_buf->data_fd = -1;
           eos_ion_buf->dev_fd = -1;
           rc = HYPV_STATUS_FAIL;
       }
       else
       {
          HYP_VIDEO_MSG_INFO("Alloc ion memory: fd (dev:%d data:%d) len %d flags %#x mask %#x",
                              eos_ion_buf->dev_fd, eos_ion_buf->data_fd, (unsigned int)alloc_data.len,
                              (unsigned int)alloc_data.flags, (unsigned int)alloc_data.heap_id_mask);
       }
    }

    return rc;
}
#endif

/**===========================================================================

FUNCTION dec_enc_free_eos_buffer

@brief  Free decoder and encode EOS buffers

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_free_eos_buffer( fe_io_session_t *fe_ioss )
{
    hyp_internal_buffer* eos_ion_buf = &fe_ioss->eos_buffer ;

    if ( 0 <= eos_ion_buf->data_fd )
    {
       close(eos_ion_buf->data_fd);
       eos_ion_buf->data_fd = -1;
    }

    if ( 0 <= eos_ion_buf->dev_fd )
    {
       ion_close(eos_ion_buf->dev_fd);
       eos_ion_buf->dev_fd = -1;
    }

    return HYPV_STATUS_SUCCESS;
}

static int submit_flush_event(void *pIoSession)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_io_session_t* fe_ioss = (fe_io_session_t*)pIoSession;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;
    struct v4l2_event event;
    hypv_session_t* hypv_session = fe_ioss->io_handle;
    uint32 flags = 0;

    HABMM_MEMSET(&event, 0, sizeof(struct v4l2_event));

    //Adding sleep of 20ms to handle any race conditions
    usleep(1000 * 20);

    HYP_VIDEO_MSG_HIGH("Sending V4L2_EVENT_MSM_VIDC_FLUSH_DONE flush_event thread,"
            " type 0x%x", hypv_session->flush_req);

    event.type = V4L2_EVENT_MSM_VIDC_FLUSH_DONE;
    *(uint32_t *)event.u.data = hypv_session->flush_req;
    hyp_enqueue(&plt_data->evt_queue, (void *)&event);
    flags = POLLPRI;
    fe_ioss->fe_cb.handler(fe_ioss->fe_cb.context, &flags);

    MM_CriticalSection_Enter(hypv_session->flush_lock);
    hypv_session->flush_req = 0;
    hypv_session->flush_clr = 0;
    MM_CriticalSection_Leave(hypv_session->flush_lock);

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_cmd

@brief  Decoder and encode FE set commands

@param [in] fe_ioss pointer
@param [in] v4l2_encoder_cmd pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_cmd(fe_io_session_t* fe_ioss, struct v4l2_decoder_cmd* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_INFO("cmd 0x%x flags 0x%x", data->cmd, data->flags);

    switch (data->cmd)
    {
    case V4L2_DEC_CMD_STOP: /* same for V4L2_ENC_CMD_STOP */
        {
            struct v4l2_buffer buf;
            struct v4l2_plane plane[NUM_OUTPUT_BUFFER_PLANE_ENC];

            HABMM_MEMSET(&buf, 0, sizeof(v4l2_buffer));
            HABMM_MEMSET(&plane, 0, NUM_OUTPUT_BUFFER_PLANE_ENC * sizeof(v4l2_plane));

            rc = dec_enc_allocate_eos_buffer(fe_ioss);
            if (HYPV_STATUS_SUCCESS != rc)
            {
                HYP_VIDEO_MSG_ERROR("EOS buffer allocation failed" );
            }
            else
            {
                HYP_VIDEO_MSG_INFO("EOS buffer allocation fd %d",
                                    fe_ioss->eos_buffer.data_fd);
                /* extradata(plane[1]) is not required in eos buffer and hence
                   explicitly set to 0 in the above memset to avoid garbage
                   addr getting mapped */
                if (V4L2_MEMORY_DMABUF == buf.memory)
                {
                    plane[0].m.fd = fe_ioss->eos_buffer.data_fd;
                }
                else
                {
                    plane[0].reserved[MSM_VIDC_BUFFER_FD] = fe_ioss->eos_buffer.data_fd;
                }
                plane[0].length = fe_ioss->vidc_input_buffer_size;
                buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.m.planes = plane;
                buf.length = 1;
                buf.flags = V4L2_BUF_FLAG_EOS;
                rc = dec_enc_qbuf(fe_ioss, &buf);
            }
            break;
        }
    case V4L2_CMD_FLUSH:
        {
            hypv_session_t* hypv_session = fe_ioss->io_handle;
            vidc_flush_mode_type flush_mode = VIDC_FLUSH_UNUSED;

            /* flush_req stores the V4L2_CMD_FLUSH mode bits requested by client;
             * flush_clr is used when handling the flush done events, where the
             * requested mode bits are cleared by the correspoding flush done events */
            MM_CriticalSection_Enter(hypv_session->flush_lock);
            hypv_session->flush_req = data->flags;
            hypv_session->flush_clr = data->flags;
            MM_CriticalSection_Leave(hypv_session->flush_lock);

            if (data->flags & V4L2_CMD_FLUSH_OUTPUT &&
                data->flags & V4L2_CMD_FLUSH_CAPTURE)
            {
                flush_mode = VIDC_FLUSH_ALL;
            }
            else if (data->flags & V4L2_CMD_FLUSH_OUTPUT)
            {
                if (fe_ioss->input_buffer_count == 0)
                {
                    HYP_VIDEO_MSG_INFO("vidc input buffer size is 0");
                }
                else
                {
                    flush_mode = VIDC_FLUSH_INPUT;
                }
            }
            else if (data->flags & V4L2_CMD_FLUSH_CAPTURE)
            {
                flush_mode = VIDC_FLUSH_OUTPUT;
            }

            HYP_VIDEO_MSG_INFO("cmd flush with mode %d: ftb_count %u etb_count %u",
                    flush_mode, hypv_session->ftb_count, hypv_session->etb_count);
            if (VIDC_FLUSH_UNUSED != flush_mode)
            {
                if ((0 < hypv_session->etb_count) || (0 < hypv_session->ftb_count))
                {
                    rc = hyp_device_ioctl(fe_ioss->io_handle, VIDC_IOCTL_FLUSH,
                                          (uint8 *)&flush_mode, sizeof(vidc_flush_mode_type),
                                           NULL, 0);
                    if (HYPV_STATUS_SUCCESS != rc)
                    {
                        HYP_VIDEO_MSG_ERROR("vidc cmd flush failed. flag = 0x%x rc = %d",
                                             data->flags, rc);
                    }
                }
                else
                {
                    HYP_VIDEO_MSG_HIGH("etb_count %u ftb_count %u", hypv_session->etb_count, hypv_session->ftb_count);
                    int32 ret = 0;
                    MM_HANDLE thread_handle = NULL;

                    ret = MM_Thread_Create(MM_Thread_DefaultPriority, 0, submit_flush_event, (void *)fe_ioss, 0, &thread_handle);
                    if (0 != ret)
                    {
                        HYP_VIDEO_MSG_ERROR("Failed to create flush_event thread");
                    }
                    else
                    {
                        HYP_VIDEO_MSG_HIGH("Created flush_event thread");
                        MM_Thread_Detach(thread_handle);
                    }
                }
            }
            else
            {
                HYP_VIDEO_MSG_ERROR("bad flush flag 0x%x", data->flags);
            }
        break;
        }
    default:
        HYP_VIDEO_MSG_ERROR("cmd 0x%x not supported", data->cmd);
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_create_dyn_buf

@brief  Create dynamic buffer entry for encoder/decoder

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type dec_enc_create_dyn_buf(fe_io_session_t *fe_ioss)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;

    HYP_VIDEO_MSG_INFO("allocate dynmaic buf entry. count = %u",
                        fe_ioss->dynmaic_buf_entry_count);

    fe_ioss->out_dynamic_info = (dynamic_buf_info *) HABMM_MALLOC(sizeof(dynamic_buf_info) *
                                                     fe_ioss->dynmaic_buf_entry_count);
    if (fe_ioss->out_dynamic_info)
    {
        for (uint32 i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
        {
            fe_ioss->out_dynamic_info[i].fd = -1;
            fe_ioss->out_dynamic_info[i].dup_fd = -1;
            fe_ioss->out_dynamic_info[i].ref_count = 0;
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("malloc dynamic buf entry failed %u",
                             fe_ioss->dynmaic_buf_entry_count );
        rc = HYPV_STATUS_FAIL;
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_destroy_dyn_buf

@brief  Destroy dynamic buffer entry for encoder/decoder

@param [in] fe_ioss pointer

@dependencies
  None

@return
  Returns void

===========================================================================*/
static void dec_enc_destroy_dyn_buf(fe_io_session_t *fe_ioss)
{
    if (fe_ioss->out_dynamic_info)
    {
        for (uint32 i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
        {
            if (-1 != fe_ioss->out_dynamic_info[i].dup_fd)
            {
                close(fe_ioss->out_dynamic_info[i].dup_fd);
            }
        }
        HYP_VIDEO_MSG_INFO("Free out dynamic info");
        HABMM_FREE( fe_ioss->out_dynamic_info );
        fe_ioss->out_dynamic_info = NULL;
    }

    return;
}

/**===========================================================================

FUNCTION dec_enc_add_dyn_buf_ref

@brief  Add ref to encoder/decoder buffer

@param [in] fe_ioss pointer
@param [in] buf pointer
@param [out] frame_data pointer
@param [out] index pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type dec_enc_add_dyn_buf_ref(fe_io_session_t *fe_ioss, struct v4l2_buffer *buf,
                                                vidc_frame_data_type *frame_data, uint32 *index)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 i = 0;
    bool found = false;
    int32 buf_fd = -1, meta_fd = -1;

    if (V4L2_MEMORY_DMABUF == buf->memory)
    {
        buf_fd = buf->m.planes[0].m.fd;
        meta_fd = buf->m.planes[1].m.fd;
    }
    else
    {
        buf_fd = buf->m.planes[0].reserved[MSM_VIDC_BUFFER_FD];
        meta_fd = buf->m.planes[1].reserved[MSM_VIDC_BUFFER_FD];
    }

    for (i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
    {
        if (fe_ioss->out_dynamic_info[i].fd == (int32)buf_fd)
        {
            found = true;
            fe_ioss->out_dynamic_info[i].ref_count++;
            HYP_VIDEO_MSG_INFO("buffer idx %u addr %p ref_count = %d metadata_fd %d metadata_alloc_len %u",
                               i, frame_data->frame_addr,
                               fe_ioss->out_dynamic_info[i].ref_count,
                               meta_fd,
                               buf->m.planes[1].length);
            fe_ioss->out_dynamic_info[i].metadata_fd = meta_fd;
            fe_ioss->out_dynamic_info[i].metadata_alloc_len = buf->m.planes[1].length;
            fe_ioss->out_dynamic_info[i].index = buf->index;
            *index = i;
            break;
        }
    }

    if (false == found)
    {
        bool check_entry = false;

        for (i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
        {
            if (-1 == fe_ioss->out_dynamic_info[i].fd)
            {
                check_entry = true;
                fe_ioss->out_dynamic_info[i].dup_fd = dup(buf_fd);
                fe_ioss->out_dynamic_info[i].fd = buf_fd;
                fe_ioss->out_dynamic_info[i].ref_count = 1;
                fe_ioss->out_dynamic_info[i].alloc_len = buf->m.planes[0].length;
                fe_ioss->out_dynamic_info[i].metadata_fd = meta_fd;
                fe_ioss->out_dynamic_info[i].metadata_alloc_len = buf->m.planes[1].length;
                fe_ioss->out_dynamic_info[i].index = buf->index;
                HYP_VIDEO_MSG_INFO("buffer idx %u fd %d ref_count %d metadata_fd %d metadata_alloc_len %u",
                                    i, buf_fd,
                                    fe_ioss->out_dynamic_info[i].ref_count,
                                    meta_fd,
                                    buf->m.planes[1].length);
                *index = i;
                break;
            }
        }
        if (false == check_entry)
        {
            HYP_VIDEO_MSG_ERROR("dynamic buffer entry full. buffer count = %u",
                                 fe_ioss->dynmaic_buf_entry_count );
            rc = HYPV_STATUS_FAIL;
        }
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_rm_dyn_buf_ref

@brief  Remove ref to encoder/decoder buffer

@param [in] fe_ioss pointer
@param [in] frame_data pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_rm_dyn_buf_ref(fe_io_session_t *fe_ioss,
                                        vidc_frame_data_type* frame_data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    uint32 i = 0;

    if (fe_ioss->out_dynamic_info)
    {
        for (i = 0; i < fe_ioss->dynmaic_buf_entry_count; i++)
        {
            if ((int32)(uintptr_t)frame_data->frame_addr == fe_ioss->out_dynamic_info[i].fd)
            {
                fe_ioss->out_dynamic_info[i].ref_count--;

                HYP_VIDEO_MSG_INFO("flag idx %u fd %p ref_count = %d",
                                    i, frame_data->frame_addr,
                                    fe_ioss->out_dynamic_info[i].ref_count);

                if (0 == fe_ioss->out_dynamic_info[i].ref_count)
                {
                    close(fe_ioss->out_dynamic_info[i].dup_fd);
                    fe_ioss->out_dynamic_info[i].fd = -1;
                    fe_ioss->out_dynamic_info[i].dup_fd = -1;
                    fe_ioss->out_dynamic_info[i].metadata_fd = -1;
                    fe_ioss->out_dynamic_info[i].index = 0;
                    HYP_VIDEO_MSG_INFO("ref count reached 0 for fd %d. close dup fd %d",
                                        fe_ioss->out_dynamic_info[i].fd,
                                        fe_ioss->out_dynamic_info[i].dup_fd);
                }
                break;
            }
        }

        if (fe_ioss->dynmaic_buf_entry_count == i)
        {
            HYP_VIDEO_MSG_ERROR("buffer %p not found", frame_data->frame_addr );
            rc = HYPV_STATUS_FAIL;
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("out_dynamic_info failed");
        rc = HYPV_STATUS_FAIL;
    }
    return rc;
}

/**===========================================================================

FUNCTION dec_enc_dqevent

@brief  Decode encode dequeue events

@param [in] fe_ioss pointer
@param [in] v4l2_event pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_dqevent(fe_io_session_t* fe_ioss, struct v4l2_event* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;

    rc = hyp_dequeue(&plt_data->evt_queue, (void *)data);
    HYP_VIDEO_MSG_INFO("Dequeue event type = 0x%x", data->type);

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_dqbuf

@brief  Decode encode dequeue buffer

@param [in] fe_ioss pointer
@param [in] v4l2_buffer pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_dqbuf(fe_io_session_t* fe_ioss, struct v4l2_buffer* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_linux_plt_data_t *plt_data = (fe_linux_plt_data_t *)fe_ioss->fe_plt_data;
    uint32 num_planes = data->length;
    event_buf_type event_buf;

    HYP_VIDEO_MSG_INFO("Dequeue buffer type %u", data->type);

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == data->type)
    {
        rc = hyp_dequeue(&plt_data->evt_output_buf_queue, (void *)&event_buf);
    }
    else if(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == data->type)
    {
        rc = hyp_dequeue(&plt_data->evt_input_buf_queue, (void *)&event_buf);
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("Bad buffer type = %u", data->type);
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        struct v4l2_plane *planes = data->m.planes;

        memcpy((void *)data, (void *)&event_buf.buf, sizeof(struct v4l2_buffer));
        memcpy((void *)planes, (void *)&event_buf.buf_planes[0],
               num_planes * sizeof(struct v4l2_plane));
        data->m.planes = planes;
        HYP_VIDEO_MSG_INFO("Dequeue: buf_type = %u flag = 0x%x tv_sec = %ld"
                           " tv_usec = %ld index = %u frame_addr = 0x%lx",
                            data->type, data->flags, data->timestamp.tv_sec,
                            data->timestamp.tv_usec, data->index,
                            data->m.planes[0].m.userptr);
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_s_secure_video

@brief  Decoder/Encoder FE set secure video

@param [in] fe_ioss pointer
@param [in] enable boolean

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_s_secure_video(fe_io_session_t *fe_ioss, boolean enable)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type secure = {enable};

    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_CONTENT_PROTECTION,
                                  sizeof(vidc_enable_type),
                                  ( uint8* )&secure);
    if (HYPV_STATUS_SUCCESS != rc)
    {
       HYP_VIDEO_MSG_ERROR("failed to set drv secure video. enable %d", secure.enable);
    }
    else
    {
       HYP_VIDEO_MSG_HIGH("set drv secure video. enable %d", secure.enable);
    }

    return rc;
}

/**===========================================================================

FUNCTION dec_enc_s_low_latency_mode

@brief  Decoder/Encoder FE set low latency mode

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_s_low_latency_mode(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type low_latency;

    if (data->value)
    {
        low_latency.enable = TRUE;
    }
    else
    {
        low_latency.enable = FALSE;
    }
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_LOW_LATENCY_MODE,
                                  sizeof(vidc_enable_type),
                                  ( uint8* )&low_latency);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set drv low latency. enable %d",
                             low_latency.enable);
    }

    return rc;
}

const v4l2fe_vidc_format *dec_enc_get_pixel_fmt_index(const v4l2fe_vidc_format fmt[],
      int size, int index, int fmt_type)
{
    int fmt_index = 0;
    int i = 0;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    const v4l2fe_vidc_format *format = NULL;

    if (!fmt || index < 0)
    {
        HYP_VIDEO_MSG_ERROR( "Invalid inputs, fmt = %pK, index = %d",
                        fmt, index);
        rc = HYPV_STATUS_FAIL;
    }

    if (HYPV_STATUS_SUCCESS == rc)
    {
        for (i = 0; i < size; i++)
        {
            if (fmt_type == fmt[i].type)
            {
                if (index == fmt_index)
                {
                    HYP_VIDEO_MSG_HIGH("Format: %s index: %d fmt_index: %d i: %d", fmt[i].name, index, fmt_index, i);
                    format = &fmt[i];
                    break;
                }
                else
                {
                    fmt_index++;
                }
            }
        }
        if (i == size)
        {
            HYP_VIDEO_MSG_INFO("Format not found");
            format = NULL;
        }
    }

    return format;
}
/**===========================================================================

FUNCTION dec_enc_s_thumbnail_mode

@brief  Decoder FE set thumbnail mode

@param [in] fe_ioss pointer
@param [in] v4l2_control pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type dec_enc_s_thumbnail_mode(fe_io_session_t *fe_ioss, struct v4l2_control* data)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    vidc_enable_type thumbnail_mode = {FALSE};

    if (data->value)
    {
        thumbnail_mode.enable = TRUE;
        fe_ioss->enable_thumbnail_mode = TRUE;
    }
    else
    {
        thumbnail_mode.enable = FALSE;
        fe_ioss->enable_thumbnail_mode = FALSE;
    }
    rc = dec_enc_set_drv_property(fe_ioss,
                                  VIDC_I_DEC_THUMBNAIL_MODE,
                                  sizeof(vidc_enable_type),
                                  ( uint8* )&thumbnail_mode);
    if (HYPV_STATUS_SUCCESS != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to set drv thumbnail mode. enable %d",
                             thumbnail_mode.enable);
    }

    return rc;
}

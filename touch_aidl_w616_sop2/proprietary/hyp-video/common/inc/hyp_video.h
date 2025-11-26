/*========================================================================

*//** @file hyp_video.h

@par FILE SERVICES:
      Hypervisor video common interface definitions. The file defines Hypervisor
      video communication messages and the data structures.

@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) 2016-2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */
/*===========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/1.0/common/inc/hyp_video.h#10 $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
06/04/22           rq          support 8255 metadata backward compatible with Codec2.1.0
01/14/22           nb          Add new hypervisor status type HYPV_STATUS_ERR_MAX_CLIENT
06/16/21           sh          Include input_tag2 as its needed for dropping inputs in codec2
05/27/21           sh          Guard passthrough structure with Linux specific macro
04/01/21           sh          Bringup video on RedBend Hypervisor
08/27/20           sh          Bringup video decode using codec2
02/05/19           rz          Change to use C++
10/30/18           sm          Update version for multistream support
09/14/18           sh          Extract buffer address when index is passed during EOS
09/07/18           aw          Update retry mechanism to use infinite retry
07/10/18           sm          Add support to use pmem handle for video buffers
05/30/18           sm          Add bmetrics support for KPI
04/25/18           sm          Updated structure for dither and pid
03/08/18           sm          Add HAB API retry mechanism
01/26/18           sm          Increase ioctl message size to handle bigger size data
01/18/18           sm          Add support for passthrough mode feature
07/25/17           sm          Fix issues with dynamic allocation mode
06/30/17           aw          Add support for version validation between FE and BE
05/08/17           sm          Update for new hyp-video architecture
04/03/17           sm          Add support for Video input FE-BE
01/30/17           sm          Update BE-FE message passing
09/29/16           hl          Make the final msg data size less than 512 bytes
07/26/16           rz          Add pid for habmm emulation in metal
06/29/16           hl          Add dynamic buffer mode
06/09/16           hl          Add support linux hypervisor for non-contiguous metadata
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/
#ifndef __HYP_VIDEO_H__
#define __HYP_VIDEO_H__

extern "C" {

#ifdef __QNXNTO__
#include <AEEStdDef.h>
#endif
#include "hyp_vidc_types.h"
#if defined(_ANDROID_) || defined(_LINUX_)
#include <linux/videodev2.h>
#endif

#define HYP_VIDEO_MAJOR_REV  1
#define HYP_VIDEO_MINOR_REV  5
#define MINOR_REV_BITS       16
#define MAJOR_REV_MASK       ((1U << MINOR_REV_BITS) - 1)
#define HYP_VIDEO_VERSION ((HYP_VIDEO_MAJOR_REV << MINOR_REV_BITS) | HYP_VIDEO_MINOR_REV)
#define GET_MAJOR_REV(ver) (ver >> MINOR_REV_BITS)
#define GET_MINOR_REV(ver) (ver & MAJOR_REV_MASK)

#define VIDEO_HYP_BE  "hyp_video_be"
#define VIDEO_HYP_FE  "hyp_video_fe"

#define MAX_DEVICE_NAME_LEN 128
#define VDEC_DEVICE "videodec"
#define VENC_DEVICE "videoenc"
#define VINPUT_DEVICE "avinput"

#define HABMM_MALLOC                 malloc
#define HABMM_FREE( mem )            {if((mem)) free(mem);(mem) = NULL;}
#define HABMM_MEMSET(src,value,len)  {if(len > 0) memset(src, value, len);}
#define HABMM_MEMCPY(dest,src,len)   {if(len > 0) memmove(dest, src, len);}

#define MAX(a,b)  ((a) > (b)? (a) : (b))
#define MIN(a,b)  ((a) < (b)? (a) : (b))

/* Hypervisor status type */
typedef enum
{
   HYPV_STATUS_FAIL = -1,
   HYPV_STATUS_SUCCESS = 0x0,
   HYPV_STATUS_PENDING,
   HYPV_STATUS_BAD_PARAMETER,
   HYPV_STATUS_ALLOC_FAIL,
   HYPV_STATUS_INVALID_STATE,
   HYPV_STATUS_VERSION_MISMATCH,
   HYPV_STATUS_ERR_MAX_CLIENT,
   HYPV_STATUS_MAX = 0x7fffffff
} hypv_status_type;

#if defined(_ANDROID_) || defined(_LINUX_)
struct v4l2_format_64b
{
    uint64 type;
    union
    {
        struct v4l2_pix_format pix;
        struct v4l2_pix_format_mplane pix_mp;
        struct v4l2_window win;
        struct v4l2_vbi_format vbi;
        struct v4l2_sliced_vbi_format sliced;
        struct v4l2_sdr_format sdr;
        struct v4l2_meta_format meta;
        uint8 raw_data[200];
    } fmt;
};

struct v4l2_event_64b
{
    uint64 type;
    union
    {
        struct v4l2_event_vsync vsync;
        struct v4l2_event_ctrl ctrl;
        struct v4l2_event_frame_sync frame_sync;
        struct v4l2_event_src_change src_change;
        struct v4l2_event_motion_det motion_det;
        uint8 data[64];
    } u;
    uint64 pending;
    uint64 sequence;
    int64  tv_sec;  //struct timespec timestamp;
    int64  tv_nsec; //struct timespec timestamp;
    uint64 id;
    uint64 reserved[8];
};

struct v4l2_plane_64b
{
    uint32           bytesused;
    uint32           length;
    union
    {
        uint32      mem_offset;
        uint64      userptr;
        int32       fd;
    } m;
    uint32           data_offset;
    uint32           reserved[11];
};

struct v4l2_buffer_64b
{
    uint64          index;
    uint64          type;
    uint64          bytesused;
    uint64          flags;
    uint64          field;
    int64           tv_sec;   //struct timeval  timestamp;
    int64           tv_usec;  //struct timeval  timestamp;
    struct v4l2_timecode    timecode;
    uint64           sequence;
    /* memory location */
    uint64           memory;
    union
    {
        uint32           offset;
        unsigned long   userptr;
        struct v4l2_plane_64b planes[2];
        int32       fd;
    } m;
    uint64           length;
    uint64           reserved2;
    union
    {
        int32       request_fd;
        uint64      reserved;
    };
};

 typedef struct
 {
     hypv_status_type status;   /**< Status of asynchronous
                              *   operation
                              */

     uint32           event_type;  /**< Uniquely identifies type of
                                 *   async message.
                                 */

     union
     {
         struct v4l2_buffer_64b buffer;
         struct v4l2_event_64b  event_data;
     }payload;
 }v4l2_passthrough_event_data_type;
#endif
/*
*
* The _64b_ extension has the same meaning for regular data structure. It is modified
* to get rid of the confusion pointer size and the alignment issue.
*
* If the server is 64bit compiled, the 32bit client needs to use the _64b structure
* to pass information; otherwise the alignment and the size of the items in the
* structure would make them different.
*
* Preferably, the hypervisor layer should use this data structure for the commnunication
* to avoid 32-bit vs 64-bit confusion.
*
*/
typedef struct
{
   /** Virtual address in Kernel space of the frame buffer */
   uint64            frame_addr;

   /** Virtual address in Kernel space for metadata associated with this
       frame buffer */
   uint64            metadata_addr;

   /** metadata buffer id that obtained from habmm export*/
   //uint64            metadata_buffer_id;

   /** Size of the buffer in bytes. [Required field].  */
   uint32            alloc_len;

   /** Buffer content length in bytes. [Required field]. */
   uint32            data_len;

   /** Gives the start of valid data in bytes from the start of buffer.
       A pointer to valid data may be obtained by adding offset to frame_addr.
       The start address that is obtained this way should be aligned to the
       alignment negotiated in the buffer requirements. If not, then the
       submitted buffer would be rejected. */
   uint32            offset;

   /** Timestamp, forwarded from input to output. Expressed in microseconds. */
   vidc_timestamp_type  timestamp;

   /** Additional data associated with the frame. Refer to VIDC_FRAME_FLAG_*
      constants above for bit assignments. */
   uint32            flags;

   /** Client/User data associated with the frame. This is returned as-is when
       frame is returned via INPUT_DONE or OUTPUT_DONE callbacks. */
   uint64            frm_clnt_data;

   /** Uncompressed frame descriptor for decoder. Populated for each decoded
       frame. This info is handy for when I_CONT_RECONFIG is enabled, where
       frame resolution and display rect can change on the fly. For encoder
       use VIDC_METADATA_INDEX_INPUT_CROP to specify the uncompressed frame
       info */
   vidc_uncompressed_frame_config_type   frame_decsp;

   /** Type of the frame */
   vidc_frame_type   frame_type;

   /** Buffer type of this frame */
   vidc_buffer_type     buf_type;

   /** Mark target. The identifier will be propagated to associated output
       buffer(s) */
   uint64            mark_target;

   /** Mark data. The identifier will be propagated to associated output
       buffer(s) */
   uint64            mark_data;

   /** InputTag **/
   uint64            input_tag;

   /** InputTag2 **/
   uint64            input_tag2;

   /** OutputTag **/
   uint64            output_tag;

   /** allocated size of metadata */
   uint32            alloc_metadata_len;

   /** Flag to represent whether the data and metadata buffers are
       contiguous or not */
   boolean           non_contiguous_metadata;

   /** Process id for the frame_addr and metadata_addr */
   uint32            pid;

   /**< PMEM handle of the frame buffer
    */
   uint64            frame_handle;

   /**< PMEM handle for metadata associated with this frame buffer
    */
   uint64            metadata_handle;

} vidc_frame_data_64b_type;



typedef struct
{
   /**< Buffer type. Allowed types are:
    *   VIDC_BUFFER_INPUT, VIDC_BUFFER_OUTPUT, VIDC_BUFFER_OUTPUT2
    */
   vidc_buffer_type        buf_type;

   /**< Flag to represent whether the data and extradata buffers are
    *   contiguous or not
    */
   boolean                 contiguous;

   /**< Size of data buffer in bytes */
   uint32                  buf_size;

   /**< Data Buffer Address */
   uint64                  buf_addr;

   /**< Size of extradata buffer to be allocated in bytes.
    *   This could be 0 if there is no extradata-buffer required
    */
   uint32                  extradata_buf_size;

   /**< Extra data buffer address. This could be NULL if extradata_buf_size = 0
    *   or contiguous is TRUE
    */
   uint64                  extradata_buf_addr;

   /** Process id for the frame_addr and metadata_addr */
   uint32                  pid;

   /**< PMEM handle of the frame buffer
    */
   uint64                  buf_handle;

   /**< PMEM handle for metadata associated with this frame buffer
    */
   uint64                  extradata_buf_handle;

} vidc_buffer_info_64b_type;

/**
* This data type carries the video sequence header for encoder/decoder.
* [property id: VIDC_I_SEQUENCE_HEADER]
*/
typedef struct
{
   /**< pointer to video sequence header */
   uint64  seq_hdr;

   /**< length of video sequence header */
   uint32  seq_hdr_len;

   /**< process id for seq_hdr virtual addr */
   uint32  pid;

} vidc_seq_hdr_64b_type;

typedef union vidc_drv_msg_payload_64b_t
{
   vidc_frame_data_64b_type frame_data;

   /*
    * The event_data_1 is the event information in 32bit word.
    * Currently, it is used to return the LTR id for VIDC_EVT_RESP_LTRUSE_FAILED
    * in encoding a frame because the specified LTR frame does not exist in the
    * LTR list.
    */
   uint32               event_data_1;
}vidc_drv_msg_payload_64b;


/**
 * Response message type
 */
typedef struct vidc_drv_msg_info_64b_type_t
{
   hypv_status_type         status;   /**< Status of asynchronous
                                      *   operation
                                      */

   uint32                  event_type;  /**< Uniquely identifies type of
                                         *   async message.
                                         */
   vidc_drv_msg_payload_64b payload;  /**< Message data */
}vidc_drv_msg_info_64b_type;

/**
 * Response message type in passthrough mode
 */
typedef struct passthrough_event_data
{
   hypv_status_type status;   /**< Status of asynchronous
                              *   operation
                              */

   uint32           event_type;  /**< Uniquely identifies type of
                                 *   async message.
                                 */

   uint8            payload[1];  /**< Message data */
}passthrough_event_data_type;

/**
 * Hypervisor response message
 */

typedef union hypvideo_event_data
{
   vidc_drv_msg_info_64b_type       vidc;
   passthrough_event_data_type      passthru;
#ifdef LINUX_PASSTHROUGH
   // Define LINUX_PASSTHROUGH macro in .mk file based on the SP info.
   // This is needed since size of event_data would change on LA/LV
   // based on the SP.
   v4l2_passthrough_event_data_type v4l2_passthru;
#endif
}hypvideo_event_data_type;

/*
 *---------------------------------------------------------------------------
 * Hypervisor Extension is the addition to support multi OS video operation
 *
 */
typedef enum
{
    HYPVIDEO_EVENT_START    = 0x00001000,
    HYPVIDEO_EVENT_FLUSH_DONE,
    HYPVIDEO_EVENT_PORT_SETTINGS_CHANGED_SUFFICIENT,
    HYPVIDEO_EVENT_PORT_SETTINGS_CHANGED_INSUFFICIENT,
    HYPVIDEO_EVENT_PORT_SETTINGS_BITDEPTH_CHANGED_INSUFFICIENT,
    HYPVIDEO_EVENT_RELEASE_BUFFER_REFERENCE,
    HYPVIDEO_EVENT_RELEASE_UNQUEUED_BUFFER,
    HYPVIDEO_EVENT_CLOSE_DONE,
    HYPVIDEO_EVENT_SYS_ERROR,
    HYPVIDEO_EVENT_HW_OVERLOAD,
    HYPVIDEO_EVENT_HW_UNSUPPORTED,
    HYPVIDEO_EVENT_ALL     = 0x00001fff
} vidc_ext_subscription_type;

typedef enum{
    /* VIDC_EXT_DPB_COLOR_FMT_NONE: dpb and output has the same format */
    VIDC_EXT_DPB_COLOR_FMT_NONE = 0,

    /* VIDC_EXT_DPB_COLOR_FMT_UBWC: dpb is UBWC while output is different */
    VIDC_EXT_DPB_COLOR_FMT_UBWC,

    /* VIDC_EXT_DPB_COLOR_FMT_TP10_UBWC: dpb is 10-bit UBWC while output is different */
    VIDC_EXT_DPB_COLOR_FMT_TP10_UBWC,
}vidc_ext_dpb_color_format_type;

typedef struct{
    /*
     * is_split: true if dpb and output is made different
     *           false if dpb and output is the same
     */
    boolean                         is_split;

    vidc_ext_dpb_color_format_type  dpb_color_format;
}vidc_ext_set_dpb_type;

#define VIDC_HYPERVISOR_IOCTL_BASE      0x00008000
 /*
  * Purpose: To start streaming the queued buffer. It may be called before
  *          the first empty buffer to force the START state even the
  *          IOCTL_CMD_START has not called before.
  *          HYPVIDEO_EVT_RESP_START should have been received before
  *          the call return.
  * API type: Synchronous
  * Prerequisites: None
  * Associated Structure: vidc_buffer_type
  * IOCTL params:
  *      InputData:     vidc_buffer_type
  *      OutputData:    NULL
  *
  */
#define VIDC_HYPERVISOR_IOCTL_STREAMON (VIDC_HYPERVISOR_IOCTL_BASE + 1)

/*
 * Purpose: To stop streaming the queued buffer. It may be called before
 *          IOCTL_CMD_FLUSH to force the STOP state even the IOCTL_CMD_STOP
 *          has not been called before.
 * API type: Synchronous
 * Prerequisites: None
 * Associated Structure: vidc_buffer_type
 * IOCTL params:
 *      InputData:     vidc_buffer_type
 *      OutputData:    NULL
 *
 */
#define VIDC_HYPERVISOR_IOCTL_STREAMOFF (VIDC_HYPERVISOR_IOCTL_BASE + 2)

/*
 * Purpose: To subscribe the callback event from server
 * API type: Synchronous
 * Prerequisites: None
 * Associated Structure: vidc_ext_subscription_type
 * IOCTL params:
 *      InputData:     vidc_ext_subscription_type
 *      OutputData:    NULL
 *
 */
#define VIDC_HYPERVISOR_IOCTL_SUBSCRIBE_EVENT (VIDC_HYPERVISOR_IOCTL_BASE + 3)

 /*
 * Purpose: To unsubscribe the callback event from server
 * API type: Synchronous
 * Prerequisites: None
 * Associated Structure: vidc_ext_subscription_type
 * IOCTL params:
 *      InputData:     vidc_ext_subscription_type
 *      OutputData:    NULL
 *
 */
#define VIDC_HYPERVISOR_IOCTL_UNSUBSCRIBE_EVENT (VIDC_HYPERVISOR_IOCTL_BASE + 4)


/*--------------------------------------------------------------------------------------------
 *   Hypervisor Extension to VIDC property
 *
 *   Property id       : VIDC_EXT_PROPERTY_DEC_SET_DPB
 *   Property Structure: vidc_ext_set_dpb_type
 *   Description       : It can set dpb different from output buffers for performance purpose.
 *                       For example, if output buffers have to be linear NV12, the user can
 *                       force the dpb to ubwc (for the 4k decode performance)
 *   SetProperty for   : Decoder
 *   Reconfigurable    : Only before LOAD_RESOURCES; should be set before get buffer format
 *                       and requirement
 *   Initial Default value :
 *
 */
#define VIDC_HYPERVISOR_PROPERTY_BASE          0x80000000
#define VIDC_HYPERVISOR_PROPERTY_DEC_SET_DPB   (VIDC_HYPERVISOR_PROPERTY_BASE+1)


#define MAX_IOCTL_DATA_PAYLOAD_SIZE    (2048)
typedef struct hypvideo_ioctl_data
{
    hypv_status_type               return_value;
    int64                          io_handle;
    uint32                         vidc_ioctl;
    uint8                          payload[MAX_IOCTL_DATA_PAYLOAD_SIZE];
}hypvideo_ioctl_data_type;

typedef struct hypvideo_close_data
{
    hypv_status_type           return_status;
    int64                      io_handle;
}hypvideo_close_data_type;

typedef enum video_session
{
    VIDEO_SESSION_UNKNOWN = 0,
    VIDEO_SESSION_DECODE  = 1,
    VIDEO_SESSION_ENCODE  = 2,
    VIDEO_SESSION_VINPUT  = 4
}video_session_type;

typedef enum hyp_platform
{
    HYP_PLATFORM_UNKNOWN = 0,
    HYP_PLATFORM_LA      = 1,
    HYP_PLATFORM_LV      = 2,
    HYP_PLATFORM_QX      = 3,
    HYP_PLATFORM_GH      = 4,
}hyp_platform_type;

typedef enum hyp_target_variant
{
    HYP_TARGET_UNKNOWN   = 0,
    HYP_TARGET_SM6150    = 1,
    HYP_TARGET_MSMNILE   = 2,
    HYP_TARGET_DIREWOLF  = 3,
    HYP_TARGET_LEMANS    = 4
}hyp_target_variant_type;

typedef struct hypvideo_open_data
{
    hypv_status_type         return_status;
    int64                    return_io_handle;
    video_session_type       video_session;
    /* FE hyp platform Id */
    hyp_platform_type        hyp_plt_id;

    /* BE enable passthrough if FE and BE platform matches.
       Translation to hypervisor abstraction interface is not
       required if passthrough mode is enabled */
    boolean                  passthrough_mode;
}hypvideo_open_data_type;

typedef struct hypvideo_msg_data
{
    /* HYPVIDEO_MSGCMD_OPEN, HYPVIDEO_MSGRESP_OPEN_RET */
    hypvideo_open_data_type     open_data;

    /* HYPVIDEO_MSGCMD_IOCTL, HYPVIDEO_MSGRESP_IOCTL_RET */
    hypvideo_ioctl_data_type    ioctl_data;

    /* HYPVIDEO_MSGCMD_CLOSE, HYPVIDEO_MSGRESP_CLOSE_RET */
    hypvideo_close_data_type    close_data;

    /* HYPVIDEO_MSGRESP_EVENT */
    hypvideo_event_data_type    event_data;
}hypvideo_msg_data_type;

typedef enum
{
    HYPVIDEO_MSGCMD_OPEN        = 0x00000001,
    HYPVIDEO_MSGCMD_IOCTL       = 0x00000002,
    HYPVIDEO_MSGCMD_CLOSE       = 0x00000003,
    HYPVIDEO_MSGRESP_OPEN_RET   = 0x80008001,
    HYPVIDEO_MSGRESP_IOCTL_RET  = 0x80008002,
    HYPVIDEO_MSGRESP_CLOSE_RET  = 0x80000003,
    HYPVIDEO_MSGRESP_EVENT      = 0x80001234,
}hypvideo_msg_id_type;


/***************************************************************************
 *
 * hypvideo_msg_type
 *
 * Define the video command message, respond, and event messages
 *
 ***************************************************************************
 */
typedef struct
{
    /* video hypervisor version */
    uint32                  version;

    /* the virtual_channle is the abstract handle returned from HAB open */
    uint32                  virtual_channel;

    /* identify different message */
    hypvideo_msg_id_type    msg_id;

    /* message number in sequence for debug purpose */
    uint32                  message_number;

    /* the send and receive time in ns for debug purpose */
    uint32                  time_stamp_ns;

    /* the size of supporting data in unit of bytes */
    uint32                  data_size;

    /* the payload corresponding to the token */
    hypvideo_msg_data_type  data;

    /* pid of the gclient */
    uint32                   pid;  // used only for hypervisor simulation in metal

}hypvideo_msg_type;

//
#define SIZE_OF_IOCTL_DATA_HEADER_IN_MSG (sizeof(hypv_status_type)+sizeof(int64)+sizeof(uint32))
#define GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(ps) (ps + SIZE_OF_IOCTL_DATA_HEADER_IN_MSG)
#define GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(mds) (mds-SIZE_OF_IOCTL_DATA_HEADER_IN_MSG)
//
#define SIZE_OF_EVENT_HEADER_IN_MSG (sizeof(hypv_status_type) + sizeof(vidc_event_type))
#define GET_MSG_DATASIZE_FROM_EVENT_VIDCMSG_SIZE(ps) (ps + SIZE_OF_EVENT_HEADER_IN_MSG)
#define GET_EVENT_VIDCMSG_SIZE_SIZE_FROM_MSG_DATASIZE(mds) (mds-SIZE_OF_EVENT_HEADER_IN_MSG)

}

#endif //__HYP_VIDEO_H__

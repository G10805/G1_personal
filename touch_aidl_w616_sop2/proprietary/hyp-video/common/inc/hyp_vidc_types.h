/*========================================================================

*//** @file hyp_vidc_types.h

@par FILE SERVICES:
      The file defines properties, status, events and message types that can
      make up the video codec hypervisor driver.

@par EXTERNALIZED FUNCTIONS:
      See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who      what, where, why
--------   ---     -------------------------------------------------------
10/23/24   ms      Add support for sending HDR10plus dynamic info
02/25/24   pc      Add support for tier for HEVC
05/24/23   pc      Add HDR10 static metadata support for HEVC encode
03/16/23   sd      Support encoder output metadata to get average frame QP
10/18/22   mm      Report crop info during decoding output port reconfig
08/22/22   nb      Add support to query Max supported B frames
07/30/22   nb      Refine setting frame QP range property
05/26/22   ll      Enable bitrate saving mode
03/28/22   sh      Enable Layer Encode for GVM
01/05/22   sd      Add support for blur filter
06/16/21   sh      Include input_tag2 as its needed for dropping inputs in codec2
05/06/21   sj      Add VP9 10 bit and HEVC TIER profiles
02/08/21   mm      Support HEIC profile
11/04/20   hl      Add Long Term Reference Support
10/12/20   sh      Add support to set VPE Color Space Conversion
10/04/20   yc      Add support for HEVC HDR10/HDR10plus
08/27/20   sh      Bringup video decode using codec2
04/21/20   sj      Add support for HEIC/HEIF encoding
01/09/20   sm      Update for low latency mode
11/18/19   sm      Extend rate control for encode usecase
10/09/19   hl      Support Ghs Integrity
08/23/19   sm      Add support for ROI QP
07/09/19   sm      Add P010 color format
06/24/19   sm      Extend flip operation
04/03/19   sm      Remove unsupported VP8 levels
03/04/19   sm      Extented AVC profile levels
01/24/19   sm      Add HEIC codec type
11/20/18   hl      Add spurious interrupt error code
11/14/18   jz      translate HFI_ERR_SESSION_NON_COMPLIANT_STREAM to VIDC_ERR_NONCOMPLIANT_STREAM
07/10/18   sm      Add support to use pmem handle for video buffers
04/25/18   sm      Updated structure for dither and pid
04/04/18   sm      Add support output crop metadata
12/12/17   sm      Add support for drop frames flags from venus
05/08/17   sm      Update for new hyp-video architecture
07/26/16   rz      Clarify comments on payload alloc_len
07/04/16   hl      Add support dynamic buffer mode
06/09/16   hL      Add support linux hypervisor for non-contiguous metadata
05/27/16   hl      Add vp9 decoder
05/19/16   am      Add VPE and MBI session type definitions.
05/11/16   am      Add UBWC and TP10 color format definitions.
05/10/16   am      Fix for 64 bit and 32 bit environments.
12/02/14   rs      Added support for MBI Extradata for encoder
07/02/14   dp      Add flush mode requested type.
05/22/14   dp      Remove ENC_VBV_HRD_BUFSIZE, decode post filter and digital zoom property.
05/16/14   yz      Support the extra data of video signal info available at decoder output
04/14/14   yz      Support setting of video signal info (video full range, video format, etc.)
03/14/14   rs      Add client fatal error status
02/20/14   dp      Change LTR HFI interface name
01/28/14   dp      Change VIDC_METADATA_PROPERTY_LTR to 0x7F100004 to resolve conflict with HFI_INDEX_EXTRADATA_ASPECT_RATIO
12/05/13   am      Add MVC buffer layout.
12/02/13   rz      Add client fatal error event
11/11/13   rz      Move VIDC_INDEX_NOMORE out of Failed category
10/15/13   dp      Update types
10/11/13   am      Support Max Frame Rate setting.
10/09/13   dp      Adding dynamic buffer allocation mode property
10/01/13   uc      Added support for Get/Set client session name for BF.
07/15/13   rz      Adding H264 8x8 transform property
06/24/13   am      Adding peak constr, text quality, low latency, bitstream restc prop
                   Adding HEVC codec, profile and level
                   Adding VIDC_METADATA_MPEG2_SEQDISP extra-data property
05/13/13   rz      Add LTR encoding
02/20/13   st      Adding VUI, AUD and VBV buffer size properties
12/14/12   hm      Adding buffer mode property
12/07/12   sk      Added VIDC_ERR_START_CODE_NOT_FOUND
12/07/12   sk      Added VIDC_FRAME_FLAG_DISCONTINUITY & VIDC_FRAME_FLAG_TEI
10/22/12   hm      Metadata property support
09/04/12   rkk     Added VIDC_I_CONTENT_PROTECTION property.
03/28/12   st      Modified vidc_buffer_info_type to include extra-data buffer addresses
12/12/11   rkk     Defined all types.
12/08/11   st      Created file. First Draft.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#ifndef __HYP_VIDC_TYPES_H__
#define __HYP_VIDC_TYPES_H__

#if defined(__QNXNTO__) || defined(__INTEGRITY)
#include "AEEstd.h" /* std typedefs, ie. byte, uint16, uint32, etc. and memcpy, memset */
#else
typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;
typedef unsigned long long uint64;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned char      uint8;
typedef unsigned char      boolean;
#define TRUE  1
#define FALSE 0
#endif
/*===========================================================================

                      DEFINITIONS AND DECLARATIONS

===========================================================================*/

/* -----------------------------------------------------------------------
**
** CONSTANTS, MACROS & TYPEDEFS
**
** ----------------------------------------------------------------------- */
#define VIDC_MAX_STRING_SIZE (256)

#define VIDC_COLOR_FORMAT_UBWC_BASE      (0x8000)
#define VIDC_COLOR_FORMAT_10_BIT_BASE    (0x4000)

#define VIDC_MAX_MATRIX_COEFFS (9)
#define VIDC_MAX_BIAS_COEFFS   (3)
#define VIDC_MAX_LIMIT_COEFFS  (6)

#define VIDC_BLUR_NONE         (0)
#define VIDC_BLUR_EXTERNAL     (1)
#define VIDC_BLUR_ADAPTIVE     (2)

/*default when layer ID isn't specified*/
#define VIDC_ALL_LAYER_ID      (0xFF)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_status_type
----------------------------------------------------------------------------*/
/**
 * Driver status types that would be emitted as a response to API call either
 * synchronosly or through messages and events
 */
typedef enum
{
   VIDC_ERR_NONE           = 0x0,

   /** No more indices can be enumerated */
   VIDC_ERR_INDEX_NOMORE,

   /** General failure */
   VIDC_ERR_FAIL            = 0x80000000,

   /** Failed while allocating memory */
   VIDC_ERR_ALLOC_FAIL,

   /** Illegal operation was requested */
   VIDC_ERR_ILLEGAL_OP,

   /** Bad paramter(s) or memory pointer(s) provided */
   VIDC_ERR_BAD_PARAM,

   /** Bad driver or client handle provided */
   VIDC_ERR_BAD_HANDLE,

   /** API is currently not supported */
   VIDC_ERR_NOT_SUPPORTED,

   /** Requested API is not supported in current device or client state */
   VIDC_ERR_BAD_STATE,

   /** Allowed maximum number of clients are already opened */
   VIDC_ERR_MAX_CLIENT,

   /** Decoding: VIDC was expecting I-frame which was not provided */
   VIDC_ERR_IFRAME_EXPECTED,

   /** Fatal irrecoverable hardware error detected */
   VIDC_ERR_HW_FATAL,

   /** Decoding: Error in input frame detected and frame processing skipped */
   VIDC_ERR_BITSTREAM_ERR,

   /** Sequence header parsing failed in decoder */
   VIDC_ERR_SEQHDR_PARSE_FAIL,

   /** Error to indicate that the buffer supplied was insufficient in size */
   VIDC_ERR_INSUFFICIENT_BUFFER,

   /** Error indicating that the device is in bad power state and can not service
      the request API. */
   VIDC_ERR_BAD_POWER_STATE,

   /** The error is returned in case a session related API call be made wihtout
       initializing a session. (For e.g. if ABORT is called on a hanlde for
       which vidc_initialize() has not been called) */
   VIDC_ERR_NO_VALID_SESSION,

   /** API was not completed and returned with a timeout */
   VIDC_ERR_TIMEOUT,

   /** API request was not accepted as command queue is full */
   VIDC_ERR_CMDQFULL,

   /** Valid start code was not found within provided compressed video frame */
   VIDC_ERR_START_CODE_NOT_FOUND,

   /** Error indicates stream is unsupported by video core */
   VIDC_ERR_UNSUPPORTED_STREAM,

   /** Error indicates frame was dropped as per VIDC_I_DEC_PICTYPE property */
   VIDC_ERR_SESSION_PICTURE_DROPPED,

   /** To indicate irrecoverable fatal client error was detected.
    *  Client should initiate session clean up. */
   VIDC_ERR_CLIENTFATAL,

   /** Error indicates stream is non-compliant */
   VIDC_ERR_NONCOMPLIANT_STREAM,

   /** Error indicates spurious interrupt handled and no further
    *  processing required */
   VIDC_ERR_SPURIOUS_INTERRUPT,
   VIDC_ERR_UNUSED   = 0xC0000000
} vidc_status_type;
/*..........................................................................*/

#define VIDC_FAILED( rc )  (((uint32)rc >= (uint32)VIDC_ERR_FAIL) ? TRUE : FALSE)

/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_event_type
----------------------------------------------------------------------------*/
/**
 * VIDC event type emitted in response to a command or an event
 * Callback registered in vidc_initialize is used give out the events
 */
typedef enum
{
   /** VIDC event base for API responses - Reserved not to be used directly. */
   VIDC_EVT_RESP_BASE     = 0x1000,

   /** Response for START API call */
   VIDC_EVT_RESP_START,

   /** Response for STOP API call */
   VIDC_EVT_RESP_STOP,

   /** Response for PAUSE API call */
   VIDC_EVT_RESP_PAUSE,

   /** Response for RESUME API call */
   VIDC_EVT_RESP_RESUME,

   /** Response for FLUSH API call to indicate flush completion when input
       buffer flush was requested */
   VIDC_EVT_RESP_FLUSH_INPUT_DONE,

   /** Response for FLUSH API call to indicate flush completion when output
       buffer flush was requested */
   VIDC_EVT_RESP_FLUSH_OUTPUT_DONE,

   /** Response for FLUSH API call to indicate flush completion when output2
       buffer flush was requested */
   VIDC_EVT_RESP_FLUSH_OUTPUT2_DONE,

   /** Response for PROCESS_INPUT_FRAME_CALL response to return input buffer
       for reuse. */
   VIDC_EVT_RESP_INPUT_DONE,

   /** Response for FILL_OUTPUT_BUFFER API call to return processed output */
   VIDC_EVT_RESP_OUTPUT_DONE,

   /** Response for LOAD_RESOURCES API call */
   VIDC_EVT_RESP_LOAD_RESOURCES,

   /** Response for RELEASE_RESOURCES API call */
   VIDC_EVT_RESP_RELEASE_RESOURCES,

   /** Response for LTR Use Failure API call */
   VIDC_EVT_RESP_LTRUSE_FAILED,

   /*.........................................................................*/

   /** VIDC events base for unsolicited indications.Reserved do not use direcly*/
   VIDC_EVT_UNSOLICITED_BASE   = 0x2000,

   /** To indicate configuration change detected via input bitstream for
       input buffer type. */
   VIDC_EVT_INPUT_RECONFIG,

   /** To indicate configuration change detected via input bitstream for
       output buffer type. */
   VIDC_EVT_OUTPUT_RECONFIG,

   /** To indicate irrecoverable fatal HW error was detected. Client should
       initiate session clean up. */
   VIDC_EVT_ERR_HWFATAL,

   /** To indicate irrecoverable fatal client error was detected.
    *  Client should initiate session clean up. */
   VIDC_EVT_ERR_CLIENTFATAL,

   /** To indicate resources were lost for the session. Client should initiate
       session clean up. */
   VIDC_EVT_RESOURCES_LOST,

   /** Informational event to notify that the output buffer reconfiguration
       has been detected but decoding would proceed normally as suffcient
       buffers (both in number and size) are available */
   VIDC_EVT_INFO_OUTPUT_RECONFIG,

   /** To indicate that a specific property setting has changed.
       Client should query for new property value. */
   VIDC_EVT_PROPERTY_CHANGED,

   /** To indicate an error/warning was detected. Based on error type client
       may ignore or close the session. */
   VIDC_EVT_SESSION_ERR,

   /** Informational event to notify that the output buffer is not longer
       referenced, and it can be released */
   VIDC_EVT_RELEASE_BUFFER_REFERENCE,

   VIDC_EVT_UNUSED      =  0x10000000
} vidc_event_type;
/*==========================================================================*/



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_property_id_type
----------------------------------------------------------------------------*/
/**
 * Property Id type for get/set property APIs.
 * It further lists all the property IDs that an application can change through
 * VIDC get/set property APIs. The properties are applicable for encoder,
 * decoder, pre/post-processing as mentioned in comment block below.
 */
typedef enum
{
   VIDC_I_COMMON_START_BASE   =   0x0,    /**< Unused Property ID */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_LIVE
 *   Property Structure: vidc_enable_type
 *   Description       : Indicates that it is a Live Appl
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Default value     : TRUE
 *   </pre>
 */
   VIDC_I_LIVE,                           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_PRIORITY
 *   Property Structure: vidc_priority_type
 *   Description       : Client can set priority for the session.
 *   SetProperty for   : Decoder, Encoder
 *   GetProperty for   : Decoder, Encoder
 *   Reconfigurable    : After vidc_initialize() any time during the session
 *   Initial Default value : VIDC_PRIORITY_MEDIUM
 *   </pre>
 */
   VIDC_I_PRIORITY,                       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_FRAME_SIZE
 *   Property Structure: vidc_frame_size_type
 *   Description       : Frame Resolution i.e frame width and frame height
 *   SetProperty for   : Encoder and Decoder
 *
 *                       Encoder Input  : Specifies input YUV dimension
 *                       Encoder Output : If different from input dimension
 *                                        scaling will be performed.
 *
 *                       Decoder Input  : Frame size of the stream. It will be
 *                                        over-written by stream properties.
 *                                        If different from output, output
 *                                        frame size will be generated and
 *                                        a reconfig would be generated
 *
 *                       Decoder Output : Decoded frame size. It will be
 *                                        over-written by stream properties.
 *                                        If different from input, error will
 *                                        be generated.
 *                                        If a scaled image is desired use
 *                                        VIDC_I_DEC_MULTI_STREAM option.
 *
 *                       Decoder Output2: Not supported. Please refer to
 *                                        VIDC_I_DEC_MULTI_STREAM
 *
 *   ROI can be mentioned with each frame in EMPTY_INPUT_BUFFER call for
 *   Encoder session
 *
 *   GetProperty for   : Encoder and Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : nWidth = 176, nHeight = 144
 *   </pre>
 **/
   VIDC_I_FRAME_SIZE,                     /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_METADATA_HEADER
 *   Property Structure: vidc_metadata_header_type
 *   Description       : Enables/Disables particular metadata and
 *                       sets different header fields for it.
 *                       NOTE: Enabling/Disabling of particular meta data
 *                       impacts buffer requirements. Hence this should be
 *                       done before allocating/setting IO buffers on driver.
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : All metadata types for Decoder, Encoder
 *                           are disabled by default.
 *   </pre>
 **/
   VIDC_I_METADATA_HEADER,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_SESSION_CODEC
 *   Property Structure: vidc_session_codec_type
 *   Description       : Session & Codec type setting for encoder/decoder.
 *                       NOTE: Setting a new session/codec type will reset
 *                       all other property settings to the default values for
 *                       for the new session/codec type.
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : n/a
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_SESSION_CODEC,                  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_PROFILE
 *   Property Structure: vidc_profile_type
 *   Description       : Encoder/Decoder profile setting
 *   SetProperty for   : Encoder (optional), Decoder (e.g. for MVC profiles)
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       Decoder: Can get after Seq. Header parsing
 *   Initial Default value : Variable.
 *   </pre>
 */
   VIDC_I_PROFILE,                        /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_LEVEL
 *   Property Structure: vidc_level_type
 *   Description       : encoder level settings
 *   SetProperty for   : Encoder (optional)
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       Decoder: Can get after Seq. Header parsing
 *   Initial Default value : Variable
 *   </pre>
 */
   VIDC_I_LEVEL,                          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_COLOR_FORMAT
 *   Property Structure: vidc_color_format_config_type
 *   Description       : Chroma format for uncompressed data. It is relevant
 *                       only for:
 *                       1) Encoder Input
 *                       2) Decoder Output
 *                       3) Decoder Output2
 *
 *   SetProperty for   : Encoder, decoder
 *   GetProperty for   : Encoder, decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : COLOR_FORMAT_NV12
 *   </pre>
 */
   VIDC_I_COLOR_FORMAT,                   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_CODEC
 *   Property Structure: vidc_capability_codec_type
 *   Description       : Enumerate codecs supported by core.
 *                       Multiple calls with increasing values of index will
 *                       enumerate the supported types.
 *   SetProperty for   : n/a
 *   GetProperty for   : Device
 *   Reconfigurable    : Can be queried any time in the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_CODEC,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_COLOR_FORMAT
 *   Property Structure: vidc_capability_color_format_type
 *   Description       : Enumerate buffer formats (color formats) supported
 *                       by core.
 *                       Multiple calls with increasing values of index will
 *                       enumerate the supported types.
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder & decoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_COLOR_FORMAT,        /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_PROFILE_LEVEL
 *   Property Structure: vidc_capability_profile_level_type
 *   Description       : Enumerate codec profile & levels supported by core.
 *                       Multiple calls with increasing values of index will
 *                       enumerate the supported types.
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder & decoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_PROFILE_LEVEL,       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_FRAME_SIZE
 *   Property Structure: vidc_capability_frame_size_type
 *   Description       : Gives the minimum & maximum supported frame size
 *                       values. Only width & height values that are multiple
 *                       of step size are supported.
 *                       Frame size values for the currently set codec type
 *                       will be returned. Client should set the codec type
 *                       before querying for this capability.
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder & decoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_FRAME_SIZE,          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_LTR_COUNT
 *   Property Structure: vidc_range_type
 *   Description       : Gives the minimum & maximum supported ltr count.
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_LTR_COUNT,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

   /**  <pre>
 *   Property id       : VIDC_I_BUFFER_REQUIREMENTS
 *   Property Structure: vidc_buffer_reqmnts_type
 *   Description       : Get/Set buffer requirements from video core.
 *                       When setting new buffer requirements, they must meet
 *                       the minimum requirements returned via Get Property.
 *   SetProperty for   : Encoder, decoder
 *   GetProperty for   : Encoder, decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       Also permitted during session reconfiguration.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_BUFFER_REQUIREMENTS,            /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_PLANE_DEF
 *   Property Structure: vidc_plane_def_type
 *   Description       : Specifies layout of raw data.
 *
 *                       nWidth and nHeight set through VIDC_I_FRAME_SIZE
 *                       cannot be more than stride set through this property.
 *
 *                       Parameters mentioned in this property will affect
 *                       IO memory requirements
 *
 *   SetProperty for   : Encoder and Decoder
 *                       The property is supported only for uncompressed domain
 *                       buffers.
 *
 *                       Encoder Input  : Supported
 *                       Encoder Output : Not Supported
 *                       Decoder Input  : Not Supported
 *                       Decoder Output1: Supported
 *                       Decoder Output2: Supported
 *
 *   GetProperty for   : Encoder and Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       Also permitted during reconfig for decoder output
 *   Initial Default value : nActualStride = 176,
 *                           nActualPlaneBufferHeight = 144
 *   </pre>
 **/
   VIDC_I_PLANE_DEF,                      /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_SCAN_FORMAT
 *   Property Structure: vidc_capability_scan_format_type
 *   Description       : Can be used to query all the supported scan formats
 *                       (progressive, interlace etc.) for input or output
 *                       buffer types. This applies to only uncompressed domain
 *                       buffers, namely:
 *                       Encoder - input
 *                       Decoder - output1 and output2
 *
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder and decoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_SCAN_FORMAT,         /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_NAL_STREAM_FORMAT
 *   Property Structure: vidc_stream_format_type
 *   Description       : Can be used to query supported nal stream formats
 *                       for a H.264 specifc encoder or decoder session
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder and decoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_NAL_STREAM_FORMAT,   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_NAL_STREAM_FORMAT
 *   Property Structure: vidc_stream_format_type
 *   Description       : Client can configure the desired stream format for
 *                       a H.264 specific decoder session. For encoder only
 *                       VIDC_NAL_FORMAT_STARTCODES is supported
 *   SetProperty for   : Decoder and Encoder
 *   GetProperty for   : Decoder and Encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : VIDC_NAL_FORMAT_STARTCODES
 *   </pre>
 */
   VIDC_I_NAL_STREAM_FORMAT,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DIVX_FORMAT
 *   Property Structure: vidc_divx_format_config_type
 *   Description       : To get/set divx format for current codec session
 *                       This property is valid only when the codec type
 *                       is set to VIDC_CODEC_DIVX
 *   SetProperty for   : NA
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_DIVX_FORMAT,                    /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_FRAME_RATE
 *   Property Structure: vidc_frame_rate_type
 *   Description       : Frame rate per second for specified buffer_type
 *                       frame rate = numerator  / denominator
 *
 *                       Encoder Input: Input frame rate. Could be used for
 *                       clock scaling and other power related caculation.
 *                       Rate control would ignore this setting and would use
 *                       timestamp instead
 *
 *                       Encoder Output: Shall be same as Input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *                       Decoder Input: To indicate input frame rate. Used for
 *                       clock and power optimization
 *
 *                       Decoder Output: Shall be equal to input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *                       Decoder Output2: Shall be equal to input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Dynamic property.
 *                       Can be updated after vidc_initialize() any time
 *                       during an encoding session
 *  Initial default value : Decoder : 15 fps (numerator = 15, denominator = 1)
 *                          Encoder : 15 fps (numerator = 15, denominator = 1)
 *  </pre>
 **/
   VIDC_I_FRAME_RATE,                     /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_MAX_FRAME_RATE
 *   Property Structure: vidc_frame_rate_type
 *   Description       : Frame rate per second for specified buffer_type
 *                       frame rate = numerator  / denominator
 *
 *                       Encoder Input: max input frame rate. Could be used for
 *                       clock scaling and other power related caculation.
 *                       Rate control would ignore this setting and would use
 *                       timestamp instead
 *
 *                       Encoder Output: Shall be same as Input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *                       Decoder Input: To indicate max input frame rate. Used for
 *                       clock and power optimization
 *
 *                       Decoder Output: Shall be equal to max input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *                       Decoder Output2: Shall be equal to max input frame rate. A
 *                       different frame rate than input is not supported.
 *
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Not allowed.
 *                       Can only be specified before starting the session.
 *
 *  Initial default value : Decoder : 15 fps (numerator = 15, denominator = 1)
 *                          Encoder : 15 fps (numerator = 15, denominator = 1)
 *  </pre>
 **/
   VIDC_I_MAX_FRAME_RATE,                 /* <<=== ==PROPERTY== ==== */

/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_MULTI_VIEW_FORMAT
 *   Property Structure: vidc_multi_view_format_type
 *   Description       : To get/set multi view format configuration: number
 *                       of views and view order.
 *   SetProperty for   : Decoder and Encoder
 *   GetProperty for   : Decoder and Encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : number of views is 1
 *   </pre>
 */
   VIDC_I_MULTI_VIEW_FORMAT,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_SEQUENCE_HEADER
 *   Property Structure: vidc_seq_hdr_type
 *   Description       : To get (encode sequence header) or set (parse
 *                       sequence header) on the video core.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Encoder. The size of the provided buffer should be
 *                       >= to the size queried using the
 *                       VIDC_I_ENC_MAX_SEQUENCE_HEADER_SIZE property.
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       for decoding session.
 *                       Get can be called any time after vidc_initialize()
 *                       during an encoding session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_SEQUENCE_HEADER,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_TARGET_BITRATE
 *   Property Structure: vidc_target_bitrate_type
 *   Description       : Encoder target bitrate
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : 64000
 *   </pre>
 */
   VIDC_I_TARGET_BITRATE,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_BUFFER_ALLOC_MODE
 *   Property Structure: vidc_buffer_alloc_mode_type
 *   Description       : Allows a client to set the buffer allocation mode
 *                       type. Allocated buffer can be either static mode
 *                       or one single ring buffer mode.
 *   SetProperty for   : Decode/Encode
 *   GetProperty for   : Decode/Encode
 *   Reconfigurable    : Only between vidc_initialize() and before buffer
 *                       allocation
 *   Initial Default value : Static mode
 *  </pre>
 **/
   VIDC_I_BUFFER_ALLOC_MODE,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_MVC_BUFFER_LAYOUT
 *   Property Structure: vidc_mvc_buffer_layout_descp_type
 *   Description       : Allows a client to set on FW to generate output in
 *                       this specific format for decoder session.
 *   SetProperty for   : Decode
 *   GetProperty for   : Decode
 *   Reconfigurable    : Only between vidc_initialize() and before buffer
 *                       allocation
 *   Initial Default value : VIDC_MVC_BUFFER_LAYOUT_TOP_BOTTOM
 *  </pre>
 **/
   VIDC_I_MVC_BUFFER_LAYOUT,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_BIT_DEPTH
 *   Property Structure: vidc_bit_depth_type
 *   Description       : Allows a client to query current stream bit depth.
 *   SetProperty for   : N/A
 *   GetProperty for   : Decode
 *   Reconfigurable    : Not configurable by client
 *   Initial Default value : VIDC_BITDEPTTH_8BIT
 *  </pre>
 **/
   VIDC_I_BIT_DEPTH,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_MAX_FRAME_RATE
 *   Property Structure: vidc_frame_rate_type
 *   Description       : Maximum supported frame rate per second i.e.
 *                       frame rate = numerator  / denominator
 *                       Driver may not support this FPS value for all
 *                       supported resolutions.
 *                       Frame rate value for the currently set codec
 *                       type will be returned. Client should set the
 *                       codec type before querying for this capability.
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *  </pre>
 **/
   VIDC_I_CAPABILITY_MAX_FRAME_RATE,      /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_MAX_MACROBLOCKS
 *   Property Structure: vidc_capability_mb_type
 *   Description       : Maximum supported macroblocks per frame and
 *                       per second.
 *                       Macroblock value for the currently set codec type
 *                       will be returned. Client should set the codec type
 *                       before querying for this capability.
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_MAX_MACROBLOCKS,     /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_MAX_TARGET_BITRATE
 *   Property Structure: vidc_target_bitrate_type
 *   Description       : Maximum supported encoder target bitrate
 *                       Target bitrate value for the currently
 *                       set codec type will be returned. Client
 *                       should set the codec type before querying for
 *                       this capability.
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_MAX_TARGET_BITRATE,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CONTENT_PROTECTION
 *   Property Structure: vidc_enable_type
 *   Description       : Property to enable/disable content protection for
 *                       the session. When enabled input/output buffers
 *                       would be allocated from appropriate heap and be
 *                       mapped to fall within the appropriate secure device
 *                       address ranges on video core.
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES.
 *                       Also can be set only when no input/output buffers have
 *                       been allocated or set.
 *   Initial Default value : FALSE (Disabled)
 *   </pre>
 */
   VIDC_I_CONTENT_PROTECTION,             /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_SESSION_NAME
 *   Property Structure: vidc_session_name_type
 *   Description       : Property to get/set client session name.
 *   SetProperty for   : Encoder, Decoder
 *   GetProperty for   : Encoder, Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES.
 *                       Also can be set only when no input/output buffers have
 *                       been allocated or set.
 *   Initial Default value : Set by the device at session start, e.g., "Client1"
 *   </pre>
 */
   VIDC_I_SESSION_NAME,                   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_TIER
 *   Property Structure: vidc_tier_type
 *   Description       : Property to get/set tier settings.
 *                       This can be set in HEVC encode and
 *                       get can be called for this in HEVC decode.
 *   SetProperty for   : Encoder
 *   GetProperty for   : Decoder, Encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *                       Decoder: Can get after Seq. Header parsing
 *   Initial Default value : Variable
 *   </pre>
 */
   VIDC_I_TIER,                          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/****************************************************************************/
/*
 * The following properties are only applicable to Encoder.
 */
   VIDC_I_ENC_START_BASE  = 0x100,        /**< Unused Property ID */

/**  <pre>
 *   Property id       : VIDC_I_ENC_MULTI_SLICE
 *   Property Structure: vidc_multi_slice_type
 *   Description       : Slice based encoding enable/disable & configuration
 *   SetProperty for   : encoder (optional)
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Off
 *   </pre>
 */
   VIDC_I_ENC_MULTI_SLICE,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_H264_ENTROPY_CTRL
 *   Property Structure: vidc_entropy_control_type
 *   Description       : H.264 Entropy Coding Control Settings
 *   SetProperty for   : H.264 encoder (optional)
 *   GetProperty for   : H.264 encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : CABAC if supported in profile,
 *                           CAVLC otherwise
 *   </pre>
 */
   VIDC_I_ENC_H264_ENTROPY_CTRL,          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_H264_DEBLOCKING
 *   Property Structure: vidc_db_control_type
 *   Description       : Deblocking filter Control Settings
 *   SetProperty for   : H.264 encoder (optional)
 *   GetProperty for   : H.264 encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Enable all blocking boundary deblocking filter
 *   </pre>
 */
   VIDC_I_ENC_H264_DEBLOCKING,            /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_RATE_CONTROL
 *   Property Structure: vidc_rate_control_type
 *   Description       : Different rate control type
 *                       Change in RC type will lead to change in the
 *                       following properties to their corresponding
 *                       "Initial default" values:
 *                       1. Initial QP (VIDC_I_ENC_SESSION_QP)
 *                       2. Intra Period (VIDC_I_ENC_INTRA_PERIOD)
 *                       3. Cyclic Intra refresh (VIDC_I_ENC_INTRA_REFRESH)
 *                       4. Some other RC related register settings.
 *
 *                       Client should first set this property and later can
 *                       change individual RC related properties.
 *   SetProperty for   : encoder (optional)
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : VIDC_RATE_CONTROL_CBR_VFR
 *   </pre>
 */
   VIDC_I_ENC_RATE_CONTROL,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_SESSION_QP
 *   Property Structure: vidc_session_qp_type
 *   Description       : Session QP for encoder.
 *                       The property takes effect when rate control mode
 *                       is VIDC_RATE_CONTROL_OFF
 *   SetProperty for   : encoder (optional)
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Based on Rate Control _type
 *   </pre>
 */
   VIDC_I_ENC_SESSION_QP,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_SESSION_QP_RANGE
 *   Property Structure: vidc_session_qp_range_type
 *   Description       : QP Range for encoder session.
 *
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Codec Specific:
                              1.    MPEG4/H263:    MinQP=1, MaxQP=31
                              2.    H264:          MinQP=1, MaxQP=51
                              3.    VP8:           MinQP=1, MaxQP=127
 *   </pre>
 */
   VIDC_I_ENC_SESSION_QP_RANGE,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_INTRA_PERIOD
 *   Property Structure: vidc_iperiod_type
 *   Description       : Encoder I-frame period pattern
 *   SetProperty for   : encoder (optional)
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : Based on Codec type
 *   </pre>
 */
   VIDC_I_ENC_INTRA_PERIOD,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MPEG4_VOP_TIMING
 *   Property Structure: vidc_vop_timing_type
 *   Description       : Settings for VOP time resolution
 *   SetProperty for   : MPEG4 encoder Only (optional)
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Based on frame rate
 *   </pre>
 */
   VIDC_I_ENC_MPEG4_VOP_TIMING,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MPEG4_SHORT_HEADER
 *   Property Structure: vidc_enable_type
 *   Description       : Short Header on/off for MPEG4 encoding session
 *   SetProperty for   : MPEG4 encoder (madatory if Short Header needed)
 *   GetProperty for   : For MPEG4 encoder session only
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Off
 *   </pre>
 */
   VIDC_I_ENC_MPEG4_SHORT_HEADER,         /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MPEG4_HEADER_EXTENSION
 *   Property Structure: uint32
 *   Description       : This property is to enable / disable Header
 *                       Extension Code. If enable, it signifies the number
 *                       of consecutive video packets between header
 *                       extension codes.
 *   SetProperty for   : MPEG4 encoder (madatory if HEC is required)
 *   GetProperty for   : For MPEG4 encoder session only
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : 0 ( HEC Off)
 *   </pre>
 */
   VIDC_I_ENC_MPEG4_HEADER_EXTENSION,     /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MPEG4_ACPRED
 *   Property Structure: vidc_enable_type
 *   Description       : Enable or disable AC prediction
 *                       for MPEG4 encoder
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Enabled
 *   </pre>
 */
   VIDC_I_ENC_MPEG4_ACPRED,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MAX_SEQUENCE_HEADER_SIZE
 *   Property Structure: vidc_seq_hdr_size_type
 *   Description       : Client can query worst case allocation needed
 *                       for querying sequence header.
 *
 *                       This size is calculated based on current
 *                       session properties.
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : After vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_ENC_MAX_SEQUENCE_HEADER_SIZE,   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_INTRA_REFRESH
 *   Property Structure: vidc_intra_refresh_type
 *   Description       : Configure Intra refresh mode and number of intra
 *                       MBs to be refresh for different refresh mdoe
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Variable
 *   </pre>
 */
   VIDC_I_ENC_INTRA_REFRESH,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_TIMESTAMP_SCALE
 *   Property Structure: vidc_fraction_type
 *   Description       : Scale the time stamp for the output buffer
                         timestampScale = numerator  / denominator
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property except for MPEG4.
 *                       For MPEG4, can be updated only between
 *                       vidc_initialize() and LOAD_RESOURCES.
 *                       For others, can be updated after vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : 1
 *   </pre>
 */
   VIDC_I_ENC_TIMESTAMP_SCALE,            /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_TEMPORAL_SPATIAL_TRADEOFF
 *   Property Structure: vidc_temporal_spatial_tradeoff_type
 *   Description       : Allows to set temporal-spatial tradeoff factor
 *                       in [0, 100].
 *                       A lower value means that more
 *                       weightage is given to picture quality ( spatial )
 *                       over frame rate ( temporal ) in Rate control
 *                       0: No consideration for frame-rate
 *                       100: Only frame rate is important
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : 50
 *   </pre>
 */
   VIDC_I_ENC_TEMPORAL_SPATIAL_TRADEOFF,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_IDR_PERIOD
 *   Property Structure: vidc_idr_period_type
 *   Description       : IDR period for codecs with IDR support
 *                       ( e.g. VIDC_CODEC_H264 )
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Anytime after vidc_initialize()
 *   Initial Default value : Variable
 *   </pre>
 */
   VIDC_I_ENC_IDR_PERIOD,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_REQUEST_SYNC_FRAME
 *   Property Structure: vidc_enable_type
 *   Description       : Request for one sync frame during an encoding session
 *                       Sync frame for H.264 is IDR, for other codecs it is I
 *                       frame.
 *   SetProperty for   : encoder
 *   GetProperty for   : n/a
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Default value     : n/a
 *   </pre>
 */
   VIDC_I_ENC_REQUEST_SYNC_FRAME,         /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_SLICE_DELIVERY_MODE
 *   Property Structure: vidc_enable_type
 *   Description       : Enable/Disable slice based delivery mode for output.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_SLICE_DELIVERY_MODE,        /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_SYNC_FRAME_SEQ_HDR
 *   Property Structure: vidc_enable_type
 *   Description       : Enable/Disable sequence header generation whenever
 *                       a sync frame is generated
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_SYNC_FRAME_SEQ_HDR,         /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_H264_IDR_S3D_FPA_NAL
 *   Property Structure: vidc_enable_type
 *   Description       : Enable/Disable FPA sei nal generation whenever
 *                       a sync frame is generated
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_H264_IDR_S3D_FPA_NAL,       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_GEN_AUDNAL
 *   Property Structure: vidc_enable_type
 *   Description       : Enable or disable generating AUD NAL for H264 encoded
 *                       stream. When enabled every NAL will being with an AUD
 *                       NAL
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_GEN_AUDNAL,            /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_VUI_TIMING_INFO
 *   Property Structure: vidc_vui_timing_info_type
 *   Description       : Set a H264 encoding to generate VUI timing info
 *                       in the encoded stream.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_VUI_TIMING_INFO,       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_BITSTREAM_RESTRC
 *   Property Structure: vidc_enable_type
 *   Description       : Set a H264 encoding to generate VUI information
                         covered under bitstream_restriction_flag. Information
                         would be generated everytime SPS/PPS is generated.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Enabled
 *   </pre>
 */
   VIDC_I_ENC_BITSTREAM_RESTRC,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_LOW_LATENCY_MODE
 *   Property Structure: vidc_enable_type
 *   Description       : Enable/Disable low latency mode for decoding/encoding.
                         The porperty informs about the low-latency behaviour of
                         the application. FW should choose the tools so that
                         end to end latencies are minimum possible.
 *   SetProperty for   : encoder, decoder
 *   GetProperty for   : encoder, decoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Enabled
 *   </pre>
 */
   VIDC_I_LOW_LATENCY_MODE,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MAX_BITRATE
 *   Property Structure: vidc_bitrate_type
 *   Description       : Applicable only for Peak Constrained VBR RC encoding.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Default value     : 0
 *   </pre>
 */
   VIDC_I_ENC_MAX_BITRATE,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_PRESERVE_TEXT_QUALITY
 *   Property Structure: vidc_enable_type
 *   Description       : Property when set indicates that the portions of video
                         frame being encoded may contain text and encoder
                         should select the tools to preserve text quality.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_PRESERVE_TEXT_QUALITY,      /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MAX_NUM_B_FRAMES
 *   Property Structure: uint32
 *   Description       : Max number of B pictures allowed for the session
                         between 2 P frames. This number could be used by the
                         FW in optimizing the DPB buffer size. The allowed
                         range for this is 0-4. The ratio of p_frames and
                         b_frames in VIDC_I_ENC_INTRA_PERIOD property shell not
                         exceed this number.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : 0
 *   </pre>
 */
   VIDC_I_ENC_MAX_NUM_B_FRAMES,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_H264_8X8_TRANSFORM
 *   Property Structure: vidc_enable_type
 *   Description       : Property to enable 8x8 Transform mode for high profile
 *                       encoding in H264. This mode can only be enabled for
 *                       VIDC_PROFILE_H264_HIGH and
 *                       VIDC_PROFILE_H264_CONSTRAINED_HIGH profiles.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disable
 *   </pre>
 */
   VIDC_I_ENC_H264_8X8_TRANSFORM,         /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_LTR_MODE
 *   Property Structure: vidc_ltr_mode_type
 *   Description       : Property to enable LTR mode for H264 or VP8 encoder
 *                       session.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disable
 *   </pre>
 */
   VIDC_I_ENC_LTR_MODE,                   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_LTR_COUNT
 *   Property Structure: uint32
 *   Description       : Property to set a number of LTR frames for H.264
 *                       or VP8 LTR encoder session.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : 0
 *   </pre>
 */
   VIDC_I_ENC_LTR_COUNT,                  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_USELTRFRAME
 *   Property Structure: vidc_use_ltr_type
 *   Description       : Indicates that the LT reference must be used to encode
 *                       the next frame.
 *                       For H264:
 *                       Payload should use LongTermFramIdx to specify next
 *                       frame to be encoded with the reference of given
 *                       LongTermFramIdx.
 *                       In case of RC drops frame which has Use pending,
 *                       firmware needs to store pending Use and apply it in
 *                       next encoded frame. In case of consecutive Use command
 *                       and RC frame drop, if there are multiple Use command
 *                       in same frame, firmware needs to honor latest Use
 *                       command along with its payload.
 *                       For HierP case:
 *                          This command is valid for both base layer and
 *                          non-base layer frame. In case of non-base layer
 *                          frame, Firmware should apply it immediately and
 *                          should not wait for base layer frame.
 *                       For VP8:
 *                       The payload represents the reference frame type to be
 *                       used, which is indicated by a bit value of ‘1’ in the
 *                       corresponding position:
 *                            Bit 0 – Golden frame
 *                            Bit 1 – Alternate frame
 *                            Bit 2 – Previous frame
 *                       In the case where RC drops frame which was to use a
 *                       specified reference frame, the Firmware shall apply
 *                       pending “use” to the next encoded frame.
 *   SetProperty for   : encoder
 *   GetProperty for   : n/a
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_start()
 *                       any time during an encoding session
 *   Default value     : n/a
 *   </pre>
 */
   VIDC_I_ENC_USELTRFRAME,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_LTRPERIOD
 *   Property Structure: uint32
 *   Description       : Configure the LTR period in VIDC_LTR_MODE_PERIODIC LTR
 *                       mode.
 *                       When dynamically changing the LTR period after
 *                       enocoder start, FW would do the best effort to adjust
 *                       to the new LTR period during transition time.
 *   SetProperty for   : encoder
 *   GetProperty for   : n/a
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_start()
 *                       any time during an encoding session
 *   Default value     : n/a
 *   </pre>
 */
   VIDC_I_ENC_LTRPERIOD,                  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_VUI_VIDOE_SIGNAL_INFO
 *   Property Structure: vidc_vui_video_signal_info_type
 *   Description       : Set MPEG4/H264/H265 encoding to generate VUI video signal info
 *                       in the encoded stream.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_VUI_VIDEO_SIGNAL_INFO,       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_ROI_MODE_TYPE
 *   Property Structure: vidc_roi_mode_type
 *   Description       : Get H264/H265 encoding ROI mode type
 *   SetProperty for   : n/a
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : n/a
 *   </pre>
 */
   VIDC_I_ENC_ROI_MODE_TYPE,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_ENABLE_GRID
 *   Property Structure: vidc_enable_type
 *   Description       : Enable grid for HEIC color format
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_ENC_ENABLE_GRID,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_FRAME_QUALITY
 *   Property Structure: vidc_frame_quality_type
 *   Description       : Encoder frame quality
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_ENC_FRAME_QUALITY,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_CONTENT_ADAPTIVE_CODING
 *   Property Structure: vidc_enable_type
 *   Description       : If enabled based on scene analysis HW will select appropriate
                         bitrate while maintaining visually lossless encode quality.
                         Support for H264/HEVC encoders for VBR mode except when layering is enabled.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Enabled
 *   </pre>
 */
   VIDC_I_ENC_CONTENT_ADAPTIVE_CODING,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_MARKLTRFRAME
 *   Property Structure: uint32
 *   Description       : property to mark LTR frame
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_ENC_MARKLTRFRAME,                /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/
/****************************************************************************/
/**  <pre>
 *   Property id       : VIDC_I_ENC_HIER_B_MAX_NUM_ENH_LAYER
 *   Property Structure: uint32
 *   Description       : Property configures HierB
                         Client needs to configure this property to enable Hier-B encoding.
                         Encoder generates bitstream with these many enhancement layers.

                         Applicable for HEVC.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : 0
 *   </pre>
 */
   VIDC_I_ENC_HIER_B_MAX_NUM_ENH_LAYER,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_HIER_P_MAX_NUM_ENH_LAYER
 *   Property Structure: uint32
 *   Description       : Property configures HierP that can be configured for static/dynamic HierP.
                         Applicable for H264.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : 0
 *   </pre>
 */
   VIDC_I_ENC_HIER_P_MAX_NUM_ENH_LAYER,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_HIER_P_HYBRID_MODE
 *   Property Structure: uint32
 *   Description       : Property to Enable Hier P using LTR frames (Hybrid model)
                         Specifies the number of HIER P enhancement layers to be encoded.
                         Only applicable for H264 Encode.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_ENC_HIER_P_HYBRID_MODE,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_HIER_P_ENH_LAYER
 *   Property Structure: uint32
 *   Description       : Configures the number of hierarchical P pictures to be encoded
                         Encoding starts with these many hierarchical P pictures ( temporal layers )
                         if set before start of session.
                         For H264, allows dynamically adding/removing temporal (Hier P)
                         layers, at arbitrary picture position, without introducing a new IDR.
                         No frame dropping shall occurs in H.264/AVC encoder because of
                         addition/removal of temporal layers.

                         In case of dynamic Hier P layer change, fw should not generate Sync frame
                         if it could accommodate new sub-gop starting at next base layer frame.
                         FW would generate sync frame only if next sub-gop exceeds configured
                         Total GOP Distance.
                         The parameter ( N enh layers ) conveyed through this property specifies the
                         number of enhancement layers to be encoded.
                         N: base layer + N enhancement layers
                         Layer ids: 0 for base layer, 1 for 1st enh layer and so on.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Dynamic property. Can be updated after
 *                       vidc_initialize()
 *                       any time during an encoding session
 *   Initial Default value : Single Layer
 *   </pre>
 */
   VIDC_I_ENC_HIER_P_ENH_LAYER,                 /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_CAPABILITY_MAX_NUM_BFRAMES
 *   Property Structure: vidc_range_type
 *   Description       : Maximum number of B-frames supported between
 *                       two P-Frames
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time
 *                       in the session.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_CAPABILITY_MAX_NUM_BFRAMES,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_ENC_HDR_INFO
 *   Property Structure: vidc_metadata_hdr_static_info
 *   Description       : Property configures HDR static metadata that can be configured for HDR10/HDR10+
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_ENC_HDR_INFO,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/****************************************************************************/
/*
 * The following properties are only applicable for Decoder.
 */
   VIDC_I_DEC_START_BASE   = 0x200,       /**< Unused Property ID */

/**  <pre>
 *   Property id       : VIDC_I_DEC_PROGRESSIVE_ONLY
 *   Property Structure: uint32 ( 1: Content is only progressive )
 *                              ( 0: Content can be either progressive or
 *                                   interlaced )
 *   Description       : To Get information if bitstream content is progressive
 *                       only or progressive/interlaced.
 *   SetProperty for   : n/a
 *   GetProperty for   : decoder
 *   Reconfigurable    : Can get after seq. hdr. parsing
 *   Default value     : n/a
 *   </pre>
 */
   VIDC_I_DEC_PROGRESSIVE_ONLY,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_CONT_ON_RECONFIG
 *   Property Structure: vidc_enable_type
 *   Description       : Configure decoder to continue decoding in case
 *                       sufficient buffer (count and size) has been
 *                       allocated to handle reconfig generated. This would
 *                       happen wihtout buffer re-allocation
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_DEC_CONT_ON_RECONFIG,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_OUTPUT_ORDER
 *   Property Structure: vidc_output_order_type
 *   Description       : Client can configure if decoder shall output the
 *                       decoded frames in display order or decode order
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : After vidc_initialize() any time during decoding
 *                       session
 *   Initial Default value : Display order output
 *   </pre>
 */
   VIDC_I_DEC_OUTPUT_ORDER,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_PICTYPE
 *   Property Structure: vidc_dec_pic_type
 *   Description       : Configure decoder to process certain picture type(s).
 *                       Enabling this operating mode can reduce processing
 *                       and significantly reduce the decoder’s resource
 *                       requirements.
 *                       If only “sync” picture type is specified:
 *                        1. memory requirement would be minimal.
 *                        2. If input frame is not a "sync" frame, then client
 *                           would receive VIDC_ERR_IFRAME_EXPECTED error in
 *                           input done callback. There would not be any output.
 *                        If "sync" and P frames are allowed and if the input
 *                        frame is not-allowed type, then client would receive
 *                        VIDC_ERR_INPUT_NOTPROCESSED error in input done
 *                        callback. There would not be any output.
 *                        For the above input done error cases, client can
 *                        continue feeding frames.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : After vidc_initialize() any time during decoding
 *                       session
 *   Initial Default value : All picture types are processed
 *   </pre>
 */
   VIDC_I_DEC_PICTYPE,                    /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_MULTI_STREAM
 *   Property Structure: vidc_multi_stream_type
 *   Description       : Client can configure which output stream to be used
 *                       for output. In special cases both the output
 *                       streams could be enabled (for example when apart
 *                       from regular output a scaled down version is also
 *                       desired from decoder). The property can be used
 *                       to configure output2 dimension.
 *                       This property when exercised on the fly can lead
 *                       to reconfig for output2.
 *
 *                       Also refer to VIDC_I_DEC_OUTPUT2_KEEP_ASPECT_RATIO
 *
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Any time after vidc_initialize()
 *   Initial Default value : VIDC_BUFFER_OUTPUT is Enabled and
 *                           VIDC_BUFFER_OUTPUT2 is disabled
 *   </pre>
 */
   VIDC_I_DEC_MULTI_STREAM,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_MB_ERROR_MAP_REPORTING
 *   Property Structure: vidc_enable_type
 *   Description       : Client can configure the driver to enable/disable
 *                       logging in MB error.
 *
 *                       Error map can be queried using
 *                       VIDC_I_DEC_MB_ERROR_MAP
 *
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Can be set any time after vidc_initialize()
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_DEC_MB_ERROR_MAP_REPORTING,     /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_MB_ERROR_MAP
 *   Property Structure: vidc_mb_error_map_type
 *   Description       : Client can query macro block error map after
 *                       MB error map reporting is enabled.
 *
 *                       Error map is cummulative in nature and is reset
 *                       everytime either,
 *                       1) An intra frame is decoded
 *                       2) The map is queried using the property
 *
 *                       Query can be made once frame decode has started.
 *                       Client must allocate sufficient buffer to fetch the
 *                       error map. Every MB's error status is represented as
 *                       a byte. For e.g. to query an error map for a session
 *                       decoding 1080p video, client must allocate at least
 *                       1920x1088/256 = 8160 bytes (failing this will make the
 *                       API return VIDC_ERR_INSUFFICIENT_BUFFER )
 *
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Can be queried any time after frame decoding has
 *                       started.
 *   Initial Default value : n/a
 *   </pre>
 */
   VIDC_I_DEC_MB_ERROR_MAP,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_DISPLAY_PIC_BUFFER_COUNT
 *   Property Structure: vidc_display_pic_buffer_count_type
 *   Description       : The property can be used to set minimum number of
 *                       decoded output buffers needed before a frame can be
 *                       emitted in display order. This would override the
 *                       standard specification.
 *                       It's client's responsibility to set the maximum needed
 *                       buffer for the whole session. In case it is less than
 *                       the actual number of buffers needed, behavior is
 *                       undefined
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Standard Specified decode picture buffer count
 *   </pre>
 */
   VIDC_I_DEC_DISPLAY_PIC_BUFFER_COUNT,   /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_OUTPUT2_KEEP_ASPECT_RATIO
 *   Property Structure: vidc_enable_type
 *   Description       : Client can enable preserving the aspect ratio
 *                       on scaled down image on output2 for decoder. Core
 *                       would maintain aspect ratio for scaling operations
 *                       The scenario is specially useful in
 *                       when I_CONT_RECONFIG is enabled and there is a change
 *                       in decoded resolution
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_DEC_OUTPUT2_KEEP_ASPECT_RATIO,  /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_MULTI_VIEW_SELECT
 *   Property Structure: vidc_multi_view_select_type
 *   Description       : To get/set view selection for multi view decoding.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : All views are returned by default
 *   </pre>
 */
   VIDC_I_DEC_MULTI_VIEW_SELECT,          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_THUMBNAIL_MODE
 *   Property Structure: vidc_enable_type
 *   Description       : Output buffer requirement for a decoder in this mode
                         is constraint to 1 It is expected that only I/IDR
                         frame would be submitted by the client in this mode
                         however a divergence shall be dealt with best effort
                         decoding.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_DEC_THUMBNAIL_MODE,             /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_FRAME_ASSEMBLY
 *   Property Structure: vidc_enable_type
 *   Description       : To enable/disable frame assembly for video decoding.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : Disabled by default
 *   </pre>
 */
   VIDC_I_DEC_FRAME_ASSEMBLY,             /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_SCS_THRESHOLD
 *   Property Structure: vidc_scs_threshold_type
 *   Description       : To set threshold value for start code search. This
                         property is used when frame assembly is enabled and
                         host sets firmware to limit the search window during
                         start code search. The value is specified in bytes and
                         is defined to be the accumulated slice size from the
                         last good start code (or valid slice header) boundary
                         and considering that no additional start code (or
                         valid slice header) was found.
 *   SetProperty for   : Decoder
 *   GetProperty for   : Decoder
 *   Reconfigurable    : Only between vidc_initialize() and LOAD_RESOURCES
 *   Initial Default value : 0
 *   </pre>
 */
   VIDC_I_DEC_SCS_THRESHOLD,              /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_DEC_OUTPUT_CROP
 *   Property Structure: vidc_dec_output_crop_type
 *   Description       : Gives decoder output crop values, including crop left offset,
 *                       crop top offset cropped width and cropped height.
 *   SetProperty for   : n/a
 *   GetProperty for   : Decoder
 *   Reconfigurable    : After vidc_initialize() anytime during the session
 *   Initial Default value : Disabled.
 *   </pre>
 */
    VIDC_I_DEC_OUTPUT_CROP,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/****************************************************************************/
/*
 * The following properties are applicable for VPE (Video pre/post processing
 * engine. An encoder session might use VPE internally for various operations.
 * In those cases these properties will also be accepted on encoding session
 */
   VIDC_I_VPE_START_BASE   = 0x300,       /**< Unused Property ID */
/**  <pre>
 *   Property id       : VIDC_I_VPE_DEINTERLACER
 *   Property Structure: vidc_enable_type
 *   Description       : Enable/Disable de-interlacing
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : After vidc_initialize() anytime during the session
 *   Initial Default value : Disabled
 *   </pre>
 */
   VIDC_I_VPE_DEINTERLACER,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_VPE_SPATIAL_TRANSFORM
 *   Property Structure: vidc_spatial_transform_type
 *   Description       : Specify rotation of flip operations in raw domain
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : After vidc_initialize() anytime during the session
 *   Initial Default value : VIDC_ROTATE_NONE and VIDC_FLIP_NONE
 *   </pre>
 */
   VIDC_I_VPE_SPATIAL_TRANSFORM,          /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_VPE_CAPABILITY_SCALE
 *   Property Structure: vidc_capability_scale_type
 *   Description       : Min/Max supported scale factor values.
 *   SetProperty for   : n/a
 *   GetProperty for   : Encoder
 *   Reconfigurable    : Can be queried after vidc_initialize() any time in
 *                       the session.
 *   Initial Default value : n/a
 *  </pre>
 **/
   VIDC_I_VPE_CAPABILITY_SCALE,           /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_VPE_CSC
 *   Property Structure: vidc_vpe_csc_type
 *   Description       : Set H264/H265 encoding to configure VPE's color space conversion
 *                       in the encoded stream.
 *   SetProperty for   : encoder
 *   GetProperty for   : encoder
 *   Reconfigurable    : Only between vidc_initialize and LOAD_RESOURCES
 *   Default value     : Disabled
 *   </pre>
 */
   VIDC_I_VPE_CSC,                       /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/

/**  <pre>
 *   Property id       : VIDC_I_VPE_BLUR_FILTER
 *   Property Structure: vidc_blur_filter_type
 *   Description       : Specify blur mode or blur resolution
 *   SetProperty for   : Encoder
 *   GetProperty for   : Encoder
 *   Reconfigurable    : After vidc_initialize() anytime during the session
 *   Initial Default value : Disabled. No blurring. Dynamic Blur config will be ignored
 *   </pre>
 */
    VIDC_I_VPE_BLUR_FILTER,               /* <<=== ==PROPERTY== ==== */
/*..........................................................................*/
/****************************************************************************/

   VIDC_I_RESERVED_START_BASE  = 0x2000,    /**< Unused Property ID */
   VIDC_PROPERTY_ID_UNUSED     = 0x10000000
} vidc_property_id_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_property_hdr_type
----------------------------------------------------------------------------*/
/**
 * Property header type which carries the property ID value and size of
 * of associated property structure.
 */
typedef struct
{
   vidc_property_id_type   prop_id;  /**< Property ID from macro */
   uint32                  size;     /**< Size of the property structure */
} vidc_property_hdr_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_priority_level_type
----------------------------------------------------------------------------*/
/** Enumeration lists the supported session priority types. */
typedef enum
{
   VIDC_PRIORITY_LOW    = 10,    /**< Low priority */
   VIDC_PRIORITY_MEDIUM = 20,    /**< Medium priority */
   VIDC_PRIORITY_HIGH   = 30,    /**< High priority */

   VIDC_PRIOIRTY_UNUSED = 0x10000000
} vidc_priority_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_priority_type
----------------------------------------------------------------------------*/
/**
 * Property to set whether the encoding / decoding session
 * is realtime or not
 * [property id: VIDC_I_PRIORITY]
 */
typedef struct
{
   vidc_priority_level_type    priority;    /**< session priority value. */
} vidc_priority_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_session_type
----------------------------------------------------------------------------*/
/** The session type, describing the domain of operation. */
typedef enum
{
   VIDC_SESSION_ENCODE = 0x00000001,      /**< Encode session. */
   VIDC_SESSION_DECODE = 0x00000002,      /**< Decode session. */
   VIDC_SESSION_VPE    = 0x00000004,      /**< VPE stand-alone session */
   VIDC_SESSION_MBI    = 0x00000008,      /**< Special session for motion vector dump */
   VIDC_SESSION_UNUSED = 0x10000000
} vidc_session_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_codec_type
----------------------------------------------------------------------------*/
/** Codec selection for Encoding or Decoding */
typedef enum
{
   VIDC_CODEC_H264         = 0x00000002,    /**< H.264 Codec  */
   VIDC_CODEC_H263         = 0x00000004,    /**< H.263 Codec  */
   VIDC_CODEC_MPEG1        = 0x00000008,    /**< MPEG1 Codec */
   VIDC_CODEC_MPEG2        = 0x00000010,    /**< MPEG2 Codec */
   VIDC_CODEC_MPEG4        = 0x00000020,    /**< MPEG4 Codec  */
   VIDC_CODEC_DIVX_311     = 0x00000040,    /**< DivX 3.11 Codec */
   VIDC_CODEC_DIVX         = 0x00000080,    /**< DivX 4.xx Codec */
   VIDC_CODEC_VC1          = 0x00000100,    /**< VC-1 Codec */
   VIDC_CODEC_SPARK        = 0x00000200,    /**< Sorenson Spark */
   VIDC_CODEC_VP8          = 0x00001000,    /**< On2 VP8 */
   VIDC_CODEC_HEVC         = 0x00002000,    /**< HEVC */
   VIDC_CODEC_VP9          = 0x00004000,    /**< VP9 */
   VIDC_CODEC_TME          = 0x00008000,    /**< TME */
   VIDC_CODEC_HEIC         = 0x00020000,    /**< HEIC */
   VIDC_CODEC_HEVC_HYBRID  = 0x80000000,    /**< HEVC Hybrid mode */

   VIDC_CODEC_UNUSED = 0x10000000
} vidc_codec_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_session_codec_type
----------------------------------------------------------------------------*/
/** This type allows get/set for session & codec type
 * [property id: VIDC_I_SESSION_CODEC]
 */
typedef struct
{
   vidc_session_type session;  /**< video operation domain */
   vidc_codec_type   codec;    /**< video codec type. */
} vidc_session_codec_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_divx_format_type
----------------------------------------------------------------------------*/
/** Divx format type */
typedef enum
{
   VIDC_CODEC_DIVX_4      = 0x00000001,     /**< Divx 4 */
   VIDC_CODEC_DIVX_5      = 0x00000002,     /**< Divx 5 */
   VIDC_CODEC_DIVX_6      = 0x00000003,     /**< Divx 6 */

   VIDC_CODEC_DIVX_UNUSED = 0x10000000
} vidc_divx_format_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_divx_format_config_type
----------------------------------------------------------------------------*/
/** Structure to get/set divx sub-codec type (format) type.
 * [property id:VIDC_I_DIVX_FORMAT]
 */
typedef struct
{
   vidc_divx_format_type   divx_format;
} vidc_divx_format_config_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_h263_profile_type
----------------------------------------------------------------------------*/
/** H.263 Profile enumerations */
typedef enum
{
   /** Baseline Profile */
   VIDC_PROFILE_H263_BASELINE  =   0x00000001,

   VIDC_PROFILE_H263_UNUSED    = 0x10000000
} vidc_h263_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mpeg2_profile_type
----------------------------------------------------------------------------*/
/** MPEG-2 Profile enumerations */
typedef enum
{
   VIDC_PROFILE_MPEG2_SIMPLE  =  0x00000001,  /**< Simple Profile */
   VIDC_PROFILE_MPEG2_MAIN    =  0x00000002,  /**< Main Profile */
   VIDC_PROFILE_MPEG2_422     =  0x00000004,  /**< 422 Profile */
   VIDC_PROFILE_MPEG2_SNR     =  0x00000008,  /**< SNR Profile */
   VIDC_PROFILE_MPEG2_SPATIAL =  0x00000010,  /**< Spatial Profile */
   VIDC_PROFILE_MPEG2_HIGH    =  0x00000020,  /**< High Profile */

   VIDC_PROFILE_MPEG2_UNUSED = 0x10000000
} vidc_mpeg2_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mpeg4_profile_type
----------------------------------------------------------------------------*/
/** MPEG-4 Profile enumerations */
typedef enum
{
   /** Simple Profile */
   VIDC_PROFILE_MPEG4_SP     = 0x00000001,

   /** Advanced Simple Profile */
   VIDC_PROFILE_MPEG4_ASP    = 0x00000002,

   VIDC_PROFILE_MPEG4_UNUSED = 0x10000000
} vidc_mpeg4_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_h264_profile_type
----------------------------------------------------------------------------*/
/** H264 Profile enumerations */
typedef enum
{
   VIDC_PROFILE_H264_BASELINE    = 0x00000001,  /**< Baseline profile */
   VIDC_PROFILE_H264_MAIN        = 0x00000002,  /**< Main profile */
   VIDC_PROFILE_H264_HIGH        = 0x00000004,  /**< High profile */
   VIDC_PROFILE_H264_STEREO_HIGH = 0x00000008,  /**< Stereo high profile */
   VIDC_PROFILE_H264_MV_HIGH     = 0x00000010,  /**< Multiview high profile */
   VIDC_PROFILE_H264_CONSTRAINED_BASE = 0x00000020,  /**< Constrained Base
                                                      *   profile */
   VIDC_PROFILE_H264_CONSTRAINED_HIGH = 0x00000040,  /**< Constrained High
                                                      *   profile */

   VIDC_PROFILE_H264_UNUSED      = 0x10000000
} vidc_h264_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vpx_profile_type
----------------------------------------------------------------------------*/
/** VPx Profile enumerations */
typedef enum
{
   VIDC_PROFILE_VPX_VERSION_0  = 0x00000001,  /**< VP7/VP8 version 0 */
   VIDC_PROFILE_VPX_VERSION_1  = 0x00000002,  /**< VP7/VP8 version 1 */
   VIDC_PROFILE_VPX_VERSION_2  = 0x00000004,  /**< VP8 version 2 */
   VIDC_PROFILE_VPX_VERSION_3  = 0x00000008,  /**< VP8 version 3 */

   VIDC_PROFILE_VPX_UNUSED = 0x10000000
} vidc_vpx_profile_type;
/*==========================================================================*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vp9_profile_type
----------------------------------------------------------------------------*/
/** VP9 Profile enumerations */
typedef enum
{
   VIDC_PROFILE_VP9_0      = 0x00000001,       /**< VP9: profile 0 */
   VIDC_PROFILE_VP9_2_10B  = 0x00000004,       /**< VP9 Profile 2, 10-bit*/
   VIDC_PROFILE_VP9_UNUSED = 0x10000000
} vidc_vp9_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_hevc_tier_profile_type
----------------------------------------------------------------------------*/
/** HEVC TIER Profile enumerations */
typedef enum
{
   VIDC_PROFILE_HEVC_TIER_MAIN        = 0x00000001,       /**< HEVC TIER: profile main */
   VIDC_PROFILE_HEVC_TIER_HIGH        = 0x00000002,       /**< HEVC TIER: profile high*/
   VIDC_PROFILE_HEVC_TIER_UNUSED      = 0x10000000
} vidc_hevc_tier_profile_type;
/*==========================================================================*/

/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vc1_profile_type
----------------------------------------------------------------------------*/
/** VC-1 Profile type */
typedef enum
{
   VIDC_PROFILE_VC1_SIMPLE    = 0x00000001,  /**< VC1 Simple profile */
   VIDC_PROFILE_VC1_MAIN      = 0x00000002,  /**< VC1 Main profile */
   VIDC_PROFILE_VC1_ADVANCE   = 0x00000004,  /**< VC1 Advanced profile */

   VIDC_PROFILE_VC1_UNUSED = 0x10000000
} vidc_vc1_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_divx_profile_type
----------------------------------------------------------------------------*/
/** Divx Profile type */
typedef enum
{
   VIDC_PROFILE_DIVX_QMOBILE  = 0x00000001,  /**< q_mobile profile */
   VIDC_PROFILE_DIVX_MOBILE   = 0x00000002,  /**< Mobile profile */
   VIDC_PROFILE_DIVX_MT       = 0x00000004,  /**< Mobile theater profile */
   VIDC_PROFILE_DIVX_HT       = 0x00000008,  /**< Home theater profile */
   VIDC_PROFILE_DIVX_HD       = 0x00000010,  /**< High definition profile */

   VIDC_PROFILE_DIVX_UNUSED = 0x10000000
} vidc_divx_profile_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_hevc_profile_type
----------------------------------------------------------------------------*/
/** HEVC Profile type */
typedef enum
{
   VIDC_PROFILE_HEVC_MAIN       = 0x00000001,  /**< HEVC Main profile */
   VIDC_PROFILE_HEVC_MAIN10     = 0x00000002,  /**< HEVC Main 10 profile */
   VIDC_PROFILE_HEVC_MAIN_STILL_PICTURE = 0x00000004, /**< HEVC Main still picture */
   VIDC_PROFILE_HEVC_UNUSED     = 0x10000000
} vidc_hevc_profile_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_profile_type
----------------------------------------------------------------------------*/
/** Data type to set Profile
 * [property id: VIDC_I_PROFILE]
 */
typedef struct
{
   /** Codec profile payload. Enumenration used shall be one of codec profile
       types (e.g. vidc_h264_profile_type etc.) */
   uint32       profile;
} vidc_profile_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_h263_level_type
----------------------------------------------------------------------------*/
/** H.263 Level enumerations */
typedef enum
{
   VIDC_LEVEL_H263_10   =  0x00000001,  /**< H.263 Level 10 */
   VIDC_LEVEL_H263_20   =  0x00000002,  /**< H.263 Level 20 */
   VIDC_LEVEL_H263_30   =  0x00000004,  /**< H.263 Level 30 */
   VIDC_LEVEL_H263_40   =  0x00000008,  /**< H.263 Level 40 */
   VIDC_LEVEL_H263_45   =  0x00000010,  /**< H.263 Level 45 */
   VIDC_LEVEL_H263_50   =  0x00000020,  /**< H.263 Level 50 */
   VIDC_LEVEL_H263_60   =  0x00000040,  /**< H.263 Level 60 */
   VIDC_LEVEL_H263_70   =  0x00000080,  /**< H.263 Level 70 */

   VIDC_LEVEL_H263_UNUSED = 0x10000000
} vidc_h263_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mpeg2_level_type
----------------------------------------------------------------------------*/
/** MPEG-2 Level enumerations */
typedef enum
{
   VIDC_LEVEL_MPEG2_LOW      = 0x00000001,  /**< MPEG-2 Low Level */
   VIDC_LEVEL_MPEG2_MAIN     = 0x00000002,  /**< MPEG-2 Main Level */
   VIDC_LEVEL_MPEG2_HIGH_14  = 0x00000004,  /**< MPEG-2 High 1440 Level */
   VIDC_LEVEL_MPEG2_HIGH     = 0x00000008,  /**< MPEG-2 High Level */

   VIDC_LEVEL_MPEG2_UNUSED = 0x10000000
} vidc_mpeg2_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mpeg4_level_type
----------------------------------------------------------------------------*/
/** MPEG-4 Level enumerations */
typedef enum
{
   VIDC_LEVEL_MPEG4_0   =  0x00000001,  /**< MPEG-4 Level 0 */
   VIDC_LEVEL_MPEG4_0b  =  0x00000002,  /**< MPEG-4 Level 0b */
   VIDC_LEVEL_MPEG4_1   =  0x00000004,  /**< MPEG-4 Level 1 */
   VIDC_LEVEL_MPEG4_2   =  0x00000008,  /**< MPEG-4 Level 2 */
   VIDC_LEVEL_MPEG4_3   =  0x00000010,  /**< MPEG-4 Level 3 */
   VIDC_LEVEL_MPEG4_4   =  0x00000020,  /**< MPEG-4 Level 4 */
   VIDC_LEVEL_MPEG4_4a  =  0x00000040,  /**< MPEG-4 Level 4a */
   VIDC_LEVEL_MPEG4_5   =  0x00000080,  /**< MPEG-4 Level 5 */
   VIDC_LEVEL_MPEG4_6   =  0x00000100,  /**< MPEG-4 Level 6 */
   VIDC_LEVEL_MPEG4_7   =  0x00000200,  /**< MPEG-4 Level 7 */
   VIDC_LEVEL_MPEG4_8   =  0x00000400,  /**< MPEG-4 Level 8 */
   VIDC_LEVEL_MPEG4_9   =  0x00000800,  /**< MPEG-4 Level 9 */
   VIDC_LEVEL_MPEG4_3b  =  0x00001000,  /**< MPEG-4 Level 3b */

   VIDC_LEVEL_MPEG4_UNUSED = 0x10000000
} vidc_mpeg4_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_h264_level_type
----------------------------------------------------------------------------*/
/** H264 Level type */
typedef enum
{
   VIDC_LEVEL_H264_1     =  0x00000001,  /**< H.264 Level 1 */
   VIDC_LEVEL_H264_1b    =  0x00000002,  /**< H.264 Level 1b */
   VIDC_LEVEL_H264_1p1   =  0x00000004,  /**< H.264 Level 1.1 */
   VIDC_LEVEL_H264_1p2   =  0x00000008,  /**< H.264 Level 1.2 */
   VIDC_LEVEL_H264_1p3   =  0x00000010,  /**< H.264 Level 1.3 */
   VIDC_LEVEL_H264_2     =  0x00000020,  /**< H.264 Level 2 */
   VIDC_LEVEL_H264_2p1   =  0x00000040,  /**< H.264 Level 2.1 */
   VIDC_LEVEL_H264_2p2   =  0x00000080,  /**< H.264 Level 2.2 */
   VIDC_LEVEL_H264_3     =  0x00000100,  /**< H.264 Level 3 */
   VIDC_LEVEL_H264_3p1   =  0x00000200,  /**< H.264 Level 3.1 */
   VIDC_LEVEL_H264_3p2   =  0x00000400,  /**< H.264 Level 3.2 */
   VIDC_LEVEL_H264_4     =  0x00000800,  /**< H.264 Level 4 */
   VIDC_LEVEL_H264_4p1   =  0x00001000,  /**< H.264 Level 4.1 */
   VIDC_LEVEL_H264_4p2   =  0x00002000,  /**< H.264 Level 4.2 */
   VIDC_LEVEL_H264_5     =  0x00004000,  /**< H.264 Level 5 */
   VIDC_LEVEL_H264_5p1   =  0x00008000,  /**< H.264 Level 5.1 */
   VIDC_LEVEL_H264_5p2   =  0x00010000,  /**< H.264 Level 5.2 */
   VIDC_LEVEL_H264_6     =  0x00020000,  /**< H.264 Level 6 */
   VIDC_LEVEL_H264_6p1   =  0x00040000,  /**< H.264 Level 6.1 */
   VIDC_LEVEL_H264_6p2   =  0x00080000,  /**< H.264 Level 6.2 */

   VIDC_LEVEL_H264_UNUSED = 0x10000000
} vidc_h264_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vc1_level_type
----------------------------------------------------------------------------*/
/** VC-1 Level type */
typedef enum
{
   VIDC_LEVEL_VC1_S_LOW     = 0x00000001, /**< Simple profile: Level Low */
   VIDC_LEVEL_VC1_S_MEDIUM  = 0x00000002, /**< Simple profile: Level Medium*/
   VIDC_LEVEL_VC1_M_LOW     = 0x00000001, /**< Main profile: Level Low */
   VIDC_LEVEL_VC1_M_MEDIUM  = 0x00000002, /**< Main profile: Level Medium */
   VIDC_LEVEL_VC1_M_HIGH    = 0x00000004, /**< Main profile: Level High */
   VIDC_LEVEL_VC1_A_0       = 0x00000008, /**< Advanced profile: Level 0 */
   VIDC_LEVEL_VC1_A_1       = 0x00000010, /**< Advanced profile: Level 1 */
   VIDC_LEVEL_VC1_A_2       = 0x00000020, /**< Advanced profile: Level 2 */
   VIDC_LEVEL_VC1_A_3       = 0x00000040, /**< Advanced profile: Level 3 */
   VIDC_LEVEL_VC1_A_4       = 0x00000080, /**< Advanced profile: Level 4 */

   VIDC_LEVEL_VC1_UNUSED    = 0x10000000
} vidc_vc1_level_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_hevc_level_type
----------------------------------------------------------------------------*/
/** HEVC Level type */
typedef enum
{
  VIDC_LEVEL_HEVC_1                   = 0x00000001, /**< Level 1 */
  VIDC_LEVEL_HEVC_2                   = 0x00000002, /**< Level 2 */
  VIDC_LEVEL_HEVC_21                  = 0x00000004, /**< Level 2.1 */
  VIDC_LEVEL_HEVC_3                   = 0x00000008, /**< Level 3 */
  VIDC_LEVEL_HEVC_31                  = 0x00000010, /**< Level 3.1 */
  VIDC_LEVEL_HEVC_4                   = 0x00000020, /**< Level 4 */
  VIDC_LEVEL_HEVC_41                  = 0x00000040, /**< Level 4.1 */
  VIDC_LEVEL_HEVC_5                   = 0x00000080, /**< Level 5 */
  VIDC_LEVEL_HEVC_51                  = 0x00000100, /**< Level 5.1 */
  VIDC_LEVEL_HEVC_52                  = 0x00000200, /**< Level 5.2 */
  VIDC_LEVEL_HEVC_6                   = 0x00000400, /**< Level 6 */
  VIDC_LEVEL_HEVC_61                  = 0x00000800, /**< Level 6.1 */
  VIDC_LEVEL_HEVC_62                  = 0x00001000, /**< Level 6.2 */

  VIDC_LEVEL_HEVC_UNUSED              = 0x10000000
} vidc_hevc_level_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_level_type
----------------------------------------------------------------------------*/
/** Data type to set level
 *  [property id: VIDC_I_LEVEL]
 */
typedef struct
{
   /** Codec level payload. Enumenration used shall be one of codec level
    *  types (e.g. vidc_h264_level_type etc.) */
   uint32   level;
} vidc_level_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_tier_type
----------------------------------------------------------------------------*/
/** Data type to set/get tier
 *  [property id: VIDC_I_TIER]
 */
typedef struct
{
   /** Codec tier payload. Enumenration used shall be of HEVC tier
    *  types (e.g. vidc_hevc_tier_profile_type etc.) */
   uint32   tier;
} vidc_tier_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_buffer_type
----------------------------------------------------------------------------*/
/** This data type lists the supported buffer types. */
typedef enum
{
   /** Input frame buffer */
   VIDC_BUFFER_INPUT            = 0x00000001,

   /** Output frame buffer for primary stream */
   VIDC_BUFFER_OUTPUT           = 0x00000002,

   /** Output frame buffer for secondary stream */
   VIDC_BUFFER_OUTPUT2          = 0x00000003,

   /** Metadata buffer associated with VIDC_BUFFER_INPUT */
   VIDC_BUFFER_METADATA_INPUT   = 0x01000002,

   /** Metadata buffer associated with VIDC_BUFFER_OUTPUT */
   VIDC_BUFFER_METADATA_OUTPUT  = 0x01000003,

   /** Metadata buffer associated with VIDC_BUFFER_OUTPUT2 */
   VIDC_BUFFER_METADATA_OUTPUT2 = 0x01000004,

   VIDC_BUFFER_UNUSED           = 0x10000000
} vidc_buffer_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_buffer_mode_type
----------------------------------------------------------------------------*/
/** This data type lists the supported buffer allocation mode types. */
typedef enum
{
   /** Buffers are statically allocated before VIDC_EVT_RESP_LOAD_RESOURCES
       is returned. */
   VIDC_BUFFER_MODE_STATIC  = 0x01000001,

   /** A single memory region is allocated and set, out of which all buffers
       of the specified type must reside. Memory region start address is set
       before VIDC_EVT_RESP_LOAD_RESOURCES is returned while the individual
       buffer addresses within the region are dynamically passed via
       empty and fill buffer calls.*/
   VIDC_BUFFER_MODE_RING    = 0x01000002,

   /** Buffers are dynamically allocated and sent vidc FTB
       The buffers can be dynamically freed and allocated during the
       life of the session. The size of all the buffers for a given
       buffer type should remain the same even if they are
       dynamically allocated, unless there's a sequence change
       leading to change in buffer requirements. This mode is only
       supported for VIDC_BUFFER_OUTPUT and VIDC_BUFFER_OUTPUT2. */
   VIDC_BUFFER_MODE_DYNAMIC    = 0x01000003,

   VIDC_BUFFER_MODE_UNUSED  = 0x10000000
} vidc_buffer_mode_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_flush_mode_type
----------------------------------------------------------------------------*/
/**
 * This type defines different flush modes.
 */
typedef enum
{
   VIDC_FLUSH_INPUT   = 0x01000001, /**< Flush input buffers only */
   VIDC_FLUSH_OUTPUT  = 0x01000002, /**< Flush output buffers only */
   VIDC_FLUSH_OUTPUT2 = 0x01000003, /**< Flush output2 buffers only */
   VIDC_FLUSH_ALL     = 0x01000004, /**< Flush all above enabled buffers
                                     *   types */

   VIDC_FLUSH_UNUSED  = 0x10000000
} vidc_flush_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_flush_mode_req_type
----------------------------------------------------------------------------*/
/**
 * This type defines different flush modes requested.
 */
typedef enum
{
   VIDC_FLUSH_REQ_INPUT   = 0x01, /**< Flush input buffers only */
   VIDC_FLUSH_REQ_OUTPUT  = 0x02, /**< Flush output buffers only */
   VIDC_FLUSH_REQ_OUTPUT2 = 0x04, /**< Flush output2 buffers only */
   VIDC_FLUSH_REQ_ALL     = VIDC_FLUSH_REQ_INPUT |
                            VIDC_FLUSH_REQ_OUTPUT |
                            VIDC_FLUSH_REQ_OUTPUT2, /**< Flush all above enabled buffers
                                                      *   types */

   VIDC_FLUSH_REQ_UNUSED  = 0x10000000
} vidc_flush_mode_req_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_type
----------------------------------------------------------------------------*/
/** This data type types list the different frame types. */
typedef enum
{
   /**< Bitstream I frame. One of the possible frame types for decoder input
        frames. Also one of the types for Encoder output frames */
   VIDC_FRAME_I          =   0x1,

   /**< Bitstream B frame. One of the possible frame types for decoder input
        frames. Also one of the types for Encoder output frames */
   VIDC_FRAME_P          =   0x2,

   /**< Bitstream P frame. One of the possible frame types for decoder input
        frames. Also one of the types for Encoder output frames */
   VIDC_FRAME_B          =   0x4,

   /**< Bitstream IDR frame. One of the possible frame types for decoder input
        frames. Also one of the types for Encoder output frames */
   VIDC_FRAME_IDR        =   0x08,

   /**< Frame is not coded. Set on decoder or encoder output frames to indicate
        that frame was not coded. */
   VIDC_FRAME_NOTCODED   =   0x7F002000,

   /**< Raw YUV frame. Expected frame type for Encoder input frames. Also used
        for Decoder output frames. */
   VIDC_FRAME_YUV        =   0x7F004000,

   VIDC_FRAME_UNUSED = 0x10000000
} vidc_frame_type;
/*==========================================================================*/

/**
 * This data type captures bit depth information
 */
typedef struct
{
   vidc_buffer_type  buffer_type;
   uint32            bit_depth;
} vidc_bit_depth_type;

/** BIT DEPTH type */
enum
{
   VIDC_BITDEPTTH_8BIT     = 0,
   VIDC_BITDEPTTH_9BIT     = 1,
   VIDC_BITDEPTTH_10BIT    = 2
};

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_size_type
----------------------------------------------------------------------------*/
/**
 * Frame Resolution: SizeX or width and SizeY or height
 * [property id: VIDC_I_FRAME_SIZE]
 */
typedef struct
{
   vidc_buffer_type  buf_type;  /**< [in] Buffer type */
   uint32            width;     /**< Horizontal frame Size */
   uint32            height;    /**< Vertical frame Size */
} vidc_frame_size_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_plane_def_type
----------------------------------------------------------------------------*/
/**
 * Frame strides and multiple
 * [property id: VIDC_I_PLANE_DEF]
 * Applies to Encoder input and decoder output frame
 */
typedef struct
{
   /**< [in] Buffer type */
   vidc_buffer_type  buf_type;

   /**< [in] Plane index to which property applies. Counting starts from 1.
        An index more than allowed for the selected buffer format will be
        returned with VIDC_ERR_INDEX_NOMORE */
   uint32            plane_index;

   /**< Minimum allowed stride for the plane. */
   uint32            min_stride;

   /**< Maximum allowed stride for the plane. */
   uint32            max_stride;

   /**< Stride multiple allowed in horizontal direction. */
   uint32            stride_multiples;

   /**< Height multiple (number of stride lines ) actual_plane_buf_height
        and min_plane_buf_height shall be a multiple of this parameter */
   uint32            min_plane_buf_height_multiple;

   /**< Buffer alignment requirement for this plane. */
   uint32            buf_alignment;

   /**< Minimum plane height supported */
   uint32            min_plane_buf_height;

   /**< Actual stride for the plane. This should adhere to restrictions put
        by min_stride, max_stride and stride_multiples. This would be used
        while calculating buffer requirement */
   int32             actual_stride;

   /**< Actual plane height for the plane This should adhere to restrictions
        put by min_plane_buf_height. This would be used while calculating
        buffer requirement */
   uint32            actual_plane_buf_height;

} vidc_plane_def_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_rate_type
----------------------------------------------------------------------------*/
/** Frame rate for Encoder or Decoder
 *  frame rate = fps_numerator / fps_denominator. (e.g. for fps
 *  7.5, fps_numerator and fps_denominator can be 15 and 2)
 *  [property id: VIDC_I_FRAME_RATE]
 */
typedef struct
{
   vidc_buffer_type    buf_type;         /**< [in] Buffer type */
   uint32              fps_denominator;  /**< Denominator of frame rate */
   uint32              fps_numerator;    /**< Numerator of frame rate */
} vidc_frame_rate_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_quality_type
----------------------------------------------------------------------------*/
/**
 * Property to set target frame quality in bits/second. This is mandatory
 * property to set while start encoding session in CQ mode.
 * [property id: VIDC_I_ENC_FRAME_QUALITY]
 */
typedef struct
{
   uint32             frame_quality;  /**< frame quality (bits/second) */
} vidc_frame_quality_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_target_bitrate_type
----------------------------------------------------------------------------*/
/**
 * Property to set target bitrate in bits/second. This is mandatory
 * property to set while start encoding session.
 * [property id: VIDC_I_TARGET_BITRATE]
 */
typedef struct
{
   uint32             target_bitrate;  /**< target bitrate (bits/second) */
} vidc_target_bitrate_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_buffer_alloc_mode_type
----------------------------------------------------------------------------*/
/**
 * Property to set buffer allocation mode type. Allocated buffers can be either
 * static or ring format. Default mode is a static allocation.
 * [property id: VIDC_I_BUFFER_ALLOC_MODE]
 */
typedef struct
{
   vidc_buffer_type             buf_type;  /**< [in] Buffer type */
   vidc_buffer_mode_type        buf_mode;  /**< [in] Buffer mode Static/Ring
                                                [out] For get property */
} vidc_buffer_alloc_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mvc_buf_layout_type
----------------------------------------------------------------------------*/
/**
 * This type defines to select buffer layaout for MVC.
 */
typedef enum
{
   /**< Views are stack one after another in memory. Luma and chroma
   planes are stacked for the first view and then the two planes
   are stacked for the second view (for NV12/NV21 format) */
   VIDC_MVC_BUFFER_LAYOUT_TOP_BOTTOM   = 0x00000000,

   /**< Both the views are row-wise interleave. There could be a gap
   arrising from alignment requirements on each view */
   VIDC_MVC_BUFFER_LAYOUT_SIDEBYSIDE   = 0x00000001,

   /**< In this mode views views are in separate sequential buffers
   E.g. buffer(N) could contain view 1 and buffer(N+1) would contain
   the next view. */
   VIDC_MVC_BUFFER_LAYOUT_SEQ          = 0x00000002
} vidc_mvc_buf_layout_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mvc_buffer_layout_descp_type
----------------------------------------------------------------------------*/
/**
 * Property to set on FW to generay MVC views in a specified format.
 * Default mode is a VIDC_MVC_BUFFER_LAYOUT_TOP_BOTTOM.
 * [property id: VIDC_I_MVC_BUFFER_LAYOUT]
 */
typedef struct
{
   vidc_mvc_buf_layout_type     layout_type;      /**< [in] Type to select
                                                   * buffer layout for each
                                                   * view */
   uint32                       right_view_first; /**< [in ]TRUE only if right
                                                   * view is first in the
                                                   * buffer layout */
   uint32                       gap;              /**< [in] gap between 2
                                                   * views */
} vidc_mvc_buffer_layout_descp_type;
/*==========================================================================*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_color_format_type
----------------------------------------------------------------------------*/
/** <pre>
 * Decoder Output Buffer formats:
 *
 * VIDC_COLOR_FORMAT_MONOCHROME: Y plane Only. Valid only for decoder
 *
 * VIDC_COLOR_FORMAT_NV12: YUV420, with Y plane first, followed by Chroma
 *                         plane. Chrome plane contains U and V pixels
 *                         interleaved (U first)
 *                     __________________________________________________
 * Y Start Address ->  |    |    |    |    | Memory Address in byte      |
 *                     | Y0 | Y1 | Y2 | Y3 | Y - components (scanline    |
 *                     |____|____|____|____|_order)______________________|
 *                     |____.____.____.________.____.____.________.______|
 *                     .____.____.____.________.____.____.________.______|
 *Chroma Start Addr -> |    |    |    |    |Memory Address in byte - U/  |
 *                     | U0 | V0 | U1 | V1 | V  components               |
 *                     |____|____|____|____|____.____.____.________._____|
 *                     |____.____.____.________.____.____.________.______|
 *                     .____.____.____.________.____.____.________.______|
 *
 * VIDC_COLOR_FORMAT_NV21: YUV420, with Y plane first, followed by Chroma
 *                         plane. Chrome plane contains U and V pixels
 *                         interleaved (V first)
 *                     __________________________________________________
 * Y Start Address ->  |    |    |    |    | Memory Address in byte      |
 *                     | Y0 | Y1 | Y2 | Y3 | Y - components (scanline    |
 *                     |____|____|____|____|_order)______________________|
 *                     |____.____.____.________.____.____.________.______|
 *                     .____.____.____.________.____.____.________.______|
 *Chroma Start Addr -> |    |    |    |    |Memory Address in byte - U/  |
 *                     | V0 | U0 | V1 | U1 | V  components               |
 *                     |____|____|____|____|____.____.____.________._____|
 *                     |____.____.____.________.____.____.________.______|
 *                     .____.____.____.________.____.____.________.______|
 *
 * VIDC_COLOR_FORMAT_NV12_4x4TILE: YUV420 in tile format. Y plane followed
 *                                 by chroma plane. Y plane contains 4x4 pels
 *                                 arranged linearly as follows:
 *
 * Let's say that we have a YUV420 image with width 128 the arrangement in
 * tile format would be:
 *
 * |Y0|Y1|Y2|Y3|Y128|Y129|Y130|Y131|Y256|Y257|Y258|Y259|Y384|Y385|Y386|Y387|
 * |Y4|Y5|Y6|Y7|Y132|Y133|Y134|Y135|Y260|Y261|Y262|Y263|Y388|Y389|Y390|Y391|
 * |Y8|Y9|Y10|Y11|Y136|Y137|Y138|Y139|Y264|Y265|Y266|Y267|Y392|Y393|Y394|Y395|
 * |Y12| ....
 *
 * |U0|V1|U1|V1|U64|V64|U65|V65|
 * |U2|V2|U3|V3|U66|V66|U67|V67|
 * |U4|V4|U5|V5|U68|V68|U69|V69|
 * |U6| ....
 *
 * VIDC_COLOR_FORMAT_NV21_4x4TILE: Same as VIDC_COLOR_FORMAT_NV12_4x4TILE
 *                                 with chroma plan starting with V data
 *                                 instead of U
 *
 * VIDC_COLOR_FORMAT_YUYV: YUV422 format. Y, U anv V pixels are arranged in a
 *                         single plane as pixel interleaved data:
 *
 * |Y0|U0|Y1|V0|Y2|U1|Y3|V1|...
 *
 * VIDC_COLOR_FORMAT_YVYU: YUV422 format. Y, U anv V pixels are arranged in a
 *                         single plane as pixel interleaved data:
 *
 * |Y0|V0|Y1|U0|Y2|V1|Y3|U1|...
 *
 * VIDC_COLOR_FORMAT_UYVY: YUV422 format. Y, U anv V pixels are arranged in a
 *                         single plane as pixel interleaved data:
 *
 * |U0|Y0|V0|Y1|U1|Y2|V1|Y3|...
 *
 * VIDC_COLOR_FORMAT_VYUY: YUV422 format. Y, U anv V pixels are arranged in a
 *                         single plane as pixel interleaved data:
 *
 * |V0|Y0|U0|Y1|V1|Y2|U1|Y3|...
 *
 * VIDC_COLOR_FORMAT_RGB565: RGB color format in which 16 bits are used to
 *                           represent a pixel. Bits and color distribution
 *                           are as follows:
 *
 * <-- Higher byte --> <-- Lower byte -->
 * |R|R|R|R|R|G|G|G|   |G|G|G|B|B|B|B|B|
 *
 * VIDC_COLOR_FORMAT_BGR565: RGB color format in which 16 bits are used to
 *                           represent a pixel. Bits and color distribution
 *                           are as follows:
 *
 * <-- Higher byte --> <-- Lower byte -->
 * |B|B|B|B|B|G|G|G|   |G|G|G|R|R|R|R|R|
 *
 * VIDC_COLOR_FORMAT_RGB888: 24 bit RGB color format. Each color is represneted
 *                           as one 1 byte in the mentioned order (R, G, B)
 *
 * VIDC_COLOR_FORMAT_BGR888: 24 bit RGB color format. Each color is represneted
 *                           as one 1 byte in the mentioned order (B, G, R)
 *
 * </pre>
 */
typedef enum
{
   VIDC_COLOR_FORMAT_MONOCHROME    = 0x00000001,
   VIDC_COLOR_FORMAT_NV12          = 0x00000002,
   VIDC_COLOR_FORMAT_NV21          = 0x00000003,
   VIDC_COLOR_FORMAT_NV12_4x4TILE  = 0x00000004,
   VIDC_COLOR_FORMAT_NV21_4x4TILE  = 0x00000005,
   VIDC_COLOR_FORMAT_YUYV          = 0x00000006,
   VIDC_COLOR_FORMAT_YVYU          = 0x00000007,
   VIDC_COLOR_FORMAT_UYVY          = 0x00000008,
   VIDC_COLOR_FORMAT_VYUY          = 0x00000009,
   VIDC_COLOR_FORMAT_RGB565        = 0x0000000A,
   VIDC_COLOR_FORMAT_BGR565        = 0x0000000B,
   VIDC_COLOR_FORMAT_RGB888        = 0x0000000C,
   VIDC_COLOR_FORMAT_BGR888        = 0x0000000D,
   VIDC_COLOR_FORMAT_YUV444        = 0x0000000E,
   VIDC_COLOR_FORMAT_YV12          = 0x0000000F,
   VIDC_COLOR_FORMAT_RGBA8888      = 0x00000010,
   VIDC_COLOR_FORMAT_YUV420_TP10   = (VIDC_COLOR_FORMAT_10_BIT_BASE + VIDC_COLOR_FORMAT_NV12),
   VIDC_COLOR_FORMAT_NV12_P010     = (VIDC_COLOR_FORMAT_10_BIT_BASE + VIDC_COLOR_FORMAT_NV12 + 0x1),
   VIDC_COLOR_FORMAT_NV12_UBWC     = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_NV12),
   VIDC_COLOR_FORMAT_YUYV_UBWC     = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_YUYV),
   VIDC_COLOR_FORMAT_YUV444_UBWC   = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_YUV444),
   VIDC_COLOR_FORMAT_YUV420_TP10_UBWC   = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_YUV420_TP10),
   VIDC_COLOR_FORMAT_RGB565_UBWC   = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_RGB565),
   VIDC_COLOR_FORMAT_RGBA8888_UBWC = (VIDC_COLOR_FORMAT_UBWC_BASE + VIDC_COLOR_FORMAT_RGBA8888),
   VIDC_COLOR_FORMAT_UNUSED        = 0x10000000
} vidc_color_format_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_color_format_config_type
----------------------------------------------------------------------------*/
/**
 * Configures the uncompressed Buffer format. This need to be set
 * before Initialization.
 * [property id: VIDC_I_COLOR_FORMAT]
 */
typedef struct
{
   /**< [in] Buffer type for which the buffer format is being get/set */
   vidc_buffer_type              buf_type;

   /**< Buffer format */
   vidc_color_format_type        color_format;

} vidc_color_format_config_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_multi_slice_mode_type
----------------------------------------------------------------------------*/
/**
 * Multi slice mode selection type.
 * Each mode is based on various parameters (m_bs, bytes etc.)
 */
typedef enum
{
   VIDC_MULTI_SLICE_OFF           = 0x00000001,   /**< No slices */
   VIDC_MULTI_SLICE_BY_MB_COUNT   = 0x00000002,   /**< Based on number of
                                                   *   m_bs */
   VIDC_MULTI_SLICE_BY_BYTE_COUNT = 0x00000003,   /**< Based on number of
                                                   *   bytes */
   VIDC_MULTI_SLICE_BY_GOB        = 0x00000004,   /**< Multi slice by GOB for
                                                   *   H.263 only*/

   VIDC_MULTI_SLICE_UNUSED = 0x10000000
} vidc_multi_slice_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_multi_slice_type
----------------------------------------------------------------------------*/
/**
 * Data type to enable/disable and configure multislice (slice based encoding)
 * settings.
 * [property id: VIDC_I_ENC_MULTI_SLICE]
 */
typedef struct
{
   /** To disable Multi-slice, it should be MULTI_SLICE_OFF; otherwise this
       signifies one of the Multi-slice selection type. Slice by GOB is only
       supported in H.263 codec */
   vidc_multi_slice_mode_type   slice_mode;

   /** If slice_mode is MULTI_SLICE_OFF or MULTI_SLICE_BY_GOB, then this field
       is ignored. Otherwise, this is size of slice; if method is MB count,
       then this is No of MB per Slice if method is byte count, then this is
       No of bytes per slice */
   uint32                       slice_size;
} vidc_multi_slice_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_entropy_mode_type
----------------------------------------------------------------------------*/
/** Entropy coding model selection for H.264 encoder. */
typedef enum
{
   /** Context-based Adaptive Binary Arithmetic Coding (CAVLC)  */
   VIDC_ENTROPY_MODE_CAVLC = 0x00000001,

   /** Context-based Adaptive Variable Length Coding (CABAC) */
   VIDC_ENTROPY_MODE_CABAC = 0x00000002,

   VIDC_ENTROPY_UNUSED     = 0x10000000
} vidc_entropy_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_cabac_model_type
----------------------------------------------------------------------------*/
/** Cabac model number (0,1,2) for encoder. */
typedef enum
{
   VIDC_CABAC_MODEL_NUMBER_0 = 0x00000001,    /**< Cabac context model 0 */
   VIDC_CABAC_MODEL_NUMBER_1 = 0x00000002,    /**< Cabac context model 1 */
   VIDC_CABAC_MODEL_NUMBER_2 = 0x00000003,    /**< Cabac context model 2 */

   VIDC_CABAC_MODEL_UNUSED   = 0x10000000
} vidc_cabac_model_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_entropy_control_type
----------------------------------------------------------------------------*/
/**
 * Entropy control settings for encoder.
 * [property id: VIDC_I_ENC_H264_ENTROPY_CTRL]
 */
typedef struct
{
   /** Set Entropy coding mode selection */
   vidc_entropy_mode_type  entropy_mode;

   /** Valid only when Entropy Selection is CABAC. This is model
       number for CABAC */
   vidc_cabac_model_type   cabac_model;
} vidc_entropy_control_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_db_mode_type
----------------------------------------------------------------------------*/
/** Deblocking filter control type for encoder. */
typedef enum
{
   /**< Disable deblocking filter */
   VIDC_DB_DISABLE               = 0x00000001,

   /**< Filtering except for slice boundaries */
   VIDC_DB_SKIP_SLICE_BOUNDARY   = 0x00000002,

   /**< Filtering for all edges, including internal edge and slice boundaries */
   VIDC_DB_ALL_BLOCKING_BOUNDARY = 0x00000003,

   VIDC_DB_MAX                   = 0x00000004,
   VIDC_DB_UNUSED                = 0x10000000
} vidc_db_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_db_control_type
----------------------------------------------------------------------------*/
/**
 *  Property to set the Deblocking Filter Control Params for encoder
 *  [property id: VIDC_I_ENC_H264_DEBLOCKING]
 */
typedef struct
{
   /** Deblocking Control */
   vidc_db_mode_type      db_mode;

   /** Alpha offset value in Slice Header (slice_alpha_c0_offset_div2 ).
      This value’s range is -6 to 6 */
   int32                  slice_alpha_offset;

   /** Beta offset value in Slice Header (slice_beta_offset_div2).
      This value’s range is -6 to 6 */
   int32                  slice_beta_offset;
} vidc_db_control_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_rate_control_mode_type
----------------------------------------------------------------------------*/
/** Encoder Rate Control type. */
typedef enum
{
   /** Rate Control is OFF. It is not recommended as it will not
       obey bitrate requirement */
   VIDC_RATE_CONTROL_OFF      = 0x01000001,

   /** Variable bit rate & Variable frame rate; Final bit rate should be within
       10 per cent of target bitrate. Mainly used for higher quality
       expectation appliation (e.g. camcorder) */
   VIDC_RATE_CONTROL_VBR_VFR  = 0x01000002,

   /** Variable bit rate & Constant frame rate - No frame drop, best
       effort Rate Control */
   VIDC_RATE_CONTROL_VBR_CFR  = 0x01000003,

   /** Constant bit rate & Variable frame rate; mainly used for strict rate
      control scenario application (e.g. video telephony). Allow frame drop but
      maintain constant bit rate. */
   VIDC_RATE_CONTROL_CBR_VFR  = 0x01000004,

   /** Constant bit rate and constant frame rate; camcorder application with
      Constant bitrate and constant frame rate */
   VIDC_RATE_CONTROL_CBR_CFR  = 0x01000005,

   /** Maximum bitrate, constant frame rate; use for low bitrate use-case.
       Achieved bitrate will be less than the maximum bitrate specified. */
   VIDC_RATE_CONTROL_MBR_CFR  = 0x01000006,

   /** Maximum bitate, variable frame rate */
   VIDC_RATE_CONTROL_MBR_VFR  = 0x01000007,

   /** Constant Quality mode. Applicable only for all I frames HEVC / HEIC encoding */
   VIDC_RATE_CONTROL_CQ       = 0x01000008,

   VIDC_RATE_CONTROL_MAX      = 0x01000009,
   VIDC_RATE_CONTROL_UNUSED   = 0x10000000
} vidc_rate_control_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_rate_control_type
----------------------------------------------------------------------------*/
/** Data type to configure the Rate Control for encoder.
 * [property id: VIDC_I_ENC_RATE_CONTROL]
 */
typedef struct
{
   vidc_rate_control_mode_type     rate_control;  /**< _type of RC */
} vidc_rate_control_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_session_qp_type
----------------------------------------------------------------------------*/
/**
 * Sets the intial QP Value for encoder;
 * [property id: VIDC_I_ENC_SESSION_QP]
 */
typedef struct
{
   uint32         i_frame_qp;    /**< QP value for Intra Frame */
   uint32         p_frame_qp;    /**< QP value for Inter Frame */
   uint32         b_frame_qp;    /**< QP value for B Frame */
} vidc_session_qp_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_session_qp_range_type
----------------------------------------------------------------------------*/
/**
 * Sets the QP Value Range for encoder;
 * [property id: VIDC_I_ENC_SESSION_QP_RANGE]
 */
typedef struct
{
   uint32         qp_min_packed;    /**< Minimum Quantization value */
                                    /**< 32 bit packed as:
                                       0-7 : Min QP for I/IDR,
                                       8-15: Min QP for P,
                                      15-23: Min QP for B */
   uint32         qp_max_packed;    /**< Maximum Quantization value */
                                    /**< 32 bit packed as:
                                        0-7: Max QP for I/IDR,
                                       8-15: Max QP for P,
                                      15-23: Max QP for B */
   uint32         layerID;   /**< Multi-view / Hier-P layer ID */
} vidc_session_qp_range_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_iperiod_type
----------------------------------------------------------------------------*/
/** <pre>
 * Data type to set I frame period pattern for encoder.
 * [property id: VIDC_I_ENC_INTRA_PERIOD]
 *
 * Video core puts a limit between number of P and B pictures, which is
 * number of B pictures between P pictures = ( b_frames / (p_frames + 1)).
 * Adhering to this limit b_frames is modified accordingly.
 * For example, if client configures:
 * (p_frames, b_frames) = (3, 3) -> (3, 4)
 * The encoded squence will look like
 * IBPBPBPBI... (in capture order) (p_frames = 3, b_frames = 4)
 * IPBPBPBIB... (in encode order)
 * (p_frames, b_frames) = (1, 3) -> (1, 2)
 * IBPBI (capture order)
 * IPBIB (encode order)
 * No error is thrown
 * </pre>
 */
typedef struct
{
   uint32            p_frames;   /**< No of P-Pictures between 2 I-Frames */
   uint32            b_frames;   /**< No of B-Pictures between 2 I-Frames */
} vidc_iperiod_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vop_timing_type
----------------------------------------------------------------------------*/
/**
 * Settings for VOP time increment and resolution for MPEG4 encoder.
 * VOP time increment will be derived from Frame rate as follows:
 * VOP time increment = (vop_time_resolution * fps_denominator )
 *                       / (fps_numerator)
 * [property id: VIDC_I_ENC_MPEG4_VOP_TIMING]
 */
typedef struct
{
   uint32   vop_time_resolution; /**< VOP Time resolution value */
} vidc_vop_timing_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_intra_refresh_mode_type
----------------------------------------------------------------------------*/
/** Intra refresh (IR) modes. */
typedef enum
{
   VIDC_INTRA_REFRESH_NONE            = 0x00000001, /**< No IR */
   VIDC_INTRA_REFRESH_CYCLIC          = 0x00000002, /**< Cyclic */
   VIDC_INTRA_REFRESH_ADAPTIVE        = 0x00000003, /**< Adaptive */
   VIDC_INTRA_REFRESH_CYCLIC_ADAPTIVE = 0x00000004, /**< Cyclic and adaptive */
   VIDC_INTRA_REFRESH_RANDOM          = 0x00000005, /**< Random */

   VIDC_INTRA_REFRESH_UNUSED           = 0x10000000
} vidc_intra_refresh_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_intra_refresh_type
----------------------------------------------------------------------------*/
/**
 * Structure to set intra refresh type and number of m_bs to refresh
 * [property id: VIDC_I_ENC_INTRA_REFRESH]
 */
typedef struct
{
   /** Intra refresh mode */
   vidc_intra_refresh_mode_type ir_mode;

   /** Minimum number of m_bs to be refreshed when mode is ADAPTIVE or RANDOM*/
   uint32            air_mb_count;

   /** The number of times a motion marked macroblock has to be intra-coded */
   uint32            air_ref_count;

   /** Number of consecutive m_bs to be coded as Intra when mode is CYCLIC */
   uint32            cir_mb_count;
} vidc_intra_refresh_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_plane_config_type
----------------------------------------------------------------------------*/
/**
 * Plane descriptor in a frame. Can be used to describe a plane (Y or U or V
 * or UV) in a frame (in uncompressed domain)
 */
typedef struct
{
   /** X co-ordinate of upper left corner of the plane from where the
       displayable data starts */
   uint32   left;

   /** Y co-ordinate of upper left corner of the plane from where the
       displayable data starts */
   uint32   top;

   /** Displayable plane width from co-ordinates (left) */
   uint32   width;

   /** Displayable plane height from co-ordinates (top) */
   uint32   height;

   /** Horizontal stride in bytes. stride >= width */
   uint32   stride;

   /** Vertical stride or number of pixel rows after which another plane
       can start scan_lines >= height */
   uint32   scan_lines;
} vidc_frame_plane_config_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_uncompressed_frame_config_type
----------------------------------------------------------------------------*/
/**
 * Data type to describe raw domain (uncompressed) YUV frame size during
 * the session
 *
 * For decoder, it describes the display frame rectangle within the decoded
 * buffer. Chroma info is not being populated currently
 */
typedef struct
{
   vidc_frame_plane_config_type luma_plane;   /**< luma plane descp */
   vidc_frame_plane_config_type chroma_plane; /**< chroma plane descp*/
} vidc_uncompressed_frame_config_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_codec_type
----------------------------------------------------------------------------*/
/**
 * This data type can be used by the client to enumerate the list of codecs
 * that are supported by the core.
 * The total number of types supported (N) is not reported separately but the
 * client can determine N by enumerating all the types. When index >= N,
 * vidc_get_property will return VIDC_ERR_INDEX_NOMORE to indicate no more types.
 * [property id: VIDC_I_CAPABILITY_CODEC]
 */
typedef struct
{
   /** [in] Enumeration index value. Ranges from 0 to N-1, where N is the total
       number of types supported. */
   uint32               index;

   /** [in] Session type for which codec capabilities are being queried */
   vidc_session_type    session;

   /** [out] Codec type corresponding to index */
   vidc_codec_type      codec;
} vidc_capability_codec_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_color_format_type
----------------------------------------------------------------------------*/
/**
 * This data type can be used by the client to enumerate the list of buffer
 * format types (color formats) that are supported by the core.
 * The total number of types supported (N) is not reported separately but the
 * client can determine N by enumerating all the types. When index >= N,
 * vidc_get_property will return VIDC_ERR_INDEX_NOMORE to indicate no more types.
 * [property id: VIDC_I_CAPABILITY_COLOR_FORMAT]
 */
typedef struct
{
   /** [in] Enumeration index value. Ranges from 0 to N-1, where N is the
       total number of types supported. */
   uint32                  index;

   /** [out] Buffer type to which the format is applicable */
   vidc_buffer_type        buf_type;

   /** [out] Buffer format type corresponding to index. _types will be returned
       in the order of recommendation with the formats at lower values of
       index having higher recommendation from VIDC. */
   vidc_color_format_type  color_format;
} vidc_capability_color_format_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_profile_level_type
----------------------------------------------------------------------------*/
/**
 * This data type can be used by the client to enumerate the list of profile
 * types that are supported by the core for currently set codec type.
 * All levels won't be enumerated and just the highest supported level for the
 * enumerated profile would be returned.
 * The total number of types supported (N) is not reported separately but the
 * client can determine N by enumerating all the types. When index >= N,
 * vidc_get_property will return VIDC_ERR_INDEX_NOMORE to indicate no more types.
 * [property id: VIDC_I_CAPABILITY_PROFILE_LEVEL]
 */
typedef struct
{
   /** [in] Enumeration index value. Ranges from 0 to N-1, where N is the
       total number of types supported. */
   uint32            index;

   /** [out] Profile type */
   uint32            profile;

   /** [out] Level type. This will be the highest supported level for the
       returned profile. */
   uint32            level;
} vidc_capability_profile_level_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_range_type
----------------------------------------------------------------------------*/
/**
 * A generic structure that is used by properties to query or
 * set a range of UNSIGNED values for a particular parameter.
 */
typedef struct
{
   /** Lower bound or minimum value of the range */
   uint32   min;

   /** Upper bound or maximum value of the range */
   uint32   max;

   /** Gives the step up value that can be used to derive the values within
       the range min to max using increments of step_size or conversely it
       can be used as step down value to derive values starting from max
       using decrements of step_size */
   uint32   step_size;
} vidc_range_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_frame_size_type
----------------------------------------------------------------------------*/
/**
 * Gives the supported range for frame width and frame height.
 * [property id: VIDC_I_CAPABILITY_FRAME_SIZE]
 */
typedef struct
{
   vidc_range_type   width;    /**< [out] gives the width range */
   vidc_range_type   height;   /**< [out] gives the height range */
} vidc_capability_frame_size_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_mb_type
----------------------------------------------------------------------------*/
/**
 * Gives the supported macroblocks per frame and macroblocks
 * per second.
 * [property id: VIDC_I_CAPABILITY_MAX_MACROBLOCKS]
 */
typedef struct
{
   uint32      mb_per_frame; /**< [out] Number of macroblocks per frame */
   uint32      mb_per_sec;   /**< [out] Number of macroblocks per second */
} vidc_capability_mb_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_output_order_mode_type
----------------------------------------------------------------------------*/
/** Output order for decoder.*/
typedef enum
{
   VIDC_DEC_ORDER_DISPLAY = 0x01000001, /**< Decoder outputs in display order*/
   VIDC_DEC_ORDER_DECODE  = 0x01000002, /**< Decoder outputs in decode order */

   VIDC_DEC_ORDER_UNUSED   = 0x10000000
} vidc_output_order_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_output_order_type
----------------------------------------------------------------------------*/
/**
 * Configures a particular output order for decoding
 * [property id: VIDC_I_DEC_OUTPUT_ORDER]
 */
typedef struct
{
   vidc_output_order_mode_type  output_order;   /**< output order value */
} vidc_output_order_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_dec_pic_type
----------------------------------------------------------------------------*/
/**
 * Configures decoder to allow various bitstream frame types
 * [property id: VIDC_I_DEC_PICTYPE]
 */
typedef struct
{
   /** Specify the picutre type(s). vidc_frame_type values should be OR'd to
       signal all pictures types which are allowed. */
   uint32 dec_pic_types;
} vidc_dec_pic_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_fraction_type
----------------------------------------------------------------------------*/
/**
 * Property to set a fractional value.
 * [property id: VIDC_I_ENC_TIMESTAMP_SCALE]
 */
typedef struct
{
   uint32             denominator; /**< Denominator of fraction */
   uint32             numerator;   /**< Numerator of farction */
} vidc_fraction_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_seq_hdr_size_type
----------------------------------------------------------------------------*/
/**
 * Structure to query the maximum size for encoded sequence heder
 * [property id: VIDC_I_ENC_MAX_SEQUENCE_HEADER_SIZE]
 */
typedef struct
{
   /** Maximum expected size of sequence header in Bytes */
   uint32         max_seq_hdr_size;
} vidc_seq_hdr_size_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_session_name_type
----------------------------------------------------------------------------*/
/** This type allows get/set for session name
 * [property id: VIDC_I_SESSION_NAME]
 */
typedef struct
{
   uint32    nBufSz;                               /**< buffer size in bytes */
   char      session_name[ VIDC_MAX_STRING_SIZE ]; /**< client session name. */
} vidc_session_name_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_yuv_scan_format_type
----------------------------------------------------------------------------*/
/** _type for different scan formats supported */
typedef enum
{
   /** All scan lines are captured progressively */
   VIDC_SCAN_PROGRESSIVE                       =  0x01,

   /** Alternate pixel lines in the frame belong to different time instance,
       with the earlier line being first in the interleave pattern */
   VIDC_SCAN_INTERLACE_INTERLEAVED_TOPFLDFIRST =  0x02,

   /** Alternate pixel lines in the frame belong to different time instance,
       with the later line being first in the interleave pattern */
   VIDC_SCAN_INTERLACE_INTERLEAVED_BOTFLDFIRST =  0x04,

   /** Frame consists of two half-planes captured at different time instance,
       with the plane captured earlier is put first in the frame */
   VIDC_SCAN_INTERLACE_TOPFIELDFIRST           =  0x08,

   /** Frame consists of two half-planes captured at different time instance,
       with the plane captured later is put first in the frame */
   VIDC_SCAN_INTERLACE_BOTFIELDFIRST           =  0x10,

   VIDC_SCAN_FORMAT_UNUSED = 0x10000000
} vidc_yuv_scan_format_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_scan_format_type
----------------------------------------------------------------------------*/
/**
 * Data type to get the supported scan format types.
 * [property id: VIDC_I_CAPABILITY_SCAN_FORMAT]
 */
typedef struct
{
   /** [in] Buffer type for which the scan format is being queried */
   vidc_buffer_type     buf_type;

   /** [out] Scan ( Interlace/Progressive) formats supported for the buffer
       type ORed in a bit-mask fashion. Please refer to
       vidc_yuv_scan_format_type for different scan formats */
   uint32               scan_format;
} vidc_capability_scan_format_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_nal_stream_fromat_type
----------------------------------------------------------------------------*/
/** _type for supported nal stream formats */
typedef enum
{
   /** H.264, Annex-B format */
   VIDC_NAL_FORMAT_STARTCODES         =  0x01,

   /** Maximum number of na_ls per frame is restricted to one.
       The nal may not complete an AU */
   VIDC_NAL_FORMAT_ONE_NAL_PER_BUFFER =  0x02,

   /** Nals in the buffer are separated by a 1 byte code, representing the
       size of the nal. Nal starts with this code */
   VIDC_NAL_FORMAT_ONE_BYTE_LENGTH    =  0x04,

   /** Nals in the buffer are separated by a 2 byte code, representing the size
       of the nal. Nal starts with this code */
   VIDC_NAL_FORMAT_TWO_BYTE_LENGTH    =  0x08,

   /** Nals in the buffer are separated by a 4 byte code, representing the size
       of the nal. Nal starts with this code */
   VIDC_NAL_FORMAT_FOUR_BYTE_LENGTH   =  0x10,

   VIDC_NAL_FORMAT_UNUSED = 0x10000000

} vidc_nal_stream_fromat_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_stream_format_type
----------------------------------------------------------------------------*/
/** Data type to get/set nal stream format(s) supported by the core
 *  [property id: VIDC_I_CAPABILITY_NAL_STREAM_FORMAT]
 *  [property id: VIDC_I_NAL_STREAM_FORMAT]
 */
typedef struct
{
   /** Nal stream format
       VIDC_I_CAPABILITY_NAL_STREAM_FORMAT - For capability query, all the
         supported formats are ORed as bit-mask
         (refer to vidc_nal_stream_fromat_type )

       VIDC_I_NAL_STREAM_FORMAT - For get/set of stream format only one of
         the supported formats is allowed
         (refer to vidc_nal_stream_fromat_type ) */
   uint32         nal_format;
} vidc_stream_format_type;

/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_multi_stream_type
----------------------------------------------------------------------------*/
/**
 * Enables/Disables stream type for a session.
 * [property id: VIDC_I_DEC_MULTI_STREAM]
 */
typedef struct
{
   /** [in] Stream (buffer type) which needs to enabled or disabled
       VIDC_BUFFER_OUTPUT and VIDC_BUFFER_OUTPUT2 are the only allowed
       variations */
   vidc_buffer_type        buf_type;

   /** TRUE: If need to enable this stream. FALSE: otherwise */
   boolean                 enable;

} vidc_multi_stream_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_enable_type
----------------------------------------------------------------------------*/
/** Enables/disables a particular feature/tool
 *  Associated properties are:
 *  [property id: VIDC_I_DEC_MB_ERROR_MAP_REPORTING]
 *  [property id: VIDC_I_DEC_OUTPUT2_KEEP_ASPECT_RATIO]
 *  [property id: VIDC_I_LIVE]
 *  [property id: VIDC_I_ENC_MPEG4_SHORT_HEADER]
 *  [property id: VIDC_I_ENC_MPEG4_ACPRED]
 *  [property id: VIDC_I_ENC_REQUEST_SYNC_FRAME]
 *  [property id: VIDC_I_DEC_CONT_ON_RECONFIG]
 *  [property id: VIDC_I_ENC_SLICE_DELIVERY_MODE]
 *  [property id: VIDC_I_ENC_GEN_AUDNAL]
 */
typedef struct
{
   /** TRUE: To enable or ON, FALSE: To disable or OFF */
   boolean        enable;
} vidc_enable_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_display_pic_buffer_count_type
----------------------------------------------------------------------------*/
/**
 * Structure to set display picture buffer count
 * [property id: VIDC_I_DEC_DISPLAY_PIC_BUFFER_COUNT]
 */
typedef struct
{
   boolean     enable;    /**< Enable/Disable the feature */
   uint32      count;     /**< Number of display picture buffer count */
} vidc_display_pic_buffer_count_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_temporal_spatial_tradeoff_type
----------------------------------------------------------------------------*/
/**
 * Structure to set temporal-spatial tradeoff factor
 * [property id: VIDC_I_ENC_TEMPORAL_SPATIAL_TRADEOFF]
 */
typedef struct
{
   /** Temporal-Spatial tradeoff factor in the range 0-100 (inclusive).
       A lower value means that more weightage is given to picture quality
       (spatial) over frame rate (temporal) in Rate control.
       0: No consideration for frame-rate, 100: Only frame rate is important */
   uint32   ts_factor;
} vidc_temporal_spatial_tradeoff_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   video format for vidc_vui_video_signal_info_type
----------------------------------------------------------------------------*/
/** Type gives the video format values */
typedef enum
{
   VIDC_VIDEO_FORMAT_COMPONENT   = 0,  /**< Component */
   VIDC_VIDEO_FORMAT_PAL         = 1,  /**< PAL */
   VIDC_VIDEO_FORMAT_NTSC        = 2,  /**< NTSC */
   VIDC_VIDEO_FORMAT_SECAM       = 3,  /**< SECAM */
   VIDC_VIDEO_FORMAT_MAC         = 4,  /**< MAC */
   VIDC_VIDEO_FORMAT_UNSPECIFIED = 5,  /**< unspecified video format */
   VIDC_VIDEO_FORMAT_MAX = 0x7fffffff
} vidc_video_format_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vui_video_signal_info_type
----------------------------------------------------------------------------*/
/**
 * Structure to set the vui video signal info parameter
 * [property id: VIDC_I_ENC_VUI_VIDEO_SIGNAL_INFO]
 */
typedef struct
{
   /**< set to FALSE (0):No video signal metadata generated.
        Set to TRUE (1) :Encode video signal metadata information with below mentioned values. */
   boolean     enable;

   /**< Same as video_format from spec;
        If unknown, Host should set: 5 for Unspecified video format */
   vidc_video_format_type      video_format;

   /**< video_range for MPEG4; video_full_range_flag for H264/HEVC;
        Allowed values:
        0 : [16 - 235] for 8 bits
        1 : [0-255] for 8 bits
        If unknown, Host should set 0  */
   uint32      video_full_Range_flag;

   /**< MPEG4: colour_description flag; HEVC/H264: colour_description_present_flag.
        Allowed values: 0, 1;
        if 1, encode color descriptions with below mentioned values */
   boolean     color_description_flag;

   uint32      colour_primaries;         /**< Same as colour_primaries from spec */
   uint32      transfer_characteristics; /**< Same as transfer_characteristics from spec */
   uint32      matrix_coeffs;            /**< Same as matrix_coeffs from spec */

} vidc_vui_video_signal_info_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vpe_csc_type
----------------------------------------------------------------------------*/
/**
 * Structure to set the VPE CSC parameter
 * [property id: VIDC_I_VPE_CSC]
 */
typedef struct
{
   /**< Input YUV color space information; values are same as mention in video spec
        for colour_primaries. (Refer to structure: HFI_VIDEO_SIGNAL_METADATA &
        field name: colour_primaries.
        Supported primaries are: 601, 601_FR, 709, BT2020 */
   uint32 colour_primaries;

   /**< FW uses default matrix to convert from
        colour_primaries to 709 color space.
        Client could overwrite default matrices via enabling
        below bits:
        Bit 0: enable/disable csc_matrix[9]
        Bit 1: enable/disable csc_bias[3]
        Bit 2: enable/disable csc_limit[6] */
   uint32 custom_matrix_enabled;
   uint32 csc_matrix[VIDC_MAX_MATRIX_COEFFS];   /**< The color space conversion matrix */
   uint32 csc_bias[VIDC_MAX_BIAS_COEFFS];       /**< The color space conversion bias */
   uint32 csc_limit[VIDC_MAX_LIMIT_COEFFS];     /**< The color space conversion limit */
} vidc_vpe_csc_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_vui_timing_info_type
----------------------------------------------------------------------------*/
/**
 * Structure to set the vui timing info parameter
 * [property id: VIDC_I_ENC_VUI_TIMING_INFO]
 */
typedef struct
{
   /**< Set/Reset to enable/disable generation of VUI information in the
        encoded stream */
   boolean     enable;

   /**< Set/Reset to indicate whether the input frames arrive at fixed
        duration. TRUE: The frame-rate is fixed; FALSE: frame-rate may
        change on the fly */
   boolean     fixed_frame_rate;

   /**< Time units/intervals in a second. For example if a 27 MHz clock
        is assumed, there will be 27000000 units in a second */
   uint32      time_scale;

} vidc_vui_timing_info_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_idr_period_type
----------------------------------------------------------------------------*/
/**
 * Structure to set IDR period.
 * [property id: VIDC_I_ENC_IDR_PERIOD]
 */
typedef struct
{
   /** IDR frame periodicity within Intra coded frames. For example a value
       of 2 would mean that every second Intra frame is an IDR.
       0: Only first frame is IDR, 1: Every intra is an IDR too */
   uint32   idr_period;
} vidc_idr_period_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_bitrate_type
----------------------------------------------------------------------------*/
/**
 * Structure to set the vui timing info parameter
 * [property id: VIDC_I_ENC_MAX_BITRATE]
 */
typedef struct
{
   /**< The bitrate, in bytes per second */
   uint32     bitrate;

   /**<  Multi-view / Hier-P layer ID */
   uint32     layer_id;

} vidc_bitrate_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_rotation_type
----------------------------------------------------------------------------*/
/** Type defines the roation transformation values */
typedef enum
{
   VIDC_ROTATE_NONE   = 0x00000001,    /**< No rotation */
   VIDC_ROTATE_90     = 0x00000002,      /**< Rotate 90 degree clock wise */
   VIDC_ROTATE_180    = 0x00000003,     /**< Rotate 180 degree clock wise */
   VIDC_ROTATE_270    = 0x00000004,     /**< Rotate 270 degree clock wise */

   VIDC_ROTATE_MAX    = 0x00000005,
   VIDC_ROTATE_UNUSED = 0x10000000
} vidc_rotation_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_flip_type
----------------------------------------------------------------------------*/
/** Type gives the flip transformation values */
typedef enum
{
   VIDC_FLIP_NONE   = 0x00000001,                       /**< No flip */
   VIDC_FLIP_HORIZ  = 0x00000002,                       /**< Horizontal flip */
   VIDC_FLIP_VERT   = 0x00000004,                       /**< Vertical flip */
   VIDC_FLIP_BOTH   = VIDC_FLIP_HORIZ | VIDC_FLIP_VERT, /**< Horizontal and Vertical flip */

   VIDC_FLIP_MAX    = 0x00000008,
   VIDC_FLIP_UNUSED = 0x10000000
} vidc_flip_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_spatial_transform_type
----------------------------------------------------------------------------*/
/**
 * Data type to set the spatial transform configuration
 * [property id: VIDC_I_VPE_SPATIAL_TRANSFORM]
 */
typedef struct
{
   vidc_rotation_type  rotate; /**< Rotation operation */
   vidc_flip_type      flip;   /**< Flip operation */
} vidc_spatial_transform_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_capability_scale_type
----------------------------------------------------------------------------*/
/**
 * Gives the supported range for frame width and frame height.
 * [property id: VIDC_I_VPE_CAPABILITY_SCALE]
 */
typedef struct
{
   vidc_range_type  scale_x;  /**< scale factor range across width */
   vidc_range_type  scale_y;  /**< scale factor range across height */
} vidc_capability_scale_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_blur_filter_type
----------------------------------------------------------------------------*/
/**
 * Data type used to set blur mode or blur resolution.
 * VIDC_BLUR_NONE (0) both external and internal adaptive blur will be disabled.
 *    Must be set before start and blur will be disabled throughout the session
 * VIDC_BLUR_EXTERNAL 1 or valid blur resolution, external blur will be enabled and
 *    clients can set the blur dynamically after the start of a session
 *    valid resolution bit[31:16] for width, bit[15:0] for height. If valid resolution
 *    is set before the start, blur resolution will take effect from the start
 * VIDC_BLUR_ADAPTIVE (2) external blur from clients will be disabled.
 *    Internal adaptive blur will be enabled. need to be set before start
 * vidc_get_property will return blur mode with VIDC_I_VPE_BLUR_FILTER.
 * [property id: VIDC_I_VPE_BLUR_FILTER]
 */
typedef struct
{
   uint32  blur_value;   /**< blur value */
} vidc_blur_filter_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_dec_output_crop_type
----------------------------------------------------------------------------*/
/**
 * Data type used to get decoder output crop info
 * [property id: VIDC_I_DEC_OUTPUT_CROP]
 */
typedef struct
{
   uint32  crop_left;    /**< crop left offset */
   uint32  crop_top;     /**< crop top offset */
   uint32  crop_width;   /**< width after crop */
   uint32  crop_height;  /**< height after crop */
} vidc_dec_output_crop_type;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_multi_view_format_type
----------------------------------------------------------------------------*/
/**
 * Data type to allow stream configuration for multi view decoding or encoding.
 * [property id: VIDC_I_MULTI_VIEW_FORMAT]
 */
typedef struct
{
   /** Number of views for this session. */
   uint32            views;

   /** The view order, indicating the inter-view dependency. Each element of
      the array specifies a unique view number. The array includes view IDs
      from 0 to n-1, where n is defined to be the number of views. */
   uint32*           view_order;
} vidc_multi_view_format_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_multi_view_select_type
----------------------------------------------------------------------------*/
/**
 * Data type to allow view selection for multi view decoding.
 * [property id: VIDC_I_DEC_MULTI_VIEW_SELECT]
 */
typedef struct
{
   /** The view to be returned by the decoder, indicated by the
       view index. */
   uint32            view_index;
} vidc_multi_view_select_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_mb_error_map_type
----------------------------------------------------------------------------*/
/**
 * Structure to query MB error map from the core
 * [property id: VIDC_I_DEC_MB_ERROR_MAP]
 */
typedef struct
{
   /** Size allocated for error_map size allocated shall be at least so many
       number of "bits" as many number of macro_blocks decoded in the last
       frame */
   uint32      error_map_size;

   /** Pointer to memory where error map will be populated. Every byte shall
       be read from LSB onwards. (e.g. error_map[1]'s 0th bit would represent
       the error status for 8th MB in raster scan order (MB count starting
       from 0) */
   uint8*      error_map;
} vidc_mb_error_map_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_seq_hdr_type
----------------------------------------------------------------------------*/
/**
 * This data type carries the video sequence header for encoder/decoder.
 * [property id: VIDC_I_SEQUENCE_HEADER]
 */
typedef struct
{
   uint8*  seq_hdr;       /**< pointer to video sequence header */
   uint32  seq_hdr_len;   /**< length of video sequence header */
   uint32  pid;           /**< process id for seq_hdr virtual addr */
} vidc_seq_hdr_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_scs_threshold_type
----------------------------------------------------------------------------*/
/**
 * This data type used to set start code search window threshold value.
 * [property id: VIDC_I_DEC_SCS_THRESHOLD]
 */
typedef struct
{
   uint32  threshold_value;       /**< Value in bytes to limit start code
                                       search window */
} vidc_scs_threshold_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_ltr_mode
----------------------------------------------------------------------------*/
/** Type gives values to select LTR mode */
typedef enum
{
   VIDC_LTR_MODE_DISABLE      = 0x00000000,  /**< LTR encoding disabled */
   VIDC_LTR_MODE_MANUAL       = 0x00000001,  /**< Via Mark/Use LTR APIs */
   VIDC_LTR_MODE_PERIODIC     = 0x00000002,  /**< Generates LTR periodically */
   VIDC_LTR_MODE_MAX          = 0x7fffffff
} vidc_ltr_mode;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_ltr_mode_type
----------------------------------------------------------------------------*/
/**
 * This data type used to set LTR mode and LTR frame count.
 * [property id: VIDC_I_ENC_LTR_MODE]
 */
typedef struct
{
   vidc_ltr_mode  ltr_mode;      /**<  Select LTR mode */
   uint32         ltr_count;     /**<  Number of LTR frames */
   uint32         trust_mode;    /**<  Ignored. For future use */
} vidc_ltr_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_use_ltr_type
----------------------------------------------------------------------------*/
/**
 * This data type used to specify encoder to use LT reference frame to use
 * encode next frame to be encoded
 * [property id: VIDC_I_ENC_USELTRFRAME]
 */
typedef struct
{
   /*
      For VIDC_LTR_MODE_MANUAL mode:
         It is a bitmap consist of LongTermFramIdx. It indicates
         which LTR frame(s) are allowed for encoding the current frame.
         The least significant bit corresponds to LTR frame with
         LongTermFrameIdx equal to.
      For VIDC_LTR_MODE_PERIODIC mode:
         It specifies the identifier of the LTR frame to be used as reference
         frame for encoding subsequent frames
   */
   uint32   ref_ltr;

   /*
      Currently Ignored. For future use.
      Only valid for VIDC_LTR_MODE_MANUAL mode:
      False : no additional constraint
      True  : the encoder shall encode subsequent frames in encoding order
              subject to the following constraints:
          - It shall not use short-term reference frames in display/input
            order older than the current frame with the LTR command applied
            for future encoding in encoding order.
          - It shall not use LTR frames not described by the most recent
            USELTRFRAME.
          - It may use LTR frames updated after the current frame in encoding
            order
   */
   boolean  use_constraint;

   /*
      Only valid for VIDC_LTR_MODE_PERIODIC mode:
      specifies the number of subsequent frames to be encoded using the LTR
      frame with its identifier nRefLTR as reference frame.
      Short-term reference frames will be used thereafter.
      The value of 0xFFFFFFFF indicates that all subsequent frames will be
      encoded using this LTR frame as reference frame.
      Between IDR frame inserted and LTRUse cmd received, FW should use
      the IDR as encoding reference frame.
   */
   uint32   frames;
} vidc_ltr_use_type;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Buffer Requirement Attributes
----------------------------------------------------------------------------*/
/* The following buffer attributes are used to provide additional
 * buffer properties. Each attribute is represented by a bit.
 */
/** Enable/Disable caching of buffer memory. Bit set = Enable. */
#define VIDC_BUFFER_ATTR_CACHING     0x00000001
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_buffer_reqmnts_type
----------------------------------------------------------------------------*/
/**
 * This type is used to negotiate frame buffer requirements.
 * [property id: VIDC_I_BUFFER_REQUIREMENTS]
 */
typedef struct
{
   /** Buffer type these requirements apply to. */
   vidc_buffer_type     buf_type;

   /** The number of buffers that will be continuously held by the vid core.
       This is a read-only field. i.e its ignored in Set Property */
   uint32               hold_count;

   /** Minimum count of buffers. Larger or equal number of buffers has to
       be used. min_count >= hold_count. This is a read-only field.
       i.e its ignored in Set Property */
   uint32               min_count;

   /** Acutual count of buffers.
       [In a get_property]: If there was no set_property yet then it would
        return the recommended count of buffers that should be allocated or
        is allocated (if allocate_buffers call was made) for optimal performance.
        If there was set_property done then it would return the value last set.
       [In a set_property]: This should be actual number of buffers that are
        allocated (if external allocator is used) or are to be allocated
        (if VIDC is doing allocation). min_count <= actual_count <= max_count */
   uint32               actual_count;

   /** Maximum count of buffers. Should be >= min_count. This is a read-only
       field. i.e. its ignored in Set Property. */
   uint32               max_count;

   /** Size of each buffer in bytes including any metadata. i.e. this is the
       size of the raw or bitstream data region of the buffer plus any
       appended metadata. This is a read-only field. i.e its ignored in
       Set Property. */
   uint32               size;

   /** The required size of the buffer region, if single a memory region were
       to be allocated for this buffer type in the dynamic buffer mode of
       operation. */
   uint32               region_size;

   /** Alignment of buffer's PHYSICAL address in bytes. Buffer data region
       start address will be aligned on a multiple of this. Metadata start
       would always be 32-bit aligned irrespective of align value here.
       This is a read-only field. i.e its ignored in Set Property. */
   uint32               align;

   /** Unique pool identifier which can be set for buffer allocations. This can
       come from driver or come from some local policy component might have. */
   uint32               buf_pool_id;

   /** Additional bitwise attributes that apply to all buffers allocated with
       these buffer requirements. Refer to VIDC_BUFFER_ATTR_* constants above
       for bit assignments. */
   uint32               attributes;
} vidc_buffer_reqmnts_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_roi_mode
----------------------------------------------------------------------------*/
/** Enum values to select ROI mode*/
typedef enum
{
   VIDC_ROI_MODE_NONE  = 0x00000000,  /**< ROI mode none */
   VIDC_ROI_MODE_2BIT  = 0x00000001,  /**< ROI mode 2BIT type */
   VIDC_ROI_MODE_2BYTE = 0x00000002,  /**< ROI mode 2BYTE type */
   VIDC_ROI_MODE_MAX   = 0x7fffffff
} vidc_roi_mode;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_roi_mode_type
----------------------------------------------------------------------------*/
/**
 * Structure to get ROI type.
 * [property id: VIDC_I_ENC_ROI_MODE_TYPE]
 */
typedef struct
{
   vidc_roi_mode   roi_mode;
} vidc_roi_mode_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_timestamp_type
----------------------------------------------------------------------------*/
/** This data type represents the time stamp of video frame. */
typedef int64 vidc_timestamp_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Frame Flags: Used to provide additional input/output frame properties.
----------------------------------------------------------------------------*/

/** End of stream, used in input or output frame. */
#define VIDC_FRAME_FLAG_EOS               0x00000001

/** Buffer contains starting timestamp for the stream which corresponds to
    first data at startup or after seek operation. */
#define VIDC_FRAME_FLAG_STARTTIME         0x00000002

/** Buffer is or should be decoded but not rendered. */
#define VIDC_FRAME_FLAG_DECODEONLY        0x00000004

/** Data in associated input buffer is corrupt. Set on output buffer that
    contain corrupt data. */
#define VIDC_FRAME_FLAG_DATACORRUPT       0x00000008

/** Buffer contains a complete frame or final segment of a partial frame. */
#define VIDC_FRAME_FLAG_ENDOFFRAME        0x00000010

/** Frame is a coded synchronization IDR frame. */
#define VIDC_FRAME_FLAG_SYNCFRAME         0x00000020

/** Meta data (extra data) is present in the buffer. */
#define VIDC_FRAME_FLAG_METADATA          0x00000040

/** Buffer contains only sequence header. */
#define VIDC_FRAME_FLAG_CODECCONFIG       0x00000080

/** Buffer doesn't contain valid timestamp info. */
#define VIDC_FRAME_FLAG_TIMESTAMPINVALID  0x00000100

/** This flag is set to indicate that the buffer contents are read-only. The
    VIDC shall not alter the contents of the buffer, and VIDC shall set this
    flag on output buffers it does not want altered by the client. */
#define VIDC_FRAME_FLAG_READONLY          0x00000200

/** Set when last byte of buffer payload aligns with an end of independently
    decodable unit that is subset of a frame (sub frame). */
#define VIDC_FRAME_FLAG_ENDOFSUBFRAME     0x00000400

/** End of Sequence Nal Buffer Flag
 *
 *  The flag is generated by an H.264 deocder on output buffer types(s) when
 *  an EOS nal is detected in the elementary stream. The flag should be set for
 *  the last frame received before EOS nal in display order. EOS NAL should
 *  be present as the last NAL in the buffer submitted. Any data after EOS NAL
 *  shall be discarded.
 *  After EOS NAL, IDR frame should be submitted, otherwise FW will employ the
 *  best effort decoding (i.e. will use concealment)
 */
#define VIDC_FRAME_FLAG_EOSEQ             0x00200000

/** Drop Frame Flag
 *
 *  This flag is generated by firmware to indicate that the
 *  frame should be dropped within the decoder/encoder component
 *  if no other pass-through flags are set.
*/
#define VIDC_FRAME_FLAG_DROP_FRAME      0x20000000

/** Data Discontinuity Indicator Flag
 *
 *  This flag is set on decoder input buffers to indicate that the transport
 *  stream packet at the MPEG transport stream layer is in discontinuity state
 *  with respect to either the continuity counter or the program clock reference.
 *  Firmware will decode the input frame and will just propagate this flag to
 *  corresponding output buffer.
 */
#define VIDC_FRAME_FLAG_DISCONTINUITY     0x80000000

/** Transport Error Indicator Flag
 *
 *  This flag is set on decoder input buffer to indicate that the transport
 *  stream packet at the MPEG transport stream layer has uncorrectable errors.
 *  Firmware will decode the input frame and will just propagate this flag to
 *  corresponding output buffer.
 */
#define VIDC_FRAME_FLAG_TEI               0x40000000

/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_frame_data_type
----------------------------------------------------------------------------*/
/**
 * This data type is used for input frame, output frame, or just to pass an
 * empty frame buffer or signal end of stream.
 *
 * alloc_len is the size of the memory space pointed by virtual memory pointer
 * and is >= 'size' of the buffer requirement negotiated in
 * vidc_buffer_reqmnts_type.
 *
 * The media data starts from frame_addr + offset.
 *
 * Metadata: There are two modes in which metadata can be passed:
 * (A) via metadata_addr pointer:
 *
 * (B) via frame_addr pointer: In this mode metadata is added starting at the
 *     first 32-bit aligned address after the end of frame data (compressed or
 *     uncompressed)
 *     i.e metadata start = ((frame_addr + offset + data_len + 3) & ~3)
 */
typedef struct
{
   /** Virtual address in Kernel space of the frame buffer */
   uint8*            frame_addr;

   /** Virtual address in Kernel space for metadata associated with this
       frame buffer */
   uint8*            metadata_addr;

   /** Payload size of the buffer in bytes. [Required field]. */
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
   uint64           frm_clnt_data;

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
   unsigned long        mark_target;

   /** Mark data. The identifier will be propagated to associated output
       buffer(s) */
   unsigned long        mark_data;

   /** InputTag **/
   unsigned long        input_tag;

   /** InputTag2 **/
   unsigned long        input_tag2;

   /** OutputTag **/
   unsigned long        output_tag;

   /** allocated metadata size in the buffer   */
   uint32               alloc_metadata_len;

   /** Flag to represent whether the data and metadata buffers are
       contiguous or not */
   boolean              non_contiguous_metadata;

   /** Process id for the frame_addr and metadata_addr
    */
   uint32                pid;

   /**< PMEM handle of the frame buffer
    */
   uint8*                frame_handle;

   /**< PMEM handle for metadata associated with this frame buffer
    */
   uint8*                metadata_handle;

} vidc_frame_data_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_buffer_info_type
----------------------------------------------------------------------------*/
/**
 * This type is used when setting or allocating input/ouput
 * and corresponding extradata buffer address. Note that the
 * IO data buffer and extradata buffers are paired through
 * out the session
 */
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
   uint8*                  buf_addr;

   /**< Size of extradata buffer to be allocated in bytes.
    *   This could be 0 if there is no extradata-buffer required
    */
   uint32                  extradata_buf_size;

   /**< Extra data buffer address. This could be NULL if extradata_buf_size = 0
    *   or contiguous is TRUE
    */
   uint8*                  extradata_buf_addr;

   /**< Process id
    */
   uint32                  pid;

   /**< PMEM handle of the frame buffer
    */
   uint8*                  buf_handle;

   /**< PMEM handle for metadata associated with this frame buffer
    */
   uint8*                  extradata_buf_handle;

} vidc_buffer_info_type;

/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_metadata_type
----------------------------------------------------------------------------*/
/**
 * The following macros are used to indicate type of meta_data. Please refer
 * to VIDC_I_METADATA_HEADER for metadata usage.
 * Terms "Metadata" and "extra data" have been used interchangeably. In general
 * metadata layout is follows:
 *
 * size (4 bytes)         -- 20+data_size
 * version (4 bytes)      -- Set using VIDC_I_METADATA_HEADER
 * port_index (4 bytes)    -- Set using VIDC_I_METADATA_HEADER
 * type (4 bytes)         -- Set using VIDC_I_METADATA_HEADER
 * data_size (4 bytes)     -- Size in bytes of metadata
 * data (data_size bytes)  -- Metadata content (explained below)
 *
 * Metadata start address is calculated as follows:
 *   Start Address = (((uint8*)Buffer data start Address + data_len + 3) & ~3)
 */
typedef enum
{
/*..........................................................................*/
   /** <pre>
    * Extradata none is to indicate the end of metadata sequence. Extradata
    * none is always generated whenever one or more metadata is requested.
    * This can not be explicitly enabled or disabled by client
    *
    * Permitted on Sessions: Encoder/Decoder
    *
    * data_size: 0
    * data: NA
    *
    * </pre>
    */
   VIDC_METADATA_NONE               =   0x00000000,
/*..........................................................................*/

   /** <pre>
    * Qp array gives out information macroblock quantization data.
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: Variable (Number of m_bs in the decoded frame)
    * data: Quantization param used for quantization of each MB in the frame
    *       Qp[0]Qp[1]...Qp[i]..Qp[num_mbs - 1],
    *       where "i" is the MB index in raster scan order. Each Qp[i] is 1
    *       byte
    * </pre>
    */
   VIDC_METADATA_QPARRAY            =   0x00000001,
/*..........................................................................*/

   /** <pre>
    * Contains information about the interlace/progressive scan format in the
    * uncompressed frame
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: 4
    * data: One of the formats described in vidc_yuv_scan_format_type
    *
    * </pre>
    */
   VIDC_METADATA_SCAN_FORMAT       =    0x00000002,
/*..........................................................................*/

   /** <pre>
    * Extra data to convey display related extra data in VC-1 which
    * can change every frame
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: Variable
    * data: (4 bytes each)
    *
    *       RESPIC
    *       RFF
    *       RANGEMAP_PRESENT
    *       RANGEMAPY
    *       RANGEMAPUV
    *       number_of_pan_scan_windows
    *       windo_wx_PS_HOFFSET (not present if number_of_pan_scan_windows = 0)
    *       windo_wx_PS_VOFFSET
    *       windo_wx_PS_WIDTH
    *       windo_wx_PS_HEIGHT
    *       :
    *       :
    *       :
    *
    * </pre>
    */
   VIDC_METADATA_FRAMELEVEL_DISP_VC1      =  0x00000003,
/*..........................................................................*/

   /** <pre>
    * Extra data to convey display related extra data in VC-1 which
    * are conveyed only in sequence header. This information is given
    * out only when there is an updated sequence header
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: 4*9
    * data: (4 bytes each)
    *
    *       PSF
    *       UVSAMP
    *       COLOR_FMT_PRESENT
    *       COLOR_PRIM
    *       TRANSFER_CHAR
    *       MATRIX_COEFF
    *       ASPECT_RATIO
    *       ASPECT_HORIZ_SIZE
    *       ASPECT_VERT_SIZE
    * </pre>
    */
   VIDC_METADATA_SEQLEVEL_DISP_VC1        =  0x00000004,
/*..........................................................................*/

   /** <pre>
    * Meta data for reporting time stamp for each frame. This timestamp is
    * dervied from SEI or VUI parameter for H.264 decoder sessions. In case
    * this meta data is enabled and SEI data is not present timestamp value of
    * 0 would be passed
    *
    * This time stamp can be used by application in case timestamp passed
    * in buffer header is not reliable
    *
    * Permitted on Sessions: Decoder (H.264)
    *
    * data_size: 8 bytes
    * data:
    * Timestamp (8 bytes)
    *
    * </pre>
    */
   VIDC_METADATA_TIMESTAMP                =   0x00000005,
/*..........................................................................*/

   /** <pre>
    * Meta data for stereoscopic 3d frame packing arrangement. A decoder
    * session would generate this data whenever an FPA SEI NAL is detected
    * in the stream. Meta-data is also generated for the case when the older
    * FPA is cancelled (i.e. when stream switches from 3d to 2d)
    *
    * When passed on encoder input, FPA SEI NAL will be generated.
    *
    * Permitted on Sessions: Encoder/Decoder (H.264)
    *
    * data_size: 4*8 bytes
    * data:
    *       Layout
    *       view order
    *       flip
    *       bQuinCunx
    *       nLeftViewLumaSiteX
    *       nLeftViewLumaSiteY
    *       nRightViewLumaSiteX
    *       nRightViewLumaSiteY
    *
    * </pre>
    */
   VIDC_METADATA_S3D_FRAME_PACKING        =   0x00000006,

   /** <pre>
    * Indicates that the data payload contains frame rate information from
    * elementary stream. The extradata is generated for every output frame
    *
    * Permitted on Sessions: Decoder (H.264)
    *
    * data_size: 4 bytes
    * data:
    *       Frame rate (4 bytes)
    *
    * </pre>
    */
   VIDC_METADATA_FRAME_RATE               =   0x00000007,

   /** <pre>
    * Indicates that the data payload contains pan-scan window information for
    * the current decoded frame. This information is generated every frame.
    *
    * Permitted on Sessions: Decoder (H.264)
    *
    * data_size: variable (4 + 32*(number of pan scans))
    * data:
    *       <Number of pan scan windows> (4 bytes)
    *       <PS 1 horizontal start offset> (4 bytes)
    *       <PS 1 vertical start offset> (4 bytes)
    *       <PS 1 width> (4 bytes)
    *       <PS 1 height> (4 bytes)
    *       <PS 2 horizontal start offset> (4 bytes)
    *       <PS 2 vertical start offset> (4 bytes)
    *       <PS 2 width> (4 bytes)
    *       <PS 2 height> (4 bytes)
    *            :
    *            :
    *            :
    * </pre>
    */
   VIDC_METADATA_PANSCAN_WINDOW           =   0x00000008,

   /** <pre>
    * Indicates that the data payload contains information derived from
    * recovery point SEI.
    *
    * Permitted on Sessions: Decoder (H.264)
    *
    * data_size: 4 byte
    * data:
    *       nFlag (4 bytes)
    *       Only following flags are defined.
    *       0:Frame reconstruction is incorrect
    *       1:Frame reconstruction is correct
    *       2:Frame reconstruction is approximately correct
    * </pre>
    */
   VIDC_METADATA_RECOVERY_POINT_SEI       =   0x00000009,

   /** <pre>
    * Indicates that the data payload contains error detection code
    * (e.g. CRC/MISR)
    *
    * Permitted on Sessions: All
    *
    * data_size: 8 + error_code_length
    * data:
    *      err_code_type (1: CRC, 2: MISR)
    *      error_code_length (in bytes)
    *      err_code[]   (as many bytes as error_code_length)
    * </pre>
    */
   VIDC_METADATA_ERR_DETECTION_CODE       =   0x0000000C,

   /** <pre>
    * Indicates that the data payload contains MPEG-2 sequence display
    * extension extra-data
    *
    * Permitted on Sessions: All
    *
    * data_size: 4*7 bytes
    * data:
    *      <nVideoFormat>    (4 bytes)
    *      <bColorDescp>     (4 bytes)
    *      <nColorPrimaries> (4 bytes)
    *      <nTransferChar>   (4 bytes)
    *      <nMatrixCoeffs>   (4 bytes)
    *      <nDispWidth>      (4 bytes)
    *      <nDispHeight>     (4 bytes)
    * </pre>
    */
   VIDC_METADATA_MPEG2_SEQDISP            =   0x0000000D,

   /** <pre>
    * Indicates that the data payload contains user data dump extracted from
    * bitstream.
    * Some types of user data are:
    *    a) Active Format Description (AFD) for H264 & MPEG2
    *    b) Closed Caption for H264 & MPEG2
    *
    * Permitted on Sessions: H264 & MPEG2
    *
    * data_size:
    * data:
    *      Binay dump as-is from the bitstream
    * </pre>
    */
   VIDC_METADATA_STREAM_USERDATA          =   0x0000000E,

   /** <pre>
    * Indicates that the data payload contains frame level QP information. For
    * H264 it is the slice level QP averaged over whole frame. For all other
    * codecs its frame level QP.
    *
    * Permitted on Sessions: H264, MPEG4, MPEG2, VC1
    *
    * data_size:4 bytes
    * data:
    *      <nFrameQP>         (4 bytes)
    * </pre>
    */
   VIDC_METADATA_FRAME_QP                 =   0x0000000F,

   /** <pre>
    * Indicates that the data payload contains information regarding number
    * of bits in the frame (i.e frame size expressed in bits) & also the number
    * of bits in frame header (i.e frame header size expressed in bits).
    * Number of bits in header is reported as:
    *    for H264, sum of all slice headers bits for all the slices in the frame.
    *    for all other codecs, number of bits in the frame header.
    *
    * Permitted on Sessions: H264, MPEG4, MPEG2, VC1
    *
    * data_size:8 bytes
    * data:
    *      <nFrameBits>         (4 bytes)
    *      <nHeaderBits>        (4 bytes)
    * </pre>
    */
   VIDC_METADATA_FRAME_BITS_INFO          =  0x00000010,

   /** <pre>
    * Indicates that the data payload contains information regarding
    * ROI QP level information for input raw buffer.
    *
    * Permitted on Sessions: Encoder (H264/HEVC)
    *
    * data_size:9 bytes + ROI QP data
    * data:
    *      <bUseRoiInfo>        (1 bytes)
    *      <nRoiMBInfoSize>     (4 bytes)
    *      <pRoiMBInfo>         (4 bytes)
    * </pre>
    */
   VIDC_METADATA_ROI_QP                   =  0x00000013,

   /** <pre>
    * Indicates that the data payload contains mastering display color volume
    * information of static metadata in the frame.
    *
    * Permitted on Sessions: HEVC
    *
    * data_size:40 bytes
    * data:
    *      <nDisplayPrimariesX>               (4 bytes * 3)
    *      <nDisplayPrimariesY>               (4 bytes * 3)
    *      <nWhitePointX>                     (4 bytes)
    *      <nWhitePointY>                     (4 bytes)
    *      <nMaxDisplayMasteringLuminance>    (4 bytes)
    *      <nMinDisplayMasteringLuminance>    (4 bytes)
    * </pre>
    */
   VIDC_EXTRADATA_MASTERING_DISPLAY_COLOUR_SEI   =  0x00000015,

   /** <pre>
    * Indicates that the data payload contains content light level information
    * of static metadata in the frame.
    *
    * Permitted on Sessions: HEVC
    *
    * data_size:8 bytes
    * data:
    *      <nMaxContentLight>                (4 bytes)
    *      <nMaxPicAverageLight>             (4 bytes)
    * </pre>
    */
   VIDC_EXTRADATA_CONTENT_LIGHT_LEVEL_SEI        =  0x00000016,
/*..........................................................................*/

   /** <pre>
    * Slice for every encoded slice in the current encoded (compressed) frame
    * The information can be used for any packetization based on
    * individual slices
    *
    * Permitted on Sessions: Encoder
    *
    * data_size: Variable (4 + 8*(number of slices))
    * data:
    * <Number of slices> (4 bytes)
    * <Slice 1 offset>   (4 bytes)
    * <Slice 1 length>   (4 bytes)
    * <Slice 2 offset>   (4 bytes)
    * <Slice 2 length>   (4 bytes)
    *        :
    *        :
    *        :
    *
    * </pre>
    */
   VIDC_METADATA_ENC_SLICE                 =  0x7F100000,
/*..........................................................................*/

   /** <pre>
    * Concealed MB extra data gives out number of concealed MB in the current
    * decoded frame
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: 4
    * data: Number of m_bs concealed
    *
    * </pre>
    */
   VIDC_METADATA_CONCEALMB                =  0x7F100001,
/*..........................................................................*/

   /** <pre>
    * Property Index extra data. i.e this extra data's payload has a
    * [metadata property index + property structure] pair.
    * Refer to "vidc_metadata_property_index_type" for supported indices.
    *
    * Only the supported indices would be parsed and processed, others will be
    * ignored
    *
    * Client should first enable this property index metadata using
    * VIDC_I_METADATA_HEADER property and then enable required property
    * indexes in vidc_metadata_property_index_type via separate
    * VIDC_I_METADATA_HEADER property calls.
    *
    * Permitted on Sessions: Encoder/Decoder.
    *
    * data_size: Variable
    * data:  Index type (vidc_metadata_prop_index_type) = 4 bytes
    *        Index structure (Variable)
    *
    * </pre>
    */
   VIDC_METADATA_PROPERTY_INDEX           =  0x7F100002,
/*..........................................................................*/

   /** <pre>
    * Indicates that the data payload contains LTR frame information on the
    * output buffer for encoder session.
    * For H264, The extra-data contains information on the LTR-ID to be used
    * for the encoded LTR frame.
    * For VP8, The extra-data contains information on the reference frame type
    * to be used for the encoded LTR frame.
    *
    * Permitted on Sessions: Encoder
    *
    * data_size: 4 bytes
    * data:
    *        <LongTermFramIdx ID>          (4 bytes)
    * comment: Change VIDC_METADATA_PROPERTY_LTR to 0x7F100004 to resolve
    *          conflict with HFI_INDEX_EXTRADATA_ASPECT_RATIO which is set
    *          to 0x7F100003
    * </pre>
    */
   VIDC_METADATA_PROPERTY_LTR             =  0x7F100004,
/*..........................................................................*/

   /** <pre>
    * Indicates that the data payload contains MBI dump on the
    * output buffer for encoder session.
    *
    * Permitted on Sessions: Encoder
    *
    * data_size: variable
    * data:

    * </pre>
    */
   VIDC_METADATA_PROPERTY_MBI           =  0x7F100005,

/*..........................................................................*/

   /** <pre>
    * Indicates that the data payload contains VUI display information on the
    * output buffer for decoder session.
    * For H264, The extra-data contains information on the VUI video signal to be used
    * for the decoded frame display.
    *
    * Permitted on Sessions: Decoder
    *
    * data_size: 4 bytes
    * data:  Index type (vidc_metadata_prop_index_type) = 4 bytes
    *        Index structure (Variable)
    *
    * comment: Specify 0x7F100006 to avoid conflict with HFI_EXTRADATA_METADATA_MBI which is set
    *          to 0x7F100005
    *
    * </pre>
    */
   VIDC_METADATA_VUI_DISPLAY_INFO     =  0x7F100006,

/*..........................................................................*/

   /** <pre>
    * Filler extra data is generated to meet alignment requirements posed by
    * the core while generating other extra data. This meta data shall be
    * ignored by client while traversing and cannot be enabled or disabled
    * explicitly
    *
    * Permitted on Sessions: Encoder/Decoder
    *
    * data_size: Variable
    * data: NA
    *
    * </pre>
    */
   VIDC_METADATA_QCOMFILLER               =  0x7FE00002,
/*..........................................................................*/

     /** <pre>
    * Indicates that the data payload contains HDR10+ metadata information,
    * which should be encoded into the output bitstream.
    *
    * Permitted on Session: Encoder (HEVC)
    *
    * data_size : Variable
    * data: via metadata_addr
    *
    * </pre>
    */
  VIDC_METADATA_HDR10_PLUS = 0x7FE00003,
/*..........................................................................*/

   VIDC_METADATA_UNUSED    =  0x10000000
} vidc_metadata_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_metadata_property_index_type
----------------------------------------------------------------------------*/
/** Enumeration for supported Property Indices for
 *  VIDC_METADATA_PROPERTY_INDEX
 */
typedef enum
{
   /** Input crop meta data.
       Permitted on Sessions: Encoder.
       meta data layout:  size <4 bytes>
                           version <4 bytes>
                           port_index <4 bytes>
                           left <4 bytes>
                           top <4 bytes>
                           width <4 bytes>
                           height <4 bytes> */
   VIDC_METADATA_PROPERTY_INDEX_INPUT_CROP  =  0x0700000E,

   /** Output crop meta data.
       Permitted on Sessions: Decoder.
       meta data layout: size <4 bytes>
                         version <4 bytes>
                         port_index <4 bytes>
                         left <4 bytes>
                         top <4 bytes>
                         display_width <4 bytes>
                         display_height <4 bytes>
                         width <4 bytes>
                         height <4 bytes> */
   VIDC_METADATA_PROPERTY_INDEX_OUTPUT_CROP = 0x0700000F,

   /** Aspect ratio meta data.
       Permitted on Sessions: Decoder.
       meta data layout: size <4 bytes>
                         version <4 bytes>
                         port_index <4 bytes>
                         aspec_ratio <4 bytes> */
   VIDC_METADATA_PROPERTY_INDEX_ASPECT_RATIO = 0x7F100003,

   VIDC_METADATA_PROPERTY_INDEX_UNUSED = 0x10000000
} vidc_metadata_property_index_type;
/*==========================================================================*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_metadata_header_type
----------------------------------------------------------------------------*/
/**
 * The following structure is used to change/set different fields of meta_data
 * buffer Header.
 * [property id: VIDC_I_METADATA_HEADER]
 */
typedef struct
{
  /** one of the vidc_metadata_type or vidc_metadata_property_index_type
      values */
  uint32       metadata_type;

  /** enable/disable flag */
  boolean      enable;

  /** version associated with this type of metadata */
  uint32       version;

  /** Port associated with this type of metadata */
  uint32       port_index;

  /** Client "type" associated with this type of metadata */
  uint32       client_type;
} vidc_metadata_header_type;
/*==========================================================================*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_metadata_frameqp_paylaod_type
----------------------------------------------------------------------------*/
/**
 * The following structure is used to set frame qp metadata payload
 * [property id: VIDC_METADATA_FRAME_QP]
 */
typedef struct
{
   /**< Decoder: For H264 it is the slice level QP averaged over
        whole frame. For all other codecs its frame level QP.
        Encoder:
        H264: It is frame QP averaged across all LCU's in a frame.
        HEVC: It is frame level QP. */
   uint32  frame_qp;

   /**< Sum of QP over frame.
        Available only for decoder. */
   uint32  sum_qp;

   /**< Sum of QP of all skipped blocks in a frame.
        Available only for decoder. */
   uint32  skip_sum_qp;

   /**< Total number of skipped blocks in a frame.
        Available only for decoder. */
   uint32  skip_num_blocks;

   /**< Total number of blocks in a frame.
        Available only for decoder. */
   uint32  total_num_blocks;
} vidc_metadata_frameqp_paylaod_type;
/*==========================================================================*/

/* Payload structure for VIDC_QMETADATA_SEI_MASTERING_DISPLAY_COLOUR */
typedef struct
{
   /* Display primaries x index 0 */
   uint16   display_primaries_x_0;

   /* Display primaries y index 0 */
   uint16   display_primaries_y_0;

   /* Display primaries x index 1 */
   uint16   display_primaries_x_1;

   /* Display primaries y index 1 */
   uint16   display_primaries_y_1;

   /* Display primaries x index 2 */
   uint16   display_primaries_x_2;

   /* Display primaries y index 2 */
   uint16   display_primaries_y_2;

   /* White point x */
   uint16   white_point_x;

   /* White point y */
   uint16   white_point_y;

   /* Maximum display mastering luminance */
   uint32   max_display_mastering_luminance;

   /* Minimum display mastering luminance */
   uint32   min_display_mastering_luminance;

} vidc_qmetadata_sei_mastering_display_colour_type;

/* Payload structure for VIDC_QMETADATA_SEI_CONTENT_LIGHT_LEVEL */
typedef struct
{
   /* Maximum picture average light level */
   uint16   max_pic_average_light_level;

   /* Maximum content light level */
   uint16   max_content_light_level;

} vidc_qmetadata_sei_content_light_level_type;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_metadata_hdr_static_info
----------------------------------------------------------------------------*/
/**
 * The following structure is used to set HDR static metadata payload
 * [ioctl id: VIDC_I_ENC_HDR_INFO]
 */
typedef struct
{
   /* HDR10 mastering display colour volume */
   vidc_qmetadata_sei_mastering_display_colour_type   mastering_disp_colour_sei;

   /* HDR10 content light level information */
   vidc_qmetadata_sei_content_light_level_type        content_light_level_sei;

} vidc_metadata_hdr_static_info;
/*==========================================================================*/

#endif /* __HYP_VIDC_TYPES_H__ */

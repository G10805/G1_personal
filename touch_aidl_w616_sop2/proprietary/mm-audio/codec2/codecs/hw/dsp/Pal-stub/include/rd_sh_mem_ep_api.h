/**
 * \file rd_sh_mem_ep_api.h
 * \brief
 *           This file contains Shared mem module APIs
 *
 * \copyright
 *  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/rd_sh_mem_ep_api.h#9 $
*/
// clang-format on

#ifndef RD_SH_MEM_EP_API_H_
#define RD_SH_MEM_EP_API_H_

/*------------------------------------------------------------------------
 * Include files
 * -----------------------------------------------------------------------*/
#include "ar_defs.h"
#include "common_enc_dec_api.h"

/**
    @h2xml_title1          {APIs of Read Shared Memory End Point Module}
    @h2xml_title_agile_rev {APIs of Read Shared Memory End Point Module}
    @h2xml_title_date      {August 13, 2018}
 */

/**
   @h2xmlx_xmlNumberFormat {int}
*/

/**
 * Data command to send an empty buffer to the shared memory end point module.
 * RD_SHARED_MEM_EP writes to this buffer & client reads.
 *
 * This command can be used to read PCM, raw compressed as well as packetized streams.
 *
 * Once buffer is filled, DATA_CMD_RSP_RD_SH_MEM_EP_DATA_BUFFER_DONE event is raised.
 */
#define DATA_CMD_RD_SH_MEM_EP_DATA_BUFFER                         0x04001003

#include "spf_begin_pack.h"
struct data_cmd_rd_sh_mem_ep_data_buffer_t
{
   uint32_t buf_addr_lsw;
   /**< Lower 32 bits of the address of the buffer */

   uint32_t buf_addr_msw;
   /**< Upper 32 bits of the address of the buffer .

        The buffer address for each frame must be a valid address
        that was mapped via #APM_CMD_SHARED_MEM_MAP_REGIONS.
        The 64-bit number formed by buf_addr_lsw and buf_addr_msw must be
        aligned to the cache-line of the processor where spf runs (64-byte alignment for Hexagon).

        @tblsubhd{For a 32-bit shared memory address} This buf_addr_msw field
        must be set to 0.

        @tblsubhd{For a 36-bit address} Bits 31 to 4 of this
        buf_addr_msw field must be set to 0. */

   uint32_t mem_map_handle;
   /**< Unique identifier for the shared memory address.

        The spf returns this handle through #APM_CMD_RSP_SHARED_MEM_MAP_REGIONS. */

   uint32_t buf_size;
   /**< Number of bytes available for the spf to write. The
        first byte starts at the buffer address.

        @values @ge 0 */

}
#include "spf_end_pack.h"
;

/** Structure for DATA_CMD_RD_SH_MEM_EP_DATA_BUFFER */
typedef struct data_cmd_rd_sh_mem_ep_data_buffer_t data_cmd_rd_sh_mem_ep_data_buffer_t;

/**
 *  Indicates that the referenced buffer has been filled and
 *  is available to the client for reading.
 *
 *  This is response to DATA_CMD_RD_SH_MEM_EP_DATA_BUFFER
 *
    The referenced buffer contains an optional array of metadata if the client
    requested it in an open command. The buffer is followed by a variable
    amount of empty space, assuming metadata is present, followed by an integer
    number of encoded frames. Metadata presence is indicated by bit 30 of the
    flags field in the flags

    Buffer format
    A multiframe buffer becomes an array of frame metadata
    information structures, and then an array of audio frames. Data
    frames start with <i>offset</i>.
    @verbatim
    MetaData 0 (optional)
    MetaData 1 (optional)
       . . .
    MetaData n (optional)
    Extra space (optional)
    Frame 0 Payload (this always starts at an offset from buf_addr)
    Frame 1 Payload
       . . .
    Frame n-1 Payload

 */
#define DATA_CMD_RSP_RD_SH_MEM_EP_DATA_BUFFER_DONE                0x05001002

/** Definition of a timestamp valid flag bitmask. */
#define RD_SH_MEM_EP_BIT_MASK_TIMESTAMP_VALID_FLAG                AR_NON_GUID(0x80000000)

/** Definition of a timestamp valid flag shift value. */
#define RD_SH_MEM_EP_SHIFT_TIMESTAMP_VALID_FLAG                   31

/** Definition of the frame metadata bitmask.
*/
#define RD_SH_MEM_EP_BIT_MASK_FRAME_METADATA_FLAG                 AR_NON_GUID(0x60000000)

/** Definition of the frame metadata shift value.
*/
#define RD_SH_MEM_EP_SHIFT_FRAME_METADATA_FLAG                    29

#include "spf_begin_pack.h"
/**
 * Complete payload struct
 * struct of memory at (buf_addr_msw|buf_addr_lsw)
 * {
 *
 * if metadata_enabled:
 *    {
 *       data_event_rd_sh_mem_ep_metadata_t metadata[num_frames];
 *       uint8_t padding1[if_any];
 *    }
 *    uint8_t frames[data_size];
 *    uint8_t padding2[if_any];
 * }
 */
struct data_cmd_rsp_rd_sh_mem_ep_data_buffer_done_t
{
   uint32_t                status;
   /**< Status message (error code).

        @values Refer to @xrefcond{Q5,80-NF774-4,80-NA610-4} */

   uint32_t                buf_addr_lsw;
   /**< Lower 32 bits of the address of the buffer being returned. */

   uint32_t                buf_addr_msw;
   /**< Upper 32 bits of the address of the buffer being returned.

        The valid, mapped, 64-bit address is the same address that
        the client provides in #DATA_CMD_RD_SH_MEM_EP_DATA_BUFFER. */

   uint32_t                 mem_map_handle;
   /**< Unique identifier for the shared memory address.

        The spf returns this handle through #DATA_CMD_RD_SH_MEM_EP_DATA_BUFFER. */

   uint32_t                 data_size;
   /**< Total size of the data frames in bytes.

        @values @ge 0  */

   uint32_t                 offset;
   /**< Offset from the buffer address to the first byte of the data frame.
    * All frames are consecutive, starting from this offset.

        @values > 0 */

   uint32_t                 timestamp_lsw;
   /**< Lower 32 bits of the 64-bit session time in microseconds of the
        first sample in the buffer. */

   uint32_t                 timestamp_msw;
   /**< Upper 32 bits of the 64-bit session time in microseconds of the
        first sample in the buffer.

        the 64bit timestamp must be interpreted as a signed number.

        Source of the timestamp depends on the source feeding data to this module.
      */

   uint32_t                 flags;
   /**< Bit field of flags.

        @values{for bit 31}
        - 1 -- Timestamp is valid
        - 0 -- Timestamp is invalid
        - To set this bit, use #RD_SH_MEM_EP_BIT_MASK_TIMESTAMP_VALID_FLAG and
        RD_SH_MEM_EP_SHIFT_TIMESTAMP_VALID_FLAG

        @values{for bit 29-30}
        - 00 -- frame metadata absent
        - 01 -- Frame metadata is present as data_event_rd_sh_mem_ep_metadata_t
        - 10 -- Reserved
        - 11 -- Reserved
        - To set these bits, use #RD_SH_MEM_EP_BIT_MASK_FRAME_METADATA_FLAG and
          #RD_SH_MEM_EP_SHIFT_FRAME_METADATA_FLAG

        All other bits are reserved; the spf sets them to 0.

        When frame metadata is available, num_frames of consecutive instances of
        %data_event_rd_sh_mem_ep_metadata_t
         start at the buffer address
        (see <b>Buffer format</b>). */

   uint32_t                  num_frames;
   /**< Number of data frames in the buffer. */
}
#include "spf_end_pack.h"
;

/* Structure DATA_CMD_RSP_RD_SH_MEM_EP_DATA_BUFFER_DONE */
typedef struct data_cmd_rsp_rd_sh_mem_ep_data_buffer_done_t data_cmd_rsp_rd_sh_mem_ep_data_buffer_done_t;

#include "spf_begin_pack.h"

/* Payload of the metadata that can be put in the data read buffer.
*/
struct data_event_rd_sh_mem_ep_metadata_t
{
   uint32_t          offset;
   /**< Offset from the buffer address in %data_cmd_rsp_rd_sh_mem_ep_data_buffer_done_t to
        the frame associated with the metadata.

        @values > 0 */

   uint32_t          frame_size;
   /**< Size of each frame in bytes (E.g. encoded frame size).

        @values > 0 */

   uint32_t          pcm_length;
   /**< Number of PCM samples per channel corresponding to each frame_size
    * (E.g. number of PCM samples per channel used for encoding a frame).

        @values > 0 */

   uint32_t          timestamp_lsw;
   /**< Lower 32 bits of the 64-bit session time in microseconds of the
        first sample in the frame. */

   uint32_t          timestamp_msw;
   /**< Upper 32 bits of the 64-bit session time in microseconds of the
        first sample in the frame.
    */

   uint32_t          flags;
   /**< Frame flags.

        @values{for bit 31}
        - 1 -- Timestamp is valid
        - 0 -- Timestamp is not valid
        - To set this bit, use #RD_SH_MEM_EP_BIT_MASK_TIMESTAMP_VALID_FLAG and
        RD_SH_MEM_EP_SHIFT_TIMESTAMP_VALID_FLAG

        All other bits are reserved; the spf sets them to 0. */
}
#include "spf_end_pack.h"
;

/* Structure for metadata that can be put in the data read buffer. */
typedef struct data_event_rd_sh_mem_ep_metadata_t data_event_rd_sh_mem_ep_metadata_t;

/**
 * Media format sent from this module to the client.
 *
 * Payload media_format_t
 *
 * Event must be registered with APM_CMD_REGISTER_MODULE_EVENTS
 */
#define DATA_EVENT_ID_RD_SH_MEM_EP_MEDIA_FORMAT                        0x06001000

/**
    EOS event sent from this module to the client.
    This serves as a discontinuity marker.

    Event must be registered with APM_CMD_REGISTER_MODULE_EVENTS
*/
#define DATA_EVENT_ID_RD_SH_MEM_EP_EOS                                 0x06001001

#include "spf_begin_pack.h"
struct data_event_rd_sh_mem_ep_eos_t
{
   uint32_t    eos_reason;
   /**< EoS raised due to one of the following discontinuities
    * - pause
    * - other */
}
#include "spf_end_pack.h"
;

typedef struct data_event_rd_sh_mem_ep_eos_t data_event_rd_sh_mem_ep_eos_t;

/**
 * Input port ID of the Read Shared Memory EP
 */
#define PORT_ID_RD_SHARED_MEM_EP_INPUT                            0x2

/**
 * Output port ID of the Read Shared Memory EP
 * This Port is connected only in OFFLOAD(MDF) use-cases
 */
#define PORT_ID_RD_SHARED_MEM_EP_OUTPUT                            0x1

/**
 * ID of the Read Shared Memory End-Point Module
 *
 * This module has only one static input port with ID 2
 *
 * This module is supported only on container with Compression-Decompression capability.
 *
 * Required container capabilities: APM_CONTAINER_CAP_ID_CD
 *
 * Supported Input Media Format:
 *    - Any
 */
#define MODULE_ID_RD_SHARED_MEM_EP                                0x07001001
/**
    @h2xmlm_module         {"MODULE_ID_RD_SHARED_MEM_EP", MODULE_ID_RD_SHARED_MEM_EP}
    @h2xmlm_displayName    {"Read Shared Memory EndPoint"}
    @h2xmlm_description    {
                            This module is used to read data from spf into the host through packet exchange mechanism.
                            For regular use-case, this module has only one static input port with ID 2.
                            For MDF use-cases where the module is automatically inserted by the ARCT,
                            this module support one static output port with ID 1 and one static input port with ID 2.
                            This module is supported only on container with Compression-Decompression capability.
                            }
    @h2xmlm_offloadInsert       { RD_EP }
    @h2xmlm_dataInputPorts      { IN  = PORT_ID_RD_SHARED_MEM_EP_INPUT}
    @h2xmlm_dataOutputPorts     { OUT = PORT_ID_RD_SHARED_MEM_EP_OUTPUT}
    @h2xmlm_dataMaxInputPorts   {1}
    @h2xmlm_dataMaxOutputPorts  {1}
    @h2xmlm_reqContCapabilities { APM_CONTAINER_CAP_CD }
    @h2xmlm_isOffloadable       {false}
    @h2xmlm_stackSize           { 1024 }
    @{                     <-- Start of the Module -->

*/

/**
 * Special value of num_frames_per_buffer to indicate that as many buffer need to be filled as possible.
 */
#define RD_SH_MEM_NUM_FRAMES_IN_BUF_AS_MUCH_AS_POSSIBLE               0

/*==============================================================================
   Param ID
==============================================================================*/

/**
   Configuration of the Read Shared Memory Module.
*/
#define PARAM_ID_RD_SH_MEM_CFG                                       0x08001007

/*==============================================================================
   Param structure definitions
==============================================================================*/

/** @h2xmlp_parameter   {"PARAM_ID_RD_SH_MEM_CFG", PARAM_ID_RD_SH_MEM_CFG}
    @h2xmlp_description {Parameter for setting the configuration of read shared memory end point module.\n }
    @h2xmlp_toolPolicy  {Calibration} */
#include "spf_begin_pack.h"
struct param_id_rd_sh_mem_cfg_t
{
   uint32_t       num_frames_per_buffer;
   /**< @h2xmle_description {The number of frames per buffer that need to be populated in the Read buffers.
                             It's an error if even one frame cannot be filled.
                             If zero, then as many frames are filled as possible (only integral number of frames).
                             If N frames cannot be filled but M can be (where M &lt; N), then buffer is released after M.}
        @h2xmle_default     {0}
        @h2xmle_range       {0..0xFFFFFFFF}
        @h2xmle_policy      {Basic}
   */

   uint32_t       enable_frame_metadata;
   /**< @h2xmle_description {In Read buffers, frame metadata will be populated using struct data_event_rd_sh_mem_ep_metadata_t if this flag is 1.
                             Otherwise no frame metadata is filled.}
        @h2xmle_default     {0}
        @h2xmle_range       {0..1}
        @h2xmle_policy      {Basic}
   */
}
#include "spf_end_pack.h"
;

/* Type definition for the above structure. */
typedef struct param_id_rd_sh_mem_cfg_t param_id_rd_sh_mem_cfg_t;


/** @}                     <-- End of the Module --> */


#endif // RD_SH_MEM_EP_API_H_

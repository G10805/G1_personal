#ifndef METADATA_API_H
#define METADATA_API_H
/**
 *   \file metadata_api.h
 * \brief
 *           This file contains metadata IDs. Definitions are not supposed to be here.
 *           definitions will be in module_cmn_metadata.
 *
 * \copyright
 *  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/dev/avs.fwk/1.0/bvodapal.avs.fwk.1.0.gecko_mdf_b243/api/modules/metadata_api.h#1 $
*/
// clang-format on

#include "ar_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

/** @h2xml_title1           {Metadata IDs}
    @h2xml_title_agile_rev  {Metadata IDs}
    @h2xml_title_date       {Apr 30 2019} */
/**
   @h2xmlx_xmlNumberFormat {int}
*/

/**
    Metadata ID for EOS. module_cmn_md_t structure
    has to set the metadata_id field to this id when the metadata
    is EOS.  The module also should check this ID before operating on
    EOS structures.
 */
#define MODULE_CMN_MD_ID_EOS       0x0A00100C


/**
 * Metadata API to support the requirement of exchanging the metadata between the client and SPF
 *
 *   Metadata in general can be defined as additional information associated with the stream data.
 *
 *   Application or usage of the metadata information would be defined based on the type of
 *   Metadata which would be specified by the metadata ID
 *
 *   The metadata is expected to be exchanged in the specified format for writing and reading both
 *   by the HLOS client and the SPF during the data command read/write operations.
 *
 *   In general all the metadata is applicable to all the channels in the stream data
 *
 *  Defining metadata association
 *     Sample associated: Metadata is applicable at the offset at which it's inserted.
 *                        This metadata suffers both algorithmic and buffering delays.
 *     Buffer associated: metadata is applicable at the offset for the buffer.
 *                        This metadata suffers buffering delays only.
 *
 *  Defining metadata flag : The flags in the metadata payload specifies various control information.
 *  More details of individual controls are defined in the payload.
 *
 */


/** Definition of a metadata client information flag bitmask. */
#define MD_HEADER_FLAGS_BIT_MASK_CLIENT_INFO                AR_NON_GUID(0x00000001)

/** Definition of a metadata client information flag shift value. */
#define MD_HEADER_FLAGS_SHIFT_CLIENT_INFO                   0


/** Definition of a metadata tracking configuration bitmask. */
#define MD_HEADER_FLAGS_BIT_MASK_TRACKING_CONFIG             AR_NON_GUID(0x00000006)

/** Definition of a metadata tracking configuration shift value. */
#define MD_HEADER_FLAGS_SHIFT_TRACKING_CONFIG_FLAG            1


/** Definition of a metadata tracking event policy flag bitmask. */
#define MD_HEADER_FLAGS_BIT_MASK_TRACKING_EVENT_POLICY        AR_NON_GUID(0x00000008)

/** Definition of a metadata tracking event policy flag shift value. */
#define MD_HEADER_FLAGS_SHIFT_TRACKING_EVENT_POLICY_FLAG      3

/** Definition of a metadata association with data bitmask. */
#define MD_HEADER_FLAGS_BIT_MASK_ASSOCIATION                  AR_NON_GUID(0x00000010)

/** Definition of a metadata association with data flag shift value. */
#define MD_HEADER_FLAGS_SHIFT_ASSOCIATION_FLAG                4



/** Definition of a external client of SPF */
#define MD_HEADER_FLAGS_CLIENT_EXTERNAL                 0

/** Definition of a internal client of SPF */
#define MD_HEADER_FLAGS_CLIENT_INERTNAL                 1

/** Definition of a metadata tracking configuration disable. */
#define MD_HEADER_FLAGS_TRACKING_CONFIG_DISABLE          0

/** Definition of a metadata tracking configuration to enable for md drops only. */
#define MD_HEADER_FLAGS_TRACKING_CONFIG_ENABLE_FOR_DROPS_ONLY          1

/** Definition of a metadata tracking configuration to enable for md drop or consume. */
#define MD_HEADER_FLAGS_TRACKING_CONFIG_ENABLE_FOR_DROP_OR_CONSUME     2

/** Definition of a metadata tracking event policy last. */
#define MD_HEADER_FLAGS_TRACKING_EVENT_POLICY_LAST       0

/** Definition of a metadata tracking event policy last. */
#define MD_HEADER_FLAGS_TRACKING_EVENT_POLICY_EACH       1

/** Definition of a metadata sample association . */
#define MD_HEADER_FLAGS_SAMPLE_ASSOCIATED                0

/** Definition of a metadata sample association . */
#define MD_HEADER_FLAGS_BUFFER_ASSOCIATED                1


/**
 * Payload for Metadata
 *
 * struct complete_metadata_api_header_t
 * {
 *    metadata_api_header_t base;
 *    uint8_t payload[payload_size];
 * }
 */
#include "spf_begin_pack.h"
struct metadata_header_t
{
   uint32_t metadata_id;
   /**< Globally Unique ID of the metadata. */

   uint32_t        flags;
   /**<
    *  Specifies the control flags for the metadata.
    *
    *
    *   BIT_0 : specifies if the metadata was from the external client or meant to be rendered to external client.
    *
    *   @values{for bit 0}
    *      - 0 -- MD_HEADER_FLAGS_CLIENT_EXTERNAL (from/to external client (HLOS))
    *      - 1 -- MD_HEADER_FLAGS_CLIENT_INERTNAL (internal propagation in SPF, used for MDF)
    *
    *    This bit is specifically used for MDF to classify if the metadata is relevant
    *    to external client versus internally generated and propagated within SPF.
    *    Should be set to ‘0’ always  for HLOS
    *
    *
    *   BIT_2_to_1 : Specifies if the Metadata is trackable.
    *   If the metadata is dropped or consumed in SPF then the client would get an event
    *   with relevant information for the metadata. There is no requirement of registering for an event.
    *
    *   @values{for bit 2_to_1}
    *      - 0 -- MD_HEADER_FLAGS_TRACKING_CONFIG_DISABLE
    *             (tracking is disabled)
    *      - 1 -- MD_HEADER_FLAGS_TRACKING_CONFIG_ENABLE_FOR_DROPS_ONLY
    *             (tracking enabled for drops only)
    *      - 2 -- MD_HEADER_FLAGS_TRACKING_CONFIG_ENABLE_FOR_DROP_OR_CONSUME
    *             (tracking enabled for all drops and consumption)
    *      - 3 -- reserved and ignored.
    *
    *     For the configuration value set to 2, the event would be raised for metadata
    *     consumption from all the sink modules including hardware endpoints.
    *
    *   BIT_3 : Specifies the policy for raising the event.
    *   For every split in the path, the metadata would be cloned to both the paths.
    *   This will increase the internal references for the specified metadata and can
    *   lead to multiple drops/consumptions.
    *
    *   @values{for bit 3}
    *      - 0 -- MD_HEADER_FLAGS_TRACKING_EVENT_POLICY_LAST ( raise event for only the last drop/consumption of md in SPF)
    *      - 1 -- MD_HEADER_FLAGS_TRACKING_EVENT_POLICY_EACH ( raise event for every drop/consumption of md in SPF )
    *
    *    BIT_4: Specifies the metadata association
    *
    *    @values{for bit 4}
    *      - 0 -- MD_HEADER_FLAGS_SAMPLE_ASSOCIATED
    *      - 1 -- MD_HEADER_FLAGS_BUFFER_ASSOCIATED
    *
    *      Other bit fields are reserved and should be set to zero while writing the payload.
    */

   uint32_t offset;
   /**< Start offset in the data buffer from which metadata is applicable.
    *
    *   For cases where offset is a valid value, the following criteria apply for specifying the offset.
    *
    *       For raw compressed, offset is in bytes.
    *       For PCM & packetized, offset in samples (per channel).
    *           For interleaved data (most packetized format are also interleaved),
    *               metadata is applicable from sample at (data buffer +  (sample_offset * bytes_per_sample * num channels )
    *           For deinterleaved packed data,
    *               metadata is applicable from sample at (data buffer + offset * bytes_per_sample) in each channel
    *           For deinterleaved data unpacked,
    *               metadata is applicable from sample at (channel data buffer + (sample_offset * bytes_per_sample))
    *   Effective sample_offset in above cases must be less than buffer end
    */

   uint32_t        token_lsw;
   /* Unique token applicable for tracking Metadata.
    * The same token will be used while raising the Metadata tracking event
    *
    * Lower 32 bits of the token field
    */

    uint32_t  token_msw;
   /*
    * Higher 32 bits of the token field
    */

   uint32_t payload_size  ;
   /**< Total size of the payload in bytes for the specified metadata ID */

#if defined(__H2XML__)
   uint8_t payload[0];
   /**
    * Specifes the metadata payload of size defined in this API.
    * This payload is specific to the metadata ID.
    *
    * Guideline for metadata payload :
    *  The metadata payload should not contain any further memory references.
    *
    */
#endif
}
#include "spf_end_pack.h"
;

/* Structure for metadata header. */
typedef struct metadata_header_t metadata_header_t;

/** Definition of a metadata last instance flag bitmask. */
#define MD_TRACKING_FLAGS_BIT_MASK_LAST_INSTANCE       AR_NON_GUID(0x00000001)

/** Definition of a metadata last instance flag shift value. */
#define MD_TRACKING_FLAGS_IS_LAST_INSTANCE_FLAG         0

/** tracking status of the metadata when the meta-data is consumed. */
#define MD_TRACKING_STATUS_IS_CONSUMED                 AR_NON_GUID(0x00000000)

/** tracking status of the metadata when the meta-data is dropped. */
#define MD_TRACKING_STATUS_IS_DROPPED                  AR_NON_GUID(0x00000001)

/**
 * The metadata event is raised if the metadata is set as tracking metadata.
 * The event will be sent to the client from SPF in the following scenarios.
 *
 *  Metadata is dropped in the SPF and cannot be rendered to client through the data commands.
 *  Metadata is consumed by the the module and not propagated further.
 *  Note that all sink modules including the hardware end-points would consume the metadata.
 */

#define EVENT_ID_MODULE_CMN_METADATA_TRACKING_EVENT 0x0300100C

/**
 * Payload for Metadata tracking event
 */

#include "spf_begin_pack.h"
struct metadata_tracking_event_t
{
   uint32_t metadata_id;
   /**< Globally Unique ID of the metadata. */

   uint32_t source_module_instance;
   /**< Specifies the source module of the metadata.
    *
    * Client Metadata:
    *      For metadata from the client, this is basically derived from the source port ID in the data command from
    * client.
    * Module metadata to client:
    *      If the metadata is generated by a module in SPF, then this would be the module instance ID of the module.
    */
   uint32_t  module_instance_id;
   /**<  Specifies the module instance ID of the module which is raising the event when MD is consumed/dropped.
         Invalid (0) when the status is set as dropped.
   */
   uint32_t  flags;
   /**<
   *  Specifies the control flags for the metadata event.
   *
   *
   *   BIT_0 : specifies if this event corresponds to the last instance of metadata in the in SPF.
   *   @values{for bit 0}
   *      - 0 -- FALSE (not last instance)
   *      - 1 -- TRUE ( yes, its last instance)
   *
   *      1. For every split in the path, the metadata would be cloned to both the paths. This will increase
   *         the internal reference count for the specified MD in the SPF.
   *      2. If the event is generated in one path, while the MD in the other path is still active,
   *         this bit_field would be set as FALSE.
   *         This indicates that there is a possibility for more events corresponding to this MD
   *      3. If the metadata tracking is set as last drop/consumption, then there will be only one event and
   *         this bit would be set always.
   *
   *   Other bits are reserved for future use and should be set to zero.
   */

   uint32_t        token_lsw;
   /** <
    *  Specifies the LSW of the unique token.
    *
    *  Client Metadata:
    *         For Metadata from the client, this is specified in the medata API header and the same value would be used here.
    *  Module metadata to client:
    *         If the metadata is generated by a module in SPF, the module would need to specify the token.
    *         It can be configured to the module by the client, if the MD needs to be trackable.    *
    */

   uint32_t        token_msw;
   /** <
    *  Specifies the MSW of the unique token.
    */

   uint32_t        status;
   /**<
    * Specifies the status information for the metadata.
    *
   *   BIT_0 : Specifies if the metadata is consumed or dropped.
   *   @values{for bit 0}
    *      - 0 -- MD_TRACKING_STATUS_IS_CONSUMED; MD is consumed
    *      - 1 -- MD_TRACKING_STATUS_IS_DROPPED; MD is dropped
    *
    *      Other values are reserved for future use.
    */

}
#include "spf_end_pack.h"
;

/* Structure for metadata tracking  event. */
typedef struct metadata_tracking_event_t metadata_tracking_event_t;

/**
*  Metadata ID for Encoder frame info.
*/
#define MODULE_CMN_MD_ID_ENCODER_FRAME_INFO              0x0A001052


/* Payload of the metadata for encoder frame info,
*  that can be put in the read metadata buffer
 **/
#include "spf_begin_pack.h"
struct module_cmn_md_encoder_per_frame_info_t
{
   uint32_t          frame_size;
   /**< Size of each frame in bytes (E.g. encoded frame size).

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

        All other bits are reserved; the spf sets them to 0.
     */
}
#include "spf_end_pack.h"
;

/* Structure for encoder frame info metadata */
typedef struct module_cmn_md_encoder_per_frame_info_t module_cmn_md_encoder_per_frame_info_t;


/**
*  Metadata ID for Codec.2.0 requirement.
*/

/** Metadata ID to specify the Start of the buffer
 *  This metadata has no payload.
 */
#define MODULE_CMN_MD_ID_BUFFER_START          0x0A001053


/* Payload of the metadata to specify the start of the buffer
 **/
#include "spf_begin_pack.h"
struct module_cmn_md_buffer_start_t
{
   uint32_t buffer_index_lsw;
   /** <
    *  Specifies the lower 32 bits of the 64-bit unique buffer index
    */

   uint32_t buffer_index_msw;
   /** <
    *  SSpecifies the higher 32 bits of the 64-bit unique buffer index
    */
}
#include "spf_end_pack.h"
;

/* Structure for Buffer Start metadata */
typedef struct module_cmn_md_buffer_start_t module_cmn_md_buffer_start_t;


/** Metadata ID to specify the end of the buffer
 */
#define MODULE_CMN_MD_ID_BUFFER_END            0x0A001054


/** Definition of metadata error status/result flag bitmask. */
#define MD_END_PAYLOAD_FLAGS_BIT_MASK_ERROR_RESULT                AR_NON_GUID(0x00000001)

/** Definition of metadata error status/result flag shift value. */
#define MD_END_PAYLOAD_FLAGS_SHIFT_ERROR_RESULT                   0

/** Definition of metadata error result value. */
#define MD_END_RESULT_FAILED                                      1

/* Payload of the metadata to specify the end of the buffer
 **/
#include "spf_begin_pack.h"
struct module_cmn_md_buffer_end_t
{
   uint32_t buffer_index_lsw;
   /** <
    *  Specifies the lower 32 bits of the 64-bit unique buffer index
    */

   uint32_t buffer_index_msw;
   /** <
    *  Specifies the higher 32 bits of the 64-bit unique buffer index
    */

   uint32_t flags;
   /**< Field to specify any related information to from/to the client.
    *
    *   BIT_0 : specifies if all the input data is processed successfully or failed.
    *       0 : success
    *       1 : failed
    *
    *          Other bit field values are reserved for future use
    */
}
#include "spf_end_pack.h"
;

/* Structure for Buffer End metadata */
typedef struct module_cmn_md_buffer_end_t module_cmn_md_buffer_end_t;

/** Metadata ID to specify the Media format
 * Media format as metadata is propagated by the endpoint module in SPF to the client.
 * Client should configure the end-point module with appropriate configuration
 * to enable the propagation from SPF to Client in the data response.
 */
#define MODULE_CMN_MD_ID_MEDIA_FORMAT                      0x0A001056

/**
 * Payload media_format_t
 *
 */

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /* METADATA_API_H */

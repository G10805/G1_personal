/*========================================================================

            VIDC Interface for video hypervisor

*//** @file hyp_vidc_inf.h
   This file defines the VIDC interface used in video FE and BE hypervisor
   implementation.

Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---    --------------------------------------------------------
05/08/17   sm     Update for new hyp-video architecture
06/21/13   rz     Suppport SSR with normal shutdown call sequence
05/13/13   rz     Add LTR encoding
03/15/12   hm     Added responce message data structure
12/13/11   rkk    Created file.
 ========================================================================== */

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#ifndef __HYP_VIDC_INF_H__
#define __HYP_VIDC_INF_H__

#include "hyp_vidc_types.h"

/* -----------------------------------------------------------------------
**
** CONSTANTS, MACROS & TYPEDEFS
**
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
**
** IOCTL DEFINES
**
** ----------------------------------------------------------------------- */

/** IOCTL Base value for VIDC ioctl definitions. */
#define VIDC_IOCTL_BASE      0x0


/** <pre>
 *
 * Purpose: To Set video codec properties as enumerated in
 *          vidc_property_id_type.
 *
 * API type: Synchronous
 *
 * Prerequisites: Refer to each property ID description.
 *
 * IOCTL params:
 * SET:      InputData:     vidc_drv_property_type
 *           OutputData:    NULL
 *
 * </pre>
 */
#define VIDC_IOCTL_SET_PROPERTY       (VIDC_IOCTL_BASE + 1)

/** <pre>
 *
 * Purpose: To Get video codec properties as enumerated in
 *          vidc_property_id_type.
 *
 * API type: Synchronous
 *
 * Prerequisites: Refer to each property ID description.
 *
 * IOCTL params:
 * GET:      InputData:     vidc_drv_property_type
 *           OutputData:    vidc_prop_XXX_type
 *                          (struct associated with vidc_property_id_type
 *                           from vidc_drv_property_type will be copied into
 *                           output buffer and will be populated with get data)
 *
 * </pre>
 */
#define VIDC_IOCTL_GET_PROPERTY       (VIDC_IOCTL_BASE + 2)

/** <pre>
 *
 * Purpose: To request a single buffer allocation.
 *
 * API type: Synchronous
 *
 * Prerequisites: Buffer requirements should have been negotiated
 *
 * IOCTL params:
 * InputData:     vidc_buffer_info_type
 *                The address of the allocated buffer is returned in
 *                buf_addr field of the structure.
 * OutputData:    NULL
 *
 * </pre>
 */
#define VIDC_IOCTL_ALLOCATE_BUFFER    (VIDC_IOCTL_BASE + 3)


/** <pre>
 *
 * Purpose: To inform the driver of an externally allocated buffer address.
 *          i.e register a buffer address with driver.
 *          To be used only if an external allocator allocated this buffer.
 *
 * API type: Synchronous
 *
 * Prerequisites: Buffer requirements should have been negotiated
 *
 * Mandatory: Yes, If external allocator is being used to allocate buffers.
 *            The driver has to be made aware of all buffer addresses before
 *            issuing a VIDC_IOCTL_START.
 *
 * IOCTL params:
 * InputData:     vidc_buffer_info_type
 *                The address of the buffer to be registered should be set in
 *                buf_addr field of the structure.
 * OutputData:    NULL
 *
 * </pre>
 */
#define VIDC_IOCTL_SET_BUFFER          (VIDC_IOCTL_BASE + 4)


/** <pre>
 *
 * Purpose: To request freeing/de-registering of a single buffer.
 *          Driver would free the buffer if buffer was allocated via
 *          VIDC_IOCTL_ALLOCATE_BUFFER.
 *          Driver would de-register buffer address from driver buffer pool
 *          if the buffer was registered (set) via VIDC_IOCTL_SET_BUFFER
 *
 * API type: Synchronous
 *
 * Prerequisites: None
 *
 * Mandatory: Yes, if VIDC_IOCTL_ALLOCATE_BUFFER was called  OR
 *            before trying to register new set of buffer addresses via
 *            VIDC_IOCTL_SET_BUFFER calls.
 *
 * IOCTL params:
 * InputData:     vidc_buffer_info_type
 *                buf_size & buf_mode field of the structure can be ignored.
 *                The address of the buffer to be freed/de-registered is set
 *                in buf_addr field of the structure.
 * OutputData:    NULL
 *
 * </pre>
 */
#define VIDC_IOCTL_FREE_BUFFER         (VIDC_IOCTL_BASE + 5)

/** <pre>
 *
 * Purpose: To notify the driver for allocation of all the static resources.
 *
 *          This call also serves as a "commit" for the property configuration.
 *          The property configuration would be validated by the driver.
 *          After completion of LOAD_RESOURCES only dyanamic properties can
 *          be set on driver.
 *
 * API type: Asynchronous
 *
 * Prerequisites: None.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_LOAD_RESOURCES
 * NOTE: If static buffer allocation mode is being used, then only when all
 *       input and output buffers are allocated or set on driver would the
 *       driver send VIDC_EVT_RESP_LOAD_RESOURCES.
 * </pre>
 */
#define VIDC_IOCTL_LOAD_RESOURCES              (VIDC_IOCTL_BASE + 6)

/** <pre>
 *
 * Purpose: Release all static resources.
 *
 * API type: Asynchronous
 *
 * Prerequisites: None.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_RELEASE_RESOURCES
 *
 * </pre>
 */
#define VIDC_IOCTL_RELEASE_RESOURCES           (VIDC_IOCTL_BASE + 7)

/** <pre>
 *
 * Purpose: To activate the video codec session for video frame processing.
 *          After async reply indicating success the driver will be ready to
 *          accept input frames for processing.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_LOAD_RESOURCES is completed successfully.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_START
 *
 * </pre>
 */
#define VIDC_IOCTL_START              (VIDC_IOCTL_BASE + 8)


/** <pre>
 *
 * Purpose: To pause the video session.
 *
 *          If there are pending input frames in the pipeline that are yet to
 *          be processed then the driver will hold on to those buffers. All of
 *          them will be processed after the session is resumed via the RESUME
 *          IOCTL.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_LOAD_RESOURCES is completed successfully.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_PAUSE
 *
 * </pre>
 */
#define VIDC_IOCTL_PAUSE                  (VIDC_IOCTL_BASE + 9)

/** <pre>
 *
 * Purpose: To resume a paused video session.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_CMD_PAUSE is completed successfully.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_RESUME
 *
 * </pre>
 */
#define VIDC_IOCTL_RESUME                 (VIDC_IOCTL_BASE + 10)


/** <pre>
 *
 * Purpose: To deactivate video session from frame processing state.
 *
 *          If there are pending input frames in the pipeline that are yet to
 *          be processed then all of them will be discarded and returned while
 *          stopping the session. Any un-filled output buffers would
 *          be returned as well.
 *          If the client wants these pending frames to be processed then
 *          it should submit an input frame with buffer flag VIDC_FRAME_FLAG_EOS
 *          set. It should then wait till it receives an output frame with
 *          VIDC_FRAME_FLAG_EOS buffer flag set. Such output frame would
 *          indicate that all input frames in the pipeline are completely
 *          processed. Client can then issue the STOP IOCTL.
 *
 *          This STOP IOCTL does not clear all the set properties. Those are
 *          still retained. If the client wants to restart the video
 *          session for frame processing it should call VIDC_IOCTL_START to
 *          reactivate the session before submitting any frame for processing.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_CMD_START is completed successfully.
 *
 * IOCTL params:
 * InputData: NULL, OutputData: NULL
 *
 * Asynchronous respone message code:
 * All input & output buffers with the driver would be returned via the
 * VIDC_EVT_RESP_INPUT_DONE & VIDC_EVT_RESP_OUTPUT_DONE messages.
 * Finally VIDC_EVT_RESP_STOP to indicate the completion of STOP IOCTL.
 *
 * </pre>
 */
#define VIDC_IOCTL_STOP                   (VIDC_IOCTL_BASE + 11)

/** <pre>
 *
 * Purpose: To pass an input video buffer (frame) for processing.
 *
 *          Note for session resume:
 *          If encode frame requests are made after resuming a paused
 *          session, the input frame timestamps should NOT have deltas that
 *          reflect the time that the session was paused. i.e the client should
 *          provide the adjusted timestamp which has the pause time deleted.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_CMD_START is completed successfully.
 *
 * IOCTL params:
 * InputData:     vidc_frame_data_type
 * OutputData:    NULL
 *
 * Asynchronous respone message code:
 * VIDC_EVT_RESP_INPUT_DONE to return the input frame buffer for reuse.
 * The output buffer corresponding to this input frame processing would be
 * separately returned via a VIDC_EVT_RESP_OUTPUT_DONE.
 *
 * </pre>
 */
#define VIDC_IOCTL_EMPTY_INPUT_BUFFER        (VIDC_IOCTL_BASE + 12)


/** <pre>
 *
 * Purpose: To provide the video driver with an unfilled output buffer to
 *          "fill in" the processed video output frame.
 *
 *          This is the only way to specificaly indicate to the driver that
 *          the output buffer is unfilled. It should be called irrespective
 *          of whether driver is the allocator or an external allocator
 *          is being used for the output buffers.
 *
 *          This buffer would be added to driver's unfilled output buffers
 *          queue and will later get used to place the processed output frame
 *          contents
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_CMD_START is completed successfully.
 *
 * IOCTL params:
 * InputData:     vidc_frame_data_type
 * OutputData:    NULL
 *
 * Asynchronous response message code:
 * VIDC_EVT_RESP_OUTPUT_DONE will be given to return the output buffer with
 * the filled output frame.
 *
 * </pre>
 */
#define VIDC_IOCTL_FILL_OUTPUT_BUFFER     (VIDC_IOCTL_BASE + 13)


/** <pre>
 *
 * Purpose: To clear (flush out) the queued buffers. The associated structure
 *          will carry information on which type of buffers (input, output
 *          or both) need to be flushed.
 *
 * API type: Asynchronous
 *
 * Prerequisites: VIDC_IOCTL_CMD_START is completed successfully.
 *
 * Associated Structure: vidc_flush_mode_type
 *
 * IOCTL params:
 * InputData:     vidc_flush_mode_type
 * OutputData:    NULL
 *
 * Asynchronous response message code:
 * VIDC_EVT_RESP_INPUT_DONE will be sent to return each flushed input buffer
 * if input flush is requested. After all input buffers are flushed
 * VIDC_EVT_RESP_FLUSH_INPUT_DONE would be sent to indicate end of input buffer
 * flush.
 * VIDC_EVT_RESP_OUTPUT_DONE will be sent to return each flushed output/output2
 * buffer if output/output2 flush is requested. Client should refer to
 * vidc_buffer_type within vidc_frame_data_type to identify output/output2
 * buffers. After all output/output2 buffers are flushed
 * VIDC_EVT_RESP_FLUSH_OUTPUT_DONE/VIDC_EVT_RESP_FLUSH_OUTPUT2_DONE
 * would be sent to indicate end of output/output2 buffer flush.
 *
 * </pre>
 */
#define VIDC_IOCTL_FLUSH                        (VIDC_IOCTL_BASE + 14)


/** <pre>
 *
*  Purpose: To get a designated callback from driver
 *
*  API type: Asynchronous
 *
*  Associated Structure: vidc_drv_msg_info_type
 *
 * params:
*  InputData:     vidc_drv_msg_info_type
 *
 * Asynchronous response message code:
*  The designated msg will be sent to client
*
 * </pre>
 */
#define VIDC_PING_GET_CALLBACK                  (VIDC_IOCTL_BASE + 15)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   msg_payload_t
----------------------------------------------------------------------------*/
/**
 * message payload
 */
typedef union vidc_drv_msg_payload_t
{
   vidc_frame_data_type frame_data;
   uint32               event_data_1;
}vidc_drv_msg_payload;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_drv_msg_info_type_t
----------------------------------------------------------------------------*/
/**
 * Response message type
 */
typedef struct vidc_drv_msg_info_type_t
{
   vidc_status_type    status;   /**< Status of asynchronous
                                    *   operation
                                    */

   vidc_event_type    event_type;      /**< Uniquely identifies type of
                                    *   async message.
                                    */
   vidc_drv_msg_payload  payload;  /**< Message data */
}vidc_drv_msg_info_type;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   vidc_drv_property_type_t
----------------------------------------------------------------------------*/
/**
 * A wrapper structure to encapulate vidc_property_hdr_type and property id
 * associated structure.
 */
typedef struct vidc_drv_property_type_t
{
   vidc_property_hdr_type    prop_hdr;    /**< vidc property header >*/

   uint8                     payload[1];  /**< A pointer to structue associated
                                           *  to vidc_property_id_type.
                                           */
}vidc_drv_property_type;

#endif /* __HYP_VIDC_INF_H__ */

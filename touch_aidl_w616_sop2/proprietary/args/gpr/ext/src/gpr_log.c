/**
 * \file gpr_log.c
 * \brief
 *    This file contains logger implementation
 *
 *
 * \copyright
 *  Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/gpr.common/1.0/ext/src/gpr_log.c#2 $
*/
// clang-format on

/******************************************************************************
 * Includes                                                                    *
 *****************************************************************************/
#include "ar_osal_mem_op.h"
#include "ar_osal_mutex.h"
#include "ar_osal_heap.h"
#include "ar_osal_error.h"
#include "ar_osal_timer.h"
#include "ar_guids.h"
#include "ar_msg.h"
#include "gpr_api_i.h"
#include "gpr_api_log.h"
#include "gpr_msg_if.h"

/*****************************************************************************
 * Defines                                                                   *
 ****************************************************************************/

#define GPR_LOG_HISTORY_BYTE_SIZE_V (4 * 1024)

/**
 * Indicates that the entire payload should be logged - not just the header
 */
//#define GPR_LOG_PACKET_PAYLOAD 1

#define GPR_LOG_MAGIC_WORD_LEN 8
// Used to indicate start of gpr packets in the cicrular buffer
#define GPR_LOG_SYNC_WORD 0xE91111E9

/*****************************************************************************
 * Structure definitions                                                     *
 ****************************************************************************/

typedef struct gpr_log_history_t gpr_log_history_t;
struct gpr_log_history_t
{
   uint8_t  start_marker[GPR_LOG_MAGIC_WORD_LEN];
   uint32_t size;
   uint32_t offset;
   uint32_t log_payloads;
   uint8_t  log[GPR_LOG_HISTORY_BYTE_SIZE_V];
   uint8_t  end_marker[GPR_LOG_MAGIC_WORD_LEN];
};

/*****************************************************************************
 * Variables                                                                 *
 ****************************************************************************/

static ar_osal_mutex_t   gpr_log_history_mutex = NULL;
static gpr_log_history_t gpr_log_history;

/*****************************************************************************
 * Function Definitions                                                      *
 ****************************************************************************/

GPR_INTERNAL uint32_t gpr_log_init(void)
{
   uint32_t result = AR_EOK;

   result = ar_osal_mutex_create(&gpr_log_history_mutex);
   if (AR_EOK != result)
   {
	  if(NULL != gpr_log_history_mutex)
	  {
      (void)ar_osal_mutex_destroy(gpr_log_history_mutex);
		 gpr_log_history_mutex = NULL;
      }
      return AR_EFAILED;
   }

   (void)ar_mem_cpy(gpr_log_history.start_marker, sizeof(gpr_log_history.start_marker), "GPRSTART", 8);
   gpr_log_history.size = GPR_LOG_HISTORY_BYTE_SIZE_V;
   (void)ar_mem_set(gpr_log_history.log, 0xFF, sizeof(gpr_log_history.log));
   (void)ar_mem_cpy(gpr_log_history.end_marker, sizeof(gpr_log_history.end_marker), "GPR STOP", 8);

   gpr_log_history.log_payloads = 0;
   gpr_log_history.offset       = 0;
#ifdef GPR_LOG_PACKET_PAYLOAD
   gpr_log_history.log_payloads = 1;
#endif

   return result;
}

GPR_INTERNAL uint32_t gpr_log_deinit(void)
{
   AR_MSG(DBG_HIGH_PRIO, "GPR Log: Deinitializing gpr logger");
   if(NULL != gpr_log_history_mutex)
   {
   (void)ar_osal_mutex_destroy(gpr_log_history_mutex);
      gpr_log_history_mutex = NULL;
   }
   return AR_EOK;
}

static void gpr_log_circ_write(uint8_t *const       buf,
                               uint32_t const       buf_size,
                               uint32_t *           offset,
                               const uint8_t *const data,
                               uint32_t const       data_size)
{
   uint32_t endpos;
   uint32_t segment_size;
   uint32_t remaining_size;

   remaining_size = ((data_size > buf_size) ? buf_size : data_size);
   endpos         = (*offset + remaining_size);
   segment_size   = (buf_size - *offset);

   /* Check whether buffer is full or not */
   if (endpos < buf_size)
   {
      (void)ar_mem_cpy(&buf[*offset], segment_size, data, remaining_size);
      *offset = endpos;
   }
   else
   {
      if (endpos == buf_size)
      {
         /* Reset offset to 0 if end position and buffer size are equals */
         (void)ar_mem_cpy(&buf[*offset], segment_size, data, remaining_size);
         *offset = 0;
      }
      else
      {
         /* Since current data cannot be fitted into the buffer, write only
          * data which cab be fitted at he end of buffer and write rest of the data
          * from begining of the buffer
          */
         (void)ar_mem_cpy(&buf[*offset], segment_size, data, segment_size);
         remaining_size -= segment_size;
         *offset = remaining_size;
         (void)ar_mem_cpy(buf, buf_size, &data[segment_size], remaining_size);
      }
   }
}

GPR_INTERNAL int32_t gpr_log_packet(gpr_packet_t *packet)
{
   uint32_t packet_size;
   uint32_t marker;

   uint16_t dst_domain_id;
   uint16_t src_domain_id;

   uint64_t time = ar_timer_get_time_in_us();

   src_domain_id = packet->src_domain_id;
   dst_domain_id = packet->dst_domain_id;

   packet_size = GPR_PKT_GET_PACKET_BYTE_SIZE(packet->header);

   (void)ar_osal_mutex_lock(gpr_log_history_mutex);

   marker = GPR_LOG_SYNC_WORD;

   gpr_log_circ_write(gpr_log_history.log,
                      gpr_log_history.size,
                      &gpr_log_history.offset,
                      ((uint8_t *)&marker),
                      sizeof(marker));
   /* Write the sync word. */
   gpr_log_circ_write(gpr_log_history.log,
                      gpr_log_history.size,
                      &gpr_log_history.offset,
                      ((uint8_t *)&time),
                      sizeof(time));
   /* Write the timestamp. */
#ifdef GPR_LOG_PACKET_PAYLOAD
   gpr_log_circ_write(gpr_log_history.log,
                      gpr_log_history.size,
                      &gpr_log_history.offset,
                      ((uint8_t *)packet),
                      packet_size);
#else
   gpr_log_circ_write(gpr_log_history.log,
                      gpr_log_history.size,
                      &gpr_log_history.offset,
                      ((uint8_t *)packet),
                      sizeof(gpr_packet_t));
#endif
   /* Write the packet. */
   gpr_log_history.offset = (((gpr_log_history.offset + 3) >> 2) << 2);
   /* Align to 32-bits. */

   (void)ar_osal_mutex_unlock(gpr_log_history_mutex);

   return AR_EOK;
}

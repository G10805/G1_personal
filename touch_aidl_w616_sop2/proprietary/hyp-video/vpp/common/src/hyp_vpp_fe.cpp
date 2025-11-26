/*===========================================================================

*//** @file hyp_vpp_fe.c
This file provides interface functions for VPP FE native driver and HAB
with core VPP FE driver

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "AEEStdDef.h"
#include "MMSignal.h"
#include "hyp_vpppriv.h"
#include "hyp_vppinf.h"

#include "stdlib.h"
#include "hyp_vpp.h"

#include "MMThread.h"
#include "hyp_vpp_fe_translation.h"
#include "MMTimer.h"
#include "MMCriticalSection.h"
#include "hyp_vpp_buf_manager.h"
#include "hyp_vpp_debug.h"

#define WAIT_FOR_RESPONSE_ACK()                                   \
   do {                                                           \
      MM_HANDLE MsgSetHandle = 0;                                 \
      hypvpp_session->time_out = 0;                               \
      MM_SignalQ_TimedWait (                                      \
            hypvpp_session->habmm_queue_info.msg_signal_queue     \
            [HABMM_SIGNAL_RESPONSE_ACK].state_signal_queue,       \
            HVFE_TIMEOUT_INTERVAL_IN_MS,                          \
            (void **)&MsgSetHandle, &hypvpp_session->time_out );  \
   } while(0)

#define VALID_HANDLE_RET_IF_FAIL(handle)                          \
   do {                                                           \
      hypvpp_session_t* session = (hypvpp_session_t*)(handle);    \
      if (!session || VALID_HANDLE == session->handle_header) {   \
         HYP_VPP_MSG_INFO("invalid handle");                      \
         return HYPVPP_STATUS_BAD_PARAMETER;                      \
      }                                                           \
   } while(0)

std::map<uint32, uint32> msg_map = {
    {HYPVPP_MSGCMD_OPEN, HYPVPP_MSGRESP_OPEN_RET},
    {HYPVPP_MSGCMD_CLOSE, HYPVPP_MSGRESP_CLOSE_RET},
    {HYPVPP_MSGCMD_DRAIN, HYPVPP_MSGRESP_DRAIN_RET},
};

std::map<uint32, const char*> msg_str_map = {
    {HYPVPP_MSGCMD_OPEN, "open"},
    {HYPVPP_MSGCMD_CLOSE, "close"},
    {HYPVPP_MSGCMD_DRAIN, "drain"},
};

std::map<hypvpp_status_type, vpp_error> hyp_vpp_type_map = {
   {HYPVPP_STATUS_FAIL, VPP_ERR},
   {HYPVPP_STATUS_SUCCESS, VPP_OK},
   {HYPVPP_STATUS_PENDING, VPP_PENDING},
   {HYPVPP_STATUS_INVALID_STATE, VPP_ERR_STATE},
   {HYPVPP_STATUS_INVALID_CONFIG, VPP_ERR_INVALID_CFG},
   {HYPVPP_STATUS_BAD_PARAMETER, VPP_ERR_PARAM},
   {HYPVPP_STATUS_NO_MEM, VPP_ERR_NO_MEM},
   {HYPVPP_STATUS_NO_RESOURCE, VPP_ERR_RESOURCES},
   {HYPVPP_STATUS_HW_ERROR, VPP_ERR_HW},
   {HYPVPP_STATUS_FATAL, VPP_ERR_FATAL},
   {HYPVPP_STATUS_VERSION_MISMATCH, VPP_ERR},
   {HYPVPP_STATUS_MAX, VPP_ERR},
};

static int hypvpp_resp_handler(void*);
static int hypvpp_cb_event_handler(void*);

static hypvpp_status_type start_response_handler(hypvpp_session_t* hypvpp_session);
static hypvpp_status_type stop_response_handler(hypvpp_session_t* hypvpp_session);

vpp_error hypvpp_status_to_vpp_error (hypvpp_status_type type)
{
   vpp_error ret = VPP_ERR;

   auto iter = hyp_vpp_type_map.find(type);
   if (iter != hyp_vpp_type_map.end()) {
      ret = iter->second;
   }

   return ret;
}

#define PRINT_VPP_BUF(tag, port, buf)                                                          \
   do {                                                                                        \
      if (buf)                                                                                 \
      {                                                                                        \
         struct vpp_mem_buffer mem_buf = buf->pixel;                                           \
         struct vpp_mem_buffer meta_buf = buf->extradata;                                      \
         HYP_VPP_MSG_LOW("%s: %s: time-stamp:%" PRIu64 " flags:0x%x cookie-in-out:%p fd:%d offset:%d" \
               " alloc_len:%u filled_len:%u meta fd:%d offset:%d alloc_len:%u filled_len:%u",\
               tag, port == VPP_PORT_INPUT ? "INPUT" : "OUTPUT",                               \
               buf->timestamp, buf->flags, buf->cookie_in_to_out,                              \
               mem_buf.fd, mem_buf.offset, mem_buf.alloc_len, mem_buf.filled_len,              \
               meta_buf.fd, meta_buf.offset, meta_buf.alloc_len, meta_buf.filled_len           \
               );                                                                              \
      }                                                                                        \
   } while(0)

statistics::statistics()
   : monitor_thread(NULL),
   monitor_lock(NULL),
   monitor_sig(NULL),
   monitor_sig_q(NULL),
   num_input_queue(0),
   num_output_done(0),
   prev_num_input_queue(0),
   prev_num_output_done(0),
   last_sys_time_ms(0),
   total_latency(0),
   timeout(1000),
   started(false),
   thread_stop(false)
{
   int32 ret = 0;

   MM_CriticalSection_Create(&monitor_lock);
   ret = MM_SignalQ_Create(&monitor_sig_q);
   if (ret)
   {
      HYP_VPP_MSG_ERROR("failed to create monitor signalQ");
   }
   else
   {
      ret = MM_Signal_Create(monitor_sig_q, NULL, NULL, &monitor_sig);
      if (ret)
      {
         HYP_VPP_MSG_ERROR("failed to create monitor signal");
      }
   }

   if (!ret)
   {
      if (MM_Thread_CreateEx (MM_Thread_DefaultPriority,
               0, thread_loop, this, HYPV_THREAD_STACK_SIZE,
               "monitor_thread", &monitor_thread) != 0)
      {
         HYP_VPP_MSG_ERROR("failed to create monitor thread");
         ret = -1;
      }
   }

   if (ret)
   {
      if (monitor_lock)
      {
         MM_CriticalSection_Release(monitor_lock);
      }
      if (monitor_sig)
      {
         MM_Signal_Release(monitor_sig);
      }
      if (monitor_sig_q)
      {
         MM_SignalQ_Release(monitor_sig_q);
      }
   }
}

statistics::~statistics()
{
   int ret;

   thread_stop = true;

   if (monitor_sig)
   {
      MM_Signal_Set(monitor_sig);
   }
   if (monitor_thread)
   {
      MM_Thread_Join(monitor_thread, &ret);
      MM_Thread_Release(monitor_thread);
   }
   if (monitor_lock)
   {
      MM_CriticalSection_Release(monitor_lock);
   }
   if (monitor_sig)
   {
      MM_Signal_Release(monitor_sig);
   }
   if (monitor_sig_q)
   {
      MM_SignalQ_Release(monitor_sig_q);
   }
}

void statistics::input_queue()
{
   MM_CriticalSection_Enter(monitor_lock);
   num_input_queue++;
   MM_CriticalSection_Leave(monitor_lock);
}

void statistics::output_done()
{
   MM_CriticalSection_Enter(monitor_lock);
   num_output_done++;
   MM_CriticalSection_Leave(monitor_lock);
}

void statistics::start()
{
   if (!started)
   {
      MM_Signal_Set(monitor_sig);
      started = true;
   }
}

int statistics::thread_loop(void* ctx)
{
   int ret = 0;
   int time_out = 0;
   statistics* stat = static_cast<statistics*>(ctx);

   while (!stat->thread_stop)
   {
      MM_SignalQ_TimedWait(stat->monitor_sig_q, stat->timeout, NULL, &time_out);
      uint64_t now = stat->current_time_ms();

      MM_CriticalSection_Enter(stat->monitor_lock);
      if (stat->last_sys_time_ms)
      {
         uint64_t time_diff_ms = now - stat->last_sys_time_ms;
         uint32_t num_output_done_diff = stat->num_output_done - stat->prev_num_output_done;

         float avg_in_fps = (stat->num_input_queue - stat->prev_num_input_queue) * 1000.0 / time_diff_ms;
         float avg_out_fps = num_output_done_diff * 1000.0 / time_diff_ms;
         uint64_t avg_latency = num_output_done_diff ? stat->total_latency / num_output_done_diff : 0;

         HYP_VPP_MSG_PERF("input queue:%u->%u output done:%u->%u time diff:%" PRIu64 "ms",
               stat->prev_num_input_queue, stat->num_input_queue,
               stat->prev_num_output_done, stat->num_output_done, time_diff_ms);
         HYP_VPP_MSG_PERF("avg in fps:%.1f out fps:%.1f avg latency:%" PRIu64 "ms",
               avg_in_fps, avg_out_fps, avg_latency);

         stat->total_latency = 0;
         stat->prev_num_input_queue = stat->num_input_queue;
         stat->prev_num_output_done = stat->num_output_done;
      }
      stat->last_sys_time_ms = now;
      MM_CriticalSection_Leave(stat->monitor_lock);
   }

   return ret;
}

void statistics::add_entry(struct vpp_buffer *buf)
{
   MM_CriticalSection_Enter(monitor_lock);
   in_out_time_map.emplace(buf->cookie_in_to_out, current_time_ms());
   HYP_VPP_MSG_LOW("add entry cookie %p", buf->cookie_in_to_out);
   MM_CriticalSection_Leave(monitor_lock);
}

uint64_t statistics::get_latency(struct vpp_buffer *buf)
{
   uint64_t start_time = 0;
   uint64_t latency = 0;
   uint64_t now = 0;

   MM_CriticalSection_Enter(monitor_lock);
   now = current_time_ms();
   auto itr = in_out_time_map.find(buf->cookie_in_to_out);
   if (itr != in_out_time_map.end())
   {
      start_time = itr->second;
      latency = now - start_time;
      if (!(buf->flags & VPP_BUFFER_FLAG_PENDING_OUTPUT)) {
         in_out_time_map.erase(itr);
         HYP_VPP_MSG_LOW("remove entry cookie %p", buf->cookie_in_to_out);
      }
      total_latency += latency;
   }
   MM_CriticalSection_Leave(monitor_lock);

   return latency;
}

/**===========================================================================

  FUNCTION hyp_vpp_init

  @brief  Hypervisor VPP FE init function that interface with HAB

  @param [in] flag
  @param [in] hyp callback pointer

  @dependencies
  None

  @return
  Returns hypervisor session pointer

  ===========================================================================*/
void* hyp_vpp_init(uint32_t flags, vpp_callbacks callback)
{
   UNUSED(flags);

   int32 rc = 0;
   hypvpp_session_t* hypvpp_session = NULL;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_init_data_type return_init;

   hypvpp_session = (hypvpp_session_t *)HABMM_MALLOC(sizeof(hypvpp_session_t));
   if (NULL == hypvpp_session)
   {
      status = HYPVPP_STATUS_NO_MEM;
      HYP_VPP_MSG_ERROR("malloc hyp vpp session failed");
   }
   else
   {
      HYP_VPP_MSG_INFO("hypvpp(%p) init", hypvpp_session);

      HABMM_MEMSET(hypvpp_session, 0, sizeof(hypvpp_session_t));
      hypvpp_session->habmm_handle = VALID_HANDLE;

      MM_CriticalSection_Create(&hypvpp_session->lock_buffer);
      hypvpp_map_queue_init(&hypvpp_session->lookup_queue);

      rc = MM_SignalQ_Create(&hypvpp_session->state_synch_obj_q);
      if (rc)
      {
         HYP_VPP_MSG_ERROR("MM SignalQ Create failed");
      }
      else
      {
         rc = MM_Signal_Create(hypvpp_session->state_synch_obj_q, NULL, NULL, &hypvpp_session->state_synch_obj);
         if (rc)
         {
            HYP_VPP_MSG_ERROR("MM Signal Create failed");
         }
      }
      if (0 != rc)
      {
         HYP_VPP_MSG_ERROR("failed, invalid node");

         if (hypvpp_session->lock_buffer)
         {
            MM_CriticalSection_Release(hypvpp_session->lock_buffer);
         }
         if (hypvpp_session->state_synch_obj)
         {
            MM_Signal_Release(hypvpp_session->state_synch_obj);
         }
         if (hypvpp_session->state_synch_obj_q)
         {
            MM_SignalQ_Release(hypvpp_session->state_synch_obj_q);
         }
      }

      hypvpp_session->vpp_cb = callback;

      hypvpp_session->dl_handle = dlopen(HAB_LIB, RTLD_NOW);

      if (NULL == hypvpp_session->dl_handle)
      {
         status = HYPVPP_STATUS_FAIL;
         HYP_VPP_MSG_ERROR("dlopen load hab lib %s failed",
               HAB_LIB);
      }

      if (HYPVPP_STATUS_SUCCESS == status)
      {
         hypvpp_session->habmm_if.pfOpen = (hyp_habmm_socket_open) dlsym(hypvpp_session->dl_handle, "habmm_socket_open");
         hypvpp_session->habmm_if.pfClose = (hyp_habmm_socket_close) dlsym(hypvpp_session->dl_handle, "habmm_socket_close");
         hypvpp_session->habmm_if.pfSend = (hyp_habmm_socket_send) dlsym(hypvpp_session->dl_handle, "habmm_socket_send");
         hypvpp_session->habmm_if.pfRecv = (hyp_habmm_socket_recv) dlsym(hypvpp_session->dl_handle, "habmm_socket_recv");
         hypvpp_session->habmm_if.pfImport = (hyp_habmm_import) dlsym(hypvpp_session->dl_handle, "habmm_import");
         hypvpp_session->habmm_if.pfExport = (hyp_habmm_export) dlsym(hypvpp_session->dl_handle, "habmm_export");
         hypvpp_session->habmm_if.pfUnImport = (hyp_habmm_unimport) dlsym(hypvpp_session->dl_handle, "habmm_unimport");
         hypvpp_session->habmm_if.pfUnExport = (hyp_habmm_unexport) dlsym(hypvpp_session->dl_handle, "habmm_unexport");

         if (!hypvpp_session->habmm_if.pfOpen ||
               !hypvpp_session->habmm_if.pfClose ||
               !hypvpp_session->habmm_if.pfSend ||
               !hypvpp_session->habmm_if.pfRecv ||
               !hypvpp_session->habmm_if.pfImport ||
               !hypvpp_session->habmm_if.pfExport ||
               !hypvpp_session->habmm_if.pfUnImport ||
               !hypvpp_session->habmm_if.pfUnExport)
         {
            HYP_VPP_MSG_ERROR("falied to dlsym hab lib");
            dlclose(hypvpp_session->dl_handle);
            hypvpp_session->dl_handle = NULL;
            status = HYPVPP_STATUS_FAIL;
         }

         if (HYPVPP_STATUS_SUCCESS == status)
         {
            if (0 != pthread_mutex_init(&hypvpp_session->event_handler_mutex, NULL))
            {
               HYP_VPP_MSG_ERROR("failed to init event handler mutex");
               status = HYPVPP_STATUS_FAIL;
            }
            else if (0 != pthread_cond_init(&hypvpp_session->event_handler_cond, NULL))
            {
               HYP_VPP_MSG_ERROR("failed to init event handler cond");
               status = HYPVPP_STATUS_FAIL;
            }
            else if (0 != MM_CriticalSection_Create (&hypvpp_session->hab_lock))
            {
               HYP_VPP_MSG_ERROR("failed to create critical section for device ioctl");
               status = HYPVPP_STATUS_FAIL;
            }
         }
      }

      if (HYPVPP_STATUS_SUCCESS == status)
      {
         if (0 != habmm_init_queue(&hypvpp_session->habmm_queue_info, MAX_MSG_QUEUE_SIZE, NUM_HVFE_MSG_QUEUE, NUM_HVFE_SIGNAL))
         {
            HYP_VPP_MSG_ERROR("init queue failed");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            unsigned long ts = 0;
            if (0 != hypvpp_session->habmm_if.pfOpen(&hypvpp_session->habmm_handle, MM_VID_3, 0, 0))
            {
               HYP_VPP_MSG_ERROR("habmm socket open failed");
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               // open connection with registered device
               if (HYPVPP_STATUS_SUCCESS != (status = start_response_handler(hypvpp_session)))
               {
                  HYP_VPP_MSG_ERROR("failed to start response handler");
               }
               else
               {
                  /** Compose msg data for device init and call send() */
                  MM_Time_GetTime( &ts );
                  hypvpp_session->hypvpp_time_stamp = (uint32)ts;

                  /** Hypervisor abstraction */
                  msg.msg_id = HYPVPP_MSGCMD_INIT;
                  msg.message_number = ++hypvpp_session->hypvpp_msg_number;
                  msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
                  msg.data_size = sizeof(hypvpp_init_data_type);
                  msg.version = HYP_VPP_VERSION;
                  msg.virtual_channel = hypvpp_session->habmm_handle;

                  if (HYPVPP_STATUS_SUCCESS == status)
                  {
                     msg.pid = (uint32) getpid();
                     HYP_VPP_MSG_HIGH("hvfe init pid %u", msg.pid);
                     if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
                     {
                        HYP_VPP_MSG_ERROR("habmm socket send failed");
                        status = HYPVPP_STATUS_FAIL;
                     }
                     else
                     {
                        WAIT_FOR_RESPONSE_ACK();
                        if (0 != hypvpp_session->time_out)
                        {
                           HYP_VPP_MSG_ERROR("device init failed, wait ACK timeout");
                           status = HYPVPP_STATUS_FAIL;
                        }
                        else
                        {
                           status = translate_habmm_to_hvfe(hypvpp_session,
                                 HYPVPP_MSGRESP_INIT_RET,
                                 (void*)&hypvpp_session->hypvpp_cmd_resp,
                                 sizeof(msg),
                                 (void*)&return_init);

                           if (HYPVPP_STATUS_SUCCESS != status)
                           {
                              HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
                           }
                           else if (HYPVPP_STATUS_SUCCESS != return_init.return_status)
                           {
                              HYP_VPP_MSG_ERROR("device init returned error, version %s, ret status %d",
                                    HYPVPP_STATUS_VERSION_MISMATCH == return_init.return_status ? "mismatch": "match", return_init.return_status);
                              status = HYPVPP_STATUS_FAIL;
                           }
                           else
                           {
                              hypvpp_session->device_handle =  return_init.return_io_handle;
                              HYP_VPP_MSG_HIGH("init return_io_handle:0x%llx", (unsigned long long)return_init.return_io_handle);
                           }
                        }
                     }
                  }
               }
            }
         }
      }

   }


   if (HYPVPP_STATUS_SUCCESS != status && NULL != hypvpp_session)
   {
      /** Release all the allocated resources as term will not be called
        when init fails
        */
      hypvpp_session->exit_resp_handler = TRUE;
      pthread_mutex_lock(&hypvpp_session->event_handler_mutex);
      pthread_cond_signal(&hypvpp_session->event_handler_cond);
      pthread_mutex_unlock(&hypvpp_session->event_handler_mutex);
      if (hypvpp_session->habmm_if.pfClose)
        hypvpp_session->habmm_if.pfClose(hypvpp_session->habmm_handle);

      status = stop_response_handler(hypvpp_session);
      if (HYPVPP_STATUS_SUCCESS != status)
      {
         HYP_VPP_MSG_ERROR("stop_response_handler failed with status %d", status);
      }

      pthread_mutex_destroy(&hypvpp_session->event_handler_mutex);
      pthread_cond_destroy(&hypvpp_session->event_handler_cond);
      hypvpp_map_queue_deinit(&hypvpp_session->lookup_queue);

      if (NULL != hypvpp_session->hab_lock)
      {
         MM_CriticalSection_Release (hypvpp_session->hab_lock);
         hypvpp_session->hab_lock = NULL;
      }

      habmm_deinit_queue(&hypvpp_session->habmm_queue_info);
      if (hypvpp_session->dl_handle && dlclose(hypvpp_session->dl_handle))
      {
         char *dl_error = NULL;
         dl_error = (char *)dlerror();
         HYP_VPP_MSG_ERROR("dlclose hab lib failed %s", dl_error);
      }
      else
      {
         HYP_VPP_MSG_HIGH("dlclose hab lib successful");
      }

      HABMM_FREE(hypvpp_session);
   }
   else
   {
      if (hypvpp_session && (vpp_debug_level & HYP_PRIO_PERF))
      {
         hypvpp_session->stat = new statistics();
         HYP_VPP_MSG_LOW("enable fps/latency statistics");
      }
   }

   return hypvpp_session;
}

/**===========================================================================

  FUNCTION hyp_vpp_term

  @brief  Hypervisor VPP FE term function that interface with HAB

  @param [in] context

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_term (void* context)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) term", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Release Daemon BE thread and msg & callback queue */
   MM_Time_GetTime( &ts );

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_TERM;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = (uint32)ts;
   msg.data_size = sizeof(hypvpp_return_data_type);
   msg.virtual_channel = hypvpp_session->habmm_handle;

   MM_CriticalSection_Enter(hypvpp_session->hab_lock);
   if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
   {
      status = HYPVPP_STATUS_FAIL;
   }
   else
   {
      WAIT_FOR_RESPONSE_ACK();
      if (0 != hypvpp_session->time_out)
      {
         HYP_VPP_MSG_ERROR("wait ACK timeout");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         status = translate_habmm_to_hvfe(hypvpp_session,
               HYPVPP_MSGRESP_TERM_RET,
               (void*)&hypvpp_session->hypvpp_cmd_resp,
               sizeof(msg),
               (void*)&return_data);
         if (HYPVPP_STATUS_SUCCESS != status)
         {
            HYP_VPP_MSG_ERROR("failed translate habmm to FE with status=%d", status);
         }
         else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
         {
            HYP_VPP_MSG_ERROR("device term failed status=%d",return_data.return_status);
            status = return_data.return_status;
         }
         else
         {
            HYP_VPP_MSG_HIGH("device term successfully");
         }
      }
   }
   MM_CriticalSection_Leave(hypvpp_session->hab_lock);

   hypvpp_session->exit_resp_handler = TRUE;
   pthread_mutex_lock(&hypvpp_session->event_handler_mutex);
   pthread_cond_signal(&hypvpp_session->event_handler_cond);
   pthread_mutex_unlock(&hypvpp_session->event_handler_mutex);

   status = stop_response_handler(hypvpp_session);
   if (HYPVPP_STATUS_SUCCESS != status)
   {
      HYP_VPP_MSG_ERROR("stop_response_handler failed with status %d", status);
   }

   pthread_mutex_destroy(&hypvpp_session->event_handler_mutex);
   pthread_cond_destroy(&hypvpp_session->event_handler_cond);

   hypvpp_map_cleanup(&hypvpp_session->habmm_if, &hypvpp_session->lookup_queue);
   hypvpp_map_queue_deinit(&hypvpp_session->lookup_queue);
   hypvpp_session->habmm_if.pfClose(hypvpp_session->habmm_handle);
   if (NULL != hypvpp_session->hab_lock)
   {
      MM_CriticalSection_Release (hypvpp_session->hab_lock);
      hypvpp_session->hab_lock = NULL;
   }
   habmm_deinit_queue(&hypvpp_session->habmm_queue_info);

   if (dlclose(hypvpp_session->dl_handle))
   {
      char *dl_error = NULL;
      dl_error = (char *)dlerror();
      HYP_VPP_MSG_ERROR("dlclose hab lib failed %s", dl_error);
   }
   else
   {
      HYP_VPP_MSG_HIGH("dlclose hab lib successful");
   }

   hypvpp_session->dl_handle = NULL;
   if (hypvpp_session->stat)
   {
      delete hypvpp_session->stat;
      hypvpp_session->stat = NULL;
   }

   HABMM_FREE(hypvpp_session);

   return status;
}

/**===========================================================================

  FUNCTION start_response_handler

  @brief  Hypervisor VPP FE function to invoke HAB response thread handler

  @param [in] hypervisor session pointer

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
static hypvpp_status_type start_response_handler(hypvpp_session_t* hypvpp_session)
{
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;

   hypvpp_session->exit_resp_handler = FALSE;

   if (MM_Thread_CreateEx ( MM_Thread_DefaultPriority,
            0, hypvpp_resp_handler, (void*)hypvpp_session, HYPV_THREAD_STACK_SIZE,
            "hypvppFeResponsehandler", (MM_HANDLE*)&hypvpp_session->hvfe_response_cb_thread) != 0)
   {
      HYP_VPP_MSG_ERROR("failed to create hyp VPP FE response handler thread");
      status = HYPVPP_STATUS_FAIL;
   }
   else
   {
      if (MM_Thread_CreateEx ( MM_Thread_DefaultPriority,
               0, hypvpp_cb_event_handler, (void*)hypvpp_session, HYPV_THREAD_STACK_SIZE,
               "hypvppFeEventhandler", (MM_HANDLE*)&hypvpp_session->hvfe_event_cb_thread) != 0)
      {
         HYP_VPP_MSG_ERROR("failed to create hyp VPP FE event handler thread");
         status = HYPVPP_STATUS_FAIL;
      }
   }

   return status;
}

/**===========================================================================

  FUNCTION stop_response_handler

  @brief  Hypervisor VPP FE function to stop HAB response thread handler

  @param [in] hypervisor session pointer

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
static hypvpp_status_type stop_response_handler(hypvpp_session_t* hypvpp_session)
{
   int rc;

   // shut down the callback loop and free memory
   if (hypvpp_session->hvfe_response_cb_thread)
   {
      hypvpp_session->exit_resp_handler = TRUE;
      MM_Thread_Join( hypvpp_session->hvfe_response_cb_thread, &rc);
      MM_Thread_Release(hypvpp_session->hvfe_response_cb_thread);
      hypvpp_session->hvfe_response_cb_thread = NULL;
   }

   if (hypvpp_session->hvfe_event_cb_thread)
   {
      MM_Thread_Join(hypvpp_session->hvfe_event_cb_thread, &rc);
      MM_Thread_Release(hypvpp_session->hvfe_event_cb_thread);
      hypvpp_session->hvfe_event_cb_thread = NULL;
   }

   return HYPVPP_STATUS_SUCCESS;
}

/**===========================================================================

  FUNCTION hypvpp_resp_handler

  @brief  Hypervisor VPP FE HAB response handler

  @param [in] context

  @return
  Returns int

  ===========================================================================*/
static int hypvpp_resp_handler(void* context)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   int done = 0;
   uint32 size_bytes = 0;
   habmm_msg_desc_t hab_msg;
   int32 rc = 0;
   boolean ret = TRUE;

   while (!done)
   {
      /*
       ** Retrieve a msg from the queue and finish all messages in the queue
       */
      while (hypvpp_session->exit_resp_handler == FALSE)
      {
         size_bytes = sizeof(habmm_msg_desc_t);
         HABMM_MEMSET(&hab_msg, 0, sizeof(habmm_msg_desc_t));
         rc = hypvpp_session->habmm_if.pfRecv(hypvpp_session->habmm_handle, (void*)&hab_msg, &size_bytes, 0, 0);
         if (0 == rc)
         {
            HYP_VPP_MSG_LOW("hypvpp(%p) pid=%u msg_id=0x%x msg_num=%u msg_size=%u",
                            hypvpp_session, hab_msg.pid, hab_msg.msg_id,
                            hab_msg.message_number, hab_msg.data_size);
            if (HYPVPP_MSGRESP_EVENT == hab_msg.msg_id || HYPVPP_MSGRESP_CALLBACK == hab_msg.msg_id)
            {
               if (HYPVPP_MSGRESP_EVENT == hab_msg.msg_id)
               {
                  HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.event_data, (const void *)&hab_msg.data.event_data,
                               sizeof(hypvpp_event_data_type));
               }
               else
               {
                  HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.callback_data, (const void *)&hab_msg.data.callback_data,
                               sizeof(hypvpp_callback_data_type));
               }

               pthread_mutex_lock(&hypvpp_session->event_handler_mutex);
               ret = habmm_enqueue_buffer_list(&hypvpp_session->habmm_queue_info, HABMM_IO_CALLBACK_INDEX,
                                               (habmm_msg_desc_t *)&hab_msg);
               if (TRUE == ret)
               {
                  pthread_cond_signal(&hypvpp_session->event_handler_cond);
               }
               else
               {
                  HYP_VPP_MSG_ERROR("habmm buffer list enqueue failed ");
               }
               pthread_mutex_unlock(&hypvpp_session->event_handler_mutex);
            }
            else if (HYPVPP_MSGRESP_INIT_RET == hab_msg.msg_id)
            {
               HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.init_data, (const void *)&hab_msg.data.init_data,
                     sizeof(hypvpp_init_data_type));
               MM_Signal_Set(hypvpp_session->habmm_queue_info.msg_signal_queue[HABMM_SIGNAL_RESPONSE_ACK].state_change_signal);
            }
            else if (HYPVPP_MSGRESP_GET_BUF_REQ_RET == hab_msg.msg_id)
            {
               HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.buf_req_data, (const void *)&hab_msg.data.buf_req_data,
                     sizeof(hypvpp_buf_req_data_type));
               MM_Signal_Set(hypvpp_session->habmm_queue_info.msg_signal_queue[HABMM_SIGNAL_RESPONSE_ACK].state_change_signal);
            }
            else if (HYPVPP_MSGRESP_OPEN_RET == hab_msg.msg_id || HYPVPP_MSGRESP_CLOSE_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_SET_CTRL_RET == hab_msg.msg_id || HYPVPP_MSGRESP_SET_PARAM_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_QUEUE_INPUT_BUF_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_QUEUE_OUTPUT_BUF_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_FLUSH_RET == hab_msg.msg_id || HYPVPP_MSGRESP_RECONFIG_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_SET_VID_PROP_RET == hab_msg.msg_id
                     || HYPVPP_MSGRESP_DRAIN_RET == hab_msg.msg_id)
            {
               HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.return_data, (const void *)&hab_msg.data.return_data,
                     sizeof(hypvpp_return_data_type));
               MM_Signal_Set(hypvpp_session->habmm_queue_info.msg_signal_queue[HABMM_SIGNAL_RESPONSE_ACK].state_change_signal);
            }
            else if (HYPVPP_MSGRESP_TERM_RET == hab_msg.msg_id)
            {
               HABMM_MEMCPY((void *)&hypvpp_session->hypvpp_cmd_resp.return_data, (const void *)&hab_msg.data.return_data,
                     sizeof(hypvpp_return_data_type));
               hypvpp_session->exit_resp_handler = TRUE;

               MM_Signal_Set(hypvpp_session->habmm_queue_info.msg_signal_queue[HABMM_SIGNAL_RESPONSE_ACK].state_change_signal);
            }
         }
         else if (-EINTR == rc)
         {
            HYP_VPP_MSG_HIGH("hypvpp(%p) receive error, retrying", hypvpp_session);
            continue;
         }
         else
         {
            HYP_VPP_MSG_ERROR("hypvpp(%p) socket recv failed: hab ret code %d, stop loop", hypvpp_session, rc);
            hypvpp_session->exit_resp_handler = TRUE;
            hypvpp_session->time_out = 1;
            pthread_mutex_lock(&hypvpp_session->event_handler_mutex);
            pthread_cond_signal(&hypvpp_session->event_handler_cond);
            pthread_mutex_unlock(&hypvpp_session->event_handler_mutex);
            MM_Signal_Set(hypvpp_session->habmm_queue_info.msg_signal_queue[HABMM_SIGNAL_RESPONSE_ACK].state_change_signal);
            break;
         }
      }

      if (TRUE == hypvpp_session->exit_resp_handler)
      {
         done = 1;
      }
   }

   return 0;
}

/**===========================================================================

  FUNCTION hypvpp_cb_event_handler

  @brief  Hypervisor VPP FE HAB callback event handler

  @param [in] context

  @return
  Returns int

  ===========================================================================*/
static int hypvpp_cb_event_handler(void* session)
{
   int ret = 0;
   hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)session;
   hypvpp_callback_data_type callback_data;
   hypvpp_event_data_type event_data;
   int done = 0;
   habmm_msg_desc_t *hypvpp_msg = NULL;

   if (hypvpp_session == NULL)
   {
      HYP_VPP_MSG_ERROR("hyp vpp session is NULL");
      ret = -1;
   }
   else
   {
      HABMM_MEMSET(&callback_data, 0, sizeof(hypvpp_callback_data_type));
      HABMM_MEMSET(&event_data, 0, sizeof(hypvpp_event_data_type));

      while (!done)
      {
         /*
          ** Retrieve a msg from the queue and finish all the messages in the queue
          */
         while (FALSE == hypvpp_session->exit_resp_handler)
         {
            pthread_mutex_lock(&hypvpp_session->event_handler_mutex);
            hypvpp_msg = (habmm_msg_desc_t *)habmm_dequeue_buffer_list(&hypvpp_session->habmm_queue_info,
                  HABMM_IO_CALLBACK_INDEX);
            if (NULL == hypvpp_msg)
            {
               /* wait for new event */
               pthread_cond_wait(&hypvpp_session->event_handler_cond, &hypvpp_session->event_handler_mutex);
            }
            pthread_mutex_unlock(&hypvpp_session->event_handler_mutex);

            if (NULL != hypvpp_msg)
            {
               if (HYPVPP_MSGRESP_CALLBACK == hypvpp_msg->msg_id)
               {
                  HYP_VPP_MSG_INFO("hypvpp(%p) got a callback type:0x%x", hypvpp_session, hypvpp_msg->data.callback_data.event_type);
                  rc = translate_habmm_to_hvfe(hypvpp_session,
                        hypvpp_msg->msg_id,
                        (void*)&hypvpp_msg->data,
                        sizeof(hypvpp_msg_data_type),
                        (void*)&callback_data);
                  if (HYPVPP_STATUS_SUCCESS != rc)
                  {
                     HYP_VPP_MSG_ERROR("failed to translate from habmm to FE");
                  }
                  else
                  {
                     struct vpp_buffer* vpp_buf = &callback_data.buffer_done;
                     if (HYPVPP_EVT_RESP_INPUT_BUF_DONE == callback_data.event_type)
                     {
                        PRINT_VPP_BUF("DQBUF", VPP_PORT_INPUT, vpp_buf);
                        hypvpp_session->vpp_cb.input_buffer_done(hypvpp_session->vpp_cb.pv, vpp_buf);
                     }
                     else if (HYPVPP_EVT_RESP_OUTPUT_BUF_DONE == callback_data.event_type)
                     {
                        if (hypvpp_session->stat)
                        {
                           uint64_t latency = hypvpp_session->stat->get_latency(vpp_buf);
                           if (latency)
                           {
                              HYP_VPP_MSG_LOW("output cookie %p latency %" PRIu64 "ms",
                                    vpp_buf->cookie_in_to_out, latency);
                           }
                           hypvpp_session->stat->output_done();
                        }

                        PRINT_VPP_BUF("DQBUF", VPP_PORT_OUTPUT, vpp_buf);
                        hypvpp_session->vpp_cb.output_buffer_done(hypvpp_session->vpp_cb.pv, vpp_buf);
                     }
                  }
               }
               else if (HYPVPP_MSGRESP_EVENT == hypvpp_msg->msg_id)
               {
                  rc = translate_habmm_to_hvfe(hypvpp_session,
                        hypvpp_msg->msg_id,
                        (void*)&hypvpp_msg->data,
                        sizeof(hypvpp_msg_data_type),
                        (void*)&event_data);
                  if (HYPVPP_STATUS_SUCCESS != rc)
                  {
                     HYP_VPP_MSG_ERROR("failed to translate from habmm to FE");
                  }
                  else
                  {
                     HYP_VPP_MSG_LOW("hypvpp(%p) vpp posts event:0x%x", hypvpp_session, (unsigned int)event_data.event.type);
                     hypvpp_session->vpp_cb.vpp_event(hypvpp_session->vpp_cb.pv, event_data.event);
                  }
               }
            }
         }
         if (TRUE == hypvpp_session->exit_resp_handler)
         {
            done = 1;
         }
      }
   }

   return ret;
}

/**===========================================================================

  FUNCTION _hyp_vpp_ctl_no_param

  @brief  Internal hypervisor VPP FE control API. Use for control without extra parameters,
          e.g. hyp_vpp_open and hyp_vpp_close.

  @param [in] hypvpp_session
  @param [in] message id

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
static hypvpp_status_type _hyp_vpp_ctl_no_param(hypvpp_session_t* hypvpp_session, uint32 msg_id)
{
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;
   uint32 msg_ret_id = msg_map[msg_id];
   const char* msg_str = msg_str_map[msg_id];

   HYP_VPP_MSG_INFO("hypvpp(%p) %s", hypvpp_session, msg_str);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device open and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = msg_id;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.data_size = sizeof(hypvpp_return_data_type);
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe %s pid %u", msg_str, msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         WAIT_FOR_RESPONSE_ACK();
         if (0 != hypvpp_session->time_out)
         {
            HYP_VPP_MSG_ERROR("device %s failed, wait ACK timeout", msg_str);
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            status = translate_habmm_to_hvfe(hypvpp_session,
                  msg_ret_id,
                  (void*)&hypvpp_session->hypvpp_cmd_resp,
                  sizeof(msg),
                  (void*)&return_data);

            if (HYPVPP_STATUS_SUCCESS != status)
            {
               HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
            }
            else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
            {
               HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                     HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
               status = HYPVPP_STATUS_FAIL;
            }
         }
      }
      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_open

  @brief  Hypervisor VPP FE open API

  @param [in] hypvpp_session
  @param [in] message id

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_open(void* context)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type ret;

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);
   ret = _hyp_vpp_ctl_no_param(hypvpp_session, HYPVPP_MSGCMD_OPEN);

   return ret;
}

/**===========================================================================

  FUNCTION hyp_vpp_close

  @brief  Hypervisor VPP FE close API

  @param [in] context

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_close(void* context)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type ret;

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);
   ret = _hyp_vpp_ctl_no_param(hypvpp_session, HYPVPP_MSGCMD_CLOSE);

   return ret;
}

/**===========================================================================

  FUNCTION hyp_vpp_set_ctl

  @brief  Hypervisor VPP FE control API

  @param [in] context
  @param [in] hqv_control

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_set_ctrl(void* context, struct hqv_control ctrl)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;
   uint32_t frc_seg_payload_size = 0;
   uint32 num_frc_segment = 0;
   vpp_ctrl_frc_segment *frc_segments = NULL;

   HYP_VPP_MSG_INFO("hypvpp(%p) set control", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device open and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_SET_CTRL;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;
   msg.data_size = sizeof(hypvpp_set_ctrl_data_type);
   msg.data.ctrl_data.ctrl = ctrl;

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe set control pid %u", msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         if (HQV_CONTROL_FRC == ctrl.ctrl_type)
         {
            num_frc_segment = ctrl.frc.num_segments;
            frc_seg_payload_size = num_frc_segment * sizeof(vpp_ctrl_frc_segment);
            frc_segments = (vpp_ctrl_frc_segment*)HABMM_MALLOC(frc_seg_payload_size);
            if (frc_segments)
            {
               HABMM_MEMCPY(frc_segments, ctrl.frc.segments, frc_seg_payload_size);
               if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, frc_segments, frc_seg_payload_size, 0 /*flags*/))
               {
                  HYP_VPP_MSG_ERROR("habmm socket send failed");
                  status = HYPVPP_STATUS_FAIL;
               }
               else
               {
                  status = HYPVPP_STATUS_SUCCESS;
               }
            }
            HABMM_FREE(frc_segments);
         }

         if (HYPVPP_STATUS_SUCCESS == status)
         {
            WAIT_FOR_RESPONSE_ACK();
            if (0 != hypvpp_session->time_out)
            {
               HYP_VPP_MSG_ERROR("device set control failed, wait ACK timeout");
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               status = translate_habmm_to_hvfe(hypvpp_session,
                     HYPVPP_MSGRESP_SET_CTRL_RET,
                     (void*)&hypvpp_session->hypvpp_cmd_resp,
                     sizeof(msg),
                     (void*)&return_data);

               if (HYPVPP_STATUS_SUCCESS != status)
               {
                  HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
               }
               else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
               {
                  HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                        HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
                  status = HYPVPP_STATUS_FAIL;
               }
            }
         }
      }

      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_set_param

  @brief  Hypervisor VPP FE param API

  @param [in] context
  @param [in] vpp_port
  @param [in] vpp_port_param

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_set_param(void* context, enum vpp_port port, struct vpp_port_param param)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_set_param_data_type param_data;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) set param", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&param_data, 0, sizeof(hypvpp_set_param_data_type));
   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device open and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_SET_PARAM;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;
   msg.data.param_data.port = port;
   msg.data.param_data.param = param;
   msg.data_size = sizeof(hypvpp_set_param_data_type);

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe set param pid %u", msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         WAIT_FOR_RESPONSE_ACK();
         if (0 != hypvpp_session->time_out)
         {
            HYP_VPP_MSG_ERROR("device set param failed, wait ACK timeout");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            status = translate_habmm_to_hvfe(hypvpp_session,
                  HYPVPP_MSGRESP_SET_CTRL_RET,
                  (void*)&hypvpp_session->hypvpp_cmd_resp,
                  sizeof(msg),
                  (void*)&return_data);

            if (HYPVPP_STATUS_SUCCESS != status)
            {
               HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
            }
            else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
            {
               HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                     HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
               status = HYPVPP_STATUS_FAIL;
            }
         }
      }
      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_queue_buf

  @brief  Hypervisor VPP FE queue buf API

  @param [in] context
  @param [in] enum vpp_port
  @param [in] struct vpp_buffer

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_queue_buf(void* context, enum vpp_port port, struct vpp_buffer *buf)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   hypvpp_queue_buf_data_type buf_data;
   unsigned long ts = 0;
   uint32 msg_id = 0;
   uint32 msg_ret_id = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) queue %s buf", hypvpp_session, port == VPP_PORT_INPUT ? "input" : "output");
   PRINT_VPP_BUF("QBUF", port, buf);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));
   HABMM_MEMSET(&buf_data, 0, sizeof(hypvpp_queue_buf_data_type));

   if (VPP_PORT_INPUT == port)
   {
      msg_id = HYPVPP_MSGCMD_QUEUE_INPUT_BUF;
      msg_ret_id = HYPVPP_MSGRESP_QUEUE_INPUT_BUF_RET;
      if (hypvpp_session->stat)
      {
         hypvpp_session->stat->start();
         hypvpp_session->stat->add_entry(buf);
         hypvpp_session->stat->input_queue();
      }
   }
   else if (VPP_PORT_OUTPUT == port)
   {
      msg_id = HYPVPP_MSGCMD_QUEUE_OUTPUT_BUF;
      msg_ret_id = HYPVPP_MSGRESP_QUEUE_OUTPUT_BUF_RET;
   }
   else
   {
      HYP_VPP_MSG_ERROR("Port %d is invalid", port);
      status = HYPVPP_STATUS_FAIL;
   }

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      /** Compose msg data for device open and call send() */
      MM_Time_GetTime( &ts );
      hypvpp_session->hypvpp_time_stamp = (uint32)ts;

      status = translate_hvfe_to_habmm(hypvpp_session, msg_id, (void*)buf, sizeof(vpp_buffer), (void*)&buf_data.buffer_data);
      if (HYPVPP_STATUS_SUCCESS == status)
      {
         /** Hypervisor abstraction */
         msg.msg_id = msg_id;
         msg.message_number = ++hypvpp_session->hypvpp_msg_number;
         msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
         msg.data_size = sizeof(hypvpp_return_data_type);
         msg.version = HYP_VPP_VERSION;
         msg.virtual_channel = hypvpp_session->habmm_handle;
         msg.data.buf_data = buf_data;
         msg.pid = (uint32) getpid();
         HYP_VPP_MSG_HIGH("hvfe queue %s buf pid %u", port == VPP_PORT_INPUT? "input" : "output", msg.pid);

         MM_CriticalSection_Enter(hypvpp_session->hab_lock);
         if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
         {
            HYP_VPP_MSG_ERROR("habmm socket send failed");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            WAIT_FOR_RESPONSE_ACK();
            if (0 != hypvpp_session->time_out)
            {
               HYP_VPP_MSG_ERROR("device queue %s buf failed, wait ACK timeout",
                     port == VPP_PORT_INPUT? "input" : "output");
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               status = translate_habmm_to_hvfe(hypvpp_session, msg_ret_id,
                     (void*)&hypvpp_session->hypvpp_cmd_resp, sizeof(msg), (void*)&return_data);

               if (HYPVPP_STATUS_SUCCESS != status)
               {
                  HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
               }
               else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
               {
                  HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                        HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
                  status = HYPVPP_STATUS_FAIL;
               }
            }
         }
         MM_CriticalSection_Leave(hypvpp_session->hab_lock);
      }
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_get_buf_requirements

  @brief  Hypervisor VPP FE get buf requirement API

  @param [in] context
  @param [in] struct vpp_requirements

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_get_buf_requirements(void* context, struct vpp_requirements *req)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_buf_req_data_type buf_req;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) get buf requirement", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&buf_req, 0, sizeof(hypvpp_buf_req_data_type));

   if (!req)
   {
      HYP_VPP_MSG_ERROR("Requirement is invalid %p", req);
      status = HYPVPP_STATUS_FAIL;
   }
   else
   {
      /** Compose msg data for device open and call send() */
      MM_Time_GetTime( &ts );
      hypvpp_session->hypvpp_time_stamp = (uint32)ts;

      /** Hypervisor abstraction */
      msg.msg_id = HYPVPP_MSGCMD_GET_BUF_REQ;
      msg.message_number = ++hypvpp_session->hypvpp_msg_number;
      msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
      msg.data_size = sizeof(hypvpp_buf_req_data_type);
      msg.version = HYP_VPP_VERSION;
      msg.virtual_channel = hypvpp_session->habmm_handle;

      if (HYPVPP_STATUS_SUCCESS == status)
      {
         msg.pid = (uint32) getpid();
         MM_CriticalSection_Enter(hypvpp_session->hab_lock);
         HYP_VPP_MSG_HIGH("hvfe get buf requirement pid %u", msg.pid);
         if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
         {
            HYP_VPP_MSG_ERROR("habmm socket send failed");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            WAIT_FOR_RESPONSE_ACK();
            if (0 != hypvpp_session->time_out)
            {
               HYP_VPP_MSG_ERROR("device get buf requirement failed, wait ACK timeout");
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               status = translate_habmm_to_hvfe(hypvpp_session, HYPVPP_MSGRESP_GET_BUF_REQ_RET,
                     (void*)&hypvpp_session->hypvpp_cmd_resp, sizeof(msg), (void*)&buf_req);
               if (HYPVPP_STATUS_SUCCESS != status)
               {
                  HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
               }
               else if (HYPVPP_STATUS_SUCCESS != buf_req.return_status)
               {
                  HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                        HYPVPP_STATUS_VERSION_MISMATCH == buf_req.return_status ? "mismatch": "match", buf_req.return_status);
                  status = HYPVPP_STATUS_FAIL;
               }
               else
               {
                  HABMM_MEMCPY(req, &buf_req.buf_req_playload, sizeof(struct vpp_requirements));
               }
            }
         }
         MM_CriticalSection_Leave(hypvpp_session->hab_lock);
      }
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_drain

  @brief  Hypervisor VPP FE drain API

  @param [in] hypvpp_session

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_drain(void* context)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type ret = HYPVPP_STATUS_FAIL;

   if (NULL != hypvpp_session)
   {
      ret = _hyp_vpp_ctl_no_param(hypvpp_session, HYPVPP_MSGCMD_DRAIN);
   }

   return ret;
}

/**===========================================================================

  FUNCTION hyp_vpp_flush

  @brief  Hypervisor VPP FE flush API

  @param [in] hypvpp_session
  @param [in] enum vpp_port

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_flush(void* context, enum vpp_port port)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) flush port:%d", hypvpp_session, port);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device flush and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_FLUSH;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.data_size = sizeof(hypvpp_flush_data_type);
   msg.data.flush_data.port = port;
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe flush pid %u", msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         WAIT_FOR_RESPONSE_ACK();
         if (0 != hypvpp_session->time_out)
         {
            HYP_VPP_MSG_ERROR("device flush failed, wait ACK timeout");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            status = translate_habmm_to_hvfe(hypvpp_session,
                  HYPVPP_MSGRESP_FLUSH_RET,
                  (void*)&hypvpp_session->hypvpp_cmd_resp,
                  sizeof(msg),
                  (void*)&return_data);
            if (HYPVPP_STATUS_SUCCESS != status)
            {
               HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
            }
            else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
            {
               HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                     HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               HYP_VPP_MSG_INFO("flush port %d return successfully", port);

            }
         }
      }
      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_reconfigure

  @brief  Hypervisor VPP FE reconfigure API

  @param [in] context
  @param [in] struct vpp_port_param input_param
  @param [in] struct vpp_port_param output_param

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_reconfigure(void* context, struct vpp_port_param input_param,
                                 struct vpp_port_param output_param)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) reconfig", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device flush and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_RECONFIG;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.data_size = sizeof(hypvpp_reconfig_data_type);
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;
   msg.data.reconfig_data.input_param = input_param;
   msg.data.reconfig_data.output_param = output_param;

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe reconfig pid %u", msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         WAIT_FOR_RESPONSE_ACK();
         if (0 != hypvpp_session->time_out)
         {
            HYP_VPP_MSG_ERROR("device reconfig failed, wait ACK timeout");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            status = translate_habmm_to_hvfe(hypvpp_session,
                  HYPVPP_MSGRESP_RECONFIG_RET,
                  (void*)&hypvpp_session->hypvpp_cmd_resp,
                  sizeof(msg),
                  (void*)&return_data);
            if (HYPVPP_STATUS_SUCCESS != status)
            {
               HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
            }
            else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
            {
               HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                     HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               HYP_VPP_MSG_INFO("VPP reconfig return successfully");

            }
         }
      }
      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

/**===========================================================================

  FUNCTION hyp_vpp_set_vid_prop

  @brief  Hypervisor VPP FE set video property API

  @param [in] context
  @param [in] struct vpp_property

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hyp_vpp_set_vid_prop(void* context, struct video_property prop)
{
   hypvpp_session_t* hypvpp_session = (hypvpp_session_t*)context;
   hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvpp_return_data_type return_data;
   unsigned long ts = 0;

   HYP_VPP_MSG_INFO("hypvpp(%p) set video property", hypvpp_session);

   VALID_HANDLE_RET_IF_FAIL(hypvpp_session);

   HABMM_MEMSET(&return_data, 0, sizeof(hypvpp_return_data_type));

   /** Compose msg data for device set video property and call send() */
   MM_Time_GetTime( &ts );
   hypvpp_session->hypvpp_time_stamp = (uint32)ts;

   /** Hypervisor abstraction */
   msg.msg_id = HYPVPP_MSGCMD_SET_VID_PROP;
   msg.message_number = ++hypvpp_session->hypvpp_msg_number;
   msg.time_stamp_ns = hypvpp_session->hypvpp_time_stamp;
   msg.data_size = sizeof(hypvpp_vid_prop_data_type);
   msg.version = HYP_VPP_VERSION;
   msg.virtual_channel = hypvpp_session->habmm_handle;
   msg.data.prop_data.prop = prop;

   if (HYPVPP_STATUS_SUCCESS == status)
   {
      msg.pid = (uint32) getpid();
      MM_CriticalSection_Enter(hypvpp_session->hab_lock);
      HYP_VPP_MSG_HIGH("hvfe set video property %u", msg.pid);
      if (0 != hypvpp_session->habmm_if.pfSend(hypvpp_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
      {
         HYP_VPP_MSG_ERROR("habmm socket send failed");
         status = HYPVPP_STATUS_FAIL;
      }
      else
      {
         WAIT_FOR_RESPONSE_ACK();
         if (0 != hypvpp_session->time_out)
         {
            HYP_VPP_MSG_ERROR("device set video property failed, wait ACK timeout");
            status = HYPVPP_STATUS_FAIL;
         }
         else
         {
            status = translate_habmm_to_hvfe(hypvpp_session,
                  HYPVPP_MSGRESP_SET_VID_PROP_RET,
                  (void*)&hypvpp_session->hypvpp_cmd_resp,
                  sizeof(msg),
                  (void*)&return_data);
            if (HYPVPP_STATUS_SUCCESS != status)
            {
               HYP_VPP_MSG_ERROR("translate from habmm to FE failed with status %d", status);
            }
            else if (HYPVPP_STATUS_SUCCESS != return_data.return_status)
            {
               HYP_VPP_MSG_ERROR("device returned error, version %s, ret status %d",
                     HYPVPP_STATUS_VERSION_MISMATCH == return_data.return_status ? "mismatch": "match", return_data.return_status);
               status = HYPVPP_STATUS_FAIL;
            }
            else
            {
               HYP_VPP_MSG_INFO("VPP set video property return successfully");

            }
         }
      }
      MM_CriticalSection_Leave(hypvpp_session->hab_lock);
   }

   return status;
}

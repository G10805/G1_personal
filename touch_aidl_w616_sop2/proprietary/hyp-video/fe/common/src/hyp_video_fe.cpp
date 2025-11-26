/*===========================================================================

*//** @file hyp_video_fe.c
  This file provides interface functions for video FE native driver and HAB
  with core video FE driver

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/1.0/fe/common/src/hyp_video_fe.c#10 $

when        who        what, where, why
--------   --------    -------------------------------------------------------
06/30/24    bf         Enhance debug_level setting mechanism under linux
05/20/24    tg         Add hab recv timeout to check if need to exit
03/13/24    gh         Wakeup hvfe_event_cb_thread to exit when videoCore paused
12/22/23    bh         Fix alloc-free-mismatch caused memory leakage issue
08/30/23    nb         Fix compilation errors due to additon of new compiler flags
06/22/23    mm         Add flush lock to avoid race condition issue
06/02/23    mm         Fix stop time out issue
11/28/22    tg         Fix timeout to make WAIT_FOR_RESPONSE_ACK really can time out
11/02/22    sk         Use flag and change header path for kernel 5.15
06/04/22    rq         support 8255 metadata backward compatible with Codec2.1.0
05/10/22    fa         add return status value log when device open fail
02/14/22    sj         Synchronize lock handling
07/07/21    sj         Fix KW issues
05/27/21    sh         Guard passthrough structure with Linux specific macro
04/01/21    sh         Bringup video on RedBend Hypervisor
08/27/20    sh         Bringup video decode using codec2
09/09/19    sm         Clean up resources on device open fail
03/14/19    sm         Handle stop cmd for encoder
02/15/19    rz         Use internal buffer to handle EOS
01/30/19    rz         Add decoder dynamic input buffer mode
09/19/18    sm         Fix NULL pointer dereferencing
09/07/18    aw         Update retry mechanism to use infinite retry
08/21/18    aw         Fix Klockwork P1, compilation and MISRA warning
08/07/18    sm         Update errno for hab recv retry mechanism
06/12/18    sm         Use a common macro for memset
06/05/18    sm         Update hypervisor session fields to be more readable
03/08/18    sm         Add HAB API retry mechanism
02/20/18    sm         Implement passthrough in QX FE and BE hypervisor
01/26/18    sm         Fix logging in ioctl error condition
01/18/18    sm         Add support for passthrough mode feature
01/16/18    sm         Populate HAB virtual channel as part of the HAB messages
11/23/17    sm         Update return type from MM HAB API
09/20/17    sm         Update video hypervisor emulation macro
09/18/17    sm         Use official habmm header
07/25/17    sm         Fix LA logging
06/30/17    aw         Add support for version validation between FE and BE
06/28/17    aw         Unify and update all logs in hyp-video
06/23/17    sm         Streamline hypervisor context structure and definitions
05/08/17    sm         Update for new hyp-video architecture
04/03/17    sm         Add support for Video input FE-BE
02/02/17    hl         Support video encode
01/30/17    sm         Update BE-FE message passing
09/29/16    hl         Add ppga memory to test hypervisor LA
08/16/16    hl         Fix for android compile
08/16/16    rz         Dynamically loading hab library
08/08/16    rz         Use official released habmm.h
08/03/16    rz         Add dummmy read/write for client dynamically loading FE or native ioctl lib
07/27/16    rz         Fix csim hypervisor tests
07/26/16    rz         Add pid for hab emulation in metal enviornment
07/15/16    hl         Add code to support LA target
07/08/16    hl         Isolate video data from ioctl
06/22/16    henryl     Add dynamic buffer mode support
06/20/16    henryl     Clean up and add error messages
06/01/16    hl         Add FE and BE to support Hypervisor interface

============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#ifdef __QNXNTO__
#include "AEEStdDef.h"
#endif
#include "hyp_video_fe.h"
#include "hyp_videopriv_fe.h"
#include "hyp_videopriv.h"
#include "hyp_vidc_inf.h"
#include "hyp_vidc_types.h"
#ifdef WIN32
#include "ioctlServerWinApi.h"
#endif

#include "stdlib.h"
#include "hyp_video.h"

#include "MMThread.h"
#include "hyp_video_fe_translation.h"
#include "MMTimer.h"
#include "MMCriticalSection.h"
#include "hyp_buffer_manager.h"
#include "hyp_debug.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if defined(_ANDROID_) || defined(_LINUX_)
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_vidc_utils.h>
#else
#include <media/msm_vidc_utils.h>
#endif
#endif

#ifdef __QNXNTO__
#include <process.h>
#endif

#define WAIT_FOR_RESPONSE_ACK() \
{ \
    MM_HANDLE MsgSetHandle = 0; \
    hypv_session->time_out = 0; \
    MM_SignalQ_TimedWait ( hypv_session->habmm_queue_info.MsgSignalQ   \
                          [HABMM_SIGNAL_RESPONSE_ACK].stateSignalQ,  \
                           HVFE_TIMEOUT_INTERVAL_IN_MS,  \
                          (void **)&MsgSetHandle, &hypv_session->time_out ); \
}

int debug_level;
static hyp_target_variant get_target_variant();
static int hypvResponseHandler(void*);
static int hypvCallbackEventHandler(void*);

static hypv_status_type start_response_handler(hypv_session_t* hypv_session);
static hypv_status_type stop_response_handler(hypv_session_t* hypv_session);

/**===========================================================================

FUNCTION hyp_device_open

@brief  Hypervisor FE open function that interface with HAB

@param [in] name of the component
@param [in] hyp callback pointer

@dependencies
  None

@return
  Returns hypervisor session pointer

===========================================================================*/
hypv_session_t* hyp_device_open(char* name, hypv_callback_t* callback)
{
   hypv_session_t* hypv_session = NULL;
   hypv_status_type status = HYPV_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvideo_open_data_type return_open;
   boolean bLoadTestHab = FALSE;

   if (NULL == name || NULL == callback)
   {
      HYP_VIDEO_MSG_ERROR("NULL server/device name or callback, callback = 0x%p name = %s",
              callback, name);
      return NULL;
   }

   hypv_session = (hypv_session_t *)HABMM_MALLOC(sizeof(hypv_session_t));
   if (hypv_session == NULL)
   {
      HYP_VIDEO_MSG_ERROR("malloc hypv session failed");
      return NULL;
   }
   HABMM_MEMSET(hypv_session, 0, sizeof(hypv_session_t));

   hypv_session->target_variant = get_target_variant();

   if ( !access((char *)VIDEO_HYP_EMULATION_SIGNATURE, R_OK))
   {
      // test internal socket HAB in QNX
      bLoadTestHab = TRUE;
   }

   if ( bLoadTestHab )
   {
      hypv_session->dl_handle = dlopen(VIDEO_HYP_EMULATION_HAB_LIB, RTLD_LOCAL );
   }
   else
   {
#if defined(__QNXNTO__) || defined(WIN32)
      hypv_session->dl_handle = dlopen(HAB_LIB, RTLD_LOCAL);
#else
      hypv_session->dl_handle = dlopen(HAB_LIB, RTLD_NOW);
#endif
   }

   if (NULL == hypv_session->dl_handle)
   {
      HYP_VIDEO_MSG_ERROR("dlopen load hab lib %s failed, bLoadTestHab %d",
                      bLoadTestHab?VIDEO_HYP_EMULATION_HAB_LIB:HAB_LIB, bLoadTestHab);
      return NULL;
   }
   else
   {
      HYP_VIDEO_MSG_HIGH("dlopen load hab lib %s successful, bLoadTestHab %d",
                      bLoadTestHab?VIDEO_HYP_EMULATION_HAB_LIB:HAB_LIB, bLoadTestHab);
   }

   hypv_session->habmm_if.pfOpen = (hyp_habmm_socket_open) dlsym(hypv_session->dl_handle, "habmm_socket_open");
   hypv_session->habmm_if.pfClose = (hyp_habmm_socket_close) dlsym(hypv_session->dl_handle, "habmm_socket_close");
   hypv_session->habmm_if.pfSend = (hyp_habmm_socket_send) dlsym(hypv_session->dl_handle, "habmm_socket_send");
   hypv_session->habmm_if.pfRecv = (hyp_habmm_socket_recv) dlsym(hypv_session->dl_handle, "habmm_socket_recv");
   hypv_session->habmm_if.pfImport = (hyp_habmm_import) dlsym(hypv_session->dl_handle, "habmm_import");
   hypv_session->habmm_if.pfExport = (hyp_habmm_export) dlsym(hypv_session->dl_handle, "habmm_export");
   hypv_session->habmm_if.pfUnImport = (hyp_habmm_unimport) dlsym(hypv_session->dl_handle, "habmm_unimport");
   hypv_session->habmm_if.pfUnExport = (hyp_habmm_unexport) dlsym(hypv_session->dl_handle, "habmm_unexport");

   if (!hypv_session->habmm_if.pfOpen ||
        !hypv_session->habmm_if.pfClose ||
        !hypv_session->habmm_if.pfSend ||
        !hypv_session->habmm_if.pfRecv ||
        !hypv_session->habmm_if.pfImport ||
        !hypv_session->habmm_if.pfExport ||
        !hypv_session->habmm_if.pfUnImport ||
        !hypv_session->habmm_if.pfUnExport)
   {
      HYP_VIDEO_MSG_ERROR("dlsym hab lib failed");
      dlclose(hypv_session->dl_handle);
      hypv_session->dl_handle = NULL;
      return NULL;
   }

   if (0 != pthread_mutex_init(&hypv_session->event_handler_mutex, NULL))
   {
      HYP_VIDEO_MSG_ERROR("failed to init event handler mutex");
      status = HYPV_STATUS_FAIL;
   }
   else if (0 != pthread_cond_init(&hypv_session->event_handler_cond, NULL))
   {
      HYP_VIDEO_MSG_ERROR("failed to init event handler cond");
      status = HYPV_STATUS_FAIL;
   }
   else if (0 != MM_CriticalSection_Create (&hypv_session->hEnterLock))
   {
      HYP_VIDEO_MSG_ERROR("failed to create critical section for device ioctl");
      status = HYPV_STATUS_FAIL;
   }
   else if (0 != MM_CriticalSection_Create (&hypv_session->flush_lock))
   {
      HYP_VIDEO_MSG_ERROR("failed to create critical section for flush");
      status = HYPV_STATUS_FAIL;
   }

   if (HYPV_STATUS_SUCCESS == status)
   {
      if (0 != habmm_init_queue(&hypv_session->habmm_queue_info, MAX_MSG_QUEUE_SIZE, NUM_HVFE_MSG_QUEUE, NUM_HVFE_SIGNAL))
      {
         HYP_VIDEO_MSG_ERROR("init queue failed");
         status = HYPV_STATUS_FAIL;
      }
      else
      {
         unsigned long ts = 0;

         if (0 != hypv_session->habmm_if.pfOpen(&hypv_session->habmm_handle, MM_VID, 0, 0))
         {
            HYP_VIDEO_MSG_ERROR("habmm socket open failed");
            status = HYPV_STATUS_FAIL;
         }
         else
         {
            // open connection with registered device
            hypv_session->hvfe_client_cb = *callback;
            if ( HYPV_STATUS_SUCCESS != (status = start_response_handler(hypv_session)))
            {
               HYP_VIDEO_MSG_ERROR("start response handler failed");
            }
            else
            {
               /** Compose msg data for device open and call send() */
               MM_Time_GetTime( &ts );
               hypv_session->hypvid_time_stamp = (uint32)ts;

               /** Hypervisor abstraction */
               msg.msg_id = HYPVIDEO_MSGCMD_OPEN;
               msg.message_number = ++hypv_session->hypvid_msg_number;
               msg.time_stamp_ns = hypv_session->hypvid_time_stamp;
               msg.data_size = sizeof(hypvideo_open_data_type);
               msg.version = HYP_VIDEO_VERSION;
               msg.virtual_channel = hypv_session->habmm_handle;
               msg.data.open_data.hyp_plt_id = ((fe_io_session_t *)callback->data)->hyp_plt_id;
               HYP_VIDEO_MSG_HIGH("hyp_plt_id %d", msg.data.open_data.hyp_plt_id);
               if (0 == strncmp(name, VDEC_DEVICE, MAX_DEVICE_NAME_LEN))
               {
                   msg.data.open_data.video_session = VIDEO_SESSION_DECODE;
                   hypv_session->video_session = VIDEO_SESSION_DECODE;
               }
               else if (0 == strncmp(name, VENC_DEVICE, MAX_DEVICE_NAME_LEN))
               {
                   msg.data.open_data.video_session = VIDEO_SESSION_ENCODE;
                   hypv_session->video_session = VIDEO_SESSION_ENCODE;
               }
               else if (0 == strncmp(name, VINPUT_DEVICE, MAX_DEVICE_NAME_LEN))
               {
                   msg.data.open_data.video_session = VIDEO_SESSION_VINPUT;
                   hypv_session->video_session = VIDEO_SESSION_VINPUT;
               }
               else
               {
                   msg.data.open_data.video_session = VIDEO_SESSION_UNKNOWN;
                   HYP_VIDEO_MSG_ERROR("device open failed, unknwon session");
                   status = HYPV_STATUS_FAIL;
               }
               if (HYPV_STATUS_SUCCESS == status)
               {
#ifndef WIN32
                   msg.pid = (uint32) getpid();
                   HYP_VIDEO_MSG_HIGH("hvfe open pid %u", msg.pid);
#endif

                   if (0 != hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
                   {
                       HYP_VIDEO_MSG_ERROR("habmm socket send failed");
                       status = HYPV_STATUS_FAIL;
                   }
                   else
                   {
                       WAIT_FOR_RESPONSE_ACK();
                       if (0 != hypv_session->time_out)
                       {
                           HYP_VIDEO_MSG_ERROR("device open failed, wait ACK timeout");
                           status = HYPV_STATUS_FAIL;
                       }
                       else
                       {
                           status = translate_habmm_to_hvfe(hypv_session,
                           HYPVIDEO_MSGRESP_OPEN_RET,
                           (void*)&hypv_session->hypvCmdResponse,
                           sizeof(msg),
                           (void*)&return_open);

                           if (HYPV_STATUS_SUCCESS != status)
                           {
                               HYP_VIDEO_MSG_ERROR("translate from habmm to FE failed with status %d", status);
                           }
                           else if (HYPV_STATUS_SUCCESS != return_open.return_status)
                           {
                               HYP_VIDEO_MSG_ERROR("device open returned error, version %s, ret status %d",
                                       HYPV_STATUS_VERSION_MISMATCH == return_open.return_status ? "mismatch": "match", return_open.return_status);
                               status = HYPV_STATUS_FAIL;
                           }
                           else
                           {
                                hypv_session->handle_64b =  return_open.return_io_handle;
                                hypv_session->passthrough_mode = return_open.passthrough_mode;
                                hypv_session->hyp_platform = return_open.hyp_plt_id;
                                HYP_VIDEO_MSG_HIGH("passthrough mode %d platform %d", hypv_session->passthrough_mode, hypv_session->hyp_platform);
                           }
                       }
                   }
                }
             }
         }
      }
   }

   if ( HYPV_STATUS_SUCCESS != status )
   {
      /** Release all the allocated resources as device_close will not be called
      when device_open fails
      */
      hypv_session->exit_resp_handler = TRUE;
      pthread_mutex_lock(&hypv_session->event_handler_mutex);
      pthread_cond_signal(&hypv_session->event_handler_cond);
      pthread_mutex_unlock(&hypv_session->event_handler_mutex);
      hypv_session->habmm_if.pfClose(hypv_session->habmm_handle);

      status = stop_response_handler(hypv_session);
      if (HYPV_STATUS_SUCCESS != status)
      {
         HYP_VIDEO_MSG_ERROR("stop_response_handler failed with status %d", status);
      }

      pthread_mutex_destroy(&hypv_session->event_handler_mutex);
      pthread_cond_destroy(&hypv_session->event_handler_cond);

      if (NULL != hypv_session->hEnterLock)
      {
         MM_CriticalSection_Release (hypv_session->hEnterLock);
         hypv_session->hEnterLock = NULL;
      }
      if (NULL != hypv_session->flush_lock)
      {
         MM_CriticalSection_Release (hypv_session->flush_lock);
         hypv_session->flush_lock = NULL;
      }

      habmm_deinit_queue(&hypv_session->habmm_queue_info);
      if (dlclose(hypv_session->dl_handle))
      {
         char *dl_error = NULL;
         dl_error = (char *)dlerror();
         HYP_VIDEO_MSG_ERROR("dlclose hab lib failed %s", dl_error);
      }
      else
      {
         HYP_VIDEO_MSG_HIGH("dlclose hab lib successful");
      }

      HABMM_FREE(hypv_session);
   }

   return hypv_session;
}

/**===========================================================================

FUNCTION hyp_device_close

@brief  Hypervisor FE close function that interface with HAB

@param [in] hypervisor session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_device_close(hypv_session_t* hypv_session)
{
   hypv_status_type status = HYPV_STATUS_SUCCESS;
   habmm_msg_desc_t msg;
   hypvideo_close_data_type ret_close;
   unsigned long ts = 0;

   if (hypv_session == NULL)
   {
      HYP_VIDEO_MSG_ERROR("hypv session is NULL");
      HABMM_FREE(hypv_session);
      return HYPV_STATUS_FAIL;
   }

   /** Release Daemon BE thread and msg & callback queue */
   MM_Time_GetTime( &ts );

   /** Hypervisor abstraction */
   msg.msg_id = HYPVIDEO_MSGCMD_CLOSE;
   msg.message_number = ++hypv_session->hypvid_msg_number;
   msg.time_stamp_ns = (uint32)ts;
   msg.data.close_data.io_handle = hypv_session->handle_64b;
   msg.data_size = sizeof(hypvideo_close_data_type);
   msg.virtual_channel = hypv_session->habmm_handle;

   if (0 != hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg, sizeof(msg), 0 /*flags*/))
   {
      status = HYPV_STATUS_FAIL;
   }
   else
   {
      WAIT_FOR_RESPONSE_ACK();
      if (0 != hypv_session->time_out)
      {
         HYP_VIDEO_MSG_ERROR("wait ACK timeout");
         status = HYPV_STATUS_FAIL;
      }
      else
      {
         status = translate_habmm_to_hvfe(hypv_session,
            HYPVIDEO_MSGRESP_CLOSE_RET,
            (void*)&hypv_session->hypvCmdResponse,
            sizeof(msg),
            (void*)&ret_close);
         if (HYPV_STATUS_SUCCESS != status)
         {
            HYP_VIDEO_MSG_ERROR("failed translate habmm to FE with status=%d", status);
         }
         else if (HYPV_STATUS_SUCCESS != ret_close.return_status)
         {
            HYP_VIDEO_MSG_ERROR("device close failed status=%d",ret_close.return_status);
            status = ret_close.return_status;
         }
         else
         {
            HYP_VIDEO_MSG_HIGH("device close successfully");
         }
      }
   }

   hypv_session->exit_resp_handler = TRUE;
   HYP_VIDEO_MSG_INFO("wakeup event thread");
   pthread_mutex_lock(&hypv_session->event_handler_mutex);
   pthread_cond_signal(&hypv_session->event_handler_cond);
   pthread_mutex_unlock(&hypv_session->event_handler_mutex);
   stop_response_handler(hypv_session);

   hypv_map_cleanup(&hypv_session->habmm_if, &hypv_session->lookup_queue);
   hypv_map_cleanup_ext(&hypv_session->habmm_if, &hypv_session->lookup_queue_ex);
   hypv_session->habmm_if.pfClose(hypv_session->habmm_handle);

   pthread_mutex_destroy(&hypv_session->event_handler_mutex);
   pthread_cond_destroy(&hypv_session->event_handler_cond);

   if (NULL != hypv_session->hEnterLock)
   {
      MM_CriticalSection_Release (hypv_session->hEnterLock);
      hypv_session->hEnterLock = NULL;
   }
   if (NULL != hypv_session->flush_lock)
   {
      MM_CriticalSection_Release (hypv_session->flush_lock);
      hypv_session->flush_lock = NULL;
   }
   habmm_deinit_queue(&hypv_session->habmm_queue_info);

   if (dlclose( hypv_session->dl_handle ))
   {
      char *dl_error = NULL;
      dl_error = (char *)dlerror();
      HYP_VIDEO_MSG_ERROR("dlclose hab lib failed %s", dl_error);
   }
   else
   {
      HYP_VIDEO_MSG_HIGH("dlclose hab lib successful");
   }

   hypv_session->dl_handle = NULL;

   HABMM_FREE(hypv_session);

   return status;
}

/**===========================================================================

FUNCTION hyp_device_ioctl

@brief  Hypervisor FE ioctl function that interface with HAB

@param [in] hypervisor session pointer
@param [in] cmd
@param [in] input buffer pointer
@param [in] input buffer size
@param [out] output buffer pointer
@param [in] output buffer size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_device_ioctl
(
 hypv_session_t* hypv_session,
 uint32 aCommand,
 uint8 *aInBuf,
 uint32 aInBufSize,
 uint8* aOutBuf,
 uint32 aOutBufSize
 )
{

   unsigned long ts = 0;
   hypv_status_type status = HYPV_STATUS_SUCCESS;
   habmm_msg_desc_t msg;

   if (hypv_session == NULL)
   {
      HYP_VIDEO_MSG_ERROR("hypv session is null");
      return HYPV_STATUS_FAIL;
   }
   // the device_ioctl can be called from different thread
   // so protect it to ensure no race condition

   /** Hypervisor abstraction */
   msg.msg_id = HYPVIDEO_MSGCMD_IOCTL;

   MM_Time_GetTime( &ts );
   msg.message_number = ++hypv_session->hypvid_msg_number;
   msg.time_stamp_ns = (uint32) ts;
   msg.data.ioctl_data.io_handle = hypv_session->handle_64b;
   msg.data.ioctl_data.vidc_ioctl = aCommand;
   msg.data_size = GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(aInBufSize);
   msg.virtual_channel = hypv_session->habmm_handle;
   if (TRUE == hypv_session->passthrough_mode)
   {
      HABMM_MEMCPY((void*)msg.data.ioctl_data.payload, (void *)aInBuf, aInBufSize);
   }
   else
   {
      status = translate_hvfe_to_habmm( hypv_session,
                                        aCommand,
                                        (void*)aInBuf,
                                        aInBufSize,
                                        (void*)msg.data.ioctl_data.payload);
      if (0 != status)
      {
         if ((aCommand == VIDC_IOCTL_FREE_BUFFER)  &&
             ((VIDEO_SESSION_DECODE == hypv_session->video_session) ||
              (VIDEO_SESSION_ENCODE == hypv_session->video_session)))
         {
            status = HYPV_STATUS_SUCCESS;
         }
         return(status);
      }
   }

   switch (aCommand)
   {
      case VIDC_IOCTL_FREE_BUFFER:
      case VIDC_IOCTL_SET_BUFFER:
      case VIDC_IOCTL_ALLOCATE_BUFFER:
      {
         aInBufSize = sizeof(vidc_buffer_info_64b_type);
         msg.data_size = GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(aInBufSize);
         break;
      }
      case VIDC_IOCTL_EMPTY_INPUT_BUFFER:
      case VIDC_IOCTL_FILL_OUTPUT_BUFFER:
      {
         aInBufSize = sizeof(vidc_frame_data_64b_type);
         msg.data_size = GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(aInBufSize);
         break;
      }
      default:
         HYP_VIDEO_MSG_INFO("ioctl command - 0x%x", aCommand);
         break;
   }

   MM_CriticalSection_Enter(hypv_session->hEnterLock);
   if (0 != hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg, sizeof(habmm_msg_desc_t), 0 /*flags*/))
   {
      HYP_VIDEO_MSG_ERROR("habmm socket send failed");
      status = HYPV_STATUS_FAIL;
   }
   else
   {
      WAIT_FOR_RESPONSE_ACK();
      if (0 != hypv_session->time_out)
      {
         HYP_VIDEO_MSG_ERROR("Timeout occurred at device ioctl aCommand 0x%x", aCommand);
         status = HYPV_STATUS_FAIL;
      }
      else
      {
         status = hypv_session->hypvCmdResponse.ioctl_data.return_value;

         if (HYPV_STATUS_SUCCESS == status)
         {
            if ((aOutBuf != NULL) && (aOutBufSize != 0))
            {
               if (TRUE == hypv_session->passthrough_mode)
               {
                   HABMM_MEMCPY((void *)aOutBuf, (void *)&hypv_session->hypvCmdResponse.ioctl_data.payload, aOutBufSize);
               }
               else
               {
                  status = translate_habmm_to_hvfe(hypv_session,
                                                   HYPVIDEO_MSGRESP_IOCTL_RET,
                                                   (void*)&hypv_session->hypvCmdResponse,
                                                   aOutBufSize,
                                                   (void*)aOutBuf);
                  if (HYPV_STATUS_SUCCESS != status)
                  {
                     HYP_VIDEO_MSG_ERROR("failed translate from habmm to FE with status %d",status);
                  }
               }
            }

            if ((FALSE == hypv_session->passthrough_mode) &&
                (aCommand == VIDC_IOCTL_FREE_BUFFER)  &&
                ((VIDEO_SESSION_DECODE == hypv_session->video_session) ||
                (VIDEO_SESSION_ENCODE == hypv_session->video_session)))
            {
               // free the hypv map after submitting to the BE
               vidc_buffer_info_type* pCmdBuffer;
               pCmdBuffer = (vidc_buffer_info_type*)aInBuf;
               if (pCmdBuffer->buf_addr != 0)
               {
                  hypv_map_entry_t *find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pCmdBuffer->buf_addr);
                  if (NULL == find_entry)
                  {
                      HYP_VIDEO_MSG_ERROR("failed to find an entry node for buf addr 0x%p",pCmdBuffer->buf_addr);
                  }
                  else
                  {
                      if (HYPV_STATUS_SUCCESS != hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, pCmdBuffer->buf_addr, find_entry->bufferid))
                      {
                         HYP_VIDEO_MSG_ERROR("failed to free the ioctl map on buf address 0x%p",
                                              pCmdBuffer->buf_addr);
                         status = HYPV_STATUS_FAIL;
                      }
                  }
               }
               if ( !pCmdBuffer->contiguous && pCmdBuffer->extradata_buf_addr != 0)
               {
                  hypv_map_entry_t *find_entry = hypv_map_to_lookup(&hypv_session->lookup_queue, pCmdBuffer->extradata_buf_addr);
                  if (NULL == find_entry)
                  {
                      HYP_VIDEO_MSG_ERROR("failed to find an entry node for extradata_buf_addr 0x%p",pCmdBuffer->extradata_buf_addr);
                  }
                  else
                  {
                      if (HYPV_STATUS_SUCCESS != hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue, pCmdBuffer->extradata_buf_addr, find_entry->bufferid))
                      {
                         HYP_VIDEO_MSG_ERROR("failed to free the ioctl map on extradata buf address 0x%p",
                                              pCmdBuffer->extradata_buf_addr);
                         status = HYPV_STATUS_FAIL;
                      }
                  }
               }
            }
         }
         else
         {
            HYP_VIDEO_MSG_ERROR("aCommand[0x%x] failed with status[0x%x]", aCommand, (unsigned int)status);
         }
      }
   }
   MM_CriticalSection_Leave(hypv_session->hEnterLock);
   return(status);
}

/**===========================================================================

FUNCTION hyp_device_read

@brief  Hypervisor FE read function that interface with HAB

@param [in] hypervisor session pointer
@param [out] buffer pointer
@param [in] buffer size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_device_read
(
   hypv_session_t   *hypv_session,
   uint8             *aBuf,
   uint32            aBufSize
)
{
   UNUSED(hypv_session);
   UNUSED(aBuf);
   UNUSED(aBufSize);
   HYP_VIDEO_MSG_ERROR("device read unsupported");
   return HYPV_STATUS_FAIL;
}

/**===========================================================================

FUNCTION hyp_device_write

@brief  Hypervisor FE write function that interface with HAB

@param [in] hypervisor session pointer
@param [in] buffer pointer
@param [in] buffer size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_device_write
(
   hypv_session_t   *hypv_session,
   uint8             *aBuf,
   uint32            aBufSize
)
{
   UNUSED(hypv_session);
   UNUSED(aBuf);
   UNUSED(aBufSize);
   HYP_VIDEO_MSG_ERROR("device write unsupported");
   return HYPV_STATUS_FAIL;
}

/**===========================================================================

FUNCTION hyp_device_ping

@brief  Hypervisor FE ping function that interface with HAB. This indicate the
        BE to return immediately.

@param [in] hypervisor session pointer
@param [in] cmd
@param [in] buffer pointer
@param [in] buffer size

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hyp_device_ping
(
    hypv_session_t* hypv_session,
    uint32 aCommand,
    uint8 *aBuf,
    uint32 aBufSize
)
{
   unsigned long ts = 0;
   hypv_status_type status = HYPV_STATUS_SUCCESS;
   habmm_msg_desc_t msg;

   if (hypv_session == NULL)
   {
      HYP_VIDEO_MSG_ERROR("hypv session is NULL");
      return HYPV_STATUS_FAIL;
   }
   // the device_ioctl and device_ping can be called from different thread
   // so protect it to ensure no race condition
   if (VIDC_PING_GET_CALLBACK == aCommand)
   {
       msg.msg_id = HYPVIDEO_MSGCMD_IOCTL;

       MM_Time_GetTime( &ts );
       msg.message_number = ++hypv_session->hypvid_msg_number;
       msg.time_stamp_ns = (uint32) ts;
       msg.data.ioctl_data.io_handle = hypv_session->handle_64b;
       msg.data.ioctl_data.vidc_ioctl = aCommand;
       msg.data_size = GET_MSG_DATASIZE_FROM_IOCTL_PAYLOAD_SIZE(aBufSize);
       msg.virtual_channel = hypv_session->habmm_handle;
       if (TRUE == hypv_session->passthrough_mode)
       {
           HABMM_MEMCPY((void*)msg.data.ioctl_data.payload, (void *)aBuf, aBufSize);
       }
       else
       {
           status = translate_hvfe_to_habmm( hypv_session,
                                             aCommand,
                                             (void*)aBuf,
                                             aBufSize,
                                             (void*)msg.data.ioctl_data.payload);
           if (HYPV_STATUS_SUCCESS != status)
           {
               HYP_VIDEO_MSG_ERROR("failed translate from FE to habmm with status %d", status);
           }
       }
       if (HYPV_STATUS_SUCCESS == status)
       {
           MM_CriticalSection_Enter(hypv_session->hEnterLock);
           if (0 != hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg, sizeof(hypvideo_msg_data_type), 0 /*flags*/))
           {
               HYP_VIDEO_MSG_ERROR("habmm socket send failed");
               status = HYPV_STATUS_FAIL;
           }
           else
           {
               WAIT_FOR_RESPONSE_ACK();
               if (0 != hypv_session->time_out)
               {
                   HYP_VIDEO_MSG_ERROR("Timeout occurred aCommand 0x%x", aCommand);
                   status = HYPV_STATUS_FAIL;
               }

               if (HYPV_STATUS_SUCCESS == status)
               {
                   status = hypv_session->hypvCmdResponse.ioctl_data.return_value;
                   if (HYPV_STATUS_SUCCESS != status)
                   {
                       HYP_VIDEO_MSG_ERROR("aCommand[0x%x] failed with status[0x%x]", aCommand, (unsigned int)status);
                   }
               }
           }
           MM_CriticalSection_Leave(hypv_session->hEnterLock);
       }
   }
   else
   {
       HYP_VIDEO_MSG_ERROR("Unknown PING command[0x%x]", aCommand);
   }

   return(status);
}

/**===========================================================================

FUNCTION start_response_handler

@brief  Hypervisor FE function to invoke HAB response thread handler

@param [in] hypervisor session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type start_response_handler(hypv_session_t* hypv_session)
{
   hypv_session->exit_resp_handler = FALSE;
   if ( MM_Thread_CreateEx ( MM_Thread_DefaultPriority,
      0, hypvResponseHandler, (void*)hypv_session, HYPV_THREAD_STACK_SIZE,
      "hypvFeResponsehandler", (MM_HANDLE*)&hypv_session->hvfe_response_cb_thread ) != 0 )
   {
      HYP_VIDEO_MSG_ERROR("failed to create hyp video FE response handler thread");
      return HYPV_STATUS_FAIL;
   }

   if ( MM_Thread_CreateEx ( MM_Thread_DefaultPriority,
      0, hypvCallbackEventHandler, (void*)hypv_session, HYPV_THREAD_STACK_SIZE,
      "hypvFeEventhandler", (MM_HANDLE*)&hypv_session->hvfe_event_cb_thread ) != 0 )
   {
      HYP_VIDEO_MSG_ERROR("failed to create hyp video FE event handler thread");
      return HYPV_STATUS_FAIL;
   }
   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION stop_response_handler

@brief  Hypervisor FE function to stop HAB response thread handler

@param [in] hypervisor session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type stop_response_handler(hypv_session_t* hypv_session)
{
   int rc;

   // shut down the callback loop and free memory
   if (hypv_session->hvfe_response_cb_thread)
   {
#if defined(__QNXNTO__) || defined(WIN32)
      MM_Thread_Join((MM_HANDLE)((long)hypv_session->hvfe_response_cb_thread), &rc);
      MM_Thread_Release((MM_HANDLE)((long)hypv_session->hvfe_response_cb_thread));
#else
      MM_Thread_Join( hypv_session->hvfe_response_cb_thread, &rc);

      MM_Thread_Release(hypv_session->hvfe_response_cb_thread);
#endif
      hypv_session->hvfe_response_cb_thread = 0;
   }

   if (hypv_session->hvfe_event_cb_thread)
   {
#if defined(__QNXNTO__) || defined(WIN32)
      MM_Thread_Join((MM_HANDLE)hypv_session->hvfe_event_cb_thread, &rc);
      MM_Thread_Release((MM_HANDLE)hypv_session->hvfe_event_cb_thread);
#else
      MM_Thread_Join(hypv_session->hvfe_event_cb_thread, &rc);
      MM_Thread_Release(hypv_session->hvfe_event_cb_thread);
#endif
      hypv_session->hvfe_event_cb_thread = 0;
   }
   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION hypvResponseHandler

@brief  Hypervisor FE HAB response handler

@param [in] hypervisor session pointer

@dependencies
  None

@return
  Returns int

===========================================================================*/
static int hypvResponseHandler(void* pIoSession)
{
   hypv_session_t* hypv_session = (hypv_session_t*)pIoSession;
   int done = 0;
   uint32 size_bytes = 0;
   habmm_msg_desc_t msgBufferNode;
   int32 rc = 0;

   while (!done)
   {
      /*
      ** Retrieve a msg from the queue and finish all the messages in the queue
      */
      while (hypv_session->exit_resp_handler == FALSE)
      {
         size_bytes = sizeof(habmm_msg_desc_t);
         HABMM_MEMSET(&msgBufferNode, 0, sizeof(habmm_msg_desc_t));
         rc = hypv_session->habmm_if.pfRecv(hypv_session->habmm_handle, (void*)&msgBufferNode,
            &size_bytes, HVFE_TIMEOUT_INTERVAL_IN_MS, HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
         if (0 == rc)
         {
            HYP_VIDEO_MSG_LOW("pid=%u msg_id=0x%x msg_num=%u msg_size=%u",
                           msgBufferNode.pid, msgBufferNode.msg_id,
                           msgBufferNode.message_number, msgBufferNode.data_size);
            if (HYPVIDEO_MSGRESP_EVENT == msgBufferNode.msg_id)
            {
               boolean ret;
               if ((TRUE == hypv_session->passthrough_mode) && (HYP_PLATFORM_LA == hypv_session->hyp_platform))
               {
                   HABMM_MEMCPY((void *)&hypv_session->hypvCmdResponse.event_data, (const void *)&msgBufferNode.data.event_data,
                          sizeof(hypvideo_event_data_type));
               }
               else
               {
                   HABMM_MEMCPY((void *)&hypv_session->hypvCmdResponse.event_data, (const void *)&msgBufferNode.data.event_data,
                          sizeof(vidc_drv_msg_info_64b_type));
               }
               pthread_mutex_lock(&hypv_session->event_handler_mutex);
               ret = habmm_enqueue_buffer_list(&hypv_session->habmm_queue_info, HABMM_IO_CALLBACK_INDEX,
                                              (habmm_msg_desc_t *)&msgBufferNode);
               if (TRUE == ret)
               {
                  pthread_cond_signal(&hypv_session->event_handler_cond);
               }
               else
               {
                  HYP_VIDEO_MSG_ERROR("habmm buffer list enqueue failed ");
               }
               pthread_mutex_unlock(&hypv_session->event_handler_mutex);
            }
            else if (HYPVIDEO_MSGRESP_OPEN_RET == msgBufferNode.msg_id)
            {
               HABMM_MEMCPY((void *)&hypv_session->hypvCmdResponse.open_data, (const void *)&msgBufferNode.data.open_data,
                      sizeof(hypvideo_open_data_type));
               MM_Signal_Set(hypv_session->habmm_queue_info.MsgSignalQ[HABMM_SIGNAL_RESPONSE_ACK].stateChangeSignal);
            }
            else if (HYPVIDEO_MSGRESP_CLOSE_RET == msgBufferNode.msg_id)
            {
               HABMM_MEMCPY((void *)&hypv_session->hypvCmdResponse.close_data, (const void *)&msgBufferNode.data.close_data,
                      sizeof(hypvideo_close_data_type));
               MM_Signal_Set(hypv_session->habmm_queue_info.MsgSignalQ[HABMM_SIGNAL_RESPONSE_ACK].stateChangeSignal);
               hypv_session->exit_resp_handler = TRUE;
               // Signal the event queue to exit as well
               pthread_mutex_lock(&hypv_session->event_handler_mutex);
               pthread_cond_signal(&hypv_session->event_handler_cond);
               pthread_mutex_unlock(&hypv_session->event_handler_mutex);
            }
            else if (HYPVIDEO_MSGRESP_IOCTL_RET == msgBufferNode.msg_id)
            {
               HABMM_MEMCPY((void *)&hypv_session->hypvCmdResponse.ioctl_data, (const void *)&msgBufferNode.data.ioctl_data,
                      sizeof(hypvideo_ioctl_data_type));
               MM_Signal_Set(hypv_session->habmm_queue_info.MsgSignalQ[HABMM_SIGNAL_RESPONSE_ACK].stateChangeSignal);
            }
         }
         else if (-EINTR == rc)
         {
            HYP_VIDEO_MSG_HIGH("hab recv EINTR, retry");
            continue;
         }
         else if (-EAGAIN == rc)
         {
            HYP_VIDEO_MSG_HIGH("hab recv EAGAIN, retry");
            continue;
         }
         else if (-ETIMEDOUT == rc)
         {
            // Retry when hab recv time out to check if need to exit to close this video session.
            // See habmm_socket_recv timeout case in Linux kernel hab_msg_dequeue.
            // Here timeout is what we want to avoid the case hab recv always blocking, in which
            // FE already sent HYPVIDEO_MSGCMD_CLOSE to BE but no reply gets back forever. In this
            // case, hyp_video_close would be always blocking on stop_response_handler to wait for
            // exit of hypvResponseHandler thread that's blocking on hab recv forever, and
            // hyp_video_close hold the g_hyp_video_handle_lock that's needed by hyp_video_open,
            // so never could open a new video session in this situation. All is dead.
            HYP_VIDEO_MSG_HIGH("hab recv ETIMEDOUT, retry");
            continue;
         }
         else
         {
            HYP_VIDEO_MSG_ERROR("socket recv failed: hab ret code %d, stop loop", rc);
            hypv_session->exit_resp_handler = TRUE;
            hypv_session->time_out = 1;
            pthread_mutex_lock(&hypv_session->event_handler_mutex);
            pthread_cond_signal(&hypv_session->event_handler_cond);
            pthread_mutex_unlock(&hypv_session->event_handler_mutex);

            MM_Signal_Set(hypv_session->habmm_queue_info.MsgSignalQ[HABMM_SIGNAL_RESPONSE_ACK].stateChangeSignal);

            break;
         }
      }

      if (hypv_session->exit_resp_handler == TRUE)
      {
         done = 1;
      }
   }

   return 0;
}

/**===========================================================================

FUNCTION hypvCallbackEventHandler

@brief  Hypervisor FE HAB callback event handler

@param [in] hypervisor session pointer

@dependencies
  None

@return
  Returns int

===========================================================================*/
static int hypvCallbackEventHandler(void* pIoSession)
{
   hypv_status_type rc = HYPV_STATUS_SUCCESS;
   hypv_session_t* hypv_session = (hypv_session_t*)pIoSession;
   hypvideo_event_data_type eventMsg;
   int done = 0;
   habmm_msg_desc_t *pMsgBufferNode = NULL;

   if (hypv_session == NULL)
   {
      HYP_VIDEO_MSG_ERROR("hypv session is NULL");
      return -1;
   }
   else
   {
      HABMM_MEMSET(&eventMsg, 0, sizeof(hypvideo_event_data_type));
   }
   while (!done)
   {
      /*
      ** Retrieve a msg from the queue and finish all the messages in the queue
      */
      while(FALSE == hypv_session->exit_resp_handler)
      {
         pthread_mutex_lock(&hypv_session->event_handler_mutex);
         pMsgBufferNode = (habmm_msg_desc_t *)habmm_dequeue_buffer_list(&hypv_session->habmm_queue_info,
                           HABMM_IO_CALLBACK_INDEX);
         if (NULL == pMsgBufferNode)
         {
            /* wait for new event */
            pthread_cond_wait(&hypv_session->event_handler_cond, &hypv_session->event_handler_mutex);
         }
         pthread_mutex_unlock(&hypv_session->event_handler_mutex);

         if (NULL != pMsgBufferNode)
         {
            if (HYPVIDEO_MSGRESP_EVENT == pMsgBufferNode->msg_id)
            {
               int size = 0;
               if (TRUE == hypv_session->passthrough_mode)
               {
                  if (HYP_PLATFORM_LA == hypv_session->hyp_platform)
                  {
                      size = sizeof(hypvideo_event_data_type);
                      HABMM_MEMCPY((void *)&eventMsg, &pMsgBufferNode->data.event_data, size);
#ifdef LINUX_PASSTHROUGH
                      switch (eventMsg.v4l2_passthru.event_type)
                      {
                          case VIDC_EVT_RESP_OUTPUT_DONE:
                          case VIDC_EVT_RESP_INPUT_DONE:
                          {
                              hypv_map_entry_ex_t *find_entry = NULL;
                              uint32 index = 0;
                              index = eventMsg.v4l2_passthru.payload.buffer.index;

                              //clear previous stale buffer
                              find_entry = hypv_map_get_entry(&hypv_session->lookup_queue_ex, index, TRUE, eventMsg.v4l2_passthru.payload.buffer.type);
                              if (NULL == find_entry)
                              {
                                  HYP_VIDEO_MSG_INFO("No stale entry node for buf index %d", index);
                              }
                              else
                              {
                                  hypv_map_free_ext(&hypv_session->habmm_if, &hypv_session->lookup_queue_ex, index, find_entry->frame_addr, find_entry->frame_bufferid);
                                  HYP_VIDEO_MSG_INFO("Clearing stale buf index %d buf type %llx", index, (long long)eventMsg.v4l2_passthru.payload.buffer.type);
                              }

                              find_entry = hypv_map_get_entry(&hypv_session->lookup_queue_ex, index, FALSE, eventMsg.v4l2_passthru.payload.buffer.type);
                              if (NULL == find_entry)
                              {
                                  HYP_VIDEO_MSG_ERROR("Failed to find an entry for buff index %d", index);
                              }
                              else
                              {
                                  eventMsg.v4l2_passthru.payload.buffer.m.planes[0].reserved[0] = (uint32)(uintptr_t)find_entry->frame_addr;
                                  eventMsg.v4l2_passthru.payload.buffer.m.planes[0].m.fd = (uint32)(uintptr_t)find_entry->frame_addr;
                                  eventMsg.v4l2_passthru.payload.buffer.m.planes[1].reserved[0] = (uint32)(uintptr_t)find_entry->extradata_addr;
                                  eventMsg.v4l2_passthru.payload.buffer.m.planes[1].m.fd = (uint32)(uintptr_t)find_entry->extradata_addr;
                                  if (eventMsg.v4l2_passthru.payload.buffer.flags & V4L2_BUF_FLAG_READONLY)
                                  {
                                      hypv_map_update_readonly(find_entry, TRUE);
                                  }
                              }

                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT:
                          {
                              HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_SUFFICIENT:
                          {
                              HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_SUFFICIENT event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_FLUSH_DONE:
                          {
                              HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_FLUSH_DONE event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_HW_OVERLOAD:
                          {
                              HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_HW_OVERLOAD event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED:
                          {
                              HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_SYS_ERROR:
                          {
                              HYP_VIDEO_MSG_ERROR("Received V4L2_EVENT_MSM_VIDC_SYS_ERROR event");
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE:
                          {
                              HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE event");
                              hypv_map_entry_ex_t* find_entry = NULL;
                              uint32 bufferid = 0;

                              bufferid = eventMsg.v4l2_passthru.payload.event_data.u.data[0];
                              if (0 != bufferid)
                              {
                                  find_entry = hypv_map_from_lookup_ext(&hypv_session->lookup_queue_ex, bufferid, TRUE);
                                  if (NULL != find_entry)
                                  {
                                      eventMsg.v4l2_passthru.payload.event_data.u.data[0] = (uint8)(uintptr_t)find_entry->frame_addr;
                                      hypv_map_update_readonly(find_entry, FALSE);
                                  }
                                  else
                                  {
                                      HYP_VIDEO_MSG_ERROR("Failed to entry node for bufferid %d in RELEASE event", bufferid);
                                  }
                              }
                              break;
                          }
                          case V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER:
                          {
                              HYP_VIDEO_MSG_INFO("Received V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER event");
                              break;
                          }
                      }
#endif
                  }
                  else
                  {
                      size = sizeof(vidc_drv_msg_info_type);
                      HABMM_MEMCPY((void *)&eventMsg, &pMsgBufferNode->data.event_data, size);
                  }
               }
               else
               {
                  size = sizeof(vidc_drv_msg_info_type);
                  rc = translate_habmm_to_hvfe(hypv_session,
                     HYPVIDEO_MSGRESP_EVENT,
                     (void*)&pMsgBufferNode->data,
                     size,
                     (void*)&eventMsg);
                  if ( rc != HYPV_STATUS_SUCCESS)
                  {
                      HYP_VIDEO_MSG_ERROR("failed to translate from habmm to FE");
                  }
               }
               if ( rc == HYPV_STATUS_SUCCESS)
               {
                  /** call the client callback */
                  hypv_session->hvfe_client_cb.handler((uint8 *)&eventMsg, size,
                                                       (void *) hypv_session->hvfe_client_cb.data);
               }
            }
         }
      }
      if (hypv_session->exit_resp_handler == TRUE)
      {
         done = 1;
      }
   }
   return 0;
}

/**===========================================================================

FUNCTION video_fe_open

@brief  Hypervisor video FE open API

@param [in] video component name
@param [in] flag
@param [in] callback

@dependencies
  None

@return
  Returns HVFE_HANDLE

===========================================================================*/
HVFE_HANDLE video_fe_open( const char* str, int flag, hvfe_callback_t* cb )
{

    int32 rc = 0;
    hypv_status_type hypv_status = HYPV_STATUS_SUCCESS;
    fe_io_session_t *fe_ioss = NULL;

    UNUSED(flag);
#ifdef _LINUX_
    char *env_ptr = getenv("HYPV_DEBUG_LEVEL");
    debug_level = env_ptr ? atoi(env_ptr) : 1;
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    if (property_get("vendor.hypv.debug.level", prop_val, NULL) > 0 && prop_val[0] != '\0' && prop_val[PROPERTY_VALUE_MAX-1] == '\0')
    {
        debug_level = atoi(prop_val);
    }
#elif defined _ANDROID_
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.hypv.debug.level", property_value, "1");
    debug_level = atoi(property_value);
#endif
    HYP_VIDEO_MSG_INFO("enter");
    if (!cb || !cb->handler)
    {
        HYP_VIDEO_MSG_ERROR("cb is NULL");
        return NULL;
    }
    // allocate an io session object
    fe_ioss = (fe_io_session_t *)malloc(sizeof(fe_io_session_t));

    if (fe_ioss == NULL)
    {
        HYP_VIDEO_MSG_ERROR("malloc failed fe_ioss is NULL");
        return NULL;
    }
    HABMM_MEMSET(fe_ioss, 0, sizeof(fe_io_session_t));

    fe_ioss->input_tag_entry = new std::map<uint64, uint64>;

    MM_CriticalSection_Create(&fe_ioss->lock_buffer);

    rc = MM_SignalQ_Create(&fe_ioss->state_synch_obj_q);
    if (rc)
    {
        HYP_VIDEO_MSG_ERROR("MM SignalQ Create failed");
    }
    else
    {
        rc= MM_Signal_Create(fe_ioss->state_synch_obj_q, NULL, NULL, &fe_ioss->state_synch_obj);
        if (rc)
        {
            HYP_VIDEO_MSG_ERROR("MM Signal Create failed");
        }
        else
        {
           fe_ioss->io_handle = (hypv_session_t*)HVFE_INVALID_HANDLE;
           fe_ioss->fe_cb = *cb;
           fe_ioss->eos_buffer.dev_fd = -1;
           fe_ioss->eos_buffer.data_fd = -1;
           hypv_status = plt_fe_open(str, flag, fe_ioss);
         }
    }
    if (0 != rc || HYPV_STATUS_SUCCESS != hypv_status)
    {
        HYP_VIDEO_MSG_ERROR("failed, invalid node");

        if (fe_ioss->lock_buffer)
        {
            MM_CriticalSection_Release(fe_ioss->lock_buffer);
        }
        if (fe_ioss->state_synch_obj)
        {
            MM_Signal_Release(fe_ioss->state_synch_obj);
        }
        if (fe_ioss->state_synch_obj_q)
        {
            MM_SignalQ_Release(fe_ioss->state_synch_obj_q);
        }
        if (fe_ioss->input_tag_entry)
        {
            delete fe_ioss->input_tag_entry;
        }
        free(fe_ioss);
        fe_ioss = NULL;
    }

    return (HVFE_HANDLE)fe_ioss;
}

/**===========================================================================

FUNCTION video_fe_ioctl

@brief  Hypervisor video FE ioctl API

@param [in] HVFE_HANDLE
@param [in] cmd
@param [in] data pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type video_fe_ioctl(HVFE_HANDLE handle, int cmd, void* data)
{
    fe_io_session_t *fe_ioss = (fe_io_session_t *)handle;

    if (NULL == fe_ioss || NULL == data)
    {
        HYP_VIDEO_MSG_ERROR("handle = %p or data = %p is NULL", fe_ioss, data);
        return HYPV_STATUS_FAIL;
    }

    return plt_fe_ioctl(handle, cmd, data);
}

/**===========================================================================

FUNCTION video_fe_close

@brief  Hypervisor video FE close API

@param [in] HVFE_HANDLE

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type video_fe_close(HVFE_HANDLE handle)
{
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    fe_io_session_t *fe_ioss = (fe_io_session_t *)handle;

    HYP_VIDEO_MSG_INFO("closing FE");

    if (handle == 0)
    {
        HYP_VIDEO_MSG_ERROR("failed to close FE, handle is 0");
        rc = HYPV_STATUS_FAIL;
    }
    else
    {
        rc = plt_fe_close(handle);
        MM_CriticalSection_Release(fe_ioss->lock_buffer);
        MM_Signal_Release(fe_ioss->state_synch_obj);
        MM_SignalQ_Release(fe_ioss->state_synch_obj_q);
        if (fe_ioss->input_tag_entry)
        {
            delete fe_ioss->input_tag_entry;
        }
        free(fe_ioss);
    }

    return rc;
}

hyp_target_variant_type get_target_variant()
{
    int soc_id = -1;
    int result = -1;
    hyp_target_variant_type target_variant = HYP_TARGET_UNKNOWN;
    char buffer[10] = "\0";
    FILE *device = NULL;

    device = fopen("/sys/devices/soc0/soc_id", "r");

    if (device)
    {
        /* 4 = 3 (MAX_SOC_ID_LENGTH) + 1 */
        result = fread(buffer, 1, 4, device);
        fclose(device);
    }

    if (result > 0)
    {
        soc_id = atoi(buffer);
    }

    switch (soc_id)
    {
    case 532: /* LEMANSAU_IVI */
    case 534: /* LEMANSAU_IVI_ADAS */
        target_variant = HYP_TARGET_LEMANS;
        HYP_VIDEO_MSG_HIGH("get target soc_id = %d (LEMANS) ", soc_id);
        break;
    default:
        target_variant = HYP_TARGET_UNKNOWN;
        HYP_VIDEO_MSG_ERROR("unknown target variant %d", soc_id);
    }

    return target_variant;
}

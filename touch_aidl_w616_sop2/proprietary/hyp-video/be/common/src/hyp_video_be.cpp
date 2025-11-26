/*===========================================================================

*//** @file hyp_video_be.cpp
This file implements hypervisor video be services

Copyright (c) 2017-2019, 2021, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/


/*===========================================================================
                             Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/1.0/be/common/src/hyp_video_be.c#14 $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           nb          Fix compilation errors due to additon of new compiler flags
04/01/21           sh          Bringup video on RedBend Hypervisor
02/05/19           rz          Bringup changes for 8155
09/18/18           sm          Release root after dlopen
09/07/18           aw          Update retry mechanism to use infinite retry
08/17/18           sm          Add support to set uid/gid from args
08/07/18           sm          Update errno for hab recv retry mechanism
07/10/18           sm          Add support to use pmem handle for video buffers
06/12/18           sm          Use a common macro for memset
05/30/18           sm          Add bmetrics support for KPI
05/08/18           sm          Add support for 10 bit playback
03/08/18           sm          Add HAB API retry mechanism
03/05/18           sm          Remove timeout from hab socket open
02/20/18           sm          Implement passthrough in QX FE and BE hypervisor
01/26/18           sm          Increase message size to handle bigger size data
01/18/18           sm          Add support for passthrough mode feature
01/16/18           sm          Close video session on closing HAB channel
11/23/17           sm          Update return type from MM HAB API
11/13/17           am          Exit Daemon thread on HAB errors
09/20/17           sm          Update video hypervisor emulation macro
09/18/17           sm          Use official habmm header
06/30/17           aw          Add support for version validation between FE and BE
06/28/17           aw          Unify and update all logs in hyp-video
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
04/03/17           sm          Add support for Video input FE-BE
01/30/17           sm          Update BE-FE message passing
01/13/17           sm          Synchronize HAB send calls
12/13/16           hl          Support hypervisor encoding and port changes from AGL
09/29/16           hl          fix to work with LA hypervisor
08/16/16           rz          Dynamically loading hab library
07/27/16           rz          Fix csim hypervisor tests
07/26/16           rz          Handle contiguous memory mapping for QNX OMX
07/08/16           hl          Isolate video data from ioctl
06/22/16           henryl      Add dynamic buffer mode support
06/20/16           henryl      Clean up and add error messages
06/09/16           henryl      clean up the header dependency
06/01/16           hl          Add FE and BE to support Hypervisor interface

============================================================================*/

#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include "MMThread.h"
#include "MMCriticalSection.h"
#ifdef WIN32
#include "ioctlServerWinApi.h"
#elif defined(__QNXNTO__)
#include <process.h>
#include <libbmetrics.h>
#include <login.h>
#endif
#include "hyp_queue_utility.h"
#include "hyp_video_be_translation.h"
#include "hyp_buffer_manager.h"
#include "hyp_vidc_inf.h"
#include "hyp_vidc_types.h"
#include "hyp_videopriv.h"
#include "hyp_video.h"
#include "hyp_video_be.h"
#include "hyp_debug.h"
#include "MMDebugMsg.h"
#include "MMTimer.h"
#include "AEEStdDef.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/** msg queue only for C-SIM */
#ifdef WIN32
static int hvbeCallbackManager(void*);
static int hvbe_start_callback_manager(hypv_session_t* hypv_session);
static int hvbe_stop_callback_manager(hypv_session_t* hypv_session);

/**===========================================================================

FUNCTION hvbe_open_msg_queue

@brief  Create hypervisor video BE  msg queue

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type hvbe_open_msg_queue(hypv_session_t* hypv_session)
{
   uint32 i;

   q_init(&hypv_session->msgQClient);
   q_init(&hypv_session->msgQServer);

   for (i = 0; i < MAX_NUM_MSG; i++)
   {
      q_put(&(hypv_session->msgQServer),
             q_link(&(hypv_session->msgBuff[i]),
             &(hypv_session->msgBuff[i].link)));
   }

   /*
   ** Create a signal object for queue synchronization
   */
   MM_SignalQ_Create(&(hypv_session->msgSignalQ));
   MM_Signal_Create(hypv_session->msgSignalQ,
                     NULL,
                     NULL,
                     &(hypv_session->msgSignal) );

   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION hvbe_close_msg_queue

@brief  Close hypervisor video BE  msg queue

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type hvbe_close_msg_queue(hypv_session_t* hypv_session)
{
   MM_Signal_Release(hypv_session->msgSignal);
   MM_SignalQ_Release(hypv_session->msgSignalQ);

   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION hvbe_start_callback_manager

@brief  Start hypervisor video BE callback threads

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type hvbe_start_callback_manager(hypv_session_t* hypv_session)
{
   if (hvbe_open_msg_queue(hypv_session) == HYPV_STATUS_FAIL)
   {
      return HYPV_STATUS_FAIL;
   }

   if (MM_Thread_CreateEx(MM_Thread_DefaultPriority,
      0, hvbeCallbackManager, (void*)hypv_session, HYPV_THREAD_STACK_SIZE,
      "hvbeClientCbmgr", (MM_HANDLE*)&hypv_session->hvbe_callback_thread) != 0)
   {
      HYP_VIDEO_MSG_ERROR("failed to create thread");
      return HYPV_STATUS_FAIL;
   }

   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION hvbe_stop_callback_manager

@brief  Stop hypervisor video BE callback threads

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type hvbe_stop_callback_manager(hypv_session_t* hypv_session)
{
   // shut down the callback loop and free memory
   if (hypv_session->hvbe_callback_thread)
   {
      int rc;

      hypv_session->exit_resp_handler = TRUE;
      MM_Signal_Set(hypv_session->msgSignal);
      MM_Thread_Join((MM_HANDLE)hypv_session->hvbe_callback_thread, &rc);

      MM_Thread_Release((MM_HANDLE)hypv_session->hvbe_callback_thread);
      hypv_session->hvbe_callback_thread = 0;
   }

   if(HYPV_STATUS_FAIL == hvbe_close_msg_queue(hypv_session))
   {
      return HYPV_STATUS_FAIL;
   }

   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION hvbeCallbackManager

@brief  hypervisor video BE callback threads

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
static hypv_status_type hvbeCallbackManager(void* pIoSession)
{
   hypv_session_t *hypv_session = (hypv_session_t*)pIoSession;
   ioctlClientMsgQType* msgNode = NULL;
   boolean done = FALSE;
   habmm_msg_desc_t msg2;
   int32 habmm_rc;

   while (!done)
   {
      /*
      ** Wait on a signal object to be woken up when there is a new
      ** message in the queue
      */
      if (0 != MM_SignalQ_Wait(hypv_session->msgSignalQ, NULL))
      {
         HYP_VIDEO_MSG_HIGH("Exiting msg q thread");
         break;
      }

      /*
      ** Retrieve a msg from the queue
      */
      msgNode = (ioctlClientMsgQType*) q_get(&(hypv_session->msgQClient));

      /*
      ** Finish all the messages in the queue
      */
      while(msgNode)
      {
         if (msgNode)
         {
            /** call the client callback
                It requires msg translation from driver to ilclient due to 64BIT flavor
             */
            msg2.msg_id = HYPVIDEO_MSGRESP_EVENT;

            if (VIDC_ERR_NONE != translate_hvbe_to_habmm(hypv_session, HYPVIDEO_MSGRESP_EVENT, (void*)msgNode->msg.msg_buf, (void*)&msg2))
            {
                HYP_VIDEO_MSG_ERROR("failed to translate from BE to habmm");
            }
            else
            {
               msg2.data_size= GET_MSG_DATASIZE_FROM_EVENT_VIDCMSG_SIZE(msgNode->msg.msg_len);
               MM_CriticalSection_Enter(hypv_session->sendCritSection);
               habmm_rc = habmm_socket_send(hypv_session->habmm_handle,&msg2,sizeof(habmm_msg_desc_t), 0 /*flags*/);
               MM_CriticalSection_Leave(hypv_session->sendCritSection);
               if (0 != habmm_rc)
               {
                   HYP_VIDEO_MSG_ERROR("habmm socket send failed, rc=%d", habmm_rc);
               }
            }
            q_put(&(hypv_session->msgQServer), (q_link_type *)msgNode);
         }
         msgNode = (ioctlClientMsgQType*) q_get(&(hypv_session->msgQClient));
      }
      if (hypv_session->exit_resp_handler == TRUE)
      {
          done = TRUE;
      }
   }
   return HYPV_STATUS_SUCCESS;
}

static int hvbeDaemonThread(void* pIoSession);

/**===========================================================================

FUNCTION start_hvbe_daemon_thread

@brief  Function to start hypervisor video BE daemon

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type start_hvbe_daemon_thread(hypv_session_t* hypv_session)
{
   if (MM_Thread_CreateEx(MM_Thread_DefaultPriority,
      0, hvbeDaemonThread, (void*)hypv_session, HYPV_THREAD_STACK_SIZE,
      "hvbeDaemonThread", (MM_HANDLE*)&hypv_session->hvbeDaemonThread) != 0)
   {
      HYP_VIDEO_MSG_ERROR("failed to create thread");
      return HYPV_STATUS_FAIL;
   }

   /** Start callback thread */
   hvbe_start_callback_manager(hypv_session);
   return HYPV_STATUS_SUCCESS;
}

/**===========================================================================

FUNCTION stop_hvbe_daemon_thread

@brief  Function to stop hypervisor video BE daemon

@param [in] hypv session pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type stop_hvbe_daemon_thread(hypv_session_t* hypv_session)
{
   /** Stop callback thread */
   hvbe_stop_callback_manager(hypv_session);

   /** Release hvbe daemon thread */
   if (hypv_session->hvbeDaemonThread)
   {
      int32 rc;

      hypv_session->exit_resp_handler = TRUE;
      MM_Thread_Join((MM_HANDLE)hypv_session->hvbeDaemonThread, &rc);

      if(0 != MM_Thread_Release((MM_HANDLE)hypv_session->hvbeDaemonThread))
      {
         HYP_VIDEO_MSG_ERROR("release thread failed");
      }
      hypv_session->hvbeDaemonThread = 0;
   }
   return HYPV_STATUS_SUCCESS;
}

#else

/**===========================================================================

FUNCTION hvbe_callback_handler

@brief  hypervisor BE callback handler

@param [in] msg pointer
@param [in] msg length
@param [in] private void pointer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hvbe_callback_handler(uint8 *msg, uint32 length, void *cd)
{
    hypv_session_t* hypv_session = (hypv_session_t*)cd;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;
    habmm_msg_desc_t msg2;
    vidc_drv_msg_info_type* pMsgInfo = ( vidc_drv_msg_info_type*)msg;

    /** call the client callback
        It requires msg translation from driver to ilclient due to 64BIT flavor
     */
    if (TRUE == hypv_session->multi_stream_enable &&
        VIDEO_SESSION_DECODE == hypv_session->video_session)
    {
        /* if secondary output is enabled, events for buf type OUTPUT
           is handled within BE */
        if (VIDC_BUFFER_OUTPUT == pMsgInfo->payload.frame_data.buf_type ||
            VIDC_EVT_RESP_FLUSH_OUTPUT2_DONE == pMsgInfo->event_type)
        {
            return rc;
        }
    }
    if (TRUE == hypv_session->passthrough_mode)
    {
        if ((HYP_PLATFORM_QX == hypv_session->hyp_platform) ||
            (HYP_PLATFORM_GH == hypv_session->hyp_platform))
        {
            hypv_map_entry_t* ret_buff = NULL;
            vidc_drv_msg_payload* payload = (vidc_drv_msg_payload* )(msg2.data.event_data.passthru.payload);
            HABMM_MEMCPY((void *)&msg2.data.event_data, (void *)msg, length);

            if (NULL != payload->frame_data.frame_addr || NULL != payload->frame_data.frame_handle)
            {
                ret_buff = hypv_map_to_lookup(&hypv_session->lookup_queue,
                                             (void *)(payload->frame_data.frame_addr ?
                                              payload->frame_data.frame_addr : payload->frame_data.frame_handle));
                if (NULL != ret_buff)
                {
                    payload->frame_data.frame_addr = (uint8*)(uintptr_t)ret_buff->bufferid;
                    if (VIDC_EVT_RESP_OUTPUT_DONE == pMsgInfo->event_type)
                    {
                        if ((hypv_session->hypvid_flags & HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_MASK) &&
                            !(payload->frame_data.flags & VIDC_FRAME_FLAG_READONLY))
                        {
                            hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue,
                                           payload->frame_data.frame_addr, ret_buff->bufferid);
                        }
                    }
                }
                else
                {
                    rc = HYPV_STATUS_FAIL;
                    HYP_VIDEO_MSG_ERROR("failed to lookup entry node for frame addr 0x%p",
                                        (void *)payload->frame_data.frame_addr);
                }
            }
            else
            {
                payload->frame_data.frame_addr = 0;
            }

            if ( (HYPV_STATUS_SUCCESS == rc) &&
                 payload->frame_data.non_contiguous_metadata &&
                 (0 != payload->frame_data.metadata_addr ||
                  0 != payload->frame_data.metadata_handle) )
            {
                ret_buff = hypv_map_to_lookup(&hypv_session->lookup_queue,
                                             (void *)(payload->frame_data.metadata_addr ?
                                             payload->frame_data.metadata_addr : payload->frame_data.metadata_handle));
                if (NULL != ret_buff)
                {
                    payload->frame_data.metadata_addr = (uint8*)(uintptr_t)ret_buff->bufferid;
                    if (VIDC_EVT_RESP_OUTPUT_DONE == pMsgInfo->event_type)
                    {
                        if ((hypv_session->hypvid_flags & HYPVID_FLAGS_USE_DYNAMIC_OUTPUT_BUFFER_MASK) &&
                           !(payload->frame_data.flags & VIDC_FRAME_FLAG_READONLY))
                        {
                            hypv_map_free(&hypv_session->habmm_if, &hypv_session->lookup_queue,
                                          payload->frame_data.metadata_addr, ret_buff->bufferid);
                        }
                    }
                }
                else
                {
                    rc = HYPV_STATUS_FAIL;
                    HYP_VIDEO_MSG_ERROR("failed to lookup entry node for metadata addr 0x%p",
                                        (void *)payload->frame_data.metadata_addr);
                }
            }
            else
            {
                payload->frame_data.metadata_addr = 0;
            }
        }
        else
        {
            HABMM_MEMCPY((void *)&msg2.data.event_data, (void *)msg, length);
        }
    }
    else
    {
        translate_hvbe_to_habmm(hypv_session, HYPVIDEO_MSGRESP_EVENT, (void*)msg, (void*)&msg2);
    }
    msg2.msg_id = HYPVIDEO_MSGRESP_EVENT;
    msg2.virtual_channel = hypv_session->habmm_handle;
    msg2.data_size= GET_MSG_DATASIZE_FROM_EVENT_VIDCMSG_SIZE(length);
    MM_CriticalSection_Enter(hypv_session->sendCritSection);
    hypv_session->habmm_if.pfSend(hypv_session->habmm_handle,&msg2,sizeof(habmm_msg_desc_t), 0 /*flags*/);
    MM_CriticalSection_Leave(hypv_session->sendCritSection);

    return rc;
}

#endif

/**===========================================================================

FUNCTION hvbeVersionMatch

@brief   Version validation between FE and BE

@param [in] hypv session pointer
@param [in] msg pointer

@dependencies
  None

@return
  Returns boolean

===========================================================================*/
static boolean hvbeVersionMatch(hypv_session_t* hypv_session, habmm_msg_desc_t* msg)
{
    int32 rc = 0;
    habmm_msg_desc_t msg2;
    boolean ret = TRUE;

    if (GET_MAJOR_REV(msg->version) != GET_MAJOR_REV(HYP_VIDEO_VERSION) ||
            GET_MINOR_REV(msg->version) != GET_MINOR_REV(HYP_VIDEO_VERSION))
    {
        ret = FALSE;

        msg2.msg_id = HYPVIDEO_MSGRESP_OPEN_RET;
        msg2.data.open_data.return_status = HYPV_STATUS_VERSION_MISMATCH;
        msg2.data_size = msg->data_size;
        msg2.virtual_channel = hypv_session->habmm_handle;
#ifndef WIN32
        msg2.pid = (uint32) getpid();
#endif
        HYP_VIDEO_MSG_ERROR("Version mismatch FE(%u.%u) BE(%d.%u)",
            GET_MAJOR_REV(msg->version), GET_MINOR_REV(msg->version),
            GET_MAJOR_REV(HYP_VIDEO_VERSION), GET_MINOR_REV(HYP_VIDEO_VERSION));

        MM_CriticalSection_Enter(hypv_session->sendCritSection);
        rc = hypv_session->habmm_if.pfSend(hypv_session->habmm_handle, &msg2, sizeof(habmm_msg_desc_t), 0 /*flags*/);
        MM_CriticalSection_Leave(hypv_session->sendCritSection);
        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to send socket rc %d", rc);
        }
    }
    return ret;
}

/**===========================================================================

FUNCTION hvbeDaemonThread

@brief  Hypervisor BE daemon thread

@param [in] void pointer

@dependencies
  None

@return
  Returns int

===========================================================================*/
static int hvbeDaemonThread(void* pIoSession)
{
    hypv_session_t* hypv_session = (hypv_session_t*)pIoSession;
    boolean done = FALSE;
    habmm_msg_desc_t msg;
    uint32 msg_size;
    int32 rc = 0;

    HYP_VIDEO_MSG_LOW("thread started");

    while (!done)
    {
        // habmm_recv needs the msg_size for the allocated size of the msg
        msg_size =  sizeof(habmm_msg_desc_t);
        rc = hypv_session->habmm_if.pfRecv(hypv_session->habmm_handle, (void*)&msg, &msg_size, 0, 0);
        if (hypv_session->exit_resp_handler == TRUE)
        {
           done = TRUE;
        }
        else if (0 == rc)
        {
            switch (msg.msg_id)
            {
                case HYPVIDEO_MSGCMD_OPEN:
                {
                    // Version validation
                    if (!hvbeVersionMatch(hypv_session, &msg))
                    {
                        done = TRUE;
                        break;
                    }
                    plt_hvbe_open(hypv_session, &msg);
                    break;
                }
                case HYPVIDEO_MSGCMD_IOCTL:
                {
                    plt_hvbe_ioctl(hypv_session, &msg);
                    break;
                }
                case HYPVIDEO_MSGCMD_CLOSE:
                {
                    //TODO: Need to remove this once FE populate the correct virtual_channel
                    msg.virtual_channel = hypv_session->habmm_handle;
                    plt_hvbe_close(hypv_session, &msg);
                    done = TRUE;
                    break;
                }
                default:
                {
                    HYP_VIDEO_MSG_HIGH("unrecognized msg id 0x%x", msg.msg_id );
                    break;
                }
            }
        }
        else if (-EINTR == rc)
        {
            HYP_VIDEO_MSG_HIGH("hypv receive error, retrying");
            continue;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("hypv receive (handle = %d) returned rc = %d",hypv_session->habmm_handle, rc );
            HABMM_MEMSET((void *)&msg, 0, sizeof(habmm_msg_desc_t));
            plt_hvbe_close(hypv_session, &msg);
            done = TRUE;
        }
    }
#ifndef WIN32
    // wait a while for the ack message to go through the hypervisor channel
    if (0 == rc)
    {
        HYP_VIDEO_MSG_LOW("500 ms sleep before exit handle %d", hypv_session->habmm_handle);
        MM_Timer_Sleep(500);
    }
    HYP_VIDEO_MSG_HIGH("closing habmm handle %d", hypv_session->habmm_handle);
    hypv_session->habmm_if.pfClose(hypv_session->habmm_handle);
    MM_CriticalSection_Release(hypv_session->sendCritSection);
    free(hypv_session);
#endif
    return HYPV_STATUS_SUCCESS;
                                                                      }
#ifndef WIN32

#ifdef __QNXNTO__
static inline boolean _secpol_in_use ( void )
{
    char *env = getenv ( "SECPOL_ENABLE" ) ;
    if ( env )
    {
        return (env[0] == '1');
    }
    else
    {
        return 0;
    }
}
#endif

int main(int argc, char *argv[])
{
    int32 run = 1;
    boolean  bLoadTestHab = FALSE;
    habIf habmm_if;                     // habmm function pointers
    void* dl_handle = NULL;             // habmm dlopen handle
#ifndef __QNXNTO__
    UNUSED(argc);
    UNUSED(argv);
#endif

#ifdef __QNXNTO__
    bmetrics_log_subcomponent_done(VIDEO_HYP_BE" launched" );
#endif

#ifdef _LINUX_
    char *env_ptr = getenv("HYPV_DEBUG_LEVEL");
    debug_level = env_ptr ? atoi(env_ptr) : 0;
#elif defined _ANDROID_
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.hypv.debug.level", property_value, "1");
    debug_level = atoi(property_value);
#endif

#ifdef __QNXNTO__
    MM_Debug_Initialize();
#endif
    HYP_VIDEO_MSG_INFO("Hypervisor video server started");

    HABMM_MEMSET((void *)&habmm_if, 0, sizeof(habmm_if));
    if (!access((char *)VIDEO_HYP_EMULATION_SIGNATURE, R_OK))
    {
       // test internal socket HAB in QNX
       bLoadTestHab = TRUE;
    }

    if (bLoadTestHab)
    {
       dl_handle = dlopen(VIDEO_HYP_EMULATION_HAB_LIB, RTLD_LOCAL);
    }
    else
    {
       dl_handle = dlopen(HAB_LIB, RTLD_LOCAL|RTLD_NOW);
    }


    if (NULL == dl_handle)
    {
       HYP_VIDEO_MSG_ERROR("load hab lib %s failed, bLoadTestHab %d",
                       bLoadTestHab?VIDEO_HYP_EMULATION_HAB_LIB:HAB_LIB, bLoadTestHab);

#ifdef __QNXNTO__
       MM_Debug_Deinitialize();
#endif
       return 0;
    }
    else
    {
       HYP_VIDEO_MSG_HIGH("dlopen load hab lib %s successful, bLoadTestHab %d",
                       bLoadTestHab?VIDEO_HYP_EMULATION_HAB_LIB:HAB_LIB, bLoadTestHab );
    }

    habmm_if.pfOpen = (hyp_habmm_socket_open) dlsym(dl_handle, "habmm_socket_open");
    habmm_if.pfClose = (hyp_habmm_socket_close) dlsym(dl_handle, "habmm_socket_close");
    habmm_if.pfSend = (hyp_habmm_socket_send) dlsym(dl_handle, "habmm_socket_send");
    habmm_if.pfRecv = (hyp_habmm_socket_recv) dlsym(dl_handle, "habmm_socket_recv");
    habmm_if.pfImport = (hyp_habmm_import) dlsym(dl_handle, "habmm_import");
    habmm_if.pfExport = (hyp_habmm_export) dlsym(dl_handle, "habmm_export");
    habmm_if.pfUnImport = (hyp_habmm_unimport) dlsym(dl_handle, "habmm_unimport");
    habmm_if.pfUnExport = (hyp_habmm_unexport) dlsym(dl_handle, "habmm_unexport");

    if (!habmm_if.pfOpen ||
         !habmm_if.pfClose ||
         !habmm_if.pfSend ||
         !habmm_if.pfRecv ||
         !habmm_if.pfImport ||
         !habmm_if.pfExport ||
         !habmm_if.pfUnImport ||
         !habmm_if.pfUnExport)
    {
       HYP_VIDEO_MSG_ERROR("dlsym hab lib failed" );
       dlclose(dl_handle);
       dl_handle = NULL;
       return 0;
    }

#ifdef __QNXNTO__
    bmetrics_log_subcomponent_done(VIDEO_HYP_BE" ready");
    int c = 0;

    while( ( c = getopt( argc, argv,"U:" ) ) != -1 )
    {
        switch( c )
        {
            case 'U':
            {
                if(_secpol_in_use())
                {
                    HYP_VIDEO_MSG_INFO("secpol in use");
                    if(set_ids_from_arg(optarg) != EOK )
                    {
                        HYP_VIDEO_MSG_ERROR("UID/GID setting failed");
                        MM_Debug_Deinitialize();
                        return 0;
                    }
                }
                else
                {
                    HYP_VIDEO_MSG_INFO("secpol not enabled");
                }
                break;
            }
        }
    }
#endif

    while (run)
    {
        int32 virtual_channel = 0;
        int32 rc = 0;
        MM_HANDLE threadHandle = NULL;

        HYP_VIDEO_MSG_HIGH("opening socket ");

        rc = habmm_if.pfOpen(&virtual_channel, MM_VID, 0, 0);

        if (!rc)
        {
            HYP_VIDEO_MSG_HIGH("got a connection from client");

            // start worker thread
            HYP_VIDEO_MSG_HIGH("create server worker thread to handle the client");
            hypv_session_t *hypv_session = (hypv_session_t *)HABMM_MALLOC(sizeof(hypv_session_t));
            if (hypv_session == NULL)
            {
                HYP_VIDEO_MSG_ERROR("malloc hypv session failed");
                rc = -1;
            }
            else
            {
                HABMM_MEMSET(hypv_session, 0, sizeof(hypv_session_t));

                HABMM_MEMCPY(&hypv_session->habmm_if, &habmm_if, sizeof(habIf));

                hypv_session->habmm_handle = virtual_channel;
                if (0 != MM_CriticalSection_Create(&hypv_session->sendCritSection))
                {
                    HYP_VIDEO_MSG_ERROR("failed to create send critical section");
                    free(hypv_session);
                    rc = -1;
                }
                else
                {
                    rc = MM_Thread_Create(MM_Thread_DefaultPriority, 0, hvbeDaemonThread, (void *)hypv_session, 0, &threadHandle);
                    if (rc != 0)
                    {
                        HYP_VIDEO_MSG_ERROR("failed to create server worker thread");
                        MM_CriticalSection_Release(hypv_session->sendCritSection);
                        free(hypv_session);
                    }
                    else
                    {
                        HYP_VIDEO_MSG_HIGH("created server worker thread");
                        MM_Thread_Detach(threadHandle);
                    }
                }
            }
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("video BE connection listener failed rc %d",rc);
            continue;
        }
    }


    dlclose(dl_handle);
#ifdef __QNXNTO__
    MM_Debug_Deinitialize();
#endif

    return 0;
}

#endif

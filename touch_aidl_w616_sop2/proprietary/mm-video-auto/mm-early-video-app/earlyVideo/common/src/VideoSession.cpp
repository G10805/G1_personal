/*
 **************************************************************************************************
 * Copyright (c) 2014-2020, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <time.h>
#include <sys/types.h>
#include "VideoSession.h"
#include "VideoPlatform.h"

void *VideoSessionThreadRun(void *pSessionHandle);
VidcStatus VideoSessionRun(VideoSession *pSession);
VidcStatus VideoSessionSubscribeToEvents(VideoSession *pSession);
VidcStatus VideoSessionUnsubscribeEvents(VideoSession *pSession);
VidcStatus VideoSessionGetCapabilities(VideoSession *pSession);
VidcStatus VideoSessionAddCommand(VideoSession *pSession, string cmdName, string cmdVal);
VidcStatus VideoSessionGetControl(VideoSession *pSession, struct v4l2_control *control);
VidcStatus VideoSessionSetControl(VideoSession *pSession, uint32 id, uint32 value);
VidcStatus VideoSessionProcessInputBuffer(VideoSession *pSession, BufInfo *pBufInfo);
VidcStatus VideoSessionWaitForEventDone(VideoSession *pSession, pthread_cond_t  *pCondWait, pthread_mutex_t *pMutex);
VidcStatus VideoSessionFlush(VideoSession *pSession, uint32 bufType);

uint32 g_subscribedEvents[] = { V4L2_EVENT_MSM_VIDC_FLUSH_DONE,
                                V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT,
                                V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE,
                                V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER,
                                V4L2_EVENT_MSM_VIDC_SYS_ERROR,
                                V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED,
                                V4L2_EVENT_MSM_VIDC_HW_OVERLOAD,
                                V4L2_EVENT_MSM_VIDC_MAX_CLIENTS,
                              };

/*=============================================================================

   FUNCTION:     VideoSessionInit()

   DESCRIPTION:  Initialise structure members
*//*
   PARAMETERS:
*//**    @param[in]  bIsDec - true - if its video decoder session
                          true - if its video encoder session
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionInit(VideoSession *pSession, VideoSessionType eType)
{
    VidcStatus status = VidcStatusSuccess;

    pSession->m_type = eType;

    pthread_mutex_init(&pSession->m_sessionMutex, 0);
    pthread_mutex_init(&pSession->m_mutex, 0);
    pthread_cond_init(&pSession->m_portThreadsCondWait, 0);
    pthread_cond_init(&pSession->m_asyncPortThrdCondWait, 0);
    pSession->m_pFlushMsgNode = NULL;
    pSession->m_nMaxPauseTime = 0;
    pSession->m_skipTagDataValidation = FALSE;
    pSession->m_pTagDataArray_size = 0;

    pSession->m_portParam[OUTPUT_PORT]  = (VideoPortParams *)malloc(sizeof(VideoPortParams));
    pSession->m_portParam[CAPTURE_PORT] = (VideoPortParams *)malloc(sizeof(VideoPortParams));
    pSession->m_pStreamParser           = (VideoStreamParser *)malloc(sizeof(VideoStreamParser));
    if (!pSession->m_portParam[OUTPUT_PORT] ||
        !pSession->m_portParam[CAPTURE_PORT] ||
        !pSession->m_pStreamParser)
    {
        FREE(pSession->m_portParam[OUTPUT_PORT]);
        FREE(pSession->m_portParam[CAPTURE_PORT]);
        FREE(pSession->m_pStreamParser);
        return VidcStatusAllocError;
    }

    VideoPortParamsInit(pSession->m_portParam[OUTPUT_PORT],pSession,OUTPUT_PORT);
    VideoPortParamsInit(pSession->m_portParam[CAPTURE_PORT],pSession, CAPTURE_PORT);
    pSession->m_pStreamParser->m_nlogMask = gVideoDecProp.logMask;
    pSession->m_pStreamParser->nOffsetDump = gVideoDecProp.nOffsetDump;
    status = VideoStreamParserInit(pSession->m_pStreamParser);

    pSession->ThreadRun          = VideoSessionThreadRun;
    pSession->SubscribeEvents    = VideoSessionSubscribeToEvents;
    pSession->QueryCapabilities  = VideoSessionGetCapabilities;
    pSession->Run                = VideoSessionRun;
    pSession->Close              = VideoSessionClose;
    pSession->ProcessInputBuffer = VideoSessionProcessInputBuffer;
    pSession->SetControl         = VideoSessionSetControl;
    pSession->GetControl         = VideoSessionGetControl;
    pSession->Flush              = VideoSessionFlush;
    pSession->WaitForEventDone   = VideoSessionWaitForEventDone;

    return status;
}

VidcStatus VideoSessionDeInit(VideoSession *pSession)
{
    VidcStatus status = VidcStatusSuccess;

    pthread_mutex_destroy(&pSession->m_sessionMutex);
    pthread_mutex_destroy(&pSession->m_mutex);
    pthread_cond_destroy(&pSession->m_asyncPortThrdCondWait);
    pthread_cond_destroy(&pSession->m_portThreadsCondWait);

    return status;
}
/*=============================================================================

FUNCTION:     VideoSessionClose()

DESCRIPTION:  Removes all pending commands, closes all file handles
opened and destroy the lists created
*//*
PARAMETERS:
*//**    @param[in]  none
*//*
RETURN VALUE:
*/ /**       @return  none

=============================================================================*/
VidcStatus VideoSessionClose(VideoSession* pSession)
{
    VidcStatus status = VidcStatusSuccess;

    FCLOSE(pSession->m_outStream);
    status = VideoSessionUnsubscribeEvents(pSession);

    close(pSession->m_hDriver);
    close(pSession->m_hPollEventFd);

    if (pSession->m_pStreamParser)
    {
        pSession->m_pStreamParser->Close(pSession->m_pStreamParser);
    }
    pSession->m_portParam[OUTPUT_PORT]->Close(pSession->m_portParam[OUTPUT_PORT]);
    pSession->m_portParam[CAPTURE_PORT]->Close(pSession->m_portParam[CAPTURE_PORT]);

    FREE(pSession->m_portParam[OUTPUT_PORT]);
    FREE(pSession->m_portParam[CAPTURE_PORT]);
    FREE(pSession->m_pStreamParser);

    return status;
}

/*=============================================================================

   FUNCTION:     VideoSessionOpen()

   DESCRIPTION:  Store the parent context handle passed and initialise
                 dma command strcture based on parent context data
*//*
   PARAMETERS:
*//**    @param[in]  pContext - pointer to parent video context structure
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionOpen(VideoSession* pSession)
{
    VidcStatus status = VidcStatusSuccess;
    VideoResolution res;

    pSession->m_StaticCfg.eCodecType = V4L2_PIX_FMT_H264;
    res.nWidth      = gVideoDecProp.nDefaultWidth;
    res.nHeight     = gVideoDecProp.nDefaultHeight;
    res.nCropHeight = gVideoDecProp.nDefaultHeight;
    res.nCropWidth  = gVideoDecProp.nDefaultWidth;

    pSession->m_portParam[OUTPUT_PORT]->SetResolution(pSession->m_portParam[OUTPUT_PORT], res);
    pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);

    pSession->m_StaticCfg.nFrameCnt             = NUM_FRMS_PARSE;
    pSession->m_nFrameCounter                   = 1; //Default frame counter starts with 1
    pSession->m_StaticCfg.nSkipReadAfterNFrames = 0;

    if (pSession->m_pStreamParser)
    {
        pSession->m_ParserStaticCfg.nFrameCnt = pSession->m_StaticCfg.nFrameCnt;
        pSession->m_ParserStaticCfg.sCodecType = "VIDEO_CodingAVC";
        pSession->m_ParserStaticCfg.nSkipReadAfterNFrames = pSession->m_StaticCfg.nSkipReadAfterNFrames;

        status = pSession->m_pStreamParser->Configure(pSession->m_pStreamParser,
                                                      &pSession->m_ParserStaticCfg,
                                                      gVideoDecProp.sInputRoot,
                                                      gVideoDecProp.inputDump,
                                                      gVideoDecProp.sOutRoot,
                                                      gVideoDecProp.sInputFileName,
                                                      pSession->m_sTestID,
                                                      pSession->m_StaticCfg.bSecure);

        if (status != VidcStatusSuccess)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_FILEOPEN, "%s", gVideoDecProp.sInputFileName);
            goto bailout;
        }

        if (pSession->m_type == VideoSessionTypeDecode)
        {
            pSession->m_pStreamParser->m_nPrefixHeaderMode = V4L2_DEC_S_ONLY;
            VIDCTF_PRINT_HIGH("Setting Default to Sequence header only in first frame");
        }
        if ((pSession->m_StaticCfg.eCodecType != V4L2_PIX_FMT_H264 &&
             pSession->m_StaticCfg.eCodecType != V4L2_PIX_FMT_HEVC) ||
             pSession->m_StaticCfg.bThumbnailMode)
        {
            pSession->m_pStreamParser->m_nPrefixHeaderMode = V4L2_DEC_SF;
            VIDCTF_PRINT_HIGH("Overrighting header only mode with SF mode");
        }

        pSession->m_pStreamParser->m_nSuperFrame = 1;

        if (pSession->m_pStreamParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY)
        {
            //If decode type is S only then start frame counter with 0 for 1st ETB to be seq header only.
            pSession->m_pStreamParser->m_bIsGetSeqHdr = TRUE;
            pSession->m_nFrameCounter = 0; 
        }
    }

    if (pSession->m_pStreamParser &&  pSession->m_pStreamParser->BuildOffsetTable)
    {
        status = pSession->m_pStreamParser->BuildOffsetTable(pSession->m_pStreamParser);
        VIDEO_BAILOUT_ON_FAILURE("Build Offset table failure", status, bailout);
    }

    /** Create Async Thread to start recieving events from driver */
    status = VIDC_ERROR(pthread_create(&pSession->m_hAsyncThread, NULL,
                                       VideoSessionAsyncThreadRun, pSession));

    //update time out limitation if maximum pause time is larger
    gVideoDecProp.nPollTimeout = pSession->m_nMaxPauseTime > gVideoDecProp.nPollTimeout ?
                                   pSession->m_nMaxPauseTime + 10000 : gVideoDecProp.nPollTimeout;
    gVideoDecProp.nThreadTimeout = pSession->m_nMaxPauseTime > gVideoDecProp.nThreadTimeout ?
                                   pSession->m_nMaxPauseTime + 10000 : gVideoDecProp.nThreadTimeout;
    VIDCTF_PRINT_HIGH("PollTimeOut = %d ms and ThreadTimeOut = %d ms",
                                       gVideoDecProp.nPollTimeout,
                                       gVideoDecProp.nThreadTimeout);

bailout:
    return status;
}

/*=============================================================================

   FUNCTION:     VideoSession::Start()

   DESCRIPTION:  Start Driver
*//*
   PARAMETERS:
*//**    @param[in]
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionRun(VideoSession *pSession)
{
    VidcStatus status = VidcStatusSuccess;

    if (pSession->WaitForSync)
    {
        pSession->WaitForSync(pSession);
    }
    /** Create OutputPort and CapturePort Thread to start data flow */
    status = pSession->m_portParam[CAPTURE_PORT]->CreateThreadID(pSession->m_portParam[CAPTURE_PORT]);
    VIDEO_BAILOUT_ON_FAILURE("CAPTURE_PORT thread create", status, bailout);

    status = pSession->m_portParam[OUTPUT_PORT]->CreateThreadID(pSession->m_portParam[OUTPUT_PORT]);
    VIDEO_BAILOUT_ON_FAILURE("OUTPUT_PORT thread create", status, bailout);

    if (status == VidcStatusSuccess)
    {
        pthread_join(pSession->m_portParam[OUTPUT_PORT]->GetThreadID(pSession->m_portParam[OUTPUT_PORT]), NULL);
        pthread_join(pSession->m_portParam[CAPTURE_PORT]->GetThreadID(pSession->m_portParam[CAPTURE_PORT]), NULL);
    }
bailout:
    return status;
}

/*=============================================================================

   FUNCTION:     VideoSession::GetCapabilities()

   DESCRIPTION:  GetCapabilities Driver
*//*
   PARAMETERS:
*//**    @param[in]
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionGetCapabilities(VideoSession *pSession)
{
    VidcStatus status = VidcStatusSuccess;
    struct v4l2_capability cap;
    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_QUERYCAP, &cap));

    if (status)
    {
        VIDCTF_PRINT_WARN("Failed to query capabilities");
    }
    else
    {
        VIDCTF_PRINT_INFO("Capabilities: driver_name = %s, card = %s, bus_info = %s,"
            " version = %d, capabilities = %x", cap.driver, cap.card,
            cap.bus_info, cap.version, cap.capabilities);
    }
    return status;
}
/*=============================================================================

   FUNCTION:     VideoSessionSubscribeToEvents()

   DESCRIPTION:  SubscribeToEvents
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/

VidcStatus VideoSessionSubscribeToEvents(VideoSession *pSession)
{
    VidcStatus status = VidcStatusSuccess;
    uint32 nCount = 0;
    struct v4l2_event_subscription sub;
    for (nCount = 0; nCount != sizeof(g_subscribedEvents) / sizeof(uint32); nCount++)
    {
        memset(&sub, 0, sizeof(sub));
        sub.type = g_subscribedEvents[nCount];
        VIDCTF_PRINT_HIGH("SubscribeToEvent: %d", sub.type);
        status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_SUBSCRIBE_EVENT, &sub));
        VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_SUBSCRIBE_EVENT", status, bailout);
    }

bailout:
    if (status != VidcStatusSuccess)
    {

        for (; nCount != 0; --nCount)
        {
            memset(&sub, 0, sizeof(sub));
            sub.type = g_subscribedEvents[nCount];
            status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_UNSUBSCRIBE_EVENT, &sub));
            VIDCTF_PRINT_INFO("ioctl VIDIOC_UNSUBSCRIBE_EVENT %d status:%d", sub.type, status);
        }
    }
    return status;
}
/*=============================================================================

   FUNCTION:     VideoSession::UnsubscribeEvents()

   DESCRIPTION:  UnsubscribeEvents
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionUnsubscribeEvents(VideoSession *pSession)
{
    VidcStatus status = VidcStatusSuccess;
    uint32 nCount = 0;
    struct v4l2_event_subscription sub;
    for (nCount = 0; nCount != sizeof(g_subscribedEvents) / sizeof(uint32); nCount++)
    {
        memset(&sub, 0, sizeof(sub));
        sub.type = g_subscribedEvents[nCount];
        status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_UNSUBSCRIBE_EVENT, &sub));
        VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_UNSUBSCRIBE_EVENT", status, bailout);
    }
bailout:
    return status;
}

/*=============================================================================

   FUNCTION:     VideoDecoder::SetControl()

   DESCRIPTION:  Set control param to driver
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionSetControl(VideoSession *pSession, uint32 id, uint32 value)
{
    struct v4l2_control control;
    uint32 ret = 0;

    control.id = id;
    control.value = value;
    VIDCTF_PRINT_INFO("Calling IOCTL set control for id=%d, val=%d\n", control.id, control.value);
    ret = ioctl(pSession->m_hDriver, VIDIOC_S_CTRL, &control);
    if (ret != 0)
    {
       VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_IOCTL, "VIDIOC_S_CTRL ret= %u : %s", ret, strerror(errno));
    }

    return VIDC_ERROR(ret);
}

/*=============================================================================

   FUNCTION:     VideoDecoder::GetControl()

   DESCRIPTION:  Get control param from driver
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionGetControl(VideoSession *pSession, struct v4l2_control *control)
{
    uint32 ret;
    VIDCTF_PRINT_INFO("Calling IOCTL get control for id=%d\n", control->id);
    ret = ioctl(pSession->m_hDriver, VIDIOC_G_CTRL, control);
    if (ret != 0)
    {
       VIDCTF_PRINT_INFO("Failed ioctl VIDIOC_G_CTRL ret= %u : %s", ret, strerror(errno));
    }
    return VIDC_ERROR(ret);
}

/*=============================================================================

   FUNCTION:     VideoSession::ProcessInputBuffer()

   DESCRIPTION:  Process and read Input buffer
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoSessionProcessInputBuffer(VideoSession *pSession, BufInfo *pBufInfo)
{
    VidcStatus status = VidcStatusSuccess;
    ParserFrameInfoType frameInfo = {0};

    if (pSession->m_pStreamParser)
    {
        do{
            platform_update_inputbfr_ptr(&frameInfo.pBuffer, pBufInfo->vaddr);
            frameInfo.nFrameCounter = pSession->m_nFrameCounter;
            frameInfo.nAllocLen = pBufInfo->size;
            frameInfo.nFlags = pBufInfo->flags; //EBD may have returned buffer flag which may be needed to for processdynamic command.
#ifdef PRE_MSM_NILE
            frameInfo.nIonfd = pBufInfo->ion.fd_ion_data.fd;
#else
            frameInfo.nIonfd = pBufInfo->ion.mapfd;
#endif
            frameInfo.nOffset = pBufInfo->offset;
            frameInfo.nIonHeap = pBufInfo->ion.ion_alloc_data.heap_id_mask;

            // Restore, if any actual streamParser frame counter if any seek performed
            pSession->m_nFrameCounter = frameInfo.nFrameCounter;

            frameInfo.nFlags = 0; //reset buffer flags return from EBD
            status = pSession->m_pStreamParser->GetNextFrame(pSession->m_pStreamParser, &frameInfo);
            VIDEO_BAILOUT_ON_FAILURE("GetNextFrame", status, bailout);
            // Restore, depending on how many AUs has been fatched
            pSession->m_nFrameCounter = frameInfo.nFrameCounter;

            if (!pSession->m_nFrameSkip)
            {
                break;
            }
            pSession->m_nFrameSkip--;
        } while (!frameInfo.bLastFrame);

        platform_write_inputbfr_ptr(pBufInfo->vaddr, frameInfo.nFilledLen);

        pBufInfo->flags = frameInfo.nFlags;
        pBufInfo->plane.bytesused = frameInfo.nFilledLen;
        pBufInfo->timestamp.tv_sec = frameInfo.nTimeStamp / 1000000;
        pBufInfo->timestamp.tv_usec = frameInfo.nTimeStamp % 1000000;
#if TAGDATA_ENABLE
        pBufInfo->input_tag = frameInfo.nFrameCounter;
        pBufInfo->input_tag2 = frameInfo.nFrameCounter;
#endif
        if (frameInfo.bLastFrame || frameInfo.nFilledLen == 0)
        {
            pBufInfo->flags |= V4L2_BUF_FLAG_EOS;

            VIDCTF_PRINT_HIGH("Reached EOS buffer on ProcessInputBuffer");
        }
    }

bailout:
    return status;
}

VidcStatus SignalPortThread(VideoSession *pSession, PortType port, uint32 event,uint8* data)
{
    return pSession->m_portParam[port]->PostEvent(pSession->m_portParam[port], event,data);
}

uint32 VideoSessionAsyncEventHandle(VideoSession *pSession ,struct pollfd pfd)
{
    uint32 rc = 0, pollError = 0;
    struct v4l2_event dqevent;
    VidcStatus status = VidcStatusSuccess;
    uint32* ptr = NULL;

    rc = ioctl(pfd.fd, VIDIOC_DQEVENT, &dqevent);
    if (rc)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_IOCTL, "VIDIOC_DQEVENT");
        pollError = SESSION_IOCTL_ERROR;
    }
    VIDCTF_PRINT_HIGH("VIDIOC_DQEVENT:0x%X", dqevent.type);
    switch (dqevent.type)
    {
    case V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT:
        VIDCTF_PRINT_HIGH("Port Reconfig insufficient event recieved\n");
#ifdef _UBWC_
        ptr = (uint32*)dqevent.u.data;
        pSession->m_nBitDepthMode = ptr[MSM_VIDC_BIT_DEPTH];
        pSession->m_bProgressive = ptr[MSM_VIDC_PIC_STRUCT];
        pSession->m_eColorSpace = (ptr[MSM_VIDC_COLOR_SPACE] == MSM_VIDC_BT2020 ? BT2020 : EXCEPT_BT2020);
#endif
        status = SignalPortThread(pSession, CAPTURE_PORT, dqevent.type,NULL);
        if (status != VidcStatusSuccess)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "PostEventFailed");
            pollError = SESSION_ALLOC_ERROR;
        }
        break;
    case V4L2_EVENT_MSM_VIDC_FLUSH_DONE:
        pthread_mutex_lock(&pSession->m_sessionMutex);
        pthread_cond_broadcast(&pSession->m_asyncPortThrdCondWait);
        pthread_mutex_unlock(&pSession->m_sessionMutex);
        VIDCTF_PRINT_HIGH("Flush Done Notified from AsyncThread \n");
        break;
    case V4L2_EVENT_MSM_VIDC_SYS_ERROR:
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_SYS, "Frm:%d", pSession->GetFrameDoneCount(pSession));
        pollError = dqevent.type;
        break;
    case V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED:
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_INPUT_UNSUPPORTED, "HW_UNSUPPORTED");
        pollError = dqevent.type;
        break;
    case V4L2_EVENT_MSM_VIDC_HW_OVERLOAD:
    case V4L2_EVENT_MSM_VIDC_MAX_CLIENTS:
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Unsupported Event:0x%x", dqevent.type);
        pollError = dqevent.type;
        break;
    case V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE:
        status = SignalPortThread(pSession, CAPTURE_PORT, V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE, dqevent.u.data);
        if (status != VidcStatusSuccess)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "PostEventFailed");
            pollError = SESSION_ALLOC_ERROR;
        }
        break;
    case V4L2_EVENT_MSM_VIDC_RELEASE_UNQUEUED_BUFFER:
        pSession->m_portParam[CAPTURE_PORT]->ReleaseUnQueuedBuffer(pSession->m_portParam[CAPTURE_PORT],
            (void *)dqevent.u.data);
        break;
    default:
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Some event has occurred!! Type: 0x%x", dqevent.type);
        pollError = SESSION_UNKNOWN_EVENT_ERROR;
    }
    return pollError;
}
/*=============================================================================

   FUNCTION:     VideoSession::VidcAsyncThread()

   DESCRIPTION:  Process async messages from driver through polling on driver handle
*//*
   PARAMETERS:
*//**    @param[in]  none
*//*
  RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
void* VideoSessionAsyncThreadRun(void *pSessionHandle)
{
    int rc = 0;
    VideoSession *pSession = (VideoSession *)pSessionHandle;
    struct pollfd pfds[2] = {0};
    VidcStatus status = VidcStatusSuccess;
    uint32 pollError = 0, nFdCount = 1;

    pfds[0].events = POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM | POLLRDBAND | POLLPRI;
    pfds[0].fd = pSession->m_hDriver;
    pfds[1].events = POLLIN | POLLERR;
    pfds[1].fd = pSession->m_hPollEventFd;
    nFdCount = 2;

    while (1)
    {
        rc = poll(pfds, nFdCount, gVideoDecProp.nPollTimeout);
        if (rc <= 0)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_POLLTIMEOUT, "Frm:%d,rc:%d", pSession->GetFrameDoneCount(pSession), rc);
            SignalPortThread(pSession, OUTPUT_PORT, SESSION_POLL_ERROR,NULL);
            SignalPortThread(pSession, CAPTURE_PORT, SESSION_POLL_ERROR, NULL);
            break;
        }

        VIDCTF_PRINT_INFO("events = 0x%x", pfds[0].revents);

        //First Handle buffer events and then driver events
        if ((pfds[0].revents & POLLIN) || (pfds[0].revents & POLLRDNORM))
        {
            status = pSession->m_portParam[CAPTURE_PORT]->ProcessBufferDone(pSession->m_portParam[CAPTURE_PORT]);
        }
        if ((pfds[0].revents & POLLOUT) || (pfds[0].revents & POLLWRNORM))
        {
            status = pSession->m_portParam[OUTPUT_PORT]->ProcessBufferDone(pSession->m_portParam[OUTPUT_PORT]);
        }
        if (pfds[0].revents & POLLRDBAND)
        {
            pollError = pfds[0].revents & POLLRDBAND;
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Some event has happened-%d", pfds[0].revents & POLLRDBAND);
        }
        if ((pfds[1].revents & POLLIN) || (pfds[1].revents & POLLERR))
        {
            VIDCTF_PRINT_HIGH("Asnyc thread to be exited"); // good case
            break;
        }
        if (pfds[0].revents & POLLPRI)
        {
            pollError = VideoSessionAsyncEventHandle(pSession,pfds[0]);
        }
        if (pollError)
        {
            SignalPortThread(pSession, OUTPUT_PORT, pollError,NULL);
            SignalPortThread(pSession, CAPTURE_PORT, pollError,NULL);
            pollError = 0;
        }
        if (pfds[0].revents & (POLLERR | POLLNVAL))
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_POLL, "event:%d", pfds[0].revents & POLLERR);
            pollError = SESSION_POLL_ERROR;
            break;
        }
     }

    VIDCTF_PRINT_HIGH("EXIT poll()");
    return NULL;
}

void* VideoSessionThreadRun(void* pContext)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession *pSession = (VideoSession *)pContext;

    if (!pSession)
    {
        VIDCTF_PRINT_WARN("NULL Session!!");
        return NULL;
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->Open(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error opening decode session\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->SubscribeEvents(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to subscribe to events\n");
       }
    }
 
    if (status == VidcStatusSuccess)
    {
       status = pSession->QueryCapabilities(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to query capabilities\n");
       }
    }
 
    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->EnumFormats(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to enumerate formats for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->EnumFormats(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to enumerate formats for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->SetFmt(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to set format for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->SetDefaultConfig(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to set static config for decoder\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->GetFmt(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to get format for output port\n");
       }
    }
 
    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->SetFmt(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to set format for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->GetFmt(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to get format for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->GetBufReq(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to get buffer requirements for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->GetBufReq(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to get buffer requirements for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->PrepareBuf(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to prepare buffer for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->StreamOn(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to streamon for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->QueueBuffers(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to queue buffers for capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->QueueBuffers(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to queue buffers for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->StreamOn(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to streamon for output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->Run(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to start decoder\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       pthread_mutex_lock(&pSession->m_sessionMutex);
       status = pSession->Flush(pSession, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE | V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to flush the ports\n");
       }
       pthread_mutex_unlock(&pSession->m_sessionMutex);
       if (pSession->WaitForSync)
       {
          pSession->WaitForSync(pSession);
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->Stop(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to stop decoder\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[OUTPUT_PORT]->StreamOff(pSession->m_portParam[OUTPUT_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to streamoff output port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->m_portParam[CAPTURE_PORT]->StreamOff(pSession->m_portParam[CAPTURE_PORT]);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to streamoff capture port\n");
       }
    }

    if (status == VidcStatusSuccess)
    {
       status = pSession->Close(pSession);
       if (status != VidcStatusSuccess)
       {
          VIDCTF_PRINT_WARN("Error failed to close the driver\n");
       }
    }

    if (status != VidcStatusSuccess)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "SessionOperationsFailed");
        if (pSession->m_hDriver) 
        {
            pSession->m_portParam[OUTPUT_PORT]->StreamOff(pSession->m_portParam[OUTPUT_PORT]);
            pSession->m_portParam[CAPTURE_PORT]->StreamOff(pSession->m_portParam[CAPTURE_PORT]);
            pSession->Stop(pSession);
            pSession->Close(pSession);
        }
    }

    VIDCTF_PRINT_HIGH("VidcSessionThreadRun Exit()");

    return NULL;
}

VidcStatus VideoSessionWaitForEventDone(VideoSession *pSession, pthread_cond_t  *pCondWait, pthread_mutex_t *pMutex)
{
    struct timeval now;
    struct timespec timeout;
    VidcStatus status = VidcStatusSuccess;
    int rc;

    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + (gVideoDecProp.nThreadTimeout / 1000);
    timeout.tv_nsec = now.tv_usec;
    VIDCTF_PRINT_HIGH("Waiting for EventDone");
    rc = pthread_cond_timedwait(pCondWait, pMutex, &timeout);
    if (rc != 0)
    {
        status = VidcStatusFlushTimeOutError;
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_FLUSHTIMEOUT, "status:%d", rc);
    }
    return status;
}

VidcStatus VideoSessionFlush(VideoSession *pSession, uint32 bufType)
{
    VidcStatus status = VidcStatusSuccess;
    VideoPortParams *pOutPort     = pSession->m_portParam[OUTPUT_PORT];
    VideoPortParams *pCapturePort = pSession->m_portParam[CAPTURE_PORT];
    BufInfo *pCurrentBuf = NULL;
    VIDCTF_PRINT_HIGH("Issueing driver flush for session:%d",pSession->m_type);
    /* Need to make sure that we acquire flush before calling this function*/
    status = pSession->FlushDriver(pSession, bufType);
    if (status == VidcStatusSuccess)
    {
        status = pSession->WaitForEventDone(pSession, &pSession->m_asyncPortThrdCondWait, &pSession->m_sessionMutex);
    }

    if (status != VidcStatusSuccess)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_FLUSHTIMEOUT, "");
    }

    if (!pCapturePort->WaitForBufferAlloc &&
        ((bufType & V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
        (pCapturePort->m_bInfoListSize > pCapturePort->m_bufQ->m_size)
       )
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERSRETURN,
            "Capture,PendingCnt:%d,type:%d",
            (pCapturePort->m_bInfoListSize - pCapturePort->m_bufQ->m_size),
            pSession->m_type);
        status = VidcStatusUnknownError;
    }

    return status;
}

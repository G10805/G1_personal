/*-------------------------------------------------------------------
Copyright (c) 2014-2020, 2022-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#include "VideoSessionPort.h"
#include "VideoSession.h"
#include "VideoPlatform.h"
#include <cutils/properties.h>

#ifdef SUPPORT_DMABUF
#include <BufferAllocator/BufferAllocatorWrapper.h>
#endif

#define MEM_DEVICE "/dev/ion"
#define MEM_HEAP_ID ION_CP_MM_HEAP_ID

#define SZ_4K 0x1000
#define SZ_1M 0x100000

void VideoPortParamsClose(VideoPortParams *pPortParam);
void VideoPortParamsFreeIONMemory(VideoPortParams *pPortParam, struct ion_info *buf_ion_info);
void VideoPortParamsReQueueBuffer(VideoPortParams *pPortParam, BufInfo* binfo);
void VideoPortParamsGetResolution(VideoPortParams *pPortParam, VideoResolution *pRes);
void VideoPortParamsSetResolution(VideoPortParams *pPortParam, VideoResolution res);
VidcStatus VideoPortParamsGetEnumarateFormats(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsQueueBuffers(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsStreamOn(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsStreamOff(VideoPortParams *pPortParam);

VidcStatus VideoPortParamsAllocIONBuffer(VideoPortParams *pPortParam, BufInfo *binfo, uint32 alignment, int flag);
VidcStatus VideoPortParamsAllocExtraData(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsPrepareBufs(VideoPortParams *pParams);
VidcStatus VideoPortParamsGetBufReqs(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsSetBufReqs(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsGetFmt(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsSetFmt(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsProcessBufferDone(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsQueueBuffer(VideoPortParams *pPortParam, BufInfo *binfo);
VidcStatus VideoPortParamsCreateThreadID(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsFreeBuffers(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsPostEvent(VideoPortParams *pPortParam, uint32 evtType, uint8* data);
VidcStatus VideoPortParamsReleaseUnQueuedBuffer(VideoPortParams *pPortParam, void* pData);
VidcStatus VideoPortParamsHandlePortReconfig(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsHandleFlush(VideoPortParams *pPortParam);
VidcStatus VideoPortParamsHandleMsgEvent(VideoPortParams *pPortParam, BufInfo *binfo);
VidcStatus VideoPortParamsHandleOutPutBuf(VideoPortParams *pParams, BufInfo *binfo);
VidcStatus VideoPortParamsHandleCaptureBuf(VideoPortParams *pParams, BufInfo *binfo);
VidcStatus VideoPortParamsPushEvent(VideoPortParams *pParams, BufInfo *binfo);
VidcStatus VideoPortParamsSetBuffers(VideoPortParams *pParams);
BufInfo* VideoPortParamsGetBuffer(VideoPortParams *pPortParam);
BOOL VideoPortParamsIsEmptyBufQ(VideoPortParams *pPortParam);

pthread_t VideoPortParamsGetThreadID(VideoPortParams *pPortParam)
{
    return pPortParam->m_threadID;
}
BufInfo* BufInfoListIndex(LIST_NODE *blist, uint32 idx)
{
    BufInfo *entry = NULL, *pos = NULL;
    uint32 cnt = 0;
    utils_list_for_each_entry(BufInfo, pos, blist, bInfoListNode)
    {
        if (cnt++ == idx)
        {
            entry = pos;
            break;
        }
    }
    return entry;
}

VidcStatus VideoPortParamsStartBufferAccess(VideoSession *pSession, int fd)
{
    struct dma_buf_sync buf_sync;
    int rc = 0;

    if (fd < 0)
        return VidcStatusSuccess;

    buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (rc) {
        VIDCTF_PRINT_ERROR(pSession,VIDC_ERR_BADPARAM,"Failed DMA_BUF_IOCTL_SYNC start fd : %d\n", fd);
        return VidcStatusUnknownError;
    }
    return VidcStatusSuccess;
}

VidcStatus VideoPortParamsEndBufferAccess(VideoSession *pSession, int fd)
{
    struct dma_buf_sync buf_sync;
    int rc = 0;

    if (fd < 0)
        return VidcStatusSuccess;

    buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (rc) {
        VIDCTF_PRINT_ERROR(pSession,VIDC_ERR_BADPARAM,"Failed DMA_BUF_IOCTL_SYNC end fd : %d\n", fd);
        return VidcStatusUnknownError;
    }
    return VidcStatusSuccess;
}

VidcStatus VideoPortParamsInit(VideoPortParams* pParams, VideoSession *pSession, PortType port)
{
    VidcStatus status = VidcStatusSuccess;
    if (!pParams || !pSession)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }

    memset(pParams, 0, sizeof(VideoPortParams));
    pthread_mutex_init(&pParams->m_mutex, 0);

    pParams->m_bufDoneCnt = 0;
    pParams->m_bufSubmitCnt = 0;
    pParams->m_bAllocated = FALSE;
    for(int i = 0; i < VIDEO_MAX_FRAME; i++) {
        pParams->m_extraData.ion[i].mapfd = -1;
    }
    pParams->m_bIsStreamOnOutputPlane = FALSE;

    pParams->m_hSession = pSession;

    if (CAPTURE_PORT == port)
    {
        pParams->m_bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    }
    else
    {
        pParams->m_bufType = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    }

    list_init(&pParams->m_bInfoList);
    pParams->m_bInfoListSize = 0;
    list_init(&pParams->m_bInfoDefFreeList);
    pParams->m_bInfoDefFreeListSize = 0;

    pParams->m_bufQ = (VideoMsgQueue*)malloc(sizeof(VideoMsgQueue));
    if (!pParams->m_bufQ) return VidcStatusAllocError;
    status = VideoMsgQueueInit(pParams->m_bufQ, &pParams->m_mutex);
    VIDEO_BAILOUT_ON_FAILURE("VideoMsgQueueInit", status, bailout);

    pParams->EnumFormats            = VideoPortParamsGetEnumarateFormats;
    pParams->SetFmt                 = VideoPortParamsSetFmt;
    pParams->GetFmt                 = VideoPortParamsGetFmt;
    pParams->PrepareBuf             = VideoPortParamsPrepareBufs;
    pParams->QueueBuffers           = VideoPortParamsQueueBuffers;
    pParams->GetBufReq              = VideoPortParamsGetBufReqs;
    pParams->SetBufReq              = VideoPortParamsSetBufReqs;
    pParams->Close                  = VideoPortParamsClose;
    pParams->StreamOn               = VideoPortParamsStreamOn;
    pParams->StreamOff              = VideoPortParamsStreamOff;
    pParams->IsEmptyBufQ            = VideoPortParamsIsEmptyBufQ;
    pParams->GetResolution          = VideoPortParamsGetResolution;
    pParams->SetResolution          = VideoPortParamsSetResolution;
    pParams->ProcessBufferDone      = VideoPortParamsProcessBufferDone;
    pParams->QueueBuffer            = VideoPortParamsQueueBuffer;
    pParams->GetThreadID            = VideoPortParamsGetThreadID;
    pParams->CreateThreadID         = VideoPortParamsCreateThreadID;
    pParams->PostEvent              = VideoPortParamsPostEvent;
    pParams->ReleaseUnQueuedBuffer  = VideoPortParamsReleaseUnQueuedBuffer;
    pParams->PushBuffer             = VideoPortParamsQueueBuffer;
    pParams->PushEvent              = VideoPortParamsPushEvent;
    pParams->TransferBuffer         = VideoPortParamsPushEvent;
    pParams->FreeBuffers            = VideoPortParamsFreeBuffers;

    pParams->SetBuffers             = VideoPortParamsSetBuffers;

    memset(&pParams->m_fmt, 0, sizeof(pParams->m_fmt));

    pParams->m_fmt.fmt.pix_mp.height = gVideoDecProp.nDefaultHeight;
    pParams->m_fmt.fmt.pix_mp.width = gVideoDecProp.nDefaultWidth;

bailout:
    return status;
}

VidcStatus VideoPortParamsReleaseBufferReference(VideoPortParams *pPortParam, void *pData)
{
    unsigned int *ptr = (unsigned int *)pData;
    VidcStatus status = VidcStatusSuccess;
    uint32 fd, offset;
    BufInfo *pBuf;
    uint32 bBufFound = FALSE;
    int rc;

    if (!pPortParam || !pData)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    fd = ptr[0];
    offset = ptr[1];
    VIDCTF_PRINT_INFO("ReleaseBufferReference fd=0x%x,offset=0x%x", fd, offset);

    if (pPortParam->m_bufType != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
        pPortParam->m_hSession->m_type != VideoSessionTypeDecode)
    {
        VIDCTF_PRINT_ERROR(pPortParam->m_hSession, VIDC_ERR_BADPARAM,
            "ReleaseBufferReference on Port:%d,session:%d not expected",
            pPortParam->m_bufType, pPortParam->m_hSession->m_type);
    }

    utils_list_for_each_entry(BufInfo, pBuf, &pPortParam->m_bInfoList, bInfoListNode)
    {
        if ((pBuf->ion.mapfd == (int)fd) && (pBuf->offset == offset))
        {
            pthread_mutex_lock(&pPortParam->m_mutex);
            if (pBuf->bufRefCount)
            {
                pBuf->bufRefCount--;
            }
            else
            {
                //TODO: This situation comes only if when a relesae_buf event has came first than dequeu of the buffer or same buffer returned 2 times.
                VIDCTF_PRINT_ERROR(pPortParam->m_hSession, VIDC_ERR_BADPARAM,
                    "Invalid releaseBufferReference fd=0x%x,offset=0x%x pBuf->bufRefCount%d", fd, offset, pBuf->bufRefCount);
                status = VidcStatusBadParamError;
            }
            pthread_mutex_unlock(&pPortParam->m_mutex);
            bBufFound = TRUE;
            break;
        }
    }

    if (TRUE == bBufFound) return status;

    utils_list_for_each_entry(BufInfo, pBuf, &pPortParam->m_bInfoDefFreeList, bInfoListNode)
    {
        if ((pBuf->ion.mapfd == (int)fd) && (pBuf->offset == offset))
        {
            pthread_mutex_lock(&pPortParam->m_mutex);
            if (pBuf->bufRefCount)
            {
                pBuf->bufRefCount--;
            }
            else
            {
                //TODO: This situation comes only if when a relesae_buf event has came first than dequeu of the buffer or same buffer returned 2 times.
                VIDCTF_PRINT_ERROR(pPortParam->m_hSession, VIDC_ERR_BADPARAM,
                    "Invalid releaseBufferReference fd=0x%x,offset=0x%x pBuf->bufRefCount%d", fd, offset, pBuf->bufRefCount);
                status = VidcStatusBadParamError;
            }
            if (status == VidcStatusSuccess && (pPortParam->m_hSession->m_StaticCfg.eCodecType == V4L2_PIX_FMT_VP9 ||
                pPortParam->m_hSession->m_StaticCfg.eCodecType == V4L2_PIX_FMT_VP8) &&
                (pBuf->releaseFlag & MARK_DEFERRED_FREE) && pBuf->bufRefCount == 0)
            {
                VIDCTF_PRINT_HIGH("Buffer (%p, %u) marked for FREE", (void *)pBuf->vaddr, pBuf->size);
                if (pBuf->ion.ion_device_fd >= 0)
                {
                    if (!pPortParam->m_hSession->m_StaticCfg.bSecure)
                    {
                        VideoPortParamsEndBufferAccess(pPortParam->m_hSession, pBuf->ion.mapfd);
                    }
                    if (pBuf->vaddr)
                    {
                        rc = munmap((void *)pBuf->vaddr, pBuf->size);
                        if (rc)
                        {
                            VIDCTF_PRINT_WARN("Fail to munmap(%p, %u)",
                                (void *)pBuf->vaddr, pBuf->size);
                        }
                        else
                        {
                            VIDCTF_PRINT_INFO("Successfully unmapped(%p, %u)\n",
                                (void *)pBuf->vaddr, pBuf->size);
                        }
                    }
                    VideoPortParamsFreeIONMemory(pPortParam, &pBuf->ion);
                }
                list_remove_node(&pPortParam->m_bInfoDefFreeList, &pBuf->bInfoListNode);
                free(pBuf);
                pPortParam->m_bInfoDefFreeListSize--;
            }
            pthread_mutex_unlock(&pPortParam->m_mutex);
            break;
        }
    }
    return status;
}

void VideoPortParamsClose(VideoPortParams* pParams)
{
    BufInfo *entry;
    VidcStatus status;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return;
    }

    if (pParams->FreeBufferList) //If buffer list only created
    {
        pParams->FreeBufferList(pParams);
    }
    else
    {
        status = VideoPortParamsFreeBuffers(pParams);
    }
    pthread_mutex_destroy(&pParams->m_mutex);

    list_clear(BufInfo, entry, &pParams->m_bInfoList, bInfoListNode);
    if (pParams->m_bufQ)
    {
        pParams->m_bufQ->Destroy(pParams->m_bufQ);
        free(pParams->m_bufQ);
        pParams->m_bufQ = NULL;
    }
}

VidcStatus VideoPortParamsCreateThreadID(VideoPortParams* pParams)
{
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    return VIDC_ERROR(pthread_create(&pParams->m_threadID, NULL, VideoPortParamsDataThreadRun, pParams));
}

/*=============================================================================

   FUNCTION:     VideoPortParams::GetEnumarateFormats()

   DESCRIPTION:  Enumerate formats
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsGetEnumarateFormats(VideoPortParams* pParams)
{
    struct v4l2_fmtdesc fdesc;
    VideoSession* pSession = NULL;
    fdesc.index = 0;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    fdesc.type = pParams->m_bufType;
    while (ioctl(pSession->m_hDriver, VIDIOC_ENUM_FMT, &fdesc) == 0)
    {
        VIDCTF_PRINT_INFO("fmt: description: %s, fmt: %x, flags = %x m_bufType %u index %u", fdesc.description,
            fdesc.pixelformat, fdesc.flags, fdesc.type, fdesc.index);
        fdesc.index++;
    }
    return VidcStatusSuccess;
}
uint32 MapColorFormat(uint32 format)
{
    switch (format)
    {
    case V4L2_PIX_FMT_NV12:
        return COLOR_FMT_NV12;
    case V4L2_PIX_FMT_NV21:
        return COLOR_FMT_NV21;
    case V4L2_PIX_FMT_NV12_UBWC:
        return COLOR_FMT_NV12_UBWC;
    case V4L2_PIX_FMT_NV12_TP10_UBWC:
        return COLOR_FMT_NV12_BPP10_UBWC;
    case V4L2_PIX_FMT_RGB32:
        return COLOR_FMT_RGBA8888;
    case V4L2_PIX_FMT_RGBA8888_UBWC:
        return COLOR_FMT_RGBA8888_UBWC;
    case V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS:
        return COLOR_FMT_P010;
    default:
        return format;
    }
}
/*=============================================================================

   FUNCTION:     VideoPortParams::SetFmt()

   DESCRIPTION:  Set format on specific port for the driver
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsSetFmt(VideoPortParams* pParams)
{  // TODO: Distiguish Between driver ioctls and internal function with Drv calls
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    int rc = 0;
    uint32 nPreviousBufSize = 0;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    pParams->m_fmt.type = pParams->m_bufType;
    pParams->m_fmt.fmt.pix_mp.pixelformat = pSession->GetPortFormat(pSession, pParams->m_bufType);
    nPreviousBufSize = VENUS_BUFFER_SIZE(
                        MapColorFormat(pParams->m_fmt.fmt.pix_mp.pixelformat),
                        pParams->m_fmt.fmt.pix_mp.width,
                        pParams->m_fmt.fmt.pix_mp.height);
    if (pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage < nPreviousBufSize)
        pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage = nPreviousBufSize;

    //TODO: add Drv prefix for all driver ioctl functions
    rc = ioctl(pSession->m_hDriver, VIDIOC_S_FMT, &pParams->m_fmt);
    if (errno == ENOTSUPP)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_INPUT_UNSUPPORTED, "error:%s", strerror(errno));
        status = VidcStatusUnknownError;
    }
    else if (rc != 0)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_IOCTL, "error:%s", strerror(errno));
        status = VidcStatusUnknownError;
    }
    else
    {
        /*If media info buf size more than Driver requirement, overwrite it*/
        if (nPreviousBufSize > pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage)
        {
            pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage = nPreviousBufSize;
        }
        VIDCTF_PRINT_HIGH("port:%d height = %d, width = %d, format = 0x%x, size = %d session:%d",
            pParams->m_bufType,
            pParams->m_fmt.fmt.pix_mp.height,
            pParams->m_fmt.fmt.pix_mp.width,
            pParams->m_fmt.fmt.pix_mp.pixelformat,
            pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage,pSession->m_type);
    }
    if (pParams->m_fmt.fmt.pix_mp.height*pParams->m_fmt.fmt.pix_mp.width > 1920 * 1088 &&
        pParams->m_fmt.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_NV12)
    {
        //As 4k UBWC/RGBA to NV12 takes more CPU time, increase thread timeout to 300sec
        if (gVideoDecProp.nPollTimeout < 300000)
        {
            gVideoDecProp.nPollTimeout = 300000;
        }
        if (gVideoDecProp.nThreadTimeout < 300000)
        {
            gVideoDecProp.nThreadTimeout = 300000;
            VIDCTF_PRINT_WARN("Increased Thread timeout to 300sec");
        }
    }
    return status;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::GetFmt()

   DESCRIPTION:  Get format on specific port for the driver
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsGetFmt(VideoPortParams* pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    int rc = 0;

    int extra_idx = 0;
    int extra_data_size = 0;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    pParams->m_fmt.type = pParams->m_bufType;

    rc = ioctl(pSession->m_hDriver, VIDIOC_G_FMT, &pParams->m_fmt);
    if (errno == ENOTSUPP)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_INPUT_UNSUPPORTED, "error:%s", strerror(errno));
        status = VidcStatusUnSupported;
    }
    else if (rc != 0)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_IOCTL, "error:%s", strerror(errno));
        status = VidcStatusUnknownError;
    }
    else
    {
        //TODO: Need to move SuperFrame specific code to decoder, if not applicable for encoder
        if (pSession->m_pStreamParser)
        {
            if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
            {
                uint32 nMaxFrameSize = pSession->m_pStreamParser->m_nMaxInputFrameSize * pSession->m_pStreamParser->m_nSuperFrame;
                if(nMaxFrameSize > pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage)
                {
                    pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage = nMaxFrameSize;
                    VIDCTF_PRINT_HIGH("Overwriting FW buffer size because maximum required frame size is %d", pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage);
                }
            }
        }
        VIDCTF_PRINT_HIGH("Updated port = %d: h: %d, w: %d, size: %d",
            pParams->m_bufType,
            pParams->m_fmt.fmt.pix_mp.height,
            pParams->m_fmt.fmt.pix_mp.width,
            pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage);
        //TODO: add a sanity check for height/width againt setfmt
        //FIXME: buffer count not available
        extra_idx = (pParams->m_fmt.fmt.pix_mp.num_planes - 1);
        if ((extra_idx > 0) && (extra_idx < VIDEO_MAX_PLANES))
        {
            extra_data_size = pParams->m_fmt.fmt.pix_mp.plane_fmt[extra_idx].sizeimage;
            extra_data_size = ALIGN(extra_data_size, SZ_4K);
            VIDCTF_PRINT_INFO("Required extra data size per buffer = %d, idx = %d\n", extra_data_size, extra_idx);
        }
        else if (extra_idx >= VIDEO_MAX_PLANES)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Extradata index is more than allowed: %d", extra_idx);
            return VidcStatusUnknownError;
        }
        /* If count has has not been initialize it will update the extradata_ingo again in get_bufreqs() */
        pParams->m_extraData.size = extra_data_size;
        pParams->m_extraData.count = pParams->m_bufreq.count;
        pParams->m_extraData.buffer_size = extra_data_size;
        VIDCTF_PRINT_HIGH("Update: VIDIOC_G_FMT: extra data all buffers size = %d  pParams->m_extraData.buffer_size %lu\n", pParams->m_extraData.size, pParams->m_extraData.buffer_size);
    }
    if (pParams->m_fmt.fmt.pix_mp.height*pParams->m_fmt.fmt.pix_mp.width > 1920 * 1088 &&
        pParams->m_fmt.fmt.pix_mp.pixelformat != V4L2_PIX_FMT_NV12)
    {
        //As 4k UBWC/RGBA to NV12 takes more CPU time, increase thread timeout to 300sec
        if (gVideoDecProp.nPollTimeout < 300000)
        {
            gVideoDecProp.nPollTimeout = 300000;
        }
        if (gVideoDecProp.nThreadTimeout < 300000)
        {
            gVideoDecProp.nThreadTimeout = 300000;
            VIDCTF_PRINT_WARN("Increased Thread timeout to 300sec");
        }
    }
    return status;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::GetResolution()

   DESCRIPTION:  Get resolution info
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
void VideoPortParamsGetResolution(VideoPortParams* pParams, VideoResolution *pRes)
{
    if (!pParams || !pRes)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return;
    }
    pRes->nWidth = pParams->m_fmt.fmt.pix_mp.width;
    pRes->nHeight = pParams->m_fmt.fmt.pix_mp.height;
    pRes->nStride = pParams->m_fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    pRes->nScanlines = pParams->m_fmt.fmt.pix_mp.plane_fmt[0].reserved[0];
    pRes->nCropHeight = pParams->m_nCropHeight;
    pRes->nCropWidth = pParams->m_nCropWidth;
    VIDCTF_PRINT_HIGH("Port:%d, Width:%d Height:%d Stride:%d,Scanlines:%d,CropWidth:%d,CropHeight:%d",
        pParams->m_bufType, pRes->nWidth, pRes->nHeight, pRes->nStride, pRes->nScanlines, pRes->nCropWidth, pRes->nCropHeight);
}

void VideoPortParamsSetResolution(VideoPortParams *pParams, VideoResolution res)
{
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return;
    }
    if (res.nWidth != 0 && res.nHeight != 0)
    {
        pParams->m_fmt.fmt.pix_mp.width = res.nWidth;
        pParams->m_fmt.fmt.pix_mp.height = res.nHeight;
        pParams->m_nCropWidth = res.nCropWidth;
        pParams->m_nCropHeight = res.nCropHeight;
        pParams->m_fmt.fmt.pix_mp.plane_fmt[0].bytesperline = VENUS_Y_STRIDE(MapColorFormat(pParams->m_fmt.fmt.pix_mp.pixelformat), res.nWidth);
        pParams->m_fmt.fmt.pix_mp.plane_fmt[0].reserved[0] = VENUS_Y_SCANLINES(MapColorFormat(pParams->m_fmt.fmt.pix_mp.pixelformat), res.nHeight);
        VIDCTF_PRINT_HIGH("Updating port(%d) width: %d height: %d Stride:%d,Scanlines:%d,CropWidth:%d,CropHeight:%d", pParams->m_bufType, res.nWidth, res.nHeight,
            VENUS_Y_STRIDE(MapColorFormat(pParams->m_fmt.fmt.pix_mp.pixelformat), res.nWidth), VENUS_Y_SCANLINES(MapColorFormat(pParams->m_fmt.fmt.pix_mp.pixelformat), res.nHeight),
			res.nCropWidth, res.nCropHeight);
    }
}

/*=============================================================================

FUNCTION:     VideoPortParamsQueueBuffers()

DESCRIPTION:  Queue all port buffers to Driver
*//*
PARAMETERS:
*//**    @param[in]  none
*//*
RETURN VALUE:
*/ /**       @return  none

=============================================================================*/
VidcStatus VideoPortParamsQueueBuffers(VideoPortParams* pParams)
{
    LOG_FUNC_START;
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    BufInfo *binfo;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    if (!pParams->m_bAllocated)
    {
        status = VideoPortParamsPrepareBufs(pParams);
    }

    VIDCTF_PRINT_INFO("QueueBuffers:port:%d,type:%d",pParams->m_bufType, pSession->m_type);
    while (!VideoPortParamsIsEmptyBufQ(pParams) &&
           status == VidcStatusSuccess)
    {
        binfo = VideoPortParamsGetBuffer(pParams);
        if(!binfo)
        {
            VIDCTF_PRINT_HIGH("Buffer Queue is empty");
            break;
        }
        if (binfo->buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        {
            status = pSession->ProcessInputBuffer(pSession, binfo);
            if (status != VidcStatusSuccess)
            {
                VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_INPUT, "buffer:%p,cnt:%d",
                    (void *)binfo->vaddr, pParams->m_bufSubmitCnt);
                break;
            }
        }
        else if (binfo->buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
        {
            binfo->plane.bytesused = 0;
            binfo->flags = 0;
        }
        else
        {
            /*Probably encountered a event buffer, so just free it and exit*/
            VIDCTF_PRINT_WARN("Unexpected message type %x", binfo->msgType);
            status = VidcStatusUnknownError;
            FREE(binfo);
            break;
        }
        status = VideoPortParamsQueueBuffer(pParams, binfo);
        //StreamParser already reached EOS
        if (pParams->m_bIsEOSReached || pParams->m_bIsEOS3Reached)
        {
            VIDCTF_PRINT_HIGH("EOS received on OutputPort");
            //EOS reached before creating threads so updating status
            status = VidcStatusSuccess;
            break;
        }
    }
    LOG_FUNC_END;
    return status;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::GetBufReqs()

   DESCRIPTION:  Get buffer requirements
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsGetBufReqs(VideoPortParams* pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }

    pSession = (VideoSession*)pParams->m_hSession;
	VIDCTF_PRINT_WARN("New beat being used!\n");
    pParams->m_bufreq.type = pParams->m_bufType;
    if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
    {
        pParams->m_bufcount.id = V4L2_CID_MIN_BUFFERS_FOR_OUTPUT;
    }
    else if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    {
        pParams->m_bufcount.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
    }
    else
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERCOUNT,"Invalid buf/session type\n");
        return VidcStatusXmlCfgError;
    }


    pParams->m_bufreq.memory = V4L2_MEMORY_USERPTR;

    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_G_CTRL, &pParams->m_bufcount));
    VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_REQBUFS", status, bailout);

    VIDCTF_PRINT_HIGH("VIDIOC_G_CTRL VIDIOC_REQBUFS port = %d: count = %d\n", pParams->m_bufType, pParams->m_bufcount.value);
    pParams->m_bufreq.count = pParams->m_bufcount.value;

    VIDCTF_PRINT_HIGH("VIDIOC_REQBUFS port = %d: count = %d\n", pParams->m_bufType, pParams->m_bufreq.count);

	if (pSession->m_type == VideoSessionTypeDecode && pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE && (!pSession->m_StaticCfg.bThumbnailMode)) // Incase of thumbnail mode, don't add extra buffers
	{
		pParams->m_bufreq.count += pSession->m_StaticCfg.nDisplayBufCnt; //Add two buffers extra for capture port
		VIDCTF_PRINT_INFO("WithAdditional %d buffers count = %d\n", pSession->m_StaticCfg.nDisplayBufCnt, pParams->m_bufreq.count);
	}

    if(pSession->m_type == VideoSessionTypeEncode && pSession->m_StaticCfg.nSkipReadAfterNFrames > 0 && pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
    {
        if(pParams->m_bufreq.count % 2 != 0)
        {
            pParams->m_bufreq.count += 1; //Add one more buffer to avoid back-to-back cases in Skip reading mode
            VIDCTF_PRINT_HIGH("With Additonal 1 buffer to have an even count in skip reading mode: count = %d\n", pParams->m_bufreq.count);
        }

        if(pSession->m_pStreamParser->m_nSkipReadAfterNFrames >= pParams->m_bufreq.count)
        {
            pSession->m_pStreamParser->m_nReverseReadingBasis = pParams->m_bufreq.count / 2 + 2;
        }
        else
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERCOUNT, "BufferCount Should be equal or less than SkipReadAfterNFrames.");
            status = VidcStatusXmlCfgError;
            return status;
        }
    }

	VIDCTF_PRINT_HIGH("Set : VIDIOC_REQBUFS port = %d: count = %d\n", pParams->m_bufType, pParams->m_bufreq.count);
    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_REQBUFS, &pParams->m_bufreq));
    VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_REQBUFS", status, bailout);

    pParams->m_extraData.size = ALIGN(pParams->m_extraData.buffer_size, 4);
    pParams->m_extraData.count = pParams->m_bufreq.count;
    VIDCTF_PRINT_INFO("Update: VIDIOC_REQBUFS extra data all buffers size = %d\n",
        pParams->m_extraData.size);

bailout:
    return status;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::SetBufReqs()

   DESCRIPTION:  Set buffer requirements
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsSetBufReqs(VideoPortParams* pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    uint32 nPreviousBufCount = 0;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    nPreviousBufCount = pParams->m_bufreq.count;

    pParams->m_bufreq.type = pParams->m_bufType;
    pParams->m_bufreq.memory = V4L2_MEMORY_USERPTR;

    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_REQBUFS, &pParams->m_bufreq));
    VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_REQBUFS", status, bailout);

    VIDCTF_PRINT_HIGH("VIDIOC_REQBUFS port = %d: count = %d\n", pParams->m_bufType, pParams->m_bufreq.count);
    if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
    {
        if (pParams->m_bufreq.count != 1 && pSession->m_type == VideoSessionTypeDecode) // Incase of thumbnail mode, don't add extra buffers
        {
            pParams->m_bufreq.count = MAX((unsigned int)pParams->m_bufreq.count, nPreviousBufCount); //in case of smooth streaming
            status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_REQBUFS, &pParams->m_bufreq));
            VIDEO_BAILOUT_ON_FAILURE("ioctl VIDIOC_REQBUFS", status, bailout);
        }
        pParams->m_extraData.size = pParams->m_extraData.buffer_size;
        pParams->m_extraData.count = pParams->m_bufreq.count;
        VIDCTF_PRINT_INFO("Update: VIDIOC_REQBUFS extra data all buffers size = %d\n",
            pParams->m_extraData.size);
    }
    else
    {
        pParams->m_bufreq.count = MAX(pParams->m_bufreq.count, nPreviousBufCount); //in case of smooth streaming
    }

bailout:
    return status;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::AllocExtraData()

   DESCRIPTION:  Alloc extradata ION buffer
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsAllocExtraData(VideoPortParams* pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VidcExtraDataBufInfo *edata_info = NULL;
    VideoSession* pSession = NULL;
    BufInfo binfo;
    int flag = 0;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    edata_info = &pParams->m_extraData;
    pSession = (VideoSession*)pParams->m_hSession;

    memset(&binfo, 0, sizeof(binfo));

    if (edata_info->size == 0)
    {
        status = VidcStatusSuccess;
        VIDCTF_PRINT_INFO("No extra data buffer required, edata size = %d\n",
                           edata_info->size);
        return status;
    }

    for(int i = 0; i < edata_info->count; i++) {
        if (edata_info->ion[i].mapfd >= 0)
        {
            VideoPortParamsEndBufferAccess(pSession, edata_info->ion[i].mapfd);
            status = VIDC_ERROR(munmap((void *)edata_info->ion[i].uaddr, edata_info->size));
            if (status)
            {
                VIDCTF_PRINT_WARN("Fail to munmap(%p, %u)", (void *)edata_info->ion[i].uaddr, edata_info->size);
            }
            VideoPortParamsFreeIONMemory(pParams, &edata_info->ion[i]);
        }
    }

    // Allocate the extra data ION buffers
    edata_info->size = (edata_info->size + 4095) & (~4095);
    for(int i = 0; i < edata_info->count; i++) {
        binfo.size = edata_info->size;
        binfo.ion = edata_info->ion[i];

        if (((pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
             (gVideoDecProp.nCachedBufferMask & VIDC_BUFTYPE_OUT_EXTRADATA))
            ||
            ((pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) &&
             (gVideoDecProp.nCachedBufferMask & VIDC_BUFTYPE_IN_EXTRADATA)))
            flag |= ION_FLAG_CACHED;

        status = VideoPortParamsAllocIONBuffer(pParams, &binfo, 4096, flag);
        edata_info->ion[i] = binfo.ion;
        if (status == VidcStatusSuccess && (int)edata_info->ion[i].mapfd >= 0)
        {
            edata_info->ion[i].uaddr = (char*)mmap(NULL, edata_info->size,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                edata_info->ion[i].mapfd, 0);
            if (edata_info->ion[i].uaddr == MAP_FAILED)
            {
                VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Failed to get buffer virtual address");
                for(int j = i; j >= 0; j--) {
                    VideoPortParamsFreeIONMemory(pParams, &edata_info->ion[j]);
                }
                status = VidcStatusAllocError;
            }
            else
            {
                for(int j = i; j >= 0; j--) {
                    status = VideoPortParamsStartBufferAccess(pSession, edata_info->ion[j].mapfd);
                    if (status != VidcStatusSuccess)
                    {
                        munmap(edata_info->ion[j].uaddr, edata_info->size);
                        VideoPortParamsFreeIONMemory(pParams, &edata_info->ion[j]);
                        status = VidcStatusAllocError;
                    }
                }
            }
        }
        else
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Failed to allocate memory");
            status = VidcStatusAllocError;
        }
    }
    return status;
}

inline int clip2(int x)
{
    x = x - 1;
    x = x | x >> 1;
    x = x | x >> 2;
    x = x | x >> 4;
    x = x | x >> 16;
    x = x + 1;
    return x;
}
/*=============================================================================

   FUNCTION:     VideoPortParams::AllocIONBuffer()

   DESCRIPTION:  Alloc ION buffer
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsAllocIONBuffer(VideoPortParams *pParams, BufInfo* binfo,
                                         uint32 alignment,  int flag )
{
    int rc = -EINVAL;
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    struct ion_allocation_data *alloc_data = NULL;
    uint32 buffer_size = 0;
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    alloc_data = &binfo->ion.ion_alloc_data;
    buffer_size = binfo->size;

    if (!alloc_data || buffer_size <= 0 /*|| !fd_data*/)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_MISC, "Invalid arguments to alloc_map_ion_memory\n");
        return VidcStatusBadParamError;
    }

#ifdef SUPPORT_DMABUF
    BufferAllocator *bufferAllocator;
    bufferAllocator = CreateDmabufHeapBufferAllocator();
    binfo->ion.mapfd = DmabufHeapAlloc(bufferAllocator, "qcom,system", buffer_size, 0, 0);
    FreeDmabufHeapBufferAllocator(bufferAllocator);
#else

    binfo->ion.ion_device_fd = ion_open();

    if (binfo->ion.ion_device_fd < 0) {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_DEVICEINIT, "MEM_DEVICE fd = %d", binfo->ion.ion_device_fd);
            return VidcStatusUnknownError;
    }

    alloc_data->flags = flag;
    alloc_data->heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
    if (flag & ION_FLAG_SECURE)
    {
        alloc_data->heap_id_mask = ION_HEAP(ION_SECURE_HEAP_ID);
        if (flag & ION_FLAG_CP_BITSTREAM)
        {
            alloc_data->heap_id_mask = ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID);
        }
    }

    alloc_data->len = buffer_size;

    rc = ion_alloc_fd(binfo->ion.ion_device_fd, alloc_data->len, 0,
                      alloc_data->heap_id_mask, alloc_data->flags, &binfo->ion.mapfd);
    if (rc < 0) {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_ALLOC, "ION");
            binfo->ion.mapfd = -1;
            ion_close(binfo->ion.ion_device_fd);
            binfo->ion.ion_device_fd = -1;
            return VidcStatusAllocError;
    }
#endif

    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::FreeIONMemory()

   DESCRIPTION:  Free ION Memory chuck allocated
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
void VideoPortParamsFreeIONMemory(VideoPortParams *pParams, struct ion_info *buf_ion_info)
{
    VideoSession* pSession = NULL;
    if (!pParams || !buf_ion_info)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    VIDCTF_PRINT_INFO("Closing: mapfd %d", buf_ion_info->mapfd);
    if (buf_ion_info->mapfd >= 0) {
         close(buf_ion_info->mapfd);
         buf_ion_info->mapfd = -1;
    }

#ifndef SUPPORT_DMABUF
    VIDCTF_PRINT_INFO("Closing ion_device_fd: %d\n", buf_ion_info->ion_device_fd);
    if (buf_ion_info->ion_device_fd >= 0) {
         ion_close(buf_ion_info->ion_device_fd);
         buf_ion_info->ion_device_fd = -1;
    }
#endif
}

/*=============================================================================

   FUNCTION:     VideoPortParams::StreamOn()

   DESCRIPTION:  Issue Stream on for current port object
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsStreamOn(VideoPortParams *pParams)
{
    LOG_FUNC_START;
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    int rc = 0;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    rc = ioctl(pSession->m_hDriver, VIDIOC_STREAMON, &pParams->m_bufType);
    if (rc != 0)
        status = VIDC_ERROR(errno);

    if (status == VidcStatusUnSupported)
    {
        VIDCTF_PRINT_ERROR(pSession,VIDC_ERR_INPUT_UNSUPPORTED,
            "VIDIOC_STREAMON Failed, Port %d errno %d reason %s\n",
            pParams->m_bufType, errno, strerror(errno));
    }
    else if (status != VidcStatusSuccess)
    {
        VIDCTF_PRINT_ERROR(pSession,VIDC_ERR_IOCTL,
            "VIDIOC_STREAMON Failed, Port %d errno %d reason %s\n",
            pParams->m_bufType, errno, strerror(errno));
    }

    if (status == VidcStatusSuccess &&
        pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
    {
        pParams->m_bIsStreamOnOutputPlane = TRUE;
    }

    LOG_FUNC_END;
    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::StreamOff()

   DESCRIPTION:  Issue StreamOff for current port object
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsStreamOff(VideoPortParams *pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    VIDCTF_PRINT_HIGH("VIDIOC_STREAMOFF port = %d\n", pParams->m_bufType);

    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_STREAMOFF, &pParams->m_bufType));

    VIDEO_BAILOUT_ON_FAILURE("ioctl STREAMOFF", status, bailout);

    pParams->m_bufreq.type = pParams->m_bufType;
    pParams->m_bufreq.count = 0;
    pParams->m_bufreq.memory = V4L2_MEMORY_USERPTR;
    ioctl(pSession->m_hDriver, VIDIOC_REQBUFS, &pParams->m_bufreq);

bailout:
    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParamsHandlePortReconfig()

   DESCRIPTION:  HandlePortReconfig decoder CapturePort
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsHandlePortReconfig(VideoPortParams *pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    VIDCTF_PRINT_HIGH("Handling PortReconfig for Session:%d", pSession->m_type);
    if (pSession->m_type == VideoSessionTypeDecode)
    {
        status = pSession->PortReconfig(pSession);
    }
    else if (pSession->m_type == VideoSessionTypeEncode)
    {
        VideoPortParams *pCapturePort = pSession->m_portParam[CAPTURE_PORT];
        pthread_mutex_lock(&pSession->m_sessionMutex);
        status = pCapturePort->PostEvent(pCapturePort, SESSION_RECONFIG_MSG, NULL);
        if (status == VidcStatusSuccess)
        {
            status = pSession->WaitForEventDone(pSession, &pSession->m_portThreadsCondWait, &pSession->m_sessionMutex);
            VIDCTF_PRINT_HIGH("EncoderPortReconfig Completed");
        }
        pthread_mutex_unlock(&pSession->m_sessionMutex);
    }
    return status;
}

VidcStatus VideoPortParamsHandleFlush(VideoPortParams *pParams)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    pthread_mutex_lock(&pSession->m_sessionMutex);
    if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
    {
        //Port Reconfig usecase
        status = pSession->Flush(pSession, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    }
    else
    {
        //Seek Usecase
        status = pSession->Flush(pSession, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE | V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);

    }
    //Clear flushMsgNode, after flushMsg is handled
    pSession->m_pFlushMsgNode = NULL;
    pthread_cond_broadcast(&pSession->m_portThreadsCondWait);

    pthread_mutex_unlock(&pSession->m_sessionMutex);
    VIDCTF_PRINT_HIGH("Event Notified from PortThread");
    if (status != VidcStatusSuccess)
    {
        if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        {
            pSession->m_portParam[CAPTURE_PORT]->PostEvent(pSession->m_portParam[CAPTURE_PORT],
                SESSION_UNKNOWN_EVENT_ERROR, NULL);
        }
        else
        {
            pSession->m_portParam[OUTPUT_PORT]->PostEvent(pSession->m_portParam[OUTPUT_PORT],
                SESSION_UNKNOWN_EVENT_ERROR, NULL);
        }
    }
    return status;
}
VidcStatus VideoPortParamsHandleOutPutBuf(VideoPortParams *pParams, BufInfo *binfo)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    VIDCTF_PRINT_INFO("BufferAvailable from Port-OUTPUT-idx:%d,flags:0x%x", binfo->index,binfo->flags);

    pParams->m_bufDoneCnt++;
    if (binfo->flags & V4L2_BUF_INPUT_UNSUPPORTED)
    {
        VIDCTF_PRINT_ERROR(pParams->m_hSession, VIDC_ERR_INPUT_UNSUPPORTED, "");
        status = VidcStatusOutPutThreadError;
    }
    else if ((binfo->flags & V4L2_BUF_FLAG_DATA_CORRUPT) &&
             !(pParams->m_hSession->IsDataCorruptTobeValidated &&
             FALSE == pParams->m_hSession->IsDataCorruptTobeValidated(pParams->m_hSession)) &&
             VIDC_CLASSIFICATION_FCN_DYN != pSession->m_StaticCfg.eClassificationType &&
             VIDC_CLASSIFICATION_RANDOM_SEEK != pSession->m_StaticCfg.eClassificationType &&
             VIDC_CLASSIFICATION_ERROR != pSession->m_StaticCfg.eClassificationType)
    {
        VIDCTF_PRINT_ERROR(pParams->m_hSession, VIDC_ERR_INPUT_DATA_CORRUPT, "Frm:%d", pParams->m_bufDoneCnt);
        status = VidcStatusOutPutThreadError;
    }
    /* Transcode usecase - Encode ETB EOS(from decode FBD) with valid filledlen */
    else if (binfo->flags & V4L2_BUF_FLAG_EOS && binfo->plane.bytesused == 0)
    {
        VIDCTF_PRINT_HIGH("EOS on EBD but dont exit!!");
        status = VidcStatusEOSNoExit;
    }
    else if (pParams->m_bIsEOSReached)
    {
        /*FIXME: Temporary wait to avoid busy wait, when EOS reached on OutputThread*/
        usleep(50 * 1000);
        /*In this case, Buffer will be requeued to BufQ in QueueBuffer call*/
    }
    else
    {
        if (pSession->ProcessInputBuffer)
        {
            status = pSession->ProcessInputBuffer(pSession, binfo);
        }
        if (status != VidcStatusSuccess)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_INPUT, "buffer:%p,cnt:%d,status=%d",
                (void *)binfo->vaddr, pParams->m_bufSubmitCnt, status);
            status = VidcStatusOutPutThreadError;
        }
    }
    if (status != VidcStatusSuccess)
    {
        binfo->flags = 0x0;
        VideoPortParamsReQueueBuffer(pParams, binfo);
    }
    if(pSession->m_bEOS)
    {
        VIDCTF_PRINT_HIGH("EOS on EBD exit!!");
        status = VidcStatusEOSExit;
    }
    return status;
}
VidcStatus VideoPortParamsHandleCaptureBuf(VideoPortParams *pParams, BufInfo *binfo)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;
    VIDCTF_PRINT_INFO("BufferAvailable from Port-CAPTURE-idx:%d", binfo->index);

    status = pSession->ProcessOutputDone(pSession, binfo);
    if (binfo->plane.bytesused != 0)
    {
        /*Increment buffer done count for un-flushed buffers*/
        pParams->m_bufDoneCnt++;
    }
    if (status != VidcStatusSuccess)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_OUTPUT, "buffer:%p,cnt:%d,status=%d",
            (void *)binfo->vaddr, pParams->m_bufSubmitCnt, status);
        VideoPortParamsReQueueBuffer(pParams, binfo);
        status = VidcStatusCaptureThreadError;
    }
    else if (binfo->flags & V4L2_BUF_FLAG_EOS)
    {
        VIDCTF_PRINT_HIGH("EOS received exit CapturePort thread");
        pParams->PushEvent(pParams, binfo); //push EOS buffer back to bufferQ
        pSession->m_bEOS = TRUE;
        status = VidcStatusEOSExit;
    }
    //For non VT/Transcode session, clear buffer flag before passing buffer
    if (!pParams->m_hSession->m_parentSession)
    {
        binfo->flags = 0;
    }
    return status;
}
VidcStatus VideoPortParamsHandleMsgEvent(VideoPortParams *pParams,BufInfo *binfo)
{
    VidcStatus status = VidcStatusUnknownError;
    VideoSession* pSession = NULL;
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    VIDCTF_PRINT_HIGH("Event received:0x%x", binfo->msgType);
    if (binfo->msgType == V4L2_EVENT_MSM_VIDC_SYS_ERROR ||
        binfo->msgType == V4L2_EVENT_MSM_VIDC_HW_OVERLOAD ||
        binfo->msgType == V4L2_EVENT_MSM_VIDC_MAX_CLIENTS ||
        binfo->msgType == V4L2_EVENT_MSM_VIDC_HW_UNSUPPORTED ||
        binfo->msgType == SESSION_POLL_ERROR ||
        binfo->msgType == SESSION_IOCTL_ERROR ||
        binfo->msgType == SESSION_UNKNOWN_EVENT_ERROR ||
        binfo->msgType == SESSION_ALLOC_ERROR ||
        binfo->msgType == SESSION_CRCMISMATCH_ERROR
       )
    {
        VIDCTF_PRINT_WARN("Event received:0x%x Sumbitted %d BufDone %d", binfo->msgType, pParams->m_bufSubmitCnt, pParams->m_bufDoneCnt);
        if (binfo->msgType == SESSION_POLL_ERROR && pParams->m_bufQ)
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERTIMEOUT, "Polll timeout Driver:%d, AllocCount:%d  AvailCount: %d ", pParams->m_bufType, pParams->m_bufreq.count, pParams->m_bufQ->m_size);
    }
    else if (binfo->msgType == SESSION_EOS3_EXIT_MSG)
    {
        status = VidcStatusEOSExit;
    }
    else if (binfo->msgType == SESSION_FLUSH_MSG)
    {
        status = VideoPortParamsHandleFlush(pParams);
    }
    else if (binfo->msgType == SESSION_RECONFIG_MSG) //Reconfig msg only in case of encoder
    {
        status = pSession->PortReconfig(pSession);
    }
    else if (binfo->msgType == V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE)
    {
        //binfo->vaddr holds pointer to event data
        status = VideoPortParamsReleaseBufferReference(pParams,(void *)binfo->evtdata);
    }
    else if (binfo->msgType == V4L2_EVENT_MSM_VIDC_PORT_SETTINGS_CHANGED_INSUFFICIENT)
    {
        if (VIDC_CLASSIFICATION_SMOOTH_STREAMING == pSession->m_StaticCfg.eClassificationType)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_SMSTR_INSUFF_RESOURCE, "Insufficient Resource in smooth stream");
            status = VidcStatusCaptureThreadError;
        }
        else if (VideoPortParamsHandlePortReconfig(pParams) != VidcStatusSuccess)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_SMSTR_INSUFF_RESOURCE, "Failed in HandlePortReconfig");
            status = VidcStatusCaptureThreadError;
        }
        else
        {
            status = VidcStatusSuccess; // Port reconfig successfull
        }
    }
    else
    {
        VIDCTF_PRINT_WARN("Unknown Event Message Received:0x%x", binfo->msgType);
    }
    FREE(binfo);
    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::PrepareBufs()

   DESCRIPTION:  Prepare/Alloc/set buffers buffers to driver
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsPrepareBufs(VideoPortParams *pParams)
{
    int i = 0;
    BufInfo *binfo;
    struct v4l2_buffer buf;
    struct v4l2_plane plane[VIDEO_MAX_PLANES];
    int size = 0;
    int numbufs = 0;
    int align = SZ_4K;
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    int flag = 0;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    if (pParams->WaitForBufferAlloc) //Buffers need to be allocated by another session port
    {
        return pParams->WaitForBufferAlloc(pSession);
    }

    size = pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
    numbufs = pParams->m_bufreq.count;

    size = (size + (align - 1)) & (~(align - 1));
    VIDCTF_PRINT_HIGH("Port: %d: buffer size required per buffer: %d, num of buf: %d\n",
        pParams->m_bufType, size, numbufs);

    status = VideoPortParamsAllocExtraData(pParams);
    VIDEO_BAILOUT_ON_FAILURE("Allocate Extradata buffer", status, bailout);

    if (pSession->m_StaticCfg.bSecure)
    {
        if (pSession->m_type == VideoSessionTypeDecode)
        {
            flag = (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE?
                ION_FLAG_CP_PIXEL : ION_FLAG_CP_BITSTREAM) | ION_FLAG_SECURE;
        }
        else
        {
            flag = (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE?
                ION_FLAG_CP_BITSTREAM : ION_FLAG_CP_PIXEL) | ION_FLAG_SECURE;
        }
    }
    else if (((pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
        (gVideoDecProp.nCachedBufferMask & VIDC_BUFTYPE_OUTPUT)) ||
        ((pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) &&
        (gVideoDecProp.nCachedBufferMask & VIDC_BUFTYPE_INPUT)))
    {
        flag |= ION_FLAG_CACHED;
    }

    for (i = 0; i < numbufs; i++)
    {
        int extra_idx = 0;
        binfo = (BufInfo*)malloc(sizeof(BufInfo));
        if (!binfo)
        {
            VIDCTF_PRINT_WARN("Failed to allocate memory\n");
            status = VidcStatusAllocError;
            break;
        }

        memset(binfo, 0, sizeof(BufInfo));
        memset(plane, 0, sizeof(plane));

        binfo->size = size;

        status = VideoPortParamsAllocIONBuffer(pParams,  binfo, align, flag);
        if (status != VidcStatusSuccess || binfo->ion.ion_device_fd < 0)
        {
            VIDCTF_PRINT_WARN("Failed to allocate memory\n");
            status = VidcStatusAllocError;
            break;
        }
        if (!pSession->m_StaticCfg.bSecure)
        {
            binfo->vaddr = (uint8 *)mmap(NULL, size,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                binfo->ion.mapfd, 0);
            if (binfo->vaddr == MAP_FAILED)
            {
                VIDCTF_PRINT_WARN("Failed to get buffer virtual address\n");
                VideoPortParamsFreeIONMemory(pParams, &binfo->ion);
                status = VidcStatusAllocError;
                break;
            }
            else
            {
                status = VideoPortParamsStartBufferAccess(pSession, binfo->ion.mapfd);
                if (status != VidcStatusSuccess)
                {
                    munmap(binfo->vaddr, size);
                    VideoPortParamsFreeIONMemory(pParams, &binfo->ion);
                    status = VidcStatusAllocError;
                    goto bailout;
                }
            }
        }
        binfo->buf_type = pParams->m_bufType;
        binfo->index = i;
        buf.index = i;
        buf.type = pParams->m_bufType;
        buf.memory = V4L2_MEMORY_USERPTR;
        plane[0].length = size;
        plane[0].m.userptr = (unsigned long)binfo->vaddr;
#ifdef PRE_MSMNILE
        plane[0].reserved[MSM_VIDC_BUFFER_FD] = binfo->ion.fd_ion_data.fd;
#else
        plane[0].reserved[MSM_VIDC_BUFFER_FD] = binfo->ion.mapfd;
#endif
        plane[0].reserved[MSM_VIDC_DATA_OFFSET] = 0;
        plane[0].data_offset = binfo->offset;
        extra_idx = (pParams->m_fmt.fmt.pix_mp.num_planes - 1);
        if (((extra_idx >= 1) && (extra_idx < VIDEO_MAX_PLANES)))
        {
            buf.length = pParams->m_fmt.fmt.pix_mp.num_planes;
            plane[extra_idx].length = pParams->m_extraData.buffer_size;
            //plane[extra_idx].m.userptr = (long unsigned int)(pParams->m_extraData.uaddr + i * (ALIGN(pParams->m_extraData.buffer_size ,4)));
            plane[extra_idx].m.userptr = (long unsigned int)(pParams->m_extraData.ion[i].uaddr);
            plane[extra_idx].reserved[MSM_VIDC_BUFFER_FD] = pParams->m_extraData.ion[i].mapfd;
            plane[extra_idx].reserved[MSM_VIDC_DATA_OFFSET] = 0;
            plane[extra_idx].data_offset = 0;
            binfo->extradata_vaddr = (uint8*)plane[extra_idx].m.userptr;
            binfo->extraDataPlane = plane[extra_idx];
        }
        else
        {
            buf.length = 1;
        }
        //TODO: we can remove local variable plane
        buf.m.planes = plane;
        binfo->plane = plane[0];
        binfo->bufRefCount = 0;

        VIDCTF_PRINT_HIGH("Preparing Buffer port:%d : binfo: %p, vaddr: %p, fd: %d extradata_vaddr: %p\n",
            pParams->m_bufType, binfo, binfo->vaddr, binfo->ion.mapfd, binfo->extradata_vaddr);
        pParams->m_bufQ->Push(pParams->m_bufQ, &binfo->msgQueNode); // Push to buffer queue for data processing
        list_insert_tail(&binfo->bInfoListNode, &pParams->m_bInfoList);  // Store buffer information

        pParams->m_bInfoListSize++;
    }
    pParams->m_bAllocated = TRUE;
    if (pParams->SignalBufferAlloc)
    {
        pParams->SignalBufferAlloc(pSession);
    }
bailout:
    return status;
}

/*=============================================================================

FUNCTION:     VideoPortParamsSetBuffers()

DESCRIPTION:  Set buffers buffers to driver
*//*
PARAMETERS:
*//**    @param[in]  none
*//*
RETURN VALUE:
*/ /**       @return  none

=============================================================================*/
VidcStatus VideoPortParamsSetBuffers(VideoPortParams *pParams)
{
    int i=0;
    BufInfo *binfo;
    struct v4l2_buffer buf;
    struct v4l2_plane plane[VIDEO_MAX_PLANES];
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    pParams->m_bufreq.memory = V4L2_MEMORY_USERPTR;
    status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_REQBUFS, &pParams->m_bufreq));

    if (status != VidcStatusSuccess) return status;

    utils_list_for_each_entry(BufInfo,binfo,&pParams->m_bInfoList,bInfoListNode)
    {
        int extra_idx = 0;
        if (!binfo) return VidcStatusAllocError;
        memset(plane, 0, sizeof(plane));
        buf.index = binfo->index;
        buf.type = pParams->m_bufType;
        buf.memory = V4L2_MEMORY_USERPTR;

        binfo->size = pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
        plane[0].length = binfo->size;
        plane[0].m.userptr = (unsigned long)binfo->vaddr;
        plane[0].reserved[MSM_VIDC_BUFFER_FD] = binfo->ion.mapfd;
        plane[0].reserved[MSM_VIDC_DATA_OFFSET] = 0;
        plane[0].data_offset = binfo->offset;
        extra_idx = (pParams->m_fmt.fmt.pix_mp.num_planes - 1);
        if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
            ((extra_idx >= 1) && (extra_idx < VIDEO_MAX_PLANES)))
        {
            buf.length = pParams->m_fmt.fmt.pix_mp.num_planes;
            plane[extra_idx].length = pParams->m_extraData.buffer_size;
            plane[extra_idx].m.userptr = (long unsigned int)(pParams->m_extraData.ion[i].uaddr);
            plane[extra_idx].reserved[MSM_VIDC_BUFFER_FD] = pParams->m_extraData.ion[i].mapfd;
            plane[extra_idx].reserved[MSM_VIDC_DATA_OFFSET] = 0;
            plane[extra_idx].data_offset = 0;
            binfo->extradata_vaddr = (uint8*)plane[extra_idx].m.userptr;
            binfo->extraDataPlane = plane[extra_idx];
        }
        else
        {
            buf.length = 1;
        }
        buf.m.planes = plane;
        binfo->plane = plane[0];
        if (0)
        {
            VIDCTF_PRINT_HIGH("Setting Buffer port:%d : binfo: %p, vaddr: %p, size: %d %d",
                pParams->m_bufType, binfo, binfo->vaddr, pParams->m_fmt.fmt.pix_mp.plane_fmt[0].sizeimage,binfo->size);

        }
        i++;
    }

    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::FreeBuffers()

   DESCRIPTION:  Free buffers allocated
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsFreeBuffers(VideoPortParams *pParams)
{
    int rc = 0;
    VidcStatus status = VidcStatusSuccess;
    VideoSession* pSession = NULL;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    VIDCTF_PRINT_HIGH("Type:%d,Session:%d",pParams->m_bufType,pSession->m_type);

    if (pParams->m_extraData.size)
    {
        VidcExtraDataBufInfo *edata_info = &pParams->m_extraData;
        for(int i = 0; i < edata_info->count; i++) {
            if (edata_info->ion[i].ion_device_fd >= 0)
            {
                VideoPortParamsEndBufferAccess(pSession, edata_info->ion[i].mapfd);
                rc = munmap((void *)edata_info->ion[i].uaddr, edata_info->size);
                if (rc)
                {
                    VIDCTF_PRINT_WARN("Fail to munmap(%p, %u)", (void *)edata_info->ion[i].uaddr, edata_info->size);
                }
                else
                {
                    VIDCTF_PRINT_INFO("Successfully unmapped(%p, %u)\n", (void *)edata_info->ion[i].uaddr, edata_info->size);
                }
                VideoPortParamsFreeIONMemory(pParams, &edata_info->ion[i]);
            }
        }
    }
    /*Prioritize the INPUT_UNSUPPORTED error propagation*/
    while (!VideoPortParamsIsEmptyBufQ(pParams))
    {
        BufInfo *entry = VideoPortParamsGetBuffer(pParams);
        if(entry == NULL) break;
        if (entry->flags & V4L2_BUF_INPUT_UNSUPPORTED)
        {
            VIDCTF_PRINT_ERROR(pParams->m_hSession, VIDC_ERR_INPUT_UNSUPPORTED, "");
        }
        else if (entry->flags & V4L2_BUF_FLAG_DATA_CORRUPT)
        {
            if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE &&
                ( VIDC_CLASSIFICATION_FCN_DYN != pSession->m_StaticCfg.eClassificationType &&
                VIDC_CLASSIFICATION_RANDOM_SEEK != pSession->m_StaticCfg.eClassificationType &&
                VIDC_CLASSIFICATION_ERROR != pSession->m_StaticCfg.eClassificationType ))
            {
                VIDCTF_PRINT_ERROR(pParams->m_hSession, VIDC_ERR_INPUT_DATA_CORRUPT, "");
            }
            else
            {
                VIDCTF_PRINT_HIGH("Ignoring Output Data Corrupt flag");
            }
        }
        if (entry->msgType != SESSION_MSG_NONE)
        {
            if (entry->msgType == V4L2_EVENT_MSM_VIDC_RELEASE_BUFFER_REFERENCE)
            {
                status = VideoPortParamsReleaseBufferReference(pParams, (void *)entry->evtdata);
            }
            else
            {
                if (entry->msgType == SESSION_FLUSH_MSG)
                {
                    pSession->m_bIsFlushEventDropped = TRUE;
                }
                VIDCTF_PRINT_WARN("Pending Message event:0x%x during FreeBuffers", entry->msgType);
            }
            FREE(entry);
            continue;
        }
    }

    while (!list_is_empty(&pParams->m_bInfoList))
    {
        BufInfo *binfo =  list_get_node_data(list_get_head(&pParams->m_bInfoList), BufInfo, bInfoListNode);
        //Mark buffer for deffered FREE
        pthread_mutex_lock(&pParams->m_mutex);
        if (pSession->m_type == VideoSessionTypeDecode &&
            pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
            binfo->bufRefCount )
        {
            if ((pSession->m_StaticCfg.eCodecType == V4L2_PIX_FMT_VP9 || pSession->m_StaticCfg.eCodecType == V4L2_PIX_FMT_VP8) &&
                pSession->m_bInReconfig)
            {
                binfo->releaseFlag |= MARK_DEFERRED_FREE;
                VIDCTF_PRINT_HIGH("Marking Buffer:%p as FREED", binfo->vaddr);
            }
            else
            {
                VIDCTF_PRINT_ERROR(pParams->m_hSession, VIDC_ERR_MISC, "Missing Release reference for %p", binfo->vaddr);
            }
        }
        if (!(binfo->releaseFlag & MARK_DEFERRED_FREE))
        {
            VIDCTF_PRINT_INFO("Freeing %p", (void*)binfo->vaddr);
            if (binfo->ion.ion_device_fd >= 0)
            {
                if (!pSession->m_StaticCfg.bSecure)
                {
                    VideoPortParamsEndBufferAccess(pSession, binfo->ion.mapfd);
                }
                if (binfo->vaddr)
                {
                    rc = munmap((void *)binfo->vaddr, binfo->size);
                    if (rc)
                    {
                        VIDCTF_PRINT_WARN("Fail to munmap(%p, %u)", (void *)binfo->vaddr, binfo->size);
                    }
                    else
                    {
                        VIDCTF_PRINT_INFO("Successfully unmapped(%p, %u)\n", (void *)binfo->vaddr, binfo->size);
                    }
                }
                VideoPortParamsFreeIONMemory(pParams, &binfo->ion);
            }
            list_remove_node(&pParams->m_bInfoList, &binfo->bInfoListNode);
            free(binfo);
            pParams->m_bInfoListSize--;
        }
        else
        {
            list_remove_node(&pParams->m_bInfoList, &binfo->bInfoListNode);
            pParams->m_bInfoListSize--;
            list_insert_tail(&binfo->bInfoListNode, &pParams->m_bInfoDefFreeList);
            pParams->m_bInfoDefFreeListSize++;
    }
        pthread_mutex_unlock(&pParams->m_mutex);
    }

    pParams->m_bAllocated = FALSE;
    return status;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::ProcessBufferDone()

   DESCRIPTION:  Process Output/Capture Buffer Done
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsProcessBufferDone(VideoPortParams *pParams)
{
    VidcStatus status = VidcStatusSuccess;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane plane[VIDEO_MAX_PLANES];
    BufInfo *binfo;

    VideoSession* pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    memset(&v4l2_buf, 0, sizeof(v4l2_buf));
    memset(&plane, 0, sizeof(plane));

    v4l2_buf.memory = V4L2_MEMORY_USERPTR;
    v4l2_buf.m.planes = plane;
    v4l2_buf.length = pParams->m_fmt.fmt.pix_mp.num_planes;
    v4l2_buf.type = pParams->m_bufType;

    while (!ioctl(pSession->m_hDriver, VIDIOC_DQBUF, &v4l2_buf))
    {
        //TODO: For dynamic  buffer mode use mutex, if we add buffers runtime
        binfo = BufInfoListIndex(&pParams->m_bInfoList, v4l2_buf.index);
        if (!binfo)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BADPARAM, "Invalid buffer: SESSION_UNKNOWN_EVENT_ERROR");
            return VidcStatusBadParamError;
        }
        binfo->plane = plane[0];
        binfo->flags = v4l2_buf.flags;
        binfo->timestamp = v4l2_buf.timestamp;
#if TAGDATA_ENABLE
        if (v4l2_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            binfo->input_tag = plane[0].reserved[MSM_VIDC_INPUT_TAG_1];
            binfo->input_tag2 = plane[0].reserved[MSM_VIDC_INPUT_TAG_2];
        }
#endif
        if ((pParams->m_fmt.fmt.pix_mp.num_planes > 1) &&
            (pParams->m_fmt.fmt.pix_mp.num_planes < VIDEO_MAX_PLANES)
            )
        {
            binfo->extraDataPlane = plane[v4l2_buf.length - 1];
        }
        //For read only buffer, set increment buf ref count and expect release buf event.
        //Dequeu of this buffer needs to happen first.
        pthread_mutex_lock(&pParams->m_mutex);
        if (binfo->flags & V4L2_BUF_FLAG_READONLY)
        {
            binfo->bufRefCount++;
        }
        pthread_mutex_unlock(&pParams->m_mutex);

        VIDCTF_PRINT_HIGH("Queued BufDone to PortBufQ:%d, buffer:%p Length:%d,session:%d", pParams->m_bufType, binfo->vaddr, binfo->plane.bytesused,pSession->m_type);
        pParams->TransferBuffer(pParams, binfo);
        if (pParams->m_bIsEOS3Reached && pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        {
            if (--pParams->m_nEOS3QueudBufCount == 0)//after receiving ETBs count terminating threads
            {
                pSession->m_portParam[CAPTURE_PORT]->PostEvent(pSession->m_portParam[CAPTURE_PORT],
                SESSION_EOS3_EXIT_MSG, NULL);
                pSession->m_portParam[OUTPUT_PORT]->PostEvent(pSession->m_portParam[OUTPUT_PORT],
                SESSION_EOS3_EXIT_MSG, NULL);
            }
        }
        memset(&v4l2_buf, 0, sizeof(v4l2_buf));
        v4l2_buf.memory = V4L2_MEMORY_USERPTR;
        v4l2_buf.m.planes = plane;
        v4l2_buf.length = pParams->m_fmt.fmt.pix_mp.num_planes;
        v4l2_buf.type = pParams->m_bufType;
    }
    return status;
}

VidcStatus VideoPortParamsPushEvent(VideoPortParams *pParams, BufInfo *binfo)
{
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pParams->m_bufQ->Push(pParams->m_bufQ, &binfo->msgQueNode);
    return VidcStatusSuccess;
}
VidcStatus VideoPortParamsReleaseUnQueuedBuffer(VideoPortParams *pParams, void* pData)
{
    VidcStatus status = VidcStatusSuccess;
    BufInfo *binfo;
    unsigned int *ptr = NULL;
    uint32 bufIdx = 0;
    if (!pParams || !pData)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    ptr = (unsigned int *)pData;
    bufIdx = ptr[5];
    VIDCTF_PRINT_HIGH("Release unqueued buffer event recvd fd = %d offset = %d idx = %d", ptr[0], ptr[1], bufIdx);

    if (bufIdx > pParams->m_bufreq.count)
    {
        VIDCTF_PRINT_WARN("Invalid Buffer Idx returned by driver:%u", bufIdx);
        return VidcStatusBadParamError;
    }
    binfo = BufInfoListIndex(&pParams->m_bInfoList, bufIdx);
    if (!binfo)
    {
        VIDCTF_PRINT_WARN("Invalid buffer: SESSION_UNKNOWN_EVENT_ERROR");
        return VidcStatusBadParamError;
    }
    if ((((unsigned long)(intptr_t)binfo->vaddr) & 0xFFFFFFFF) != (unsigned long)(intptr_t)ptr[2]) //FIXME: need to investigate 7f<32 bits> address issue
    {
        VIDCTF_PRINT_HIGH("Invalid Buffer addr returned by driver:%p - %p", binfo->vaddr,
            (void*)(intptr_t)ptr[2]);
        return VidcStatusBadParamError;
    }
    binfo->flags = 0;
    binfo->timestamp.tv_sec = (uint64_t)ptr[3];
    binfo->timestamp.tv_usec = (uint64_t)ptr[4];
    binfo->plane.bytesused = 0;

    VIDCTF_PRINT_HIGH("Queued BufDone to PortBufQ:%d, buffer:%p Length:%d", pParams->m_bufType, binfo->vaddr, binfo->plane.bytesused);
    status = pParams->TransferBuffer(pParams, binfo);
    return status;
}
VidcStatus VideoPortParamsPostEvent(VideoPortParams *pParams, uint32 evtType, uint8* data)
{
    VidcStatus status = VidcStatusSuccess;
    VideoSession *pSession = NULL;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    VIDCTF_PRINT_HIGH("PostEvent:0x%x to Port:%d", evtType, pParams->m_bufType);
    if (!pSession->m_bIsFlushEventDropped)
    {
        if (SESSION_FLUSH_MSG == evtType && pSession->m_pFlushMsgNode)
        {
            VIDCTF_PRINT_HIGH("Already have pending flush msg in Port:%d with node:%p", pParams->m_bufType, pSession->m_pFlushMsgNode);
            //If flush command already issued and pending flush then issue flush both ports to driver

            status = pSession->Flush(pSession, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE | V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
            //Remove flush msg node in the pending queue
            list_remove_node(&pParams->m_bufQ->m_queue, &pSession->m_pFlushMsgNode->msgQueNode);
            FREE(pSession->m_pFlushMsgNode);
            pthread_cond_broadcast(&pSession->m_portThreadsCondWait);
            return VidcStatusOperationError;
        }
    }
    BufInfo *binfo = (BufInfo*)malloc(sizeof(BufInfo));
    if (binfo)
    {
        memset(binfo, 0, sizeof(BufInfo));
        binfo->msgType = evtType; //Create Dummy Packet and Store it to BufferQueue
        if(data)
           memcpy(binfo->evtdata,data,sizeof(binfo->evtdata));
        pParams->PushEvent(pParams, binfo);
        if (SESSION_FLUSH_MSG == evtType)
        {
            pSession->m_pFlushMsgNode = binfo;
            VIDCTF_PRINT_HIGH("Updating flush msg node from port:%d", pParams->m_bufType);
        }
    }
    else
    {
        status = VidcStatusAllocError;
    }
    return status;
}

BufInfo* VideoPortParamsGetBuffer(VideoPortParams *pParams)
{
    BufInfo *binfo = NULL;
    LIST_NODE *node = pParams->m_bufQ->PopFront(pParams->m_bufQ);
    if (node != NULL)
    {
        binfo = list_get_node_data(node, BufInfo, msgQueNode);
    }
    return binfo;
}
void VideoPortParamsReQueueBuffer(VideoPortParams *pParams, BufInfo *binfo)
{
    if (!pParams || !binfo)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return;
    }
    VIDCTF_PRINT_HIGH("ReQueueBuffer(%d) buf:%p index:%d length:%d flags:0x%x",
        pParams->m_bufType, binfo->vaddr, binfo->index, binfo->plane.bytesused, binfo->flags);
    binfo->plane.bytesused = 0; // Clearing filledlen information
    if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
    {
        binfo->flags &= ~V4L2_BUF_FLAG_EOS;
    }
    VIDCTF_PRINT_HIGH("ReQueueBuffer(%d) after reset buf:%p index:%d length:%d flags:0x%x",
        pParams->m_bufType, binfo->vaddr, binfo->index, binfo->plane.bytesused, binfo->flags);
    pParams->m_bufQ->Push(pParams->m_bufQ, &binfo->msgQueNode);
}
BOOL VideoPortParamsIsEmptyBufQ(VideoPortParams *pParams)
{
    BOOL bIsEmpty;
    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pthread_mutex_lock(&pParams->m_mutex);
    bIsEmpty = list_is_empty(&pParams->m_bufQ->m_queue);
    pthread_mutex_unlock(&pParams->m_mutex);

    return bIsEmpty;
}

void *VideoPortParamsDataThreadRun(void *pPortParam)
{
        BufInfo *binfo = NULL;
        VidcStatus status = VidcStatusSuccess;
        VideoPortParams *pParams = NULL;

        VideoSession *pSession = NULL;
        if (!pPortParam)
        {
           VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
           return NULL;
        }
        pParams = (VideoPortParams*)pPortParam;
        pSession = (VideoSession*)pParams->m_hSession;

        VIDCTF_PRINT_HIGH("VidcDataThreadRun thread port:%d session:%d", pParams->m_bufType,pSession->m_type);

        if (!pParams->m_bAllocated)
        {
           /** Allocate and Queue Capture port buffers to driver */
           status = VideoPortParamsPrepareBufs(pParams);
           VIDEO_BAILOUT_ON_FAILURE("PrepareBufs", status, bailout);
        }

        while (1)
        {
            binfo = VideoPortParamsGetBuffer(pParams);

            if (binfo)
            {
               VIDCTF_PRINT_INFO("ThreadRun: param buftype %d buf type %d, msg type 0x%x EOSflag 0x%x",
                                  pParams->m_bufType,
                                  binfo->buf_type,
                                  binfo->msgType,
                                  (binfo->flags & V4L2_BUF_FLAG_EOS));

               if (binfo->msgType != SESSION_MSG_NONE)
               {
                  status = VideoPortParamsHandleMsgEvent(pParams,binfo);
                  if (status == VidcStatusSuccess)
                     continue;
               }
               else if (binfo->buf_type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
               {
                  status = VideoPortParamsHandleOutPutBuf(pParams, binfo);
               }
               else if (binfo->buf_type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
               {
                  status = VideoPortParamsHandleCaptureBuf(pParams, binfo);
               }
               else
               {
                  status = VidcStatusUnknownError;
                  VIDCTF_PRINT_WARN("Invalid Buffer event!!");
               }

               if (status == VidcStatusSuccess)
               {
                  status = pParams->PushBuffer(pParams, binfo);
               }
               if (status == VidcStatusEOSExit)
               {
                  VIDCTF_PRINT_HIGH("EOS Reached.Port=%d ID:%s", pParams->m_bufType, pSession->m_sTestID);
                  break;
               }
               if (status != VidcStatusSuccess)
               {
                  if (status == VidcStatusCaptureThreadError)
                  {
                     pSession->m_portParam[OUTPUT_PORT]->PostEvent(pSession->m_portParam[OUTPUT_PORT],
                                                                   SESSION_UNKNOWN_EVENT_ERROR, NULL);
                  }
                  else if (status == VidcStatusOutPutThreadError)
                  {
                     pSession->m_portParam[CAPTURE_PORT]->PostEvent(pSession->m_portParam[CAPTURE_PORT],
                                                                    SESSION_UNKNOWN_EVENT_ERROR, NULL);
                  }
                  break;
               }
            }
        }
bailout:
        return NULL;
}

/*=============================================================================

   FUNCTION:     VideoPortParams::QueueBuffer()

   DESCRIPTION:  Queue buffer to the driver for processing
   *//*
   PARAMETERS:
   *//**    @param[in]  none
   *//*
   RETURN VALUE:
   */ /**       @return  none

   =============================================================================*/
VidcStatus VideoPortParamsQueueBuffer(VideoPortParams *pParams, BufInfo *binfo)
{
    VidcStatus status = VidcStatusSuccess;
    struct v4l2_buffer buf;
    struct v4l2_plane plane[VIDEO_MAX_PLANES];
    int extra_idx = 0;
    unsigned long bufptr;
    BOOL bIsEOSReached;
    VideoSession *pSession = NULL;
    BOOL bBinfoEosFlag = FALSE;

    if (!pParams)
    {
        VIDCTF_PRINT_WARN("VIDC_ERR_BADPARAM NULL data received\n");
        return VidcStatusBadParamError;
    }
    pSession = (VideoSession*)pParams->m_hSession;

    if (!binfo)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_ALLOC, "PortQueueBuffer");
        return VidcStatusBadParamError;
    }

    //When buffer tunnelled through with EOSEQ flag, treat it as EOS flag
    if (pParams->m_bIsEOSReached || (pParams->m_bIsEOS3Reached &&
        pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))
    {
        /* If thread already reached EOS, no need to submit any more buffers to driver */
        VIDCTF_PRINT_HIGH("EOS reached, requeue buffers as they are not to be submitted to driver"
                          "Buf type %d flag 0x%x\n",
                          binfo->buf_type, binfo->flags);
        binfo->flags &= ~V4L2_BUF_FLAG_EOS;
        VideoPortParamsReQueueBuffer(pParams, binfo);

        return VidcStatusSuccess;
    }

    bIsEOSReached = (binfo->flags & V4L2_BUF_FLAG_EOS) ? TRUE : FALSE;

    bufptr = (unsigned long)binfo->vaddr;

    if ((binfo->flags & V4L2_BUF_FLAG_EOS) &&
        (pSession->m_StaticCfg.eEosType != VIDC_FRAME_FLAG_EOS0))
    {
        if (binfo->plane.bytesused != 0)
        {
            binfo->flags &= ~V4L2_BUF_FLAG_EOS;
        }
    }

    memset(&buf, 0, sizeof(buf));
    memset(&plane, 0, sizeof(plane));

    buf.index = binfo->index;
    buf.type = binfo->buf_type;
    buf.flags = binfo->flags;
    buf.memory = V4L2_MEMORY_USERPTR;
#if TAGDATA_ENABLE
    if (buf.type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
#if 0
        /* FIXME: TagData validation after Seek/Flush.
         *
         * When Seek/Flush occurs, the Tagdata Validation which happpens for every buffer
         * in ProcessOutputDone by validating m_pTagDataArray[input_tag] == 0 needs to be
         * memset to Zero. So that when buffers are queued again for the validation,
         * can pass for every buffers. Flush happens in Async thread, since FLUSH is a event,
         * it has a higher priorty than the ProcessOutputDone happens on base thread.
         * Therefore in some cases, we are getting Trigger from Driver as SESSION_FLUSH_DONE
         * way before all the buffers are processed and validated in base thread.
         * Therefore memset of m_pTagDataArray to 0x00 happens, before all the buffers are
         * processed, which ideally should have happened after all the buffers have been
         * processed by ProcessOutputDone. So the validation(m_pTagDataArray[input_tag] == 0)
         * of some buffer which remained before flush becomes true.
         * Therefore, when the same buffers are queued after the flush for validation, it FAILS.
         */

        if (pSession->m_skipTagDataValidation) {
            VIDCTF_PRINT_HIGH("TagData validation Enabled!");
            memset(pSession->m_pTagDataArray, 0x00, pSession->m_pStreamParser->m_nTotalInputFrames);
            pSession->m_skipTagDataValidation = FALSE;
        }
#endif
        plane[0].reserved[MSM_VIDC_INPUT_TAG_1] = binfo->input_tag;
        plane[0].reserved[MSM_VIDC_INPUT_TAG_2] = binfo->input_tag2;
    }
#endif
    plane[0].bytesused = binfo->plane.bytesused;
    plane[0].length = binfo->size;
    plane[0].m.userptr = bufptr;
    plane[0].reserved[MSM_VIDC_BUFFER_FD] = binfo->ion.mapfd;
    plane[0].reserved[MSM_VIDC_DATA_OFFSET] = 0;
    plane[0].data_offset = binfo->offset;
    extra_idx = (pParams->m_fmt.fmt.pix_mp.num_planes - 1);
        if ((extra_idx >= 1) && (extra_idx < VIDEO_MAX_PLANES))
        {
            plane[extra_idx].length = pParams->m_extraData.size;
            plane[extra_idx].m.userptr = (unsigned long)binfo->extradata_vaddr;
            plane[extra_idx].reserved[MSM_VIDC_BUFFER_FD] = pParams->m_extraData.ion[buf.index].mapfd;
            plane[extra_idx].reserved[MSM_VIDC_DATA_OFFSET] = 0;
            plane[extra_idx].data_offset = 0;
            /* vl42 expects bytesused includes data_offset */
            plane[extra_idx].bytesused = plane[extra_idx].data_offset + binfo->extradata_filled_len;
            buf.length = pParams->m_fmt.fmt.pix_mp.num_planes;
            VIDCTF_PRINT_INFO("Queueing extra data: port:(%d), index: %d, fd = %d,reserved[MSM_VIDC_DATA_OFFSET] = %u, userptr = %p,"
                " offset = %d, flags=0x%x, bytesused= %d, length= %d, num_planes = %d\n",
                binfo->buf_type, buf.index,
                plane[extra_idx].reserved[MSM_VIDC_BUFFER_FD], plane[extra_idx].reserved[MSM_VIDC_DATA_OFFSET],
                (void *)plane[extra_idx].m.userptr,
                plane[extra_idx].data_offset, buf.flags,
                plane[extra_idx].bytesused, plane[extra_idx].length, buf.length);
        }
        else
        {
            buf.length = 1;
        }

    buf.timestamp = binfo->timestamp;
    buf.m.planes = plane;

    if (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE && binfo->plane.bytesused == 0) {
        struct v4l2_decoder_cmd dec;

        // call streamon before sending stop command if streamon is not called yet.
        if (!pParams->m_bIsStreamOnOutputPlane)
        {
            status = VideoPortParamsStreamOn(pParams);
        }

        if (status == VidcStatusSuccess)
        {
            memset(&dec, 0, sizeof(dec));
            dec.cmd = V4L2_DEC_CMD_STOP;
            status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_DECODER_CMD, &dec));

            VIDCTF_PRINT_HIGH("This is Zero length buffer. Converted into STOP comamnd and queued\n");
            bBinfoEosFlag = TRUE;
            binfo->flags &= ~V4L2_BUF_FLAG_EOS;
            VideoPortParamsReQueueBuffer(pParams, binfo);
        }
        else
        {
            VIDCTF_PRINT_WARN("STOP command is not queued to Driver as StreamOn on outplane failed\n");
        }
    } else {
            VIDCTF_PRINT_INFO("Queueing buffer: port:(%d), index: %d, fd = %d,reserved[MSM_VIDC_DATA_OFFSET] = %u, userptr = %p,"
                " offset = %d, flags=0x%x, bytesused= %d, length= %d, ts= %ld-%ld\n",
                binfo->buf_type, buf.index,
                plane[0].reserved[MSM_VIDC_BUFFER_FD], plane[0].reserved[MSM_VIDC_DATA_OFFSET],
                (void *)plane[0].m.userptr,
                plane[0].data_offset, buf.flags,
                plane[0].bytesused, plane[0].length,
                buf.timestamp.tv_sec, buf.timestamp.tv_usec);
                status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_QBUF, &buf));

            if (status == VidcStatusSuccess) {
                pthread_mutex_lock(&pParams->m_mutex);
                if ((!(pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE &&
                        binfo->plane.bytesused != 0)) && (!bBinfoEosFlag))
                {
                    pParams->m_bufSubmitCnt++;
                    VIDCTF_PRINT_HIGH("Increment Buf submit count to %d", pParams->m_bufSubmitCnt);
                }
                pthread_mutex_unlock(&pParams->m_mutex);
                VIDCTF_PRINT_HIGH("Submit %d buffer:%p flags:0x%x Cnt:%d", pParams->m_bufType, (void*)bufptr,
                    buf.flags, pParams->m_bufSubmitCnt);
            }
    }

    if (status != VidcStatusSuccess)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_IOCTL, "Q_BUF port(%d)session:%d buf:%p",
            binfo->buf_type, pSession->m_type, (void*)bufptr);
        status = (pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) ?
                VidcStatusCaptureThreadError : VidcStatusOutPutThreadError;
    }
    else
    {
        VIDCTF_PRINT_INFO("QueueBuffer(%d)-%d succeed\n", binfo->buf_type, binfo->index);
        if ((bIsEOSReached) &&
            (pSession->m_StaticCfg.eEosType == VIDC_FRAME_FLAG_EOS3))
        {
            if (pParams->m_threadID && pParams->m_bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
            {
                VIDCTF_PRINT_HIGH("Exiting OutputPort thread with EOS3 option");
                pSession->m_portParam[CAPTURE_PORT]->PostEvent(pSession->m_portParam[CAPTURE_PORT],
                SESSION_EOS3_EXIT_MSG, NULL);
                status = VidcStatusEOSExit;
            }
            else//if EOS reached before creating threads then only this code will be executed
            {
                pParams->m_bIsEOS3Reached = TRUE;
                pParams->m_nEOS3QueudBufCount = pParams->m_bufreq.count - pParams->m_bufQ->m_size;
                //m_nEOS3QueudBufCount how many ETBs are queued to driver
            }
        }
        else if (pParams->m_bIsEOSReached)
        {
            //As a result of V4L2_QCOM_BUF_FLAG_EOSEQ in buffer flag
            status = VidcStatusEOSExit;
            VIDCTF_PRINT_HIGH("Tunneled EOS buffer recieved, exiting thread!!");
        }
        else if (bBinfoEosFlag)
        {
            VIDCTF_PRINT_HIGH("EOS received on OutputPort thread");
            pParams->m_bIsEOSReached = TRUE;
        }
    }
    return status;
}

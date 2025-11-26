/*
 **************************************************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "VideoDecoder.h"
#include "VideoRender.h"
#include "VideoPlatform.h"

#define GETDECOBJ(pSession) ((VideoDecoder *)pSession)

static int count = 0;

struct buffers{

    uint8 *uv, *y;
};

/* DRM specific global variables */
extern int buf_idx;
extern std::vector<connector_config> connector_cfg;
extern std::vector<plane_config> plane_cfg;
extern int fd;
extern std::mutex plane_mutex;
extern drmModeAtomicReqPtr req;

VidcStatus VideoDecoderOpen(VideoSession *pSession);
VidcStatus VideoDecoderClose(VideoSession* pSession);
VidcStatus VideoDecoderSetDefaultConfig(VideoSession *pSession);
VidcStatus VideoDecoderStop(VideoSession *pSession);
VidcStatus VideoDecoderProcessOutputDone(VideoSession *pSession, BufInfo *pBufInfo);
VidcStatus VideoDecoderFlush(VideoSession *pSession, uint32 bufType);
VidcStatus VideoPortParamsDecCaptureReconfig(VideoSession* pSession);
uint32     VideoDecoderGetPortFormat(VideoSession *pSession, enum v4l2_buf_type bufType);
uint32     VideoDecoderGetFrameDoneCount(VideoSession *pSession);
void       VideoDecoderDestroy(VideoSession *pSession);
void       VideoDecoderReleaseBufferReference(VideoSession *pSession, uint32 fd, uint32 offset);
void       VideoDecoderDeinit(VideoSession* pSession);

/*=============================================================================

     FUNCTION:     VideoDecoder::VideoDecoder()

     DESCRIPTION:  Initialize the video decoder members, and store the
                   parent context handle passed.Initialize video session
                   baseclass with true for decoder.
*//*
     PARAMETERS:
*//**    @param[in]  hCtxt - Parent video context handle
*//*
    RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VideoSession* VideoDecoderInit()
{
    VidcStatus status = VidcStatusSuccess;
    VideoDecoder *pDec = (VideoDecoder*)malloc(sizeof(VideoDecoder));
    if (!pDec)
    {
       return NULL;
    }

    memset(pDec, 0, sizeof(VideoDecoder));
    status = VideoSessionInit(&pDec->m_session, VideoSessionTypeDecode);
    VIDEO_BAILOUT_ON_FAILURE("VideoSessionInit", status, bailout);

    pDec->m_session.DeInit                         = VideoDecoderDeinit;
    pDec->m_session.Open                           = VideoDecoderOpen;
    pDec->m_session.Stop                           = VideoDecoderStop;
    pDec->m_session.Close                          = VideoDecoderClose;
    pDec->m_session.SetDefaultConfig               = VideoDecoderSetDefaultConfig;
    pDec->m_session.GetPortFormat                  = VideoDecoderGetPortFormat;
    pDec->m_session.ProcessOutputDone              = VideoDecoderProcessOutputDone;
    pDec->m_session.FlushDriver                    = VideoDecoderFlush;
    pDec->m_session.GetFrameDoneCount              = VideoDecoderGetFrameDoneCount;
    pDec->m_session.PortReconfig                   = VideoPortParamsDecCaptureReconfig;

    pDec->m_session.m_StaticCfg.nDisplayBufCnt     = gVideoDecProp.nDisplayBufCnt;
    pDec->m_session.m_StaticCfg.bDynamicBufMode    = gVideoDecProp.bDynamicBufMode;

bailout:
    if (status != VidcStatusSuccess)
    {
        free(pDec);
        return NULL;
    }
    return &pDec->m_session;
}

/*=============================================================================

FUNCTION:     VideoDecoder::InitDriver()

DESCRIPTION:  Initialise a decoder session and create driver handle
*//*
PARAMETERS:
*//**    @param[in]  none
*//*
RETURN VALUE:
*/ /**       @return  vidcStatusSuccess - if operation succeeds

=============================================================================*/
VidcStatus VideoDecoderOpen(VideoSession* pSession)
{
    VidcStatus status = VidcStatusSuccess;
    VideoDecoder *pDec = GETDECOBJ(pSession);

#ifdef __EARLYSERVICES__
    pSession->m_hDriver = open(EARLY_VID_DEC_DEVICE, O_RDWR);
#else
    pSession->m_hDriver = open(VID_DEC_DEVICE, O_RDWR);
#endif

    if (pSession->m_hDriver <= 0)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_DEVICEINIT, "Decoder");
        pSession->m_hDriver = 0;
        return VidcStatusAllocError;
    }

    pSession->m_hPollEventFd = eventfd(0, 0);
    if (pSession->m_hPollEventFd <= 0)
    {
        VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_DEVICEINIT, "Decoder Poll Event Handle");
        pSession->m_hPollEventFd = 0;
        return VidcStatusAllocError;
    }
    pSession->m_StaticCfg.eConfiguredColorFmt = V4L2_PIX_FMT_NV12;

    pSession->m_StaticCfg.eColorFmt = pSession->m_StaticCfg.eConfiguredColorFmt;
    pSession->m_StaticCfg.eBufferFormat = 0xFFFFFFFF; //Not valid for decoder

    status = VideoSessionOpen(pSession);
    VIDEO_BAILOUT_ON_FAILURE("VideoSessionOpen()", status, bailout);

    if ((gVideoDecProp.outputDump) && !pSession->m_StaticCfg.bSecure)
    {
        char sOutFileName[MAX_STR_LEN];
        STRCPY(sOutFileName, gVideoDecProp.sOutRoot);
        STRLCAT(sOutFileName, "out.yuv", sizeof(sOutFileName));
        pSession->m_outStream = fopen(sOutFileName, "wb+");
        if (!pSession->m_outStream)
        {
            VIDCTF_PRINT_WARN("Unable to open output file - %s", sOutFileName);
        }
        VIDCTF_PRINT_HIGH("Open file for writing YUV - %s success", sOutFileName);
    }
    else
    {
        pSession->m_outStream = NULL;
    }

    pSession->m_nBitDepthMode = MSM_VIDC_BIT_DEPTH_8;
    pSession->m_bProgressive = MSM_VIDC_PIC_STRUCT_PROGRESSIVE;
    pSession->m_eColorSpace = EXCEPT_BT2020;
    return status;
bailout:

    return status;
}

uint32 VideoDecoderGetFrameDoneCount(VideoSession *pSession)
{
    VideoDecoder *pDec = GETDECOBJ(pSession);
    uint32 nFrameDoneCnt = 0;

    nFrameDoneCnt = pSession->m_portParam[CAPTURE_PORT]->m_bufDoneCnt;
    return nFrameDoneCnt;
}

void  VideoDecoderDeinit(VideoSession* pSession)
{
    VideoDecoder *pDec = GETDECOBJ(pSession);
    VideoSessionDeInit(pSession);
    free((VideoDecoder*)pSession);
}

VidcStatus VideoDecoderClose(VideoSession *pSession)
{
    VideoDecoder *pDec = GETDECOBJ(pSession);

#if TAGDATA_ENABLE
    FREE(pSession->m_pTagDataArray);
#endif

    return VideoSessionClose(pSession);
}

uint32 VideoDecoderGetPortFormat(VideoSession *pSession, enum v4l2_buf_type bufType)
{
   uint32 format;
   if (bufType == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
   {
       format = pSession->m_StaticCfg.eCodecType;
   }
   else
   {
       format = pSession->m_StaticCfg.eColorFmt;
   }
   return format;
}

/*=============================================================================

     FUNCTION:     VideoDecoder::Flush()

     DESCRIPTION:  Issue flush ioctl to driver
*//*
     PARAMETERS:
*//**    @param[in]  none
*//*
    RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoDecoderFlush(VideoSession* pSession, uint32 bufType)
{
   VidcStatus status = VidcStatusSuccess;
   VideoDecoder *pDec = GETDECOBJ(pSession);
   struct v4l2_decoder_cmd dec;


   VIDCTF_PRINT_HIGH("Flushing Driver...");

   dec.flags = 0;

   if ((bufType & V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) ==
        V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
   {
      VIDCTF_PRINT_INFO("Flush Capture port");
      dec.flags |= V4L2_CMD_FLUSH_CAPTURE;
   }
   if ((bufType & V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) ==
       V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
   {
      VIDCTF_PRINT_INFO("Flush Output port");
      dec.flags |= V4L2_CMD_FLUSH_OUTPUT;
   }
   if (dec.flags == (V4L2_CMD_FLUSH_OUTPUT | V4L2_CMD_FLUSH_CAPTURE))
   {
        pSession->m_skipTagDataValidation = TRUE;
        VIDCTF_PRINT_HIGH("TagData validation Disabled!!");
   }

   dec.cmd = V4L2_CMD_FLUSH;
   status = VIDC_ERROR(ioctl(pSession->m_hDriver, VIDIOC_DECODER_CMD, &dec));
   VIDEO_BAILOUT_ON_FAILURE("ioctl FLUSH_CMD", status, bailout);

bailout:
   return status;
}

/*=============================================================================

     FUNCTION:     VideoDecoderStop()

     DESCRIPTION:  Stop Driver
*//*
     PARAMETERS:
*//**    @param[in]  none
*//*
    RETURN VALUE:
*/ /**       @return  none

 =============================================================================*/
VidcStatus VideoDecoderStop(VideoSession* pSession)
{
    VidcStatus status = VidcStatusSuccess;
    VideoDecoder *pDec = GETDECOBJ(pSession);
    char temp;

    VIDCTF_PRINT_HIGH("Stopping...\n");

    status = VIDC_ERROR(eventfd_write(pSession->m_hPollEventFd, 1));
    VIDEO_BAILOUT_ON_FAILURE("ioctl CMD_STOP", status, bailout);
    pthread_join(pSession->m_hAsyncThread, NULL);

bailout:
   return status;
}


/*=============================================================================

     FUNCTION:     VideoDecoder::ConfigDriver()

     DESCRIPTION:  Initialise a decoder session and create driver handle
*//*
     PARAMETERS:
*//**    @param[in]  none
*//*
    RETURN VALUE:
*/ /**       @return  vidcStatusSuccess - if operation succeeds

 =============================================================================*/
VidcStatus VideoDecoderSetDefaultConfig(VideoSession* pSession)
{
   VidcStatus status = VidcStatusSuccess;
   VideoDecoder *pDec = GETDECOBJ(pSession);
   uint32 nOutput2Width = 0, nOutput2Height = 0;

   if (VIDC_CLASSIFICATION_SMOOTH_STREAMING == pSession->m_StaticCfg.eClassificationType)
   {
       //TODO: driver only supports static buffer mode for smooth stream for now. Once they support for dynamic buffer mode then remove this check
       pSession->m_StaticCfg.bDynamicBufMode = FALSE;
   }

   if (VIDC_CLASSIFICATION_DECODE_ORDER == pSession->m_StaticCfg.eClassificationType)
   {
       VIDCTF_PRINT_HIGH("Enabling DecodeOrder!!");
       status = pSession->SetControl(pSession, V4L2_CID_MPEG_VIDC_VIDEO_DECODE_ORDER,
           V4L2_MPEG_MSM_VIDC_ENABLE);
       VIDEO_BAILOUT_ON_FAILURE("ioctl SetControl OUTPUT_ORDER", status, bailout);
   }
   if (pSession->m_StaticCfg.bThumbnailMode)
   {
       uint32 bufCnt = 0;
       VIDCTF_PRINT_HIGH("Enabling ThumbNailMode!!");
       status = pSession->SetControl(pSession, V4L2_CID_MPEG_VIDC_VIDEO_DECODE_ORDER,
           V4L2_MPEG_MSM_VIDC_ENABLE);
       VIDEO_BAILOUT_ON_FAILURE("ioctl SetControl OUTPUT_ORDER", status, bailout);

       status = pSession->SetControl(pSession, V4L2_CID_MPEG_VIDC_VIDEO_SYNC_FRAME_DECODE,
           V4L2_MPEG_MSM_VIDC_ENABLE);
       VIDEO_BAILOUT_ON_FAILURE("ioctl SetControl SYNC_FRAME_DECODE_ENABLE", status, bailout);

       /* Setting sync frame decoding on driver might change buffer
        * requirements so update them here */
       status = pSession->m_portParam[CAPTURE_PORT]->GetBufReq(pSession->m_portParam[CAPTURE_PORT]);
       VIDEO_BAILOUT_ON_FAILURE("CapturePort Config", status, bailout);

       bufCnt = pSession->m_portParam[CAPTURE_PORT]->m_bufreq.count;

   }

#if TAGDATA_ENABLE
   if (pSession->m_pStreamParser)
   {
       if (pSession->m_pStreamParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY)
       {
            /* ETB sent == FBD Received == Header +  Total Frame */
            pSession->m_pTagDataArray_size = pSession->m_pStreamParser->m_nTotalInputFrames + 1;
       }
       else if (pSession->m_pStreamParser->m_nPrefixHeaderMode == V4L2_DEC_SF)
       {
             /* ETB sent == FBD Received without Flag "V4L2_BUF_FLAG_END_OF_SUBFRAME" == Total Frame */
             pSession->m_pTagDataArray_size = pSession->m_pStreamParser->m_nTotalInputFrames;
       }

       pSession->m_pTagDataArray = (uint8*)malloc(pSession->m_pTagDataArray_size);
       if (!pSession->m_pTagDataArray)
       {
           status = VidcStatusAllocError;
           goto bailout;
       }
       memset(pSession->m_pTagDataArray, 0x00, pSession->m_pTagDataArray_size);
       VIDCTF_PRINT_HIGH("TagData validation enabled");
   }
#endif

   if (nOutput2Width && nOutput2Height)
   {
       VideoResolution res;
       VIDCTF_PRINT_HIGH("Enabling DownScaling!!");
       pDec->m_decCfg.bDownScaler = TRUE;
       pDec->m_decCfg.nOutput2Width = nOutput2Width;
       pDec->m_decCfg.nOutput2Height = nOutput2Height;

       res.nWidth = nOutput2Width; res.nCropWidth = nOutput2Width;
       res.nHeight = nOutput2Height; res.nCropHeight = nOutput2Height;
       pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);
       // TODO: Output2 aspect ratio validation in FBD based outputport aspect ratio
       status = pSession->m_portParam[CAPTURE_PORT]->SetFmt(pSession->m_portParam[CAPTURE_PORT]);
       VIDEO_BAILOUT_ON_FAILURE("CapturePort SetFmt Config", status, bailout);

       status = pSession->m_portParam[CAPTURE_PORT]->GetFmt(pSession->m_portParam[CAPTURE_PORT]);
       VIDEO_BAILOUT_ON_FAILURE("CapturePort GetFmt Config", status, bailout);

   }else {
       //When downscaler is disabled, the resolution on OUTPUT and CAPTURE port is expected to be same
       VideoResolution res;
       pSession->m_portParam[OUTPUT_PORT]->GetResolution(pSession->m_portParam[OUTPUT_PORT], &res);
       VIDCTF_PRINT_INFO("Default Config: Output Port: Width:%d Height:%d", res.nWidth, res.nHeight);
   }

   if (gVideoDecProp.nCachedBufferMask == DEADVALUE)
   {
       //By default decoder input buffers are the only buffers cached
       gVideoDecProp.nCachedBufferMask = VIDC_BUFTYPE_INPUT;
   }
bailout:
   return status;
}

VidcStatus VideoDecoderProcessOutputDone(VideoSession* pSession, BufInfo *pBufInfo)
{
    VideoDecoder *pDec = GETDECOBJ(pSession);
    VidcStatus status = VidcStatusSuccess;
    BOOL bIsResChange = FALSE;
    VideoResolution res;
    VideoResolution bufres;
    char extradata_msg[LOCAL_STR_LEN];
    uint32 bufCnt = 0;
    uint32 input_tag_idx = 0;
    uint32 input_tag2_idx = 0;
    uint8 *base_addr = NULL, *y_addr = NULL, *uv_addr = NULL, *base_addr_uv = NULL;
    uint32_t offset = 0;
    int ret = 0;

#if TAGDATA_ENABLE
    if (pSession->m_pTagDataArray) {
        if (!pSession->m_skipTagDataValidation && !(pBufInfo->flags & V4L2_BUF_FLAG_END_OF_SUBFRAME)) {

            if (pSession->m_pStreamParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY)
            {
               /* ETB sent == FBD Received == Header +  Total Frame
                * Total Frame       : 5
                * SuperFrame        : 3
                * ETB's sent        : 6
                * Tagdata size      : 6
                * index             : 0-5
                * ETB Tagdata       : {1, 4, 7, 10, 13, 16}
                * Eg. For calculating Input_tag_idx from FBD for Validation
                * (1-1)/3   = 0
                * (4-1)/3   = 1
                * (7-1)/3   = 2
                * (10-1)/3  = 3
                * (13-1)/3  = 4
                * (16-1)/3  = 5
                */
                input_tag_idx   = (pBufInfo->input_tag - 1) / pSession->m_pStreamParser->m_nSuperFrame;
                input_tag2_idx  = (pBufInfo->input_tag2 -1) / pSession->m_pStreamParser->m_nSuperFrame;
            }
            else if (pSession->m_pStreamParser->m_nPrefixHeaderMode == V4L2_DEC_SF)
            {
               /* ETB sent == FBD Received without Flag "V4L2_BUF_FLAG_END_OF_SUBFRAME" == Total Frame
                * Total Frame       : 5
                * SuperFrame        : 3
                * ETB's sent        : 5
                * Tagdata size      : 5
                * index             : 0-4
                * ETB Tagdata       : {4, 7, 10, 13, 16}
                * Eg. For calculating Input_tag_idx from FBD for Validation
                * 4: ((4 - 1)/3 - 1) = 1-1 = 0
                * 7: ((7 - 1)/3 - 1) = 2-1 = 1
                * 10:((10 - 1)/3 - 1) = 3-1 = 2
                * 13:((13 - 1)/3 - 1) = 4-1 = 3
                * 16:((16 - 1)/3 - 1) = 5-1 = 4
                */
                input_tag_idx   = ((pBufInfo->input_tag - 1) / pSession->m_pStreamParser->m_nSuperFrame) - 1;
                input_tag2_idx  = ((pBufInfo->input_tag2 -1) / pSession->m_pStreamParser->m_nSuperFrame) - 1;
            }

            if (pBufInfo->input_tag && input_tag_idx <= pSession->m_pTagDataArray_size)
            {
                if (pSession->m_pTagDataArray[input_tag_idx]) {
                    VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_TAGDATA, "TagData error, received FBD value1 %u", pBufInfo->input_tag);
                    status = VidcStatusUnSupported;
                    goto bailout;
                }

                pSession->m_pTagDataArray[input_tag_idx]++;
            }
            if (pBufInfo->input_tag2 && input_tag2_idx <= pSession->m_pTagDataArray_size)
            {
                if (pSession->m_pTagDataArray[input_tag2_idx]) {
                    VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_TAGDATA, "Marktarget error, received FBD value %u", pBufInfo->input_tag2);
                    status = VidcStatusUnSupported;
                    goto bailout;
                }

                pSession->m_pTagDataArray[input_tag2_idx]++;
            }
        }
        pBufInfo->input_tag = 0;
        pBufInfo->input_tag2 = 0;
    }
#endif

    if(pBufInfo->plane.bytesused == 0)
    {
        VIDCTF_PRINT_HIGH("Ignoring zero length buffer @ %p input_tag %d", (void*)(intptr_t)pBufInfo->vaddr, pBufInfo->input_tag);
        return status;
    }

    memset(&res,0x0,sizeof(VideoResolution));
    memset(&bufres,0x0,sizeof(VideoResolution));
    memset(extradata_msg, 0x00, LOCAL_STR_LEN);
    pSession->m_portParam[CAPTURE_PORT]->GetResolution(pSession->m_portParam[CAPTURE_PORT],&res);
    res.nColorFmt = bufres.nColorFmt = pDec->m_session.m_StaticCfg.eColorFmt;

    //Colour format can change if interlace is detected in extradata
    if(res.nColorFmt != bufres.nColorFmt) {
        res.nColorFmt = bufres.nColorFmt;
    }

#ifndef CROP_EXDATA
        bufres.nStartX = pBufInfo->plane.reserved[2];
        bufres.nStartY = pBufInfo->plane.reserved[3];
        bufres.nCropWidth = pBufInfo->plane.reserved[4];
        bufres.nCropHeight = pBufInfo->plane.reserved[5];
        bufres.nWidth = pBufInfo->plane.reserved[6];
        bufres.nHeight = pBufInfo->plane.reserved[7];
        VIDCTF_PRINT_HIGH("oldCROP: %u %u %u %u %u %u",bufres.nStartX,
                bufres.nStartY,
                bufres.nCropWidth ,
                bufres.nCropHeight,
                bufres.nWidth,
                bufres.nHeight);
#endif

    if(bufres.nCropWidth == 0 && bufres.nCropHeight == 0)
    {
	bufres.nCropWidth = res.nWidth;
	bufres.nCropHeight = res.nHeight;
    }

    VIDCTF_PRINT_HIGH("resCROP: %u %u %u %u %u %u",res.nStartX,
                                        res.nStartY,
                                        res.nCropWidth ,
                                        res.nCropHeight,
                                        res.nWidth,
                                        res.nHeight);
    if ((res.nCropWidth  != bufres.nCropWidth
         || res.nCropHeight != bufres.nCropHeight) ||
       (res.nStartX  != bufres.nStartX
         || res.nStartY != bufres.nStartY))
    {
        VIDCTF_PRINT_HIGH("Updating Session Resolution found(%dx%d) Current(%dx%d) left: %d, top: %d",
                        bufres.nCropWidth, bufres.nCropHeight,
                        res.nWidth, res.nHeight,
                        bufres.nStartX, bufres.nStartY);
        res.nCropWidth = bufres.nCropWidth;
        res.nCropHeight = bufres.nCropHeight;
        res.nWidth = bufres.nWidth;
        res.nHeight = bufres.nHeight;
        res.nStartX = bufres.nStartX;
        res.nStartY = bufres.nStartY;
        pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);
        pSession->m_portParam[CAPTURE_PORT]->GetResolution(pSession->m_portParam[CAPTURE_PORT], &res);

    }

    res.misrPayload = bufres.misrPayload;
    res.nInterlaceFormat = bufres.nInterlaceFormat;

    if (pSession->m_ActiveCurRes.nCropWidth == 0 && pSession->m_ActiveCurRes.nCropHeight == 0)
    {
        pSession->m_ActiveCurRes.nCropWidth = res.nCropWidth;
        pSession->m_ActiveCurRes.nCropHeight = res.nCropHeight;
        pSession->m_ActiveCurRes.nStartX = res.nStartX;
        pSession->m_ActiveCurRes.nStartY = res.nStartY;
    }

    if (pSession->m_StaticCfg.bThumbnailMode)
    {
        bufCnt = pSession->m_portParam[CAPTURE_PORT]->m_bufreq.count;
        if (pSession->m_StaticCfg.eCodecType != V4L2_PIX_FMT_VP9 && bufCnt != 1)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERCOUNT, "Codec:%x, cnt:%d",
                               pSession->m_StaticCfg.eCodecType, bufCnt);
            status = VidcStatusUnknownError;
        }
        else if (pSession->m_StaticCfg.eCodecType == V4L2_PIX_FMT_VP9 && bufCnt != 8)
        {
            VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_BUFFERCOUNT, "Codec:%x, cnt:%d",
                               pSession->m_StaticCfg.eCodecType, bufCnt);
            status = VidcStatusUnknownError;
        }
    }

    if (!pSession->m_StaticCfg.bSecure)
    {
        platform_write_update_outputbfr_ptr(&pBufInfo->vaddr,
            pBufInfo->plane.data_offset + pBufInfo->plane.bytesused);
    }

    if (buf_idx >= MAX_BUFFER)
       buf_idx = 0;

    base_addr = (uint8 *)plane_cfg[0].fb[buf_idx].ptr;
    base_addr_uv = (uint8 *)((uint8 *)plane_cfg[0].fb[buf_idx].ptr +
                                     (res.nCropHeight * plane_cfg[0].fb[buf_idx].pitch));

    offset = 0;
    y_addr = pBufInfo->vaddr;
    uv_addr = y_addr + (res.nCropWidth * VENUS_Y_SCANLINES(COLOR_FMT_NV12, res.nCropHeight));

    for (int i = 0; i < res.nCropHeight; i++)
    {
        memcpy((void *)(base_addr + offset), (void *)y_addr, res.nCropWidth);
        if (pDec->m_session.m_outStream)
           fwrite((void *)y_addr, res.nCropWidth, 1, pDec->m_session.m_outStream);
        offset += plane_cfg[0].fb[buf_idx].pitch;
        y_addr += res.nCropWidth;
    }

    offset = 0;
    for (int j = 0; j < (res.nCropHeight / 2); j++)
    {
        memcpy((void *)(base_addr_uv + offset), (void *)uv_addr, res.nCropWidth);
        if (pDec->m_session.m_outStream)
           fwrite((void *)uv_addr, res.nCropWidth, 1, pDec->m_session.m_outStream);
        offset += plane_cfg[0].fb[buf_idx].pitch;
        uv_addr += res.nCropWidth;
    }

    count++;
    for (int i = 0; i < (int) connector_cfg.size(); i++) {
        uint32_t flags = DRM_MODE_ATOMIC_ALLOW_MODESET;

        /* update fb */
        update_fb(req, i, buf_idx);

        /* asynchronous commit */
        flags |= DRM_MODE_ATOMIC_NONBLOCK;

        ret = drmModeAtomicCommit(fd, req, flags, 0);
        if (ret) {
           VIDCTF_PRINT_INFO("commit failed: %s\n",
                              strerror(errno));
           update_possible_crtcs();
        }
        if (count == 1)
        {
           place_marker("K - EarlyVideo first frame render done");
           VIDCTF_PRINT_INFO("Logs: VideoDecoder-first frame render done");
        }
        drmModeAtomicSetCursor(req, 0);
    }
    buf_idx++;

    VIDCTF_PRINT_HIGH("FrameDone buffer:%p flags:0x%x timestamp %ld res w %d h %d color_format %d interlace_fmt 0x%x",
        pBufInfo->vaddr, pBufInfo->flags, pBufInfo->timestamp.tv_sec,
        res.nCropWidth, res.nCropHeight, res.nColorFmt, res.nInterlaceFormat);

bailout:

    if (status != VidcStatusSuccess)
    {
       //To post Error Event OUTPUT PORT thread to close gracefully
       status = VidcStatusCaptureThreadError;
    }

    platform_restore_outputbfr_ptr(&pBufInfo->vaddr);
    VIDCTF_PRINT_INFO("ProcessOutputDone for buffer:%p completed",pBufInfo->vaddr);
    return status;
}

void VideoSessionDecideReconfigColorFormat(VideoSession *pSession, VideoResolution *pRes)
{
    pSession->m_StaticCfg.eColorFmt = pSession->m_StaticCfg.eConfiguredColorFmt;
    if (V4L2_PIX_FMT_NV12_TP10_UBWC == pSession->m_StaticCfg.eConfiguredColorFmt &&
        pSession->m_nBitDepthMode == MSM_VIDC_BIT_DEPTH_8) {
        //Change the capture port color format to 8-bit UBWC
        pSession->m_StaticCfg.eColorFmt = V4L2_PIX_FMT_NV12_UBWC;
        VIDCTF_PRINT_HIGH(" Output color format changed to UBWC due to 8bit port-reconfig\n");
    }
    else if (V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS == pSession->m_StaticCfg.eConfiguredColorFmt &&
        pSession->m_nBitDepthMode == MSM_VIDC_BIT_DEPTH_8) {
        //Change the capture port color format to 8-bit NV12
        pSession->m_StaticCfg.eColorFmt = V4L2_PIX_FMT_NV12;
        VIDCTF_PRINT_HIGH(" Output color format changed to NV12 due to 8bit port-reconfig\n");
    }
    else if (V4L2_PIX_FMT_NV12_UBWC == pSession->m_StaticCfg.eConfiguredColorFmt &&
        pSession->m_nBitDepthMode == MSM_VIDC_BIT_DEPTH_10) {
        //Change the capture port color format to 10-bit TP10
        pSession->m_StaticCfg.eColorFmt = V4L2_PIX_FMT_NV12_TP10_UBWC;
        VIDCTF_PRINT_HIGH(" Output color format changed to TP10 due to 10bit port-reconfig\n");
    }
    else if (V4L2_PIX_FMT_NV12 == pSession->m_StaticCfg.eConfiguredColorFmt &&
        pSession->m_nBitDepthMode == MSM_VIDC_BIT_DEPTH_10) {
        //Change the capture port color format to 10-bit P010
        pSession->m_StaticCfg.eColorFmt = V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS;
        VIDCTF_PRINT_HIGH(" Output color format changed to P010 due to 10bit port-reconfig\n");
    }
}
VidcStatus VideoPortParamsDecCaptureReconfig(VideoSession* pSession)
{
    VidcStatus status = VidcStatusSuccess;
    VideoResolution res,old_res;
    VideoPortParams *pCapturePort = pSession->m_portParam[CAPTURE_PORT];
    VideoPortParams *pOutport = pSession->m_portParam[OUTPUT_PORT];
    VideoDecoder *pDec = GETDECOBJ(pSession);
    BOOL bIsReconfigRequired = TRUE;
    uint32 oldBufCount = pCapturePort->m_bufreq.count;
    BufInfo *binfo = NULL;
    uint32 size = 0, i;
    BOOL bIsEosExitMsg = FALSE;
    pSession->m_bInReconfig = TRUE;
    pSession->m_bIsFlushEventDropped = FALSE;
    //Wait, if parent session need to do sync on buffer operations
    if (pSession->WaitForSync)
    {
        if (pSession->m_parentSession->m_type == VideoSessionTypeTranscode)
        {
            pSession->WaitForSync(pSession);
        }
    }
    pOutport->GetResolution(pOutport, &old_res);
    status = pOutport->GetFmt(pOutport);
    VIDEO_BAILOUT_ON_FAILURE("GetFmt pOutport", status, bailout);
    pOutport->GetResolution(pOutport, &res);
    VIDCTF_PRINT_HIGH(" Resolutions for input port Old : %dx%d New: %dx%d \n",
        old_res.nWidth, old_res.nHeight, res.nWidth, res.nHeight);
    if ((res.nWidth * res.nHeight <= old_res.nWidth*old_res.nHeight) &&
        (pCapturePort->m_bufreq.count <= oldBufCount)
        )
    {
        //bIsReconfigRequired = FALSE;
        //TODO: Need to optimize based on buffer count
        //Need to calculate buffer count based on level
        VIDCTF_PRINT_HIGH("Pending Optimizing DecoderCapturePortReconfig");
    }
    else
    {
        VIDCTF_PRINT_HIGH("Starting DecoderCapturePortReconfig");
    }
    if (bIsReconfigRequired)
    {
        pthread_mutex_lock(&pSession->m_sessionMutex);
        status = pOutport->PostEvent(pOutport, SESSION_FLUSH_MSG, NULL);
        if (status == VidcStatusSuccess)
        {
            status = pSession->WaitForEventDone(pSession, &pSession->m_portThreadsCondWait, &pSession->m_sessionMutex);
            VIDCTF_PRINT_HIGH("EventDone Recieved from OutputPort");
        }
        else if (status == VidcStatusOperationError)
        {
            status = VidcStatusSuccess;
            VIDCTF_PRINT_HIGH("Ignore Operation error from PostEvent");
        }
        pthread_mutex_unlock(&pSession->m_sessionMutex);
        //If FlushTimeOut Occurs and EOS type is 3,
        //Then there are chances that output port is terminated without processing Flush,
        //In this case, check for EOS3_EXIT msg in capture port queue.
        //If found, ignore FlushTimeOut error and return Success status.
        if (status == VidcStatusFlushTimeOutError &&
            pSession->m_StaticCfg.eEosType == VIDC_FRAME_FLAG_EOS3)
        {
            size = pCapturePort->m_bufQ->m_size;
            for(i = 0; i < size; i++)
            {
                binfo = NULL;
                LIST_NODE *node = pCapturePort->m_bufQ->PopFront(pCapturePort->m_bufQ);
                if (node != NULL)
                {
                    binfo = list_get_node_data(node, BufInfo, msgQueNode);
                }
                if (binfo)
                {
                    if (binfo->msgType == SESSION_EOS3_EXIT_MSG)
                    {
                        bIsEosExitMsg = TRUE;
                    }
                    pCapturePort->m_bufQ->Push(pCapturePort->m_bufQ, &binfo->msgQueNode);
                }
            }
        }
        if (bIsEosExitMsg)
        {
            status = VidcStatusSuccess;
            // Ignore FlushTimeOut error.
            return status;
        }
        VIDEO_BAILOUT_ON_FAILURE("Flush Capture", status, bailout);
    }
    status = pCapturePort->StreamOff(pCapturePort);
    VIDEO_BAILOUT_ON_FAILURE("StreamOff Capture", status, bailout);
    if (bIsReconfigRequired)
    {
        status = pCapturePort->FreeBuffers(pCapturePort);
        VIDEO_BAILOUT_ON_FAILURE("FreeBufs Capture", status, bailout);
        VideoSessionDecideReconfigColorFormat(pSession, &res);
        //check based on MB's whether new resolution falling in between range of
        // smooth stream max/min resolution
        if(pDec->m_decCfg.bSmoothStreaming)
        {
            VIDCTF_PRINT_INFO("checking if reconfig event is allowed for this resolution %dx%d \n",
                                                                       res.nWidth, res.nHeight);
            if(((res.nWidth * res.nHeight) >= (pDec->m_decCfg.nMinWidth * pDec->m_decCfg.nMinHeight))&&
               ((res.nWidth * res.nHeight) <= (pDec->m_decCfg.nMaxWidth * pDec->m_decCfg.nMaxHeight)))
            {
                 VIDCTF_PRINT_ERROR(pSession, VIDC_ERR_SMSTR_INSUFF_RESOURCE,
                                   "new resolutoin %dx%d is within smooth streaming range",
                                                                  res.nWidth, res.nHeight);
                 status =  VidcStatusCaptureThreadError;
                 goto bailout;
            }
        }
        if (pDec->m_decCfg.bDownScaler)
        {
            if (pDec->m_decCfg.nOutput2Width < res.nWidth || pDec->m_decCfg.nOutput2Height < res.nHeight)
            {
                VIDCTF_PRINT_HIGH("Enabling DownScaling!!");
                res.nWidth = pDec->m_decCfg.nOutput2Width; res.nCropWidth = pDec->m_decCfg.nOutput2Width;
                res.nHeight = pDec->m_decCfg.nOutput2Height; res.nCropHeight = pDec->m_decCfg.nOutput2Height;
                pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);
                status = pSession->m_portParam[CAPTURE_PORT]->SetFmt(pSession->m_portParam[CAPTURE_PORT]);
                VIDEO_BAILOUT_ON_FAILURE("CapturePort SetFmt Config", status, bailout);
            }
            else
            {
                //disable downscalar
                pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);
                status = pSession->m_portParam[CAPTURE_PORT]->SetFmt(pSession->m_portParam[CAPTURE_PORT]);
                VIDEO_BAILOUT_ON_FAILURE("CapturePort SetFmt Config", status, bailout);
                VIDCTF_PRINT_HIGH("disabling DownScaling!!");
            }
        }
        else
        {
                pSession->m_portParam[CAPTURE_PORT]->SetResolution(pSession->m_portParam[CAPTURE_PORT], res);
                status = pSession->m_portParam[CAPTURE_PORT]->SetFmt(pSession->m_portParam[CAPTURE_PORT]);
                VIDEO_BAILOUT_ON_FAILURE("CapturePort SetFmt Config", status, bailout);
        }
        status = pCapturePort->GetFmt(pCapturePort);
        VIDEO_BAILOUT_ON_FAILURE("GetFmt Capture", status, bailout);
        status = pCapturePort->GetBufReq(pCapturePort);
        VIDEO_BAILOUT_ON_FAILURE("GetBufReqs Capture", status, bailout);
        if (pSession->m_bIsFlushEventDropped)
        {
           status = pCapturePort->PostEvent(pCapturePort, SESSION_FLUSH_MSG,NULL);
        }
        pSession->m_bIsFlushEventDropped = FALSE;
        status = pCapturePort->PrepareBuf(pCapturePort);
        VIDEO_BAILOUT_ON_FAILURE("PrepareBufs Capture", status, bailout);
    }
    status = pCapturePort->StreamOn(pCapturePort);
bailout:
    pSession->m_bInReconfig = FALSE;
    return status;
}

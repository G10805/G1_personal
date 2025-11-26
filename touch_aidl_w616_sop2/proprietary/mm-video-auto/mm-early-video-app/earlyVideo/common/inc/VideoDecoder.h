/*-------------------------------------------------------------------
Copyright (c) 2014-2017, 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VIDEODECODER_H
#define _VIDEODECODER_H

#include "VideoSession.h"


typedef struct VideoDecConfig
{
   BOOL           bInterlace;
   BOOL           bDownScaler;
   uint32         eDecFormat;
   uint32         nOutput2Width;
   uint32         nOutput2Height;
   BOOL           bSmoothStreaming;
   uint32         nMinWidth;
   uint32         nMinHeight;
   uint32         nMaxWidth;
   uint32         nMaxHeight;
   BOOL           bLowLatency;
   uint32         nPriority;
   uint32         OpRate;
}VideoDecConfig;


typedef struct VideoDecoder
{
    VideoSession    m_session;
    VideoDecConfig  m_decCfg;
    uint32          m_nPlanes; // output buffer length

}VideoDecoder;

VideoSession* VideoDecoderInit();
void VideoDecoderDeinit(VideoSession* pSession);

#endif

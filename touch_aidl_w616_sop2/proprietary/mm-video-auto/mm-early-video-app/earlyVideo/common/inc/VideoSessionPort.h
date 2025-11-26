/*-------------------------------------------------------------------
Copyright (c) 2014-2017, 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VIDEOSESSIONPORT_H
#define _VIDEOSESSIONPORT_H

#include <stdbool.h>
#include "VideoPlatform.h"
#include "VideoMsgQueue.h"

#define DISPLAY_BUF_CNT 2
#define MARK_DEFERRED_FREE    0x1

typedef struct ion_info
{
   int ion_device_fd;
   int mapfd;
   struct ion_allocation_data ion_alloc_data;
   char* uaddr;
}ion_info;

typedef struct VidcExtraDataBufInfo {
    unsigned long buffer_size;
    int count;
    int size;
    struct ion_info ion[VIDEO_MAX_FRAME];
}VidcExtraDataBufInfo;

/* Structure to keep track of Available Input/Output Buffers */
typedef struct BufInfo
{
    uint32 offset;
    uint8* vaddr;
    uint8* extradata_vaddr;
    uint32 extradata_filled_len;
    uint32 size;
    struct ion_info ion;
    enum v4l2_buf_type buf_type;
    int index;
    struct v4l2_plane plane;
    struct v4l2_plane extraDataPlane;
    uint32 flags;
    struct timeval timestamp;
    LIST_NODE bInfoListNode;
    LIST_NODE msgQueNode;
    uint32 input_tag;
    uint32 input_tag2;
    uint32 msgType;
    uint32 releaseFlag;
    uint32 bufRefCount;
    uint8  evtdata[64]; // Event Data from struct v4l2_event
}BufInfo;

typedef enum
{
   OUTPUT_PORT,
   CAPTURE_PORT,
   MAX_PORTS
}PortType;

typedef struct VideoPortParams VideoPortParams;
struct VideoPortParams
{
    struct v4l2_requestbuffers m_bufreq;
    struct v4l2_control m_bufcount;
    struct v4l2_format m_fmt;
    struct v4l2_fmtdesc m_fdesc;
    enum v4l2_buf_type m_bufType;

    LIST_NODE m_bInfoDefFreeList;  /* List of Buffers marked for Deferred Free */
    uint32 m_bInfoDefFreeListSize;
    LIST_NODE m_bInfoList;  /* Stores the port Buffer information */
    uint32 m_bInfoListSize;
    VideoMsgQueue *m_bufQ;
    struct VideoSession* m_hSession;         /* Session handle for port params*/
    pthread_t m_threadID;
    VidcExtraDataBufInfo m_extraData;

    pthread_mutex_t  m_mutex;  //To protect shared data in VideoPortParams

    uint32 m_bufSubmitCnt;
    uint32 m_bufDoneCnt;
    BOOL m_bIsEOSReached;
    BOOL m_bIsEOS3Reached;//To know whether EOS reached before creating output thread
    BOOL m_bAllocated; //Is port Allocated
    BOOL m_bIsStreamOnOutputPlane; //Is StreamOn(OutputPlane) called.

    uint32 m_nCropWidth;
    uint32 m_nCropHeight;

    uint32 m_nEOS3QueudBufCount;// To know how many ETBs are given to driver from  the output
    //queue req buf count, if the EOS reached before creating output thread and if the EOS type
    //is EOS3

    VidcStatus (*EnumFormats)(VideoPortParams* pParams);
    VidcStatus (*SetFmt)(VideoPortParams* pParams);
    VidcStatus (*GetFmt)(VideoPortParams* pParams);
    VidcStatus (*GetBufReq)(VideoPortParams* pParams);
    VidcStatus (*SetBufReq)(VideoPortParams* pParams);
    VidcStatus (*PrepareBuf)(VideoPortParams* pParams);
    VidcStatus (*QueueBuffers)(VideoPortParams *pPortParam);
    VidcStatus (*StreamOn)(VideoPortParams *pPortParam);
    VidcStatus (*StreamOff)(VideoPortParams *pPortParam);
    VidcStatus (*Deinit)(VideoPortParams *pPortParam);
    void       (*Close)(VideoPortParams *pPortParam);

    VidcStatus (*QueueAvailableBuffer)(VideoPortParams *pPortParam);
    BOOL       (*IsEmptyBufQ)(VideoPortParams *pPortParam);
    void       (*GetResolution)(VideoPortParams *pPortParam, VideoResolution* pRes);
    void       (*SetResolution)(VideoPortParams *pParams, VideoResolution res);
    VidcStatus (*ProcessBufferDone)(VideoPortParams *pPortParam);
    VidcStatus (*QueueBuffer)(VideoPortParams *pPortParam, BufInfo *binfo);
    pthread_t  (*GetThreadID)(VideoPortParams *pPortParam);
    VidcStatus (*CreateThreadID)(VideoPortParams *pPortParam);
    VidcStatus (*PostEvent)(VideoPortParams *pPortParam, uint32 evtType, uint8* data);
    VidcStatus (*ReleaseUnQueuedBuffer)(VideoPortParams *pPortParam, void* pData);
    VidcStatus (*WaitForBufferAlloc)(struct VideoSession* pSession);
    void       (*SignalBufferAlloc)(struct VideoSession* pChildSession);
    void       (*FreeBufferList)(VideoPortParams *pPortParam);
    VidcStatus (*PushBuffer)(VideoPortParams *pParams, BufInfo *binfo);
    VidcStatus (*PushEvent)(VideoPortParams *pParams, BufInfo *binfo);
    VidcStatus (*TransferBuffer)(VideoPortParams *pParams, BufInfo *binfo);
    VidcStatus (*SetBuffers)(VideoPortParams *pParams);
    VidcStatus (*FreeBuffers)(VideoPortParams *pParams);

};

VidcStatus   VideoPortParamsInit(VideoPortParams* pParams, struct VideoSession *pSession, PortType port);
void* VideoPortParamsDataThreadRun(void *pPortParam);
VidcStatus VideoPortParamsStartBufferAccess(struct VideoSession *pSession, int fd);
VidcStatus VideoPortParamsEndBufferAccess(struct VideoSession *pSession, int fd);

#endif

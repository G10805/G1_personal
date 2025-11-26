/*-------------------------------------------------------------------
Copyright (c) 2014-2020, 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VIDEOSESSION_H
#define _VIDEOSESSION_H

#include "VideoSessionPort.h"
#include "VideoStreamParserInterface.h"

#define SESSION_MSG_NONE             0x0000000
#define SESSION_POLL_ERROR           0x8001011
#define SESSION_IOCTL_ERROR          0x8001012
#define SESSION_UNKNOWN_EVENT_ERROR  0x8001013
#define SESSION_ALLOC_ERROR          0x8001014
#define SESSION_CRCMISMATCH_ERROR    0x8001015
#define SESSION_EOS3_EXIT_MSG        0x8001016
#define SESSION_FLUSH_MSG            0x8001017
#define SESSION_RECONFIG_MSG         0x8001018
#define DEADVALUE                    ((uint32_t) 0xDEADDEAD) //in decimal : 3735936685



typedef enum VideoColorSpace {
    BT2020 = 0,
    EXCEPT_BT2020,
    UNKNOWN_COLORSPACE
} VideoColorSpace;

/* Definition of Video Session Class */
typedef struct VideoSession
{
    uint8*             m_pTagDataArray;        /* Validate tag_data */
    FILE*              m_outStream;            /* File Descriptor : Output Bit Stream File*/
    VideoStreamParser* m_pStreamParser;   //TODO: make it static allocation

    pthread_t          m_hAsyncThread;

    VideoPortParams*   m_portParam[MAX_PORTS];//TODO: static allocation

    uint32             m_nFrameCounter; // TODO: add comment/rename as inputFrmCnt

    int                m_hDriver;            /* Current video driver handle */
    int                m_hPollEventFd;       /* Current poll event handle */
    VideoSessionError  m_eErrorNum;
    char               m_sTestID[MAX_STR_LEN];
    VideoSessionBeatStaticConfig m_StaticCfg;  /* structure to hold video encoder/decoder common config data */
    VideoSessionStaticConfig m_ParserStaticCfg;  /* structure to hold video encoder/decoder common config data */
    VideoSessionType   m_type;
    pthread_t          m_sessionThread;
    LIST_NODE          m_sessionThreadEntry;

    pthread_mutex_t    m_mutex;                 //For protecting session common data like m_eErrorNum
    pthread_mutex_t    m_sessionMutex;          // session thread mutex to protect flush operations
    pthread_cond_t     m_asyncPortThrdCondWait; //To serialize operations between async and portThread
    pthread_cond_t     m_portThreadsCondWait;   //To serialize operations between output/capture threads
    BufInfo            *m_pFlushMsgNode;        //Initialized by port thread to indicate flush msg to other thread

    struct VideoSession*      m_parentSession;
    BOOL               m_bIsFlushEventDropped;            // to handle freeing flush msg
    uint32             m_nFrameSkip;
    uint32             m_nSkipDynCmd;           //Indicates how many dynamic commands are dropped
    VideoResolution    m_ActiveCurRes;
    uint32             m_nBitDepthMode;
    BOOL               m_bIsThread;
    BOOL               m_bInReconfig;
    BOOL               m_bProgressive;
    BOOL               m_bColorPrimariesExData; //0 to validate against reference , 1 to enable dump
    BOOL               m_bEOS;
    BOOL               m_skipTagDataValidation;
    enum VideoColorSpace  m_eColorSpace;
    uint32             m_nMaxPauseTime;
    uint32             m_pTagDataArray_size;
    void*      (*ThreadRun)(void *pSessionHandle);
    VidcStatus (*Open)(struct VideoSession *pSession);
    VidcStatus (*SubscribeEvents)(struct VideoSession *pSession);
    VidcStatus (*QueryCapabilities)(struct VideoSession *pSession);
    VidcStatus (*Run)(struct VideoSession *pSession);
    VidcStatus (*Flush)(struct VideoSession *pSession, uint32 bufType);
    VidcStatus (*Stop)(struct VideoSession *pSession);
    VidcStatus (*Close)(struct VideoSession* pSession);
    void       (*DeInit)(struct VideoSession* pSession);

    VidcStatus (*SetDefaultConfig)(struct VideoSession* pSession);

    VidcStatus (*GetControl)(struct VideoSession *pSession, struct v4l2_control *control);
    VidcStatus (*SetControl)(struct VideoSession *pSession, uint32 id, uint32 value);

    VidcStatus (*ProcessInputBuffer)(struct VideoSession *pSession, BufInfo *pBufInfo);
    PortType   (*GetReconfigPort)(struct VideoSession *pSession);

    uint32     (*GetPortFormat)(struct VideoSession *pSession, enum v4l2_buf_type bufType);
    VidcStatus (*ProcessOutputDone)(struct VideoSession *pSession, BufInfo *pBufInfo);
    uint32     (*GetFrameDoneCount)(struct VideoSession* pSession);
    BOOL       (*IsDataCorruptTobeValidated)(struct VideoSession* pSession);
    VidcStatus (*FlushDriver)(struct VideoSession *pSession, uint32 bufType);
    VidcStatus (*WaitForEventDone)(struct VideoSession *pSession, pthread_cond_t  *pCondWait, pthread_mutex_t *pMutex);
    void       (*WaitForSync)(struct VideoSession *pSession);
    VidcStatus (*PortReconfig)(struct VideoSession* pSession);

}VideoSession;

VidcStatus VideoSessionInit(VideoSession* pSession, VideoSessionType eType);
VidcStatus VideoSessionOpen(struct VideoSession *pSession);
VidcStatus VideoSessionClose(VideoSession* pSession);
VidcStatus VideoSessionDeInit(VideoSession* pSession);

void* VideoSessionAsyncThreadRun(void *pSessionHandle);

extern uint32 MapColorFormat(uint32 format);
#endif

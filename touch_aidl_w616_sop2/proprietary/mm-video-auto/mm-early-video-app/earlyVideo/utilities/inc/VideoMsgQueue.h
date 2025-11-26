/*
 **************************************************************************************************
 * Copyright (c) 2014-2017, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _VIDEOMSGQUEUE_H
#define _VIDEOMSGQUEUE_H

#include <pthread.h>
#include <time.h>
#include "VideoPlatform.h"

typedef struct VideoMsgQueue VideoMsgQueue;
struct VideoMsgQueue
{
    LIST_NODE m_queue;
    pthread_mutex_t  *m_pMutex;  // Reuse same mutex from its parent object
    pthread_cond_t   m_condWait;
    uint32 m_size;
    void   (*Destroy)(VideoMsgQueue *pMsgQ);
    void   (*Push)(VideoMsgQueue *pMsgQ, LIST_NODE *entry);
    LIST_NODE*  (*PopFront)(VideoMsgQueue *pMsgQ);
};

VidcStatus VideoMsgQueueInit(VideoMsgQueue *pMsgQ, pthread_mutex_t  *pMutex);
#endif

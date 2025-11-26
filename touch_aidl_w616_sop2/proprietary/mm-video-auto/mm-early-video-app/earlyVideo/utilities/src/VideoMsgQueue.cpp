/*
 **************************************************************************************************
 * Copyright (c) 2014-2017, 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "VideoMsgQueue.h"

void   VideoMsgQueueDestroy(VideoMsgQueue *pMsgQ);
void   VideoMsgQueuePush(VideoMsgQueue *pMsgQ, LIST_NODE *entry);

LIST_NODE *VideoMsgQueuePopFront(VideoMsgQueue *pMsgQ);

VidcStatus VideoMsgQueueInit(VideoMsgQueue *pMsgQ, pthread_mutex_t  *pMutex)
{
    VidcStatus status = VidcStatusSuccess;

    if (!pMsgQ)
       return VidcStatusBadParamError;

    pthread_cond_init(&pMsgQ->m_condWait, 0);

    pMsgQ->m_size = 0;

    pMsgQ->m_pMutex = pMutex;

    pMsgQ->Destroy  = VideoMsgQueueDestroy;
    pMsgQ->PopFront = VideoMsgQueuePopFront;
    pMsgQ->Push     = VideoMsgQueuePush;

    list_init(&pMsgQ->m_queue);

    return status;
}

void VideoMsgQueueDestroy(VideoMsgQueue* pMsgQ)
{
    pthread_cond_destroy(&pMsgQ->m_condWait);
}

void VideoMsgQueuePush(VideoMsgQueue* pMsgQ, LIST_NODE *entry)
{
    pthread_mutex_lock(pMsgQ->m_pMutex);

    VIDCTF_PRINT_INFO("Queue=%p push: pEntry:%p,size:%d", pMsgQ, entry, pMsgQ->m_size);

    list_insert_tail(entry, &pMsgQ->m_queue);
    pMsgQ->m_size++;

    pthread_cond_broadcast(&pMsgQ->m_condWait);
    pthread_mutex_unlock(pMsgQ->m_pMutex);
}

LIST_NODE*  VideoMsgQueuePopFront(VideoMsgQueue* pMsgQ)
{
    struct timeval now;
    struct timespec timeout;
    int rc = 0;
    LIST_NODE* pEntry = NULL;

    pthread_mutex_lock(pMsgQ->m_pMutex);

    if (list_is_empty(&pMsgQ->m_queue))
    {
        VIDCTF_PRINT_INFO("Queue empty, so waiting for buffer availability..");

        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + (gVideoDecProp.nThreadTimeout/1000) ;
        timeout.tv_nsec = now.tv_usec;

        rc = pthread_cond_timedwait(&pMsgQ->m_condWait, pMsgQ->m_pMutex, &timeout);
    }

    if (!rc && !list_is_empty(&pMsgQ->m_queue))
    {
        pEntry = list_get_head(&pMsgQ->m_queue);;

        VIDCTF_PRINT_INFO("Queue=%p pop: pEntry:%p,size:%d", pMsgQ, pEntry, pMsgQ->m_size);

        if (pEntry)
           list_remove_node(&pMsgQ->m_queue,pEntry);

        pMsgQ->m_size--;
    }
    else
    {
        VIDCTF_PRINT_WARN("Timeout, no buffer available for %d ms", gVideoDecProp.nThreadTimeout);
    }

    pthread_mutex_unlock(pMsgQ->m_pMutex);

    return pEntry;
}

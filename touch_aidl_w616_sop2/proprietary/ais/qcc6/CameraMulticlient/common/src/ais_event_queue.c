/**
 * @file ais_event_queue.c
 *
 * @brief implements event queue with priorities.
 *
 * Copyright (c) 2018-2019, 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ais_event_queue.h"
#include "ais_log.h"

#define AIS_LOG_EQ(level, fmt...) AIS_LOG2(EVENT_QUEUE, level, fmt)

#define AIS_LOG_EQ_ERR(fmt...) AIS_LOG(EVENT_QUEUE, ERROR, fmt)
#define AIS_LOG_EQ_WARN(fmt...) AIS_LOG(EVENT_QUEUE, WARN, fmt)
#define AIS_LOG_EQ_HIGH(fmt...) AIS_LOG(EVENT_QUEUE, HIGH, fmt)
#define AIS_LOG_EQ_MED(fmt...) AIS_LOG(EVENT_QUEUE, MED, fmt)
#define AIS_LOG_EQ_LOW(fmt...) AIS_LOG(EVENT_QUEUE, LOW, fmt)


/**
 * converts event to camera queue index which holds events with different priorities
 *
 * @param event one event
 * @return camera queue index
 *
 */
unsigned int ais_event_queue_event_2_idx(uint32_t event)
{
    switch (event)
    {
    case QCARCAM_EVENT_INPUT_SIGNAL:
    case QCARCAM_EVENT_ERROR:
        return 0;
    case QCARCAM_EVENT_FRAME_READY:
    default:
        return 1;
    }
}

/**
 * pushes one event structures into the specific queue
 *
 * @param p points to event queue structures
 * @param p_event points to an event structure
 * @return 0: success, others: failed
 *
 */
static int ais_event_queue_push(s_ais_event_queue *p, s_ais_event *p_event)
{
    int rc = 0;
    CameraResult ret;
    boolean is_full = FALSE;
    unsigned int idx = 0;

    AIS_LOG_EQ_LOW("E 0x%p 0x%p", p, p_event);

    if (p->fcn_event_2_idx != NULL)
    {
        idx = p->fcn_event_2_idx(p_event->event_id);
        if (idx >= p->numQueues)
        {
            AIS_LOG_EQ_ERR("%p, invalid idx %d for event_id 0x%x ", p, idx, p_event->event_id);
            idx = 0;
        }
    }

    ret = CameraQueueIsFull(p->queue[idx], &is_full);
    if (ret != CAMERA_SUCCESS)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (is_full)
    {
        AIS_LOG_EQ_WARN("event Q %p[%d] full - dropped head event", p, idx);
        CameraQueueDropHead(p->queue[idx]);
    }

    p_event->event_cnt = p->cnt;

    ret = CameraQueueEnqueue(p->queue[idx], p_event);
    if (rc != CAMERA_SUCCESS)
    {
        rc = -2;
        goto EXIT_FLAG;
    }
    else
    {
        p->cnt++;
    }

EXIT_FLAG:

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_ERROR,
            "x 0x%p 0x%p %d", p, p_event, rc);

    return rc;
}

/**
 * pops out one event structures from the specific queue
 *
 * @param p points to event queue structures
 * @param p_event points to an event structure
 * @return 0: success, others: failed
 *
 */
static int ais_event_queue_pop(s_ais_event_queue *p, s_ais_event *p_event)
{
    int rc = -1;
    unsigned int i;

    AIS_LOG_EQ_LOW("E 0x%p 0x%p", p, p_event);

    for (i = 0; i < p->numQueues; i++)
    {
        CameraResult ret = CameraQueueDequeue(p->queue[i], p_event);
        if (ret == CAMERA_SUCCESS)
        {
            rc = 0;
            break;
        }
    }

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_LOW : AIS_LOG_LVL_MED,
            "X 0x%p 0x%p %d", p, p_event, rc);

    return rc;
}

/**
 * initializes an event queue, and creates internal priority queues with specified capacity
 *
 * @param p points to event queue structures
 * @param numPriorityQueues the number of priority queues which can hold events with different priorities
 * @param queueCapacity the capacity of one queues
 * @param fcn_event_2_idx functions pointer which defines the priorities for different events
 * @return 0: success, others: failed
 *
 */
int ais_event_queue_init(s_ais_event_queue *p, unsigned int numPriorityQueues, unsigned int queueCapacity,
        unsigned int (*fcn_event_2_idx)(uint32_t event))
{
    int rc = -1;
    unsigned int i = 0;
#ifndef __INTEGRITY
    pthread_condattr_t cattr;
#endif
    CameraQueueCreateParamType param;

    AIS_LOG_EQ_MED("E 0x%p %d %d", p, numPriorityQueues, queueCapacity);

    if (p == NULL || numPriorityQueues == 0 || numPriorityQueues > AIS_EVENT_QUEUE_MAX_NUM ||
            queueCapacity == 0 || queueCapacity > AIS_EVENT_QUEUE_MAX_CAPACITY)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (p->status)
    {
        AIS_LOG_EQ_ERR("%p already initialized", p);
        rc = -10;
        goto EXIT_FLAG;
    }

    memset(p, 0, sizeof(s_ais_event_queue));
    p->numQueues = numPriorityQueues;
    p->fcn_event_2_idx = fcn_event_2_idx;

    memset(&param, 0, sizeof(param));
    param.nCapacity = queueCapacity;
    param.nDataSizeInBytes = sizeof(s_ais_event);
    param.eLockType = CAMERAQUEUE_LOCK_NONE;

    for (i = 0; i < p->numQueues; i++)
    {
        CameraResult ret = CameraQueueCreate(&p->queue[i], &param);
        if (ret != CAMERA_SUCCESS)
        {
            rc = -2;
            goto EXIT_ERROR;
        }
    }

#ifndef __INTEGRITY
    rc = pthread_condattr_init(&cattr);
    if (rc != 0)
    {
        rc = -5;
        goto EXIT_ERROR;
    }

    rc = pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (rc != 0)
    {
        rc = -6;
        goto EXIT_ERROR;
    }

    rc = pthread_cond_init(&p->cond, &cattr);
#else
    rc = pthread_cond_init(&p->cond, NULL);
#endif
    if (rc != 0)
    {
        rc = -3;
        goto EXIT_ERROR;
    }

    rc = pthread_mutex_init(&p->mutex, NULL);
    if (rc != 0)
    {
        pthread_cond_destroy(&p->cond);
        rc = -4;
        goto EXIT_ERROR;
    }

    p->status = true;

EXIT_ERROR:

    if (rc != 0 && p != NULL)
    {
        for (i--; i >= 0; i--)
        {
            CameraQueueDestroy(p->queue[i]);
        }
    }

EXIT_FLAG:
    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_MED : AIS_LOG_LVL_ERROR,
            "X 0x%p %d %d %d", p, numPriorityQueues, queueCapacity, rc);

    return rc;
}

/**
 * deinitializes an event queue, destroy internal queues and release all resources
 *
 * @param p points to event queue structures
 * @return 0: success, others: failed
 *
 */
int ais_event_queue_deinit(s_ais_event_queue *p)
{
    int rc = 0;
    unsigned int i;

    AIS_LOG_EQ_MED("E 0x%p", p);

    if (p == NULL)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (!p->status)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    for (i = 0; i < p->numQueues; i++)
    {
        rc = CameraQueueDestroy(p->queue[i]);
    }

    pthread_cond_destroy(&p->cond);
    pthread_mutex_destroy(&p->mutex);

    memset(p, 0, sizeof(s_ais_event_queue));

EXIT_FLAG:

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_MED : AIS_LOG_LVL_ERROR,
            "X %s 0x%p %d", __func__, p, rc);

    return rc;
}

/**
 * signal waiting thread
 *
 * @param p points to event queue structures
 * @return 0: success, others: failed
 *
 */
int ais_event_queue_signal(s_ais_event_queue *p)
{
    int rc = 0;

    AIS_LOG_EQ_MED("E 0x%p", p);

    if (p == NULL)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (!p->status)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    rc = pthread_cond_signal(&p->cond);

EXIT_FLAG:

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_MED : AIS_LOG_LVL_ERROR,
            "X 0x%p %d", p, rc);

    return rc;
}

/**
 * enqueues an event, and signals other thread
 *
 * @param p points to event queue structures
 * @param p_event points to an event structure
 * @return 0: success, others: failed
 *
 */
int ais_event_queue_enqueue(s_ais_event_queue *p, s_ais_event *p_event)
{
    int rc;

    AIS_LOG_EQ_MED("E 0x%p 0x%p", p, p_event);

    if (p == NULL || p_event == NULL)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (!p->status)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    rc = pthread_mutex_lock(&p->mutex);
    if (rc != 0)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    rc = ais_event_queue_push(p, p_event);

    pthread_mutex_unlock(&p->mutex);

    if (rc == 0)
    {
        //signal the other pthread to get the event
        pthread_cond_signal(&p->cond);
    }

EXIT_FLAG:

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_MED : AIS_LOG_LVL_ERROR,
            "X 0x%p 0x%p %d", p, p_event, rc);

    return rc;
}

/**
 * dequeues an event with timed-out time.
 * if no event is available in timed-out time, it returns.
 *
 * @param p points to event queue structures
 * @param p_event points to an event structure
 * @param timeout timed-out time in millisecond
 * @return 0: success, others: failed
 *
 */
int ais_event_queue_dequeue(s_ais_event_queue *p, s_ais_event *p_event, unsigned int timeout)
{
    int rc;
    struct timespec ts;

    AIS_LOG_EQ_MED("E %s 0x%p 0x%p %d", p, p_event, timeout);

    if (p == NULL || p_event == NULL)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    if (!p->status)
    {
        rc = -1;
        goto EXIT_FLAG;
    }

    rc = pthread_mutex_lock(&p->mutex);
    if (rc != 0)
    {
        rc = -2;
        goto EXIT_FLAG;
    }

    //try to get a new event
    rc = ais_event_queue_pop(p, p_event);
    if (rc == 0)
    {
        pthread_mutex_unlock(&p->mutex);
        goto EXIT_FLAG;
    }

    memset(&ts, 0, sizeof(ts));

    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += (timeout / 1000);
    ts.tv_nsec += (timeout % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }

    //get one signal for a new event
    rc = pthread_cond_timedwait(&p->cond, &p->mutex, &ts);
    if (rc == 0)
    {
        rc = ais_event_queue_pop(p, p_event);
    }
    else
    {
        rc = -3;
    }

    pthread_mutex_unlock(&p->mutex);

EXIT_FLAG:

    AIS_LOG_EQ(rc == 0 ? AIS_LOG_LVL_MED : AIS_LOG_LVL_MED,
            "X 0x%p 0x%p %d %d", p, p_event, timeout, rc);

    return rc;
}


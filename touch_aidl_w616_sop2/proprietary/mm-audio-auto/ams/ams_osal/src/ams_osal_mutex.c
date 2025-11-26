/**
 * \file ams_osal_mutex.c
 *
 * \brief
 *      This file implements mutex apis. Recursive mutexes are always used
 *      for thread-safe programming.
 *
 * \copyright
 *  Copyright (c) 2018-2020, 2022 by Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define AMS_OSAL_MUTEX_LOG_TAG     "ams_osal_mutex"
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <log/log.h>
#include "ams_osal_mutex.h"
#include "ams_osal_error.h"

/* Internal lock definition */
typedef struct ams_osal_int_mutex {
    pthread_mutex_t mutex;
} ams_osal_int_mutex_t;


int32_t ams_osal_mutex_init(ams_osal_mutex_t mutex __unused)
{
    return AMS_ENOTIMPL;
}


int32_t ams_osal_mutex_deinit(ams_osal_mutex_t mutex __unused)
{
    return AMS_ENOTIMPL;
}


size_t ams_osal_mutex_get_size(void)
{
    return sizeof(ams_osal_int_mutex_t);
}



int32_t ams_osal_mutex_create(ams_osal_mutex_t *ams_osal_mutex)
{
    int32_t rc;
    ams_osal_int_mutex_t* the_mutex;

    if (NULL == ams_osal_mutex) {
        return AMS_EBADPARAM;
    }

    the_mutex = ((ams_osal_int_mutex_t *) malloc(sizeof(ams_osal_int_mutex_t)));
    if (NULL == the_mutex) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: failed to allocate memory for mutex\n", __func__);
        rc = AMS_ENOMEMORY;
        goto exit;
    }

    rc = pthread_mutex_init(&the_mutex->mutex, NULL);
    if (rc) {
        rc = AMS_EFAILED;
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: failed to initialize mutex\n", __func__);
        goto fail;
    }

    *ams_osal_mutex = the_mutex;
    return 0;

fail:
    free(the_mutex);

exit:
    return rc;
}



int32_t ams_osal_mutex_destroy(ams_osal_mutex_t ams_osal_mutex)
{
    int32_t rc = 0;
    ams_osal_int_mutex_t *the_mutex = ams_osal_mutex;

    if (NULL == the_mutex) {
        return AMS_EBADPARAM;
    }

    rc = pthread_mutex_destroy(&the_mutex->mutex);
    if (rc) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: Failed to destroy mutex\n", __func__);
        rc = AMS_EFAILED;
        goto exit;
    }
    free(the_mutex);

exit:
    return rc;
}


int32_t ams_osal_mutex_lock(ams_osal_mutex_t ams_osal_mutex)
{
    int32_t rc;
    ams_osal_int_mutex_t *the_mutex = ams_osal_mutex;

    if (NULL == the_mutex) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: ams_osal_mutex is NULL\n", __func__);
        return AMS_EBADPARAM;
    }

    rc = pthread_mutex_lock(&the_mutex->mutex);
    if (rc) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: Failed to lock ams_osal_mutex\n", __func__);
        rc = AMS_EFAILED;
    }
    return rc;
}


int32_t ams_osal_mutex_try_lock(ams_osal_mutex_t ams_osal_mutex)
{
    int32_t rc;
    ams_osal_int_mutex_t *the_mutex = ams_osal_mutex;

    if (NULL == the_mutex) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: ams_osal_mutex is NULL\n", __func__);
        return AMS_EBADPARAM;
    }

    rc = pthread_mutex_trylock(&the_mutex->mutex);
    if (rc) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: Failed to lock ams_osal_mutex\n", __func__);
        rc = AMS_EFAILED;
    }
    return rc;
}


int32_t ams_osal_mutex_unlock(ams_osal_mutex_t ams_osal_mutex)
{
    int32_t rc;
    ams_osal_int_mutex_t *the_mutex = ams_osal_mutex;

    if (NULL == the_mutex) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: ams_osal_mutex is NULL\n", __func__);
        return AMS_EBADPARAM;
    }

    rc = pthread_mutex_unlock(&the_mutex->mutex);
    if (rc) {
        ALOGE(AMS_OSAL_MUTEX_LOG_TAG,"%s: Failed to release ams_osal_mutex\n", __func__);
        rc = AMS_EFAILED;
    }
    return rc;
}

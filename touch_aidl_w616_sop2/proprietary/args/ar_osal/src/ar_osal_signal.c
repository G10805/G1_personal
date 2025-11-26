/**
 * \file ar_osal_signal.c
 *
 * \brief
 *      This file defines signal variable implementaion.
 *
 * \copyright
 *  Copyright (c) 2018-2021 by Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define AR_OSAL_SIGNAL_LOG_TAG   "COSI"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include "ar_osal_signal.h"
#include "ar_osal_log.h"
#include "ar_osal_error.h"
#include <cutils/properties.h>

#define PROC_PANIC_PATH     "/proc/sysrq-trigger"
#define ADSP_LOADER_PATH    "/sys/kernel/boot_adsp/ssr"

/* Internal condition definition */
typedef struct osal_int_signal {
	pthread_mutex_t osal_mutex;
	pthread_cond_t osal_cond;
	bool signalled;
} osal_int_signal_t;

#define AR_OSAL_SIG_TOUT 5
int32_t ar_osal_tout_cnt = 0; /* timeout counter */
int32_t ar_osal_tout_limit = -1; /* -1 indicates limit not initialized */

_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_init(_In_ ar_osal_signal_t signal __unused)
{
    return AR_ENOTIMPL;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_deinit(_In_ ar_osal_signal_t signal __unused)
{
    return AR_ENOTIMPL;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
size_t ar_osal_signal_get_size()
{
    return sizeof(osal_int_signal_t);
}


_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_create(_Inout_ ar_osal_signal_t *ret_signal)
{
    int32_t rc;
    osal_int_signal_t *the_signal;
    pthread_condattr_t attr;
    if (NULL == ret_signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }
    the_signal = (osal_int_signal_t *)malloc(sizeof(osal_int_signal_t));
    if (NULL == the_signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: failed to allocate signal memory\n", __func__);
        rc = AR_ENOMEMORY;
        goto done;
    }

    rc = pthread_mutex_init(&the_signal->osal_mutex, NULL);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to init mutex, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto err_mutex;
    }
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    rc = pthread_cond_init(&the_signal->osal_cond, &attr);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to init cond, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto err_cond;
    }
    the_signal->signalled = false;
    *ret_signal = the_signal;
    goto done;
err_cond:
    pthread_mutex_destroy(&the_signal->osal_mutex);
err_mutex:
    free(the_signal);
done:
   return rc;
}


_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_signal_wait(_In_ ar_osal_signal_t signal)
{
    int32_t rc;
    osal_int_signal_t *the_signal;
    if (NULL == signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }
    the_signal = (osal_int_signal_t *)signal;
    rc = pthread_mutex_lock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to acquire lock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto done;
    }
    if (true == the_signal->signalled)
    {
        rc = pthread_mutex_unlock(&the_signal->osal_mutex);
        if (rc) {
            AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
            rc = AR_EFAILED;
        }
        goto done;
    }

    rc = pthread_cond_wait(&the_signal->osal_cond, &the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to wait on signal, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto err_cond;
    }

    rc = pthread_mutex_unlock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto done;
    }
err_cond:
    pthread_mutex_unlock(&the_signal->osal_mutex);
done:
    return rc;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_panic(void)
{
    char panic_set ='c';
    int fd_sysrq = 0;
    bool crash = false;
    char value[256] = {0};

    property_get("persist.vendor.audio.induce_crash", value, "");
    if (!strncmp("true", value, sizeof("true"))) {
         crash = true;
    }

    if(crash)
    {
        fd_sysrq = open(PROC_PANIC_PATH, O_WRONLY);

        if(fd_sysrq < 0) {
            AR_LOG_ERR("%s: open (%s) fail - %s (%d)", __func__,
                 PROC_PANIC_PATH, strerror(errno), errno);
            //ignore if panic path can't be opened
        } else if (write(fd_sysrq, &panic_set, 1) < 0) {
            AR_LOG_ERR("%s: write (%s) fail - %s (%d)", __func__, PROC_PANIC_PATH, strerror(errno), errno);
        }
    }
    return 0;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
void ar_osal_ssr(void) {
    int fd_dsplder;

    fd_dsplder = open(ADSP_LOADER_PATH, O_WRONLY);

    if(fd_dsplder < 0) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG, "%s: open (%s) fail - %s (%d)", __func__,
            ADSP_LOADER_PATH, strerror(errno), errno);
    } else if (write(fd_dsplder, "1", 1) < 0) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG, "%s: write (%s) fail - %s (%d)", __func__,
            ADSP_LOADER_PATH, strerror(errno), errno);
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
void ar_osal_tout_inc(void) {
    int32_t prop_tout;

    ar_osal_panic();

    if (ar_osal_tout_limit > 0) {
        ar_osal_tout_cnt++;
        if (ar_osal_tout_cnt >= ar_osal_tout_limit) {
	    AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: timeout exceed %d trigger SSR\n",
		       __func__, ar_osal_tout_cnt);
            ar_osal_ssr();
	    ar_osal_tout_cnt = 0;
        }
    } else if (ar_osal_tout_limit == -1) {
#ifdef _ANDROID_
        prop_tout = property_get_int32("persist.vendor.audio.tout_limit", -1);
#else
        prop_tout = -1;
#endif
        AR_LOG_INFO(AR_OSAL_SIGNAL_LOG_TAG,"%s: limit set=%d\n", __func__,
		   prop_tout);

        if (prop_tout == -1) /* property not set or actually set to -1 */
            ar_osal_tout_limit = AR_OSAL_SIG_TOUT;
	else if (prop_tout > 0)
	    ar_osal_tout_limit = prop_tout;
	else /* 0 or negative value other than -1 disable SSR from app-proc */
	    ar_osal_tout_limit = 0;

	ar_osal_tout_inc();
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
int32_t ar_osal_signal_timedwait(_In_ ar_osal_signal_t signal, _In_ int64_t timeout_in_nsec)
{
    int32_t rc;
    osal_int_signal_t *the_signal;
    struct timespec osal_ts;

    if (NULL == signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }

    clock_gettime(CLOCK_MONOTONIC, &osal_ts);
    osal_ts.tv_sec += (timeout_in_nsec / 1000000000);
    osal_ts.tv_nsec += (timeout_in_nsec % 1000000000);

    if (osal_ts.tv_nsec >= 1000000000) {
        osal_ts.tv_sec += 1;
        osal_ts.tv_nsec -= 1000000000;
    }

    the_signal = (osal_int_signal_t *)signal;
    rc = pthread_mutex_lock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to acquire lock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto done;
    }

    if (true == the_signal->signalled) {
        rc = pthread_mutex_unlock(&the_signal->osal_mutex);
        if (rc) {
            AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
            rc = AR_EFAILED;
        }
        goto done;
    }

    rc = pthread_cond_timedwait(&the_signal->osal_cond, &the_signal->osal_mutex, &osal_ts);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to wait on signal, rc = %d\n", __func__, rc);
        ar_osal_tout_inc();
        rc = AR_EFAILED;
        goto err_cond;
    } else
        ar_osal_tout_cnt = 0; /* reset timeout count if signal is received successfully */

    rc = pthread_mutex_unlock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
    }
err_cond:
    pthread_mutex_unlock(&the_signal->osal_mutex);
done:
    return rc;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_set(_In_ ar_osal_signal_t signal)
{
    int32_t rc;
    osal_int_signal_t *the_signal;

    if (NULL == signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }

    the_signal = (osal_int_signal_t *)signal;
    rc = pthread_mutex_lock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to acquire lock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto done;
    }
    the_signal->signalled = true;

    rc = pthread_cond_broadcast(&the_signal->osal_cond);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to signal on signal, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
		goto err_cond;
    }

    rc = pthread_mutex_unlock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
    }
err_cond:
    pthread_mutex_unlock(&the_signal->osal_mutex);
done:
    return rc;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_destroy(_In_ ar_osal_signal_t signal)
{
    int32_t rc;
    osal_int_signal_t *the_signal;

    if (NULL == signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }

    the_signal = (osal_int_signal_t *)signal;

    the_signal-> signalled = false;
    rc = pthread_mutex_lock(&the_signal->osal_mutex);

    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to acquire lock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        return rc;
    }
    rc = pthread_cond_destroy(&the_signal->osal_cond);

    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to destroy signal, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto err_cond;
    }
    rc = pthread_mutex_unlock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto err_destroy;
    }
    rc = pthread_mutex_destroy(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to destroy mutex, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
    }
    goto done;

err_cond:
    pthread_mutex_unlock(&the_signal->osal_mutex);
err_destroy:
    pthread_mutex_destroy(&the_signal->osal_mutex);
done:
    free(signal);
    return rc;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
int32_t ar_osal_signal_clear(_In_ ar_osal_signal_t signal)
{
    int32_t rc;
    osal_int_signal_t *the_signal;

    if (NULL == signal) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: signal is NULL\n", __func__);
        rc = AR_EBADPARAM;
        goto done;
    }

    the_signal = (osal_int_signal_t *)signal;
    rc = pthread_mutex_lock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to acquire lock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
        goto done;
    }

    the_signal->signalled = false;

    rc = pthread_mutex_unlock(&the_signal->osal_mutex);
    if (rc) {
        AR_LOG_ERR(AR_OSAL_SIGNAL_LOG_TAG,"%s: Failed to unlock, rc = %d\n", __func__, rc);
        rc = AR_EFAILED;
    }

done:
    return rc;
}

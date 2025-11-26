/**
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#define LOG_TAG "ar_osal_test"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "ar_osal_signal.h"
#include "ar_osal_mutex.h"
#include "ar_osal_thread.h"
#include "ar_osal_error.h"
#include "ar_osal_types.h"
#include "ar_osal_test.h"
#include "ar_osal_log.h"

void* lock = NULL;
void* signal1 = NULL;
void* signal2 = NULL;
void* signal3 = NULL;
void* signal4 = NULL;

void testtrylockwait()
{
    int status = AR_EOK;
    status = ar_osal_signal_wait(signal1);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_wait error: %d\n",status);
        return;
    }

    if((status = ar_osal_mutex_try_lock(lock)) != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"mutex is null\n");
        status = ar_osal_signal_set(signal2);
        if(AR_EOK != status)
        {
            AR_LOG_ERR(LOG_TAG,"ar_osal_signal_set error %d\n",status);
            return;
        }
        AR_LOG_INFO(LOG_TAG,"signal 2 set\n");
    }
    AR_LOG_INFO(LOG_TAG,"lock acquired this by thread %lld\n",(long long int)ar_osal_thread_get_id());
    status = ar_osal_mutex_unlock(lock);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_mutex_unlock error:%d\n",status);
        return;
    }
}

void testtrylocksignal()
{
    int status;
    status = ar_osal_mutex_lock(lock);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_mutex_lock error %d\n",status);
        return;
    }
    AR_LOG_INFO(LOG_TAG,"lock acquired by thread %lld\n",(long long int)ar_osal_thread_get_id());
    status = ar_osal_signal_set(signal1);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_set error %d\n",status);
        return;
    }
    AR_LOG_INFO(LOG_TAG,"waiting for signal 2\n");
    status = ar_osal_signal_wait(signal2);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_wait error %d\n",status);
        return;
    }
    status = ar_osal_mutex_unlock(lock);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_mutex_unlock error %d\n",status);
        return;
    }
    AR_LOG_INFO(LOG_TAG,"lock released by	thread %lld\n",(long long int)ar_osal_thread_get_id());
}

bool trylocktestcase(void)
{
    void* tid1=NULL;
    void* tid2=NULL;
    ar_osal_thread_attr_t tattr;
    int status;
    AR_LOG_INFO(LOG_TAG,"testcase 1:");
    status = ar_osal_thread_attr_init(&tattr);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"osal_thread_attr_init error:%d\n",status);
        goto exit;
    }
    status = ar_osal_mutex_create(&lock);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_mutex_create error:%d\n",status);
        goto exit;
    }
    status = ar_osal_signal_create(&signal1);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_create error:%d\n",status);
        goto exit;
    }
    status = ar_osal_signal_create(&signal2);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_create error:%d\n",status);
        goto exit;
    }
    status = ar_osal_thread_create(&tid1,&tattr,testtrylocksignal,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error:%d\n",status);
        goto exit;
    }
    status = ar_osal_thread_create(&tid2,&tattr,testtrylockwait,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error:%d\n",status);
        goto exit;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid1))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error:%d\n",status);
        goto exit;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid2))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error:%d\n",status);
        goto exit;
    }

    return true;
exit:
    return false;
}

bool nulltestcase(void)
{
    int status;
    status = ar_osal_mutex_create(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_mutex_create api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter, error code %d\n",status);

    status = ar_osal_mutex_lock(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_mutex_lock api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_mutex_lock api, error code %d\n",status);

    status = ar_osal_mutex_unlock(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_mutex_unlock api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_mutex_unlock api, error code %d\n",status);

    status = ar_osal_mutex_try_lock(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_mutex_try_lock api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_mutex_try_lock api, error code %d\n",status);

    status = ar_osal_signal_create(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_signal_create api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_signal_create api, error code %d\n",status);

    status = ar_osal_signal_wait(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_signal_wait api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_signal_wait api, error code %d\n",status);

    status = ar_osal_signal_set(NULL);
    if( AR_EOK == status)
    {
        AR_LOG_ERR(LOG_TAG,"problem with ar_osal_signal_set api: successfully returning the NULL parameter\n");
        return false;
    }
    else
        AR_LOG_ERR(LOG_TAG,"error recognised due to sending null as a parameter by ar_osal_signal_set api, error code %d\n",status);
    return true;
}

void signalthread()
{
    sleep(10);
    int status;
    status = ar_osal_signal_set(signal3);
    AR_LOG_INFO(LOG_TAG,"signal 3 is set\n");
    if( AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_set error: %d\n",status);
        return;
    }
}

void waitingthread1()
{
    int status;
    AR_LOG_INFO(LOG_TAG,"waiting for signal  by thread %lld \n",(long long int)ar_osal_thread_get_id());
    status = ar_osal_signal_wait(signal3);
    if( AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_wait error: %d\n",status);
        return;
    }
    AR_LOG_INFO(LOG_TAG,"thread %lld wait signalled\n",(long long int)ar_osal_thread_get_id());
    sleep(10);
    status = ar_osal_signal_clear(signal3);
    if( AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_clear error: %d\n",status);
        return;
    }
}

void waitingthread2()
{
    int status;
    status = ar_osal_signal_timedwait(signal3,1000000);
    if(AR_EOK != status)
    {
        if(-ETIMEDOUT == status)
        {
            AR_LOG_ERR(LOG_TAG,"timed out and wait failed \n");
            return;
        }
        else
        {
            AR_LOG_ERR(LOG_TAG,"ar_osal_signal_timedwait error:%d\n",status);
        }
    }
}

bool signalwaittestcase(void)
{
    void* tid1=NULL;
    void* tid2=NULL;
    void* tid3=NULL;
    void* tid4=NULL;
    ar_osal_thread_attr_t tattr;
    int status;
    AR_LOG_INFO(LOG_TAG,"test case 3\n");
    status = ar_osal_thread_attr_init(&tattr);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"osal_thread_attr_init error:%d\n",status);
        goto exit1;
    }

    status = ar_osal_signal_create(&signal3);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_create error: %d\n",status);
        goto exit1;
    }
    status = ar_osal_signal_create(&signal4);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"failed to create the signal\n error code %d\n",status);
        goto exit1;
    }
    status = ar_osal_thread_create(&tid1,&tattr,signalthread,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error: %d\n",status);
        goto exit1;
    }
    status = ar_osal_thread_create(&tid2,&tattr,waitingthread1,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error: %d\n",status);
        goto exit1;
    }
    status = ar_osal_thread_create(&tid3,&tattr,waitingthread1,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error: %d\n",status);
        goto exit1;
    }
    status = ar_osal_thread_create(&tid4,&tattr,waitingthread2,NULL);
    if (AR_EOK != status )
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_create error: %d\n",status);
        goto exit1;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid1))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error: %d\n",status);
        goto exit1;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid2))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error: %d\n",status);
        goto exit1;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid3))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error: %d\n",status);
        goto exit1;
    }
    if (AR_EOK != ar_osal_thread_join_destroy(tid4))
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_thread_join_destroy error: %d\n",status);
        goto exit1;
    }
    return true;
exit1:
    return false;
}

void cleanup(void)
{
    int status;
    status = ar_osal_signal_destroy(signal1);
	if(AR_EOK != status)
	{
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_destroy error:%d\n",status);
	}
    status = ar_osal_signal_destroy(signal2);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_destroy error:%d\n",status);
    }
    status = ar_osal_signal_destroy(signal3);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_destroy error:%d\n",status);
    }
    status = ar_osal_signal_destroy(signal4);
    if(AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"ar_osal_signal_destroy error:%d\n",status);
    }
}

void ar_test_signal_mutex_thread_main(void)
{
    bool result;
    result = trylocktestcase();
    if(result == true){
        AR_LOG_INFO(LOG_TAG,"testcase 1 passed without any errors\n");
    }
    else{
        AR_LOG_ERR(LOG_TAG,"error in testcase 1\n");
    }
    result = nulltestcase();
    if(result == true){
        AR_LOG_INFO(LOG_TAG,"testcase 2 passed without any errors\n");
    }
    else{
        AR_LOG_ERR(LOG_TAG,"error in testcase 2\n");
    }
    result = signalwaittestcase();
    if(result == true){
        AR_LOG_INFO(LOG_TAG,"testcase 3 passed without any errors\n");
    }
    else{
        AR_LOG_ERR(LOG_TAG,"error in testcase 3\n");
    }
    cleanup();
return;
}

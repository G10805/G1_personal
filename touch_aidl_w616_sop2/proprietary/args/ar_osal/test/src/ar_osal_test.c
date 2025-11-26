/**
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#define LOG_TAG "ar_osal_test"
#include <stdio.h>
#include "ar_osal_test.h"
#include "ar_osal_log.h"
int main()
{
    AR_LOG_INFO(LOG_TAG,"*******************************************************************\n");
    AR_LOG_INFO(LOG_TAG," signal mutex thread test case starting \n");
    /* signal thread test case*/
    ar_test_signal_mutex_thread_main();
    AR_LOG_INFO(LOG_TAG," signal thread test case ended \n");
    AR_LOG_INFO(LOG_TAG,"*******************************************************************\n");
    AR_LOG_INFO(LOG_TAG," shmem test case starting \n");
    /* shmem test case*/
    ar_test_shmem_main();
    AR_LOG_INFO(LOG_TAG," shmem test case ended \n");
    AR_LOG_INFO(LOG_TAG,"*******************************************************************\n");
    AR_LOG_INFO(LOG_TAG," file IO test case starting \n");
    /* file io test case*/
    ar_test_file_main();
    AR_LOG_INFO(LOG_TAG," file IO test case ended \n");
    AR_LOG_INFO(LOG_TAG,"*******************************************************************\n");

    return 0;
}

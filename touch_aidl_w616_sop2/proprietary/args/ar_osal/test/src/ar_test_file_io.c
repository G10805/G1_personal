/**
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#define LOG_TAG "ar_osal_test"
#include <stdio.h>
#include <unistd.h>
#include "ar_osal_file_io.h"
#include "ar_osal_error.h"
#include "ar_osal_types.h"
#include "ar_osal_test.h"
#include "ar_osal_log.h"

char file_data[] = "write data to file \n";
char *test_file = "test_text.txt";


void ar_test_file_main(void)
{
    int32_t status = AR_EOK;
    ar_fhandle fhandle = NULL;
    size_t fsize = 0;
    size_t bytes_written = 0;
    size_t bytes_read = 0;
    char_t data_read[22] = { 0 };

    /* open file*/
    status = ar_fopen(&fhandle, test_file, AR_FOPEN_READ_WRITE);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to open the file %s:error:%d \n", status);
        goto end;
    }

    fsize = ar_fsize(fhandle);
    /* write data */
    status = ar_fwrite(fhandle, file_data, sizeof(file_data), &bytes_written);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to write into the file %d \n", status);
        goto end;
    }
    AR_LOG_INFO(LOG_TAG,"bytes written %d\n", bytes_written);
    if (bytes_written != sizeof(file_data))
    {
        AR_LOG_INFO(LOG_TAG,"failed write file size not matching \n");
    }

    status = ar_fclose(fhandle);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to seek file %d \n", status);
        goto end;
    }
    fhandle = NULL;

    status = ar_fopen(&fhandle, test_file, AR_FOPEN_READ_ONLY);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to open the file %s:error:%d \n", status);
        goto end;
    }
    /* get file size */
    fsize = ar_fsize(fhandle);
    AR_LOG_INFO(LOG_TAG,"file size after write %d \n", fsize);

    /* seek file */
    status = ar_fseek(fhandle, 0, AR_FSEEK_END);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to seek file %d \n", status);
        goto end;
    }
    /* seek file */
    status = ar_fseek(fhandle, 0, AR_FSEEK_BEGIN);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to seek file %d \n", status);
        goto end;
    }
    /* read data */
    status = ar_fread(fhandle, data_read, sizeof(file_data), &bytes_read);
    if (status != AR_EOK)
    {
        AR_LOG_ERR(LOG_TAG,"failed to read the file %d \n", status);
        goto end;
    }

    if (bytes_read != sizeof(file_data))
    {
        AR_LOG_ERR(LOG_TAG,"failed to read the right size bytes_read(%d) file_data(%d)\n", bytes_read, sizeof(file_data));
    }
    AR_LOG_INFO(LOG_TAG,"read data: %s", data_read);
end:
    if (NULL != fhandle)
    {
        status = ar_fclose(fhandle);
        if (status != AR_EOK)
        {
            AR_LOG_ERR(LOG_TAG,"failed to close file %d \n", status);
        }
        status = ar_fdelete(test_file);
        if (status != AR_EOK)
        {
            AR_LOG_ERR(LOG_TAG,"failed to delete file %d \n", status);
        }
    }
    return;
}

/**
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#define LOG_TAG "ar_osal_test"
#include "ar_osal_test.h"
#include "ar_util_list.h"
#include "ar_osal_log.h"
#include "ar_osal_types.h"
#include "ar_osal_error.h"
#include "ar_osal_mutex.h"
#include <malloc.h>

void * mutex_handle = NULL;
#define AR_NODES_MAX (10)

static void list_lock_enter_fn_t(void)
{
    if (NULL != mutex_handle)
    {
        if (AR_EOK != ar_osal_mutex_lock(mutex_handle))
        {
            AR_LOG_ERR(LOG_TAG,"failed to acquire mutex \n");
        }
    }
}

static void list_lock_leave_fn_t(void)
{
    if (NULL != mutex_handle)
    {
        if (AR_EOK != ar_osal_mutex_unlock(mutex_handle))
        {
            AR_LOG_ERR(LOG_TAG,"failed to release mutex \n");
        }
    }
}

void ar_test_util_list_main(void)
{
    int32_t status = AR_EOK;
    ar_list_node_t *nodes[AR_NODES_MAX] = { NULL };
    int32_t list_count = 0;
    ar_list_node_t *removed_node = NULL;
    ar_list_t test_list;

    status = ar_osal_mutex_create(&mutex_handle);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"mutex creation failed %d \n", status);
        goto end;
    }

    status = ar_list_init(&test_list, list_lock_enter_fn_t, list_lock_leave_fn_t);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"list init failed %d \n", status);
        goto end;
    }

    AR_LOG_INFO(LOG_TAG,"check if list is empty %d \n", ar_list_is_empty(&test_list));
    AR_LOG_INFO(LOG_TAG,"list head %p \n", ar_list_get_head(&test_list));
    AR_LOG_INFO(LOG_TAG,"list tail %p \n", ar_list_get_tail(&test_list));

    for (list_count = 0; list_count < AR_NODES_MAX; )
    {
        nodes[list_count] = malloc(sizeof(ar_list_node_t));
        if (NULL == nodes[list_count])
        {
            AR_LOG_ERR(LOG_TAG,"node[%d] allocation failed \n", list_count);
            status = AR_ENOMEMORY;
            goto end;
        }
        list_count++;
        status = ar_list_init_node(nodes[list_count - 1]);
        if (AR_EOK != status)
        {
            AR_LOG_ERR(LOG_TAG,"list node[%d] init failed %d \n", list_count - 1, status);
            goto end;
        }
        AR_LOG_INFO(LOG_TAG,"list node[%d]:%p created \n", list_count - 1, nodes[list_count - 1]);
    }

    for (int32_t i = 0; i < list_count; i++)
    {
        status = ar_list_add_tail(&test_list, nodes[i]);
        if (AR_EOK != status)
        {
            AR_LOG_ERR(LOG_TAG,"failed to add node[%d] to list err:%d \n", i, status);
            goto end;
        }
        AR_LOG_INFO(LOG_TAG," node[%d][%p] added to the list \n", i, nodes[i]);
    }
    AR_LOG_INFO(LOG_TAG,"list size %d \n", test_list.size);

    status = ar_list_remove_head(&test_list, &removed_node);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"failed to remove node from list head %d \n", status);
        goto end;
    }

    AR_LOG_INFO(LOG_TAG,"node[%p] removed from the list head \n", removed_node);
    AR_LOG_INFO(LOG_TAG,"list size %d \n", test_list.size);

    removed_node = NULL;
    status = ar_list_remove_head(&test_list, &removed_node);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"failed to remove node from list head %d \n", status);
        goto end;
    }
    AR_LOG_INFO(LOG_TAG,"node[%p] removed from the list head \n", removed_node);
    AR_LOG_INFO(LOG_TAG,"delete node[%p] from the list\n", nodes[5]);
    AR_LOG_INFO(LOG_TAG,"list size %d \n", test_list.size);

    status = ar_list_delete(&test_list, nodes[5]);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"failed to delete node[%p], err: %d \n", nodes[5], status);
    }

    AR_LOG_INFO(LOG_TAG,"node[%p] deleted from list \n", nodes[5]);
    AR_LOG_INFO(LOG_TAG,"list size %d \n", test_list.size);

    AR_LOG_INFO(LOG_TAG,"check if list is empty %d \n", ar_list_is_empty(&test_list));
    AR_LOG_INFO(LOG_TAG,"list head %p \n", ar_list_get_head(&test_list));
    AR_LOG_INFO(LOG_TAG,"list tail %p \n", ar_list_get_tail(&test_list));

    status = ar_list_clear(&test_list);
    if (AR_EOK != status)
    {
        AR_LOG_ERR(LOG_TAG,"failed to clear the list \n");
    }
    AR_LOG_INFO(LOG_TAG,"list cleared \n");
    AR_LOG_INFO(LOG_TAG,"list size %d \n", test_list.size);
end:
    for (int32_t i = 0; i < list_count; i++)
    {
        if (nodes[i])
        {
            AR_LOG_INFO(LOG_TAG," free node[%d][%p] \n", i, nodes[i]);
            free(nodes[i]);
        }
    }
    if (NULL != mutex_handle)
    {
        ar_osal_mutex_destroy(mutex_handle);
        mutex_handle = NULL;
    }
    return;
}

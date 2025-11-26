/*===========================================================================
 VPP hypervisor IO buffer management

 *//** @file hyp_buffer_manager.cpp
 This file provides functions to map the file description between local and remote
 process using habmm APIs

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "hyp_vpp_buf_manager.h"
#include "habmm.h"
#include "hyp_vpp_debug.h"
#include "MMCriticalSection.h"

#define MAX_EXPORT_RETRY 5

/**===========================================================================

  FUNCTION hypvpp_map_queue_init

  @brief  Initilize the member in lookup queue

  @param [in] queue handle

  @dependencies
  None

  @return
  Returns zero value on success else failure

  ===========================================================================*/
int hypvpp_map_queue_init
(
    hypvpp_lookup_table_t* queue
)
{
    int ret = 0;

    if (queue)
    {
        ret = MM_CriticalSection_Create(&queue->mutex);
    }
    else
    {
        ret = -EINVAL;
        HYP_VPP_MSG_ERROR("lookup table is null");
    }

    return ret;
}

/**===========================================================================

  FUNCTION hypvpp_map_queue_deinit

  @brief  Deinitilize the member in lookup queue

  @param [in] queue handle

  @dependencies
  None

  @return
  Returns zero value on success else failure

  ===========================================================================*/
int hypvpp_map_queue_deinit
(
    hypvpp_lookup_table_t* queue
)
{
    int ret = 0;

    if (queue)
    {
        ret = MM_CriticalSection_Release(queue->mutex);
        queue->mutex = NULL;
    }
    else
    {
        ret = -EINVAL;
        HYP_VPP_MSG_ERROR("lookup table is null");
    }

    return ret;
}

/**===========================================================================

  FUNCTION hypvpp_map_from_lookup

  @brief  Given a share id finds an entry node

  @param [in] queue handle
  @param [in] share_id  the share id exported by the remote process

  @dependencies
  None

  @return
  Returns the entry node that associates with the share id

  ===========================================================================*/
hypvpp_map_entry_t* hypvpp_map_from_lookup
(
    hypvpp_lookup_table_t* queue,
    uint32 share_id
)
{
    hypvpp_map_entry_t* entry = queue->linkhead;

    MM_CriticalSection_Enter(queue->mutex);

    while (entry)
    {
        if (entry->share_id == share_id)
        {
            break;
        }
        entry = entry->next;
    }

    MM_CriticalSection_Leave(queue->mutex);

    return entry;
}

/**===========================================================================

  FUNCTION hypvpp_map_to_lookup

  @brief  Given a file description of the buffer finds an entry node

  @param [in] queue handle
  @param [in] fd  the file description given by the local process

  @dependencies
  None

  @return
  Returns the entry node that associates with the file description

  ===========================================================================*/
hypvpp_map_entry_t* hypvpp_map_to_lookup
(
    hypvpp_lookup_table_t* queue,
    int32 fd
)
{
    hypvpp_map_entry_t* entry = queue->linkhead;

    MM_CriticalSection_Enter(queue->mutex);

    while (entry)
    {
        if (entry->fd == fd)
        {
            break;
        }
        entry = entry->next;
    }

    MM_CriticalSection_Leave(queue->mutex);

    return entry;
}


/**===========================================================================

  FUNCTION hypvpp_map_free

  @brief  Given a file description in the server process
  free its cached map entry if there is any

  @param [in] hab API interface
  @param [in] queue handle
  @param [in] fd file description
  @param [in] share_id  share id

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hypvpp_map_free
(
    habIf* pHab_if,
    hypvpp_lookup_table_t* queue,
    int32 fd,
    uint32 share_id
)
{
    hypvpp_status_type ret = HYPVPP_STATUS_FAIL;
    hypvpp_map_entry_t* prev = NULL;
    hypvpp_map_entry_t* entry = NULL;
    void* unimport_fd = (void*)(unsigned long)fd;

    if (!pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VPP_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        ret = HYPVPP_STATUS_FAIL;
    }
    else
    {
        MM_CriticalSection_Enter(queue->mutex);
        entry = queue->linkhead;

        while (entry)
        {
            if ((entry->fd == fd) && (entry->share_id == share_id))
            {
                ret = HYPVPP_STATUS_SUCCESS;
                entry->refcnt -= 1;
                if (0 != entry->refcnt)
                {
                    break;
                }
                if (0 == entry->is_export)
                {
                    if (0 != pHab_if->pfUnImport(entry->habmmhandle, entry->share_id, unimport_fd, HABMM_EXPIMP_FLAGS_FD))
                    {
                        HYP_VPP_MSG_ERROR("failed to unimport share id %u fd %d", entry->share_id, entry->fd);
                        ret = HYPVPP_STATUS_FAIL;
                    }
                    else
                    {
                        ret = HYPVPP_STATUS_SUCCESS;
                        HYP_VPP_MSG_LOW("unimport fd %d size %u export id %u buf type 0x%x",
                                entry->fd, entry->size, entry->share_id, (unsigned int)entry->buf_type);
                    }
                }
                else
                {
                    if (0 != pHab_if->pfUnExport(entry->habmmhandle, entry->share_id, 0))
                    {
                        HYP_VPP_MSG_ERROR("failed to unexport share id %u fd %d", entry->share_id, entry->fd);
                        ret = HYPVPP_STATUS_FAIL;
                    }
                    else
                    {
                        ret = HYPVPP_STATUS_SUCCESS;
                        HYP_VPP_MSG_LOW("unexport fd %d size %u export id %u buf type 0x%x",
                                entry->fd, entry->size, entry->share_id, (unsigned int)entry->buf_type);
                    }
                }
                if (prev)
                {
                    prev->next = entry->next;
                }
                else
                {
                    queue->linkhead = entry->next;
                }
                if (queue->linktail == entry)
                {
                    queue->linktail = prev;
                }

                free(entry);
                break;
            }
            prev = entry;
            entry = entry->next;
        }
        MM_CriticalSection_Leave(queue->mutex);
    }

    return ret;
}

/**===========================================================================

  FUNCTION hypvpp_map_cleanup

  @brief  free all allocated heap entries

  @param [in] hab API interface
  @param [in] queue handle

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hypvpp_map_cleanup
(
    habIf* pHab_if,
    hypvpp_lookup_table_t* queue
)
{
    hypvpp_map_entry_t* tmp = NULL;
    hypvpp_map_entry_t* entry = NULL;
    hypvpp_status_type rc = HYPVPP_STATUS_SUCCESS;
    void* unimport_fd = NULL;


    if (!pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VPP_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        rc = HYPVPP_STATUS_FAIL;
    }
    else
    {
        MM_CriticalSection_Enter(queue->mutex);
        entry = queue->linkhead;
        while (entry)
        {
            if (0 == entry->is_export)
            {
                unimport_fd = (void*)(unsigned long)entry->fd;
                if (0 != pHab_if->pfUnImport(entry->habmmhandle, entry->share_id, unimport_fd, HABMM_EXPIMP_FLAGS_FD))
                {
                    HYP_VPP_MSG_ERROR("failed to unimport share id %u", entry->share_id);
                    rc = HYPVPP_STATUS_FAIL;
                }
            }
            else
            {
                if (0 != pHab_if->pfUnExport(entry->habmmhandle, entry->share_id, 0))
                {
                    HYP_VPP_MSG_ERROR("failed to unexport share id %u", entry->share_id);
                    rc = HYPVPP_STATUS_FAIL;
                }
            }

            tmp = entry;
            entry = entry->next;
            free(tmp);
        }
        queue->linkhead = NULL;
        queue->linktail = NULL;
        MM_CriticalSection_Leave(queue->mutex);
    }

    return rc;
}

/**===========================================================================

  FUNCTION hypvpp_map_cleanup_buf_type

  @brief  free all allocated heap entries of a buffer type

  @param [in] hab API interface
  @param [in] queue handle
  @param [in] buffer type

  @dependencies
  None

  @return
  Returns hypvpp_status_type

  ===========================================================================*/
hypvpp_status_type hypvpp_map_cleanup_buf_type
(
    habIf* pHab_if,
    hypvpp_lookup_table_t* queue,
    hypvpp_buffer_type buf_type
)
{
    hypvpp_status_type status = HYPVPP_STATUS_FAIL;
    hypvpp_map_entry_t* prev = NULL;
    hypvpp_map_entry_t* tmp = NULL;
    hypvpp_map_entry_t* entry = NULL;
    void* unimport_fd = NULL;

    if (!pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VPP_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        status = HYPVPP_STATUS_FAIL;
    }
    else
    {
        MM_CriticalSection_Enter(queue->mutex);
        entry = queue->linkhead;

        while (entry)
        {
            if (entry->buf_type == buf_type)
            {
                status = HYPVPP_STATUS_SUCCESS;
                if (0 == entry->is_export)
                {
                    unimport_fd = (void*)(unsigned long)entry->fd;
                    if (0 != pHab_if->pfUnImport(entry->habmmhandle, entry->share_id, unimport_fd, HABMM_EXPIMP_FLAGS_FD))
                    {
                        HYP_VPP_MSG_ERROR("failed to unimport share id %u fd %d", entry->share_id, entry->fd);
                        status = HYPVPP_STATUS_FAIL;
                    }
                }
                else
                {
                    if (0 != pHab_if->pfUnExport(entry->habmmhandle, entry->share_id, 0))
                    {
                        HYP_VPP_MSG_ERROR("failed to unexport share id %u", entry->share_id);
                        status = HYPVPP_STATUS_FAIL;
                    }
                }

                if (prev)
                {
                    prev->next = entry->next;
                }
                else
                {
                    queue->linkhead = entry->next;
                }
                if (queue->linktail == entry)
                {
                    queue->linktail = prev;
                }

                tmp = entry;
                entry = entry->next;
                free(tmp);
            }
            else
            {
                prev = entry;
                entry = entry->next;
            }
        }
        MM_CriticalSection_Leave(queue->mutex);
    }

    return status;
}

/**===========================================================================

  FUNCTION hypvpp_map_add_from_remote

  @brief

  This function creates a map entry if the entry does not exist or add a reference
  count for existing map entry

  @param [in] hab API interface
  @param [in] queue handle
  @param [in] habmmhandle   the virtual channel(habmm handle) from habmm open
  @param [in] size  size of memory chunk to be mapped
  @param [in] share_id  the share id exported by the remote process
  @param [in] buf_type  either input or output

  @dependencies
  None

  @return
  Returns file description

  ===========================================================================*/
int32 hypvpp_map_add_from_remote
(
    habIf* pHab_if,
    hypvpp_lookup_table_t* queue,
    int32 habmmhandle,
    uint32 size,
    uint32 share_id,
    hypvpp_buffer_type buf_type
)
{
    hypvpp_map_entry_t* entry;
    hypvpp_map_entry_t* find_entry;
    void* ret_va = NULL;
    int32 ret_fd = -1;

    if (!pHab_if || !pHab_if->pfImport)
    {
        HYP_VPP_MSG_ERROR("NULL pHab_if or pfImport, pHab_if = 0x%p", pHab_if);
    }
    else
    {
        MM_CriticalSection_Enter(queue->mutex);

        find_entry = queue->linkhead;

        while (find_entry)
        {
            if ((find_entry->share_id == share_id) &&
                    (find_entry->buf_type == buf_type) &&
                    (find_entry->size == size))
            {
                break;
            }
            find_entry = find_entry->next;
        }

        if (NULL == find_entry)
        {
            if (size)
            {
                if (0 != pHab_if->pfImport(habmmhandle, (void**)&ret_va, VPPFE_ALIGN(size, 4096), share_id, HABMM_EXPIMP_FLAGS_FD))
                {
                    HYP_VPP_MSG_ERROR("failed to import share id %u", share_id);
                }
                else if (NULL == ret_va)
                {
                    HYP_VPP_MSG_ERROR("returned va is NULL for share id(%u), import failed", share_id);
                }
                else
                {
                    entry = (hypvpp_map_entry_t*) malloc(sizeof(hypvpp_map_entry_t));
                    if (NULL == entry)
                    {
                        HYP_VPP_MSG_ERROR("malloc failed");
                        ret_va = NULL;
                    }
                    else
                    {
                        ret_fd = (int32)(unsigned long)ret_va;
                        entry->fd = ret_fd;
                        entry->share_id = share_id;
                        entry->next = NULL;
                        entry->size = size;
                        entry->refcnt = 1;
                        entry->habmmhandle = habmmhandle;
                        entry->is_export = 0;
                        entry->buf_type = buf_type;

                        if (queue->linktail)
                        {
                            queue->linktail->next = entry;
                        }
                        else
                        {
                            queue->linkhead = entry;
                        }
                        queue->linktail = entry;
                    }
                }
            }
        }
        else
        {
            find_entry->refcnt++;
            ret_fd = find_entry->fd;
        }

        MM_CriticalSection_Leave(queue->mutex);
    }

    return ret_fd;
}

/**===========================================================================

  FUNCTION hypvpp_map_add_to_remote

  @brief

  This function creates a map entry if the entry does not exist or add a reference
  count for existing map entry

  @param [in] hab API interface
  @param [in] queue handle
  @param [in] habmmhandle  the virtual channel(habmm handle) from habmm open
  @param [in] fd  file description
  @param [in] size  size of memory chunk to be mapped
  @param [in] buf_type  either input or output

  @dependencies
  None

  @return
  Returns export id into the remote process

  ===========================================================================*/
uint32 hypvpp_map_add_to_remote
(
    habIf* pHab_if,
    hypvpp_lookup_table_t* queue,
    int32 habmmhandle,
    int32 fd,
    uint32 size,
    hypvpp_buffer_type buf_type
)
{
    uint32 export_id = 0;
    hypvpp_map_entry_t*   find_entry;
    hypvpp_map_entry_t*   entry;
    int32 rc = 0;
    void* export_fd = (void*)(unsigned long)fd;

    if (!pHab_if || !pHab_if->pfExport || !pHab_if->pfUnExport)
    {
        HYP_VPP_MSG_ERROR("NULL pHab_if or pfExport, pHab_if = %p", pHab_if);
    }
    else
    {
        MM_CriticalSection_Enter(queue->mutex);

        find_entry = queue->linkhead;

        while (find_entry)
        {
            if ((find_entry->fd == fd) &&
                    (find_entry->buf_type == buf_type) &&
                    (find_entry->size == size))
            {
                break;
            }
            find_entry = find_entry->next;
        }

        if (NULL == find_entry)
        {
            if (size)
            {
                for (int i = 0; i < MAX_EXPORT_RETRY && -ENOMEM != rc; i++)
                {
                    if (0 != (rc = pHab_if->pfExport(habmmhandle, export_fd, VPPFE_ALIGN(size, 4096), &export_id, HABMM_EXPIMP_FLAGS_FD)))
                    {
                        HYP_VPP_MSG_ERROR("failed to export fd %d size %u buf type 0x%x retry %d rc %d",
                                fd, VPPFE_ALIGN(size, 4096), (unsigned int)buf_type, i, rc);
                    }
                    else
                    {
                        if (i > 0)
                        {
                            HYP_VPP_MSG_ERROR("Retry export success fd %d size %u buf type 0x%x export id %u retry %d",
                                    fd, VPPFE_ALIGN(size, 4096), (unsigned int)buf_type, export_id, i);
                        }
                        break;
                    }
                }
                if (0 == export_id)
                {
                    HYP_VPP_MSG_ERROR("failed to export. export id 0 fd %d size %u buf type 0x%x",
                            fd, size, (unsigned int)buf_type);
                }
                else
                {
                    entry = (hypvpp_map_entry_t*) malloc(sizeof(hypvpp_map_entry_t));
                    if (NULL == entry)
                    {
                        HYP_VPP_MSG_ERROR( "malloc failed");
                        if (0 != pHab_if->pfUnExport(habmmhandle, export_id, 0))
                        {
                            HYP_VPP_MSG_ERROR("failed to unexport id %u", export_id);
                        }
                        export_id = 0;
                    }
                    else
                    {
                        HYP_VPP_MSG_INFO("export fd %d size %u export id %u buf type 0x%x",
                                fd, size, export_id, (unsigned int)buf_type);
                        entry->fd = fd;
                        entry->share_id = export_id;
                        entry->next = NULL;
                        entry->size = size;
                        entry->refcnt = 1;
                        entry->habmmhandle = habmmhandle;
                        entry->buf_type = buf_type;
                        entry->is_export = 1;

                        if (queue->linktail)
                        {
                            queue->linktail->next = entry;
                        }
                        else
                        {
                            queue->linkhead = entry;
                        }
                        queue->linktail = entry;
                    }
                }
            }
        }
        else
        {
            export_id = find_entry->share_id;
            find_entry->refcnt++;
        }

        MM_CriticalSection_Leave(queue->mutex);
    }

    return export_id;
}

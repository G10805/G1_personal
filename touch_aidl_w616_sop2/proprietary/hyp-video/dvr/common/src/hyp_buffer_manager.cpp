/*===========================================================================
 Video hypervisor IO buffer management

 *//** @file hyp_buffer_manager.c
 This file provides functions to map the virtual address between local
 and remote process using habmm APIs

Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

 *//*========================================================================*/

/*===========================================================================
 Edit History

$Header: $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
01/15/20           sh          Bringup DVR on LA GVM on Hana
04/10/19           sh          Disable import/unimport of buffers during muxing
10/25/18           sm          Clean up logging
09/19/18           sm          Fix NULL pointer dereferencing
08/22/18           sm          Initial version of hypervisor DVR
=============================================================================*/

/*========================================================================
 Define Macro
 ========================================================================*/

/*===========================================================================
 Include Files
 ============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#ifdef _ANDROID_
#include "comdef.h"
#endif
#include "hyp_buffer_manager.h"
#include "MMDebugMsg.h"
#include "habmm.h"
#include "hyp_debug.h"

//###############################################################
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**===========================================================================

FUNCTION hypv_map_from_lookup

@brief  Given an exported  buffer id finds an entry node

@param [in] queue queue handle
@param [in] habmmhandle   the virtual channel(habmm handle) from habmm open
@param [in] bufferid  the buffer id exported by the remote process

@dependencies
  None

@return
  Returns the entry node that associates with the buffer id

===========================================================================*/
hypv_map_entry_t* hypv_map_from_lookup
(
    hypv_lookup_table_t* queue,
    int32 habmmhandle,
    uint32 bufferid
)
{
    hypv_map_entry_t* aEntry = queue->linkhead;

    while (aEntry)
    {
        if (aEntry->habmmhandle == habmmhandle && aEntry->bufferid == bufferid)
        {
            break;
        }
        aEntry = aEntry->next;
    }
    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_to_lookup

@brief  Given a virtual address of the buffer finds an entry node

@param [in] queue queue handle
@param [in] habmmhandle   the virtual channel(habmm handle) from habmm open
@param [in] va  the virtual address given by the local process

@dependencies
  None

@return
  Returns the entry node that associates with the virtual address

===========================================================================*/
hypv_map_entry_t* hypv_map_to_lookup
(
    hypv_lookup_table_t* queue,
    int32 habmmhandle,
    void* va
)
{
    hypv_map_entry_t* aEntry = queue->linkhead;

    while (aEntry)
    {
        if (aEntry->habmmhandle == habmmhandle && aEntry->va == va)
        {
            break;
        }
        aEntry = aEntry->next;
    }
    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_free

@brief  Given a virtual address in the server process
        free its cached map entry if there is any

@param [in] hab API interface
@param [in] queue queue handle
@param [in] virt  the server virtual address that was cached

@dependencies
  None

@return
  Returns hdvr_status_type

===========================================================================*/
hdvr_status_type hypv_map_free
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    void* virt
)
{
    hdvr_status_type aStatus = HDVR_STATUS_FAIL;
    hypv_map_entry_t* aPrev = NULL;
    hypv_map_entry_t* aEntry = queue->linkhead;

    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HDVR_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    while (aEntry)
    {
        if (aEntry->va == virt)
        {
            aStatus = HDVR_STATUS_SUCCESS;
            aEntry->refcnt -= 1;
            if (aEntry->refcnt != 0)
            {
                break;
            }
            if (0 == aEntry->is_export)
            {
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, 0))
               {
                  HYP_VIDEO_MSG_ERROR("failed to unimport buffer id %d va 0x%p", aEntry->bufferid, aEntry->va);
                  aStatus = HDVR_STATUS_FAIL;
               }
            }
            else
            {
               if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->bufferid, HABMM_EXPIMP_FLAGS_FD))
               {
                  HYP_VIDEO_MSG_ERROR("failed to unexport buffer id %d", aEntry->bufferid);
                  aStatus = HDVR_STATUS_FAIL;
               }
            }
            if (aPrev)
            {
               aPrev->next = aEntry->next;
            }
            else
            {
               queue->linkhead = aEntry->next;
            }
            if (queue->linktail == aEntry)
            {
               queue->linktail = aPrev;
            }

            free(aEntry);
            break;
        }
        aPrev = aEntry;
        aEntry = aEntry->next;
    }
    pthread_mutex_unlock(&mutex);
    return aStatus;
}

/**===========================================================================

FUNCTION hypv_map_cleanup

@brief  free all allocated heap entries

@param [in] hab API interface
@param [in] queue queue handle

@dependencies
  None

@return
  Returns hdvr_status_type

===========================================================================*/
hdvr_status_type hypv_map_cleanup
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue
)
{
    void* aPtr = NULL;
    hypv_map_entry_t* aEntry = NULL;
    hdvr_status_type rc = HDVR_STATUS_SUCCESS;


    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HDVR_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    aEntry = queue->linkhead;
    while (aEntry)
    {
        aPtr = aEntry;
        if (0 == aEntry->is_export)
        {
           if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, 0))
           {
              HYP_VIDEO_MSG_ERROR("failed to unimport buffer id %d", aEntry->bufferid);
              rc = HDVR_STATUS_FAIL;
           }
        }
        else
        {
           if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->bufferid, HABMM_EXPIMP_FLAGS_FD))
           {
              HYP_VIDEO_MSG_ERROR("failed to unexport buffer id %d", aEntry->bufferid);
              rc = HDVR_STATUS_FAIL;
           }
        }
        aEntry = aEntry->next;
        free(aPtr);
    }
    queue->linkhead = NULL;
    queue->linktail = NULL;
    pthread_mutex_unlock(&mutex);
    return rc;
}

/**===========================================================================

FUNCTION hypv_map_add_from_remote

@brief

   This function creates a map entry if the entry does not exist or add a reference
   count for existing map entry

@param [in] hab API interface
@param [in] queue queue handle
@param [in] habmmhandle   the virtual channel(habmm handle) from habmm open
@param [in] size  size of memory chunk to be mapped
@param [in] bufferid  the buffer id exported by the remote process

@dependencies
  None

@return
  Returns virtual address pointer mapped into the local process

===========================================================================*/
void* hypv_map_add_from_remote
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    int32 habmmhandle,  /* the virtual channel(habmm handle) from habmm open */
    uint32 size,        /* size of the memory chunk to be allocated */
    uint32 bufferid     /* the buffer id exported by the remote process */
)
{
    hypv_map_entry_t* aEntry;
    void* ret_va = NULL;

    if ( !pHab_if || !pHab_if->pfImport )
    {
        HYP_VIDEO_MSG_ERROR("pHab_if=%p or pfImport is NULL", pHab_if);
        return NULL;
    }

    pthread_mutex_lock(&mutex);
    aEntry = hypv_map_from_lookup(queue, habmmhandle, bufferid);
    if (aEntry)
    {
        ret_va = aEntry->va;
    }
    else if (size)
    {
        if (0 != pHab_if->pfImport(habmmhandle, (void**)&ret_va, BUF_ALIGN(size, 4096), bufferid, 0))
        {
            HYP_VIDEO_MSG_ERROR("failed to import buffer id %d", bufferid);
        }
        else if (NULL == ret_va)
        {
            HYP_VIDEO_MSG_ERROR("returned va is NULL for bufferid(%d), import failed", bufferid);
        }
        else
        {
            aEntry = (hypv_map_entry_t*)malloc(sizeof(hypv_map_entry_t));
            if (aEntry == NULL)
            {
                HYP_VIDEO_MSG_ERROR("malloc failed");
                ret_va = NULL;
            }
            else
            {
                aEntry->va = ret_va;
                aEntry->bufferid = bufferid;
                aEntry->next = NULL;
                aEntry->size = size;
                aEntry->refcnt = 1;
                aEntry->habmmhandle = habmmhandle;
                aEntry->is_export = 0;

                if (queue->linktail)
                {
                    queue->linktail->next = aEntry;
                }
                else
                {
                    queue->linkhead = aEntry;
                }
                queue->linktail = aEntry;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    return ret_va;
}

/**===========================================================================

FUNCTION hypv_map_add_to_remote

@brief

   This function creates a map entry if the entry does not exist or add a reference
   count for existing map entry

@param [in] hab API interface
@param [in] queue queue handle
@param [in] habmmhandle  the virtual channel(habmm handle) from habmm open
@param [in] virt  local process virtual address
@param [in] size  size of memory chunk to be mapped

@dependencies
  None

@return
  Returns export id into the remote process

===========================================================================*/
uint32 hypv_map_add_to_remote
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    int32 habmmhandle,  /* the virtual channel(habmm handle) from habmm open */
    void* virt,         /* local process virtual address */
    uint32 size         /* size of the memory chunk to be allocated */
)
{
    uint32 export_id = 0;
    hypv_map_entry_t* aEntry;

    if ( !pHab_if || !pHab_if->pfExport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if=%p or pfExport or pfUnExport", pHab_if);
        return 0;
    }

    pthread_mutex_lock(&mutex);
    aEntry = hypv_map_to_lookup(queue, habmmhandle, virt);
    if (aEntry)
    {
        export_id = aEntry->bufferid;
    }
    else if (size)
    {
        if (0 != pHab_if->pfExport(habmmhandle, virt, BUF_ALIGN(size, 4096), &export_id, HABMM_EXPIMP_FLAGS_FD))
        {
            HYP_VIDEO_MSG_ERROR("failed to export virt addr 0x%p", virt);
        }
        else if (0 == export_id)
        {
            HYP_VIDEO_MSG_ERROR("return export id is 0, export va 0x%p failed", virt);
        }
        else
        {
            aEntry = (hypv_map_entry_t*)malloc(sizeof(hypv_map_entry_t));
            if (aEntry == NULL)
            {
                HYP_VIDEO_MSG_ERROR( "malloc failed");
                if (0 != pHab_if->pfUnExport(habmmhandle, export_id, 0))
                {
                   HYP_VIDEO_MSG_ERROR("failed to unexport id=%d",export_id);
                }
                export_id = 0;
            }
            else
            {
                aEntry->va = virt;
                aEntry->bufferid = export_id;
                aEntry->next = NULL;
                aEntry->size = size;
                aEntry->refcnt = 1;
                aEntry->habmmhandle = habmmhandle;
                aEntry->is_export = 1;

                if (queue->linktail)
                {
                    queue->linktail->next = aEntry;
                }
                else
                {
                    queue->linkhead = aEntry;
                }
                queue->linktail = aEntry;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    return export_id;
}

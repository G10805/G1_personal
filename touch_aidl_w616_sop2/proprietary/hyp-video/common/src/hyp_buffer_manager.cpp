/*===========================================================================
 Video hypervisor IO buffer management

 *//** @file hyp_buffer_manager.cpp
 This file provides functions to map the virtual address between local and remote process
 using habmm APIs

Copyright (c) 2016-2021, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

 *//*========================================================================*/

/*===========================================================================
 Edit History

$Header: //deploy/qcom/qct/platform/qnp/qnx/auto/components/rel/vm_video.qxa_qa/1.0/common/src/hyp_buffer_manager.c#4 $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           nb          Fix compilation errors due to additon of new compiler flags
07/12/21           bf          Judge whether aEntry isn't NULL before adding it into queue
04/01/21           sh          Bringup video on RedBend Hypervisor
08/27/20           sh          Bringup video decode using codec2
07/10/19           sm          Avoid HAB export retry in OOM
05/31/19           sm          Add HAB export retry on failure
05/30/19           sm          Add support for hab export using ion fd
03/06/19           rz          Handle decoder output dynamic map/unmap/export/unexport buffers
02/15/19           rz          Update logs for hab export
02/05/19           rz          Bringup changes for 8155
10/30/18           sm          Clean buffer mapping based on buffer type
08/21/18           aw          Fix Klockwork P1, compilation and MISRA warning
07/10/18           sm          Add support to use pmem handle for video buffersadb
02/15/18           sm          Update habmm header location
09/20/17           sm          Add import/export 4k alignment to meet HAB requirement
09/18/17           sm          Use official habmm header
06/28/17           aw          Unify and update all logs in hyp-video
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
01/30/17           sm          Remove dependency on PPGA
08/16/16           rz          Fix hypv_map_cleanup crash
08/16/16           rz          Dynamically loading hab library
07/15/16           hl          Add code to support LA target
06/22/16           hl          Add dynamic buffer mode support
06/01/16           hl          Add FE and BE to support Hypervisor interface

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
#include <errno.h>
#include "hyp_buffer_manager.h"
#include "MMDebugMsg.h"
#include "habmm.h"
#include "hyp_debug.h"

#define MAX_EXPORT_RETRY 5

//###############################################################
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**===========================================================================

FUNCTION hypv_map_from_lookup

@brief  Given an exported  buffer id finds an entry node

@param [in] queue queue handle
@param [in] bufferid  the buffer id exported by the remote process

@dependencies
  None

@return
  Returns the entry node that associates with the buffer id

===========================================================================*/
hypv_map_entry_t* hypv_map_from_lookup
(
    hypv_lookup_table_t* queue,
    uint32 bufferid
)
{
    hypv_map_entry_t* aEntry = queue->linkhead;

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ( aEntry->bufferid == bufferid )
        {
            break;
        }
        aEntry = aEntry->next;
    }

    pthread_mutex_unlock(&mutex);

    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_from_lookup_ext

@brief  Given an exported  buffer id finds an entry node

@param [in] queue queue handle
@param [in] bufferid  the buffer id exported by the remote process
@param [in] is_readonly readonly flag for the associated buffer

@dependencies
  None

@return
  Returns the entry node that associates with the buffer id

===========================================================================*/
hypv_map_entry_ex_t* hypv_map_from_lookup_ext
(
    hypv_lookup_table_ex_t* queue,
    uint32 bufferid,
    boolean is_readonly
)
{
    hypv_map_entry_ex_t* aEntry = queue->linkhead;

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ((aEntry->frame_bufferid == bufferid) &&
            (aEntry->is_readonly == is_readonly))
        {
            break;
        }
        aEntry = aEntry->next;
    }

    pthread_mutex_unlock(&mutex);

    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_update_readonly

@brief  update readonly flag in the map entry

@param [in] entry map entry
@param [in] is_readonly readonly flag

@dependencies
  None

@return
  None

===========================================================================*/
void hypv_map_update_readonly
(
    hypv_map_entry_ex_t* entry,
    boolean is_readonly
)
{
    pthread_mutex_lock(&mutex);

    entry->is_readonly = is_readonly;

    pthread_mutex_unlock(&mutex);
}

/**===========================================================================

FUNCTION hypv_map_to_lookup

@brief  Given a virtual address of the buffer finds an entry node

@param [in] queue queue handle
@param [in] va  the virtual address given by the local process

@dependencies
  None

@return
  Returns the entry node that associates with the virtual address

===========================================================================*/
hypv_map_entry_t* hypv_map_to_lookup
(
    hypv_lookup_table_t* queue,
    void* va
)
{
    hypv_map_entry_t* aEntry = queue->linkhead;

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if (aEntry->va == va)
        {
            break;
        }
        aEntry = aEntry->next;
    }

    pthread_mutex_unlock(&mutex);

    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_to_lookup_ext

@brief  Given a virtual address of the buffer finds an entry node

@param [in] queue queue handle
@param [in] frame_addr  the virtual address given by the local process
@param [in] is_readonly readonly flag for the associated buffer

@dependencies
  None

@return
  Returns the entry node that associates with the virtual address

===========================================================================*/
hypv_map_entry_ex_t* hypv_map_to_lookup_ext
(
    hypv_lookup_table_ex_t* queue,
    void* frame_addr,
    boolean is_readonly
)
{
    hypv_map_entry_ex_t* aEntry = queue->linkhead;

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ((aEntry->frame_addr == frame_addr) && (aEntry->is_readonly == is_readonly))
        {
            break;
        }
        aEntry = aEntry->next;
    }

    pthread_mutex_unlock(&mutex);

    return aEntry;
}

/**===========================================================================

FUNCTION hypv_map_get_entry

@brief

   This function gets the map entry in the lookup table

@param [in] queue queue handle
@param [in] index index of the buffer
@param [in] is_stale stale info of the buffer
@param [in] buf_type either input or output

@dependencies
  None

@return
  Returns map entry in the lookup table

===========================================================================*/
hypv_map_entry_ex_t* hypv_map_get_entry
(
    hypv_lookup_table_ex_t* queue,
    uint32 index,
    boolean is_stale,
    uint32 buf_type
)
{
    hypv_map_entry_ex_t* aEntry = queue->linkhead;

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ((aEntry->index == index) &&
            (aEntry->is_stale == is_stale) &&
            (aEntry->buf_type == buf_type))
        {
            break;
        }
        aEntry = aEntry->next;
    }

    pthread_mutex_unlock(&mutex);

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
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_free
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    void* virt,
    uint32 bufferid
)
{
    hypv_status_type aStatus = HYPV_STATUS_FAIL;
    hypv_map_entry_t* aPrev = NULL;
    hypv_map_entry_t* aEntry = queue->linkhead;

    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ((aEntry->va == virt) && (aEntry->bufferid == bufferid))
        {
            aStatus = HYPV_STATUS_SUCCESS;
            aEntry->refcnt -= 1;
            if (aEntry->refcnt != 0)
            {
               break;
            }
            if (0 == aEntry->is_export)
            {
#ifdef USE_BUFFER_HANDLE
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, HABMM_EXPIMP_FLAGS_FD))
#else
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, 0))
#endif
               {
                  HYP_VIDEO_MSG_ERROR("failed to unimport buffer id %u va 0x%p", aEntry->bufferid, aEntry->va);
                  aStatus = HYPV_STATUS_FAIL;
               }
            }
            else
            {
               if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->bufferid, 0))
               {
                  HYP_VIDEO_MSG_ERROR("failed to unexport buffer id %u", aEntry->bufferid);
                  aStatus = HYPV_STATUS_FAIL;
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

FUNCTION hypv_map_free_ext

@brief  Given a virtual address in the server process
        free its cached map entry if there is any

@param [in] hab API interface
@param [in] queue queue handle
@param [in] index index of the buffer
@param [in] frame_addr  the server virtual address that was cached
@param [in] frame_bufferid  export/import id of the buffer

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_free_ext
(
    habIf*                pHab_if,
    hypv_lookup_table_ex_t* queue,
    uint32 index,
    void* frame_addr,
    uint32 frame_bufferid
)
{
    hypv_status_type aStatus = HYPV_STATUS_FAIL;
    hypv_map_entry_ex_t* aPrev = NULL;
    hypv_map_entry_ex_t* aEntry = queue->linkhead;

    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);

    while (aEntry)
    {
        if ((aEntry->index == index) &&
            (aEntry->frame_bufferid == frame_bufferid) &&
            (aEntry->frame_addr == frame_addr))
        {
            aStatus = HYPV_STATUS_SUCCESS;
            aEntry->refcnt -= 1;
            if (aEntry->refcnt != 0)
            {
               break;
            }
            if (0 == aEntry->is_export)
            {
#ifdef USE_BUFFER_HANDLE
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, HABMM_EXPIMP_FLAGS_FD))
#else
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, 0))
#endif
               {
                   HYP_VIDEO_MSG_ERROR("failed to unimport frame buffer id %u va 0x%p", aEntry->frame_bufferid, aEntry->frame_addr);
                   aStatus = HYPV_STATUS_FAIL;
               }

               if (0 != aEntry->extradata_bufferid)
               {
#ifdef USE_BUFFER_HANDLE
                   if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, HABMM_EXPIMP_FLAGS_FD))
#else
                   if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, 0))
#endif
                   {
                       HYP_VIDEO_MSG_ERROR("failed to unimport extradata buffer id %u va 0x%p", aEntry->extradata_bufferid, aEntry->extradata_addr);
                   }
               }
            }
            else
            {
                if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->frame_bufferid, 0))
                {
                    HYP_VIDEO_MSG_ERROR("failed to unexport frame buffer id %u", aEntry->frame_bufferid);
                    aStatus = HYPV_STATUS_FAIL;
                }

                if (0 != aEntry->extradata_bufferid)
                {
                    if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->extradata_bufferid, 0))
                    {
                        HYP_VIDEO_MSG_ERROR("failed to unexport extradata buffer id %u", aEntry->extradata_bufferid);
                        aStatus = HYPV_STATUS_FAIL;
                    }
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
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_cleanup
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue
)
{
    void* aPtr = NULL;
    hypv_map_entry_t* aEntry = NULL;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;


    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    aEntry = queue->linkhead;
    while (aEntry)
    {
        aPtr = aEntry;
        if (0 == aEntry->is_export)
        {
#ifdef USE_BUFFER_HANDLE
           if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, HABMM_EXPIMP_FLAGS_FD))
#else
           if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, 0))
#endif
           {
              HYP_VIDEO_MSG_ERROR("failed to unimport buffer id %u", aEntry->bufferid);
              rc = HYPV_STATUS_FAIL;
           }
        }
        else
        {
           if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->bufferid, 0))
           {
              HYP_VIDEO_MSG_ERROR("failed to unexport buffer id %u", aEntry->bufferid);
              rc = HYPV_STATUS_FAIL;
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

FUNCTION hypv_map_cleanup_ext

@brief  free all allocated heap entries and mmap entries.

@param [in] hab API interface
@param [in] queue queue handle

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_cleanup_ext
(
    habIf*                pHab_if,
    hypv_lookup_table_ex_t* queue
)
{
    void* aPtr = NULL;
    hypv_map_entry_ex_t* aEntry = NULL;
    hypv_status_type rc = HYPV_STATUS_SUCCESS;


    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    aEntry = queue->linkhead;
    while (aEntry)
    {
        aPtr = aEntry;
        if (0 == aEntry->is_export)
        {
#ifdef USE_BUFFER_HANDLE
           if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, HABMM_EXPIMP_FLAGS_FD))
#else
           if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, 0))
#endif
           {
              HYP_VIDEO_MSG_ERROR("failed to unimport frame buffer id %u", aEntry->frame_bufferid);
              rc = HYPV_STATUS_FAIL;
           }

           if (0 != aEntry->extradata_bufferid)
           {
#ifdef USE_BUFFER_HANDLE
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, HABMM_EXPIMP_FLAGS_FD))
#else
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, 0))
#endif
               {
                   HYP_VIDEO_MSG_ERROR("failed to unimport extradata buffer id %u", aEntry->extradata_bufferid);
                   rc = HYPV_STATUS_FAIL;
               }
           }
        }
        else
        {
           if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->frame_bufferid, 0))
           {
               HYP_VIDEO_MSG_ERROR("failed to unexport frame buffer id %u", aEntry->frame_bufferid);
               rc = HYPV_STATUS_FAIL;
           }

           if (0 != aEntry->extradata_bufferid)
           {
               if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->extradata_bufferid, 0))
               {
                   HYP_VIDEO_MSG_ERROR("failed to unexport extradata buffer id %u", aEntry->extradata_bufferid);
                   rc = HYPV_STATUS_FAIL;
               }
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

FUNCTION hypv_map_cleanup_buf_type

@brief  free all allocated heap entries of a buffer type

@param [in] hab API interface
@param [in] queue queue handle
@param [in] buffer type

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_cleanup_buf_type
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    vidc_buffer_type buf_type
)
{
    hypv_status_type aStatus = HYPV_STATUS_FAIL;
    hypv_map_entry_t* aPrev = NULL, *aTemp = NULL;
    hypv_map_entry_t* aEntry = queue->linkhead;

    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    while (aEntry)
    {
        if (aEntry->buf_type == buf_type)
        {
            aStatus = HYPV_STATUS_SUCCESS;
            if (0 == aEntry->is_export)
            {
#ifdef USE_BUFFER_HANDLE
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, HABMM_EXPIMP_FLAGS_FD))
#else
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->bufferid, aEntry->va, 0))
#endif
               {
                  HYP_VIDEO_MSG_ERROR("failed to unimport buffer id %u va 0x%p", aEntry->bufferid, aEntry->va);
                  aStatus = HYPV_STATUS_FAIL;
               }
            }
            else
            {
               if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->bufferid, 0))
               {
                  HYP_VIDEO_MSG_ERROR("failed to unexport buffer id %u", aEntry->bufferid);
                  aStatus = HYPV_STATUS_FAIL;
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

            aTemp = aEntry;
            aEntry = aEntry->next;
            free(aTemp);
        }
        else
        {
            aPrev = aEntry;
            aEntry = aEntry->next;
        }
    }
    pthread_mutex_unlock(&mutex);

    return aStatus;
}

/**===========================================================================

FUNCTION hypv_map_cleanup_buf_type_ext

@brief  free all allocated heap entries and mmap entries
        of a buffer type.

@param [in] hab API interface
@param [in] queue queue handle
@param [in] buffer type

@dependencies
  None

@return
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_cleanup_buf_type_ext
(
    habIf*                  pHab_if,
    hypv_lookup_table_ex_t* queue,
    uint32                  buf_type
)
{
    hypv_status_type aStatus = HYPV_STATUS_FAIL;
    hypv_map_entry_ex_t* aPrev = NULL, *aTemp = NULL;
    hypv_map_entry_ex_t* aEntry = queue->linkhead;

    if ( !pHab_if || !pHab_if->pfUnImport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfUnImport or pfUnExport");
        return HYPV_STATUS_FAIL;
    }

    pthread_mutex_lock(&mutex);
    while (aEntry)
    {
        if (aEntry->buf_type == buf_type)
        {
            aStatus = HYPV_STATUS_SUCCESS;
            if (0 == aEntry->is_export)
            {
#ifdef USE_BUFFER_HANDLE
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, HABMM_EXPIMP_FLAGS_FD))
#else
               if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->frame_bufferid, aEntry->frame_addr, 0))
#endif
               {
                  HYP_VIDEO_MSG_ERROR("failed to unimport frame buffer id %u va 0x%p", aEntry->frame_bufferid, aEntry->frame_addr);
                  aStatus = HYPV_STATUS_FAIL;
               }

               if (0 != aEntry->extradata_bufferid)
               {
#ifdef USE_BUFFER_HANDLE
                   if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, HABMM_EXPIMP_FLAGS_FD))
#else
                   if (0 != pHab_if->pfUnImport(aEntry->habmmhandle, aEntry->extradata_bufferid, aEntry->extradata_addr, 0))
#endif
                   {
                       HYP_VIDEO_MSG_ERROR("failed to unimport extradata buffer id %u va 0x%p", aEntry->extradata_bufferid, aEntry->extradata_addr);
                   }
               }
            }
            else
            {
               if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->frame_bufferid, 0))
               {
                  HYP_VIDEO_MSG_ERROR("failed to unexport frame buffer id %u", aEntry->frame_bufferid);
                  aStatus = HYPV_STATUS_FAIL;
               }

               if (0 != aEntry->extradata_bufferid)
               {
                   if (0 != pHab_if->pfUnExport(aEntry->habmmhandle, aEntry->extradata_bufferid, 0))
                   {
                       HYP_VIDEO_MSG_ERROR("failed to unexport extradata buffer id %u", aEntry->extradata_bufferid);
                       aStatus = HYPV_STATUS_FAIL;
                   }
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

            aTemp = aEntry;
            aEntry = aEntry->next;
            free(aTemp);
        }
        else
        {
            aPrev = aEntry;
            aEntry = aEntry->next;
        }
    }
    pthread_mutex_unlock(&mutex);

    return aStatus;
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
@param [in] buf_type  either input or output

@dependencies
  None

@return
  Returns virtual address pointer mapped into the local process

===========================================================================*/
void* hypv_map_add_from_remote
(
    habIf*                 pHab_if,
    hypv_lookup_table_t*   queue,
    int32                  habmmhandle,  /* the virtual channel(habmm handle) from habmm open */
    uint32                 size,        /* size of the memory chunk to be allocated */
    uint32                 bufferid,     /* the buffer id exported by the remote process */
    vidc_buffer_type       buf_type /* either input or otuput buffer */
)
{
    hypv_map_entry_t* aEntry;
    hypv_map_entry_t* find_entry;
    void* ret_va = NULL;

    if ( !pHab_if || !pHab_if->pfImport )
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfImport, pHab_if = 0x%p", pHab_if);
        return NULL;
    }

    pthread_mutex_lock(&mutex);

    find_entry = queue->linkhead;

    while (find_entry)
    {
        if ((find_entry->bufferid == bufferid) &&
            (find_entry->buf_type == buf_type) &&
            (find_entry->size == size))
        {
            break;
        }
        find_entry = find_entry->next;
    }

    if ( NULL == find_entry )
    {
       if (size)
       {
#ifdef USE_BUFFER_HANDLE
          if (0 != pHab_if->pfImport(habmmhandle, (void**)&ret_va, V4L2FE_ALIGN(size, 4096), bufferid, HABMM_EXPIMP_FLAGS_FD))
#else
          if (0 != pHab_if->pfImport(habmmhandle, (void**)&ret_va, V4L2FE_ALIGN(size, 4096), bufferid, 0))
#endif
          {
             HYP_VIDEO_MSG_ERROR("failed to import buffer id %u", bufferid);
          }
          else if (NULL == ret_va)
          {
             HYP_VIDEO_MSG_ERROR("returned va is NULL for bufferid(%u), import failed", bufferid);
          }
          else
          {
             aEntry = (hypv_map_entry_t*) malloc(sizeof(hypv_map_entry_t));
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
                aEntry->buf_type = buf_type;

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
    }
    else
    {
       find_entry->refcnt++;
       ret_va = find_entry->va;
    }

    pthread_mutex_unlock(&mutex);

    return ret_va;
}

/**===========================================================================

FUNCTION hypv_map_add_from_remote_ext

@brief

   This function creates a map entry if the entry does not exist

@param [in] hab API interface
@param [in] queue queue handle
@param [in] habmmhandle  the virtual channel(habmm handle) from habmm open
@param [in] index index of the buffer
@param [in] frame_bufferid  the buffer id exported by the remote process for frame buffer
@param [in] frame_size  size of memory chunk to be mapped for frame buffer
@param [in] extradata_bufferid  the buffer id exported by the remote process for extradata buffer
@param [in] extradata_size  size of memory chunk to be mapped for extradata buffer
@param [in] buf_type  either input or output

@dependencies
  None

@return
  Returns map entry

===========================================================================*/
hypv_map_entry_ex_t* hypv_map_add_from_remote_ext
(
    habIf*                 pHab_if,
    hypv_lookup_table_ex_t*   queue,
    int32                  habmmhandle,
    uint32                 index,
    uint32                 frame_bufferid,
    uint32                 frame_size,
    uint32                 extradata_bufferid,
    uint32                 extradata_size,
    uint32                 buf_type
)
{
    hypv_map_entry_ex_t* aEntry = NULL;
    hypv_map_entry_ex_t* find_entry = NULL;
    void* frame_addr = NULL;
    void* extradata_addr = NULL;

    if ( !pHab_if || !pHab_if->pfImport )
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfImport, pHab_if = 0x%p", pHab_if);
        return NULL;
    }

    pthread_mutex_lock(&mutex);

    find_entry = queue->linkhead;

    while (find_entry)
    {
        if ((find_entry->index == index) &&
            (find_entry->buf_type == buf_type))
        {
            find_entry->is_stale = 1;
            find_entry = NULL;
            break;
        }
        find_entry = find_entry->next;
    }

    if (NULL == find_entry)
    {
        if (frame_size)
        {
#ifdef USE_BUFFER_HANDLE
            if (0 != pHab_if->pfImport(habmmhandle, (void**)&frame_addr, V4L2FE_ALIGN(frame_size, 4096), frame_bufferid, HABMM_EXPIMP_FLAGS_FD))
#else
            if (0 != pHab_if->pfImport(habmmhandle, (void**)&frame_addr, V4L2FE_ALIGN(frame_size, 4096), frame_bufferid, 0))
#endif
            {
                HYP_VIDEO_MSG_ERROR("failed to import frame buffer id %u", frame_bufferid);
            }
        }
        if ((0 != extradata_bufferid) && extradata_size)
        {
#ifdef USE_BUFFER_HANDLE
            if (0 != pHab_if->pfImport(habmmhandle, (void**)&extradata_addr, V4L2FE_ALIGN(extradata_size, 4096), extradata_bufferid, HABMM_EXPIMP_FLAGS_FD))
#else
            if (0 != pHab_if->pfImport(habmmhandle, (void**)&extradata_addr, V4L2FE_ALIGN(extradata_size, 4096), extradata_bufferid, 0))
#endif
            {
                HYP_VIDEO_MSG_ERROR("failed to import extradata buffer id %u extradata_size %u", extradata_bufferid, extradata_size);
            }
        }

        if ((NULL == frame_addr) ||
            ((NULL == extradata_addr) && (0 != extradata_bufferid)))
        {
            HYP_VIDEO_MSG_ERROR("returned va is %p for frame bufferid(%u), va is %p for extradata bufferid (%u) import failed",
                         frame_addr, frame_bufferid, extradata_addr, extradata_bufferid);
        }
        else
        {
            aEntry = (hypv_map_entry_ex_t*)malloc(sizeof(hypv_map_entry_ex_t));
            if (NULL == aEntry)
            {
                HYP_VIDEO_MSG_ERROR("malloc failed");
#ifdef USE_BUFFER_HANDLE
                if (0 != pHab_if->pfUnImport(habmmhandle, frame_bufferid, frame_addr, HABMM_EXPIMP_FLAGS_FD))
#else
                if (0 != pHab_if->pfUnImport(habmmhandle, frame_bufferid, frame_addr, 0))
#endif
                {
                    HYP_VIDEO_MSG_ERROR("failed to unimport frame buffer id %u", frame_bufferid);
                }

                if ((0 != extradata_bufferid) && extradata_size)
                {
#ifdef USE_BUFFER_HANDLE
                    if (0 != pHab_if->pfUnImport(habmmhandle, extradata_bufferid, extradata_addr, HABMM_EXPIMP_FLAGS_FD))
#else
                    if (0 != pHab_if->pfUnImport(habmmhandle, extradata_bufferid, extradata_addr, 0))
#endif
                    {
                        HYP_VIDEO_MSG_ERROR("failed to unimport extradata buffer id %u", extradata_bufferid);
                    }
                }
            }
            else
            {

                aEntry->index = index;
                aEntry->frame_addr = frame_addr;
                aEntry->frame_size = frame_size;
                aEntry->extradata_addr = extradata_addr;
                aEntry->extradata_size = extradata_size;
                aEntry->frame_bufferid = frame_bufferid;
                aEntry->extradata_bufferid = extradata_bufferid;
                aEntry->refcnt = 1;
                aEntry->habmmhandle = habmmhandle;
                aEntry->buf_type = buf_type;
                aEntry->is_export = 0;
                aEntry->is_stale = 0;
                aEntry->is_readonly = 0;
                aEntry->next = NULL;

                if (queue->linktail)
                {
                    queue->linktail->next = aEntry;
                }
                else
                {
                    queue->linkhead = aEntry;
                }
                queue->linktail = aEntry;

                find_entry = aEntry;
            }
        }
    }

    pthread_mutex_unlock(&mutex);

    return find_entry;
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
@param [in] buf_type  either input or output

@dependencies
  None

@return
  Returns export id into the remote process

===========================================================================*/
uint32 hypv_map_add_to_remote
(
    habIf*                 pHab_if,
    hypv_lookup_table_t*   queue,
    int32                  habmmhandle,   /* the virtual channel(habmm handle) from habmm open */
    void*                  virt,          /* local process virtual address */
    uint32                 size,          /* size of the memory chunk to be allocated */
    vidc_buffer_type       buf_type,       /* either input or otuput buffer */
    boolean                export_as_fd  /* flag to indicate buffer to export as fd or va*/
)
{
    uint32 export_id = 0;
    hypv_map_entry_t*   find_entry;
    hypv_map_entry_t*   aEntry;
    int32 rc = 0;

    if ( !pHab_if || !pHab_if->pfExport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfExport, pHab_if = %p", pHab_if);
        return 0;
    }

    pthread_mutex_lock(&mutex);

    find_entry = queue->linkhead;

    while (find_entry)
    {
        if ((find_entry->va == virt) &&
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
          uint32 export_flag = 0;
#ifndef __QNXNTO__
          //ToDo Remove buf_type check
          if (VIDC_BUFFER_INPUT == buf_type ||
              VIDC_BUFFER_OUTPUT == buf_type ||
              VIDC_BUFFER_OUTPUT2 == buf_type ||
              true == export_as_fd)
          {
              export_flag = HABMM_EXPIMP_FLAGS_FD;
          }
          else
          {
              export_flag = HABMM_EXP_MEM_TYPE_DMA;
          }
#endif
          for (int i = 0; i < MAX_EXPORT_RETRY && -ENOMEM != rc; i++)
          {
              if (0 != (rc = pHab_if->pfExport(habmmhandle, virt, V4L2FE_ALIGN(size, 4096), &export_id, export_flag)))
              {
                  HYP_VIDEO_MSG_ERROR("failed to export virt addr %p size %u buf type 0x%x retry %d rc %d",
                                       virt, V4L2FE_ALIGN(size, 4096), (unsigned int)buf_type, i, rc);
              }
              else
              {
                  if (i > 0)
                  {
                     HYP_VIDEO_MSG_ERROR("Retry export success virt addr %p size %u buf type 0x%x export id %u retry %d",
                                          virt, V4L2FE_ALIGN(size, 4096), (unsigned int)buf_type, export_id, i);
                  }
                  break;
              }
          }
          if (0 == export_id)
          {
              HYP_VIDEO_MSG_ERROR("failed to export. export id 0 virt addr %p size %u buf type 0x%x",
                                   virt, size, (unsigned int)buf_type);
          }
          else
          {
              aEntry = ( hypv_map_entry_t* ) malloc(sizeof(hypv_map_entry_t));
              if (aEntry == NULL)
              {
                  HYP_VIDEO_MSG_ERROR( "malloc failed");
                  if (0 != pHab_if->pfUnExport(habmmhandle, export_id, 0))
                  {
                     HYP_VIDEO_MSG_ERROR("failed to unexport id %u", export_id);
                  }
                  export_id = 0;
              }
              else
              {
                  HYP_VIDEO_MSG_INFO("export virt addr/fd %p size %u export id %u buf type 0x%x",
                                      virt, size, export_id, (unsigned int)buf_type);
                  aEntry->va = virt;
                  aEntry->bufferid = export_id;
                  aEntry->next = NULL;
                  aEntry->size = size;
                  aEntry->refcnt = 1;
                  aEntry->habmmhandle = habmmhandle;
                  aEntry->buf_type = buf_type;
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
    }
    else
    {
       export_id = find_entry->bufferid;
       find_entry->refcnt++;
    }

    pthread_mutex_unlock(&mutex);

    return export_id;
}
/**===========================================================================

FUNCTION hypv_map_add_to_remote_ext

@brief

   This function creates a map entry if the entry does not exist

@param [in] hab API interface
@param [in] queue queue handle
@param [in] habmmhandle  the virtual channel(habmm handle) from habmm open
@param [in] index index of the buffer
@param [in] frame_addr  local process virtual address for frame buffer
@param [in] frame_size  size of memory chunk to be mapped for frame buffer
@param [in] extradata_addr  local process virtual address for extradata buffer
@param [in] extradata_size  size of memory chunk to be mapped for extradata buffer
@param [in] buf_type  either input or output

@dependencies
  None

@return
  Returns map entry

===========================================================================*/
hypv_map_entry_ex_t* hypv_map_add_to_remote_ext
(
    habIf*                 pHab_if,
    hypv_lookup_table_ex_t*   queue,
    int32                  habmmhandle,
    uint32                 index,
    void*                  frame_addr,
    uint32                 frame_size,
    void*                  extradata_addr,
    uint32                 extradata_size,
    uint32                 buf_type
)
{
    hypv_map_entry_ex_t*   find_entry = NULL;
    hypv_map_entry_ex_t*   aEntry = NULL;
    int32 rc = 0;
    uint32 frame_export_id = 0;
    uint32 extradata_export_id = 0;

    if ( !pHab_if || !pHab_if->pfExport || !pHab_if->pfUnExport)
    {
        HYP_VIDEO_MSG_ERROR("NULL pHab_if or pfExport, pHab_if = %p", pHab_if);
        return 0;
    }

    pthread_mutex_lock(&mutex);

    find_entry = queue->linkhead;

    while (find_entry)
    {
        if ((find_entry->frame_addr == frame_addr) &&
            (find_entry->is_readonly == TRUE) &&
            (find_entry->buf_type == buf_type))
        {
            break;
        }

        find_entry = find_entry->next;
    }

    if (NULL == find_entry)
    {
        find_entry = queue->linkhead;

        while (find_entry)
        {
            if ((find_entry->index == index) &&
                (find_entry->buf_type == buf_type))
            {
                find_entry->is_stale = TRUE;
                find_entry = NULL;
                break;
            }
            find_entry = find_entry->next;
        }
    }

    if (NULL == find_entry)
    {
        if (frame_size)
        {
            uint32 export_flag = 0;
            export_flag = HABMM_EXPIMP_FLAGS_FD;

            for (int i = 0; i < MAX_EXPORT_RETRY && -ENOMEM != rc; i++)
            {
                if (0 != (rc = pHab_if->pfExport(habmmhandle, frame_addr, V4L2FE_ALIGN(frame_size, 4096), &frame_export_id, export_flag)))
                {
                    HYP_VIDEO_MSG_ERROR("failed to export frame addr %p size %u buf type 0x%x retry %d rc %d",
                                       frame_addr, V4L2FE_ALIGN(frame_size, 4096), buf_type, i, rc);
                }
                else
                {
                    if (i > 0)
                    {
                        HYP_VIDEO_MSG_ERROR("Retry export success frame addr %p size %u buf type 0x%x export id %u retry %d",
                                          frame_addr, V4L2FE_ALIGN(frame_size, 4096), buf_type, frame_export_id, i);
                    }
                    break;
                }
            }
       }

       if ((0 != extradata_addr) && extradata_size)
       {
            uint32 export_flag = 0;
            export_flag = HABMM_EXPIMP_FLAGS_FD;

            for (int i = 0; i < MAX_EXPORT_RETRY && -ENOMEM != rc; i++)
            {
                if (0 != (rc = pHab_if->pfExport(habmmhandle, extradata_addr, V4L2FE_ALIGN(extradata_size, 4096), &extradata_export_id, export_flag)))
                {
                    HYP_VIDEO_MSG_ERROR("failed to export extradata addr %p size %u buf type 0x%x retry %d rc %d",
                                       extradata_addr, V4L2FE_ALIGN(extradata_size, 4096), buf_type, i, rc);
                }
                else
                {
                    if (i > 0)
                    {
                        HYP_VIDEO_MSG_ERROR("Retry export success extradata addr %p size %u buf type 0x%x export id %u retry %d",
                                          extradata_addr, V4L2FE_ALIGN(extradata_size, 4096), buf_type, extradata_export_id, i);
                    }
                    break;
                }
            }
       }

       if ((0 == frame_export_id) ||
          ((0 == extradata_export_id) && (0 != extradata_addr)))
       {
           HYP_VIDEO_MSG_ERROR("failed to export. export id %u frame addr %p size %u  export id %u extaradata addr %p size %u buf type 0x%x",
                               frame_export_id, frame_addr, frame_size, extradata_export_id, extradata_addr, extradata_size, buf_type);
       }
       else
       {
           aEntry = (hypv_map_entry_ex_t*) malloc(sizeof(hypv_map_entry_ex_t));
           if (aEntry == NULL)
           {
               HYP_VIDEO_MSG_ERROR( "malloc failed");
               if (0 != pHab_if->pfUnExport(habmmhandle, frame_export_id, 0))
               {
                   HYP_VIDEO_MSG_ERROR("failed to unexport id %u", frame_export_id);
               }
               frame_export_id = 0;

               if (0 != extradata_export_id)
               {
                   if (0 != pHab_if->pfUnExport(habmmhandle, extradata_export_id, 0))
                   {
                       HYP_VIDEO_MSG_ERROR("failed to unexport id %u", extradata_export_id);
                   }
                   extradata_export_id = 0;
               }
            }
            else
            {
                HYP_VIDEO_MSG_INFO("export frame addr/fd %p size %u export id %u extradata addr %p size %u export id %u buf type 0x%x",
                                  frame_addr, frame_size, frame_export_id, extradata_addr, extradata_size,extradata_export_id, buf_type);
                aEntry->index = index;
                aEntry->frame_addr = frame_addr;
                aEntry->frame_size = frame_size;
                aEntry->extradata_addr = extradata_addr;
                aEntry->extradata_size = extradata_size;
                aEntry->frame_bufferid = frame_export_id;
                aEntry->extradata_bufferid = extradata_export_id;
                aEntry->refcnt = 1;
                aEntry->habmmhandle = habmmhandle;
                aEntry->buf_type = buf_type;
                aEntry->is_export = 1;
                aEntry->is_stale = 0;
                aEntry->is_readonly = 0;
                aEntry->next = NULL;
                if (queue->linktail)
                {
                    queue->linktail->next = aEntry;
                }
                else
                {
                    queue->linkhead = aEntry;
                }
                queue->linktail = aEntry;
                find_entry = aEntry;
            }
        }
    }

    pthread_mutex_unlock(&mutex);

    return find_entry;
}

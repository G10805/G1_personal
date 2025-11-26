/*===========================================================================
 Video hypervisor IO buffer management

 *//** @file hyp_buffer_manager.h
 This file provides data structure and function api to share and map the
 virtual address between local and remote process using habmm APIs

Copyright (c) 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

 *//*========================================================================*/
/*===========================================================================
 Edit History

$Header:$

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/22/18           sm          Initial version of hypervisor DVR
=============================================================================*/
#ifndef __HYP_BUFFER_MANAGER_H__
#define __HYP_BUFFER_MANAGER_H__

#include "hyp_dvrpriv.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct hypv_map_entry_t hypv_map_entry_t;

struct hypv_map_entry_t
{
    void*             va;          /* virtual address in local process */
    uint32            size;        /* map size */
    uint32            refcnt;      /* reference count for each map */
    int32             habmmhandle; /* habmm virtual channel(handle) */
    uint32            bufferid;    /* export id which is unique per buffer */
    int32             is_export;   /* flag to determine export or import */
    hypv_map_entry_t* next;
};

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
    int32                habmmhandle,
    uint32               bufferid
);

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
    int32                habmmhandle,
    void*                va
);

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
    habIf*               pHab_if,
    hypv_lookup_table_t* queue,
    void*                virt
);

/**===========================================================================

FUNCTION hypv_map_cleanup

@brief  free all allocated heap entries and mmap entries.

@param [in] hab API interface
@param [in] queue queue handle

@dependencies
  None

@return
  Returns hdvr_status_type

===========================================================================*/
hdvr_status_type hypv_map_cleanup
(
    habIf*               pHab_if,
    hypv_lookup_table_t* queue
);

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
    habIf*               pHab_if,
    hypv_lookup_table_t* queue,
    int32                habmmhandle, /* the virtual channel(habmm handle) from habmm open */
    uint32               size,        /* size of the memory chunk to be allocated */
    uint32               bufferid     /* the buffer id exported by the remote process */
);

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
    habIf*               phab_if,
    hypv_lookup_table_t* queue,
    int32                habmmhandle, /* the virtual channel(habmm handle) from habmm open */
    void*                virt,        /* local process virtual address */
    uint32               size         /* size of the memory chunk to be allocated */
);

#ifdef __cplusplus
}
#endif

#endif

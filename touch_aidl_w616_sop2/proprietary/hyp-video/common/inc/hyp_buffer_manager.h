/*===========================================================================
 Video hypervisor IO buffer management

 *//** @file hyp_buffer_manager.h
 This file provides data structure and function api to share and map the
 virtual address between local and remote process using habmm APIs

Copyright (c) 2016-2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

 *//*========================================================================*/
/*===========================================================================
 Edit History

$Header:$

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
04/01/21           sh          Bringup video on RedBend Hypervisor
08/27/20           sh          Bringup video decode using codec2
03/06/19           rz          Handle decoder output dynamic map/unmap/export/unexport buffers
10/30/18           sm          Clean buffer mapping based on buffer type
06/23/17           sm          Streamline hypervisor context structure and definitions
05/08/17           sm          Update for new hyp-video architecture
08/16/16           rz          Dynamically loading hab library
07/08/16           hl          Isolate video data from ioctl
07/07/16           hl          Support dynamic buffer unexport and unimport
06/01/16           hl          Add FE and BE to support Hypervisor interface

=============================================================================*/
#ifndef __HYP_BUFFER_MANAGER_H__
#define __HYP_BUFFER_MANAGER_H__

#include "hyp_vidc_types.h"
#include "hyp_videopriv.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct hypv_map_entry_t hypv_map_entry_t;
typedef struct hypv_map_entry_ex_t hypv_map_entry_ex_t;

struct hypv_map_entry_ex_t
{
    uint32 index;              // index of the buffer
    void* frame_addr;          // frame buffer virtual address in local process
    uint32 frame_size;         // frame buffer map size
    void* extradata_addr;      // extradata buffer virtual address in local process
    uint32 extradata_size;     // extradata buffer map size
    uint32 frame_bufferid;     // frame buffer export id
    uint32 extradata_bufferid; // extradata buffer export id
    uint32 refcnt;             // reference count for each map
    int32 habmmhandle;         // habmm virtual channel(handle)
    uint32 buf_type;           // either input or output
    int32 is_export;           // flag to determine export or import
    int32 is_stale;            // flag to determine if the buffer is stale
    int32 is_readonly;         // flag to determine if the buffer is readonly
    hypv_map_entry_ex_t* next;
};
struct hypv_map_entry_t
{
    void* va;          // virtual address in local process
    uint32 size;       // map size
    uint32 refcnt;     // reference count for each map
    int32 habmmhandle; //habmm virtual channel(handle)
    uint32 bufferid;   //export id which is unique per buffer
    vidc_buffer_type buf_type; //either input or output
    int32 is_export;  //flag to determine export or import
    hypv_map_entry_t* next;
};

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
);

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
  Returns hypv_status_type

===========================================================================*/
hypv_status_type hypv_map_free
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    void* virt,
    uint32 bufferid
);

/**===========================================================================

FUNCTION hypv_map_cleanup

@brief  free all allocated heap entries and mmap entries.

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
);

/**===========================================================================

FUNCTION hypv_map_cleanup_buf_type

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
hypv_status_type hypv_map_cleanup_buf_type
(
    habIf*                pHab_if,
    hypv_lookup_table_t* queue,
    vidc_buffer_type buf_type
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
@param [in] buf_type  either input or output

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
    uint32 bufferid,    /* the buffer id exported by the remote process */
    vidc_buffer_type buf_type /* either input or otuput buffer */
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
@param [in] buf_type  either input or output

@dependencies
  None

@return
  Returns export id into the remote process

===========================================================================*/
uint32 hypv_map_add_to_remote
(
    habIf*                phab_if,
    hypv_lookup_table_t* queue,
    int32 habmmhandle,  /* the virtual channel(habmm handle) from habmm open */
    void* virt,         /* local process virtual address */
    uint32 size,        /* size of the memory chunk to be allocated */
    vidc_buffer_type buf_type, /* either input or otuput buffer */
    boolean export_as_fd      /* flag to indicate buffer to export as fd or va*/
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);
#ifdef __cplusplus
}
#endif

#endif

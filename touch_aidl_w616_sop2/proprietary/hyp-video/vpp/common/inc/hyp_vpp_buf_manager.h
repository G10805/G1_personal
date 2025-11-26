/*===========================================================================
VPP hypervisor IO buffer management

*//** @file hyp_vpp_buf_manager.h
This file provides data structure and function api to share and map the
file description between local and remote process using habmm APIs

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

#ifndef __HYP_VPP_BUFFER_MANAGER_H__
#define __HYP_VPP_BUFFER_MANAGER_H__

#include "hyp_vpppriv.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct hypvpp_map_entry_t hypvpp_map_entry_t;

struct hypvpp_map_entry_t
{
    int32 fd;                     /* file description */
    uint32 size;                  /* map size */
    uint32 refcnt;                /* reference count for each map */
    int32 habmmhandle;            /* habmm virtual channel(handle) */
    uint32 share_id;              /* share id (export id/import id) which is unique per buffer */
    hypvpp_buffer_type buf_type;  /* either input or output */
    int32 is_export;              /* flag to determine export or import */
    hypvpp_map_entry_t* next;
};

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
);

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
);

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
);

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
);

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
);

/**===========================================================================

  FUNCTION hypvpp_map_cleanup

  @brief  free all allocated heap entries and mmap entries.

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
);

/**===========================================================================

  FUNCTION hypvpp_map_cleanup_buf_type

  @brief  free all allocated heap entries and mmap entries
  of a buffer type.

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
);

/**===========================================================================

  FUNCTION hypvpp_map_add_from_remote

  @brief

  This function creates a map entry if the entry does not exist or add a reference
  count for existing map entry

  @param [in] hab API interface
  @param [in] queue handle
  @param [in] habmmhandle   the virtual channel(habmm handle) from habmm open
  @param [in] size  size of memory chunk to be mapped
  @param [in] share_id  id shared by the remote process
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
    int32 habmmhandle,            /* the virtual channel(habmm handle) from habmm open */
    uint32 size,                  /* size of the memory chunk to be allocated */
    uint32 share_id,              /* the share id exported by the remote process */
    hypvpp_buffer_type buf_type   /* either input or output buffer */
);

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
    habIf* phab_if,
    hypvpp_lookup_table_t* queue,
    int32 habmmhandle,             /* the virtual channel(habmm handle) from habmm open */
    int32 fd,                      /* file description to share */
    uint32 size,                   /* size of the memory chunk to be allocated */
    hypvpp_buffer_type buf_type    /* either input or otuput buffer */
);

#ifdef __cplusplus
}
#endif

#endif

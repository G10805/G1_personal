/**
 * \file apm_memmap_api.h
 * \brief 
 *  	 This file contains the structure definitions for the memory map commands
 * 
 * \copyright
 *  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/apm/apm_memmap_api.h#9 $
*/
// clang-format on

#ifndef _APM_MEMMAP_API_H_
#define _APM_MEMMAP_API_H_

#include "ar_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/** @addtogroup apm_common
@{ */

/** @ingroup apm_cmd_mem_map
    Commands the APM to map multiple shared memory regions.
    All the mapped regions must be from the same memory pool.

  @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_MEM_MAP_REGIONS
    @par

  @apr_msgpayload{agn_cmd_shared_mem_map_regions_t}
    @tablens{weak__apm__cmd__shared__mem__map__regions__t}

  @mappayload{apm_shared_map_region_payload_t}
    @table{weak__apm__shared__map__region__payload__t}

  @return
    #APM_CMD_RSP_SHARED_MEM_MAP_REGIONS.

  @dependencies
    None. @newpage
*/
#define APM_CMD_SHARED_MEM_MAP_REGIONS           0x0100100C

/** Enumeration for the shared memory 8_4K pool ID. */
#define APM_MEMORY_MAP_SHMEM8_4K_POOL            3

/** @addtogroup apm_common
@{ */

/** @ingroup apm_cmd_mem_map
    Commands the APM to map multiple MDF shared memory regions.
    All the mapped regions must be from the same memory pool.

  @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_SATELLITE_MEM_MAP_REGIONS
    @par

  @apr_msgpayload{apm_cmd_shared_satellite_mem_map_regions_t}
    @tablens{weak__apm__cmd__shared__satellite__mem__map__regions__t}

  @mappayload{apm_shared_map_region_payload_t}
    @table{weak__apm__shared__map__region__payload__t}

  @return
    #APM_CMD_RSP_SHARED_SATELLITE_MEM_MAP_REGIONS.

  @dependencies
    None. @newpage
*/
#define APM_CMD_SHARED_SATELLITE_MEM_MAP_REGIONS      0x01001026

/** @} */ /* end_addtogroup apm_common */

/** @addtogroup apm_cmd_mem_map
@{ */

/** Definition of the is_virtual flag shift value. */
#define APM_MEMORY_MAP_SHIFT_IS_VIRTUAL           0

/** Definition of the is_virtual flag bitmask. */
#define APM_MEMORY_MAP_BIT_MASK_IS_VIRTUAL        (0x00000001UL)

/** Definition of the physical memory flag. */
#define APM_MEMORY_MAP_PHYSICAL_MEMORY            0

/** Definition of the virtual memory flag. */
#define APM_MEMORY_MAP_VIRTUAL_MEMORY             1

/** Definition of the is_uncached flag shift value. */
#define APM_MEMORY_MAP_SHIFT_IS_UNCACHED           1

/** Definition of the is_uncached flag bitmask. */
#define APM_MEMORY_MAP_BIT_MASK_IS_UNCACHED        (0x00000002UL)

/** Definition of the is-offset-mode flag shift value. */
#define APM_MEMORY_MAP_SHIFT_IS_OFFSET_MODE           2

/** Definition of the is-offset-mode flag bitmask. */
#define APM_MEMORY_MAP_BIT_MASK_IS_OFFSET_MODE        (0x00000004UL)

/** Definition of the is_mem_loaned flag shift value. */
#define APM_MEMORY_MAP_SHIFT_IS_MEM_LOANED         3

/** Definition of the is_mem_loaned flag bitmask. */
#define APM_MEMORY_MAP_BIT_MASK_IS_MEM_LOANED      (0x00000008UL)
/** @} */ /* end_addtogroup apm_cmd_mem_map */

#include "spf_begin_pack.h"

/** @weakgroup weak_apm_cmd_shared_mem_map_regions_t
@{ */
/** Immediately following this structure are num_regions of the
    <b>Map payload</b> (%apm_shared_map_region_payload_t).
*/
struct apm_cmd_shared_mem_map_regions_t
{
   uint16_t mem_pool_id;
   /**< Type of memory on which this memory region is mapped.

        @values
        - #APM_MEMORY_MAP_SHMEM8_4K_POOL
        - Other values are reserved

        The memory ID implicitly defines the characteristics of the memory.
        Characteristics can include alignment type, permissions, etc.

        APM_MEMORY_MAP_SHMEM8_4K_POOL is shared memory, byte addressable, and
        4 KB aligned. 
        
        */

   uint16_t num_regions;
   /**< Number of regions to map.

        @values Any value greater than zero

        if is_offset_mode bit in the property_flag set, then num_regions can only be 1.*/

   uint32_t property_flag;
   /**< Configures one common property for all regions in the payload. No
        two regions in the same command can have different properties.

        @values 0x00000000 to 0x00000007

        Bit 0 is the IsVirtual flag that indicates physical or virtual
        mapping:
          - 0 -- The shared memory address provided in
                 apm_shared_map_region_payload_t is a physical address. The
                 shared memory must be mapped (a hardware TLB entry), and a
                 software entry must be added for internal bookkeeping.

          - 1 -- The shared memory address provided in the map
                 payload[usRegions] is a virtual address. The shared memory
                 must not be mapped (because the hardware TLB entry is already
                 available), but a software entry must be added for internal
                 bookkeeping. \n
                 This bit value (1) is useful if two services within the aDSP
                 are communicating via the APR. They can directly communicate
                 via the virtual address instead of the physical address. The
                 virtual regions must be contiguous.

        Bit 1 - is_uncached - indicates whether the memory must be mapped as cached memory or uncached.
              Note: most memories must be mapped as cached. Some special cases need uncached memory.
           - 0  - cached.
           - 1  - uncached.

        Bit 2 - is_offset_mode -  indicates whether the memory will be subsequently referred to with offsets.
        After mapping a physical region using this command, a mem-map-handle is returned.
        Subsequently the client uses mem-map-handle in any command that uses addresses/offsets
        belonging to memory region mapped in this command.
           - 0 - subsequent commands using mem-map-handle returned by APM_CMD_RSP_SHARED_MEM_MAP_REGIONS,
                 will use physical or virtual addresses
                 (E.g. buf_addr_lsw & buf_addr_msw in data_cmd_wr_sh_mem_ep_data_buffer_t will be phy or virt addresses).
           - 1 - subsequent commands using mem-map-handle returned by APM_CMD_RSP_SHARED_MEM_MAP_REGIONS,
                 will use offsets. (E.g. buf_addr_lsw & buf_addr_msw in data_cmd_wr_sh_mem_ep_data_buffer_t will be offsets).
                 The offset is from the base address.
                 Base address is defined as the address given by the apm_shared_map_region_payload_t.
        Bit 3 - is_client_loaned_memory – specifies if the client manages the memory or allows the APM to manage the memory.
        The Client owned memory, the shared memory is managed by the client. 
        The Client loaned memory, the shared memory is mapped by the client and managed by APM.  
            0 – Client owned memory 
            1 – Client Loaned memory
                Incase of loaned memory, client would need to relinquish the memory access permissions for MDF usecases.

        Bits 31 to 4 are reserved and must be set to 0. */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_shared_mem_map_regions_t apm_cmd_shared_mem_map_regions_t;
#include "spf_begin_pack.h"
/** Immediately following this structure are num_regions of the
    <b>Map payload</b> (%apm_shared_map_region_payload_t).
*/
struct apm_cmd_shared_satellite_mem_map_regions_t
{
  uint32_t master_mem_handle;
   /**<
      Specifies the memory handle of the master processor corresponding to 
      the shared memory reqion being mapped to the satellite process domain.
      This would've been sent in the response after mapping the memory to the 
      master processor.
   */
   uint32_t satellite_proc_domain_id;
   /**< 
        Specifies the satellite process domain ID to which the memory needs to be mapped.
        @values
        - #APM_PROC_DOMAIN_ID_MDSP
        - #APM_PROC_DOMAIN_ID_SDSP
        - #APM_PROC_DOMAIN_ID_CDSP
   */
   uint16_t mem_pool_id;
   /**< Type of memory on which this memory region is mapped.

        @values
        - #APM_MEMORY_MAP_SHMEM8_4K_POOL
        - Other values are reserved

        The memory ID implicitly defines the characteristics of the memory.
        Characteristics can include alignment type, permissions, etc.

        APM_MEMORY_MAP_SHMEM8_4K_POOL is shared memory, byte addressable, and
        4 KB aligned. 

        */   
   uint16_t num_regions;
   /**< Number of regions to map.

        @values 1, Since only offset mode is supported.

        if is_offset_mode bit in the property_flag set, then num_regions can only be 1.*/

   uint32_t property_flag;
   /**< Configures one common property for all regions in the payload. No
        two regions in the same command can have different properties.

        @values 0x00000000 to 0x00000007

        Bit 0 is the IsVirtual flag that indicates physical or virtual
        mapping:
          - 0 -- The shared memory address provided in
                 apm_shared_map_region_payload_t is a physical address. The
                 shared memory must be mapped (a hardware TLB entry), and a
                 software entry must be added for internal bookkeeping.

          - 1 -- The shared memory address provided in the map
                 payload[usRegions] is a virtual address. The shared memory
                 must not be mapped (because the hardware TLB entry is already
                 available), but a software entry must be added for internal
                 bookkeeping. \n
                 This bit value (1) is useful if two services within the aDSP
                 are communicating via the GPR. They can directly communicate
                 via the virtual address instead of the physical address. The
                 virtual regions must be contiguous.

        Bit 1 - is_uncached - indicates whether the memory must be mapped as cached memory or uncached.
              Note: most memories must be mapped as cached. Some special cases need uncached memory.
           - 0  - cached.
           - 1  - uncached.

        Bit 2 - is_offset_mode -  indicates whether the memory will be subsequently referred to with offsets.
        After mapping a physical region using this command, a mem-map-handle is returned.
        Subsequently the client uses mem-map-handle in any command that uses addresses/offsets
        belonging to memory region mapped in this command.
           - 1 - subsequent commands using mem-map-handle returned by APM_CMD_RSP_SHARED_MEM_MAP_REGIONS,
                 will use offsets. (E.g. buf_addr_lsw & buf_addr_msw in data_cmd_wr_sh_mem_ep_data_buffer_t will be offsets).
                 The offset is from the base address.
                 Base address is defined as the address given by the apm_shared_map_region_payload_t.
        
        Bit 3 - is_client_loaned_memory – specifies if the client manages the memory or allows the APM to manage the memory.
        The Client owned memory, the shared memory is managed by the client. 
        The Client loaned memory, the shared memory is mapped by the client and managed by APM.  
            0 – Client owned memory 
            1 – Client Loaned memory
                Incase of loaned memory, client would need to relinquish the memory access permissions for MDF usecases.
        Bits 31 to 4 are reserved and must be set to 0. */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_shared_satellite_mem_map_regions_t apm_cmd_shared_satellite_mem_map_regions_t;

/** @} */ /* end_weakgroup weak_apm_cmd_shared_mem_map_regions_t */

#include "spf_begin_pack.h"

/** @weakgroup weak_apm_shared_map_region_payload_t
@{ */
/** Payload of the map region: used by the
    apm_cmd_shared_mem_map_regions_t structure.
*/
struct apm_shared_map_region_payload_t
{
   uint32_t shm_addr_lsw;
   /**< Lower 32 bits of the shared memory address of the memory region to
        map. This word must be contiguous memory, and it must be
        4&nbsp;KB aligned.

        @values Non-zero 32-bit value */

   uint32_t shm_addr_msw;
   /**< Upper 32 bits of the shared memory address of the memory region to
        map. This word must be contiguous memory, and it must be
        4&nbsp;KB aligned.

        @values
        - For a 32-bit shared memory address, this field must be set to 0.
        - For a 36-bit address, bits 31 to 4 must be set to
          0.
        - For a 64-bit shared memory address, non-zero 32-bit value.
        @tablebulletend */

   uint32_t mem_size_bytes;
   /**< Number of bytes in the region.

        @values Multiples of 4 KB

        APM always maps the regions as virtual contiguous memory, but
        the memory size must be in multiples of 4 KB to avoid gaps in the
        virtually contiguous mapped memory. */
}
#include "spf_end_pack.h"
;

typedef struct apm_shared_map_region_payload_t apm_shared_map_region_payload_t;

/** @} */ /* end_weakgroup weak_apm_shared_map_region_payload_t */

/** @weakgroup weak_apm_cmd_rsp_shared_mem_map_regions_t
@{ */

/** @ingroup apm_resp_mem_map
    Returns the memory map handle in response to an
    #APM_CMD_SHARED_MEM_MAP_REGIONS command.

   @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_MEM_UNMAP_REGIONS
    @par

  @apr_msgpayload{apm_cmd_rsp_shared_mem_map_regions_t}
    @table{weak__apm__cmdrsp__shared__mem__map__regions__t}

  @return
    None.

  @dependencies
    An #APM_CMD_SHARED_MEM_MAP_REGIONS command must have been
    issued.
*/
#define APM_CMD_RSP_SHARED_MEM_MAP_REGIONS         0x02001001

#include "spf_begin_pack.h"

struct apm_cmd_rsp_shared_mem_map_regions_t
{
   uint32_t mem_map_handle;
   /**< Unique identifier for the shared memory address.

        The aDSP returns this handle through
        #APM_CMD_SHARED_MEM_MAP_REGIONS

        @values Any 32-bit value

        The APM uses this handle to retrieve the shared memory attributes.
        This handle can be an abstract representation of the shared memory
        regions that are being mapped. */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_rsp_shared_mem_map_regions_t apm_cmd_rsp_shared_mem_map_regions_t;


/** @ingroup apm_resp_mem_map
    Returns the memory map handle in response to an
    #APM_CMD_SHARED_SATELLITE_MEM_MAP_REGIONS command.

   @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_MEM_UNMAP_REGIONS
    @par

  @apr_msgpayload{apm_cmd_rsp_shared_satellite_mem_map_regions_t}
    @table{weak__apm__cmdrsp__shared__mem__map__regions__t}

  @return
    None.

  @dependencies
    An #APM_CMD_SHARED_SATELLITE_MEM_MAP_REGIONS command must have been
    issued.
*/
#define APM_CMD_RSP_SHARED_SATELLITE_MEM_MAP_REGIONS    0x02001009

#include "spf_begin_pack.h"

struct apm_cmd_rsp_shared_satellite_mem_map_regions_t   
{
   uint32_t mem_map_handle;
   /**< Unique identifier for the shared memory address.
     The satellite process domain uses this handle to retrieve the
     shared memory attributes. This handle can be an abstract
     representation of the shared memory regions that are being mapped.
 */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_rsp_shared_satellite_mem_map_regions_t apm_cmd_rsp_shared_satellite_mem_map_regions_t;

/** @} */ /* end_weakgroup weak_apm_cmd_rsp_shared_mem_map_regions_t */

/** @ingroup apm_cmd_mem_unmap
    Commands the APM to unmap multiple shared memory regions
    that were previously mapped via
    #APM_CMD_SHARED_MEM_MAP_REGIONS.

  @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_MEM_UNMAP_REGIONS
    @par

  @apr_msgpayload{apm_cmd_shared_mem_unmap_regions_t}
    @table{weak__apm__cmd__shared__mem__unmap__regions__t}

  @return
    GPR_IBASIC_RSP_RESULT.

  @dependencies
    A corresponding #APM_CMD_SHARED_MEM_MAP_REGIONS command must have been
    issued.
*/
#define APM_CMD_SHARED_MEM_UNMAP_REGIONS          0x0100100D

#include "spf_begin_pack.h"

struct apm_cmd_shared_mem_unmap_regions_t
{
   uint32_t mem_map_handle;
   /**< Unique identifier for the shared memory address.

        The APM returns this handle through
        #APM_CMD_SHARED_MEM_MAP_REGIONS

        @values Any 32-bit value

        The APM uses this handle to retrieve the associated shared
        memory regions and unmap them. */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_shared_mem_unmap_regions_t apm_cmd_shared_mem_unmap_regions_t;

/** @ingroup apm_cmd_mem_unmap
    Commands the APM to unmap multiple shared memory regions
    that were previously mapped via
    #APM_CMD_SHARED_MEM_MAP_REGIONS.

  @apr_hdr_fields
    Opcode -- APM_CMD_SHARED_SATELLITE_MEM_UNMAP_REGIONS
    @par

  @apr_msgpayload{apm_cmd_shared_mem_unmap_regions_t}
    @table{weak__apm__cmd__shared__mem__unmap__regions__t}

  @return
    GPR_IBASIC_RSP_RESULT.

  @dependencies
    A corresponding #APM_CMD_SHARED_MEM_MAP_REGIONS command must have been
    issued.
*/
#define APM_CMD_SHARED_SATELLITE_MEM_UNMAP_REGIONS          0x01001027

#include "spf_begin_pack.h"

struct apm_cmd_shared_satellite_mem_unmap_regions_t
{
   uint32_t satellite_proc_domain_id;
   /**< 
        Specifies the satellite process domain ID whose memory needs to be unmapped.
        @values
        - #APM_PROC_DOMAIN_ID_MDSP
        - #APM_PROC_DOMAIN_ID_SDSP
        - #APM_PROC_DOMAIN_ID_CDSP
   */
   uint32_t master_mem_handle;
   /**< Unique identifier for the shared memory address.  The satellite process domain 
        uses this handle to retrieve the associated shared memory regions and unmap them.

        The APM returns this handle through
        #APM_CMD_SHARED_SATELLITE_MEM_MAP_REGIONS

        @values Any 32-bit value
   */
}
#include "spf_end_pack.h"
;

typedef struct apm_cmd_shared_satellite_mem_unmap_regions_t apm_cmd_shared_satellite_mem_unmap_regions_t;


/** @} */ /* end_weakgroup weak_apm_cmd_shared_mem_unmap_regions_t */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _APM_MEMMAP_API_H_ */

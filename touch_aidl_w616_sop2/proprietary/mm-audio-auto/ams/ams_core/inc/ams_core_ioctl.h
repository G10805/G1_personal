/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#include <stdint.h>
/** Tunnel command send diretcly to AMS FW. */
#define AMS_CORE_CMD_TUNNEL 0x00015000
typedef struct
{
    uint32_t processor_id; // [in] 0 for MDSP
    uint32_t opcode;       // [in]
    uint32_t payload_size; // [in]
    uint32_t response_size;
    uint8_t payload[0];
} ams_core_tunnel_req_t;
typedef struct
{
    uint32_t response_code;
    uint32_t response_size;
    uint8_t response[0];
} ams_core_tunnel_rsp_t;
typedef struct
{
    ams_core_tunnel_req_t *req;
    ams_core_tunnel_rsp_t *rsp; // [out]
} ams_core_tunnel_t;

/** Enable endpoint of the graph. */
#define AMS_CORE_CMD_ENABLE_ENDPOINT 0x00015001

/** Disable endpoint of the graph. */
#define AMS_CORE_CMD_DISABLE_ENDPOINT 0x00015002

/** Set clk attribute. */
#define AMS_CORE_CMD_SET_CLK_ATTR 0x00015003

typedef enum
{
    AMS_CORE_HW_INTERFACE_TDM1 = 1,
    AMS_CORE_HW_INTERFACE_TDM2,
    AMS_CORE_HW_INTERFACE_TDM3,
    AMS_CORE_HW_INTERFACE_TDM4,
    AMS_CORE_HW_INTERFACE_TDM5,
} ams_core_hw_interface_id_t;
/**
  @brief Payload for the #AMS_CORE_CMD_ENABLE_ENDPOINT and #AMS_CORE_CMD_DISABLE_ENDPOINT ioctl
*/
typedef struct
{
    uint32_t flags; // [in] Optional flag: AMS_CORE_ENDPOINT_FLAG_SHARED
    bool source;    // true: tx, false: rx
    struct
    {
        ams_core_hw_interface_id_t hw_interface_id;
        uint32_t num_channels;
        uint32_t sample_rate;
        uint32_t bit_width;
        uint16_t data_format;
        uint16_t sync_mode;
        uint16_t sync_src;
        uint16_t nslots_per_frame;
        uint16_t ctrl_data_out_enable;
        uint16_t ctrl_invert_sync_pulse;
        uint16_t ctrl_sync_data_delay;
        uint16_t slot_width;
        uint32_t slot_mask;
    } tdm_params; // [in]
} ams_core_endpoint_t;

typedef struct
{
    uint16_t ep_id;
    uint16_t attr;
} ams_core_clk_attr_t;

/** Get cached graph info. */
#define AMS_CORE_CMD_GET_CACHED_GRAPH_INFO 0x00015004
// payload for get cached graph
typedef struct
{
    uint32_t idx;
    uint32_t state;
    uint32_t fw_handle;
    uint32_t gr_descr_sz;
} ams_core_cached_graph_info_t;

#define AMS_CORE_CMD_GET_CACHED_GRAPH_NUM 0x00015005
// payload
typedef struct
{
    uint32_t num;
} ams_core_cached_graph_num_t;

#define AMS_CORE_CMD_ALLOC_SHMEM 0x00015007

// payload
typedef struct
{
    uint32_t pid;
    uint32_t client_id; // idx in shmem array info on server
    uint32_t len;
} ams_core_shmem_alloc_req_t;

typedef struct
{
    uint32_t va_lsw; // virtual addr low
    uint32_t va_msw; // virtual addr high
    uint32_t mem_map_handle;
    uint32_t pa_lsw; // physical addr low
    uint32_t pa_msw; // physical addr high
    int64_t heap_fd;
} ams_core_shmem_alloc_rsp_t;

typedef struct
{
    ams_core_shmem_alloc_req_t req;
    ams_core_shmem_alloc_rsp_t resp;
} ams_core_shmem_alloc_t;

#define AMS_CORE_CMD_FREE_SHMEM 0x00015008
typedef struct
{
    uint32_t client_id; // idx in shmem array info on server
    uint32_t pid;
    uint32_t va_lsw; // virtual addr low
    uint32_t va_msw; // virtual addr high
    uint32_t mem_map_handle;
    uint32_t pa_lsw; // physical addr low
    uint32_t pa_msw; // physical addr high
} ams_core_shmem_free_t;

/** Get cached graph description. */
#define AMS_CORE_CMD_GET_CACHED_GRAPH_DESCR 0x0001500C
// payload for get cached graph
typedef struct
{
    uint32_t idx;
    uint32_t gr_descr_sz;
    uint8_t gr_descr[0];
} ams_core_cached_graph_descr_t;

#define AMS_CORE_CMD_OPERATE_DEVICE_QUERY 0x0001500D
// payload
typedef struct
{
    uint32_t device_id;
    uint32_t state;
} ams_core_operate_device_query_t;
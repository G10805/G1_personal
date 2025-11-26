#ifndef AMS_IMPL_H
#define AMS_IMPL_H
/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include "ams_osal_mutex.h"
#include "ams.h"

struct ams_endpoint_el
{
    ams_endpoint_t val;
    struct ams_endpoint_el *nxt_el;
    uint32_t used_as_sink_cnt;
    uint32_t used_as_source_cnt;
    uint32_t visit_cnt;
};

struct ams_endpoint_list
{
    uint32_t el_num;
    struct ams_endpoint_el *first;
    struct ams_endpoint_el *last;
};

struct ams_module_el
{
    ams_module_t val;
    struct ams_module_el *nxt_el;
    uint32_t used_as_sink_cnt;
    uint32_t used_as_source_cnt;
    uint32_t visit_cnt;
};

struct ams_module_list
{
    uint32_t el_num;
    struct ams_module_el *first;
    struct ams_module_el *last;
};

struct ams_connection_el
{
    ams_connection_t val;
    struct ams_connection_el *nxt_el;
    uint32_t visited;
};

struct ams_connection_list
{
    uint32_t el_num;
    struct ams_connection_el *first;
    struct ams_connection_el *last;
};

struct ams_graph_path_el
{
    uint32_t type; // endpoint=0, module=1
    uint32_t id;   // unique id of the el
    struct ams_graph_path_el *nxt_el;
};

struct ams_graph_path
{
    uint32_t el_num;
    struct ams_graph_path_el *first;
    struct ams_graph_path_el *last;
};

struct ams_graph_property_el
{
    ams_graph_property_t val;
    struct ams_graph_property_el *nxt_el;
};

struct ams_graph_property_list
{
    uint32_t el_num;
    struct ams_graph_property_el *first;
    struct ams_graph_property_el *last;
};

struct ams_graph
{
    // handle returned on create graph
    ams_graph_handle_t handle;
    // graph status info
    uint32_t fw_handle;
    uint32_t fw_status;
    // some basic settings
    ams_graph_basic_params_t base_param;
    // graph description
    struct ams_endpoint_list sinks;
    struct ams_endpoint_list sources;
    struct ams_module_list modules;
    struct ams_connection_list connections;
    struct ams_graph_property_list props;
    //
    struct ams_graph *nxt_el;
    struct ams_graph *prev_el;
    ams_osal_mutex_t rwlock;
};

struct ams_graph_list
{
    uint32_t el_num; // actual num of graphs
    struct ams_graph *first;
    struct ams_graph *last;
    ams_osal_mutex_t rwlock;
};

struct ams_cb
{
    ams_event_handler_t func;
    void *data;
    struct ams_cb *nxt_el;
    struct ams_cb *prev_el;
};

struct ams_cb_list
{
    uint32_t el_num;
    struct ams_cb *first;
    struct ams_cb *last;
    ams_osal_mutex_t rwlock;
};

struct ams_mem_block
{
    void *mem_block_addr;
    uint32_t block_size;
    uint32_t inuse;
    struct ams_mem_block *nxt_el;
};

struct ams_mem_list
{
    uint32_t el_num;
    struct ams_mem_block *first;
    struct ams_mem_block *last;
    ams_osal_mutex_t rwlock;
};

struct shared_mem
{
    uint32_t len;
    uint32_t address_lsw; // virtual addr low
    uint32_t address_msw; // virtual addr high
    uint32_t mem_map_handle;
    uint32_t pa_lsw; // physical address low
    uint32_t pa_msw; // physical address high
    uint64_t ion_mem_fd;
    struct ams_mem_list mem_list;
    ams_osal_mutex_t rwlock;
};

struct ssr_drv
{
    void *hssr;
    uint32_t ssr_adsp_st;
    uint32_t ssr_mdsp_st;
    ams_osal_mutex_t rwlock;
};

struct platform_drv
{
    struct ssr_drv ssr;
    uint32_t h;
    uint32_t version;
    struct shared_mem smem;
};

struct ams_session
{
    struct ams_graph_list gr_lst;
    struct ams_cb_list cb_lst;
    struct platform_drv drv;
};

#endif

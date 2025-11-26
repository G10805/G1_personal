/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "ams_util.h"
#include "ams_platform.h"

static struct ams_connection_el *ams_util_get_next_connection_by_source_id(
    struct ams_graph *gr,
    struct ams_connection_el *ci,
    uint32_t id)
{
    uint32_t found = 0;

    if (ci == NULL)
        ci = gr->connections.first;
    while (ci != NULL)
    {
        if (((ci->val.source.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE && ci->val.source.module.id == id) ||
             (ci->val.source.type == AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT && ci->val.source.endpoint.id == id)) &&
            ci->visited == 0)
        {
            AMS_LIB_LOGD("found next connection with source id =%d", id);
            found = 1;
            break;
        }
        ci = ci->nxt_el;
    }
    if (!found)
        ci = NULL;

    return ci;
}

static ams_status_t ams_util_graph_check_path(
    struct ams_graph *gr,
    struct ams_graph_path *pp_l) // nodes with no incoming edges)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    struct ams_connection_el *ci = NULL;
    struct ams_endpoint_el *e;
    struct ams_module_el *m;
    struct ams_graph_path_el *pi = NULL;
    uint32_t idi = 0, idj = 0;
    pi = pp_l->first;
    while (pi != NULL)
    {
        // get connection list with source id = id
        idi = pi->id;
        pp_l->first = pi->nxt_el;
        free(pi);
        pp_l->el_num--;
        ci = NULL;
        do
        {
            ci = ams_util_get_next_connection_by_source_id(gr, ci, idi);
            if (ci != NULL)
            {
                ci->visited = 1;
                if (ci->val.destination.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE)
                {
                    // new id
                    idj = ci->val.destination.module.id;
                    m = ams_util_graph_get_module_by_id(gr, idj);
                    // ci->dest = (void *)m;
                    if (m != NULL)
                    {
                        m->visit_cnt++;
                        if (m->visit_cnt == m->used_as_sink_cnt)
                        {
                            // add new node to the path
                            pi = calloc(1, sizeof(struct ams_graph_path_el));
                            if (pi == NULL)
                            {
                                r = AMS_STATUS_GENERAL_ERROR;
                                goto exit;
                            }
                            pi->type = 1;
                            pi->id = m->val.id;
                            pi->nxt_el = NULL;
                            ADD_NEW_LIST_EL(pp_l, pi);
                            AMS_LIB_LOGD("New module %d added to the path", pi->id)
                        }
                    }
                    else
                    {
                        AMS_LIB_LOGD("Dest module with id =%d not exists!", idj);
                        r = AMS_STATUS_GRAPH_TOPO_ERROR;
                        goto exit;
                    }
                }
                else
                {
                    // new id
                    idj = ci->val.destination.endpoint.id;
                    e = ams_util_graph_get_endpoint_by_id(gr, idj);
                    if (e != NULL)
                    {
                        e->visit_cnt++;
                        // TODO: if endpoint can only be used either as sink or source stop when endpoint is encountered
                        if (e->visit_cnt == e->used_as_sink_cnt)
                        {
                            // add new node to the path
                            pi = calloc(1, sizeof(struct ams_graph_path_el));
                            if (pi == NULL)
                            {
                                r = AMS_STATUS_GENERAL_ERROR;
                                goto exit;
                            }
                            pi->type = 0;
                            pi->id = e->val.id;
                            pi->nxt_el = NULL;
                            ADD_NEW_LIST_EL(pp_l, pi);
                            AMS_LIB_LOGD("New endpoint %d added to the path", pi->id);
                        }
                    }
                    else
                    {
                        AMS_LIB_LOGD("Dest endpoint with id =%d not exists!", idj);
                        r = AMS_STATUS_GRAPH_TOPO_ERROR;
                        goto exit;
                    }
                }
            }
        } while (ci != NULL);
        pi = pp_l->first;
    }
exit:
    REMOVE_LIST_EL_ALL(pi, pp_l);
    return r;
}

static ams_status_t ams_util_check_unvisited_connections(
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_connection_el *ci = NULL;
    // check if there are unmarked connections
    ci = gr->connections.first;
    while (ci != NULL)
    {
        if (ci->visited == 0)
        {
            r = AMS_STATUS_GRAPH_TOPO_ERROR;
            AMS_LIB_LOGE("Topology error!Connection source=%d, sink=%d not visited!", ci->val.source.module.id, ci->val.destination.module.id);
            break;
        }
        ci = ci->nxt_el;
    }

    return r;
}

ams_status_t ams_util_graph_list_add_new_graph(
    ams_session_t amss,
    struct ams_graph **gr)
{
    ams_status_t r = 0;
    struct ams_graph *p_gr = NULL;

    if (amss == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    struct ams_session *p_ses = amss;
    struct ams_graph_list *p_gr_lst = &p_ses->gr_lst;

    // create new graph;
    p_gr = calloc(1, sizeof(struct ams_graph));
    if (p_gr == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    // make unique handle out of struct address
    p_gr->handle = (uint32_t *)p_gr;

    ADD_NEW_LIST2_EL(p_gr_lst, p_gr);
    *gr = p_gr;

exit:
    return r;
}

ams_status_t ams_util_graph_list_remove_graph(
    struct ams_graph_list *p_gr_lst,
    struct ams_graph *gr)
{
    ams_status_t r = 0;
    struct ams_graph *grtmp = NULL;

    if (p_gr_lst == NULL || gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    ams_util_graph_list_remove_graph_elements(p_gr_lst, gr);
    // destroy lock
    if (ams_osal_mutex_destroy(gr->rwlock))
    {
        AMS_LIB_LOGE("Unable to destroy rwlock!");
    }
    if (p_gr_lst != NULL)
    {
        REMOVE_LIST2_EL(p_gr_lst, gr, grtmp);
    }

exit:
    return r;
}

ams_status_t ams_util_graph_list_remove_graph_elements(
    struct ams_graph_list *p_gr_lst,
    struct ams_graph *gr)
{
    ams_status_t r = 0;

    if (p_gr_lst == NULL || gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    // free memory of the graph elements
    struct ams_endpoint_el *p_endpoint = gr->sinks.first;
    struct ams_endpoint_list *psinks = &gr->sinks;
    REMOVE_LIST_EL_ALL(p_endpoint, psinks);

    p_endpoint = gr->sources.first;
    struct ams_endpoint_list *psources = &gr->sources;
    REMOVE_LIST_EL_ALL(p_endpoint, psources);

    struct ams_module_el *p_module = gr->modules.first;
    struct ams_module_list *pmodules = &gr->modules;
    REMOVE_LIST_EL_ALL(p_module, pmodules);

    struct ams_connection_el *p_con = gr->connections.first;
    struct ams_connection_list *pcons = &gr->connections;
    REMOVE_LIST_EL_ALL(p_con, pcons);

    struct ams_graph_property_el *p_prop = gr->props.first;
    struct ams_graph_property_list *pprops = &gr->props;
    REMOVE_LIST_EL_ALL(p_prop, pprops);
    // invalidate handle so it cannot be found
    gr->handle = NULL;
    //    gr->fw_handle = 0;

exit:
    return r;
}

struct ams_graph *ams_util_graph_list_find_graph(
    ams_session_t amss,
    ams_graph_handle_t hgr)
{
    struct ams_graph *p_gr = NULL;

    if (amss == NULL || hgr == NULL)
    {
        goto exit;
    }

    p_gr = amss->gr_lst.first;
    while (p_gr != NULL)
    {
        if (MAKE_INT_HANDLE(p_gr->handle) == MAKE_INT_HANDLE(hgr))
        {
            goto exit;
        }
        else
        {
            p_gr = p_gr->nxt_el;
        }
    }
exit:

    return p_gr;
}

struct ams_graph *ams_util_graph_list_find_graph_fw_handle(
    ams_session_t amss,
    uint32_t fw_handle)
{
    struct ams_graph *p_gr = NULL;

    if (amss == NULL || fw_handle == 0)
    {
        goto exit;
    }

    p_gr = amss->gr_lst.first;
    while (p_gr != NULL)
    {
        if (p_gr->fw_handle == fw_handle)
        {
            AMS_LIB_LOGD("Found graph by hw handle %08x", p_gr->fw_handle);
            goto exit;
        }
        else
        {
            p_gr = p_gr->nxt_el;
        }
    }
exit:

    return p_gr;
}

ams_status_t ams_util_cb_list_add_new_cb(
    ams_session_t amss,
    struct ams_cb **cb)
{
    ams_status_t r = 0;
    struct ams_cb *p_cb = NULL;

    if (amss == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    struct ams_session *p_ses = amss;
    struct ams_cb_list *p_cb_lst = &p_ses->cb_lst;

    // create new graph;
    p_cb = calloc(1, sizeof(struct ams_cb)); // assume that NULL is 0!
    if (p_cb == NULL)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }

    if (p_cb_lst->el_num == 0)
    {
        // single element
        p_cb_lst->first = p_cb;
        p_cb_lst->last = p_cb;
    }
    else
    {
        p_cb->prev_el = p_cb_lst->last;
        p_cb_lst->last->nxt_el = p_cb;
        p_cb_lst->last = p_cb;
    }
    p_cb_lst->el_num++;

    *cb = p_cb;

exit:
    return r;
}

ams_status_t ams_util_cb_list_remove_cb(
    ams_session_t amss,
    struct ams_cb *cb)
{
    ams_status_t r = 0;
    struct ams_cb *p_cb = cb;
    struct ams_cb *p_cb_tmp;

    if (amss == NULL || cb == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    struct ams_session *p_ses = amss;
    struct ams_cb_list *p_cb_lst = &p_ses->cb_lst;

    REMOVE_LIST2_EL(p_cb_lst, p_cb, p_cb_tmp);
    AMS_LIB_LOGD("Cb list num = %d", p_cb_lst->el_num);
exit:
    return r;
}

ams_status_t ams_util_print_graph_info(
    ams_session_t amss,
    ams_graph_handle_t gh)
{
    ams_status_t r = 0;
    struct ams_graph *gr = ams_util_graph_list_find_graph(amss, gh);
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    AMS_LIB_LOGD(":");
    AMS_LIB_LOGD("    Graph Handle=%ld", (uint64_t)gr->handle);
    AMS_LIB_LOGD("    fw handle=%d", gr->fw_handle);
    AMS_LIB_LOGD("    fw status=%d", gr->fw_status);

    r = ams_util_print_endpoint_info(gr);
    AMS_LIB_LOGD("    Endpoint info ret=%d", r);

    r = ams_util_print_module_info(gr);
    AMS_LIB_LOGD("    Module info ret=%d", r);

    r = ams_util_print_connection_info(gr);
    AMS_LIB_LOGD("    Connection info ret=%d", r);

exit:
    return r;
}

uint32_t ams_util_graph_list_get_graph_num(
    ams_session_t amss)
{
    struct ams_session *p_ses = amss;
    if (p_ses != NULL)
        return p_ses->gr_lst.el_num;
    return 0;
}

ams_status_t ams_util_print_baseparam_info(
    ams_graph_basic_params_t *base_param)
{
    ams_status_t r = 0;
    if (base_param == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    AMS_LIB_LOGD(":");
    AMS_LIB_LOGD("    Block size=%d", base_param->block_size);
    AMS_LIB_LOGD("    Flags=%d", base_param->flags);
    AMS_LIB_LOGD("    Proc id=%d", base_param->processor_id);
    AMS_LIB_LOGD("    Sample rate=%d", base_param->sample_rate);

exit:
    return r;
}

ams_status_t ams_util_print_endpoint_info(
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    ams_endpoint_t *e;
    struct ams_endpoint_list *e_l = &gr->sinks;
    struct ams_endpoint_el *e_el = e_l->first;

    AMS_LIB_LOGD(":");
    AMS_LIB_LOGD("Num sinks=%d", e_l->el_num);
    while (e_el != NULL)
    {
        e = &e_el->val;
        AMS_LIB_LOGD("    id=%d", e->id);
        AMS_LIB_LOGD("    type flag=%d", e->type);
        AMS_LIB_LOGD("    Tdm_param.bit_widht=%d", e->tdm_params.bit_width);
        e_el = e_el->nxt_el;
    }
    e_l = &gr->sources;
    e_el = e_l->first;
    AMS_LIB_LOGD("Num sources=%d", e_l->el_num);
    while (e_el != NULL)
    {
        e = &e_el->val;
        AMS_LIB_LOGD("    id=%d", e->id);
        AMS_LIB_LOGD("    type flag=%d", e->type);
        AMS_LIB_LOGD("    Tdm_param.bit_widht=%d", e->tdm_params.bit_width);
        e_el = e_el->nxt_el;
    }

exit:
    return r;
}

ams_status_t ams_util_print_module_info(
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    ams_module_t *m;
    struct ams_module_list *m_l = &gr->modules;
    struct ams_module_el *m_el = m_l->first;

    AMS_LIB_LOGD(":");
    AMS_LIB_LOGD("    Num modules=%d", m_l->el_num);
    while (m_el != NULL)
    {
        m = &m_el->val;
        AMS_LIB_LOGD("    Module id=%d", m->id);
        AMS_LIB_LOGD("    Module tag=%s", m->capiv2_info.tag);
        m_el = m_el->nxt_el;
    }

exit:
    return r;
}

ams_status_t ams_util_print_connection_info(
    struct ams_graph *gr)
{
    ams_status_t r = 0;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    ams_connection_t *c;
    struct ams_connection_list *c_l = &gr->connections;
    struct ams_connection_el *c_el = c_l->first;

    AMS_LIB_LOGD(":");
    AMS_LIB_LOGD("    Num connections=%d", c_l->el_num);
    while (c_el != NULL)
    {
        c = &c_el->val;
        if (c->source.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE)
        {
            AMS_LIB_LOGD("    source module=%d", c->source.module.id);
        }
        else
        {
            AMS_LIB_LOGD("    source endpoint=%d", c->source.endpoint.id);
        }
        if (c->destination.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE)
        {
            AMS_LIB_LOGD("    dest modules=%d", c->destination.module.id);
        }
        else
        {
            AMS_LIB_LOGD("    dest endpoint=%d", c->destination.endpoint.id);
        }
        c_el = c_el->nxt_el;
    }
exit:
    return r;
}

ams_status_t ams_util_check_graph_baseparam(
    ams_graph_basic_params_t *base_param)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    if (base_param == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    // TODO: check if other param should be checked
exit:
    return r;
}

static inline ams_status_t ams_util_check_endpoint_tdm_param(ams_endpoint_t *e)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (e == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.num_channels == 0 || e->tdm_params.num_channels > DSP_AMS_MAX_NUM_CH)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.bit_width == 0 || e->tdm_params.bit_width > DSP_AMS_SAMPLE_BITWIDTH_MAX)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.data_format != AMS_TDM_LINEAR_PCM_DATA)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.sync_mode != AMS_TDM_SHORT_SYNC_BIT_MODE && e->tdm_params.sync_mode != AMS_TDM_LONG_SYNC_MODE && e->tdm_params.sync_mode != AMS_TDM_SHORT_SYNC_SLOT_MODE)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.sync_src != AMS_TDM_SYNC_SRC_EXTERNAL && e->tdm_params.sync_src != AMS_TDM_SYNC_SRC_INTERNAL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.ctrl_data_out_enable != AMS_TDM_CTRL_DATA_OE_DISABLE && e->tdm_params.ctrl_data_out_enable != AMS_TDM_CTRL_DATA_OE_ENABLE)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.ctrl_invert_sync_pulse != AMS_TDM_SYNC_NORMAL && e->tdm_params.ctrl_invert_sync_pulse != AMS_TDM_SYNC_INVERT)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.ctrl_sync_data_delay != AMS_TDM_DATA_DELAY_0_BCLK_CYCLE && e->tdm_params.ctrl_sync_data_delay != AMS_TDM_DATA_DELAY_1_BCLK_CYCLE && e->tdm_params.ctrl_sync_data_delay != AMS_TDM_DATA_DELAY_2_BCLK_CYCLE)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if (e->tdm_params.slot_width != 16 && e->tdm_params.slot_width != 24 && e->tdm_params.slot_width != 32)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
exit:
    return r;
}

ams_status_t ams_util_check_endpoint_param(
    ams_endpoint_t *e)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (e == NULL || e->q_factor > 31 || (e->type != AMS_ENDPOINT_TYPE_SOURCE && e->type != AMS_ENDPOINT_TYPE_SINK) ||
        (e->flags != AMS_ENDPOINT_EXCLUSIVE && e->flags != AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT && e->flags != AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT))
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    if ((r = ams_util_check_endpoint_tdm_param(e)) != AMS_STATUS_SUCCESS)
    {
        goto exit;
    }
    r = ams_util_check_hw_intf_id(e->tdm_params.hw_interface_id);
    if (r != AMS_STATUS_SUCCESS)
    {
        goto exit;
    }

exit:
    return r;
}

static inline int ams_util_get_null_char_pos(
    char *str,
    uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        if (str[i] == '\0')
        {
            break;
        }
    }
    return i;
}

// check some formal rules on new added module
// TODO: check if any other needed
ams_status_t ams_util_check_module_param(
    ams_module_t *m)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    if ((m->capiv2_info.tag[0] == '\0' && m->capiv2_info.shared_obj_filename[0] != '\0') ||
        (m->capiv2_info.shared_obj_filename[0] == '\0' && m->capiv2_info.tag[0] != '\0'))
    {
        AMS_LIB_LOGE("Check module:Tag or filename wrong for statically module!");
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    // check strings are not null terminated
    if (ams_util_get_null_char_pos(m->capiv2_info.tag, sizeof(m->capiv2_info.tag)) == sizeof(m->capiv2_info.tag))
    {
        // no null symbol found!
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Module with id=%d has not-null terminated tag string!", m->id);
        goto exit;
    }
    if (ams_util_get_null_char_pos(m->capiv2_info.shared_obj_filename, sizeof(m->capiv2_info.shared_obj_filename)) == sizeof(m->capiv2_info.shared_obj_filename))
    {
        // no null symbol found!
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Module with id=%d has not-null terminated obj-filename string!", m->id);
        goto exit;
    }
    // TODO: check if other param should be checked

exit:
    return r;
}

ams_status_t ams_util_check_connection_param(
    struct ams_graph *gr,
    ams_connection_t *c)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_module_el *m = NULL;
    struct ams_endpoint_el *e = NULL;
    // check if source and sink
    if (c == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    if (c->source.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE)
    {
        // source endpoint
        // check if module exists in the graph
        m = ams_util_graph_get_module_by_id(gr, c->source.module.id);
        if (m == NULL)
        {
            AMS_LIB_LOGD("Source module =%d not found", c->source.module.id);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        else
        {
            // increase cnt
            m->used_as_source_cnt++;
        }
    }
    else
    {
        // source endpoint
        // check if endpoint exist in the graph
        e = ams_util_graph_get_endpoint_by_id(gr, c->source.endpoint.id);
        if (e == NULL)
        {
            AMS_LIB_LOGD("Source endpoint =%d not found", c->source.endpoint.id);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        else
        {
            e->used_as_source_cnt++;
        }
    }

    if (c->destination.type == AMS_CONNECTION_ELEMENT_TYPE_MODULE)
    {
        // sink endpoint
        // check if module exists in the graph
        m = ams_util_graph_get_module_by_id(gr, c->destination.module.id);
        if (m == NULL)
        {
            AMS_LIB_LOGD("Destination module =%d not found", c->destination.module.id);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        else
        {
            m->used_as_sink_cnt++;
        }
    }
    else
    {
        // sink endpoint
        // check if endpoint exist in the graph
        e = ams_util_graph_get_endpoint_by_id(gr, c->destination.endpoint.id);
        if (e == NULL)
        {
            AMS_LIB_LOGD("Destination endpoint =%d not found", c->destination.endpoint.id);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        else
        {
            e->used_as_sink_cnt++;
        }
    }

exit:
    return r;
}

ams_status_t ams_util_check_graph(
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_endpoint_el *eel;
    struct ams_module_el *mel;
    struct ams_connection_el *cel;
    uint32_t id;
    struct ams_graph_path *pp_l = NULL;
    struct ams_graph_path_el *pel = NULL;
    // check graph validity here
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    // no sink, source or connections
    if (gr->sinks.el_num == 0 || /*gr->sources.el_num == 0 ||*/ gr->connections.el_num == 0)
    {
        r = AMS_STATUS_GRAPH_TOPO_ERROR;
        AMS_LIB_LOGE("No sink/connections in the graph!");
        goto exit;
    }
    // check connections
    cel = gr->connections.first;
    while (cel != NULL)
    {
        if (ams_util_check_connection_param(gr, &cel->val) != AMS_STATUS_SUCCESS)
        {
            r = AMS_STATUS_INPUT_ERROR;
            AMS_LIB_LOGE("Wrong connection param!");
            goto exit;
        }
        cel = cel->nxt_el;
    }
    // check if unconnected modules or endpoints exist
    mel = gr->modules.first;
    while (mel != NULL)
    {
        if ((mel->used_as_sink_cnt == 0 && mel->used_as_source_cnt == 0) ||
            (mel->used_as_sink_cnt > 0 && mel->used_as_source_cnt == 0))
        // used_as_sink_cnt == 0 && used_as_source_cnt > 0) - allowed
        {
            r = AMS_STATUS_GRAPH_TOPO_ERROR;
            AMS_LIB_LOGE("Module id=%d wrongly used as sink or not connected!", mel->val.id);
            goto exit;
        }
        mel = mel->nxt_el;
    }
    eel = gr->sinks.first;
    while (eel != NULL)
    {
        if (eel->used_as_sink_cnt == 0 || eel->used_as_source_cnt > 0)
        {
            r = AMS_STATUS_GRAPH_TOPO_ERROR;
            AMS_LIB_LOGE("Sink endpoint id=%d not connected or used as a source!", eel->val.id);
            goto exit;
        }
        eel = eel->nxt_el;
    }
    eel = gr->sources.first;
    while (eel != NULL)
    {
        if (eel->used_as_source_cnt == 0 || eel->used_as_sink_cnt > 0)
        {
            r = AMS_STATUS_GRAPH_TOPO_ERROR;
            AMS_LIB_LOGE("Source endpoint id=%d not connected or used as a sink!", eel->val.id);
            goto exit;
        }
        eel = eel->nxt_el;
    }
    // search for loops in the graph starting from sources
    eel = gr->sources.first;
    pp_l = calloc(1, sizeof(struct ams_graph_path));
    if (pp_l == NULL)
    {
        AMS_LIB_LOGE("Cannot allocate memory");
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    while (eel != NULL)
    {
        id = eel->val.id;
        eel->visit_cnt++;
        pel = calloc(1, sizeof(struct ams_graph_path_el));
        if (pel == NULL)
        {
            AMS_LIB_LOGE("Cannot allocate memory");
            r = AMS_STATUS_GENERAL_ERROR;
            goto exit;
        }
        pel->id = id;
        pel->nxt_el = NULL;
        pel->type = 0;

        ADD_NEW_LIST_EL(pp_l, pel);
        r = ams_util_graph_check_path(gr, pp_l);
        if (r != AMS_STATUS_SUCCESS)
        {
            AMS_LIB_LOGE("Path started from %d cannot be checked!Stop check", id);
            goto exit;
        }
        eel = eel->nxt_el;
    }
    // search for loops in the graph starting from modules
    mel = gr->modules.first;
    while (mel != NULL)
    {
        id = mel->val.id;
        if (mel->used_as_sink_cnt == 0)
        {
            // check starting from modules which has no input connections(sources)
            mel->visit_cnt++;
            pel = calloc(1, sizeof(struct ams_graph_path_el));
            if (pel == NULL)
            {
                AMS_LIB_LOGE("Cannot allocate memory");
                r = AMS_STATUS_GENERAL_ERROR;
                goto exit;
            }
            pel->id = id;
            pel->nxt_el = NULL;
            pel->type = 1;
            ADD_NEW_LIST_EL(pp_l, pel);
            r = ams_util_graph_check_path(gr, pp_l);
            if (r != AMS_STATUS_SUCCESS)
            {
                AMS_LIB_LOGE("Path started from %d cannot be checked!Stop check", id);
                goto exit;
            }
        }
        mel = mel->nxt_el;
    }
    r = ams_util_check_unvisited_connections(gr);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Graph has loops!");
        goto exit;
    }

exit:
    if (pp_l != NULL)
        free(pp_l);
    return r;
}

ams_status_t ams_util_check_hw_intf_id(
    uint32_t id)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    if (id < AMS_HW_INTERFACE_TDM1 || id > AMS_HW_INTERFACE_TDM5)
    {
        AMS_LIB_LOGE("Bad HW id %d!", id);
        r = AMS_STATUS_INPUT_ERROR;
    }

    return r;
}

struct ams_module_el *ams_util_graph_get_module_by_id(
    struct ams_graph *gr,
    uint32_t id)
{
    struct ams_module_el *module_el = NULL;
    uint32_t found = 0;

    if (gr == NULL)
    {
        goto exit;
    }

    // check if id is used by another module
    module_el = gr->modules.first;
    while (module_el != NULL)
    {
        if (module_el->val.id == id)
        {
            found = 1;
            AMS_LIB_LOGD("Module with id =%d exists!", id);
            goto exit;
        }
        module_el = module_el->nxt_el;
    }
    if (!found)
        module_el = NULL;
exit:
    return module_el;
}

struct ams_endpoint_el *ams_util_graph_get_endpoint_by_id(
    struct ams_graph *gr,
    uint32_t id)
{
    struct ams_endpoint_el *e_el = NULL;
    uint32_t found = 0;

    e_el = gr->sinks.first;
    while (e_el != NULL)
    {
        if (e_el->val.id == id)
        {
            AMS_LIB_LOGD("Sink with id =%d exists!", id);
            found = 1;
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    e_el = gr->sources.first;
    while (e_el != NULL)
    {
        if (e_el->val.id == id)
        {
            found = 1;
            AMS_LIB_LOGD("Source with id =%d exists!", id);
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    if (!found)
        e_el = NULL;
exit:
    return e_el;
}

struct ams_endpoint_el *ams_util_graph_get_endpoint_by_port_id(
    struct ams_graph *gr,
    uint32_t id)
{
    struct ams_endpoint_el *e_el = NULL;
    uint32_t found = 0;

    e_el = gr->sinks.first;
    while (e_el != NULL)
    {
        if (e_el->val.tdm_params.hw_interface_id == id)
        {
            AMS_LIB_LOGD("Sink with id =%d exists!", id);
            found = 1;
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    e_el = gr->sources.first;
    while (e_el != NULL)
    {
        if (e_el->val.tdm_params.hw_interface_id == id)
        {
            found = 1;
            AMS_LIB_LOGD("Source with id =%d exists!", id);
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    if (!found)
        e_el = NULL;
exit:
    return e_el;
}

// check if id is alredy used in a particulaR graph before adding element to the graph
ams_status_t ams_util_check_unique_id(
    struct ams_graph *gr,
    uint32_t id)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    struct ams_endpoint_el *endpoint_el;
    struct ams_module_el *module_el;

    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    module_el = ams_util_graph_get_module_by_id(gr, id);
    if (module_el != NULL)
    {
        // ID already in use!
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    endpoint_el = ams_util_graph_get_endpoint_by_id(gr, id);
    if (endpoint_el != NULL)
    {
        // ID already in use!
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

exit:
    return r;
}

// check if id is alredy used in a particulaR graph by other element
ams_status_t ams_util_check_unique_endpoint_id(
    struct ams_graph *gr,
    struct ams_endpoint_el *ep_el)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    struct ams_module_el *module_el;

    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    module_el = ams_util_graph_get_module_by_id(gr, ep_el->val.id);
    if (module_el != NULL)
    {
        // ID already in use!
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    struct ams_endpoint_el *e_el = NULL;

    e_el = gr->sinks.first;
    while (e_el != NULL)
    {
        if (e_el->val.id == ep_el->val.id && ep_el != e_el)
        {
            AMS_LIB_LOGD("Sink with id =%d exists!", e_el->val.id);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        e_el = e_el->nxt_el;
    }
    e_el = gr->sources.first;
    while (e_el != NULL)
    {
        if (e_el->val.id == ep_el->val.id && ep_el != e_el)
        {
            r = AMS_STATUS_INPUT_ERROR;
            AMS_LIB_LOGD("Source with id =%d exists!", e_el->val.id);
            goto exit;
        }
        e_el = e_el->nxt_el;
    }

exit:
    return r;
}

// check if id is alredy used in a particulaR graph by other element
ams_status_t ams_util_check_unique_module_id(
    struct ams_graph *gr,
    struct ams_module_el *m_el)
{
    ams_status_t r = AMS_STATUS_SUCCESS;

    struct ams_endpoint_el *endpoint_el;
    struct ams_module_el *module_el;

    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

    module_el = gr->modules.first;
    while (module_el != NULL)
    {
        if (module_el->val.id == m_el->val.id && m_el != module_el)
        {
            r = AMS_STATUS_INPUT_ERROR;
            AMS_LIB_LOGD("Module with id =%d exists!", module_el->val.id);
            goto exit;
        }
        module_el = module_el->nxt_el;
    }

    endpoint_el = ams_util_graph_get_endpoint_by_id(gr, m_el->val.id);
    if (endpoint_el != NULL)
    {
        // ID already in use!
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }

exit:
    return r;
}

ams_status_t ams_util_graph_serialize(
    struct ams_graph *gr,
    void *ser_buf,
    uint32_t ser_buf_size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    // calculate and get buffer for payload
    void *ppld = NULL;
    uint32_t pld_len = 0;
    if (gr == NULL /*|| gr->fw_status!=DSP_AMS_GRAPH_INIT*/)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }

    pld_len = sizeof(dsp_ams_cmd_open_graph_t) + gr->modules.el_num * sizeof(ams_module_t) + gr->connections.el_num * sizeof(ams_connection_t) + (gr->sinks.el_num + gr->sources.el_num) * sizeof(ams_endpoint_t);
    AMS_LIB_LOGD("Payload buffer length+%d", pld_len);
    if (pld_len > ser_buf_size)
    {
        AMS_LIB_LOGE("Not enough long buffer(%d), needed (%d)", ser_buf_size, pld_len);
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    ppld = ser_buf;
    if (ppld == NULL)
    {
        AMS_LIB_LOGE("No allocated memory for graph payload!");
        pld_len = 0;
        r = AMS_STATUS_GENERAL_ERROR;
        goto exit;
    }
    // serialization: fill payload buffer
    uint8_t *pbuf_fil = ppld;

    dsp_ams_cmd_open_graph_t *pgr_ser = ppld;
    pgr_ser->sample_rate = gr->base_param.sample_rate;
    pgr_ser->block_size = gr->base_param.block_size;
    pgr_ser->flags = gr->base_param.flags;
    pgr_ser->num_endpoints = gr->sinks.el_num + gr->sources.el_num;
    pgr_ser->num_modules = gr->modules.el_num;
    pgr_ser->num_connections = gr->connections.el_num;
    pbuf_fil += sizeof(dsp_ams_cmd_open_graph_t);

    uint32_t len;
    struct ams_endpoint_el *p_e_s = gr->sinks.first;
    len = sizeof(ams_endpoint_t);
    LIST_EL_SZD(p_e_s, pbuf_fil, len);
    // sources
    p_e_s = gr->sources.first;
    LIST_EL_SZD(p_e_s, pbuf_fil, len);
    // modules
    struct ams_module_el *p_m_s = gr->modules.first;
    len = sizeof(ams_module_t);
    LIST_EL_SZD(p_m_s, pbuf_fil, len);
    // connections
    struct ams_connection_el *p_c_s = gr->connections.first;
    len = sizeof(ams_connection_t);
    LIST_EL_SZD(p_c_s, pbuf_fil, len);

exit:
    // *ser_buf_size = pld_len;
    return r;
}

ams_status_t ams_util_graph_deserialize(
    struct ams_graph *gr,
    void *ser_buf,
    uint32_t ser_buf_size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    uint8_t *pld_ptr = ser_buf;

    struct ams_graph *new_graph = gr;
    // new_graph = calloc(1, sizeof(struct ams_graph));
    if (new_graph == NULL || ser_buf == NULL || ser_buf_size == 0)
    {
        r = AMS_STATUS_GENERAL_ERROR;
        AMS_LIB_LOGE("NULL pointer/buff!");
        goto exit;
    }
    // deserialize
    uint32_t i, nume, numm, numc;
    dsp_ams_cmd_open_graph_t *pgr_ser = (dsp_ams_cmd_open_graph_t *)pld_ptr;
    new_graph->base_param.sample_rate = pgr_ser->sample_rate;
    new_graph->base_param.block_size = pgr_ser->block_size;
    new_graph->base_param.flags = pgr_ser->flags;
    AMS_LIB_LOGD("Graph descr:sr=%d!", new_graph->base_param.sample_rate);
    AMS_LIB_LOGD("Graph descr:block_size=%d!", new_graph->base_param.block_size);
    AMS_LIB_LOGD("Graph descr:flags=%d!", new_graph->base_param.flags);
    nume = pgr_ser->num_endpoints;
    numm = pgr_ser->num_modules;
    numc = pgr_ser->num_connections;
    AMS_LIB_LOGD("Graph descr:nume=%d, numm=%d, numc=%d!", nume, numm, numc);
    if (sizeof(dsp_ams_cmd_open_graph_t) + nume * sizeof(ams_endpoint_t) +
            numm * sizeof(ams_module_t) + numc * sizeof(ams_connection_t) >
        ser_buf_size)
    {
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Buff size not big enough for graph elements!");
        // free(new_graph);
        // new_graph = NULL;
        goto exit;
    }
    pld_ptr += sizeof(dsp_ams_cmd_open_graph_t);
    ams_endpoint_t *e = (void *)pld_ptr;
    for (i = 0; i < nume; i++)
    {
        struct ams_endpoint_el *new_el;
        struct ams_endpoint_list *dest_e_list;
        new_el = calloc(1, sizeof(struct ams_endpoint_el));
        if (new_el == NULL)
        {
            r = DSP_AMS_STATUS_NO_RESOURCE;
            goto exit;
        }
        // sink or source
        if (e[i].type == AMS_ENDPOINT_TYPE_SINK)
        {
            // sink
            dest_e_list = &new_graph->sinks;
        }
        else
        {
            // source
            dest_e_list = &new_graph->sources;
        }
        ADD_NEW_LIST_EL(dest_e_list, new_el);
        // copy fields
        memcpy(&dest_e_list->last->val, &e[i], sizeof(ams_endpoint_t));
        pld_ptr += sizeof(ams_endpoint_t);
    }
    ams_module_t *m = (ams_module_t *)pld_ptr;
    for (i = 0; i < numm; i++)
    {
        struct ams_module_list *dest_m_list;
        struct ams_module_el *new_el;
        new_el = calloc(1, sizeof(struct ams_module_el));
        if (new_el == NULL)
        {
            r = DSP_AMS_STATUS_NO_RESOURCE;
            goto exit;
        }
        dest_m_list = &new_graph->modules;
        ADD_NEW_LIST_EL(dest_m_list, new_el);
        // copy fields
        memcpy(&dest_m_list->last->val, &m[i], sizeof(ams_module_t));
        pld_ptr += sizeof(ams_module_t);
    }
    ams_connection_t *c = (ams_connection_t *)pld_ptr;
    for (i = 0; i < numc; i++)
    {
        struct ams_connection_list *dest_c_list;
        struct ams_connection_el *new_el;
        new_el = calloc(1, sizeof(struct ams_connection_el));
        if (new_el == NULL)
        {
            r = DSP_AMS_STATUS_NO_RESOURCE;
            goto exit;
        }
        dest_c_list = &new_graph->connections;
        ADD_NEW_LIST_EL(dest_c_list, new_el);
        // copy fields
        memcpy(&dest_c_list->last->val, &c[i], sizeof(ams_connection_t));
        pld_ptr += sizeof(ams_connection_t);
    }
exit:
    // *gr = new_graph;
    return r;
}

uint32_t ams_util_graph_get_hw_handle(
    ams_session_t amss,
    ams_graph_handle_t gh)
{
    struct ams_session *p_ses = amss;
    struct ams_graph *p_gr = NULL;
    uint32_t hdl = 0;
    if (p_ses == NULL)
    {
        AMS_LIB_LOGE("Null session!");
        goto exit;
    }
    if (gh == NULL)
    {
        AMS_LIB_LOGE("Null graph handle!");
        goto exit;
    }
    p_gr = ams_util_graph_list_find_graph(amss, gh);
    if (p_gr == NULL)
    {
        goto exit;
    }
    hdl = p_gr->fw_handle;
exit:
    return hdl;
}

void *ams_util_ssr_get_handle(
    ams_session_t amss)
{
    void *hdl = NULL;
    struct ams_session *p_ses = amss;
    if (p_ses == NULL)
    {
        AMS_LIB_LOGE("Null session!");
        goto exit;
    }
    hdl = p_ses->drv.ssr.hssr;
exit:
    return hdl;
}
#ifndef AMS_UTIL_DONT_CHECK_TDM_INTF
static uint32_t ams_util_check_conflict_tdm_param(ams_endpoint_t *e, struct ams_endpoint_el *pel)
{
    int32_t diff = 0;
    while (pel != NULL)
    {
        if (pel->val.tdm_params.hw_interface_id == e->tdm_params.hw_interface_id)
        {
            AMS_LIB_LOGD("Find same hw intf %d used", pel->val.tdm_params.hw_interface_id);
            diff = pel->val.tdm_params.num_channels == e->tdm_params.num_channels ? 0 : 1;
            diff |= pel->val.tdm_params.bit_width == e->tdm_params.bit_width ? 0 : 1;
            diff |= pel->val.tdm_params.data_format == e->tdm_params.data_format ? 0 : 1;
            diff |= pel->val.tdm_params.sync_mode == e->tdm_params.sync_mode ? 0 : 1;
            diff |= pel->val.tdm_params.sync_src == e->tdm_params.sync_src ? 0 : 1;
            diff |= pel->val.tdm_params.nslots_per_frame == e->tdm_params.nslots_per_frame ? 0 : 1;
            diff |= pel->val.tdm_params.ctrl_data_out_enable == e->tdm_params.ctrl_data_out_enable ? 0 : 1;
            diff |= pel->val.tdm_params.ctrl_invert_sync_pulse == e->tdm_params.ctrl_invert_sync_pulse ? 0 : 1;
            diff |= pel->val.tdm_params.ctrl_sync_data_delay == e->tdm_params.ctrl_sync_data_delay ? 0 : 1;
            diff |= pel->val.tdm_params.slot_width == e->tdm_params.slot_width ? 0 : 1;
            diff |= pel->val.tdm_params.slot_mask == e->tdm_params.slot_mask ? 0 : 1;
            if (diff)
            {
                // error: same TDM, different param!
                AMS_LIB_LOGE("error: same TDM %d, different param used!", pel->val.tdm_params.hw_interface_id);
                break;
            }
        }
        pel = pel->nxt_el;
    }

    return diff;
}
#endif
ams_status_t ams_util_check_graph_tdm_intf(ams_session_t amss, struct ams_graph *pgr_self, ams_endpoint_t *e)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
#ifndef AMS_UTIL_DONT_CHECK_TDM_INTF
    struct ams_session *p_ses = amss;
    struct ams_graph *pgr = p_ses->gr_lst.first;
    struct ams_graph *pgr_nxt = NULL;
    struct ams_endpoint_el *pel = NULL;
    int32_t reslock_gr_el = 0;
    while (pgr != NULL && pgr != pgr_self)
    {
        LOCK_FOR_READ(reslock_gr_el, &pgr->rwlock);
        if (e->type == AMS_ENDPOINT_TYPE_SINK)
        {
            pel = pgr->sinks.first;
        }
        else if (e->type == AMS_ENDPOINT_TYPE_SOURCE)
        {
            pel = pgr->sources.first;
        }
        else
        {
            AMS_LIB_LOGE("Unknown type!");
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        if (ams_util_check_conflict_tdm_param(e, pel))
        {
            AMS_LIB_LOGE("Conflicting TDP param for the same TDM id=%d and type=%d", e->tdm_params.hw_interface_id, e->type);
            r = AMS_STATUS_INPUT_ERROR;
            goto exit;
        }
        pgr_nxt = pgr->nxt_el;
        UNLOCK_READ_WRITE(reslock_gr_el, &pgr->rwlock);
        pgr = pgr_nxt;
    }
exit:
    if (pgr)
        UNLOCK_READ_WRITE(reslock_gr_el, &pgr->rwlock);
#endif
    return r;
}

ams_status_t ams_util_update_graph_fwinfo_on_ssr(
    struct ams_graph *gr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (gr == NULL)
    {
        r = AMS_STATUS_INPUT_ERROR;
        goto exit;
    }
    gr->fw_status = DSP_AMS_GRAPH_INIT;
    gr->fw_handle = 0;
exit:
    return r;
}

ams_status_t ams_util_check_graph_property(struct ams_graph *gr, ams_graph_property_t *prop)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    switch (prop->prop_id)
    {
    case AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR:
    {
        struct ams_endpoint_el *ep = ams_util_graph_get_endpoint_by_port_id(gr, prop->u.exclv_ep_clk_attr.exclv_ep_id);
        if (ep == NULL || ep->val.flags != AMS_ENDPOINT_EXCLUSIVE)
        {
            r = AMS_STATUS_INPUT_ERROR;
            AMS_LIB_LOGE("Property invalid!");
            goto exit;
        }
    }
    break;
    case AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_BUILD:
    case AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_DESTROY:
    case AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG:
        // AMS FW will do error check
        break;
    default:
        r = AMS_STATUS_INPUT_ERROR;
        AMS_LIB_LOGE("Property invalid!");
    }
exit:
    return r;
}

ams_status_t ams_util_session_restore_graph_info(struct ams_session *pses, ams_cached_graph_descr_t *param, struct ams_graph **ppgr)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    struct ams_graph *pgr = NULL;

    r = ams_util_graph_list_add_new_graph((ams_session_t)pses, &pgr);

    if (pgr == NULL)
    {
        AMS_LIB_LOGE("Cannot add new graph to the list!");
        goto exit;
    }
    r = ams_util_graph_deserialize(pgr, param->gr_descr, param->gr_descr_sz);
    if (r != AMS_STATUS_SUCCESS)
    {
        AMS_LIB_LOGE("Cannot deserialize graph!");
        goto exit;
    }
exit:
    *ppgr = pgr;
    return r;
}

ams_status_t ams_util_memcpy(void *dst, uint32_t dst_size,
                             const void *src, uint32_t src_size)
{
    ams_status_t r = AMS_STATUS_SUCCESS;
    if (dst == NULL || src == NULL)
    {
        AMS_LIB_LOGE("Null Param : dst[0x%lx], src[0x%lx]", (uintptr_t)dst, (uintptr_t)src);
        r = AMS_STATUS_INPUT_ERROR;
    }
    else
    {
        uint32_t copy_size = (dst_size <= src_size) ? dst_size : src_size;
        memcpy(dst, src, copy_size);
    }
    return r;
}

#ifndef AMS_UTIL_H
#define AMS_UTIL_H
/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <string.h>
// #include <pthread.h>
#include "ams_osal_mutex.h"
#include "ams.h"
#include "ams_impl.h"
#include "ams_log.h"
#include "ams_platform.h"
// free memory of the graph elements
#define REMOVE_LIST_EL_ALL(head, list)  \
    {                                   \
        while (head != NULL)            \
        {                               \
            head = list->first->nxt_el; \
            free(list->first);          \
            list->el_num--;             \
            list->first = head;         \
        }                               \
        if (list->el_num == 0)          \
            list->last = NULL;          \
    }

#define MAKE_INT_HANDLE(x) ((uint64_t *)x)

// single linked list
#define ADD_NEW_LIST_EL(list, new_el)        \
    {                                        \
        if (new_el != NULL)                  \
        {                                    \
            if (list->el_num == 0)           \
            {                                \
                list->first = new_el;        \
                list->first->nxt_el = NULL;  \
                list->last = list->first;    \
            }                                \
            else                             \
            {                                \
                list->last->nxt_el = new_el; \
                list->last = new_el;         \
            }                                \
            list->last->nxt_el = NULL;       \
            list->el_num++;                  \
        }                                    \
    }
#if 0
#define REMOVE_LIST2_DBG(list)                                               \
    if (list->el_num == 0)                                                   \
    {                                                                        \
        if (list->first != NULL)                                             \
            AMS_LIB_LOGE("Oops!Empty list but not null first!");             \
        if (list->last != NULL)                                              \
            AMS_LIB_LOGE("Oops!Empty list but not null last!");              \
    }                                                                        \
    if (list->el_num == 1)                                                   \
    {                                                                        \
        if (list->first != list->last)                                       \
            AMS_LIB_LOGE("Oops!First!=last when 1 el in the list!");         \
        if (list->first == NULL)                                             \
            AMS_LIB_LOGE("Oops!Wrong first (NULL) when 1 el in the list !"); \
        if (list->last == NULL)                                              \
            AMS_LIB_LOGE("Oops!Wrong last (NULL) when 1 el in the list !");  \
    }
#else
#define REMOVE_LIST2_DBG(list)
#endif
// double-linked list
#define REMOVE_LIST2_EL(list, el, tmp)                                 \
    {                                                                  \
        if (el != NULL)                                                \
        {                                                              \
            if (list->el_num == 0)                                     \
                AMS_LIB_LOGE("Oops!Removing from list with no list!"); \
            if (el == list->first)                                     \
            {                                                          \
                tmp = el->nxt_el;                                      \
                if (tmp != NULL)                                       \
                {                                                      \
                    tmp->prev_el = NULL;                               \
                }                                                      \
                else                                                   \
                {                                                      \
                    list->last = tmp;                                  \
                }                                                      \
                list->first = tmp;                                     \
            }                                                          \
            else if (el == list->last)                                 \
            {                                                          \
                tmp = el->prev_el;                                     \
                if (tmp != NULL)                                       \
                {                                                      \
                    tmp->nxt_el = NULL;                                \
                }                                                      \
                else                                                   \
                {                                                      \
                    list->first = tmp;                                 \
                }                                                      \
                list->last = tmp;                                      \
            }                                                          \
            else                                                       \
            {                                                          \
                tmp = el->prev_el;                                     \
                tmp->nxt_el = el->nxt_el;                              \
                tmp = el->nxt_el;                                      \
                tmp->prev_el = el->prev_el;                            \
            }                                                          \
            free(el);                                                  \
            list->el_num--;                                            \
            REMOVE_LIST2_DBG(list)                                     \
        }                                                              \
    }
// double-linked list
#define ADD_NEW_LIST2_EL(list, new_el)    \
    {                                     \
        if (list->el_num == 0)            \
        {                                 \
            list->first = new_el;         \
            list->last = new_el;          \
            list->first->prev_el = NULL;  \
            list->first->nxt_el = NULL;   \
            list->last->prev_el = NULL;   \
        }                                 \
        else                              \
        {                                 \
            new_el->prev_el = list->last; \
            list->last->nxt_el = new_el;  \
            list->last = new_el;          \
        }                                 \
        list->last->nxt_el = NULL;        \
        list->el_num++;                   \
    }

#define LIST_EL_SZD(p_s, b_l, len)       \
    {                                    \
        while (p_s != NULL)              \
        {                                \
            memcpy(b_l, &p_s->val, len); \
            p_s = p_s->nxt_el;           \
            b_l += len;                  \
        }                                \
    }

#define LOCK_FOR_READ(reslock, l)                       \
    {                                                   \
        if (reslock || ams_osal_mutex_lock(l))          \
        {                                               \
            AMS_LIB_LOGE("Unable to lock for reading"); \
            r = AMS_STATUS_GENERAL_ERROR;               \
            goto exit;                                  \
        }                                               \
        else                                            \
        {                                               \
            reslock = 1;                                \
        }                                               \
    }

#define LOCK_FOR_WRITE(reslock, l)                      \
    {                                                   \
        if (reslock || ams_osal_mutex_lock(l))          \
        {                                               \
            AMS_LIB_LOGE("Unable to lock for writing"); \
            r = AMS_STATUS_GENERAL_ERROR;               \
            goto exit;                                  \
        }                                               \
        else                                            \
        {                                               \
            reslock = 1;                                \
        }                                               \
    }

#define UNLOCK_READ_WRITE(reslock, l)                         \
    {                                                         \
        if (reslock)                                          \
        {                                                     \
            if (ams_osal_mutex_unlock(l))                     \
            {                                                 \
                AMS_LIB_LOGE("Unable to unlock for reading"); \
                r = AMS_STATUS_GENERAL_ERROR;                 \
            }                                                 \
            else                                              \
            {                                                 \
                reslock = 0;                                  \
            }                                                 \
        }                                                     \
    }

#define MAKE_EXT_HANDLE(x) ((void *)x)

#define MAKE_64BIT_ADDR(lo, hi) ((uint8_t *)((((uint64_t)hi) << 32) | ((uint64_t)lo)))

ams_status_t ams_util_graph_list_add_new_graph(ams_session_t amss, struct ams_graph **gr);

ams_status_t ams_util_graph_list_remove_graph(struct ams_graph_list *p_gr_lst, struct ams_graph *gr);

ams_status_t ams_util_graph_list_remove_graph_elements(struct ams_graph_list *p_gr_lst, struct ams_graph *gr);

struct ams_graph *ams_util_graph_list_find_graph(ams_session_t amss, ams_graph_handle_t hgr);

ams_status_t ams_util_cb_list_add_new_cb(ams_session_t amss, struct ams_cb **cb);

ams_status_t ams_util_cb_list_remove_cb(ams_session_t amss, struct ams_cb *cb);

ams_status_t ams_util_check_graph_baseparam(ams_graph_basic_params_t *base_param);

ams_status_t ams_util_check_endpoint_param(ams_endpoint_t *e);

ams_status_t ams_util_check_module_param(ams_module_t *m);

ams_status_t ams_util_check_connection_param(struct ams_graph *gr, ams_connection_t *c);

ams_status_t ams_util_check_graph(struct ams_graph *gr);

ams_status_t ams_util_check_hw_intf_id(uint32_t id);

uint32_t ams_util_graph_list_get_graph_num(ams_session_t amss);

struct ams_module_el *ams_util_graph_get_module_by_id(struct ams_graph *gr, uint32_t id);

struct ams_endpoint_el *ams_util_graph_get_endpoint_by_id(struct ams_graph *gr, uint32_t id);

// for debugging
ams_status_t ams_util_print_graph_info(ams_session_t amss, void *hgr);

ams_status_t ams_util_print_baseparam_info(ams_graph_basic_params_t *base_param);

ams_status_t ams_util_print_endpoint_info(struct ams_graph *gr);

ams_status_t ams_util_print_module_info(struct ams_graph *gr);

ams_status_t ams_util_print_connection_info(struct ams_graph *gr);

ams_status_t ams_util_check_unique_id(struct ams_graph *gr, uint32_t id);

ams_status_t ams_util_check_unique_module_id(struct ams_graph *gr, struct ams_module_el *m_el);

ams_status_t ams_util_check_unique_endpoint_id(struct ams_graph *gr, struct ams_endpoint_el *ep_el);

uint32_t ams_util_graph_get_hw_handle(
    ams_session_t amss,
    ams_graph_handle_t gh);

ams_status_t ams_util_graph_serialize(
    struct ams_graph *gr,
    void *ser_buf,
    uint32_t ser_buf_size);

ams_status_t ams_util_graph_deserialize(
    struct ams_graph *gr,
    void *ser_buf,
    uint32_t ser_buf_size);

void *ams_util_ssr_get_handle(ams_session_t amss);

ams_status_t ams_util_check_graph_tdm_intf(
    ams_session_t amss,
    struct ams_graph *pgr_self,
    ams_endpoint_t *e);

ams_status_t ams_util_update_graph_fwinfo_on_ssr(struct ams_graph *gr);

struct ams_endpoint_el *ams_util_graph_get_endpoint_by_port_id(struct ams_graph *gr, uint32_t id);

ams_status_t ams_util_check_graph_property(struct ams_graph *gr, ams_graph_property_t *prop);

ams_status_t ams_util_session_restore_graph_info(struct ams_session *pses, ams_cached_graph_descr_t *param, struct ams_graph **ppgr);

struct ams_graph *ams_util_graph_list_find_graph_fw_handle(
    ams_session_t amss,
    uint32_t fw_handle);

ams_status_t ams_util_memcpy(void *dst, uint32_t dst_size,
                             const void *src, uint32_t src_size);

#endif

/*
 **************************************************************************************************
 * Copyright (c) 2014-2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include <VideoComDef.h>
#include <assert.h>
#include <stdio.h>

#include "list.h"

BOOL list_is_empty(LIST_NODE *p_list_head)
{
   assert(p_list_head != NULL);

   return (p_list_head->p_next_node == p_list_head);
}

void list_init(LIST_NODE *p_list_head)
{
   assert(p_list_head != NULL);

   p_list_head->p_next_node = p_list_head;
   p_list_head->p_prev_node = p_list_head;
}

void list_insert_head(LIST_NODE *p_list_node,LIST_NODE *p_list_head )
{
    assert(p_list_head != NULL);
    assert(p_list_node != NULL);

    p_list_node->p_prev_node = p_list_head;
    p_list_node->p_next_node = p_list_head->p_next_node;

    p_list_head->p_next_node->p_prev_node = p_list_node;
    p_list_head->p_next_node = p_list_node;
}

void list_insert_tail(LIST_NODE *p_list_node, LIST_NODE *p_list_head)
{
    assert(p_list_head != NULL);
    assert(p_list_node != NULL);

    p_list_node->p_prev_node = p_list_head->p_prev_node;
    p_list_node->p_next_node = p_list_head;

    p_list_head->p_prev_node->p_next_node = p_list_node;
    p_list_head->p_prev_node = p_list_node;
}

void list_insert_after(LIST_NODE *p_list_head,
                       LIST_NODE   *p_list_node_new,
                       LIST_NODE   *p_list_node_ref)
{
    assert(p_list_head != NULL);
    assert(p_list_node_new != NULL);

    if (list_is_empty(p_list_head))
    {
       // list empty

       assert(p_list_node_ref == NULL);
       list_insert_head(p_list_head, p_list_node_new);
    }
    else
    {
       // list not empty

       assert(list_check_node(p_list_head, p_list_node_ref));

       p_list_node_new->p_prev_node = p_list_node_ref;
       p_list_node_new->p_next_node = p_list_node_ref->p_next_node;

       p_list_node_ref->p_next_node = p_list_node_new;
       p_list_node_new->p_next_node->p_prev_node = p_list_node_new;
    }
}

void list_insert_before(LIST_NODE *p_list_head,
                        LIST_NODE   *p_list_node_new,
                        LIST_NODE   *p_list_node_ref)
{
    assert(p_list_head != NULL);
    assert(p_list_node_new != NULL);

    if (list_is_empty(p_list_head))
    {
       // list empty

       assert(p_list_node_ref == NULL);
       list_insert_head(p_list_head, p_list_node_new);;
    }
    else
    {
       // list not empty
       assert(list_check_node(p_list_head, p_list_node_ref));

       p_list_node_new->p_next_node = p_list_node_ref;
       p_list_node_new->p_prev_node = p_list_node_ref->p_prev_node;

       p_list_node_ref->p_prev_node = p_list_node_new;
       p_list_node_new->p_prev_node->p_next_node = p_list_node_new;
    }
}

void list_remove_node(LIST_NODE *p_list_head,
                      LIST_NODE   *p_list_node)
{
    assert(p_list_head != NULL);
    assert(p_list_node != NULL);

    assert(p_list_head->p_next_node != p_list_head);
    assert(list_check_node(p_list_head, p_list_node));

    p_list_node->p_prev_node->p_next_node = p_list_node->p_next_node;
    p_list_node->p_next_node->p_prev_node = p_list_node->p_prev_node;
}

LIST_NODE *list_remove_head(LIST_NODE *p_list_head)
{
    LIST_NODE *p_list_node;

    assert(p_list_head != NULL);
    assert(p_list_head->p_next_node != p_list_head);

    p_list_node = p_list_head->p_next_node;

    list_remove_node(p_list_head, p_list_node);

    return p_list_node;
}

LIST_NODE *list_remove_tail(LIST_NODE *p_list_head)
{
    LIST_NODE *p_list_node;

    assert(p_list_head != NULL);
    assert(p_list_head->p_next_node != p_list_head);

    p_list_node = p_list_head->p_prev_node;

    list_remove_node(p_list_head, p_list_node);

    return p_list_node;
}

LIST_NODE *list_search_node(LIST_NODE *p_list_head,
                            void        *p_key_value,
                            PFN_COMPARE *pfn_compare)
{
    LIST_NODE *p_list_node_tmp;

    assert(p_list_head != NULL);

    p_list_node_tmp = p_list_head->p_next_node;

    while (p_list_node_tmp != p_list_head)
    {
       if (pfn_compare(p_key_value, p_list_node_tmp))
       {
          break;
       }

       p_list_node_tmp = p_list_node_tmp->p_next_node;
    }

    return p_list_node_tmp;
}

LIST_NODE *list_get_head(LIST_NODE *p_list_head)
{
   assert(p_list_head->p_next_node != p_list_head);

   return p_list_head->p_next_node;
}

LIST_NODE *list_get_tail(LIST_NODE *p_list_head)
{
   assert(p_list_head->p_next_node != p_list_head);

   return p_list_head->p_prev_node;
}

LIST_NODE *list_get_next_node(LIST_NODE *p_list_node)
{
    assert(p_list_node != NULL);

    return p_list_node->p_next_node;
}

LIST_NODE *list_get_prev_node(LIST_NODE *p_list_node)
{
    assert(p_list_node != NULL);

    return p_list_node->p_prev_node;
}

unsigned int list_get_size(LIST_NODE *p_list_head)
{
    unsigned int list_size = 0;
    LIST_NODE *p_list_node_tmp;

    assert(p_list_head != NULL);

    p_list_node_tmp = p_list_head->p_next_node;

    while (p_list_node_tmp != p_list_head)
    {
       p_list_node_tmp = p_list_node_tmp->p_next_node;

       list_size++;
    }

    return list_size;
}

BOOL list_check_node(LIST_NODE *p_list_head, LIST_NODE *p_list_node)
{
    LIST_NODE *p_list_node_tmp;

    assert(p_list_head != NULL);
    assert(p_list_node != NULL);

    p_list_node_tmp = p_list_head->p_next_node;

    while (p_list_node_tmp != p_list_head)
    {
       if (p_list_node_tmp == p_list_node)
       {
          return true;
       }

       p_list_node_tmp = p_list_node_tmp->p_next_node;
    }

    return false;
}

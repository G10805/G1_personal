/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#ifndef ADR_UTIL_H
#define ADR_UTIL_H

#include <math.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/un.h>  /*struct sockaddr_un*/

#include "log.h"
#include "adr_data_struct.h"

#define PATH_LEN 128
#define VAR_LOG "/var/log"
#define MAX_EPOLL_EVENT 50
#define EPOLL_TIMEOUT 1000     //1s
#define nsec_2_sec 1000000000
#define nsec_2_msec 1000000

#define rtc_init_tm(tm, y, m, d, h, mi, s)    \
    do {                                        \
        (tm)->tm_year = (y) - 1900;             \
        (tm)->tm_mon = (m) - 1;                 \
        (tm)->tm_mday = (d);                    \
        (tm)->tm_hour = (h);                    \
        (tm)->tm_min = (mi);                    \
        (tm)->tm_sec = (s);                     \
    } while (0)

/*
  * @ get ring arrary index,
  * member:
  *     @len: the length of arrary
  *     @cur: cur positoon
  *     @offset: the offset of @in
  *     @out: the final index of the ring
  */
#define get_ring_idx(len, cur, offset, out)     \
        do                                      \
        {                                       \
            if ((cur + offset) >= len) {        \
                out = cur + offset - len;       \
            } else if((cur + offset) < 0) {     \
                    out = len + (cur + offset); \
            } else {                            \
                out = cur + offset;             \
            }                                  \
        }while (0)

#define get_ring_avilable_len(len, start, end, out)     \
        do                                      \
        {                                       \
            out = end + len - start;            \
            if (out >= len)                     \
                out -= len;                     \
        }while (0)

int set_rtc_time(struct rtc_time *tm);
int get_rtc_time(struct rtc_time *tm);
void open_rtc_dev();
void close_rtc_dev();

void get_boot_time(struct timespec* boottime);
void diff_tm_timespec (struct tm *param1,
        struct timespec* param2, struct timespec* out);
void diff_timespec_timespec (struct timespec *param1,
        struct timespec* param2, struct timespec* out);


double str2float(const char* p, const char* end);
int str2int(const char* p, const char* end);


/** \class module_list
 *
 * \brief doubly-linked list
 *
 * The list head is of "struct wl_list" type, and must be initialized
 * using wl_list_init().  All entries in the list must be of the same
 * type.  The item type must have a "struct wl_list" member. This
 * member will be initialized by wl_list_insert(). There is no need to
 * call wl_list_init() on the individual item. To query if the list is
 * empty in O(1), use wl_list_empty().
 *
 * Let's call the list reference "struct wl_list foo_list", the item type as
 * "item_t", and the item member as "struct wl_list link".
 *
 * The following code will initialize a list:
 * \code
 * struct wl_list foo_list;
 *
 * struct item_t {
 *     int foo;
 *     struct wl_list link;
 * };
 * struct item_t item1, item2, item3;
 *
 * wl_list_init(&foo_list);
 * wl_list_insert(&foo_list, &item1.link);    // Pushes item1 at the head
 * wl_list_insert(&foo_list, &item2.link);    // Pushes item2 at the head
 * wl_list_insert(&item2.link, &item3.link);    // Pushes item3 after item2
 * \endcode
 *
 * The list now looks like [item2, item3, item1]
 *
 * Iterate the list in ascending order:
 * \code
 * item_t *item;
 * wl_list_for_each(item, foo_list, link) {
 *     Do_something_with_item(item);
 * }
 * \endcode
 */
struct module_list {
    struct module_list *prev;
    struct module_list *next;
};

void
module_list_init(struct module_list *list);

void
module_list_insert(struct module_list *list, struct module_list *elm);

void
module_list_remove(struct module_list *elm);

int
module_list_length(const struct module_list *list);

int
module_list_empty(const struct module_list *list);

void
module_list_insert_list(struct module_list *list, struct module_list *other);

#define module_container_of(ptr, sample, member)                \
    (__typeof__(sample))((char *)(ptr) -                \
                 offsetof(__typeof__(*sample), member))
/* If the above macro causes problems on your compiler you might be
 * able to find an alternative name for the non-standard __typeof__
 * operator and add a special case here */

#define module_list_for_each(pos, head, member)                \
    for (pos = module_container_of((head)->next, pos, member);    \
         &pos->member != (head);                    \
         pos = module_container_of(pos->member.next, pos, member))

#define module_list_for_each_safe(pos, tmp, head, member)            \
    for (pos = module_container_of((head)->next, pos, member),        \
         tmp = module_container_of((pos)->member.next, tmp, member);    \
         &pos->member != (head);                    \
         pos = tmp,                            \
         tmp = module_container_of(pos->member.next, tmp, member))

#define module_list_for_each_reverse(pos, head, member)            \
    for (pos = module_container_of((head)->prev, pos, member);    \
         &pos->member != (head);                    \
         pos = module_container_of(pos->member.prev, pos, member))

#define module_list_for_each_reverse_safe(pos, tmp, head, member)        \
    for (pos = module_container_of((head)->prev, pos, member),    \
         tmp = module_container_of((pos)->member.prev, tmp, member);    \
         &pos->member != (head);                    \
         pos = tmp,                            \
         tmp = module_container_of(pos->member.prev, tmp, member))

#ifndef container_of
#define container_of(ptr, type, member) ({                \
        const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])
#endif

unsigned char sentence_checksum_calc(const char *sentence,int len);

char* get_log_path();
int close_log_file(FILE *fp);
int open_log_file(char *suffix, FILE **fp);
int safe_recvfrom(int sockfd, char* buf, int len);
int safe_sendto(int fd, const struct sockaddr_un *addr, const char* buff, int len);
int set_fcntl(int fd, int mode);
int epoll_add_fd(int epfd, int fd);

#endif

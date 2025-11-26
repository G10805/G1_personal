/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t NlinterceptorMsg;

#define INTERCEPTOR_REQ_BASE                                                        ((NlinterceptorMsg)(0x0000))

#define INTERCEPTOR_CREATE_SOCKET_REQ                                               (NlinterceptorMsg)(INTERCEPTOR_REQ_BASE + 0x0000)
#define INTERCEPTOR_CLOSE_SOCKET_REQ                                                (NlinterceptorMsg)(INTERCEPTOR_REQ_BASE + 0x0001)
#define INTERCEPTOR_SUBSCRIBE_GROUP_REQ                                             (NlinterceptorMsg)(INTERCEPTOR_REQ_BASE + 0x0002)
#define INTERCEPTOR_UNSUBSCRIBE_GROUP_REQ                                           (NlinterceptorMsg)(INTERCEPTOR_REQ_BASE + 0x0003)

#define INTERCEPTOR_REQ_COUNT                                                       (INTERCEPTOR_UNSUBSCRIBE_GROUP_REQ - INTERCEPTOR_CREATE_SOCKET_REQ + 1)
#define IS_INTERCEPTOR_REQ(t)                                                       (((t) >= INTERCEPTOR_CREATE_SOCKET_REQ) && ((t) <= INTERCEPTOR_UNSUBSCRIBE_GROUP_REQ))
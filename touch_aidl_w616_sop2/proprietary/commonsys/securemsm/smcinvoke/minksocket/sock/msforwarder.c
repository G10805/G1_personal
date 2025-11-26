/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "object.h"
#include "check.h"
#include "cdefs.h"
#include "heap.h"
#include "lxcom_sock.h"
#include "msforwarder.h"
#include "memscpy.h"
#include "logging.h"

static inline int atomic_add(int *pn, int n)
{
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

/*================================================================
 * MSForwarder
 *================================================================*/

static inline void
MSForwarder_delete(MSForwarder *me)
{
  if (me->conn) {
    //Send a close to release our handle
    if (me->handle >= 0) {
      MinkSocket_sendClose(me->conn, me->handle);
    }
    MinkSocket_unregisterForwarder(me->conn);
  }
  heap_free(me);
}

int MSForwarder_detach(MSForwarder *me) {
  int handle = me->handle;
  if (me->conn) {
      MinkSocket_unregisterForwarder(me->conn);
      me->conn = NULL;
  }
  return handle;
}

static int32_t
MSForwarder_invoke(void *cxt, ObjectOp op, ObjectArg *args, ObjectCounts k)
{
  MSForwarder *me = (MSForwarder*) cxt;
  ObjectOp method = ObjectOp_methodID(op);

  if (ObjectOp_isLocal(op)) {
    switch (method) {
    case Object_OP_retain:
      if (me->refs > INT_MAX - 1)
        return Object_ERROR_MAXDATA;
      atomic_add(&me->refs, 1);
      return Object_OK;

    case Object_OP_release:
      if (me->refs < 1)
        return Object_ERROR_MAXDATA;
      if (atomic_add(&me->refs, -1) == 0) {
        MSForwarder_delete(me);
      }
      return Object_OK;

    case Object_OP_unwrapFd:
      if (k != ObjectCounts_pack(0, 1, 0, 0)) {
        break;
      }
      int fd = -1;
      memscpy(args[0].b.ptr, args[0].b.size, &fd, sizeof(fd));
      return Object_OK;
    }

    return Object_ERROR;
  }

  return MinkSocket_invoke(me->conn, me->handle, op, args, k);
}

MSForwarder *MSForwarderFromObject(Object obj)
{
  return (obj.invoke == MSForwarder_invoke ? (MSForwarder*) obj.context : NULL);
}

Object MSForwarder_new(MinkSocket *conn, int handle)
{
  MSForwarder *me = HEAP_ZALLOC_REC(MSForwarder);

  if (!me) {
    return Object_NULL;
  }

  me->refs = 1;
  me->handle = handle;
  me->conn = conn;
  MinkSocket_registerForwarder(conn);
  return (Object) { MSForwarder_invoke, me };
}

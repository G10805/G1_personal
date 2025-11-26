/********************************************************************
 Copyright (c) 2018 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <unistd.h>
#include "memscpy.h"
#include "object.h"
#include "heap.h"
#include "fdwrapper.h"

static inline int atomic_add(int *pn, int n)
{
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

/*================================================================
 * DescriptorObject
 *================================================================*/

static inline void
FdWrapper_delete(FdWrapper *me)
{
  close(me->descriptor);
  heap_free(me);
}

static int32_t
FdWrapper_invoke(void *cxt, ObjectOp op, ObjectArg *args, ObjectCounts k)
{
  FdWrapper *me = (FdWrapper*) cxt;
  ObjectOp method = ObjectOp_methodID(op);

  switch (method) {
  case Object_OP_retain:
    atomic_add(&me->refs, 1);
    return Object_OK;

  case Object_OP_release:
    if (atomic_add(&me->refs, -1) == 0) {
      FdWrapper_delete(me);
    }
    return Object_OK;

  case Object_OP_unwrapFd:
    if (k != ObjectCounts_pack(0, 1, 0, 0)) {
      break;
    }
    memscpy(args[0].b.ptr, args[0].b.size,
            &me->descriptor, sizeof(me->descriptor));
    return Object_OK;
  }

  return Object_ERROR;
}

FdWrapper *FdWrapperFromObject(Object obj)
{
  return (obj.invoke == FdWrapper_invoke ? (FdWrapper*) obj.context : NULL);
}

Object FdWrapper_new(int fd)
{
  FdWrapper *me = HEAP_ZALLOC_REC(FdWrapper);

  if (!me) {
    return Object_NULL;
  }

  me->refs = 1;
  me->descriptor = fd;
  return (Object) { FdWrapper_invoke, me };
}

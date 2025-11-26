/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include "msforwarder.h"
#include "qtest.h"
#include "heap.h"
#include "simheap.h"

static inline uint32_t readNum(const void *p) {
  return *((const uint32_t *)p);
}

static inline void writeNum(void *p, uint32_t i) {
  *((uint32_t *)p) = i;
}

static inline int atomic_add(int *pn, int n)
{
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

uintptr_t gMinkPeerUIDTLSKey = (uintptr_t) readNum;
uintptr_t gMinkPeerGIDTLSKey = (uintptr_t) writeNum;

static inline Object Foo_new(void);

typedef struct {
  int refs;
} Opener;

#define IOpener_OP_open 0
#define IID_Foo 0
static inline Object Opener_new(void);
static int32_t IOpener_invoke(ObjectCxt cxt, ObjectOp op,
                              ObjectArg *args, ObjectCounts k)
{
  Opener *me = (Opener *)cxt;

  switch (ObjectOp_methodID(op)) {
  case Object_OP_retain:
    atomic_add(&me->refs, 1);
    return Object_OK;

  case Object_OP_release:
    if (atomic_add(&me->refs, -1) == 0) {
      heap_free(me);
    }
    return Object_OK;

  case IOpener_OP_open:
    {
      int guid = readNum(args[0].bi.ptr);
      if (guid == IID_Foo) {
        args[1].o = Foo_new();
      } else {
        return Object_ERROR_BADOBJ;
      }
      break;
    }
  }
  return Object_OK;
}

static inline Object Opener_new(void)
{
  Opener *me = HEAP_ZALLOC_TYPE(Opener);
  me->refs = 1;
  return (Object){ IOpener_invoke, me};
}

static int32_t IOpener_open(Object me, int openid, Object *po)
{
  ObjectArg a[2];
  a[0].bi = (ObjectBufIn) { &openid, sizeof(int32_t) };

  int32_t res = Object_invoke(me, IOpener_OP_open, a,
                              ObjectCounts_pack(1,0,0,1));
  *po = a[1].o;
  return res;
}


typedef struct {
  int refs;
  int id;
} Foo;

#define Foo_OP_getId 0
#define Foo_OP_setId 1
#define FOO_MAGIC_ID 894848

static int32_t Foo_invoke(ObjectCxt cxt, ObjectOp op,
                          ObjectArg *args, ObjectCounts k)
{
  Foo *me = (Foo *)cxt;

  switch (ObjectOp_methodID(op)) {
  case Object_OP_retain:
    atomic_add(&me->refs, 1);
    return Object_OK;

  case Object_OP_release:
    if (atomic_add(&me->refs, -1) == 0) {
      heap_free(me);
    }
    return Object_OK;

  case Foo_OP_getId:
    writeNum(args[0].b.ptr, me->id);
    break;

  case Foo_OP_setId:
    me->id = readNum(args[0].bi.ptr);
    break;
  }

  return 0;
}

static inline Object Foo_new(void)
{
  Foo *me = HEAP_ZALLOC_TYPE(Foo);
  me->refs = 1;
  me->id = FOO_MAGIC_ID;
  return (Object){ Foo_invoke, me};
}

static inline int Foo_refs(Object obj) {
  Foo *me = (Foo *)obj.context;
  return me->refs;
}

static inline int Foo_getId(Object me, uint32_t *pid)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { pid, sizeof(uint32_t) };

  return Object_invoke(me, Foo_OP_getId, a, ObjectCounts_pack(0,1,0,0));
}

static inline int Foo_setId(Object me, uint32_t id)
{
  ObjectArg a[1];
  a[0].bi = (ObjectBufIn) { &id, sizeof(uint32_t) };

  return Object_invoke(me, Foo_OP_setId, a, ObjectCounts_pack(1,0,0,0));
}


int main(void)
{
  Object opener_svc = Opener_new();
  MinkSocket *c1 = MinkSocket_new(Object_NULL);
  MinkSocket *c2 = MinkSocket_new(opener_svc);
  int sv[2];

  //create a sockpair
  qt_assert(0 == socketpair(AF_UNIX, SOCK_STREAM, 0, sv));

  MinkSocket_start(c1, sv[0]);
  MinkSocket_start(c2, sv[1]);

  Object opener = MSForwarder_new(c1, 0);

  Object foo;
  qt_eqi(0, IOpener_open(opener, 0, &foo));

  uint32_t id=0;
  Foo_getId(foo, &id);
  qt_eqi(FOO_MAGIC_ID, id);

  Foo_setId(foo, 93499);
  Foo_getId(foo, &id);
  qt_eqi(93499, id);

  qt_eqi(1, Foo_refs(foo));
  Object_release(foo);

  //Release objects
  Object_release(opener);
  Object_release(opener_svc);

  MinkSocket_release(c1);
  MinkSocket_release(c2);
  simheap_checkLeaks(0);

  return 0;
}

/********************************************************************
 Copyright (c) 2018 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <unistd.h>
#include <time.h>
#include "ObjectTableMT_test.h"
#include "qtest.h"
#include "minkipc.h"
#include "msforwarder.h"
#include "heap.h"
#include "minksocket.c"

static inline Object Foo_new(void);

static inline uint32_t readNum(const void *p) {
  return *((const uint32_t *)p);
}

static inline void writeNum(void *p, uint32_t i) {
  *((uint32_t *)p) = i;
}

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
  Object obj;
} Foo;

#define Foo_OP_getId 0
#define Foo_OP_setId 1
#define Foo_OP_getObject 2
#define Foo_OP_setObject 3

#define FOO_MAGIC_ID 894848
static inline int Foo_getId(Object me, uint32_t *pid);

static Foo sFoo;

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

     if (!Object_isNull(me->obj)) {
        uint32_t id;
        Foo_getId(me->obj, &id);
      }

      Object_ASSIGN_NULL(me->obj);
      if (me != &sFoo) {
        heap_free(me);
      }
    }
    return Object_OK;

  case Foo_OP_getId:
    writeNum(args[0].b.ptr, me->id);
    break;

  case Foo_OP_setId:
    me->id = readNum(args[0].bi.ptr);
    break;

  case Foo_OP_getObject:
    args[0].o = me->obj;
    me->obj = Object_NULL;
    break;

  case Foo_OP_setObject:
    Object_replace(&me->obj, args[0].o);
    break;
  }

  return 0;
}

static inline Object Foo_local(void)
{
  Foo *me = &sFoo;
  me->refs = 1;
  me->id = FOO_MAGIC_ID;
  return (Object){ Foo_invoke, me};
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


static inline int Foo_setObject(Object me, Object obj)
{
  ObjectArg a[1];
  a[0].o = obj;
  return Object_invoke(me, Foo_OP_setObject, a, ObjectCounts_pack(0,0,1,0));
}

static inline int Foo_getObject(Object me, Object *obj)
{
  ObjectArg a[1];
  int32_t err = Object_invoke(me, Foo_OP_getObject, a, ObjectCounts_pack(0,0,0,1));
  *obj = a[0].o;
  return err;
}

#define qt_local(o) qt_assert(MSForwarderFromObject(o) == NULL)
#define qt_remote(o) qt_assert(MSForwarderFromObject(o) != NULL)

int main(void)
{
  Object opener = Object_NULL;
  Object opener_svc = Opener_new();
  MinkIPC *server = MinkIPC_startService("/tmp/testfile2", opener_svc);
  MinkIPC *client = MinkIPC_connect("/tmp/testfile2", &opener);

  qt_remote(opener);

  Object_release(opener_svc);

  Object remoteFoo;
  qt_eqi(0, IOpener_open(opener, 0, &remoteFoo));
  qt_remote(remoteFoo);
  Object_release(opener);

  // create a local foo,
  // save and get back from the remote side
  Object localFoo = Foo_local();
  qt_local(localFoo);
  qt_eqi(0, Foo_setObject(remoteFoo, localFoo));
  Object_release(localFoo);

  qt_eqi(1, ((Foo *)localFoo.context)->refs);

  localFoo = Object_NULL;

  qt_eqi(0, Foo_getObject(remoteFoo, &localFoo));
  Object_release(localFoo);
  Object_release(remoteFoo);

  //give time for process the release message
  time_t start = time(NULL);
  //wait 2 sec at most
  while((0 != Foo_refs(localFoo) &&
         ((double)(time(NULL) - start)) < 3.0)) {
    sleep(0);
  }

  qt_eqi(0, Foo_refs(localFoo));

  //Release objects
  MinkIPC_release(client);
  MinkIPC_release(server);

  return 0;
}

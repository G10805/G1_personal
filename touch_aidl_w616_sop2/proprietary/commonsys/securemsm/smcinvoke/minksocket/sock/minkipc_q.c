/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include "qtest.h"
#include "heap.h"
#include "minkipc.h"
#include "msforwarder.h"
#include "fdwrapper.h"
#include "lxcom_sock.h"
#include "memscpy.h"

static inline Object Foo_new(void);

static inline uint32_t readNum(const void *p) {
  return *((const uint32_t *)p);
}

static inline void writeNum(void *p, uint32_t i) {
  *((uint32_t *)p) = i;
}

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

typedef struct {
  int refs;
} Opener;

#define FOR_ARGS(ndxvar, counts, section)                       \
  for (size_t ndxvar = ObjectCounts_index##section(counts);     \
       ndxvar < (ObjectCounts_index##section(counts)            \
                 + ObjectCounts_num##section(counts));          \
       ++ndxvar)

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
    case Object_OP_unwrapFd:
      if (k != ObjectCounts_pack(0, 1, 0, 0)) {
        break;
      }
      int fd = -1;
      memscpy(args[0].b.ptr, args[0].b.size, &fd, sizeof(fd));
      return Object_OK;
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

#define Foo_OP_noop 0
#define Foo_OP_getId 1
#define Foo_OP_setId 2
#define Foo_OP_getObject 3
#define Foo_OP_setObject 4
#define Foo_OP_unalignedSet 5
#define Foo_OP_echo 6
#define Foo_OP_bufferONull 7
#define Foo_OP_maxArgs 8
#define Foo_OP_ObjectONull 9

#define FOO_MAGIC_ID 894848
#define TEST_BUFFER "TEST"

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
      Object_ASSIGN_NULL(me->obj);
      heap_free(me);
    }
    return Object_OK;

  case Object_OP_unwrapFd:
    if (k != ObjectCounts_pack(0, 1, 0, 0)) {
      break;
    }
    int fd = -1;
    memscpy(args[0].b.ptr, args[0].b.size, &fd, sizeof(fd));
    return Object_OK;

  case Foo_OP_getId:
    writeNum(args[0].b.ptr, me->id);
    break;

  case Foo_OP_setId:
    me->id = readNum(args[0].bi.ptr);
    break;

  case Foo_OP_getObject:
    args[0].o = me->obj;
    Object_retain(me->obj);
    break;

  case Foo_OP_setObject:
    Object_replace(&me->obj, args[0].o);
    break;

  case Foo_OP_unalignedSet:
    writeNum(args[2].b.ptr, (uint32_t)(uintptr_t)args[1].bi.ptr);
    break;

  case Foo_OP_echo:
    memcpy(args[1].b.ptr, args[0].b.ptr, args[1].b.size);
    break;

  case Foo_OP_bufferONull:
    args[0].b = (ObjectBuf) { TEST_BUFFER, args[0].b.size };
    break;

  case Foo_OP_maxArgs:
    FOR_ARGS(i, k, BO) {
      ObjectBuf* src = &args[i - ObjectCounts_numBI(k)].b;
      memscpy(args[i].b.ptr, args[i].b.size, src->ptr, src->size);
    }
    FOR_ARGS(i, k, OO) {
      Object_replace(&args[i].o, args[i - ObjectCounts_numOI(k)].o);
      args[i].o = args[i - ObjectCounts_numOI(k)].o;
    }
    break;

  case Foo_OP_ObjectONull:
    args[0].o = Object_NULL;
    args[1].o = Foo_new();
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

static inline int Foo_unalignedSet(Object me, uint32_t *pid)
{
  ObjectArg a[3];
  uint32_t id = FOO_MAGIC_ID;
  a[0].bi = (ObjectBufIn) { &id, 1 };
  a[1].bi = (ObjectBufIn) { &id, sizeof(uint32_t) };
  a[2].b = (ObjectBuf) { pid, sizeof(uint32_t) };
  return Object_invoke(me, Foo_OP_unalignedSet, a, ObjectCounts_pack(2,1,0,0));
}

static inline int Foo_bufferONull(Object me, uint32_t size)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { NULL, size * 1 };
  return Object_invoke(me, Foo_OP_bufferONull, a, ObjectCounts_pack(0,1,0,0));
}

static inline int Foo_ObjectONull(Object me, Object* obj_real, Object* obj_null)
{
  ObjectArg a[2];
  int ret_val = Object_invoke(me, Foo_OP_ObjectONull, a, ObjectCounts_pack(0,0,0,2));
  *obj_null = a[0].o;
  *obj_real = a[1].o;
  return ret_val;
}

static inline Object Foo_newFd(void)
{
  int fd = open("/tmp/somefile", O_CREAT|O_RDWR, 0666);
  qt_assert(fd > 0);
  return FdWrapper_new(fd);
}

#define MAX_BUF_SIZE 1024*512
char req[MAX_BUF_SIZE];
char resp[MAX_BUF_SIZE];

static inline void setupArgs(ObjectArg* args, size_t size, int init_val)
{

  args[0].bi.ptr = req;
  args[0].bi.size = size;
  memset(req, init_val, args[0].bi.size);
  args[1].b.ptr = resp;
  args[1].b.size = args[0].bi.size;
  memset(resp, 0, args[0].bi.size);

}

#define qt_local(o) qt_assert(MSForwarderFromObject(o) == NULL)
#define qt_remote(o) qt_assert(MSForwarderFromObject(o) != NULL)
#define qt_fd(o) qt_assert(FdWrapperFromObject(o) != NULL)

int main(void)
{
  Object opener = Object_NULL;
  Object opener_svc = Opener_new();
  MinkIPC *server = MinkIPC_startService("/tmp/testfile", opener_svc);
  MinkIPC *client = MinkIPC_connect("/tmp/testfile", &opener);

  //Shall get OO from server
  Object remoteFoo;
  qt_eqi(0, IOpener_open(opener, 0, &remoteFoo));
  qt_remote(remoteFoo);

  //Shall get BO from server
  uint32_t id=0;
  Foo_getId(remoteFoo, &id);
  qt_eqi(FOO_MAGIC_ID, id);

  //Shall send BO and get back as BI
  Foo_setId(remoteFoo, 93499);
  Foo_getId(remoteFoo, &id);
  qt_eqi(93499, id);

  // Shall send OI (local) and get back as OO
  Object localFoo = Foo_new();
  qt_local(localFoo);
  qt_eqi(0, Foo_setObject(remoteFoo, localFoo));
  Object localFoo2 = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &localFoo2));
  qt_local(localFoo2);
  Object_release(localFoo2);

  localFoo2 = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &localFoo2));
  qt_local(localFoo2);

  Foo_setId(localFoo2, 923);
  Foo_getId(localFoo2, &id);
  qt_eqi(923, id);
  Object_release(localFoo2);

  localFoo2 = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &localFoo2));
  qt_local(localFoo2);

  Foo_setId(localFoo2, 923);
  Foo_getId(localFoo2, &id);
  qt_eqi(923, id);
  Object_release(localFoo2);

  //Shall send OI (remote) and get back as OO
  qt_eqi(0, Foo_setObject(remoteFoo, remoteFoo));
  Object remoteFoo2 = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &remoteFoo2));
  qt_remote(remoteFoo2);
  Foo_setId(remoteFoo2, 923);
  Foo_getId(remoteFoo2, &id);
  qt_eqi(923, id);
  Object_release(remoteFoo2);
  remoteFoo2 = Object_NULL;

  qt_eqi(0, Foo_getObject(remoteFoo, &remoteFoo2));
  qt_remote(remoteFoo2);
  Object_release(remoteFoo2);

  //Shall send OI (local Fd) and get back as OO
  Object localFd = Foo_newFd();
  qt_fd(localFd);
  qt_eqi(0, Foo_setObject(remoteFoo, localFd));
  int ix;
  qt_eqi(Object_OK, Object_unwrapFd(localFd, &ix));
  qt_assert(ix != -1);
  Object_release(localFd);
  qt_assert(fcntl(ix, F_GETFD) == -1);
  ix = -1;
  localFd = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &localFd));
  qt_eqi(Object_OK, Object_unwrapFd(localFd, &ix));
  qt_assert(ix > 0);
  qt_assert(fcntl(ix, F_GETFD) != -1);
  qt_fd(localFd);
  Object_release(localFd);

  ix = -1;
  localFd = Object_NULL;
  qt_eqi(0, Foo_getObject(remoteFoo, &localFd));
  qt_eqi(Object_OK, Object_unwrapFd(localFd, &ix));
  qt_assert(ix > 0);
  qt_assert(fcntl(ix, F_GETFD) != -1);
  qt_fd(localFd);

  Object_release(localFd);

  //Max arg test make sure 60 args over minksocket works
  ObjectArg maxargs[LXCOM_MAX_ARGS];
  ObjectCounts counts = ObjectCounts_pack(15,15,15,15);
  char outbuf[15][4];
  Object testObj = Foo_new();
  memset(outbuf, 0, sizeof(outbuf));
  memset(maxargs, 0, sizeof(maxargs));

  FOR_ARGS(i, counts, BI) {
    maxargs[i].bi = (ObjectBufIn) { TEST_BUFFER, 4 };
  }
  FOR_ARGS(i, counts, BO) {
    maxargs[i].b = (ObjectBuf) { outbuf[i - ObjectCounts_numBI(counts)], 4 };
  }
  FOR_ARGS(i, counts, OI) {
    maxargs[i].o = testObj;
  }
  FOR_ARGS(i, counts, OO) {
    maxargs[i].o = Object_NULL;
  }

  qt_eqi(0, Object_invoke(remoteFoo, Foo_OP_maxArgs, maxargs, counts));

  FOR_ARGS(i, counts, BO) {
    qt_eqi(0, memcmp(maxargs[i].b.ptr,
                     maxargs[i - ObjectCounts_numBI(counts)].b.ptr,
                     maxargs[i].b.size));
  }
  FOR_ARGS(i, counts, OO) {
    qt_assert(!Object_isNull(maxargs[i].o));
    qt_assert(maxargs[i].o.invoke == Foo_invoke);
    Object_release(testObj);
  }
  Object_release(testObj);

  //Test that NULL out objs preserve their order in the
  //out objs list
  Object nullOut = Foo_new();
  Object backup  = nullOut;
  Object realObj = Object_NULL;
  qt_eqi(0, Foo_ObjectONull(remoteFoo, &realObj, &nullOut));
  qt_assert(Object_isNull(nullOut));
  Object_release(backup);
  Object_release(realObj);



  // Create a 1byte and 4byte BIs and make sure
  // the received side buffer is aligned correctly
  uint32_t pval = 0;
  qt_eqi(0, Foo_unalignedSet(remoteFoo, &pval));
  qt_eqi(0, pval%4);

  //Shall roundtrip BI/BO upto 512K
  {
    ObjectArg a[2];

    for (int i=1; i<=16; i++) {
      setupArgs(a, 1<<i, i);
      qt_eqi(0, Object_invoke(remoteFoo, Foo_OP_echo,
                              a, ObjectCounts_pack(1,1,0,0)));
      qt_eqi(0, memcmp(a[0].bi.ptr, a[1].b.ptr, a[0].bi.size));

      setupArgs(a, MAX_BUF_SIZE, 1);
      qt_eqi(0, Object_invoke(remoteFoo, Foo_OP_echo,
                                a, ObjectCounts_pack(1,1,0,0)));
      qt_eqi(0, memcmp(a[0].bi.ptr, a[1].b.ptr, a[0].bi.size));
    }
  }

  //nullptr for a nonzero buffer size is not a valid usecase
  qt_eqi(Object_ERROR_INVALID, Foo_bufferONull(remoteFoo, 4));
  //expected size of 0 and nullptr is a valid usecase
  qt_eqi(0, Foo_bufferONull(remoteFoo, 0));

  //Release objects
  Object_release(localFoo);
  Object_release(remoteFoo);
  Object_release(opener);
  Object_release(opener_svc);

  MinkIPC_release(client);
  MinkIPC_release(server);

  return 0;
}

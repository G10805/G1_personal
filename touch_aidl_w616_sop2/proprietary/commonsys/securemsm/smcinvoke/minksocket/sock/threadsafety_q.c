/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <unistd.h>
#include "threadpool.h"
#include "qtest.h"
#include "minkipc.h"
#include "heap.h"

#define TOINT(p) (*(const uint32_t *)p)
#define TOINT_PTR(p) ((uint32_t *)p)

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

#define test_op_add 0
#define test_op_multiply 1
#define test_op_getobject 2

typedef struct {
  int refs;
} TestState;

static int32_t test_invoke(ObjectCxt cxt, ObjectOp op, ObjectArg *a, ObjectCounts k)
{
  TestState *me = (TestState *)cxt;
  int methodId = ObjectOp_methodID(op);
  switch (methodId) {
  case Object_OP_retain:
    atomic_add(&me->refs, 1);
    return Object_OK;

  case Object_OP_release:
    if (atomic_add(&me->refs, -1) == 0) {
      heap_free(me);
    }
    return Object_OK;

    case test_op_add: {
      //Add two bufIn .. return the value
      uint32_t n1 = TOINT(a[0].bi.ptr);
      uint32_t n2 = TOINT(a[1].bi.ptr);
      uint32_t n3 = TOINT(a[2].bi.ptr);
      *TOINT_PTR(a[3].b.ptr) = n1 + n2 + n3;
      return 0;
    }

    case test_op_multiply: {
      //Add two bufIn .. return the value
      uint32_t n1 = TOINT(a[0].bi.ptr);
      uint32_t n2 = TOINT(a[1].bi.ptr);
      *TOINT_PTR(a[2].b.ptr) = n1 * n2;
      return 0;
    }
    case test_op_getobject: {
      atomic_add(&me->refs, 1);
      a[0].o = (Object) { test_invoke, me };
      return 0;
    }
  }

  return -1;
}

static inline Object Test_new(void)
{
  TestState *me = HEAP_ZALLOC_TYPE(TestState);
  me->refs = 1;
  return (Object){ test_invoke, me};
}

#define TEST_OBJECT ((Object) { test_invoke, NULL })

#define qt__isEQorFailure(a, b)  ((a) == (b) || (b) < 0 )

#define qt_eqif(a, b)   qt__assert_eq(a, b, qt__isEQorFailure, intmax_t, "%"PRIdMAX)

static inline Object test_getCalculator(Object me)
{
  ObjectArg a[1];
  Object_invoke(me, test_op_getobject, a, ObjectCounts_pack(0,0,0,1));
  return a[0].o;
}


static int32_t multiply(Object o, int32_t x, int32_t y) {
    uint32_t resp = 0;
    ObjectArg args[4];
    args[0].b.ptr = &x;
    args[0].b.size = sizeof(x);
    args[1].b.ptr = &y;
    args[1].b.size = sizeof(y);
    args[2].b.ptr = &resp;
    args[2].b.size = sizeof(resp);

    //multiply 2 numbers and return number
    int err = Object_invoke(o, test_op_multiply, args,
                            ObjectCounts_pack(2,1,0,0));
    if (err == 0) {
      return resp;
    }

    return -1;
}

static int32_t add(Object o, int32_t x, int32_t y, int32_t z) {
    uint32_t resp = 0;
    ObjectArg args[4];
    args[0].b.ptr = &x;
    args[0].b.size = sizeof(x);
    args[1].b.ptr = &y;
    args[1].b.size = sizeof(y);
    args[2].b.ptr = &z;
    args[2].b.size = sizeof(z);
    args[3].b.ptr = &resp;
    args[3].b.size = sizeof(resp);

    //add 3 numbers and return sum
    int err = Object_invoke(o, test_op_add, args,
                            ObjectCounts_pack(3,1,0,0));
    if (err == 0) {
      return resp;
    }

    return -1;
}


Object calculator;

static void *workFunc(void *args)
{
  int i = (intptr_t)args;

  qt_eqif(i*231, multiply(calculator, i, 231));

  Object calc1 = test_getCalculator(calculator);
  qt_eqif(i*10, multiply(calc1, i, 10));
  qt_eqif(i+100, add(calc1, i, 10, 90));

  Object calc2 = test_getCalculator(calculator);
  qt_eqif(i*100, multiply(calc2, i, 100));
  qt_eqif(i+200, add(calc1, i, 110, 90));

  Object_release(calc1);
  Object_release(calc2);
  return NULL;
}


int main(void)
{
  Object mainCalc = Test_new();
  MinkIPC *c1 = MinkIPC_startService("/tmp/x1", mainCalc);
  Object_release(mainCalc);

  MinkIPC *c2 = MinkIPC_connect("/tmp/x1", &calculator);

  qt_eqif(100, multiply(calculator, 2, 50));

  ThreadPool *pool = ThreadPool_new();

  for (int i=0; i < 1000; i++) {
    ThreadWork *tw = malloc(sizeof(ThreadWork));
    QNode_construct(&tw->n);
    tw->workFunc = &workFunc;
    tw->args = (void *)(intptr_t)i;
    ThreadPool_queue(pool, tw);
  }

  ThreadPool_wait(pool);

  //Release the forwarder object
  Object_release(calculator);

  MinkIPC_release(c2);
  MinkIPC_release(c1);
}

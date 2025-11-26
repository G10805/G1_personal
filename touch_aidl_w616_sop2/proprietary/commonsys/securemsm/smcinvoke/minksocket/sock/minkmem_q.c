/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include "qtest.h"
#include "simheap.h"
#include "minkipc.h"
#include "lxcom_sock.h"

#define TOINT(p) (*(const uint32_t *)p)
#define TOINT_PTR(p) ((uint32_t *)p)

#define test_op_return_methodid_as_err 1111

static int32_t test_invoke(ObjectCxt cxt, ObjectOp op, ObjectArg *a, ObjectCounts k)
{
  int methodId = ObjectOp_methodID(op);
  switch (methodId) {
  case 1: {
    //One bufIn
    uint32_t n = TOINT(a[0].bi.ptr);
    return n;
  }
  case 2: {
    //Two bufIn .. return 2nd one
    uint32_t n = TOINT(a[1].bi.ptr);
    return n;
  }

  case 3: {
    //bufIn .. return double the value
    uint32_t n = TOINT(a[0].bi.ptr);
    *TOINT_PTR(a[1].b.ptr) = n*2;
    return 0;
  }

  case 4: {
    //Add two bufIn .. return the value
    uint32_t n1 = TOINT(a[0].bi.ptr);
    uint32_t n2 = TOINT(a[1].bi.ptr);
    *TOINT_PTR(a[2].b.ptr) = n1 + n2;
    return 0;
  }

  case test_op_return_methodid_as_err:
    return test_op_return_methodid_as_err;

    //default:
    //printf("Invoked: %d\n", methodId);
  }
  return 0;
}

#define TEST_OBJECT ((Object) { test_invoke, NULL })

#define qt__isEQorFailure(a, b)  ((a) == (b) || (b) < 0 )

#define qt_eqif(a, b)   qt__assert_eq(a, b, qt__isEQorFailure, intmax_t, "%"PRIdMAX)

int main(void)
{
  for (int i=0; i < 100; i++) {

    //printf("fail at: %d\n", i);
    simheap_failAt(i, 0);

    Object o = Object_NULL;
    MinkIPC *c1 = MinkIPC_startService("/tmp/websec", TEST_OBJECT);
    if (!c1) {
      continue;
    }

    MinkIPC *c2 = MinkIPC_connect("/tmp/websec", &o);
    if (!c2) {
      MinkIPC_release(c1);
      continue;
    }

    if (Object_isNull(o)) {
      MinkIPC_release(c1);
      MinkIPC_release(c2);
      continue;
    }

    //printf("real start\n");

    //send no-op
    qt_eqif(Object_OK, Object_invoke(o, 0, NULL, 0));

    qt_eqif(test_op_return_methodid_as_err,
           Object_invoke(o, test_op_return_methodid_as_err, NULL, 0));

    //max args is limited to 60, with no way to test for that since
    //objectCounts_pack will cap at 60.
    ObjectArg maxargs[LXCOM_MAX_ARGS+1];
    memset(maxargs, 0, sizeof(maxargs));

    //bad args values
    maxargs[2].b.size=104040904;
    qt_eqif(Object_ERROR_INVALID,
           Object_invoke(o, 0, maxargs, ObjectCounts_pack(6,6,0,0)));


    uint32_t v[4] = {100, 101, 102, 103};
    ObjectArg args[4];
    args[0].b.ptr = v;
    args[0].b.size = sizeof(*v);
    args[1].b.ptr = v+1;
    args[1].b.size = sizeof(*v);
    args[2].b.ptr = v+2;
    args[2].b.size = sizeof(*v);

    qt_eqif(101, Object_invoke(o, 2, args, ObjectCounts_pack(2,0,0,0)));

    //send a number and get back its double
    qt_eqif(0, Object_invoke(o, 3, args, ObjectCounts_pack(1,1,0,0)));

    //add 2 numbers and return sum
    qt_eqif(0, Object_invoke(o, 4, args, ObjectCounts_pack(2,1,0,0)));


    //sleep(0);

    //Release the forwarder object
    Object_release(o);

    MinkIPC_release(c1);
    MinkIPC_release(c2);
    simheap_checkLeaks(0);
  }
  return 0;
}

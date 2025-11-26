/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include "msforwarder.h"
#include "qtest.h"

MinkSocket conn;

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

struct MinkSocket {
  int refs;
  int msForwarderCount;
};

static MinkSocket *MinkSocket_init(MinkSocket *me)
{
  me->refs = 1;
  return me;
}

void MinkSocket_retain(MinkSocket *me)
{
  ++me->refs;
}

void MinkSocket_release(MinkSocket *me)
{
  --me->refs;
}

void MinkSocket_registerForwarder(MinkSocket *me)
{
  atomic_add(&me->msForwarderCount, 1);
  atomic_add(&me->refs, 1);
}

void MinkSocket_unregisterForwarder(MinkSocket *me) {
  atomic_add(&me->msForwarderCount, -1);
  atomic_add(&me->refs, -1);
}


static int invoke_count = 0;
int32_t MinkSocket_invoke(MinkSocket *me, int32_t h,
                ObjectOp op, ObjectArg *args, ObjectCounts k)
{
  invoke_count++;
  return 0;
}

static int close_count = 0;
int32_t MinkSocket_sendClose(MinkSocket *me, int h)
{
  close_count++;
  return 0;
}


int main(void)
{
  MinkSocket_init(&conn);

  Object o = MSForwarder_new(&conn, 0);
  Object_invoke(o, 1, NULL, 0);
  qt_eqi(conn.refs, 2);
  Object_release(o);
  qt_eqi(conn.refs, 1);
  qt_eqi(invoke_count, 1);
  qt_eqi(close_count, 1);

  return 0;
}

/********************************************************************
 Copyright (c) 2016, 2024 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "threadpool.h"
#include "heap.h"
#include "logging.h"

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

struct  ThreadPool {
  int refs;
  pthread_t aThreads[THREADPOOL_MAX_THREADS];
  QList qlWork;
  pthread_mutex_t qMtx;
  pthread_cond_t qCnd;
  bool bDone;
  int nThreads;
  int nIdleThreads;
  bool bNeedsDelete;
};


static int QList_count(QList *pq)
{
  QNode *pn;
  int num = 0;

  QLIST_FOR_ALL(pq, pn) {
    ++num;
  }
  return num;
}

static void ThreadPool_delete(ThreadPool *me)
{
  pthread_mutex_destroy(&me->qMtx);
  pthread_cond_destroy(&me->qCnd);
  QNode *qn;
  QNode *qnn;
  QLIST_NEXTSAFE_FOR_ALL(&me->qlWork, qn, qnn) {
    QNode_dequeue(qn);
    free(qn);
  }

  heap_free(me);
  //LOGE("%s pool =%p\n", __func__, me);
}

static void *thread_entrypoint(void *arg)
{
  ThreadPool *me = (ThreadPool *)arg;
  ThreadWork *w = NULL;
  //LOGE("%s pool = %p\n", __func__, me);
  while (!me->bDone) {
    pthread_mutex_lock(&me->qMtx);

    while (!me->bDone && QList_isEmpty(&me->qlWork)) {
      ++me->nIdleThreads;
      pthread_cond_wait(&me->qCnd, &me->qMtx);
      --me->nIdleThreads;
    }

    if (me->bDone) {
      pthread_mutex_unlock(&me->qMtx);
      //LOGE("%s pool %p, thread done\n", __func__, me);
      pthread_exit(NULL);
    }

    w = (ThreadWork *)QList_pop(&me->qlWork);
    if (w == NULL) {
      pthread_mutex_unlock(&me->qMtx);
      pthread_exit(NULL);
    }

    pthread_mutex_unlock(&me->qMtx);
    w->workFunc(w->args);
    free(w);
  }

  pthread_mutex_unlock(&me->qMtx);
  if (me->bNeedsDelete) {
    ThreadPool_delete(me);
    pthread_detach(pthread_self());
  }
  pthread_exit(NULL);
}

static void ThreadPool_createThread(ThreadPool *me)
{
  for (int i=0; i < THREADPOOL_MAX_THREADS; ++i) {
    if (me->aThreads[i] == 0) {
  //LOGE("%s pool %p\n", __func__, me);
      pthread_create(&me->aThreads[i], NULL, &thread_entrypoint, me);
      ++me->nThreads;
      return;
    }
  }
}


ThreadPool *ThreadPool_new(void)
{
  ThreadPool *me = HEAP_ZALLOC_TYPE(ThreadPool);
  if (me == NULL) {
    return NULL;
  }

  if (0 != pthread_mutex_init(&me->qMtx, NULL)) {
    goto bail;
  }

  if (0 != pthread_cond_init(&me->qCnd, NULL)) {
    goto bail;
  }
  //LOGE("%s pool %p\n", __func__, me);

  QList_construct(&me->qlWork);
  me->bDone = false;
  me->refs = 1;
  return me;

 bail:
  ThreadPool_release(me);
  return NULL;
}

void ThreadPool_retain(ThreadPool *me)
{
  atomic_add(&me->refs, 1);
}

void ThreadPool_stop(ThreadPool *me)
{
  pthread_mutex_lock(&me->qMtx);
  me->bDone = true;
  pthread_cond_broadcast(&me->qCnd);
  pthread_mutex_unlock(&me->qMtx);
  //LOGE("%s pool %p\n", __func__, me);

  for (int i=0; i < THREADPOOL_MAX_THREADS; ++i) {
    if (me->aThreads[i] != 0 && me->aThreads[i] != pthread_self()) {
      pthread_join(me->aThreads[i], NULL);
      me->nThreads--;
    }
  }
}

void ThreadPool_wait(ThreadPool *me)
{
  while (!QList_isEmpty(&me->qlWork)  ||
         me->nThreads != me->nIdleThreads) {
    sleep(0);
  }
}

void ThreadPool_release(ThreadPool *me)
{
  if (atomic_add(&me->refs, -1) == 0) {
    ThreadPool_stop(me);
    if (me->nThreads > 0) {
      me->bNeedsDelete = true;
    } else {
      ThreadPool_delete(me);
    }
  }
}

void ThreadPool_queue(ThreadPool *me, ThreadWork *work)
{
  pthread_mutex_lock(&me->qMtx);
  if (me->bDone) {
    pthread_cond_broadcast(&me->qCnd);
    pthread_mutex_unlock(&me->qMtx);
    free(work);
    return;
  }

  QList_appendNode(&me->qlWork, &work->n);
  if (QList_count(&me->qlWork) > me->nIdleThreads) {
  //LOGE("%s pool %p\n", __func__, me);
    ThreadPool_createThread(me);
  }

  pthread_cond_broadcast(&me->qCnd);
  pthread_mutex_unlock(&me->qMtx);
}

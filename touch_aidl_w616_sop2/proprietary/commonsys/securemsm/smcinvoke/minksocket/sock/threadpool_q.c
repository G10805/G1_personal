/********************************************************************
 Copyright (c) 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "threadpool.h"
#include "qtest.h"

static pthread_mutex_t gMtx = PTHREAD_MUTEX_INITIALIZER;
static int COUNT_TO = 10000;
static int gSum = 0;
static int addme = 1;

static void *workFunc(void *args)
{
   pthread_mutex_lock(&gMtx);
   gSum += *(int *)args;
   pthread_mutex_unlock(&gMtx);
   return NULL;
}


int main(int argc, char **argv)
{
   ThreadPool *pTP = ThreadPool_new();

   for (int i=0; i < COUNT_TO; ++i) {
      ThreadWork *tw = malloc(sizeof(ThreadWork));
      QNode_construct(&tw->n);
      tw->workFunc = &workFunc;
      tw->args = (void *)&addme;

      ThreadPool_queue(pTP, tw);
   }

   ThreadPool_wait(pTP);
   ThreadPool_release(pTP);
   qt_eqi(gSum, COUNT_TO);

   return 0;
}

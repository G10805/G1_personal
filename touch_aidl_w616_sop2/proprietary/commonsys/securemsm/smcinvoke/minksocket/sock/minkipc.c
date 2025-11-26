/********************************************************************
 Copyright (c) 2016-2017, 2021 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "check.h"
#include "heap.h"
#include "minkipc.h"
#include "minksocket.h"
#include "msforwarder.h"
#include "fdwrapper.h"
#include "cdefs.h"
#include "qlist.h"
#include "logging.h"

#define CHECK_CLEAN(expr) \
  do { if (!(expr)) { CHECK_LOG(); goto cleanup; } } while (0)

#if defined(OFFTARGET)
extern
size_t strlcpy(char *dst, const char *src, size_t size);
#endif

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

static bool is_qrtr_sock(int sock) {
  return 0;
}

struct MinkIPC {
  int refs;
  bool bServer;
  bool bReady;
  union {
    struct sockaddr_un addr;
  } sockaddr;
  int sock;
  bool bDone;
  bool bServerDone;
  Object opener;
  pthread_t listenerThread;
  MinkSocket *conn;
  QList servedConnectionsList;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

typedef struct {
  QNode qn;
  MinkSocket *conn;
} ServerNode;

uintptr_t gMinkPeerUIDTLSKey = (uintptr_t) MinkIPC_startService;
uintptr_t gMinkPeerGIDTLSKey = (uintptr_t) MinkIPC_startServiceOnSocket;

static void MinikIPC_cleanupConnections(MinkIPC *me, bool deadOnly) {
  QNode * pqn = NULL;
  QNode * pqn_next = NULL;
  QLIST_NEXTSAFE_FOR_ALL(&me->servedConnectionsList, pqn, pqn_next) {
    ServerNode* node = c_containerof(pqn, ServerNode, qn);
    if (node) {
      if (!MinkSocket_isConnected(node->conn) || !deadOnly) {
        QNode_dequeue(pqn);
        MinkSocket_close(node->conn, Object_ERROR_UNAVAIL);
        MinkSocket_release(node->conn);
        free(node);
      }
    }
  }
}

static ServerNode *ServerNode_new(Object opener)  {
  ServerNode *connection = HEAP_ZALLOC_REC(ServerNode);
  if (!connection) {
    return NULL;
  }

  MinkSocket *sock = MinkSocket_new(opener);
  if (!sock) {
    free(connection);
    return NULL;
  }

  connection->conn = sock;
  return connection;
}

static void *MinkIPC_QRTR_Service(void *pv)
{
  return NULL;
}

static void *MinkIPC_service(void *pv)
{
  MinkIPC *me = (MinkIPC *)pv;

  if (me == NULL) {
    LOGE("Abort as me is NULL unexpectedly");
    return NULL;
  }
  me->bReady = true;
  pthread_cond_signal(&me->cond);

  do {
    int sock = accept(me->sock, NULL, NULL);
    if (sock > 0) {
      ServerNode *node = ServerNode_new(me->opener);
      if (node) {
        pthread_mutex_lock(&me->mutex);
        QList_appendNode(&me->servedConnectionsList, &node->qn);
        MinkSocket_start(node->conn, sock);
        MinikIPC_cleanupConnections(me, true);
        pthread_mutex_unlock(&me->mutex);
      } else {
        shutdown(sock, SHUT_RDWR);
        close(sock);
      }
    }
    if (me == NULL) {
      LOGE("Abort as me is NULL unexpectedly");
      return NULL;
    }
  } while (!me->bDone);
  me->bServerDone = true;
  pthread_cond_signal(&me->cond);

  return NULL;
}

static MinkIPC *
MinkIPC_new(const char *service, int sock, Object opener)
{
  MinkIPC *me = HEAP_ZALLOC_REC(MinkIPC);
  if (!me) {
    return NULL;
  }

  me->refs = 1;
  me->bDone = false;
  me->conn = NULL;
  if (service) {
    me->sockaddr.addr.sun_family = AF_UNIX;
    strlcpy(me->sockaddr.addr.sun_path, service, sizeof(me->sockaddr.addr.sun_path) - 1);
    me->sock = socket(AF_UNIX, SOCK_STREAM, 0);
  } else {
      me->sock = sock;
  }

  QList_construct(&me->servedConnectionsList);
  Object_ASSIGN(me->opener, opener);
  me->bServer =  !(Object_isNull(opener));
  CHECK_CLEAN(me->sock != -1);
  CHECK_CLEAN(!pthread_mutex_init(&me->mutex, NULL));
  CHECK_CLEAN(!pthread_cond_init(&me->cond, NULL));

  return me;

 cleanup:
  LOGE("%s: release minkIPC = %p\n", __func__, me);
  MinkIPC_release(me);
  return NULL;
}

static MinkIPC *
MinkIPC_QRTR_new(int service, int sock, Object opener)
{
  return NULL;
}

#define MAX_QUEUE_LENGTH 5

static MinkIPC *MinkIPC_beginService(const char *service, int sock, Object opener)
{
  MinkIPC *me = NULL;
  if (is_qrtr_sock(sock)) {
    me = MinkIPC_QRTR_new(0, sock, opener);
  } else {
    me = MinkIPC_new(service, sock, opener);
  }

  if (!me) {
    LOGE ("%s: Failed to create new MinkIP...Creturning NULL\n", __func__);
    return NULL;
  }

  pthread_key_create((pthread_key_t*) &gMinkPeerUIDTLSKey, NULL);
  pthread_key_create((pthread_key_t*) &gMinkPeerGIDTLSKey, NULL);

  if (service != NULL) {
    //Recreate the file if one exists already
    unlink(me->sockaddr.addr.sun_path);
    CHECK_CLEAN (!bind(me->sock, (struct sockaddr*)&me->sockaddr.addr,
                   sizeof(me->sockaddr.addr)));
  }

  if (is_qrtr_sock(sock)) {
    CHECK_CLEAN (!pthread_create(&me->listenerThread, NULL, MinkIPC_QRTR_Service, me));
  } else {
    CHECK_CLEAN (!listen(me->sock, MAX_QUEUE_LENGTH) );

    //create a thread to wait for connections...
    CHECK_CLEAN (!pthread_create(&me->listenerThread, NULL, MinkIPC_service, me));
  }

  pthread_mutex_lock(&me->mutex);
  while (!me->bReady) {
    pthread_cond_wait(&me->cond, &me->mutex);
  }
  pthread_mutex_unlock(&me->mutex);

  return me;

 cleanup:
    MinkIPC_release(me);
    return NULL;
}

MinkIPC *MinkIPC_startService(const char *service, Object opener)
{
  return MinkIPC_beginService(service, -1, opener);
}



MinkIPC * MinkIPC_startServiceOnSocket(int sock, Object opener)
{
  return MinkIPC_beginService(NULL, sock, opener);
}



/**
   wait for the service to finish ..
   waits until stopped or the service dies
**/
void MinkIPC_join(MinkIPC *me) {
  if (me->bServer && me->listenerThread) {
    //wait for thread to die
    pthread_join(me->listenerThread, NULL);
  }
}

MinkIPC* MinkIPC_connect_QRTR(int service, Object *obj)
{
  return NULL;
}

MinkIPC* MinkIPC_connect(const char *service, Object *obj)
{
  MinkIPC *me = MinkIPC_new(service, -1, Object_NULL);
  if (!me) {
    return NULL;
  }

  if (connect(me->sock, (struct sockaddr *)&me->sockaddr.addr,
              sizeof(me->sockaddr.addr)) == -1) {
    MinkIPC_release(me);
    return NULL;
  }

  //create a domain
  me->conn = MinkSocket_new(Object_NULL);
  if (me->conn) {
    MinkSocket_start(me->conn, me->sock);
    *obj = MSForwarder_new(me->conn, 0);
  }
  return me;
}

int MinkIPC_getClientInfo(uid_t* uid, gid_t* gid)
{
  gid_t* pgid = (gid_t*) pthread_getspecific((pthread_key_t) gMinkPeerGIDTLSKey);
  uid_t* puid = (uid_t*) pthread_getspecific((pthread_key_t) gMinkPeerUIDTLSKey);

  if (pgid == NULL || puid == NULL)
    return -1;

  *uid = *puid;
  *gid = *pgid;

  return 0;
}

static void MinkIPC_stop(MinkIPC *me)
{
  pthread_mutex_lock(&me->mutex);

  Object_ASSIGN_NULL(me->opener);
  me->bDone = true;

  MinikIPC_cleanupConnections(me, false);

  if (me->sock != -1) {
    shutdown(me->sock, SHUT_RDWR);
    close(me->sock);
    me->sock = -1;
  }

  if (me->conn) {
    MinkSocket_release(me->conn);
    me->conn = NULL;
  }

  pthread_mutex_unlock(&me->mutex);
  if (me->listenerThread) {
    //Wait for thread to die, but we cannot join here, since the caller
    //might have caller MinkIPC_join. So let's use cond for it
    pthread_mutex_lock(&me->mutex);
    while (!me->bServerDone) {
      pthread_cond_wait(&me->cond, &me->mutex);
    }
    pthread_mutex_unlock(&me->mutex);
  }
}

void MinkIPC_retain(MinkIPC *me)
{
  atomic_add(&me->refs, 1);
}

void MinkIPC_release(MinkIPC *me)
{
  if (atomic_add(&me->refs, -1) == 0) {
    MinkIPC_stop(me);
    pthread_mutex_destroy(&me->mutex);
    pthread_cond_destroy(&me->cond);
    heap_free(me);
  }
}

void MinkIPC_wrapFd(int fd, Object *obj) {
  *obj = FdWrapper_new(fd);
}

/********************************************************************
 Copyright (c) 2016-2017, 2020-2022, 2024 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/


#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#if defined(USE_GLIB) && !defined (__aarch64__)
/* FIXME SDX60:
   The Olympic bits/socket.h header is patched out to only include the required
   asm/socket.h if __USE_MISC is defined. Explicitly define this flag as a
   workaround for 32-bit LE targets until the root-issue can be addressed in
   the kernel. Otherwise SO_PEERCRED is not defined. */
#define __USE_MISC 1
#endif
#include <sys/socket.h>

#include <sys/uio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <poll.h>
#include "memscpy.h"
#include "cdefs.h"
#include "check.h"
#include "lxcom_sock.h"
#include "msforwarder.h"
#include "bbuf.h"
#include "ObjectTableMT.h"
#include "threadpool.h"
#include "fdwrapper.h"
#include "logging.h"
int qrtr_sendto(int sock, uint32_t node, uint32_t port, const void *data, unsigned int sz)
{
  return 0;
}

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#define BEGIN_INVOKE_ID 1
#define MAX_OBJECT_COUNT 1024
#define MSG_PREALLOCED_SIZE 1024
#define MAX_BUFFER_ALLOCATION 4*1024*1024 // 4MB: Max page size in linux kernel
#define MAX_UDP_PAYLOAD (64*1024)

#if defined(OE) || defined(OFFTARGET)
struct ucred {
  uint32_t pid;
  uint32_t uid;
  uint32_t gid;
};
#endif

struct MinkSocket {
  int refs;
  int32_t dispatchErr;
  bool bDone;
  ObjectTableMT table;
  int sock;
  int sockPair[2];
  uint32_t invoke_id;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  QList qlInvokes;
  ThreadPool *pool;
  uid_t peer_uid;
  gid_t peer_gid;
  bool peer_available;
  int msForwarderCount;
  uint32_t node;
  uint32_t port;
};

static inline int atomic_add(int *pn, int n) {
  return __sync_add_and_fetch(pn, n);  // GCC builtin
}

static bool is_qrtr_sock(int sock) {
  return 0;
}

//return how much to add to the alignment
#define PADDED(x) ((__typeof__(x))((x) + (((uint64_t)(~(x))+1) & (LXCOM_MSG_ALIGNMENT-1))))

#if 0
static inline size_t PADDED(size_t x)
{
  size_t pad = (uintptr_t) x % LXCOM_MSG_ALIGNMENT;
  if (pad)
    pad = LXCOM_MSG_ALIGNMENT - pad;
  return pad;
}
#endif


typedef union {
  lxcom_msg msg;
  lxcom_hdr hdr;
  uint8_t buf[MSG_PREALLOCED_SIZE];
  uint64_t _aligned_unused;
} msgbuf_t;

typedef struct InvokeInfo {
  QNode qn;
  uint32_t invoke_id;
  int handle;
  ObjectOp op;
  ObjectArg *args;
  ObjectCounts k;
  int result;
  bool bComplete;
  pthread_cond_t cond;
} InvokeInfo;

static void InvokeInfo_init(InvokeInfo *me, int32_t h,
                ObjectOp op, ObjectArg *args, ObjectCounts k)
{
  C_ZERO(*me);
  me->handle = h;
  me->op = op;
  me->args = args;
  me->k = k;
}

static inline void
InvokeInfo_setResult(InvokeInfo *me, int32_t result) {
  me->bComplete = true;
  me->result = result;
  pthread_cond_signal(&me->cond);
}


#define FOR_ARGS(ndxvar, counts, section)                       \
  for (size_t ndxvar = ObjectCounts_index##section(counts);     \
       ndxvar < (ObjectCounts_index##section(counts)            \
                 + ObjectCounts_num##section(counts));          \
       ++ndxvar)

#include <errno.h>
/*
 * Return -1 on error, 0 on success
 */
#define IO_REPEAT(func, fd, ptr, size)                         \
  while (size > 0) {                                           \
    ssize_t cb = func(fd, ptr, size, MSG_NOSIGNAL);            \
    if (cb <= 0) {                                             \
      return -1;                                               \
    }                                                          \
    if (cb <= (ssize_t) size) {                                \
      ptr = (cb + (char *) ptr);                               \
      size -= (size_t) cb;                                     \
    }                                                          \
  }                                                            \
  return 0;

/*
 * Return -1 on error, 0 on success
 */
static int send_all(int fd, void *ptr, size_t size)
{
  IO_REPEAT(send, fd, ptr, size);
}

/*
 * Return -1 on error, 0 or N (number of fds found) on success
 */
static int recv_msg(int fd, void *ptr, size_t size, int *fds, int num_fds)
{
  struct msghdr msg;
  C_ZERO(msg);
  struct iovec io = { .iov_base = ptr, .iov_len = size };
  struct cmsghdr *cmsg;
  int fd_count = 0;

  char buffer[sizeof(struct cmsghdr) + (ObjectCounts_maxOI  * sizeof(int))];
  msg.msg_iov = &io;
  msg.msg_iovlen = 1;
  msg.msg_control = buffer;
  /*The msg_controllen should be determined based on the actual length of the fd*/
  msg.msg_controllen = sizeof(struct cmsghdr) + num_fds * sizeof(int);
  //setup control data buffer
  cmsg = CMSG_FIRSTHDR(&msg);
  if (!cmsg) {
    return -1;
  }
  //init fd buffer to -1
  memset(CMSG_DATA(cmsg), -1, num_fds);

  while (io.iov_len > 0) {
    C_ZERO(buffer); //reset the control buffer
    memset(CMSG_DATA(cmsg), -1, num_fds);

    ssize_t cb = recvmsg(fd, &msg, MSG_NOSIGNAL);
    if (cb <= 0) {
      return -1;
    }

    //collect ancillary data
    int msg_fds = cmsg->cmsg_len > sizeof(struct cmsghdr)
                    ? (cmsg->cmsg_len - sizeof(struct cmsghdr)) / sizeof(int)
                    : 0;

    for(int i = 0; i < msg_fds; i++) {
      if (fd_count >= num_fds) {
        for (int x = 0; x < fd_count; x++) {
          //close fds that were collected
          close(fds[x]);
        }

        for (int y = i; y < msg_fds; y++) {
         //close fds that we weren't expecting
          int tmp;
          memscpy(&tmp, sizeof(int), CMSG_DATA(cmsg)+(y*sizeof(int)), sizeof(int));
          close(tmp);
        }

        return -1; // this shouldn't have happened
      }
      fd_count++;
    }

    memscpy(fds, num_fds*sizeof(int), CMSG_DATA(cmsg), fd_count*sizeof(int));
    if (cb <= (ssize_t) io.iov_len) {
        io.iov_base = (void *) (cb + (char*)io.iov_base);
        io.iov_len -= cb;
    }
  }
  return fd_count; //number of fds returned
}

static int sendv_all_qrtr(MinkSocket *me, struct iovec *iov, size_t iovLen, int *fds, int num_fds)
{
  return 0;
}

/*
 * Return -1 on error, 0 on success
 */
static int sendv_all(MinkSocket *me, struct iovec *iov, size_t iovLen, int *fds, int num_fds)
{
  int fd;
  struct msghdr msg;
  C_ZERO(msg);
  msg.msg_control = NULL;
  char buffer[sizeof(struct cmsghdr) + ObjectCounts_maxOO * sizeof(int)];
  C_ZERO(buffer);
  if (is_qrtr_sock(me->sock))
  	return sendv_all_qrtr(me, iov, iovLen, fds, num_fds);
  else
  	fd = me->sock;

  if (num_fds > 0) {
    struct cmsghdr *cmsg;
    msg.msg_control = buffer;
    /*The msg_controllen should be determined based on the actual length of the fd*/
    msg.msg_controllen = sizeof(struct cmsghdr) + num_fds * sizeof(int);
    cmsg = CMSG_FIRSTHDR(&msg);
    if(!cmsg) {
      return -1;
    }
    cmsg->cmsg_len = msg.msg_controllen;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    memscpy(CMSG_DATA(cmsg), num_fds*sizeof(int), fds, num_fds*sizeof(int));
  }

  while (iovLen > 0) {
    msg.msg_iov = iov;
    msg.msg_iovlen = iovLen;
    ssize_t cb = sendmsg(fd, &msg, MSG_NOSIGNAL);
    if (cb < 0) {
      return -1;
    }

    if (msg.msg_control) {
      msg.msg_control = NULL;
      msg.msg_controllen = 0;
    }
    // Note corner case:
    // When cb = 0, we can still eliminate iovs, if they are zero-length.
    while (iovLen > 0) {
      if (cb < (ssize_t) iov->iov_len) {
        iov->iov_len -= cb;
        iov->iov_base = (void*) (cb + (char*)iov->iov_base);
        break;
      } else {
        cb -= iov->iov_len;
        ++iov;
        --iovLen;
      }
    }
  }
  return 0;
}



#define ERR_CLEAN(e) \
  do {                                         \
    if(!Object_isOK(err = (e))) {              \
      CHECK_LOG(); goto cleanup; }             \
  } while(0)

#define CHECK_CLEAN(expr) \
  do {                                           \
    if (!(expr)) { CHECK_LOG(); goto cleanup; }  \
  } while (0)

#define ObjectCounts_numObjects(k)  (ObjectCounts_numOI(k) + \
                                     ObjectCounts_numOO(k))

#define ObjectCounts_indexObjects(k) \
  ObjectCounts_indexOI(k)

#define ObjectCounts_indexBUFFERS(k) \
  ObjectCounts_indexBI(k)

#define ObjectCounts_numBUFFERS(k) \
  (ObjectCounts_numBI(k) + ObjectCounts_numBO(k))

#define ObjectCounts_numIn(k) \
  (ObjectCounts_numBUFFERS(k) + ObjectCounts_numOI(k))

#define ObjectCounts_numOut(k) \
  (ObjectCounts_numBO(k) + ObjectCounts_numOO(k))

#define ObjectCounts_sizeofInvReq(k) \
  (c_offsetof(lxcom_inv_req, a) + ObjectCounts_numIn(k) * sizeof(lxcom_arg))

#define ObjectCounts_sizeofInvSucc(k) \
  (c_offsetof(lxcom_inv_succ, a) + ObjectCounts_numOut(k) * sizeof(lxcom_arg))

#define CHECK_OBJ_INDEX_RETURN(i)                \
  do {                                           \
    if ((i) >= LXCOM_MAX_ARGS) {                 \
      LOGE("%s: Out of index: index = %d\n",     \
           __func__, (uint32_t)(i));             \
      return Object_ERROR_MAXARGS;               \
    }                                            \
  } while (0)

MinkSocket *MinkSocket_QRTR_new(Object opener, int sock, uint32_t node, uint32_t port)
{
  return NULL;
}

MinkSocket *MinkSocket_new(Object opener)
{
  MinkSocket *me = HEAP_ZALLOC_REC(MinkSocket);
  if (!me) {
    return NULL;
  }

  me->refs = 1;
  me->sock = -1;
  me->bDone = false;
  me->msForwarderCount = 0;
  me->invoke_id = BEGIN_INVOKE_ID;
  QList_construct(&me->qlInvokes);
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, me->sockPair)) {
    goto cleanup;
  }

  pthread_mutex_init(&me->mutex, NULL);
  pthread_cond_init(&me->cond, NULL);
  me->pool = ThreadPool_new();
  CHECK_CLEAN(me->pool);

  CHECK_CLEAN(!ObjectTableMT_construct(&me->table, MAX_OBJECT_COUNT));
  if (!Object_isNull(opener)) {
    CHECK_CLEAN(ObjectTableMT_addObject(&me->table, opener) != -1);
  }
  return me;

 cleanup:
  MinkSocket_release(me);
  return NULL;
}

static inline bool MinkSocket_isReady(MinkSocket *me)
{
  return (me->sock != -1);
}

void MinkSocket_retain(MinkSocket *me)
{
  atomic_add(&me->refs, 1);
}

void MinkSocket_delete(MinkSocket *me)
{
  if (me->pool) {
    ThreadPool_release(me->pool);
  }
  if (me->sockPair[0] >=0 &&  !is_qrtr_sock(me->sock)) {
    close(me->sockPair[0]);
  }
  if (me->sockPair[1] >=0 &&  !is_qrtr_sock(me->sock)) {
    close(me->sockPair[1]);
  }
  pthread_mutex_destroy(&me->mutex);
  pthread_cond_destroy(&me->cond);
  ObjectTableMT_destruct(&me->table);
  heap_free(me);
}

void MinkSocket_close(MinkSocket *me, int32_t err)
{
  //lock
  pthread_mutex_lock(&me->mutex);

  me->bDone = true;
  if (me->sock != -1) {
    if (!is_qrtr_sock(me->sock))
      close(me->sock);
    me->sock = -1;
    for (int i = 0; i < THREADPOOL_MAX_THREADS; i++ ) {
      if (write(me->sockPair[1], "x", 1)) { } // interrupt poll.
    }
  }

  QNode *pqn;
  while ((pqn = QList_pop(&me->qlInvokes))) {
    InvokeInfo_setResult(c_containerof(pqn, InvokeInfo, qn), err);
  }

  //unlock
  pthread_mutex_unlock(&me->mutex);
}

bool MinkSocket_isConnected(MinkSocket *me) {
  pthread_mutex_lock(&me->mutex);
  if (me->bDone && me->sock == -1) {
    pthread_mutex_unlock(&me->mutex);
    return false;
  }
  pthread_mutex_unlock(&me->mutex);
  return true;
}

void MinkSocket_registerForwarder(MinkSocket *me)
{
  atomic_add(&me->msForwarderCount, 1);
  MinkSocket_retain(me);
}


void MinkSocket_unregisterForwarder(MinkSocket *me)
{
  atomic_add(&me->msForwarderCount, -1);
  if(atomic_add(&me->refs, 0) == 1) {
  }
  MinkSocket_release(me);
}

void MinkSocket_release(MinkSocket *me)
{
  if (atomic_add(&me->refs, -1) == 0) {
    MinkSocket_close(me, Object_ERROR_UNAVAIL);
    MinkSocket_delete(me);
  }
}

int MinkSocket_detach(MinkSocket *me)
{
  if (me->msForwarderCount != 0 || me->sock == -1) return -1;

  int retFd = -1;
  retFd = dup(me->sock);
  MinkSocket_close(me, Object_ERROR_UNAVAIL);
  return retFd;
}

int MinkSocket_detachObject(Object *obj)
{
  MSForwarder *forwarder = MSForwarderFromObject(*obj);
  if (forwarder) {
    return MSForwarder_detach(forwarder);
  }
  return -1;
}

void MinkSocket_attachObject(MinkSocket *me, int handle, Object *obj)
{
  *obj = MSForwarder_new(me, handle);
}

static uint8_t PadBuf[8] = {0xF};
static int32_t insertIOV(struct iovec *iov, int32_t nMaxIov,
                         int32_t *pnUsed,
                         void *ptr, size_t size,
                         uint32_t *szOnWire)
{
  if (*pnUsed >= nMaxIov) {
    return Object_ERROR_MAXARGS;
  }

  int32_t pad = PADDED(*szOnWire) - *szOnWire;
  if(pad) {
    iov[(*pnUsed)++] = (struct iovec) { PadBuf, pad };
    *szOnWire += pad;
  }

  if (*pnUsed >= nMaxIov) {
    return Object_ERROR_MAXARGS;
  }

  iov[(*pnUsed)++] = (struct iovec) { ptr, size };
  *szOnWire += size;
  return Object_OK;
}

static int
MinkSocket_createArgs(MinkSocket *me, const InvokeInfo *pii,
                     lxcom_inv_req *req,
                     struct iovec *iov, int32_t szIov,
                     int32_t *nIov,
                     int *fds, int num_fds)
{
  int32_t err;
  C_ZERO(*req);
  req->hdr.type = LXCOM_REQUEST;
  req->hdr.invoke_id = pii->invoke_id;
  req->op = pii->op;
  req->handle = pii->handle;
  req->k = pii->k;
  int fd_index = 0;

  for(int i = 0; i < num_fds; i++) {
    fds[i] = -1; //unset all the fds
  }

  *nIov = 0;
  err = insertIOV(iov, szIov, nIov,
                  req, ObjectCounts_sizeofInvReq(pii->k),
                  &req->hdr.size);
  if (Object_OK != err) {
    return Object_ERROR_INVALID;
  }

  FOR_ARGS(i, pii->k, BO) {
    if (!pii->args[i].b.ptr && pii->args[i].b.size != 0) {
      return Object_ERROR_INVALID;
    }
  }

  // Req Header for:
  // HDR | BI Sizes | BO Sizes | OI handles
  FOR_ARGS(i, pii->k, BI) {
    err = insertIOV(iov, szIov, nIov,
                    pii->args[i].b.ptr, pii->args[i].b.size,
                    &req->hdr.size);
    if (Object_OK != err) {
      return Object_ERROR_INVALID;
    }
  }

  //Copy buffer-sizes into header
  FOR_ARGS(i, pii->k, BUFFERS) {
    req->a[i].size = pii->args[i].b.size;
  }

  //Copy object handles into header
  FOR_ARGS(i, pii->k, OI) {
    if (Object_isNull(pii->args[i].o)) {
      // nothing to do
    } else {
      //Forwarder already.. peel and send the handle
      MSForwarder *po = MSForwarderFromObject(pii->args[i].o);
      int fd = -1;
      if (Object_OK == Object_unwrapFd(pii->args[i].o, &fd)) { }

      if (po && po->conn == me) {
        req->a[i].o.flags = LXCOM_CALLEE_OBJECT;
        req->a[i].o.handle = po->handle;
      } else if (fd > 0) {
        fds[fd_index] = fd;
        fd_index++;
        req->a[i].o.flags = LXCOM_DESCRIPTOR_OBJECT;
        req->a[i].o.handle = 0;
      } else {
        //New object, need entry in table
        int h = ObjectTableMT_addObject(&me->table, pii->args[i].o);
        if (h == -1) {
          return Object_ERROR_KMEM;
        }

        req->a[i].o.flags = LXCOM_CALLER_OBJECT;
        req->a[i].o.handle = h;
      }
    }
  }

  return Object_OK;
}

/*
 * Process arguments returned from the kernel.  For buffer arguments there
 * is nothing to do.  For returned objects, the `inv` structure holds newly
 * allocated descriptors, around which we construct new forwarder instances.
 */
static int32_t
MinkSocket_marshalOut(MinkSocket *me, lxcom_inv_succ *succ, InvokeInfo *pii,
                      int *fds, int num_fds)
{
  int err;
  size_t indexBO = ObjectCounts_numBO(pii->k);
  int fd_index = 0;
  FOR_ARGS(i, pii->k, OO) {
    if (succ->a[indexBO].o.flags & LXCOM_CALLEE_OBJECT) {
      // new outbound object
      pii->args[i].o = MSForwarder_new(me, succ->a[indexBO].o.handle);
      if (Object_isNull(pii->args[i].o)) {
        ERR_CLEAN(Object_ERROR_KMEM);
      }
    } else if (succ->a[indexBO].o.flags & LXCOM_CALLER_OBJECT) {
      pii->args[i].o = ObjectTableMT_recoverObject(&me->table,
                                                 succ->a[indexBO].o.handle);
      if (Object_isNull(pii->args[i].o)) {
        ERR_CLEAN(Object_ERROR_BADOBJ);
      }
    } else if(succ->a[indexBO].o.flags & LXCOM_DESCRIPTOR_OBJECT) {
      if (fd_index >= num_fds) {
        //We didn't receive enough Fds to give out
        ERR_CLEAN(Object_ERROR_UNAVAIL);
      }
      pii->args[i].o = FdWrapper_new(fds[fd_index]);
      if (Object_isNull(pii->args[i].o)) {
        ERR_CLEAN(Object_ERROR_KMEM);
      }

      //so we don't double close if an error occurs
      fds[fd_index] = -1;
      fd_index++;
    } else {
      pii->args[i].o = Object_NULL;
    }
    ++indexBO;
  }

  return Object_OK;

  /*
   * If marshaling of any object has failed, we will be returning an error.
   * In that case the caller will not receive output objects so we must
   * free any allocated object data (and release the descriptors they hold).
   */

 cleanup:

  FOR_ARGS(i, pii->k, OO) {
    Object_ASSIGN_NULL(pii->args[i].o);
  }

  for (int i = 0; i < num_fds; i++) {
    if (fds[i] != -1) {
      close(fds[i]);
    }
  }
  return Object_ERROR_UNAVAIL;
}



//Called from invoker thread...
int32_t
MinkSocket_invoke(MinkSocket *me, int32_t h,
                ObjectOp op, ObjectArg *args, ObjectCounts k)
{
  if (!MinkSocket_isReady(me)) {
    return Object_ERROR_UNAVAIL;
  }

  int32_t err;
  lxcom_inv_req req;
  struct iovec iov[LXCOM_MAX_ARGS*2];
  int32_t nIov = 0;
  InvokeInfo ii;
  int fds[ObjectCounts_maxOI];

  InvokeInfo_init(&ii, h, op, args, k);

  if (0 != pthread_cond_init(&ii.cond, NULL)) {
    return Object_ERROR_KMEM;
  }

  ERR_CLEAN(MinkSocket_createArgs(me, &ii, &req, iov,
                                  C_LENGTHOF(iov), &nIov,
                                  fds, C_LENGTHOF(fds)));
  int num_fds = 0;
  for (int i = 0; i < (int) C_LENGTHOF(fds); i++) {
    if (fds[i] != -1) {
      num_fds++;
    }
  }

  pthread_mutex_lock(&me->mutex);

  //Allocate an ID for this invocation
  ii.invoke_id = req.hdr.invoke_id = me->invoke_id++;
  QList_appendNode(&me->qlInvokes, &ii.qn);

  if (-1 == sendv_all(me, iov, nIov, fds, num_fds)) {
    QNode_dequeue(&ii.qn);
    err = Object_ERROR_UNAVAIL;
  }

  //wait for the response
  if (Object_OK == err) {
    while (!ii.bComplete) {
      pthread_cond_wait(&ii.cond, &me->mutex);
    }
    QNode_dequeueIf(&ii.qn);
    err = ii.result;
  }
  pthread_mutex_unlock(&me->mutex);

cleanup:

  pthread_cond_destroy(&ii.cond);

  return err;
}

static InvokeInfo *
MinkSocket_getInvokeInfo(MinkSocket *me, uint32_t id)
{
  pthread_mutex_lock(&me->mutex);

  QNode *pqn;
  QLIST_FOR_ALL(&me->qlInvokes, pqn) {
    InvokeInfo *pii = c_containerof(pqn, InvokeInfo, qn);
    if (pii->invoke_id == id) {
      pthread_mutex_unlock(&me->mutex);
      return pii;
    }
  }
  pthread_mutex_unlock(&me->mutex);
  return NULL;
}

int32_t
MinkSocket_sendClose(MinkSocket *me, int handle)
{
  int32_t ret;
  if (!MinkSocket_isReady(me)) {
    return Object_ERROR_UNAVAIL;
  }

  int sz = sizeof(lxcom_inv_close);
  lxcom_inv_close cls = (lxcom_inv_close) { sz, LXCOM_CLOSE, handle};

  pthread_mutex_lock(&me->mutex);
  if (is_qrtr_sock(me->sock)) {
    ret = qrtr_sendto(me->sock, me->node, me->port, &cls, sz);
  } else {
    ret = send_all(me->sock, &cls, sz);
  }
  pthread_mutex_unlock(&me->mutex);
  return (ret == -1) ? Object_ERROR_UNAVAIL : Object_OK;
}


/* Handles Invoke Success messages and marshals out arguments
 *
 * The Success buffer (mb->msg.succ of type lxcom_inv_succ) is
 * used to send information about out buffers and out objects.
 * MinkSocket_SendInvokeSuccess populates this with output argument
 * details.
 * Indicies from 0-numBO pertain to out Buffers and
 * Indicies from numBo-numOO pertain to out Objects
*/
static int32_t
MinkSocket_recvInvocationSuccess(MinkSocket *me, msgbuf_t *mb,
                                 ThreadWork *work, int *fds, int num_fds)
{
  int32_t err;
  InvokeInfo *pii = MinkSocket_getInvokeInfo(me, mb->hdr.invoke_id);
  if (!pii) {
    ERR_CLEAN(Object_ERROR_UNAVAIL);
  }

  size_t size = ObjectCounts_sizeofInvSucc(pii->k);
  if (size > mb->hdr.size) {
    ERR_CLEAN(Object_ERROR_MAXARGS);
  }

  if (ObjectCounts_total(pii->k) > C_LENGTHOF(mb->msg.succ.a)) {
    ERR_CLEAN(Object_ERROR_MAXARGS);
  }

  int iBO = 0;
  FOR_ARGS(i, pii->k, BO) {
    size = PADDED(size);
    /* Added Handling of zero length, prevents SIGABRT due to
     * invalid src buffer with zero length
     */
    if(mb->msg.succ.a[iBO].size != 0)
    {
      if (size > mb->hdr.size) {
        ERR_CLEAN(Object_ERROR_MAXARGS);
      }

      if (pii->args[i].b.size < mb->msg.succ.a[iBO].size) {
        ERR_CLEAN(Object_ERROR_INVALID);
      }

      memscpy(pii->args[i].b.ptr, pii->args[i].b.size,
              mb->buf + size, mb->msg.succ.a[iBO].size);
      size += mb->msg.succ.a[iBO].size;
      if (size > mb->hdr.size) {
        ERR_CLEAN(Object_ERROR_MAXARGS);
      }
    }
    pii->args[i].b.size = mb->msg.succ.a[iBO].size;
    iBO++;
  }

  ERR_CLEAN( MinkSocket_marshalOut(me, &mb->msg.succ, pii, fds, num_fds) );
  if (work) {
   ThreadPool_queue(me->pool, work);
   work = NULL;
  }

  InvokeInfo_setResult(pii, Object_OK);

 cleanup:
  if (work) {
   ThreadPool_queue(me->pool, work);
  }

  return err;
}


static int32_t
MinkSocket_recvInvocationError(MinkSocket *me, msgbuf_t *mb)
{
  if (mb->hdr.size != sizeof(lxcom_inv_err)) {
    return Object_ERROR_INVALID;
  }

  int32_t err;
  InvokeInfo *pii = MinkSocket_getInvokeInfo(me, mb->hdr.invoke_id);
  if (pii) {
    InvokeInfo_setResult(pii, mb->msg.err.err);
    err = Object_OK;
  } else {
    err = Object_ERROR_UNAVAIL;
  }

  return err;
}

static int32_t MinkSocket_wireToBOArgs(MinkSocket *me, lxcom_inv_req *req,
                                       ObjectArg *args,
                                       void **ppvBuf, int32_t size)
{
  int32_t boSize = 0;
  void *bo;

  FOR_ARGS(i, req->k, BO) {
    args[i].b.size = req->a[i].size;
    if (args[i].b.size != 0)
      boSize += PADDED(args[i].b.size);
    else
      boSize += LXCOM_MSG_ALIGNMENT;
  }

  if (boSize > size) {
    if (boSize > MAX_BUFFER_ALLOCATION - 4) {
      return Object_ERROR_INVALID;
    }
    bo = heap_zalloc(boSize+4);
    if (NULL == bo) {
      return Object_ERROR_INVALID;
    }
    *ppvBuf = bo;
  } else {
    bo = *ppvBuf;
  }

  BBuf bbuf;
  BBuf_construct(&bbuf, bo, boSize+4);
  FOR_ARGS(i, req->k, BO) {
    args[i].b.ptr = BBuf_alloc(&bbuf, req->a[i].size);
  }

  return Object_OK;
}


static int32_t
MinkSocket_recvClose(MinkSocket *me, msgbuf_t *mb)
{
  if (mb->hdr.size != sizeof(lxcom_inv_close)) {
    return Object_ERROR_INVALID;
  }

  ObjectTableMT_releaseHandle(&me->table, mb->msg.close.handle);
  return Object_OK;
}

static int32_t
MinkSocket_sendInvokeSuccess(MinkSocket *me, lxcom_inv_req *req,
                             ObjectArg *args)
{
  lxcom_inv_succ succ;
  C_ZERO(succ);
  succ.hdr.type = LXCOM_SUCCESS;
  succ.hdr.invoke_id = req->hdr.invoke_id;

  int32_t err;
  int32_t nIov = 0;
  struct iovec iov[LXCOM_MAX_ARGS*2];

  err = insertIOV(iov, C_LENGTHOF(iov), &nIov,
                  &succ, ObjectCounts_sizeofInvSucc(req->k),
                  &succ.hdr.size);
  if (Object_OK != err) {
    LOGE("%s: err %x from insertIOV 1\n", __func__, err);
    return err;
  }

  size_t numBO = ObjectCounts_numBO(req->k);
  //Copy BO sizes into header
  for (size_t i=0; i<numBO; ++i) {
    int argi = ObjectCounts_indexBO(req->k)+i;
    succ.a[i].size = args[argi].b.size;

    err = insertIOV(iov, C_LENGTHOF(iov), &nIov,
                  args[argi].b.ptr, args[argi].b.size,
                  &succ.hdr.size);
    if (Object_OK != err) {
      LOGE("%s: err %x from insertIOV 2\n", __func__, err);
      return err;
    }
  }

  int fds[ObjectCounts_maxOO];
  int fd_index = 0;
  for (int i=0; i < ObjectCounts_maxOO; i++) {
    fds[i] = -1;
  }

  int iOO = numBO;
  FOR_ARGS(i, req->k, OO) {
    CHECK_OBJ_INDEX_RETURN(iOO);
    if (Object_isNull(args[i].o)) {
      succ.a[iOO].o.flags = 0;
      succ.a[iOO].o.handle = 0;
    } else {
      MSForwarder *po = MSForwarderFromObject(args[i].o);
      int fd = -1;
      if (Object_OK == Object_unwrapFd(args[i].o, &fd)) { }

      if (po && po->conn == me) {
        //Forwarder already.. peel and send the handle
        succ.a[iOO].o.flags = LXCOM_CALLER_OBJECT;
        succ.a[iOO].o.handle = po->handle;
      } else if (fd > 0) {
        succ.a[iOO].o.flags = LXCOM_DESCRIPTOR_OBJECT;
        succ.a[iOO].o.handle = 0;
        fds[fd_index] = fd;
        fd_index++;
      } else {
        //New object, need entry in table
        int h = ObjectTableMT_addObject(&me->table, args[i].o);
        if (h == -1) {
          LOGE("%s:  Error adding object to table\n", __func__);
          return Object_ERROR_KMEM;
        }
        succ.a[iOO].o.flags = LXCOM_CALLEE_OBJECT;
        succ.a[iOO].o.handle = h;
      }
    }
    ++iOO;
  }

  pthread_mutex_lock(&me->mutex);
  if (-1 == sendv_all(me, iov, nIov, fds, fd_index)) {
    LOGE("%s:  Error sending message: returning Object_ERROR_UNAVAIL \n", __func__);
    err = Object_ERROR_UNAVAIL;
  }

  pthread_mutex_unlock(&me->mutex);
  return err;
}



static int32_t
MinkSocket_sendInvokeError(MinkSocket *me, lxcom_inv_req *req,
                           int32_t error)
{
  int32_t ret;
  lxcom_inv_err err;
  err.hdr.type = LXCOM_ERROR;
  err.hdr.size = sizeof(err);
  err.hdr.invoke_id = req->hdr.invoke_id;
  err.err = error;

  pthread_mutex_lock(&me->mutex);
  if (is_qrtr_sock(me->sock)) {
    ret = qrtr_sendto(me->sock, me->node, me->port, &err, err.hdr.size);
  } else {
    ret = send_all(me->sock, &err, err.hdr.size);
  }
  pthread_mutex_unlock(&me->mutex);
  return (ret == -1) ? Object_ERROR_UNAVAIL : Object_OK;
}


static int32_t
MinkSocket_recvInvocationRequest(MinkSocket *me, msgbuf_t *mb,
                                 ThreadWork *work, int *fds, int num_fds)
{
  int32_t err = Object_OK;
  struct {
    uint8_t buf[MSG_PREALLOCED_SIZE];
    uint64_t _aligned_unused;
  } bufBO;
  void *pvBO = bufBO.buf;

  size_t size = ObjectCounts_sizeofInvReq(mb->msg.req.k);
  if (size > mb->hdr.size) {
    ERR_CLEAN(Object_ERROR_INVALID);
  }

  ObjectArg args[LXCOM_MAX_ARGS];
  FOR_ARGS(i, mb->msg.req.k, BI) {
    size = PADDED(size);
    if (size > mb->hdr.size) {
      ERR_CLEAN(Object_ERROR_INVALID);
    }

    args[i].b.ptr = (uint8_t *)mb->buf + size;
    args[i].b.size = mb->msg.req.a[i].size;
    size += mb->msg.req.a[i].size;
    if (size > mb->hdr.size) {
      ERR_CLEAN(Object_ERROR_INVALID);
    }
  }

  if (0 != ObjectCounts_numBO(mb->msg.req.k)) {
    ERR_CLEAN(MinkSocket_wireToBOArgs(me, &mb->msg.req, args,
                                      &pvBO, MSG_PREALLOCED_SIZE));
  }

  int fd_index = 0;
  FOR_ARGS(i, mb->msg.req.k, OI) {
    if (mb->msg.req.a[i].o.flags & LXCOM_CALLER_OBJECT) {
      // new outbound object
      args[i].o = MSForwarder_new(me, mb->msg.req.a[i].o.handle);
      if (Object_isNull(args[i].o)) {
        ERR_CLEAN(Object_ERROR_UNAVAIL);
      }
    } else if (mb->msg.req.a[i].o.flags & LXCOM_CALLEE_OBJECT) {
      args[i].o = ObjectTableMT_recoverObject(&me->table,
                                              mb->msg.req.a[i].o.handle);
      if (Object_isNull(args[i].o)) {
        ERR_CLEAN(Object_ERROR_UNAVAIL);
      }
    } else if (mb->msg.req.a[i].o.flags & LXCOM_DESCRIPTOR_OBJECT) {
      if (fd_index >= num_fds) {
        // The expected fd object count doesn't match the number of fds
        // actually received
        ERR_CLEAN(Object_ERROR_UNAVAIL);
      }
      args[i].o = FdWrapper_new(fds[fd_index]);
      if (Object_isNull(args[i].o)) {
        ERR_CLEAN(Object_ERROR_UNAVAIL);
      }

      //so we don't double close if an error occurs
      fds[fd_index] = -1;
      fd_index++;
    } else {
        args[i].o = Object_NULL;
    }
  }

  //get the object out
  Object o = ObjectTableMT_recoverObject(&me->table,
                                       mb->msg.req.handle);
  if (work) {
   ThreadPool_queue(me->pool, work); //queue up the next read
   work = NULL;
  }

  if (!Object_isNull(o)) {
    int32_t oiErr =
      Object_invoke(o, ObjectOp_methodID(mb->msg.req.op),
                    args, mb->msg.req.k);
    if (Object_OK == oiErr) {
      //send Success unless there is an internal error
      err = MinkSocket_sendInvokeSuccess(me, &mb->msg.req, args);
      // Even though invoke op is a success, it is possible that transport
      // could not get enough memory to send response back. In that case,
      // send an error that requires just 4KB or less and high chance of
      // getting allocated even under stress
      if (err == Object_ERROR_KMEM)
        err = MinkSocket_sendInvokeError(me, &mb->msg.req, err);
    } else {
      //send error
      //transport err SHOULD be Object_OK else fd will be closed
      err = MinkSocket_sendInvokeError(me, &mb->msg.req, oiErr);
    }

    //Release all input Object references since we're done with them
    FOR_ARGS(i, mb->msg.req.k, OI) {
      Object_ASSIGN_NULL(args[i].o);
    }

    //Release all output Object references since we're done with them
    if (Object_isOK(oiErr)) {
      FOR_ARGS(i, mb->msg.req.k, OO) {
        Object_ASSIGN_NULL(args[i].o);
      }
    }

    Object_release(o);
  } else {
    LOGE("%s: target object %d is NULL !!\n", __func__, mb->msg.req.handle);
#ifdef OFFTARGET
    abort();
#endif
  }

 cleanup:
  if (work) {
   ThreadPool_queue(me->pool, work);
  }

  if (pvBO != bufBO.buf) {
    heap_free(pvBO);
  }

  for (int i = 0; i < num_fds; i++) {
    //close all fds found on input
    if (fds[i] != -1) {
      close(fds[i]);
    }
  }

  return err;
}

static int32_t
MinkSocket_reader(MinkSocket *me)
{
  int32_t err = Object_OK;
  msgbuf_t msgbuf;
  msgbuf_t *mb = &msgbuf;

  if (!me->bDone && me->sock != -1) {
    struct pollfd pbits[2];
    pbits[0].fd = me->sockPair[0];
    pbits[0].events = POLLIN;

    pbits[1].fd = me->sock;
    pbits[1].events = POLLIN;

    poll(pbits, 2, -1);

    if (pbits[0].revents & POLLIN) { //close the reader thread
        goto cleanup;
    }

    int fds[ObjectCounts_maxOI];
    int hdr_status = recv_msg(me->sock, mb, sizeof(lxcom_hdr),
                              fds, C_LENGTHOF(fds));
    if (hdr_status < 0) {
      return Object_ERROR_UNAVAIL;
    }

    if (mb->hdr.size > MSG_PREALLOCED_SIZE) {
      if (mb->hdr.size > MAX_BUFFER_ALLOCATION) {
        return Object_ERROR_INVALID;
      }
      mb = (msgbuf_t *)heap_zalloc(mb->hdr.size);
      if (NULL == mb) {
        return Object_ERROR_KMEM;
      }
      mb->hdr = msgbuf.hdr;
    }

    int bdy_status = recv_msg(me->sock, (uint8_t *)mb+sizeof(lxcom_hdr),
                      mb->hdr.size - sizeof(lxcom_hdr),
                      fds+hdr_status, C_LENGTHOF(fds)-hdr_status);
    if (bdy_status < 0) {
      err = Object_ERROR_UNAVAIL;
      for (int i = 0; i < hdr_status; i++) {
        close(fds[i]); //close any fds we picked up previously
      }
      goto cleanup;
    }

    ThreadWork* work = NULL;
    if (!me->bDone && me->sock != -1 && err == Object_OK) {
      work = HEAP_ZALLOC_REC(ThreadWork);
      if(!work) {
          return Object_ERROR_KMEM;
      }
      ThreadWork_init(work, MinkSocket_dispatch, me);
      if (mb->hdr.type != LXCOM_REQUEST &&
          mb->hdr.type != LXCOM_SUCCESS) {
        /* Invoke Request and Success messages may have
         * objects to marshal. So they will queue up this
         * work after marshalling objects. For everything else
         * queue up the work now.
         */
        ThreadPool_queue(me->pool, work);
      }
    }

    switch (mb->hdr.type) {
    case LXCOM_REQUEST:
      err = MinkSocket_recvInvocationRequest(me, mb, work, fds,
                                             hdr_status + bdy_status);
      break;
    case LXCOM_SUCCESS:
      err = MinkSocket_recvInvocationSuccess(me, mb, work, fds,
                                             hdr_status + bdy_status);
      break;
    case LXCOM_ERROR:
      err = MinkSocket_recvInvocationError(me, mb);
      break;
    case LXCOM_CLOSE:
      err = MinkSocket_recvClose(me, mb);
      break;
    default:
      err = Object_ERROR_UNAVAIL;
    }
  }

 cleanup:
  if (mb != &msgbuf) {
    heap_free(mb);
  }
  return err;
}

void *MinkSocket_dispatch(void *pv)
{
  MinkSocket *me = (MinkSocket *)pv;
  int32_t err = Object_OK;

  if (me->peer_available) {
    pthread_setspecific((pthread_key_t) gMinkPeerUIDTLSKey, &me->peer_uid);
    pthread_setspecific((pthread_key_t) gMinkPeerGIDTLSKey, &me->peer_gid);
  } else {
    pthread_setspecific((pthread_key_t) gMinkPeerUIDTLSKey, NULL);
    pthread_setspecific((pthread_key_t) gMinkPeerGIDTLSKey, NULL);
  }

  if (MinkSocket_isReady(me)) {
    //if (is_qrtr_sock(me->sock)) {
    //  err = MinkSocket_QRTR_reader(me);
    //} else {
      err = MinkSocket_reader(me);
    //}
  }

  if (Object_OK != err) {
    me->dispatchErr = err;
    MinkSocket_close(me, err);
    // close all gateway handles as remote
    // cannot trigger a release after fd is closed
    ObjectTableMT_closeAllHandles(&me->table);
  }

  pthread_setspecific((pthread_key_t) gMinkPeerUIDTLSKey, NULL);
  pthread_setspecific((pthread_key_t) gMinkPeerGIDTLSKey, NULL);

  return NULL;
}

void MinkSocket_start(MinkSocket *me, int sock)
{
  struct ucred creds;
  socklen_t szCreds = sizeof(creds);

  memset(&creds, 0, szCreds);

  me->peer_available = false;
  if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds) == 0) {
      me->peer_gid = creds.gid;
      me->peer_uid = creds.uid;
      me->peer_available = true;
  }

  me->sock = sock;
  ThreadWork* work = HEAP_ZALLOC_REC(ThreadWork);
  ThreadWork_init(work, MinkSocket_dispatch, me);
  ThreadPool_queue(me->pool, work);
}

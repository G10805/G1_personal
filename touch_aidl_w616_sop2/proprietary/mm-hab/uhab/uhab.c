//******************************************************************************************************************************
// Copyright (c) 2016-2019, 2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  uhab.c
* @brief Implements HAB client library
********************************************************************************************************************************
*/
#include <errno.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#else
#include <io.h>
#include <assert.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdarg.h>

#ifdef __QNXNTO__
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <dll_utils_i.h>
#endif

#include "habmm.h"
#if defined(__ANDROID__)
#include <linux/hab_ioctl.h>
#include <android/log.h>
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "cutils/trace.h"
#elif defined(__linux__)
#include "linux/hab_ioctl.h"
#ifdef HAB_DESKTOP_USER
#include <bsd/string.h>
#elif defined(USE_GLIB)
#include <glib.h>
#define strlcpy g_strlcpy
#endif
#else
#include "hab_ioctl.h"
#ifdef __INTEGRITY
#include "pmem.h"
#include "pmemext.h"
#elif defined(WIN32)
#include <zmq.h>
#else
//desktop
#endif
#endif

#ifdef __linux__
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#define DEFAULT_HAB_DEV_NODE "/dev/hab"
#endif

#ifdef __INTEGRITY
#define gettid() ((unsigned int)(uintptr_t)pthread_self())
#endif

#ifdef WIN32
extern size_t strlcpy(char *dst, char const *src, size_t s);
#define gettid() pthread_getw32threadid_np(pthread_self ())
#endif

/* Macros */
// char uhab_ver_rev[]="$Date: 2020/04/07 $ $Change: 23301301 $ $Revision: #76 $";

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096U   // if this has not been defined so far
#endif

static long g_log_level = 0;

#ifdef __QNXNTO__
#include "logger_utils.h"
#define UHAB_LOG(_fmt_, ...) \
    do { \
        logger_log(QCLOG_AMSS_MM, 0, _SLOG_INFO, "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        if (g_log_level > 0) { \
            logger_log(QCLOG_AMSS_MM, 0, _SLOG_INFO, "%s: " _fmt_, __func__, ##__VA_ARGS__); \
        } \
    } while(0)
#elif defined (ANDROID)
#define UHAB_LOG(_fmt_, ...) \
    do { \
        __android_log_print(ANDROID_LOG_INFO, "uhab", "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#ifdef _DEBUG
#define UHAB_DBG(_fmt_, ...) \
    do { \
        __android_log_print(ANDROID_LOG_DEBUG, "uhab", "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#else
#define UHAB_DBG(_fmt_, ...)
#endif
#elif defined (__INTEGRITY)
#define UHAB_LOG(_fmt_, ...) \
    do { \
        fprintf(stderr, "uhab:info(%X:%X):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        if (g_log_level > 0) fprintf(stderr, "uhab:debug(%X:%X):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__); \
    } while(0)
#else
#define UHAB_LOG(_fmt_, ...) \
    do { \
        (void)fprintf(stderr, "uhab:info(%d:%lu):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        if (g_log_level > 0) {(void)fprintf(stderr, "uhab:debug(%d:%lu):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__);} \
    } while(0)
#endif

#if defined (__linux__) || defined (__QNXNTO__) || defined(WIN32)
static void initialize_logging(void)
{
	const char *str_dbg;

	str_dbg = getenv("HAB_DEBUG");
	if (str_dbg != NULL)
	{
		errno = 0;
		g_log_level = strtol(str_dbg, NULL, 10);
		if (errno != 0) {
			UHAB_LOG("strtol failed with errno %d\n", errno);
		}
	}
}
#endif

//#define DO_UHAB_TRACE
//#define DO_UHAB_ATRACE

#ifdef DO_UHAB_TRACE
#define UHAB_TRACE_IN(__vcid__) UHAB_LOG(">>> vc %X", (__vcid__))
#define UHAB_TRACE_OUT(__vcid__) UHAB_LOG("<<< vc %X", (__vcid__))
#elif defined(DO_UHAB_ATRACE)
#define UHAB_TRACE_IN(__vcid__) ATRACE_BEGIN(__func__)
#define UHAB_TRACE_OUT(__vcid__) ATRACE_END()
#else
#define UHAB_TRACE_IN(__vcid__)
#define UHAB_TRACE_OUT(__vcid__)
#endif

#ifdef __linux__
#define CDEV_NUM_MAX (MM_ID_MAX / 100 + 1)
#endif

/* Global variables */
#ifdef WIN32
void *fd = NULL; // this is the p2p context for win32 case
#elif defined(__linux__)
/**
 * fd_arr's elements will be initialized as -1 (invalid fd) when the
 * function "get_devname_refcnt_by_mmid" concludes which dev node(s) to
 * use in the process lifecycle.
 */
static int fd_arr[CDEV_NUM_MAX] = {0};
#else
static int fd = -1;
#endif

#ifdef __INTEGRITY
Connection connection;
#endif

/* mutex lock is used to protect the refcnt/refcnt_arr and gimp_mmap_list */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef __linux__
/**
 * enum hab_open_dev_node - The state of process open hab device node
 * @OPEN_DEV_TBD: The process does not open any hab device node
 * @OPEN_DEV_HAB: Process open default /dev/hab node
 * @OPEN_DEV_HAB_GRP: Process open /dev/hab-<mmid/pchan-grp-name> node
 *
 * The initial state of hab_open_dev_node is OPEN_DEV_TBD.
 * When the process calls habmm_socket_open for the first time and
 * successfully opens the /dev/hab-<mmid/pchan-grp-name> node, the state
 * of hab_open_dev_node switches to OPEN_DEV_HAB_GRP.
 * If open fails and the failure reason is EACCES or ENOENT, the state
 * of hab_open_dev_node switches to OPEN_DEV_HAB.
 * Otherwise the state of hab_open_dev_node remains OPEN_DEV_TBD
 * and habmm_socket_open directly returns failure.
 */
enum hab_open_dev_node {
	OPEN_DEV_TBD,
	OPEN_DEV_HAB,
	OPEN_DEV_HAB_GRP,
};

static enum hab_open_dev_node open_dev_node = OPEN_DEV_TBD;
/**
 * refcnt is equal to the number of currently opened vchans.
 * refcnt is used to protect fd from abnormal release.
 *
 * The value of refcnt changes with the operation on fd in
 * habmm_socket_open and habmm_socket_close.
 * When using fd in other APIs, we did not operate refcnt.
 * To avoid "use fd after closed", hab clients should avoid
 * "use vchan after closed".
 * */
static int refcnt_arr[CDEV_NUM_MAX] = {0};
#else
static int refcnt = 0;
#endif

typedef enum imp_mmap_node_type {
	HAB_IMP_MMAP_NODE_FD = 0,
	HAB_IMP_MMAP_NODE_UVA,
} imp_mmap_node_t;

struct imp_mmap_node {
	imp_mmap_node_t type;
	uint32_t exportid;
	int64_t value;
	uint32_t size;
	struct imp_mmap_node *next;
};

#if !defined(__QNXNTO__) && !defined(__INTEGRITY)
static struct imp_mmap_node *gimp_mmap_list = NULL;
static int impmmap_total = 0;
#endif

#ifdef __linux__
#ifndef HAB_MMID_MAP_NODE
#define HAB_MMID_MAP_NODE(mmid) "hab-"
#endif

/* vchan's handle[31:20] is the major of MMID */
#define VC_HANDLE2MAJOR_MMID(handle) ((uint32_t)handle>>20)

/**
 * These macros are used to help operate the fd array and refcnt array corresponding to mmid
 * relying on MMID[15:0] is major MMID.
 *
 * Basically it is used to try to leverage the code w/ other OSes and also the previous code
 * in Linux, so we don't tend to directly use fd_arr or ref_arr in different HAB APIs.
 */
#define GET_MMID_FD(mm_ip_id) ( \
{ \
	uint32_t __major_mmid = (mm_ip_id) & 0xFFFFU; \
	fd_arr[__major_mmid / 100]; \
} \
)

#define __SET_MMID_FD(mm_ip_id, new_value) ( \
{ \
	uint32_t __major_mmid = (mm_ip_id) & 0xFFFFU; \
	fd_arr[__major_mmid / 100] = (new_value); \
} \
)

#define SET_MMID_FD(mm_ip_id, new_value) ( \
{ \
	if (open_dev_node == OPEN_DEV_HAB) \
		__SET_MMID_FD(0, (new_value)); \
	else \
		__SET_MMID_FD((mm_ip_id), fd); \
} \
)

#define GET_MMID_REFCNT(mm_ip_id) ( \
{ \
	uint32_t __major_mmid = (mm_ip_id) & 0xFFFFU; \
	refcnt_arr[__major_mmid / 100]; \
} \
)

#define __SET_MMID_REFCNT(mm_ip_id, new_value) ( \
{ \
	uint32_t __major_mmid = (mm_ip_id) & 0xFFFFU; \
	refcnt_arr[__major_mmid / 100] = (new_value); \
} \
)

#define SET_MMID_REFCNT(mm_ip_id, new_value) ( \
{ \
	if (open_dev_node == OPEN_DEV_HAB) \
		__SET_MMID_REFCNT(0, (new_value)); \
	else \
		__SET_MMID_REFCNT((mm_ip_id), (new_value)); \
} \
)

static inline int get_fd_by_mmid(uint32_t mmid)
{
	int fd = -1;

	if (open_dev_node == OPEN_DEV_HAB_GRP)
		fd = GET_MMID_FD(mmid);
	else if (open_dev_node == OPEN_DEV_HAB)
		fd = GET_MMID_FD(0);
	else {
		UHAB_LOG("open_dev_node is invalid %d\n", open_dev_node);
	}

	return fd;
}
#else
#define SET_MMID_FD(mm_ip_id, new_value)
#define SET_MMID_REFCNT(mm_ip_id, new_value)
#endif

#ifdef __QNXNTO__
#define HAB_DEVNODE "/dev/hab/hab"

#define FIELD_SIZE(__struct__, __member__) sizeof(((__struct__*)0)->__member__)

static int hab_DevCtl(_Uint16t type,
    const void *snd, int snd_size,
    const void *snd2, int snd2_size,
    void *rcv, int rcv_size,
    void *rcv2, int rcv2_size) {
    struct _io_msg    hdr;
    int                status = -1;
    iov_t            siov[3] = { {&hdr, sizeof(hdr)}, {(void*)snd, snd_size}, {(void*)snd2, snd2_size}};
    iov_t            riov[2] = { {rcv, rcv_size}, {rcv2, rcv2_size}}; // this is only formated for hab_recv to have size first, then actual msg. for hab_open, only one rcv buffer is provided
    int                sparts = 1, rparts = 0;

	/* Send: Part 1 */
	hdr.type = _IO_MSG;
	hdr.combine_len = sizeof(hdr);
	hdr.mgrid = _IOMGR_QC_HAB;
	hdr.subtype = type;

	if (snd != NULL)
		sparts++;
	if (snd2 != NULL)
		sparts++;

	// for variable size of the receive it has to depend on the reply header, which is not used here
	// predefined protocol is used instead. fixed size reply first, variable size reply at the end
	if (rcv != NULL)
		rparts = 1; // simple one part reply, no copy is needed. reply filled to sender buffer directly
	if (rcv2 != NULL) {
	  	//assert(rcv); // first recv buffer has to be provided
		if (rcv2_size == FIELD_SIZE(struct hab_recv, sizebytes)) {
			rparts = 2; // recv's reply size is returned through rpart, the data is directly copied in driver
			SETIOV(&riov[0], rcv2, rcv2_size);
			SETIOV(&riov[1], rcv, rcv_size);
		} else if (rcv_size == FIELD_SIZE(struct hab_info, ids)) {
			rparts = 2; // rcv2 actual filled length is not returned
			// rpart order is normal
		}
	}

	status = MsgSendv_r(fd, siov, sparts, riov, rparts); /* return exact status */

	if (status != 0 && status != -ETIMEDOUT && status != -EAGAIN) {
		UHAB_LOG("MsgSendv failed to send message (device_id:0x%p, type:0x%x), return %d\n", snd, type, status);
	}

    return status;
}
#endif
#ifdef __INTEGRITY
#define BUF_IDX_RECV 4
#define BUF_IDX_RECV_SZ_REQ 5
#define BUF_IDX_MAX 7

// this version handles up to 2 return values from driver
static int hab_DevCtl(int type,
	const void *snd, int snd_size,
	const void *snd2, int snd2_size,
	void *rcv, int rcv_size,
	void *rcv2, int rcv2_size)
{
	Error e = Success;
	int32_t ret = 0;
	Buffer B[BUF_IDX_MAX] = {0}; // please make sure this struct matches the driver side
	int buf_idx  = 0;
	unsigned int pid = (unsigned int)getpid();

	B[buf_idx].BufferType =  DataImmediate;
	B[buf_idx].TheAddress = type;
	B[buf_idx].Transferred = 0;
	buf_idx++;

	B[buf_idx].BufferType =  DataImmediate;
	B[buf_idx].TheAddress = (Address)pid;
	B[buf_idx].Length = sizeof(pid);
	B[buf_idx].Transferred = 0;
	buf_idx++;

	B[buf_idx].BufferType =  DataBuffer;
	B[buf_idx].TheAddress = (Address)snd;
	B[buf_idx].Length = snd_size;
	B[buf_idx].Transferred = 0;
	buf_idx++;

	B[buf_idx].BufferType =  DataBuffer | WaitForReply;
	B[buf_idx].TheAddress = (Address)snd2;
	B[buf_idx].Length = snd2_size;
	B[buf_idx].Transferred = 0;
	buf_idx++;

	B[buf_idx].BufferType =  DataBuffer;
	B[buf_idx].TheAddress = (Address)rcv;
	B[buf_idx].Length = rcv_size;
	B[buf_idx].Transferred = 0;
	buf_idx++;

	B[buf_idx].BufferType =  DataImmediate;
	B[buf_idx].Transferred = 0; // recv() required size if incoming buffer is too small
	buf_idx++;

	B[buf_idx].BufferType =  DataImmediate | LastBuffer;
	B[buf_idx].Transferred = 0; // return value

	e = SynchronousSend(connection,B);
	ret = (int)B[buf_idx].TheAddress; // return value is from driver directly
	if (e != Success) {
		UHAB_LOG("failed to send 0x%x of snd1 %d, snd2 %d, ret %d, recv %d, recv2 %d\n",
			type, snd_size, snd2_size, e, ret, rcv_size, rcv2_size);
		// ToDo: do we need to convert INTEGRITY-include/INTEGRITY_enum_error.h to posix errno?
		ret = -EPERM;
	} else if (ret && ret != -ETIMEDOUT) {
	  //UHAB_LOG("hab driver return error! ret %d, ret length %d, ret transfered %d\n",
	  //	ret, B[BUF_IDX_RECV].Length, B[BUF_IDX_RECV].Transferred);
	}

	if (rcv2 != NULL) { // for hab_recv and hab_query only with vmid
		// recv size will not be provided if there is error already
		uint32_t *recv_size = (uint32_t *)rcv2; // rcv2 is msg retrieved size, but names for query
		if (ret)
			*recv_size = (int)B[BUF_IDX_RECV_SZ_REQ].TheAddress; //if failed, provide caller the size required! how about query failure?
		else
			*recv_size = (uint32_t)B[BUF_IDX_RECV].Transferred; //if succeeded, take the rcv size of msg received in recv_buffer. how about query?
	}

	if (buf_idx >= BUF_IDX_MAX)
		UHAB_LOG("connection buffer index %d overflow %d\n", buf_idx, BUF_IDX_MAX);
	return ret;
}
#endif
#ifdef __linux__
// linux version handle return value within the ioctl parameter, the rest is ignored
static int hab_DevCtl(int type,
	const void *snd, int snd_size,
	const void *snd2, int snd2_size,
	void *rcv, int rcv_size,
	void *rcv2, int rcv2_size, int fd)
{
	int ret = ioctl(fd, (unsigned long)type, snd);
	if (ret == -1) {
		ret = -errno;
	} else {
		if (ret != 0) {
			UHAB_LOG("unexpected error code %d returned\n", ret);
		}
	}

	return ret;
}
#endif
#ifdef WIN32
extern int create_fd(void **fd);
extern int destroy_fd(void *fd);
extern 	int hab_DevCtl(int type,
	const void *snd, int snd_size,
	const void *snd2, int snd2_size,
	void *rcv, int rcv_size,
	void *rcv2, int rcv2_size);
#endif


#if !defined(__QNXNTO__) && !defined(__INTEGRITY)
//==================== utility functions ====================
static struct imp_mmap_node *create_imp_mmap_node (uint32_t export_id,
			imp_mmap_node_t type, int64_t value, uint32_t size_bytes) {
	struct imp_mmap_node *node = NULL;

	node = (struct imp_mmap_node *)malloc(sizeof(struct imp_mmap_node));
	if (node == NULL) {
		UHAB_LOG("failed to malloc imp_mmap_node\n");
		return NULL;
	}

	node->type = type;
	node->exportid = export_id;
	node->value = value;
	node->size = size_bytes;

	return node;
}

static void list_add(struct imp_mmap_node * node, struct imp_mmap_node ** head) {
	// add to the head
	struct imp_mmap_node *tmp = NULL;
	(void)pthread_mutex_lock(&mutex);
	if (!(*head)) {
		*head = node;
		node->next = NULL;
	} else {
		tmp = *head;
		*head = node;
		node->next = tmp;
	}
	impmmap_total++;
	(void)pthread_mutex_unlock(&mutex);
}

// return node has the same user address, or NULL returned if not found
// input is export id and uva
static struct imp_mmap_node * list_detach(imp_mmap_node_t type,
		int64_t value, uint32_t exp_id, struct imp_mmap_node ** head) {
	struct imp_mmap_node *tmp = NULL, *prev = NULL;

	(void)pthread_mutex_lock(&mutex);
	if (!(*head)) {
		UHAB_LOG("wrong list head NULL\n");
		(void)pthread_mutex_unlock(&mutex);
		return NULL;
	}

	tmp = *head;
	if (tmp->value == value && tmp->type == type && tmp->exportid == exp_id) { // head is the one
		*head = tmp->next;
		impmmap_total--;
		(void)pthread_mutex_unlock(&mutex);
		return tmp;
	}
	prev = tmp;
	while (tmp != NULL) {
		if ((tmp->value == value) && (tmp->type == type) && (tmp->exportid == exp_id)) {
            prev->next = tmp->next; // detach the current one
			impmmap_total--;
            UHAB_DBG("find matching type %d uva 0x%llx expid %d to detach\n", type, (long long)value, exp_id);
            break;
		}
		prev = tmp;
		tmp = tmp->next; // traves to the tail (NULL)
	}
	(void)pthread_mutex_unlock(&mutex);
    return tmp;
}
#endif

#define DEVNAMELEN 20
/**
 * get_devname_refcnt_by_mmid
 * When hab clients call habmm_socket_open, uhab will use this function to try to get
 * the name of the device node to be operated and the corresponding refcnt.
 * In order to be compatible with some set up w/o /dev/hab-<*> mmid/pchan groups nodes
 * and/or the access permission to such dev nodes, we design a state machine, and the
 * state of the state machine is stored in the variable open_dev_node.
 *
 *
 * The initial state of open_dev_node is OPEN_DEV_TBD. When the process calls function
 * get_devname_refcnt_by_mmid and successfully opens the /dev/hab-<mmid/pchan-grp-name>
 * node, the state of hab_open_dev_node switches to OPEN_DEV_HAB_GRP. The value of devname
 * is set to /dev/hab-<mmid/pchan-grp-name>, refcnt is set to 0, and the global array
 * fd_arr is initialized to -1.
 *
 * If /dev/hab-<mmid/pchan-grp-name> node open fails and the failure reason is EACCES or
 * ENOENT, the state of hab_open_dev_node switches to OPEN_DEV_HAB. The value of devname
 * is set to /dev/hab, refcnt is set to 0, and the global array fd_arr is initialized to -1.
 *
 * Otherwise the state of hab_open_dev_node remains OPEN_DEV_TBD and function
 * get_devname_refcnt_by_mmid directly returns failure.
 *
 *
 * If the value of open_dev_node is OPEN_DEV_HAB_GRP, when the process calls
 * get_devname_refcnt_by_mmid, We will get the corresponding refcnt value from ref_attr
 * according to mmid. And the value of devname is set to /dev/hab-<mmid/pchan-grp-name>.
 *
 * If the value of open_dev_node is OPEN_DEV_HAB, when the process calls
 * get_devname_refcnt_by_mmid, We will get the /dev/hab refcnt value from ref_attr
 * And the value of devname is set to /dev/hab.
 *
 * Params:
 * mm_ip_id - mmid(input)
 * dev_name - device name that the process will open(output)
 * refcnt - refcnt corresponding to mm_ip_id(output)
 *
 * Return:
 * 0: success, < 0: failure
*/
static int get_devname_refcnt_by_mmid(uint32_t mm_ip_id, char *dev_name, int *refcnt)
{
	int err = 0;
#ifdef __linux__
	int fd = -1;

	if (NULL == HAB_MMID_MAP_NODE(mm_ip_id & 0xFFFFU))
		return -EINVAL;

	if (open_dev_node == OPEN_DEV_TBD) {
		snprintf(dev_name, DEVNAMELEN, "/dev/%s", HAB_MMID_MAP_NODE(mm_ip_id & 0xFFFFU));
		if (-1 == (fd = open(dev_name, O_RDWR))) {
			err = errno;
			if (err == EACCES || err == ENOENT) {
				memset(dev_name, 0, DEVNAMELEN);
				snprintf(dev_name, DEVNAMELEN, DEFAULT_HAB_DEV_NODE);
				memset(fd_arr, -1, sizeof(fd_arr));
				*refcnt = 0;

				/**
				 * We need to ensure that memset fd_arr is executed before open_dev_node.
				 * Therefore, an if statement is added to make a logical connection between fd_arr and
				 * open_dev_node to prevent the compiler from reordering.
				 */
				if (fd_arr[0] == -1)
					open_dev_node = OPEN_DEV_HAB;
				UHAB_LOG("open test failed for /dev/%s, and will fall back to use default %s, ret %d\n",
					HAB_MMID_MAP_NODE(mm_ip_id & 0xFFFFU), DEFAULT_HAB_DEV_NODE, err);
				err = 0;
			} else {
				UHAB_LOG("open test failed for %s, and not decided, err %s (errno %d)\n",
					dev_name, strerror(err), err);
				memset(dev_name, 0, DEVNAMELEN);
				err = -err;
			}
		} else {
			memset(fd_arr, -1, sizeof(fd_arr));
			*refcnt = 0;

			if (fd_arr[0] == -1)
				open_dev_node = OPEN_DEV_HAB_GRP;

			UHAB_LOG("open test passed for %s\n", dev_name);
			/**
			 * the test here is used to decide which dev node(s) to use in this hab client process lifecycle,
			 * /dev/hab or those pchan/mmid grp node(s). In order to simplify the subsequent logic,
			 * we close this fd after completing the test.
			 */
			close(fd);
		}
	} else if (open_dev_node == OPEN_DEV_HAB_GRP) {
		*refcnt = GET_MMID_REFCNT(mm_ip_id);
		/* Only when the value of refcnt is 0, we need to use dev_name to open the device node */
		if (!*refcnt) {
			memset(dev_name, 0, DEVNAMELEN);
			snprintf(dev_name, DEVNAMELEN, "/dev/%s", HAB_MMID_MAP_NODE(mm_ip_id & 0xFFFFU));
		}
	} else if (open_dev_node == OPEN_DEV_HAB) {
		*refcnt = GET_MMID_REFCNT(0);
		if(!*refcnt) {
			memset(dev_name, 0, DEVNAMELEN);
			snprintf(dev_name, DEVNAMELEN, DEFAULT_HAB_DEV_NODE);
		}
	} else {
		UHAB_LOG("the value of open_dev_node is wrong %d\n", open_dev_node);
		err = -EINVAL;
	}
#endif

	return err;
}

//=================== UHAB functions =======================
/* habmm_socket_open
Params:
handle - An opaque handle associated with a successful virtual channel creation
MM_ID - multimedia ID used to allocate the physical channels to service all
the virtual channels created through this open
timeout - timeout value specified by the client to avoid forever block
flags - future extension

Return:
status (success/failure/timeout)
*/
int32_t habmm_socket_open(int32_t *handle, uint32_t mm_ip_id,
		uint32_t timeout, uint32_t flags)
{
	int32_t ret = 0;
	int refcnt = 0;
	char dev_name[DEVNAMELEN] = {0};
#ifdef __linux__
	int fd;
#endif

	struct hab_open arg = {
		.vcid = 0, // out
		.mmid = mm_ip_id,
		.timeout = timeout,
		.flags = flags,
	};

	if (handle == NULL) {
		UHAB_LOG("Error! handle pointer is NULL!\n");
		return -EINVAL;
	}

	if ((mm_ip_id & 0xFFFFU) >= (uint32_t)MM_ID_MAX) {
		UHAB_LOG("Error! MMID invalid!\n");
		return -EINVAL;
	}

	UHAB_TRACE_IN(*handle);

	(void)pthread_mutex_lock(&mutex);
	ret = get_devname_refcnt_by_mmid(mm_ip_id, dev_name, &refcnt);
	if (ret) {
		(void)pthread_mutex_unlock(&mutex);
		UHAB_TRACE_OUT(*handle);
		return ret;
	}

	if (0 == refcnt) { // first time
#if defined (__linux__) || defined (__QNXNTO__)
		initialize_logging();
#endif
#ifdef __QNXNTO__
		UHAB_DBG("hab: refcnt %d wait for hab device node..\n", refcnt);
		Resource_BlockWait(HAB_DEVNODE);
		snprintf(dev_name, DEVNAMELEN, "%s", HAB_DEVNODE);
#endif
#ifdef __INTEGRITY
#ifdef HAB_FE
		while (RequestResource((Object*)&connection, "habSndConnection_fe", NULL)
			!= Success)
			usleep(10000);
#else
		while (RequestResource((Object*)&connection, "habSndConnection", NULL)
			!= Success)
			usleep(10000);
#endif
		fd = 0; // this is not used in GHS
		refcnt++;
#elif defined(WIN32)
		create_fd(&fd);
		assert(fd);
#else
		fd = open(dev_name, O_RDWR);  // ToDo: single node only
		if (-1 != fd) {
			UHAB_DBG("hab: opened fd %d\n", fd);
			refcnt++;
			SET_MMID_FD(mm_ip_id, fd);
		} else {
			UHAB_LOG("hab: open %s failed, error code %d %s\n", dev_name, errno, strerror(errno));
			ret = -errno;
		}
#endif
	} else {
		refcnt++;
#ifdef __linux__
		fd = get_fd_by_mmid(mm_ip_id);
		if (fd < 0) {
			(void)pthread_mutex_unlock(&mutex);
			UHAB_LOG("mmid %d Invalid fd %d\n", mm_ip_id, fd);
			UHAB_TRACE_OUT(*handle);
			return -EINVAL;
		}
#endif
		UHAB_DBG("hab: re-using fd %d, cnt %d, mmid %d, timeout %d\n", fd, refcnt, mm_ip_id, timeout);
	}
	/* update refcnt_arr before unlock */
	SET_MMID_REFCNT(mm_ip_id, refcnt);
	/* We must release the lock since we need parallel open/close. */
	(void)pthread_mutex_unlock(&mutex);

	if (-1 != fd) {
#ifdef __linux__
		ret = hab_DevCtl(IOCTL_HAB_VC_OPEN, &arg, (int)sizeof(arg), NULL, 0, &arg.vcid, (int)sizeof(arg.vcid), NULL, 0, fd);
#else
		ret = hab_DevCtl(IOCTL_HAB_VC_OPEN, &arg, (int)sizeof(arg), NULL, 0, &arg.vcid, (int)sizeof(arg.vcid), NULL, 0);
#endif
		if (ret != 0) {
			if (ret != -ETIMEDOUT) {
				UHAB_LOG("fd %d, refcnt %d, return %d, vcid %x\n", fd, refcnt, ret, arg.vcid);
			}

			(void)pthread_mutex_lock(&mutex);
#ifdef __linux__
			if (open_dev_node == OPEN_DEV_HAB)
				refcnt = GET_MMID_REFCNT(0);
			else
				refcnt = GET_MMID_REFCNT(mm_ip_id);
#endif
			if (refcnt > 0) {
				/* refcnt is equal to the number of vchan cnt */
				refcnt--;
				if (refcnt == 0) {
#ifdef WIN32
					// failed to create fd/context, no further activity in this by marking fd to -1
					UHAB_LOG("no further communication to hab driver if the very first hab socket open failed\n");
#elif !defined(__INTEGRITY)
					UHAB_DBG("fd %d, refcnt %d, return %d, vcid %x, close fd now...\n", fd, refcnt, ret, arg.vcid);
					(void)close(fd);
#endif
					fd = -1;
					SET_MMID_FD(mm_ip_id, fd);
				}
				SET_MMID_REFCNT(mm_ip_id, refcnt);
			} else {
				UHAB_LOG("Error refcnt %d, fd %d return %d, mmid %d\n", refcnt, fd, ret, mm_ip_id);
			}
			(void)pthread_mutex_unlock(&mutex);
		} else {
				UHAB_LOG("opened fd %d, node %s, refcnt %d, return %d, vcid %x\n",
						fd, dev_name, refcnt, ret, arg.vcid);
		}
	}

	*handle = (ret == 0) ? arg.vcid : 0;
	if ((ret == 0) && (arg.vcid == 0)) {
		UHAB_LOG("Error! ret from driver indicates success but vcid is zero\n");
		ret = -EFAULT;
	}

	UHAB_TRACE_OUT(*handle);

	return ret;
}

/* habmm_socket_close
Params:

handle - handle to the virtual channel that was created by habmm_socket_open
       - special case
         When the handle is actually MMID rather than the vchan's handle, it
         means to cancel any further open over this MMID in this process.
Return:
status - (success/failure)
*/

int32_t habmm_socket_close(int32_t handle)
{
	int32_t ret = 0;
	uint32_t is_vchan = 0;
	uint32_t major_mmid;
#ifdef __linux__
	int fd;
	int refcnt;
#endif

	struct hab_close arg = {
		.vcid = handle,
		.flags = 0,
	};

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	/*
	 * sort of tricky/hard-coding, and tightly based on the fact,
	 *  - vchan's handle[31:20] is the major of MMID, and major is > 100.
	 *  - MMID[23:0] with major as MMID[15:0] and minior as MMID[23:16] is
	 *    passed in the open-cancel case.
	 * So need relevant change when the vchan handle and MMID layout change.
	 */
	is_vchan = ((uint32_t)handle>>24);

	if (is_vchan == 0U)
		major_mmid = handle & 0xFFFFU;
	else
		major_mmid = VC_HANDLE2MAJOR_MMID(handle);

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	/* no input value check here, and relying on the ret value */
	ret = hab_DevCtl(IOCTL_HAB_VC_CLOSE, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0, fd);
#else
	/* no input value check here, and relying on the ret value */
	ret = hab_DevCtl(IOCTL_HAB_VC_CLOSE, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
#endif



	(void)pthread_mutex_lock(&mutex);
#ifdef __linux__
	if (open_dev_node == OPEN_DEV_HAB_GRP)
		refcnt = GET_MMID_REFCNT(major_mmid);
	else
		refcnt = GET_MMID_REFCNT(0);
#endif
	/*
	 * do not touch refcnt or fd in such below cases,
	 *  - when it is MMID open-cancel case
	 *  - failed to close a vchan due to some reasons(e.g., invalid handle)
	 */
	if ((ret != 0) || (is_vchan == 0U)) {
		UHAB_LOG("fd %d, refcnt %d, handle 0x%x, is_vchan %u, return %d, vcid %x\n",
			fd, refcnt, handle, is_vchan, ret, arg.vcid);
		UHAB_TRACE_OUT(handle);
		(void)pthread_mutex_unlock(&mutex);
		return ret;
	}

	if (refcnt > 0) {
		refcnt--;
		SET_MMID_REFCNT(major_mmid, refcnt);

		if (0 == refcnt) {
			UHAB_LOG("close fd %d, refcnt %d, vcid %X\n", fd, refcnt, handle);
	#ifdef WIN32
			destroy_fd(fd);
	#elif !defined(__INTEGRITY)
			if (close(fd) != 0) {
				UHAB_LOG("Error! fd close failed\n");
			}
	#endif
			fd = -1;
			SET_MMID_FD(major_mmid, fd);
		} else {
			UHAB_LOG("skip close fd %d, refcnt %d, vcid %X\n", fd, refcnt, handle);
		}
	} else {
		UHAB_LOG("Error want to close fd %d, vcid %X, but refcnt %d\n",
				fd, handle, refcnt);
		ret = -EINVAL;
	}
	(void)pthread_mutex_unlock(&mutex);

	UHAB_TRACE_OUT(handle);
	return ret;
}

/* habmm_socket_send
Params:

handle - handle created by habmm_socket_open
src_buff - data to be send across the virtual channel
size_bytes - size of the data to be send. Either the whole packet is sent or not
flags - future extension

Return:
status (success/fail/disconnected)
*/

int32_t habmm_socket_send(int32_t handle, void *src_buff,
			uint32_t size_bytes, uint32_t flags)
{
	int32_t ret = 0;
	struct hab_send arg = {
		.vcid = handle,
		.data = (uintptr_t) src_buff,
		.sizebytes = size_bytes,
		.flags = flags,
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__
	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if ((src_buff == NULL) || (size_bytes == 0U)) {
		UHAB_LOG("Error! Nothing to send, src_buff(%p), size_bytes(%u)! vcid %X\n",
				src_buff, size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_SEND, &arg, (int)sizeof(arg), src_buff, (int)arg.sizebytes, NULL, 0, NULL, 0, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_SEND, &arg, (int)sizeof(arg), src_buff, (int)arg.sizebytes, NULL, 0, NULL, 0);
#endif
	if (ret != 0) {
		UHAB_LOG("fd %d, return %d, vcid %X\n",fd, ret, handle);
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}


/* habmm_socket_recv
Params:

handle - communication channel created by habmm_socket_open
dst_buff - buffer pointer to store received data
size_bytes - size of the dst_buff. returned value shows the actual
bytes received.
timeout - timeout value specified by the client to avoid forever block
flags - The following flags are supported:
		HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING
		HABMM_SOCKET_RECV_FLAGS_TIMEOUT

Return:
status (success/failure/timeout/disconnected)
*/
int32_t habmm_socket_recv(int32_t handle, void *dst_buff, uint32_t *size_bytes,
					uint32_t timeout, uint32_t flags)
{
	int32_t ret = 0;
	struct hab_recv arg = {
		.vcid = handle,
		.data = (uintptr_t) dst_buff,
		.sizebytes = 0,
		.flags = flags,
		.timeout = timeout,
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__

	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if ((dst_buff == NULL) || (size_bytes == NULL)) {
		UHAB_LOG("Error! NULL pointer, dst_buff(%p), size_bytes(%p)! vcid %X\n", dst_buff, size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (*size_bytes == 0U) {
		UHAB_LOG("Error! attemps to read 0 byte! vcid %X\n", handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	arg.sizebytes = *size_bytes;

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_RECV, &arg, (int)sizeof(arg), NULL, 0, dst_buff, (int)arg.sizebytes, &arg.sizebytes, (int)sizeof(arg.sizebytes), fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_RECV, &arg, (int)sizeof(arg), NULL, 0, dst_buff, (int)arg.sizebytes, &arg.sizebytes, (int)sizeof(arg.sizebytes));
#endif
	if (ret != 0) {
		UHAB_LOG("fd %d, return %d, received bytes %d, vcid %X\n", fd, ret,
             arg.sizebytes, handle);
	}

	if ((ret == 0) && (*size_bytes < arg.sizebytes)) {
		UHAB_LOG("Error! read more bytes than dst_buff size, dst_buff: %u, actual: %u\n", *size_bytes, arg.sizebytes);
		UHAB_TRACE_OUT(handle);
		ret = -EFAULT;
	}

	*size_bytes = arg.sizebytes; // return actual received bytes

	UHAB_TRACE_OUT(handle);
	return ret;
}

/* habmm_socket_sendto
Params:

handle - handle created by habmm_socket_open
src_buff - data to be send across the virtual channel
size_bytes - size of the data to be send. The packet is fully sent on success,
or not sent at all upon any failure
remote_handle - the destination of this send using remote FE's virtual
channel handle
flags - future extension

Return:
status (success/fail/disconnected)
*/
int32_t habmm_socket_sendto(int32_t handle, void *src_buff, uint32_t size_bytes,
				int32_t remote_handle, uint32_t flags)
{
	UHAB_TRACE_IN(handle);
	(void)handle;
	(void)src_buff;
	(void)size_bytes;
	(void)remote_handle;
	(void)flags;
	UHAB_TRACE_OUT(handle);
	return 0;
}


/* habmm_socket_recvfrom
Params:

handle - communication channel created by habmm_socket_open
dst_buff - buffer pointer to store received data
size_bytes - size of the dst_buff. returned value shows the actual
bytes received.
timeout - timeout value specified by the client to avoid forever block
remote_handle - the FE who sent this message through the connected
virtual channel to BE.
flags - future extension

Return:
status (success/failure/timeout/disconnected)
*/
int32_t habmm_socket_recvfrom(int32_t handle, void *dst_buff,
	uint32_t *size_bytes, uint32_t timeout,
	int32_t *remote_handle, uint32_t flags)
{
	UHAB_TRACE_IN(handle);
	(void)handle;
	(void)dst_buff;
	(void)size_bytes;
	(void)timeout;
	(void)remote_handle;
	(void)flags;
	UHAB_TRACE_OUT(handle);
	return 0;
}

/*
Params:

handle - communication channel created by habmm_socket_open
buff_to_share - buffer to be exported
size_bytes - size of the exporting buffer in bytes
export_id - to be returned by this call upon success
flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_export(int32_t handle, void *buff_to_share, uint32_t size_bytes,
				uint32_t *export_id, uint32_t flags)
{
	int32_t ret = 0;
	struct hab_export arg = {
		.vcid = handle,
		.buffer = (uintptr_t) buff_to_share,
		.sizebytes = size_bytes,
		.flags = flags
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__
	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	if ((buff_to_share == NULL) && ((flags & HABMM_EXPIMP_FLAGS_FD) == 0U)) {
#else
	if (buff_to_share == NULL) {
#endif
		UHAB_LOG("Error! buff_to_share pointer is NULL! vcid %X\n", handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __INTEGRITY
	if (!(flags & HABMM_EXPIMP_FLAGS_FD)) {
		uint64_t port_id = 0;
		ret = pmem_get_portable_id(buff_to_share, &port_id);
		if (ret) {
			UHAB_LOG("pmem_get_portable_id %llx failed. Invalid address 0x%p, vcid %X\n", port_id, buff_to_share, handle);
			return -EINVAL;
		}
		arg.buffer = (uintptr_t) port_id;
	}
#elif WIN32
	arg.buffer = (uintptr_t)((uint8_t*)buff_to_share - (uint8_t*)page_allocator_query_base());
#endif
	if ((size_bytes == 0U) || ((size_bytes % PAGE_SIZE) != 0U)) {
		UHAB_LOG("Error! export size (%d) has to be page aligned and non-zero! vcid %X\n", size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (export_id == NULL) {
		UHAB_LOG("Error! export_id pointer is NULL! vcid %X\n", handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	UHAB_DBG("exported bytes %d, memory %p, vcid %X\n", arg.sizebytes, buff_to_share, handle);

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_VC_EXPORT, &arg, (int)sizeof(arg), NULL, 0, &arg.exportid, (int)sizeof(arg.exportid), NULL, 0, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_VC_EXPORT, &arg, (int)sizeof(arg), NULL, 0, &arg.exportid, (int)sizeof(arg.exportid), NULL, 0);
#endif
	if (ret != 0) {
		UHAB_LOG("return %d, export id %d, vcid %X\n", ret, arg.exportid, handle);
	} else {
		*export_id = arg.exportid;

		UHAB_DBG("return %d, export id %d, vcid %X\n", ret, arg.exportid, handle);
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
Params:

handle - communication channel created by habmm_socket_open
buff_shared - buffer to be imported. returned upon success
size_bytes - size of the imported buffer in bytes
import_id - received when exporter sent its exporting ID through
habmm_socket_send() previously
flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_import(int32_t handle, void **buff_shared, uint32_t size_bytes,
				uint32_t export_id, uint32_t flags)
{
	int32_t ret = 0;
	struct hab_import arg = {
		.vcid = handle,
		.sizebytes = size_bytes,
		.exportid = export_id,
		.flags = flags
	};
#if !defined(__QNXNTO__) && !defined(__INTEGRITY)
	struct imp_mmap_node * node;
	imp_mmap_node_t type;
	struct hab_unimport unarg = {
		.vcid = handle,
		.exportid = export_id,
		.flags = flags
	};
	int32_t ret2 = 0;
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#endif
#ifdef __linux__
	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if ((size_bytes == 0U) || ((size_bytes % PAGE_SIZE) != 0U)) {
		UHAB_LOG("Error! import size (%d) has to be page aligned and non-zero! vcid %X\n", size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (buff_shared == NULL) {
		UHAB_LOG("Error! buff_shared pointer is NULL! vcid %X\n", handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#endif
	UHAB_DBG("fd %d, exported bytes %d, export id %d, vcid %X\n", fd, arg.sizebytes, arg.exportid, handle);

#ifdef __linux__
	ret = hab_DevCtl(IOCTL_HAB_VC_IMPORT, &arg, (int)sizeof(arg), NULL, 0, &arg.index, (int)sizeof(arg.index), NULL, 0, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_VC_IMPORT, &arg, (int)sizeof(arg), NULL, 0, &arg.index, (int)sizeof(arg.index), NULL, 0);
#endif
	if (ret != 0) {
		UHAB_LOG("fd %d, return %d, add map memory %llx, vcid %X\n", fd, ret, (unsigned long long)arg.index, handle);
		UHAB_LOG ("failed to import export id %d\n", export_id);
		UHAB_TRACE_OUT(handle);
		return ret;
	} else {
		UHAB_DBG("fd %d, return %d, add map memory %llx, vcid %X\n", fd, ret, (unsigned long long)arg.index, handle);
	}

#ifdef __QNXNTO__
	// Memory is already mapped to this process by the HAB process
	*buff_shared = (void*) arg.index;
#elif  __INTEGRITY
	if (flags & HABMM_EXPIMP_FLAGS_FD) {
		*buff_shared = (void*) arg.index;
		ret = 0;
	} else {
		uint64_t port_id = arg.index;
		ret = pmem_map_from_id(/*PMEM_GRAPHICS_COMMAND_ID*/PMEM_MDP_ID, port_id, buff_shared);//mmid matters if this is loopback testing
	}
	if (ret){
		UHAB_LOG("pmem_map_from_id failed, address = 0x%llx, vcid %X\n", arg.index, handle);
		ret = -EINVAL;
	}
#elif WIN32
	*buff_shared = (void *)(arg.index + (uint8_t*)page_allocator_query_base());
#else
	if ((flags & HABMM_EXPIMP_FLAGS_FD) != 0U) {
		*buff_shared = *(void**)&arg.kva; // Linux variants
		type = HAB_IMP_MMAP_NODE_FD;
		ret = 0;
	} else {
		void *pvoid = NULL;

		type = HAB_IMP_MMAP_NODE_UVA;
		if ((flags & HABMM_IMP_FLAGS_FIXED) != 0U) {
			pvoid =  mmap(*buff_shared, size_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)arg.index);
		} else {
			pvoid =  mmap(NULL, size_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)arg.index);
		}

		if (MAP_FAILED == pvoid && EFAULT == errno) {
			int grant_fd = (int)arg.kva;

			UHAB_DBG("failed to do hab dev mmap error code %s(%d), try mmap from grant fd\n", strerror(errno), errno);
			if (grant_fd >= 0) {
				if ((flags & HABMM_IMP_FLAGS_FIXED) != 0U) {
					pvoid = mmap(*buff_shared, size_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, grant_fd, (off_t)arg.index);
				} else {
					pvoid = mmap(NULL, size_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, grant_fd, (off_t)arg.index);
				}
				(void)close (grant_fd);
			} else {
				UHAB_LOG("grand fd(%d) is invalid\n", grant_fd);
			}
		}

		if (MAP_FAILED == pvoid) { // -1
			UHAB_LOG("failed to mmap %d bytes at index %llx on fd %d vcid %X uva %p flags %X error code %s(%d)\n",
					 size_bytes, (unsigned long long)arg.index, fd, handle, *buff_shared, flags, strerror(errno), errno);
			ret = -ENOMEM;
			*buff_shared = NULL;
#ifdef __linux__
			ret2 = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &unarg, (int)sizeof(unarg), NULL, 0, NULL, 0, NULL, 0, fd);
#else
			ret2 = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &unarg, (int)sizeof(unarg), NULL, 0, NULL, 0, NULL, 0);
#endif
			if (ret2 != 0) {
				UHAB_LOG("failed to unimport vcid %X expid %d size %d flags %X return %d\n",
						handle, export_id, flags, size_bytes, ret2);
			}

			UHAB_TRACE_OUT(handle);
			return ret;
		}

		*buff_shared = pvoid;
	}

	node = create_imp_mmap_node(export_id, type, (int64_t)*buff_shared, size_bytes);
	if (node == NULL) {
		UHAB_LOG("failed to create imp_mmap_node\n");
		ret = -ENOMEM;

		if (type == HAB_IMP_MMAP_NODE_FD) {
			(void)close ((int)(intptr_t)*buff_shared);
		} else {
			/* type == HAB_IMP_MMAP_NODE_UVA */
			int ret_temp = munmap((void*) *buff_shared, size_bytes);

			if (ret_temp != 0) {
				UHAB_LOG("failed to munmap, return %d, error code %s(%d)\n", ret_temp, strerror(errno), errno);
			}
		}
#ifdef __linux__
		ret2 = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &unarg, (int)sizeof(unarg), NULL, 0, NULL, 0, NULL, 0, fd);
#else
		ret2 = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &unarg, (int)sizeof(unarg), NULL, 0, NULL, 0, NULL, 0);
#endif
		if (ret2 != 0) {
			UHAB_LOG("failed to unimport vcid %X expid %d size %d flags %X return %d\n",
					handle, export_id, flags, size_bytes, ret2);
		}

		UHAB_TRACE_OUT(handle);
		return ret;
	}

	list_add(node, &gimp_mmap_list);
#endif

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
Params:

in handle - communication channel created by habmm_socket_open
in export_id - all resource allocated with export_id are to be freed
in flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_unexport(int32_t handle, uint32_t export_id, uint32_t flags) {
	int32_t ret = 0;
	struct hab_unexport arg = {
		.vcid = handle,
        .exportid = export_id,
        .flags = flags
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__
	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_VC_UNEXPORT, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_VC_UNEXPORT, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
#endif
	if (ret != 0) {
		UHAB_LOG("return %d on exportid %d, vcid %X\n", ret, arg.exportid, handle);
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
Params:

in handle - communication channel created by habmm_socket_open
in export_id - received when exporter sent its exporting ID through
               habmm_socket_send() previously
in buff_shared - received from habmm_import() together with export_id
in flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_unimport(int32_t handle, uint32_t export_id, void *buff_shared, uint32_t flags) {
	int32_t ret = 0;
	struct hab_unimport arg = {
		.vcid = handle,
		.exportid = export_id,
		.flags = flags
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__
	int fd;
#endif

#if !defined(__QNXNTO__) && !defined(__INTEGRITY) && !defined(WIN32)
	struct imp_mmap_node *node = NULL;
	imp_mmap_node_t type;

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if ((flags & HABMM_EXPIMP_FLAGS_FD) != 0U) {
		type = HAB_IMP_MMAP_NODE_FD;
	} else {
		type = HAB_IMP_MMAP_NODE_UVA;
	}

	if ((type == HAB_IMP_MMAP_NODE_UVA) && (buff_shared == NULL)) {
		UHAB_LOG("Error! NULL memory buffer to be unimported\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	node = list_detach(type, (int64_t)buff_shared, export_id, &gimp_mmap_list);
	if (node == NULL) {
		UHAB_LOG("failed to find node type %d expid %d\n", type, export_id);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (type == HAB_IMP_MMAP_NODE_UVA) {
		ret = munmap(buff_shared, node->size);
		if (ret != 0) {
			UHAB_LOG("return %d on 0x%llx, size %d, exportid %d, vcid %X, error code %s\n",
					ret, (long long)node->value, node->size, node->exportid, handle, strerror(errno));
			UHAB_LOG("failed to unmap address 0x%llx, but continue to unimport in khab!\n", (long long)node->value);
		} else {
			UHAB_DBG("return %d on 0x%llx size %d exportid %d, vcid %X\n",
				ret, (long long)node->value, node->size, node->exportid, handle);
		}
	}
	if ((type == HAB_IMP_MMAP_NODE_FD) &&
			(0U == (flags & HABMM_UNIMP_FLAGS_FD_ALREADY_CLOSED))) {
		(void)close((int)(intptr_t)buff_shared); // fd
	}

	free(node);
#elif defined(__INTEGRITY)
	UHAB_TRACE_IN(handle);

	if ((flags & HABMM_EXPIMP_FLAGS_FD) == 0) { // do we have to tie uva to exp_id and store to a node?
		ret = pmem_unmap(buff_shared); // pmem v1 assume portable id (pa) is always continuous
		if (ret) {
			UHAB_LOG("failed to pmem_unmap address %p ret %d\n", buff_shared, ret);
			UHAB_TRACE_OUT(handle);
			return ret; // client has to use correct uva do we need to tie uva and exp_id?
		}
	}
#else
	UHAB_TRACE_IN(handle);
	(void)buff_shared;
#endif
#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &arg, (int)sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
#endif
	if (ret != 0) {
		UHAB_LOG("failed to unimport ret %d, vcid %X\n", ret, handle);
	} else {
	    UHAB_DBG("unimport successful: exportid %d, vcid %X\n", export_id, handle);
    }

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
 * Description:
 *
 * Query various information of the opened hab socket.
 *
 * Params:
 *
 * in handle - communication channel created by habmm_socket_open
 * in habmm_socket_info - retrieve socket information regarding local and remote
 *                        VMs
 * in flags - future extension
 *
 * Return:
 * status (success/failure)
 *
 */
int32_t habmm_socket_query(int32_t handle, struct hab_socket_info *info,
		uint32_t flags)
{
	int32_t ret = 0;
	char names[sizeof(info->vmname_remote)*2U] = {0}; /* single retrieving buffer for two names */
	__u64 tmp = 0U;

	struct hab_info arg = {
		.vcid = handle,
		.names = (uintptr_t) names,
		.namesize = (uint32_t)sizeof(names),
		.flags = flags
	};
	uint32_t major_mmid = VC_HANDLE2MAJOR_MMID(handle);
#ifdef __linux__
	int fd;
#endif

	UHAB_TRACE_IN(handle);

	if (handle == 0) {
		UHAB_LOG("Error! vcid 0 is given\n");
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	if (info == NULL) {
		ret = -EINVAL;
		UHAB_LOG("return %d on query vcid %X, NULL parameter\n", ret, handle);
		UHAB_TRACE_OUT(handle);
		return ret;
	}

	if (major_mmid >= MM_ID_MAX) {
		UHAB_LOG("Error! invalid major mmid %d is given\n", major_mmid);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

#ifdef __linux__
	fd = get_fd_by_mmid(major_mmid);
	if (fd < 0) {
		UHAB_LOG("major mmid %d Invalid fd %d\n", major_mmid, fd);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	ret = hab_DevCtl(IOCTL_HAB_VC_QUERY, &arg, (int)sizeof(arg), NULL, 0,
					 &arg.ids, (int)sizeof(arg.ids), (void*)(uintptr_t)arg.names, (int)arg.namesize, fd);
#else
	ret = hab_DevCtl(IOCTL_HAB_VC_QUERY, &arg, (int)sizeof(arg), NULL, 0,
					 &arg.ids, (int)sizeof(arg.ids), (void*)(uintptr_t)arg.names, (int)arg.namesize);
#endif
	if (ret != 0) {
		UHAB_LOG("return %d on query vcid %X\n", ret, handle);
	} else {
		tmp = arg.ids & 0xFFFFFFFFU;
		info->vmid_local = (int32_t)tmp;
		tmp = (arg.ids & 0xFFFFFFFF00000000UL) >> 32;
		info->vmid_remote = (int32_t)tmp;

#if !defined(__INTEGRITY)
		// ToDo GHS: names needs to be returned properly. now it is conflicting with recv's size
		char *nm = (char*)(uintptr_t)arg.names;
		(void)strlcpy(info->vmname_local, nm, sizeof(info->vmname_local));
		(void)strlcpy(info->vmname_remote, &nm[sizeof(info->vmname_remote)], sizeof(info->vmname_remote)); // single buffer for two names
#else
		// ToDo GHS: names needs to be returned properly. now it is conflicting with recv's size
		info->vmname_remote[0] = (char)0;
		info->vmname_local[0] = (char)0;
#endif
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
int habmm_vmm_register_event_notification(const char *name, const hab_vmm_client_handler client_func,
									uint32_t vmid_event_mask, uint32_t level,
									void *priv_data, void **handle)
{
	return vmm_register_event_notification(name, client_func, vmid_event_mask, level, priv_data, handle);
}

int habmm_vmm_unregister_event_notification(void *handle)
{
	return vmm_unregister_event_notification(handle);
}
#elif defined(__QNXNTO__)
int habmm_vmm_register_event_notification(const char *name, const hab_vmm_client_handler client_func,
									uint32_t vmid_event_mask, uint32_t level,
									void *priv_data, void **handle)
{
	return ENOSYS;
}

int habmm_vmm_unregister_event_notification(void *handle)
{
	return ENOSYS;
}
#endif

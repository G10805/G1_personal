/*========================================================================

*//** @file hyp_video_intercept.cpp

@par DESCRIPTION:
Linux video decoder hypervisor front-end implementation

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/30/24   bf      Enhance debug_level setting mechanism under linux
03/20/24   mm      Correct the number of pending events in queue
03/19/24   su      Update event queue size
12/22/23   bh      Fix alloc-free-mismatch caused memory leakage issue
09/06/23   su      Exit pthread_timedwait only if there is a valid event
08/30/23   nb      Fix compilation errors due to additon of new compiler flags
08/30/23   br      Change boot kpi markers
03/23/22   rm      Handle the poll error case and retry poll in EINTR or EAGAIN case
07/13/21   bf      Add out of bound check for Array index in close_thread()
03/23/21   rq      No unloading hyp video lib just because hyp_video_open error
03/23/21   rq      Add place_marker when FE open
08/21/20   sh      Add initial version of the file
========================================================================== */

/* =======================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/version.h>
#include "hyp_video_intercept.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif /* USE_GLIB */

#define MAX_HYP_VIDEO_HANDLE             32
#define HYP_VIDEO_HANDLE_SIGNATURE       0x2bcd0000
#define HYP_VIDEO_HANDLE_SIGNATURE_MASK  0xffff0000
#define HYP_VIDEO_HANDLE_MASK            0x0000ffff
#define POLL_TIMEOUT                0xffffffff
#define MAX_EVENTS                  50
#define MAX_COUNT                   20

#define IS_HYPERVISOR_VIDEO_HANDLE(fd) ((fd & HYP_VIDEO_HANDLE_SIGNATURE_MASK)==HYP_VIDEO_HANDLE_SIGNATURE)
#define HYP_INITIALIZED  (g_hyp_video_handle_count > 0)
#define NUM_PENDING_EVENTS(front, rear) (((rear) >= (front)) ? ((rear) - (front)) : (MAX_EVENTS - ((front) - (rear))))

typedef void* HYP_VIDEO_HANDLE;
typedef int (*hyp_video_callback_handler_t)(void *context, void *message);

struct hyp_video_intercept {
    HYP_VIDEO_HANDLE  handle;
    short event_flags[MAX_EVENTS];
    bool exit_flag;
    unsigned int event_q_front;
    unsigned int event_q_rear;
    pthread_t thread_id;
    pthread_cond_t cond;
    pthread_mutex_t lock;
};

struct hyp_video_callback_t
{
    hyp_video_callback_handler_t handler;
    void* context;
};

typedef void (*cb)(int flag);
typedef HYP_VIDEO_HANDLE (*video_fe_open_func)(const char*, int, hyp_video_callback_t*);
typedef int (*video_fe_ioctl_func)(HYP_VIDEO_HANDLE, int, void*);
typedef int (*video_fe_close_func)(HYP_VIDEO_HANDLE);

static void *hyp_video_lib_handle = NULL;
static video_fe_open_func video_fe_open = NULL;
static video_fe_ioctl_func video_fe_ioctl = NULL;
static video_fe_close_func video_fe_close = NULL;
static pthread_mutex_t g_hyp_video_handle_lock = PTHREAD_MUTEX_INITIALIZER;
static struct hyp_video_intercept g_hyp_video_handle[MAX_HYP_VIDEO_HANDLE];
static int g_hyp_video_handle_count  = 0;
static int event_handler(void *context, void *messages);
int debug_level = 0x1;


/**===========================================================================

FUNCTION place_marker

@brief  Print the KPI markers

@param [in] string of the marker, up to 128 bytes

@dependencies
  None

@return
  None

===========================================================================*/
static int onetime_flag = 1;
#define MARKER_STR_LEN 128
#define KPI_VALUES_STR "/sys/kernel/debug/bootkpi/kpi_values"

void place_marker(char const *str)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
   int fd = open(KPI_VALUES_STR, O_WRONLY);
   if (fd > 0) {
      char marker[MARKER_STR_LEN] = {0};
      strlcpy(marker, str, sizeof(marker));
      write(fd, marker, strlen(marker));
      close(fd);
   }
#else
   /* debug info, hence not adding boot_kpi string.
    */
   HYP_VIDEO_MSG_ERROR("%s", str);
#endif
}

static int insert_handle(HYP_VIDEO_HANDLE handle, int index)
{
    int rc = 0;

    memset(&g_hyp_video_handle[index], 0, sizeof(struct hyp_video_intercept));

    if (pthread_mutex_init(&g_hyp_video_handle[index].lock, NULL) != 0) {
        HYP_VIDEO_MSG_ERROR("Failed to initialize pthread lock");
        rc = -1;
    }

    if (rc >= 0 && pthread_cond_init(&g_hyp_video_handle[index].cond, NULL) != 0) {
        HYP_VIDEO_MSG_ERROR("Failed to initialize pthread cond");
        pthread_mutex_destroy(&g_hyp_video_handle[index].lock);
        rc = -1;
    }

    if (rc >= 0) {
        g_hyp_video_handle[index].handle = handle;
        g_hyp_video_handle_count++;
    }

    return rc;
}

static int get_index(void)
{
    int rc = 0;

    if (g_hyp_video_handle_count >= MAX_HYP_VIDEO_HANDLE) {
        HYP_VIDEO_MSG_ERROR("handle count %d, Reached MAXIMUM handle count",
                             g_hyp_video_handle_count);
        rc = -1;
    } else {
        int i;

        for (i = 0; i < MAX_HYP_VIDEO_HANDLE; i++) {
            if (g_hyp_video_handle[i].handle == 0) {
                rc = i;
                break;
            }
        }

        if (i >= MAX_HYP_VIDEO_HANDLE) {
            HYP_VIDEO_MSG_ERROR("failed to find empty slot");
            rc = -1;
        }
    }

    return rc;
}

static int hyp_video_init(void)
{
    int rc = 0;

    hyp_video_lib_handle = dlopen("libhyp_video_fe.so", RTLD_NOW);
    if (hyp_video_lib_handle == NULL) {
        hyp_video_lib_handle = dlopen("libhyp_video_fe.so.0", RTLD_NOW);
    }
    if (hyp_video_lib_handle == NULL) {
        HYP_VIDEO_MSG_ERROR("failed to open libhyp_video_fe");
        rc = -1;
    } else {
        video_fe_open = (video_fe_open_func)dlsym(hyp_video_lib_handle, "video_fe_open");
        if (video_fe_open == NULL) {
            HYP_VIDEO_MSG_ERROR("failed to get video_fe_open handle");
            rc = -1;
        } else {
            video_fe_ioctl = (video_fe_ioctl_func)dlsym(hyp_video_lib_handle, "video_fe_ioctl");
            if (video_fe_ioctl == NULL) {
                HYP_VIDEO_MSG_ERROR("failed to get video_fe_ioctl handle");
                rc = -1;
            } else {
                video_fe_close = (video_fe_close_func)dlsym(hyp_video_lib_handle, "video_fe_close");
                if (video_fe_close == 0) {
                    HYP_VIDEO_MSG_ERROR("failed to get video_fe_close handle");
                    rc = -1;
                }//video_fe_close
            } //video_fe_ioctl
        } //video_fe_open
    } //hyp_video_lib_handle

    if (rc < 0 && hyp_video_lib_handle) {
        dlclose(hyp_video_lib_handle);
        hyp_video_lib_handle = NULL;
    }

    return rc;
}

static void hyp_video_deinit(void)
{
    if (hyp_video_lib_handle)
    {
        dlclose(hyp_video_lib_handle);
        hyp_video_lib_handle = NULL;
    }

    return;
}

int hyp_video_open(const char *str, int flag)
{
    int rc = 0;

#ifdef _LINUX_
    char *env_ptr = getenv("HYPV_DEBUG_LEVEL");
    debug_level = env_ptr ? atoi(env_ptr) : 1;
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    if (property_get("vendor.hypv.debug.level", prop_val, NULL) > 0 && prop_val[0] != '\0' && prop_val[PROPERTY_VALUE_MAX-1] == '\0') {
        debug_level = atoi(prop_val);
    }
#elif defined _ANDROID_
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.hypv.debug.level", property_value, "1");
    debug_level = atoi(property_value);
#endif

    pthread_mutex_lock(&g_hyp_video_handle_lock);

    if (!HYP_INITIALIZED) {
        if ((rc = hyp_video_init()) < 0) {
            HYP_VIDEO_MSG_ERROR("Failed to initialize hypervisor");
            pthread_mutex_unlock(&g_hyp_video_handle_lock);
            return rc;
        }
    }

    int index = get_index();
    if (index < 0) {
        rc = -1;
    } else {
        struct hyp_video_callback_t cb;

        cb.handler = event_handler;
        cb.context = &g_hyp_video_handle[index];
        HYP_VIDEO_HANDLE hyp_video_handle = video_fe_open(str, flag, &cb);
        HYP_VIDEO_MSG_INFO("video fe open handle = %p", hyp_video_handle);

        if (hyp_video_handle == NULL) {
            HYP_VIDEO_MSG_ERROR("Failed to open video FE");
            rc = -1;
        } else {
            if (insert_handle(hyp_video_handle, index) < 0) {
                HYP_VIDEO_MSG_ERROR("failed to add hvfe handle");
                video_fe_close(hyp_video_handle);
                rc = -1;
            } else {
                rc = (HYP_VIDEO_HANDLE_SIGNATURE | index);
            }
        }
    }

    if (onetime_flag) {
        place_marker("M - Hyp Video FE Opens");
       onetime_flag = 0;
    }

    if (!HYP_INITIALIZED) {
        hyp_video_deinit();
    }

    pthread_mutex_unlock(&g_hyp_video_handle_lock);

    return rc;
}

int hyp_video_ioctl(int fd, int cmd, void *data)
{
    int rc = 0;

    if (!HYP_INITIALIZED) {
        HYP_VIDEO_MSG_ERROR("hypervisor not initialized");
        return -1;
    }

    if (IS_HYPERVISOR_VIDEO_HANDLE(fd)) {
        int fd_index = fd & HYP_VIDEO_HANDLE_MASK;
        if (fd_index >= MAX_HYP_VIDEO_HANDLE) {
            HYP_VIDEO_MSG_ERROR("invalid fd_index = %d", fd_index);
            rc = -1;
        } else {
            rc = video_fe_ioctl(g_hyp_video_handle[fd_index].handle, cmd, data);
            HYP_VIDEO_MSG_INFO("fd %d, fd_index %d, cmd 0x%x, data 0x%p, rc %d",
                              fd, fd_index, (unsigned int)cmd, data, rc);
        }
    } else {
        HYP_VIDEO_MSG_ERROR("native ioctl: fd %d, cmd 0x%x, data 0x%p",
                             fd, (unsigned int)cmd, data);
        rc = ioctl(fd, cmd, data);
    }

    return rc;
}

static int event_handler(void *context, void *messages)
{
    struct hyp_video_intercept *handle = (struct hyp_video_intercept *)context;
    int flags = *(int *)messages;

    HYP_VIDEO_MSG_INFO("event flag 0x%x", (unsigned int)flags);
    pthread_mutex_lock(&handle->lock);
    handle->event_flags[handle->event_q_rear++] = flags;
    handle->event_q_rear %= MAX_EVENTS;
    HYP_VIDEO_MSG_INFO("cond signal. num_pending_events %u event_q_front %u event_q_rear %u",
                        NUM_PENDING_EVENTS(handle->event_q_front, handle->event_q_rear),
                        handle->event_q_front, handle->event_q_rear);
    pthread_cond_signal(&handle->cond);
    pthread_mutex_unlock(&handle->lock);

    return 0;
}

static void* close_thread(void *fds)
{
    struct pollfd *pfds = (struct pollfd *)fds;
    int fd_index = pfds[0].fd & HYP_VIDEO_HANDLE_MASK;
    struct pollfd exit_fd;
    int rc = 0, max_retry_count = MAX_COUNT;
    bool loop = true;

    if (fd_index >= MAX_HYP_VIDEO_HANDLE) {
        HYP_VIDEO_MSG_ERROR("invalid fd index %d", fd_index);
    } else {
        struct hyp_video_intercept *handle = &g_hyp_video_handle[fd_index];
        HYP_VIDEO_MSG_INFO("close thread created. fd = %d", fd_index);
        exit_fd.events = POLLIN | POLLERR;
        exit_fd.fd = pfds[1].fd;

        while (loop) {
            rc = poll(&exit_fd, 1, POLL_TIMEOUT);
            if (!rc) {
                HYP_VIDEO_MSG_ERROR("Poll Timeout");
                loop = false;
            } else if (rc < 0 && errno != EINTR && errno != EAGAIN) {
                HYP_VIDEO_MSG_ERROR("Error while polling: %d, errno = %d", rc, errno);
                loop = false;
            } else if (rc < 0 && (errno == EINTR || errno == EAGAIN)) {
                HYP_VIDEO_MSG_INFO("Poll retry");
                max_retry_count--;
                if (!max_retry_count) {
                    loop = false;
                    HYP_VIDEO_MSG_ERROR("Poll reached max retry count rc: %d, errno = %d", rc, errno);
                }
            } else if (rc > 0) {
                HYP_VIDEO_MSG_INFO("Poll Success");
                loop = false;
            }
        }

        if ((exit_fd.revents & POLLIN) || (exit_fd.revents & POLLERR) || (rc <= 0)) {
            handle->exit_flag = true;
            pthread_cond_signal(&handle->cond);
        }
    }

    delete []pfds;

    return NULL;
}

int hyp_video_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    struct timespec ts;
    int ret = 0;
    bool is_exit = false;

    if (nfds == 0)
        return -1;

    if (!HYP_INITIALIZED) {
        HYP_VIDEO_MSG_ERROR("hypervisor not initialized");
        return -1;
    }

    if (IS_HYPERVISOR_VIDEO_HANDLE(fds[0].fd)) {
        int fd_index = fds[0].fd & HYP_VIDEO_HANDLE_MASK;

        if (fd_index >= MAX_HYP_VIDEO_HANDLE) {
            HYP_VIDEO_MSG_ERROR("invalid fd index %d", fd_index);
            ret = -1;
        } else {
            struct hyp_video_intercept *handle = &g_hyp_video_handle[fd_index];

            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec = 0;
            fds[1].revents = fds[0].revents = 0;

            if (handle->thread_id == 0) {
                struct pollfd *pfds = new struct pollfd[2];
                if (pfds == nullptr) {
                    HYP_VIDEO_MSG_ERROR("failed to new pollfd[2]");
                    return -1;
                }
                pfds[0].fd = fds[0].fd;
                pfds[1].fd = fds[1].fd;

                if (pthread_create(&handle->thread_id, 0, close_thread, pfds)) {
                    HYP_VIDEO_MSG_ERROR("failed to create close_thread");
                    handle->thread_id = 0;
                    return -1;
                }
            }

            pthread_mutex_lock(&handle->lock);
            do
            {
               if (!NUM_PENDING_EVENTS(handle->event_q_front, handle->event_q_rear) &&
                     !handle->exit_flag) {
                  /* ETIMEDOUT will return as 110 */
                  ret = pthread_cond_timedwait(&handle->cond, &handle->lock, &ts);
               }
               else
               {
                  HYP_VIDEO_MSG_INFO("hyp_video_poll: process pending flag");
                  is_exit = true;
               }
            } while ((ret != ETIMEDOUT) && !is_exit);

            if (ret == ETIMEDOUT) {
                HYP_VIDEO_MSG_INFO("hyp poll timeout");
                ret = 0;
            } else if (ret == 0) {
                if (handle->exit_flag == true) {
                    HYP_VIDEO_MSG_INFO("hyp poll exit");
                    fds[1].revents = POLLIN;
                    if (handle->thread_id != 0) {
                        pthread_join(handle->thread_id, NULL);
                    }
                    handle->exit_flag = false;
                    handle->thread_id = 0;
                } else {
                    fds[0].revents = handle->event_flags[handle->event_q_front++];
                    handle->event_q_front %= MAX_EVENTS;
                    HYP_VIDEO_MSG_INFO("hyp poll fd %d events 0x%x pending events %u",
                                        fds[0].fd, (unsigned int)fds[0].revents,
                                        NUM_PENDING_EVENTS(handle->event_q_front, handle->event_q_rear));
                }
                ret = 1;
            }

            pthread_mutex_unlock(&handle->lock);
        }
    } else {
        HYP_VIDEO_MSG_ERROR("unknown fd = %d", fds[0].fd);
    }

    return ret;
}

int hyp_video_close(int fd)
{
    int rc = 0;

    if (!HYP_INITIALIZED) {
        HYP_VIDEO_MSG_ERROR("hypervisor not initialized");
        return -1;
    }

    if (IS_HYPERVISOR_VIDEO_HANDLE(fd)) {
        int fd_index = fd & HYP_VIDEO_HANDLE_MASK;

        if ((fd_index >= MAX_HYP_VIDEO_HANDLE) || (fd_index < 0)) {
            HYP_VIDEO_MSG_ERROR("invalid fd %d", fd_index);
            rc = -1;
        } else {
            pthread_mutex_lock(&g_hyp_video_handle_lock);
            rc = video_fe_close(g_hyp_video_handle[fd_index].handle);
            g_hyp_video_handle[fd_index].handle = 0;
            pthread_cond_destroy(&g_hyp_video_handle[fd_index].cond);
            pthread_mutex_destroy(&g_hyp_video_handle[fd_index].lock);
            if (--g_hyp_video_handle_count == 0)
                hyp_video_deinit();
            pthread_mutex_unlock(&g_hyp_video_handle_lock);
        }
    } else {
        rc = close(fd);
    }

    return rc;
}

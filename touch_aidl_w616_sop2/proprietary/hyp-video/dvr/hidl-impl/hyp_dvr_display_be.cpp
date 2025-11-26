/*===========================================================================

*//** @file hyp_dvr_display_be.cpp
This file implements dvr hal functions

Copyright (c) 2019-2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header:  $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
08/30/23           br          Change boot kpi markers
11/02/22           sk          Use flag and change header path for kernel 5.15
05/27/22           nd          Fix KW issues
07/07/21           su          Initialize varibles to avoid K/W errors
03/24/20           sh          Add P010 format
01/15/20           sh          Bringup DVR on LA GVM on Hana
01/22/19           sh          Impement HIDL interface for DVR
07/10/19           sm          Wait on fence before updating the buffer
============================================================================*/
#include "hyp_dvr_display_be.h"
#include <dlfcn.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/version.h>
#include "MMCriticalSection.h"
#include "MMTimer.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#include <binder/ProcessState.h>
#include <poll.h>
#ifdef SUPPORT_DMABUF
#include <vidc/media/msm_media_info.h>
#else
#include <media/msm_media_info.h>
#endif

#define MAX_BUFFERS             6
#define FENCE_WAIT_TIMEOUT      60
#define BOOTKPI_DEVICE       "/sys/kernel/debug/bootkpi/kpi_values"
#define DVR_FIFO_NAME   "/mnt/vendor/dvr_ctl/dvrpipe"
#define DVR_INPUT_POLL_TIMEOUT               2000 //ms
#define MAX_MSG_LEN     80
#define DISPLAY_FILE_NAME    "/mnt/vendor/dvr_mux/hdvr_disp_output.yuv"
#define PREVIEW_MM_SUBID   2

NATIVE_HANDLE_DECLARE_STORAGE(dvrHandle, MAX_BUFFERS, 0);
struct native_handle* native_fd_handle;
int debug_level = 0x1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int dvr_pipe_fd = -1;
static boolean preview_enable = FALSE;
MM_HANDLE dvr_thread_handle = NULL;

struct win_buf
{
    void *va;
};

static struct win_buf window_bufs[MAX_BUFFERS];

namespace vendor {
namespace qti {
namespace dvr {
namespace V1_0 {
namespace implementation {

Return<void> DvrDisplay::getDisplayInfo(DvrDisplay::getDisplayInfo_cb _hidl_cb)
{
    mDisplayReady = TRUE;
    _hidl_cb(0, mDisplayInfo);

    return Void();
}

Return<int32_t> DvrDisplay::registerCallback(const sp<IDvrDisplayStreamCB>& streamObj)
{
    mDisplayStreamObj = streamObj;
    HYP_VIDEO_MSG_LOW("streamObj registered");
    return 0;
}

Return<int32_t> DvrDisplay::setDisplayBuffers(const hidl_handle& hidl_hndl)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    const native_handle_t* fdHandle = hidl_hndl.getNativeHandle();

    for (int32_t i = 0; i < MAX_BUFFERS; i++)
    {
        int32_t fd = fdHandle->data[i];
        window_bufs[i].va = (char *)mmap(NULL, mDisplayInfo.buf_size,
                            PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

        if(window_bufs[i].va == (void *)-1)
        {
            HYP_VIDEO_MSG_ERROR("Failed to map buffer: %s", strerror(errno));
            hdvr_status = HDVR_STATUS_FAIL;
            break;
        }
        HYP_VIDEO_MSG_LOW("index = %d fd = %d va = %p size = %d",
                  i, fd, window_bufs[i].va, mDisplayInfo.buf_size);
    }

    if (HDVR_STATUS_FAIL == hdvr_status)
    {
        HYP_VIDEO_MSG_ERROR("Deinitializung display");
        display_deinit(mDisplayInfo.buf_size);
    }

    return hdvr_status;
}

hdvr_status_type DvrDisplay::is_display_ready()
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    if (!mDisplayReady)
    {
        hdvr_status = HDVR_STATUS_FAIL;
    }

    return hdvr_status;
}

hdvr_status_type DvrDisplay::display_init()
{
    int32_t hdvr_status = HDVR_STATUS_SUCCESS;

    hdvr_status = mDisplayStreamObj->display_init();

    return static_cast<hdvr_status_type>(hdvr_status);
}

void DvrDisplay::display_buffer(char* va, uint32 size)
{
    int32_t buf_index = mDisplayStreamObj->get_display_buf_index();
    HYP_VIDEO_MSG_LOW("display_buffer :: buf_index %d size %d", buf_index, size);

    if (MAX_BUFFERS > buf_index)
    {
        int32_t ret = 0;
        HABMM_MEMCPY(window_bufs[buf_index].va, va, size);
        ret = mDisplayStreamObj->display_buffer(buf_index);
        if (!ret)
        {
            HYP_VIDEO_MSG_ERROR("Failed to display buffer for index %d", buf_index);
        }
    }
    else
    {
        HYP_VIDEO_MSG_ERROR("Received invalid buffer index %d", buf_index);
    }
}

void DvrDisplay::display_deinit(uint32 size)
{
    for (unsigned i = 0; i < MAX_BUFFERS; i++)
    {
         if (NULL != window_bufs[i].va)
         {
             if (0 != munmap(window_bufs[i].va, size))
             {
                 HYP_VIDEO_MSG_ERROR("Failed to unmap buffer on index %d", i);
             }
             window_bufs[i].va = NULL;
         }
    }
    mDisplayStreamObj->display_deinit();
}

static int dvr_input_poll_thread(void *msg)
{
    char msg_id[MAX_MSG_LEN] = {0};
    int32 done = 0;
    int32 rc = 0;
    int32 enable = 0;
    struct pollfd fds[1];
    (void)msg;

    HYP_VIDEO_MSG_LOW("thread started");
    fds[0].events = POLLIN | POLLPRI | POLLERR;
    while (!done)
    {
        fds[0].fd = dvr_pipe_fd;
        rc = poll(fds, 1, DVR_INPUT_POLL_TIMEOUT);
        if (-1 == rc)
        {
            HYP_VIDEO_MSG_ERROR("error in polling pipe. rc = %d", rc);
        }
        else
        {
            if (fds[0].revents & POLLIN)
            {
                rc = read(dvr_pipe_fd, &msg_id, sizeof(msg_id));
                if (-1 == rc)
                {
                    HYP_VIDEO_MSG_ERROR("error in reading pipe. rc = %d", rc);
                }
                else
                {
                    enable = atoi(msg_id);
                    if (0 == enable)
                    {
                        preview_enable = FALSE;
                    }
                    else if (1 == enable)
                    {
                        preview_enable = TRUE;
                    }
                    HYP_VIDEO_MSG_ERROR("read message in the pipe %d preview_enable = %d",
                                         enable, preview_enable);
                }
            }
        }
    }

    return 0;
}

static void dvr_pipe_init(void)
{
    int32 rc = 0;

    /* start a thread to monitor inputs */
    if (0 == access(DVR_FIFO_NAME, F_OK))
    {
        unlink(DVR_FIFO_NAME);
    }

    /* create named pipe */
    rc = mkfifo(DVR_FIFO_NAME, 0660);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("failed to create named pipe. rc = %d", rc);
    }
    else
    {
        if (0 != access(DVR_FIFO_NAME, F_OK))
        {
            HYP_VIDEO_MSG_ERROR("Named pipe File doesn't exist");
            return;
        }
        chmod(DVR_FIFO_NAME, 0660);

        dvr_pipe_fd = open(DVR_FIFO_NAME, O_RDWR);
        if (-1 == dvr_pipe_fd)
        {
            HYP_VIDEO_MSG_ERROR("failed to open named pipe");
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("opened named pipe");
            if (NULL == dvr_thread_handle)
            {
                rc = MM_Thread_Create(MM_Thread_DefaultPriority, 0, dvr_input_poll_thread,
                                      (void *)&dvr_pipe_fd, 0, &dvr_thread_handle);
                if (0 != rc)
                {
                    HYP_VIDEO_MSG_ERROR("failed to create. dynamic preview control will be disabled");
                    close(dvr_pipe_fd);
                }
                else
                {
                    HYP_VIDEO_MSG_HIGH("dvr input thread created");
                    MM_Thread_Detach(dvr_thread_handle);
                }
            }
        }
    }
}

static void add_kpi_marker(const char *marker)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
    if (0 == access(BOOTKPI_DEVICE, F_OK))
    {
        int fd = open(BOOTKPI_DEVICE, O_RDWR);

        if (-1 < fd)
        {
            write(fd, marker, strlen(marker));
            close(fd);
        }
    }
#else
    /* debug info, hence not adding boot_kpi string.
     */
    HYP_VIDEO_MSG_ERROR("%s", marker);
#endif
}

hdvr_status_type DvrDisplay::hdvr_version_check(hdvr_session_t* hdvr_session,
                                           hypdvr_msg_type* msg)
{
    int32 rc = 0;
    hypdvr_msg_type msg2;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;

    if (GET_MAJOR_REV(msg->version) != GET_MAJOR_REV(HYP_DVR_VERSION) ||
        GET_MINOR_REV(msg->version) != GET_MINOR_REV(HYP_DVR_VERSION))
    {
        hdvr_status = HDVR_STATUS_FAIL;

        msg2.msg_id = HYPDVR_MSGRESP_OPEN_RET;
        msg2.data.open_data.return_status = HDVR_STATUS_VERSION_MISMATCH;
        msg2.data_size = msg->data_size;
        msg2.virtual_channel = hdvr_session->habmm_handle;
        msg2.pid = (uint32) getpid();
        HYP_VIDEO_MSG_ERROR("Version mismatch FE(%d.%d) BE(%d.%d)",
                             GET_MAJOR_REV(msg->version), GET_MINOR_REV(msg->version),
                             GET_MAJOR_REV(HYP_DVR_VERSION), GET_MINOR_REV(HYP_DVR_VERSION));

        MM_CriticalSection_Enter(hdvr_session->send_crit_section);
        rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                           sizeof(hypdvr_msg_type), 0);
        MM_CriticalSection_Leave(hdvr_session->send_crit_section);
        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("failed to send socket rc %d", rc);
        }
    }

    return hdvr_status;
}

static uint32 calc_buffer_size(uint32 color_format, uint32 width, uint32 height)
{
    enum color_fmts color;

    switch (color_format)
    {
    case HDVR_PIXEL_FORMAT_TYPE_NV12:
        color = COLOR_FMT_NV21;
        break;
    case HDVR_PIXEL_FORMAT_TYPE_RGB888:
        color = COLOR_FMT_RGBA8888;
        break;
    case HDVR_PIXEL_FORMAT_TYPE_P010:
        color = COLOR_FMT_P010;
        break;
    default:
        color = COLOR_FMT_NV21;
    }

    return (VENUS_BUFFER_SIZE(color, width, height));
}

void DvrDisplay::hdvr_be_ioctl(hdvr_session_t* hdvr_session, hypdvr_msg_type* pMsgBufferNode)
{
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;

    pthread_mutex_lock(&mutex);
    msg2.pid = (uint32) getpid();

    HABMM_MEMSET(msg2.data.ioctl_data.payload, 0,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));
    /* initialize return ioctl payload */
    HABMM_MEMCPY(msg2.data.ioctl_data.payload, pMsgBufferNode->data.ioctl_data.payload,
                 GET_IOCTL_PAYLOAD_SIZE_FROM_MSG_DATASIZE(pMsgBufferNode->data_size));

    switch (pMsgBufferNode->data.ioctl_data.dvr_ioctl)
    {
        case IOCTL_HDVR_DISP_BUF_INFO:
        {
            hdvr_disp_buf64_info* buf_info = (hdvr_disp_buf64_info* )msg2.data.ioctl_data.payload;
            char *va = NULL;

            if (0 != access(DVR_FIFO_NAME, F_OK))
            {
                HYP_VIDEO_MSG_HIGH("dvr pipe file doesn't exist");
                close(dvr_pipe_fd);
                dvr_pipe_init();
            }

            if (FALSE == hdvr_session->recv_disp_buf)
            {
                hdvr_session->recv_disp_buf = TRUE;
                add_kpi_marker("M - DVR:First display buf");
                mDisplayInfo.width = buf_info->width;
                mDisplayInfo.height = buf_info->height;
                mDisplayInfo.format = buf_info->format;
                mDisplayInfo.buf_size = hdvr_session->display_buf_size =
                           calc_buffer_size(buf_info->format, buf_info->width, buf_info->height);
            }
            HYP_VIDEO_MSG_LOW("display buf info export_id=%d width=%d "
                              "height=%d format=%d size=%d fill_len=%d",
                               buf_info->export_id, buf_info->width,
                               buf_info->height, buf_info->format,
                               buf_info->buf_size, buf_info->fill_len);
            va = (char *)hypv_map_add_from_remote(&hdvr_session->habmm_if, &hdvr_session->lookup_queue,
                                                   hdvr_session->habmm_handle, buf_info->buf_size,
                                                   buf_info->export_id);
            if (NULL == va)
            {
                HYP_VIDEO_MSG_ERROR("Error in retriving va from export id %d",
                                     buf_info->export_id);
                hdvr_status = HDVR_STATUS_FAIL;
            }
            else if (TRUE == preview_enable)
            {
                HYP_VIDEO_MSG_INFO("va = %p export id = %d",
                                    va, buf_info->export_id);

                if (HDVR_STATUS_SUCCESS == is_display_ready())
                {
                    if (FALSE == hdvr_session->display_init_done)
                    {
                        if (HDVR_STATUS_SUCCESS != display_init())
                        {
                            HYP_VIDEO_MSG_ERROR("error in display init.dump to file");
                            hdvr_session->display_dump_fd = open(DISPLAY_FILE_NAME,
                               O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
                            if (-1 == hdvr_session->display_dump_fd)
                            {
                                HYP_VIDEO_MSG_ERROR("fail to open file %s to dump display frames",
                                                     DISPLAY_FILE_NAME);
                                hdvr_status = HDVR_STATUS_FAIL;
                            }
                        }
                        else
                        {
                            hdvr_session->display_init_done = TRUE;
                        }
                    }

                    if (TRUE == hdvr_session->display_init_done)
                    {
                        display_buffer(va, buf_info->fill_len);
                        if (FALSE == hdvr_session->first_preview)
                        {
                            hdvr_session->first_preview = TRUE;
                            add_kpi_marker("M - DVR:First frame to display");
                        }
                    }
                }
            }
            else
            {
                if (TRUE == hdvr_session->display_init_done)
                {
                    display_deinit(hdvr_session->display_buf_size);
                    hdvr_session->display_init_done = FALSE;
                }
            }
            if (-1 != hdvr_session->display_dump_fd)
            {
                write(hdvr_session->display_dump_fd, va, buf_info->fill_len);
            }
            break;
        }
        default:
        {
            HYP_VIDEO_MSG_ERROR("bad ioctl cmd = %d",
                                 pMsgBufferNode->data.ioctl_data.dvr_ioctl);
            hdvr_status = HDVR_STATUS_FAIL;
        }
    }
    msg2.data.ioctl_data.return_value = hdvr_status;
    msg2.data.ioctl_data.dvr_ioctl = pMsgBufferNode->data.ioctl_data.dvr_ioctl;
    msg2.msg_id = HYPDVR_MSGRESP_IOCTL_RET;
    msg2.data_size = pMsgBufferNode->data_size;

    MM_CriticalSection_Enter(hdvr_session->send_crit_section);
    rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                       sizeof(hypdvr_msg_type), 0);
    MM_CriticalSection_Leave(hdvr_session->send_crit_section);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
    }
    pthread_mutex_unlock(&mutex);
}

void DvrDisplay::hdvr_be_open(hdvr_session_t* hdvr_session,
                         hypdvr_msg_type* pMsgBufferNode)
{
    int handle = -1;
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;

    pthread_mutex_lock(&mutex);
    handle = 0x1;
    hdvr_session->handle_64b = (int64)(handle);
    msg2.msg_id = HYPDVR_MSGRESP_OPEN_RET;
    msg2.data.open_data.return_io_handle = (int64)(handle);
    msg2.data.open_data.return_status = hdvr_status;
    msg2.data.open_data.drv_session = (hypdvr_session_type)2;
    msg2.data_size = pMsgBufferNode->data_size;
    msg2.pid = (uint32) getpid();

    MM_CriticalSection_Enter(hdvr_session->send_crit_section);
    rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                       sizeof(hypdvr_msg_type), 0);
    MM_CriticalSection_Leave(hdvr_session->send_crit_section);
    if (0 != rc)
    {
        HYP_VIDEO_MSG_ERROR("habmm socket send fail rc = %d", rc);
    }
    pthread_mutex_unlock(&mutex);
}

void DvrDisplay::hdvr_be_close(hdvr_session_t* hdvr_session,
                          hypdvr_msg_type* pMsgBufferNode)
{
    int32 rc = 0;
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    hypdvr_msg_type msg2;

    pthread_mutex_lock(&mutex);
    HYP_VIDEO_MSG_LOW("closing dvr BE");

    if (TRUE == hdvr_session->display_init_done)
    {
        display_deinit(hdvr_session->display_buf_size);
        hdvr_session->display_init_done = FALSE;
    }
    else
    {
        if (-1 != hdvr_session->display_dump_fd)
        {
            rc = close(hdvr_session->display_dump_fd);
            if (0 != rc)
            {
                HYP_VIDEO_MSG_ERROR("failed to close display fd");
            }
        }
    }

    hypv_map_cleanup(&hdvr_session->habmm_if, &hdvr_session->lookup_queue);

    if ( NULL != hdvr_session->signal_handle)
    {
        MM_Signal_Release(hdvr_session->signal_handle);
        MM_SignalQ_Release(hdvr_session->signal_handle_q);
    }

    if (NULL != pMsgBufferNode)
    {
        msg2.msg_id = HYPDVR_MSGRESP_CLOSE_RET;
        msg2.data_size = pMsgBufferNode->data_size;
        msg2.data.close_data.return_status = hdvr_status;

        MM_CriticalSection_Enter(hdvr_session->send_crit_section);
        rc = hdvr_session->habmm_if.pfSend(hdvr_session->habmm_handle, &msg2,
                                           sizeof(hypdvr_msg_type), 0);
        MM_CriticalSection_Leave(hdvr_session->send_crit_section);

        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("habmm socket send failed rc=%d", rc);
        }
    }
    pthread_mutex_unlock(&mutex);
}

hdvr_status_type DvrDisplay::hdvr_handle_request(hdvr_session_t* hdvr_session)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    boolean done = FALSE;
    hypdvr_msg_type msg;
    uint32 msg_size;
    int32 rc = 0;
    uint32 retry = 0;

    while (!done)
    {
        // habmm_recv needs the msg_size for the allocated size of the msg
        msg_size =  sizeof(hypdvr_msg_type);
        rc = hdvr_session->habmm_if.pfRecv(hdvr_session->habmm_handle,
                                           (void*)&msg, &msg_size, 0, 0);
        if (hdvr_session->exit_resp_handler == TRUE)
        {
           done = TRUE;
        }
        else if (0 == rc)
        {
            retry = 0;
            switch (msg.msg_id)
            {
                case HYPDVR_MSGCMD_OPEN:
                {
                    // Version validation
                    if (HDVR_STATUS_SUCCESS != hdvr_version_check(hdvr_session, &msg))
                    {
                        done = TRUE;
                        break;
                    }
                    hdvr_be_open(hdvr_session, &msg);
                    break;
                }
                case HYPDVR_MSGCMD_IOCTL:
                {
                    hdvr_be_ioctl(hdvr_session, &msg);
                    break;
                }
                case HYPDVR_MSGCMD_CLOSE:
                {
                    hdvr_be_close(hdvr_session, &msg);
                    done = TRUE;
                    break;
                }
                default:
                {
                    HYP_VIDEO_MSG_HIGH("unrecognized msg id %d", msg.msg_id );
                    break;
                }
            }
        }
        else if ((-1 == rc) && (EAGAIN == errno) && (retry++ < MAX_HAB_RECV_RETRY))
        {
            continue;
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("hdvr receive (handle = 0x%x) returned rc = 0x%x",
                                 hdvr_session->habmm_handle, rc );
            HABMM_MEMSET((void *)&msg, 0, sizeof(hypdvr_msg_type));
            hdvr_be_close(hdvr_session, NULL);
            done = TRUE;
        }
    }
    // wait a while for the ack message to go through the hypervisor channel
    if (0 == rc)
    {
        HYP_VIDEO_MSG_LOW("500ms sleep before exit handle %d",
                           hdvr_session->habmm_handle);
        MM_Timer_Sleep(500);
    }

    return hdvr_status;
}

int DvrDisplay::thread_func(void *handle)
{
    hdvr_status_type hdvr_status = HDVR_STATUS_SUCCESS;
    habIf habmm_if;
    void* dl_handle = NULL;
    int32 rc = 0;
    DvrDisplay *pDvrDisplay = (DvrDisplay*)handle;

    HABMM_MEMSET((void *)&habmm_if, 0, sizeof(habmm_if));
    dl_handle = dlopen(HAB_LIB, RTLD_LOCAL);

    if (NULL == dl_handle)
    {
        HYP_VIDEO_MSG_ERROR("load hab lib %s failed", HAB_LIB);
        MM_Debug_Deinitialize();
        hdvr_status = HDVR_STATUS_FAIL;
    }
    else
    {
        HYP_VIDEO_MSG_HIGH("dlopen load hab lib %s successful", HAB_LIB );

        habmm_if.pfOpen = (hyp_habmm_socket_open)dlsym(dl_handle, "habmm_socket_open");
        habmm_if.pfClose = (hyp_habmm_socket_close)dlsym(dl_handle, "habmm_socket_close");
        habmm_if.pfSend = (hyp_habmm_socket_send)dlsym(dl_handle, "habmm_socket_send");
        habmm_if.pfRecv = (hyp_habmm_socket_recv)dlsym(dl_handle, "habmm_socket_recv");
        habmm_if.pfImport = (hyp_habmm_import)dlsym(dl_handle, "habmm_import");
        habmm_if.pfExport = (hyp_habmm_export)dlsym(dl_handle, "habmm_export");
        habmm_if.pfUnImport = (hyp_habmm_unimport)dlsym(dl_handle, "habmm_unimport");
        habmm_if.pfUnExport = (hyp_habmm_unexport)dlsym(dl_handle, "habmm_unexport");

        if (!habmm_if.pfOpen     ||
            !habmm_if.pfClose    ||
            !habmm_if.pfSend     ||
            !habmm_if.pfRecv     ||
            !habmm_if.pfImport   ||
            !habmm_if.pfExport   ||
            !habmm_if.pfUnImport ||
            !habmm_if.pfUnExport)
        {
            HYP_VIDEO_MSG_ERROR("dlsym hab lib failed" );
            dlclose(dl_handle);
            dl_handle = NULL;
            hdvr_status = HDVR_STATUS_FAIL;
        }
    }

    while (HDVR_STATUS_SUCCESS == hdvr_status)
    {
        int32 virtual_channel = 0;
        int32 rc = 0;

        rc = habmm_if.pfOpen(&virtual_channel, HAB_MMID_CREATE(MM_VID_2, PREVIEW_MM_SUBID), 0, 0);
        add_kpi_marker("M - DVR:Open for preview from host");

#ifdef _LINUX_
        char *env_ptr = getenv("HDVR_DEBUG_LEVEL");
        debug_level = env_ptr ? atoi(env_ptr) : 1;
#elif defined _ANDROID_
        char property_value[PROPERTY_VALUE_MAX] = {0};

        property_get("vendor.hdvr.debug.level", property_value, "1");
        debug_level = atoi(property_value);
#endif

        if (!rc)
        {
            hdvr_session_t *hdvr_session = (hdvr_session_t* )HABMM_MALLOC(sizeof(hdvr_session_t));
            if (hdvr_session == NULL)
            {
                HYP_VIDEO_MSG_ERROR("malloc hdvr session failed");
                rc = -1;
            }
            else
            {
                HABMM_MEMSET(hdvr_session, 0, sizeof(hdvr_session_t));

                HABMM_MEMCPY(&hdvr_session->habmm_if, &habmm_if, sizeof(habIf));

                hdvr_session->habmm_handle = virtual_channel;
                hdvr_session->v4l2_write_fd = -1;
                hdvr_session->v4l2_read_fd = -1;
                hdvr_session->display_dump_fd = -1;
                hdvr_session->record_fd = -1;
                if (0 != MM_CriticalSection_Create(&hdvr_session->send_crit_section))
                {
                    HYP_VIDEO_MSG_ERROR("failed to create send critical section");
                    rc = -1;
                }
                else
                {
                    pDvrDisplay->hdvr_handle_request(hdvr_session);
                }

                // wait a while for the ack message to go through the hypervisor channel
                HYP_VIDEO_MSG_LOW("500 ms sleep before exit handle %d",
                                   hdvr_session->habmm_handle);
                MM_Timer_Sleep(500);

                HYP_VIDEO_MSG_HIGH("closing habmm handle %d",
                                    hdvr_session->habmm_handle);
                hdvr_session->habmm_if.pfClose(hdvr_session->habmm_handle);
                if (NULL != hdvr_session->send_crit_section)
                {
                    MM_CriticalSection_Release(hdvr_session->send_crit_section);
                }
                free(hdvr_session);
            }
        }
        else
        {
            HYP_VIDEO_MSG_ERROR("dvr BE connection listener failed rc %d", rc);
            continue;
        }
    }

    return rc;
}

void DvrDisplay::start_preview_thread()
{
    if (NULL == mThread)
    {
        int rc = 0;

        rc = MM_Thread_Create(MM_Thread_DefaultPriority, 0, thread_func,
                              (void *)this, 0, &mThread);

        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("Failed to create preview thread");
        }
    }
}

void DvrDisplay::stop_preview_thread()
{
    int rc = 0;

    if (NULL != mThread)
    {
        rc = MM_Thread_Join(mThread, &rc);
        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("Failed to join the preview thread");
        }
        rc = MM_Thread_Release(mThread);
        if (0 != rc)
        {
            HYP_VIDEO_MSG_ERROR("Failed to release the preview thread");
        }
        mThread = NULL;
    }
}

DvrDisplay::DvrDisplay()
{
    MM_Debug_Initialize();
    mDisplayInfo.width = 1920;
    mDisplayInfo.height = 1024;
    mDisplayInfo.format = HDVR_PIXEL_FORMAT_TYPE_NV12;
    mDisplayInfo.buf_size = calc_buffer_size(HDVR_PIXEL_FORMAT_TYPE_NV12, 1920, 1024);
    mDisplayReady = FALSE;
    mThread = NULL;
    start_preview_thread();
}

DvrDisplay::~DvrDisplay()
{
    stop_preview_thread();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace dvr
}  // namespace qti
}  // namespace vendor


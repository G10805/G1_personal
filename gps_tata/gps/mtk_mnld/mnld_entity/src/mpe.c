// SPDX-License-Identifier: MediaTekProprietary
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "mtk_lbs_utility.h"
#include "mnld.h"
#include "mtk_gps.h"
#include "mpe.h"
#include "gps_dbg_log.h"
#include "mpe_common.h"
#include "data_coder.h"
#include "gps_controller.h"
#include "mtk_auto_log.h"
#include "nmea_parser.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MPE"
#endif

MPECallBack gMpeCallBackFunc = NULL;
static int g_fd_mpe = -1;
static int g_fd_mpe2mnl = -1;
extern unsigned char gMpeThreadExist;

int mnl2adr_gps_open(void) {
    LOGD("mnl2adr_gps_open");
    char buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_MNL2ADR_GPS_OPEN);
    put_int(buff, &offset, 0);//No payload, length is 0

    return safe_sendto(MNLD_MNL2ADR_SOCKET, buff, offset);
}

int mnl2adr_gps_close(void) {
    LOGD("mnl2adr_gps_close");
    char buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE);
    put_int(buff, &offset, 0);//No payload, length is 0

    return safe_sendto(MNLD_MNL2ADR_SOCKET, buff, offset);
}

int mnl2adr_mnl_reboot(void) {
    LOGD("mnl2adr_mnl_reboot");
    char buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_MNL2ADR_MNL_REBOOT);
    put_int(buff, &offset, 0);//No payload, length is 0

    return safe_sendto(MNLD_MNL2ADR_SOCKET, buff, offset);
}

int mnl2adr_open_gps_done(void) {
    LOGD("mnl2adr_open_gps_done");
    char buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_MNL2ADR_GPS_OPEN_DONE);
    put_int(buff, &offset, 0);//No payload, length is 0

    return safe_sendto(MNLD_MNL2ADR_SOCKET, buff, offset);
}

int mnl2adr_close_gps_done(void) {
    LOGD("mnl2adr_close_gps_done");
    char buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(buff, &offset, MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE_DONE);
    put_int(buff, &offset, 0);//No payload, length is 0

    return safe_sendto(MNLD_MNL2ADR_SOCKET, buff, offset);
}

char nmea_sbuff[MNL_MPE_NMEA_MAX_SIZE] = {0};

int mnl2adr_send_nmea_data(const char * nmea_buffer, const UINT32 length) {
    //LOGD("len:%d, %s", length, nmea_buffer);
    int offset = 0;
    if(length < (MNL_MPE_NMEA_MAX_SIZE - 2)) {
        memset(nmea_sbuff, 0, MNL_MPE_NMEA_MAX_SIZE);
        put_int(nmea_sbuff, &offset, MNL_ADR_TYPE_MNL2ADR_NMEA_DATA);
        put_int(nmea_sbuff, &offset, length);
        put_binary(nmea_sbuff, &offset, nmea_buffer, length);

        return safe_sendto(MNLD_MNL2ADR_SOCKET, nmea_sbuff, offset);
    } else {
        LOGE("over length(%d):%d", MNL_MPE_NMEA_MAX_SIZE, length);
        return -1;
    }
}

int mnl2mpe_set_log_path(char* path, int status_flag, int mode_flag) {
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(mnl2mpe_buff, &offset, CMD_SEND_FROM_MNLD);
    put_int(mnl2mpe_buff, &offset, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN + 2*sizeof(INT32));
    put_int(mnl2mpe_buff, &offset, status_flag);
    put_int(mnl2mpe_buff, &offset, mode_flag);
    put_binary(mnl2mpe_buff, &offset, (const char*)path, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);

    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
        LOGD("MPE not enable\n");
        return MTK_GPS_ERROR;
    }
    if(safe_sendto(MNLD_MPE_SOCKET, mnl2mpe_buff, MNL_MPE_MAX_BUFF_SIZE) == -1) {
        LOGE("safe_sendto fail:[%s]%d", strerror(errno), errno);
        return MTK_GPS_ERROR;
    }
    return 0;
}

int mpe2mnl_hdlr(char *buff) {
    int type, length;
    int offset = 0;

    type = get_int(buff, &offset, MNL_MPE_MAX_BUFF_SIZE);
    length = get_int(buff, &offset, MNL_MPE_MAX_BUFF_SIZE);

    LOGD("type=%d length=%d\n", type, length);

    switch (type) {
        case CMD_MPED_REBOOT_DONE: {
            mtklogger_mped_reboot_message_update();
            break;
        }
        case CMD_START_MPE_RES:
        case CMD_STOP_MPE_RES:
        case CMD_SEND_SENSOR_RAW_RES:
        case CMD_SEND_SENSOR_CALIBRATION_RES:
        case CMD_SEND_SENSOR_FUSION_RES:
        case CMD_SEND_GPS_AIDING_RES:
        case CMD_SEND_ADR_STATUS_RES:
        case CMD_SEND_GPS_TIME_REQ: {
            if (mnld_is_gps_started_done()) {
                mtk_gps_mnl_get_sensor_info((UINT8 *)buff, length + sizeof(MPE_MSG));
            }
            break;
        }

        case MNL_ADR_TYPE_ADR2MNL_NMEA_DATA: {
            char nmea[MNL_MPE_NMEA_MAX_SIZE] = {0};
            get_binary(buff, &offset, nmea, MNL_MPE_NMEA_MAX_SIZE, MNL_MPE_NMEA_MAX_SIZE);

            mtk_mnl_nmea_parser_process(nmea, length);
            break;
        }
        case MNL_ADR_TYPE_ADR2MNL_PMTK_CMD: {
            char pmtk[MNL_MPE_NMEA_MAX_SIZE];
            get_binary(buff, &offset, pmtk, MNL_MPE_NMEA_MAX_SIZE, sizeof(pmtk));
            gps_controller_rcv_pmtk(pmtk);
            break;
        }
        default: {
           LOGE("unknown cmd=%d\n", type);
           break;
       }
    }
    return 0;
}

int mtk_gps_mnl_trigger_mpe(void) {
    int ret = MTK_GPS_ERROR;
    #ifdef MTK_ADR_SUPPORT
    UINT16 mpe_len;
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};

    mpe_len = mtk_gps_set_mpe_info((UINT8 *)mnl2mpe_buff);
    //LOGD("mpemsg len=%d\n", mpe_len);

    if (mpe_len > 0) {
        if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
            LOGE("MPE not enable\n");
            return MTK_GPS_ERROR;
        }
        if(safe_sendto(MNLD_MPE_SOCKET, mnl2mpe_buff, MNL_MPE_MAX_BUFF_SIZE) == -1){
            LOGE("safe_sendto fail:[%s]%d", strerror(errno), errno);
        }else{
            ret = MTK_GPS_SUCCESS;
        }
    }
    #endif
    return ret;
}

/*static void mpe_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mpe_thread_timeout() dump and exit.");
        mnld_block_exit();
    } else {
        LOGE("mpe_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}*/

int mnl_mpe_thread_init() {
    int ret;
    LOGD("mpe init");

    gMpeCallBackFunc = mtk_gps_mnl_trigger_mpe;
    ret = mtk_gps_mnl_mpe_callback_reg((MPECallBack *)gMpeCallBackFunc);
    LOGD("register mpe cb %d,gMpeCallBackFunc= %p,mtk_gps_mnl_trigger_mpe=%p\n",
        ret, gMpeCallBackFunc, mtk_gps_mnl_trigger_mpe);
    return 0;
}

static void* mpe_main_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    struct epoll_event events[MAX_EPOLL_EVENT];
    memset(&events, 0, sizeof(events));
    UNUSED(arg);

    mnl2mpe_hdlr_init();
    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("epoll_create failure reason=[%s]%d\n",
            strerror(errno), errno);
        return 0;
    }
    if (g_fd_mpe > 0) {
        if (epoll_add_fd(epfd, g_fd_mpe) == -1) {
            LOGE("epoll_add_fd() failed for g_fd_epo failed");
            return 0;
        }
    } else {
        LOGW("g_fd_mpe invalid");
    }

    if (g_fd_mpe2mnl > 0) {
        if (epoll_add_fd(epfd, g_fd_mpe2mnl) == -1) {
            LOGE("epoll_add_fd() failed for g_fd_epo failed");
            return 0;
        }
    } else {
        LOGW("g_fd_mpe2mnl invalid");
    }

    while (1) {
        int i;
        int n;
        LOGD("wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_mpe) {
                if (events[i].events & EPOLLIN) {
                    mnl2mpe_hdlr(g_fd_mpe);
                }
            } else if (events[i].data.fd == g_fd_mpe2mnl) {
                if (events[i].events & EPOLLIN) {
                    char mpe2mnl_buff[MNL_MPE_NMEA_MAX_SIZE] = {0};
                    int read_len = 0;
                    read_len = safe_recvfrom(g_fd_mpe2mnl, mpe2mnl_buff, sizeof(mpe2mnl_buff));
                    if (read_len <= 0) {
                        LOGE("safe_recvfrom() failed read_len=%d", read_len);
                    } else {
                        mpe2mnl_hdlr(mpe2mnl_buff);
                    }
                }
            } else {
                LOGE("unknown fd=%d", events[i].data.fd);
            }
        }
    }

    LOGE("exit");
    pthread_exit(NULL);
    return 0;
}

int mpe_function_init(int self_recv) {
    pthread_t main_thread_handle;

    mpe_sys_read_mpe_conf_flag();
    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
        LOGD("MPE not enable\n");
        return MTK_GPS_SUCCESS;
    }

    if (self_recv) {
        g_fd_mpe = socket_bind_udp(MNLD_MPE_SOCKET);
    } else {
        g_fd_mpe = -1;
    }
    g_fd_mpe2mnl = socket_bind_udp(MNLD_ADR2MNL_SOCKET);

    if(pthread_create(&main_thread_handle, NULL, mpe_main_thread, NULL)) {
        LOGE("MPE main thread init failed");
    }

    return MTK_GPS_SUCCESS;
}

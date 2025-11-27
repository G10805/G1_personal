/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#ifndef MODULE_DAEMON_H
#define MODULE_DAEMON_H

#include <sys/un.h>  /*struct sockaddr_un*/
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "module.h"

/*name: algo_mnld_param
  *role: algo_mnld_param is describe mnld & adr data interact
  *member:
  *    @adr2mnl_socket_fd: send data socket(UDP)
  *    @mnl2adr_socket_fd: receive data socket(UDP)
  *    @event_list: cache event from mnld(MPE_MSG_RING)
  *    @event_mutex: provide protect for event_list
  *    @priv: cache daemon_device
  */
typedef struct adr_mnld_param {
    int adr2mnl_socket_fd;
    int mnl2adr_socket_fd;
    struct sockaddr_un sock_adr2mnl_addr;
    struct sockaddr_un sock_mnl2adr_addr;

    struct module_list event_list;
    pthread_mutex_t event_mutex;
    pthread_cond_t event_cond;

    pthread_t mnld2adr_recv_thread;
    int32_t mnld2adr_recv_thread_exit;

    pthread_t mnld2adr_hdlr_thread;
    int32_t mnld2adr_hdlr_thread_exit;

    void *priv;
} adr_mnld_param;

typedef struct {
    char *log_path;
    int open_sanity_test;
    int open_data_store;
    int open_algo_mnld;
    int open_fake_odom;
}DAEMON_CONFIG_PARAM;

/*name: daemon_device
  *role: daemon_device is a special module, control all other moudles, such as GNSS, ALGO...
  *member:
  *    @flag_gps_open: flag for stop/deinit XXX_driver, in a sitiuation, program receive
            MNL_ADR_TYPE_MNL2ADR_GPS_OPEN from mnld then init/start XX_driver,
            but do not receive MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE from mnld,
            main daemon catch some signal,
            need go to exit, we should stop/deinit XXX_driver based on the flag
  */
typedef struct daemon_device {
    char *name;
    struct module *mod;
    void (*upload)(module *, module_box *);

    pthread_cond_t cond;
    pthread_mutex_t mutex;

    module *gnss;
    module *sensor;
    module *odom;
    module *algo;
    module *daemon;
    module *debug;
    module *sanity;
    module *store;

    modules_Inf *modInf;
    struct module_list *client_list;
    struct adr_mnld_param *param;

    int flag_gps_open;
    pthread_mutex_t flag_gps_open_mutex;

    int running;

    DAEMON_CONFIG_PARAM daemon_config;
} daemon_device;

void daemon_parse_commandline(daemon_device *dev, int argc, char *argv[]);
void daemon_driver_init(daemon_device *dev);
void daemon_driver_start(daemon_device *dev);
void daemon_driver_stop(daemon_device *dev);
void daemon_driver_deinit(daemon_device *dev);
void daemon_sighlr(int signo);

#endif

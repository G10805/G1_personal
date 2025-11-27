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

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include "module.h"
#include "adr_util.h"

#define ADR_DEBUG_SOCKET_PORT 7004
#define DEBUG_MSG_LEN 4

typedef struct debug_device {
    struct module *mod;
    void (*upload)(module *, module_box *);

    int pgps_sockfd;    //socket connect from powerGPS

    int server_sockfd;
    pthread_t pgps_thread;
    int32_t pgpsthread_exit;

    module_box *box[DEBUG_MSG_LEN];

} debug_device;

static int send_buf_to_power_gps(debug_device *dev, char *buf, int len)
{
    int ret = 0;
    int cur_pos = 0;

    while(cur_pos < len && dev->pgps_sockfd > 0) {
        ret = send(dev->pgps_sockfd, &buf[cur_pos], len - cur_pos, 0);
        if (ret == len - cur_pos)
            break;

        if (ret <= 0) {
            LOG_ERROR("SOCKET ERROR errno(%s):%d", strerror(errno), errno);
            if (errno == EAGAIN || errno == EINTR)
            {
                LOG_ERROR("send to power GPS buffer full, wait(10ms)");
                usleep(10000);
                continue;
            }
            if (errno == ECONNRESET || errno == EPIPE)
            {
                //dev->pgps_sockfd = -1;
                LOG_ERROR("send_buf_to_power_gps buffer client connect is reset");
            }

            break;
        } else {
            cur_pos += ret;
        }
    }

    return ret;
}

//cmd maybe need  send to gps or algo or both
static void command_broadcast(debug_device *dev, char *buf, int len)
{
    if (!memcmp(buf, "$PADR", 5)) {
        //send to gnss & algo
        module_box *padr_box = get_available_module_box(dev->mod);
        padr_box->data = buf;
        padr_box->count = len;
        padr_box->type = CMD_ADR_PADR;
        dev->upload(dev->mod, padr_box);
        release_module_box(padr_box);
    } else if (!memcmp(buf, "$PMTK", 5)) {
        //send to gnss
        module_box *pmtk_box = get_available_module_box(dev->mod);
        pmtk_box->data = buf;
        pmtk_box->count = len;
        pmtk_box->type = CMD_ADR_PMTK;
        dev->upload(dev->mod, pmtk_box);
        release_module_box(pmtk_box);
    } else {
        LOG_ERROR("cmd(%s) type try to broadcast seems not defined", buf);
        abort();
    }
}

static void *adr_powergps_thread(void *data)
{
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    static char socket_rcv_buf[256];
    debug_device *dev = (debug_device *)data;
    memset(&serveraddr, 0, sizeof(serveraddr));

    dev->server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (dev->server_sockfd == -1)
    {
        LOG_ERROR("socket create fail:%d %s", errno, strerror(errno));
        return NULL;
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(ADR_DEBUG_SOCKET_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int on = 1;
    if((setsockopt(dev->server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0){
        LOG_ERROR("setsockopt fail:%d %s", errno, strerror(errno));
        close(dev->server_sockfd);
        dev->server_sockfd = 0;
        return NULL;
    }
    if (bind(dev->server_sockfd ,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        LOG_ERROR("bind fail:%d %s", errno, strerror(errno));
        close(dev->server_sockfd);
        dev->server_sockfd = 0;
        return NULL;
    }

    if (listen(dev->server_sockfd , 5) == -1) {
        LOG_ERROR("listen fail:%d %s", errno, strerror(errno));
        close(dev->server_sockfd);
        dev->server_sockfd = 0;
        return NULL;
    }

    socklen_t length = sizeof(clientaddr);
    dev->pgps_sockfd = accept(dev->server_sockfd,(struct sockaddr *)&clientaddr, &length);
    if (dev->pgps_sockfd <= 0) {
        LOG_ERROR("accept error:%d %s", errno, strerror(errno));
        return NULL;
    }
    LOG_INFO("get connect from powerGPS & socket:%d", dev->pgps_sockfd);

    while (!dev->pgpsthread_exit)
    {
        memset(socket_rcv_buf, 0, sizeof(socket_rcv_buf));
        LOG_DEBUG("listen the client socket is %d", dev->pgps_sockfd);
        int len = recv(dev->pgps_sockfd, socket_rcv_buf, sizeof(socket_rcv_buf), 0);

        LOG_INFO("recv_buf %s %d", socket_rcv_buf, len);

        if (len > 0) {
             //parser & send data to adr algo & mnld
            command_broadcast(dev, socket_rcv_buf, len);
        } else {    //powerGPS close
            LOG_ASSERT("powerGPS close errno(%s):%d", strerror(errno), errno);
            if (dev->pgps_sockfd) {
                close(dev->pgps_sockfd);
                dev->pgps_sockfd = -1;
                LOG_INFO("closed recv socket fd ");
            }
            dev->pgps_sockfd = accept(dev->server_sockfd,(struct sockaddr *)&clientaddr, &length);
            LOG_INFO("get connect from powerGPS & socket:%d", dev->pgps_sockfd);
            if (dev->pgps_sockfd <= 0) {
                LOG_ERROR("accept error:%d %s", errno, strerror(errno));
                break;
            } else {
                LOG_ASSERT("accept powerGPS ok, pgps_sockfd:%d", dev->pgps_sockfd);
            }
        }
    }
    dev->pgpsthread_exit = 1;
    pthread_exit(0);
}

static int32_t debug_driver_init(struct module *mod)
{
    int32_t ret = -1;
    struct debug_device *dev =
        (debug_device*)module_get_driver_data(mod);

    ret = pthread_create(&dev->pgps_thread, NULL, adr_powergps_thread, (void *)dev);
    if (ret) {
        LOG_ERROR("fail to create powerGPS thread");
        abort();
    }
    return ret;
}

static void debug_driver_download(struct module *mod, module_box *box)
{
    struct debug_device *dev;
    dev = (debug_device*)module_get_driver_data(mod);

    switch(box->type) {
        case CMD_GNSS_ACK:
        case CMD_ALGO_ACK:
        case DATA_ADR_ALGO:
        case DATA_GNSS_NMEA:
            LOG_DEBUG("receive data:%.*s", box->count, (char *)(box->data));
            send_buf_to_power_gps(dev, (char *)(box->data), box->count);
            break;
    }

}

static void debug_driver_deinit(struct module *mod)
{
    struct debug_device *dev;
    dev = (debug_device*)module_get_driver_data(mod);

    if (!dev->pgpsthread_exit) {
        if (dev->pgps_sockfd) {
            shutdown(dev->pgps_sockfd, SHUT_RDWR);
            close(dev->pgps_sockfd);
            LOG_INFO("closed recv socket fd ");
        }
        if (dev->server_sockfd) {
            shutdown(dev->server_sockfd, SHUT_RDWR);
            close(dev->server_sockfd);
            LOG_INFO("closed server socket fd ");
        }
        dev->pgpsthread_exit = 1;
        pthread_join(dev->pgps_thread, NULL);
        LOG_INFO("powerGPS thread exit~");
    }

    module *debug_mod = dev->mod;
    memset(dev, 0, sizeof(debug_device));
    dev->mod = debug_mod;

}
 
static void debug_driver_registry(struct module *mod, process_data callback)
{
    struct debug_device *dev;
    dev = (debug_device*)module_get_driver_data(mod);

    dev->upload = callback;
}
 
static module_driver debug_driver = {
    .name = "mtk_debug",        
    .capability = DATA_ADR_ALGO | DATA_GNSS_NMEA | CMD_ALGO_ACK | CMD_GNSS_ACK,
    .init = debug_driver_init,
    .download = debug_driver_download,
    .deinit = debug_driver_deinit,
    .registry = debug_driver_registry,
};

void module_debug_deinit(module *mod)
{
    struct debug_device *dev;
    dev = (debug_device*)module_get_driver_data(mod);
    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_debug_init(module *mod)
{
    struct debug_device *dev;

    dev = (debug_device*)zalloc(sizeof(debug_device));
    if (!dev) {
        LOG_ERROR("Fail to create debug_device");
        return -1;
    }
    //set_debug_ctx(dev);
    dev->mod = mod;
    module_set_name(mod, debug_driver.name);
    module_register_driver(mod, &debug_driver, (void *)dev);
    return 0;
}


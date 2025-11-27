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
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>

/*include header files from algo*/
#include "adr_release_type.h"
#include "adr_release_api.h"

/*include header files from mnl*/
#include "adr.h"
#include "mtk_gps_type.h"

/*include FW header filed*/
#include "zalloc.h"
#include "log.h"
#include "module_daemon.h"
#include "adr_util.h"
#include "data_coder.h"
#include "config-parser.h"

#define ADR_DEBUG 0
#define OPEN_FAKE_ODOM_DEGAULT 0

char *log_path;
int  log_dbg_level = (L_INFO);
int  log_algo_level = (L_DEBUG);
int  log_vehicle_level = (L_INFO);

extern struct adr_config *config;

int module_daemon_init(module *mod);
void module_daemon_deinit(module *mod);

extern struct daemon_device *g_daemon_ctx;

typedef struct MPE_MSG_EVENT {
    struct module_list link;
    MPE_MSG *msg;
} MPE_MSG_EVENT;

int adr_mnld_socket_setup(adr_mnld_param *param)
{
    if((param->mnl2adr_socket_fd = socket(PF_LOCAL, SOCK_DGRAM, 0)) == -1) {
            LOG_ERROR("open mnl2adr udp socket failed, errno[%d]:%s",
                    errno, strerror(errno));
        return -1;
    }
    //if when block mode, receive thread can't exit
    /*if(set_fcntl(param->mnl2adr_socket_fd, O_NONBLOCK) < 0) {
        LOG_ERROR("set mnl2adr_socket_fd to O_NONBLOCK mode failed");
        return -1;
    }*/
    memset(&param->sock_mnl2adr_addr, 0, sizeof(param->sock_mnl2adr_addr));
    param->sock_mnl2adr_addr.sun_path[0] = 0;
    memcpy(param->sock_mnl2adr_addr.sun_path + 1, MNLD_MNL2ADR_SOCKET,
            strlen(MNLD_MNL2ADR_SOCKET));
    param->sock_mnl2adr_addr.sun_family = AF_UNIX;
    unlink(MNLD_MNL2ADR_SOCKET);

    /*!<when adr as receiver, its role is server>*/
    if(bind(param->mnl2adr_socket_fd,(const struct sockaddr *)&param->sock_mnl2adr_addr,
                sizeof(param->sock_mnl2adr_addr)) < 0) {
        LOG_ERROR("bind failed, path[%s] reason[%s] errno[%d]",
                param->sock_mnl2adr_addr.sun_path + 1, strerror(errno), errno);
        close(param->mnl2adr_socket_fd);
        return -1;
    }
    /*!<when launch receive socket, then setup send socket>*/
    if((param->adr2mnl_socket_fd = socket(PF_LOCAL, SOCK_DGRAM, 0)) < 0) {
        LOG_ERROR("socket() failed reason=[%s]%d",
                    strerror(errno), errno);
        shutdown(param->mnl2adr_socket_fd, SHUT_RDWR);
        close(param->mnl2adr_socket_fd);
        return -1;
     }
    /*if when block mode, adr_algo_data_ready_thread can't exit.
            it will sendback nmea to mnld through daemon*/
    if(set_fcntl(param->adr2mnl_socket_fd, O_NONBLOCK) < 0) {
        LOG_ERROR("set adr2mnl_socket_fd to O_NONBLOCK mode failed");
        return -1;
    }

     memset(&param->sock_adr2mnl_addr, 0, sizeof(param->sock_adr2mnl_addr));
     param->sock_adr2mnl_addr.sun_path[0] = 0;
     memcpy(param->sock_adr2mnl_addr.sun_path + 1, MNLD_ADR2MNL_SOCKET,
                strlen(MNLD_ADR2MNL_SOCKET));
     param->sock_adr2mnl_addr.sun_family = AF_UNIX;

    return 0;
}

int algo_mnld_handle_mes(daemon_device *dev, UINT32 type)
{
    if(dev->daemon_config.open_algo_mnld){
        char mpe2mnl_buff[MNL_ADR_MAX_BUFF_SIZE] = {0};
        int mpe2mnl_offset = 0;
        int fd = dev->param->adr2mnl_socket_fd;
        int ret, size_adr2mnl = sizeof(adr2mnl_msg_struct);
        struct sockaddr_un addr = dev->param->sock_adr2mnl_addr;
        adr2mnl_msg_struct *adr2mnl_ptr;

        switch(type) {
            case CMD_START_MPE_REQ:
                put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_START_MPE_RES);
                put_int(mpe2mnl_buff, &mpe2mnl_offset, 0);
                if(safe_sendto(fd, &addr, (char *)&mpe2mnl_buff, mpe2mnl_offset) < 0)
                    LOG_ERROR("send CMD_START_MPE_REQ failed");
                break;
            case CMD_SET_GPS_AIDING_REQ:
                put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_SEND_GPS_AIDING_RES);
                put_int(mpe2mnl_buff, &mpe2mnl_offset, 0);
                if(safe_sendto(fd, &addr, (char *)&mpe2mnl_buff, mpe2mnl_offset) < 0)
                    LOG_ERROR("send CMD_SET_GPS_AIDING_REQ failed");
                break;
            case CMD_GET_ADR_STATUS_REQ:
                adr2mnl_ptr = (adr2mnl_msg_struct *)zalloc(size_adr2mnl);
                //get adr status
                ret = adr_get_adr2mnl_msg(adr2mnl_ptr);
                if(ret == -1){
                    LOG_ERROR("get adr2mnl message failed, don't send to mnld.");
                } else{
                    put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_SEND_ADR_STATUS_RES);
                    put_binary(mpe2mnl_buff, &mpe2mnl_offset, (const char*)adr2mnl_ptr, size_adr2mnl);
                    if (safe_sendto(fd, &addr, (char *)&mpe2mnl_buff, mpe2mnl_offset) < 0)
                        LOG_ERROR("send CMD_SEND_GPS_AIDING_RES failed");
                }
                if(NULL != adr2mnl_ptr){
                    free(adr2mnl_ptr);
                    adr2mnl_ptr = NULL;
                }
                break;
            default:
                LOG_ERROR("algo can not handle the cmd from mnld, type:%u.", type);
                return -1;
        }
    }else{
        //LOG_INFO("not open algo mnld path");
    }
    return 0;
}

void daemon_gps_open(daemon_device *dev)
{
    dev->modInf->ops_forall("init", dev->modInf->private);
    dev->modInf->ops_forall("registry", dev->modInf->private);
    dev->modInf->ops_forall("start", dev->modInf->private);
    dev->flag_gps_open = 1;
}

void daemon_gps_open_done(daemon_device *dev)
{
    dev->modInf->ops_forall("start_done", dev->modInf->private);
}

void daemon_gps_close(daemon_device *dev)
{
    dev->modInf->ops_forall("stop", dev->modInf->private);
    dev->modInf->ops_forall("deinit", dev->modInf->private);
    dev->flag_gps_open = 0;
}

void mnld2adr_event_work(adr_mnld_param *param, MPE_MSG *msg)
{
    int mnl2adr_offset = 0;
    daemon_device *dev = (daemon_device *)param->priv;
    UINT32 type = get_int((char *)msg, &mnl2adr_offset, MNL_ADR_MAX_BUFF_SIZE);
    UINT32 length = get_int((char *)msg, &mnl2adr_offset, MNL_ADR_MAX_BUFF_SIZE);

    LOG_DEBUG("receive cmd from mnld, type:%u, length:%d.", type, length);

    pthread_mutex_lock(&dev->flag_gps_open_mutex);
    if(type != MNL_ADR_TYPE_MNL2ADR_GPS_OPEN && dev->flag_gps_open) {
        switch(type) {
            case CMD_START_MPE_REQ:
            case CMD_SET_GPS_AIDING_REQ:
            case CMD_GET_ADR_STATUS_REQ:
                algo_mnld_handle_mes(dev, type);
                break;
            #if ADR_DEBUG
            #else
            case MNL_ADR_TYPE_MNL2ADR_GPS_OPEN_DONE:
                LOG_ERROR("[ADR_OPEN_DONE] receive open done cmd from mnld, type:%u.", type);
                daemon_gps_open_done(dev);
                LOG_ERROR("[ADR_OPEN_DONE] send pmtk876 ok done.");
                break;
            case MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE:
                LOG_ERROR("[ADR_CLOSE] receive close cmd from mnld, type:%u.", type);
                daemon_gps_close(dev);
                LOG_ERROR("[ADR_CLOSE] module finish stop/deinit!!!");
                break;
             case MNL_ADR_TYPE_MNL2ADR_MNL_REBOOT:
                LOG_ERROR("[ADR_REBOOT] receive reboot cmd from mnld, type:%u.", type);
                daemon_gps_close(dev);
                LOG_ERROR("[ADR_REBOOT] module finished stop/deinit!!!");
                 //daemon_gps_open(dev);
                 //LOG_INFO("[ADR_REBOOT] module finished init/start!!!");
                 break;
            #endif
            case MNL_ADR_TYPE_MNL2ADR_NMEA_DATA:
            {
                module_box *box = get_available_module_box(dev->mod);
                char nmea[MNL_ADR_NMEA_MAX_SIZE] = {0};
                get_binary((char *)msg, &mnl2adr_offset, nmea, MNL_ADR_NMEA_MAX_SIZE, MNL_ADR_NMEA_MAX_SIZE);
                LOG_DEBUG("nmea:%s", nmea);

                if(box){
                    box->data = (void *)nmea;//nmea or raw data
                    box->count = length;
                    box->type = DATA_DAEMON_NMEA;
                    dev->upload(dev->mod, box);
                    release_module_box(box);
                }
                break;
            }
            default:
            {
                LOG_INFO("not care the cmd from mnld %d, type:%u.", dev->flag_gps_open, type);
                break;
            }
        }
    }else if(type == MNL_ADR_TYPE_MNL2ADR_GPS_OPEN && !dev->flag_gps_open){
        /* if module driver init failed, remove the module from Framework*/
        LOG_INFO("[ADR_OPEN] receive open cmd from mnld, type:%u.", type);
        daemon_gps_open(dev);
        LOG_INFO("[ADR_OPEN] module finish init/start!!!");
    }else if(type != MNL_ADR_TYPE_MNL2ADR_GPS_OPEN && !dev->flag_gps_open){
        switch(type) {
            case MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE_DONE:
                LOG_INFO("not care the cmd from mnld %d, type:%u.", dev->flag_gps_open, type);
                break;
            default:
                LOG_ERROR("adr has not been inited %d, can not handle comand type:%u", dev->flag_gps_open, type);
        }
    }else{
        LOG_ERROR("adr has been inited %d, receive open command again:%u", dev->flag_gps_open, type);
    }
    pthread_mutex_unlock(&dev->flag_gps_open_mutex);
}

void free_event(MPE_MSG_EVENT *event){
    if(event){
        if(event->msg){
            free(event->msg);
            event->msg = NULL;
        }
        free(event);
        event = NULL;
    }
}

void *mnld2adr_event_handler_thread(void *data)
{
    adr_mnld_param *param = (adr_mnld_param *)data;
    MPE_MSG_EVENT *event;

    while (!param->mnld2adr_hdlr_thread_exit) {
        pthread_mutex_lock(&param->event_mutex);
        pthread_cond_wait(&param->event_cond, &param->event_mutex);
        pthread_mutex_unlock(&param->event_mutex);
        while(1) {
            pthread_mutex_lock(&param->event_mutex);
            /*!<find the last element>*/
            event = module_container_of(param->event_list.prev, event, link);
            pthread_mutex_unlock(&param->event_mutex);
            /*!<if the element is the header>*/
            if (&event->link == &param->event_list)
                break;

            mnld2adr_event_work(param, event->msg);
            pthread_mutex_lock(&param->event_mutex);
            module_list_remove(&event->link);
            pthread_mutex_unlock(&param->event_mutex);
            free_event(event);
        }
    }
    pthread_exit(0);
}

void *mnld2adr_recv_thread(void *data)
{
    adr_mnld_param *param = (adr_mnld_param *)data;
    char mnl2adr_buff[MNL_ADR_MAX_BUFF_SIZE] = {0};
    MPE_MSG_EVENT *event;
    int recvlen, n, i;
    struct epoll_event events[MAX_EPOLL_EVENT];
    memset(&events, 0, sizeof(events));
    int epfd = epoll_create(MAX_EPOLL_EVENT);

    if (epfd == -1) {
        LOG_ERROR("epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return NULL;
    }
    if(epoll_add_fd(epfd, param->mnl2adr_socket_fd) == -1) {
        LOG_ERROR("epoll_add_fd() for mnl2adr_socket_fd failed");
        close(epfd);
        return NULL;
    }

    while (!param->mnld2adr_recv_thread_exit) {
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT, EPOLL_TIMEOUT);
        LOG_DEBUG("mnld2adr_recv_thread epoll wait return ");

        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOG_ERROR("epoll_wait failure reason=[%s]%d",strerror(errno), errno);
                param->mnld2adr_recv_thread_exit = 1;
                break;
            }
        }
        for(i = 0; i < n; i++){
            if (events[i].data.fd == param->mnl2adr_socket_fd) {
                if (events[i].events & EPOLLIN) {
                    memset(mnl2adr_buff, 0, MNL_ADR_MAX_BUFF_SIZE);
                    recvlen = safe_recvfrom(param->mnl2adr_socket_fd, mnl2adr_buff,
                                sizeof(mnl2adr_buff));
                    if (recvlen < 0) {
                        LOG_ERROR("receive from mnld failed recvlen=%d", recvlen);
                        param->mnld2adr_recv_thread_exit = 1;
                        break;
                    }
                    if( recvlen >= (int)sizeof(MPE_MSG)) {//normal message
                        LOG_DEBUG("receive message len is %d", recvlen);
                        event = (MPE_MSG_EVENT *)zalloc(sizeof(MPE_MSG_EVENT));
                        if(event != NULL) {
                            event->msg = (MPE_MSG *)zalloc(recvlen);
                            memcpy(event->msg, &mnl2adr_buff[0], recvlen);
                            module_list_init(&event->link);
                            pthread_mutex_lock(&param->event_mutex);
                            module_list_insert(&param->event_list, &event->link);
                            pthread_mutex_unlock(&param->event_mutex);
                            pthread_cond_broadcast(&param->event_cond);
                        }
                    }
                }else {
                    LOG_ERROR("events[%d].events %d", i, events[i].events);
                }
            }else {
                LOG_ERROR("unknown fd=%d, mnl2adr_socket_fd=%d",
                        events[i].data.fd, param->mnl2adr_socket_fd);
            }
        }
    }

    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }

    LOG_INFO("mnld2adr_recv_thread exit~");
    pthread_exit(0);
}

int adr_mnld_deinit(adr_mnld_param *param)
{
    if (param){
        daemon_device *dev = param->priv;

        param->mnld2adr_recv_thread_exit = 1;
        //pthread_join(param->mnld2adr_recv_thread, NULL);
        LOG_ERROR("stop mnld2adr_recv_thread ok!");
        param->mnld2adr_hdlr_thread_exit = 1;
        pthread_cond_broadcast(&param->event_cond);
        //pthread_join(param->mnld2adr_hdlr_thread, NULL);
        LOG_ERROR("stop mnld2adr_hdlr_thread ok!");

        if (param->mnl2adr_socket_fd) {
            LOG_DEBUG("close mnl2algo_socket_fd %d ~",param->mnl2adr_socket_fd );
            shutdown(param->mnl2adr_socket_fd, SHUT_RDWR);
            close(param->mnl2adr_socket_fd);
        }
        if (param->adr2mnl_socket_fd) {
            LOG_DEBUG("close adr2mnl_socket_fd %d ~",param->adr2mnl_socket_fd );
            shutdown(param->adr2mnl_socket_fd, SHUT_RDWR);
            close(param->adr2mnl_socket_fd);
        }
        pthread_cond_destroy(&param->event_cond);
        pthread_mutex_destroy(&param->event_mutex);

        pthread_mutex_lock(&dev->flag_gps_open_mutex);
        if(dev->flag_gps_open){
            dev->modInf->ops_forall("stop", dev->modInf->private);
            dev->modInf->ops_forall("deinit", dev->modInf->private);
            dev->flag_gps_open = 0;
            LOG_DEBUG("module finish stop/deinit!!!");
        }
        pthread_mutex_unlock(&dev->flag_gps_open_mutex);
        free(param);
        param = NULL;
    }
    return 0;
}

adr_mnld_param * adr_mnld_init(void *priv)
{
    adr_mnld_param *param;
    int32_t ret = 0;
    param = (adr_mnld_param*)zalloc(sizeof(adr_mnld_param));
    if (!param) {
        LOG_ERROR("Fail to create adr_mnld_param");
        return NULL;
    }

    param->priv = priv;

    if (adr_mnld_socket_setup(param) < 0) {
        LOG_ERROR("setup socket adr <--> mnld fail");
        if (param){
            if (param->mnl2adr_socket_fd) {
                LOG_DEBUG("close mnl2algo_socket_fd %d ~",param->mnl2adr_socket_fd );
                shutdown(param->mnl2adr_socket_fd, SHUT_RDWR);
                close(param->mnl2adr_socket_fd);
            }
            if (param->adr2mnl_socket_fd) {
                LOG_DEBUG("close adr2mnl_socket_fd %d ~",param->adr2mnl_socket_fd );
                shutdown(param->adr2mnl_socket_fd, SHUT_RDWR);
                close(param->adr2mnl_socket_fd);
            }
            free(param);
            param = NULL;
        }
        return NULL;
    }
    LOG_DEBUG("setup socket adr <--> mnld ok");
    module_list_init(&param->event_list);
    pthread_mutex_init(&param->event_mutex, NULL);
    pthread_cond_init(&param->event_cond, NULL);

    param->mnld2adr_recv_thread_exit = 0;
    ret = pthread_create(&param->mnld2adr_recv_thread,
                NULL, mnld2adr_recv_thread,(void *)param);
    if (ret != 0) {
        LOG_ERROR("can't create mnld2algo_handler_thread,%s",strerror(errno));
        adr_mnld_deinit(param);
        return NULL;
    }
    LOG_DEBUG("create mnld2algo_handler_thread ok");

    param->mnld2adr_hdlr_thread_exit = 0;
    ret = pthread_create(&param->mnld2adr_hdlr_thread,
                NULL, mnld2adr_event_handler_thread,(void *)param);
    if (ret != 0) {
        LOG_ERROR("can't create mnld2adr_event_handler_thread,%s", strerror(errno));
        adr_mnld_deinit(param);
        return NULL;
    }

    LOG_DEBUG("create mnld2adr_event_handler_thread ok");

    return param;
}

static void 
daemon_send_nmea_to_mnld(daemon_device *dev, char *buf, int len)
{
    char adr2mnl_buff[MNL_ADR_MAX_BUFF_SIZE] = {0};
    int adr2mnl_offset = 0;
    int ret = 0;
    if(len <= MNL_ADR_MAX_BUFF_SIZE) {
        put_int(adr2mnl_buff, &adr2mnl_offset, MNL_ADR_TYPE_ADR2MNL_NMEA_DATA);
        put_int(adr2mnl_buff, &adr2mnl_offset, len);
        put_binary(adr2mnl_buff, &adr2mnl_offset, (const char*)buf, len);

        ret = safe_sendto(dev->param->adr2mnl_socket_fd,&dev->param->sock_adr2mnl_addr, 
                        adr2mnl_buff, adr2mnl_offset);
        if(ret < 0)
            LOG_ERROR("send nmea to mnld failed, ret[%d],buf[%s]", ret, buf);
        else
            LOG_DEBUG("send nmea to mnld ok, ret[%d],buf[%s]", ret, buf);

    } else {
        LOG_ERROR("nmea length > MNL_ADR_MAX_BUFF_SIZE[%d], droped",
            MNL_ADR_MAX_BUFF_SIZE);
    }
}

//receive data from adr algo & send to mnld
static void daemon_driver_download(module *mod, module_box *box)
{
    struct daemon_device *dev;
    dev = (daemon_device*)module_get_driver_data(mod);
    LOG_DEBUG("Daemon: recieve data from:%s, data type id %d", box->from, box->type);

    //TO-DO: send to mnld
    switch(box->type) {
        case DATA_ADR_ALGO_MNLD:
            daemon_send_nmea_to_mnld(dev, (char *)(box->data), box->count);
            break;
    }

}

static int daemon_load_config_file(DAEMON_CONFIG_PARAM *daemon_config)
{
    struct adr_config_section *section;

    section = adr_config_get_section(config, "daemon", NULL, NULL);
    if (section) {
        adr_config_section_get_string(section, "log_path",
                    &daemon_config->log_path, "/data/adr_log");
        adr_config_section_get_int32_t(section, "open_sanity_test",
                    &daemon_config->open_sanity_test, 0);
        adr_config_section_get_int32_t(section, "open_data_store",
                    &daemon_config->open_data_store, 0);
    }else {
       LOG_ERROR("get daemon section failed");
       return -1;
   }

    section = adr_config_get_section(config, "adr_algo", NULL, NULL);
    if (section) {
        adr_config_section_get_int32_t(section, "open_algo_mnld",
                    &daemon_config->open_algo_mnld, 1);
    }else {
       LOG_ERROR("get adr_algo section failed");
       return -1;
   }

     section = adr_config_get_section(config, "odom", NULL, NULL);
     if (section) {
         adr_config_section_get_int32_t(section, "open_fake_odom",
            &daemon_config->open_fake_odom, OPEN_FAKE_ODOM_DEGAULT);
     }else {
        LOG_ERROR("get odom section failed");
        return -1;
    }

    return 0;
}

void daemon_driver_init(daemon_device *dev)
{
    pthread_cond_init(&dev->cond, NULL);
    pthread_mutex_init(&dev->mutex, NULL);
    pthread_mutex_init(&dev->flag_gps_open_mutex, NULL);

    daemon_load_config_file(&dev->daemon_config);
    log_path = dev->daemon_config.log_path;

    dev->running = 1;

    if(!dev->daemon_config.open_fake_odom)
        dev->odom = dev->modInf->create(module_odom_init, dev->modInf->private);
    dev->algo = dev->modInf->create(module_adr_algo_init, dev->modInf->private);
    dev->sensor = dev->modInf->create(module_sensor_init, dev->modInf->private);
    dev->debug = dev->modInf->create(module_debug_init, dev->modInf->private);
    dev->gnss = dev->modInf->create(module_gnss_init, dev->modInf->private);
    dev->daemon = dev->modInf->create(module_daemon_init, dev->modInf->private);

    if(dev->daemon_config.open_sanity_test){
        dev->sanity = dev->modInf->create(module_sanity_init, dev->modInf->private);
    }
    if(dev->daemon_config.open_data_store){
        dev->store = dev->modInf->create(module_store_init, dev->modInf->private);
    }
}

void daemon_driver_deinit(daemon_device *dev)
{
    if(dev->gnss)
        dev->modInf->destroy(module_gnss_deinit, dev->gnss);
    if(dev->sensor)
        dev->modInf->destroy(module_sensor_deinit, dev->sensor);
    if(dev->odom)
        dev->modInf->destroy(module_odom_deinit, dev->odom);
    if(dev->algo)
        dev->modInf->destroy(module_adr_algo_deinit, dev->algo);
    if(dev->debug)
        dev->modInf->destroy(module_debug_deinit, dev->debug);
    if(dev->daemon)
        dev->modInf->destroy(module_daemon_deinit, dev->daemon);
    if(dev->daemon_config.open_sanity_test && dev->sanity){
        dev->modInf->destroy(module_sanity_deinit, dev->sanity);
    }
    if(dev->daemon_config.open_data_store && dev->store){
        dev->modInf->destroy(module_store_deinit, dev->store);
    }

    pthread_cond_destroy(&dev->cond);
    pthread_mutex_destroy(&dev->mutex);
    pthread_mutex_destroy(&dev->flag_gps_open_mutex);
}

void daemon_driver_start(daemon_device *dev)
{
    #if ADR_DEBUG
    dev->modInf->ops_forall("init", dev->modInf->private);
    dev->modInf->ops_forall("registry", dev->modInf->private);
    dev->modInf->ops_forall("start", dev->modInf->private);
    pthread_mutex_lock(&dev->flag_gps_open_mutex);
    dev->flag_gps_open = 1;
    LOG_INFO("ADR_DEBUG finish init/start");
    pthread_mutex_unlock(&dev->flag_gps_open_mutex);
    #endif
    if(!(dev->param= adr_mnld_init((void *)dev))){
        LOG_ERROR("daemon start failed");
        abort();
    }else {
        LOG_INFO("daemon start successfully");
    }
}

void daemon_driver_stop(daemon_device *dev)
{
    adr_mnld_deinit(dev->param);
}

void daemon_parse_commandline(daemon_device *dev, int argc, char *argv[])
{
    UNUSED(dev);
    for (int i = 1; i < argc; i++)
    {
        if (strcmp("-d", argv[i]) == 0)
        {
            set_log_level(&log_dbg_level, atoi(argv[++i]));
        }
        else if (strcmp("-da", argv[i]) == 0)
        {
            set_log_level(&log_algo_level, atoi(argv[++i]));
        }
        else if (strcmp("-dv", argv[i]) == 0)
        {
            set_log_level(&log_vehicle_level, atoi(argv[++i]));
        }/*
        else if (strcmp("-t", argv[i]) == 0)
        {
            dev->open_sanity_test = 1;
        }
        else if (strcmp("-s", argv[i]) == 0)
        {
            dev->open_data_store = 1;
            dev->log_path = argv[++i];
        }*/
        else if (strcmp("-h", argv[i]) == 0)
        {
           usage(0);
        }
        else if (strncmp("-",argv[i],1) == 0)
        {
           usage(-1);
        }
    }
}

void daemon_sighlr(int signo)
{
    if ((signo == SIGUSR1) || (signo == SIGINT)) {
        LOG_INFO("daemon_sighlr catch signal:%d", signo);
        pthread_mutex_lock(&g_daemon_ctx->mutex);
        g_daemon_ctx->running = 0;
        pthread_mutex_unlock(&g_daemon_ctx->mutex);
        pthread_cond_signal(&g_daemon_ctx->cond);
    }
    else if (signo == SIGPIPE)
        LOG_INFO("daemon_sighlr SIGPIPE ignore\n");
}

static void daemon_driver_registry(struct module *mod, process_data callback)
{
   struct daemon_device *dev;
   dev = (daemon_device*)module_get_driver_data(mod);

   dev->upload = callback;
}

static module_driver daemon_driver = {
    .name = "adr_daemon",
    .capability = DATA_ADR_ALGO_MNLD,
    .download = daemon_driver_download,
    .registry = daemon_driver_registry,
};

//no need to malloc device more
int module_daemon_init(module *mod)
{
    struct daemon_device *dev = g_daemon_ctx;

    if (!dev)
        return -1;

    dev->mod = mod;
    module_set_name(mod, daemon_driver.name);
    module_register_driver(mod, &daemon_driver, (void *)dev);
    return 0;
}

void module_daemon_deinit(module *mod)
{
    module_register_driver(mod, NULL, NULL);
}

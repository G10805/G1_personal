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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#include "hal2mnl_interface.h"
#include "module.h"
#include "adr_util.h"
#include "adr_data_convert.h"
#include "module_gnss_helper.h"
#include "config-parser.h"

extern struct adr_config *config;
int gps_gsa_fix_3dmode = 0;
int upload_number = 0;
//int mnld_connect = 0;
struct timespec upload_timediff;

#define GNSS_MSG_LEN 4
#define GNSS_MODULE_STAT_MSG_LEN 4
#define MNLD_DEBUG_SOCKET_PORT_DEFAULT 7000
#define OPEN_FAKE_ODOM_DEGAULT 0

typedef struct {
    int32_t mnld_debug_socket_port;
    int32_t open_fake_odom;
    int32_t open_baro;
}GNSS_CONFIG_PARAM;

typedef struct gnss_pos_stat_msg {
    adr_gnss_msg_struct adrdata;
    int32_t busy;
} gnss_pos_stat_msg;

typedef struct gnss_time_stat_msg {
    struct timespec spec;
    int32_t busy;
} gnss_time_stat_msg;

typedef struct gnss_module_stat_msg {
    gnss_module_data nmearaw;
    struct module_list link;
    int32_t busy;
} gnss_module_stat_msg;

typedef struct gnss_device {
    char *name;
    struct module *mod;
    void (*upload)(module *, module_box *);
    
    void *gpsmodule;

    pthread_t dataready_thread;
    int32_t dataready_thread_exit;
    pthread_mutex_t dataready_mutex;
    pthread_cond_t dataready_cond;

    pthread_t command_thread;
    int32_t command_thread_exit;
    pthread_mutex_t command_mutex;
    pthread_cond_t command_cond;

    int mnld_sockfd;//server
    int gnss_sockfd;//client
    gnss_module_data *nmearaw;
    gnss_module_stat_msg *gnss_stat_msg[GNSS_MODULE_STAT_MSG_LEN];
    gnss_module_stat_msg *w_gnss_stat_msg;
    struct module_list gnss_ready_list;
    pthread_mutex_t gnss_ready_list_mutex;

    module_box *box[GNSS_MSG_LEN];

    //from sensor module
    double baro;    //%0.2f/OMRON raw data
    double baroT;    //%0.3f/OMRON raw data
    struct timespec baro_boottime;
    //from odom
    double odom;
    struct timespec odom_boottime;

    gnss_pos_stat_msg *pos[GNSS_POS_DATA_LEN];   //gnss data to adr
    struct gnss_time_stat_msg *timediff[GNSS_TIM_DATA_LEN]; //gnss diff time
    int timediff_pos;

    GNSS_CONFIG_PARAM gnss_config;
} gnss_device;

static gnss_device *g_gnss_ctx = NULL;

static gnss_module_stat_msg *get_next_gnss_module_stat_msg(gnss_device *dev)
{
    int32_t cnt = 0;

    while(cnt < GNSS_MODULE_STAT_MSG_LEN) {
        if (!dev->gnss_stat_msg[cnt]) {
            dev->gnss_stat_msg[cnt] = (gnss_module_stat_msg *)zalloc(sizeof(gnss_module_stat_msg));
            module_list_init(&dev->gnss_stat_msg[cnt]->link);
            return dev->gnss_stat_msg[cnt];
        } else {
            if (dev->gnss_stat_msg[cnt]->busy == 0){
                return dev->gnss_stat_msg[cnt];
            }
        }
        cnt += 1;
    }
    return NULL;
}

static void free_gnss_module_stat_msg(gnss_device *dev)
{
    int32_t cnt = 0;

    while(cnt < GNSS_MODULE_STAT_MSG_LEN) {
        if (dev->gnss_stat_msg[cnt]) {
            free(dev->gnss_stat_msg[cnt]);
            dev->gnss_stat_msg[cnt] = NULL;
        }
        cnt += 1;
    }
}

static gnss_pos_stat_msg *get_next_pos_data_stat_msg(gnss_device *dev)
{
    int32_t cnt = 0;

    while(cnt < GNSS_POS_DATA_LEN) {
        if (!dev->pos[cnt]) {
            dev->pos[cnt] = (gnss_pos_stat_msg *)zalloc(sizeof(gnss_pos_stat_msg));
            return dev->pos[cnt];
        } else {
            if (dev->pos[cnt]->busy == 0)
                 return dev->pos[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

static void free_pos_data_stat_msg(gnss_device *dev)
{
    int32_t cnt = 0;

    while(cnt < GNSS_POS_DATA_LEN) {
        if (dev->pos[cnt]) {
            free(dev->pos[cnt]);
            dev->pos[cnt] = NULL;
        }
        cnt += 1;
    }
}

static gnss_time_stat_msg *get_next_time_data_stat_msg(gnss_device *dev)
{
    int32_t tmp = dev->timediff_pos;
    if (!dev->timediff[dev->timediff_pos]) {
        dev->timediff[tmp] = (gnss_time_stat_msg *)
                    zalloc(sizeof(gnss_time_stat_msg));
        if(dev->timediff[tmp]){
            dev->timediff_pos +=1;
        } else {
            LOG_ERROR("zalloc time buffer failed");
            abort();
        }
    } else {
        dev->timediff_pos +=1;
    }
    if(dev->timediff_pos == GNSS_TIM_DATA_LEN){
        dev->timediff_pos = 0;
    }
    return dev->timediff[tmp];
}

int get_gnss_average_time_diff(struct timespec *result)
{
    struct gnss_device *dev = g_gnss_ctx;
    int32_t sum_len = 0;
    uint64_t  avg_nsec = 0, tv_sec = 0, tv_nsec = 0;

    while(sum_len < GNSS_TIM_DATA_LEN && dev->timediff[sum_len]) {
        tv_sec = tv_sec + dev->timediff[sum_len]->spec.tv_sec;
        tv_nsec = tv_nsec + dev->timediff[sum_len]->spec.tv_nsec;
        sum_len += 1;
    }
    if(sum_len){
        avg_nsec = (tv_sec / sum_len) * 1000000000 + ((tv_sec % sum_len)* 1000000000 + tv_nsec) / sum_len;
        result->tv_sec = avg_nsec / 1000000000;
        result->tv_nsec = avg_nsec % 1000000000;
        return 1;
    } else{
        return 0;
    }
}

int get_gnss_std_deviation_time_diff(struct timespec *avg_time_diff, struct timespec *result)
{
    struct gnss_device *dev = g_gnss_ctx;
    int32_t len = 0, avg_len = 0;
    uint64_t deviation_nsec = 0, std_deviation_nsec, nsec = 0;
    while(len < GNSS_TIM_DATA_LEN && dev->timediff[len]) {
        nsec = (uint64_t)labs((dev->timediff[len]->spec.tv_sec - avg_time_diff->tv_sec )* 1000000000
                              + (dev->timediff[len]->spec.tv_nsec - avg_time_diff->tv_nsec) );
        if(nsec){
            deviation_nsec += (uint64_t)pow((double)nsec, 2);
        }
        len += 1;
    }
    avg_len = len - 1;
    if(deviation_nsec && avg_len > 0 ){
        std_deviation_nsec = (uint64_t)sqrt((double)deviation_nsec / avg_len);
        result->tv_sec = std_deviation_nsec / 1000000000;
        result->tv_nsec = std_deviation_nsec % 1000000000;
        LOG_DEBUG("standard deviation [%ld sec, %ld nsec]",result->tv_sec, result->tv_nsec);
        return 1;
    } else{
        return 0;
    }
}
int get_diff_uploadtime_avg_time(struct timespec *avg_time_diff, struct timespec *result){
    if(avg_time_diff->tv_sec > upload_timediff.tv_sec ||
         (avg_time_diff->tv_sec == upload_timediff.tv_sec &&
          avg_time_diff->tv_nsec > upload_timediff.tv_nsec)){
        diff_timespec_timespec(avg_time_diff, &upload_timediff, result);
        LOG_DEBUG("return diff [%ld sec, %ld nsec] ", result->tv_sec, result->tv_nsec);
        return 1;
    } else {
        diff_timespec_timespec(&upload_timediff, avg_time_diff, result);
        LOG_DEBUG("return diff [%ld sec, %ld nsec] ", result->tv_sec, result->tv_nsec);
        return 1;
    }
    return 0;
}

static void free_time_data_stat_msg(gnss_device *dev)
{
    int32_t cnt = 0;

    while(cnt < GNSS_TIM_DATA_LEN) {
        if (dev->timediff[cnt]) {
            free(dev->timediff[cnt]);
            dev->timediff[cnt] = NULL;
        }
        cnt += 1;
    }
}

struct timespec *get_available_timespec()
{
    struct gnss_device *dev = g_gnss_ctx;

    gnss_time_stat_msg *time = get_next_time_data_stat_msg(dev);
    return &(time->spec);
}

static void *gnss_data_ready_thread(void *data)
{
    struct gnss_device *dev = (struct gnss_device *)data;
    gnss_module_stat_msg *gnss_stat_msg;
    gnss_module_data *nmearaw;
    module_box *box;

    LOG_DEBUG(" gnss_data_ready_thread starting");
    while(!dev->dataready_thread_exit){
        pthread_mutex_lock(&dev->dataready_mutex);
        pthread_cond_wait(&dev->dataready_cond, &dev->dataready_mutex);
        pthread_mutex_unlock(&dev->dataready_mutex);
        /*find*/
        while(1) {
            pthread_mutex_lock(&dev->gnss_ready_list_mutex);
            if (module_list_empty(&dev->gnss_ready_list)) {
                pthread_mutex_unlock(&dev->gnss_ready_list_mutex);
                break;
            }
            gnss_stat_msg = module_container_of(dev->gnss_ready_list.prev, gnss_stat_msg, link);
            pthread_mutex_unlock(&dev->gnss_ready_list_mutex);

            box = get_available_module_box(dev->mod);
            nmearaw = &gnss_stat_msg->nmearaw;
            gnss_pos_stat_msg *gnss = get_next_pos_data_stat_msg(dev);
            if (!gnss) {
                LOG_ERROR("gnss fail to get available buffer for adr");
                continue;
            }
            gnss_to_adr_msg((void *)nmearaw, (void *)&gnss->adrdata);
            nmearaw->sv_out.nmea.sentence[nmearaw->sv_out.nmea.len] = '\0';
            nmearaw->sv_out.nmea.len += 1;
            gnss->busy = 1;
            box->data = (void *)&gnss->adrdata;
            box->priv = (void *)&nmearaw->sv_out.nmea;    //nmea sentence
            box->count = sizeof(gnss->adrdata);
            box->type = DATA_GNSS_PARSERD;
            dev->upload(dev->mod, box);
            gnss->busy = 0;

            box->data = (void *)nmearaw;
            box->count = 1;
            box->type = DATA_GNSS_STORE;
            dev->upload(dev->mod, box);

            release_module_box(box);

            pthread_mutex_lock(&dev->gnss_ready_list_mutex);
            module_list_remove(&gnss_stat_msg->link);
            module_list_init(&gnss_stat_msg->link);
            pthread_mutex_unlock(&dev->gnss_ready_list_mutex);

            pthread_mutex_lock(&dev->dataready_mutex);
            gnss_stat_msg->busy = 0;
            pthread_mutex_unlock(&dev->dataready_mutex);
        }
    }
    LOG_DEBUG("data ready thread exit!~");
    pthread_exit(0);
}

static int mnl_debug_data_handle(int len, int gnss_sockfd,
    char *socket_rcv_buf, gnss_device *dev)
{
    int index;
    static int max_cache_len = 256;
    char gpsrcv_cache[max_cache_len];
    int gpsrcv_index = 0;
    int gpsrcv_len = 0;
    memset(gpsrcv_cache, 0, max_cache_len);

    for(index = 0; index < len; index++)
    {
        gpsrcv_cache[gpsrcv_index] = socket_rcv_buf[index];
        if (gpsrcv_index >= max_cache_len) {
            LOG_ERROR("The frame cache from libmnl %s is out"
                " of max(%d)\n", gpsrcv_cache, max_cache_len);
            gpsrcv_index = 0;
            continue;
        }

        if (gpsrcv_cache[gpsrcv_index] == '\n') {
            if (gnss_sockfd == -1) {
                gpsrcv_index = 0;
                break;
            }

            gpsrcv_len = gpsrcv_index + 1;

            if (gpsrcv_cache[0] != '$') {
                gpsrcv_index = 0;
                continue;
            }

            if ((!memcmp(gpsrcv_cache, "$PMTK010", 8))||(!memcmp(gpsrcv_cache, "$PMTK001", 8))||
                (!memcmp(gpsrcv_cache, "$PMTK705", 8))) {
                module_box *box = get_available_module_box(dev->mod);
                box->data = (void *)gpsrcv_cache;
                box->count = gpsrcv_len;
                box->type = CMD_GNSS_ACK;
                dev->upload(dev->mod, box);
                release_module_box(box);

                LOG_DEBUG("CMD GNSS ACK:%s", gpsrcv_cache);
            }
            gpsrcv_index = 0;
        } else {
            gpsrcv_index += 1;
        }
    }
    return 0;
}

static void *gnss_command_thread(void *data)
{
    char socket_rcv_buf[4096];
    gnss_device *dev = (gnss_device *)data;
    int read_len, n, i;
    struct epoll_event events[MAX_EPOLL_EVENT];
    memset(&events, 0, sizeof(events));
    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOG_ERROR("epoll_create failure reason=[%s]%d",
            strerror(errno), errno);
        return NULL;
    }

    LOG_INFO("sockfd:%d", dev->gnss_sockfd);

    if(epoll_add_fd(epfd, dev->gnss_sockfd) == -1) {
        LOG_ERROR("epoll_add_fd() for gnss_sockfd failed");
        close(epfd);
        return NULL;
    }
    dev->command_thread_exit = 0;
    while (!dev->command_thread_exit) {
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT, EPOLL_TIMEOUT);
        LOG_DEBUG("gnss_command_thread epoll wait return %d command_thread_exit %d", n, dev->command_thread_exit);

        if (n == -1) {
            //if (errno == EINTR) {
                //continue;
            //} else {
                LOG_ERROR("epoll_wait failure reason=[%s]%d",strerror(errno), errno);
                dev->command_thread_exit = 1;
                break;
            //}
        } else if (n == 0){
            if (dev->command_thread_exit == 1)
                break;
        }else {
            for(i = 0; i< n; i++){
                if (events[i].data.fd == dev->gnss_sockfd) {
                    if (events[i].events & EPOLLIN) {
                        memset(socket_rcv_buf, 0, sizeof(socket_rcv_buf));
                        read_len = safe_recvfrom(dev->gnss_sockfd, socket_rcv_buf, sizeof(socket_rcv_buf));
                        if (read_len <= 0) {
                            LOG_ERROR("safe_recvfrom() failed read_len=%d", read_len);
                            dev->command_thread_exit = 1;
                            break;
                        } else {
                            mnl_debug_data_handle(read_len, dev->gnss_sockfd, socket_rcv_buf, dev);
                        }
                    }else {
                        LOG_ERROR("events[%d].events %d, errno(%s):%d",
                            i, events[i].events, strerror(errno), errno);
                    }
                }else {
                    LOG_ERROR("unknown fd=%d, gnss_sockfd=%d", events[i].data.fd, dev->gnss_sockfd);
                }
            }
        }
    }

    if(epfd != -1){
        close(epfd);
        epfd = -1;
    }

    LOG_INFO("command thread exit!~");
    pthread_exit(0);
}

void gnss_notify_time_msg(struct timespec *time)
{
    struct gnss_device *dev = g_gnss_ctx;
    struct module_box *box = get_available_module_box(dev->mod);

    if(!box){
        LOG_ERROR("get box failed");
        abort();
    }
     box->data = (void *)time;
     box->count = 1;
     box->type = DATA_GNSS_TIME;
     dev->upload(dev->mod, box);
     release_module_box(box);
     upload_timediff = *time;
}

static int gnss_load_config_file(GNSS_CONFIG_PARAM * gnss_config)
{
    struct adr_config_section *section;

    section = adr_config_get_section(config, "gnss", NULL, NULL);
    if (section) {
        adr_config_section_get_int32_t(section, "mnld_debug_socket_port",
            &gnss_config->mnld_debug_socket_port, MNLD_DEBUG_SOCKET_PORT_DEFAULT);
    }
   else {
       LOG_ERROR("get gnss section failed");
       return -1;
   }

    section = adr_config_get_section(config, "sensor", NULL, NULL);
    if(section){
        adr_config_section_get_int32_t(section, "open_baro",
            &gnss_config->open_baro, 1);
    }
    else {
        LOG_ERROR("get sensor section failed");
        return -1;
    }

    section = adr_config_get_section(config, "odom", NULL, NULL);
    if (section) {
        adr_config_section_get_int32_t(section, "open_fake_odom",
            &gnss_config->open_fake_odom, OPEN_FAKE_ODOM_DEGAULT);
    }
    else {
        LOG_ERROR("get odom section failed");
        return -1;
    }

    return 0;
}

void gnss_nmea_cb(gnss_device *dev, const char* nmea, int length)
{
    LOG_DEBUG("receive sentences: %s", nmea);
    static gnss_module_data *nmearaw;
    gnss_module_stat_msg *gnss_stat_msg;
    if (!dev->w_gnss_stat_msg) {
        dev->w_gnss_stat_msg = get_next_gnss_module_stat_msg(dev);
        if (!dev->w_gnss_stat_msg){
            LOG_ERROR("gnss data ready thread process too slow");
            return;
        }
        nmea_reader_init(&dev->w_gnss_stat_msg->nmearaw);
    }
     nmearaw = &(dev->w_gnss_stat_msg->nmearaw);

    //parser
    gnss_parser_prefunc(nmearaw, nmea, length);

    if ((nmearaw->frame_flag & FRAME_END) &&
            (!(nmearaw->frame_flag & FRAME_START))) {
        nmea_reader_init(nmearaw);
        LOG_ERROR("receive error frame");
        return;
    }

    /*only when whole frame received, then go next*/
    if (nmearaw->frame_flag != (FRAME_START | FRAME_END))
        return;
    //update matched sv info
    gnss_parser_postfunc(nmearaw);

    struct timespec diff;

    if(dev->gnss_config.open_fake_odom){
        nmearaw->sensor.odo = 0;
    }else{
        diff_timespec_timespec(&nmearaw->drvtime, &dev->odom_boottime, &diff);
        if (diff.tv_sec <= 1) {
            nmearaw->sensor.odo = dev->odom;
            LOG_DEBUG("odom[%lf, %ld s, %ld ns], GNSS[%ld s, %ld ns]",
                nmearaw->sensor.odo, dev->odom_boottime.tv_sec, dev->odom_boottime.tv_nsec,
                nmearaw->drvtime.tv_sec, nmearaw->drvtime.tv_nsec);
        } else {
                nmearaw->sensor.odo = -9999;
                LOG_WARN("the time interval between odom[%ld s, %ld ns] and GNSS[%ld s, %ld ns]"
                " > 1s", dev->odom_boottime.tv_sec, dev->odom_boottime.tv_nsec,
                nmearaw->drvtime.tv_sec, nmearaw->drvtime.tv_nsec);
        }
    }

    if(dev->gnss_config.open_baro){
        diff_timespec_timespec(&nmearaw->drvtime, &dev->baro_boottime, &diff);
        if (diff.tv_sec <= 1) {
            nmearaw->sensor.baro = dev->baro;
            nmearaw->sensor.baroT = dev->baroT;
            LOG_DEBUG("baro[%lf, %lf, %ld s, %ld ns], GNSS[%ld s, %ld ns]",
                nmearaw->sensor.baro, nmearaw->sensor.baroT,
                dev->baro_boottime.tv_sec, dev->baro_boottime.tv_nsec,
                nmearaw->drvtime.tv_sec, nmearaw->drvtime.tv_nsec);
        } else {
            nmearaw->sensor.baro = -9999;
            nmearaw->sensor.baroT = -9999;
            LOG_WARN("the time interval between baro[%ld s, %ld ns] and GNSS[%ld s, %ld ns]"
                " > 1s", dev->baro_boottime.tv_sec, dev->baro_boottime.tv_nsec,
                nmearaw->drvtime.tv_sec, nmearaw->drvtime.tv_nsec);
        }
    }else{
        nmearaw->sensor.baro = -9999;
        nmearaw->sensor.baroT = -9999;
    }
    pthread_mutex_lock(&dev->dataready_mutex);
    dev->w_gnss_stat_msg->busy = 1;
    pthread_mutex_unlock(&dev->dataready_mutex);

    pthread_mutex_lock(&dev->gnss_ready_list_mutex);
    module_list_insert(&dev->gnss_ready_list, &dev->w_gnss_stat_msg->link);
    pthread_mutex_unlock(&dev->gnss_ready_list_mutex);

    pthread_cond_broadcast(&dev->dataready_cond);

    gnss_stat_msg = get_next_gnss_module_stat_msg(dev);
    if (!gnss_stat_msg) {
        dev->w_gnss_stat_msg = NULL;
        LOG_ERROR("GNSS data process seems too slow, buffer cost out:"
            " reuse last frame");
    } else {
        dev->w_gnss_stat_msg = gnss_stat_msg;
        nmea_reader_init(&dev->w_gnss_stat_msg->nmearaw);
    }
}

static int32_t gnss_device_data_init(gnss_device *dev)
{
    if(dev->gnss_config.open_fake_odom){
        dev->odom = 0;
    } else{
        dev->odom = -9999;
    }
    dev->baro = -9999;
    dev->baroT = -9999;
    dev->gnss_sockfd = -1;
    dev->command_thread_exit = 1;

    return 0;
}

static int gnss_driver_init(struct module *mod)
{
    int ret = -1;
    struct gnss_device *dev =
        (gnss_device*)module_get_driver_data(mod);

    ret = gnss_load_config_file(&dev->gnss_config);
    if (ret != 0) {
        LOG_ERROR("something wrong in load gnss config file");
        return -1;
    }

    memset(&upload_timediff, 0, sizeof(upload_timediff));
    gnss_device_data_init(dev);

    dev->w_gnss_stat_msg = get_next_gnss_module_stat_msg(dev);
    if (!dev->w_gnss_stat_msg)
    {
        LOG_ERROR("can't find a available gnss module_stat msg for writing");
        goto init_failed;
    }

    module_list_init(&dev->gnss_ready_list);
    pthread_mutex_init(&dev->gnss_ready_list_mutex, NULL);

    dev->nmearaw = (gnss_module_data *)zalloc(sizeof(gnss_module_data));
    nmea_reader_init(dev->nmearaw);

    pthread_cond_init(&dev->dataready_cond, NULL);
    pthread_mutex_init(&dev->dataready_mutex, NULL);

    ret = pthread_create(&dev->dataready_thread, NULL, gnss_data_ready_thread,(void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create thread for gnss data ready ,%s",strerror(errno));
        free(dev->nmearaw);
        goto init_failed;
    }
    dev->dataready_thread_exit = 0;

    return ret;

init_failed:
    free_gnss_module_stat_msg(dev);
    return -1;
}

static void gnss_driver_start_done(struct module *mod)
{
    int ret = -1;
    struct gnss_device *dev;
    dev = (gnss_device*)module_get_driver_data(mod);

    if(dev->gnss_sockfd == -1)
    {
        dev->gnss_sockfd = gnss_command_socket_open(dev->gnss_config.mnld_debug_socket_port);
    }
    if(dev->gnss_sockfd == -1){
        LOG_ERROR("Socket open error");
        return;
    }

    char *pmtk_cmd = "$PMTK876,1*26\r\n";
    if(send_buf_to_libmnl(&dev->gnss_sockfd, pmtk_cmd, strlen(pmtk_cmd)) <= 0){
        LOG_ERROR("send %s to libmnl failed", pmtk_cmd);
    }
    /*!<only after adr open done, can connect the debug socket>*/
    if(dev->command_thread_exit){
        ret = pthread_create(&dev->command_thread, NULL, gnss_command_thread,(void *)dev);
        if (ret != 0) {
            LOG_ERROR("can't create thread for gnss command,%s",strerror(errno));
            return;
        }
    }
}

static void gnss_align_sensor(gnss_device *dev, sensor_iic_data *data)
{
    dev->baro = data->baro;
    dev->baroT = data->baroT;
    dev->baro_boottime = data->baro_boottime;
    LOG_DEBUG("receive baro[%f %f]", data->baro, data->baroT);
}

static void gnss_align_odom(gnss_device *dev, odom_uart_data *data)
{
    dev->odom = (double) data->odom;
    //dev->rudder = (double)data->rudder;
    //dev->gear = data->gear;
    dev->odom_boottime = data->odom_boottime;
}

static void gnss_driver_download(struct module *mod, module_box *box)
{
    struct gnss_device *dev;
    dev = (gnss_device*)module_get_driver_data(mod);

    LOG_DEBUG("GNSS: recieve data from:%s, data type id %d", box->from, box->type);

    switch(box->type) {
        case DATA_SENSOR_BARO:
            gnss_align_sensor(dev, (struct sensor_iic_data *)(box->data));
            break;
        case DATA_ODOM:
            gnss_align_odom(dev, (struct odom_uart_data *)(box->data));
            break;
        case CMD_ADR_PMTK:
            send_buf_to_libmnl(&dev->gnss_sockfd, (char *)box->data, box->count);
            break;
        case DATA_DAEMON_NMEA:
            gnss_nmea_cb(dev, (char *)(box->data), box->count);
            break;
        default :
            LOG_ERROR("can not process this type");
    }
}

static void gnss_driver_stop(struct module *mod)
{
    struct gnss_device *dev;
    dev = (gnss_device*)module_get_driver_data(mod);

    LOG_INFO("start stop gnss command thread......");

    if(dev->gnss_sockfd != -1){
        close(dev->gnss_sockfd);
        dev->gnss_sockfd = -1;
    }

    dev->command_thread_exit = 1;
    pthread_join(dev->command_thread, NULL);
    LOG_INFO("stop gnss command thread ok......");

    if (dev->nmearaw)
        free(dev->nmearaw);
}

static void gnss_driver_deinit(struct module *mod)
{
    struct gnss_device *dev;
    dev = (gnss_device*)module_get_driver_data(mod);

    dev->dataready_thread_exit = 1;
    pthread_cond_broadcast(&dev->dataready_cond);
    pthread_join(dev->dataready_thread, NULL);

    pthread_cond_destroy(&dev->dataready_cond);
    pthread_mutex_destroy(&dev->dataready_mutex);

    if (dev->gpsmodule)
        dlclose(dev->gpsmodule);

    free_time_data_stat_msg(dev);
    free_pos_data_stat_msg(dev);
    free_gnss_module_stat_msg(dev);
    gps_gsa_fix_3dmode = 0;
    upload_number = 0;
//    mnld_connect = 0;
    memset(&upload_timediff, 0, sizeof(upload_timediff));

    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(gnss_device));
    dev->mod = store_mod;
}

static void gnss_driver_registry(struct module *mod, process_data callback)
{
   struct gnss_device *dev;
   dev = (gnss_device*)module_get_driver_data(mod);

   dev->upload = callback;
}

static module_driver gnss_driver = {
    .name = "mtk_gnss",
    .capability = DATA_ODOM | DATA_SENSOR_BARO | CMD_ADR_PMTK | DATA_DAEMON_NMEA,
    .init = gnss_driver_init,
    .start_done = gnss_driver_start_done,
    .download = gnss_driver_download,
    .stop = gnss_driver_stop,
    .deinit = gnss_driver_deinit,
    .registry = gnss_driver_registry,
};

void module_gnss_deinit(module *mod)
{
    struct gnss_device *dev;
    dev = (gnss_device*)module_get_driver_data(mod);
    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_gnss_init(module *mod)
{
    struct gnss_device *dev;

    dev = (gnss_device*)zalloc(sizeof(gnss_device));
    if (!dev) {
        LOG_ERROR("Fail to create gnss_device");
        return -1;
    }
    g_gnss_ctx = dev;
    dev->mod = mod;
    module_set_name(mod, gnss_driver.name);
    module_register_driver(mod, &gnss_driver, (void *)dev);
    return 0;
}

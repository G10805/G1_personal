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

#include "module.h"
#include "adr_cmd.h"
#include "config-parser.h"
#include "adr_release_type.h"
#include "module_adr_algo_helper.h"

extern int  log_algo_level;
extern struct adr_config *config;

#define ADR_ALGO_MSG_LEN 4
#define ADR_ALGO_MODULE_STAT_MSG_LEN 8
#define ADR_ALGO_CACHE_NMEA_LEN 32

//typedef void (*PmtkRetFunc)(char* pmtk_msg, int pmtk_len);
//typedef void (*PvtRetFunc)(adr_pvt_msg_struct *pvt_msg);

typedef struct {
    adr_user_param_struct user_parm_struct;
}ADR_ALGO_CONFIG_PARAM;

typedef struct algo_module_stat_msg {
    adr_pvt_msg_struct pvtdata;
    struct module_list link;
    int32_t busy;
} algo_module_stat_msg;

typedef struct gnss_nmea_stat_data {
    gnss_nmea_data nmea;
    double ts_sec;
    int ts_millisec;
    int32_t busy;
    int64_t monotonic_raw;
    struct module_list link;
} gnss_nmea_stat_data;

typedef struct adr_algo_device {
    char *name;
    struct module *mod;
    void (*upload)(module *, module_box *);

    PmtkRetFunc algo_pmtk_cb;
    PvtRetFunc algo_pvt_cb;
    /*!<waitting to be process>*/
    struct module_list pvt_list;
    pthread_mutex_t pvt_list_mutex;

    module_box *msg[ADR_ALGO_MSG_LEN];

    gnss_nmea_stat_data *in_Nmeas_stat[ADR_ALGO_CACHE_NMEA_LEN];

    struct module_list nmea_list;
    pthread_mutex_t nmea_list_mutex;
    gnss_nmea_data in_Nmea;
    gnss_nmea_data out_Nmea;    /*<need add cache buffer to make thread desync>*/

    pthread_t dataready_thread;
    int32_t dataready_thread_exit;
    pthread_mutex_t dataready_mutex;
    pthread_cond_t dataready_cond;

    algo_module_stat_msg *algo_stat_msg[ADR_ALGO_MODULE_STAT_MSG_LEN];
    algo_module_stat_msg *w_algo_stat_msg;

    char *algo_cmd;

    ADR_ALGO_CONFIG_PARAM adr_algo_config;
} adr_algo_device;

static adr_algo_device *g_adr_algo_ctx = NULL;

static int adr_algo_load_config_file(ADR_ALGO_CONFIG_PARAM *adr_algo_config)
{
    adr_user_param_struct *user_parm_struct = &adr_algo_config->user_parm_struct;

    struct adr_config_section *section;
    char *p = NULL;

    section = adr_config_get_section(config, "acc", NULL, NULL);
    if (section) {
        adr_config_section_get_double(section, "a_d_sigma",
                    &user_parm_struct->acc_config.a_d_sigma, 0.004903325080872);
        adr_config_section_get_double(section, "a_w_sigma",
                    &user_parm_struct->acc_config.a_w_sigma, 0.176519702911377);
        adr_config_section_get_double(section, "tau_a",
                    &user_parm_struct->acc_config.tau_a, 300.0);
        adr_config_section_get_double(section, "a_zero_pt_offset",
                    &user_parm_struct->acc_config.a_zero_pt_offset, 0.6864655);
        adr_config_section_get_double(section, "a_offset_sigma",
                    &user_parm_struct->acc_config.a_offset_sigma, 0.63743225);
        adr_config_section_get_int32_t(section, "a_x_axis",
                    &user_parm_struct->acc_config.a_x_axis, 2);
        adr_config_section_get_int32_t(section, "a_y_axis",
                    &user_parm_struct->acc_config.a_y_axis, 1);
        adr_config_section_get_int32_t(section, "a_z_axis",
                    &user_parm_struct->acc_config.a_z_axis, 3);
        user_parm_struct->acc_config.valid = 1;

        LOG_INFO("load acc ok!");
    }

    section = adr_config_get_section(config, "gyro", NULL, NULL);
    if (section) {
        adr_config_section_get_double(section, "g_d_sigma",
                    &user_parm_struct->gyro_config.g_d_sigma, 0.000104719755120);
        adr_config_section_get_double(section, "g_w_sigma",
                    &user_parm_struct->gyro_config.g_w_sigma, 0.008290313946973);
        adr_config_section_get_double(section, "tau_g",
                    &user_parm_struct->gyro_config.tau_g, 300.0);
        adr_config_section_get_double(section, "g_zero_pt_offset",
                    &user_parm_struct->gyro_config.g_zero_pt_offset, 0.017453292519943);
        adr_config_section_get_double(section, "g_offset_sigma",
                    &user_parm_struct->gyro_config.g_offset_sigma, 0.017453292519943);
        adr_config_section_get_double(section, "g_z_neg_scale_factor",
                    &user_parm_struct->gyro_config.g_z_neg_sf, 1.002);
        adr_config_section_get_double(section, "g_z_pos_scale_factor",
                    &user_parm_struct->gyro_config.g_z_pos_sf, 1.000);
        adr_config_section_get_int32_t(section, "g_x_axis",
                    &user_parm_struct->gyro_config.g_x_axis, 2);
        adr_config_section_get_int32_t(section, "g_y_axis",
                    &user_parm_struct->gyro_config.g_y_axis, 1);
        adr_config_section_get_int32_t(section, "g_z_axis",
                    &user_parm_struct->gyro_config.g_z_axis, 3);
        user_parm_struct->gyro_config.valid = 1;

        LOG_INFO("load gyro ok!");
    }

    section = adr_config_get_section(config, "odometer", NULL, NULL);
    if (section) {
        adr_config_section_get_double(section, "scale_factor",
                    &user_parm_struct->odom_config.scale_factor, 1.0);
        user_parm_struct->odom_config.valid = 1;

        LOG_INFO("load odometer ok!");
    }

    section = adr_config_get_section(config, "lever-arm", NULL, NULL);
    if (section) {
        adr_config_section_get_double(section, "x",
                    &user_parm_struct->lever_arm_config.x, 0);
        adr_config_section_get_double(section, "y",
                    &user_parm_struct->lever_arm_config.y, 0);
        adr_config_section_get_double(section, "z",
                    &user_parm_struct->lever_arm_config.z, 0);
        user_parm_struct->lever_arm_config.valid = 1;

        LOG_INFO("load lever-arm ok!");
    }

    section = adr_config_get_section(config, "alignment", NULL, NULL);
    if (section) {
        adr_config_section_get_double(section, "roll",
                    &user_parm_struct->alignment_config.roll, 0);
        adr_config_section_get_double(section, "pitch",
                    &user_parm_struct->alignment_config.pitch, 0);
        adr_config_section_get_double(section, "yaw",
                    &user_parm_struct->alignment_config.yaw, 0);
        user_parm_struct->alignment_config.valid = 1;

        LOG_INFO("load alignment ok!");
    }

    section = adr_config_get_section(config, "frequency", NULL, NULL);
    if (section) {
        adr_config_section_get_int32_t(section, "gnss_freq",
                    &user_parm_struct->freq_config.gnss_freq, 1);
        adr_config_section_get_int32_t(section, "mems_freq",
                    &user_parm_struct->freq_config.mems_freq, 50);
        adr_config_section_get_int32_t(section, "output_freq",
                    &user_parm_struct->freq_config.output_freq, 1);
        user_parm_struct->freq_config.valid = 1;

        LOG_INFO("load user_parm_struct->freq_config.gnss_freq : %d ",
            user_parm_struct->freq_config.gnss_freq);
    }

    section = adr_config_get_section(config, "turn-on", NULL, NULL);
    if (section) {
        adr_config_section_get_string(section, "turn_on_path",
                    &p, "/data/adr/adr_archive.dat");
        if(p) {
            memcpy(user_parm_struct->turn_on_config.turn_on_path, p, strlen(p));
            user_parm_struct->turn_on_config.valid = 1;
            free(p);
            p = NULL;
        }else {
            LOG_ERROR("adr config get turn on path failed");
        }
    }

     section = adr_config_get_section(config, "adr_algo", NULL, NULL);
     if (section) {
         adr_config_section_get_int32_t(section, "open_algo_mnld",
                     &user_parm_struct->algo_config.enable_feedback, 1);
         user_parm_struct->algo_config.valid = 1;

         LOG_INFO("load adr_algo, enable_feedback : %d ",
             user_parm_struct->algo_config.enable_feedback);
    }else
        LOG_ERROR("load adr_algo section failed");

    return 0;
}

//get element from data ready list to overwrite
gnss_nmea_stat_data *get_new_element_from_pool (adr_algo_device *dev)
{
    int32_t cnt = 0;
    while(cnt < ADR_ALGO_CACHE_NMEA_LEN) {
        if (!dev->in_Nmeas_stat[cnt]) {
            dev->in_Nmeas_stat[cnt] =
                (gnss_nmea_stat_data *)zalloc(sizeof(gnss_nmea_stat_data));
            module_list_init(&dev->in_Nmeas_stat[cnt]->link);
            dev->in_Nmeas_stat[cnt]->busy = 0;
            return dev->in_Nmeas_stat[cnt];
        } else {
            if (dev->in_Nmeas_stat[cnt]->busy == 0)
                 return dev->in_Nmeas_stat[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

//release a ele to pool
void release_element_to_pool (gnss_nmea_stat_data *ele)
{
    ele->busy = 0; 
    //memset(ele, 0, sizeof(gnss_nmea_stat_data));
}

//release a element to pool
void free_element_to_pool (adr_algo_device *dev)
{
    int32_t cnt = 0;
    while(cnt < ADR_ALGO_CACHE_NMEA_LEN) {
        if (dev->in_Nmeas_stat[cnt]) {
            free(dev->in_Nmeas_stat[cnt]);
            dev->in_Nmeas_stat[cnt] = NULL;
        }
        cnt += 1;
    }
}

//get element from data ready list to overwrite
gnss_nmea_stat_data *get_oldest_element (adr_algo_device *dev)
{
    gnss_nmea_stat_data *nmea_stat_data = NULL;
    pthread_mutex_lock(&dev->nmea_list_mutex);  
    if (module_list_empty(&dev->nmea_list)) {
        pthread_mutex_unlock(&dev->nmea_list_mutex);
        return NULL;
    }
    nmea_stat_data = module_container_of(dev->nmea_list.prev, nmea_stat_data, link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
    return nmea_stat_data;
}

//pickout element from data ready list to overwrite
gnss_nmea_stat_data *pickout_oldest_element (adr_algo_device *dev)
{
    gnss_nmea_stat_data *nmea_stat_data = NULL;
    pthread_mutex_lock(&dev->nmea_list_mutex);  
    if (module_list_empty(&dev->nmea_list)) {
        pthread_mutex_unlock(&dev->nmea_list_mutex);
        return NULL;
    }
    nmea_stat_data = module_container_of(dev->nmea_list.prev, nmea_stat_data, link);
    module_list_remove(&nmea_stat_data->link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
    module_list_init(&nmea_stat_data->link);
    return nmea_stat_data;
}

//get element from data ready list to overwrite
gnss_nmea_stat_data *get_newest_element (adr_algo_device *dev)
{
    gnss_nmea_stat_data *nmea_stat_data = NULL;
    pthread_mutex_lock(&dev->nmea_list_mutex);  
    if (module_list_empty(&dev->nmea_list)) {
        pthread_mutex_unlock(&dev->nmea_list_mutex);
        return NULL;
    }
    nmea_stat_data = module_container_of(dev->nmea_list.next, nmea_stat_data, link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
    return nmea_stat_data;
}

//pickout element from data ready list to overwrite
gnss_nmea_stat_data *pickout_newest_element (adr_algo_device *dev)
{
    gnss_nmea_stat_data *nmea_stat_data = NULL;
    pthread_mutex_lock(&dev->nmea_list_mutex);  
    if (module_list_empty(&dev->nmea_list)) {
        pthread_mutex_unlock(&dev->nmea_list_mutex);
        return NULL;
    }
    nmea_stat_data =
        module_container_of(dev->nmea_list.next, nmea_stat_data, link);
    module_list_remove(&nmea_stat_data->link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
    module_list_init(&nmea_stat_data->link);
    return nmea_stat_data;
}

void insert_element (adr_algo_device *dev, gnss_nmea_stat_data *ele)
{
    pthread_mutex_lock(&dev->nmea_list_mutex);
    module_list_insert(&dev->nmea_list, &ele->link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
}

void remove_element (adr_algo_device *dev, gnss_nmea_stat_data *ele)
{
    pthread_mutex_lock(&dev->nmea_list_mutex);
    module_list_remove(&ele->link);
    module_list_init(&ele->link);
    pthread_mutex_unlock(&dev->nmea_list_mutex);
}

static algo_module_stat_msg *get_next_algo_module_stat_msg(adr_algo_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ADR_ALGO_MODULE_STAT_MSG_LEN) {
        if (!dev->algo_stat_msg[cnt]) {
            dev->algo_stat_msg[cnt] =
                (algo_module_stat_msg *)zalloc(sizeof(algo_module_stat_msg));
            module_list_init(&dev->algo_stat_msg[cnt]->link);
            return dev->algo_stat_msg[cnt];
        } else {
            if (dev->algo_stat_msg[cnt]->busy == 0)
                 return dev->algo_stat_msg[cnt];
        }
        cnt += 1;
    }
    return NULL;
}

static void free_algo_module_stat_msg(adr_algo_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ADR_ALGO_MODULE_STAT_MSG_LEN) {
        if (dev->algo_stat_msg[cnt]) {
            free(dev->algo_stat_msg[cnt]);
            dev->algo_stat_msg[cnt] = NULL;
        }
        cnt += 1;
    }
}
gnss_nmea_stat_data *in_Nmeas[ADR_ALGO_CACHE_NMEA_LEN];

/*static gnss_nmea_stat_data *get_next_gnss_nmea_stat_data(adr_algo_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ADR_ALGO_CACHE_NMEA_LEN) {
        if (!dev->in_Nmeas_stat[cnt]) {
            dev->in_Nmeas_stat[cnt] =
                (gnss_nmea_stat_data *)zalloc(sizeof(gnss_nmea_stat_data));
            module_list_init(&dev->in_Nmeas_stat[cnt]->link);
            dev->in_Nmeas_stat[cnt]->busy = 0;
            return dev->in_Nmeas_stat[cnt];
        } else {
            if (dev->in_Nmeas_stat[cnt]->busy == 0)
                 return dev->in_Nmeas_stat[cnt];
        }
        cnt += 1;
    }
    return NULL;
}*/

static void free_gnss_nmea_stat_data(adr_algo_device *dev)
{
    int32_t cnt = 0;

    while(cnt < ADR_ALGO_CACHE_NMEA_LEN) {
        if (dev->in_Nmeas_stat[cnt]) {
            free(dev->in_Nmeas_stat[cnt]);
            dev->in_Nmeas_stat[cnt] = NULL;
        }
        cnt += 1;
    }
}

void adr_algo_padr_handler(adr_algo_device *dev, char* sentence, int len)
{
    LOG_DEBUG("receive adr cmd len:%d, sentence:%s", len, sentence);
    char *pmtk = NULL;
    if (len < 7)
        return;
    if (memcmp(sentence, "$PADR", 5))
        return;
    if (strncmp(sentence, control_cmd[1].adr_cmd, strlen(control_cmd[1].adr_cmd))==0 ||
        strncmp(sentence, control_cmd[2].adr_cmd, strlen(control_cmd[2].adr_cmd))==0 ||
        strncmp(sentence, control_cmd[3].adr_cmd, strlen(control_cmd[3].adr_cmd))==0 ||
        strncmp(sentence, control_cmd[4].adr_cmd, strlen(control_cmd[4].adr_cmd))==0) {
        //send to gnss & wait 
        pmtk = get_pmtk_from_padr(sentence, len);
        if (!pmtk) {
            LOG_ERROR("the cmd:%s not support", sentence);
            return;
        }
        dev->algo_cmd = zalloc(len);
        memcpy(dev->algo_cmd, sentence, len);
        LOG_DEBUG("the pmtk cmd:%s", pmtk);
        module_box *algo_pmtk_box = get_available_module_box(dev->mod);
        algo_pmtk_box->data = (void *)pmtk;
        algo_pmtk_box->count = len;
        algo_pmtk_box->type = CMD_ADR_PMTK;
        dev->upload(dev->mod, algo_pmtk_box);
        release_module_box(algo_pmtk_box);
    } else {
        adr_read_cmd(sentence, len);
    }
}

void adr_algo_pmtk_handler(adr_algo_device *dev, char* sentence, int len)
{
    LOG_DEBUG("receive gnss ack cmd len:%d, sentence:%s", len, sentence);
    if (len < 7)
        return;
    if (memcmp(sentence, "$PMTK010,001", 12))
        return;

    if (dev->algo_cmd) {
        LOG_DEBUG("begin to send adr cmd len:%d, ADR_CMD:%s", len, dev->algo_cmd);
        adr_read_cmd(dev->algo_cmd, strlen(dev->algo_cmd));
        free(dev->algo_cmd);
        dev->algo_cmd = NULL;
    }
}

void adr_PmtkRetFunc(char* pmtk_ack, int pmtk_len)
{
    adr_algo_device *dev = g_adr_algo_ctx;
    LOG_DEBUG("receive pmtk_msg is %s\n", pmtk_ack);

    module_box *algo_box = get_available_module_box(dev->mod);
    algo_box->data = (void *)pmtk_ack;
    algo_box->count = pmtk_len;
    algo_box->type = CMD_ALGO_ACK;
    dev->upload(dev->mod, algo_box);
    release_module_box(algo_box);
}

void adr_PvtRetFunc(adr_pvt_msg_struct *pvt_msg)
{
    algo_module_stat_msg *algo_stat_msg;
    adr_algo_device *dev = g_adr_algo_ctx;

    if (!pvt_msg)
        return;

    algo_stat_msg = get_next_algo_module_stat_msg(dev);
    if (!algo_stat_msg) {   //if data ready thread too slow, wait
        LOG_WARN("algo data buffer seems too small, discard adr output");
        return;
    }

    LOG_DEBUG("algo output time[%f %d]", pvt_msg->ts_sec, pvt_msg->ts_millisec);

    memcpy(&algo_stat_msg->pvtdata, pvt_msg, sizeof(adr_pvt_msg_struct));
    pthread_mutex_lock(&dev->pvt_list_mutex);
    module_list_insert(&dev->pvt_list, &algo_stat_msg->link);
    pthread_mutex_unlock(&dev->pvt_list_mutex);

    pthread_mutex_lock(&dev->dataready_mutex);
    algo_stat_msg->busy = 1;
    pthread_mutex_unlock(&dev->dataready_mutex);
    //dev->w_algo_stat_msg = NULL;
    pthread_cond_broadcast(&dev->dataready_cond);
}

static int adr_algo_stop_thread(adr_algo_device *dev)
{
    dev->dataready_thread_exit = 1;
    pthread_cond_broadcast(&dev->dataready_cond);
    pthread_join(dev->dataready_thread, NULL);

    LOG_DEBUG("adr algo thread exit~");

    return 0;
}

/*int32_t first_is_bigger(double ts1, int milisec1, double ts2, int milisec2)
{
    uint64_t in1 = (ts1 * 1000) + milisec1;
    uint64_t in2 = (ts2 * 1000) + milisec2;

    if(in1 > in2)
        return 1;
    else if(in1 == in2)
        return 0;
    else
        return -1;
}*/

int32_t first_is_bigger(int64_t in1, int64_t in2)
{
    if(in1 > in2)
        return 1;
    else if(in1 == in2)
        return 0;
    else
        return -1;
}


//return ms
int32_t time_subtract(double ts1, int milisec1, double ts2, int milisec2)
{
    uint64_t in1 = (ts1 * 1000) + milisec1;
    uint64_t in2 = (ts2 * 1000) + milisec2;

    return (in1 - in2);
}


/*gnss_nmea_stat_data* algo_pick_out_nmea(adr_algo_device *dev, double ts, int milisec)
{
    gnss_nmea_stat_data *n, *target;
    int32_t ret = 0;

    module_list_for_each_reverse(n, &dev->nmea_list, link) {
        if(first_is_bigger(ts, milisec, n->ts_sec, n->ts_millisec) >= 0)
            return n;
        else {
            module_list_remove(&n->link);
            n->busy = 0;
            LOG_DEBUG("remove nmea_time[s ms][%lf %d],"
                "algo_time[s ms][%lf %d]" , n->ts_sec, n->ts_millisec,
                ts, milisec);
        }
    }
    return NULL;
}*/

void *adr_algo_data_ready_thread(void *data)
{
    struct adr_algo_device *dev = (struct adr_algo_device *)data;
    adr_pvt_msg_struct *pvt_msg;
    gnss_nmea_stat_data *target, *cur, *tmp;
    algo_module_stat_msg *algo_stat_msg;
    char latlon[40] = {0};
    char speedbuf[16] = {0};
    char latHemi = ' ';
    char lonHemi = ' ';
    int32_t found = 0, ret = 0, cnt = 0;
    char buf_sentence[GNSS_NMEA_SENTENCE_SIZE] = {0};

    while(!dev->dataready_thread_exit)
    {
        pthread_mutex_lock(&dev->dataready_mutex);
        pthread_cond_wait(&dev->dataready_cond, &dev->dataready_mutex);
        LOG_DEBUG("receive data ready event to store & upload");
        pthread_mutex_unlock(&dev->dataready_mutex);
        cnt = 0;
        while(1) {
            pthread_mutex_lock(&dev->pvt_list_mutex);
            if (module_list_empty(&dev->pvt_list)) {
                pthread_mutex_unlock(&dev->pvt_list_mutex);
                break;
            }

            algo_stat_msg = module_container_of(dev->pvt_list.prev,algo_stat_msg, link);
            pthread_mutex_unlock(&dev->pvt_list_mutex);

            pvt_msg = (adr_pvt_msg_struct *)&algo_stat_msg->pvtdata;
            // [CRITICAL] !!!!!!!!!!!!!! Test code, remove after debugging !!!!!!!!!!!!!!
            //pvt_msg->lat = 4746.452000;
            //pvt_msg->lon = -12210.981000;
            //LOG_INFO("[DBG] Set the lat and lon to %0.5f, %0.5f\n", pvt_msg->lat, pvt_msg->lon);
            latHemi = (pvt_msg->lat > 0)?'N':'S';
            lonHemi = (pvt_msg->lon > 0)?'E':'W';
            double lat_wo_sign = (pvt_msg->lat >= 0) ? pvt_msg->lat : -pvt_msg->lat;
            double lon_wo_sign = (pvt_msg->lon >= 0) ? pvt_msg->lon : -pvt_msg->lon;
            snprintf(latlon,40, "%0.6f,%c,%0.6f,%c,", lat_wo_sign, latHemi, lon_wo_sign, lonHemi);
            snprintf(speedbuf,16, "%0.2f,%0.3f",pvt_msg->velocity, pvt_msg->heading);
            target = NULL;
            cur = NULL;
            found  = 0;
            pthread_mutex_lock(&dev->nmea_list_mutex);
            module_list_for_each_safe(cur, tmp, &dev->nmea_list, link) {
                /*ret = first_is_bigger(cur->ts_sec, cur->ts_millisec,
                    pvt_msg->ts_sec, pvt_msg->ts_millisec);*/
                ret = first_is_bigger(cur->monotonic_raw, pvt_msg->monotonic_raw);

                LOG_DEBUG("ret[%d] found[%d] find nmea sentence[s ms][%f %d] out[s ms][%f %d]",
                    ret, found, cur->ts_sec, cur->ts_millisec,
                    pvt_msg->ts_sec, pvt_msg->ts_millisec);
                if (ret <= 0){
                    if (!found){
                        found = 1;
                        target = cur;
                    } else {
                        LOG_DEBUG("remove element, len[%d]", module_list_length(&dev->nmea_list));
                        module_list_remove(&cur->link);
                        module_list_init(&cur->link);
                        cur->busy = 0;
                    }
                }
            }
            pthread_mutex_unlock(&dev->nmea_list_mutex);
            if (!target) {
                LOG_ERROR("not get matched nmea sentence,len[%d] oldest time[%f %d]",
                    module_list_length(&dev->nmea_list), cur->ts_sec, cur->ts_millisec);
                //abort();
                break;
            }
            memset(dev->out_Nmea.sentence, '\0', GNSS_NMEA_SENTENCE_SIZE);
            update_gga_rmc_position(target->nmea.sentence, dev->out_Nmea.sentence,
                (uint64_t)pvt_msg->ts_sec, pvt_msg->ts_millisec ,latlon,
                pvt_msg->fix_state, speedbuf, pvt_msg->alt);

            if(pvt_msg->fix_state > 1) {
                update_pmtk_position(dev->out_Nmea.sentence, buf_sentence,
                    pvt_msg->epx, pvt_msg->epy, pvt_msg->epv, pvt_msg->climb,
                    pvt_msg->epd, pvt_msg->eps, pvt_msg->epc);
                    strncpy(dev->out_Nmea.sentence, buf_sentence, GNSS_NMEA_SENTENCE_SIZE);
            }

            memset(buf_sentence, '\0', GNSS_NMEA_SENTENCE_SIZE);
            algo_to_adr2mnld_msg(dev->out_Nmea.sentence, buf_sentence);

            module_box *algo_box = get_available_module_box(dev->mod);

            algo_box->data = (void *)dev->out_Nmea.sentence;
            algo_box->count = strlen(dev->out_Nmea.sentence);
            algo_box->type = DATA_ADR_ALGO;
            dev->upload(dev->mod, algo_box);

            algo_box->data = (void *)buf_sentence;
            algo_box->count = strlen(buf_sentence);
            algo_box->type = DATA_ADR_ALGO_MNLD;
            dev->upload(dev->mod, algo_box);

            algo_box->data = (void *)pvt_msg;
            algo_box->count = sizeof(*pvt_msg);
            algo_box->type = DATA_ADR_ALGO_PVT;
            dev->upload(dev->mod, algo_box);

            release_module_box(algo_box);

            pthread_mutex_lock(&dev->pvt_list_mutex);
            module_list_remove(&algo_stat_msg->link);
            module_list_init(&algo_stat_msg->link);
            pthread_mutex_unlock(&dev->pvt_list_mutex);

            pthread_mutex_lock(&dev->dataready_mutex);
            algo_stat_msg->busy = 0;
            pthread_mutex_unlock(&dev->dataready_mutex);
        }
   }
    pthread_exit(0);
}

static int32_t adr_algo_driver_init(struct module *mod)
{
    adr_algo_device *dev =
        (adr_algo_device*)module_get_driver_data(mod);
    int ret = -1;

    module_list_init(&dev->nmea_list);
    module_list_init(&dev->pvt_list);
    dev->algo_pvt_cb = adr_PvtRetFunc;
    dev->algo_pmtk_cb = adr_PmtkRetFunc;

    ret = adr_algo_load_config_file(&dev->adr_algo_config);
    if (!ret) {
        ret = adr_set_param(&dev->adr_algo_config.user_parm_struct,
            dev->algo_pvt_cb, dev->algo_pmtk_cb);
        if (ret) {
            LOG_ERROR("Failed set parameter to adr algo");
        }
    }

    pthread_mutex_init(&dev->pvt_list_mutex, NULL);
    pthread_mutex_init(&dev->nmea_list_mutex, NULL);

    _adr_init_object();
    adr_set_log_level(log_algo_level);

    pthread_cond_init(&dev->dataready_cond, NULL);
    pthread_mutex_init(&dev->dataready_mutex, NULL);
    dev->dataready_thread_exit = 0;
    ret = pthread_create(&dev->dataready_thread, NULL, adr_algo_data_ready_thread,(void *)dev);
    if (ret != 0) {
        LOG_ERROR("can't create thread for algo data ready ,%s",strerror(errno));
        return -1;
    }

    return 0;
}

void algo_align_nmea_sentence(adr_algo_device *dev, module_box *box)
{
    gnss_nmea_stat_data *nmea_stat_data = NULL;

    nmea_stat_data = get_new_element_from_pool(dev);
    if (!nmea_stat_data){
        LOG_WARN("fail to get nmea stat data, overwrite oldest element");
        nmea_stat_data = pickout_oldest_element(dev);
    }
    if (!nmea_stat_data){
        LOG_ERROR("something wrong, fail get buffer to nmea");
        abort();
    }
    nmea_stat_data->nmea = *(gnss_nmea_data *)box->priv;
    nmea_stat_data->ts_sec =
        ((adr_gnss_msg_struct *)(box->data))->ts_sec;
    nmea_stat_data->ts_millisec =
        ((adr_gnss_msg_struct *)(box->data))->ts_millisec;
    nmea_stat_data->monotonic_raw =
        ((adr_gnss_msg_struct *)(box->data))->monotonic_raw;
    nmea_stat_data->busy = 1;
    insert_element(dev, nmea_stat_data);
    LOG_DEBUG("adr nmea sentence time[%f %d]", nmea_stat_data->ts_sec,nmea_stat_data->ts_millisec);
    adr_read_data((adr_gnss_msg_struct *)box->data, NULL, 0);
}

static void adr_algo_driver_download(struct module *mod, module_box *box)
{
    struct adr_algo_device *dev =
        (adr_algo_device*)module_get_driver_data(mod);

    LOG_DEBUG("Recieve data from:%s, data type id %d", box->from, box->type);

    switch(box->type) {
        case DATA_SENSOR_MEMS:
            adr_read_data(NULL, (adr_mems_msg_struct *)(box->data), (int)(box->count));
            break;
        case DATA_GNSS_PARSERD:
            algo_align_nmea_sentence(dev, box);
            break;
        case CMD_ADR_PADR:
            adr_algo_padr_handler(dev, (char*)(box->data),(int)(box->count));
            break;
        case CMD_GNSS_ACK:
            adr_algo_pmtk_handler(dev, (char*)(box->data),(int)(box->count));
            break;
        default :
            LOG_ERROR("can not process this type");
    }
}

static void adr_algo_driver_deinit(struct module *mod)
{
    struct adr_algo_device *dev =
        (adr_algo_device*)module_get_driver_data(mod);

    _adr_destory_object();//free adr so thread first
    adr_algo_stop_thread(dev);
    free_algo_module_stat_msg(dev);
    free_gnss_nmea_stat_data(dev);
    pthread_cond_destroy(&dev->dataready_cond);
    pthread_mutex_destroy(&dev->dataready_mutex);
    pthread_mutex_destroy(&dev->nmea_list_mutex);
    pthread_mutex_destroy(&dev->pvt_list_mutex);

    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(adr_algo_device));
    dev->mod = store_mod;
}

static void adr_algo_driver_registry(struct module *mod, process_data callback)
{
   struct adr_algo_device *dev;
   dev = (adr_algo_device*)module_get_driver_data(mod);

   dev->upload = callback;
}

static module_driver adr_algo_driver = {
    .name = "mtk_adr_algo",
    .capability = DATA_SENSOR_MEMS | DATA_GNSS_PARSERD | CMD_ADR_PADR | CMD_GNSS_ACK,
    .init = adr_algo_driver_init,
    .download = adr_algo_driver_download,
    .deinit = adr_algo_driver_deinit,
    .registry = adr_algo_driver_registry,
};

void module_adr_algo_deinit(module *mod)
{
    struct adr_algo_device *dev;
    dev = (adr_algo_device*)module_get_driver_data(mod);

    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_adr_algo_init(module *mod)
{
    struct adr_algo_device *dev;

    dev = (adr_algo_device*)zalloc(sizeof(adr_algo_device));
    if (!dev) {
        LOG_ERROR("Fail to create adr_algo_device");
        return -1;
    }
    g_adr_algo_ctx = dev;
    dev->mod = mod;
    module_set_name(mod, adr_algo_driver.name);
    module_register_driver(mod, &adr_algo_driver, (void *)dev);
    return 0;
}

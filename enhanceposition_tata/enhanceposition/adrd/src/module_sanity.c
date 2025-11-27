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
#include <time.h>
#include <string.h>

#include "module.h"
#include "adr_release_type.h"

typedef enum {
    GPS_TIME = 0,
    BARO_NOISE,
    MEMS_TIME,
    MEMS_NOISE,
    ADR_TIME,
    ADR_NOISE,
    SD_LOG
} SANITY_FAILED_TYPE;

typedef enum {
    FAIL = 0,
    PASS
} SANITY_RESULT;


typedef struct {
    int inited;
    int completed;
    time_t prev_s;              // previous sec
    double prev_ms;             // previous millisecond (s)
    time_t curr_s;              // previous sec
    double curr_ms;             // previous millisecond (s)
    double elapse_time;         // elapse time (s)

    // setting
    double interval;            // timestamp interval (s)
    double interval_tol;        // timestamp tolerance (s)
    double interval_tol2;       // timestamp tolerance (s)
    double interval_max_tol;    // sync event (s)
    double duration;            // st duration (s)

    // analysis info
    unsigned int backtrack;     // number of timestamp backtrack
    unsigned int defect;        // number of timestamp exceed tolerance
    unsigned int defect2;       // number of timestamp exceed tolerance
    unsigned int total;         // total number of timestamp
    double mean;
    double var;
} sanity_time_t;

typedef struct {
    double backtrack_p;
    double defect_p;
    double defect2_p;
    double total_p;
    double var;
} sanity_time_spec_t;

typedef struct {
    int completed;

    // setting
    double interval;            // timestamp interval (s)
    double duration;            // st duration (s)

    // analysis info
    unsigned int total;
    double mean;
    double var;
} sanity_noise_t;

typedef struct {
   double var;
} sanity_noise_spec_t;


typedef struct sanity_device {
    struct module *mod;
    void (*upload)(module *, module_box *);

    FILE *sanity_fp;

    int gps_is_fix;
} sanity_device;

time_t sanity_utc_to_time_t(double ts_sec)
{
    struct tm tmp_tm;
    memset(&tmp_tm, 0, sizeof(tmp_tm));
    char tmp_arr[20] = {0};
    char y[5] = {0};
    char m[3] = {0};
    char d[3] = {0};
    char hr[3] = {0};
    char min[3] = {0};
    char sec[3] = {0};

    sprintf(tmp_arr, "%lf", ts_sec);

    memcpy(y, tmp_arr, 4);
    memcpy(m, tmp_arr + 4, 2);
    memcpy(d, tmp_arr + 6, 2);
    memcpy(hr, tmp_arr + 8, 2);
    memcpy(min, tmp_arr + 10, 2);
    memcpy(sec, tmp_arr + 12, 2);

    y[4]   = '\0';
    m[2]   = '\0';
    d[2]   = '\0';
    hr[2]  = '\0';
    min[2] = '\0';
    sec[2] = '\0';

    sscanf(y, "%d", &(tmp_tm.tm_year));
    sscanf(m, "%d", &(tmp_tm.tm_mon));
    sscanf(d, "%d", &(tmp_tm.tm_mday));
    sscanf(hr, "%d", &(tmp_tm.tm_hour));
    sscanf(min, "%d", &(tmp_tm.tm_min));
    sscanf(sec, "%d", &(tmp_tm.tm_sec));
    tmp_tm.tm_year -= 1900;
    tmp_tm.tm_mon -= 1;

    return (mktime(&tmp_tm));
}

void sanity_write_result_log(sanity_device *dev, int category, int axis, int ispass, char *log)
{
    char *result[] = {"FAIL", "PASS"};
    switch(category) {
        case GPS_TIME:
            fprintf(dev->sanity_fp, "Sanity test(GNSS, time): %s\nLOG: %s\n\n", result[ispass], log);
            break;
        case BARO_NOISE:
            fprintf(dev->sanity_fp, "Sanity test(GNSS, noise, barometer): %s\nLOG: %s\n\n", result[ispass], log);
            break;
        case MEMS_TIME:
            fprintf(dev->sanity_fp, "Sanity test(MEMS, time): %s\nLOG: %s\n\n", result[ispass], log);
            break;
        case MEMS_NOISE:
            fprintf(dev->sanity_fp, "Sanity test(MEMS, noise, %d): %s\nLOG: %s\n\n", axis, result[ispass], log);
            break;
        case ADR_TIME:
            fprintf(dev->sanity_fp, "Sanity test(ADR, time): %s\nLOG: %s\n\n", result[ispass], log);
            break;
        case ADR_NOISE:
            fprintf(dev->sanity_fp, "Sanity test(ADR, noise, %d): %s\nLOG: %s\n\n", axis, result[ispass], log);
            break;
        case SD_LOG:
            fprintf(dev->sanity_fp, "Sanity test(SD log file is empty): %s\nLOG: %s\n\n", result[ispass], log);
            break;
        default:
            fprintf(dev->sanity_fp, "The result category is not defined\n");
            break;
    }
    fflush(dev->sanity_fp);
}

double sanity_time_diff(time_t start_sec, double start_ms, time_t end_sec, double end_ms)
{
    return (difftime(end_sec, start_sec) + end_ms - start_ms);
}

void sanity_update_mean_var(double input, unsigned int n, double *mean, double *var)
{
    if (n == 0) return;     // n > 0 update MEAN
    double prev_mean = *mean;
    *mean += (input - prev_mean) / n;

    if (n == 1) return;     // n > 1 update VAR
    *var = ((*var) * (n - 2) + (input - prev_mean) * (input - (*mean))) / (n - 1);
}

int sanity_timestamp_check(sanity_time_t *sanity_time, double ts_sec, int ts_millisec)
{
    LOG_ERROR("new utc time [%f s, %d ms]", ts_sec, ts_millisec);
    double time_diff;
    LOG_ERROR("sanity_time->inited %d", sanity_time->inited);

    if (!sanity_time->inited)
    {
        sanity_time->prev_s = sanity_utc_to_time_t(ts_sec);
        sanity_time->prev_ms = (ts_millisec / 1000.0);
        sanity_time->inited = 1;
        return 0;
    }
    sanity_time->curr_s = sanity_utc_to_time_t(ts_sec);
    sanity_time->curr_ms = (ts_millisec / 1000.0);
    time_diff = sanity_time_diff(sanity_time->prev_s, sanity_time->prev_ms, sanity_time->curr_s, sanity_time->curr_ms);
    LOG_ERROR("new time_t [%ld s, %f ms], last time_t [%ld s, %f ms], timediff [%f ms]",
               sanity_time->curr_s, sanity_time->curr_ms, sanity_time->prev_s, sanity_time->prev_ms, time_diff);

    if (time_diff >= 0)
    {
        if (time_diff < sanity_time->interval_max_tol)
        {
            if (time_diff > (sanity_time->interval + sanity_time->interval_tol) || time_diff < (sanity_time->interval - sanity_time->interval_tol))
            {
                sanity_time->defect++;
            }
            if (time_diff > (sanity_time->interval + sanity_time->interval_tol2) || time_diff < (sanity_time->interval - sanity_time->interval_tol2))
            {
                sanity_time->defect2++;
            }
            sanity_update_mean_var(time_diff, ++(sanity_time->total), &(sanity_time->mean), &(sanity_time->var));
            sanity_time->elapse_time += time_diff;

            if (sanity_time->elapse_time >= sanity_time->duration)
            {
                return 1;
            }
        }
    }
    else
    {
        sanity_time->backtrack++;
    }
    sanity_time->prev_s = sanity_time->curr_s;
    sanity_time->prev_ms = sanity_time->curr_ms;
    return 0;
}

int sanity_noise_check(sanity_noise_t *sanity_noise, double input)
{
    sanity_update_mean_var(input, ++(sanity_noise->total), &(sanity_noise->mean), &(sanity_noise->var));
    if (sanity_noise->total >= (sanity_noise->duration / sanity_noise->interval))
    {
        return 1;
    }
    return 0;
}

void sanity_align_adr_algo_pvt(sanity_device *dev, module_box *box)
{
    adr_pvt_msg_struct *pvt = (adr_pvt_msg_struct *)(box->data);

    static sanity_time_t sanity_time = {
        .inited = 0, .completed = 0, .prev_s = 0, .prev_ms = 0.0, .curr_s = 0, .curr_ms = 0.0, .elapse_time = 0.0, \
        .interval = 1, .interval_tol = 0.0101, .interval_tol2 = 0.0101, .interval_max_tol = 60.0, .duration = 300.0, \
        .backtrack = 0u, .defect = 0u, .defect2 = 0u, .total = 0u, .mean = 0.0, .var = 0.0
    };
    static sanity_time_spec_t sanity_time_spec = {
        // backtrack: not allow
        // defect: allow 1 data
        // total: allow 1 data
        // var: allow 1 data about maximum of 0.4 s difference
        .backtrack_p = 0.0, .defect_p = 0.5, .defect2_p = 0.5, .total_p = 0.5, .var = 0.005 * 0.005
    };
    static sanity_noise_t sanity_noise[3] = {
        { .completed = 0, .interval = 1, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 1, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 1, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 }
    };
    static sanity_noise_spec_t sanity_noise_spec[3] = {
        // lat, lon ('), alt (m)
        // 1m ~= 0.00000900900901 deg, 1m var ~= 2.92e-07' var ~= 0.0000003 'var
        { .var = 0.0000003 }, { .var = 0.0000003 }, { .var = 1.0 }
    };

    if (pvt != NULL && !sanity_time.completed && dev->gps_is_fix && pvt->fix_state > 0 && sanity_timestamp_check(&sanity_time, pvt->ts_sec, pvt->ts_millisec))
    {
        double backtrack_p = (sanity_time.backtrack * 100.0 / sanity_time.total);
        double defect_p = (sanity_time.defect * 100.0 / sanity_time.total);
        double total_p = (sanity_time.total * 100.0 / floor(sanity_time.elapse_time / sanity_time.interval));
        char log[500] = {0};
        sprintf(log, "Sanity test(ADR, time), mean(s): %f, var(s): %f, "
            "backtrack(%%): %f, defect(%%): %f, continuity(%%): %f\n",
            sanity_time.mean, sanity_time.var, backtrack_p, defect_p, total_p);
        if (backtrack_p <= sanity_time_spec.backtrack_p
            && defect_p <= sanity_time_spec.defect_p
            && fabs(total_p - 100.0) <= sanity_time_spec.total_p
            && sanity_time.var <= sanity_time_spec.var) 
        {
            sanity_write_result_log(dev, ADR_TIME, 0, PASS, log);
        }
        else
        {
            sanity_write_result_log(dev, ADR_TIME, 0, FAIL, log);
        }
        sanity_time.completed = 1;
    }
    if (pvt != NULL && pvt->fix_state > 0)
    {
        double tmp_pvt[3] = {pvt->lat, pvt->lon, pvt->alt};

        int i;
        for (i = 0; i < 3; i++)
        {
            if (!sanity_noise[i].completed && sanity_noise_check(&sanity_noise[i], tmp_pvt[i]))
            {
                char log[500] = {0};

                sprintf(log, "Sanity test(ADR, noise, %d), mean(deg minute, m): "
                    "%0.7f, var(deg minute, m): %0.7f\n", i,
                    sanity_noise[i].mean, sanity_noise[i].var);
                if (sanity_noise[i].var <= sanity_noise_spec[i].var)
                {
                    sanity_write_result_log(dev, ADR_NOISE, i, PASS, log);
                }
                else
                {
                    sanity_write_result_log(dev, ADR_NOISE, i, FAIL, log);
                }
                sanity_noise[i].completed = 1;
            }
        }
    }
}

void sanity_align_mems(sanity_device *dev, module_box *box)
{
    adr_mems_msg_struct* mems = (adr_mems_msg_struct *)(box->data);
    int count = (int)(box->count);
    static sanity_time_t sanity_time = {
        .inited = 0, .completed = 0, .prev_s = 0, .prev_ms = 0.0, .curr_s = 0, .curr_ms = 0.0, .elapse_time = 0.0, \
        .interval = 0.02, .interval_tol = 0.0021, .interval_tol2 = 0.0101, .interval_max_tol = 60.0, .duration = 300.0, \
        .backtrack = 0u, .defect = 0u, .defect2 = 0u, .total = 0u, .mean = 0.0, .var = 0.0
    };
    static sanity_time_spec_t sanity_time_spec = {
        // backtrack: not allow
        // defect: allow 1%
        // defect2: allow 0.01%
        // total: allow 1 data
        // var: allow 1 data about maximum of 0.4 s difference
        .backtrack_p = 0.0, .defect_p = 1, .defect2_p = 0.01, .total_p = 0.01, .var = 0.003 * 0.003
    };
    static sanity_noise_t sanity_noise[6] = {
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 },
        { .completed = 0, .interval = 0.02, .duration = 60.0, .total = 0u, .mean = 0.0, .var = 0.0 }
    };
    static sanity_noise_spec_t sanity_noise_spec[6] = {
        // Accelerometer (m/s^2), Gyroscope (rad/s)
        { .var = 0.02 * 0.02 }, { .var = 0.02 * 0.02 }, { .var = 0.04 * 0.04 },
        { .var = 0.003 * 0.003 }, { .var = 0.003 * 0.003 }, { .var = 0.003 * 0.003 }
    };

    int i;
    for (i = 0; i < count; i++)
    {
        if ((mems + i) != NULL && !sanity_time.completed && dev->gps_is_fix && sanity_timestamp_check(&sanity_time, (mems + i)->ts_sec, (mems + i)->ts_millisec))
        {
            double backtrack_p = (sanity_time.backtrack * 100.0 / sanity_time.total);
            double defect_p = (sanity_time.defect * 100.0 / sanity_time.total);
            double defect2_p = (sanity_time.defect2 * 100.0 / sanity_time.total);
            double total_p = (sanity_time.total * 100.0 / floor(sanity_time.elapse_time / sanity_time.interval));
            char log[500] = {0};

            sprintf(log, "Sanity test(MEMS, time), mean(s): %f, var(s): %f, "
                "backtrack(%%): %f, defect(%%): %f, defect2(%%): %f, "
                "continuity(%%): %f\n", sanity_time.mean, sanity_time.var,\
                backtrack_p, defect_p, defect2_p, total_p);
            if (backtrack_p <= sanity_time_spec.backtrack_p
                && defect_p <= sanity_time_spec.defect_p
                && defect2_p <= sanity_time_spec.defect2_p
                && fabs(total_p - 100.0) <= sanity_time_spec.total_p
                && sanity_time.var <= sanity_time_spec.var)
            {
                sanity_write_result_log(dev, MEMS_TIME, 0, PASS, log);
            }
            else
            {
                sanity_write_result_log(dev, MEMS_TIME, 0, FAIL, log);
            }
            sanity_time.completed = 1;
        }
        if ((mems + i) != NULL)
        {
            adr_sensor_struct *acc_ptr = &((mems + i)->acc);
            adr_sensor_struct *gyro_ptr = &((mems + i)->gyro);
            double tmp_mems[6] = {acc_ptr->x, acc_ptr->y, acc_ptr->z, gyro_ptr->x, gyro_ptr->y, gyro_ptr->z};

            int j;
            for (j = 0; j < 6; j++)
            {
                if (!sanity_noise[j].completed && sanity_noise_check(&sanity_noise[j], tmp_mems[j]))
                {
                    char log[500] = {0};

                    sprintf(log, "Sanity test(MEMS, noise, %d), mean(m/s, rad/s)"
                        ": %f, var(m/s, rad/s): %f\n", j, sanity_noise[j].mean,
                        sanity_noise[j].var);
                    if (sanity_noise[j].var <= sanity_noise_spec[j].var)
                    {
                        sanity_write_result_log(dev, MEMS_NOISE, j, PASS, log);
                    }
                    else
                    {
                        sanity_write_result_log(dev, MEMS_NOISE, j, FAIL, log);
                    }
                    sanity_noise[j].completed = 1;
                }
            }
        }
    }
}

void sanity_align_nmea_sentence(sanity_device *dev, module_box *box)
{
    adr_gnss_msg_struct *gnss = (adr_gnss_msg_struct *)(box->data);
    if(!gnss){
        LOG_ERROR("receive nmea sentence is null");
        abort();
    }
    static sanity_time_t sanity_time = {
        .inited = 0, .completed = 0, .prev_s = 0, .prev_ms = 0.0, .curr_s = 0, .curr_ms = 0.0, .elapse_time = 0.0, \
        .interval = 0.2, .interval_tol = 0.0101, .interval_tol2 = 0.0101, .interval_max_tol = 60.0, .duration = 300.0, \
        .backtrack = 0u, .defect = 0u, .defect2 = 0u, .total = 0u, .mean = 0.0, .var = 0.0
    };
    static sanity_time_spec_t sanity_time_spec = {
        // backtrack: not allow
        // defect: allow 1 data
        // total: allow 1 data
        // var: allow 1 data about maximum of 0.4 s difference
        .backtrack_p = 0.0, .defect_p = 0.1, .defect2_p = 0.1, .total_p = 0.1, .var = 0.005 * 0.005
    };
    static sanity_noise_t sanity_noise = {
        .completed = 0, .interval = 0.2, .duration = 30.0, .total = 0u, .mean = 0.0, .var = 0.0
    };
    static sanity_noise_spec_t sanity_noise_spec = {
        // Pa (under 50 cm precision)
        .var = 6.25 * 6.25
    };
    if (!dev->gps_is_fix && gnss->fix_mode == 3 && gnss->ts_millisec % 200 == 0){
        dev->gps_is_fix = 1;
    }

    if (!sanity_time.completed && dev->gps_is_fix && sanity_timestamp_check(&sanity_time, gnss->ts_sec, gnss->ts_millisec))
    {
        double backtrack_p = (sanity_time.backtrack * 100.0 / sanity_time.total);
        double defect_p = (sanity_time.defect * 100.0 / sanity_time.total);
        double total_p = (sanity_time.total * 100.0 / floor(sanity_time.elapse_time / sanity_time.interval));
        char log[500] = {0};

        sprintf(log, "Sanity test(GNSS, time), mean(s): %f, var(s): %f, "
            "backtrack(%%): %f, defect(%%): %f, continuity(%%): %f\n",
            sanity_time.mean, sanity_time.var, backtrack_p, defect_p, total_p);

        if (backtrack_p <= sanity_time_spec.backtrack_p && defect_p <= sanity_time_spec.defect_p && fabs(total_p - 100.0) <= sanity_time_spec.total_p && sanity_time.var <= sanity_time_spec.var) 
        {
            sanity_write_result_log(dev, GPS_TIME, 0, PASS, log);
        }
        else
        {
            sanity_write_result_log(dev, GPS_TIME, 0, FAIL, log);
        }
        sanity_time.completed = 1;
    }
    if (!sanity_noise.completed && sanity_noise_check(&sanity_noise, gnss->baro))
    {
        char log[500] = {0};

        sprintf(log, "Sanity test(GNSS, noise, barometer), mean(pa): %f, "
            "var(pa): %f\n", sanity_noise.mean, sanity_noise.var);
        if (sanity_noise.var <= sanity_noise_spec.var)
        {
            sanity_write_result_log(dev, BARO_NOISE, 0, PASS, log);

        }
        else
        {
            sanity_write_result_log(dev, BARO_NOISE, 0, FAIL, log);
        }
        sanity_noise.completed = 1;
    }
}

static void sanity_driver_download(struct module *mod, module_box *box)
{
    struct sanity_device *dev;
    dev = (sanity_device*)module_get_driver_data(mod);

    LOG_DEBUG("Recieve data from:%s, data type id %d", box->from, box->type);

    switch(box->type) {
        case DATA_GNSS_PARSERD:
            sanity_align_nmea_sentence(dev, box);
            break;
        case DATA_SENSOR_MEMS:
            sanity_align_mems(dev, box);
            break;
        case DATA_ADR_ALGO_PVT:
            sanity_align_adr_algo_pvt(dev, box);
            break;
        default :
                LOG_ERROR("can not process this type");
    }
}

static int32_t sanity_driver_init(struct module *mod)
{
    int32_t ret = -1;
    struct sanity_device *dev =
        (sanity_device*)module_get_driver_data(mod);

    if(open_log_file("SANITY.LOG", &dev->sanity_fp) != 0) {
        LOG_ERROR("open log file: SANITY.LOG failed");
        return -1;
    }
    return ret;
}

static void sanity_driver_deinit(struct module *mod)
{
    struct sanity_device *dev;
    dev = (sanity_device*)module_get_driver_data(mod);
    close_log_file(dev->sanity_fp);
    module *store_mod = dev->mod;
    memset(dev, 0, sizeof(sanity_device));
    dev->mod = store_mod;
}

static void sanity_driver_registry(struct module *mod, process_data callback)
{
    struct sanity_device *dev;
    dev = (sanity_device*)module_get_driver_data(mod);

    dev->upload = callback;
}

static module_driver sanity_driver = {
    .name = "mtk_sanity",
    .capability = DATA_GNSS_PARSERD | DATA_SENSOR_MEMS |DATA_ADR_ALGO_PVT,
    .init = sanity_driver_init,
    .download = sanity_driver_download,
    .deinit = sanity_driver_deinit,
    .registry = sanity_driver_registry,
};

void module_sanity_deinit(module *mod)
{
    struct sanity_device *dev;
    dev = (sanity_device*)module_get_driver_data(mod);

    free(dev);
    module_register_driver(mod, NULL, NULL);
}

int module_sanity_init(module *mod)
{
    struct sanity_device *dev;

    dev = (sanity_device*)zalloc(sizeof(sanity_device));
    if (!dev) {
        LOG_ERROR("Fail to create sanity_device");
        return -1;
    }
    dev->mod = mod;
    module_set_name(mod, sanity_driver.name);
    module_register_driver(mod, &sanity_driver, (void *)dev);
    return 0;
}

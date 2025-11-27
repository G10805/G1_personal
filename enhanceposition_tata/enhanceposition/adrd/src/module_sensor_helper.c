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
#include <math.h>

#include "module_sensor_helper.h"
#include "log.h"

extern double baro_d_last;
extern double baro_t_last;
extern double acc_t_last;
extern double gyro_t_last;

int32_t checkAcc(double x, double y, double z, double t, int64_t timestamp)
{
    int err_trace = 0;

    if (x == 0 && y == 0 && z == 0)
        err_trace += 1;
    else if (fabs(x) > 19.6133 || fabs(y) > 19.6133 || fabs(z) > 19.6133)
        err_trace += 2;

    if (t > 85 || t < -40 || t == 0)
        err_trace += 4;
    if (fabs(t - acc_t_last) > 2)
        err_trace += 8;

    if (err_trace) {
        if (acc_t_last == 1024) {
            LOG_INFO("dump acc first data information");
            LOG_INFO("acc x,y,z,t,lt= %lf, %lf, %lf, %lf, %lf", x, y, z, t, acc_t_last);
            LOG_INFO("acc timestamp= %"PRId64,timestamp);
        } else {
            LOG_ERROR("dump acc abnormal(0x%02x) information!!!", err_trace);
            LOG_ERROR("acc x,y,z,t,lt= %lf, %lf, %lf, %lf, %lf", x, y, z, t, acc_t_last);
            LOG_ERROR("acc timestamp= %"PRId64,timestamp);
        }

    }

    acc_t_last = t;
    return 1;
}

int32_t checkGyro(double x, double y, double z, double t, int64_t timestamp)
{
    int err_trace = 0;

    if (x == 0 && y == 0 && z == 0)
        err_trace += 16;
    else if (fabs(x) > 2.181662 || fabs(y) > 2.181662 || fabs(z) > 2.181662)
        err_trace += 32;

    if (t > 85 || t < -40 || t == 0)
        err_trace += 64;
    if (fabs(t - gyro_t_last) > 2)
        err_trace += 128;

    if (err_trace) {
        if (gyro_t_last == 1024) {
            LOG_INFO("dump gyro first data information");
            LOG_INFO("gyro x,y,z,t,lt= %lf, %lf, %lf, %lf, %lf", x, y, z, t, gyro_t_last);
            LOG_INFO("gyro timestamp= %"PRId64,timestamp);

        } else {
            LOG_ERROR("dump gyro abnormal(0x%02x) information!!!", err_trace);
            LOG_ERROR("gyro x,y,z,t,lt= %lf, %lf, %lf, %lf, %lf", x, y, z, t, gyro_t_last);
            LOG_ERROR("gyro timestamp= %"PRId64,timestamp);
        }

    }

    gyro_t_last = t;
    return 1;
}

int32_t checkBaro(double data, double temperature, int64_t timestamp)
{
    int err_trace = 0;

    if (data > 110000 || data < 30000)
        err_trace += 1;
    if (temperature > 85 || temperature < -40 || temperature == 0)
        err_trace += 4;

    if (fabs(data - baro_d_last) > 5000)
        err_trace += 2;
    if (fabs(temperature - baro_t_last) > 20000)
        err_trace += 8;

    if (err_trace) {
        if (baro_t_last == 1024) {
            LOG_INFO("dump baro first data information");
            LOG_INFO("baro data, t= %lf, %lf", data, temperature);
            LOG_INFO("baro datal, tl= %lf, %lf", baro_d_last, baro_t_last);
            LOG_INFO("baro timestamp= %"PRId64, timestamp);
        } else {
            LOG_ERROR("dump baro abnormal(0x%02x) information!!!", err_trace);
            LOG_ERROR("baro data, t= %lf, %lf", data, temperature);
            LOG_ERROR("baro datal, tl= %lf, %lf", baro_d_last, baro_t_last);
            LOG_ERROR("baro timestamp= %"PRId64, timestamp);
        }
    }

    baro_d_last = data;
    baro_t_last = temperature;
    return 1;
}


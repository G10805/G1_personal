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

#include "adr_data_convert.h"
#include "module_gnss_helper.h"
#include "module_odom_helper.h"
#include "module_sensor_helper.h"
#include "adr_util.h"

void odom_to_carInfo_msg(void *in, void *out)
{
    odom_module_data *src = (odom_module_data *)in;
    odom_uart_data *dst = (odom_uart_data *)out;

    dst->odom = src->odom;
    dst->rudder = src->rudder;
    dst->gear = src->gear;
    dst->odom_boottime = src->timestamp;
}

void sensor_to_baro_msg(void *in, void *out)
{
    sensor_module_data *src = (sensor_module_data *)in;
    sensor_iic_data *dst = (sensor_iic_data *)out;

    dst->baro = src->baro;
    dst->baroT = src->baroT;
    dst->baro_boottime = src->sensor_boottime;
}

void sensor_to_adr_msg(void *in, void *out)
{
    sensor_module_data *src = (sensor_module_data *)in;
    adr_mems_msg_struct *dst = (adr_mems_msg_struct *)out;

    dst->ts_sec = src->ts;
    dst->ts_millisec = src->ts_ms;
    dst->acc.x = src->acc_x;
    dst->acc.y = src->acc_y;
    dst->acc.z = src->acc_z;
    dst->acc.t = src->acc_T;
    dst->gyro.x = src->gyro_x;
    dst->gyro.y = src->gyro_y;
    dst->gyro.z = src->gyro_z;
    dst->gyro.t = src->gyro_T;
    dst->can1 = src->odom;
    dst->can2 = src->gear;
    dst->can3 = src->rudder;
}


void gnss_to_adr_msg(void *in, void *out)
{
    gnss_module_data *src = (gnss_module_data *)in;
    adr_gnss_msg_struct *dst = (adr_gnss_msg_struct *)out;

    dst->ts_sec = (int64_t)(src->nmearmc.utc_data_time);
    dst->ts_millisec = src->nmearmc.utc_time_msec;
    dst->lon = src->nmeagga.lng;
    dst->lat = src->nmeagga.lat;
    dst->alt = src->nmeagga.alt;
    dst->velocity.n = src->pmtkvned.velocity_n;
    dst->velocity.e = src->pmtkvned.velocity_e;
    dst->velocity.d = src->pmtkvned.velocity_d;
    dst->heading = src->nmearmc.bearing;
    dst->h_accuracy = src->nmeaepe.epe_2d;
    dst->v_accuracy = src->nmeaepe.epe_vert;
    dst->vel_accuracy.n = src->pmtkvned.vn_acc;
    dst->vel_accuracy.e = src->pmtkvned.ve_acc;
    dst->vel_accuracy.d = src->pmtkvned.vd_acc;
    dst->heading_accuracy = src->pmtkvned.heading_acc;
    dst->fix_mode = src->nmeagsa.fix_mode;
    dst->fix_nsv = src->nmeagsa.sate_num;
    dst->pdop= src->nmeagsa.pdop;
    dst->hdop = src->nmeagsa.hdop;
    dst->vdop = src->nmeagsa.vdop;
    dst->baro = src->sensor.baro;
    dst->baro_t= src->sensor.baroT;
    dst->odom = src->sensor.odo;
    dst->nsv = src->sv_out.nsv;
    dst->nsv_ptr = (adr_nsv_struct *)(src->sv_out.satedatas);
    dst->monotonic_raw = (int64_t)src->drvtime.tv_sec * nsec_2_sec + src->drvtime.tv_nsec;
}


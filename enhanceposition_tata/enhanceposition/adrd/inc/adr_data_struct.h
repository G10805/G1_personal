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
#ifndef ADR_DATA_STRUCT_H
#define ADR_DATA_STRUCT_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#if defined(__ANDROID_OS__)
#include <hardware/gps.h>
#elif defined(__LINUX_OS__)
#include "gps.h"
#endif

#include "adr_release_type.h"

#define GNSS_NMEA_SENTENCE_SIZE 10*1024

typedef struct gnss_nmea_data {
    char sentence[GNSS_NMEA_SENTENCE_SIZE];
    int32_t len;
} gnss_nmea_data;

typedef struct odom_uart_data {
    float odom;
    struct timespec vehicle_speed_boottime;

    float l_front_speed;
    float r_front_speed;
    float l_rear_speed;
    float r_rear_speed;
    struct timespec wheel_speed_boottime;

    float rudder;
    struct timespec angle_boottime;

    int gear;
    struct timespec gear_boottime;

    struct timespec odom_boottime;
} odom_uart_data;

/*
typedef struct {
    float vehicle_speed;
    struct timespec vehicle_speed_boottime;

    float l_front_speed;
    float r_front_speed;
    float l_rear_speed;
    float r_rear_speed;
    struct timespec wheel_speed_boottime;

    float angle;
    struct timespec angle_boottime;

    int gear;
    struct timespec gear_boottime;
}VEHICLE_INFO;*/

typedef struct sensor_iic_data {
    double baro;
    double baroT;
    struct timespec baro_boottime;
} sensor_iic_data;

#endif //ADR_DATA_STRUCT_H

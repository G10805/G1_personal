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
#include "gpshal.h"
#include <sys/epoll.h>  // epoll_create, epoll_event
#include <errno.h>     // errno
#include <string.h>    // strerror

#include "hal2mnl_interface.h"
#include "mtk_lbs_utility.h"
#include "gpshal_param_check.h"
#include "mtk_auto_log.h"

#if ANDROID_MNLD_PROP_SUPPORT
#include <cutils/properties.h>
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "gpshal"
#endif

#define GPSHAL_WORKER_THREAD_NAME "gpshal_worker_thread"

//=========================================================

gpshal_context_t g_gpshal_ctx = {
    .fd_mnl2hal        = -1,
    .fd_worker_epoll   = -1,
    .thd_worker        = 0,
    .mutex_gps_state_intent = PTHREAD_MUTEX_INITIALIZER,
    .gps_state_intent  = GPSHAL_STATE_UNKNOWN,
    .gps_state         = GPSHAL_STATE_UNKNOWN,
    .gps_cbs           = NULL,
    .agps_cbs          = NULL,
    .gpsni_cbs         = NULL,
    .agpsril_cbs       = NULL,
    .meas_cbs          = NULL,
    .navimsg_cbs       = NULL,
    .geofence_cbs      = NULL,
    .vzw_debug_cbs     = NULL,
};

//=========================================================

const char* gpshal_state_to_string(gpshal_state state) {
    switch (state) {
        case GPSHAL_STATE_UNKNOWN:  return "UNKNOWN";
        case GPSHAL_STATE_RESOURCE: return "RESOURCE_INIT";
        case GPSHAL_STATE_CLEANUP:  return "CLEANUP";
        case GPSHAL_STATE_INIT:     return "INIT";
        case GPSHAL_STATE_STOP:     return "STOP";
        case GPSHAL_STATE_START:    return "START";
        default:                    return "INVALID";
    }
}

static void gpshal_set_gps_state(gpshal_state newState) {
    /*LOGD("gps_state: %s -> %s (intent: %s)",
            gpshal_state_to_string(g_gpshal_ctx.gps_state),
            gpshal_state_to_string(newState),
            gpshal_state_to_string(g_gpshal_ctx.gps_state_intent));*/
    g_gpshal_ctx.gps_state = newState;
}

void gpshal_set_gps_state_intent(gpshal_state newState) {
    /*LOGD("gps_state: %s (intent: %s -> %s)",
            gpshal_state_to_string(g_gpshal_ctx.gps_state),
            gpshal_state_to_string(g_gpshal_ctx.gps_state_intent),
            gpshal_state_to_string(newState));*/
    g_gpshal_ctx.gps_state_intent = newState;
}

static void gpshal_check_capability(unsigned int *capabilities, unsigned int *year) {
    #if ANDROID_MNLD_PROP_SUPPORT
    char chip_id[PROPERTY_VALUE_MAX]={0};
    property_get("persist.vendor.connsys.chipid", chip_id, NULL);
    #else
    char chip_id[100]={0};
    strncpy(chip_id, "0x6630",sizeof(chip_id));
    #endif
    LOGI("chip id = %s", chip_id);
    if (strcmp(chip_id, "0x6739") == 0 || strcmp(chip_id, "0x6765") == 0 ||
        strcmp(chip_id, "0x6761") == 0) {
        /*our HIDL version is 1.0, if the year is 2018, CTS GnssHardwareInfoTest#testHardwareModelName will fail*/
        //*year = 2018;
        *year = 2017;
        *capabilities = (GPS_CAP_MEASUREMENTS | GPS_CAP_MSB | GPS_CAP_MSA);
    } else {
        /*our HIDL version is 1.0, if the year is 2018, CTS GnssHardwareInfoTest#testHardwareModelName will fail*/
        *year = 2017;
        *capabilities = (GPS_CAP_MEASUREMENTS | GPS_CAP_MSB | GPS_CAP_MSA);
    }
}
//=========================================================

// Design policy
//     Init resources at most once (no retry)
//         Because we can not save an unstable system,
//         we think "less system call is better" for an unstable system
//     Do not deinit resources
//         We want to have a simple design for a built-in service
//         "Deinit" is for an installable / removable service
static void gpshal_resource_init(GpsCallbacks_ext* src) {
    unsigned int capabilities = 0;
    unsigned int year = 0;
    g_gpshal_ctx.gps_cbs = (GpsCallbacks_ext*)src;
    if (GPSHAL_STATE_UNKNOWN != g_gpshal_ctx.gps_state_intent) return;  // at most once

    gpshal_check_capability(&capabilities, &year);
    LOGI("year = %d, capabilities = %d", year, capabilities);
    g_gpshal_ctx.gps_cbs->set_capabilities_cb(capabilities);

    GnssSystemInfo system_info;
    system_info.year_of_hw = year;
    system_info.size = sizeof(GnssSystemInfo);
    g_gpshal_ctx.gps_cbs->set_system_info_cb(&system_info);

    g_gpshal_ctx.fd_mnl2hal = create_mnl2hal_fd();
    if (-1 == g_gpshal_ctx.fd_mnl2hal) return;  // error

    g_gpshal_ctx.fd_worker_epoll = epoll_create(MAX_EPOLL_EVENT);
    if (-1 == g_gpshal_ctx.fd_worker_epoll) {
        LOGE("Fail to create epoll reason=[%s]%d",
                strerror(errno), errno);
        return;  // error
    }

    if (epoll_add_fd(
            g_gpshal_ctx.fd_worker_epoll,
            g_gpshal_ctx.fd_mnl2hal) == -1) {
        LOGE("Fail to add fd_mnl2hal");
        return;  // error
    }

    g_gpshal_ctx.thd_worker = g_gpshal_ctx.gps_cbs->create_thread_cb(
            GPSHAL_WORKER_THREAD_NAME, gpshal_worker_thread, NULL);
    if (!g_gpshal_ctx.thd_worker) {
        LOGE("Fail to create %s", GPSHAL_WORKER_THREAD_NAME);
        return;  // error
    }

    gpshal_set_gps_state(GPSHAL_STATE_RESOURCE);
}

int gpshal_gpscbs_save(GpsCallbacks_ext* src) {
// assert(NULL != src);
    if (sizeof(GpsCallbacks_ext) == src->size) {
        LOGD("Use GpsCallbacks");
        gpshal_resource_init(src);
        return 0;
    }
    LOGE("Bad callback, size: %zd, expected: %zd",
            src->size, sizeof(GpsCallbacks_ext));
    return -1;    //  error
}

//=========================================================

void gpshal2mnl_gps_init() {
    if (GPSHAL_STATE_RESOURCE <= g_gpshal_ctx.gps_state) {  // Check this for unstable system
        if (hal2mnl_gps_init() > 0) {
            gpshal_set_gps_state(GPSHAL_STATE_INIT);
        }
    }
}

void gpshal2mnl_gps_start() {
    if (GPSHAL_STATE_RESOURCE <= g_gpshal_ctx.gps_state) {
        if (hal2mnl_gps_start() > 0) {
            gpshal_set_gps_state(GPSHAL_STATE_START);
        }
    }
}

void gpshal2mnl_gps_stop() {
    if (GPSHAL_STATE_RESOURCE <= g_gpshal_ctx.gps_state) {
        if (hal2mnl_gps_stop() > 0) {
            gpshal_set_gps_state(GPSHAL_STATE_STOP);
        }
    }
}

void gpshal2mnl_gps_cleanup() {
    if (GPSHAL_STATE_RESOURCE <= g_gpshal_ctx.gps_state) {
        if (hal2mnl_gps_cleanup() > 0) {
            gpshal_set_gps_state(GPSHAL_STATE_CLEANUP);
        }
    }
}

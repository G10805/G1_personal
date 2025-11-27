/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/**
 *  @file adr_release_api.h
 *  @brief Defined several API between framework and library
 *
 *  For ADR API usage, please include "adr_release_api.h" and "adr_release_type.h"
 *  Used pthread_mutex_t to protect shared data structures from concurrent modifications.
 *
 *  @author Li-Min Lin (mtk07825) <li-min.lin@mediatek.com>
 */
#ifndef INCLUDE_ADR_RELEASE_API_H_
#define INCLUDE_ADR_RELEASE_API_H_

#include "./adr_release_type.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef void (*PmtkRetFunc)(char*, int);
typedef void (*PvtRetFunc)(adr_pvt_msg_struct *);

/**
 *  Set the user defined parameters from the configuration file
 *
 *  @param user_param_ptr User parameters
 *  @param pvt_ret_func The callback function to send the result
 *  @param pmtk_ret_func The callback function to send the response
 *  @return Success(0) or failed(-1)
 */
int adr_set_param(adr_user_param_struct* user_param_ptr, PvtRetFunc pvt_ret_func, PmtkRetFunc pmtk_ret_func);

/**
 *  Initial ADR object
 *  [Important] please call this function after function adr_set_param
 */
void _adr_init_object();

/**
 *  Destroy ADR object
 *  [Important] please call this function before deinit schedule
 */
void _adr_destory_object();

/**
 *  Set the log level: L_VERBOSE = 0, L_DEBUG, L_INFO, L_WARN, L_ERROR, L_ASSERT, L_SUPPRESS
 *  @param level Log level
 *  @return Success(0) or failed(-1)
 */
int adr_set_log_level(int level);

/**
 *  Input the PowerGPS command and send the response through the callback function
 *
 *  @param cmd PowerGPS command (e.g. PADRxxx)
 *  @param cmd_len The length of PowerGPS command
 *  @return Success(0) or failed(-1)
 */
int adr_read_cmd(char* cmd, int cmd_len);

/**
 *  Input the required GNSS/MEMS data and send the result through the callback function
 *
 *  @param gnss_ptr One GNSS information
 *  @param mems_ptr Several MEMS information
 *  @param mems_count Number of MEMS
 *  @return Success(0) or failed(-1)
 */
int adr_read_data(adr_gnss_msg_struct* gnss_ptr, adr_mems_msg_struct* mems_ptr, int mems_count);

/**
 *  Update the ADR to MNL message
 *
 *  @param adr2mnl_ptr ADR to MNL message
 *  @return Success(0) or failed(-1)
 */
int adr_get_adr2mnl_msg(adr2mnl_msg_struct *adr2mnl_ptr);

#ifdef __cplusplus
    }
#endif

#endif  // INCLUDE_ADR_RELEASE_API_H_

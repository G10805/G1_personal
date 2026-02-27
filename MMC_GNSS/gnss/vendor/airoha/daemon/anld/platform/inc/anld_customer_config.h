/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#ifndef ANLD_CUSTOMER_CONFIG_H
#define ANLD_CUSTOMER_CONFIG_H
#define ANLD_CUSTOMER_CONFIG_INCLUDED
#define AG3335_UART_DEV "/dev/ttyUSB0"
#define AG3335_SPI_DEV "/dev/spidev1.0"
#ifdef __ANDROID_OS__
#define ANLD_CONFIG_XML_PATH "/vendor/etc/gnss/anld_user_setting.xml"
#define ANLD_FIRMWARE_VERSION_FILE "/vendor/etc/gnss/firmware.txt"
#elif defined AIROHA_SIMULATION
#define ANLD_CONFIG_XML_PATH "daemon/anld/prebuilt/internal/anld_user_setting.xml"  // the same dir with anld
#define ANLD_FIRMWARE_VERSION_FILE "daemon/anld/gnss_demo/firmware.txt"
#endif
/**
 * @brief if need to open uart when power on and close when power off
 *
 * 0: Uart will be open before calling POWER ON, will be close before calling
 * POWER OFF 1: Uart will be open when
 *
 */
#define OPEN_UART_BEFORE_POWER_ON 1
#define OPEN_UART_BEFORE_OPEN_DSP 2
#define OPEN_UART_TIMING OPEN_UART_BEFORE_POWER_ON
//===== Uart Active Function ======
#define UART_HAS_ACTIVE_FUNCTION 0
#define _STR1(R) #R
#define STR1(R) _STR1(R)
//#define TRACE_ENABLE
#define COMMUNICATION_INTERFACE_UART 0
#define COMMUNICATION_INTERFACE_SPI 1
#define COMMUNICATION_INTERFACE_SELECE COMMUNICATION_INTERFACE_UART
#undef COM_SPI
#undef COM_UART
#if COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_SPI
#define COM_SPI
#elif COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_UART
#define COM_UART
#define UART_MAX_SUPPORT_BAUDRATE 921600
#endif
/** Superior Tool */
// DO NOT ENABLE TO USER
// #define SUPERIOR_DEBUG_ENABLE
#endif
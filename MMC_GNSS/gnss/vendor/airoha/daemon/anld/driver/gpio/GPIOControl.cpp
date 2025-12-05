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
/*
 * @Author: your name
 * @Date: 2020-07-30 14:58:47
 * @LastEditTime: 2020-08-04 13:54:02
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \anld\gpio\GPIOControl.cpp
 */
#include "GPIOControl.h"
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include "ConfigLoader.h"
#include "simulation.h"
#include <cutils/properties.h>
#define GNSS_NODE_EXT_CTRL
// #define KERNEL_NODE_HAS_READ_POWER_STATUS_FEATURE
using Airoha::Configuration;
namespace Airoha{
namespace GPIO {
pthread_mutex_t driverLock =  PTHREAD_MUTEX_INITIALIZER;
std::mutex driverMutex;
enum class PowerState {
    UNKNOWN,
    ON,
    OFF
};
static PowerState powerState = PowerState::UNKNOWN;
void portPowerOnChip(){
    std::lock_guard<std::mutex> lock(driverMutex);
#if 0
    // Check custom system vendor property
    char vendorProp[PROP_VALUE_MAX] = {0};
    int propRet = property_get("persist.visteon.vendor.gnss.OFF", vendorProp, "false");
    if (propRet <= 0) {
        strcpy(vendorProp, "false");
        LOG_E("property_get failed, propRet=%d, setting vendorProp to 'false'", propRet);
    }
    LOG_D("Visteon Vendor property persist.visteon.vendor.gnss.ON: %s", vendorProp);
    if (strcmp(vendorProp, "false") == 0) {
        LOG_D("Visteon PowerOn proceeding as vendor property is true");
        const char* reset_node = "/sys/kernel/gnss_reset_ctrl/gnss_reset";
        int gnssResetFd = open(reset_node, O_WRONLY);
        if (gnssResetFd < 0) {
            LOG_E("Visteon Failed to open GNSS reset sysfs node %s", reset_node);
            return;
        }
        const char* cmd = "gnssreseton";
        int ret = write(gnssResetFd, cmd, strlen(cmd));
        if (ret < 0) {
            LOG_E("Visteon Failed to write reset on command");
            close(gnssResetFd);
            return;
        }
        LOG_E("Visteon Successfully wrote reset on command: %s", cmd);
        close(gnssResetFd);
        powerState = PowerState::ON;
    } else {
        LOG_D("Visteon PowerOn skipped as vendor property is not true");
    }
#endif
    powerState = PowerState::ON;
    LOG_E("Visteon GNSS portPowerOnChip (RESET_N - HIGH) - Returned here");
}
void portPowerOffChip(){
    std::lock_guard<std::mutex> lock(driverMutex);
#if 0
    const char* reset_node = "/sys/kernel/gnss_reset_ctrl/gnss_reset";
    
    int gnssResetFd = open(reset_node, O_WRONLY);
    if (gnssResetFd < 0) {
        LOG_E("Visteon Failed to open GNSS reset sysfs node %s", reset_node);
        return;
    }

    const char* cmd = "gnssresetoff";
    int ret = write(gnssResetFd, cmd, strlen(cmd));
    if (ret < 0) {
        LOG_E("Visteon Failed to write reset off command");
        close(gnssResetFd);
        return;
    }

    LOG_E("Visteon Successfully wrote reset off command: %s", cmd);
    close(gnssResetFd);
#endif
    powerState = PowerState::OFF;
    LOG_E("Visteon GNSS portPowerOnChip (RESET_N - LOW) - Returned here");
}

static std::string toAsciiUpper(const char *c_str) {
    std::string str = c_str;
    for (char &c : str) {
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
    }
    return str;
}
DriverPowerStatus portGetPowerStatus() {
    switch(powerState) {
        case PowerState::ON:
            return POWER_STATUS_POWER_ON;
        case PowerState::OFF:
            return POWER_STATUS_POWER_OFF;
        case PowerState::UNKNOWN:
        default:
            return POWER_STATUS_UNKNOWN;
    }
}
void portGenerateInterrupt(){
#ifdef MOCK_GPIO
    mockGenerateInterrupt();
    return;
#endif
    portGnerateInterruptLow();
    usleep(10000);
    portGenerateInterruptHigh();
    
}
void portGnerateInterruptLow(){
    pthread_mutex_lock(&driverLock);
    int fd = open("/dev/airoha_gps",O_RDWR|O_NOCTTY|O_NONBLOCK);
    if(fd < 0){
        pthread_mutex_unlock(&driverLock);
        LOG_E("open airoha gps driver failed");
        return ;
    }
    char msg_low[10] = "DI"; //driven low
    int ret = write(fd,msg_low,10);
    LOG_D("successful write %d,msg:%s",ret,msg_low);

    close(fd);
    pthread_mutex_unlock(&driverLock);
    return ;
}
void portGenerateInterruptHigh(){
    pthread_mutex_lock(&driverLock);
    int fd = open("/dev/airoha_gps",O_RDWR|O_NOCTTY|O_NONBLOCK);
    if(fd < 0){
        pthread_mutex_unlock(&driverLock);
        LOG_E("open airoha gps driver failed");
        return ;
    }
    char msg_high[10] = "DF"; //driven high
    int ret = write(fd,msg_high,10);
    LOG_D("successful write %d,msg %s",ret,msg_high);
    close(fd);
    pthread_mutex_unlock(&driverLock);
    return ;
}
void portLockSystem() {
    pthread_mutex_lock(&driverLock);
    int fd = open("/dev/airoha_gps", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        pthread_mutex_unlock(&driverLock);
        LOG_E("open airoha gps driver failed");
        return;
    }
    char msg_high[10] = "LOCKWA";
    int ret = write(fd, msg_high, 10);
    LOG_D("successful write %d,msg %s", ret, msg_high);
    close(fd);
    pthread_mutex_unlock(&driverLock);
    return;
}
void portUnlockSystem() {
    pthread_mutex_lock(&driverLock);
    int fd = open("/dev/airoha_gps", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        pthread_mutex_unlock(&driverLock);
        LOG_E("open airoha gps driver failed");
        return;
    }
    char msg_high[10] = "UNLOCK";
    int ret = write(fd, msg_high, 10);
    LOG_D("successful write %d,msg %s", ret, msg_high);
    close(fd);
    pthread_mutex_unlock(&driverLock);
    return;
}
void portResetChip() {
    // to be implememted
    return;
}
// Mock handle
#ifdef MOCK_GPIO
void mockPowerOnChip(){
    LOG_D("Mock FUnction: pull up");

}
void mockPowerOffChip(){
    LOG_D("Mock FUnction: pull off");

}
void mockGenerateInterrupt(){
    LOG_D("Mock FUnction: interrupt generate!,send wakeup");
    
}
#endif
}  // namespace GPIO
} //namespace Airoha

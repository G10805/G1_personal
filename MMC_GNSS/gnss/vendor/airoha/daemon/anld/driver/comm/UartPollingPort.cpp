/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

/*
 * @Author: your name
 * @Date: 2020-08-14 11:32:29
 * @LastEditTime: 2020-08-14 15:29:37
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \anld\driver\comm\UartPollingPort.cpp
 */
#include "UartPollingPort.h"
#include "config.h"
#include "unistd.h"
#include <fcntl.h>
#include <termios.h>
#include "simulation.h"
#include <mutex>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <string.h>
using namespace Airoha::Config;
using Airoha::Communicator::UartPoll;
using Airoha::Communicator::PortStatus;
using Airoha::Communicator::PortEndian;
UartPoll::UartPoll() {
    uartFd = 0;
    stopFlag = 0;
    endian = PortEndian::PORT_BIG_ENDIAN;
}

UartPoll::~UartPoll() {
    // make sure the port is back to idle when instance deconstruct
    portClose();
}

PortStatus UartPoll::portOpen(PortNum number) {
    UartConfig *pConfig = NULL;
    //mutex will be unlock automatically
    std::lock_guard<std::mutex> locker(uartMutex);
    pConfig = (UartConfig *)customerGetConfig(number);
    uartFd = open(pConfig->devName,O_RDWR);
    if(uartFd < 0){
        LOG_E("uart could not init");
        return PortStatus::PORT_STATUS_ERROR_OPEN_FAILED;
    }
    struct termios ttyConfig;
    memset(&ttyConfig,0,sizeof(ttyConfig));
    int result = tcgetattr(uartFd,&ttyConfig);
    if(result != 0){
        LOG_E("get attr failed!!%d,%s",result,strerror(errno));
        return PortStatus::PORT_STATUS_ERROR_CONFIG_FAILED;
    }
    cfsetispeed(&ttyConfig,baudrateMapping(pConfig->baudrate));
    cfsetospeed(&ttyConfig,baudrateMapping(pConfig->baudrate));
    ttyConfig.c_cflag |= CS8;
    //no echo, raw mode
    ttyConfig.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH|ISIG);
    ttyConfig.c_oflag &= ~(ONLCR|OPOST);
    ttyConfig.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                       | INLCR | IGNCR | ICRNL | IXON);
    ttyConfig.c_cc[VTIME] = 1;
    ttyConfig.c_cc[VMIN] = 0;
    result = tcsetattr(uartFd, TCSANOW, &ttyConfig);
    if(result != 0){
        return PortStatus::PORT_STATUS_ERROR_CONFIG_FAILED;
    }
    return PortStatus::PORT_STATUS_OK;              

}

PortStatus UartPoll::portClose() {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd > 0){
        close(uartFd);
    }
    return PortStatus::PORT_STATUS_OK;
}

PortStatus UartPoll::portSendPolling(const uint8_t* buffer, size_t length) {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    if(length == 0 || buffer == NULL){
        return PortStatus::PORT_STATUS_ERROR_INVALID_PARAMETER;
    }
    size_t sendLength = length;
    size_t i = 0;
    while(stopFlag == 0 && sendLength > 0){
        int sendNum = write(uartFd,buffer+i,sendLength);
        i += sendNum;
        sendLength -= sendNum;
    }
    if(sendLength == 0){
        return PortStatus::PORT_STATUS_OK;
    }
    if(stopFlag > 0){
        return PORT_STATUS_ERROR_USER_TERMINATE;
    }
    //should not come here
    assert(0);
    return PORT_STATUS_ERROR;
    
}

PortStatus UartPoll::portReceivePolling(uint8_t* buffer, size_t length) {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    if(length == 0 || buffer == NULL){
        return PortStatus::PORT_STATUS_ERROR_INVALID_PARAMETER;
    }
    size_t readLength = length;
    size_t i = 0;;
    while(stopFlag == 0 && readLength > 0){
        int receiveNum = read(uartFd,buffer+i,readLength);
        i += receiveNum;
        readLength -= receiveNum;
    }
    if(readLength == 0){
        return PortStatus::PORT_STATUS_OK;
    }
    if(stopFlag > 0){
        return PORT_STATUS_ERROR_USER_TERMINATE;
    }
    //should not come here
    assert(0);
    return PORT_STATUS_ERROR;
}

PortStatus UartPoll::portPutUint8(uint8_t source) {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    //for 1 bytes , we just send 1 time
    int sendLen = write(uartFd,&source,1);
    if(sendLen != sizeof(uint8_t)){
        return PortStatus::PORT_STATUS_ERROR;
    }
    return PortStatus::PORT_STATUS_OK;
}

PortStatus UartPoll::portPutUint16(uint16_t source) {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    uint8_t sendBuf[2]; // 2*8 = 16
    if(endian == PortEndian::PORT_BIG_ENDIAN){
        sendBuf[1] = source & 0xFF;
        sendBuf[0] = (source >> 8) & 0xFF;
    }else{
        sendBuf[1] = (source >> 8) & 0xFF;
        sendBuf[0] = source & 0xFF;
    }
    int sendLen = write(uartFd,sendBuf,2);
    if(sendLen != sizeof(uint16_t)){
        return PortStatus::PORT_STATUS_ERROR;
    }
    return PortStatus::PORT_STATUS_OK;
}

PortStatus UartPoll::portPutUint32(uint32_t source) {
    std::lock_guard<std::mutex> locker(uartMutex);
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    uint8_t sendBuf[4]; // 4*8 = 16
    if(endian == PortEndian::PORT_BIG_ENDIAN){
        sendBuf[3] = source & 0xFF;
        sendBuf[2] = (source >> 8) & 0xFF;
        sendBuf[1] = (source >> 16) & 0xFF;
        sendBuf[0] = (source >> 24) & 0xFF;
    }else{
        sendBuf[0] = source & 0xFF;
        sendBuf[1] = (source >> 8) & 0xFF;
        sendBuf[2] = (source >> 16) & 0xFF;
        sendBuf[3] = (source >> 24) & 0xFF;
    }
    int sendLen = write(uartFd,sendBuf,4);
    if(sendLen != sizeof(uint16_t)){
        return PortStatus::PORT_STATUS_ERROR;
    }
    return PortStatus::PORT_STATUS_OK;    
}

PortStatus UartPoll::portGetUint8(uint8_t &result) {
    std::lock_guard<std::mutex> locker(uartMutex);
    uint8_t rcvBuf[1] = {0};
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    int readLen = read(uartFd,rcvBuf,1);
    if(readLen == -1){
        return PortStatus::PORT_STATUS_ERROR;
    }else if(readLen == 0){
        return PortStatus::PORT_STATUS_ERROR_TIMEOUT;
    }else if(readLen == 1){
        result = rcvBuf[0];
        return PortStatus::PORT_STATUS_OK;  
    }
    //should not come to here

    return PortStatus::PORT_STATUS_ERROR_UNEXPECTED;
}

PortStatus UartPoll::portGetUint16(uint16_t &result) {
    std::lock_guard<std::mutex> locker(uartMutex);
    uint8_t rcvBuf[2] = {0};
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    int readLen = read(uartFd,rcvBuf,2);
    if(readLen == -1){
        return PortStatus::PORT_STATUS_ERROR;
    }else if(readLen == 0){
        return PortStatus::PORT_STATUS_ERROR_TIMEOUT;
    }else if(readLen == 2){
        if(endian == PortEndian::PORT_BIG_ENDIAN){
            result = (rcvBuf[0] << 8) | (rcvBuf[1]);
        }else{
            result = (rcvBuf[0]) | (rcvBuf[1] << 8);
        }
        return PortStatus::PORT_STATUS_OK;  
    }
    //should not come to here
    return PortStatus::PORT_STATUS_ERROR_UNEXPECTED;    
}

PortStatus UartPoll::portGetUint32(uint32_t &result) {
    std::lock_guard<std::mutex> locker(uartMutex);
    uint8_t rcvBuf[4] = {0};
    if(uartFd <= 0){
        return PortStatus::PORT_STATUS_ERROR_NOT_INIT;
    }
    int readLen = read(uartFd,rcvBuf,4);
    if(readLen == -1){
        return PortStatus::PORT_STATUS_ERROR;
    }else if(readLen == 0){
        return PortStatus::PORT_STATUS_ERROR_TIMEOUT;
    }else if(readLen == 4){
        if(endian == PortEndian::PORT_BIG_ENDIAN){
            result = (rcvBuf[0] << 24) | (rcvBuf[1] << 16) | (rcvBuf[2] << 8) | (rcvBuf[3]);
        }else{
            result = (rcvBuf[3] << 24) | (rcvBuf[2] << 16) | (rcvBuf[1] << 8) | (rcvBuf[0]);
        }
        return PortStatus::PORT_STATUS_OK;  
    }    
    return PortStatus::PORT_STATUS_ERROR_UNEXPECTED;    
}

bool UartPoll::portStop(uint8_t flag) {
    stopFlag = flag;
    return true;
}



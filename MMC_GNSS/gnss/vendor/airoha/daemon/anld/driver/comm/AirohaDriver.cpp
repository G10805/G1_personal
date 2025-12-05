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

#define LOG_TAG "AIROHA_DRIVER"
#include <sys/signalfd.h>
#include <signal.h>
#include "AirohaDriver.h"
#include <poll.h>
#include "simulation.h"
#include <unistd.h>
#include <fcntl.h>
using Airoha::AirohaDriverInterface;
void* AirohaDriverInterface::driverThread(void *vp){
    AirohaDriverInterface *p = (AirohaDriverInterface *)vp;
    struct pollfd poll_fd[2];

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    int f_socket = signalfd(-1, &set, SFD_NONBLOCK | SFD_CLOEXEC);
    LOG_D("AirohaDriverInterface driver thread run, %p",vp);
    uint8_t buffer[1024];
    while(1){
        if(!(p->isRunning)){
            break;
        }
        poll_fd[0].fd = p->driverFd;
        poll_fd[0].events = POLLIN;
        poll_fd[1].fd = f_socket;
        poll_fd[1].events = POLLIN;
        int poll_ret = poll(poll_fd,2,-1);
        if(poll_ret <= 0){
            continue;
        }
        // if this is signal fd
        if(poll_fd[1].revents & POLLIN){
            break;
        }
        int k = read(p->driverFd,buffer,20);
        if(k>0){
            buffer[k] = 0;
            //ALOGD("%s",buffer);
            //callback to user
            LOG_D("read from driver :%s",(char *)buffer);
            p->onDriverMessage(DriverMessage::DRIVER_MESSAGE_INTERRUPT_NOTIFY);
        }else{
            continue;
        }
    }
    LOG_D("close signal fd");
    close(f_socket);
    //LOG_D("uart rx end, %s",p->portName);
    return NULL;

}
void AirohaDriverInterface::openDriver(const char *name){
    this->driverFd = open(name, O_RDWR|O_NONBLOCK);
}
void AirohaDriverInterface::closeDriver(){
    close(this->driverFd);
}
    
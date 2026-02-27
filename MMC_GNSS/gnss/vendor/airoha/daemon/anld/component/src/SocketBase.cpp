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

#define LOG_TAG "AirSocket"

#include "SocketBase.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <simulation.h>
#include <sys/epoll.h>
#include <fcntl.h>
#if defined(__ANDROID_OS__)
#include <cutils/sockets.h>
#endif

//-1 means fail
int SocketBase::createTCPSocket(const char * abstractName, int supportClient) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un un_s;
    memset(&un_s, 0, sizeof(un_s));
    if (fd < 0) {
        LOG_E("New socket error");
        return -1;
    }
    un_s.sun_family = AF_UNIX;
    un_s.sun_path[0] = '\0';
    strcpy(&(un_s.sun_path[1]), abstractName);
    LOG_D("create IPC socket, fd:%d", fd);
    unlink(un_s.sun_path);
    if (::bind(fd, (struct sockaddr*)&un_s, (socklen_t)sizeof(un_s)) < 0) {
        LOG_E("bind error %d,%s", errno, strerror(errno));
        close(fd);
        return -1;
    }
    if (listen(fd, supportClient) < 0) { // support 1 client
        close(fd);
        return -1;
    }
    return fd;
}
int SocketBase::createTCPClient(const char *abstractName) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un un_s;
    memset(&un_s, 0, sizeof(un_s));
    if (fd < 0) {
        LOG_E("New socket error %d", __LINE__);
        return -1;
    }
    un_s.sun_family = AF_UNIX;
    un_s.sun_path[0] = '\0';
    strcpy(&(un_s.sun_path[1]), abstractName);
    if (::connect(fd, (struct sockaddr *)&un_s, (socklen_t)sizeof(un_s)) < 0) {
        LOG_E("connect error %d,%s", errno, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}
int SocketBase::createTCPSocket(const char *ip, int port, int supportClient) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        LOG_E("create TCP socket FAILED, %s[%d]", strerror(errno), errno);
        return -1;
    }
    struct sockaddr_in addrinfo;
    memset(&addrinfo, 0, sizeof(addrinfo));
    addrinfo.sin_family = AF_INET;
    addrinfo.sin_port = htons(port);
    inet_aton(ip, &addrinfo.sin_addr);
    ::bind(fd, (sockaddr *)&addrinfo, sizeof(addrinfo));
    if (listen(fd, supportClient) < 0) { // support 1 client
        LOG_E("listen failed, %s[%d]", strerror(errno), errno);
        close(fd);
        return -1;
    }
    LOG_I("create tcp socket:%s %d %d", ip, port, supportClient);
    return fd;
}
int SocketBase::createAndroidSocket(const char *androidSocketName) {
    int socketFd = -1;
#if defined(__ANDROID_OS__)
    socketFd =  android_get_control_socket(androidSocketName);
#elif defined(__LINUX_OS__)
    socketFd =  socket(AF_UNIX, SOCK_STREAM, 0);
    (void)androidSocketName;
#endif
    
    if (socketFd == -1) {
        LOG_E("get control socket failed!!");
        //assert(0);
    }
    return socketFd;
    //else will trigger error
}
SocketBase::SocketBase(int fileDescriptor) {
    fd = fileDescriptor;
}
int SocketBase::addSocketDescriporInEpoll(int epollfd, int socketfd) {
    struct epoll_event e_event;;
    memset(&e_event, 0, sizeof(e_event));
    e_event.data.fd = socketfd;
    e_event.events = EPOLLIN;
    setBlock(socketfd, false);
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &e_event);
}
int SocketBase::removeEpollSocket(int epollfd, int socketfd) {
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, NULL);
}
SocketBase::~SocketBase() {}
int SocketBase::setBlock(int fd, bool block) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (block) {
        flag = flag & (~O_NONBLOCK);
    }
    else {
        flag = flag | O_NONBLOCK;
    }
    if (fcntl(fd, F_SETFL, flag) < 0) {
        return false;
    }
    return true;
}
int SocketBase::createUdpClient(bool is_abstract, const char *name) {
    int fd;
    int size;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if (is_abstract) {
        addr.sun_path[0] = 0;
        memcpy(addr.sun_path + 1, name, strlen(name));
    } else {
        strncpy(addr.sun_path, name, sizeof(addr.sun_path));
    }
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) {
        LOG_E("socket() failed, reason=[%s]%d", strerror(errno), errno);
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, size) == -1) {
        LOG_E("connect() failed, abstract=%d name=[%s] reason=[%s]%d",
             is_abstract, name, strerror(errno), errno);
        close(fd);
        return -1;
    }
    return fd;
}
int SocketBase::createUdpServerAndListen(bool is_abstract, const char *name) {
    int fd;
    int size;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    size = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if (is_abstract) {
        addr.sun_path[0] = 0;
        memcpy(addr.sun_path + 1, name, strlen(name));
    } else {
        strncpy(addr.sun_path, name, sizeof(addr.sun_path));
        if (unlink(addr.sun_path) == -1) {
            LOG_E("unlink() failed, reason=[%s]%d", strerror(errno), errno);
        }
    }
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) {
        LOG_E("socket() failed, reason=[%s]%d", strerror(errno), errno);
        return -1;
    }
    if (::bind(fd, (struct sockaddr *)&addr, size) == -1) {
        LOG_E("bind() failed, abstract=%d name=[%s] reason=[%s]%d", is_abstract,
             name, strerror(errno), errno);
        close(fd);
        return -1;
    }
    return fd;
}
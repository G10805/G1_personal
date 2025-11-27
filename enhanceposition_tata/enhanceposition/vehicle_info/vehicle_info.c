// SPDX-License-Identifier: MediaTekProprietary
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
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "vehicle_info.h"

#define UART_BAUDRATE_DEFAULT 921600
#define UART_DATABITS_DEFAULT 8
#define UART_STOPBITS_DEFAULT 2
#define UART_PARITY_DEFAULT 'n'

static speed_t baudrate_value[] = {B921600,B460800, B230400,B115200, B57600, B9600};
static int baudrate_name[] = {921600, 460800, 230400, 115200, 57600,9600};

#define READ_BUF_LEN 32
#define ODB_CMD_START "<<CAN=ON\r\n"
#define ODB_CMD_STOP "<<CAN=OFF\r\n"
#define ODO_DATA_LEN 28 //CAN2 20 CAN3 28

int vehicle_log_level = L_INFO;

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

    int32_t gear;
    struct timespec gear_boottime;
}VEHICLE_INFO;

typedef struct {
    int fd;
    int baudrate;
    int databits;
    int parity;
    int stopbits;
    char *uart_hw_name;
    int vehicle_data_cnt;
    VEHICLE_INFO vehicle_data;
}UART_INFO;

unsigned int get_time(void)
{
       struct timeval tv;

       gettimeofday(&tv, NULL);

       return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static inline void *zalloc(size_t size)
{
    return calloc(1, size);
}

int32_t vehicle_set_log_level(void *handle, int32_t level)
{
    UNUSED(handle);
    vehicle_log_level = level;
    return V_SUCCESS;
}

int32_t vehicle_get_vehicle_speed(void *handle, float *vehicle_speed,
            struct timespec *timestamp)
{
    UART_INFO *uart_info = (UART_INFO *)handle;
    VEHICLE_INFO *v_data = &uart_info->vehicle_data;

    *vehicle_speed = v_data->vehicle_speed;
    *timestamp = v_data->vehicle_speed_boottime;

    return V_SUCCESS;
}

int32_t vehicle_get_wheel_speed(void *handle, float *l_front_speed,
            float *r_front_speed, float *l_rear_speed, float *r_rear_speed,
            struct timespec *timestamp)
{
    UNUSED(handle);
    UNUSED(l_front_speed);
    UNUSED(r_front_speed);
    UNUSED(l_rear_speed);
    UNUSED(r_rear_speed);
    UNUSED(timestamp);

    return V_NOTSUPPORT;
}

int32_t vehicle_get_gear(void *handle, int32_t *gear, struct timespec *timestamp)
{
    UART_INFO *uart_info = (UART_INFO *)handle;
    VEHICLE_INFO *v_data = &uart_info->vehicle_data;

    *gear = v_data->gear;
    *timestamp = v_data->gear_boottime;

    return V_SUCCESS;
}

int32_t vehicle_get_steering_angle(void *handle, float *angle,
            struct timespec *timestamp)
{
    UART_INFO *uart_info = (UART_INFO *)handle;
    VEHICLE_INFO *v_data = &uart_info->vehicle_data;

    *angle = v_data->angle;
    *timestamp = v_data->wheel_speed_boottime;

    return V_SUCCESS;
}

unsigned char sentence_checksum_calc(const char *sentence,int len)
{
    unsigned char checksum = 0;
    while (*sentence && len)
    {
        checksum ^= (unsigned char)*sentence++;
        len--;
    }
    return checksum;
}

/*
*get device node name of uart device, eg:/dev/ttyS1
*/
char *get_device_node(char *dev_path)
{
    char *saveptr = NULL;
    char *token = NULL;
    char *tmp_token = token;
    /*reuse memory*/
    char *dev_node = dev_path;

    token = strtok_r(dev_path, "/", &saveptr);

    while(token){
        tmp_token = token;
        token = strtok_r(NULL, "/", &saveptr);
    }
    sprintf(dev_node, "/dev/%s", tmp_token);
    return dev_node;
}

/*
 * Find device path of uart device, eg:/sys/devices/XXX
 */
char *find_uart_device(char *uart_hw_name)
{
    char command[PATH_LEN] = {'\0'};
    char *dev_path = NULL;
    FILE *fd;
    int len = 0;

    snprintf(command, PATH_LEN, "find /sys/devices -name %s", uart_hw_name);
    fd = popen(command, "r");
    if(fd) {
        dev_path = (char *)zalloc(PATH_LEN);
        fgets(dev_path, PATH_LEN, fd);
        pclose(fd);
        len = strlen(dev_path);
        if(len) {
            /*the last character in path is '\n', remove it*/
            dev_path[len -1] = '\0';
            memset(command, '\0', PATH_LEN);
            snprintf(command, PATH_LEN, "find %s -name ttyS*", dev_path);
            fd = popen(command, "r");
            if(fd) {
                memset(dev_path, '\0', PATH_LEN);
                fgets(dev_path, PATH_LEN, fd);
                pclose(fd);
                len = strlen(dev_path);
                if(len) {
                    /*the last character in path is '\n', remove it*/
                    dev_path[len -1] = '\0';
                    return dev_path;
                } else {
                    free(dev_path);
                    dev_path = NULL;
                    LOG_ERROR("there isn't the device path, command%s", command);
                    return NULL;
                }
            } else {
                free(dev_path);
                dev_path = NULL;
                LOG_ERROR("popen(%s, \"r\") failed, error:%s", command, strerror(errno));
                return NULL;
            }
        } else {
            free(dev_path);
            dev_path = NULL;
            LOG_ERROR("there isn't the device path, command%s", command);
            return NULL;
        }
    } else {
        LOG_ERROR("popen(%s, \"r\") failed, error:%s", command, strerror(errno));
        return NULL;
    }
}

int32_t open_uart_device(int *uart_fd, char *device)
{
    int fd;
    char *filename = get_device_node(device);

    fd = open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        LOG_ERROR("open %s failed error:%d\n", filename, fd);
        return V_ERROR;
    }

    *uart_fd = fd;
    LOG_DEBUG("open %s success with fd:%d\n", filename, *uart_fd);
    return V_SUCCESS;
}

int32_t uart_can_set_param(UART_INFO *param)
{
    int uart_fd = param->fd;
    int ret = 0;
    unsigned int i;
    struct termios options;

    tcflush(uart_fd, TCIOFLUSH);
    ret = tcgetattr(uart_fd,&options);
    if (ret != 0)
    {
        LOG_ERROR("tcgetattr fail, ret: %d ", ret);
        return V_ERROR;
    }

    //set baudrate
    for (i= 0; i < sizeof(baudrate_name) / sizeof(int); i++)
    {
        LOG_INFO("baudrate_name[%d]=%d, baudrate=%d ",
                  i, baudrate_name[i], param->baudrate);
        if (param->baudrate == baudrate_name[i])
        {
            cfsetispeed(&options, baudrate_value[i]);
            cfsetospeed(&options, baudrate_value[i]);
            break;
        }
    }
    if (i == sizeof(baudrate_name) / sizeof(int))
    {
        LOG_ERROR("unsupported baudrate: %d ", param->baudrate);
    }

    //set data bits
    options.c_cflag &= ~CSIZE;
    switch (param->databits)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        LOG_ERROR("unsupported databits: %d ", param->databits);
        return V_ERROR;
    }

    //set parity
    switch (param->parity)
    {
    case 'n':
    case 'N'://no checking
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O'://odd parity
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E'://even parity
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    default:
        LOG_ERROR("unsupported parity: %d ", param->parity);
        return V_ERROR;
    }

    //set stop bits
    switch (param->stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        LOG_ERROR("unsupported stopbits: %d ", param->stopbits);
        return V_ERROR;
    }

    options.c_cc[VTIME] = 150;
    options.c_cc[VMIN] = 0;

    //raw data input
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag  &= ~OPOST;

    ret = tcsetattr(uart_fd, TCSANOW, &options);
    if (ret != 0)
    {
        LOG_ERROR("tcsetattr fail, ret: %d ", ret);
        return V_ERROR;
    }

    LOG_INFO("tcsetattr OK, c_iflag[0x%x], c_cflag[0x%x], c_lflag[0x%x]",
              options.c_iflag, options.c_cflag, options.c_lflag);

    return V_SUCCESS;
}


int32_t vehicle_set_config(UART_INFO *uart_info)
{
    VEHICLE_INFO *v_data = &uart_info->vehicle_data;

    if (!strcmp(PACKAGE_ARCH, "aarch64"))
        uart_info->uart_hw_name = "11019000.serial";
#if !defined(__ANDROID_OS__)
    else if ((!strcmp(PACKAGE_ARCH, "cortexa7hf-neon-vfpv4")) && (!strcmp(TARGET_PLATFORM, "mt2731")))
        uart_info->uart_hw_name = "11003000.serial";
#endif
    else if (!strcmp(PACKAGE_ARCH, "cortexa7hf-neon-vfpv4"))
        uart_info->uart_hw_name = "11004000.serial";
    else{
        LOG_ERROR("can not recongnize ARCH:%s", PACKAGE_ARCH);
        return V_ERROR;
    }
    uart_info->baudrate = UART_BAUDRATE_DEFAULT;
    uart_info->databits = UART_DATABITS_DEFAULT;
    uart_info->stopbits = UART_STOPBITS_DEFAULT;
    uart_info->parity = UART_PARITY_DEFAULT;
    v_data->l_front_speed = -9999;
    v_data->r_front_speed= -9999;
    v_data->l_rear_speed= -9999;
    v_data->r_rear_speed = -9999;

    return V_SUCCESS;
}

int32_t send_data(int fd,const char * buf, int len)
{
    LOG_DEBUG("send_data fd %d  buf %s len %d", fd, buf, len);

    int ret = write(fd, buf, len);
    if (ret < 0){
        LOG_ERROR("send_data write %s fail ,ret: %d", buf, ret);
        return V_ERROR;
    }
    return V_SUCCESS;
}

int32_t vehicle_can_enable(UART_INFO *uart_info, int delay_cnt, char *cmd)
{
    uart_info->vehicle_data_cnt = 0;

    while (delay_cnt > 0)
    {
        send_data(uart_info->fd, cmd, strlen(cmd));
        sleep(1);
        if (uart_info->vehicle_data_cnt)
        {
            return V_SUCCESS;
        }
        delay_cnt--;
    }

    return V_ERROR;
}

int32_t vehicle_start(void *handle)
{
    UART_INFO *uart_info = (UART_INFO *)handle;

    LOG_INFO("vehicle starting .......");
    int delay_cnt = 10;//max 10s to wait vehicle setup

    /*vehicle setup */
    return vehicle_can_enable(uart_info, delay_cnt, ODB_CMD_START);
}

int32_t vehicle_can_disable(UART_INFO *uart_info, int delay_cnt, char *cmd)
{
    int last_cnt = uart_info->vehicle_data_cnt;

    while (delay_cnt > 0)
    {
        send_data(uart_info->fd, cmd, strlen(cmd));
        sleep(1);
        if (last_cnt == uart_info->vehicle_data_cnt)
        {
            return V_SUCCESS;
        }
        last_cnt = uart_info->vehicle_data_cnt;
        delay_cnt--;
    }

    return V_ERROR;
}

int32_t vehicle_stop(void *handle)
{
    LOG_INFO("vehicle stopping .......");

    UART_INFO *uart_info = (UART_INFO *)handle;
    int delay_cnt = 10;//max 10s to wait vehicle setup
    int ret = 0;
    /*vehicle stop */
    ret = vehicle_can_disable(uart_info, delay_cnt, ODB_CMD_STOP);
    if (ret < 0){
        LOG_ERROR("uart can disable failed");
    }

    return ret;
}

int32_t vehicle_init(void **handle)
{
    UART_INFO *uart_info = (UART_INFO *)zalloc(sizeof(UART_INFO));
    char *uart_device = NULL;
    LOG_INFO("vehicle initing ......");

    if(V_SUCCESS != vehicle_set_config(uart_info)) {
        LOG_ERROR("set config failed");
        return V_ERROR;
    }
    uart_device = find_uart_device(uart_info->uart_hw_name);
    if (!uart_device) {
        LOG_ERROR("no uart device %s found", uart_info->uart_hw_name);
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;
        return V_ERROR;
    }
    if (V_SUCCESS != open_uart_device(&uart_info->fd, uart_device)) {
        LOG_ERROR("open uart device failed");
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        if(uart_device) {
            free(uart_device);
            uart_device = NULL;
        }
        return V_ERROR;
    }
    int flags = fcntl(uart_info->fd, F_SETFL, 0); /* reset file asscee mode, ext:O_NONBLOCK */
    if (flags == -1) {
        LOG_ERROR("fcntl() failed invalid flags=%d reason=[%s]%d", flags, strerror(errno), errno);
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        if(uart_device) {
            free(uart_device);
            uart_device = NULL;
        }
        return V_ERROR;
    }

    if(uart_can_set_param(uart_info) != 0) {
        LOG_ERROR("set param failed");
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        if(uart_device) {
            free(uart_device);
            uart_device = NULL;
        }
        return V_ERROR;
    }

    LOG_INFO("using uart device : %s, fd : %d",uart_device, uart_info->fd);
    free(uart_device);
    *handle = (void *)uart_info;
    return V_SUCCESS;
}

int32_t vehicle_deinit(void **handle)
{
    LOG_INFO("vehicle deiniting... ");
    UART_INFO *uart_info = (UART_INFO *)*handle;

    if(uart_info){
        if (uart_info->fd != -1)
            close(uart_info->fd);

        free(*handle);
        *handle = NULL;
    } else {
        LOG_ERROR("uart_info is NULL ");
        return V_ERROR;
    }

    return V_SUCCESS;
}

int32_t vehicle_update(void *handle)
{
    UART_INFO *uart_info = (UART_INFO *)handle;
    VEHICLE_INFO *v_data = &uart_info->vehicle_data;
    float* vehicle_speed = &(v_data->vehicle_speed);
    float *angle = &(v_data->angle);
    int *gear = &(v_data->gear);
    int fd = uart_info->fd;
    static char odo_buf[ODO_DATA_LEN+1] = {0};
    static int odo_buf_start = 0;
    static int odo_buf_end = 0;
    static int odo_buf_len = 0;
    int ret = -1;
    char buf[READ_BUF_LEN] = {0}; //Can data ">>CAN2=XXX.YY,G,CC\r\n"
    char *g = NULL;
    char *p = NULL;

    char checksum[4] = {0};
    char c_odo[7] = {0};
    char c_rudd[9] = {0};
    unsigned char crc;
    float modo = 0.0;
    float mrudd = 0.0;
    int mgear = 0;

    ret = read(fd, buf, READ_BUF_LEN - 1);
    if (ret <= 0)
    {
        if(ret == 0)
           LOG_WARN("read can data timeout");
        else
           LOG_ERROR("read can data %s, ret[%d] error[%d]", strerror(errno), ret, errno);
        return V_ERROR;
    }
    LOG_DEBUG("recieved raw buf:len %d, %s", ret, buf);
    buf[ret]='\0';
    if (ret >= 2 && (strncmp(buf,">>",2) == 0))
    {
        odo_buf_start = 1;
        if (strstr(buf,"\n"))
        {
            odo_buf_start = 0;
            odo_buf_end  = 1;
            odo_buf_len = 20;
            strncpy(odo_buf,buf,(ret > ODO_DATA_LEN)?ODO_DATA_LEN:ret);
        }
        else
        {
            LOG_DEBUG("odo data start (%s) len (%d)", buf, ret);
            strncpy(odo_buf,buf,(ret > ODO_DATA_LEN)?ODO_DATA_LEN:ret);
            odo_buf_end = 0;
            odo_buf_len = ret;
        }
    }
    else if ((ret >= 1) && odo_buf_start)
    {
        strncpy(odo_buf+odo_buf_len,buf,
                (ret > (ODO_DATA_LEN - odo_buf_len))?(ODO_DATA_LEN - odo_buf_len):ret);
        odo_buf_len += ret;
        if (odo_buf_len > ODO_DATA_LEN)
            odo_buf_len = ODO_DATA_LEN;
        if (strstr(buf,"\n"))
        {
            LOG_DEBUG("odo data end (%s) len (%d)", buf, ret);
            odo_buf_start = 0;
            odo_buf_end = 1; //recieved end flag
        }
    }
    if (odo_buf_end && odo_buf_len >= 20)
    {
        LOG_DEBUG("recieved can data %s",odo_buf);
        p = strstr(odo_buf, "=");
        if (p == NULL)
        {
            LOG_ERROR("format is not as expected,miss \"=\"  ");
            return V_ERROR;
        }
        strncpy(c_odo,p+1,6);
        modo = atof(c_odo);
        modo = modo / 3.6; //km/h to ms/s
        g = strstr(p,",");
        if (g == NULL)
        {
            LOG_ERROR("format is not as expected,miss \",\"  ");
            return V_ERROR;
        }
        switch (g[1])
        {
        case 'P':
            mgear = 0;
            break;
        case 'D':
            mgear = 1;
            break;
        case 'S':
            mgear = 2;
            break;
        case 'L':
            mgear = 3;
            break;
        case 'N':
            mgear = 4;
            break;
        case 'R':
            mgear = 6;
            modo = -modo;
            break;
        default:
            break;
        }
        g = strstr(odo_buf,"*");
        if (g == NULL){
            LOG_ERROR("format is not as expected,miss \"*\"  ");
            return V_ERROR;
        }
        switch (odo_buf[5]) // 2 ,3
        {
        case '2':
            LOG_DEBUG("checksum len %td, value %02X\n",g-p-1,crc);
            crc = sentence_checksum_calc(p+1,g-p-1);
            break;
        case '3':
            LOG_DEBUG("checksum len %td, value %02X\n",g-p-1,crc);
            crc = sentence_checksum_calc(p+1,g-p-1);
            strncpy(c_rudd,p+10,g-p-10);
            mrudd = atof(c_rudd);
            break;
        default:
            LOG_ERROR("checksum failed");
            return V_ERROR;
        }
        snprintf(checksum,3,"%02X",crc);
        if (strncmp(g+1,checksum,2) == 0)
        {
            *vehicle_speed = modo;
            *angle = mrudd;
            *gear = mgear;
            clock_gettime(CLOCK_BOOTTIME, &v_data->vehicle_speed_boottime);
            v_data->gear_boottime = v_data->vehicle_speed_boottime;
            v_data->angle_boottime = v_data->vehicle_speed_boottime;
            v_data->wheel_speed_boottime = v_data->vehicle_speed_boottime;

            uart_info->vehicle_data_cnt ++;
            LOG_DEBUG("vehicle_speed %03.2f, angle %05.1f\n", modo, mrudd);
        }
        else
        {
            LOG_ERROR("check sum error checksum %s,%s ",checksum,g+1);
            return V_ERROR;
        }
    } else {
        LOG_DEBUG("recieved can cmd %s, len, %d\n",buf,ret);
        return V_SUCCESS;
    }
    return V_SUCCESS;
}

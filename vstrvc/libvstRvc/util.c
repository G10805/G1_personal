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
* MediaTek Inc. (C) 2015. All rights reserved.
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
#include "util_rvc.h"
#include "osal.h"
#include "drm_display.h"
#include "videodev2.h"
#include "xml.h"
#include "module_v4l2.h"
#include "libxml/parser.h"

typedef struct device_info {
	char name[64];
	char path[64];
	struct device_info *next;
} device_info;

static device_info device_list;
int dynamic_log_level = -1;

void device_path_init()
{
	int i;
	FILE *file;
	char name[64];
	char dev_path[64];
	char find_path[256];
	int len;
	device_info *next;

	LOG_INFO("device_path_init start");

	memset(&device_list, 0, sizeof(device_info));
	next = &device_list;

	for (i = 0; i < 100; i++) {
		sprintf(dev_path, "/dev/video%d", i);
		sprintf(find_path, "/sys/class/video4linux/video%d/name", i);

		file = fopen(find_path, "r");
		if (!file)
			break;
		memset(name, 0, sizeof(name));
		len = fread(name, 1, sizeof(name), file);
		fclose(file);
		if (len <= 0)
			continue;
		name[len] = '\0';

		next->next = (device_info*)malloc(sizeof(device_info));
		next = next->next;
		next->next = NULL;
		strcpy(next->name, name);
		strcpy(next->path, dev_path);
	}
	LOG_INFO("device_path_init exit");
}

void device_path_init_snd_boot()
{
	LOG_INFO("Enter");
	int i;
	int fd;
	char dev_path[64];
	char find_path[256];
	int len;
	int ret;
	device_info *next;
	struct v4l2_capability cap;

	LOG_INFO("device_path_init start");

	memset(&device_list, 0, sizeof(device_info));
	next = &device_list;
	for (i = 0; i < 100; i++) {
		sprintf(dev_path, "/dev/video%d", i);
		LOG_DBG("%s", dev_path);
		if(access(dev_path, F_OK) !=0) {
			break;
		}
		do {
			ret = v4l2_open(dev_path, &fd);
			if (ret != 0) {
				LOG_VERBOSE(10, "v4l2_open error, will retry");
				usleep(10000);
			}
		} while (ret!=0);
		memset(&cap, 0, sizeof(cap));
		if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0){
			if(strlen(cap.driver)!=0) {
				LOG_DBG("cap.driver is %s", cap.driver);
				next->next = (device_info*)malloc(sizeof(device_info));
				memset(next->next, 0, sizeof(device_info));
				next = next->next;
				next->next = NULL;
				strcpy(next->name, cap.driver);
				strcpy(next->path, dev_path);
			}
		} else {
			LOG_DBG("%s query capability fail", dev_path);
		}
		close(fd);
	}
	LOG_INFO("device_path_init_snd_boot exit");
}

int set_device_path_to_argv(char *dev_name)
{
	device_info *next;
	next = device_list.next;
	sprintf(dev_name, "--device_name:");
	while (next) {
		strcat(dev_name, next->name);
		strcat(dev_name, ",");
		next = next->next;
	}
	dev_name[strlen(dev_name)-1]='\0';
	return 0;
}
int get_device_path_from_argv(char *dev_name)
{
	device_info *next;
	int idx = 0;
	char name[64];
	char dev_path[64];
	char *p1,*p2;

	memset(&device_list, 0, sizeof(device_info));
	next = &device_list;
	p1 = strchr(dev_name, ':');
	if ( p1==NULL ) {
		LOG_ERR("argv format is wrong, return");
		return -1;
	}
	p1++;
	idx = 0;
	while (p1 != NULL) {
		p2 = strchr(p1, ',');
		if (p2 != NULL) {
			next->next = (device_info*)malloc(sizeof(device_info));
			memset(next->next, 0, sizeof(device_info));
			next = next->next;
			next->next = NULL;
			strncpy(next->name, p1, p2-p1);
			sprintf(next->path, "/dev/video%d", idx);
			p2++;
			idx++;
			p1 = p2;
		} else {
			next->next = (device_info*)malloc(sizeof(device_info));
			memset(next->next, 0, sizeof(device_info));
			next = next->next;
			next->next = NULL;
			strcpy(next->name, p1);
			sprintf(next->path, "/dev/video%d", idx);
			break;
		}
	}
	return 0;
}

char* device_get_path(char *name)
{
	device_info *next;
	if (!name)
		return NULL;

	next = device_list.next;
	while (next) {
		if (strstr(next->name, name))
			return next->path;
		next = next->next;
	}
	return NULL;
}

char* device_get_name(char *path)
{
	device_info *next;
	if (!path)
		return NULL;

	next = device_list.next;
	while (next) {
		if (strstr(next->path, path))
			return next->name;
		next = next->next;
	}
	return NULL;
}
int print_device_info()
{
	device_info *next;
	next = device_list.next;
	while (next) {
		LOG_DBG("next->name is %s", next->name);
		next = next->next;
	}
	return 0;
}
unsigned long string_to_value(char *type, char *value)
{
	if (!type || !value)
		return 0;
	if (strcmp(type, "int") == 0) {
		return atoi(value);
	} else if (strcmp(type, "string") == 0) {
		return (unsigned long)value;
	} else if (strcmp(type, "V4L2") == 0) {
		return find_v4l2_type(value);
	} else if (strcmp(type, "DRM") == 0) {
		return find_drm_type(value);
	}

	return 0;
}

unsigned int find_v4l2_type(char *type_name)
{
	if (strcmp("RGB888", type_name) == 0)
		return V4L2_PIX_FMT_RGB24;
	else if (strcmp("RGB565", type_name) == 0)
		return V4L2_PIX_FMT_RGB565;
	else if (strcmp("UYVY", type_name) == 0)
		return V4L2_PIX_FMT_UYVY;
	else if (strcmp("YUYV", type_name) == 0)
		return V4L2_PIX_FMT_YUYV;
	else if (strcmp("NV21", type_name) == 0)
		return V4L2_PIX_FMT_NV21;
	else if (strcmp("NV21M", type_name) == 0)
		return V4L2_PIX_FMT_NV21M;
	else if (strcmp("MT21", type_name) == 0)
		return V4L2_PIX_FMT_MT21;
	else if (strcmp("H264", type_name) == 0)
		return V4L2_PIX_FMT_H264;
	else if (strcmp("BGR888", type_name) == 0)
		return V4L2_PIX_FMT_BGR24;
	else if (strcmp("ARGB8888", type_name) == 0)
		return V4L2_PIX_FMT_ARGB32;
	else if (strcmp("ABGR8888", type_name) == 0)
		return V4L2_PIX_FMT_ABGR32;
	else if (strcmp("NV16", type_name) == 0)
		return V4L2_PIX_FMT_NV16;
	else if (strcmp("NV16M", type_name) == 0)
		return V4L2_PIX_FMT_NV16M;
	else if (strcmp("NV61", type_name) == 0)
		return V4L2_PIX_FMT_NV61;
	else if (strcmp("NV61M", type_name) == 0)
		return V4L2_PIX_FMT_NV61M;
	return 0;
}

unsigned int find_drm_type(char *type_name)
{
	if (strcmp("RGB888", type_name) == 0)
		return DRM_FORMAT_RGB888;
	else if (strcmp("RGB565", type_name) == 0)
		return DRM_FORMAT_RGB565;
	else if (strcmp("UYVY", type_name) == 0)
		return DRM_FORMAT_UYVY;
	else if (strcmp("YUYV", type_name) == 0)
		return DRM_FORMAT_YUYV;
	else if (strcmp("BGR888", type_name) == 0)
		return DRM_FORMAT_BGR888;
	else if (strcmp("ARGB8888", type_name) == 0)
		return DRM_FORMAT_ARGB8888;
	else if (strcmp("ABGR8888", type_name) == 0)
		return DRM_FORMAT_ABGR8888;
	return 0;
}

void log_level_init(void *handle)
{
	char *log_level;
	log_level = xml_get_setting(handle, "log_level");
	if (log_level) {
		dynamic_log_level = atoi(log_level);
		xmlFree(log_level);
	}
}

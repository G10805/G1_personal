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
#include <unistd.h>
#include "module.h"
#include "module_v4l2.h"
#include "cam_engine.h"

typedef struct camera_device {
	char	name[DEV_NAME_LENGTH];
	char	dev_name[DEV_NAME_LENGTH];

	pthread_mutex_t m_mutex;
	sem_t m_cam_data_proc_sem;
	sem_t m_cam_data_done_sem;

	bool cam_start;
	unsigned int cam_idx;
	unsigned int width;
	unsigned int height;
	unsigned int format;
	unsigned int pix_format;
	unsigned char *ptr;
	int buf_fd;
} camera_device;

void *camera_data_process(void *data, void *cb_info)
{
	struct camera_device *dev = (struct camera_device *)data;
	CAM_CB_INFO *ptr_cb_info = (CAM_CB_INFO *)cb_info;

	LOG_VERBOSE(1000, "camera_data_process +");
	if (ptr_cb_info->width == dev->width
		&& ptr_cb_info->height == dev->height
		/*&& ptr_cb_info->forcc == dev->format*/)
	{
		MUTEX_LOCK(dev->m_mutex);
		if (dev->cam_start)
		{
			SEM_POST(dev->m_cam_data_proc_sem);
			dev->ptr = ptr_cb_info->va;
			dev->buf_fd = ptr_cb_info->buf_fd;
			SEM_WAIT(dev->m_cam_data_done_sem);
		}
		MUTEX_UNLOCK(dev->m_mutex);
	}
	else {
		LOG_ERR("recv data:(w,h,f) = (%d,%d,%d)", ptr_cb_info->width, ptr_cb_info->height, ptr_cb_info->forcc);
	}
	LOG_VERBOSE(1000, "camera_data_process -");

	return NULL;
}

static int camera_mem_caps(void *private, int cap, int *val)
{
	(void)private;

	switch (cap) {
		case CAPS_MEM_INPUT:
		case CAPS_MEM_OUTPUT:
			*val = MEMORY_ALLOC_SELF;
			break;
		default:
			return -1;
	}
	return 0;
}

static void* camera_mem_alloc(void *private, int type, int *size)
{
	struct camera_device *dev;
	dev = (camera_device*)private;

	LOG_VERBOSE(1000, "camera_mem_alloc");
	if (dev->cam_start)
	{
		SEM_WAIT(dev->m_cam_data_proc_sem);
		if (NULL != dev->ptr)
			*size = dev->width * dev->height * 2;
	}

	return dev->ptr;
}

static int camera_mem_free(void *private, void *buff)
{
	struct camera_device *dev;
	dev = (camera_device*)private;

	LOG_VERBOSE(1000, "camera_mem_free");
	if (dev->cam_start)
	{
		dev->ptr = NULL;
		dev->buf_fd = 0;
		SEM_POST(dev->m_cam_data_done_sem);
	}

	return 0;
}

static void* camera_mem_get_ptr(void *private, void *buff)
{
	struct camera_device *dev;
	dev = (camera_device*)private;

	LOG_VERBOSE(1000, "camera_mem_get_ptr");

	return dev->ptr;
}

static int camera_mem_get_buf_fd(void *private, void *buff)
{
	struct camera_device *dev;
	dev = (camera_device*)private;
	return dev->buf_fd;
}

static memory_ops camera_mem_ops = {
	.caps = camera_mem_caps,
	.alloc = camera_mem_alloc,
	.free = camera_mem_free,
	.get_ptr = camera_mem_get_ptr,
	.get_buf_fd = camera_mem_get_buf_fd,
};


static void camera_driver_config(struct module *mod, char *name, unsigned long value)
{
	struct camera_device *dev;
	dev = (camera_device*)module_get_driver_data(mod);

	if (strcmp(name, "name") == 0) {
		memset(dev->name, 0, sizeof(dev->name));
		strncpy(dev->name, (char *)value, sizeof(dev->name) - 1);
	} else if (strcmp(name, "camera_index") == 0) {
		dev->cam_idx = (unsigned int)value;
	} else if (strcmp(name, "width") == 0) {
		dev->width = (unsigned int)value;
	} else if (strcmp(name, "height") == 0) {
		dev->height = (unsigned int)value;
	} else if (strcmp(name, "pix_format") == 0) {
		dev->pix_format = (unsigned int)value;
	}
}

static int camera_driver_init(struct module *mod)
{
	struct camera_device *dev;
	dev = (camera_device*)module_get_driver_data(mod);

	MUTEX_INIT(dev->m_mutex);
	SEM_INIT(dev->m_cam_data_proc_sem, 0);
	SEM_INIT(dev->m_cam_data_done_sem, 0);

	MUTEX_LOCK(dev->m_mutex);
    if (!cam_engine_init(dev->cam_idx))
	{
		LOG_ERR("cam_engine_init failed");
		MUTEX_UNLOCK(dev->m_mutex);
		return -1;
	}
	if (!cam_engine_start())
	{
		LOG_ERR("cam_engine_start failed");
		MUTEX_UNLOCK(dev->m_mutex);
		return -1;
	}
	if (!cam_engine_reg_cb(camera_data_process, dev))
	{
		LOG_ERR("cam_engine_reg_cb failed");
		MUTEX_UNLOCK(dev->m_mutex);
		return -1;
	}

	dev->cam_start = true;
	MUTEX_UNLOCK(dev->m_mutex);

	LOG_INFO("camera_driver_init done");

	return 0;
}

static void camera_driver_deinit(struct module *mod)
{
	struct camera_device *dev;
	dev = (camera_device*)module_get_driver_data(mod);

	MUTEX_LOCK(dev->m_mutex);
	cam_engine_stop();
	dev->cam_start = false;
	MUTEX_UNLOCK(dev->m_mutex);
	LOG_INFO("camera stopped");
}

static int camera_driver_handle_frame(struct module *mod)
{
	struct shm_buff *shm;
	struct camera_device *dev;
	// 'alloc' will dequeue frame, and 'free' enqueue by next module
#if 0
	struct shm_buff *out_frame;
	if (module_get_memory_type(mod, CAPS_MEM_OUTPUT) == MEMORY_ALLOC_SELF) {
		shm = module_alloc_buffer(mod, CAPS_MEM_OUTPUT, 0);
		if (shm) {
			module_push_frame(mod, shm);
		} else {
			usleep(5000);
		}
	} else {
		out_frame = module_next_alloc_buffer(mod, 0);
		if (out_frame) {
			shm = module_alloc_buffer(mod, CAPS_MEM_OUTPUT, 0);
			if (shm) {
				memcpy(shm_get_ptr(out_frame), shm_get_ptr(shm), out_frame->size > shm->size ? shm->size : out_frame->size);
				shm_free(shm);
			}
			module_push_frame(mod, out_frame);
		} else {
			usleep(5000);
		}
	}
#else
	LOG_DBG("Enter");
	shm = module_alloc_buffer(mod, BUFFER_OUTPUT, 0);
	if (shm) {
		LOG_ONCE("push first frame");
		LOG_VERBOSE(1000, "get frame");
		module_push_frame(mod, shm);
	}

#endif
	usleep(1 * 1000); // we use non-blocking mode, sleep reduce cpu loading

	dev = (camera_device*)module_get_driver_data(mod);

	if (dev->cam_start)
	{
		SEM_POST(dev->m_cam_data_done_sem);
	}

	LOG_DBG("Leave");
	return 0;
}

static int camera_driver_pause(struct module *mod)
{
	LOG_INFO("camera_driver_pause");
	return 0;
}

static module_driver camera_driver = {
	.config = camera_driver_config,
	.init = camera_driver_init,
	.deinit = camera_driver_deinit,
	.handle_frame = camera_driver_handle_frame,
	.pause = camera_driver_pause,
};

int camera_module_init(module *mod)
{
	struct camera_device *dev;
	dev = (camera_device*)malloc(sizeof(camera_device));
	memset(dev, 0, sizeof(*dev));

	module_set_name(mod, "camera_engine");
	module_register_driver(mod, &camera_driver, dev);
	module_register_allocator(mod, &camera_mem_ops, dev);
	return 0;
}

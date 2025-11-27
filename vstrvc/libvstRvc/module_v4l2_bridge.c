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
#include "module_v4l2.h"

static MUTEX_HANDLE(mutex_bridge_init);
static int bridge_inited = 0;
static struct v4l2_device *bridge_dev;
static int reference_bridge_count = 0;


static int v4l2_bridge_driver_init(struct module *mod)
{
	struct v4l2_device *dev;
	dev = (v4l2_device*)module_get_driver_data(mod);
	int ret = 0;
	MUTEX_LOCK(mutex_bridge_init);
	if(bridge_inited == 0){
		if (v4l2_driver_init(mod) != 0) {
			ret = -1;
			goto exit;
		}
		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == dev->cap_buff.buffer_type)
			dev->cap_buff.crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		else
			dev->cap_buff.crop.type = dev->cap_buff.buffer_type;
		struct v4l2_selection sel = {0};
		if (v4l2_s_crop(dev->fd, &dev->cap_buff.crop) !=0) {
			LOG_WARN("v4l2_s_crop failed");
		}
		bridge_inited = 1;
		bridge_dev = dev;
	} else {
		dev = bridge_dev;
	}
	reference_bridge_count ++;
exit:
	MUTEX_UNLOCK(mutex_bridge_init);
	return ret;
}

void v4l2_bridge_driver_deinit(struct module *mod)
{
	LOG_INFO("Enter");
	struct v4l2_device *dev;
	//struct shm_buff *shm;
	dev = (v4l2_device*)module_get_driver_data(mod);

	MUTEX_LOCK(mutex_bridge_init);
	reference_bridge_count --;
	LOG_INFO("reference_count is %d", reference_bridge_count);
	if(reference_bridge_count == 0) {
		LOG_INFO("reference_count is %d", reference_bridge_count);
		v4l2_device_deinit(dev);
		bridge_inited = 0;
	}
	MUTEX_UNLOCK(mutex_bridge_init);

	LOG_INFO("Leave");
}


static int v4l2_bridge_mem_caps(void *private, int cap, int *val)
{
	(void)private;

	switch (cap) {
		case CAPS_MEM_INPUT:
		case CAPS_MEM_OUTPUT:
			*val = MEMORY_ALLOC_OTHERS;
			break;
		default:
			return -1;
	}
	return 0;
}

static memory_ops v4l2_bridge_mem_ops = {
	.caps = v4l2_bridge_mem_caps,
	.alloc = v4l2_mem_alloc,
	.free = v4l2_mem_free,
	.get_ptr = v4l2_mem_get_ptr,
};

static int bridge_push_output_frame(module *mod)
{
	int i;
	struct v4l2_device *dev;
	struct v4l2_buffer_info *buf_info;
	static int last_flag = 0;
	shm_buff *shm;

	dev = (v4l2_device*)module_get_driver_data(mod);
	v4l2_device_deqbuf(dev, V4L2_BUFF_CAPTURE);

	if (dev->last_flag != last_flag) {
		link_config_modules(mod->ln, "disable_layer", dev->last_flag);
		last_flag = dev->last_flag;
	}

	shm = (shm_buff*)queue_deq(dev->cap_buff.waiting_queue);
	if (!shm)
		return 0;
	void *ptr = shm_get_ptr(shm);

	buf_info = &dev->cap_buff;
	MUTEX_LOCK(buf_info->mutex_stat);
	for(i = 0; i < buf_info->count; i++) {
		if (buf_info->buff_stat[i] == V4L2_BUFF_STAT_DEQUEUED
				&& buf_info->va[i].va_list == (void**)ptr) {
			buf_info->buff_stat[i] = V4L2_BUFF_STAT_FREE;
			MUTEX_UNLOCK(buf_info->mutex_stat);

			module_push_frame(mod, shm);

			return 0;
		}
	}
	MUTEX_UNLOCK(buf_info->mutex_stat);
	queue_enq_head(dev->cap_buff.waiting_queue, shm);

	return 0;
}


static int v4l2_bridge_driver_handle_frame(struct module *mod)
{
	LOG_VERBOSE(300000, "Enter");
	int ret;
	struct v4l2_device *dev;
	struct shm_buff  *out_frame;
	dev = (v4l2_device*)module_get_driver_data(mod);

	if (dev->cap_buff.waiting_queue && (dev->capture_queued_buf_count > 0))
		bridge_push_output_frame(mod);

	/*
	* push output frame
	*/
	if (module_get_memory_type(mod, CAPS_MEM_OUTPUT) == MEMORY_ALLOC_OTHERS) {
		if (v4l2_device_has_free_buffer(dev, V4L2_BUFF_CAPTURE)) {
			out_frame = module_next_alloc_buffer(mod, dev->cap_buff.sizeimage);
			if (out_frame) {
				ret = v4l2_device_queue_usrptr(dev, V4L2_BUFF_CAPTURE, shm_get_ptr(out_frame));
				if (ret < 0)
					shm_free(out_frame);
				else
					queue_enq(dev->cap_buff.waiting_queue, out_frame);
			}
		}
	} else {
		out_frame = module_alloc_buffer(mod,
						BUFFER_OUTPUT, dev->cap_buff.sizeimage); //alloc will deque buff
		if (out_frame) {
			module_push_frame(mod, out_frame);
		}
	}
	usleep(5000);
	LOG_VERBOSE(300000, "Leave");
	return 0;
}

static module_driver v4l2_bridge_driver = {
	.config = v4l2_driver_config,
	.init = v4l2_bridge_driver_init,
	.deinit = v4l2_bridge_driver_deinit,
	.handle_frame = v4l2_bridge_driver_handle_frame,
	.pause = v4l2_driver_pause,
};

int v4l2_bridge_module_init(module *mod)
{
	struct v4l2_device *dev;
	dev = (v4l2_device*)malloc(sizeof(v4l2_device));
	memset(dev, 0, sizeof(*dev));
	static int init_once = 0;

	module_set_name(mod, "ovl2_bridge");
	module_register_driver(mod, &v4l2_bridge_driver, dev);
	module_register_allocator(mod, &v4l2_bridge_mem_ops, dev);
	if (init_once == 0) {
		MUTEX_INIT(mutex_bridge_init);
		init_once = 1;
	}
	return 0;
}

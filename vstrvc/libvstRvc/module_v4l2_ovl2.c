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
#include "module_v4l2.h"

int v4l2_ovl2_mem_caps(void *private, int cap, int *val)
{
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

memory_ops v4l2_ovl2_mem_ops = {
	.caps = v4l2_ovl2_mem_caps,
	.alloc = v4l2_mem_alloc,
	.free = v4l2_mem_free,
	.get_ptr = v4l2_mem_get_ptr,
};

static MUTEX_HANDLE(mutex_ovl2_init);
static struct v4l2_device ovl2_dev = {0};
static int reference_count = 0;

static int v4l2_ovl2_driver_init(struct module *mod)
{
	struct v4l2_device *dev;
	dev = (v4l2_device*)module_get_driver_data(mod);
	int ret = 0;

	MUTEX_LOCK(mutex_ovl2_init);
	if(reference_count == 0){
		if (v4l2_driver_init(mod) != 0) {
			ret = -1;
			goto exit;
		}

		if (V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == dev->out_buff.buffer_type)
			dev->out_buff.crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		else
			dev->out_buff.crop.type = dev->out_buff.buffer_type;

		struct v4l2_selection sel = {0};
		if (v4l2_ss_crop(dev->fd, &dev->out_buff.crop) !=0) {
			LOG_WARN("v4l2_s_crop failed");
		}
	}

	reference_count ++;
exit:
	MUTEX_UNLOCK(mutex_ovl2_init);
	return ret;
}
int v4l2_ovl2_driver_handle_frame(struct module *mod)
{
	LOG_VERBOSE(300000, "Enter");
	int ret;
	struct v4l2_device *dev;
	struct shm_buff *in_frame, *out_frame;

	dev = (v4l2_device*)module_get_driver_data(mod);
	if (dev->out_buff.waiting_queue)
		v4l2_release_input_frame(mod);

	/*
	* queue input frame
	*/
	in_frame = module_get_frame(mod);
	if (in_frame) {
		LOG_VERBOSE(1000, "get frame");

		if (module_if_ours_buffer(mod, in_frame)) {
			shm_free(in_frame); // free will enqueue frame
		} else if (dev->out_buff.memory_type == V4L2_MEMORY_USERPTR) {
			ret = v4l2_device_queue_usrptr(dev, V4L2_BUFF_OUTPUT, shm_get_ptr(in_frame));
			if (ret < 0)
				shm_free(in_frame);
			else
				queue_enq(dev->out_buff.waiting_queue, in_frame);
		} else if (dev->out_buff.memory_type == V4L2_MEMORY_MMAP) {
			v4l2_device_deqbuf_all(dev, V4L2_BUFF_OUTPUT);
			v4l2_device_queue_usrptr(dev, V4L2_BUFF_OUTPUT, shm_get_ptr(in_frame));
			shm_free(in_frame);
		} else {
			MOD_ERR(mod, "wrong memory type %d", dev->out_buff.memory_type);
			shm_free(in_frame);
		}
	}
	usleep(5000);
	LOG_VERBOSE(300000, "Leave");
	return 0;
}
void v4l2_ovl2_driver_deinit(struct module *mod)
{
	LOG_INFO("Enter");
	struct v4l2_device *dev;
	struct shm_buff *shm;
	dev = (v4l2_device*)module_get_driver_data(mod);

	MUTEX_LOCK(mutex_ovl2_init);
	reference_count --;
	LOG_INFO("reference_count is %d", reference_count);
	if(reference_count == 0) {
		LOG_INFO("reference_count is %d", reference_count);
		v4l2_driver_deinit(mod);
	}
	MUTEX_UNLOCK(mutex_ovl2_init);

	LOG_INFO("Leave");
}

int v4l2_ovl2_driver_pause(struct module *mod)
{
	LOG_INFO("Enter");
	struct v4l2_device *dev;

	dev = (v4l2_device*)module_get_driver_data(mod);
	while (dev->output_queued_buf_count > 0) {
		v4l2_release_input_frame(mod);
		usleep(1000);
	}

	LOG_INFO("Leave");
	return 0;
}

static module_driver v4l2_ovl2_driver = {
	.config = v4l2_driver_config,
	.init = v4l2_ovl2_driver_init,
	.deinit = v4l2_ovl2_driver_deinit,
	.handle_frame = v4l2_ovl2_driver_handle_frame,
	.pause = v4l2_ovl2_driver_pause,
};

static void v4l2_ovl2_module_deinit(module *mod)
{
}

int v4l2_ovl2_module_init(module *mod)
{
	struct v4l2_device *dev;
	static int init_once = 0;

	module_set_name(mod, "v4l2_ovl2");
	module_register_driver(mod, &v4l2_ovl2_driver, &ovl2_dev);
	module_register_allocator(mod, &v4l2_ovl2_mem_ops, &ovl2_dev);
	if (init_once == 0) {
		init_once = 1;
		MUTEX_INIT(mutex_ovl2_init);
	}

	module_set_deinit(mod, v4l2_ovl2_module_deinit);
	return 0;
}

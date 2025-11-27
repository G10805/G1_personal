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
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include "CamEngine.h"
#include "osal.h"
#include "cam_engine.h"

using namespace android;
using namespace CAMCLIENT;
using namespace Mtk;

typedef struct camera_engine {
	sp<CamEngine> client;
	CAM_CB_FUNC_T mCallback;
	void *mParam;
} camera_engine;

struct IMEM_BUF_INFO
{
	int	 size;
	int	  memID;
	void  	 *virtAddr;
	void	 *phyAddr;
	int	  bufSecu;
	int	  bufCohe;
	int	  useNoncache;
};

camera_engine g_cam_engine;

static int gMemDevfd = -1;
static int mDevCnt = 0;

static void* buffer_mmap(int fd, void *addr, size_t length, int prot, int flags, int share_fd, off_t offset)
{
	void *mapping_address = NULL;

	mapping_address =  mmap(addr, length, prot, flags, share_fd, offset);

	if (mapping_address == MAP_FAILED) {
		//ERROR("ion_mmap failed fd = %d, addr = 0x%p, len = %zu, prot = %d, flags = %d, share_fd = %d, 0x%p: %s\n", fd, addr, length,
			  //prot, flags, share_fd, mapping_address, strerror(errno));
	}

	return mapping_address;
}

static int buffer_munmap(int fd, void *addr, size_t length)
{
	int ret = munmap(addr, length);

	if (ret < 0) {
		//ERROR("ion_munmap failed fd = %d, addr = 0x%p, len = %zu, %d: %s\n", fd, addr, length,
			  //ret, strerror(errno));
	}
	return ret;
}

int open_imem_drv() {
	mDevCnt ++;

	if (gMemDevfd > 0)
		return gMemDevfd;
 
	gMemDevfd = open("dev/dma_heap/mtk_mm", O_RDWR | O_CLOEXEC);
	if (gMemDevfd <= 0) {
		//ERROR("ion device open FAIL \n");
	}
	//ERROR("open_imem_drv gIonfd:%d", gMemDevfd);
	return gMemDevfd;
}

void close_imem_drv() 
{
	if (gMemDevfd < 0) {
		//ERROR("ionfd %d error", gMemDevfd);
		return;
	}

	mDevCnt--;
	if (mDevCnt == 0) {
		close(gMemDevfd);
		gMemDevfd = -1;
	}
}

int map_vir_addr(struct IMEM_BUF_INFO* pInfo)
{
	pInfo->virtAddr = (void *)buffer_mmap(gMemDevfd, NULL, pInfo->size, PROT_READ|PROT_WRITE, MAP_SHARED, pInfo->memID, 0);
	if (!pInfo->virtAddr)
	{
		//ERROR("Cannot map ion buffer. memID(0x%x)/size(0x%x)/va(%p)\n",pInfo->memID,pInfo->size,(void*)pInfo->virtAddr);
		return -1;
	}
	//INFO("mapVirtBuf mID[0x%x]/size[%d]/VA[%p]\n",pInfo->memID,pInfo->size,(void*)pInfo->virtAddr);

	return 0;

}

int unmap_vir_addr(struct IMEM_BUF_INFO* pInfo)
{
	buffer_munmap(gMemDevfd, (char*)pInfo->virtAddr, pInfo->size);
	return 0;
}

void *cam_engine_data_process(void *buf, void *data, void *ext_data, void *ext2)
{
	printf("camera_engine_data_process");
	camera_engine *engine = (camera_engine *)data;

	if (NULL == engine)
	{
		printf("invalid patam");
		return NULL;
	}

	ImgBufInfo *imgBuf = (ImgBufInfo *)(buf);
	int share_fd = imgBuf->mFD;
	uint8_t dataInfo = *(uint8_t *)ext2;

	int size = imgBuf->height * imgBuf->stride * 2; // yuv422 ,bpp = 2
	struct IMEM_BUF_INFO dmaBuffer = {0};
	dmaBuffer.memID = share_fd;
	dmaBuffer.size = size;
	if (map_vir_addr(&dmaBuffer) < 0)
	{
		LOG_ERR("map vir buf fail\n");
		return NULL;
	}
	unsigned char *pBuf = (unsigned char *)dmaBuffer.virtAddr;

	CAM_CB_INFO cam_cb_info;
	cam_cb_info.width = imgBuf->width;
	cam_cb_info.height = imgBuf->height;
	cam_cb_info.timestamp = *(static_cast<uint64_t *>(ext_data));
	cam_cb_info.forcc = imgBuf->format;
	cam_cb_info.buf_fd = share_fd;
	cam_cb_info.va = pBuf;

#ifdef DBG_DUMP
	static int output_yuv_file = -1;
	if (-1 == output_yuv_file)
		 output_yuv_file = open(DUMP_FILE, O_WRONLY | O_CREAT | O_APPEND, 0664);
	if (output_yuv_file > 0)
	{
		write(output_yuv_file, pBuf, dmaBuffer.size);
		sync();
	}
#endif
	if (NULL != engine->mCallback && NULL != engine->mParam)
	{
		engine->mCallback(engine->mParam, &cam_cb_info);
	}

	unmap_vir_addr(&dmaBuffer);

	return NULL;
}

bool cam_engine_init()
{
	g_cam_engine.client = CamEngineMgr::getInstance(false)->getCamEngine(CamEngineMgr::CAMERA_TYPE_CAMSV, false);
	if (NULL == g_cam_engine.client)
	{
		return false;
	}

	if(open_imem_drv()<=0)
	{
		return false;
	}

	return true;
}

bool cam_engine_start()
{
	if (NULL != g_cam_engine.client)
	{
		status_t ret = g_cam_engine.client->start();
		if (ret != OK) {
			return false;
		}
		g_cam_engine.client->registerDataProcCB(cam_engine_data_process, &g_cam_engine);
	}
	return true;
}

bool cam_engine_reg_cb(CAM_CB_FUNC_T callback, void *param)
{
	if (NULL != g_cam_engine.client)
	{
		g_cam_engine.mCallback = callback;
		g_cam_engine.mParam = param;
	}
	return true;
}

void cam_engine_stop()
{
	if (NULL != g_cam_engine.client)
	{
		g_cam_engine.client->registerDataProcCB(NULL, &g_cam_engine);
		g_cam_engine.client->stop();
	}
}


void cam_engine_deinit()
{
	if (NULL != g_cam_engine.client)
	{
		g_cam_engine.client->stop();
	}

	close_imem_drv();
}


//******************************************************************************************************************************
// Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  khab_test.c
* @brief Implements uhabtest khab testing functions
********************************************************************************************************************************
*/
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#if 0

#ifdef __ANDROID__
#include <linux/khab_drv_test_ioctl.h>
#else
#include "../api/khab_drv_test_ioctl.h"
#endif

#define PAGE_SIZE 4096
#include "habtest.h"

static int32_t khabtest_fd = -1;

int32_t khabtest_send(int32_t vc_id, int32_t size, int32_t pattern) {
	struct khab_send params;
	int32_t ret;
	char *pdata = malloc(size);

	memset(pdata, pattern, size);
	params.data = pdata;
	params.sizebytes = size; // send exact string, the Null terminator will be added at BE side
	ret = ioctl(khabtest_fd, IOCTL_KHAB_SEND, &params ,sizeof(params));
	if (ret)
		printf("khabtest failed to send (%d)\n", ret);
	free(pdata);
	return ret;
}

int32_t khabtest_recv(int32_t vc_id, int32_t size) {
	struct khab_recv params;
	int32_t ret;
	char *pdata = malloc(size);

	params.data = pdata;
	params.sizebytes = size;
	params.flags = 0; //blocking mode
	ret = ioctl(khabtest_fd, IOCTL_KHAB_RECV, &params, sizeof(params));
	if (ret)
		printf("khabtest failed to recv (%d)\n", ret);

	//((char*)params.data)[params.sizebytes] = 0;
	//printf("%s\n", (char*)params.data);
	free(pdata);
	return ret;
}

static int32_t pg_cnt;
int32_t khabtest_export(int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void** pv, uint8_t cached) {
	struct khab_exp params = {
		.num_pgs = size / PAGE_SIZE,
		.pattern = pattern,
		.expid = -1
	};
	int32_t ret;

	printf("khabtest_export %d pages, pattern %d\n", params.num_pgs, params.pattern);

	ret = ioctl(khabtest_fd, IOCTL_KHAB_EXP, &params ,sizeof(params));
	if (ret) {
		printf("khabtest failed to export %d\n", ret);
	} else {
		*export_id = params.expid;
		printf("khabtest_export return export_id %d\n", params.expid);
	}
	pg_cnt = size / PAGE_SIZE;
	return ret;
}

int32_t khabtest_unexport (int32_t vc_id, int32_t expid) {
	struct khab_unexp params = {
		.num_pgs = pg_cnt,
		.expid = expid
	};
	int ret;

	printf("khabtest_unexport %d pages, export_id %d\n", params.num_pgs, params.expid);

	ret = ioctl(khabtest_fd, IOCTL_KHAB_UNEXP, &params ,sizeof(params));
	if (ret)
		printf("khabtest failed to unexport %d\n", ret);
	else
		printf("khabtest_unexport export_id %d successfully\n", params.expid);
	return ret;
}

int32_t khabtest_open(int32_t *vc_id_ret, uint32_t mmid, int32_t flags) {
	khabtest_fd = open("/dev/khab_test_front", O_RDWR);
	if (khabtest_fd == -1)
		printf("failed to open khabtest (%d)\n", khabtest_fd);
	return khabtest_fd;
}

int32_t khabtest_close(int32_t vc_id) {
	if (khabtest_fd != -1)
		return close(khabtest_fd);
	else
		return -ENODEV;
}

int32_t khabtest_test_mem(int vc_id, int tx_rx, int mm_id, int num_bytes, int loop_count) {
	struct khab_testmem params = {
		.tx_rx = tx_rx,
		.mm_id = mm_id,
		.mem_size = num_bytes,
		.loop_count = loop_count,
	};
	int ret;

	printf("starting khab memtest...\n");
	ret = ioctl(khabtest_fd, IOCTL_KHAB_TESTMEM, &params ,sizeof(params));
	if (ret) {
		printf("failed in khab memory test! ret %d\n", ret);
	} else {
		printf("khab memory test succeeded\n");
	}
	return ret;
}

#else

#include "habtest.h"

// stub functions
int32_t khabtest_send(int32_t vc_id, int32_t size, int32_t pattern) {
	return 0;
}

int32_t khabtest_recv(int32_t vc_id, int32_t size) {
	return 0;
}

int32_t khabtest_export(int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void** pv, uint8_t cached) {
	return 0;
}

int32_t khabtest_unexport (int32_t vc_id, int32_t expid) {
	return 0;
}

int32_t khabtest_open(int32_t *vc_id_ret, uint32_t mmid, uint32_t timeout_ms, int32_t flags) {
	return 0;
}

int32_t khabtest_close(int32_t vc_id) {
	return 0;
}

int32_t khabtest_test_mem (int vc_id, int tx_rx, int mm_id, int mem_size, int loop) {
	return 0;
}

int32_t khabtest_query (int vc_id) {
	return 0;
}

#endif
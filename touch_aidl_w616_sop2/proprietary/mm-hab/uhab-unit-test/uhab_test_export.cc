/*
* Copyright (c) 2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdarg.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <ion/ion.h>
#include "gtest/gtest.h"

extern "C" {
#include "habmm.h"
}
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
void * invalid_address = (void *) 0xffffff8000000000;

struct ion_allocation_data  data_ok;
struct ion_allocation_data  data_ok_1;
char *pdata = NULL;
int ion_fd_ok=0;
int32_t size_ok=4096;
uint8_t cached_ok=0;
int rc_ok = 0;
int rc_ok_1 = 0;


namespace {
TEST(UhabSocketExportTest, UhabExport){
	int32_t handle;
	int32_t open_r;
	int32_t export_r;
	int32_t close_r;
	uint32_t export_id;
	uint32_t export_id_1;
	int32_t unexport_r;
	uint32_t export_id1;
	uint32_t export_id2;
	char size_10[10] = "abcdefghi";

	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_export begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err;
	}

	ion_fd_ok = ion_open();
	if (ion_fd_ok <= 0) {
		printf("vc_export_ion.failed to open ion device! return ion ion_fd_ok=%d\n", ion_fd_ok);
		ion_fd_ok = 0;
		habmm_socket_close(handle);
		goto err;
	} else {
		printf("vc_export_ion.open.passed ion_fd_ok=%d\n", ion_fd_ok);
	}

	data_ok.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
	data_ok.align = 4096;
	data_ok.handle = -1;
	data_ok.len = (size_t)size_ok;
	data_ok.flags = (unsigned int)cached_ok;

	rc_ok = ion_alloc_fd(ion_fd_ok, data_ok.len, data_ok.align, data_ok.heap_id_mask, data_ok.flags, &data_ok.handle);
	if (rc_ok < 0) {
		printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc_ok=%d\n", rc_ok);
		close(ion_fd_ok);
		ion_fd_ok = 0;
		habmm_socket_close(handle);
		goto err;
	}

	pdata = (char *)mmap(NULL,size_ok,PROT_READ|PROT_WRITE, MAP_SHARED, data_ok.handle, 0);
	pdata[0]='a';
	pdata[4095]='b';
	printf("read the write date: a and b pdata[0]=%d  pdata[4095]=%d\n", pdata[0], pdata[4095]);

	data_ok_1.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
	data_ok_1.align = 4096;
	data_ok_1.handle = -1;
	data_ok_1.len = (size_t)size_ok;
	data_ok_1.flags = (unsigned int)cached_ok;

	rc_ok_1 = ion_alloc_fd(ion_fd_ok, data_ok_1.len, data_ok_1.align, data_ok_1.heap_id_mask, data_ok_1.flags, &data_ok_1.handle);
	if (rc_ok_1 < 0) {
		printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc_ok_1=%d\n", rc_ok_1);
		close(ion_fd_ok);
		ion_fd_ok = 0;
		habmm_socket_close(handle);
		goto err;
	}

	/* Positive testing */
	printf("Test_habmm_export Positive testing : size_bytes is 4096 and flag is 0x00000001\n");
	export_r = habmm_export(handle, pdata, 4096, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	export_id1 = export_id;
	printf("Test_habmm_export Positive testing size_bytes is 4096 and flag is 0x00000001 the return value export_r=%d export_id=%d \n", export_r, export_id);
	EXPECT_EQ(0, export_r);

	printf("Test_habmm_export Positive testing : size_bytes is 4096 and flag is 0x00010000\n");
	export_r = habmm_export(handle, *(void**)&data_ok_1.handle, 4096, &export_id_1, HABMM_EXPIMP_FLAGS_FD);
	export_id2 = export_id_1;
	printf("Test_habmm_export Positive testing size_bytes is 4096 and flag is 0x00010000 the return value export_r=%d export_id_1=%d \n", export_r, export_id_1);
	EXPECT_EQ(0, export_r);

	/* unexport the space */
	unexport_r = habmm_unexport(handle, export_id1, HABMM_EXP_MEM_TYPE_DMA);
	printf("unexport the space flag is 0x00000001 the return value unexport_r=%d export_id1=%d \n", unexport_r, export_id1);
	EXPECT_EQ(0, unexport_r);

	unexport_r = habmm_unexport(handle, export_id2, HABMM_EXPIMP_FLAGS_FD);
	printf("unexport the space flag is 0x00010000 the return value unexport_r=%d export_id2=%d \n", unexport_r, export_id2);
	EXPECT_EQ(0, unexport_r);

	/* Negative testing */
	/* 1.Size is not page aligned */
	printf("1.habmm_export Negative testing : Size is not page aligned flag=0x00000001\n");
	export_r = habmm_export(handle, size_10, 10, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	printf("1.habmm_export Negative testing the return value export_r=%d  flag=0x00000001\n", export_r);
	EXPECT_LT(export_r, 0);

	printf("1.habmm_export Negative testing : Size is not page aligned flag=0x00010000\n");
	export_r = habmm_export(handle, size_10, 10, &export_id_1, HABMM_EXPIMP_FLAGS_FD);
	printf("1.habmm_export Negative testing the return value export_r=%d  flag=0x00010000\n", export_r);
	EXPECT_LT(export_r, 0);

	/* 2.handle is error */
	printf("2.habmm_export Negative testing : handle is error flag=0x00000001\n");
	export_r = habmm_export(0, pdata, 4096, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	printf("2.habmm_export Negative testing the return value export_r=%d flag=0x00000001 \n", export_r);
	EXPECT_LT(export_r, 0);

	printf("2.habmm_export Negative testing : handle is error flag=0x00010000\n");
	export_r = habmm_export(11, *(void**)&data_ok_1.handle, 4096, &export_id_1, HABMM_EXPIMP_FLAGS_FD);
	printf("2.habmm_export Negative testing the return value export_r=%d  flag=0x00010000\n", export_r);
	EXPECT_LT(export_r, 0);

	/* 3.buff_to_share is null or invalid address */
	printf("3.habmm_export Negative testing : buff_to_share is null  flag=0x00000001\n");
	export_r = habmm_export(handle, NULL, 4096, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	printf("3.habmm_export Negative testing the return value export_r=%d flag=0x00000001 \n", export_r);
	EXPECT_LT(export_r, 0);

	printf("3.habmm_export Negative testing : buff_to_share is null  flag=0x00010000\n");
	export_r = habmm_export(handle, NULL, 4096, &export_id_1, HABMM_EXPIMP_FLAGS_FD);
	printf("3.habmm_export Negative testing the return value export_r=%d flag=0x00010000 \n", export_r);
	EXPECT_LT(export_r, 0);

	printf("3.habmm_export Negative testing : buff_to_share is invalid address  flag=0x00000001\n");
	export_r = habmm_export(handle, invalid_address, 4096, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	printf("3.habmm_export Negative testing the return value export_r=%d flag=0x00000001 \n", export_r);
	EXPECT_LT(export_r, 0);

	printf("3.habmm_export Negative testing : buff_to_share is invalid address  flag=0x00010000\n");
	export_r = habmm_export(handle, invalid_address, 4096, &export_id_1, HABMM_EXPIMP_FLAGS_FD);
	printf("3.habmm_export Negative testing the return value export_r=%d flag=0x00010000 \n", export_r);
	EXPECT_LT(export_r, 0);

	/* 4.export_id is null */
	printf(" 4.habmm_export Negative testing : export_id is null    flag=0x00000001\n");
	export_r = habmm_export(handle, pdata, 4096, NULL, HABMM_EXP_MEM_TYPE_DMA);
	printf("4.habmm_export Negative testing the return value export_r=%d flag=0x00000001 \n", export_r);
	EXPECT_LT(export_r, 0);

	printf("4.habmm_export Negative testing : export_id is null    flag=0x00010000\n");
	export_r = habmm_export(handle, *(void**)&data_ok_1.handle, 4096, NULL, HABMM_EXPIMP_FLAGS_FD);
	printf("4.habmm_export Negative testing the return value export_r=%d flag=0x00010000 \n", export_r);
	EXPECT_LT(export_r, 0);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("habmm_export close  the return value close_r=%d  \n", close_r);

	/* testing for HAB_DEBUG version */
	if(0 != setenv("HAB_DEBUG", "1", 1))
		printf("Error setting HAB_DEBUG env! errno: %d (%s)\n", errno, strerror(errno));

	/* re-run some cases to cover UHAB_DBG branch to test DEBUG printing */
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	export_r = habmm_export(handle, pdata, 4096, &export_id, HABMM_EXP_MEM_TYPE_DMA);
	EXPECT_EQ(0, export_r);
	unexport_r = habmm_unexport(handle, export_id, HABMM_EXP_MEM_TYPE_DMA);
	EXPECT_EQ(0, unexport_r);
	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	unsetenv("HAB_DEBUG");

	// free the ion memory
	close(data_ok.handle);
	data_ok.handle = -1;
	data_ok.len = 0;

	close(data_ok_1.handle);
	data_ok_1.handle = -1;
	data_ok_1.len = 0;
	close(ion_fd_ok);
	ion_fd_ok = 0;

	err:
		printf("Can't open the mmid socket\n");
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	errno = 0;
	return RUN_ALL_TESTS();
}

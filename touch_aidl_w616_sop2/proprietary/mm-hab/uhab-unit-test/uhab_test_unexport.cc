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

struct ion_allocation_data  data_ok;
struct ion_allocation_data  data_ok_1;
char *pdata = NULL;
int ion_fd_ok=0;
int32_t size_ok=4096;
uint8_t cached_ok=0;
int rc_ok = 0;
int rc_ok_1 = 0;
int close_r;

namespace {
TEST(UhabSocketUnexportTest, UhabUnexport){
	int32_t handle;
	int32_t open_r;
	int32_t export_r;
	int32_t unexport_r;
	uint32_t export_id1;
	uint32_t export_id2;
	uint32_t wrong_export_id1 = 1000;
	uint32_t wrong_export_id2 = 2000;

	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_unexport begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
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
		printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc_ok=%d\n",rc_ok);
		close(ion_fd_ok);
		ion_fd_ok = 0;
		habmm_socket_close(handle);
		goto err;
	}

	pdata = (char *)mmap(NULL,size_ok,PROT_READ|PROT_WRITE, MAP_SHARED, data_ok.handle, 0);
	pdata[0]='a';
	pdata[4095]='b';
	printf("read the write date: a and b pdata[0]=%d  pdata[4095]=%d\n",pdata[0],pdata[4095]);

	data_ok_1.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
	data_ok_1.align = 4096;
	data_ok_1.handle = -1;
	data_ok_1.len = (size_t)size_ok;
	data_ok_1.flags = (unsigned int)cached_ok;

	rc_ok_1 = ion_alloc_fd(ion_fd_ok, data_ok_1.len, data_ok_1.align, data_ok_1.heap_id_mask, data_ok_1.flags, &data_ok_1.handle);
	if (rc_ok_1 < 0) {
		printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc_ok_1=%d\n",rc_ok_1);
		close(ion_fd_ok);
		ion_fd_ok = 0;
		habmm_socket_close(handle);
		goto err;
	}

	printf("Test_habmm_unexport  size_bytes is 4096 and flag is 0x00000001\n");
	export_r = habmm_export(handle, pdata, 4096, &export_id1, HABMM_EXP_MEM_TYPE_DMA);
	printf("Test_habmm_unexport  size_bytes is 4096 and flag is 0x00000001 the return value export_r=%d export_id=%d \n", export_r, export_id1);
	EXPECT_EQ(0, export_r);

	printf("Test_habmm_unexport  size_bytes is 4096 and flag is 0x00000000\n");
	export_r = habmm_export(handle, *(void**)&data_ok_1.handle, 4096, &export_id2, HABMM_EXPIMP_FLAGS_FD);
	printf("Test_habmm_unexport  size_bytes is 4096 and flag is 0x00000000 the return value export_r=%d export_id=%d \n", export_r, export_id2);
	EXPECT_EQ(0, export_r);

	/* Negative testing */
	/* 1.input invalid export_id    */
	printf("1.Test_habmm_unexport Negative testing :input invalid export_id  flag=0x00000000\n");
	unexport_r=habmm_unexport(handle, wrong_export_id1, 0x00000000);
	printf("1.Test_habmm_unexport Negative testing the return value unexport_r=%d  flag=0x00000000\n", unexport_r);
	EXPECT_LT(unexport_r, 0);

	printf("1.Test_habmm_unexport Negative testing :input invalid export_id  flag=0x00000000\n");
	unexport_r = habmm_unexport(handle, wrong_export_id2, 0x00000000);
	printf("1.Test_habmm_unexport Negative testing the return value unexport_r=%d  flag=0x00000000\n", unexport_r);
	EXPECT_LT(unexport_r, 0);

	/* 2.input invalid handle     */
	printf("2.Test_habmm_unexport Negative testing :input invalid handle  flag=0x00000000\n");
	unexport_r = habmm_unexport(0, export_id1, 0x00000000);
	printf("2.Test_habmm_unexport Negative testing the return value unexport_r=%d  flag=0x00000000\n", unexport_r);
	EXPECT_LT(unexport_r, 0);

	printf("2.Test_habmm_unexport Negative testing :input invalid handle  flag=0x00000000\n");
	unexport_r = habmm_unexport(1, export_id2, 0x00000000);
	printf("2.Test_habmm_unexport Negative testing the return value unexport_r=%d  flag=0x00000000\n", unexport_r);
	EXPECT_LT(unexport_r, 0);

	/* Positive testing */
	printf("Test_habmm_unexport Positive testing:\n");
	unexport_r = habmm_unexport(handle, export_id1, 0x00000000);
	printf("Test_habmm_unexport Positive testing flag is 0x00000000 the return value unexport_r=%d export_id1=%d \n", unexport_r, export_id1);
	EXPECT_EQ(0, unexport_r);

	unexport_r = habmm_unexport(handle, export_id2, 0x00000000);
	printf("Test_habmm_unexport Positive testing flag is 0x00000000 the return value unexport_r=%d export_id2=%d \n", unexport_r, export_id2);
	EXPECT_EQ(0, unexport_r);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_unexport  the return value close_r=%d  \n", close_r);

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
	return RUN_ALL_TESTS();
}

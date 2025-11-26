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
int32_t handle;
int32_t open_r;
int32_t recv_r;
int32_t import_r;
int32_t unimport_r;
int32_t close_r;
char export_id1;
char export_id2;
char export_id3;
char export_id4;
uint32_t wrong_export_id1 = 1000;
uint32_t wrong_export_id2 = 2000;
uint32_t size_export_id = 1;
char *size_page1 = NULL, *size_page2 = NULL, *size_page3 = NULL, *size_page4 = NULL;

namespace {

TEST(UhabSocketUnimportTestn, UhabUnimportn){
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_unimport begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err1;
	}
	recv_r = habmm_socket_recv(handle, &export_id1, &size_export_id, 0, 0);
	printf("Test_habmm_unimport : habmm_socket_recv the return value recv_r=%d  export_id1=%d\n", recv_r, export_id1);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id2, &size_export_id, 0, 0);
	printf("Test_habmm_unimport : habmm_socket_recv the return value recv_r=%d  export_id2=%d\n", recv_r, export_id2);
	EXPECT_EQ(0, recv_r);

	printf("Test_habmm_unimport :Test_habmm_import size_bytes is 4096 and flag is 0x00000001\n");
	import_r = habmm_import(handle, (void **)&size_page1, 4096, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("Test_habmm_unimport: Test_habmm_import size_bytes is 4096 and flag is 0x00000001 the return value import_r=%d export_id1=%d \n", import_r, export_id1);
	EXPECT_EQ(0, import_r);

	import_r = habmm_import(handle, (void **)&size_page2, 4096, export_id2, 0x00000000);
	printf("Test_habmm_unimport: Test_habmm_import size_bytes is 4096 and flag is 0x00000000 the return value import_r=%d export_id2=%d \n", import_r, export_id2);
	EXPECT_EQ(0, import_r);

	/* Negative testing */
	/* 1.input invalid export_id   */
	printf("1.Test_habmm_unimport Negative testing :input invalid export_id  flag=0x00000000\n");
	unimport_r = habmm_unimport(handle, wrong_export_id1, (void *)size_page1, 0x00000000);
	printf("1.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);

	printf("1.Test_habmm_unimport Negative testing :input invalid export_id  flag=0x00000000\n");
	unimport_r = habmm_unimport(handle, wrong_export_id2, (void *)size_page2, 0x00000000);
	printf("1.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);

	/* 2.input invalid handle   */
	printf("2.Test_habmm_unimport Negative testing :input invalid handle  flag=0x00000000\n");
	unimport_r = habmm_unimport(0, export_id1, (void *)size_page1, 0x00000000);
	printf("2.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);

	printf("2.Test_habmm_unimport Negative testing :input invalid handle  flag=0x00000000\n");
	unimport_r = habmm_unimport(1, export_id2, (void *)size_page2, 0x00000000);
	printf("2.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);

	/* 3.input error buff_shared  or  invalid address  */
	printf("3.Test_habmm_unimport Negative testing :input invalid address  flag=0x00000000\n");
	unimport_r = habmm_unimport(handle, export_id1, invalid_address, 0x00000000);
	printf("3.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);

	printf("3.Test_habmm_unimport Negative testing :input invalid address  flag=0x00000000\n");
	unimport_r = habmm_unimport(handle, export_id2, invalid_address, 0x00000000);
	printf("3.Test_habmm_unimport Negative testing the return value unimport_r=%d  flag=0x00000000\n", unimport_r);


	/* 4.input flag is not HABMM_EXPIMP_FLAGS_FD and buff_shared null */
	unimport_r = habmm_unimport(handle, export_id1, NULL, HABMM_IMP_FLAGS_FIXED);
	printf("4.Test_habmm_unimport Negative testing: flag is not HABMM_EXPIMP_FLAGS_FD and buff_shared null, return %d\n", unimport_r);
	EXPECT_EQ(-EINVAL, unimport_r);

	/* 5.type is HAB_IMP_MMAP_NODE_UVA and munmap fail */
	unimport_r = habmm_unimport(handle, wrong_export_id1, (void *)size_page2, HABMM_IMP_FLAGS_FIXED);
	printf("5.Test_habmm_unimport Negative testing: type is HAB_IMP_MMAP_NODE_UVA and invalid export id, return %d\n", unimport_r);
	EXPECT_LT(unimport_r, 0);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_unimport close  the return value close_r=%d  \n", close_r);

	err1:
		printf("Can't open the mmid socket1\n");
}

TEST(UhabSocketUnimportTestp, UhabUnimportp){
	open_r = habmm_socket_open(&handle, 601, 0,0);
	printf("Test_habmm_unimport begin: habmm_socket_open the return value open_r=%d  \n", open_r);
	if(open_r < 0)
	{
		goto err;
	}
	recv_r = habmm_socket_recv(handle, &export_id1, &size_export_id, 10, 0);
	printf("Test_habmm_unimport : habmm_socket_recv the return value recv_r=%d  export_id1=%d\n", recv_r, export_id1);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id2, &size_export_id, 10, 0);
	printf("Test_habmm_unimport : habmm_socket_recv the return value recv_r=%d  export_id2=%d\n", recv_r, export_id2);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id3, &size_export_id, 0, 0);
	printf("Test_habmm_unimport : habmm_socket_recv the return value recv_r=%d  export_id3=%d\n", recv_r, export_id3);
	EXPECT_EQ(0, recv_r);

	printf("Test_habmm_unimport :Test_habmm_import size_bytes is 4096 and flag is 0x00000001\n");
	import_r = habmm_import(handle, (void **)&size_page1, 4096, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("Test_habmm_unimport: Test_habmm_import size_bytes is 4096 and flag is 0x00000001 the return value import_r=%d export_id1=%d \n", import_r, export_id1);
	EXPECT_EQ(0, import_r);

	import_r = habmm_import(handle, (void **)&size_page2, 4096, export_id2, 0x00000000);
	printf("Test_habmm_unimport: Test_habmm_import size_bytes is 4096 and flag is 0x00000000 the return value import_r=%d export_id2=%d \n", import_r, export_id2);
	EXPECT_EQ(0, import_r);


	import_r = habmm_import(handle, (void **)&size_page3, 4096, export_id3, HABMM_EXPIMP_FLAGS_FD);
	printf("Test_habmm_unimport: Test_habmm_import size_bytes is 4096 and flag is 0x00000000 the return value import_r=%d export_id3=%d \n", import_r, export_id3);
	EXPECT_EQ(0, import_r);

	/* Positive testing */
	printf("Test_habmm_unimport Positive testing:\n");
	unimport_r = habmm_unimport(handle, export_id1, (void *)size_page1, HABMM_EXP_MEM_TYPE_DMA);
	printf("Test_habmm_unimport Positive testing  the return value unimport_r=%d export_id1=%d flag=0x00000001\n", unimport_r, export_id1);
	EXPECT_EQ(0, unimport_r);

	printf("Test_habmm_unimport Positive testing:\n");
	unimport_r = habmm_unimport(handle, export_id2, (void *)size_page2, 0x00000000);
	printf("Test_habmm_unimport Positive testing  the return value unimport_r=%d export_id2=%d flag=0x00000000\n", unimport_r, export_id2);
	EXPECT_EQ(0, unimport_r);

	printf("Test_habmm_unimport Positive testing:\n");
	unimport_r = habmm_unimport(handle, export_id3, (void *)size_page3, HABMM_EXPIMP_FLAGS_FD );
	printf("Test_habmm_unimport Positive testing  the return value unimport_r=%d export_id3=%d flag=0x00010000\n", unimport_r, export_id3);
	EXPECT_EQ(0, unimport_r);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_unimport close  the return value close_r=%d  \n", close_r);

	/* testing for HAB_DEBUG version */
	if(0 != setenv("HAB_DEBUG", "1", 1))
		printf("Error setting HAB_DEBUG env! errno: %d (%s)\n", errno, strerror(errno));

	/* re-run some cases to cover UHAB_DBG branch to test DEBUG printing */
	open_r = habmm_socket_open(&handle, 601, 0,0);
	EXPECT_EQ(0, open_r);
	recv_r = habmm_socket_recv(handle, &export_id4, &size_export_id, 0, 0);
	EXPECT_EQ(0, recv_r);
	import_r = habmm_import(handle, (void **)&size_page4, 4096, export_id4, HABMM_IMPORT_FLAGS_CACHED);
	EXPECT_EQ(0, import_r);
	unimport_r = habmm_unimport(handle, export_id4, (void *)size_page4, HABMM_EXP_MEM_TYPE_DMA);
	EXPECT_EQ(0, unimport_r);
	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);

	unsetenv("HAB_DEBUG");

	err:
		printf("Can't open the mmid socket\n");
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	errno = 0;
	return RUN_ALL_TESTS();
}

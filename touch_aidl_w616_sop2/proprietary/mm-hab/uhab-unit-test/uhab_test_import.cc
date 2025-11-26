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

namespace {
TEST(UhabSocketImportTest, UhabImport){
	int32_t handle;
	int32_t open_r;
	int32_t recv_r;
	int32_t import_r;
	int32_t close_r;
	char export_id1;
	char export_id2;
	char export_id3;
	char export_id4;
	char export_id5;
	char export_id6;
	int32_t unimport_r;
	void *pvoid1;
	char *pvoid1_1;
	int grant_fd1;
	void *pvoid2;
	char *pvoid2_2;
	int grant_fd2;
	uint32_t wrong_export_id = 1000;
	uint32_t size_export_id = 1;
	char *size_10 = NULL;
	char *size_page = NULL;
	char *size_page1 = NULL, *size_page2 = NULL, *size_page3 = NULL;
	char *size_page4 = NULL, *size_page5 = NULL, *size_page6 = NULL;

	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_import begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err;
	}
	recv_r = habmm_socket_recv(handle, &export_id1, &size_export_id, 0, 0);
	printf("Test_habmm_import : habmm_socket_recv the return value recv_r=%d  export_id1=%d\n", recv_r, export_id1);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id2, &size_export_id, 0, 0);
	printf("Test_habmm_import : habmm_socket_recv the return value recv_r=%d  export_id2=%d\n", recv_r, export_id2);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id3, &size_export_id, 0, 0);
	printf("Test_habmm_import : habmm_socket_recv the return value recv_r=%d  export_id3=%d\n", recv_r, export_id3);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id4, &size_export_id, 0, 0);
	printf("Test_habmm_import : habmm_socket_recv the return value recv_r=%d  export_id4=%d\n", recv_r, export_id4);
	EXPECT_EQ(0, recv_r);

	recv_r = habmm_socket_recv(handle, &export_id5, &size_export_id, 0, 0);
	printf("Test_habmm_import : habmm_socket_recv the return value recv_r=%d  export_id5=%d\n", recv_r, export_id5);
	EXPECT_EQ(0, recv_r);

	/* Positive testing */
	printf("Test_habmm_import Positive testing : size_bytes is 4096 and flag is 0x00000001\n");
	import_r = habmm_import(handle, (void **)&size_page1, 4096, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("Test_habmm_import Positive testing size_bytes is 4096 and flag is 0x00000001 the return value import_r=%d export_id1=%d \n", import_r, export_id1);
	EXPECT_EQ(0, import_r);
	size_page1[0] = 'a';
	size_page1[4095] = 'b';
	printf("read the write date flag is 1: a and b size_page1[0]=%d  size_page1[4095]=%d\n", size_page1[0], size_page1[4095]);

	printf("Test_habmm_import Positive testing : size_bytes is 4096 and flag is 0x00010000\n");
	import_r = habmm_import(handle, (void **)&size_page2, 4096, export_id2, HABMM_EXPIMP_FLAGS_FD);
	printf("Test_habmm_import Positive testing size_bytes is 4096 and flag is 0x00010000 the return value import_r=%d export_id2=%d \n", import_r, export_id2);
	EXPECT_EQ(0, import_r);

	grant_fd1 = (unsigned long)size_page2;
	pvoid1 = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, grant_fd1, 0);
	if (MAP_FAILED == pvoid1)
	{
		printf("pvoid1 failed to mmap the size_bytes 4096\n");
	}
	pvoid1_1 = (char *)pvoid1;
	pvoid1_1[0] = 'c';
	pvoid1_1[4095] = 'd';
	printf("read the write date flag is 10000: c and d pvoid1_1[0]=%d  pvoid1_1[4095]=%d\n", pvoid1_1[0], pvoid1_1[4095]);

	printf("Test_habmm_import Positive testing : size_bytes is 4096 and flag is 0x00000000\n");
	import_r = habmm_import(handle, (void **)&size_page3, 4096, export_id3, 0x00000000);
	printf("Test_habmm_import Positive testing size_bytes is 4096 and flag is 0x00000000 the return value import_r=%d export_id3=%d \n", import_r, export_id3);
	EXPECT_EQ(0, import_r);
	size_page3[0] = 'e';
	size_page3[4095] = 'f';
	printf("read the write date flag is 0: e and f size_page3[0]=%d  size_page3[4095]=%d\n", size_page3[0], size_page3[4095]);

	printf("Test_habmm_import Positive testing : size_bytes is 4096 and flag is 0x00010001\n");
	import_r = habmm_import(handle, (void **)&size_page4, 4096, export_id4, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	printf("Test_habmm_import Positive testing size_bytes is 4096 and flag is 0x00010001 the return value import_r=%d export_id4=%d \n", import_r, export_id4);
	EXPECT_EQ(0, import_r);

	import_r = habmm_import(handle, (void **)&size_page5, 4096, export_id5, HABMM_IMP_FLAGS_FIXED);
	printf("Test_habmm_import Positive testing size_bytes is 4096 and flag is 0x00010000 the return value import_r=%d export_id5=%d \n", import_r, export_id5);
	EXPECT_EQ(0, import_r);

	grant_fd2 = (unsigned long)size_page4;
	pvoid2 = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, grant_fd2, 0);
	if (MAP_FAILED == pvoid2)
	{
		printf("pvoid2  failed to mmap the size_bytes 4096\n");
	}
	pvoid2_2 = (char *)pvoid2;
	pvoid2_2[0]='g';
	pvoid2_2[4095]='h';
	printf("read the write date flag is 10000: g and h pvoid2_2[0]=%d  pvoid2_2[4095]=%d\n", pvoid2_2[0], pvoid2_2[4095]);

	/* unimport the space */
	unimport_r = habmm_unimport(handle, export_id1, (void *)size_page1, HABMM_IMPORT_FLAGS_CACHED);
	printf("unimport the space  the return value unimport_r=%d export_id1=%d flag=0x00000001\n", unimport_r, export_id1);
	EXPECT_EQ(0, unimport_r);

	unimport_r = habmm_unimport(handle, export_id2, (void *)size_page2, HABMM_EXPIMP_FLAGS_FD);
	printf("unimport the space  the return value unimport_r=%d export_id2=%d flag=0x00010000\n", unimport_r, export_id2);
	EXPECT_EQ(0, unimport_r);

	unimport_r = habmm_unimport(handle, export_id3, (void *)size_page3, 0x00000000);
	printf("unimport the space  the return value unimport_r=%d export_id1=%d flag=0x00000000\n", unimport_r, export_id3);
	EXPECT_EQ(0, unimport_r);

	unimport_r = habmm_unimport(handle, export_id4, (void *)size_page4, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	printf("unimport the space  the return value unimport_r=%d export_id4=%d flag=0x00010001\n", unimport_r, export_id4);
	EXPECT_EQ(0, unimport_r);

	unimport_r = habmm_unimport(handle, export_id5, (void *)size_page5, HABMM_IMP_FLAGS_FIXED);
	EXPECT_EQ(0, unimport_r);

	/* Negative testing */
	/* 1.input invalid  size_bytes */
	printf("1.habmm_import Negative testing :input invalid  size_bytes  flag=0x00000001\n");
	import_r = habmm_import(handle, (void **)&size_10, 10, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("1.habmm_import Negative testing the return value import_r=%d  flag=0x00000001\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("1.habmm_import Negative testing :input invalid  size_bytes flag=0x00010000\n");
	import_r = habmm_import(handle, (void **)&size_10, 10, export_id2, HABMM_EXPIMP_FLAGS_FD);
	printf("1.habmm_import Negative testing the return value import_r=%d  flag=0x00010000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("1.habmm_import Negative testing :input invalid  size_bytes  flag=0x00000000\n");
	import_r = habmm_import(handle, (void **)&size_10, 10, export_id3, 0x00000000);
	printf("1.habmm_import Negative testing the return value import_r=%d  flag=0x00000000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("1.habmm_import Negative testing :input invalid  size_bytes flag=0x00010001\n");
	import_r = habmm_import(handle, (void **)&size_10, 10, export_id4, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	printf("1.habmm_import Negative testing the return value import_r=%d  flag=0x00010001\n", import_r);
	EXPECT_LT(import_r, 0);

	/* 2.input invalid  export_id */
	printf("2.habmm_import Negative testing :input invalid  export_id  flag=0x00000001\n");
	import_r = habmm_import(handle, (void **)&size_page, 4096, wrong_export_id, HABMM_IMPORT_FLAGS_CACHED);
	printf("2.habmm_import Negative testing the return value import_r=%d  flag=0x00000001\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("2.habmm_import Negative testing :input invalid  export_id flag=0x00010000\n");
	import_r = habmm_import(handle, (void **)&size_page, 4096, wrong_export_id, HABMM_EXPIMP_FLAGS_FD);
	printf("2.habmm_import Negative testing the return value import_r=%d  flag=0x00010000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("2.habmm_import Negative testing :input invalid  export_id  flag=0x00000000\n");
	import_r = habmm_import(handle, (void **)&size_page, 4096, wrong_export_id, 0x00000000);
	printf("2.habmm_import Negative testing the return value import_r=%d  flag=0x00000000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("2.habmm_import Negative testing :input invalid  export_id flag=0x00010001\n");
	import_r = habmm_import(handle, (void **)&size_page, 4096, wrong_export_id, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	printf("2.habmm_import Negative testing the return value import_r=%d  flag=0x00010001\n", import_r);
	EXPECT_LT(import_r, 0);

	import_r = habmm_import(handle, (void **)&size_page, 4096, wrong_export_id, HABMM_IMP_FLAGS_FIXED);
	printf("habmm_import Negative testing : invalid export id and flag is HABMM_IMP_FLAGS_FIXED, return value import_r=%d \n", import_r);
	EXPECT_LT(import_r, 0);

	/* 3.input invalid handle */
	printf("3.habmm_import Negative testing :input invalid handle  flag=0x00000001\n");
	import_r = habmm_import(0, (void **)&size_page, 4096, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("3.habmm_import Negative testing the return value import_r=%d  flag=0x00000001\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("3.habmm_import Negative testing :input invalid handle  flag=0x00010000\n");
	import_r = habmm_import(1, (void **)&size_page, 4096, export_id2, HABMM_EXPIMP_FLAGS_FD);
	printf("3.habmm_import Negative testing the return value import_r=%d  flag=0x00010000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("3.habmm_import Negative testing :input invalid handle  flag=0x00000000\n");
	import_r = habmm_import(1, (void **)&size_page, 4096, export_id3, 0x00000000);
	printf("3.habmm_import Negative testing the return value import_r=%d  flag=0x00000000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("3.habmm_import Negative testing :input invalid handle  flag=0x00010001\n");
	import_r = habmm_import(1, (void **)&size_page, 4096, export_id4, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	printf("3.habmm_import Negative testing the return value import_r=%d  flag=0x00010001\n", import_r);
	EXPECT_LT(import_r, 0);

	/* 4.buff_shared is null */
	printf("4.habmm_import Negative testing :buff_shared is null  flag=0x00000001\n");
	import_r = habmm_import(handle, NULL, 4096, export_id1, HABMM_IMPORT_FLAGS_CACHED);
	printf("4.habmm_import Negative testing the return value import_r=%d  flag=0x00000001\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("4.habmm_import Negative testing :buff_shared is null  flag=0x00010000\n");
	import_r = habmm_import(handle, NULL, 4096, export_id2, HABMM_EXPIMP_FLAGS_FD);
	EXPECT_LT(import_r, 0);
	printf("4.habmm_import Negative testing the return value import_r=%d  flag=0x00010000\n", import_r);

	printf("4.habmm_import Negative testing :buff_shared is null  flag=0x00000000\n");
	import_r = habmm_import(handle, NULL, 4096, export_id3, 0x00000000);
	printf("4.habmm_import Negative testing the return value import_r=%d  flag=0x00000000\n", import_r);
	EXPECT_LT(import_r, 0);

	printf("4.habmm_import Negative testing :buff_shared is null  flag=0x00010001\n");
	import_r = habmm_import(handle, NULL, 4096, export_id4, HABMM_EXPIMP_FLAGS_FD|HABMM_IMPORT_FLAGS_CACHED);
	EXPECT_LT(import_r, 0);
	printf("4.habmm_import Negative testing the return value import_r=%d  flag=0x00010001\n", import_r);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("habmm_import close  the return value close_r=%d  \n", close_r);

	/* testing for HAB_DEBUG version */
	if(0 != setenv("HAB_DEBUG", "1", 1))
		printf("Error setting HAB_DEBUG env! errno: %d (%s)\n", errno, strerror(errno));

	/* re-run some cases to cover UHAB_DBG branch to test DEBUG printing */
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	recv_r = habmm_socket_recv(handle, &export_id6, &size_export_id, 0, 0);
	EXPECT_EQ(0, recv_r);
	import_r = habmm_import(handle, (void **)&size_page6, 4096, export_id6, HABMM_IMPORT_FLAGS_CACHED);
	EXPECT_EQ(0, import_r);
	size_page6[0] = 'a';
	size_page6[4095] = 'b';
	unimport_r = habmm_unimport(handle, export_id6, (void *)size_page6, HABMM_IMPORT_FLAGS_CACHED);
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

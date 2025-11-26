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
TEST(UhabSocketOpenTest, UhabOpen){
	int32_t handle;
	int32_t handle2;
	int32_t open_r;
	int32_t close_r;

	/* Positive testing */
	printf("Test_uhabmm_socket_open Positive testing begin: valid mmid is 601\n");
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("uhabmm_socket_open Positive testing the return value open_r=%d handle=0x%x\n", open_r, handle);
	EXPECT_EQ(0, open_r);
	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_uhabmm_socket_open  the return value close_r=%d  \n", close_r);

	printf("Continuous open with same mmid, re-using fd and it should return 0\n", close_r);
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	open_r = habmm_socket_open(&handle2, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	close_r = habmm_socket_close(handle2);
	EXPECT_EQ(0, close_r);

	/* Negative testing */
	/* 1.input invalid mmid */
	printf("1.Test_uhabmm_socket_open Negative testing begin: invalid mmid is 603\n");
	open_r = habmm_socket_open(&handle, 603, 0, 0);
	printf("1.uhabmm_socket_open Negative testing the return value open_r=%d\n", open_r);
	EXPECT_LT(open_r, 0);

	/* 2.input handle is null */
	printf("2.Test_uhabmm_socket_open Negative testing begin: handle is null\n");
	open_r = habmm_socket_open(NULL, 601, 0, 0);
	printf("2.uhabmm_socket_open Negative testing the return value open_r=%d\n", open_r);
	EXPECT_LT(open_r, 0);

	/* 3.mmid exceed MM_ID_MAX */
	open_r = habmm_socket_open(&handle, MM_ID_MAX, 0, 0);
	printf("3.mmid set as MM_ID_MAX, return open handle =%d\n", open_r);
	EXPECT_EQ(open_r, -EINVAL);

	/* testing for HAB_DEBUG version */
	if(0 != setenv("HAB_DEBUG", "1", 1))
		printf("Error setting HAB_DEBUG env! errno: %d (%s)\n", errno, strerror(errno));

	/* re-run some cases to cover UHAB_DBG branch to test DEBUG printing */
	open_r = habmm_socket_open(&handle, 603, 0, 0);
	EXPECT_LT(open_r, 0);
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	open_r = habmm_socket_open(&handle2, 601, 0, 0);
	EXPECT_EQ(0, open_r);
	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	close_r = habmm_socket_close(handle2);
	EXPECT_EQ(0, close_r);

	unsetenv("HAB_DEBUG");
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	errno = 0;
	return RUN_ALL_TESTS();
}

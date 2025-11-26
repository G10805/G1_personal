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
TEST(UhabSocketCloseTest, UhabClose){
	int32_t handle;
	int32_t open_r;
	int32_t close_r;

	printf("Test_habmm_socket_close begin\n");
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_socket_close  the habmm_socket_open return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err;
	}

	/* Positive testing */
	printf("Test_habmm_socket_close Positive testing:\n");
	close_r = habmm_socket_close(handle);
	printf("Test_habmm_socket_close Positive testing  the return value close_r=%d \n", close_r);
	EXPECT_EQ(0, close_r);
	if(close_r == 0)
	{
		open_r = habmm_socket_open(&handle, 601, 0, 0);
	}

	/* Negative testing */
	/* 1.input invalid handle   */
	printf("1.Test_habmm_socket_close Negative testing :input invalid handle  \n");
	close_r = habmm_socket_close(112);
	printf("1.Test_habmm_socket_close Negative testing the return value close_r=%d  \n", close_r);
	EXPECT_LT(close_r, 0);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_socket_close  the return value close_r=%d  \n", close_r);

	/* 2.close twice   */
	close_r = habmm_socket_close(handle);
	printf("2.Test_habmm_socket_close Negative testing close twice the return value close_r=%d  \n", close_r);
	EXPECT_LT(close_r, 0);

	/* 3.vcid 0 is given */
	close_r = habmm_socket_close(0);
	printf("3.Test_habmm_socket_close vcid 0 is given, return value close_r=%d  \n", close_r);
	EXPECT_EQ(close_r, -EINVAL);

	err:
		printf("Can't open the mmid socket\n");
}
}  // namespace


int main(int argc, char **argv) {
	InitGoogleTest();
	return RUN_ALL_TESTS();
}

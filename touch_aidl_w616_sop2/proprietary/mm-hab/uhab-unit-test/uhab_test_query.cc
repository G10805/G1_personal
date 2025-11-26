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
TEST(UhabSocketQueryTest, UhabQuery){
	int32_t handle;
	int32_t open_r;
	int32_t query_r;
	int32_t close_r;
	struct hab_socket_info info;

	printf("Test_habmm_socket_query begin\n");
	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_socket_query  the habmm_socket_open return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err;
	}

	/* Positive testing */
	printf("Test_habmm_socket_query Positive testing:\n");
	query_r = habmm_socket_query(handle, &info, 1);
	printf("Test_habmm_socket_query Positive testing  the return value query_r=%d \n", query_r);
	EXPECT_EQ(0, query_r);

	printf("Test_habmm_socket_query hab_socket_info ： vmid local %d remote %d, vmname local %s remote %s\n",
		info.vmid_local, info.vmid_remote, info.vmname_local, info.vmname_remote);

	/* Negative testing */
	/* 1.input invalid handle   */
	printf("1.Test_habmm_socket_query Negative testing :input invalid handle  \n");
	query_r = habmm_socket_query(1, &info, 1);
	printf("1.Test_habmm_socket_query Negative testing the return value query_r=%d  \n", query_r);
	EXPECT_LT(query_r, 0);

	/* 2.input info is null   */
	printf("2.Test_habmm_socket_query Negative testing :input info is null  \n");
	query_r = habmm_socket_query(handle, NULL, 1);
	printf("2.Test_habmm_socket_query Negative testing the return value query_r=%d  \n", query_r);
	EXPECT_LT(query_r, 0);

	/* 3.input null handle   */
	query_r = habmm_socket_query(0, &info, 1);
	printf("3.Test_habmm_socket_query Negative testing :input null handle, return=%d \n", query_r);
	EXPECT_EQ(-EINVAL, query_r);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_socket_query  the return value close_r=%d  \n", close_r);

	err:
		printf("Can't open the mmid socket\n");
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	return RUN_ALL_TESTS();
}

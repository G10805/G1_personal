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
TEST(UhabSocketSendTest, UhabSend){
	int32_t handle;
	int32_t open_r;
	int32_t send_r;
	int32_t close_r;
	void* buff_p = malloc(65535);
	void* buff_n = malloc(65560);
	char size_1 = 'a';
	int32_t count = 10;

	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_socket_send begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		goto err;
	}

	/* Positive testing */
	printf("Test_habmm_socket_send Positive testing : size_bytes is 1 and flag is 0\n");
	send_r = habmm_socket_send(handle, &size_1, 1, 0);
	printf("Test_habmm_socket_send Positive testing size_bytes is 1 and flag is 0 the return value send_r=%d  \n", send_r);
	EXPECT_EQ(0, send_r);

	printf("Test_habmm_socket_send Positive testing : size_bytes is 65535 and flag is 0\n");
	send_r = habmm_socket_send(handle, buff_p, 65535, 0);
	printf("Test_habmm_socket_send Positive testing size_bytes is 65535 and flag is 0 the return value send_r=%d  \n", send_r);
	EXPECT_EQ(0, send_r);

	printf("Test_habmm_socket_send Positive testing : size_bytes is 1 and flag is HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING\n");
	count = 10;
	while (count--) {
		send_r = habmm_socket_send(handle, &size_1, 1, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
		if (send_r == -11)
			usleep(100);
		else
			break;
	}
	printf("Test_habmm_socket_send Positive testing size_bytes is 1 and flag is 1 the return value send_r=%d  \n", send_r);
	EXPECT_EQ(0, send_r);
	printf("Test_habmm_socket_send Positive testing : size_bytes is 65535 and flag is HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING\n");
	count = 10;
	while (count--) {
		send_r = habmm_socket_send(handle, buff_p, 65535, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
		if (send_r == -11)
			usleep(100);
		else
			break;
	}
	printf("Test_habmm_socket_send Positive testing size_bytes is 65535 and flag is 1 the return value send_r=%d  \n", send_r);
	EXPECT_EQ(0, send_r);

	/* Negative testing */
	/* 1.get invalid handle */
	printf("1.Test_habmm_socket_send Negative testing begin: input invalid handle flag is 1\n");
	send_r = habmm_socket_send(1, &size_1, 1, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
	printf("1.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 1\n", send_r);
	EXPECT_LT(send_r, 0);

	printf("1.Test_habmm_socket_send Negative testing begin: input invalid handle flag is 0\n");
	send_r = habmm_socket_send(1, &size_1, 1, 0);
	printf("1.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 0\n", send_r);
	EXPECT_LT(send_r, 0);

	/* 2.input src_buff is null */
	printf("2.Test_habmm_socket_send Negative testing : input src_buff is null  flag is 1\n");
	send_r = habmm_socket_send(handle, NULL, 1, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
	printf("2.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 1\n", send_r);
	EXPECT_LT(send_r, 0);

	printf("2.Test_habmm_socket_send Negative testing : input src_buff is null  flag is 0\n");
	send_r = habmm_socket_send(handle, NULL, 1, 0);
	printf("2.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 0\n", send_r);
	EXPECT_LT(send_r, 0);

	/* 3.input size is size_bytes exceed bound of UINT32_MAX */
	printf("3.Test_habmm_socket_send Negative testing : input size_bytes is out of bound flag is 1\n");
	send_r = habmm_socket_send(handle, buff_n, UINT32_MAX+1, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
	printf("3.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 1\n", send_r);
	EXPECT_LT(send_r, 0);

	printf("3.Test_habmm_socket_send Negative testing : input size_bytes is out of bound flag is 0\n");
	send_r = habmm_socket_send(handle, buff_n, UINT32_MAX+1, 0);
	printf("3.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 0\n", send_r);
	EXPECT_LT(send_r, 0);

	/* 4.input size is size_bytes is 0 */
	printf("4.Test_habmm_socket_send Negative testing : input size_bytes is 0 flag is 1\n");
	send_r = habmm_socket_send(handle, buff_n, 0, HABMM_SOCKET_SEND_FLAGS_NON_BLOCKING);
	printf("4.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 1\n", send_r);
	EXPECT_LT(send_r, 0);

	printf("4.Test_habmm_socket_send Negative testing : input size_bytes is 0 flag is 0\n");
	send_r = habmm_socket_send(handle, buff_n, 0, 0);
	printf("4.Test_habmm_socket_send Negative testing the return value send_r=%d  flag is 0\n", send_r);
	EXPECT_LT(send_r, 0);

	send_r = habmm_socket_send(0, buff_n, 1, 0);
	printf("5.vcid 0 is given, return value send_r=%d \n", send_r);
	EXPECT_EQ(send_r, -EINVAL);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("Test_habmm_socket_send  the return value close_r=%d  \n", close_r);

	printf("habmm_socket_sendto used for multi-FEs-to-single-BE mode, not yet implemented, full cover for it\n");
	send_r = habmm_socket_sendto(0,0,0,0,0);
	EXPECT_EQ(0, send_r);

	free(buff_p);
	free(buff_n);
	err:
		printf("Can't open the mmid socket\n");
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	return RUN_ALL_TESTS();
}

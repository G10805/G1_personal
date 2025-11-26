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
TEST(UhabSocketRecvTest, UhabRecv){
	int32_t handle;
	int32_t open_r;
	int32_t recv_r;
	int32_t close_r;
	char size_10[10] = "abcdefghi";
	char size_5[5] = "abcd";
	uint32_t right_size = 10;
	uint32_t wrong_size = 5;

	open_r = habmm_socket_open(&handle, 601, 0, 0);
	printf("Test_habmm_socket_recv begin: habmm_socket_open the return value open_r=%d handle=0x%x\n", open_r, handle);
	if(open_r < 0)
	{
		printf("Can't open the mmid socket\n");
		return;
	}

	/* Positive testing */
	printf("Test_habmm_socket_recv Positive testing : size_bytes is 10 and flag is 0\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, 1, 0);
	printf("Test_habmm_socket_recv Positive testing size_bytes is 10 and flag is 0 the return value recv_r=%d  \n", recv_r);
	EXPECT_EQ(0, recv_r);

	recv_r = -1;
	printf("Test_habmm_socket_recv Positive testing : size_bytes is 10 and flag is 1\n");
	while(1){
		recv_r = habmm_socket_recv(handle, &size_10, &right_size, 1, HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING);
		if(recv_r == 0)
			break;
		else if (recv_r == -EAGAIN) {
			sleep(1);
			right_size = 10;
		}
	}
	printf("Test_habmm_socket_recv Positive testing size_bytes is 10 and flag is 1 the return value recv_r=%d  \n", recv_r);
	EXPECT_EQ(0, recv_r);

	printf("habmm_socket_recv Positive testing : timeout value is 10*1000 and flag is HABMM_SOCKET_RECV_FLAGS_TIMEOUT\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, 10*1000, HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
	printf("Test_habmm_socket_recv Positive testing return value recv_r=%d  \n", recv_r);
	EXPECT_EQ(0, recv_r);

	printf("habmm_socket_recv Positive testing : timeout value is 30*1000 and flag is HABMM_SOCKET_RECV_FLAGS_UNINTERRUPTIBLE | HABMM_SOCKET_RECV_FLAGS_TIMEOUT\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, 30*1000, HABMM_SOCKET_RECV_FLAGS_UNINTERRUPTIBLE | HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
	printf("Test_habmm_socket_recv Positive testing return value recv_r=%d  \n", recv_r);
	EXPECT_EQ(0, recv_r);


	/* Negative testing */
	/* 1.Allocated bytes less than actual bytes */
	printf("1.habmm_socket_recv Negative testing : Alloced bytes less than actual bytes  flag is 0\n");
	recv_r = habmm_socket_recv(handle, size_5, &wrong_size, 1, 0);
	printf("1.habmm_socket_recv Negative testing the return value recv_r=%d   flag is 0\n", recv_r);
	EXPECT_LT(recv_r, 0);

	printf("1.habmm_socket_recv Negative testing : Alloced bytes less than actual bytes  flag is 1\n");
	while(1){
	recv_r = habmm_socket_recv(handle, size_5, &wrong_size, 1, HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING);
		if(recv_r < 0)
			break;
	}
	printf("1.habmm_socket_recv Negative testing the return value recv_r=%d   flag is 1\n", recv_r);
	EXPECT_LT(recv_r, 0);

	/* 2.dst_buff is null */
	printf("2.habmm_socket_recv Negative testing : dst_buff is null  flag is 0\n");
	recv_r = habmm_socket_recv(handle, NULL, &right_size, 1, 0);
	printf("2.habmm_socket_recv Negative testing the return value recv_r=%d  flag is 0 \n", recv_r);
	EXPECT_LT(recv_r, 0);
	printf("2.habmm_socket_recv Negative testing : dst_buff is null  flag is 1\n");
	while(1){
	recv_r = habmm_socket_recv(handle, NULL, &right_size, 1, HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING);
		if(recv_r<0)
			break;
	}
	printf("2.habmm_socket_recv Negative testing the return value recv_r=%d   flag is 1\n", recv_r);
	EXPECT_LT(recv_r, 0);

	/* 3.input invalid handle */
	printf("3.habmm_socket_recv Negative testing : input invalid handle flag is 0\n");
	recv_r = habmm_socket_recv(0, size_10, &right_size, 1, 0);
	printf("3.habmm_socket_recv Negative testing the return value recv_r=%d flag is 0 \n", recv_r);
	EXPECT_LT(recv_r, 0);
	recv_r = 2;
	printf("3.habmm_socket_recv Negative testing : input invalid handle flag is 1\n");
	while(1){
	recv_r = habmm_socket_recv(1, size_10, &right_size, 1, HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING);
		if(recv_r < 0)
			break;
	}
	printf("3.habmm_socket_recv Negative testing the return value recv_r=%d flag is 1 \n", recv_r);
	EXPECT_LT(recv_r, 0);

	/* 4.input timeout 0 */
	printf("4.habmm_socket_recv Negative testing : input timeout value is 0 and flag is HABMM_SOCKET_RECV_FLAGS_TIMEOUT\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, 0, HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
	printf("4.habmm_socket_recv Negative testing the return value recv_r=%d \n", recv_r);
	EXPECT_LT(recv_r, 0);

	/* 5.input timeout -1 */
	printf("5.habmm_socket_recv Negative testing : input timeout value is -1 flag is HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING | HABMM_SOCKET_RECV_FLAGS_TIMEOUT\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, -1, HABMM_SOCKET_RECV_FLAGS_NON_BLOCKING | HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
	printf("5.habmm_socket_recv Negative testing the return value recv_r=%d \n", recv_r);
	EXPECT_LT(recv_r, 0);

	printf("5.habmm_socket_recv Negative testing : timeout value is -1 and flag is HABMM_SOCKET_RECV_FLAGS_TIMEOUT\n");
	recv_r = habmm_socket_recv(handle, &size_10, &right_size, -1, HABMM_SOCKET_RECV_FLAGS_TIMEOUT);
	printf("5.Test_habmm_socket_recv Negative testing return value recv_r=%d  \n", recv_r);
	EXPECT_LT(recv_r, 0);

	close_r = habmm_socket_close(handle);
	EXPECT_EQ(0, close_r);
	printf("habmm_socket_recv  the return value close_r=%d  \n", close_r);

	printf("habmm_socket_recvfrom used for multi-FEs-to-single-BE mode, not yet implemented, full cover for it\n");
	recv_r = habmm_socket_recvfrom(0,0,0,0,0,0);
	EXPECT_EQ(0, recv_r);
}

}  // namespace

int main(int argc, char **argv) {
	InitGoogleTest();
	return RUN_ALL_TESTS();
}

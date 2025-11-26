/*
 **************************************************************************************************
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <gtest/gtest.h>
#include <inttypes.h>
#include "test_hyp_vpp.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    char *env_ptr = getenv("VPP_GTEST");
    vpp_test_debug_level = env_ptr ? atoi(env_ptr) : 0;

    return RUN_ALL_TESTS();
}


/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <gtest/gtest.h>
#include <C2PlatformSupport.h>

#include "QC2.h"

// Define tests with MOCK_ONLY or NON_MOCK_ONLY in case behavior depends on mock-up
//  NON_MOCK_ONLY - These are real tests with real encoder/decoder. Mockup is not
//                  (over)engineered to pass these tests
//  MOCK_ONLY - These are mainly negative-behavior tests that simulate controlled failures
//              and are not expected to work with real codecs.
#if HAVE_MOCK_CODEC
    #define MOCK_ONLY(qc2test) qc2test
    #define NON_MOCK_ONLY(qc2test) DISABLED_##qc2test
    #define MOCK_NAME(name) "mock." name
#else
    #define MOCK_ONLY(qc2test) DISABLED_##qc2test
    #define NON_MOCK_ONLY(qc2test) qc2test
    #define MOCK_NAME(name) name
#endif  // HAVE_MOCK_CODEC

#include "testQC2Filter.cpp"

using namespace android;

int main(int argc, char **argv) {
    updateLogLevel();
    ArgsParser::parse(argc, argv);

    // Route allocation specific magic through our store
    SetPreferredCodec2ComponentStore(QC2ComponentStore::Get());

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


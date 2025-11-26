/*
 **************************************************************************************************
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_OS_H_
#define _QC2_OS_H_

#include <string>

namespace qc2 {

/**
 * Group of OS utlities 
 * (mostly non-posix OS support goes in here)
 */
struct OS {
    static void SetThreadName(const std::string& name);

    static uint64_t CurrentTimeUs();

    struct SystemProperty {

        static std::string Get(const char *key, const char* defaultValue);

        static uint32_t GetUInt32(const char *key, uint32_t defaultValue, int base = 10);
    };

    static uint64_t GetUniqueIdForFileDescriptor(int fd);
};

};  // namespace qc2

#endif  //  _QC2_OS_H_

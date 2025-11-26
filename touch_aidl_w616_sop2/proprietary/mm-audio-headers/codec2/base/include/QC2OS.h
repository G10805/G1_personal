/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_OS_H_
#define _QC2AUDIO_OS_H_

#include <string>

namespace qc2audio {

/**
 * Group of OS utlities
 * (mostly non-posix OS support goes in here)
 */
struct OS {

    static void SetThreadName(const std::string& name);

    static uint64_t CurrentTimeUs();
};

};  // namespace qc2audio

#endif  //  _QC2AUDIO_OS_H_

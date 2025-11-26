/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_VERSION_H_
#define _QC2AUDIO_VERSION_H_

namespace qc2audio {

/// Build info
struct VersionInfo {
    /// build time and date
    static const char *date();
    /// top git commit
    static const char *commits();
    /// AU version
    static const char *baseline();
    /// host build machine and local build path
    static const char *host();
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_VERSION_H_

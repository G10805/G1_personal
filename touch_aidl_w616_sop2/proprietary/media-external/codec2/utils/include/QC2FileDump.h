/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_UTILS_FILE_DUMP_H_
#define _QC2_UTILS_FILE_DUMP_H_

#include <memory>
#include "QC2.h"
#include "QC2Buffer.h"

namespace qc2 {

/// @addtogroup utils Utilities
/// @{

/**
 * @brief Utility to dump (compressed and uncompressed) frames to file
 *
 * FileDump auto-manages the opening and closing file handles.
 * @note For Graphic buffers, will switch over to a new file in case of resolution/format change.
 */
class QC2FileDump {
 public:
    static constexpr const char * kMimeH263 = "video/3gpp";
    static constexpr const char * kMimeAVC = "video/avc";
    static constexpr const char * kMimeHEVC = "video/hevc";
    static constexpr const char * kMimeVP8 = "video/x-vnd.on2.vp8";
    static constexpr const char * kMimeVP9 = "video/x-vnd.on2.vp9";
    static constexpr const char * kMimeMPEG2 = "video/mpeg2";
    static constexpr const char * kMimeMPEG4 = "video/mp4v-es";
    static constexpr const char * kMimeHEIC = "image/vnd.android.heic";
    static constexpr const char * kMimeRAW = "video/raw";

    virtual ~QC2FileDump();

    /// version that auto-generates filename based on timestamp and component id
    static QC2Status Create(
            const std::string& mime,
            uint32_t id,
            const std::string& prefix,
            std::unique_ptr<QC2FileDump> *dump,
            bool forceUncompressed);

    /// version that dumps in given file path
    static QC2Status Create(
            const std::string& mime,
            const std::string& fileName,    // file-name with fully-qualified path
            std::unique_ptr<QC2FileDump> *dump);

    /// write-out a QC2Buffer (linear or graphic)
    QC2Status write(std::shared_ptr<QC2Buffer> buf);

    /// write-out a C2Buffer (linear or graphic)
    QC2Status write(std::shared_ptr<C2Buffer> buf);

    /// write-out a data of specified size
    QC2Status write(void *data, uint32_t size);

 private:
    class Impl;
    std::shared_ptr<Impl> mImpl;

    QC2FileDump(
        const std::string& mime, uint32_t id, bool autoFileName,
        const std::string& prefix, const std::string& fileName,
        bool forceUncompressed);
};

/// @}

};   // namespace qc2

#endif  // _QC2_UTILS_FILE_DUMP_H_

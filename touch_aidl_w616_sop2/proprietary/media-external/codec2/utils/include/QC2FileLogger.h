/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_UTILS_FILE_LOGGER_H_
#define _QC2_UTILS_FILE_LOGGER_H_

#include <memory>
#include <stdarg.h>
#include "QC2.h"

namespace qc2 {

/// @addtogroup utils Utilities
/// @{

/**
 * @brief Utility to log diagnostic/debug messages to file
 *
 * FileLogger auto-manages the opening and closing file handles.
 */
class QC2FileLogger {
 public:
    ~QC2FileLogger();

    /// Create a file-logger given the filename
    static QC2Status Create(
            const std::string& logFileName,  // file-name with fully-qualified path
            std::unique_ptr<QC2FileLogger> *fileLogger);

    /// write-out a message line
    QC2Status write(const char *format, ...);

 private:
    class Impl;
    std::unique_ptr<Impl> mImpl;

    QC2FileLogger();
};

// Convenience macros
#define QLOGF_D(logger, ...)                                        \
    do {                                                            \
        if (logger && (gC2FileLogLevel & QC2_FILE_LOG_DEBUG)) {     \
            logger->write(__VA_ARGS__);                             \
        }                                                           \
    } while (false)

#define QLOGF_I(logger, ...)                                        \
    do  {                                                           \
        if (logger && (gC2FileLogLevel & QC2_FILE_LOG_INFO)) {      \
            logger->write(__VA_ARGS__);                             \
        }                                                           \
    } while (false)

/// @}

};   // namespace qc2

#endif  // _QC2_UTILS_FILE_LOGGER_H_

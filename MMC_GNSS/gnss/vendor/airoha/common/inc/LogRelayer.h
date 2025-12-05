/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#ifndef LOGPACKER_H
#define LOGPACKER_H
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
namespace LogUtil {
const int MAX_PREFIX_SIZE = 10;
const int MAX_LOG_PATH_SIZE = 200;
const int MAX_TIME_STR_RAND_SIZE = 40;
const int MAX_LOG_FILE_NAME_SIZE =
    MAX_LOG_PATH_SIZE + MAX_PREFIX_SIZE + MAX_TIME_STR_RAND_SIZE;
const uint32_t LOG_READER_VERSION = 2;
const uint32_t INTEGRITY_CHECK_WORD = 0xDEADBEEF;
class LogSlotReader;
struct LogSlotItem;
struct LogSlotHeader;
class LogRelay;
}  // namespace LogUtil
/**
 * logSlotFile.slot
 * {currentSelectedIndex,timestamp}\n
 * {0,/tmp/logFile01,0}\n
 * {0,/tmp/logFile02,0}\n
 *
 */
struct LogUtil::LogSlotItem {
    uint32_t index;
    uint8_t logName[MAX_LOG_FILE_NAME_SIZE];
    uint8_t slotUsed;
    uint8_t reserved1;
    uint8_t reserved2;
} __attribute__((packed));
struct LogUtil::LogSlotHeader {
    uint32_t version;
    uint32_t currentSelectedIndex;
    int64_t timestamp;
    uint32_t fileCount;
    uint8_t logDirPath[MAX_LOG_PATH_SIZE];
    uint32_t reserved1;
} __attribute__((packed));
class LogUtil::LogSlotReader {
 public:
    enum ErrorCode : uint32_t {
        ERR_CODE_NO_ERROR = 0,
        ERR_CODE_SLOT_FILE_ERROR = 1 << 0,
        ERR_CODE_SLOT_UNLINK_FILE_ERROR = 1 << 1,
        ERR_CODE_SLOT_FILE_NOT_EXIST = 1 << 2,
        ERR_CODE_SLOT_SYSTEM_FATAL_DETECT = 1 << 29,
    };
    LogSlotReader(const char *logSlotFile, const char *logFilePath, int);
    LogSlotReader();
    ~LogSlotReader();
    bool setSlotFile(const char *logSlotFile);
    bool pushLogFile(const char *fileName);
    void outputAllLog(FILE *file, std::vector<std::string> &fileList);
    void outputAllLog(FILE *file, const char *absPath);
    void outputAllLog(FILE *file);
    uint32_t error();
    static bool slotIntegrityCheck(const char *filename, size_t maxFileNum);
    const char *latestFilename();

 private:
    bool closeSlotFile();
    bool openSlotFile();
    bool array2SlotFile();
    bool slotFile2Array();
    bool isFileExist(const char *fileName);
    void initSlotArray();
    void getFileList(std::vector<std::string> &fileList);
    void getFileList(const char *absPath, std::vector<std::string> &fileList);
    int64_t generateTimestamp();
    bool catLogFile(FILE *dstFile, FILE *srcFile);
    uint32_t begin();
    uint32_t next(uint32_t current);
    struct LogSlotItem *slotItemArray;
    struct LogSlotHeader slotHeader;
    char slotFileName[MAX_LOG_FILE_NAME_SIZE];
    char logDataPathName[MAX_LOG_PATH_SIZE];
    size_t maxFileNum;
    FILE *slotFileFd;
    uint32_t mError;
};
/**
 * Log Relayer to save log.
 * thread-safe: yes
 */
class LogUtil::LogRelay {
 public:
    LogRelay(const char *userLogPath, const char *userPrefix, size_t maxFileNum,
             size_t maxSingleFileSize);
    ~LogRelay();
    size_t relayLog(const void *data, size_t length);
    void startLog();
    void stopLog();
    void restartLog();
    // Trace Support
    static void relayTrace(const char *format, ...);
    static void relayEvent(const char *format, ...);
    static void relayTraceBinary(const void *data, uint32_t length);
    static void initTrace(const char *path, int fileNum, int fileSize);
    static void initEventLog(const char *path, int fileNum, int fileSize);
    static void deinitEventLog();
    static void deinitTrace();
    // SD Card Log Support
    static void relaySdLog(const char *format, ...);
    static bool initSdLog();
    static bool deinitSdLog();
    static void restartTrace();
    static void restartEvent();

 private:
    char prefix[MAX_PREFIX_SIZE];
    char logPath[MAX_LOG_PATH_SIZE];
    char mUserDebugSlot[MAX_LOG_FILE_NAME_SIZE];
    size_t mMaxFileNum;
    size_t mMaxSingleFileSize;
    std::unique_ptr<LogSlotReader> logSlot;
    FILE *currentFile;
    size_t writtenCharsNum;
    size_t singleFileThreshold;
    void createNewLogFile();
    bool createFileWithLatest();
    void clearLogFiles();
    size_t getLogNum();
    // Event Support
    // Trace Support
    static LogRelay *eventLogRelayer;
    static std::mutex eventMutex;
    // Trace Support
    static LogRelay *traceLogRelayer;
    static std::mutex traceMutex;
    // SD Support
    static LogRelay *sSdLogRelayer;
    static std::mutex sSdLogMutex;
    bool mLogErrorOccur;
};
#endif  // LOGPACKER_H

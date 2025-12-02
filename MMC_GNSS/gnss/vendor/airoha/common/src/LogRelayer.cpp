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
#define LOG_TAG "LR"
#include "LogRelayer.h"
#include <assert.h>
#include <dirent.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <memory>
#include "algorithm/inc/crc_cal.h"
#include "simulation.h"
#include "time.h"
// Log file permission 0660
#define LOG_FILE_PERMISSION (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
using LogUtil::LogRelay;
using LogUtil::LogSlotItem;
using LogUtil::LogSlotReader;
#define LR_LOG_D(format, ...) LOG_D(format, ##__VA_ARGS__);
#define LR_LOG_I(format, ...) LOG_I(format, ##__VA_ARGS__);
#define LR_LOG_W(format, ...) LOG_W(format, ##__VA_ARGS__);
#define LR_LOG_E(format, ...) LOG_E(format, ##__VA_ARGS__);
LogRelay *LogRelay::traceLogRelayer = nullptr;
LogRelay *LogRelay::sSdLogRelayer = nullptr;
LogRelay *LogRelay::eventLogRelayer = nullptr;
std::mutex LogRelay::sSdLogMutex;
std::mutex LogRelay::traceMutex;
std::mutex LogRelay::eventMutex;
LogSlotReader::LogSlotReader(const char *logSlotFile, const char *logFilePath,
                             int maxLogFileNum) {
    slotFileFd = nullptr;
    slotItemArray = nullptr;
    mError = 0;
    LR_LOG_D("LogSlotReader");
    snprintf(slotFileName, sizeof(slotFileName), "%s/%s", logFilePath,
             logSlotFile);
    strncpy(logDataPathName, logFilePath, MAX_LOG_PATH_SIZE);
    LR_LOG_I("Slot File %s exist:%d,data_dir:%s", slotFileName,
             isFileExist(slotFileName), logDataPathName);
    maxFileNum = maxLogFileNum;
    if (isFileExist(slotFileName)) {
        if (!LogSlotReader::slotIntegrityCheck(slotFileName, maxLogFileNum)) {
            mError |= ERR_CODE_SLOT_FILE_ERROR;
            if (unlink(slotFileName) != 0) {
                LR_LOG_E("unlink file error!!!");
                mError |= ERR_CODE_SLOT_UNLINK_FILE_ERROR;
                mError |= ERR_CODE_SLOT_SYSTEM_FATAL_DETECT;
            }
        }
        if (mError & ERR_CODE_SLOT_SYSTEM_FATAL_DETECT) {
            // when system fatal error detected, not save log
            return;
        } else if (mError & ERR_CODE_SLOT_FILE_ERROR) {
            initSlotArray();
            array2SlotFile();
        } else {
            slotFile2Array();
        }
    } else {
        mError |= ERR_CODE_SLOT_FILE_NOT_EXIST;
        initSlotArray();
        array2SlotFile();
    }
    chmod(slotFileName, LOG_FILE_PERMISSION);
#ifndef AIROHA_SIMULATION
    // 1021 = gps, 1000 = system
    chown(slotFileName, 1021, 1000);
#endif
}
LogSlotReader::LogSlotReader() {
    slotFileFd = nullptr;
    slotItemArray = nullptr;
}
bool LogSlotReader::setSlotFile(const char *logSlotFile) {
    strncpy(slotFileName, logSlotFile, MAX_LOG_FILE_NAME_SIZE);
    strncpy(slotFileName, logSlotFile, MAX_LOG_FILE_NAME_SIZE);
    if (isFileExist(logSlotFile)) {
        slotFile2Array();
        return true;
    } else {
        return false;
    }
}
LogSlotReader::~LogSlotReader() {
    LR_LOG_D("slot reader close..");
    if (slotItemArray) {
        delete[](slotItemArray);
    }
    LR_LOG_D("slot reader finish..");
}
bool LogSlotReader::isFileExist(const char *fileName) {
    FILE *file = fopen(fileName, "r");
    bool fileExist = false;
    if (file) {
        fileExist = true;
        fclose(file);
    } else {
        fileExist = false;
    }
    return fileExist;
}
bool LogSlotReader::array2SlotFile() {
    slotFileFd = fopen(slotFileName, "wb+");
    if (slotFileFd == nullptr) {
        LR_LOG_E("open slot file error: %d", errno);
        mError |= ERR_CODE_SLOT_SYSTEM_FATAL_DETECT;
        return false;
    }
    fwrite(&slotHeader, 1, sizeof(slotHeader), slotFileFd);
    fwrite(slotItemArray, 1, maxFileNum * sizeof(LogSlotItem), slotFileFd);
    algorithm::CrcCalculator<uint32_t> crcInstance(0x04C11DB7, 0xFFFFFFFF,
                                                   0xFFFFFFFF, true, true);
    uint32_t crc32v = crcInstance.calculateCrc(&slotHeader, sizeof(slotHeader),
                                               crcInstance.getInitialValue());
    crc32v = crcInstance.calculateCrc(slotItemArray,
                                      maxFileNum * sizeof(LogSlotItem), crc32v);
    fwrite(&crc32v, 1, sizeof(crc32v), slotFileFd);
    fflush(slotFileFd);
    fclose(slotFileFd);
    return true;
}
bool LogSlotReader::slotFile2Array() {
    slotFileFd = fopen(slotFileName, "rb");
    assert(slotFileFd);
    fread(&slotHeader, 1, sizeof(slotHeader), slotFileFd);
    LR_LOG_D("%s %p", __FUNCTION__, slotItemArray);
    if (!slotItemArray) {
        LR_LOG_D("create slotArray %d", slotHeader.fileCount);
        slotItemArray = new LogSlotItem[slotHeader.fileCount];
    }
    strncpy(logDataPathName, (char *)slotHeader.logDirPath, MAX_LOG_PATH_SIZE);
    maxFileNum = slotHeader.fileCount;
    fread(slotItemArray, 1, maxFileNum * sizeof(LogSlotItem), slotFileFd);
    fclose(slotFileFd);
    return true;
}
bool LogSlotReader::pushLogFile(const char *fileName) {
    if (slotItemArray[slotHeader.currentSelectedIndex].slotUsed == false) {
        // do nothing
    } else {
        if (slotHeader.currentSelectedIndex == maxFileNum - 1) {
            slotHeader.currentSelectedIndex = 0;
        } else {
            slotHeader.currentSelectedIndex =
                slotHeader.currentSelectedIndex + 1;
        }
    }
    // delete file
    //    if()
    //    strncpy(&(slotItemArray[slotHeader.currentSelectedIndex].logName),fileName,
    //    )
    LR_LOG_D("Current Index %d", slotHeader.currentSelectedIndex);
    if (slotItemArray[slotHeader.currentSelectedIndex].slotUsed) {
        // delete logfile if exist
        char totalFile[MAX_LOG_FILE_NAME_SIZE] = {0};
        int ret = snprintf(
            totalFile, MAX_LOG_FILE_NAME_SIZE, "%s/%s", logDataPathName,
            (const char *)slotItemArray[slotHeader.currentSelectedIndex]
                .logName);
        if (ret >= MAX_LOG_FILE_NAME_SIZE) {
            LR_LOG_E("filename maybe truncated");
        }
        int deleteRet = unlink(totalFile);
        LR_LOG_D("delete expire file: %s, errno:%d, %d(%s)", totalFile,
                 deleteRet, errno, strerror(errno));
    }
    strncpy((char *)slotItemArray[slotHeader.currentSelectedIndex].logName,
            fileName, MAX_LOG_FILE_NAME_SIZE - 1);
    slotItemArray[slotHeader.currentSelectedIndex].slotUsed = true;
    return array2SlotFile();
}
void LogSlotReader::initSlotArray() {
    slotHeader.version = LOG_READER_VERSION;
    slotHeader.timestamp = generateTimestamp();
    slotHeader.currentSelectedIndex = 0;
    slotHeader.fileCount = maxFileNum;
    slotHeader.reserved1 = INTEGRITY_CHECK_WORD;
    strncpy((char *)slotHeader.logDirPath, logDataPathName,
            sizeof(slotHeader.logDirPath));
#ifdef __ANDROID_OS__
    LR_LOG_D("init slot array, %zu", maxFileNum);
#else
    LR_LOG_D("init slot array, %d", maxFileNum);
#endif
    slotItemArray = new LogSlotItem[maxFileNum];
    for (size_t i = 0; i < maxFileNum; i++) {
        slotItemArray[i].index = (uint32_t)i;
        memset(slotItemArray[i].logName, 0, sizeof(slotItemArray[i].logName));
        slotItemArray[i].slotUsed = false;
        slotItemArray[i].reserved1 = 0xFF;
        slotItemArray[i].reserved2 = 0xFF;
    }
}
int64_t LogSlotReader::generateTimestamp() {
    time_t t = 0;
    return time(&t);
}
void LogSlotReader::outputAllLog(FILE *dstFile,
                                 std::vector<std::string> &fileList) {
    assert(dstFile);
    FILE *srcFile = nullptr;
    std::vector<std::string>::iterator it;
    for (it = fileList.begin(); it != fileList.end(); it++) {
        srcFile = fopen(it->c_str(), "rb");
        if (srcFile == nullptr) {
            LR_LOG_E("log file not found:%s", it->c_str());
            continue;
        }
        LR_LOG_D("copy %s", it->c_str());
        catLogFile(dstFile, srcFile);
        fclose(srcFile);
    }
}
void LogSlotReader::outputAllLog(FILE *file, const char *absPath) {
    std::vector<std::string> fileList;
    LR_LOG_D("output all log,%p,%s", file, absPath);
    getFileList(absPath, fileList);
    outputAllLog(file, fileList);
};
void LogSlotReader::getFileList(std::vector<std::string> &fileList) {
    uint32_t index = begin();
    bool round = false;
    LR_LOG_D("=== Pack File List===");
    while (slotItemArray[index].slotUsed &&
           (round == false || index != begin())) {
        round = true;
        char totalFile[MAX_LOG_FILE_NAME_SIZE] = {0};
        int ret = snprintf(totalFile, MAX_LOG_FILE_NAME_SIZE, "%s/%s",
                           logDataPathName,
                           (const char *)slotItemArray[index].logName);
        if (ret >= MAX_LOG_FILE_NAME_SIZE) {
            LR_LOG_E("file truncated");
        }
        fileList.push_back(totalFile);
        LR_LOG_D("%s packed", totalFile);
        index = next(index);
    }
    LR_LOG_D("=== Pack File List End===");
}
void LogSlotReader::getFileList(const char *absPath,
                                std::vector<std::string> &fileList) {
    uint32_t index = begin();
    bool round = false;
    LR_LOG_D("=== Pack File List===");
    while (slotItemArray[index].slotUsed &&
           (round == false || index != begin())) {
        round = true;
        char totalFile[MAX_LOG_FILE_NAME_SIZE] = {0};
        int ret = snprintf(totalFile, MAX_LOG_FILE_NAME_SIZE, "%s/%s", absPath,
                           (const char *)slotItemArray[index].logName);
        if (ret >= MAX_LOG_FILE_NAME_SIZE) {
            LR_LOG_E("=== Filename truncated %d===", __LINE__);
        }
        fileList.push_back(totalFile);
        LR_LOG_D("%s packed", totalFile);
        index = next(index);
    }
    LR_LOG_D("=== Pack File List End===");
}
void LogSlotReader::outputAllLog(FILE *file) {
    std::vector<std::string> fileList;
    getFileList(fileList);
    outputAllLog(file, fileList);
}
uint32_t LogSlotReader::error() { return mError; }
uint32_t LogSlotReader::begin() {
    uint32_t needCheckIndex = next(slotHeader.currentSelectedIndex);
    if (slotItemArray[needCheckIndex].slotUsed) {
        return needCheckIndex;
    }
    return 0;
}
uint32_t LogSlotReader::next(uint32_t current) {
    if (current == maxFileNum - 1) {
        return 0;
    }
    return current + 1;
}
bool LogSlotReader::catLogFile(FILE *dstFile, FILE *srcFile) {
    assert(dstFile && srcFile);
    size_t readCount;
    size_t readStep = 1024;
    char buffer[1024];
    while ((readCount = fread(buffer, 1, readStep, srcFile)) > 0) {
        fwrite(buffer, 1, readCount, dstFile);
        fflush(dstFile);
    }
    return true;
}
const char * LogSlotReader::latestFilename() {
    if (slotItemArray[slotHeader.currentSelectedIndex].logName[0] == 0) {
        return nullptr;
    }
    return (const char *)slotItemArray[slotHeader.currentSelectedIndex].logName;
}
LogRelay::LogRelay(const char *userLogPath, const char *userPrefix,
                   size_t maxFileNum, size_t maxSingleFileSize) {
    mLogErrorOccur = false;
    snprintf(mUserDebugSlot, sizeof(mUserDebugSlot), "%s_%s", userPrefix,
             "debug.slot");
    strncpy(logPath, userLogPath, MAX_LOG_PATH_SIZE);
    strncpy(prefix, userPrefix, MAX_PREFIX_SIZE - 1);
    mMaxFileNum = maxFileNum;
    mMaxSingleFileSize = maxSingleFileSize;
    // because some error before
    if (getLogNum() > maxFileNum) {
        LR_LOG_E("logs file too much!!!");
        clearLogFiles();
    }
    startLog();
}
LogRelay::~LogRelay() { stopLog(); }
size_t LogRelay::relayLog(const void *data, size_t length) {
#ifndef __ANDROID_OS__
    // LR_LOG_D("relay log %p,%d", data, length);
#else
    // LR_LOG_D("relay log %p,%zu", data, length);
#endif
    if (mLogErrorOccur) {
        return 0;
    }
    if (writtenCharsNum + length > singleFileThreshold) {
        createNewLogFile();
        writtenCharsNum = 0;
    }
    if (currentFile == nullptr) {
        return 0;
    }
    fwrite(data, 1, length, currentFile);
    fflush(currentFile);
    writtenCharsNum += length;
    return length;
}
void LogRelay::startLog() {
    // because some error before
    // logSlot = new LogSlotReader(mUserDebugSlot, logPath, mMaxFileNum);
    logSlot =
        std::make_unique<LogSlotReader>(mUserDebugSlot, logPath, mMaxFileNum);
    singleFileThreshold = mMaxSingleFileSize;
    writtenCharsNum = 0;
    currentFile = nullptr;
    if ((logSlot->error() & LogSlotReader::ERR_CODE_SLOT_FILE_ERROR) ||
        (logSlot->error() & LogSlotReader::ERR_CODE_SLOT_FILE_NOT_EXIST)) {
        clearLogFiles();
    }
    if (logSlot->error() & LogSlotReader::ERR_CODE_SLOT_SYSTEM_FATAL_DETECT) {
        mLogErrorOccur = true;
        return;
    }
    if (!createFileWithLatest()) {
        mLogErrorOccur = true;
    }
}
void LogRelay::stopLog() {
    if (currentFile) {
        fclose(currentFile);
        currentFile = nullptr;
    }
    logSlot = nullptr;
}
void LogRelay::restartLog() {
    stopLog();
    startLog();
}
void LogRelay::createNewLogFile() {
    char singleFileName[MAX_PREFIX_SIZE + MAX_TIME_STR_RAND_SIZE] = {0};
    char totalFilename[MAX_LOG_FILE_NAME_SIZE] = {0};
    char fileNameWithDuplicate[MAX_LOG_FILE_NAME_SIZE] = {0};
    char randStr[5] = {0};
    int duplicateNum = 0;
    struct tm timestu;
    time_t t = time(nullptr);
    for (size_t i = 0; i < 4; i++) {
        randStr[i] = rand() % 9 + '0';
    }
    LR_LOG_D("rand str %s", randStr);
    gmtime_r(&t, &timestu);
    snprintf(singleFileName, sizeof(singleFileName),
             "%s_%d_%02d_%02d_%02d%02d%02d_%s.log", prefix,
             timestu.tm_year + 1900, timestu.tm_mon + 1, timestu.tm_mday,
             timestu.tm_hour, timestu.tm_min, timestu.tm_sec, randStr);
    FILE *checkFile = NULL;
    while (1) {
        snprintf(totalFilename, sizeof(totalFilename), "%s/%s_%d", logPath,
                 singleFileName, duplicateNum);
        checkFile = fopen(totalFilename, "r");
        if (checkFile == NULL) {
            break;
        } else {
            LOG_D("LogFile Exist(%s), regen..", totalFilename);
            fclose(checkFile);
            duplicateNum++;
        }
    }
    snprintf(fileNameWithDuplicate, sizeof(fileNameWithDuplicate), "%s_%d",
             singleFileName, duplicateNum);
    // snprintf(totalFilename, sizeof(totalFilename), "%s/%s",
    //          logPath,
    //          singleFileName
    //          );
    if (currentFile) {
        fclose(currentFile);
    }
    LR_LOG_I("new log file:%s", totalFilename);
    currentFile = fopen(totalFilename, "wb+");
    // do not need assert
    // assert(currentFile);
    if (currentFile == nullptr) {
        LR_LOG_E("!!log maybe drop because create fd failed!!");
        mLogErrorOccur = true;
        return;
    }
    chmod(totalFilename, LOG_FILE_PERMISSION);
#ifndef AIROHA_SIMULATION
    // 1021: gps, 1000: system
    chown(totalFilename, 1021, 1000);
#endif
    if (!logSlot->pushLogFile(fileNameWithDuplicate)) {
        LR_LOG_E("push log file error");
        if (logSlot->error() &
            LogSlotReader::ERR_CODE_SLOT_SYSTEM_FATAL_DETECT) {
            LR_LOG_E("detect slot fatal error, stop logging..!!");
            mLogErrorOccur = true;
        }
    }
}
bool LogRelay::createFileWithLatest() {
    const char *latestFile = logSlot->latestFilename();
    if (latestFile == nullptr) {
        LR_LOG_E("no latest file, create one");
        createNewLogFile();
        return true;
    }
    char totalFilename[MAX_LOG_FILE_NAME_SIZE] = {0};
    snprintf(totalFilename, sizeof(totalFilename), "%s/%s", logPath,
                latestFile);
    currentFile = fopen(totalFilename, "rb+");
    if (!currentFile) {
        LR_LOG_E("open file %s error", totalFilename);
        return false;
    }
    fseek(currentFile, 0, SEEK_END);
    writtenCharsNum = ftell(currentFile);
    return true;
}
void LogRelay::clearLogFiles() {
    LR_LOG_D("clear log file..");
    DIR *dir;
    struct dirent *ptr;
    dir = opendir(logPath);
    if (dir != NULL) {
        while ((ptr = readdir(dir)) != NULL) {
            if ((strcmp(ptr->d_name, ".") == 0) ||
                (strcmp(ptr->d_name, "..") == 0)) {
                LR_LOG_D("clearLogFiles Skip %s", ptr->d_name);
                continue;
            } else if (strstr(ptr->d_name, "debug.slot") != NULL) {
                LR_LOG_D("clearLogFiles Skip %s", ptr->d_name);
                continue;
            } else if (ptr->d_type == DT_REG &&
                       strstr(ptr->d_name, prefix) !=
                           NULL) {  // delete file which name contain specific
                                    // prefix
                char full[MAX_LOG_FILE_NAME_SIZE] = {0};
                int ret = snprintf(full, MAX_LOG_FILE_NAME_SIZE, "%s/%s",
                                   logPath, ptr->d_name);
                if (ret < MAX_LOG_FILE_NAME_SIZE) {
                    LR_LOG_D("clearLogFiles Delete %s, %s", full, ptr->d_name);
                    if (unlink(full) != 0) {
                        LR_LOG_E("clearLogFiles unlink error %d", errno);
                    }
                }
            }
        }
        closedir(dir);
    }
    LR_LOG_D("clear log done");
}
size_t LogRelay::getLogNum() {
    LR_LOG_D("get log num");
    DIR *dir;
    struct dirent *ptr;
    size_t logNum = 0;
    dir = opendir(logPath);
    if (dir != NULL) {
        while ((ptr = readdir(dir)) != NULL) {
            if ((strcmp(ptr->d_name, ".") == 0) ||
                (strcmp(ptr->d_name, "..") == 0)) {
                LR_LOG_D("get log num Skip %s", ptr->d_name);
                continue;
            } else if (strstr(ptr->d_name, "debug.slot") != NULL) {
                LR_LOG_D("get log num Skip %s", ptr->d_name);
                continue;
            } else if (ptr->d_type == DT_REG &&
                       strstr(ptr->d_name, prefix) != NULL) {
                logNum++;
            }
        }
        closedir(dir);
    }
    LR_LOG_D("get log num:%zu", logNum);
    return logNum;
}
void LogRelay::relayTrace(const char *format, ...) {
    // avoid call this then deinit
    std::lock_guard<std::mutex> locker(traceMutex);
    if (!traceLogRelayer) return;
    char timeStr[35] = {0};
    char logStr[1030];
    struct tm local;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    // INT_LOG("%ld", time(0));
    // INT_LOG("%ld", spec.tv_sec);
    localtime_r(&(spec.tv_sec), &local);
    int year = local.tm_year + 1900;
    int month = local.tm_mon + 1;
    int day = local.tm_mday;
    int hour = local.tm_hour;
    int min = local.tm_min;
    int sec = local.tm_sec;
    snprintf(timeStr, 34, "%d-%02d-%02d %02d-%02d-%02d.%03ld[%s]", year, month,
             day, hour, min, sec, spec.tv_nsec / 1000000, local.tm_zone);
    // snprintf(timeStr,29,"%d",local.tm_mday);
    char output[1600];
    va_list ap;
    va_start(ap, format);
    vsnprintf(logStr, 1020, format, ap);
    va_end(ap);
    snprintf(output, 1600, "%s %s\n", timeStr, logStr);
    traceLogRelayer->relayLog(output, strlen(output));
    return;
}
void LogRelay::relayEvent(const char *format, ...) {
    // avoid call this then deinit
    std::lock_guard<std::mutex> locker(eventMutex);
    if (!eventLogRelayer) return;
    char timeStr[35] = {0};
    char logStr[1030];
    struct tm local;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    // INT_LOG("%ld", time(0));
    // INT_LOG("%ld", spec.tv_sec);
    localtime_r(&(spec.tv_sec), &local);
    int year = local.tm_year + 1900;
    int month = local.tm_mon + 1;
    int day = local.tm_mday;
    int hour = local.tm_hour;
    int min = local.tm_min;
    int sec = local.tm_sec;
    snprintf(timeStr, 34, "%d-%02d-%02d %02d-%02d-%02d.%03ld[%s]", year, month,
             day, hour, min, sec, spec.tv_nsec / 1000000, local.tm_zone);
    // snprintf(timeStr,29,"%d",local.tm_mday);
    char output[1600];
    va_list ap;
    va_start(ap, format);
    vsnprintf(logStr, 1020, format, ap);
    va_end(ap);
    snprintf(output, 1600, "%s %s\n", timeStr, logStr);
    eventLogRelayer->relayLog(output, strlen(output));
    return;
}
void LogRelay::relayTraceBinary(const void *data, uint32_t length) {
    // avoid call this then deinit
    std::lock_guard<std::mutex> locker(traceMutex);
    if (!traceLogRelayer) return;
    char timeStr[64] = {0};
    const char *start = "===== START =====";
    const char *end = "===== END =====";
    struct tm local;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    // INT_LOG("%ld", time(0));
    // INT_LOG("%ld", spec.tv_sec);
    localtime_r(&(spec.tv_sec), &local);
    int year = local.tm_year + 1900;
    int month = local.tm_mon + 1;
    int day = local.tm_mday;
    int hour = local.tm_hour;
    int min = local.tm_min;
    int sec = local.tm_sec;
    snprintf(timeStr, sizeof(timeStr),
             "%d-%02d-%02d %02d-%02d-%02d.%03ld[%s] %s\n", year, month, day,
             hour, min, sec, spec.tv_nsec / 1000000, local.tm_zone, start);
    // snprintf(timeStr,29,"%d",local.tm_mday);
    traceLogRelayer->relayLog(timeStr, strlen(timeStr));
    traceLogRelayer->relayLog(data, length);
    snprintf(timeStr, sizeof(timeStr),
             "%d-%02d-%02d %02d-%02d-%02d.%03ld[%s] %s\n", year, month, day,
             hour, min, sec, spec.tv_nsec / 1000000, local.tm_zone, end);
    traceLogRelayer->relayLog(timeStr, strlen(timeStr));
    return;
}
void LogRelay::initTrace(const char *path, int fileNum, int fileSize) {
    if (traceLogRelayer) {
        delete (traceLogRelayer);
    }
    LR_LOG_E("=====Init Trace=====");
#ifndef AIROHA_SIMULATION
    traceLogRelayer = new LogRelay(path, "trace_r_", fileNum, fileSize);
#else
    traceLogRelayer = new LogRelay("./log/", "trace_r_", fileNum, fileSize);
#endif
}
void LogRelay::initEventLog(const char *path, int fileNum, int fileSize) {
    if (eventLogRelayer) {
        delete (eventLogRelayer);
    }
    LR_LOG_E("=====Init Trace=====");
#ifndef AIROHA_SIMULATION
    eventLogRelayer = new LogRelay(path, "event_r_", fileNum, fileSize);
#else
    eventLogRelayer = new LogRelay("./log/", "event_r_", fileNum, fileSize);
#endif
}
void LogRelay::deinitEventLog() {
    std::lock_guard<std::mutex> locker(eventMutex);
    LR_LOG_E("=====deinit Event (%p)=====", eventLogRelayer);
    if (eventLogRelayer) {
        delete (eventLogRelayer);
        eventLogRelayer = nullptr;
    }
}
void LogRelay::deinitTrace() {
    std::lock_guard<std::mutex> locker(traceMutex);
    LR_LOG_E("=====deinit Trace (%p)=====", traceLogRelayer);
    if (traceLogRelayer) {
        delete (traceLogRelayer);
        traceLogRelayer = nullptr;
    }
}
// For Internal Use
#define SD_CARD_DIR "/storage/"
#define ANLD_SDLOG_BUFFER_LEN 2048
#define ANLD_SDLOG_HEADER_LEN 64
void LogRelay::relaySdLog(const char *format, ...) {
    sSdLogMutex.lock();
    if (sSdLogRelayer == nullptr) {
        sSdLogMutex.unlock();
        return;
    }
    sSdLogMutex.unlock();
    char buf[ANLD_SDLOG_BUFFER_LEN];
    char header[ANLD_SDLOG_HEADER_LEN];
    va_list ap;
    va_start(ap, format);
    int ret_buf = vsnprintf(buf, ANLD_SDLOG_BUFFER_LEN, format, ap);
    va_end(ap);
    if ((ret_buf > 0 && ret_buf > ANLD_SDLOG_BUFFER_LEN) || (ret_buf < 0)) {
        LR_LOG_E("SD Log Drop!");
        return;
    }
    struct tm local;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    // INT_LOG("%ld", time(0));
    // INT_LOG("%ld", spec.tv_sec);
    localtime_r(&(spec.tv_sec), &local);
    int ret_hdr = snprintf(header, ANLD_SDLOG_HEADER_LEN, "[%ld.%03ld[%s]] ",
                           spec.tv_sec, spec.tv_nsec / 1000000, local.tm_zone);
    if ((ret_hdr > 0 && ret_hdr > ANLD_SDLOG_HEADER_LEN) || (ret_hdr < 0)) {
        LR_LOG_E("SD Log Drop!");
        return;
    }
    sSdLogMutex.lock();
    if (sSdLogRelayer == nullptr) {
        sSdLogMutex.unlock();
        return;
    }
    sSdLogRelayer->relayLog(header, ret_hdr);
    sSdLogRelayer->relayLog(buf, ret_buf);
    sSdLogMutex.unlock();
    return;
}
bool LogRelay::initSdLog() {
    std::lock_guard<std::mutex> locker(sSdLogMutex);
    if (sSdLogRelayer) {
        delete (sSdLogRelayer);
    }
    LR_LOG_D("=====Init SD Card Log (%s)=====", SD_CARD_DIR);
    DIR *d = opendir(SD_CARD_DIR);
    if (d == NULL) {
        LR_LOG_E("=====Init SD Card: read dir error(%d) =====", errno);
        return false;
    }
    dirent *dic;
    char logPath[256];
    bool found = false;
    while ((dic = readdir(d)) != NULL) {
        LR_LOG_D("Dir:%s", dic->d_name);
        if (strcmp(dic->d_name, ".") == 0) {
            continue;
        }
        if (strcmp(dic->d_name, "..") == 0) {
            continue;
        }
        if (strcmp(dic->d_name, "self") == 0) {
            continue;
        }
        if (strcmp(dic->d_name, "emulated") == 0) {
            continue;
        }
        snprintf(logPath, sizeof(logPath), "%s/%s/anld_log", SD_CARD_DIR,
                 dic->d_name);
        found = true;
        break;
    }
    closedir(d);
    if (!found) {
        LR_LOG_E("=====Init SD Card: could not find a valid path =====");
        return false;
    }
    int ret = mkdir(logPath, 0777);
    if (ret != 0 && errno != EEXIST) {
        LR_LOG_D("SD Card Log Init Error(%s)!! %d", logPath, errno);
        return false;
    }
    sSdLogRelayer = new LogRelay(logPath, "sd_log_", 20, 10 * 1024 * 1024);
    LR_LOG_D("---------------------------------------------------");
    LR_LOG_D("# Init SD Card Log Succes at %s  #", logPath);
    LR_LOG_D("--------------------------------------------------");
    return true;
}
bool LogRelay::deinitSdLog() {
    std::lock_guard<std::mutex> locker(sSdLogMutex);
    delete (sSdLogRelayer);
    sSdLogRelayer = nullptr;
    return true;
}
bool LogSlotReader::slotIntegrityCheck(const char *filename,
                                       size_t maxFileNum) {
    LR_LOG_D("IntergrityCheck:");
    FILE *fd = fopen(filename, "rb");
    size_t ret = 0;
    if (fd == nullptr) {
        LR_LOG_E("IntergrityCheck: failed at open slot");
        return false;
    }
    LogSlotHeader header;
    ret = fread(&header, 1, sizeof(header), fd);
    if (ret != sizeof(header)) {
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at read header");
        return false;
    }
    if (header.fileCount != maxFileNum) {
        fclose(fd);
        LR_LOG_E("IntergrityCheck: fileCount not match");
        return false;
    }
    // avoid new a 0 size array
    if (header.fileCount == 0) {
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at fileCount");
        return false;
    }
    if (header.reserved1 != INTEGRITY_CHECK_WORD) {
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at reserved word");
        return false;
    }
    size_t totalItemSize = sizeof(LogSlotItem) * header.fileCount;
    LogSlotItem *item = (LogSlotItem *)malloc(totalItemSize);
    ret = fread(item, 1, totalItemSize, fd);
    if (ret != totalItemSize) {
        free(item);
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at read Item");
        return false;
    }
    // crc32 check
    algorithm::CrcCalculator<uint32_t> crcInstance(0x04C11DB7, 0xFFFFFFFF,
                                                   0xFFFFFFFF, true, true);
    uint32_t crc32v = crcInstance.calculateCrc(&header, sizeof(header),
                                               crcInstance.getInitialValue());
    crc32v = crcInstance.calculateCrc(item, totalItemSize, crc32v);
    uint32_t checksum = 0;
    fseek(fd, sizeof(uint32_t) * -1, SEEK_END);
    ret = fread(&checksum, 1, sizeof(checksum), fd);
    if (ret != sizeof(checksum)) {
        free(item);
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at read checksum");
        return false;
    }
    if (checksum != crc32v) {
        free(item);
        fclose(fd);
        LR_LOG_E("IntergrityCheck: failed at crc32");
        return false;
    }
    free(item);
    fclose(fd);
    LR_LOG_E("IntergrityCheck: PASS");
    return true;
}
void LogRelay::restartTrace() {
    std::lock_guard<std::mutex> locker(traceMutex);
    if (traceLogRelayer) traceLogRelayer->restartLog();
}
void LogRelay::restartEvent() {
    std::lock_guard<std::mutex> locker(eventMutex);
    if (eventLogRelayer) eventLogRelayer->restartLog();
}

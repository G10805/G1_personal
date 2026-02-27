/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#define LOG_TAG "ADVENCED"
#include "AdvancedLogger.h"
#include <android/log.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <mutex>
#ifdef AIROHA_SIMULATION
#define INT_LOGD(format,...) printf(format "\n",##__VA_ARGS__)
#define INT_LOGI(format,...) printf(format "\n",##__VA_ARGS__)
#define INT_LOGW(format,...) printf(format "\n",##__VA_ARGS__)
#define INT_LOGE(format,...) printf(format "\n",##__VA_ARGS__)
#else
#define INT_LOGD(format, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, "ANLD_LOG", format, ##__VA_ARGS__)
#define INT_LOGW(format, ...) \
    __android_log_print(ANDROID_LOG_WARN, "ANLD_LOG", format, ##__VA_ARGS__)
#define INT_LOGE(format, ...) \
    __android_log_print(ANDROID_LOG_ERROR, "ANLD_LOG", format, ##__VA_ARGS__)
#endif
AdvancedLogger *AdvancedLogger::debuggerLogger = nullptr;
AdvancedLogger *AdvancedLogger::revLogger = nullptr;
AdvancedLogger *AdvancedLogger::uartLogger = nullptr;
AdvancedLogger::LogLevel AdvancedLogger::defaultLevel = AdvancedLogger::LogLevel::ADV_LOG_LEVEL_DEBUG;
char AdvancedLogger::sLogCatagory[12] = "ANLD_LOG";
std::mutex AdvancedLogger::sMutex;
AdvancedLogger::AdvancedLogger(const char * tDirName, const char *tPrefix, int mode){
    std::lock_guard<std::mutex> locker(sMutex);
    //if(access(dirName, F_OK) == -1){
    //    if(mkdir(dirName, 0777) != 0){
    //        INT_LOGE("FileDir Error! %s,%s,%d", dirName,strerror(errno),errno);
    //    }
    //}
    //TODO: max file path
    strncpy(dirName, tDirName, sizeof(dirName) - 1);
    strncpy(prefix, tPrefix, sizeof(prefix) - 1);
    initFile();
    if(mode == LogMode::MODE_ANLD_LOG){
        assert(debuggerLogger == NULL);
        debuggerLogger = this;
    }else if(mode == LogMode::MODE_UART_LOG){
        assert(revLogger == NULL);
        revLogger = this;
    }else if(mode == LogMode::MODE_UART_UNFORMAT_LOG){
        assert(uartLogger == NULL);
        uartLogger = this;
    }
    cmode = mode;

}
void AdvancedLogger::initFile() {
    memset(filePath, 0, sizeof(filePath));
    int ret = snprintf(filePath, 255, "%s%s", dirName, getFilenameByTime(prefix).c_str());
    if (ret >= 255) {
        INT_LOGE("init log file truncated occur");
    }
    filePointer = fopen(filePath, "wb+");
    if (filePointer == NULL) {
        INT_LOGE("File open error %s,ERROR:%s,%d", filePath, strerror(errno), errno);
    }
    else {
        INT_LOGE("=====================================\n");
        INT_LOGE("Advance Log initial at %s\n", filePath);
        INT_LOGE("=====================================\n");
    }
}
void AdvancedLogger::recreateFile() {
    if (filePointer) {
        fclose(filePointer);
    }
    else {
        INT_LOGW("previous file isn't exist");
    }
    filePointer = NULL;
    initFile();
}
bool AdvancedLogger::isValidLogger(){
    return (filePointer != NULL);
}
AdvancedLogger::~AdvancedLogger(){
    std::lock_guard<std::mutex> locker(sMutex);
    if(filePointer){
        fclose(filePointer);
        filePointer = NULL;
    }
    if(cmode == LogMode::MODE_ANLD_LOG){
        debuggerLogger = NULL;
    }else if(cmode == LogMode::MODE_UART_LOG){
        revLogger = NULL;
    }else if(cmode == LogMode::MODE_UART_UNFORMAT_LOG){
        uartLogger = NULL;
    }
}

void AdvancedLogger::advanceLog(LogLevel lev, const char *format,...){
    // avoid call this then deinit
    //  std::lock_guard<std::mutex> locker(sMutex);
    char timeStr[35] = {0};
    char logStr[1030];
    struct tm local;
    struct timespec spec;
    if(lev < defaultLevel || defaultLevel == ADV_LOG_LEVEL_NONE){
        return;
    }
    clock_gettime(CLOCK_REALTIME, &spec);
    //INT_LOG("%ld", time(0));
    //INT_LOG("%ld", spec.tv_sec);
    localtime_r(&(spec.tv_sec),&local);
    snprintf(timeStr,34,"%04d-%02d-%02d %02d:%02d:%02d.%03ld[%s]", 
                         local.tm_year + 1900, local.tm_mon+1, local.tm_mday,
                        local.tm_hour, local.tm_min, local.tm_sec, spec.tv_nsec/1000000, local.tm_zone);
    //snprintf(timeStr,29,"%d",local.tm_mday);
    char output[1600];
    va_list ap;
    va_start(ap, format);
    vsnprintf(logStr,1020,format,ap);
    va_end(ap);


    snprintf(output,1600, "%s %s\n", timeStr, logStr);
#ifdef AIROHA_SIMULATION
    INT_LOGD("%s %s", timeStr, logStr);
    fflush(stdout);
#else
    switch (lev){
        case LogLevel::ADV_LOG_LEVEL_DEBUG:
            __android_log_print(ANDROID_LOG_DEBUG, sLogCatagory, "%s", logStr);
            break;
        case LogLevel::ADV_LOG_LEVEL_INFO:
            __android_log_print(ANDROID_LOG_INFO, sLogCatagory, "%s", logStr);
            break;
        case LogLevel::ADV_LOG_LEVEL_WARNING:
            __android_log_print(ANDROID_LOG_WARN, sLogCatagory, "%s", logStr);
            break;
        case LogLevel::ADV_LOG_LEVEL_ERROR:
            __android_log_print(ANDROID_LOG_ERROR, sLogCatagory, "%s", logStr);
            break;
        default:
            break;
    }

#endif
    if(!debuggerLogger) return;
    debuggerLogger->redirectLog((uint8_t*)output,strlen(output));
    return;
}
void AdvancedLogger::advanceRawData(uint8_t *data, size_t length){
    if(!revLogger) return;
    revLogger->redirectRawData(data,length);
    return;
}
void AdvancedLogger::advanceUart(uint8_t *data, size_t length){
    if(!uartLogger) return;
    uartLogger->redirectRawData(data,length);
    return;

}
std::string AdvancedLogger::getFilenameByTime(const char *filenamePrefix){
    std::string filename = "";
    filename += filenamePrefix;
    time_t t = time(0);
    char time_str[30] = {0};
    strftime(time_str,30,"%Y_%m_%d_%H_%M_%S",localtime(&t));
    filename += time_str;
    filename += ".log";
    return filename;
}
void AdvancedLogger::redirectLog(uint8_t *data, size_t length){
    std::lock_guard<std::mutex> locker(insMutex);
    if (!filePointer) {
        return;
    }
    else {
        if (access(filePath, F_OK) != 0) {
            //handle file not exist
            recreateFile();
        }
        fwrite(data, 1, length, filePointer);
        fflush(filePointer);
    }

}
void AdvancedLogger::redirectRawData(uint8_t *data, size_t length){
    std::lock_guard<std::mutex> locker(insMutex);
    if (!filePointer) {
        return;
    }
    else {
        if (access(filePath, F_OK) != 0) {
            //handle file not exist
            recreateFile();
        }
        fwrite(data, 1, length, filePointer);
        fflush(filePointer);
    }
}
void AdvancedLogger::setLogLevel(LogLevel lev){
    defaultLevel = lev;
}
void AdvancedLogger::setLogCatagory(const char *catagory) {
    strncpy(sLogCatagory, catagory, sizeof(sLogCatagory));
}

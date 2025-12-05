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

#ifndef _ADVANCED_LOGGER_H
#define _ADVANCED_LOGGER_H
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <stdio.h>

#include <mutex>
#ifndef AIROHA_SIMULATION
#include <android/log.h>
// == Android Log Definition
#endif

class AdvancedLogger{

public:
    enum LogMode{
        MODE_ANLD_LOG = 0,
        MODE_UART_LOG = 1,
        MODE_UART_UNFORMAT_LOG = 2,
    };
    enum LogLevel{
        ADV_LOG_LEVEL_NONE,
        ADV_LOG_LEVEL_DEBUG,
        ADV_LOG_LEVEL_INFO,
        ADV_LOG_LEVEL_WARNING,
        ADV_LOG_LEVEL_ERROR,
    };
    AdvancedLogger();
    AdvancedLogger(const char * dirName, const char *prefix, int mode);
    bool isValidLogger();
    void redirectLog(uint8_t *data, size_t length);
    void redirectRawData(uint8_t *data, size_t length);
    ~AdvancedLogger();
    static void advanceLog(LogLevel lev, const char *format,...)__attribute__((format(printf,2,3)));
    static void advanceRawData(uint8_t *data, size_t length);
    static void advanceUart(uint8_t *data, size_t length);
    static std::string getFilenameByTime(const char *filenamePrefix);
    static void setLogLevel(LogLevel lev);
    static void setLogCatagory(const char *catagory);

 private:
    AdvancedLogger(const char * dirName, const char *fileName);
    FILE *filePointer;
    static LogLevel defaultLevel;
    static char sLogCatagory[12];
    static AdvancedLogger *debuggerLogger;
    static AdvancedLogger *revLogger;
    static AdvancedLogger *uartLogger;
    int cmode;
    static std::mutex sMutex;
    std::mutex insMutex;
    char filePath[256];
    char dirName[256];
    char prefix[50];
    void initFile();
    void recreateFile();
    

};




#endif
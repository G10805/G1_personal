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
#define LOG_TAG "PAIRD"
#include "PairUtil.h"
#include <airoha_std.h>
#include <assert.h>
#include <string.h>
#include "simulation.h"
PAIRData::PAIRData() {}
PAIRData::~PAIRData() {
    if (this->mParamList.size() == 0) {
        return;
    }
    for (char *x : mParamList) {
        assert(x);
        free(x);
    }
}
bool PAIRData::setCommand(const char* rawBytes) {
    // LOG_D("pair decode set command:%s", rawBytes);
    std::string raw = rawBytes;
    bool ret = string2vector(raw);
    if (ret == false) {
        LOG_E("Set command error!!:%s", rawBytes);
        commandNumber = INVALID_COMMAND_NUMBER;
        validCommand = false;
        return false;
    }
    const char* pairHeader = getParamByIndex(0);
    // LOG_D("PAIR header:%s", pairHeader);
    if (strlen(pairHeader) < 8) {
        LOG_E("PAIR header error!!:%s", pairHeader);
        validCommand = false;
        return false;
    }
    char headerStr[6] = {0};
    headerStr[0] = pairHeader[0];
    headerStr[1] = pairHeader[1];
    headerStr[2] = pairHeader[2];
    headerStr[3] = pairHeader[3];
    headerStr[4] = pairHeader[4];
    // int handleNum =
    // sscanf(pairHeader.c_str(),"%5s%3d",headerStr,&commandNumber);
    if (pairHeader[0] != '$') {
        LOG_E("PAIR command format ERROR,do not start with $ %s", headerStr);
        commandNumber = INVALID_COMMAND_NUMBER;
        validCommand = false;
        return false;
    }
    // if(strcmp(headerStr,"$PAIR") != 0){
    //     LOG_E("PAIR command format ERROR,%s",headerStr);
    //     commandNumber = INVALID_COMMAND_NUMBER;
    //     validCommand = false;
    //     return false;
    // }
    talker = std::string(headerStr + 1);
    commandNumber = pairHeader + 5;
    // LOG_D("PAIR Data: talker %s command Number:%s", talker.c_str(), commandNumber.c_str());
    validCommand = true;
    totalCmd = raw;
    return true;
}
const char* PAIRData::getCommand() const { return totalCmd.c_str(); }
const char* PAIRData::getCommandNubmber() const {
    return commandNumber.c_str();
}
int PAIRData::getCommandNubmberInt() const {
    if (isDigital(commandNumber)) {
        return stoi(commandNumber);
    } else {
        return -1;
    }
}
bool PAIRData::isDigital(const std::string& numString) {
    size_t length = numString.size();
    for (size_t i = 0; i < length; i++) {
        if (numString[i] >= '0' && numString[i] <= '9') {
            continue;
        } else {
            return false;
        }
    }
    return true;
}
const char* PAIRData::getParamByIndex(size_t index) const {
    if (index >= getParamSize()) {
        LOG_E("get param Error: index > size");
        return "";
    }
    return this->mParamList[index];
}
int PAIRData::getParamInt(size_t index) const {
    if (index > getParamSize()) {
        return 0;
    }
    int res = std::stoi(this->mParamList[index]);
    return res;
}
size_t PAIRData::getParamSize() const { return this->mParamList.size(); }
void PAIRData::getFullPairCommand(const char* buf, size_t bufLen, char* dstBuf,
                                  size_t dstBufLen) {
    // const int wait_ticket = 0xFFFFFFFF;
    // int32_t ret_len = 0;
    const char* ind;
    uint8_t checkSumL = 0, checkSumR;
    if (bufLen + 7 > dstBufLen) {
        assert(0);
        return;
    }
    ind = buf;
    while ((size_t)(ind - buf) < bufLen) {
        checkSumL ^= *ind;
        // LOG_D("checksumL = 0x%x",checkSumL);
        ind++;
    }
    dstBuf[0] = '$';
    memcpy(dstBuf + 1, buf, bufLen);
    dstBuf[bufLen + 1] = '*';
    checkSumR = checkSumL & 0x0F;
    checkSumL = (checkSumL >> 4) & 0x0F;
    dstBuf[bufLen + 2] =
        checkSumL >= 10 ? checkSumL + 'A' - 10 : checkSumL + '0';
    dstBuf[bufLen + 3] =
        checkSumR >= 10 ? checkSumR + 'A' - 10 : checkSumR + '0';
    dstBuf[bufLen + 4] = '\r';
    dstBuf[bufLen + 5] = '\n';
    dstBuf[bufLen + 6] = '\0';
}
bool PAIRData::string2vector(const std::string& sentence) {
    // unsigned long pos = 0;
    int posStart = 0;
    int posEnd = -1;
    int meetStar = 0;
    for (unsigned long i = 0; i < sentence.size(); i++) {
        if (sentence[i] != ',' && sentence[i] != '*') {
            continue;
        }
        if (sentence[i] == '*') {
            meetStar++;
            if (meetStar > 1) {
                LOG_W("nmea format error :%s", __FUNCTION__);
            }
        }
        posEnd = i;
        if (posEnd <= posStart) {
            LOG_E("string1vector wrong format!why code here?");
            return false;
        }
        if (posEnd - posStart == 1) {
            this->pushField("");
        } else {
            if (posStart == 0) {  // means first element
                this->pushField(sentence.substr(posStart, posEnd - posStart));
            } else {
                this->pushField(sentence.substr(posStart + 1, posEnd - posStart - 1));
            }
        }
        posStart = posEnd;
    }
    return true;
}
void PAIRData::pushField(const std::string &str) {
    size_t len = str.size() + 1;
    char* field = (char *)malloc(len);
    memset(field, 0, len);
    memcpy(field, str.data(), str.size());
    mParamList.push_back(field);
}
const char* PAIRData::getTalker() const { return talker.c_str(); }
#define MAX_SIMPLE_COMMAND_LENGTH 20
#define MAX_COMMAND_LENGTH_NOTICE \
    "Command Error: Command too long!!, max command length is 20"
SimpleCommand::SimpleCommand(const char* command, size_t length) {
    size_t i = 0;
    size_t suffixStart = 0;
    char num[20] = {0};
    bool suffixSet = false;
    error = ErrorCode::FATAL_ERROR;
    if (length > 20) {
        error = ErrorCode::COMMAND_TOO_LONG;
    }
    for (i = 0; i < length && command[i] != 0; i++) {
        if (!readableTest(command[i])) {
            error = ErrorCode::FORMAT_NOT_MATCH_FOR_CHAR_DETECT;
            return;
        }
        if (command[i] >= '0' && command[i] <= '9' && suffixSet == false) {
            suffixStart = i;
            suffixSet = true;
            if (suffixStart == 0) {
                error = ErrorCode::FORMAT_NOT_MATCH_FOR_STRUCT_DETECT;
                return;
            }
            prefix = std::string(command, suffixStart);
            break;
        }
    }
    size_t j = 0;
    for (; i < length && command[i] != 0; i++) {
        if (!(command[i] >= '0' && command[i] <= '9')) {
            error = ErrorCode::FORMAT_NOT_MATCH_FOR_CHAR_DETECT;
            return;
        }
        num[j] = command[i];
        j++;
    }
    num[j] = 0;
    if (j == 0) {
        num[0] = '0';
    }
    number = std::stoi(num);
    error = ErrorCode::NO_ERROR;
}
bool SimpleCommand::readableTest(char character) {
    if (character >= '0' && character <= '9') {
        return true;
    }
    if (character >= 'A' && character <= 'Z') {
        return true;
    }
    return false;
}
std::string SimpleCommand::getReason(ErrorCode code) {
    switch (code) {
        case ErrorCode::NO_ERROR:
            return "Command is OK!";
        case ErrorCode::FORMAT_NOT_MATCH_FOR_CHAR_DETECT:
            return "Command Error: Command can only contains 0-9(suffix) "
                   "A-Z(prefix)";
        case ErrorCode::FORMAT_NOT_MATCH_FOR_STRUCT_DETECT:
            return "Command Error: Command can be only XXXXyyy, X in A-Z, and "
                   "y in 0-9";
        case ErrorCode::COMMAND_TOO_LONG:
            return MAX_COMMAND_LENGTH_NOTICE;
        default:
            return "Command Error: Fatal Error 1";
    }
    return "Command Error: Fatal Error 2";
}
int SimpleCommand::getNumber() const { return number; }
SimpleCommand::ErrorCode SimpleCommand::getError() { return error; }
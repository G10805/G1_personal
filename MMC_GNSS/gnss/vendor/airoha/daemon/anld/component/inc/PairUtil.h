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

#ifndef PAIR_DECODE_H
#define PAIR_DECODE_H
#include <unistd.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <airoha_std.h>
enum PairNumber {
    PAIR_ACK = 1,
    PAIR_REQUEST_AIDING = 10,
    PAIR_INDICATION_SYSTEM_MESSAGE = 11,
    PAIR_SDK_VERSION_REPORT = 21,
    PAIR_TEST_CW_MODE = 393,
    PAIR_EPH_NOTIFY = 535,
    PAIR_EPH_GET_DATA = 532,
    PAIR_IO_SET_DATA_TYPE = 862,
};
/**
 * @brief Only handle command begin with $PAIR
 * 
 */
class PAIRData{
public:
    PAIRData();
    ~PAIRData();
    PAIRData(const PAIRData &) = delete;
    const PAIRData &operator=(const PAIRData &) = delete;
    // bool setCommand(std::string & raw); //deprecated
    bool setCommand(const char *raw);
    const char *getCommand() const;
    const char *getTalker() const;
    const char *getCommandNubmber() const;
    /**
     * @brief Get the Command Nubmber Int of a command
     * 
     * @return int 
     * if PAIR command is like PAIREPH, than this will return -1
     */
    int getCommandNubmberInt() const;
    /**
     * @brief Get the Param By Index object
     * 
     * @param index the index you want to query
     * @return std::string 
     * Notice: index 0 always represent PAIRxxx, and 1 is the first param
     */
    const char *getParamByIndex(size_t index) const;
    /**
     * @brief Get the Param in Int By Index object
     * 
     * @param index the index you want to query
     * @return std::string 
     * Notice: index 0 always represent PAIRxxx, and 1 is the first param
     */
    int getParamInt(size_t index) const;
    size_t getParamSize() const;
    static const int INVALID_COMMAND_NUMBER = -1; 
    static void getFullPairCommand(const char* buf, size_t bufLen, char* dstBuf, size_t tempBufLen);
private:
    std::string talker;
    std::string commandNumber;
    std::vector<char *> mParamList;
    bool string2vector(const std::string &sentence);
    void pushField(const std::string &str);
    bool validCommand;
    static bool isDigital(const std::string &numString);
    std::string totalCmd;
};

class SimpleCommand{
public:
    enum class ErrorCode : int{
        NO_ERROR,
        FORMAT_NOT_MATCH_FOR_CHAR_DETECT,
        FORMAT_NOT_MATCH_FOR_STRUCT_DETECT,
        FATAL_ERROR,
        COMMAND_TOO_LONG,
    };
    SimpleCommand(const char *command, size_t length);
    std::string getCommand();
    std::string getPrefix();
    int getNumber() const;
    ErrorCode getError();
    static std::string getReason(ErrorCode code);
private:
    std::string command;
    std::string prefix;
    int number;
    ErrorCode error;
    bool readableTest(char needCheck);
};



#endif
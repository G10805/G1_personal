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
/**
 * Mini-Loader is a small host that receive GPS data, which allow developers to
 * debug the serial port, or easier read data from GPS when main driver is not
 * start.
 */
#ifndef MINI_LOADER_H
#define MINI_LOADER_H
#include <unistd.h>
#include <condition_variable>
#include <list>
#include <mutex>
#include <string>
#include "driver/comm/uart_driver.h"
using airoha::communicator::uart_size_t;
using airoha::communicator::UartDriver;
class MiniLoader : public UartDriver {
    static const size_t MAX_VERSION_BUFFER_SIZE = 256;
    static const size_t MAX_RX_BUFFER_SIZE = 2048;

 public:
    MiniLoader();
    MiniLoader(const char *ttyName, int uartBaudrate,
               UartDriver::FlowControl fc);
    int runOnce();
    int runUntil(int maxCount);
    const std::string &getVersion();
    bool isNmeaReceived();
    bool isVersionReceived();

 protected:
    void onDataIn(const void *buffer, uart_size_t length) override;

 private:
    int queryVersion();
    int queryNmea();
    int run();
    bool waitVersion(int timeoutMs);
    bool waitNmea(int timeoutMs, const char *token);
    /** Data Parser Func Start */
    size_t inputAndParser(const void *buffer, size_t size);
    /**
     * @brief A simple function to parser nmea
     *
     * @return ssize_t valid position remain for next parse, -1 if no '$' found
     */
    ssize_t parser();
    ssize_t getDollerChar(size_t i);
    ssize_t getStarChar(size_t i);
    bool isValidNmea(size_t pos, size_t length);
    void pushNmea(size_t pos, size_t length);
    void moveHead(size_t start);
    /** Data Parser Func End */
    uint8_t mBuffer[MAX_RX_BUFFER_SIZE];
    size_t mCurrentWp;
    std::list<std::string> mValidNmeaList;
    std::string mVersion;
    std::condition_variable_any mNmeaGotCond;
    std::mutex mMutex;
    bool mNmeaReceived;
    bool mVersionReceived;
};
#endif

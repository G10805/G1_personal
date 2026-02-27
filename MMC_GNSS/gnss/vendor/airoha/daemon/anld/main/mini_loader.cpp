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
#define LOG_TAG "MiniLoader"
#include "mini_loader.h"
#include <string.h>
#include "component/inc/PairUtil.h"
#include "driver/gpio/GPIOControl.h"
#include "platform/inc/anld_customer_config.h"
#include "simulation.h"
#include "wakelock/air_wakelock.h"
using airoha::WakeLock;
// Init a uart driver with none flowcontrol
// About flow control: If the gps chip use hardware flowcontrol, here the
// parameter should be set to FC_HARDWARE, but for software flowcontrol it is
// not mandatory, the only thing to do is not sending some character like XON or
// XOFF, XESCAPE(0x77)
MiniLoader::MiniLoader()
    : UartDriver(AG3335_UART_DEV, 921600, UartDriver::FC_NONE) {
    LOG_D("Construct mini-loader");
    this->mCurrentWp = 0;
    mVersion = "UNKNOWN";
    mNmeaReceived = false;
    mVersionReceived = false;
    return;
}
MiniLoader::MiniLoader(const char *ttyName, int uartBaudrate,
                       UartDriver::FlowControl fc)
    : UartDriver(ttyName, uartBaudrate, fc) {
    LOG_D("Construct mini-loader with UART");
    this->mCurrentWp = 0;
    mVersion = "UNKNOWN";
    mNmeaReceived = false;
    mVersionReceived = false;
}
#define VERSION_RETRY_COUNT (5)
#define VERSION_RETRY_RESET_THRESHOLD (2)
#define POWER_ON_WAIT_TIME_MS (1000000)
#define POWER_OFF_WAIT_TIME_MS (20000)
int MiniLoader::runOnce() {
    LOG_D("Visteon: ##### GPS MINI LOADER START (Run Once)#####");
    LOG_D("##### GPS MINI LOADER START #####");
    Airoha::GPIO::portPowerOffChip();
    usleep(POWER_OFF_WAIT_TIME_MS);
    Airoha::GPIO::portPowerOnChip();
    usleep(POWER_ON_WAIT_TIME_MS);
    LOG_D("Visteon: uartDriver open #####");
    UartDriver::open();
    LOG_D("Visteon: uartDriver open done #####");
    queryVersion();
    queryNmea();
    UartDriver::close();
    Airoha::GPIO::portPowerOffChip();
    usleep(20000);
    LOG_D("##### GPS MINI LOADER EXIT #####");
    return 0;
}
int MiniLoader::runUntil(int maxCount) {
    LOG_D("Visteon: ##### GPS MINI LOADER START (Run Until)#####");
    WakeLock::getInstnace()->acquireLock(WakeLock::WAKELOCK_USER_MINILOADER);
    Airoha::GPIO::portPowerOffChip();
    usleep(POWER_OFF_WAIT_TIME_MS);
    int i = 0;
    while (i < maxCount) {
        Airoha::GPIO::portPowerOnChip();
        usleep(POWER_ON_WAIT_TIME_MS);
        LOG_D("Visteon: uartDriver open runUntil #####");
        UartDriver::open();
        LOG_D("Visteon: uartDriver open runUntil done #####");
        queryVersion();
        queryNmea();
        UartDriver::close();
        Airoha::GPIO::portPowerOffChip();
        usleep(20000);
        if (mNmeaReceived && mVersionReceived) break;
        i++;
    }
    WakeLock::getInstnace()->releaseLock(WakeLock::WAKELOCK_USER_MINILOADER);
    LOG_D("##### GPS MINI LOADER EXIT #####");
    return 0;
}
int MiniLoader::queryVersion() {
    int versionRetry = VERSION_RETRY_COUNT;
    const char *PAIR_GET_VERSION = "$PAIR020*38\r\n";
    while (versionRetry) {
        if (versionRetry <= VERSION_RETRY_RESET_THRESHOLD) {
            Airoha::GPIO::portResetChip();
            // Wait for power on because of reset.
            usleep(POWER_ON_WAIT_TIME_MS);
        }
        Airoha::GPIO::portGenerateInterrupt();
        uart_size_t sent = UartDriver::sendData(
            (const uint8_t *)PAIR_GET_VERSION, strlen(PAIR_GET_VERSION));
        if (sent != strlen(PAIR_GET_VERSION)) {
            LOG_E("send pair command error %zu/%zu", sent,
                  strlen(PAIR_GET_VERSION));
        }
        bool versionGot = waitVersion(500);
        if (versionGot) {
            LOG_I("successfully get version");
            mVersionReceived = true;
            break;
        };
        versionRetry--;
    }
    return 0;
}
int MiniLoader::queryNmea() {
    const char *PAIR_OPEN_DSP = "$PAIR002*38\r\n";
    Airoha::GPIO::portGenerateInterrupt();
    uart_size_t sent = UartDriver::sendData((const uint8_t *)PAIR_OPEN_DSP,
                                            strlen(PAIR_OPEN_DSP));
    if (sent != strlen(PAIR_OPEN_DSP)) {
        LOG_E("send pair command error %zu/%zu", sent, strlen(PAIR_OPEN_DSP));
        return -1;
    }
    mNmeaReceived = waitNmea(1500, "PAIR001,002,0");
    if (mNmeaReceived) {
        LOG_I("successfully get NMEA");
    };
    return 0;
}
void MiniLoader::onDataIn(const void *buffer, uart_size_t length) {
    size_t hasParse = 0;
    while (length > 0) {
        size_t acceptNum =
            inputAndParser((const uint8_t *)buffer + hasParse, length);
        hasParse += acceptNum;
        length -= acceptNum;
        if (acceptNum == 0) {
            // if no data is accept, means buffer full, just clear buffer
            mCurrentWp = 0;
        }
    }
}
size_t MiniLoader::inputAndParser(const void *buffer, size_t size) {
    size_t inputLength = 0;
    if (size + this->mCurrentWp > MAX_RX_BUFFER_SIZE) {
        inputLength = MAX_RX_BUFFER_SIZE - this->mCurrentWp;
    } else {
        inputLength = size;
    }
    memcpy(mBuffer + mCurrentWp, buffer, inputLength);
    mCurrentWp += inputLength;
    ssize_t ret = parser();
    if (ret > 0) {
        moveHead(ret);
    } else if (ret == -1) {
        // parse finish or no '$' found, clear all data
        mCurrentWp = 0;
    } else {
        LOG_W("inputAndParser: not expect parser return %zd", ret);
    }
    return inputLength;
}
ssize_t MiniLoader::parser() {
    size_t i = 0;
    while (i < mCurrentWp) {
        ssize_t posDoller = getDollerChar(i);
        if (posDoller == -1) {
            return -1;
        }
        ssize_t posStar = getStarChar((size_t)posDoller);
        if (posStar == -1) {
            return posDoller;
        }
        if ((size_t)posStar + 4 > mCurrentWp) {
            return posDoller;
        }
        // all pattern met, try validate nmea
        size_t nmeaLength = posStar - posDoller + 1 + 4;
        bool valid = isValidNmea((size_t)posDoller, nmeaLength);
        if (valid) {
            pushNmea((size_t)posDoller, nmeaLength);
        }
        i = posDoller + nmeaLength;
    }
    // if ccome here, just clear the whole buffer
    return -1;
}
ssize_t MiniLoader::getDollerChar(size_t i) {
    for (; i < mCurrentWp; i++) {
        if (mBuffer[i] == 0x24) {
            return (ssize_t)i;
        }
    }
    return -1;
}
ssize_t MiniLoader::getStarChar(size_t i) {
    for (; i < mCurrentWp; i++) {
        if (mBuffer[i] == (uint8_t)'*') {
            return (ssize_t)i;
        }
    }
    return -1;
}
bool MiniLoader::isValidNmea(size_t pos, size_t length) {
    if (length > 512) {
        LOG_E("nmea too long!");
        return false;
    }
    if (length < 7) {
        return false;
    }
    const char *p = (const char *)(mBuffer + pos);
    uint8_t checksum = 0;
    uint8_t checksumL = 0;
    uint8_t checksumR = 0;
    // char nmeaPureString[352] = {0};
    size_t i = 0;
    // $GNGGA,235942.012,,,,,0,0,,,M,,M,,*5E\r\n
    for (i = 1; i < length - 5; i++) {
        checksum ^= p[i];
    }
    i++;
    checksumL = (p[i] >= 'A') ? (p[i] - 'A' + 10) : p[i] - '0';
    checksumR = (p[i + 1] >= 'A') ? (p[i + 1] - 'A' + 10) : p[i + 1] - '0';
    if (checksum == ((checksumL << 4) | (checksumR))) {
        return true;
    }
    return false;
}
void MiniLoader::pushNmea(size_t pos, size_t length) {
    std::lock_guard<std::mutex> locker(mMutex);
    std::string s(((const char *)mBuffer) + pos, length);
    LOG_D("Push NMEA: %s", s.c_str());
    mValidNmeaList.push_back(s);
    mNmeaGotCond.notify_one();
}
void MiniLoader::moveHead(size_t start) {
    if (mCurrentWp < start) {
        LOG_E("moveHead unexpected value: %zu, %zu", start, mCurrentWp);
    }
    size_t validLength = mCurrentWp - start + 1;
    for (size_t i = 0; i < validLength; i++) {
        mBuffer[i] = mBuffer[start + i];
    }
}
bool MiniLoader::waitVersion(int timeoutMs) {
    bool gotVersion = false;
    std::lock_guard<std::mutex> locker(mMutex);
    std::chrono::time_point<std::chrono::steady_clock> startTime =
        std::chrono::steady_clock::now();
    while (1) {
        std::cv_status status = mNmeaGotCond.wait_until(
            mMutex, startTime + std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout) {
            LOG_D("timeout without getting valid nmea");
            break;
        }
        // if not cause by timeout, than we try to parse data.
        if (mValidNmeaList.size() == 0) {
            LOG_W("condition notified but nmea list size is 0");
            continue;
        }
        for (const std::string &x : mValidNmeaList) {
            if (x.find("$PAIR020") != std::string::npos) {
                // find version str
                PAIRData pair;
                pair.setCommand(x.c_str());
                mVersion = pair.getParamByIndex(1);
                gotVersion = true;
                break;
            }
        }
        if (gotVersion) {
            break;
        }
    }
    return gotVersion;
}
bool MiniLoader::waitNmea(int timeoutMs, const char *token) {
    std::lock_guard<std::mutex> locker(mMutex);
    std::chrono::time_point<std::chrono::steady_clock> startTime =
        std::chrono::steady_clock::now();
    while (1) {
        std::cv_status status = mNmeaGotCond.wait_until(
            mMutex, startTime + std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout) {
            LOG_D("timeout without getting valid nmea");
            break;
        }
        // if not cause by timeout, than we try to parse data.
        if (mValidNmeaList.size() == 0) {
            LOG_W("condition notified but nmea list size is 0");
            continue;
        }
        for (const std::string &x : mValidNmeaList) {
            if (x.find(token) != std::string::npos) {
                // find version str
                return true;
            }
        }
    }
    return false;
}
const std::string &MiniLoader::getVersion() { return mVersion; }
bool MiniLoader::isNmeaReceived() { return mNmeaReceived; }

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
#ifndef __DEFAULT_PLATFORM_H
#define __DEFAULT_PLATFORM_H
#include <airoha_std.h>
#include <mutex>
#include <vector>
#include "airo_gps.h"
#include "AdvancedLogger.h"
#include "AirohaDriver.h"
#include "anld_service_interface.h"
#include "spi_driver.h"
#include "uart_driver.h"
#include "anld_customer_config.h"
#include "platform_command.h"
#include "platform_virtual_driver.h"
#include "ConfigLoader.h"
#ifndef ANLD_CUSTOMER_CONFIG_INCLUDED
#error "Must Include Config File"
#endif
using Airoha::AirohaDriverInterface;
using Airoha::AnldServiceMessage;
using Airoha::IAnldServiceHandle;
using airoha::communicator::SpiDriver;
using airoha::communicator::UartDriver;
using LogUtil::LogRelay;
using Airoha::Gnss::PreGnssLocation;

int portDownloadChip();
int downloadChipByConfig();
class DefaultPlatform : public IAnldServiceHandle {
 public:
    DefaultPlatform(int debugPort, int PowerGPSPort);
    ~DefaultPlatform();
    DefaultPlatform() = delete;
    void setLogPath(const std::string &path);
    int onServiceMessage(AnldServiceMessage message, void *data,
                         ANL_SIZE_T len) override;
    void sendDebugPortMessage(const void *buffer, ANL_SIZE_T length);
    ssize_t sendPowerGPSMessage(const void *buffer, ANL_SIZE_T length);
    void setUartLogRecord(bool enable);
    LogRelay *getLogRelayer();
 private:
    enum class DebugMessageType {
        TYPE_PAIR,
        TYPE_PANL,
    };
    struct Position {
        Position() {
            latitude = 0.0;
            longitude = 0.0;
            altitude = 0.0;
            fix_quality = 0;
        }
        double latitude;
        double longitude;
        double altitude;
        uint8_t fix_quality;
    };
    int onDataOut(void *data, ANL_SIZE_T len);
    int onPowerOn();
    int onPowerOff();
    int onRTCWakeup();
    int onWakeUpChip();
    int onNmea(const char *buffer, ANL_SIZE_T length);
    int onPowerSuspend();
    int onPowerResume();
    int prepareDataConn();
    int suspendDataConn();
    int openDebugPort();
    int closeDebugPort();
    int onFactoryMessage(FactoryMessage *msg);
    int onDeleteAidingData();
    int epollFd;
    int debugServerFd;
    int debugClientFd;
    int powerGPSServerFd;
    int powerGPSClientFd;
    int signalFd;
    void handleDebugServerMsg(bool isClientIn);
    void handlePowerGPSMsg(bool isClientIn);
    pthread_t platformThread;
    static void *platformThreadFunc(void *);
    int debugPort;
    int powerGPSPort;
    void handleDebugMessage(DebugMessageType type, char *buf, int len);
    void rawPAIRHandler(char *buf, int len);
    void userSimpleCmdHandler(char *buf, int len);
    std::vector<int> pairlist;
    std::vector<int> pairWaitAckList;
    std::vector<int> pairForceAckList;
    void checkAppPairResponse(const std::string &data);
    void checkAppPairAck(std::string &data, int ackCmd, int status);
    // Interrupt Handler
    // compatible with /dev/airoha_gps
    int gpsIntFd;
    std::string mLogPath;
    void openGpsInterrupt();
    void closeGpsInterrupt();
    void handleGpsInterrupt();
    void lockLinuxPower();
    void unlockLinuxPower();
    void showOpenInfo();
    void showCloseInfo();
    void handleUartStatusChange();
    void platformSetUartActive(bool active);
    void handlePropMsgRelay(const PropMessageRelayPayload *);
    void initPlatformLocation();
    void loadPlatformLocationFromConfig();
    void fillPlatformLocation(double latitude, double longitude,
                              double altitude, bool valid = true);
    UartVirtualDriver *mUartComm;
    SpiVirtualDriver *mSpiComm;
    IComVirtualDriver *mComVirtualDriver;
    LogRelay *comRelayer;
    bool logEnable;
    SimpleCommandHandler *mSimCmdHdr;
    
#define UART_PWR_OWNER_DATA_CONNECT (1 << 0)
#define UART_PWR_OWNER_WAKE_LOCK (1 << 1)
#define UART_PWR_OWNER_USER_OPEN (1 << 2)
#define UART_PWR_OWNER_NEED_OPEN_MASK                         \
    (UART_PWR_OWNER_DATA_CONNECT | UART_PWR_OWNER_USER_OPEN | \
     UART_PWR_OWNER_WAKE_LOCK)
#define UART_PWR_OWNER_NEED_LOCK_MASK \
    (UART_PWR_OWNER_DATA_CONNECT | UART_PWR_OWNER_WAKE_LOCK)
    uint32_t uart_pwr_owner;
    bool uart_lock_status;
    bool data_conn_status;
    /**
     * Instance DATA
     */
    double mInstClockDrift;
    Position mInstPosition;
    /**
     * Instance DATA End
     */
    PreGnssLocation mPlatformLocation;
};
const char *platformMessageEnumToString(AnldServiceMessage);

#endif
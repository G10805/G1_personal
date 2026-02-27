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
#pragma once
#include "LogRelayer.h"
#include "spi_driver.h"
#include "uart_driver.h"
/**
 * Uart Sanity Check for following condition.
 * 1. uart must be LOCK when data in.
 * 2. uart must be LOCK when data out.
 * 3. uart will be auto unlock when close.
 */
// #define UART_SANITY_CHECK
using airoha::communicator::spi_size_t;
using airoha::communicator::SpiDriver;
using airoha::communicator::uart_size_t;
using airoha::communicator::UartDriver;
using LogUtil::LogRelay;
// pre define header
class DefaultPlatform;
enum PlatformComStatus {
    PSC_OK,
    PSC_IO_ERROR,
};
class IComVirtualDriver {
    /**
     * Power Level:
     *      connect -> power resume -> sendData

     * *      if sendData Call before power resume, it will return failed
 */
 public:
    enum ExtraCommand {
        EXTRA_COMMAND_PORT_POWER_ON,
        EXTRA_COMMAND_PORT_POWER_OFF,
    };
    // Config
    // static const size_t kMaxLogFileNum = 20;
    // static const size_t kMaxSingleLogFileSize = (5 * 1024 * 1024);
    IComVirtualDriver();
    virtual PlatformComStatus connect() = 0;
    virtual PlatformComStatus disconnect() = 0;
    virtual PlatformComStatus sendData(const void *buf, size_t length) = 0;
    virtual PlatformComStatus powerSuspend() = 0;
    virtual PlatformComStatus powerResume() = 0;
    virtual PlatformComStatus interrupt() = 0;
    virtual PlatformComStatus prepareSession();
    /**
     * This function is call when dsp on.
     * because in this case data is continously comming,
     * and chip may not sleep.
     * \return
     */
    virtual PlatformComStatus consecutiveDataOpen() = 0;
    virtual PlatformComStatus consecutiveDataClose() = 0;
    virtual PlatformComStatus onExtraCommand(ExtraCommand command);
    virtual ~IComVirtualDriver() {}
    void recordLog(const void *buffer, size_t length);
    void logRecordOpen(const char *userLogPath, const char *userPrefix,
                       size_t maxFileNum, size_t maxSingleFileSize);
    void logRecordClose();
    void logMessage(const char *message);

 private:
    LogRelay *relayer;
    pthread_mutex_t relayMutex;
};
class UartVirtualDriver : public IComVirtualDriver, public UartDriver {
 public:
    UartVirtualDriver(DefaultPlatform* parent, const char *devName, int baudrate, FlowControl fc);
    PlatformComStatus connect() override;
    PlatformComStatus disconnect() override;
    PlatformComStatus sendData(const void *buf, size_t length) override;
    PlatformComStatus powerSuspend() override;
    PlatformComStatus powerResume() override;
    PlatformComStatus consecutiveDataOpen() override;
    PlatformComStatus consecutiveDataClose() override;
    PlatformComStatus interrupt() override;
    void setQcPlatformEnable(bool enable);
 protected:
    void onDataIn(const void *buffer, uart_size_t length) override;

 private:
    using UartLockFlags = uint8_t;
    enum UartLockFlag : uint8_t{
        ULF_POWER_RESUME_LOCK = 1 << 1,
        ULF_CONSECUTIVE_DATA_LOCK = 1 << 2,
    };
    void handleUartLock();
    void lockUart();
    void unlockUart();
    std::string uartSwFcTxConvert(const void *data, size_t length);
    void uartSwFcRxConvert(std::string &arr);
    pthread_mutex_t mMutex;
    bool mIsUartLock;
    bool mIsSwFlowControl;
    bool mEscapeCharMark;
    DefaultPlatform * mParent;
    UartLockFlags mLockFlags;
    // Sanity Check Member
#ifdef UART_SANITY_CHECK
    bool mSanityLocker;
#endif
    bool mIsQcPlatform;
};
class SpiVirtualDriver : public IComVirtualDriver, public SpiDriver {
 public:
    SpiVirtualDriver(const char *devName, DefaultPlatform *platform);
    void triggerReadOnce();
    // Function from IComVirtualDriver
    PlatformComStatus connect() override;
    PlatformComStatus disconnect() override;
    PlatformComStatus sendData(const void *buf, size_t length) override;
    PlatformComStatus powerSuspend() override;
    PlatformComStatus powerResume() override;
    PlatformComStatus consecutiveDataOpen() override;
    PlatformComStatus consecutiveDataClose() override;
    PlatformComStatus interrupt() override;
    PlatformComStatus onExtraCommand(ExtraCommand command) override;

 protected:
    void onDataIn(const uint8_t *data, spi_size_t length) override;

 private:
    DefaultPlatform *mParent;
};

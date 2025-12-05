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
#ifndef PLATFORM_INC_DOWNLOAD_INTERFACE_H_
#define PLATFORM_INC_DOWNLOAD_INTERFACE_H_
// to get interface config
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>
#include "download_manager.h"
#define UART_RING_BUFFER_DEBUG
#define UART_RX_SUPPORT_RINGBUFFER (1)
#define UART_SUPPORT_HARDWARE_FLOW_CONTROL (0)
#define UART_SUPPORT_SOFTWARE_FLOW_CONTROL (1)
#if UART_SOFTWARE_FLOW_CONTROL_ENABLE && (!UART_RX_SUPPORT_RINGBUFFER)
#error "Software flowcontrol could not enable if not support ringbuffer"
#endif
/**
 * ========================
 * Interface With Power ON/OFF
 * =========================
 */
class BasicInterface: public IDownloadPlatformInterface {
 public:
    ErrorCode powerOn() override;
    ErrorCode powerOff() override;
    ErrorCode powerReset() override;
};
/**
 * ========================
 * Interface With Power ON/OFF  End =========
 * =========================
 */
/**
 * ===========================
 *   Uart Download Interface ==========================
 * ===========================
 */
class UartDownloadInterface : public BasicInterface {
    static const uint32_t default_delay_ms = 1000;
    static const pTime_t kDefaultTimeoutMs = 2000;
    static const dSize_t kDefaultWriteStep = 4096;
    static const dSize_t kUartTxEscapeBuffer = 4096;
    static const int kHandshakeBaudrate = B115200;
    static const int kDaBaudrate = B921600;

 public:
    class RBuffer {
     public:
        enum ErrorCode : int {
            RB_ERROR_BUFFER_FULL = -1,
            RB_ERROR_SYSTEM_ERROR = -2,
            RB_ERROR_PARAMETER_ERROR = -3,
            RB_ERROR_CANCELLATION_RECEIVED = -4,
            RB_ERROR_CANNOT_WRITE_FULL = -5,
            RB_ERROR_FATEL_SYSETM_ERROR = -10,
        };
        RBuffer(int bufferSize);
        ~RBuffer();
        /**
         * @brief write buffer to ringbuffer
         *
         * @param buf
         * @param length
         * @param fullWrite 1 means data[0:length-1] should be write totally.
         * 0 means allow a part of data to be write first, and return the
         * bytesWritten.
         * @param timeoutMs  0 means nonblock, -1 means infinity.
         * @return int if success, return the bytes write to buffer
         * if timeout happen or free size is not enough when fullWrite set to
         * True, return 0. 
         * if failed, return ErrorCode 
         * write function will try to wait until freeSize > length. 
         * If two thread call read at the same time, data maybe incontinous
         */
        int write(const void *buf, int length, bool fullWrite,
                  int timeoutMs);
        /**
         * @brief read bytes
         *
         * @param buf
         * @param toRead
         * @param fullRead
         * @param timeoutMs timeout ms, 0 means nonblock, -1 means infinity.
         * @return int num of bytes that has been read, 0 means buffer empty
         */
        int read(void *buf, int toRead, bool fullRead, int timeoutMs);
        /**
         * @brief wait for data
         *
         * @param timeoutMs -1 means infinite
         * @return int return how many bytes are in buffer. It should be >= 0,
         * if 0 means a timeout happen. if return value is < 0, it reference to
         * error in ErrorCode.
         * If two thread call read at the same time, a data corruption will
         * happen.
         */
        int waitForData(int timeoutMs);
        /**
         * @brief wait for empty
         *
         * @param timeoutMs -1 means infinite
         * @return int return free buffer size. It should be >= 0,
         * if 0 means a timeout happen. if return value is < 0, it reference to
         * error in ErrorCode.
         */
        int waitForFree(int timeoutMs);
        /**
         * @brief Cancel all the waiting process
         *
         * @return int
         */
        int setCancel(bool enable);
        int space();
        int used();
     private:
        int mCapacity;
        int getUsedSize();
        int getFreeSize();
        /**
         * @brief Read ringbuffer data
         * 
         * @param buffer 
         * @param readLength 
         * @return int the bytes read.
         */
        int pureRead(void *buffer, int readLength);
        /**
         * @brief write to ringbuffer
         * 
         * @param buffer 
         * @param writeLength 
         * @return int the bytes written.
         */
        int pureWrite(const void *buffer, int writeLength);
        uint8_t *mBuf;
        int mWp;
        int mRp;
        uint8_t mWrap;
        uint32_t mCalcellationMarker;
        static const uint32_t CANCELLATION_MARK_TRUE = 0xCA157542;
        static const uint32_t CANCELLATION_MARK_FALSE = 0x245751AC;
        pthread_cond_t mWaitDataCond;
        pthread_cond_t mWaitSpaceCond;
        pthread_mutex_t mRwMutex;
        pthread_condattr_t mCondAttr;
        bool mIsObjectInvalid;
    };

    explicit UartDownloadInterface(const char *devName);
    ErrorCode openPort() override;
    ErrorCode closePort() override;
    CharEx readU8() override;
    CharEx readU16() override;
    CharEx readU32() override;
    ErrorCode readBytes(void *buffer, dSize_t length,
                                uint32_t *bytesRead) override;
    ErrorCode sendU8(uint8_t dataU8) override;
    ErrorCode sendU16(uint16_t dataU16) override;
    ErrorCode sendU32(uint32_t dataU32) override;
    ErrorCode sendBytes(const void *buffer, dSize_t length) override;
    ErrorCode raiseSpeed(int baudrate) override;
    ErrorCode setFlowControl(FlowControl fc) override;
    int32_t getHandshakeDelayMs() override;
    int32_t getRWDelayMs() override;
    void notifyStatus(DownloadStatus status) override;
    ~UartDownloadInterface();
    void setQcPlatfrom(bool enable);
 private:
    char mDevName[200];
    int mUartFd;
    FlowControl mFlowControl;
    void reverse(uint8_t *to_reverse, dSize_t length);
    ErrorCode uartReadBytes(void *buffer, dSize_t length);
    ErrorCode uartWriteBytes(const void *buffer, dSize_t length, dSize_t *bytesWritten = nullptr);
    /**
     * @brief 
     * 
     * @param data the src data need to be escape
     * @param length length of src
     * @param dstBuffer dst buffer
     * @param dstLength size of dstbuffer
     * @param escapedLength how many src data is escape
     * @return int how many data valid in dstBuffer, -1 means error
     */
    int uartSwFcTxConvert(const void *data, int length,
                                                 void *dstBuffer, int dstLength,
                                                 int *escapedLength);
    /**
     * @brief 
     * 
     * @param data the src data need to be reverse-escape
     * @param length length of src
     * @param dstBuffer dst buffer
     * @param dstLength size of dstbuffer
     * @param escapedLength how many src data is reverse-escape
     * @return int int how many data valid in dstBuffer, -1 means error
     */
    int uartSwFcRxConvert(const void *data, int length,
                                                 void *dstBuffer, int dstLength,
                                                 int *escapedLength);
    // for uint8 read, there is a timeout, default is 50ms for brom,
    // and 1000s for da
    pTime_t mU8Timeout;
#if UART_RX_SUPPORT_RINGBUFFER
    int mSocketPair[2];
    RBuffer *mUartBuffer;
    pthread_t mReadThreadHdr;
    static void *uartReadThd(void *);
#endif
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL
    bool mEscapedMarker;
#endif
    bool mEnableQcPlatform;
};
/**
 * ===========================
 *   Uart Download Interface End ======================
 * ===========================
 */
/**
 * ===========================
 *   SPI Download Interface ==========================
 * ===========================
 */
class SpiDownloadInterface : public BasicInterface{
 public:
    SpiDownloadInterface(const char *devName);
    ErrorCode openPort() override;
    ErrorCode closePort() override;
    CharEx readU8() override;
    CharEx readU16() override;
    CharEx readU32() override;
    ErrorCode readBytes(void *buffer, dSize_t length,
                                uint32_t *bytesRead) override;
    ErrorCode sendU8(uint8_t dataU8) override;
    ErrorCode sendU16(uint16_t dataU16) override;
    ErrorCode sendU32(uint32_t dataU32) override;
    ErrorCode sendBytes(const void *buffer, dSize_t length) override;
    // only use in uart
    int32_t getHandshakeDelayMs() override;
    int32_t getRWDelayMs() override;
    ~SpiDownloadInterface();
    void notifyStatus(DownloadStatus status) override;

 private:
    char mDevName[200];
    ErrorCode spiReadBytes(void *buffer, dSize_t length);
    ErrorCode spiWriteBytes(const void *buffer, dSize_t length);
    ErrorCode spiTransfer(uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len);
    ErrorCode spiQuerySlaveStatus(uint8_t *status);
    uint8_t mTxBuffer[4097];
    uint8_t mRxBuffer[4097];
    int spiFd;
    bool mForceReverseBit;
    // spi cmd
    const static uint8_t kSpisCfgRdCmd = 0x02;
    const static uint8_t kSpisRdCmd = 0x81;
    const static uint8_t kSpisCfgWrCmd = 0x04;
    const static uint8_t kSpisWrCmd = 0x06;
    const static uint8_t kSpisWsCmd = 0x08;
    const static uint8_t kSpisRsCmd = 0x0a;
    const static uint8_t kSpisPowerOnCmd = 0x0e;
    const static uint8_t kSpisPowerOffCmd = 0x0c;
    const static uint8_t kSpisCtCmd = 0x10;
    const static uint32_t kSpiAddress = 0x55aa0000;
    const static uint32_t kSpisSlavevOnOffset = 0;
    const static uint32_t kSpisSlavevOnMask =
        (0x1 << kSpisSlavevOnOffset);
    const static uint32_t kSpisSlaveFifoReadyOffset = 2;
    const static uint32_t kSpisSlaveFifoReadyMask = (0x1 << kSpisSlaveFifoReadyOffset);
    const static uint32_t kSpisSlaveXferFinishOffset = 5;
    const static uint32_t kSpisSlaveXferFinishMask =
        (0x1 << kSpisSlaveXferFinishOffset);
    const static uint32_t kSpisSlaveReadErrorOffset = 3;
    const static uint32_t kSpisSlaveReadErrorMask = (0x1 << kSpisSlaveReadErrorOffset);
    const static uint32_t kSpisSlaveWriteErrorOffset = 4;
    const static uint32_t kSpisSlaveWriteErrorMask =
        (0x1 << kSpisSlaveWriteErrorOffset);
};
/**
 * ===========================
 *   SPI Download Interface End ======================
 * ===========================
 */
#endif  // PLATFORM_INC_DOWNLOAD_INTERFACE_H_
